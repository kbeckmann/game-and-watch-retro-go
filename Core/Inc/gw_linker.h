#pragma once

#include <stdint.h>

extern uint8_t __SAVE_START__;
extern uint8_t __SAVE_END__;
extern uint8_t __EXTFLASH_START__;

extern uint8_t __NULLPTR_LENGTH__;

extern uint8_t _Stack_Redzone_Size;
extern uint8_t _stack_redzone;

extern uint8_t _heap_start;
extern uint8_t _heap_end;

