#pragma once

#define DRAM_ATTR_SNES __attribute__((section (".itcram_hot_data")))
#define IRAM_ATTR_SNES __attribute__((section (".itcram_hot_text")))

// #define ATTR_SNES_HOT_1 __attribute__((section (".itcram_hot_data")))
// #define ATTR_SNES_HOT_2 __attribute__((section (".snes_hot2")))
// #define ATTR_SNES_HOT_3 __attribute__((section (".snes_hot3")))
// #define ATTR_SNES_HOT_4 __attribute__((section (".snes_hot4")))
// #define ATTR_SNES_HOT_5

