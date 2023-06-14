// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_CONTINUOUS_SCALE_HPP_INCLUDED
#define PHODISPL_CONTINUOUS_SCALE_HPP_INCLUDED

#include <bitset>
#include <chrono>



class continuous_scale {
  public:
    continuous_scale(std::chrono::milliseconds);



    void activate_up()     { movement_.set(move_up);     }
    void deactivate_up()   { movement_.reset(move_up);   }

    void activate_down()   { movement_.set(move_down);   }
    void deactivate_down() { movement_.reset(move_down); }

    void deactivate()      { movement_.reset();          }



    [[nodiscard]] float next_sample();



  private:
    static constexpr size_t move_up  {0};
    static constexpr size_t move_down{1};

    float                                 rate_;
    std::chrono::steady_clock::time_point last_sample_;
    std::bitset<2>                        movement_;
};

#endif // PHODISPL_CONTINUOUS_SCALE_HPP_INCLUDED
