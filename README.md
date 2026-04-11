# Arcade64-EssencePatch
What is Arcade64 Essence Patch?

This is a version that was developed to include unofficial support for the [ARCADE64](https://arcade.mameworld.info/) emulator that has not yet been included or recognized to date, which does not allow the incorporation of corrections, new mechanisms and optimization for the system.

I am only supporting the operating systems 64x bits, Windows 7, Windows 8, Windows 10 and Windows 11.

What has been optimized in this version?
---------------------------------------
This command will help us have much easier access to the settings:

* To enter the BIOS, press the "0" key.
* To play in windowed mode, press the "Spacebar" key.
* To accelerate the game by 3x, press the "Backspace" key.

Players 1 and 2 use this controller by default:

* To move, use the arrow keys "Up, Down, Left, Right."
* To perform actions, use the "A, S, D, Z, X, C, Q, W, E" keys.

Added custom buttons and autofire.

Removed warning screen, startups, Decryption screen texts.

Removed the following annoying messages: “WRONG LENGTH”, “NO_DUMP”, “WRONG CHECKSUMS”, “BAD_DUMP”.

Support reading IPS (By Eziochiu)

Added movement optimization V.4 for all fighting games (By GSC2007).

The Arcade64 "GUI" EKMAME source code has been implemented (By KAZE).

It is already pre-configured, enforce aspect ratio is disabled, full screen mode and tricks are already enabled by default.

The cheat reference function, if the cloned game does not have a cheat file, it will reference the cheat file of the main ROM. (By KAZE).

Supports UI DPI (The resolution 1920 x 1080 / 2560 x 1600) (By 缘来是你).

Supports optimized search function (By 缘来是你).

Supports XML export (By 缘来是你).

Supports IPS optimization (By 缘来是你).

Supports game list language files and multilingual title display (By 缘来是你).

Supports Skip CRC/IPS Check (By 缘来是你).

NEOGEO, PGM, driver supports key combination settings (By KAZE).

How to compile
---------------------------------------
In order to compile this version we will need to download the [ARCADE64](https://github.com/Robbbert/abcdefg/tags) source codes. How do we know which version we need? We will have to locate the latest compilation that I have released publicly.

And we will apply this command to start the compilation:
```
make OSD=winui PTR64=1 SUBTARGET=arcade SYMBOLS=0 NO_SYMBOLS=1 DEPRECATED=0
```

The compilation [TOOL](https://github.com/mamedev/buildtools/releases) is suggested to be 7.0 msys64 (Jan 11, 2022).

Open Source Software Projects
------------------------------
Although the source code is free to use, please note that the use of this code for any commercial exploitation or use of the project for fundraising purposes is prohibited.
