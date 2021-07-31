#pragma GCC optimize ("-O0")

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "gw_flash.h"
#include "main.h"
#include "utils.h"

#define DBG(...) printf(__VA_ARGS__)
// #define DBG(...)

// Convenience macro to access the struct for a command
// in the active flash configuration.
#define CMD(cmd_name) &flash.config->commands[CMD_##cmd_name]

// 3-byte JEDEC ID to uint32_t
#define JEDEC_ID(_x0, _x1, _x2) ( (uint32_t) ( \
     ((_x0)       ) |                          \
     ((_x1) <<   8) |                          \
     ((_x2) <<  16)                            \
))

// Convenience macro to initialize a flash_cmd_t struct
#define CMD_DEF(_cmd, _instr_lines, _addr_lines, _addr_size, _data_lines, _dummy) \
{                                                                                 \
    .cmd         = (_cmd),                                                        \
    .instr_lines = (_instr_lines),                                                \
    .addr_lines  = (_addr_lines),                                                 \
    .addr_size   = (_addr_size),                                                  \
    .data_lines  = (_data_lines),                                                 \
    .dummy       = (_dummy)                                                       \
}

// Convenience macro to initialize a flash_config_t struct
#define FLASH_CONFIG_DEF(_commands, _erase1_size, _erase2_size, _erase3_size, _erase4_size, _set_quad, _init_fn) \
{                                                                                                                \
    .commands    = (_commands),                                                                                  \
    .erase_sizes = { (_erase1_size), (_erase2_size), (_erase3_size), (_erase4_size) },                           \
    .set_quad    = (_set_quad),                                                                                  \
    .init_fn     = (_init_fn),                                                                                   \
}

#define JEDEC_CONFIG_DEF(_x0, _x1, _x2, _name, _config) \
{                                                       \
    .jedec_id.u32 = JEDEC_ID((_x0), (_x1), (_x2)),      \
    .name         = (_name),                            \
    .config       = (_config),                          \
}

// Generic Status Register (SR) bits
#define STATUS_WIP_Pos   (0U)
#define STATUS_WIP_Msk   (1UL << STATUS_WIP_Pos)
#define STATUS_WEL_Pos   (1U)
#define STATUS_WEL_Msk   (1UL << STATUS_WEL_Pos)

// MX (Macronix) specific SR bits
#define STATUS_BP0_Pos   (2U)
#define STATUS_BP0_Msk   (1UL << STATUS_BP0_Pos)
#define STATUS_BP1_Pos   (3U)
#define STATUS_BP1_Msk   (1UL << STATUS_BP1_Pos)
#define STATUS_BP2_Pos   (4U)
#define STATUS_BP2_Msk   (1UL << STATUS_BP2_Pos)
#define STATUS_BP3_Pos   (5U)
#define STATUS_BP3_Msk   (1UL << STATUS_BP3_Pos)
#define STATUS_QE_Pos    (6U)
#define STATUS_QE_Msk    (1UL << STATUS_QE_Pos)
#define STATUS_SRWD_Pos  (7U)
#define STATUS_SRWD_Msk  (1UL << STATUS_SRWD_Pos)

// S (Spansion/Cypress/Infineon) specific CR bits
#define S_CR_QUAD_Pos    (1U)
#define S_CR_QUAD_Msk    (1UL << S_CR_QUAD_Pos)

typedef enum {
    LINES_0,        // Mapped to HAL_OSPI_*_NONE
    LINES_1,        // Mapped to HAL_OSPI_*_1_LINE
    LINES_4,        // Mapped to HAL_OSPI_*_4_LINES
} lines_t;

typedef enum {
    ADDR_SIZE_8B,   // Mapped to HAL_OSPI_ADDRESS_8_BITS
    ADDR_SIZE_16B,  // Mapped to HAL_OSPI_ADDRESS_16_BITS
    ADDR_SIZE_24B,  // Mapped to HAL_OSPI_ADDRESS_24_BITS
    ADDR_SIZE_32B,  // Mapped to HAL_OSPI_ADDRESS_32_BITS
} addr_size_t;

