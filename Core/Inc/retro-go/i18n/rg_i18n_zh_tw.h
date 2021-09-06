#pragma once
//Stand 繁体中文

//Core\Src\porting\gb\main_gb.c
#define s_Palette               "調色板"

//Core\Src\porting\nes\main_nes.c
//#define s_Palette "調色板" dul
#define s_Default               "默認"

//Core\Src\porting\odroid_overlay.c
#define s_Full                  0x07
#define s_Fill                  0x08

#define s_Yes                   "○ 是"
#define s_No                    "× 否"
#define s_PlsChose              "請選擇："
#define s_OK                    "○ 確定"
#define s_Confirm               "資訊確認"
#define s_Brightness            "亮度"
#define s_Volume                "音量"
#define s_OptionsTit            "系統設置"
#define s_FPS                   "幀率"
#define s_BUSY                  "負載（CPU）"
#define s_Scaling               "縮放"
#define s_SCalingFull           "全屏"
#define s_Filtering             "過濾"
#define s_FilteringNone         "關閉"
#define s_Speed                 "速度"
#define s_Speed_Unit            "倍"
#define s_Save_Cont             "■ 保存進度"
#define s_Save_Quit             "▲ 保存退出"
#define s_Reload                "∞ 重新載入"
#define s_Options               "◎ 遊戲設置"
#define s_Power_off             "ω 關機休眠"
#define s_Quit_to_menu          "× 退出遊戲"
#define s_Retro_Go_options      "遊戲選項"

#define s_Theme_Title           "介面"
#define s_Theme_sList           "簡單列表"
#define s_Theme_CoverH          "垂直捲動"
#define s_Theme_CoverV          "水準滾動"

//Core\Src\retro-go\rg_emulators.c

#define s_Title_Date_Format               "%02d-%02d 周%s %02d:%02d:%02d"
#define fmt_Title_Date_Format(outstr,datefmt,day,month,weekday,hour,minutes,seconds) sprintf(outstr,datefmt,month,day,weekday,hour,minutes,seconds) 

#define s_File                  "名稱: "
#define s_Type                  "類型: "
#define s_Size                  "大小: "
#define s_ImgSize               "圖像: "
#define s_Close                 "× 關閉"
#define s_GameProp              "遊戲檔案屬性"
#define s_Resume_game           "＞ 繼續遊戲"
#define s_New_game              "◇ 開始遊戲"
#define s_Del_favorite          "☆ 移除收藏"
#define s_Add_favorite          "★ 添加收藏"
#define s_Delete_save           "□ 刪除進度"
#define s_Confiem_del_save      "您確認要刪除已保存的遊戲進度？"

//Core\Src\retro-go\rg_main.c
#define s_Second_Unit               "秒"
#define s_Version                   "版    本： "
#define s_Author                    "特別貢獻： "
#define s_Author_                   "       ： "
#define s_UI_Mod                    "介面美化： "
#define s_Lang                      "繁體中文： "
#define s_LangAuthor                "中秋快樂"
#define s_Debug_menu                "∞ 調試信息"
#define s_Reset_settings            "≡ 重置設定"
//#define s_Close                   "Close"
#define s_Retro_Go                  "關於 Retro-Go"
#define s_Confirm_Reset_settings    "您確定要重置所有設定資訊？"

#define s_Flash_JEDEC_ID            "存儲 JEDEC ID"
#define s_Flash_Name                "存儲晶片"
#define s_Flash_SR                  "存儲狀態"
#define s_Flash_CR                  "存儲配置"
#define s_Smallest_erase            "最小單位"
#define s_DBGMCU_IDCODE             "DBGMCU IDCODE"
#define s_Enable_DBGMCU_CK          "啟用 DBGMCU CK"
#define s_Disable_DBGMCU_CK         "禁用 DBGMCU CK"
//#define s_Close                   "Close"
#define s_Debug_Title               "調試選項"
#define s_Idle_power_off            "空閒待機"

#define s_Time                      "時間："
#define s_Date                      "日期："
#define s_Time_Title                "時間"
#define s_Hour                      "時："
#define s_Minute                    "分："
#define s_Second                    "秒："
#define s_Time_setup                "時間設定"

#define s_Day                       "日  ："
#define s_Month                     "月  ："
#define s_Year                      "年  ："
#define s_Weekday                   "星期："
#define s_Date_setup                "日期設定"

#define s_Weekday_Mon                "一"
#define s_Weekday_Tue                "二"
#define s_Weekday_Wed                "三"
#define s_Weekday_Thu                "四"
#define s_Weekday_Fri                "五"
#define s_Weekday_Sat                "六"
#define s_Weekday_Sun                "日"

#define s_Date_Format               "20%02d年%02d月%02d日 周%s"
#define fmtDate(x,fmt,d,m,y,w) sprintf(x,fmt,y,m,d,w) 
