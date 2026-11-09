#pragma once

#include <cstdint>
#include <vector>

namespace autodeco {

struct RGB {
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
};

/**
 * A theme is just a palette plus a couple of flags. Everything the mod
 * generates is derived from this, so adding a new theme means adding one
 * entry in Themes.cpp and nothing else.
 */
struct Theme {
    char const* id;
    char const* name;

    // Color channel 1000
    RGB background;
    // Color channel 1001
    RGB ground1;
    // Color channel 1002
    RGB ground2;
    // Color channel 1003 (the ground line)
    RGB line;
    // Color channel 1004 (3D line)
    RGB threeDL;
    // Color channel 1008 (light background)
    RGB lightBG;

    // Used by the pulse triggers
    RGB accent;

    // Whether this theme looks right with a screen shake at the start
    bool shake;
};

std::vector<Theme> const& allThemes();

} // namespace autodeco