const uint32_t instruction_line_map[] = {
    [LINES_0] = HAL_OSPI_INSTRUCTION_NONE,
    [LINES_1] = HAL_OSPI_INSTRUCTION_1_LINE,
    [LINES_4] = HAL_OSPI_INSTRUCTION_4_LINES,
};

const uint32_t address_line_map[] = {
    [LINES_0] = HAL_OSPI_ADDRESS_NONE,
    [LINES_1] = HAL_OSPI_ADDRESS_1_LINE,
    [LINES_4] = HAL_OSPI_ADDRESS_4_LINES,
};

const uint32_t address_size_map[] = {
    [ADDR_SIZE_8B]  = HAL_OSPI_ADDRESS_8_BITS,
    [ADDR_SIZE_16B] = HAL_OSPI_ADDRESS_16_BITS,
    [ADDR_SIZE_24B] = HAL_OSPI_ADDRESS_24_BITS,
    [ADDR_SIZE_32B] = HAL_OSPI_ADDRESS_32_BITS,
};

const uint32_t data_line_map[] = {
    [LINES_0] = HAL_OSPI_DATA_NONE,
    [LINES_1] = HAL_OSPI_DATA_1_LINE,
    [LINES_4] = HAL_OSPI_DATA_4_LINES,
};

enum {
    CMD_WRSR,       // Write Status Register
    CMD_RDSR,       // Read Status Register
    CMD_RDCR,       // Read Configuration Register
    CMD_RDAR,       // Read Any Register
    CMD_WREN,       // Write Enable
    CMD_RDID,       // Read Identification
    CMD_RSTEN,      // Reset Enable
    CMD_RST,        // Reset

    CMD_CE,         // Chip Erase
    CMD_ERASE1,     // Usually 4kB  20h
    CMD_ERASE2,     // Usually 32kB 52h
    CMD_ERASE3,     // Usually 64kB d8h
    CMD_ERASE4,     // Usually unsupported

    CMD_PP,         // Page Program
    CMD_READ,       // Read Data Bytes

    CMD_COUNT,
};

typedef void (*init_fn_t)(void);

typedef struct {
    uint8_t            cmd;             // Command / Instruction
    uint8_t            instr_lines : 2; // Instruction Lines
    lines_t            addr_lines  : 2; // Address Lines
    addr_size_t        addr_size   : 2; // Address Size
    uint8_t            data_lines  : 2; // Data Lines
    uint8_t            dummy;           // Dummy Cycles
} flash_cmd_t;

typedef struct {
    const flash_cmd_t *commands;
    uint32_t           erase_sizes[4];  // [0] = ERASE1, ... [3] = ERASE4
    bool               set_quad;        // If quad mode should be enabled
    init_fn_t          init_fn;         // Chip/vendor specific init function
} flash_config_t;

typedef union {
    uint32_t u32;
    uint8_t  u8[4];
} jedec_id_t;

typedef struct {
    jedec_id_t            jedec_id;
    const char           *name;
    const flash_config_t *config;
} jedec_config_t;

const flash_cmd_t cmds_spi_24b[CMD_COUNT] = {
    // cmd                  cmd  i_lines  a_lines         a_size  d_lines  dummy
    [CMD_WRSR]   = CMD_DEF(0x01, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,    0),
    [CMD_RDSR]   = CMD_DEF(0x05, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,    0),
    [CMD_RDCR]   = CMD_DEF(0x15, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,    0),
    [CMD_WREN]   = CMD_DEF(0x06, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,    0),
    [CMD_RDID]   = CMD_DEF(0x9F, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,    0),
    [CMD_RSTEN]  = CMD_DEF(0x66, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,    0),
    [CMD_RST]    = CMD_DEF(0x99, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,    0),
    [CMD_CE]     = CMD_DEF(0x60, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,    0), // CE    Chip Erase
    [CMD_ERASE1] = CMD_DEF(0x20, LINES_1, LINES_1, ADDR_SIZE_24B, LINES_0,    0), // SE    Sector Erase
    [CMD_ERASE2] = CMD_DEF(0x52, LINES_1, LINES_1, ADDR_SIZE_24B, LINES_0,    0), // BE32K Block Erase 32K
    [CMD_ERASE3] = CMD_DEF(0xD8, LINES_1, LINES_1, ADDR_SIZE_24B, LINES_0,    0), // BE    Block Erase 64K
    [CMD_ERASE4] = { },
    [CMD_PP]     = CMD_DEF(0x02, LINES_1, LINES_1, ADDR_SIZE_24B, LINES_1,    0), // PP
    [CMD_READ]   = CMD_DEF(0x0B, LINES_1, LINES_1, ADDR_SIZE_24B, LINES_1,    8), // FAST_READ dummy=8
};

