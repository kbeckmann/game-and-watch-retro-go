/*
***************************************************
*                Warning!!!!!!!                   *
*  This file must be saved with EUC-JP Encoding   *
***************************************************
*/

#pragma once
//日本語
//If you can translate, please feed back the translation results to me, thank you


//Core\Src\porting\gb\main_gb.c
#define s_Palette               "Palette"

//Core\Src\porting\nes\main_nes.c
//#define s_Palette "Palette" dul
#define s_Default               "Default"

//Core\Src\porting\odroid_overlay.c
#define s_Full                  0x07
#define s_Fill                  0x08

#define s_No_Cover              "no Cover"

#define s_Yes                   "はい"
#define s_No                    "なし"
#define s_PlsChose              "Question"
#define s_OK                    "OK"
#define s_Confirm               "Confirm"
#define s_Brightness            "Brightness"
#define s_Volume                "Volume"
#define s_OptionsTit            "Options"
#define s_FPS                   "FPS"
#define s_BUSY                  "BUSY"
#define s_Scaling               "Scaling"
#define s_SCalingFull           "Full"
#define s_Filtering             "Filtering"
#define s_FilteringNone         "None"
#define s_Speed                 "Speed"
#define s_Speed_Unit            "x"
#define s_Save_Cont             "Save & Continue"
#define s_Save_Quit             "Save & Quit"
#define s_Reload                "Reload"
#define s_Options               "Options"
#define s_Power_off             "Power off"
#define s_Quit_to_menu          "Quit to menu"
#define s_Retro_Go_options      "Retro-Go"

#define s_Theme_Title           "UI Theme"
#define s_Theme_sList           "Simple List"
#define s_Theme_CoverV          "Coverflow H"
#define s_Theme_CoverH          "Coverflow V"
#define s_Theme_CoverLightV     "CoverLight V"
#define s_Theme_CoverLightH     "CoverLight H"

//Core\Src\retro-go\rg_emulators.c

#define s_Title_Date_Format               "%02d-%02d %s %02d:%02d:%02d"
#define fmt_Title_Date_Format(outstr,datefmt,day,month,weekday,hour,minutes,seconds) sprintf(outstr,datefmt,day,month,weekday,hour,minutes,seconds) 


#define s_File                  "File"
#define s_Type                  "種類"
#define s_Size                  "Size"
#define s_ImgSize               "ImgSize"
#define s_Close                 "クローズ"
#define s_GameProp              "Properties"
#define s_Resume_game           "Resume game"
#define s_New_game              "New game"
#define s_Del_favorite          "Del favorite"
#define s_Add_favorite          "Add favorite"
#define s_Delete_save           "Delete save"
#define s_Confiem_del_save      "Delete save file?"

//Core\Src\retro-go\rg_main.c
#define s_Second_Unit               "s"
#define s_Version                   "Ver.   :"
#define s_Author                    "By     :"
#define s_Author_                   "       :"
#define s_UI_Mod                    "UI Mod :"
#define s_Lang                      "日本語"
#define s_LangAuthor                "Default"
#define s_Debug_menu                "Debug_menu"
#define s_Reset_settings            "Reset settings"
//#define s_Close                   "Close"
#define s_Retro_Go                  "About Retro-Go"
#define s_Confirm_Reset_settings    "Reset all settings?"

#define s_Flash_JEDEC_ID            "Flash JEDEC ID"
#define s_Flash_Name                "Flash Name"
#define s_Flash_SR                  "Flash SR"
#define s_Flash_CR                  "Flash CR"
#define s_Smallest_erase            "Smallest erase"
#define s_DBGMCU_IDCODE             "DBGMCU IDCODE"
#define s_Enable_DBGMCU_CK          "Enable DBGMCU CK"
#define s_Disable_DBGMCU_CK         "Disable DBGMCU CK"
//#define s_Close                   "Close"
#define s_Debug_Title               "Debug"
#define s_Idle_power_off            "Idle power off"

#define s_Time                      "Time"
#define s_Date                      "Date"
#define s_Time_Title                "TIME"
#define s_Hour                      "Hour"
#define s_Minute                    "Minute"
#define s_Second                    "Second"
#define s_Time_setup                "Time setup"

#define s_Day                       "Day"
#define s_Month                     "Month"
#define s_Year                      "Year"
#define s_Weekday                   "Weekday"
#define s_Date_setup                "Date setup"

#define s_Weekday_Mon                "Mon"
#define s_Weekday_Tue                "Tue"
#define s_Weekday_Wed                "Wed"
#define s_Weekday_Thu                "Thu"
#define s_Weekday_Fri                "Fri"
#define s_Weekday_Sat                "Sat"
#define s_Weekday_Sun                "Sun"

#define s_Date_Format               "%02d.%02d.20%02d %s"
#define fmtDate(outstr,datefmt,day,month,year,weekday) sprintf(outstr,datefmt,day,month,year,weekday) 

