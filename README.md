# PMC

PMC is a simple, yet flexible code injection framework for Pokémon White 2. It's powered by the `libRPM` dynamic loading library under the hood, reducing the process of code injection to no more than dragging and dropping a file. PMC will hook itself into the game's overlay loading routine during `GameInit()` and from there on will silently swap patch modules in and out as the parent process needs them.

## Building PMC

PMC can be built using the CMake build system, provided you supply its dependencies ([ExtLib](https://github.com/HelloOO7/ExtLib) and [libRPM](https://github.com/HelloOO7/libRPM)) either in the sibling directory of the repo, or in the `Framework`'s `externals` directory (a `--recursive` clone will download the submodules into that location automatically).

PMC targets the ARM architecture, meaning you'll want to pick the appropriate toolchain - specifically `arm-none-eabi-gcc` - to use with CMake. You can get the latest version here: https://developer.arm.com/downloads/-/gnu-rm

And as always, you can snatch a binary build from the [releases](https://github.com/kingdom-of-ds-hacking/PMC/releases) page.