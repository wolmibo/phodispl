#include "phodispl/config.hpp"
#include "phodispl/window.hpp"

#include "build-config.h"

#include <bits/getopt_core.h>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include <getopt.h>

#include <pixglot/codecs.hpp>

#include <logcerr/log.hpp>



namespace {
  void print_help() {
    std::cout <<
      "Usage: phodispl [options] <input1>..\n"
      "\n"
      "Options:\n"
      " -h, --help           show this help and exit\n"
      " -v, --version        show version information and exit\n"
      " -V, --verbose        enable verbose logging (use twice for debug output)\n"
      " -c, --config=PATH    load configuration from PATH\n"
      "\n"
      "As input, you can provide ...\n"
      " ... a single file to show the file and phodispl lets you navigate\n"
      "   through all the image files in the same directory.\n"
      " ... a single directory to show the first image file in this\n"
      "   directory and phodispl lets you navigate through all the images in this\n"
      "   directory.\n"
      " ... multiple files to show the first image and to only let you navigate\n"
      "   through the selected images.\n"
    << std::endl;
  }



  void print_version() {
    std::cout << "phodispl " VERSION_STR "\n\n";

    std::vector<std::string_view> format_list;

    for (const auto& codec: pixglot::list_codecs()) {
      std::ranges::copy(pixglot::mime_types(codec), back_inserter(format_list));
    }

    int  max_width{80};
    int  width    {0};
    bool not_first{false};

    std::cout << "Supported formats:\n";
    for (const auto& fmt: format_list) {
      if (not_first) {
        std::cout << ", ";
        width += 2;
      }
      if (width > max_width) {
        std::cout << "\n";
        width = 0;
      }
      std::cout << fmt;
      width += fmt.size();
      not_first = true;
    }
    std::cout << std::endl;
  }





  [[nodiscard]] std::string list_formats() {
    std::string out;
    for (const auto& codec: pixglot::list_codecs()) {
      for (const auto& mime: pixglot::mime_types(codec)) {
        out += std::string{mime} + ";";
      }
    }
    return out;
  }



  void print_desktop(const std::filesystem::path& path) {
    std::cout <<
      "[Desktop Entry]\n"
      "Type=Application\n"
      "Name=PhoDispl\n"
      "Comment=\"Highly responsive, minimalistic image viewer for wayland\"\n"
      "Terminal=false\n"
      "Exec=" << path.string() << " %U\n"
      "TryExec=" << path.string() << "\n"
      "Keywords=Picture;Slideshow;Graphics;\n"
      "Categories=Viewer;2DGraphics;RasterGraphics;\n"
      "MimeType=" << list_formats()
      << std::endl;
  }



  [[nodiscard]] logcerr::severity verbosity_level(int verbosity) {
    switch (verbosity) {
      case 0:  return logcerr::severity::log;
      case 1:  return logcerr::severity::verbose;
      default: return logcerr::severity::debug;
    }
  }
}



int main(int argc, char* argv[]) {
  std::span args{argv, static_cast<size_t>(argc)};



  bool                       help       {false};
  bool                       version    {false};
  int                        verbosity  {0};
  std::optional<std::string> config_path{};

  std::optional<std::filesystem::path> desktop_file_path{};
  std::optional<std::filesystem::path> check_config{};

  std::vector<std::filesystem::path> filenames;

  static std::array<option, 7> long_options = {
    option{"help",         no_argument,       nullptr, 'h'},
    option{"version",      no_argument,       nullptr, 'v'},
    option{"verbose",      no_argument,       nullptr, 'V'},
    option{"config",       required_argument, nullptr, 'c'},
    option{"desktop-file", required_argument, nullptr, 1000},
    option{"check-config", required_argument, nullptr, 1001},
    option{nullptr,        0,                 nullptr, 0},
  };

  int c {-1};
  while ((c = getopt_long(args.size(), args.data(), "hvVc:",
                          long_options.data(), nullptr)) != -1) {
    switch (c) {
      case 'h':
        help = true;
        break;
      case 'v':
        version = true;
        break;
      case 'V':
        verbosity++;
        break;
      case 'c':
        if (optarg != nullptr) {
          config_path = std::string{optarg};
        }
        break;
      case 1000:
        if (optarg != nullptr) {
          desktop_file_path = std::filesystem::path{optarg};
        }
        break;
      case 1001:
        if (optarg != nullptr) {
          check_config = std::filesystem::path{optarg};
        }
        break;
      default:
        break;
    }
  }

  logcerr::output_level(verbosity_level(verbosity));

  while (optind < argc) {
    filenames.emplace_back(args[optind++]);
  }

  if (help) {
    print_help();
    return 0;
  }

  if (version) {
    print_version();
    return 0;
  }

  if (desktop_file_path) {
    print_desktop(*desktop_file_path);
    return 0;
  }

  int error{};

  try {
    if (check_config) {
      load_config(*check_config, true);
      return 0;
    }


    load_config(config_path);

    window{filenames}.run();

  } catch (std::exception& ex) {
    logcerr::error(ex.what());
    error = -1;
  } catch (...) {
    logcerr::error("...");
    error = -2;
  }


  return error;
}