const flash_cmd_t cmds_quad_24b_mx[CMD_COUNT] = {
    // cmd                  cmd  i_lines  a_lines         a_size  d_lines  dummy
    [CMD_WRSR]   = CMD_DEF(0x01, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,    0),
    [CMD_RDSR]   = CMD_DEF(0x05, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,    0),
    [CMD_RDCR]   = CMD_DEF(0x15, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,    0),
    [CMD_WREN]   = CMD_DEF(0x06, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,    0),
    [CMD_RDID]   = CMD_DEF(0x9F, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,    0),
    [CMD_RSTEN]  = CMD_DEF(0x66, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,    0),
    [CMD_RST]    = CMD_DEF(0x99, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,    0),
    [CMD_CE]     = CMD_DEF(0x60, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,    0), // CE    Chip Erase
    [CMD_ERASE1] = CMD_DEF(0x20, LINES_1, LINES_1, ADDR_SIZE_24B, LINES_0,    0), // SE    Sector Erase
    [CMD_ERASE2] = CMD_DEF(0x52, LINES_1, LINES_1, ADDR_SIZE_24B, LINES_0,    0), // BE32K Block Erase 32K
    [CMD_ERASE3] = CMD_DEF(0xD8, LINES_1, LINES_1, ADDR_SIZE_24B, LINES_0,    0), // BE    Block Erase 64K
    [CMD_ERASE4] = { },
    [CMD_PP]     = CMD_DEF(0x38, LINES_1, LINES_4, ADDR_SIZE_24B, LINES_4,    0), // 4PP
    [CMD_READ]   = CMD_DEF(0xEB, LINES_1, LINES_4, ADDR_SIZE_24B, LINES_4,    6), // 4READ dummy=6
};

const flash_cmd_t cmds_quad_32b_mx[CMD_COUNT] = {
    // cmd                  cmd  i_lines  a_lines         a_size  d_lines  dummy
    [CMD_WRSR]   = CMD_DEF(0x01, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,    0),
    [CMD_RDSR]   = CMD_DEF(0x05, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,    0),
    [CMD_RDCR]   = CMD_DEF(0x15, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,    0),
    [CMD_WREN]   = CMD_DEF(0x06, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,    0),
    [CMD_RDID]   = CMD_DEF(0x9F, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,    0),
    [CMD_RSTEN]  = CMD_DEF(0x66, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,    0),
    [CMD_RST]    = CMD_DEF(0x99, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,    0),
    [CMD_CE]     = CMD_DEF(0x60, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,    0), // CE    Chip Erase
    [CMD_ERASE1] = CMD_DEF(0x21, LINES_1, LINES_1, ADDR_SIZE_32B, LINES_0,    0), // SE    Sector Erase
    [CMD_ERASE2] = CMD_DEF(0x5C, LINES_1, LINES_1, ADDR_SIZE_32B, LINES_0,    0), // BE32K Block Erase 32K
    [CMD_ERASE3] = CMD_DEF(0xDC, LINES_1, LINES_1, ADDR_SIZE_32B, LINES_0,    0), // BE    Block Erase 64K
    [CMD_ERASE4] = { },
    [CMD_PP]     = CMD_DEF(0x3E, LINES_1, LINES_4, ADDR_SIZE_32B, LINES_4,    0), // 4PP4B
    [CMD_READ]   = CMD_DEF(0xEC, LINES_1, LINES_4, ADDR_SIZE_32B, LINES_4,    6), // 4READ4B dummy=6
};

