#!/bin/bash
set -xe

# col (placeholder)
dd if=/dev/zero of=roms/col/placeholder.col bs=1024 count=16

# gb
curl -L -o roms/gb/matrix-rain.gb https://github.com/wtjones/matrix-rain-gb/releases/download/0.0.3/matrix-rain.gb

# gg + sms
curl -L -o sega_tween.zip https://files.scene.org/get/demos/artists/ben_ryves/sega_tween.zip
unzip sega_tween.zip
mv 'Sega Tween (Normal).gg' roms/gg
mv 'Sega Tween (Normal).sms' roms/sms

# nes
curl -L -o roms/nes/pwn.nes https://github.com/Vector35/PwnAdventureZ/blob/master/PwnAdventureZ-csaw-withkeys.nes?raw=true

# pce
curl -L -o UP-19PCE.zip http://blockos.org/releases/demos/pce/UP-19PCE.zip
unzip UP-19PCE.zip
mv UP-19PCE.pce ./roms/pce/

# sg (placeholder)
dd if=/dev/zero of=roms/col/placeholder.col bs=1024 count=16
