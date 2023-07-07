#include "logcerr/log.hpp"
#include "phodispl/config-types.hpp"
#include "phodispl/config.hpp"
#include "phodispl/file-listing.hpp"
#include "phodispl/fs-watcher.hpp"

#include <algorithm>
#include <filesystem>

#include <pixglot/codecs.hpp>





namespace {
  template<typename Fnc, typename... Args>
  void invoke_save(Fnc&& cb, Args&&... args) {
    if (cb) {
      std::invoke(std::forward<Fnc>(cb), std::forward<Args>(args)...);
    }
  }
}





file_listing::file_listing(
    fs_watcher::callback                callback,
    std::vector<std::filesystem::path>  initial_files
) :
  initial_files_{std::move(initial_files)},
  callback_     {std::move(callback)}
{
  for (auto& file: initial_files_) {
    file = std::filesystem::absolute(file);
  }
}





file_listing::startup_mode file_listing::determine_startup_mode() const {
  if (initial_files_.empty()) {
    return startup_mode::empty;
  }

  if (initial_files_.size() == 1) {
    if (std::filesystem::is_directory(initial_files_.front())) {
      return startup_mode::single_dir;
    }
    return startup_mode::single_file;
  }

  return startup_mode::multi;
}





namespace {
  [[nodiscard]] bool satisfies(const std::filesystem::path& path, listing_mode mode) {
    switch (mode) {
      case listing_mode::always:
        return true;
      case listing_mode::exists:
        return std::filesystem::exists(path);
      case listing_mode::supported:
        return pixglot::determine_codec(path).has_value();
    }
    return false;
  }
}





std::optional<std::filesystem::path> file_listing::initial_file() const {
  switch (determine_startup_mode()) {
    case startup_mode::single_dir:
      return {};

    case startup_mode::single_file:
      if (satisfies(initial_files_.front(), global_config().fl_single_file)) {
        return initial_files_.front();
      }

      return {};

    case startup_mode::multi:
      for (const auto& p: initial_files_) {
        if (std::filesystem::is_directory(p)) {
          return {};
        }

        if (satisfies(p, global_config().fl_multi_file)) {
          return p;
        }
      }
      return {};

    case startup_mode::empty:
    default:
      return {};
  }
}





void file_listing::clear() {
  if (fs_watcher_) {
    fs_watcher_->unwatch();
  }

  std::lock_guard lock{mutex_};

  demotion_candidate_.reset();
  file_list_.clear();
  mode_list_.clear();
}





void file_listing::demote_initial_file() {
  std::lock_guard lock{mutex_};

  if (!demotion_candidate_) {
    return;
  }

  if (auto it = std::ranges::find(file_list_, *demotion_candidate_);
      it != file_list_.end()) {

    auto& mode = mode_list_[it - file_list_.begin()];

    mode.first = global_config().fl_single_file_parent_dir;

    bool was_listed = mode.second;
    mode.second = satisfies(*it, mode.first);

    if (!mode.second && was_listed) {
      invoke_save(callback_, *it, fs_watcher::action::removed);
    }
  }

  demotion_candidate_.reset();
}





void file_listing::populate_item_unsafe(
    const std::filesystem::path&        path,
    listing_mode                        mode
) {
  file_list_.emplace_back(path);
  mode_list_.emplace_back(mode, satisfies(path, mode));
}





void file_listing::populate_directory_unsafe(
    const std::filesystem::path&        path,
    listing_mode                        mode
) {
  if (std::filesystem::is_directory(path)) {
    populate_item_unsafe(path, mode);
  }

  for (const auto& p: std::filesystem::directory_iterator{path}) {
    if (p.is_directory()) {
      continue;
    }

    populate_item_unsafe(p.path(), mode);
  }
}





