#include <stdio.h>
#include <stdbool.h>

#include <stm32h7xx_hal.h>

#include "bq24072.h"

#include "utils.h"

#define BQ24072_BATTERY_FULL    13000
#define BQ24072_BATTERY_LOWBAT  11000

#define BQ24072_PROFILING   0

typedef enum {
    BQ24072_PIN_CHG,
    BQ24072_PIN_PGOOD,
    BQ24072_PIN_COUNT       // Keep this last
} bq24072_pin_t;

// PE7 - CHG
// PE8 - CE
// PA2 - PGOOD
// PC4 - Battery voltage

static const struct {
    uint32_t        pin;
    GPIO_TypeDef*   bank;
} bq_pins[BQ24072_PIN_COUNT] = {
    [BQ24072_PIN_CHG]   = { .pin = GPIO_PIN_7, .bank = GPIOE},
    [BQ24072_PIN_PGOOD] = { .pin = GPIO_PIN_2, .bank = GPIOA},
};

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1;

#if BQ24072_PROFILING
static volatile uint32_t bq24072_battery_value;
#endif // BQ24072_PROFILING

static struct {
    uint16_t    value;
    bool        charging;
    bool        power_good;
    struct {
        int             percent;
        bq24072_state_t state;
    }           last;
} bq24072_data;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    bq24072_data.value = HAL_ADC_GetValue(hadc);

#if BQ24072_PROFILING == 1
    bq24072_battery_value = bq24072_data.value;
#endif

    HAL_ADC_Stop_IT(hadc);
}

int32_t bq24072_init(void)
{
    // Read initial states
    bq24072_handle_power_good();
    bq24072_handle_charging();
    bq24072_poll();

    // Start timer for voltage poll
    HAL_TIM_Base_Start_IT(&htim1);

    return 0;
}

void bq24072_handle_power_good(void)
{
    bq24072_data.power_good = !(HAL_GPIO_ReadPin(bq_pins[BQ24072_PIN_PGOOD].bank, bq_pins[BQ24072_PIN_PGOOD].pin) == GPIO_PIN_SET);
}

void bq24072_handle_charging(void)
{
    bq24072_data.charging = !(HAL_GPIO_ReadPin(bq_pins[BQ24072_PIN_CHG].bank, bq_pins[BQ24072_PIN_CHG].pin) == GPIO_PIN_SET);
}

bq24072_state_t bq24072_get_state(void)
{
    if (bq24072_data.power_good)
    {
        if (bq24072_data.charging)
        {
            return BQ24072_STATE_CHARGING;
        }
        else
        {
            return BQ24072_STATE_FULL;
        }
    }
    else
    {
        if (!bq24072_data.charging)
        {
            return BQ24072_STATE_DISCHARGING;
        }
        else
        {
            return BQ24072_STATE_MISSING;
        }
    }
}

int bq24072_get_percent(void)
{
    int     span = BQ24072_BATTERY_FULL - BQ24072_BATTERY_LOWBAT;

    if (bq24072_get_state() == BQ24072_STATE_MISSING)
    {
        return 0;
    }

    if (bq24072_data.value - BQ24072_BATTERY_LOWBAT <= 0)
    {
        return 0;
    }
    else if (bq24072_data.value >= BQ24072_BATTERY_FULL)
    {
        return 100;
    }
    else
    {
        return (bq24072_data.value - BQ24072_BATTERY_LOWBAT)*100 / span;
    }
}

int bq24072_get_percent_filtered(void)
{
    int             percent;
    bq24072_state_t state;

    if (bq24072_data.value == 0)
    {
        // No value read from ADC yet
        return 0;
    }

    state = bq24072_get_state();
    percent = bq24072_get_percent();

    if (state != bq24072_data.last.state)
    {
        bq24072_data.last.state = state;
        bq24072_data.last.percent = percent;

        return percent;
    }

    switch (state)
    {
        case BQ24072_STATE_MISSING:
        case BQ24072_STATE_FULL:
            return percent;

        case BQ24072_STATE_CHARGING:
            if (percent > bq24072_data.last.percent)
            {
                bq24072_data.last.percent = percent;
            }

            return bq24072_data.last.percent;

        case BQ24072_STATE_DISCHARGING:
            if (percent < bq24072_data.last.percent)
            {
                bq24072_data.last.percent = percent;
            }

            return bq24072_data.last.percent;
    }

    return percent;
}

void bq24072_poll(void)
{
    HAL_ADC_Start_IT(&hadc1);
}

