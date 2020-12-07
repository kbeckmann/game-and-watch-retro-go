#!/bin/bash

echo "Which console are you using (NES or GB): "
read console

echo "What is the full name of the ROM you are running, e.g. \"Super_Tilt_Bro_(E).nes\": "
read rom

HASH1=$(git describe --all --long --dirty=-dirty)
HASH2=$(cd retro-go-stm32; git describe --all --long --dirty=-dirty)

echo "Please post the following in the #support channel in the Discord https://discord.gg/vVcwrrHTNJ "
echo "---------------"
echo "I am having an issue with [$console] when running [$rom]."
echo "Git hash of main repo [$HASH1] and submodule [$HASH2]."
echo "My problem is: <fill in here>"
echo "---------------"

