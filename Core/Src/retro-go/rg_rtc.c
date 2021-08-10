#include "main.h"
#include "rg_rtc.h"
#include "stm32h7xx_hal.h"

RTC_TimeTypeDef GW_currentTime = {0};
RTC_DateTypeDef GW_currentDate = {0};

// Getters
uint8_t GW_GetCurrentHour(void) {

    // Get time. According to STM docs, both functions need to be called at once.
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    return GW_currentTime.Hours;

}
uint8_t GW_GetCurrentMinute(void) {

    // Get time. According to STM docs, both functions need to be called at once.
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    return GW_currentTime.Minutes;
}
uint8_t GW_GetCurrentSecond(void) {

    // Get time. According to STM docs, both functions need to be called at once.
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    return GW_currentTime.Seconds;
}

uint8_t GW_GetCurrentMonth(void) {

    // Get time. According to STM docs, both functions need to be called at once.
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    return GW_currentDate.Month;
}
uint8_t GW_GetCurrentDay(void) {

    // Get time. According to STM docs, both functions need to be called at once.
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    return GW_currentDate.Date;
}

uint8_t GW_GetCurrentWeekday(void) {

    // Get time. According to STM docs, both functions need to be called at once.
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    return GW_currentDate.WeekDay;
}
uint8_t GW_GetCurrentYear(void) {

    // Get time. According to STM docs, both functions need to be called at once.
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    return GW_currentDate.Year;
}
/*
// Setters
uint8_t GW_SetCurrentHour(const uint8_t hour) {}
uint8_t GW_SetCurrentMinute(const uint8_t minute) {}

uint8_t GW_SetCurrentMonth(const uint8_t month) {}
uint8_t GW_SetCurrentDay(const uint8_t day) {}

uint8_t GW_SetCurrentWeekday(const uint8_t weekday) {}
uint8_t GW_SetCurrentYear(const uint8_t year) {}

// Callbacks for UI purposes
bool hour_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat) {}
bool minute_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat) {}

bool month_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat) {}
bool day_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat) {}

bool weekday_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat) {}
bool year_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat) {}
*/