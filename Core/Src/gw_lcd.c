#include <string.h>

#include "gw_lcd.h"
#include "stm32h7xx_hal.h"
#include "main.h"

#if GW_LCD_MODE_LUT8
uint8_t framebuffer1[GW_LCD_WIDTH * GW_LCD_HEIGHT];
uint8_t framebuffer2[GW_LCD_WIDTH * GW_LCD_HEIGHT];
#else
uint16_t framebuffer1[GW_LCD_WIDTH * GW_LCD_HEIGHT];
uint16_t framebuffer2[GW_LCD_WIDTH * GW_LCD_HEIGHT];
#endif // GW_LCD_MODE_LUT8

uint16_t *fb1 = framebuffer1;
uint16_t *fb2 = framebuffer2;

uint8_t emulator_framebuffer[(256 + 8 + 8) * 240];

extern LTDC_HandleTypeDef hltdc;

extern DAC_HandleTypeDef hdac1;
extern DAC_HandleTypeDef hdac2;

uint32_t active_framebuffer;
uint32_t frame_counter;

void lcd_backlight_off()
{
  HAL_DAC_Stop(&hdac1, DAC_CHANNEL_1);
  HAL_DAC_Stop(&hdac1, DAC_CHANNEL_2);
  HAL_DAC_Stop(&hdac2, DAC_CHANNEL_1);
}

void lcd_backlight_set(uint8_t brightness)
{
  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_8B_R, brightness);
  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_8B_R, brightness);
  HAL_DAC_SetValue(&hdac2, DAC_CHANNEL_1, DAC_ALIGN_8B_R, brightness);

  HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
  HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);
  HAL_DAC_Start(&hdac2, DAC_CHANNEL_1);
}

void lcd_backlight_on()
{
  lcd_backlight_set(255);
}

static void gw_set_power_1V8(uint32_t p) {
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, p == 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
static void gw_set_power_3V3(uint32_t p) {
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, p == 1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
static void gw_lcd_set_chipselect(uint32_t p) {
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, p == 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
static void gw_lcd_set_reset(uint32_t p) {
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, p == 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void gw_lcd_spi_tx(SPI_HandleTypeDef *spi, uint8_t *pData) {
  gw_lcd_set_chipselect(1);
  HAL_Delay(2);
  HAL_SPI_Transmit(spi, pData, 2, 100);
  HAL_Delay(2);
  wdog_refresh();
  gw_lcd_set_chipselect(0);
  HAL_Delay(2);
}

void lcd_deinit(SPI_HandleTypeDef *spi) {
  // Power off
  gw_set_power_1V8(0);
  gw_set_power_3V3(0);
}

void lcd_init(SPI_HandleTypeDef *spi, LTDC_HandleTypeDef *ltdc) {
  // Disable LCD Chip select
  gw_lcd_set_chipselect(0);

  // LCD reset
  gw_lcd_set_reset(1);

  // Wake up !
  // Enable 1.8V &3V3 power supply
  gw_set_power_3V3(1);
  gw_set_power_1V8(1);
  HAL_Delay(20);
  wdog_refresh();

  // Lets go, bootup sequence.
  /* reset sequence */
  gw_lcd_set_reset(0);
  HAL_Delay(1);
  gw_lcd_set_reset(1);
  HAL_Delay(15);
  gw_lcd_set_reset(0);
  HAL_Delay(1);
  wdog_refresh();

  gw_lcd_spi_tx(spi, (uint8_t *)"\x08\x80");
  gw_lcd_spi_tx(spi, (uint8_t *)"\x6E\x80");
  gw_lcd_spi_tx(spi, (uint8_t *)"\x80\x80");

  // change x00 one of those lines to flip the screen :)
  gw_lcd_spi_tx(spi, (uint8_t *)"\x68\x00");
  gw_lcd_spi_tx(spi, (uint8_t *)"\xd0\x00");
  gw_lcd_spi_tx(spi, (uint8_t *)"\x1b\x00");
  gw_lcd_spi_tx(spi, (uint8_t *)"\xe0\x00");

  gw_lcd_spi_tx(spi, (uint8_t *)"\x6a\x80");
  gw_lcd_spi_tx(spi, (uint8_t *)"\x80\x00");
  gw_lcd_spi_tx(spi, (uint8_t *)"\x14\x80");

  wdog_refresh();

  HAL_LTDC_SetAddress(ltdc,(uint32_t) &fb1, 0);

  memset(fb1, 0, sizeof(framebuffer1));
  memset(fb2, 0, sizeof(framebuffer1));
}

void HAL_LTDC_ReloadEventCallback (LTDC_HandleTypeDef *hltdc) {
  frame_counter++;
  if (active_framebuffer == 0) {
    HAL_LTDC_SetAddress(hltdc, (uint32_t) fb2, 0);
  } else {
    HAL_LTDC_SetAddress(hltdc, (uint32_t) fb1, 0);
  }
}

uint32_t is_lcd_swap_pending(void)
{
  return (uint32_t) ((hltdc.Instance->SRCR) & (LTDC_SRCR_VBR | LTDC_SRCR_IMR));
}

void lcd_swap(void)
{
  HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
  active_framebuffer = active_framebuffer ? 0 : 1;
}

void lcd_sync(void)
{
  void *active = lcd_get_active_buffer();
  void *inactive = lcd_get_inactive_buffer();

  if (active != inactive) {
    memcpy(inactive, active, sizeof(framebuffer1));
  }
}

void* lcd_get_active_buffer(void)
{
  return active_framebuffer ? fb2 : fb1;
}

void* lcd_get_inactive_buffer(void)
{
  return active_framebuffer ? fb1 : fb2;
}

void lcd_reset_active_buffer(void)
{
  HAL_LTDC_SetAddress(&hltdc, (uint32_t) fb1, 0);
  active_framebuffer = 0;
}

void lcd_set_buffers(uint16_t *buf1, uint16_t *buf2)
{
  fb1 = buf1;
  fb2 = buf2;
}

void lcd_wait_for_vblank(void)
{
  uint32_t old_counter = frame_counter;
  while (old_counter == frame_counter) {
    __asm("nop");
  }
}

