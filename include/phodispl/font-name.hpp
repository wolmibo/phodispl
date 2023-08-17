// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_FONT_NAME_HPP_INCLUDED
#define PHODISPL_FONT_NAME_HPP_INCLUDED

#include <filesystem>
#include <string>
#include <string_view>



class font_name {
  public:
    explicit font_name(std::string);



    [[nodiscard]] const std::filesystem::path& path() const { return path_; }
    [[nodiscard]] std::string_view             name() const { return name_; }



    [[nodiscard]] auto operator<=>(const font_name&) const = default;



  private:
    std::string           name_;
    std::filesystem::path path_;
};



#endif // PHODISPL_FONT_NAME_HPP_INCLUDED
