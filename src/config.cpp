#include "phodispl/config.hpp"

#include <fstream>
#include <iostream>
#include <optional>

#include <iconfigp/color.hpp>
#include <iconfigp/exception.hpp>
#include <iconfigp/find-config.hpp>
#include <iconfigp/parser.hpp>
#include <iconfigp/path.hpp>
#include <iconfigp/section.hpp>
#include <iconfigp/value-parser.hpp>

#include <logcerr/log.hpp>



namespace { namespace global_state {
  config configuration;
}}



const config& global_config() {
  return global_state::configuration;
}





namespace {
  [[nodiscard]] std::string read_file(const std::filesystem::path& path) {
    std::ifstream input{path, std::ios::ate};
    if (!input) {
      throw std::runtime_error{"Unable to open config file \"" + path.string() + "\""};
    }

    auto length = input.tellg();
    input.seekg(0);

    std::string buffer(length, ' ');
    input.read(buffer.data(), length);
    return buffer;
  }



  void load_config(const std::filesystem::path& path, bool strict) {
    logcerr::debug("config path: \"{}\"", path.string());
    iconfigp::preferred_root_path(path.parent_path());
    global_state::configuration = config{read_file(path), strict};
  }
}



void load_config(const std::optional<std::filesystem::path>& path, bool strict) {
  if (path) {
    load_config(*path, strict);
  } else if (auto dot_path = iconfigp::find_config_file("phodispl")) {
    load_config(*dot_path, strict);
  }
}







template<> struct iconfigp::case_insensitive_parse_lut<animation_interpolation> {
  static constexpr std::string_view name {"animation-interpolation"};
  static constexpr std::array<std::pair<std::string_view, animation_interpolation>, 3>
  lut {
    std::make_pair("linear",     animation_interpolation::linear),
    std::make_pair("sinusoidal", animation_interpolation::sinusoidal),
    std::make_pair("immediate",  animation_interpolation::immediate)
  };
};



template<> struct iconfigp::case_insensitive_parse_lut<scale_filter> {
  static constexpr std::string_view name {"scale-filter"};
  static constexpr std::array<std::pair<std::string_view, scale_filter>, 2> lut {
    std::make_pair("linear",  scale_filter::linear),
    std::make_pair("nearest", scale_filter::nearest),
  };
};



namespace {
  template<typename T> struct src_t { using type = T; };
  template<> struct src_t<std::chrono::milliseconds> { using type = uint32_t;         };
  template<> struct src_t<std::string>               { using type = std::string_view; };

  template<typename S>
  void update(S& value, iconfigp::opt_ref<const iconfigp::key_value> pair) {
    if (pair) {
      if constexpr (std::is_same_v<S, typename src_t<S>::type>) {
        value = iconfigp::parse<S>(*pair);
      } else {
        value = static_cast<S>(iconfigp::parse<typename src_t<S>::type>(*pair));
      }
    }
  }
}



config::config(std::string_view content, bool strict) {
  try {
    auto root = iconfigp::parser::parse(content);



    update<scale_filter>(filter,      root.unique_key("scale-filter"));
    update<bool>        (watch_fs,    root.unique_key("watch-fs"));
    update<float>       (gamma,       root.unique_key("gamma"));
    update<float>       (input_speed, root.unique_key("input-speed"));



    if (auto cache = root.subsection("cache")) {
      update(cache_keep_forward,  cache->unique_key("keep-forward"));
      update(cache_load_forward,  cache->unique_key("load-forward"));
      update(cache_keep_backward, cache->unique_key("keep-backward"));
      update(cache_load_backward, cache->unique_key("load-backward"));

      cache_keep_forward  = std::max(cache_keep_forward,  cache_load_forward);
      cache_keep_backward = std::max(cache_keep_backward, cache_load_backward);
    }



    if (auto theme = root.subsection("theme")) {
      update(theme_font,          theme->unique_key("font"));

      update(theme_heading_size,  theme->unique_key("heading-size"));
      update(theme_text_size,     theme->unique_key("text-size"));

      update(theme_heading_color, theme->unique_key("heading-color"));
      update(theme_text_color,    theme->unique_key("text-color"));

      update(theme_background,    theme->unique_key("background"));
    }



    if (auto animation = root.subsection("animation")) {
      update(animation_view_next_interpolation,
          animation->unique_key("view-next-interpolation"));
      update(animation_view_next_ms, animation->unique_key("view-next-ms"));

      update(animation_view_snap_interpolation,
          animation->unique_key("view-snap-interpolation"));
      update(animation_view_snap_ms, animation->unique_key("view-snap-ms"));
    }





    if (auto message =
        iconfigp::generate_unused_message(root, content, logcerr::is_colored())) {
      logcerr::warn("config file contains unused keys");
      logcerr::print_raw_sync(std::cerr, *message);
      if (strict) {
        throw std::runtime_error{"config file contains unused keys"};
      }
    }
  } catch (const iconfigp::exception& ex) {
    logcerr::error(ex.what());
    logcerr::print_raw_sync(std::cerr,
        iconfigp::format_exception(ex, content, logcerr::is_colored()));

    throw;
  }
}
