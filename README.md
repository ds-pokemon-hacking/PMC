# PMC

PMC is a simple, yet flexible code injection framework for Pokémon Black 2 and White 2. It's powered by the `libRPM` dynamic loading library under the hood, reducing the process of code injection to no more than dragging and dropping a file. PMC will hook itself into the game's overlay loading routine during `GameInit()` and from there on will silently swap patch modules in and out as the parent process needs them.

## Installation
You can either snatch a binary build from the [releases](https://github.com/kingdom-of-ds-hacking/PMC/releases) page, or build it from source using the instructions below.

Once you do that, open your project in `CTRMap-CE` (ensuring the `CTRMapV` plugin is installed), go to the `Extras` tab, and install PMC.

## Building PMC
### Prerequisites
PMC can be built using the CMake build system.

Requirements:
- [CMake](https://cmake.org/)
- [ExtLib](https://github.com/HelloOO7/ExtLib)
- [libRPM](https://github.com/HelloOO7/libRPM)
- [arm-none-eabi toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) (follow instructions based on your system).

### Building
Assuming you are building PMC through the terminal (or some Linux-based environment), you can follow the steps below. For Windows users, you will just need to supplement the commands with the correct equivalents.

First, clone the PMC repository and change directory into it:
```
$ git clone https://github.com/kingdom-of-ds-hacking/PMC.git --recursive
$ cd PMC
``````

Then, to setup the `build` folder, run the following commands:
```
$ mkdir build
$ cd build
```

After this, you can now run the following command:
```
$ cmake .. -DPMC_PLATFORM=ARMv5T -DPMC_GAME=...
```

You will need to set the `PMC_GAME` variable to either `IRDO` (White 2, USA), or `IREO` (Black 2, USA). This can be done either from the command line (via `-DPMC_GAME=<game name>`), or through the tool you use to build CMake projects.

If all goes well, you should have a `PMC.elf`, which can then be linked with the RPM Authoring Tools and a proper external symbol database (ESDB). These ESDBs should be provided with the [swan](https://github.com/kingdom-of-ds-hacking/swan) headers repository.