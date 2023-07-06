// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_FILE_LISTING_HPP_INCLUDED
#define PHODISPL_FILE_LISTING_HPP_INCLUDED

#include "phodispl/config-types.hpp"
#include "phodispl/fs-watcher.hpp"

#include <filesystem>
#include <functional>
#include <optional>
#include <span>



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
    std::vector<std::filesystem::path> initial_files_;
    fs_watcher::callback               callback_;

    struct fs_info {
      std::filesystem::path path;
      listing_mode          mode;

      [[nodiscard]] bool satisfied() const;
    };

    std::vector<fs_info>                 file_list_;
    std::optional<std::filesystem::path> demotion_candidate_;

    std::optional<fs_watcher>            fs_watcher_;


    enum class startup_mode {
      empty,
      single_file,
      single_dir,
      multi
    };

    [[nodiscard]] startup_mode determine_startup_mode() const;


    void on_file_changed(const std::filesystem::path&, fs_watcher::action);


    void populate_item(std::vector<std::filesystem::path>&,
        const std::filesystem::path&, listing_mode);
    void populate_directory(std::vector<std::filesystem::path>&,
        const std::filesystem::path&, listing_mode);
};

#endif // PHODISPL_FILE_LISTING_HPP_INCLUDED
