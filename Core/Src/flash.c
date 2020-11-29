#include "flash.h"

static quad_mode_t g_quad_mode = SPI_MODE;
static spi_chip_vendor_t g_vendor = VENDOR_MX;

void set_cmd_lines(OSPI_RegularCmdTypeDef *cmd, quad_mode_t quad_mode, spi_chip_vendor_t vendor, uint8_t has_address, uint8_t has_data)
{
  if (quad_mode == SPI_MODE) {
    cmd->InstructionMode     = HAL_OSPI_INSTRUCTION_1_LINE;
    cmd->AddressMode         = has_address ? HAL_OSPI_ADDRESS_1_LINE : HAL_OSPI_ADDRESS_NONE;
    cmd->DataMode            = has_data ? HAL_OSPI_DATA_1_LINE : HAL_OSPI_DATA_NONE;
  } else {
    // QUAD_MODE
    if (vendor == VENDOR_MX) {
      cmd->InstructionMode   = HAL_OSPI_INSTRUCTION_1_LINE;
      cmd->AddressMode       = has_address ? HAL_OSPI_ADDRESS_4_LINES : HAL_OSPI_ADDRESS_NONE;
      cmd->DataMode          = has_data ? HAL_OSPI_DATA_4_LINES : HAL_OSPI_DATA_NONE;
    } else {
      // VENDOR_ISSI
      cmd->InstructionMode   = HAL_OSPI_INSTRUCTION_4_LINES;
      cmd->AddressMode       = has_address ? HAL_OSPI_ADDRESS_4_LINES : HAL_OSPI_ADDRESS_NONE;
      cmd->DataMode          = has_data ? HAL_OSPI_DATA_4_LINES : HAL_OSPI_DATA_NONE;
    }
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

  if (g_vendor == VENDOR_ISSI) {
    set_cmd_lines(&sCommand, g_quad_mode, g_vendor, 0, 1);
  } else {
    set_cmd_lines(&sCommand, SPI_MODE, g_vendor, 0, 1);
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

  set_cmd_lines(&sCommand, quad_mode, g_vendor, 0, len > 0);

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

void OSPI_Init(OSPI_HandleTypeDef *hospi, quad_mode_t quad_mode, spi_chip_vendor_t vendor)
{
  if (vendor == VENDOR_ISSI) {
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

  g_vendor = vendor;
  g_quad_mode = quad_mode;

  if (quad_mode == QUAD_MODE && vendor == VENDOR_ISSI) {
    // Enable QPI mode
    OSPI_WriteBytes(hospi, 0x35, 0, NULL, 0, SPI_MODE);
  }
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



void  _OSPI_Program(OSPI_HandleTypeDef *hospi, uint32_t address, uint8_t *buffer, size_t buffer_size)
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

  // For MX vendor in quad mode, use the 4PP command
  if (g_quad_mode == QUAD_MODE && g_vendor == VENDOR_MX) {
    sCommand.Instruction         = 0x38; // 4PP
  }

  set_cmd_lines(&sCommand, g_quad_mode, g_vendor, 1, 1);

  if(buffer_size > 256) {
    Error_Handler();
  }

  if (HAL_OSPI_Command(hospi, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }

  if(HAL_OSPI_Transmit(hospi, buffer, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    Error_Handler();
  }

  // Wait for Write In Progress Bit to be zero
  do {
    OSPI_ReadBytes(hospi, 0x05, &status, 1);
  } while((status & 0x01) == 0x01);
}

void  OSPI_Program(OSPI_HandleTypeDef *hospi, uint32_t address, uint8_t *buffer, size_t buffer_size) {
  unsigned iterations = buffer_size / 256;
  unsigned dest_page = address / 256;

  for(int i = 0; i < iterations; i++) {
    OSPI_NOR_WriteEnable(hospi);
    _OSPI_Program(hospi, (i + dest_page) * 256, buffer + (i * 256), buffer_size > 256 ? 256 : buffer_size);
    buffer_size -= 256;
  }
}

void  OSPI_NOR_WriteEnable(OSPI_HandleTypeDef *hospi)
{
  OSPI_WriteBytes(hospi, 0x06, 0, NULL, 0, g_quad_mode);
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

  set_cmd_lines(&sCommand, g_quad_mode, g_vendor, 1, 1);

  if (g_quad_mode) {
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