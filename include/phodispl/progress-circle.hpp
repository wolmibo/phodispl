#ifndef PHODISPL_PROGRESS_CIRCLE_HPP_INCLUDED
#define PHODISPL_PROGRESS_CIRCLE_HPP_INCLUDED

#include "phodispl/box.hpp"

#include <chrono>
#include <memory>

#include <gl/program.hpp>



class viewport;

class progress_circle {
  public:
    explicit progress_circle(std::shared_ptr<viewport>);


    void render(float, float) const;



  private:
    std::shared_ptr<viewport> viewport_;

    gl::program               shader_;
    GLint                     shader_progress_uniform_  {-1};
    GLint                     shader_alpha_uniform_     {-1};
    GLint                     shader_continuous_uniform_{-1};
    box                       box_;

    std::chrono::high_resolution_clock::time_point start_;



    [[nodiscard]] std::chrono::milliseconds clock() const {
      return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start_
      );
    }
};

#endif // PHODISPL_PROGRESS_CIRCLE_HPP_INCLUDED
