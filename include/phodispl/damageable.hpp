#ifndef PHODISPL_DAMAGEABLE_HPP_INCLUDED
#define PHODISPL_DAMAGEABLE_HPP_INCLUDED

#include <atomic>



class damageable {
  public:
    [[nodiscard]] bool damaged()     const { return damaged_; }
    [[nodiscard]] bool take_damage()       { return damaged_.exchange(false); }

    void undamage()     { damaged_ = false; }

    void damage(bool d) { damaged_ = damaged_ || d; }
    void damage()       { damaged_ = true; }



  private:
    std::atomic<bool> damaged_ = true;
};

#endif // PHODISPL_DAMAGEABLE_HPP_INCLUDED
