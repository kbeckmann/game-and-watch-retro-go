#pragma once

typedef struct
{
    const uint32_t codepage;
    const unsigned char *extra_font; 
    const char *s_LangUI;
    const char *s_LangTitle;
    const char *s_LangName;
    // Core\Src\porting\gb\main_gb.c =======================================
    const char *s_Palette;
    //=====================================================================
    // Core\Src\porting\nes\main_nes.c =====================================
    // const char *s_Palette "Palette" dul
    const char *s_Default;
    //=====================================================================
    // Core\Src\porting\gw\main_gw.c =======================================
    const char *s_copy_RTC_to_GW_time;
    const char *s_copy_GW_time_to_RTC;
    const char *s_LCD_filter;
    const char *s_Display_RAM;
    const char *s_Press_ACL;
    const char *s_Press_TIME;
    const char *s_Press_ALARM;
    const char *s_filter_0_none;
    const char *s_filter_1_medium;
    const char *s_filter_2_high;
    //=====================================================================
    // Core\Src\porting\odroid_overlay.c ===================================
    const char *s_Full;
    const char *s_Fill;
    const char *s_No_Cover;
    const char *s_Yes;
    const char *s_No;
    const char *s_PlsChose;
    const char *s_OK;
    const char *s_Confirm;
    const char *s_Brightness;
    const char *s_Volume;
    const char *s_OptionsTit;
    const char *s_FPS;
    const char *s_BUSY;
    const char *s_Scaling;
    const char *s_SCalingOff;
    const char *s_SCalingFit;
    const char *s_SCalingFull;
    const char *s_SCalingCustom;
    const char *s_Filtering;
    const char *s_FilteringNone;
    const char *s_FilteringOff;
    const char *s_FilteringSharp;
    const char *s_FilteringSoft;
    const char *s_Speed;
    const char *s_Speed_Unit;
    const char *s_Save_Cont;
    const char *s_Save_Quit;
    const char *s_Reload;
    const char *s_Options;
    const char *s_Power_off;
    const char *s_Quit_to_menu;
    const char *s_Retro_Go_options;
    const char *s_Font;
    const char *s_Colors;
    const char *s_Theme_Title;
    const char *s_Theme_sList;
    const char *s_Theme_CoverV;
    const char *s_Theme_CoverH;
    const char *s_Theme_CoverLightV;
    const char *s_Theme_CoverLightH;
    //=====================================================================
    // Core\Src\retro-go\rg_emulators.c ====================================
    const char *s_File;
    const char *s_Type;
    const char *s_Size;
    const char *s_ImgSize;
    const char *s_Close;
    const char *s_GameProp;
    const char *s_Resume_game;
    const char *s_New_game;
    const char *s_Del_favorite;
    const char *s_Add_favorite;
    const char *s_Delete_save;
    const char *s_Confiem_del_save;
    //=====================================================================
    // Core\Src\retro-go\rg_main.c =========================================
    const char *s_Second_Unit;
    const char *s_Version;
    const char *s_Author;
    const char *s_Author_;
    const char *s_UI_Mod;
    const char *s_Lang;
    const char *s_LangAuthor;
    const char *s_Debug_menu;
    const char *s_Reset_settings;
    const char *s_Retro_Go;
    const char *s_Confirm_Reset_settings;
    const char *s_Flash_JEDEC_ID;
    const char *s_Flash_Name;
    const char *s_Flash_SR;
    const char *s_Flash_CR;
    const char *s_Smallest_erase;
    const char *s_DBGMCU_IDCODE;
    const char *s_Enable_DBGMCU_CK;
    const char *s_Disable_DBGMCU_CK;
    const char *s_Debug_Title;
    const char *s_Idle_power_off;
    const char *s_Splash_Option;
    const char *s_Splash_On;
    const char *s_Splash_Off;
    const char *s_Time;
    const char *s_Date;
    const char *s_Time_Title;
    const char *s_Hour;
    const char *s_Minute;
    const char *s_Second;
    const char *s_Time_setup;
    const char *s_Day;
    const char *s_Month;
    const char *s_Year;
    const char *s_Weekday;
    const char *s_Date_setup;
    const char *s_Weekday_Mon;
    const char *s_Weekday_Tue;
    const char *s_Weekday_Wed;
    const char *s_Weekday_Thu;
    const char *s_Weekday_Fri;
    const char *s_Weekday_Sat;
    const char *s_Weekday_Sun;
    const char *s_Title_Date_Format;
    const char *s_Date_Format;
    const char *s_Time_Format;
    const int (*fmt_Title_Date_Format)(char *outstr, const char *datefmt, uint16_t day, uint16_t month, const char *weekday, uint16_t hour, uint16_t minutes, uint16_t seconds);
    // const char *fmt_Title_Date_Format(outstr,datefmt,day,month,weekday,hour,minutes,seconds) sprintf(outstr,datefmt,day,month,weekday,hour,minutes,seconds)
    const int (*fmtDate)(char *outstr, const char *datefmt, uint16_t day, uint16_t month, uint16_t year, const char *weekday);
    // const char *fmtDate(outstr,datefmt,day,month,year,weekday) sprintf(outstr,datefmt,day,month,year,weekday)
    const int (*fmtTime)(char *outstr, const char *timefmt, uint16_t hour, uint16_t minutes, uint16_t seconds);
    //=====================================================================
    //           ------------ end ---------------
} lang_t;
