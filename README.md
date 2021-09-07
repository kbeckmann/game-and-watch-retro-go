# Emulator collection for Nintendo® Game & Watch™

This is a port of the [retro-go](https://github.com/ducalex/retro-go) emulator collection that is intended to run on the Nintendo® Game & Watch™: Super Mario Bros. 2020 edition.

Supported emulators:

- ColecoVision (col)
- Gameboy / Gameboy Color (gb/gbc)
- Game & Watch / LCD Games (gw)
- Nintendo Entertainment System (nes)
- PC Engine / TurboGrafx-16 (pce)
- Sega Game Gear (gg)
- Sega Master System (sms)
- Sega SG-1000 (sg)

## Controls

Buttons are mapped as you would expect for each emulator. `GAME` is mapped to `START`,
and `TIME` is mapped to `SELECT`. `PAUSE/SET` brings up the emulator menu.

By default, pressing the power-button while in a game will automatically trigger
a save-state prior to putting the system to sleep. Note that this WILL overwrite
the previous save-state for the current game.

### Macros

Holding the `PAUSE/SET` button while pressing other buttons have the following actions:

- `PAUSE/SET` + `TIME` = Toggle speedup between 1x and the last non-1x speed. Defaults to 1.5x.
- `PAUSE/SET` + `UP` = Brightness up.
- `PAUSE/SET` + `DOWN` = Brightness down.
- `PAUSE/SET` + `RIGHT` = Volume up.
- `PAUSE/SET` + `LEFT` = Volume down.
- `PAUSE/SET` + `B` = Load state.
- `PAUSE/SET` + `A` = Save state.
- `PAUSE/SET` + `POWER` = Poweroff WITHOUT save-stating.

## How to report issues

:exclamation: Please read this before reporting issues.

Please include the following:

- Name of the emulator (nes, gb, etc.)
- The full name of the ROM you are running, e.g. "Super_Tilt_Bro_(E).nes"
- The git hash of this repo and the submodule. Please run the following and include the output in the report: `git describe --all --long --dirty=-dirty; cd retro-go-stm32; git describe --all --long --dirty=-dirty`

With this information, please head over to the [Discord](https://discord.gg/vVcwrrHTNJ) and post in the #support channel. If you don't want to use discord for some reason, please create an issue.

### Troubleshooting

- Do you have any changed files, even if you didn't intentionally change them? Please run `git reset --hard` to ensure an unchanged state.
- Did you pull but forgot to update the submodule? Run `git submodule update --init --recursive` to ensure that the submodules are in sync.
- Run `make clean` and then build again. The makefile should handle incremental builds, but please try this first before reporting issues.
- If you have limited resources on your computer, remove the `-j$(nproc)` flag from the `make` command, i.e. run `make flash`.
- It is still not working? Try the classic trouble shooting methods: Disconnect power to your debugger and G&W and connect again. Try programming the [Base](https://github.com/ghidraninja/game-and-watch-base) project first to ensure you can actually program your device.
- Still not working? Ok, head over to #support on the discord and let's see what's going on.

## How to build

### Prerequisites

- You will need version 10 or later of [arm-gcc-none-eabi toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads). **10.2.0 and later are known to work well**. Please make sure it's installed either in your PATH, or set the environment variable `GCC_PATH` to the `bin` directory inside the extracted directory (e.g. `/opt/gcc-arm-none-eabi-10-2020-q4-major/bin`, `/Applications/ARM/bin` for macOS).
- `lz4` needs to be installed in order to compress ROMs.
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
#     * If you'd like to apply more advanced experimental ROM compression, add the
#       field COMPRESS=lzma to the make command.

make -j8 flash
```

### If you are a developer

- If you need to change the project settings and generate c-code from stm32cubemx, make sure to not have a dirty working copy as the tool will overwrite files that will need to be perhaps partially reverted. Also update Makefile.common in case new drivers are used.

## Build and flash using Docker

To reduce the number of potential pitfalls in installation of various software, a Dockerfile is provided containing everything needed to compile and flash retro-go to your Nintendo® Game & Watch™. Note that Linux is required as a host OS in order to flash the target.

Steps to build and flash from a docker container (on Linux, e.g. Archlinux or Ubuntu):

```bash
# Clone this repo
git clone --recursive https://github.com/kbeckmann/game-and-watch-retro-go

# cd into it
cd game-and-watch-retro-go

# Place roms in ./roms/gb and ./roms/nes accordingly

# Build the docker image (takes a while)
docker build -f Dockerfile --tag kbeckmann/retro-go-builder .

# Run it with usb passthrough. Set your ADAPTER and LARGE_FLASH appropriately.
docker run --rm -it --privileged -v /dev/bus/usb:/dev/bus/usb kbeckmann/retro-go-builder make ADAPTER=stlink EXTFLASH_SIZE_MB=1 -j$(nproc) flash

# In case you get access errors when flashing, you may run sudo inside the docker container. The proper way is to fix the udev rules, but at least this is a way forward in case you are stuck.
# docker run --rm -it --privileged -v /dev/bus/usb:/dev/bus/usb kbeckmann/retro-go-builder sudo -E make ADAPTER=stlink EXTFLASH_SIZE_MB=1 -j$(nproc) flash
```

## Experimental

Features mentioned in this section are disabled by default and should be considered
as upcoming features that need more testing. Give them a try!

### Advanced ROM Compression

The current default compression method is `lz4`, which is incredibly fast to both
compress and decompress. However, it's compression ratio pales in comparison
compared to some other compression method. We recently added [zopfli](andhttps://github.com/google/zopfli) and lzma
as selectable compressors to generate data to be decompressed on-device. This
yields a higher compression ratio (see graph below), but at the cost of
compression speed and (more importantly) decompression speed. Note that this
benchmark was done on a desktop, not on-device. Decompression has to be fast
enough to not be noticeable to the user, especially for gameboy games where memory
banks are dynamically decompressed on-demand.

<img src="assets/decompression-benchmark-annotated.jpg" width="800"/>

To use lzma compression, make sure the python dependencies are installed
and add `COMPRESS=lzma` to the `make` command. For example:

```
make -j8 EXTFLASH_SIZE_MB=16 COMPRESS=lzma flash
```

### Place external flash data at an offset

By specifying EXTFLASH_OFFSET you may place the external flash data at an offset to allow for multi booting.

## Backing up and restoring save state files

Save states can be backed up using `./scripts/saves_backup.sh build/gw_retro_go.elf`. Make sure to use the elf file that matches what is running on your device! It is a good idea to keep this elf file in case you want to back up at a later time. This can also be achieved with `make flash_saves_backup`.

This downloads all save states to the local directory `./save_states`. Each save state will be located in `./save_states/<emu>/<rom name>.save`.

After this, it's safe to change roms, pull new code and build & flash the device.

Save states can then be programmed to the device using a newer elf file with new code and roms. To do this, run `./scripts/saves_restore.sh build/gw_retro_go.elf` - this time with the _new_ elf file that matches what's running on the device. Save this elf file for backup later on. This can also be achieved with `make flash_saves_restore`.

`saves_restore.sh` will upload all save state files that you have backed up that are also included in the elf file. E.g Let's say you back up saves for rom A, B and C. Later on, you add a new rom D but remove A, then build and flash. When running the script, the save states for B and C will be programmed and nothing else.

You can also erase all of the save slots by running `make flash_saves_erase`.

## Upgrading the flash

The Nintendo® Game & Watch™ comes with a 1MB external flash. This can be upgraded.

The flash operates at 1.8V so make sure the one you change to also matches this.

The recommended flash to upgrade to is MX25U12835FM2I-10G. It's 16MB, the commands are compatible with the stock firmware and it's also the largest flash that comes in the same package as the original.

:exclamation: Make sure to backup and unlock your device before changing the external flash. The backup process requires the external flash to contain the original data.

## Advanced Flash Examples

### Tim's patched firmware
In this example, we'll be compiling retro-go to be used with a 64MB (512Mb) `MX25U51245GZ4I00` flash
chip and [tim's patched firmware](https://www.schuerewegen.tk/gnw/#win_stock_firmware_patcher).
The internal patched stock firmware is located at `0x08000000`, which corresponds to `INTFLASH_BANK=1`.
The internal retro-go firmware will be flashed to `0x08100000`, which corresponds to `INTFLASH_BANK=2`.
The stock extflash firmware is `1048576` bytes long at address `0x90000000`.
The stock extflash is taking up the first 1MB of the 64MB external flash chip, so we will set
`EXTFLASH_SIZE_MB=63`. Since we have to flash it after the end of the stock extflash,
we will set `EXTFLASH_OFFSET=1048576`. We can now build the firmware with the
following command:

```
make clean
make -j8 EXTFLASH_SIZE_MB=63 EXTFLASH_OFFSET=1048576 INTFLASH_BANK=2
```

To flash the produced binaries to your device, you have two options:
1. Using a windows computer with [these SPI flash external loaders](https://www.schuerewegen.tk/download/STM32CubeProgrammer%20External%20Loaders%20%282021-05-02%29.zip):

```
STM32_Programmer_CLI.exe -c port=SWD -w gw_retro_go_intflash.bin 0x08100000
STM32_Programmer_CLI.exe -c port=SWD reset=HWrst -w gw_retro_go_extflash.bin 0x90100000 -el "PATH_TO_THE_STLDR_FILE\MX25U51245G_GAME-AND-WATCH.stldr" -rst
```

2. Use a [patched version of openocd](https://github.com/kbeckmann/ubuntu-openocd-git-builder) that allows access to the undocumented flash regions of the microcontroller. For mac users, you can use [this homebrew formula](https://github.com/northskysl/homebrew-core/blob/master/Formula/open-ocd.rb):

```
make -j8 EXTFLASH_SIZE_MB=63 EXTFLASH_OFFSET=1048576 INTFLASH_BANK=2 flash
```

## Contact, discussion

Please join the [Discord](https://discord.gg/vVcwrrHTNJ).

## LICENSE

This project is licensed under the GPLv2. Some components are also available under the MIT license. Respective copyrights apply to each component.
