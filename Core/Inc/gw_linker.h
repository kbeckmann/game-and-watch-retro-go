#pragma once

#include <stdint.h>

extern uint8_t __SAVEFLASH_START__;
extern uint8_t __SAVEFLASH_END__;
extern uint8_t __EXTFLASH_START__;

extern uint8_t __NULLPTR_LENGTH__;

extern uint8_t _Stack_Redzone_Size;
extern uint8_t _stack_redzone;

extern uint8_t _heap_start;
extern uint8_t _heap_end;


extern uint32_t _siramdata;
extern uint32_t __ram_exec_start__;
extern uint32_t __ram_exec_end__;

extern uint32_t _sitcram_hot;
extern uint32_t __itcram_hot_start__;
extern uint32_t __itcram_hot_end__;
extern uint8_t __configflash_start__;
extern uint8_t __configflash_end__;


// If this is not an array the compiler might put in a memory_chk with dest_size 1...
extern void * __RAM_EMU_START__[];
extern void * _OVERLAY_NES_LOAD_START[];
extern uint8_t _OVERLAY_NES_SIZE;
extern void * _OVERLAY_NES_BSS_START[];
extern uint8_t _OVERLAY_NES_BSS_SIZE;
extern void * _OVERLAY_GB_LOAD_START[];
extern uint8_t _OVERLAY_GB_SIZE;
extern void * _OVERLAY_GB_BSS_START[];
extern uint8_t _OVERLAY_GB_BSS_SIZE;
extern void * _OVERLAY_SMS_LOAD_START[];
extern uint8_t _OVERLAY_SMS_SIZE;
extern void * _OVERLAY_SMS_BSS_START[];
extern uint8_t _OVERLAY_SMS_BSS_SIZE;
extern void * _OVERLAY_PCE_LOAD_START[];
extern uint8_t _OVERLAY_PCE_SIZE;
extern void * _OVERLAY_PCE_BSS_START[];
extern uint8_t _OVERLAY_PCE_BSS_SIZE;
extern void * _OVERLAY_GW_LOAD_START[];
extern uint8_t _OVERLAY_GW_SIZE;
extern void * _OVERLAY_GW_BSS_START[];
extern uint8_t _OVERLAY_GW_BSS_SIZE;

extern uint8_t *_NES_ROM_UNPACK_BUFFER;
extern uint8_t _NES_ROM_UNPACK_BUFFER_SIZE;

extern uint8_t *_GB_ROM_UNPACK_BUFFER;
extern uint8_t _GB_ROM_UNPACK_BUFFER_SIZE;
