# Emulator collection for Nintendo® Game & Watch™

This is a port of the [retro-go](https://github.com/ducalex/retro-go) emulator collection that runs on the Nintendo® Game & Watch™: Super Mario Bros. system.

Supported emulators:

- ColecoVision (col)
- Gameboy / Gameboy Color (gb/gbc)
- Game & Watch / LCD Games (gw)
- Nintendo Entertainment System (nes)
- PC Engine / TurboGrafx-16 (pce)
- Sega Game Gear (gg)
- Sega Master System (sms)
- Sega SG-1000 (sg)

## Table of Contents
- [Emulator collection for Nintendo® Game & Watch™](#emulator-collection-for-nintendo-game--watch)
  - [Table of Contents](#table-of-contents)
  - [Controls](#controls)
    - [Macros](#macros)
  - [Troubleshooting / FAQ](#troubleshooting--faq)
  - [How to build](#how-to-build)
    - [Prerequisites](#prerequisites)
    - [Building](#building)
    - [Information for developers](#information-for-developers)
  - [Build and flash using Docker](#build-and-flash-using-docker)
  - [Backing up and restoring save state files](#backing-up-and-restoring-save-state-files)
  - [Screenshots](#screenshots)
  - [Upgrading the flash](#upgrading-the-flash)
  - [Advanced Flash Examples](#advanced-flash-examples)
    - [Custom Firmware (CFW)](#custom-firmware-cfw)
  - [Discord, support and discussion](#discord-support-and-discussion)
  - [LICENSE](#license)

## Controls

Buttons are mapped as you would expect for each emulator. `GAME` is mapped to `START`,
and `TIME` is mapped to `SELECT`. `PAUSE/SET` brings up the emulator menu.

By default, pressing the power-button while in a game will automatically trigger
a save-state prior to putting the system to sleep. Note that this WILL overwrite
the previous save-state for the current game.

### Macros

Holding the `PAUSE/SET` button while pressing other buttons have the following actions:

| Button combination    | Action                                                                 |
| --------------------- | ---------------------------------------------------------------------- |
| `PAUSE/SET` + `GAME`  | Store a screenshot. (Disabled by default on 1MB flash builds)          |
| `PAUSE/SET` + `TIME`  | Toggle speedup between 1x and the last non-1x speed. Defaults to 1.5x. |
| `PAUSE/SET` + `UP`    | Brightness up.                                                         |
| `PAUSE/SET` + `DOWN`  | Brightness down.                                                       |
| `PAUSE/SET` + `RIGHT` | Volume up.                                                             |
| `PAUSE/SET` + `LEFT`  | Volume down.                                                           |
| `PAUSE/SET` + `B`     | Load state.                                                            |
| `PAUSE/SET` + `A`     | Save state.                                                            |
| `PAUSE/SET` + `POWER` | Poweroff WITHOUT save-stating.                                         |

## Troubleshooting / FAQ

- Run `make help` to get a list of options to configure the build, and targets to perform various actions.
- Add `STATE_SAVING=0` as a parameter to `make` to disable save state support if more space is required.
- Do you have any changed files, even if you didn't intentionally change them? Please run `git reset --hard` to ensure an unchanged state.
- Did you run `git pull` but forgot to update the submodule? Run `git submodule update --init --recursive` to ensure that the submodules are in sync or run `git pull --recurse-submodules` instead.
- Run `make clean` and then build again. The makefile should handle incremental builds, but please try this first before reporting issues.
- If you have limited resources on your computer, remove the `-j$(nproc)` flag from the `make` command, i.e. run `make flash`.
- If you have changed the external flash and are having problems:
  - Run `make flash_test` to test it. This will erase the flash, write, read and verify the data.
  - If your chip was bought from e.g. ebay, aliexpress or similar places, you might have gotten a fake or bad clone chip. You can set `EXTFLASH_FORCE_SPI=1` to disable quad mode which seems to help for some chips.
- It is still not working? Try the classic trouble shooting methods: Disconnect power to your debugger and G&W and connect again. Try programming the [Base](https://github.com/ghidraninja/game-and-watch-base) project first to ensure you can actually program your device.
- Still not working? Ok, head over to #support on the discord and let's see what's going on.

## How to build

### Prerequisites

- You will need version 10 or later of [arm-gcc-none-eabi toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads). **10.2.0 and later are known to work well**. Please make sure it's installed either in your PATH, or set the environment variable `GCC_PATH` to the `bin` directory inside the extracted directory (e.g. `/opt/gcc-arm-none-eabi-10-2020-q4-major/bin`, `/Applications/ARM/bin` for macOS).
- In order to run this on a Nintendo® Game & Watch™ [you need to first unlock it](https://github.com/ghidraninja/game-and-watch-backup/).

### Building

Note: `make -j8` is used as an example. You may use `make -j$(nproc)` on Linux or `make -j$(sysctl -n hw.logicalcpu)` on Mac, or just write the number of threads you want to use, e.g. `make -j8`.

```bash
# Configure the debug adapter you want to use.
# stlink is also the default, but you may set it to something else:
# export ADAPTER=jlink
# export ADAPTER=rpi
export ADAPTER=stlink

# Clone this repo with submodules:

git clone --recurse-submodules https://github.com/kbeckmann/game-and-watch-retro-go

cd game-and-watch-retro-go

# Install python dependencies, this is optional for basic uses (but recommended!)
python3 -m pip install -r requirements.txt

# Place roms in the appropriate folders:
# cp /path/to/rom.gb ./roms/gb/
# cp /path/to/rom.nes ./roms/nes/
# etc. for each rom-emulator combination.

# On a Mac running make < v4 you have to manually download the HAL package by running:
# make download_sdk

# Build and program external and internal flash.
# Notes:
#     * If you are using a modified unit with a larger external flash,
#       set the EXTFLASH_SIZE_MB to its size in megabytes (MB) (16MB used in the example):
#           make -j8 EXTFLASH_SIZE_MB=16 flash

make -j8 flash
```

### Information for developers

If you need to change the project settings and generate c-code from stm32cubemx, make sure to not have a dirty working copy as the tool will overwrite files that will need to be perhaps partially reverted. Also update Makefile.common in case new drivers are used.

## Build and flash using Docker

<details>
  <summary>
    If you are familiar with Docker and prefer a solution where you don't have to manually install toolchains and so on, expand this section and read on.
  </summary>

  To reduce the number of potential pitfalls in installation of various software, a Dockerfile is provided containing everything needed to compile and flash retro-go to your Nintendo® Game & Watch™: Super Mario Bros. system. This Dockerfile is written tageting an x86-64 machine running Linux.

  Steps to build and flash from a docker container (running on Linux, e.g. Archlinux or Ubuntu):

  ```bash
  # Clone this repo
  git clone --recursive https://github.com/kbeckmann/game-and-watch-retro-go

  # cd into it
  cd game-and-watch-retro-go

  # Place roms in the appropriate directory inside ./roms/

  # Build the docker image (takes a while)
  make docker_build

  # Run the container.
  # The current directory will be mounted into the container and the current user/group will be used.
  # In order to be able to flash the device, the container is started with --priviliged and also mounts
  # in /dev/bus/usb. See Makefile.common for the exact command line that is executed if curious.
  make docker

  # Build and flash from inside the container:
  docker@76f83f2fc562:/opt/workdir$ make ADAPTER=stlink EXTFLASH_SIZE_MB=1 -j$(nproc) flash
  ```

</details>

## Backing up and restoring save state files

Save states can be backed up using `./scripts/saves_backup.sh build/gw_retro_go.elf`. Make sure to use the elf file that matches what is running on your device! It is a good idea to keep this elf file in case you want to back up at a later time. This can also be achieved with `make flash_saves_backup`.

This downloads all save states to the local directory `./save_states`. Each save state will be located in `./save_states/<emu>/<rom name>.save`.

After this, it's safe to change roms, pull new code and build & flash the device.

Save states can then be programmed to the device using a newer elf file with new code and roms. To do this, run `./scripts/saves_restore.sh build/gw_retro_go.elf` - this time with the _new_ elf file that matches what's running on the device. Save this elf file for backup later on. This can also be achieved with `make flash_saves_restore`.

`saves_restore.sh` will upload all save state files that you have backed up that are also included in the elf file. E.g Let's say you back up saves for rom A, B and C. Later on, you add a new rom D but remove A, then build and flash. When running the script, the save states for B and C will be programmed and nothing else.

You can also erase all of the save slots by running `make flash_saves_erase`.

## Screenshots

Screenshots can be captured by pressing `PAUSE/SET` + `GAME`. This feature is disabled by default if the external flash is 1MB (stock units), because it takes up 150kB in the external flash.

Screenshots can be downloaded by running `make dump_screenshot`, and will be saved as a 24-bit RGB PNG.

## Upgrading the flash

The Nintendo® Game & Watch™ comes with a 1MB external flash. This can be upgraded.

The flash operates at 1.8V so make sure the one you change to also matches this.

The recommended flash to upgrade to is MX25U12835FM2I-10G. It's 16MB, the commands are compatible with the stock firmware and it's also the largest flash that comes in the same package as the original.

:exclamation: Make sure to backup and unlock your device before changing the external flash. The backup process requires the external flash to contain the original data.

## Advanced Flash Examples

### Custom Firmware (CFW)
In order to install both the CFW (modified stock rom) and retro-go at the same time, a [patched version of openocd](https://github.com/kbeckmann/ubuntu-openocd-git-builder) needs to be installed and used.

Since we're using the patched version of openocd, we are also going to specify the `EXTENDED=1` flag, which can free up to an additional 128KB of extflash.

In this example, we'll be compiling retro-go to be used with a 64MB (512Mb) `MX25U51245GZ4I00` flash chip and [custom firmware](https://github.com/BrianPugh/game-and-watch-patch). The internal custom firmware will be located at `0x08000000`, which corresponds to `INTFLASH_BANK=1`. The internal retro-go firmware will be flashed to `0x08100000`, which corresponds to `INTFLASH_BANK=2`. The configuration of custom firmware described below won't use any extflash, so no `EXTFLASH_OFFSET` is specified. We can now build and flash the firmware with the following command:

```bash
make clean
make -j8 EXTFLASH_SIZE_MB=64 INTFLASH_BANK=2 EXTENDED=1 flash
```

To flash the custom firmware, [follow the CFW README](https://github.com/BrianPugh/game-and-watch-patch#retro-go). But basically, after you install the dependencies and place the correct files in the directory, run:
```bash
# In the game-and-watch-patch folder
make PATCH_PARAMS="--internal-only" flash_patched_int
```

## Discord, support and discussion

Please join the [Discord](https://discord.gg/vVcwrrHTNJ).

## LICENSE

This project is licensed under the GPLv2. Some components are also available under the MIT license. Respective copyrights apply to each component.
