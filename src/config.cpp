#include "phodispl/config.hpp"

#include "phodispl/path-compare.hpp"

#include <fstream>
#include <iostream>
#include <optional>

#include <iconfigp/color.hpp>
#include <iconfigp/exception.hpp>
#include <iconfigp/find-config.hpp>
#include <iconfigp/format.hpp>
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





ICONFIGP_DEFINE_ENUM_LUT(animation_curve,
    "linear",        linear,
    "sinusoidal",    sinusoidal,
    "immediate",     immediate)

ICONFIGP_DEFINE_ENUM_LUT(scale_filter,
    "linear",        linear,
    "nearest",       nearest)

ICONFIGP_DEFINE_ENUM_LUT(listing_mode,
    "always",        always,
    "exists",        exists,
    "supported",     supported)

ICONFIGP_DEFINE_ENUM_LUT(path_compare_method,
    "lexicographic", lexicographic,
    "semantic",      semantic)



ICONFIGP_DEFINE_VALUE_PARSER(font_name,
    "(<font name> | <path to font file>)",
    [](std::string_view input) { return font_name{std::string{input}}; })





namespace {
  template<typename T> struct src_t { using type = T; };
  template<> struct src_t<std::chrono::milliseconds>{ using type = uint32_t;            };
  template<> struct src_t<std::string>              { using type = std::string_view;    };
  template<> struct src_t<path_compare>             { using type = path_compare_method; };

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
      update(animation_view_next_curve, animation->unique_key("view-next-interpolation"));
      update(animation_view_next_curve, animation->unique_key("view-next-curve"));
      update(animation_view_next_ms,    animation->unique_key("view-next-ms"));

      update(animation_view_snap_curve, animation->unique_key("view-snap-interpolation"));
      update(animation_view_snap_curve, animation->unique_key("view-snap-curve"));
      update(animation_view_snap_ms,    animation->unique_key("view-snap-ms"));

      update(animation_ui_fade_curve,   animation->unique_key("ui-fade-curve"));
      update(animation_ui_fade_ms,      animation->unique_key("ui-fade-ms"));

      update(animation_ui_highlight_curve, animation->unique_key("ui-highlight-curve"));
      update(animation_ui_highlight_ms,    animation->unique_key("ui-highlight-ms"));
    }



    if (auto fl = root.subsection("file-listing")) {
      update(fl_empty_wd,               fl->unique_key("empty-wd"));
      update(fl_empty_wd_dir,           fl->unique_key("empty-wd-dir"));

      update(fl_single_file,            fl->unique_key("single-file"));
      update(fl_single_file_parent,     fl->unique_key("single-file-parent"));
      update(fl_single_file_parent_dir, fl->unique_key("single-file-parent-dir"));
      update(fl_single_file_demote,     fl->unique_key("single-file-demote"));

      update(fl_single_dir,             fl->unique_key("single-dir"));

      update(fl_multi_file,             fl->unique_key("multi-file"));
      update(fl_multi_dir,              fl->unique_key("multi-dir"));

      update(fl_compare_function,       fl->unique_key("sort-mode"));
    }



    if (auto il = root.subsection("image-loading")) {
      update(il_show_loading,      il->unique_key("show-loading"));

      update(il_partial,           il->unique_key("partial"));
      update(il_partial_threshold, il->unique_key("partial-threshold-ms"));
      update(il_partial_interval,  il->unique_key("partial-interval-ms"));
      update(il_partial_flush,     il->unique_key("partial-flush"));

      update(il_play_available,    il->unique_key("play-available"));
    }





    if (auto message =
        iconfigp::format_unused_message(root, content, logcerr::is_colored())) {
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





namespace {
  void throw_error(std::string_view str) {
    throw std::runtime_error{"config value mismatch for " + std::string{str}};
  }



  template<typename L, typename R>
  void assert_eq(L&& l, R&& r, std::string_view str) {
    if (std::forward<L>(l) != std::forward<R>(r)) {
      throw_error(str);
    }
  }



  void assert_eq(const color& l, const color& r, std::string_view str) {
    constexpr float delta = 1.f / 255.f;

    if (std::abs(l[0] - r[0]) > delta ||
        std::abs(l[1] - r[1]) > delta ||
        std::abs(l[2] - r[2]) > delta ||
        std::abs(l[3] - r[3]) > delta) {
      throw_error(str);
    }
  }
}



void config::assert_equal(const config& rhs) const {
#define ASSEQ(x) assert_eq(x, rhs.x, #x) //NOLINT(*-macro-usage)
  ASSEQ(filter);
  ASSEQ(watch_fs);
  ASSEQ(gamma);
  ASSEQ(input_speed);

  ASSEQ(animation_view_next_curve);
  ASSEQ(animation_view_next_ms);
  ASSEQ(animation_view_snap_curve);
  ASSEQ(animation_view_snap_ms);
  ASSEQ(animation_ui_fade_curve);
  ASSEQ(animation_ui_fade_ms);
  ASSEQ(animation_ui_highlight_curve);
  ASSEQ(animation_ui_highlight_ms);

  ASSEQ(theme_heading_size);
  ASSEQ(theme_text_size);
  ASSEQ(theme_heading_color);
  ASSEQ(theme_text_color);
  ASSEQ(theme_background);
  ASSEQ(theme_font);

  ASSEQ(cache_keep_forward);
  ASSEQ(cache_load_forward);
  ASSEQ(cache_keep_backward);
  ASSEQ(cache_load_backward);

  ASSEQ(fl_empty_wd);
  ASSEQ(fl_empty_wd_dir);
  ASSEQ(fl_single_file);
  ASSEQ(fl_single_file_parent);
  ASSEQ(fl_single_file_parent_dir);
  ASSEQ(fl_single_file_demote);
  ASSEQ(fl_single_dir);
  ASSEQ(fl_multi_file);
  ASSEQ(fl_multi_dir);
  ASSEQ(fl_compare_function);

  ASSEQ(il_show_loading);
  ASSEQ(il_partial);
  ASSEQ(il_partial_threshold);
  ASSEQ(il_partial_interval);
  ASSEQ(il_partial_flush);
  ASSEQ(il_play_available);
#undef ASSEQ
}
