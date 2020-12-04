# WIP/POC NES (and more) Emulator for Nintendo Game and Watch

This is a very quick and dirty port of the [retro-go](https://github.com/ducalex/retro-go) emulator for the Nintendo Game and Watch.

Currently playable
- GB
- NES

# Common steps to get build
- Clone this repo with submodules: `git clone --recurse-submodules https://github.com/kbeckmann/game-and-watch-retro-go`
- Clone and build `https://github.com/ghidraninja/game-and-watch-flashloader`. The 'game-and-watch-flashloader' folder must be placed in the same dir as 'game-and-watch-retro-go' folder.
- Generate HAL and support files by opening `gw_retrogo.ioc` in `stm32cubemx` and press the `Generate` button.

## GB

- Follow the common steps above
- Import a GB ROM file: `./update_gb_rom.sh my_rom.gb``
- Build `make -f Makefile.gb -j`
- Program external flash `make -f Makefile.gb flash_extmem`
- Program internal flash `make -f Makefile.gb flash`

### GB Features / todo
- [x] Key input support
- [x] Audio support (works not so well)
- [x] Video support (uses RGB565 color mode)
- [ ] Mute audio
- [ ] Power button -> deep sleep (loses all memory when started)
- [ ] VSync
- [ ] Sate saving/loading
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
- [x] Power button -> deep sleep (loses all memory when started)
- [ ] VSync
- [ ] Sate saving/loading
- [ ] Support multiple ROMs
- [ ] OSD menu


# LICENSE
This project is licensed under the GPLv2. Some components are also available under the MIT license. Respective copyrights apply to each component.
