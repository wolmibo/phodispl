#ifndef PHODISPL_CONFIG_HPP_INCLUDED
#define PHODISPL_CONFIG_HPP_INCLUDED

#include "phodispl/animation.hpp"
#include "phodispl/config-types.hpp"

#include <chrono>
#include <filesystem>
#include <optional>
#include <string_view>




struct config {
  public:
    config() = default;

    explicit config(std::string_view, bool = false);

    void assert_equal(const config&) const;



    scale_filter filter     {scale_filter::linear};
    bool         watch_fs   {true};
    float        gamma      {2.2f};
    float        input_speed{1.f};



    animation_curve           animation_view_next_curve{animation_curve::sinusoidal};
    std::chrono::milliseconds animation_view_next_ms{0};

    animation_curve           animation_view_snap_curve{animation_curve::sinusoidal};
    std::chrono::milliseconds animation_view_snap_ms{0};



    uint32_t    theme_heading_size {32};
    uint32_t    theme_text_size    {18};

    color       theme_heading_color{0.25f,  0.5f, 0.75f, 1.0f};
    color       theme_text_color   {0.75f, 0.75f, 0.75f, 1.0f};

    color       theme_background   {0.f, 0.f, 0.f, 1.f};

    std::string theme_font{"Sans"};



    uint32_t cache_keep_backward{3};
    uint32_t cache_load_backward{1};
    uint32_t cache_keep_forward{3};
    uint32_t cache_load_forward{2};



    bool         fl_empty_wd              {true};
    listing_mode fl_empty_wd_dir          {listing_mode::supported};

    listing_mode fl_single_file           {listing_mode::always};
    bool         fl_single_file_parent    {true};
    listing_mode fl_single_file_parent_dir{listing_mode::supported};
    bool         fl_single_file_demote    {true};

    listing_mode fl_single_dir            {listing_mode::supported};

    listing_mode fl_multi_file            {listing_mode::always};
    listing_mode fl_multi_dir             {listing_mode::supported};



    bool                      il_play_available   {true};

    bool                      il_partial          {true};
    bool                      il_partial_flush    {true};
    std::chrono::milliseconds il_partial_threshold{250};
    std::chrono::milliseconds il_partial_interval {20};
};



[[nodiscard]] const config& global_config();

void load_config(const std::optional<std::filesystem::path>&, bool = false);

#endif // PHODISPL_CONFIG_HPP_INCLUDED
