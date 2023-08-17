#ifndef PHODISPL_WINDOW_HPP_INCLUDED
#define PHODISPL_WINDOW_HPP_INCLUDED

#include "phodispl/animation.hpp"
#include "phodispl/continuous-scale.hpp"
#include "phodispl/image-display.hpp"
#include "phodispl/image-source.hpp"
#include "phodispl/nav-button.hpp"

#include <chrono>
#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

#include <win/application.hpp>



class window : public win::application {
  public:
    window(const window&) = delete;
    window(window&&)      = delete;
    window& operator=(const window&) = delete;
    window& operator=(window&&)      = delete;

    ~window() override = default;

    explicit window(std::vector<std::filesystem::path>);



  private:
    image_display                    image_display_;
    image_source                     image_source_;

    nav_button                       nav_left_;
    nav_button                       nav_right_;

    std::chrono::steady_clock::time_point
                                     last_left_click_{};

    bool                             dragging_      {false};
    bool                             pinching_      {false};
    float                            last_scale_    {1.f};
    win::vec2<float>                 last_position_ {0.f, 0.f};
    continuous_scale                 zoom_scale_    {std::chrono::milliseconds{5}};
    continuous_scale                 move_x_scale_  {std::chrono::milliseconds{1}};
    continuous_scale                 move_y_scale_  {std::chrono::milliseconds{1}};


    continuous_scale                 exposure_scale_{std::chrono::milliseconds{10}};



    enum class input_mode {
      standard,
      exposure_control
    }                               input_mode_{input_mode::standard};

    void set_input_mode(input_mode);
    void clear_input_mode(input_mode);



    void update_title();



    void input_mode_scale(continuous_scale::direction, bool);

    void input_mode_apply_scale(int);


    void on_update() override;

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
    void on_swipe_cancel() override {};
    void on_swipe_finish() override;

    void on_pinch_begin(win::vec2<float> /*pos*/) override;
    void on_pinch_update(win::vec2<float> /*delta*/,
                                    float /*scale*/, float /*rotation*/) override;
    void on_pinch_cancel() override {};
    void on_pinch_finish() override {};
};

#endif // PHODISPL_WINDOW_HPP_INCLUDED
