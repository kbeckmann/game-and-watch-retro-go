#include "main.h"
#include "gw_flash.h"
#include <string.h>

static quad_mode_t g_quad_mode = SPI_MODE;

void set_cmd_lines(OSPI_RegularCmdTypeDef *cmd, quad_mode_t quad_mode, uint8_t has_address, uint8_t has_data)
{
  if (quad_mode == SPI_MODE) {
    cmd->InstructionMode     = HAL_OSPI_INSTRUCTION_1_LINE;
    cmd->AddressMode         = has_address ? HAL_OSPI_ADDRESS_1_LINE : HAL_OSPI_ADDRESS_NONE;
    cmd->DataMode            = has_data ? HAL_OSPI_DATA_1_LINE : HAL_OSPI_DATA_NONE;
  } else if(quad_mode == QUAD_MODE) {
    cmd->InstructionMode   = HAL_OSPI_INSTRUCTION_4_LINES;
      cmd->AddressMode       = has_address ? HAL_OSPI_ADDRESS_4_LINES : HAL_OSPI_ADDRESS_NONE;
      cmd->DataMode          = has_data ? HAL_OSPI_DATA_4_LINES : HAL_OSPI_DATA_NONE;
  } else if(quad_mode == HALF_QUAD_MODE) {
    cmd->InstructionMode   = HAL_OSPI_INSTRUCTION_1_LINE;
    cmd->AddressMode       = has_address ? HAL_OSPI_ADDRESS_4_LINES : HAL_OSPI_ADDRESS_NONE;
    cmd->DataMode          = has_data ? HAL_OSPI_DATA_4_LINES : HAL_OSPI_DATA_NONE;
  } else {
    Error_Handler();
  }
}

void OSPI_ReadBytes(OSPI_HandleTypeDef *hospi, uint8_t instruction, uint8_t *data, size_t len)
{
  OSPI_RegularCmdTypeDef  sCommand;
  memset(&sCommand, 0x0, sizeof(sCommand));
  sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId               = 0;
  sCommand.Instruction           = instruction;
  sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  
  sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.NbData                = len;
  sCommand.DummyCycles           = 0;
  sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;
  sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
  sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;

  wdog_refresh();

  if(g_quad_mode == HALF_QUAD_MODE) {
    set_cmd_lines(&sCommand, SPI_MODE, 0, 1);
  } else {
    set_cmd_lines(&sCommand, g_quad_mode, 0, 1);
  }

  if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }

  if(HAL_OSPI_Receive(hospi, data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    Error_Handler();
  }
}

void OSPI_WriteBytes(OSPI_HandleTypeDef *hospi, uint8_t instruction, uint8_t dummy_cycles, uint8_t *data, size_t len, quad_mode_t quad_mode)
{
  OSPI_RegularCmdTypeDef  sCommand;
  memset(&sCommand, 0x0, sizeof(sCommand));
  sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId               = 0;
  sCommand.Instruction           = instruction;
  sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  
  sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.NbData                = len;
  sCommand.DummyCycles           = dummy_cycles;
  sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;
  sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
  sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;

  wdog_refresh();

  set_cmd_lines(&sCommand, quad_mode, 0, len > 0);

  if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }

  if (len > 0) {
    if(HAL_OSPI_Transmit(hospi, data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
      Error_Handler();
    }
  }
}

void OSPI_Init(OSPI_HandleTypeDef *hospi, quad_mode_t quad_mode)
{
  if (quad_mode == QUAD_MODE) {
    // Disable quad mode (will do nothing in SPI mode)
    OSPI_WriteBytes(hospi, 0xf5, 0, NULL, 0, QUAD_MODE);
    HAL_Delay(2);
  }

  // Enable Reset
  OSPI_WriteBytes(hospi, 0x66, 0, NULL, 0, SPI_MODE);
  HAL_Delay(2);

  // Reset
  OSPI_WriteBytes(hospi, 0x99, 0, NULL, 0, SPI_MODE);
  HAL_Delay(20);

  g_quad_mode = quad_mode;

  if (quad_mode == QUAD_MODE) {
      OSPI_WriteBytes(hospi, 0x35, 0, NULL, 0, SPI_MODE);
  } else if(quad_mode == HALF_QUAD_MODE) {
     // WRSR - Write Status Register
      // Set Quad Enable bit (6) in status register. Other bits = 0.
      uint8_t wr_status = 1<<6;
      uint8_t rd_status = 0xff;

      // Enable write to be allowed to change the status register
      OSPI_NOR_WriteEnable(hospi);

      // Loop until rd_status is updated
      while ((rd_status & wr_status) != wr_status) {
        OSPI_WriteBytes(hospi, 0x01, 0, &wr_status, 1, SPI_MODE);
        OSPI_ReadBytes(hospi, 0x05, &rd_status, 1);
      }
  }
}

