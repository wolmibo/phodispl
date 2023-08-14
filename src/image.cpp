#include "phodispl/config.hpp"
#include "phodispl/image.hpp"

#include <chrono>

#include <gl/base.hpp>

#include <logcerr/log.hpp>

#include <pixglot/codecs.hpp>
#include <pixglot/codecs-magic.hpp>
#include <pixglot/decode.hpp>



namespace {
  template<typename Ptr>
  std::intptr_t ptr_to_int(Ptr p) {
    return reinterpret_cast<std::intptr_t>(p); // NOLINT(*reinterpret-cast)
  }
}





image::~image() {
  logcerr::debug("destroying image \"{}\" ({})", path_.string(), ptr_to_int(this));
}





std::shared_ptr<image> image::create(const std::filesystem::path& path) {
  std::shared_ptr<image> img{new image(path)};

  logcerr::debug("created new image \"{}\" ({})", path.string(), ptr_to_int(img.get()));

  return img;
}





void image::clear() {
  std::lock_guard lock{frames_mutex_};

  logcerr::debug("cleared image \"{}\" ({})", path_.string(), ptr_to_int(this));

  loading_started_  = false;
  loading_finished_ = false;
  error_.reset();
  frames_.clear();
  ptoken_ = {};

  codec_ = {};
  metadata_ = {};
}





void image::abort_loading() {
  if (loading()) {
    ptoken_.stop();
  }
}





namespace {
  [[nodiscard]] sequence_clock frame_seq_from_image(const pixglot::image& img) {
    std::vector<std::chrono::microseconds> times;
    times.reserve(img.size());
    for (const auto& frame: img.frames()) {
      times.emplace_back(frame.duration());
    }

    return sequence_clock{std::span{times}};
  }
}



void image::load() {
  if (loading_started_ || loading_finished_) {
    logcerr::debug("attempting to load \"{}\"", path_.string());
    return;
  }

  try {
    pixglot::reader reader{path_};
    std::vector<std::byte> buffer(pixglot::recommended_magic_size);
    std::ignore = reader.peek(buffer);
    codec_ = pixglot::determine_codec(buffer);

    loading_started_ = true;

    auto weak_this = weak_from_this();
    ptoken_.frame_begin_callback([weak_this](const pixglot::frame_view& f) {
      if (auto self = weak_this.lock()) {
        { std::lock_guard guard{self->frames_mutex_};
          self->frames_.emplace_back(f);

          self->frame_partial_last_update_ =
            self->frame_partial_load_begin_ = std::chrono::steady_clock::now();
        }
        self->damage();
      }
    });

    bool animated      {false};
    bool build_sequence{global_config().il_play_available &&
                        global_config().il_show_loading &&
                        frame_sequence_.size() == 0};


    ptoken_.frame_callback([weak_this, build_sequence, &animated](pixglot::frame& f) {
      if (auto self = weak_this.lock(); self && build_sequence) {
        self->frame_sequence_.append(f.duration());

        animated = animated || (f.duration() > std::chrono::milliseconds{0});

        if (!animated) {
          self->frame_sequence_.pause();
        }
      }
    });

    ptoken_.flush_uploads(global_config().il_partial_flush);

    pixglot::output_format requested_format;
    requested_format.storage_type(pixglot::storage_type::gl_texture);
    requested_format.alpha_mode  (pixglot::alpha_mode::premultiplied);
    requested_format.gamma       (global_config().gamma);

    auto img = pixglot::decode(reader, ptoken_.access_token(), requested_format);

    for (const auto& w: img.warnings()) {
      logcerr::warn(w);
    }
    if (auto seq = frame_seq_from_image(img); !seq.equals_sequence(frame_sequence_)) {
      frame_sequence_ = std::move(seq);
      if (!img.animated()) {
        frame_sequence_.pause();
      }
    }

    metadata_ = std::move(img.metadata());

  } catch (pixglot::decoding_aborted& ex) {
    logcerr::debug(ex.message());
    clear();
    return;
  } catch (pixglot::base_exception& ex) {
    logcerr::error(ex.message());
    error_ = ex.make_unique();
  } catch (std::exception& ex) {
    logcerr::error(ex.what());
    error_ = std::make_unique<pixglot::base_exception>(ex.what());
  } catch (...) {
    logcerr::error("unknown exception");
    error_ = std::make_unique<pixglot::base_exception>("unknown exception");
  }

  glFinish();
  damage();

  logcerr::debug("finished loading \"{}\"", path_.string());
  loading_started_ = loading_finished_ = true;
}





std::optional<pixglot::frame_view> image::current_frame() const {
  std::lock_guard lock{frames_mutex_};

  if (current_frame_ < frames_.size()) {
    return frames_[current_frame_];
  }
  return {};
}





void image::seek_frame(ssize_t offset) {
  std::lock_guard lock{frames_mutex_};

  if (auto s = frames_.size(); s > 1) {
    current_frame_ = (current_frame_ + s + offset) % s;

    frame_sequence_.pause();
    frame_sequence_.set_index(current_frame_);
    damage();
  }
}





void image::update() {
  std::lock_guard lock(frames_mutex_);

  auto next = frame_sequence_.position_index();
  damage(next != current_frame_);
  current_frame_ = next;


  if (global_config().il_partial && global_config().il_show_loading) {
    using namespace std::chrono;

    auto now = steady_clock::now();

    if (duration_cast<milliseconds>(now - frame_partial_load_begin_) >
        global_config().il_partial_threshold) {
      if (duration_cast<milliseconds>(now - frame_partial_last_update_) >
          global_config().il_partial_interval) {
        frame_partial_last_update_ = now;

        ptoken_.upload_available();
      }
    }
  }
}





void image::toggle_animation() {
  if (frame_sequence_.paused()) {
    frame_sequence_.resume();
  } else {
    frame_sequence_.pause();
  }
}
