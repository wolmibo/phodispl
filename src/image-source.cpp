#include "phodispl/image-source.hpp"

#include "phodispl/file-listing.hpp"

#include <algorithm>
#include <filesystem>
#include <mutex>
#include <optional>
#include <thread>

#include <logcerr/log.hpp>





namespace {
  template<typename Fnc, typename... Args>
  void invoke_save(Fnc&& cb, Args&&... args) {
    if (cb) {
      std::invoke(std::forward<Fnc>(cb), std::forward<Args>(args)...);
    }
  }
}



image_source::image_source(
    callback&&                         cb,
    std::vector<std::filesystem::path> fnames,
    const win::application&            app
) :
  callback_{std::move(cb)},

  cache_{
    [this](const auto& img, size_t prio) {
      schedule_image(img, prio);
    },
    [this](const auto& img, bool current) {
      unload_image(img, current);
    }
  },

  file_listing_{
    std::bind_front(&image_source::on_file_changed, this),
    std::move(fnames)
  },

  worker_thread_{
    [this, context = app.window().share_context()](const std::stop_token& stoken) {
      logcerr::thread_name("load");
      context.bind();
      logcerr::debug("entering load loop");
      this->work_loop(stoken);
      logcerr::debug("exiting load loop");
    }
  },

  filesystem_context_{app.window().share_context()},
  main_thread_id_    {std::this_thread::get_id()}
{
  std::unique_lock lock{cache_mutex_};

  if (auto initial = file_listing_.initial_file()) {
    cache_.add(*initial);
  }

  populate_cache(std::move(lock));
}





