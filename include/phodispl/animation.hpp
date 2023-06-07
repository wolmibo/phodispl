#ifndef PHODISPL_ANIMATION_HPP_INCLUDED
#define PHODISPL_ANIMATION_HPP_INCLUDED

#include "phodispl/animation-interpolation.hpp"

#include <chrono>
#include <utility>



constexpr float mix(float a, float b, float v) {
  return (1.0 - v) * a + v * b;
}



template<typename T>
class animation {
  public:
    animation(const T& init, float duration, animation_interpolation ipol) :
      src_{init}, tgt_{init}, duration_{duration}, ipol_{ipol}
    {}

    animation(float duration, animation_interpolation ipol) :
      duration_{duration}, ipol_{ipol}
    {}





    [[nodiscard]] float factor() const {
      if (ipol_ == animation_interpolation::immediate) {
        return 1.0;
      }

      float t = elapsed() / duration_;
      if (t >= 1.0) {
        return 1.0;
      }

      return animation_interpolation_eval(ipol_, t);
    }



    T operator*() const {
      return mix(src_, tgt_, factor());
    }



    [[nodiscard]] float elapsed() const {
      return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - start_
      ).count();
    }



    [[nodiscard]] bool changed() {
      if (is_running()) {
        return true;
      }
      return std::exchange(changed_, false);
    }





    [[nodiscard]] bool is_running() const {
      return ipol_ != animation_interpolation::immediate
        && elapsed() <= duration_;
    }

    operator bool() const {
      return is_running();
    }





    animation& operator=(const T& v) {
      work_in_progress_ = false;
      changed_          = true;

      src_ = tgt_ = v;

      return *this;
    }



    void animate_to(const T& tgt, bool immediate = false) {
      work_in_progress_ = false;
      if (immediate || ipol_ == animation_interpolation::immediate) {
        src_ = tgt_ = tgt;
      } else {
        src_   = **this;
        tgt_   = tgt;
        start_ = std::chrono::system_clock::now();
      }
      changed_ = true;
    }



    [[nodiscard]] T& edit() {
      if (!work_in_progress_) {
        tgt_ = src_ = **this;
        work_in_progress_ = true;
      }
      return tgt_;
    }





    T* operator->() {
      if (!work_in_progress_) {
        tgt_ = src_ = **this;
        work_in_progress_ = true;
      }
      return &tgt_;
    }



    void animate_edited(bool immediate = false) {
      work_in_progress_ = false;
      if (immediate || ipol_ == animation_interpolation::immediate) {
        src_ = tgt_;
      } else {
        start_ = std::chrono::system_clock::now();
      }
      changed_ = true;
    }





  private:
    T     src_{};
    T     tgt_{};
    float duration_;
    bool  changed_          = true;
    bool  work_in_progress_ = false;

    animation_interpolation               ipol_;
    std::chrono::system_clock::time_point start_{};
};

#endif // PHODISPL_ANIMATION_HPP_INCLUDED
