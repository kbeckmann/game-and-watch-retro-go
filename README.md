# Emulator collection for Nintendo® Game & Watch™

This is a very quick and dirty port of the [retro-go](https://github.com/ducalex/retro-go) emulator collection that is intended to run on the Nintendo® Game & Watch™ 2020 edition.

Currently playable
- GB
- NES


# How to report issues

:exclamation: Please read this before reporting issues.

You may run the script `./report_issue.sh` and follow the steps lined out, or continue reading:

Please include the following:

- Which console (NES or GB)
- The full name of the ROM you are running, e.g. "Super_Tilt_Bro_(E).nes"
- The git hash of this repo and the submodule. Run the following: `git describe --all --long --dirty=-dirty; cd retro-go-stm32; git describe --all --long --dirty=-dirty`

With this information, please head over to the [Discord](https://discord.gg/vVcwrrHTNJ) and post in the #support channel. If you don't want to use discord for some reason, please create an issue.


## Troubleshooting

- Do you have any changed files, even if you didn't intentionally change them? Please run `git reset --hard` to ensure an unchanged state.
- Did you pull but forgot to update the submodule? Run `git submodule update --init --recursive` to ensure that the submodules are in sync.
- Always run `make -j clean` before building something new. The makefile should handle incremental builds, but please do this first before reporting issues.
- It is still not working? Try the classic trouble shooting methods: Disconnect power to your debugger and G&W and connect again. Try programming the [Base](https://github.com/ghidraninja/game-and-watch-base) project first to ensure you can actually program your device.
- Still not working? Ok, head over to #support on the discord and let's see what's going on.


# Building steps

- You will need a recent [arm-gcc-none-eabi toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads). Please make sure it's installed either in your PATH, or set the environment variable `GCC_PATH` to the `bin` directory inside the extracted directory (e.g. `/opt/gcc-arm-none-eabi-10-2020-q4-major/bin`).
- In order to run this on a Nintendo® Game & Watch™ [you need to first unlock it](https://github.com/ghidraninja/game-and-watch-backup/).
- Clone this repo with submodules:
    - `git clone --recurse-submodules https://github.com/kbeckmann/game-and-watch-retro-go`
- Clone and build `https://github.com/ghidraninja/game-and-watch-flashloader`.
    - `git clone https://github.com/ghidraninja/game-and-watch-flashloader`
    - `cd game-and-watch-flashloader`
    - `make -j`
- Note that `game-and-watch-flashloader` must be placed in the same directory as `game-and-watch-retro-go`, e.g. `/home/user/projects/game-and-watch-flashloader` and `/home/user/projects/game-and-watch-retro-go`.
- Place GB roms in `roms/gb/` and NES roms in `roms/nes`.
- Run `python3 parse_roms.py` to import the roms to the project.
- Build and program external and internal flash: `make -j flash_all`


If you are a developer:
- If you need to change the project settings and generate c-code from stm32cubemx, make sure to not have a dirty working copy as the tool will overwrite files that will need to be perhaps partially reverted. Also update Makefile.common in case new drivers are used.


## Known issues (Please do not report these)

- Settings are not persistent


## GB Features / todo

- [x] Key input support
- [x] Audio support (works well!)
- [x] Video support (uses RGB565 color mode)
- [X] Audio volume
- [X] Power button -> deep sleep (saves and loads state)
- [X] VSync
- [X] State saving/loading
- [X] Support multiple ROMs
- [X] OSD menu


## NES Features / todo
- [x] Key input support
- [x] Audio support (works well)
- [x] Video support (uses indexed colors w/ a configurable palette)
- [X] Audio volume
- [X] Power button -> deep sleep (saves and loads state)
- [X] VSync
- [X] State saving/loading
- [X] Support multiple ROMs
- [X] OSD menu


# Contact, discussion

Please join the [Discord](https://discord.gg/vVcwrrHTNJ).


# LICENSE

This project is licensed under the GPLv2. Some components are also available under the MIT license. Respective copyrights apply to each component.

