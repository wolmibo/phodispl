#ifndef PHODISPL_FS_WATCHER_HPP_INCLUDED
#define PHODISPL_FS_WATCHER_HPP_INCLUDED

#include <filesystem>
#include <functional>
#include <span>
#include <thread>
#include <vector>



class inotify_event;

class fs_watcher {
  public:
    enum class action {
      added,
      changed,
      removed,
    };

    using callback =
      std::move_only_function<void(const std::filesystem::path&, action) const>;



    fs_watcher(const fs_watcher&) = delete;
    fs_watcher(fs_watcher&&)      = delete;
    fs_watcher& operator=(const fs_watcher&) = delete;
    fs_watcher& operator=(fs_watcher&&)      = delete;

    ~fs_watcher();
    fs_watcher(callback&&);



    void watch(std::span<const std::filesystem::path>);
    void unwatch();



  private:
    struct pipe_fd {
      int read {-1};
      int write{-1};

      pipe_fd(const pipe_fd&) = delete;
      pipe_fd(pipe_fd&&)      = delete;
      pipe_fd& operator=(const pipe_fd&) = delete;
      pipe_fd& operator=(pipe_fd&&)      = delete;

      ~pipe_fd();
      pipe_fd();
    };

    struct watch_item {
      int                   wd;
      std::filesystem::path path;
    };

    using watch_iter = std::vector<watch_item>::iterator;



    callback                callback_;

    pipe_fd                 watch_pipe_;
    int                     fd_          {-1};

    std::jthread            watch_thread_;
    std::mutex              mutex_;

    std::vector<watch_item> file_watches_;



    void watch_loop();



    void add_watch(const std::filesystem::path&, bool);



    void handle_event          (const inotify_event*);
    void handle_file_event     (watch_iter, uint32_t);
    void handle_directory_event(watch_iter, uint32_t, const std::filesystem::path&);

    watch_iter remove_watch(watch_iter);

    void invoke_callback(const std::filesystem::path& path, action act) const {
      if (callback_) {
        callback_(path, act);
      }
    }
};

#endif // PHODISPL_FS_WATCHER_HPP_INCLUDED