const flash_cmd_t cmds_quad_32b_s[CMD_COUNT] = {
    // cmd                  cmd  i_lines  a_lines         a_size  d_lines  dummy
    [CMD_WRSR]   = CMD_DEF(0x01, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,     0),
    [CMD_RDSR]   = CMD_DEF(0x05, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,     0),
    [CMD_RDCR]   = CMD_DEF(0x35, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,     0),
    [CMD_RDAR]   = CMD_DEF(0x65, LINES_1, LINES_1, ADDR_SIZE_24B, LINES_1,     8),
    [CMD_WREN]   = CMD_DEF(0x06, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,     0),
    [CMD_RDID]   = CMD_DEF(0x9F, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,     0),
    [CMD_RSTEN]  = CMD_DEF(0x66, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,     0),
    [CMD_RST]    = CMD_DEF(0x99, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,     0),
    [CMD_CE]     = CMD_DEF(0x60, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,     0), // BE    Bulk Erase
    [CMD_ERASE1] = CMD_DEF(0xDC, LINES_1, LINES_1, ADDR_SIZE_32B, LINES_0,     0), // 4SE   Sector Erase 256K
    [CMD_ERASE2] = { },
    [CMD_ERASE3] = { },
    [CMD_ERASE4] = { },
    [CMD_PP]     = CMD_DEF(0x12, LINES_1, LINES_1, ADDR_SIZE_32B, LINES_1,     0), // 4PP (no 4PP4B)
    [CMD_READ]   = CMD_DEF(0xEC, LINES_1, LINES_4, ADDR_SIZE_32B, LINES_4, 2 + 8), // 4QIOR
};

const flash_cmd_t cmds_quad_24b_issi[CMD_COUNT] = {
    // cmd                  cmd  i_lines  a_lines         a_size  d_lines  dummy
    [CMD_WRSR]   = CMD_DEF(0x01, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,    0),
    [CMD_RDSR]   = CMD_DEF(0x05, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,    0),
    [CMD_RDCR]   = CMD_DEF(0x15, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,    0),
    [CMD_WREN]   = CMD_DEF(0x06, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,    0),
    [CMD_RDID]   = CMD_DEF(0x9F, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_1,    0),
    [CMD_RSTEN]  = CMD_DEF(0x66, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,    0),
    [CMD_RST]    = CMD_DEF(0x99, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,    0),
    [CMD_CE]     = CMD_DEF(0x60, LINES_1, LINES_0, ADDR_SIZE_24B, LINES_0,    0), // CE    Chip Erase
    [CMD_ERASE1] = CMD_DEF(0x20, LINES_1, LINES_1, ADDR_SIZE_24B, LINES_0,    0), // SE    Sector Erase
    [CMD_ERASE2] = CMD_DEF(0x52, LINES_1, LINES_1, ADDR_SIZE_24B, LINES_0,    0), // BE32K Block Erase 32K
    [CMD_ERASE3] = CMD_DEF(0xD8, LINES_1, LINES_1, ADDR_SIZE_24B, LINES_0,    0), // BE    Block Erase 64K
    [CMD_ERASE4] = { },
    [CMD_PP]     = CMD_DEF(0x38, LINES_1, LINES_1, ADDR_SIZE_24B, LINES_4,    0), // PPQ
    [CMD_READ]   = CMD_DEF(0xEB, LINES_1, LINES_1, ADDR_SIZE_24B, LINES_4,    6), // FRQIO dummy=6
};

static void init_spansion(void);
static void init_mx(void);
static void init_issi(void);

