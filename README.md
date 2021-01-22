# Emulator collection for Nintendo® Game & Watch™

This is a very quick and dirty port of the [retro-go](https://github.com/ducalex/retro-go) emulator collection that is intended to run on the Nintendo® Game & Watch™ 2020 edition.

Currently playable

- GB / GBC (Gameboy / Gameboy Color)
- NES (Nintendo Entertainment System)
- SMS (Sega Master System)
- GG (Sega Game Gear)

## How to report issues

:exclamation: Please read this before reporting issues.

You may run the script `./report_issue.sh` and follow the steps lined out, or continue reading:

Please include the following:

- Which console
- The full name of the ROM you are running, e.g. "Super_Tilt_Bro_(E).nes"
- The git hash of this repo and the submodule. Run the following: `git describe --all --long --dirty=-dirty; cd retro-go-stm32; git describe --all --long --dirty=-dirty`

With this information, please head over to the [Discord](https://discord.gg/vVcwrrHTNJ) and post in the #support channel. If you don't want to use discord for some reason, please create an issue.

### Troubleshooting

- Do you have any changed files, even if you didn't intentionally change them? Please run `git reset --hard` to ensure an unchanged state.
- Did you pull but forgot to update the submodule? Run `git submodule update --init --recursive` to ensure that the submodules are in sync.
- Always run `make clean` before building something new. The makefile should handle incremental builds, but please do this first before reporting issues.
- If you have limited resources, remove the `-j$(nproc)` flag from the `make` command, i.e. run `make flash`.
- It is still not working? Try the classic trouble shooting methods: Disconnect power to your debugger and G&W and connect again. Try programming the [Base](https://github.com/ghidraninja/game-and-watch-base) project first to ensure you can actually program your device.
- Still not working? Ok, head over to #support on the discord and let's see what's going on.

## How to build

### Prerequisites

- You will need a recent [arm-gcc-none-eabi toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads). **10.2.0 and later are known to work well**. Please make sure it's installed either in your PATH, or set the environment variable `GCC_PATH` to the `bin` directory inside the extracted directory (e.g. `/opt/gcc-arm-none-eabi-10-2020-q4-major/bin`, `/Applications/ARM/bin` for macOS).
- In order to run this on a Nintendo® Game & Watch™ [you need to first unlock it](https://github.com/ghidraninja/game-and-watch-backup/).

### Building

Note: `make -j8` is used as an example. You may use `make -j$(nproc)` on Linux or `make -j$(sysctl -n hw.logicalcpu)` on Mac, or just write the number of threads you want to use, e.g. `make -j8`.

```bash
# Configure the debug adapter you want to use.
# stlink is also the default, but you may set it to something else:
# export ADAPTER=jlink
# export ADAPTER=rpi
export ADAPTER=stlink

# Clone and build flashloader:

git clone https://github.com/ghidraninja/game-and-watch-flashloader

cd game-and-watch-flashloader

make -j8

cd ..

# Clone this repo with submodules:

git clone --recurse-submodules https://github.com/kbeckmann/game-and-watch-retro-go

cd game-and-watch-retro-go

# Place GB roms in `./roms/gb/`, NES roms in `./roms/nes/`, SMS roms in `./roms/sms/`, GG roms in `./roms/gg/`:
# cp /path/to/rom.gb ./roms/gb/
# cp /path/to/rom.nes ./roms/nes/
# cp /path/to/rom.sms ./roms/sms/
# cp /path/to/rom.nes ./roms/gg/

# On a Mac running make < v4 you have to manually download the HAL package using :
# % make download_sdk

# Build and program external and internal flash.
# Note: If you are using the 16MB external flash, build using:
#           make -j8 LARGE_FLASH=1 flash
#       A custom flash size may be specified with the EXTFLASH_SIZE variable.

make -j8 flash
```

### If you are a developer

- If you need to change the project settings and generate c-code from stm32cubemx, make sure to not have a dirty working copy as the tool will overwrite files that will need to be perhaps partially reverted. Also update Makefile.common in case new drivers are used.

### Known issues (Please do not report these)

- Settings are not persistent

### GB Features / todo

- [x] Key input support
- [x] Audio support (works well!)
- [x] Video support (uses RGB565 color mode)
- [X] Audio volume
- [X] Power button -> deep sleep (saves and loads state)
- [X] VSync
- [X] State saving/loading
- [X] Support multiple ROMs
- [X] OSD menu

### NES Features / todo

- [x] Key input support
- [x] Audio support (works well)
- [x] Video support (uses indexed colors w/ a configurable palette)
- [X] Audio volume
- [X] Power button -> deep sleep (saves and loads state)
- [X] VSync
- [X] State saving/loading
- [X] Support multiple ROMs
- [X] OSD menu

## Build and flash using Docker

To reduce the number of potential pitfalls in installation of various software, a Dockerfile is provided containing everything needed to compile and flash retro-go to your Nintendo® Game & Watch™.

Steps to build and flash from a docker container (on Linux, e.g. Archlinux or Ubuntu):

```bash
# Clone this repo
git clone --recursive https://github.com/kbeckmann/game-and-watch-retro-go

# cd into it
cd game-and-watch-retro-go

# Place roms in ./roms/gb and ./roms/nes accordingly

# for MacOS users, run make download_sdk

# Build the docker image (takes a while)
docker build -f Dockerfile --tag kbeckmann/retro-go-builder .

# Run it with usb passthrough. Set your ADAPTER and LARGE_FLASH appropriately.
docker run --rm -it --privileged -v /dev/bus/usb:/dev/bus/usb kbeckmann/retro-go-builder make ADAPTER=stlink LARGE_FLASH=0 -j$(nproc) flash

# In case you get access errors when flashing, you may run sudo inside the docker container. The proper way is to fix the udev rules, but at least this is a way forward in case you are stuck.
# docker run --rm -it --privileged -v /dev/bus/usb:/dev/bus/usb kbeckmann/retro-go-builder sudo -E make ADAPTER=stlink LARGE_FLASH=0 -j$(nproc) flash

```

## Backing up and restoring save state files

Save states can be backed up using `./dump_saves.sh build/gw_retro_go.elf`. Make sure to use the elf file that matches what is running on your device! It is a good idea to keep this elf file in case you want to back up at a later time.

This downloads all save states to the local directory `./save_states`. Each save state will be located in `./save_states/<emu>/<rom name>.save`.

After this, it's safe to change roms, pull new code and build & flash the device.

Save states can then be programmed to the device using a newer elf file with new code and roms. To do this, run `./program_saves.sh build/gw_retro_go.elf` - this time with the _new_ elf file that matches what's running on the device. Save this elf file for backup later on.

`program_saves.sh` will upload all save state files that you have backed up that are also included in the elf file. E.g Let's say you back up saves for rom A, B and C. Later on, you add a new rom D but remove A, then build and flash. When running the script, the save states for B and C will be programmed and nothing else.

## Contact, discussion

Please join the [Discord](https://discord.gg/vVcwrrHTNJ).

## LICENSE

This project is licensed under the GPLv2. Some components are also available under the MIT license. Respective copyrights apply to each component.
