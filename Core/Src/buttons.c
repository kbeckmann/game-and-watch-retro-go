#include "buttons.h"

#include "stm32h7xx_hal.h"
#include "main.h"
// HAL_GPIO_ReadPin
// #define BTN_PAUSE_Pin GPIO_PIN_13
// #define BTN_PAUSE_GPIO_Port GPIOC
// #define BTN_GAME_Pin GPIO_PIN_1
// #define BTN_GAME_GPIO_Port GPIOC
// #define BTN_TIME_Pin GPIO_PIN_4
// #define BTN_TIME_GPIO_Port GPIOC
// #define BTN_A_Pin GPIO_PIN_9
// #define BTN_A_GPIO_Port GPIOD
// #define BTN_Left_Pin GPIO_PIN_11
// #define BTN_Left_GPIO_Port GPIOD
// #define BTN_Down_Pin GPIO_PIN_14
// #define BTN_Down_GPIO_Port GPIOD
// #define BTN_Right_Pin GPIO_PIN_15
// #define BTN_Right_GPIO_Port GPIOD
// #define BTN_Up_Pin GPIO_PIN_0
// #define BTN_Up_GPIO_Port GPIOD
// #define BTN_B_Pin GPIO_PIN_5
// #define BTN_B_GPIO_Port GPIOD
#include <stdbool.h>
#define B_Left (1 << 0)
#define B_Up (1 << 1)
#define B_Right (1 << 2)
#define B_Down (1 << 3)
#define B_A (1 << 4)
#define B_B (1 << 5)
#define B_TIME (1 << 6)
#define B_GAME (1 << 7)
#define B_PAUSE (1 << 8)

uint32_t buttons_get() {
    bool left = HAL_GPIO_ReadPin(BTN_Left_GPIO_Port, BTN_Left_Pin) == GPIO_PIN_RESET;
    bool right = HAL_GPIO_ReadPin(BTN_Right_GPIO_Port, BTN_Right_Pin) == GPIO_PIN_RESET;
    bool up = HAL_GPIO_ReadPin(BTN_Up_GPIO_Port, BTN_Up_Pin) == GPIO_PIN_RESET ;
    bool down = HAL_GPIO_ReadPin(BTN_Down_GPIO_Port, BTN_Down_Pin) == GPIO_PIN_RESET;
    bool a = HAL_GPIO_ReadPin(BTN_A_GPIO_Port, BTN_A_Pin) == GPIO_PIN_RESET;
    bool b = HAL_GPIO_ReadPin(BTN_A_GPIO_Port, BTN_B_Pin) == GPIO_PIN_RESET;
    bool time = HAL_GPIO_ReadPin(BTN_TIME_GPIO_Port, BTN_TIME_Pin) == GPIO_PIN_RESET;
    bool game = HAL_GPIO_ReadPin(BTN_GAME_GPIO_Port, BTN_GAME_Pin) == GPIO_PIN_RESET;
    bool pause = HAL_GPIO_ReadPin(BTN_PAUSE_GPIO_Port, BTN_PAUSE_Pin) == GPIO_PIN_RESET;

    return (
        left | (up << 1) | (right << 2) | (down << 3) | (a << 4) | (b << 5) | (time << 6) | (game << 7) | (pause << 8)
    );


}