// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_CONTINUOUS_SCALE_HPP_INCLUDED
#define PHODISPL_CONTINUOUS_SCALE_HPP_INCLUDED

#include <bitset>
#include <chrono>
#include <utility>



class continuous_scale {
  public:
    enum class direction : size_t {
      up = 0,
      down = 1
    };



    continuous_scale(std::chrono::milliseconds);



    void activate_up()     { set(direction::up, true);  }
    void deactivate_up()   { set(direction::up, false); }

    void activate_down()   { set(direction::down, true);  }
    void deactivate_down() { set(direction::down, false); }

    void deactivate()      { movement_.reset(); }



    void set(direction dir, bool active = true) {
      movement_.set(std::to_underlying(dir), active);
    }



    [[nodiscard]] operator bool() const {
      return movement_.any();
    }





    [[nodiscard]] float next_sample();



  private:
    float                                 rate_;
    std::chrono::steady_clock::time_point last_sample_;
    std::bitset<2>                        movement_;
};

#endif // PHODISPL_CONTINUOUS_SCALE_HPP_INCLUDED
