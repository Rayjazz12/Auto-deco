#include "Themes.hpp"

namespace autodeco {

namespace {

Theme const g_themes[] = {
    {
        "hell",
        "Hell",
        {46, 8, 8},    // bg
        {24, 4, 4},    // g1
        {12, 2, 2},    // g2
        {255, 64, 24}, // line
        {70, 10, 10},  // 3dl
        {60, 10, 10},  // lbg
        {255, 96, 32}, // accent
        true,          // shake
    },
    {
        "futuristic",
        "Futuristic",
        {8, 18, 46},
        {6, 12, 30},
        {3, 7, 18},
        {0, 220, 255},
        {10, 40, 80},
        {20, 60, 120},
        {0, 230, 255},
        false,
    },
    {
        "neon",
        "Neon",
        {18, 4, 34},
        {10, 2, 20},
        {6, 1, 12},
        {255, 0, 200},
        {40, 0, 70},
        {36, 8, 64},
        {0, 255, 190},
        false,
    },
    {
        "heaven",
        "Heaven",
        {150, 200, 240},
        {120, 170, 220},
        {96, 140, 196},
        {255, 255, 255},
        {200, 225, 255},
        {210, 236, 255},
        {255, 255, 210},
        false,
    },
    {
        "void",
        "Void",
        {4, 4, 6},
        {2, 2, 3},
        {1, 1, 2},
        {120, 120, 140},
        {10, 10, 14},
        {12, 10, 20},
        {140, 120, 255},
        true,
    },
    {
        "retro",
        "Retro",
        {30, 20, 60},
        {20, 12, 40},
        {14, 8, 28},
        {255, 200, 60},
        {50, 24, 40},
        {44, 26, 60},
        {255, 160, 0},
        false,
    },
    {
        "toxic",
        "Toxic",
        {6, 20, 6},
        {4, 14, 4},
        {2, 8, 2},
        {150, 255, 0},
        {12, 40, 12},
        {18, 52, 18},
        {120, 255, 60},
        true,
    },
    {
        "sunset",
        "Sunset",
        {48, 16, 42},
        {34, 10, 30},
        {22, 6, 20},
        {255, 140, 90},
        {80, 24, 40},
        {96, 36, 56},
        {255, 190, 90},
        false,
    },
};

} // namespace

std::vector<Theme> const& allThemes() {
    static std::vector<Theme> const themes(std::begin(g_themes), std::end(g_themes));
    return themes;
}

} // namespace autodeco
