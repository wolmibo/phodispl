#include "phodispl/path-compare.hpp"

#include <compare>
#include <utility>



// clang-tidy doesn't like (a <=> b) != 0
// (which is the correct way according to cppreference)
//NOLINTBEGIN(*-use-nullptr)

namespace {
  [[nodiscard]] bool is_number(char c) {
    return '0' <= c && c <= '9';
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





  enum class compare_mode : int {
    case_insensitive = 'a' - 'A',
    case_sensitive   = 512 - 'A'
  };



  [[nodiscard]] int relocate(char c, compare_mode mode) {
    if (c < 0) {
      // non-ascii after all ascii
      return c + 1024;
    }

    if ('A' <= c && c <= 'Z') {
      return c + std::to_underlying(mode);
    }

    if (c > 'z') {
      return c - 'z' + 'A';
    }

    return c;
  }



  [[nodiscard]] std::strong_ordering compare_prefix(
      std::string_view lhs,
      std::string_view rhs,
      compare_mode     mode
  ) {
    auto min = std::min(lhs.size(), rhs.size());

    for (size_t i = 0; i < min; ++i) {
      if (auto res = (relocate(lhs[i], mode) <=> relocate(rhs[i], mode)); res != 0) {
        return res;
      }
    }

    return std::strong_ordering::equal;
  }





  struct word {
    std::string_view number;
    int              next_character;
  };

  [[nodiscard]] word next_word_from_nonempty(std::string_view& str) {
    if (!is_number(str.front())) {
      word out {{}, relocate(str.front(), compare_mode::case_insensitive)};
      str.remove_prefix(1);
      return out;
    }


    size_t i{1};
    while (i < str.size() && is_number(str[i])) { ++i; }

    word out{str.substr(0, i), -1};
    str.remove_prefix(i);

    if (!str.empty()) {
      out.next_character = relocate(str.front(), compare_mode::case_insensitive);
    }

    return out;
  }
}



bool semantic_compare(std::string_view lhs, std::string_view rhs) {
  auto lhss{lhs};
  auto rhss{rhs};

  while (!lhss.empty() && !rhss.empty()) {
    auto lw = next_word_from_nonempty(lhss);
    auto rw = next_word_from_nonempty(rhss);

    if (!lw.number.empty() && !rw.number.empty()) {
      if (auto res = compare_number(lw.number, rw.number); res != 0) {
        return res < 0;
      }
    }

    if (lw.next_character < rw.next_character) { return true;  }
    if (lw.next_character > rw.next_character) { return false; }
  }



  if (auto res = compare_prefix(lhs, rhs, compare_mode::case_sensitive); res != 0 ) {
    return res < 0;
  }

  return lhs.size() < rhs.size();
}

//NOLINTEND(*-use-nullptr)