image_source::~image_source() {
  worker_thread_.request_stop();

  {
    std::lock_guard lock{scheduled_images_lock_};
    if (loading_image_) {
      (*loading_image_)->abort_loading();
    }
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  {
    std::unique_lock<std::mutex> lock{worker_mutex_};
    worker_wakeup_.notify_one();
  }
}





void image_source::unload_image(const std::shared_ptr<image>& image, bool /*current*/) {
  unschedule_image(image);
  image->clear();
}





void image_source::schedule_image(const std::shared_ptr<image>& image, size_t priority) {
  {
    std::lock_guard lock{scheduled_images_lock_};

    if (priority == 0) {
      for (auto& [prio, _]: scheduled_images_) {
        ++prio;
      }
    }

    auto it = std::ranges::find(scheduled_images_, image, &prio_shared_image::second);

    if (it == scheduled_images_.end()) {
      scheduled_images_.emplace_back(priority, image);
    } else if (it->first != priority) {
      it->first = priority;
    } else {
      return;
    }

    std::ranges::sort(scheduled_images_, std::ranges::greater(),
        &prio_shared_image::first);

    if (!scheduled_images_.empty()) {
      if (scheduled_images_.back().first == 0 &&
          loading_image_ &&
          scheduled_images_.back().second != *loading_image_) {

        (*loading_image_)->abort_loading();
        aborted_loading_ = true;
      }
    }

    auto jt = std::ranges::find(unscheduled_images_, image.get());
    if (jt != unscheduled_images_.end()) {
      std::swap(*jt, unscheduled_images_.back());
      unscheduled_images_.pop_back();
    }
  }

  std::unique_lock<std::mutex> lock{worker_mutex_};
  worker_wakeup_.notify_one();
}





void image_source::unschedule_image(const std::shared_ptr<image>& image) {
  std::lock_guard lock{scheduled_images_lock_};

  auto it = std::ranges::find(scheduled_images_, image, &prio_shared_image::second);
  if (it != scheduled_images_.end()) {
    if (loading_image_ && *loading_image_ == it->second) {
      (*loading_image_)->abort_loading();
      aborted_loading_ = true;
    }

    scheduled_images_.erase(it);
  } else if (std::ranges::find(unscheduled_images_, image.get())
             == unscheduled_images_.end()) {
    unscheduled_images_.emplace_back(image.get());
  }
}





std::shared_ptr<image> image_source::next_scheduled_image() {
  std::lock_guard<std::mutex> lock{scheduled_images_lock_};

  if (scheduled_images_.empty()) {
    return {};
  }

  auto img = std::move(scheduled_images_.back().second);
  scheduled_images_.pop_back();

  loading_image_ = img;

  return img;
}





void image_source::work_loop(const std::stop_token& stoken) {
  while (true) {
    while (!stoken.stop_requested()) {
      if (auto img = next_scheduled_image()) {
        if (!(*img)) {
          logcerr::debug("loading \"{}\"({})", img->path().string(),
              reinterpret_cast<std::intptr_t>(img.get())); // NOLINT(*reinterpret-cast)
          img->load();

          bool requires_recache{false};

          {
            std::lock_guard lock{scheduled_images_lock_};
            loading_image_.reset();
            requires_recache = std::exchange(aborted_loading_, false);

            if (std::ranges::find(unscheduled_images_, img.get())
                != unscheduled_images_.end()) {
              logcerr::debug("dropping image immediately after loading");
              img->clear();
            }
            unscheduled_images_.clear();
          }

          if (requires_recache) {
            logcerr::debug("re-caching after aborting loading");
            std::lock_guard lock{cache_mutex_};
            cache_.ensure_loaded();
          }
        }
      } else {
        break;
      }
    }

    if (stoken.stop_requested()) {
      break;
    }

    std::unique_lock<std::mutex> notify_lock{worker_mutex_};
    worker_wakeup_.wait(notify_lock);
  }
}





void image_source::next_image() {
  {
    std::lock_guard<std::mutex> lock{cache_mutex_};

    if (cache_.size() <= 1) {
      return;
    }

    cache_.next();

    invoke_save(callback_, cache_.current(), image_change::next);
  }

  file_listing_.demote_initial_file();
}



void image_source::previous_image() {
  {
    std::lock_guard<std::mutex> lock{cache_mutex_};

    if (cache_.size() <= 1) {
      return;
    }

    cache_.previous();

    invoke_save(callback_, cache_.current(), image_change::next);
  }

  file_listing_.demote_initial_file();
}



void image_source::reload_current() {
  std::lock_guard<std::mutex> lock{cache_mutex_};

  cache_.invalidate_current();

  invoke_save(callback_, cache_.current(), image_change::reload);
}





void image_source::populate_cache(std::unique_lock<std::mutex> cache_lock) {
  auto lock = std::move(cache_lock);

  bool first_sent = !cache_.empty();
  if (first_sent) {
    invoke_save(callback_, cache_.current(), image_change::next);
  }

  file_listing_.clear();
  auto files = file_listing_.populate();

  cache_.set(files);

  if (!first_sent && !cache_.empty()) {
    invoke_save(callback_, cache_.current(), image_change::next);
  }
}





void image_source::reload_file_list() {
  std::unique_lock lock{cache_mutex_};

  cache_.invalidate_all();
  populate_cache(std::move(lock));

  invoke_save(callback_, current(), image_change::reload);
}





std::shared_ptr<image> image_source::current() const {
  std::lock_guard lock{cache_mutex_};

  return cache_.current();
}






void image_source::on_file_changed(
    const std::filesystem::path& path,
    fs_watcher::action           action
) {
  std::lock_guard lock{cache_mutex_};

  std::optional<win::context_guard> context;
  if (std::this_thread::get_id() != main_thread_id_) {
    context.emplace(filesystem_context_);
  }

  auto current    = cache_.current();
  bool is_current = current && current->path() == path;


  if (action == fs_watcher::action::removed) {
    logcerr::debug("file removed: {}", path.string());

    cache_.remove(path);

    if (is_current) {
      invoke_save(callback_, cache_.current(), image_change::replace_deleted);
    }

  } else if (is_current) {
    logcerr::debug("file changed: {}*", path.string());

    cache_.invalidate_current();

    invoke_save(callback_, cache_.current(), image_change::reload);

  } else {
    logcerr::debug("file changed: {}", path.string());

    bool first = !current;

    cache_.add(path);

    if (first) {
      invoke_save(callback_, cache_.current(), image_change::next);
    }
  }
}
