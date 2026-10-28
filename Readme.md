# Short Description

Smart Search makes SkyUI find items by what they are.

# Description

SkyUI only searches names. Smart Search lets you find items by type, material, magic effect, and enchantment too.

This doesn't modify any SkyUI files. You can use the same search box. Works in Inventory, Magic, Crafting, Container, Barter, and Gift menus, and with SkyUI UI theme overhauls(eg: norden ui).

## Features

- Search by type. Eg:- weapon, armor, potion, book, scroll, ore, ingot, soul gem.
- Search by material. Eg:- steel, ebony, dragonplate. Use m:steel to match material only.
- Search weapon type. Eg:- sword, dagger, bow, greatsword, mace.
- Search armor slot & weight. Eg:- helmet, boots, shield, heavy, light.
- Search magic by school. Eg:- destruction, restoration, conjuration, illusion.
- Search magic effects. Eg:- fire, paralysis, bound weapon, heal, summon, absorb.
- Search enchantment. Eg:- frost for frost weapons, fire for fire weapons.
- Search item state. Eg:- equipped for worn gear, favorite for starred items.
- Search a specific category with a tag. Eg:- w: weapon, a: armor, b: book, p: potion, f: food, i: ingredient, e: enchanted, m: material. (w: will list all weapons, so you can narrow it down)
- Customizable & translatable search words. Add your own words to any keyword, or map them to another language, in SmartSearch_<lang>.ini. Eg:- add "blade" to the sword keyword, or search in your own language. The included English SmartSearch_en.ini file is live, not just a template. Edit it to add your own aliases(keep your own updated ini in a seperate folder so updates wont replace them).

## Requirements

- Skyrim SE 1.5.97 or AE 1.6.x or AE 1.7.104
- SKSE64
- Address Library for SKSE Plugins
- SkyUI 5.2SE or SkyUI 6.x

## Installation

Safe to install/remove mid-game.

## Building from Source

- Visual Studio 2022 (C++23)
- [CommonLibSSE-NG](https://github.com/alandtse/CommonLibSSE-NG) (7.1.0)
  - Set the `CommonLibSSEPath` environment variable pointing to your CommonLibSSE-NG checkout, or place it at `extern/CommonLibSSE-NG`.