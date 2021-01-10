#pragma once
#ifndef BQ24072_H
#define BQ24072_H

#include <stdint.h>

typedef enum {
    BQ24072_STATE_MISSING,
    BQ24072_STATE_CHARGING,
    BQ24072_STATE_DISCHARGING,
    BQ24072_STATE_FULL,
} bq24072_state_t;

int32_t bq24072_init(void);

void bq24072_handle_power_good(void);
void bq24072_handle_charging(void);

bq24072_state_t bq24072_get_state(void);
int bq24072_get_percent(void);
int bq24072_get_percent_filtered(void);

void bq24072_poll(void);

#endif // BQ24072_H