const flash_config_t config_spi_24b       = FLASH_CONFIG_DEF(cmds_spi_24b,       0x01000,  0x8000, 0x10000, 0,  false, NULL);
const flash_config_t config_quad_24b_mx   = FLASH_CONFIG_DEF(cmds_quad_24b_mx,   0x01000,  0x8000, 0x10000, 0,   true, init_mx);
const flash_config_t config_quad_32b_mx   = FLASH_CONFIG_DEF(cmds_quad_32b_mx,   0x01000,  0x8000, 0x10000, 0,   true, init_mx);
const flash_config_t config_quad_32b_s    = FLASH_CONFIG_DEF(cmds_quad_32b_s,    0x40000,       0,       0, 0,   true, init_spansion);
const flash_config_t config_quad_24b_issi = FLASH_CONFIG_DEF(cmds_quad_24b_issi, 0x01000,  0x8000, 0x10000, 0,   true, init_issi);

const jedec_config_t jedec_map[] = {
    // MX 24 bit address
    JEDEC_CONFIG_DEF(0xC2, 0x25, 0x34, "MX25U8035F",  &config_quad_24b_mx),   // Stock 1MB
    JEDEC_CONFIG_DEF(0xC2, 0x25, 0x37, "MX25U6432F",  &config_quad_24b_mx),   // 8MB
    JEDEC_CONFIG_DEF(0xC2, 0x25, 0x38, "MX25U1283xF", &config_quad_24b_mx),   // 16MB MX25U12832F, MX25U12835F

    // MX 32 bit address
    JEDEC_CONFIG_DEF(0xC2, 0x25, 0x39, "MX25U25635F", &config_quad_32b_mx),   // 32 MB
    JEDEC_CONFIG_DEF(0xC2, 0x25, 0x3A, "MX25U51245G", &config_quad_32b_mx),   // 64 MB

    // Cypress/Infineon 32 bit address
    // These chips only have 64kB erase size which won't work well with the rest of the code.
    // JEDEC_CONFIG_DEF(0x01, 0x02, 0x20, "S25FS512S",   &config_quad_32b_s), // 64 MB
    // JEDEC_CONFIG_DEF(0x34, 0x2B, 0x1A, "S25FS512S",   &config_quad_32b_s), // 64 MB

    // ISSI 24 bit *untested*
    // TODO: Test and uncomment when it's confirmed they work well.
    // JEDEC_CONFIG_DEF(0x17, 0x60, 0x18, "IS25LP128F",  &config_quad_24b_issi), // 16MB
    // JEDEC_CONFIG_DEF(0x17, 0x70, 0x18, "IS25WP128F",  &config_quad_24b_issi), // 16MB
};

// Driver struct
static struct {
    OSPI_HandleTypeDef   *hospi;
    jedec_id_t            jedec_id;
    const flash_config_t *config;
    const char           *name;
    bool                  mem_mapped_enabled;
} flash = {
    .config = &config_spi_24b, // Default config to use to probe status etc.
    .name = "Unknown",
};

static void set_ospi_cmd(OSPI_RegularCmdTypeDef *ospi_cmd,
                         const flash_cmd_t *cmd,
                         uint32_t address,
                         uint8_t *data,
                         size_t len)
{
    memset(ospi_cmd, 0x0, sizeof(*ospi_cmd));

    ospi_cmd->OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
    ospi_cmd->FlashId = 0;
    ospi_cmd->Instruction = cmd->cmd;
    ospi_cmd->InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
    ospi_cmd->InstructionMode = instruction_line_map[cmd->instr_lines];

    ospi_cmd->AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    ospi_cmd->DummyCycles = cmd->dummy;
    ospi_cmd->DQSMode = HAL_OSPI_DQS_DISABLE;
    ospi_cmd->SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
    ospi_cmd->InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;

    ospi_cmd->Address = address;
    ospi_cmd->AddressSize = address_size_map[cmd->addr_size];
    ospi_cmd->AddressMode = address_line_map[cmd->addr_lines];

    ospi_cmd->NbData = len;
    ospi_cmd->DataMode = data_line_map[cmd->data_lines];
}

