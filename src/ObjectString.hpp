#pragma once

#include <string>

#include "Themes.hpp"

namespace autodeco {

/** Knobs that come from the mod's settings / the editor state. */
struct ApplyOptions {
    /** Seconds the background colors take to fade in (0 = instant). */
    float fadeIn = 0.5f;
    /** 0 = off, 1 = low, 2 = medium, 3 = high. */
    int pulseDensity = 2;
    /** Add a short screen shake at the level start. */
    bool shake = false;
    /** Add oscillating move triggers for the selected background objects. */
    bool drift = false;
    /** Group ID the drift triggers target. */
    int driftGroup = 9999;
    /** How far (in editor units) the background drifts. */
    int driftDistance = 120;
    /** Roughly how long the level is; used to spread triggers out. */
    float levelLength = 3000.f;
};

/**
 * Builds a GD level-object string containing every trigger the theme needs.
 * The result is fed straight into LevelEditorLayer::createObjectsFromString.
 *
 * This function is deliberately free of Geode/GD headers so it can be unit
 * tested outside the game.
 */
std::string buildTriggerString(Theme const& theme, ApplyOptions const& opts);

/** Number of objects in a string produced by buildTriggerString. */
int countObjects(std::string const& objectString);

} // namespace autodeco
