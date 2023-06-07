#ifndef WIN_CONTEXT_NATIVE_HPP_INCLUDED
#define WIN_CONTEXT_NATIVE_HPP_INCLUDED

namespace win {

class context_native {
  public:
    context_native(const context_native&)     = default;
    context_native(context_native&&) noexcept = default;
    context_native& operator=(const context_native&)     = default;
    context_native& operator=(context_native&&) noexcept = default;

    virtual ~context_native() = default;
    context_native() = default;



    virtual void bind()    const = 0;
    virtual void release() const = 0;
};

}

#endif // WIN_CONTEXT_NATIVE_HPP_INCLUDED
