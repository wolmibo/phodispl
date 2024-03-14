#include "phodispl/fs-watcher.hpp"

#include <algorithm>
#include <array>
#include <filesystem>

#include <fcntl.h>
#include <poll.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <logcerr/log.hpp>



fs_watcher::pipe_fd::pipe_fd() {
  std::array<int, 2> rw{};
  if (pipe(rw.data()) == 0) {
    read  = rw[0];
    write = rw[1];
  }
}



fs_watcher::pipe_fd::~pipe_fd() {
  if (read  >= 0) { close(read);  }
  if (write >= 0) { close(write); }
}





namespace {
  [[nodiscard]] int create_fd() {
    int fd = inotify_init();

    if (fd < 0) {
      logcerr::error("unable to initialize inotify");
      return -1;
    }

    // NOLINTNEXTLINE(*-vararg)
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) < 0) {
      logcerr::error("unable to set inotify to non-blocking");
      return -1;
    }

    return fd;
  }
}




fs_watcher::fs_watcher(callback&& cb) :
  callback_    {std::move(cb)},
  fd_          {create_fd()},

  watch_thread_{[this]() {
    logcerr::thread_name("fsys");
    logcerr::debug("entering watch loop");
    watch_loop();
    logcerr::debug("exiting watch loop");
  }}
{}





fs_watcher::~fs_watcher() {
  if (fd_ >= 0) {
    write(watch_pipe_.write, "done", 4);
  }
}





namespace {
  [[nodiscard]] std::array<pollfd, 2> create_poll_pair(int fd1, int fd2) {
    return {
      pollfd {
        .fd      = fd1,
        .events  = POLLIN,
        .revents = 0,
      },
      pollfd {
        .fd      = fd2,
        .events  = POLLIN,
        .revents = 0,
      }
    };
  }





  class event_reader {
    public:
      explicit event_reader(int fd) : fd_{fd} {}

      const inotify_event* next() {
        if (as_event() == nullptr) {
          fill_buffer();
        }

        if (valid_range_.empty()) {
          return nullptr;
        }

        if (const auto* event = as_event(); event != nullptr) {
          valid_range_ = valid_range_.subspan(sizeof(inotify_event) + event->len);
          return event;
        }

        throw std::runtime_error{"incomplete event in buffer"};
      }



    private:
      int fd_{-1};

      std::array<std::byte, 16 * (sizeof(inotify_event) + NAME_MAX + 1)> buffer_{};
      std::span<const std::byte>                                         valid_range_;



      [[nodiscard]] bool has_full_event() const {
        return valid_range_.size() >= sizeof(inotify_event) &&
          valid_range_.size() >= sizeof(inotify_event) + as_event()->len;
      }



      [[nodiscard]] const inotify_event* as_event() const {
        if (valid_range_.size() < sizeof(inotify_event)) {
          return nullptr;
        }

        //NOLINTNEXTLINE(*reinterpret-cast)
        const auto* event = reinterpret_cast<const inotify_event*>(valid_range_.data());

        if (valid_range_.size() < sizeof(inotify_event) + event->len) {
          return nullptr;
        }

        return event;
      }



      void move_range_to_start() {
        if (!valid_range_.empty() && valid_range_.data() != buffer_.data()) {
          std::ranges::copy(valid_range_, buffer_.begin());
          valid_range_ = std::span{buffer_.data(), valid_range_.size()};
        }
      }



      void fill_buffer() {
        move_range_to_start();

        int len = read(fd_, buffer_.data() + valid_range_.size(),
            buffer_.size() - valid_range_.size());

        if (len == -1) {
          if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
          }

          throw std::runtime_error{"error on reading on inotify fd"};
        }

        valid_range_ = std::span{buffer_.data(), valid_range_.size() + len};
      }
  };
}



void fs_watcher::watch_loop() {
  if (fd_ < 0) {
    return;
  }


  try {
    event_reader reader{fd_};

    auto events = create_poll_pair(fd_, watch_pipe_.read);

    while (true) {
      if (poll(events.data(), events.size(), -1) < 0) {
        throw std::runtime_error{"error on polling fd"};
      }

      if ((events[1].revents & POLLIN) != 0) {
        break;
      }

      if ((events[0].revents & POLLIN) != 0) {
        for (const auto* event = reader.next(); event != nullptr; event = reader.next()) {
          if (event->mask != 0) {
            std::lock_guard lock{mutex_};
            handle_event(event);
          }
        }
      }
    }

  } catch (std::exception& ex) {
    logcerr::error(ex.what());
  }

  close(fd_);
}





