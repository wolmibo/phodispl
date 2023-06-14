#include "phodispl/continuous-scale.hpp"

using namespace std::chrono;



continuous_scale::continuous_scale(milliseconds rate) :
  rate_       {static_cast<float>(duration_cast<microseconds>(rate).count())},
  last_sample_{steady_clock::now()}
{}





float continuous_scale::next_sample() {
  auto sample = steady_clock::now();

  float abs = duration_cast<microseconds>(sample - last_sample_).count() / rate_;

  last_sample_ = sample;

  return (movement_[move_up] ? abs : 0.f) - (movement_[move_down] ? abs : 0.f);
}