void OSPI_DisableMemoryMapped(OSPI_HandleTypeDef *hospi)
{
  HAL_OSPI_Abort(hospi);
  // This will *ONLY* work if you absolutely don't
  // look at the memory mapped address. 
  // See here:
  // https://community.st.com/s/question/0D50X00009XkaJuSAJ/stm32f7-qspi-exit-memory-mapped-mode
  // Even having a debugger open at 0x9000_0000 will break this.
}

void OSPI_ChipErase(OSPI_HandleTypeDef *hospi)
{
  uint8_t status;

  // Send Chip Erase command
  OSPI_WriteBytes(hospi, 0x60, 0, NULL, 0, g_quad_mode);

  // Wait for Write In Progress Bit to be zero
  do {
    OSPI_ReadBytes(hospi, 0x05, &status, 1);
    HAL_Delay(100);
  } while((status & 0x01) == 0x01);
}

void _OSPI_Erase(OSPI_HandleTypeDef *hospi, uint8_t instruction, uint32_t address)
{
  uint8_t status;
  OSPI_RegularCmdTypeDef  sCommand;

  memset(&sCommand, 0x0, sizeof(sCommand));
  sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId               = 0;
  sCommand.Instruction           = instruction;
  sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  sCommand.Address               = address;
  sCommand.AddressSize           = HAL_OSPI_ADDRESS_24_BITS;
  sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.NbData                = 0;
  sCommand.DummyCycles           = 0;
  sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;
  sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_ONLY_FIRST_CMD;
  sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;

  if(g_quad_mode == HALF_QUAD_MODE) {
    set_cmd_lines(&sCommand, SPI_MODE, 1, 0);
  } else {
    set_cmd_lines(&sCommand, g_quad_mode, 1, 0);
  }

  if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }

  // Wait for Write In Progress Bit to be zero
  do {
    OSPI_ReadBytes(hospi, 0x05, &status, 1);
  } while((status & 0x01) == 0x01);
}

// Erases a 64kB block
void OSPI_BlockErase64(OSPI_HandleTypeDef *hospi, uint32_t address)
{
  _OSPI_Erase(hospi, 0xD8, address); // Block Erase (64kB)
}

// Erases a 32kB block
void OSPI_BlockErase32(OSPI_HandleTypeDef *hospi, uint32_t address)
{
  _OSPI_Erase(hospi, 0x52, address); // Block Erase (32kB)
}

// Erases a 4kB sector
void OSPI_SectorErase(OSPI_HandleTypeDef *hospi, uint32_t address)
{
  _OSPI_Erase(hospi, 0x20, address); // Sector Erase (4kB)
}

void  _OSPI_Program(OSPI_HandleTypeDef *hospi, uint32_t address, const uint8_t *buffer, size_t buffer_size)
{
  uint8_t status;
  OSPI_RegularCmdTypeDef  sCommand;

  memset(&sCommand, 0x0, sizeof(sCommand));
  sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId               = 0;
  sCommand.Instruction           = 0x02; // PP
  sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  sCommand.Address               = address;
  sCommand.AddressSize           = HAL_OSPI_ADDRESS_24_BITS;
  sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.NbData = buffer_size;
  sCommand.DummyCycles           = 0;
  sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;
  sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_ONLY_FIRST_CMD;
  sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;

  if(g_quad_mode == HALF_QUAD_MODE) {
    sCommand.Instruction         = 0x38; // 4PP
  }

  set_cmd_lines(&sCommand, g_quad_mode, 1, 1);

  if(buffer_size > 256) {
    Error_Handler();
  }

  if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_OSPI_Transmit(hospi, (uint8_t *) buffer, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    Error_Handler();
  }

  // Wait for Write In Progress Bit to be zero
  do {
    OSPI_ReadBytes(hospi, 0x05, &status, 1);
  } while((status & 0x01) == 0x01);
}