void fs_watcher::watch(std::span<const std::filesystem::path> list) {
  if (fd_ < 0) { return; }

  std::lock_guard lock{mutex_};

  for (const auto& path: list) {
    add_watch(path, true);
  }
}





void fs_watcher::unwatch() {
  if (fd_ < 0) { return; }

  std::lock_guard lock{mutex_};

  for (const auto& wi: file_watches_) {
    inotify_rm_watch(fd_, wi.wd);
  }
  file_watches_.clear();
}





namespace {
  [[nodiscard]] uint32_t create_mask(const std::filesystem::path& path, bool exp) {
    if (std::filesystem::is_directory(path)) {
      return IN_MOVED_TO | IN_MOVED_FROM | IN_DELETE_SELF | IN_CREATE;
    }

    if (exp) {
      return IN_MOVE_SELF | IN_CLOSE_WRITE | IN_DELETE_SELF;
    }

    return IN_CLOSE_WRITE | IN_DELETE_SELF;
  }
}



void fs_watcher::add_watch(const std::filesystem::path& path, bool exp) {
  auto it = std::ranges::find(file_watches_, path, &watch_item::path);
  if (it != file_watches_.end()) {
    return;
  }

  auto watch_path = std::filesystem::absolute(path);

  if (std::filesystem::is_directory(watch_path)) {
    for (const auto& iter: std::filesystem::directory_iterator(watch_path)) {
      if (!std::filesystem::is_directory(iter.path())) {
        add_watch(iter.path(), false);
      }
    }
  }


  auto mask = create_mask(watch_path, exp);

  if (int wd = inotify_add_watch(fd_, watch_path.string().c_str(), mask); wd >= 0) {
    file_watches_.emplace_back( watch_item {
      .wd   = wd,
      .path = std::move(watch_path)
    });
  }
}





fs_watcher::watch_iter fs_watcher::remove_watch(watch_iter it) {
  inotify_rm_watch(fd_, it->wd);

  std::swap(*it, file_watches_.back());
  file_watches_.pop_back();

  return it;
}





namespace {
  [[nodiscard]] std::filesystem::path get_path(
      const std::filesystem::path& parent,
      const inotify_event*         event
  ) {
    if (event->len == 0) {
      logcerr::warn("Got inotify event with empty name field");
      return parent;
    }

    std::string_view name{static_cast<const char*>(event->name), event->len};

    return parent / std::filesystem::path(name);
  }
}



void fs_watcher::handle_event(const inotify_event* event) {
  auto it = std::ranges::find(file_watches_, event->wd, &watch_item::wd);

  if (it != file_watches_.end()) {
    if (std::filesystem::is_directory(it->path)) {
      handle_directory_event(it, event->mask, get_path(it->path, event));
    } else {
      handle_file_event(it, event->mask);
    }
  }
}





void fs_watcher::handle_file_event(watch_iter it, uint32_t mask) {
  if (((mask & IN_DELETE_SELF) != 0) ||
      (((mask & IN_MOVE_SELF) != 0) && !std::filesystem::exists(it->path))) {
    invoke_callback(it->path, action::removed);
    remove_watch(it);

  } else if ((mask & IN_CLOSE_WRITE) != 0) {
    invoke_callback(it->path, action::changed);
  }
}





void fs_watcher::handle_directory_event(
    watch_iter                   it,
    uint32_t                     mask,
    const std::filesystem::path& p
) {
  if ((mask & IN_MOVED_TO) != 0) {
    invoke_callback(p, action::changed);
    add_watch(p, false);

  } else if ((mask & IN_CREATE) != 0) {
    invoke_callback(p, action::added);
    add_watch(p, false);

  } else if ((mask & IN_MOVED_FROM) != 0) {
    for (auto jt = file_watches_.begin(); jt != file_watches_.end(); ) {
      if (jt->path == p) {
        invoke_callback(p, action::removed);
        jt = remove_watch(jt);
      } else {
        ++jt;
      }
    }

  } else if ((mask & IN_DELETE_SELF) != 0) {
    for (auto jt = file_watches_.begin(); jt != file_watches_.end(); ) {
      if (jt->path.parent_path() == p || jt == it) {
        invoke_callback(p, action::removed);
        jt = remove_watch(jt);
      } else {
        ++jt;
      }
    }
  }
}
