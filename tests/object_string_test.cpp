// Plain C++ tests for the trigger-string generator. These do not need the
// Geode SDK or the game. Build them with:
//   g++ -std=c++20 -Wall -Wextra -Isrc tests/object_string_test.cpp
//       src/ObjectString.cpp src/Themes.cpp -o /tmp/auto-deco-tests
//
// They check the parts that are easy to get wrong and impossible to notice
// without Geometry Dash: the object string syntax, the trigger IDs and the
// per-theme trigger counts.

#include "../src/ObjectString.hpp"
#include "../src/Themes.hpp"

#include <cstdlib>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, std::string const& what) {
    if (!condition) {
        ++failures;
        if (failures <= 20) {
            std::cerr << "FAIL: " << what << "\n";
        }
    }
}

struct Property {
    int key;
    std::string value;
};

struct Object {
    int id;
    std::vector<Property> properties;

    bool has(int key) const {
        for (auto const& property : properties) {
            if (property.key == key) return true;
        }
        return false;
    }

    std::string get(int key) const {
        for (auto const& property : properties) {
            if (property.key == key) return property.value;
        }
        return {};
    }
};

std::vector<std::string> split(std::string const& text, char separator) {
    std::vector<std::string> parts;
    std::string current;
    for (char character : text) {
        if (character == separator) {
            parts.push_back(current);
            current.clear();
        } else {
            current += character;
        }
    }
    if (!current.empty()) parts.push_back(current);
    return parts;
}

std::vector<Object> parse(std::string const& objectString) {
    std::vector<Object> objects;
    for (auto const& chunk : split(objectString, ';')) {
        auto fields = split(chunk, ',');
        if (fields.size() < 2) continue;

        // Every field is a key/value pair: "1,<object id>,2,<x>,3,<y>,..."
        if (fields.size() % 2 != 0) {
            ++failures;
            std::cerr << "FAIL: odd number of fields in object\n";
            continue;
        }

        Object object{};
        for (std::size_t i = 0; i + 1 < fields.size(); i += 2) {
            int const key = std::atoi(fields[i].c_str());
            object.properties.push_back({key, fields[i + 1]});
        }
        // Key 1 is the object ID itself.
        for (auto const& property : object.properties) {
            if (property.key == 1) object.id = std::atoi(property.value.c_str());
        }
        objects.push_back(object);
    }
    return objects;
}

int countOf(std::vector<Object> const& objects, int id) {
    int total = 0;
    for (auto const& object : objects) {
        if (object.id == id) ++total;
    }
    return total;
}

autodeco::ApplyOptions defaults() {
    autodeco::ApplyOptions opts;
    opts.levelLength = 5000.f;
    opts.fadeIn = 0.5f;
    opts.pulseDensity = 2;
    opts.shake = true;
    opts.drift = true;
    opts.driftGroup = 9999;
    opts.driftDistance = 120;
    return opts;
}

