#ifndef PHODISPL_IMAGE_SOURCE_HPP_INCLUDED
#define PHODISPL_IMAGE_SOURCE_HPP_INCLUDED

#include "phodispl/damageable.hpp"
#include "phodispl/fs-watcher.hpp"
#include "phodispl/image-cache.hpp"
#include "phodispl/image.hpp"

#include <condition_variable>
#include <deque>
#include <filesystem>
#include <thread>
#include <utility>
#include <vector>

#include <win/application.hpp>



class config;

enum class image_change {
  none,
  next,
  previous,
  reload,
  replace_deleted,
};



class image_source {
  public:
    using callback =
      std::move_only_function<void(std::shared_ptr<image>, image_change)>;



    image_source(const image_source&) = delete;
    image_source(image_source&&)      = delete;

    image_source& operator=(const image_source&) = delete;
    image_source& operator=(image_source&&)      = delete;

    explicit image_source(callback&&,
        std::vector<std::filesystem::path>, const win::application&);

    ~image_source();



    void next_image();
    void previous_image();

    void reload_current();
    void reload_file_list();



    [[nodiscard]] operator bool() const { return !cache_.empty(); }

    [[nodiscard]] std::shared_ptr<image> current() const;





  private:
    callback                            callback_;
    image_cache                         cache_;
    mutable std::mutex                  cache_mutex_;

    using prio_shared_image = std::pair<size_t, std::shared_ptr<image>>;
    std::vector<prio_shared_image>      scheduled_images_;
    std::vector<const image*>           unscheduled_images_;
    std::mutex                          scheduled_images_lock_;

    std::vector<std::filesystem::path>  startup_files_;
    std::jthread                        worker_thread_;
    std::mutex                          worker_mutex_;
    std::condition_variable             worker_wakeup_;

    fs_watcher                          filesystem_watcher_;
    win::context                        filesystem_context_;



    void unload_image    (const std::shared_ptr<image>&, bool);
    void schedule_image  (const std::shared_ptr<image>&, size_t);
    void unschedule_image(const std::shared_ptr<image>&);
    [[nodiscard]] std::shared_ptr<image> next_scheduled_image();

    void work_loop(const std::stop_token&);

    void file_event(const std::filesystem::path&, fs_watcher::action);



    void populate_cache(std::unique_lock<std::mutex>);




    void set_images_to(const std::vector<std::filesystem::path>&);



    void update_cache_unguarded(bool=false);
    void update_cache_all_unguarded();
    void update_cache_local_unguarded();



    void remove_file_unguarded(const std::filesystem::path&);
};

#endif // PHODISPL_IMAGE_SOURCE_HPP_INCLUDED
