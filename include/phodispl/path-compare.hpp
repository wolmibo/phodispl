// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_PATH_COMPARE_HPP_INCLUDED
#define PHODISPL_PATH_COMPARE_HPP_INCLUDED

#include <filesystem>
#include <string_view>



enum class path_compare_method {
  lexicographic,
  semantic
};




[[nodiscard]] bool semantic_compare(std::string_view, std::string_view);



class path_compare {
  public:
    path_compare(path_compare_method method) : method_{method} {}



    [[nodiscard]] bool operator()(
        const std::filesystem::path& lhs,
        const std::filesystem::path& rhs
    ) const {
      if (method_ == path_compare_method::semantic) {
        return semantic_compare(lhs.native(), rhs.native());
      }
      return lhs < rhs;
    }



  private:
    path_compare_method method_;
};

#endif // PHODISPL_PATH_COMPARE_HPP_INCLUDED
