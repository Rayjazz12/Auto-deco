# Auto Deco

A [Geode](https://geode-sdk.org) mod for Geometry Dash 2.2081 that themes a whole level in one click.

Finish your layout, open the editor, press **Auto Deco**, pick a theme, and the mod writes the
background colours, an animated background pulse, an optional screen shake and a drifting
background layer into the level for you.

## Themes

| Theme | Look |
| --- | --- |
| Hell | Dark reds, molten orange ground line, screen shake |
| Futuristic | Deep blue with cyan highlights |
| Neon | Purple with pink lines and a mint pulse |
| Heaven | Bright sky blue, white lines |
| Void | Near-black with a violet pulse and shake |
| Retro | Indigo with a warm gold line |
| Toxic | Dark green with acid-green lines and shake |
| Sunset | Plum background with a warm orange line |

Themes live in `src/Themes.cpp`. Adding one is a single entry: id, name, six colours
(background, ground, ground 2, line, 3D line, light background), an accent colour for the
pulses, and whether it wants a screen shake.

## Using it

1. Open a level in the editor.
2. Optional: select the decoration objects you want to drift (your background layer, for
   example). If nothing is selected, the mod applies colours and pulses only.
3. Click **Auto Deco** on the left edge of the screen and pick a theme.
4. Not happy? `Ctrl+Z` undoes the creation, or press **Remove theme** in the same panel.

Everything is added through the editor, so it is saved with the level and other players see it
without the mod.

## What it generates

| Trigger | What it does |
| --- | --- |
| Color (899) × 6 | Sets background (1000), ground (1001, 1002), line (1003), 3D line (1004) and light background (1008) |
| Pulse (1006) | Repeats the theme accent colour on the background channel along the level |
| Shake (1520) | Short screen shake at level start, for the themes that want it |
| Move (901) | Alternating `+`/`-` offsets on the drift group, so the background sways instead of jumping |

## Settings

- **Colour fade-in** – seconds the colours take to fade in (0 = instant).
- **Background pulse density** – Off / Low / Medium / High.
- **Screen shake** – allow the shake trigger on themes that ask for it.
- **Drift the selected objects** – group the selection and add the swaying move triggers.
- **Drift group ID** – defaults to 9999; change it if that group is already used.
- **Drift distance** – how far the background moves per sway.

## Building

Requires the Geode SDK matching `mod.json` (currently Geode **v5.11.2**, GD **2.2081**).

```sh
# one-time setup
geode sdk install
geode sdk install-binaries

# build (installs into your GD profile)
geode build

# or with CMake directly
cmake -B build
cmake --build build --config RelWithDebInfo
```

If CMake complains that the Geode or GD version does not match, set `"geode"` in `mod.json`
to the version of the SDK you have installed.

The mod ID is `rayjazz.auto-deco`; change it (and `"developer"`) in `mod.json` if you publish
under a different name on the Geode index.

### Testing the trigger generator without the game

The part that is easy to get wrong — the generated level-object string — is plain C++ with no
Geode dependencies, so it can be tested on its own:

```sh
g++ -std=c++20 -Wall -Wextra -Isrc tests/object_string_test.cpp \
    src/ObjectString.cpp src/Themes.cpp -o /tmp/auto-deco-tests
/tmp/auto-deco-tests
```

The tests parse the generated string the same way the game does and check the trigger IDs,
colour channels, pulse/move keys, and counts for every theme, plus the no-effects and
short-level cases.

## Status / limitations

- The trigger string generator is unit-tested (see above) and all 8 themes pass.
- The in-game part (editor button, theme panel, applying, undo) has **not** been run inside
  Geometry Dash, because it cannot be built or launched in this environment (no GD, no Geode
  SDK for Windows/macOS). Expect to adjust button placement or trigger values after a first
  in-game test.
- **Remove theme** only knows about objects created in the current editor session. After
  closing and reopening the level, use `Ctrl+Z` or delete the triggers manually.
- The wind-back-and-forth drift is a sway, not true parallax: GD has no single "moving
  background" object, so real parallax still comes from the decoration objects you select.
- Colours are set with built-in channels, so they override the level's saved background
  colour. That is intentional, but it means player-created colour triggers later in the level
  still win, as usual in GD.