void OSPI_Program(OSPI_HandleTypeDef *hospi, uint32_t address, const uint8_t *buffer, size_t buffer_size) {
  unsigned iterations = (buffer_size + 255) / 256;
  unsigned dest_page = address / 256;

  for(int i = 0; i < iterations; i++) {
    OSPI_NOR_WriteEnable(hospi);
    _OSPI_Program(hospi, (i + dest_page) * 256, buffer + (i * 256), buffer_size > 256 ? 256 : buffer_size);
    buffer_size -= 256;
  }
}


void _OSPI_Read(OSPI_HandleTypeDef *hospi, uint32_t address, uint8_t *buffer, size_t buffer_size)
{
  OSPI_RegularCmdTypeDef  sCommand;

  memset(&sCommand, 0x0, sizeof(sCommand));
  sCommand.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId               = 0;
  sCommand.Instruction           = 0x0B; // FAST_READ
  sCommand.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  sCommand.Address               = address;
  sCommand.AddressSize           = HAL_OSPI_ADDRESS_24_BITS;
  sCommand.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.NbData = buffer_size;
  sCommand.DummyCycles           = 8;
  sCommand.DQSMode               = HAL_OSPI_DQS_DISABLE;
  sCommand.SIOOMode              = HAL_OSPI_SIOO_INST_ONLY_FIRST_CMD;
  sCommand.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;

  set_cmd_lines(&sCommand, g_quad_mode, 1, 1);

  if(buffer_size > 256) {
    Error_Handler();
  }

  if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }

  if(HAL_OSPI_Receive(hospi, buffer, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    Error_Handler();
  }
}

void OSPI_Read(OSPI_HandleTypeDef *hospi, uint32_t address, uint8_t *buffer, size_t buffer_size)
{
  unsigned iterations = buffer_size / 256;
  unsigned dest_page = address / 256;

  for(int i = 0; i < iterations; i++) {
    _OSPI_Read(hospi, (i + dest_page) * 256, buffer + (i * 256), buffer_size > 256 ? 256 : buffer_size);
    buffer_size -= 256;
  }
}

void  OSPI_NOR_WriteEnable(OSPI_HandleTypeDef *hospi)
{
  if(g_quad_mode == HALF_QUAD_MODE) {
    OSPI_WriteBytes(hospi, 0x06, 0, NULL, 0, SPI_MODE);
  } else {
    OSPI_WriteBytes(hospi, 0x06, 0, NULL, 0, g_quad_mode);
  }
}


void OSPI_EnableMemoryMappedMode(OSPI_HandleTypeDef *spi) {
  OSPI_MemoryMappedTypeDef sMemMappedCfg;

  OSPI_RegularCmdTypeDef sCommand = {
    .Instruction = 0x0b, // FAST READ
    .SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD,
    .AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE,
    .OperationType = HAL_OSPI_OPTYPE_READ_CFG,
    .FlashId = 0,
    .InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE,
    .InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS,
    .AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE,
    .DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE,
    .DQSMode = HAL_OSPI_DQS_DISABLE,
    .AddressSize = HAL_OSPI_ADDRESS_24_BITS,
    .SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD,
    .DummyCycles = 8,
    .AlternateBytesSize = HAL_OSPI_ALTERNATE_BYTES_8_BITS,
    .AlternateBytes = 0x00,
    .NbData = 0,
    .AlternateBytes = 0x00,
  };

  set_cmd_lines(&sCommand, g_quad_mode, 1, 1);

  if (g_quad_mode != SPI_MODE) {
    sCommand.Instruction = 0xeb;
    sCommand.DummyCycles = 6;
  }

  /* Memory-mapped mode configuration for Linear burst read operations */
  if (HAL_OSPI_Command(spi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    Error_Handler();
  }

  // Use read instruction for write (in order to not alter the flash by accident)
  sCommand.OperationType = HAL_OSPI_OPTYPE_WRITE_CFG;
  if (HAL_OSPI_Command(spi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    Error_Handler();
  }

  /*Disable timeout counter for memory mapped mode*/
  sMemMappedCfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_DISABLE;
  sMemMappedCfg.TimeOutPeriod = 0;
  /*Enable memory mapped mode*/
  if (HAL_OSPI_MemoryMapped(spi, &sMemMappedCfg) != HAL_OK) {
    Error_Handler();
  }
}
