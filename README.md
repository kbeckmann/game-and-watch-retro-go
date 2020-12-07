# WIP/POC NES (and more) Emulator for Nintendo Game and Watch

This is a very quick and dirty port of the [retro-go](https://github.com/ducalex/retro-go) emulator for the Nintendo Game and Watch.

Currently playable
- GB
- NES


# How to report issues

:exclamation: Please read this before reporting issues.

Include the following:

- Which console (NES or GB)
- The full name of the ROM you are running, e.g. "Super_Tilt_Bro_(E).nes"
- The git hash of this repo and the submodule. Run the following: `git describe --all --long; cd retro-go-stm32; git describe --all --long`

With this information, please head over to the [Discord](https://discord.gg/vVcwrrHTNJ) and post in the #support channel. If you don't want to use discord for some reason, please create an issue.


## Troubleshooting

- Do you have any changed files, even if you didn't intentionally change them? Please run `git reset --hard` to ensure an unchanged state.
- Did you pull but forgot to update the submodule? Run `git submodule update --init --recursive` to ensure that the submodules are in sync.
- Always run `make -f Makefile.nes -j clean` or `make -f Makefile.nes -j clean` before building something new. The makefile should handle incremental builds, but please do this first before reporting issues.
- It is still not working? Try the classic trouble shooting methods: Disconnect power to your debugger and G&W and connect again. Try programming the [Base](https://github.com/ghidraninja/game-and-watch-base) project first to ensure you can actually program your device.
- Still not working? Ok, head over to #support on the discord and let's see what's going on.


# Common building steps

- Clone this repo with submodules: `git clone --recurse-submodules https://github.com/kbeckmann/game-and-watch-retro-go`
- Clone and build `https://github.com/ghidraninja/game-and-watch-flashloader`. The 'game-and-watch-flashloader' folder must be placed in the same dir as 'game-and-watch-retro-go' folder.
- Generate HAL and support files by opening `gw_retrogo.ioc` in `stm32cubemx` and press the `Generate` button. This will however change a couple of files, so run `git reset --hard` to recover the original contents.


## GB

- Follow the common steps above
- Import a GB ROM file: `./update_gb_rom.sh my_rom.gb``
- Build `make -f Makefile.gb -j`
- Program external flash `make -f Makefile.gb flash_extmem`
- Program internal flash `make -f Makefile.gb flash`


### Known issues (Please do not report these)

- [ ] Audio sounds bad, glitchy


### GB Features / todo

- [x] Key input support
- [x] Audio support (works not so well)
- [x] Video support (uses RGB565 color mode)
- [X] Mute audio
- [X] Power button -> deep sleep (saves and loads state)
- [X] VSync
- [X] Sate saving/loading
- [ ] Support multiple ROMs
- [ ] OSD menu


## NES

- Follow the common steps above
- Import a NES ROM file: `./update_nes_rom.sh my_rom.nes``
- Build `make -f Makefile.nes -j`
- Program external flash `make -f Makefile.nes flash_extmem`
- Program internal flash `make -f Makefile.nes flash`


### NES Features / todo
- [x] Key input support
- [x] Audio support (works well)
- [x] Video support (uses indexed colors w/ a configurable palette)
- [x] Mute audio
- [X] Power button -> deep sleep (saves and loads state)
- [X] VSync
- [X] Sate saving/loading
- [ ] Support multiple ROMs
- [ ] OSD menu


# Contact, discussion

Please join the [Discord](https://discord.gg/vVcwrrHTNJ).


# LICENSE

This project is licensed under the GPLv2. Some components are also available under the MIT license. Respective copyrights apply to each component.

