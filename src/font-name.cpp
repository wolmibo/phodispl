#include "phodispl/font-name.hpp"

#include <memory>

#include <fontconfig/fontconfig.h>





namespace {
  struct fc_pattern_destructor {
    void operator()(FcPattern* pat) const { FcPatternDestroy(pat); }
  };

  using fc_pattern = std::unique_ptr<FcPattern, fc_pattern_destructor>;



  [[nodiscard]] auto fc_init() {
    if (FcInit() != FcTrue) {
      throw std::runtime_error{"unable to initialize fontconfig"};
    }

    struct fc_destructor { void operator()(const int* /*dummy*/) const { FcFini(); } };
    static constexpr int non_null_address{0};

    return std::unique_ptr<const int, fc_destructor>{&non_null_address};
  }
}



[[nodiscard]] std::filesystem::path resolve_font_name(const std::string& name) {
  auto fc_library_guard = fc_init();

  static_assert(sizeof(FcChar8) == sizeof(char));
  //NOLINTNEXTLINE(*-reinterpret-cast)
  fc_pattern pattern{FcNameParse(reinterpret_cast<const FcChar8*>(name.c_str()))};
  if (!pattern) {
    throw std::runtime_error{"unable to find font " + name + " (FcNameParse)"};
  }

  if (FcConfigSubstitute(nullptr, pattern.get(), FcMatchPattern) != FcTrue) {
    throw std::runtime_error{"unable to find font " + name + " (FcConfigSubstitute)"};
  }
  FcDefaultSubstitute(pattern.get());

  FcResult result{};
  fc_pattern font{FcFontMatch(nullptr, pattern.get(), &result)};
  if (!font || result != FcResultMatch) {
    throw std::runtime_error{"unable to find font " + name + " (FcFontMatch)"};
  }

  FcChar8* file{nullptr};
  if (FcPatternGetString(font.get(), FC_FILE, 0, &file) != FcResultMatch ||
      file == nullptr) {
    throw std::runtime_error{"unable to obtain filename for font " + name};
  }

  //NOLINTNEXTLINE(*-reinterpret-cast)
  return reinterpret_cast<const char*>(file);
}
