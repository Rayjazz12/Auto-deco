#include "ObjectString.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace autodeco {

namespace {

// Trigger object IDs
constexpr int kColorTrigger = 899;
constexpr int kPulseTrigger = 1006;
constexpr int kShakeTrigger = 1520;
constexpr int kMoveTrigger = 901;

// Color channels
constexpr int kChannelBackground = 1000;
constexpr int kChannelGround1 = 1001;
constexpr int kChannelGround2 = 1002;
constexpr int kChannelLine = 1003;
constexpr int kChannel3DL = 1004;
constexpr int kChannelLightBG = 1008;

// Property keys (see gd.docs / GD level string format)
constexpr int kKeyX = 2;
constexpr int kKeyY = 3;
constexpr int kKeyRed = 7;
constexpr int kKeyGreen = 8;
constexpr int kKeyBlue = 9;
constexpr int kKeyDuration = 10;
constexpr int kKeyTargetColor = 23;
constexpr int kKeyMoveX = 28;
constexpr int kKeyMoveY = 29;
constexpr int kKeyEasing = 30;
constexpr int kKeyOpacity = 35;
constexpr int kKeyFadeIn = 45;
constexpr int kKeyHold = 46;
constexpr int kKeyFadeOut = 47;
constexpr int kKeyTargetGroup = 51;
constexpr int kKeyPulseTargetType = 52;
constexpr int kKeyShakeStrength = 75;
constexpr int kKeyShakeInterval = 84;

/** Triggers all sit near the bottom of the level, like GD's own ones. */
constexpr float kTriggerY = 15.f;
constexpr float kTriggerStartX = 15.f;

std::string formatNumber(float value, int decimals = 3) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(decimals) << value;
    auto text = stream.str();
    if (text.find('.') != std::string::npos) {
        while (!text.empty() && text.back() == '0') {
            text.pop_back();
        }
        if (!text.empty() && text.back() == '.') {
            text.pop_back();
        }
    }
    if (text == "-0") {
        text = "0";
    }
    return text;
}

int pulseSpacing(int density) {
    switch (density) {
        case 1: return 1400;
        case 2: return 800;
        case 3: return 450;
        default: return 0;
    }
}

void appendKey(std::string& out, int key, std::string const& value) {
    out += ',';
    out += std::to_string(key);
    out += ',';
    out += value;
}

void appendKey(std::string& out, int key, int value) {
    appendKey(out, key, std::to_string(value));
}

void appendColor(std::string& out, RGB const& color) {
    appendKey(out, kKeyRed, static_cast<int>(color.r));
    appendKey(out, kKeyGreen, static_cast<int>(color.g));
    appendKey(out, kKeyBlue, static_cast<int>(color.b));
}

/** Starts a new object in the string, up to (not including) the ';'. */
void startObject(std::string& out, int objectID, float x, float y) {
    out += "1,";
    out += std::to_string(objectID);
    appendKey(out, kKeyX, formatNumber(x, 2));
    appendKey(out, kKeyY, formatNumber(y, 2));
}

} // namespace

int countObjects(std::string const& objectString) {
    return static_cast<int>(std::count(objectString.begin(), objectString.end(), ';'));
}

std::string buildTriggerString(Theme const& theme, ApplyOptions const& opts) {
    std::string out;

    auto addColorTrigger = [&](int channel, RGB const& color) {
        startObject(out, kColorTrigger, kTriggerStartX, kTriggerY);
        appendKey(out, kKeyTargetColor, channel);
        appendColor(out, color);
        appendKey(out, kKeyDuration, formatNumber(opts.fadeIn, 2));
        appendKey(out, kKeyOpacity, std::string("1"));
        out += ';';
    };

    // Recolor every built-in background channel once, at the level start.
    addColorTrigger(kChannelBackground, theme.background);
    addColorTrigger(kChannelGround1, theme.ground1);
    addColorTrigger(kChannelGround2, theme.ground2);
    addColorTrigger(kChannelLine, theme.line);
    addColorTrigger(kChannel3DL, theme.threeDL);
    addColorTrigger(kChannelLightBG, theme.lightBG);

    // Repeated pulses on the background give the theme some life.
    int const spacing = pulseSpacing(opts.pulseDensity);
    if (spacing > 0 && opts.levelLength > kTriggerStartX) {
        constexpr int maxPulses = 3000;
        constexpr float fade = 0.55f;
        constexpr float hold = 0.15f;
        int pulses = 0;

        for (float x = kTriggerStartX + spacing; x < opts.levelLength && pulses < maxPulses; x += spacing) {
            startObject(out, kPulseTrigger, x, kTriggerY);
            appendKey(out, kKeyTargetGroup, kChannelBackground);
            appendKey(out, kKeyPulseTargetType, 0); // 0 = channel, 1 = group
            appendColor(out, theme.accent);
            appendKey(out, kKeyFadeIn, formatNumber(fade, 2));
            appendKey(out, kKeyHold, formatNumber(hold, 2));
            appendKey(out, kKeyFadeOut, formatNumber(fade, 2));
            out += ';';
            ++pulses;
        }
    }

    if (opts.shake) {
        startObject(out, kShakeTrigger, kTriggerStartX, kTriggerY);
        appendKey(out, kKeyDuration, formatNumber(0.4f, 2));
        appendKey(out, kKeyShakeStrength, formatNumber(3.f, 2));
        appendKey(out, kKeyShakeInterval, formatNumber(0.1f, 2));
        out += ';';
    }

    // Alternate +distance / -distance move triggers so the grouped background
    // drifts one way, then back, for the length of the level.
    if (opts.drift && opts.driftGroup > 0 && opts.driftDistance > 0 && opts.levelLength > 200.f) {
        constexpr float span = 700.f;
        constexpr float duration = 9.f;
        int const steps = std::clamp(
            static_cast<int>((opts.levelLength - 100.f) / span), 1, 24
        );

        for (int i = 0; i < steps; ++i) {
            float const x = 30.f + static_cast<float>(i) * span;
            int const offset = (i % 2 == 0) ? opts.driftDistance : -opts.driftDistance;

            startObject(out, kMoveTrigger, x, kTriggerY);
            appendKey(out, kKeyTargetGroup, opts.driftGroup);
            appendKey(out, kKeyMoveX, offset);
            appendKey(out, kKeyMoveY, 0);
            appendKey(out, kKeyDuration, formatNumber(duration, 2));
            appendKey(out, kKeyEasing, 0); // linear, so the drift never jerks
            out += ';';
        }
    }

    return out;
}

} // namespace autodeco
