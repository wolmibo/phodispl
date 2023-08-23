#include "phodispl/formatting.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <span>



namespace {
  [[nodiscard]] bool is_whitespace(char32_t c) {
    return c == ' ' || c == '\n';
  }



  [[nodiscard]] std::u32string_view next_word(std::u32string_view& text) {
    if (text.front() == '\n') {
      text.remove_prefix(1);
      return U"\n";
    }

    while (!text.empty() && is_whitespace(text.front())) {
      text.remove_prefix(1);
    }

    size_t count{0};
    while (count < text.size() && !is_whitespace(text[count])) {
      count++;
    }


    auto sub = text.substr(0, count);
    text.remove_prefix(count);

    return sub;
  }
}



std::u32string wrap_text(
    std::u32string_view                   text,
    float                                 width,
    measure_function<std::u32string_view> measure
) {
  std::u32string output;

  float space_width{measure(U" ")};

  float current_width{0.f};


  while (!text.empty()) {
    auto nw = next_word(text);
    auto w  = measure(nw);
    if (nw == U"\n") {
      current_width = 0.f;
      output.push_back('\n');
      continue;
    }

    if (current_width + w > width) {
      current_width = 0.f;
      output.push_back('\n');
    } else if (!nw.empty() && current_width != 0.f) {
      output.push_back(' ');
      current_width += space_width;
    }
    output.append(nw);
    current_width += w;
  }


  return output;
}





std::filesystem::path nice_path(const std::filesystem::path& p) {
  std::vector<std::string> candidates;
  candidates.reserve(3);

  candidates.emplace_back(std::filesystem::weakly_canonical(p).string());
  const auto& canonical = candidates.front();

  std::error_code ec{};
  auto wd = std::filesystem::current_path(ec).string();
  if (!ec) {
    if (canonical.starts_with(wd)) {
      candidates.emplace_back('.' + canonical.substr(wd.size()));
    }
  }

  if (const auto* path = std::getenv("HOME"); path != nullptr && *path != 0) {
    if (canonical.starts_with(path)) {
      candidates.emplace_back('~' + canonical.substr(std::strlen(path)));
    }
  }

  return std::move(*std::ranges::min_element(candidates, {}, &std::string::size));
}





namespace {
  constexpr std::u32string_view prefix{U"…/"};
  constexpr char32_t ellipsis{prefix[0]};
  constexpr char32_t pathsep {prefix[1]};



  [[nodiscard]] std::u32string_view sv(const char32_t* c) {
    return {c, 1};
  }



  [[nodiscard]] std::pair<float, std::vector<std::pair<std::u32string, float>>>split_path(
      const std::filesystem::path&           path,
      measure_function<std::u32string_view>& measure
  ) {
    std::vector<std::pair<std::u32string, float>> list;
    float total{0.f};

    for (const auto& p: path) {
      auto str   = p.u32string();
      auto width = measure(str);
      total += width;

      list.emplace_back(std::move(str), width);
    }

    return {total, list};
  }



  [[nodiscard]] std::u32string join_path_segments(
      std::span<std::pair<std::u32string, float>> segments
  ) {
    std::u32string output;
    for (size_t i = 0; i < segments.size(); ++i) {
      if (i > 0) {
        output.push_back(pathsep);
      }

      if (segments[i].first != U"/") {
        output += segments[i].first;
      }
    }
    return output;
  }



  [[nodiscard]] std::u32string_view extension(std::u32string_view name) {
    auto pos = name.rfind(U'.');
    if (pos == std::u32string_view::npos) {
      return name.substr(0, 0);
    }
    return name.substr(pos);
  }



