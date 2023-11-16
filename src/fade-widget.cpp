#include "phodispl/fade-widget.hpp"

#include "phodispl/config.hpp"



fade_widget::fade_widget() :
  opacity_(
    0.f,
    global_config().animation_ui_fade_ms.count(),
    global_config().animation_ui_fade_curve
  )
{
  register_update_function(std::bind_front(&fade_widget::update, this));
}





void fade_widget::show() {
  wants_hide_ = false;

  if (visible_ && !might_hide_) {
    return;
  }

  opacity_.animate_to(1.f);

  visible_    = true;
  might_hide_ = false;
}





void fade_widget::hide() {
  if (locked_) {
    wants_hide_ = true;
    return;
  }

  if (!visible_ || might_hide_) {
    return;
  }

  opacity_.animate_to(0.f);

  might_hide_ = true;
}





void fade_widget::update() {
  if (might_hide_ && *opacity_ < 1e-4f) {
    might_hide_ = false;
    visible_    = false;
    opacity_.set_to(0.f);
    invalidate();
  }

  if (opacity_.changed()) {
    invalidate();
  }
}





void fade_widget::lock() {
  locked_ = true;

  show();

  invalidate();
}



void fade_widget::unlock() {
  locked_ = false;

  if (wants_hide_) {
    hide();
  }

  invalidate();
}
