#ifndef PHODISPL_VIEW_TRANSFORM_HPP_INCLUDED
#define PHODISPL_VIEW_TRANSFORM_HPP_INCLUDED

#include "phodispl/animation.hpp"

#include <memory>
#include <optional>



struct view_info;
class frame;
class box;



enum class view_mode {
  fit,
  clip,
  custom,
};



class view_transform {
  public:
    explicit view_transform(std::shared_ptr<view_info>);



    void translate(float, float);
    void scale(float, float = 0.f, float = 0.f);

    void snap_absolute_scale(float);
    void snap_fit();
    void snap_clip();



    void update(const frame&);

    void invalidate() { valid_ = false; }

    [[nodiscard]] bool valid() const { return valid_; }



    void save_state();
    void restore_state();



    [[nodiscard]] bool take_change();

    [[nodiscard]] box to_box()const;



  private:
    struct vt {
      float scale {1.f};
      float cx    {0.f};
      float cy    {0.f};
    };

    friend vt mix(vt, vt, float);



    std::shared_ptr<view_info>          view_info_;

    view_mode                           view_mode_      {view_mode::fit};
    view_mode                           view_mode_saved_{view_mode::fit};

    animation<vt>                       trafo_;
    vt                                  trafo_saved_;

    std::optional<std::array<float, 2>> size_;

    bool                                valid_      = false;



    void recalculate(bool);
};

#endif // PHODISPL_VIEW_TRANSFORM_HPP_INCLUDED
