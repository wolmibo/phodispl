#ifndef WIN_MOUSE_BUTTON_HPP_INCLUDED
#define WIN_MOUSE_BUTTON_HPP_INCLUDED

#include <cstdint>

#include <linux/input-event-codes.h>



namespace win {

enum class mouse_button : uint32_t {
  left    = BTN_LEFT,
  right   = BTN_RIGHT,
  middle  = BTN_MIDDLE,
  side    = BTN_SIDE,
  extra   = BTN_EXTRA,
  forward = BTN_FORWARD,
  back    = BTN_BACK,
  task    = BTN_TASK,
};

}

#endif // WIN_MOUSE_BUTTON_HPP_INCLUDED