void validate(std::string const& objectString, std::string const& themeName) {
    check(!objectString.empty(), themeName + ": string is not empty");
    check(objectString.back() == ';', themeName + ": string ends with ';'");
    check(objectString.find(",,") == std::string::npos, themeName + ": no empty fields");
    check(objectString.find("  ") == std::string::npos, themeName + ": no double spaces");

    auto objects = parse(objectString);
    check(!objects.empty(), themeName + ": has objects");
    check(static_cast<int>(objects.size()) == autodeco::countObjects(objectString),
          themeName + ": countObjects matches parsed objects");

    for (auto const& object : objects) {
        check(object.id == 899 || object.id == 1006 || object.id == 1520 || object.id == 901,
              themeName + ": only known trigger IDs");
        check(object.has(2) && object.has(3), themeName + ": object has x and y");
        check(!object.get(2).empty() && !object.get(3).empty(), themeName + ": x and y have values");
        for (auto const& property : object.properties) {
            check(property.key > 0, themeName + ": property keys are positive");
            check(!property.value.empty(), themeName + ": property values are not empty");
        }
    }

    // Six colour channels: bg, g1, g2, line, 3dl, lbg
    check(countOf(objects, 899) == 6, themeName + ": six colour triggers");
    check(countOf(objects, 1520) == 1, themeName + ": one shake trigger");

    // Colours target the built-in channels and carry RGB values.
    std::set<std::string> channels;
    for (auto const& object : objects) {
        if (object.id != 899) continue;
        check(object.has(23), themeName + ": colour trigger has a target channel");
        check(object.has(7) && object.has(8) && object.has(9), themeName + ": colour trigger has rgb");
        channels.insert(object.get(23));
    }
    check(channels == std::set<std::string>({"1000", "1001", "1002", "1003", "1004", "1008"}),
          themeName + ": colour triggers hit all six channels");

    // Pulses: medium density over 5000 units -> 6 pulses, on the bg channel.
    int pulses = countOf(objects, 1006);
    check(pulses > 0, themeName + ": has pulses");
    for (auto const& object : objects) {
        if (object.id != 1006) continue;
        check(object.get(51) == "1000", themeName + ": pulse targets the background channel");
        check(object.get(52) == "0", themeName + ": pulse targets a channel, not a group");
        check(object.has(45) && object.has(46) && object.has(47), themeName + ": pulse has fade/hold/fade");
    }

    // Drift: move triggers targeting the configured group.
    int moves = countOf(objects, 901);
    check(moves > 0, themeName + ": has move triggers");
    for (auto const& object : objects) {
        if (object.id != 901) continue;
        check(object.get(51) == "9999", themeName + ": move targets the drift group");
        check(object.has(28) && object.get(29) == "0", themeName + ": move is horizontal");
        check(object.has(10), themeName + ": move has a duration");
    }
}

} // namespace

int main() {
    auto const& themes = autodeco::allThemes();
    check(themes.size() >= 6, "at least six themes");
    check(themes.size() <= 8, "picker fits on one page");

    for (auto const& theme : themes) {
        check(theme.id && *theme.id, "theme has an id");
        check(theme.name && *theme.name, "theme has a name");
        validate(autodeco::buildTriggerString(theme, defaults()), theme.name);
    }

    // Pulses off: only colours, shake and drift remain.
    auto noPulse = defaults();
    noPulse.pulseDensity = 0;
    auto noPulseString = autodeco::buildTriggerString(themes.front(), noPulse);
    check(countOf(parse(noPulseString), 1006) == 0, "pulse density Off removes pulses");

    // Everything off: just the six colour triggers.
    auto bare = defaults();
    bare.pulseDensity = 0;
    bare.shake = false;
    bare.drift = false;
    auto bareString = autodeco::buildTriggerString(themes.front(), bare);
    auto bareObjects = parse(bareString);
    check(bareObjects.size() == 6, "no effects leaves only the six colour triggers");
    check(countOf(bareObjects, 899) == 6, "no effects still recolours everything");

    // Shorter level: fewer pulses, still at least one.
    auto shortLevel = defaults();
    shortLevel.levelLength = 900.f;
    auto shortString = autodeco::buildTriggerString(themes.front(), shortLevel);
    auto shortObjects = parse(shortString);
    check(countOf(shortObjects, 1006) == 1, "short level gets one pulse");
    check(countOf(shortObjects, 901) >= 1, "short level still gets a drift trigger");

    // Zero-length level must not produce a broken string.
    auto tiny = defaults();
    tiny.levelLength = 0.f;
    auto tinyString = autodeco::buildTriggerString(themes.front(), tiny);
    check(tinyString.back() == ';', "tiny level string is still terminated");
    check(countOf(parse(tinyString), 899) == 6, "tiny level still recolours");

    if (failures == 0) {
        std::cout << "all " << themes.size() << " themes passed trigger-string checks\n";
        return 0;
    }
    std::cerr << failures << " check(s) failed\n";
    return 1;
}
