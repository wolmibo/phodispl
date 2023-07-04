#include "phodispl/formatting.hpp"



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
