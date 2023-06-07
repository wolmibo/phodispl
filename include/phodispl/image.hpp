#ifndef PHODISPL_IMAGE_HPP_INCLUDED
#define PHODISPL_IMAGE_HPP_INCLUDED

#include "phodispl/damageable.hpp"
#include "phodispl/frame.hpp"
#include "phodispl/sequence-clock.hpp"

#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>
#include <vector>

#include <pixglot/exception.hpp>
#include <pixglot/progress-token.hpp>



class image :
  public damageable,
  public std::enable_shared_from_this<image>
{
  public:
    image(const image&) = delete;
    image(image&&)      = delete;
    image& operator=(const image&) = delete;
    image& operator=(image &&)     = delete;

    ~image();



    [[nodiscard]] static std::shared_ptr<image> create(const std::filesystem::path&);



    void load();
    void update();
    void clear();

    void append_frame(pixglot::frame&);

    void next_frame()     { seek_frame(1);  }
    void previous_frame() { seek_frame(-1); }

    void toggle_animation();



    [[nodiscard]] const std::filesystem::path& path() const { return path_; }



    [[nodiscard]] operator bool()  const { return loading_started_;   }

    [[nodiscard]] float progress() const { return ptoken_.progress(); }
    [[nodiscard]] bool  loading()  const {
      return *this && !ptoken_.finished() && !error_;
    }



    [[nodiscard]] const pixglot::base_exception* error() const { return error_.get(); }



    [[nodiscard]] std::shared_ptr<frame> current_frame() const;

    [[nodiscard]] size_t frame_count()   const { return frames_.size(); }
    [[nodiscard]] size_t frame_index()   const { return current_frame_; }





  private:
    std::filesystem::path                    path_;

    std::atomic<bool>                        loading_started_{false};
    pixglot::progress_token                  ptoken_;
    std::unique_ptr<pixglot::base_exception> error_;

    std::vector<std::shared_ptr<frame>>      frames_;
    mutable std::mutex                       frames_mutex_;
    size_t                                   current_frame_{0};
    sequence_clock                           frame_sequence_;



    void seek_frame(ssize_t);

    explicit image(std::filesystem::path path) : path_{std::move(path)} {}
};





inline auto operator<=>(const image& lhs, const image& rhs) {
  return lhs.path() <=> rhs.path();
}

inline bool operator==(const image& lhs, const image& rhs) {
  return lhs.path() == rhs.path();
}

inline auto operator<=>(const image& lhs, const std::filesystem::path& rhs) {
  return lhs.path() <=> rhs;
}

inline bool operator==(const image& lhs, const std::filesystem::path& rhs) {
  return lhs.path() == rhs;
}



inline auto operator<=>(
    const std::shared_ptr<image>& img,
    const std::filesystem::path&  rhs
) {
  return img->path() <=> rhs;
}



inline bool operator==(
    const std::shared_ptr<image>& img,
    const std::filesystem::path&  rhs
) {
  return img->path() == rhs;
}

#endif // PHODISPL_IMAGE_HPP_INCLUDED
