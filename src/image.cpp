#include "phodispl/config.hpp"
#include "phodispl/image.hpp"

#include <gl/base.hpp>

#include <pixglot/decode.hpp>

#include <logcerr/log.hpp>



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

  loading_started_ = false;
  error_.reset();
  frames_.clear();
  ptoken_ = {};
}





namespace {
  [[nodiscard]] sequence_clock frame_seq_from_image(const pixglot::image& img) {
    std::vector<std::chrono::microseconds> times;
    times.reserve(img.frames.size());
    for (const auto& frame: img.frames) {
      times.emplace_back(frame.duration);
    }

    return sequence_clock{std::span{times}};
  }
}



void image::load() {
  if (loading_started_) {
    logcerr::debug("attempting to load \"{}\"", path_.string());
    return;
  }

  loading_started_ = true;

  auto weak_this = weak_from_this();
  ptoken_.frame_callback([weak_this](pixglot::frame& f) {
    if (auto lock = weak_this.lock()) {
      lock->append_frame(f);
    }
  });

  pixglot::output_format requested_format;
  requested_format.target    = pixglot::pixel_target::gl_texture;
  requested_format.alpha     = pixglot::alpha_mode::premultiplied;
  requested_format.gamma     = global_config().gamma;
  requested_format.endianess = std::endian::native;

  try {
    auto img = pixglot::decode(pixglot::reader{path_},
                 ptoken_.access_token(), requested_format);

    for (const auto& w: img.warnings) {
      logcerr::warn(w);
    }
    if (auto seq = frame_seq_from_image(img); !seq.equals_sequence(frame_sequence_)) {
      frame_sequence_ = std::move(seq);
      if (!img.animated) {
        frame_sequence_.pause();
      }
    }
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
}





std::shared_ptr<frame> image::current_frame() const {
  std::lock_guard lock{frames_mutex_};

  if (current_frame_ < frames_.size()) {
    return frames_[current_frame_];
  }
  return {};
}





void image::append_frame(pixglot::frame& f) {
  {
    std::lock_guard lock{frames_mutex_};

    frames_.emplace_back(std::make_shared<frame>(std::move(f)));
  }
  damage();
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
}





void image::toggle_animation() {
  if (frame_sequence_.paused()) {
    frame_sequence_.resume();
  } else {
    frame_sequence_.pause();
  }
}
