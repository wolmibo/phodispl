#ifndef PHODISPL_IMAGE_HPP_INCLUDED
#define PHODISPL_IMAGE_HPP_INCLUDED

#include "phodispl/damageable.hpp"
#include "phodispl/sequence-clock.hpp"

#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>
#include <vector>

#include <pixglot/exception.hpp>
#include <pixglot/frame.hpp>
#include <pixglot/metadata.hpp>
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

    void next_frame()     { seek_frame(1);  }
    void previous_frame() { seek_frame(-1); }

    void toggle_animation();



    [[nodiscard]] const std::filesystem::path& path() const { return path_; }



    [[nodiscard]] operator bool()  const { return loading_started_;   }

    [[nodiscard]] float progress() const { return ptoken_.progress(); }
    [[nodiscard]] bool  loading()  const { return loading_started_ && !loading_finished_;}
    [[nodiscard]] bool  finished() const { return loading_finished_; }

    void abort_loading();



    [[nodiscard]] const pixglot::base_exception* error() const { return error_.get(); }



    [[nodiscard]] std::optional<pixglot::frame_view> current_frame() const;

    [[nodiscard]] size_t frame_count()   const { return frames_.size(); }
    [[nodiscard]] size_t frame_index()   const { return current_frame_; }



    [[nodiscard]] const pixglot::metadata& metadata() const { return metadata_; }
    [[nodiscard]] pixglot::codec           codec()    const { return codec_;    }





  private:
    std::filesystem::path                    path_;

    std::atomic<bool>                        loading_started_ {false};
    std::atomic<bool>                        loading_finished_{false};
    pixglot::progress_token                  ptoken_;
    std::unique_ptr<pixglot::base_exception> error_;

    std::vector<pixglot::frame_view>         frames_;
    mutable std::mutex                       frames_mutex_;
    size_t                                   current_frame_{0};
    sequence_clock                           frame_sequence_;

    std::chrono::steady_clock::time_point    frame_partial_load_begin_;
    std::chrono::steady_clock::time_point    frame_partial_last_update_;

    pixglot::metadata                        metadata_;
    pixglot::codec                           codec_{pixglot::codec::ppm};




    void seek_frame(ssize_t);

    explicit image(std::filesystem::path path) : path_{std::move(path)} {}
};

#endif // PHODISPL_IMAGE_HPP_INCLUDED