static void OSPI_ReadBytes(const flash_cmd_t *cmd,
                           uint32_t address,
                           uint8_t *data,
                           size_t len)
{
    OSPI_RegularCmdTypeDef ospi_cmd;

    // DBG("RB %d 0x%08x 0x%08X %d\n", cmd->cmd, address, data, len);

    assert(flash.mem_mapped_enabled == false);

    set_ospi_cmd(&ospi_cmd,
                 cmd,
                 address,
                 (uint8_t *) data,
                 len);

    wdog_refresh();

    HAL_StatusTypeDef res;
    res = HAL_OSPI_Command(flash.hospi, &ospi_cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
    if (res != HAL_OK) {
        Error_Handler();
    }

    if (HAL_OSPI_Receive(flash.hospi, data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }
}

static void OSPI_WriteBytes(const flash_cmd_t *cmd,
                            uint32_t address,
                            const uint8_t *data,
                            size_t len)
{
    OSPI_RegularCmdTypeDef ospi_cmd;

    // DBG("WB %d 0x%08x 0x%08X %d\n", cmd->cmd, address, data, len);

    assert(flash.mem_mapped_enabled == false);

    set_ospi_cmd(&ospi_cmd,
                 cmd,
                 address,
                 (uint8_t *) data,
                 len);

    wdog_refresh();

    if (HAL_OSPI_Command(flash.hospi, &ospi_cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }

    if (len > 0) {
        if (HAL_OSPI_Transmit(flash.hospi, (uint8_t *) data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            Error_Handler();
        }
    }
}

static void wait_for_status(uint8_t mask, uint8_t value)
{
    uint8_t status;

    do {
        OSPI_ReadBytes(CMD(RDSR), 0, &status, 1);
        wdog_refresh();

#if 0
        printf("Status: %02X\n", status);
        HAL_Delay(500);
#endif
    } while ((status & mask) != value);
}

void OSPI_EnableMemoryMappedMode(void)
{
    OSPI_MemoryMappedTypeDef sMemMappedCfg;
    OSPI_RegularCmdTypeDef ospi_cmd;
    const flash_cmd_t *cmd = CMD(READ);

    assert(flash.mem_mapped_enabled == false);

    set_ospi_cmd(&ospi_cmd, cmd, 0, NULL, 0);

    // Memory-mapped mode configuration for linear burst read operations
    ospi_cmd.OperationType = HAL_OSPI_OPTYPE_READ_CFG;
    if (HAL_OSPI_Command(flash.hospi, &ospi_cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }

    // Use read instruction for write (in order to not alter the flash by accident)
    ospi_cmd.OperationType = HAL_OSPI_OPTYPE_WRITE_CFG;
    if (HAL_OSPI_Command(flash.hospi, &ospi_cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        Error_Handler();
    }

    // Disable timeout counter for memory mapped mode
    sMemMappedCfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_DISABLE;
    sMemMappedCfg.TimeOutPeriod = 0;

    // Enable memory mapped mode
    if (HAL_OSPI_MemoryMapped(flash.hospi, &sMemMappedCfg) != HAL_OK) {
        Error_Handler();
    }

    flash.mem_mapped_enabled = true;
}

void OSPI_DisableMemoryMappedMode(void)
{
    assert(flash.mem_mapped_enabled == true);

    HAL_OSPI_Abort(flash.hospi);

    flash.mem_mapped_enabled = false;

    // This will *ONLY* work if you absolutely don't
    // look at the memory mapped address.
    // See here:
    // https://community.st.com/s/question/0D50X00009XkaJuSAJ/stm32f7-qspi-exit-memory-mapped-mode
    // Even having a debugger open at 0x9000_0000 will break this.
}

static void _OSPI_Erase(const flash_cmd_t *cmd, uint32_t address)
{
    OSPI_WriteBytes(cmd, address, NULL, 0);

    // Wait for Write In Progress Bit to become zero
    wait_for_status(STATUS_WIP_Msk, 0);
}

void OSPI_ChipErase(void)
{
    DBG("CE\n");
    _OSPI_Erase(CMD(CE), 0); // Chip Erase
}

bool OSPI_Erase(uint32_t *address, uint32_t *size)
{
    // Performs one erase command per call with the largest size possible.
    // Sets *address and *size to values that should be passed to
    // OSPI_Erase in the next iteration.
    // Returns true when done.

    assert(address != NULL);
    assert(size != NULL);

    uint32_t req_address = *address;
    uint32_t req_size = *size;

    DBG("E 0x%lx %ld\n", req_address, req_size);

    // Assumes that erase sizes are sorted: 4 > 3 > 2 > 1.
    // Assumes that erase sizes are powers of two.

    const flash_cmd_t * erase_cmd[] = {
        CMD(ERASE1),
        CMD(ERASE2),
        CMD(ERASE3),
        CMD(ERASE4),
    };

    for (int i = 3; i >= 0; i--) {
        uint32_t erase_size = flash.config->erase_sizes[i];

        if (erase_size == 0) {
            continue;
        }

        if ((req_size >= erase_size) && ((req_address & (erase_size - 1)) == 0)) {
            *size = req_size - erase_size;
            *address = req_address + erase_size;

            DBG("Erasing block (%ld): 0x%08lx (%ld left)\n", erase_size, req_address, *size);

            OSPI_NOR_WriteEnable();
            _OSPI_Erase(erase_cmd[i], req_address);

            return (*size == 0);
        }
    }

    DBG("No suitable erase command found for addr=%08lx size=%ld!\n", *address, *size);

    assert(!"Unsupported erase operation!");

    return false;
}

void OSPI_EraseSync(uint32_t address, uint32_t size)
{
    bool ret;

    do {
        ret = OSPI_Erase(&address, &size);
    } while (ret == false);
}

void OSPI_PageProgram(uint32_t address,
                      const uint8_t *buffer,
                      size_t buffer_size)
{
    assert(buffer_size <= 256);

    DBG("PP cmd=%02X addr=0x%lx buf=%p len=%d\n", (*CMD(PP)).cmd, address, buffer, buffer_size);

    OSPI_WriteBytes(CMD(PP), address, buffer, buffer_size);

    // Wait for Write In Progress Bit to become zero
    wait_for_status(STATUS_WIP_Msk, 0);
}

void OSPI_NOR_WriteEnable(void)
{
    OSPI_WriteBytes(CMD(WREN), 0, NULL, 0);

    // Wait for Write Enable Latch to be set
    wait_for_status(STATUS_WEL_Msk, STATUS_WEL_Msk);
}

void OSPI_Program(uint32_t address,
                  const uint8_t *buffer,
                  size_t buffer_size)
{
    unsigned iterations = (buffer_size + 255) / 256;
    unsigned dest_page = address / 256;

    assert((address & 0xff) == 0);

    for (int i = 0; i < iterations; i++) {
        OSPI_NOR_WriteEnable();
        OSPI_PageProgram((i + dest_page) * 256,
                         buffer + (i * 256),
                         buffer_size > 256 ? 256 : buffer_size);
        buffer_size -= 256;
    }
}

void OSPI_ReadJedecId(uint8_t dest[3])
{
    OSPI_ReadBytes(CMD(RDID), 0, dest, 3);
}

void OSPI_ReadSR(uint8_t dest[1])
{
    OSPI_ReadBytes(CMD(RDSR), 0, dest, 1);
}

void OSPI_ReadCR(uint8_t dest[1])
{
    OSPI_ReadBytes(CMD(RDCR), 0, dest, 1);
}

static void init_mx(void)
{
    uint8_t rd_status;

    DBG("%s\n", __FUNCTION__);

    OSPI_ReadBytes(CMD(RDSR), 0, &rd_status, 1);

    if (flash.config->set_quad && ((rd_status & STATUS_QE_Msk) == 0)) {
        // WRSR - Write Status Register
        // Set Quad Enable bit (6) in status register. Other bits = 0.
        uint8_t wr_status = STATUS_QE_Msk;

        DBG("Setting QE bit.\n");

        // Enable write to be allowed to change the status register
        OSPI_NOR_WriteEnable();

        // Set the QE bit
        OSPI_WriteBytes(CMD(WRSR), 0, &wr_status, 1);

        // Wait until WIP bit is cleared
        wait_for_status(STATUS_WIP_Msk, 0);

        OSPI_ReadBytes(CMD(RDSR), 0, &rd_status, 1);
        DBG("QE bit set. Status: %02X\n", rd_status);
    }
}

static void init_issi(void)
{
    // TODO!
}

static void init_spansion(void)
{
    uint8_t rd_sr;
    uint8_t rd_cr1;
    uint8_t rd_cr2;
    uint8_t rd_cr3;
    uint8_t rd_cr4;

    OSPI_ReadBytes(CMD(RDSR), 0x00, &rd_sr, 1);
    OSPI_ReadBytes(CMD(RDCR), 0x00, &rd_cr1, 1);
    OSPI_ReadBytes(CMD(RDAR), 0x03, &rd_cr2, 1);
    OSPI_ReadBytes(CMD(RDAR), 0x04, &rd_cr3, 1);
    OSPI_ReadBytes(CMD(RDAR), 0x05, &rd_cr4, 1);
    DBG("SR: %02X CR: %02X %02X %02X %02X\n", rd_sr, rd_cr1, rd_cr2, rd_cr3, rd_cr4);

    if (flash.config->set_quad && ((rd_cr1 & S_CR_QUAD_Msk) == 0)) {
        // WRSR/WRR writes to {status, config}
        // Clear SR1V and set bit 1 (QUAD) in CR1NV
        uint8_t wr_sr[] = {0x00, S_CR_QUAD_Msk};

        DBG("Setting QUAD in CR1V.\n");

        // Enable write to be allowed to change the registers
        OSPI_NOR_WriteEnable();

        OSPI_WriteBytes(CMD(WRSR), 0, wr_sr, sizeof(wr_sr));

        // Wait until WIP bit is cleared
        wait_for_status(STATUS_WIP_Msk, 0);

        OSPI_ReadBytes(CMD(RDSR), 0, &rd_sr, 1);
        OSPI_ReadBytes(CMD(RDCR), 0, &rd_cr1, 1);
        DBG("QUAD bit set. SR: %02X CR: %02X\n", rd_sr, rd_cr1);
    }
}

const char* OSPI_GetFlashName(void)
{
    return flash.name;
}

uint32_t OSPI_GetSmallestEraseSize(void)
{
    // Assumes that erase sizes are sorted: 4 > 3 > 2 > 1.
    return flash.config->erase_sizes[0];
}

void OSPI_Init(OSPI_HandleTypeDef *hospi)
{
    uint8_t status;

    flash.hospi = hospi;

    // Enable Reset
    OSPI_WriteBytes(CMD(RSTEN), 0, NULL, 0);
    HAL_Delay(2);

    // Reset
    OSPI_WriteBytes(CMD(RST), 0, NULL, 0);
    HAL_Delay(20);

    // Read ID
    OSPI_ReadBytes(CMD(RDID), 0, &flash.jedec_id.u8[0], 3);
    DBG("JEDEC_ID: %02X %02X %02X\n", flash.jedec_id.u8[0], flash.jedec_id.u8[1], flash.jedec_id.u8[2]);

    OSPI_ReadBytes(CMD(RDSR), 0, &status, 1);
    DBG("Status: %02X\n", status);

    for (int i = 0; i < ARRAY_SIZE(jedec_map); i++) {
        if ((flash.jedec_id.u32 & 0xffffff) == (jedec_map[i].jedec_id.u32 & 0xffffff)) {
            flash.config = jedec_map[i].config;
            flash.name = jedec_map[i].name;
            DBG("Found config: %s\n", flash.name);
            break;
        }
    }

    if (flash.config->init_fn) {
        flash.config->init_fn();
    }

    OSPI_EnableMemoryMappedMode();
}
