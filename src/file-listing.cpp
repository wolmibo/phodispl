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





bool file_listing::fs_info::satisfied() const {
  return satisfies(path, mode);
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
  demotion_candidate_.reset();
  file_list_.clear();
}





void file_listing::demote_initial_file() {
  if (!demotion_candidate_) {
    return;
  }

  if (auto it = std::ranges::find(file_list_, *demotion_candidate_, &fs_info::path);
      it != file_list_.end()) {

    bool was_listed = it->satisfied();

    it->mode = global_config().fl_single_file_parent_dir;

    if (!it->satisfied() && was_listed) {
      invoke_save(callback_, it->path, fs_watcher::action::removed);
    }
  }
}





void file_listing::populate_item(
    std::vector<std::filesystem::path>& list,
    const std::filesystem::path&        path,
    listing_mode                        mode
) {
  fs_info item{
    .path = std::filesystem::absolute(path),
    .mode = mode,
  };

  if (item.satisfied()) {
    list.emplace_back(item.path);
  }

  file_list_.emplace_back(std::move(item));
}





void file_listing::populate_directory(
    std::vector<std::filesystem::path>& list,
    const std::filesystem::path&        path,
    listing_mode                        mode
) {
  for (const auto& p: std::filesystem::directory_iterator{path}) {
    if (p.is_directory()) {
      continue;
    }

    populate_item(list, p.path(), mode);
  }
}





std::vector<std::filesystem::path> file_listing::populate() {
  std::vector<std::filesystem::path> list;

  switch (determine_startup_mode()) {
    case startup_mode::single_dir:
      populate_directory(list, initial_files_.front(), global_config().fl_single_dir);
      break;

    case startup_mode::multi:
      for (const auto& p: initial_files_) {
        if (std::filesystem::is_directory(p)) {
          populate_directory(list, p, global_config().fl_multi_dir);
        } else {
          populate_item(list, p, global_config().fl_multi_file);
        }
      }
      break;

    case startup_mode::single_file: {
      const auto& major = initial_files_.front();

      for (const auto& p: std::filesystem::directory_iterator{major.parent_path()}) {
        if (p.is_directory() || p.path() == major) {
          continue;
        }

        populate_item(list, p.path(), global_config().fl_single_file_parent_dir);
      }

      populate_item(list, major, global_config().fl_single_file);

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

      populate_directory(list, wd, global_config().fl_empty_wd_dir);

    } break;

    default:
      break;
  }

  std::ranges::sort(list);

  return list;
}
