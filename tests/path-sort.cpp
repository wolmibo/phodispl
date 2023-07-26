#include "phodispl/path-compare.hpp"

#include <algorithm>
#include <iostream>
#include <filesystem>
#include <span>
#include <vector>




[[nodiscard]] std::string_view stringify(path_compare_method mode) {
  switch (mode) {
    case path_compare_method::semantic:      return "semantic";
    case path_compare_method::lexicographic: return "lexicographic";
  }
  return "<unknown>";
}



void test(
    path_compare_method                    mode,
    std::span<std::filesystem::path>       input,
    std::span<const std::filesystem::path> correct
) {
  std::ranges::sort(input, path_compare{mode});

  if (!std::ranges::equal(input, correct)) {
    std::cout << "mode: " << stringify(mode) << '\n';
    std::cout << "sorted:\n";
    std::ranges::for_each(input,   [](auto&& c) { std::cout << c << '\n'; });
    std::cout << "expected sorting:" << std::endl;
    std::ranges::for_each(correct, [](auto&& c) { std::cout << c << '\n'; });
    exit(1);
  }
}



int main() {
  std::vector<std::filesystem::path> config_source{
    "file.dat", "file1.dat", "File2.dat", "file2v2.dat", "file10.dat", "filefile.dat"
  };
  std::vector<std::filesystem::path> config_semantic{config_source};

  std::vector<std::filesystem::path> config_lexi{
    "File2.dat", "file.dat", "file1.dat", "file10.dat", "file2v2.dat", "filefile.dat"
  };


  test(path_compare_method::lexicographic, config_source, config_lexi);
  test(path_compare_method::semantic,      config_source, config_semantic);
}
