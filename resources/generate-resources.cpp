#include <iostream>
#include <filesystem>
#include <fstream>
#include <span>

#include <logcerr/log.hpp>



namespace {
  std::pair<std::string, size_t> read_file(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
      throw std::runtime_error{"unable to open file " + path.string()};
    }

    std::string output = "  ";
    size_t i = 0;
    for (; input.peek() != -1; ++i) {
      if (i % 20 == 0 && i > 0) {
        output += "\n  ";
      }
      output += logcerr::format("0x{:02x},", input.get());
    }
    return {output, i};
  }



  std::string define_file(const std::filesystem::path& path, std::string_view id) {
    auto [str, size] = read_file(path);

    return logcerr::format("\n"
      "namespace {{ constexpr std::array<unsigned char, {1}> content_{0}_ {{\n"
      "{2}\n"
      "}};}}\n"
      "\n"
      "std::string_view resources::{0}_sv() {{\n"
      "  return std::string_view{{\n"
      "    //NOLINTNEXTLINE(*-reinterpret-cast)\n"
      "    reinterpret_cast<const char*>(content_{0}_.data()),\n"
      "    content_{0}_.size()\n"
      "  }};\n"
      "}}\n"
      "std::span<const std::byte> resources::{0}_data() {{\n"
      "  return std::as_bytes(std::span{{content_{0}_}});\n"
      "}}\n",
      id, size, str);
  }



  std::string declare_file(std::string_view id) {
    return logcerr::format(
        "[[nodiscard]] std::string_view           {0}_sv();\n"
        "[[nodiscard]] std::span<const std::byte> {0}_data();\n",
    id);
  }
}





int main(int argc, const char* argv[]) {
  std::span<const char*> args{argv, static_cast<size_t>(argc)};

  if (args.size() < 3 || args.size() % 2 != 1) {
    std::cout << "usage: " << args[0]
      << " <header> <source> [<resource1 name>... <resource1>...]\n";
    return 1;
  }

  std::span names = args.subspan(3, (args.size() - 3) / 2);
  std::span files = args.subspan(3 + names.size());

  if (names.size() != files.size()) {
    throw std::runtime_error{"mismatch between input files and names"};
  }





  std::ofstream source(args[2]);
  if (!source) {
    throw std::runtime_error{std::string{"unable to write to "} + args[2]};
  }

  source << "#include " << std::filesystem::path{args[1]}.filename() << "\n";
  source << "#include <array>\n";



  std::ofstream header(args[1]);
  if (!header) {
    throw std::runtime_error{std::string{"unable to write to "} + args[1]};
  }

  header << "// This file is generated during build. DO NOT EDIT.\n";
  header << "#pragma once\n";
  header << "#include <span>\n";
  header << "#include <string_view>\n";
  header << "namespace resources {\n";



  for (size_t i = 0; i < names.size(); ++i) {
    header << declare_file(names[i]);
    source << define_file(files[i], names[i]);
  }

  header << "}\n";
}
