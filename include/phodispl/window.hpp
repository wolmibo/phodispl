#ifndef PHODISPL_WINDOW_HPP_INCLUDED
#define PHODISPL_WINDOW_HPP_INCLUDED

#include "phodispl/animation.hpp"
#include "phodispl/continuous-scale.hpp"
#include "phodispl/image-source.hpp"
#include "phodispl/image-view.hpp"
#include "phodispl/movement.hpp"
#include "phodispl/stopwatch.hpp"

#include <chrono>
#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

#include <win/application.hpp>



// NOLINTNEXTLINE(*macro*)
#define IMAGE_VIEW_TRAFO(action)       \
  image_view_primary_.trafo().action;   \
  image_view_secondary_.trafo().action;



class window : public win::application {
  public:
    window(const window&) = delete;
    window(window&&)      = delete;
    window& operator=(const window&) = delete;
    window& operator=(window&&)      = delete;

    ~window() override = default;

    explicit window(std::vector<std::filesystem::path>);



  private:
    std::shared_ptr<viewport>        viewport_;

    std::shared_ptr<view_info>       view_info_;

    image_source                     image_source_;
    image_view                       image_view_primary_;
    image_view                       image_view_secondary_;
    animation<float>                 image_view_blend_;

    stopwatch                        last_left_click_;

    bool                             dragging_               {false};
    bool                             pinching_               {false};
    double                           last_scale_             {1.0};
    double                           last_x_                 {0.0};
    double                           last_y_                 {0.0};
    movement                         continuous_movement_;
    uint64_t                         last_movement_          {0};

    float                            exposure_      {1.f};
    continuous_scale                 exposure_scale_{std::chrono::milliseconds{10}};



    enum class state : size_t {
      exposure_control = 0
    };

    std::bitset<1>                  state_;

    [[nodiscard]] bool state_active(state s) { return state_[std::to_underlying(s)]; }
    void activate_state  (state s) { state_.set(std::to_underlying(s));   }
    void deactivate_state(state s) { state_.reset(std::to_underlying(s)); }



    void toggle_scale_filter();

    void update_title();



    bool on_update() override;
    void on_render() override;

    void on_rescale(win::vec2<uint32_t> /*size*/, float /*scale*/) override;

    void on_key_press  (win::key /*keycode*/) override;
    void on_key_release(win::key /*keycode*/) override;
    void on_key_leave() override;
    void on_key_enter() override {}

    void on_scroll        (win::vec2<float> /*pos*/, win::vec2<float> /*dir*/) override;

    void on_pointer_press  (win::vec2<float> /*pos*/, win::mouse_button /*btn*/) override;
    void on_pointer_release(win::vec2<float> /*pos*/, win::mouse_button /*btn*/) override;

    void on_pointer_enter (win::vec2<float> /*pos*/) override {}
    void on_pointer_move  (win::vec2<float> /*pos*/) override;
    void on_pointer_leave()                          override {}

    void on_swipe_begin (win::vec2<float> /*pos*/, uint32_t /*count*/) override;
    void on_swipe_update(win::vec2<float> /*delta*/)                   override;
    void on_swipe_cancel() override;
    void on_swipe_finish() override;

    void on_pinch_begin(win::vec2<float> /*pos*/) override;
    void on_pinch_update(win::vec2<float> /*delta*/,
                                    float /*scale*/, float /*rotation*/) override;
    void on_pinch_cancel() override {};
    void on_pinch_finish() override {};
};

#endif // PHODISPL_WINDOW_HPP_INCLUDED
