# Soulsy

Soulsy is a minimal-features Souls-style HUD for Skyrim SE and AE. It is inspired by hotkey HUD mods like Elden Equip, iEquip, and LamasTinyHud. It is in fact a fork of [LamasTinyHud](https://github.com/mlthelama/LamasTinyHUD)!

## User documentation

Soulsy lets you set hotkeys for managing what you have equipped or readied in four equipment slots:

- right hand: weapons, spells, two-handed weapons
- left hand: one-handed weapons, spells, shields, torches, lanterns
- shouts and minor powers
- a utility slot: potions, scrolls, poisons, food

Soulsy sets up _cycles_ for each of these equipment slots. For example, if you want to switch between Flames and Healing spells in your left hand, you'd add each of them to your left hand cycle. For your right hand, you might set up a long sword with an anti-undead enchantment, a dagger with Soul Trap, and a bow. Pressing the key assigned to a slot moves to the next item in your cycle and equips it (or readies it, in the case of the utility slot). If you tap the key several times quickly, you'll advance through the cycle and then equip the item you were on when you stopped tapping the button. 

Cycles are capped at 10 items max. This cap is not yet configurable.

To add or remove an item from a cycle, bring up the inventory or magic menu, hover over the item, and press the hotkey for the cycle you want to change. If the item is not in the cycle for that slot and it's appropriate for the slot, it'll be added. If it's already in that cycle, it'll be removed. The favorites menu also works.

Soulsy also has a hotkey for activating your selected utility item. This is the only category of item that Soulsy will try to activate for you; everything else needs to used the same way the base game has you use them. The last hotkey-able shortcut is for hiding and showing the HUD. There is an MCM setting if you want the HUD to fade out when you're not in combat or don't have your weapons readied.

That's it for the feature set. Soulsy does not attempt to select the best ammo, potion or poison the way iEquip does. It equips what you tell it to equip, as quickly and reliably as it can. Soulsy also does not offer in-game layout editing, though you can edit and theme the layout by editing a toml file outside the game.

### Settings options

TKTK: screenshot of MCM, explanation of defaults, etc

## How to theme the HUD

TKTKTK

## Building

Soulsy is a Rust and C++ project, using CMake to drive Cargo to build the Rust parts. The application logic is implemented in Rust, with a bridge to the C++ libraries required to implement an SKSE plugin. I have not attempted to build it anywhere other than on Windows.

- [Rust](https://rustup.rs) set up for Windows (not for WSL)
- [Visual Studio 2022](https://visualstudio.microsoft.com) with C++ compilers installed
- [CMake](https://cmake.org)
- [vcpkg](https://github.com/microsoft/vcpkg) with `VCPKG_ROOT` set in a user environment variable
- at the moment, a dependency on python to package up the archive

The plugin requires the following vcpkg libraries, which will be installed for you:

- [CommonLibSSE-NG](https://github.com/CharmedBaryon/CommonLibSSE-NG)
- [spdlog](https://github.com/gabime/spdlog)
- [simpleini](https://github.com/brofield/simpleini)  <---- maybe not
- [nanosvg](https://github.com/memononen/nanosvg) (for rastering the svgfiles)
- [imgui](https://github.com/ocornut/imgui) (for displaying ui elements)

If you have [just](https://just.systems) installed, the justfile has recipes for convenient setup and builds. `cargo install just` if you do not have it.

## Credits

All of the UI code is retained from [LamasTinyHud](https://www.nexusmods.com/skyrimspecialedition/mods/82545), so [mlthelama](https://github.com/mlthelama) gets all the props. I also learned a lot about how to make an SKSE plugin by reading their source. Give that HUD a try if you don't like the souls-game style, or want a UI you can edit in-game. The original has more features than this one does! It's also the only hotkeys hud mod I tried that worked well in my game, so that's a testimonial.

The icons for the built-in theme are the usual SkyUI icons, plus the future book true-type font. The other layout data is adapted from the [Untarnished UI skin](https://www.nexusmods.com/skyrimspecialedition/mods/82545) for LamasTinyHUD by [MinhazMurks](https://www.nexusmods.com/skyrimspecialedition/users/26341279).

[cxx](https://cxx.rs/) made developing the C++/Rust bridge a snap. This crate unlocks Rust as a viable language for all of your modding needs. The only drawback is that async Rust is not yet supported. You can work around this using channels if you need to; see the docs.

## License

GPL-3.0.
