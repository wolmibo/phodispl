// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef WIN_VIEWPORT_HPP_INCLUDED
#define WIN_VIEWPORT_HPP_INCLUDED

#include "win/widget.hpp"



namespace win {

class viewport : public widget {
  public:
    viewport(const viewport&) = delete;
    viewport(viewport&&)      = delete;
    viewport& operator=(const viewport&) = delete;
    viewport& operator=(viewport&&)      = delete;

    ~viewport() override = default;

    viewport() = default;





    void resize(vec2<float>, float);

    void render();
};

}

#endif // WIN_VIEWPORT_HPP_INCLUDED
