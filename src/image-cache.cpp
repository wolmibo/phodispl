#include "phodispl/config.hpp"
#include "phodispl/image-cache.hpp"
#include "phodispl/image.hpp"
#include "win/key.hpp"

#include <algorithm>
#include <filesystem>




namespace {
  [[nodiscard]] const std::filesystem::path& path_of_shared_image(
      const std::shared_ptr<image>& img
  ) {
    return img->path();
  }
}





std::shared_ptr<image> image_cache::current() const {
  if (index_ < images_.size()) {
    return images_[index_];
  }
  return {};
}





void image_cache::remove(const std::filesystem::path& path) {
  auto it = std::ranges::lower_bound(images_, path, {}, &path_of_shared_image);

  if (it == images_.end() || (*it)->path() != path) {
    return;
  }

  if (unload_function_) {
    unload_unsafe(it - images_.begin());
  }

  if (index_ >= static_cast<size_t>(it - images_.begin()) && index_ > 0) {
    index_--;
  }
  images_.erase(it);

  ensure_loaded();
}





namespace {
  [[nodiscard]] std::optional<size_t> load_priority(
      size_t index,
      size_t current,
      size_t mod
  ) {
    if (mod == 0) {
      return {};
    }

    auto lf = global_config().cache_load_forward;
    auto lb = global_config().cache_load_backward;

    auto fw = (mod + index - current) % mod;
    auto bw = (mod + current - index) % mod;

    if (lf + lb + 1 >= mod) {
      if (fw < bw) {
        return 2 * fw - 1;
      }
      return 2 * bw;
    }

    if (fw <= lf) {
      return 2 * fw - 1;
    }

    if (bw <= lb) {
      return 2 * bw;
    }

    return {};
  }
}





void image_cache::load_maybe(size_t index) const {
  if (!load_function_ || *images_[index]) {
    return;
  }

  if (auto prio = load_priority(index, index_, images_.size())) {
    load_function_(images_[index], *prio);
  }
}





void image_cache::add(const std::filesystem::path& path) {
  auto it = std::ranges::lower_bound(images_, path, {}, &path_of_shared_image);

  if (it != images_.end() && (*it)->path() == path) {
    invalidate(it - images_.begin());
    return;
  }

  auto inserted = images_.emplace(it, image::create(path));
  load_maybe(inserted - images_.begin());

  cleanup(1);
}





void image_cache::invalidate(const std::filesystem::path& path) {
  auto it = std::ranges::lower_bound(images_, path, {}, &path_of_shared_image);

  if (it != images_.end() && (*it)->path() == path) {
    invalidate(it - images_.begin());
  }
}





void image_cache::invalidate_current() {
  invalidate(index_);
}





void image_cache::invalidate(size_t index) {
  if (index >= images_.size()) {
    return;
  }

  if (unload_function_) {
    unload_unsafe(index);
  }

  load_maybe(index);
}





void image_cache::invalidate_all() {
  auto mod = images_.size();

  if (mod == 0) {
    return;
  }

  if (unload_function_) {
    for (size_t i = 0; i < mod; ++i) {
      unload_unsafe(i);
    }
  }

  ensure_loaded();
}





void image_cache::load_unsafe(size_t index, size_t priority) const {
  if (!*images_[index]) {
    load_function_(images_[index], priority);
  }
}





void image_cache::unload_unsafe(size_t index) const {
  if (*images_[index]) {
    unload_function_(images_[index], index == index_);
  }
}





void image_cache::ensure_loaded() const {
  auto mod = images_.size();
  if (!load_function_ || mod == 0) {
    return;
  }

  auto lf = global_config().cache_load_forward;
  auto lb = global_config().cache_load_backward;

  if (lf + lb + 1 >= mod) {
    for (size_t i = 0; i < mod; ++i) {
      if (i % 2 == 1) {
        load_unsafe((index_ + i / 2 + 1) % mod, i);
      } else {
        load_unsafe((index_ + mod - i / 2) % mod, i);
      }
    }

  } else {
    for (size_t i = 0; i < lb + 1; ++i) {
      load_unsafe((index_ + mod - i) % mod, 2 * i);
    }

    for (size_t i = 0; i < lf; ++i) {
      load_unsafe((index_ + i + 1) % mod, 2 * i + 1);
    }
  }
}





void image_cache::cleanup(size_t margin) const {
  auto mod = images_.size();
  if (!unload_function_ || mod == 0) {
    return;
  }

  auto kf = global_config().cache_keep_forward;
  auto kb = global_config().cache_keep_backward;

  if (kf + kb >= mod) {
    return;
  }

  auto remaining = mod - kf - kb - 1;

  if (remaining >= 2 * margin) {
    for (size_t i = 0; i < margin; ++i) {
      unload_unsafe((index_ +        kf + i + 1 ) % mod);
      unload_unsafe((index_ + mod - (kb + i + 1)) % mod);
    }

  } else {
    for (size_t i = kf; i < kf + remaining; ++i) {
      unload_unsafe((index_ + i + 1) % mod);
    }
  }
}





void image_cache::seek(ssize_t diff) {
  ssize_t mod = images_.size();

  if (mod == 0) {
    index_ = 0;
    return;
  }

  index_ = ((static_cast<ssize_t>(index_) + diff) % mod + mod) % mod;

  cleanup(std::abs(diff));
  ensure_loaded();
}





void image_cache::set(std::span<const std::filesystem::path> new_files) {
  std::vector<std::shared_ptr<image>> new_images;
  new_images.reserve(new_files.size());

  std::optional<std::filesystem::path> current_path;
  if (index_ < images_.size()) {
    current_path = images_[index_]->path();
  }



  for (const auto& path: new_files) {
    auto it = std::ranges::lower_bound(images_, path, {}, &path_of_shared_image);
    if (it != images_.end() && (*it)->path() == path) {
      new_images.emplace_back(std::move(*it));
      images_.erase(it);
    } else {
      new_images.emplace_back(image::create(path));
    }
  }



  images_ = std::move(new_images);
  std::ranges::sort(images_, {}, &path_of_shared_image);

  if (current_path) {
    if (auto it = std::ranges::find(images_, *current_path, &path_of_shared_image);
        it != images_.end()) {

      index_ = it - images_.begin();
    } else {
      index_ = 0;
    }
  }

  cleanup(images_.size());
  ensure_loaded();
}
