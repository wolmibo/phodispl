#include "phodispl/path-compare.hpp"

#include <algorithm>
#include <compare>
#include <utility>



// clang-tidy doesn't like (a <=> b) != 0
// (which is the correct way according to cppreference)
//NOLINTBEGIN(*-use-nullptr)

namespace {
  enum class char_type {
    path_component = 0,
    number         = 1,
    alpha          = 2,
    special        = 3,
  };



  [[nodiscard]] char_type determine_char_type(char c) {
    if (c == '.' || c == '/' || c == '\\') {
      return char_type::path_component;
    }

    if ('0' <= c && c <= '9') {
      return char_type::number;
    }

    if (c < 0 || ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
      return char_type::alpha;
    }

    return char_type::special;
  }



  [[nodiscard]] std::strong_ordering compare_number(
      std::string_view lhs,
      std::string_view rhs
  ) {
    if (auto res = (lhs.size() <=> rhs.size()); res != 0) {
      return res;
    }

    for (size_t i = 0; i < lhs.size(); ++i) {
      if (auto res = (lhs[i] <=> rhs[i]); res != 0) {
        return res;
      }
    }
    return std::strong_ordering::equal;
  }



  [[nodiscard]] int alpha_to_upper(char c) {
    if (c < 0) {
      return c + 1024;
    }

    if (c >= 'a') {
      return c - 'a' + 'A';
    }

    return c;
  }



  [[nodiscard]] std::strong_ordering compare_alpha_case_insensitive(
      std::string_view lhs,
      std::string_view rhs
  ) {
    auto min = std::min(lhs.size(), rhs.size());

    for (size_t i = 0; i < min; ++i) {
      if (auto res = (alpha_to_upper(lhs[i]) <=> alpha_to_upper(rhs[i])); res != 0) {
        return res;
      }
    }

    return lhs.size() <=> rhs.size();
  }



  struct word {
    char_type        ctype;
    std::string_view text;



    [[nodiscard]] std::strong_ordering operator<=>(const word& rhs) const {
      if (ctype == rhs.ctype) {
        switch (ctype) {
          case char_type::number: return compare_number(text, rhs.text);
          case char_type::alpha:  return compare_alpha_case_insensitive(text, rhs.text);
          default:                return text <=> rhs.text;
        }
      }

      return std::to_underlying(ctype) <=> std::to_underlying(rhs.ctype);
    }
  };



  [[nodiscard]] word next_word_from_nonempty(std::string_view& str) {
    auto type = determine_char_type(str.front());

    size_t i{1};
    while (i < str.size() && determine_char_type(str[i]) == type) { ++i; }

    word out{
      .ctype = type,
      .text  = str.substr(0, i)
    };

    str.remove_prefix(i);

    return out;
  }



  [[nodiscard]] char swap_case(char c) {
    if ('A' <= c && c <= 'Z') {
      return c - 'A' + 'a';
    }

    if ('a' <= c && c <= 'z') {
      return c - 'a' + 'A';
    }

    return c;
  }
}



bool semantic_compare(std::string_view lhs, std::string_view rhs) {
  auto lhss{lhs};
  auto rhss{rhs};

  while (!lhss.empty() && !rhss.empty()) {
    if (auto res = (next_word_from_nonempty(lhss) <=> next_word_from_nonempty(rhss));
        res != 0) {
      return res < 0;
    }
  }

  if (lhss.size() != rhss.size()) {
    return lhss.size() < rhs.size();
  }

  return std::ranges::lexicographical_compare(lhs, rhs, {}, swap_case, swap_case);
}

//NOLINTEND(*-use-nullptr)
