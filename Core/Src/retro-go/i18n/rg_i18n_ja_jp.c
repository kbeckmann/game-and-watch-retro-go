/*
***************************************************
*                Warning!!!!!!!                   *
*  This file must be saved with EUC-JP Encoding   *
***************************************************
*/
#if !defined (INCLUDED_JA_JP)
#define INCLUDED_JA_JP 0
#endif
#if INCLUDED_JA_JP==1
//#include "rg_i18n_lang.h"
// Jp lang

int ja_jp_fmt_Title_Date_Format(char *outstr, const char *datefmt, uint16_t day, uint16_t month, const char *weekday, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, datefmt, day, month, weekday, hour, minutes, seconds);
};

int ja_jp_fmt_Date(char *outstr, const char *datefmt, uint16_t day, uint16_t month, uint16_t year, const char *weekday)
{
    return sprintf(outstr, datefmt, day, month, year, weekday);
};

int ja_jp_fmt_Time(char *outstr, const char *timefmt, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, timefmt, hour, minutes, seconds);
};

const lang_t lang_ja_jp LANG_DATA = {
    .codepage = 932,
    .extra_font = cjk_ja_jp,
    .s_LangUI = "UI 言語",
    .s_LangTitle = "言語",
    .s_LangName = "Japanese",
    // Core\Src\porting\gb\main_gb.c =======================================
    .s_Palette = "Palette",
    //=====================================================================

    // Core\Src\porting\nes\main_nes.c =====================================
    //.s_Palette = "Palette" dul
    .s_Default = "Default",
    //=====================================================================

    // Core\Src\porting\gw\main_gw.c =======================================
    .s_copy_RTC_to_GW_time = "copy RTC to G&W time",
    .s_copy_GW_time_to_RTC = "copy G&W time to RTC",
    .s_LCD_filter = "LCD filter",
    .s_Display_RAM = "Display RAM",
    .s_Press_ACL = "Press ACL or reset",
    .s_Press_TIME = "Press TIME [B+TIME]",
    .s_Press_ALARM = "Press ALARM [B+GAME]",
    .s_filter_0_none = "0-none",
    .s_filter_1_medium = "1-medium",
    .s_filter_2_high = "2-high",
    //=====================================================================

    // Core\Src\porting\odroid_overlay.c ===================================
    .s_Full = "\x7",
    .s_Fill = "\x8",

    .s_No_Cover = "no Cover",

    .s_Yes = "Yes",
    .s_No = "No",
    .s_PlsChose = "Question",
    .s_OK = "OK",
    .s_Confirm = "Confirm",
    .s_Brightness = "Brightness",
    .s_Volume = "Volume",
    .s_OptionsTit = "Options",
    .s_FPS = "FPS",
    .s_BUSY = "BUSY",
    .s_Scaling = "Scaling",
    .s_SCalingOff = "Off",
    .s_SCalingFit = "Fit",
    .s_SCalingFull = "Full",
    .s_SCalingCustom = "Custom",
    .s_Filtering = "Filtering",
    .s_FilteringNone = "None",
    .s_FilteringOff = "Off",
    .s_FilteringSharp = "Sharp",
    .s_FilteringSoft = "Soft",
    .s_Speed = "Speed",
    .s_Speed_Unit = "x",
    .s_Save_Cont = "Save & Continue",
    .s_Save_Quit = "Save & Quit",
    .s_Reload = "Reload",
    .s_Options = "Options",
    .s_Power_off = "Power off",
    .s_Quit_to_menu = "Quit to menu",
    .s_Retro_Go_options = "Retro-Go",

    .s_Font = "Font",
    .s_Colors = "Colors",
    .s_Theme_Title = "UI Theme",
    .s_Theme_sList = "Simple List",
    .s_Theme_CoverV = "Coverflow V",
    .s_Theme_CoverH = "Coverflow H",
    .s_Theme_CoverLightV = "CoverLight V",
    .s_Theme_CoverLightH = "CoverLight H",
    //=====================================================================

    // Core\Src\retro-go\rg_emulators.c ====================================

    .s_File = "File",
    .s_Type = "Type",
    .s_Size = "Size",
    .s_ImgSize = "ImgSize",
    .s_Close = "Close",
    .s_GameProp = "Properties",
    .s_Resume_game = "Resume game",
    .s_New_game = "New game",
    .s_Del_favorite = "Del favorite",
    .s_Add_favorite = "Add favorite",
    .s_Delete_save = "Delete save",
    .s_Confiem_del_save = "Delete save file?",
    //=====================================================================

    // Core\Src\retro-go\rg_main.c =========================================
    .s_Second_Unit = "s",
    .s_Version = "Ver.",
    .s_Author = "By",
    .s_Author_ = "\t\t+",
    .s_UI_Mod = "UI Mod",
    .s_Lang = "日本語",
    .s_LangAuthor = "Default",
    .s_Debug_menu = "Debug menu",
    .s_Reset_settings = "Reset settings",
    //.s_Close                   = "Close",
    .s_Retro_Go = "About Retro-Go",
    .s_Confirm_Reset_settings = "Reset all settings?",

    .s_Flash_JEDEC_ID = "Flash JEDEC ID",
    .s_Flash_Name = "Flash Name",
    .s_Flash_SR = "Flash SR",
    .s_Flash_CR = "Flash CR",
    .s_Smallest_erase = "Smallest erase",
    .s_DBGMCU_IDCODE = "DBGMCU IDCODE",
    .s_Enable_DBGMCU_CK = "Enable DBGMCU CK",
    .s_Disable_DBGMCU_CK = "Disable DBGMCU CK",
    //.s_Close                   = "Close",
    .s_Debug_Title = "Debug",
    .s_Idle_power_off = "Idle power off",
    .s_Splash_Option = "Splash animation",
    .s_Splash_On = "ON",
    .s_Splash_Off = "OFF",

    .s_Time = "Time",
    .s_Date = "Date",
    .s_Time_Title = "TIME",
    .s_Hour = "Hour",
    .s_Minute = "Minute",
    .s_Second = "Second",
    .s_Time_setup = "Time setup",

    .s_Day = "Day",
    .s_Month = "Month",
    .s_Year = "Year",
    .s_Weekday = "Weekday",
    .s_Date_setup = "Date setup",

    .s_Weekday_Mon = "Mon",
    .s_Weekday_Tue = "Tue",
    .s_Weekday_Wed = "Wed",
    .s_Weekday_Thu = "Thu",
    .s_Weekday_Fri = "Fri",
    .s_Weekday_Sat = "Sat",
    .s_Weekday_Sun = "Sun",

    .s_Title_Date_Format = "%02d-%02d %s %02d:%02d:%02d",
    .s_Date_Format = "%02d.%02d.20%02d %s",
    .s_Time_Format = "%02d:%02d:%02d",

    .fmt_Title_Date_Format = ja_jp_fmt_Title_Date_Format,
    .fmtDate = ja_jp_fmt_Date,
    .fmtTime = ja_jp_fmt_Time,
    //=====================================================================
    //           ------------ end ---------------
};
#endif
