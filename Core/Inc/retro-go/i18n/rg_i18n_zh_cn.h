/*
***********************************************************
*                Warning!!!!!!!                           *
*  This file must be saved with GBK(or GB2312) Encoding   *
***********************************************************
*/

#pragma once
//Stand 简体中文

//Core\Src\porting\gb\main_gb.c =======================================
#define s_Palette               "调色板"
//=====================================================================

//Core\Src\porting\nes\main_nes.c =====================================
//#define s_Palette "调色板" dul
#define s_Default               "默认"
//=====================================================================

//Core\Src\porting\gw\main_gw.c =======================================
#define s_copy_RTC_to_GW_time   "从系统时间同步"
#define s_copy_GW_time_to_RTC   "同步时间到系统"
#define s_LCD_filter            "屏幕抗锯齿"
#define s_Display_RAM           "显示内存信息"
#define s_Press_ACL             "重置游戏" 
#define s_Press_TIME            "模拟 TIME  键 [B+TIME]"
#define s_Press_ALARM           "模拟 ALARM 键 [B+GAME]"
#define s_filter_0_none         "关"
#define s_filter_1_medium       "中"
#define s_filter_2_high         "高"
//=====================================================================

//Core\Src\porting\odroid_overlay.c ===================================
#define s_Full                  0x07
#define s_Fill                  0x08

#define s_No_Cover              "无封面"

#define s_Yes                   "○ 是"
#define s_No                    "× 否"
#define s_PlsChose              "请选择："
#define s_OK                    "○ 确定"
#define s_Confirm               "信息确认"
#define s_Brightness            "亮度"
#define s_Volume                "音量"
#define s_OptionsTit            "系统设置"
#define s_FPS                   "帧率"
#define s_BUSY                  "负载（CPU）"
#define s_Scaling               "缩放"
#define s_SCalingOff            "关闭"
#define s_SCalingFit            "自适应"
#define s_SCalingFull           "全屏"
#define s_SCalingCustom         "自定义"
#define s_Filtering             "过滤"
#define s_FilteringNone         "无"
#define s_FilteringOff          "关闭"
#define s_FilteringSharp        "锐利"
#define s_FilteringSoft         "柔和"
#define s_Speed                 "速度"
#define s_Speed_Unit            "倍"
#define s_Save_Cont             "■ 保存进度"
#define s_Save_Quit             "▲ 保存退出"
#define s_Reload                "∞ 重新加载"
#define s_Options               "◎ 游戏设置"
#define s_Power_off             "ω 关机休眠"
#define s_Quit_to_menu          "× 退出游戏"
#define s_Retro_Go_options      "游戏选项"

#define s_Colors                "色彩"

#define s_Theme_Title           "界面"
#define s_Theme_sList           "简单列表"
#define s_Theme_CoverV          "垂直滚动"  //vertical
#define s_Theme_CoverH          "水平滚动"  //horizontal
#define s_Theme_CoverLightV     "垂直欢滚"
#define s_Theme_CoverLightH     "水平欢滚"
//=====================================================================

//Core\Src\retro-go\rg_emulators.c ====================================
#define s_Title_Date_Format               "%02d-%02d 周%s %02d:%02d:%02d"
#define fmt_Title_Date_Format(outstr,datefmt,day,month,weekday,hour,minutes,seconds) sprintf(outstr,datefmt,month,day,weekday,hour,minutes,seconds) 

#define s_File                  "名称："
#define s_Type                  "类型："
#define s_Size                  "大小："
#define s_ImgSize               "图像："
#define s_Close                 "× 关闭"
#define s_GameProp              "游戏文件属性"
#define s_Resume_game           "＞ 继续游戏"
#define s_New_game              "◇ 开始游戏"
#define s_Del_favorite          "☆ 移除收藏"
#define s_Add_favorite          "★ 添加收藏"
#define s_Delete_save           "□ 删除进度"
#define s_Confiem_del_save      "您确认要删除已保存的游戏进度？"
//=====================================================================

//Core\Src\retro-go\rg_main.c =========================================
#define s_Second_Unit               "秒"
#define s_Version                   "版    本："
#define s_Author                    "特别贡献："
#define s_Author_                   "        ："
#define s_UI_Mod                    "界面美化："
#define s_Lang                      "简体中文："
#define s_LangAuthor                "挠浆糊的"
#define s_Debug_menu                "≈ 调试信息"
#define s_Reset_settings            "≡ 重置设定"
//#define s_Close                   "Close"
#define s_Retro_Go                  "关于 Retro-Go"
#define s_Confirm_Reset_settings    "您确定要重置所有设定信息？"

#define s_Flash_JEDEC_ID            "存储 JEDEC ID"
#define s_Flash_Name                "存储芯片"
#define s_Flash_SR                  "存储状态"
#define s_Flash_CR                  "存储配置"
#define s_Smallest_erase            "最小单位"
#define s_DBGMCU_IDCODE             "DBGMCU IDCODE"
#define s_Enable_DBGMCU_CK          "启用 DBGMCU CK"
#define s_Disable_DBGMCU_CK         "禁用 DBGMCU CK"
//#define s_Close                   "Close"
#define s_Debug_Title               "调试选项"
#define s_Idle_power_off            "空闲待机"

#define s_Time                      "时间："
#define s_Date                      "日期："
#define s_Time_Title                "时间"
#define s_Hour                      "时："
#define s_Minute                    "分："
#define s_Second                    "秒："
#define s_Time_setup                "时间设置"

#define s_Day                       "日  ："
#define s_Month                     "月  ："
#define s_Year                      "年  ："
#define s_Weekday                   "星期："
#define s_Date_setup                "日期设置"

#define s_Weekday_Mon                "一"
#define s_Weekday_Tue                "二"
#define s_Weekday_Wed                "三"
#define s_Weekday_Thu                "四"
#define s_Weekday_Fri                "五"
#define s_Weekday_Sat                "六"
#define s_Weekday_Sun                "日"

#define s_Date_Format               "20%02d年%02d月%02d日 周%s"
#define fmtDate(x,fmt,d,m,y,w) sprintf(x,fmt,y,m,d,w) 
//=====================================================================

