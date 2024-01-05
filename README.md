# PMC

PMC is a simple, yet flexible code injection framework for Pokémon Black 2 and White 2. It's powered by the `libRPM` dynamic loading library under the hood, reducing the process of code injection to no more than dragging and dropping a file. PMC will hook itself into the game's overlay loading routine by placing executable code in the secure area, and executing it from `main()` via a hook. From there on, PMC will silently swap patch modules in and out as the parent process needs them.

## Installation
You can either snatch a binary build from the [releases](https://github.com/kingdom-of-ds-hacking/PMC/releases) page, or build it from source using the instructions below.

Once you do that, open your project in [CTRMap-CE](https://github.com/kingdom-of-ds-hacking/CTRMap-CE) **(ensuring the latest version of the [CTRMapV](https://github.com/kingdom-of-ds-hacking/CTRMapV) plugin is installed)**, go to the `Extras` tab, and install PMC.

## Building PMC
### Prerequisites
PMC can be built using the CMake build system.

Requirements:
- [CMake](https://cmake.org/)
- [ExtLib](https://github.com/HelloOO7/ExtLib)
- [libRPM](https://github.com/HelloOO7/libRPM)
- [arm-none-eabi toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) (follow instructions based on your system).

### Building
#### Dependencies
As stated above, you will need `ExtLib` and `libRPM`. These dependencies can be provided in two ways:
- Doing a recursive clone of `PMC`.
- Cloning the `ExtLib` and `libRPM` repositories to the same parent directory as `PMC` (such that they are adjacent).

#### Visual Studio Code
PMC was built with Visual Studio Code in mind; as such, you can vastly simplify the build process by utilizing Visual Studio Code and the CMake Extension. 

1. **Ensure you have the [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) extension installed and activated, along with all of the pre-requisites.**
2. Clone the PMC repository:
```
$ git clone https://github.com/kingdom-of-ds-hacking/PMC.git --recursive
```
3. Open the PMC directory in Visual Studio Code (`File > Open Folder...`).
4. Open the command palette (`Ctrl + Shift + P`) and run `CMake: Configure`. **Resolve any errors if there are any.**
5. Open the command palette (`Ctrl + Shift + P`) and run `CMake: Build` (or click the `Build` button at the bottom of Visual Studio Code).

If all goes well, you should have a `PMC.elf`, which can then be linked with the [RPM Authoring Tools](https://github.com/HelloOO7/RPMAuthoringTools) and a proper external symbol database (ESDB). These ESDBs should be provided with the [swan](https://github.com/kingdom-of-ds-hacking/swan) headers repository, under `IRDO.yml` and `IREO.yml` for the USA Pokémon White 2 and Black 2, respectively.

#### Terminal
Assuming you are building PMC through the terminal (or some Linux-based environment), you can follow the steps below. For Windows users, you will just need to supplement the commands with the correct equivalents.

1. Clone the PMC repository and change directory into it:
```
$ git clone https://github.com/kingdom-of-ds-hacking/PMC.git --recursive
$ cd PMC
```

2. To setup the `build` folder, run the following commands:
```
$ mkdir build
$ cd build
```

3. Run the following command to build PMC:
```
$ cmake .. -DPMC_PLATFORM=ARMv5T
```

If all goes well, you should have a `PMC.elf`, which can then be linked with the [RPM Authoring Tools](https://github.com/HelloOO7/RPMAuthoringTools) and a proper external symbol database (ESDB). These ESDBs should be provided with the [swan](https://github.com/kingdom-of-ds-hacking/swan) headers repository, under `IRDO.yml` and `IREO.yml` for the USA Pokémon White 2 and Black 2, respectively.
