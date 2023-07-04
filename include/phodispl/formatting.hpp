// Copyright (c) 2023 wolmibo
// SPDX-License-Identifier: MIT

#ifndef PHODISPL_FORMATTING_HPP_INCLUDED
#define PHODISPL_FORMATTING_HPP_INCLUDED

#include <functional>
#include <string>
#include <string_view>



template<typename T>
using measure_function = std::move_only_function<float(T)>;



[[nodiscard]] std::u32string wrap_text(std::u32string_view, float,
    measure_function<std::u32string_view>);

#endif // PHODISPL_FORMATTING_HPP_INCLUDED
