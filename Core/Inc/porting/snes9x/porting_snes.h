#pragma once

#define DRAM_ATTR_SNES __attribute__((section (".itcram_hot_data")))
#define IRAM_ATTR_SNES __attribute__((section (".itcram_hot_text")))

