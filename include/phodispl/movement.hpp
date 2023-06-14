#ifndef PHODISPL_MOVEMENT_HPP_INCLUDED
#define PHODISPL_MOVEMENT_HPP_INCLUDED

#include <array>
#include <bitset>
#include <utility>



class movement {
  public:
    enum class direction : size_t {
      left  = 0,
      right = 1,
      up    = 2,
      down  = 3,
    };



    void set  (direction dir) { direction_.set  (std::to_underlying(dir)); }
    void reset(direction dir) { direction_.reset(std::to_underlying(dir)); }
    void clear()              { direction_.reset(); }

    [[nodiscard]] std::array<float, 2> to_vector() const;

    [[nodiscard]] operator bool() const { return direction_.any(); }



  private:
    std::bitset<4> direction_;
};

#endif // PHODISPL_MOVEMENT_HPP_INCLUDED