std::vector<std::filesystem::path> file_listing::populate() {
  if (!fs_watcher_ && global_config().watch_fs) {
    fs_watcher_.emplace(std::bind_front(&file_listing::on_file_changed, this));
  }

  std::vector<std::filesystem::path> list;

  { std::lock_guard lock{mutex_};
    populate_lists_unsafe();


    for (size_t i = 0; i < file_list_.size(); ++i) {
      if (mode_list_[i].second && !std::filesystem::is_directory(file_list_[i])) {
        list.emplace_back(file_list_[i]);
      }
    }
  }


  if (fs_watcher_) {
    fs_watcher_->watch(file_list_);
  }

  return list;
}





void file_listing::populate_lists_unsafe() {
  switch (determine_startup_mode()) {
    case startup_mode::single_dir:
      populate_directory_unsafe(initial_files_.front(), global_config().fl_single_dir);
      break;

    case startup_mode::multi:
      for (const auto& p: initial_files_) {
        if (std::filesystem::is_directory(p)) {
          populate_directory_unsafe(p, global_config().fl_multi_dir);
        } else {
          populate_item_unsafe(p, global_config().fl_multi_file);
        }
      }
      break;

    case startup_mode::single_file: {
      const auto& major = initial_files_.front();

      if (auto parent = major.parent_path(); std::filesystem::is_directory(parent)) {
        populate_item_unsafe(parent, global_config().fl_single_file_parent_dir);
        for (const auto& p: std::filesystem::directory_iterator{major.parent_path()}) {
          if (p.is_directory() || p.path() == major) {
            continue;
          }

          populate_item_unsafe(p.path(), global_config().fl_single_file_parent_dir);
        }
      }

      populate_item_unsafe(major, global_config().fl_single_file);

      if (global_config().fl_single_file_demote) {
        demotion_candidate_.emplace(major);
      }
    } break;

    case startup_mode::empty: if (global_config().fl_empty_wd) {
      std::error_code ec{};
      auto wd = std::filesystem::current_path(ec);

      if (ec || !std::filesystem::is_directory(wd)) {
        break;
      }

      populate_directory_unsafe(wd, global_config().fl_empty_wd_dir);

    } break;

    default:
      break;
  }
}





listing_mode file_listing::determine_mode_unsafe(const std::filesystem::path& p) const {
  std::vector<std::pair<const std::filesystem::path&, size_t>> candidates;

  for (size_t i = 0; i < file_list_.size(); ++i) {
    if (p.native().starts_with(file_list_[i].native())) {
      candidates.emplace_back(file_list_[i], i);
    }
  }

  if (candidates.empty()) {
    logcerr::warn("unable to determine listing-mode for {}", p.native());
    return listing_mode::supported;
  }

  auto it = std::ranges::min_element(candidates, {},
      [](const auto& pair) { return pair.first.native().size(); });

  return mode_list_[it->second].first;
}





void file_listing::on_file_changed(
    const std::filesystem::path& path,
    fs_watcher::action           action
) {
  std::lock_guard lock{mutex_};

  bool listed_before{false};
  bool listed_after {false};

  if (auto it = std::ranges::find(file_list_, path); it != file_list_.end()) {
    auto ix = it - file_list_.begin();

    listed_before = mode_list_[ix].second;

    if (action == fs_watcher::action::removed) {
      std::swap(file_list_[ix], file_list_.back());
      std::swap(mode_list_[ix], mode_list_.back());

      file_list_.pop_back();
      mode_list_.pop_back();

    } else {
      listed_after = mode_list_[ix].second = satisfies(path, mode_list_[ix].first);
    }

  } else {
    if (action == fs_watcher::action::removed) {
      return;
    }

    auto mode = determine_mode_unsafe(path);
    listed_after = satisfies(path, mode);

    file_list_.emplace_back(path);
    mode_list_.emplace_back(mode, listed_after);
  }


  if (listed_before && !listed_after) {
    invoke_save(callback_, path, fs_watcher::action::removed);

  } else if (!listed_before && listed_after) {
    invoke_save(callback_, path, fs_watcher::action::added);

  } else if (listed_before && listed_after) {
    invoke_save(callback_, path, fs_watcher::action::changed);
  }
}
