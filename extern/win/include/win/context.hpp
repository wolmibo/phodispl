#ifndef WIN_CONTEXT_HPP_INCLUDED
#define WIN_CONTEXT_HPP_INCLUDED

#include "win/context-native.hpp"

#include <memory>
#include <utility>



namespace win {

class context {
  public:
    context() = default;

    explicit context(std::unique_ptr<context_native> cx) :
      context_{std::move(cx)}
    {}



    [[nodiscard]] const context_native* get() const { return context_.get(); }
    [[nodiscard]]       context_native* get()       { return context_.get(); }



    void bind() const {
      if (context_) {
        context_->bind();
      }
    }



    void release() const {
      if (context_) {
        context_->release();
      }
    }



  private:
    std::unique_ptr<context_native> context_;
};





class context_guard {
  public:
    context_guard(const context_guard&) = delete;

    context_guard(context_guard&& rhs) noexcept:
      context_{std::exchange(rhs.context_, nullptr)}
    {}



    context_guard& operator=(const context_guard&) = delete;

    context_guard& operator=(context_guard&& rhs) noexcept {
      cleanup();
      context_ = std::exchange(rhs.context_, nullptr);
      return *this;
    }



    ~context_guard() {
      cleanup();
    }

    explicit context_guard(context &con) :
      context_{&con}
    {
      context_->bind();
    }




  private:
    context* context_;

    void cleanup() {
      if (context_ != nullptr) {
        context_->release();
      }
    }
};

}

#endif // WIN_CONTEXT_HPP_INCLUDED
