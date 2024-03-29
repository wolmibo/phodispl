// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_FADE_WIDGET_HPP_INCLUDED
#define PHODISPL_FADE_WIDGET_HPP_INCLUDED

#include "phodispl/animation.hpp"

#include <win/widget.hpp>



class fade_widget : public win::widget {
  public:
    fade_widget();

    void show();
    void hide();

    void lock();
    void unlock();

    [[nodiscard]] bool  visible() const { return visible_;  }
    [[nodiscard]] float opacity() const { return *opacity_; }
    [[nodiscard]] bool  locked () const { return locked_;   }



  private:
    bool             visible_   {false};
    bool             might_hide_{false};
    bool             locked_    {false};
    bool             wants_hide_{false};

    animation<float> opacity_;



    void update();
};

#endif // PHODISPL_FADE_WIDGET_HPP_INCLUDED
