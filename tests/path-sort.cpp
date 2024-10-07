#include "phodispl/path-compare.hpp"

#include <algorithm>
#include <iostream>
#include <filesystem>
#include <random>
#include <source_location>
#include <span>
#include <vector>




namespace {
  [[nodiscard]] std::string_view stringify(path_compare_method mode) {
    switch (mode) {
      case path_compare_method::semantic:      return "semantic";
      case path_compare_method::lexicographic: return "lexicographic";
    }
    return "<unknown>";
  }



  void assert(
      bool                 expression,
      std::source_location location = std::source_location::current()
  ) {
    if (!expression) {
      std::cout << "assertion failed: " << location.line() << '\n' << std::flush;
      exit(1);
    }
  }



  void test(
      path_compare_method                    mode,
      std::span<std::filesystem::path>       input,
      std::span<const std::filesystem::path> correct
  ) {
    std::mt19937_64 rng{};

    for (size_t i = 0; i < 1000; ++i) {
      std::ranges::shuffle(input, rng);
      std::ranges::sort(input, path_compare{mode});

      if (!std::ranges::equal(input, correct)) {
        std::cout << "mode: " << stringify(mode) << '\n';
        std::cout << "sorted:\n";
        std::ranges::for_each(input,   [](auto&& c) { std::cout << c << '\n'; });
        std::cout << "expected sorting:" << '\n';
        std::ranges::for_each(correct, [](auto&& c) { std::cout << c << '\n'; });
        std::cout << std::flush;
        exit(1);
      }
    }
  }
}



int main() {
  std::vector<std::filesystem::path> config_source{
    "file.dat", "file1.dat", "File1.dat", "File2.dat", "file2v2.dat", "file10.dat",
    "filefile.dat"
  };
  std::vector<std::filesystem::path> config_semantic{config_source};

  std::vector<std::filesystem::path> config_lexi{
    "File1.dat", "File2.dat", "file.dat", "file1.dat", "file10.dat", "file2v2.dat",
    "filefile.dat"
  };


  test(path_compare_method::lexicographic, config_source, config_lexi);
  test(path_compare_method::semantic,      config_source, config_semantic);


  assert(semantic_compare("earth", "electron"));
}
