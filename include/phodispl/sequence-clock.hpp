#ifndef PHODISPL_SEQUENCE_CLOCK_HPP_INCLUDED
#define PHODISPL_SEQUENCE_CLOCK_HPP_INCLUDED

#include <algorithm>
#include <chrono>
#include <span>
#include <vector>



class sequence_clock {
  public:
    using duration_type = std::chrono::microseconds;

    sequence_clock() :
      timestamps_{duration_type{0}},
      start_{now()}
    {}



    template<typename Rep, typename Period>
    explicit sequence_clock(std::span<std::chrono::duration<Rep, Period>> durations) :
      start_{now()}
    {
      timestamps_.reserve(durations.size() + 1);
      timestamps_.emplace_back(0);
      for (auto dur: durations) {
        append<Rep, Period>(dur);
      }
    }



    template<typename Rep, typename Period>
    void append(std::chrono::duration<Rep, Period> duration) {
      auto micros = std::chrono::duration_cast<duration_type>(duration);

      if (micros < duration_type{0}) {
        throw std::out_of_range{"sequence clock does not support negative timing"};
      }

      if (micros == duration_type{0}) {
        micros = duration_type{1};
      }
      timestamps_.push_back(timestamps_.back() + micros);
    }



    [[nodiscard]] size_t size() const {
      return timestamps_.size() - 1;
    }



    [[nodiscard]] duration_type duration() const {
      return timestamps_.back();
    }



    [[nodiscard]] duration_type position() const {
      if (auto dur = duration(); dur == duration_type{0}) {
        return dur;
      }
      return (clock() - start_) % duration();
    }



    [[nodiscard]] size_t position_index() const {
      auto next = std::ranges::upper_bound(timestamps_, position());

      return next - timestamps_.begin() - 1;
    }



    void reset() {
      start_ = now();
      pause_.reset();
    }

    void pause() {
      pause_ = clock();
    }

    void resume() {
      start_ += now() - clock();
      pause_.reset();
    }



    [[nodiscard]] bool paused() const {
      return pause_.has_value();
    }



    void set_index(size_t index) {
      if (index >= size()) {
        throw std::out_of_range("sequence_clock::set_index(index): index >= size()");
      }

      start_ = clock() - timestamps_[index];
    }



    [[nodiscard]] bool equals_sequence(const sequence_clock& clock) const {
      return std::ranges::equal(timestamps_, clock.timestamps_);
    }





  private:
    std::vector<duration_type>   timestamps_;
    duration_type                start_;
    std::optional<duration_type> pause_;



    [[nodiscard]] static duration_type now() {
      return std::chrono::duration_cast<duration_type>
        (std::chrono::high_resolution_clock::now().time_since_epoch());
    }



    [[nodiscard]] duration_type clock() const {
      return pause_.value_or(now());
    }
};

#endif // PHODISPL_SEQUENCE_CLOCK_HPP_INCLUDED
