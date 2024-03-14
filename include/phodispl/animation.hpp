#ifndef PHODISPL_ANIMATION_HPP_INCLUDED
#define PHODISPL_ANIMATION_HPP_INCLUDED

#include <chrono>
#include <cmath>
#include <numbers>
#include <utility>



enum class animation_curve {
  linear,
  immediate,
  sinusoidal,
};



inline float animation_interpolation_eval(animation_curve curve, float t) {
  switch (curve) {
    case animation_curve::linear:
      return t;
    case animation_curve::immediate:
      return 1.f;
    case animation_curve::sinusoidal:
      return std::sin(t * std::numbers::pi_v<float> * 0.5);
  }
  return 1.f;
}





class animation_time {
  public:
    animation_time(float duration, animation_curve interpolation) :
      duration_{duration * 1000.f}, curve_{interpolation}
    {}



    [[nodiscard]] float elapsed() const {
      return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start_
      ).count();
    }



    [[nodiscard]] float factor() const {
      if (curve_ == animation_curve::immediate) {
        return 1.0;
      }

      float t = elapsed() / duration_;
      if (t >= 1.0) {
        return 1.0;
      }

      return animation_interpolation_eval(curve_, t);
    }



    [[nodiscard]] float operator*() const {
      return factor();
    }



    [[nodiscard]] bool is_running() const {
      return curve_ != animation_curve::immediate
        && elapsed() <= duration_;
    }



    [[nodiscard]] operator bool() const {
      return is_running();
    }



    void start(bool immediate = false) {
      changed_ = true;

      if (immediate || curve_ == animation_curve::immediate) {
        start_ = {};
        return;
      }

      start_ = std::chrono::steady_clock::now();
    }



    [[nodiscard]] bool changed() {
      if (is_running()) {
        return true;
      }
      return std::exchange(changed_, false);
    }



  private:
    bool                                  changed_{true};
    float                                 duration_;
    animation_curve                       curve_;

    std::chrono::steady_clock::time_point start_;
};





template<typename T>
class animation {
  public:
    animation(const T& init, float duration, animation_curve curve) :
      clock_{duration, curve}, src_{init}, tgt_{init}
    {}

    animation(float duration, animation_curve curve) :
      clock_{duration, curve}
    {}



    [[nodiscard]] bool changed()          { return clock_.changed();          }
    [[nodiscard]] bool is_running() const { return clock_.is_running();       }
    [[nodiscard]] operator bool()   const { return static_cast<bool>(clock_); }

    [[nodiscard]] const animation_time& clock() const { return clock_; }



    T value() const {
      auto factor = clock_.factor();
      return (1.f - factor) * src_ + factor * tgt_;
    }

    T operator*() const { return value(); }



    void set_to(const T& tgt) {
      src_ = tgt_ = tgt;
      clock_.start(true);
    }


    void animate_to(const T& tgt) {
      src_ = **this;
      tgt_ = tgt;
      clock_.start();
    }



  private:
    animation_time clock_;

    T              src_{};
    T              tgt_{};
};

#endif // PHODISPL_ANIMATION_HPP_INCLUDED
