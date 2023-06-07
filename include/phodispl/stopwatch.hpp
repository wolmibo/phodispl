#ifndef PHODISPL_STOPWATCH_HPP_INCLUDED
#define PHODISPL_STOPWATCH_HPP_INCLUDED

#include <chrono>



class stopwatch {
  public:
    explicit stopwatch(bool running = true) {
      reset(running);
    }



    [[nodiscard]] bool running() const { return running_; }



    void reset(bool running) {
      running_ = running;
      offset_  = 0;
      if (running) {
        start_ = std::chrono::high_resolution_clock::now();
      }
    }

    void resume() {
      running_ = true;
      start_   = std::chrono::high_resolution_clock::now();
    }

    void pause() {
      running_ = false;
      offset_  = elapsed_mus();
    }



    void set_mus(uint64_t offset_mus) {
      reset(running_);
      offset_ = offset_mus;
    }

    void set_ms(uint64_t offset_ms) {
      set_mus(offset_ms * 1000);
    }



    [[nodiscard]] uint64_t current_ms() const {
      return current_mus() / 1000;
    }

    [[nodiscard]] uint64_t current_mus() const {
      return elapsed_mus();
    }





  private:
    uint64_t offset_  = 0;
    bool     running_ = true;

    std::chrono::high_resolution_clock::time_point start_;



    [[nodiscard]] uint64_t elapsed_mus() const {
      return std::chrono::duration_cast<std::chrono::microseconds>
        (std::chrono::high_resolution_clock::now() - start_).count() + offset_;
    }
};

#endif // PHODISPL_STOPWATCH_HPP_INCLUDED
