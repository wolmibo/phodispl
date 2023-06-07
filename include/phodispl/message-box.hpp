#ifndef PHODISPL_MESSAGE_BOX_HPP_INCLUDED
#define PHODISPL_MESSAGE_BOX_HPP_INCLUDED

#include <filesystem>
#include <memory>
#include <optional>
#include <string>



namespace pixglot {
  class base_exception;
}

class viewport;
struct config;



class message_box {
  public:
    explicit message_box(std::shared_ptr<viewport> vp) :
      viewport_{std::move(vp)}
    {}



    void render(const std::string&, const std::string&, float=1.f) const;
    void render(const pixglot::base_exception&,
                const std::optional<std::filesystem::path>&, float=1.f) const;


  private:
    std::shared_ptr<viewport> viewport_;
};

#endif // PHODISPL_MESSAGE_BOX_HPP_INCLUDED
