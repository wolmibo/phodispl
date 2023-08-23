// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_FORMATTING_HPP_INCLUDED
#define PHODISPL_FORMATTING_HPP_INCLUDED

#include <filesystem>
#include <functional>
#include <string>
#include <string_view>



[[nodiscard]] std::u32string format_byte_size(size_t);
[[nodiscard]] std::u32string to_u32string(size_t);





template<typename T>
using measure_function = std::move_only_function<float(T)>;



[[nodiscard]] std::u32string wrap_text(std::u32string_view, float,
    measure_function<std::u32string_view>);




[[nodiscard]] std::filesystem::path nice_path   (const std::filesystem::path&);
[[nodiscard]] std::u32string        shorten_path(const std::filesystem::path&, float,
    measure_function<std::u32string_view>);

#endif // PHODISPL_FORMATTING_HPP_INCLUDED
