// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_FILE_LISTING_HPP_INCLUDED
#define PHODISPL_FILE_LISTING_HPP_INCLUDED

#include "phodispl/config-types.hpp"
#include "phodispl/fs-watcher.hpp"

#include <filesystem>
#include <optional>



class file_listing {
  public:
    file_listing(const file_listing&) = delete;
    file_listing(file_listing&&) = delete;
    file_listing& operator=(const file_listing&) = delete;
    file_listing& operator=(file_listing&&) = delete;

    ~file_listing() = default;

    explicit file_listing(fs_watcher::callback, std::vector<std::filesystem::path>);



    [[nodiscard]] std::optional<std::filesystem::path> initial_file() const;
    [[nodiscard]] std::vector<std::filesystem::path>   populate();

    void clear();

    void demote_initial_file();





  private:
    std::vector<std::filesystem::path>         initial_files_;
    fs_watcher::callback                       callback_;

    std::mutex                                 mutex_;

    std::vector<std::filesystem::path>         file_list_;
    std::vector<std::pair<listing_mode, bool>> mode_list_;

    std::optional<std::filesystem::path>       demotion_candidate_;

    std::optional<fs_watcher>                  fs_watcher_;



    enum class startup_mode {
      empty,
      single_file,
      single_dir,
      multi
    };

    [[nodiscard]] startup_mode determine_startup_mode() const;


    void on_file_changed(const std::filesystem::path&, fs_watcher::action);


    void populate_item_unsafe     (const std::filesystem::path&, listing_mode);
    void populate_directory_unsafe(const std::filesystem::path&, listing_mode);
    void populate_lists_unsafe();

    [[nodiscard]] listing_mode determine_mode_unsafe(const std::filesystem::path&) const;
};

#endif // PHODISPL_FILE_LISTING_HPP_INCLUDED