  [[nodiscard]] std::u32string collapse_filename(
      std::u32string_view                    filename,
      float                                  filename_width,
      float                                  ps_w,
      float                                  el_w,
      float                                  max_width,
      measure_function<std::u32string_view>& measure,
      bool                                   root
  ) {
    std::u32string prefix{};

    if (!root) {
      max_width -= ps_w + el_w;
      prefix    = ::prefix;
    }

    if (filename_width <= max_width) {
      return prefix + std::u32string{filename};
    }

    std::u32string ext{extension(filename)};

    if (auto ext_w = measure(ext); ext_w <= max_width) {
      max_width      -= ext_w;
      filename_width -= ext_w;
      filename.remove_suffix(ext.size());
    } else {
      ext = U"";
    }


    auto back  = filename.substr(filename.size() / 2);
    auto front = filename.substr(0, filename.size() - back.size());


    while (filename_width > max_width && (front.size() + back.size() > 0)) {
      if (!front.empty()) {
        filename_width -= measure(front.substr(front.size() - 1, 1));
        front.remove_suffix(1);
      }

      if (filename_width <= max_width) {
        break;
      }

      if (!back.empty()) {
        filename_width -= measure(back.substr(0, 1));
        back.remove_prefix(1);
      }
    }


    if (front.size() + back.size() < filename.size()) {
      return prefix + std::u32string{front} + ellipsis + std::u32string{back} + ext;
    }

    return prefix + std::u32string{filename} + ext;
  }
}



std::u32string shorten_path(
    const std::filesystem::path&          path,
    float                                 max_width,
    measure_function<std::u32string_view> measure
) {
  auto [total, segments] = split_path(path, measure);

  if (segments.empty()) {
    return U"";
  }

  float ps_w = measure(sv(&pathsep));

  total += ps_w * (std::max<size_t>(segments.size(), 1) - 1);

  if (total <= max_width) {
    return join_path_segments(segments);
  }

  float el_w = measure(sv(&ellipsis));

  for (size_t i = 2; i < segments.size() - 1; ++i) {
    total -= segments[i].second;
    if (total + el_w <= max_width) {
      return join_path_segments(std::span{segments}.subspan(0, 2))
        + pathsep + ellipsis + pathsep
        + join_path_segments(std::span{segments}.subspan(i + 1));
    }
    total -= ps_w;
  }



  return collapse_filename(segments.back().first, segments.back().second, ps_w, el_w,
      max_width, measure, segments.size() == 1);
}





namespace {
  [[nodiscard]] char32_t format_regular_digit(size_t num) {
    return '0' + (num % 10);
  }

  [[nodiscard]] char32_t format_super_digit(size_t num) {
    static constexpr std::u32string_view sups{U"⁰¹²³⁴⁵⁶⁷⁸⁹"};
    return sups[num % sups.size()];
  }



  [[nodiscard]] constexpr size_t powi(size_t base, unsigned int exp) {
    size_t output{1};

    for (; exp != 0; base *= base, exp >>= 1) {
      if ((exp & 0x1) != 0) {
        output *= base;
      }
    }

    return output;
  }



  template<typename Digit>
  [[nodiscard]] std::u32string format_number(
      size_t       number,
      size_t       div,
      unsigned int precision,
      Digit&&      format_digit
  ) {
    if (div == 0) {
      return U"∞";
    }

    number = (number * powi(10, precision) * 10L) / div;

    std::u32string output;

    if ((number % 10) >= 5) {
      number += 10;
    }
    number /= 10;

    while (number > 0 || output.size() <= precision) {
      output.push_back(std::forward<Digit>(format_digit)(number));
      number /= 10;

      if (output.size() == precision) {
        output.push_back('.');
      }
    }

    if (output.empty() || output.back() == '.') {
      output.push_back(std::forward<Digit>(format_digit)(0));
    }

    std::ranges::reverse(output);

    return output;
  }
}




std::u32string format_byte_size(size_t size) {
  if (size < 1024) {
    auto num = format_number(size, 1, 0, format_regular_digit);
    num.push_back(' ');
    num.push_back('B');
    return num;
  }

  size_t index{0};
  while (size >= 1024L * 1024) {
    size /= 1024;
    index++;
  }

  unsigned int precision{3};
  for (auto shifted = size; shifted >= 10 * 1024L && precision > 0; shifted /= 10) {
    precision--;
  }

  auto num = format_number(size, 1024, precision, format_regular_digit);

  static constexpr std::string_view prefix{"KMGTPEZYRQ"};

  if (index < prefix.size()) {
    num.push_back(' ');
    num.push_back(prefix[index]);
    num.push_back('i');
  } else {
    num.append(U"×2");
    num += format_number((index + 1) * 10, 1, 0, format_super_digit);
    num.push_back(' ');
  }

  num.push_back('B');
  return num;
}
