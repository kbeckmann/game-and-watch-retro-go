/*
***********************************************************
*                Warning!!!!!!!                           *
*  This file must be saved with GBK(or GB2312) Encoding   *
***********************************************************
*/
#if !defined (INCLUDED_ZH_CN)
#define INCLUDED_ZH_CN 0
#endif
#if INCLUDED_ZH_CN==1

// Stand 简体中文

int zh_cn_fmt_Title_Date_Format(char *outstr, const char *datefmt, uint16_t day, uint16_t month, const char *weekday, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, datefmt, month, day, weekday, hour, minutes, seconds);
};

int zh_cn_fmt_Date(char *outstr, const char *datefmt, uint16_t day, uint16_t month, uint16_t year, const char *weekday)
{
    return sprintf(outstr, datefmt, year, month, day, weekday);
};

int zh_cn_fmt_Time(char *outstr, const char *timefmt, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, timefmt, hour, minutes, seconds);
};

const lang_t lang_zh_cn LANG_DATA = {
    .codepage = 936,
    .extra_font = cjk_zh_cn,
    .s_LangUI = "界面语言",
    .s_LangTitle = "游戏标题",
    .s_LangName = "S_Chinese",
    
    // Core\Src\porting\gb\main_gb.c =======================================
    .s_Palette = "调色板",
    //=====================================================================

    // Core\Src\porting\nes\main_nes.c =====================================
    //.s_Palette= "调色板" dul
    .s_Default = "默认",
    //=====================================================================

    // Core\Src\porting\gw\main_gw.c =======================================
    .s_copy_RTC_to_GW_time = "从系统时间同步",
    .s_copy_GW_time_to_RTC = "同步时间到系统",
    .s_LCD_filter = "屏幕抗锯齿",
    .s_Display_RAM = "显示内存信息",
    .s_Press_ACL = "重置游戏",
    .s_Press_TIME = "模拟 TIME  键 [B+TIME]",
    .s_Press_ALARM = "模拟 ALARM 键 [B+GAME]",
    .s_filter_0_none = "关",
    .s_filter_1_medium = "中",
    .s_filter_2_high = "高",
    //=====================================================================

    // Core\Src\porting\odroid_overlay.c ===================================
    .s_Full = "\x7",
    .s_Fill = "\x8",

    .s_No_Cover = "无封面",

    .s_Yes = "○ 是",
    .s_No = "× 否",
    .s_PlsChose = "请选择：",
    .s_OK = "○ 确定",
    .s_Confirm = "信息确认",
    .s_Brightness = "亮度",
    .s_Volume = "音量",
    .s_OptionsTit = "系统设置",
    .s_FPS = "帧率",
    .s_BUSY = "负载（CPU）",
    .s_Scaling = "缩放",
    .s_SCalingOff = "关闭",
    .s_SCalingFit = "自适应",
    .s_SCalingFull = "全屏",
    .s_SCalingCustom = "自定义",
    .s_Filtering = "过滤",
    .s_FilteringNone = "无",
    .s_FilteringOff = "关闭",
    .s_FilteringSharp = "锐利",
    .s_FilteringSoft = "柔和",
    .s_Speed = "速度",
    .s_Speed_Unit = "倍",
    .s_Save_Cont = "■ 保存进度",
    .s_Save_Quit = "▲ 保存退出",
    .s_Reload = "∞ 重新加载",
    .s_Options = "◎ 游戏设置",
    .s_Power_off = "ω 关机休眠",
    .s_Quit_to_menu = "× 退出游戏",
    .s_Retro_Go_options = "游戏选项",

    .s_Font = "字体样式",
    .s_Colors = "色彩方案",

    .s_Theme_Title = "界面样式",
    .s_Theme_sList = "简单列表",
    .s_Theme_CoverV = "垂直滚动", // vertical
    .s_Theme_CoverH = "水平滚动", // horizontal
    .s_Theme_CoverLightV = "垂直欢滚",
    .s_Theme_CoverLightH = "水平欢滚",
    //=====================================================================

    // Core\Src\retro-go\rg_emulators.c ====================================

    .s_File = "名称：",
    .s_Type = "类型：",
    .s_Size = "大小：",
    .s_ImgSize = "图像：",
    .s_Close = "× 关闭",
    .s_GameProp = "游戏文件属性",
    .s_Resume_game = "＞ 继续游戏",
    .s_New_game = "◇ 开始游戏",
    .s_Del_favorite = "☆ 移除收藏",
    .s_Add_favorite = "★ 添加收藏",
    .s_Delete_save = "□ 删除进度",
    .s_Confiem_del_save = "您确认要删除已保存的游戏进度？",
    //=====================================================================

    // Core\Src\retro-go\rg_main.c =========================================
    .s_Second_Unit = "秒",
    .s_Version = "版　　本：",
    .s_Author = "特别贡献：",
    .s_Author_ = "　　　　：",
    .s_UI_Mod = "界面美化：",
    .s_Lang = "简体中文：",
    .s_LangAuthor = "挠浆糊的",
    .s_Debug_menu = "≈ 调试信息",
    .s_Reset_settings = "≡ 重置设定",
    //.s_Close                  = "Close",
    .s_Retro_Go = "关于 Retro-Go",
    .s_Confirm_Reset_settings = "您确定要重置所有设定信息？",

    .s_Flash_JEDEC_ID = "存储 JEDEC ID",
    .s_Flash_Name = "存储芯片",
    .s_Flash_SR = "存储状态",
    .s_Flash_CR = "存储配置",
    .s_Smallest_erase = "最小单位",
    .s_DBGMCU_IDCODE = "DBGMCU IDCODE",
    .s_Enable_DBGMCU_CK = "启用 DBGMCU CK",
    .s_Disable_DBGMCU_CK = "禁用 DBGMCU CK",
    //.s_Close                  = "Close",
    .s_Debug_Title = "调试选项",
    .s_Idle_power_off = "空闲待机",
    .s_Splash_Option = "启动动画",
    .s_Splash_On = "开",
    .s_Splash_Off = "关",

    .s_Time = "时间：",
    .s_Date = "日期：",
    .s_Time_Title = "时间",
    .s_Hour = "时：",
    .s_Minute = "分：",
    .s_Second = "秒：",
    .s_Time_setup = "时间设置",

    .s_Day = "日  ：",
    .s_Month = "月  ：",
    .s_Year = "年  ：",
    .s_Weekday = "星期：",
    .s_Date_setup = "日期设置",

    .s_Weekday_Mon = "一",
    .s_Weekday_Tue = "二",
    .s_Weekday_Wed = "三",
    .s_Weekday_Thu = "四",
    .s_Weekday_Fri = "五",
    .s_Weekday_Sat = "六",
    .s_Weekday_Sun = "日",

    .s_Date_Format = "20%02d年%02d月%02d日 周%s",
    .s_Title_Date_Format = "%02d-%02d 周%s %02d:%02d:%02d",
    .s_Time_Format = "%02d:%02d:%02d",

    .fmt_Title_Date_Format = zh_cn_fmt_Title_Date_Format,
    .fmtDate = zh_cn_fmt_Date,
    .fmtTime = zh_cn_fmt_Time,
    //=====================================================================
};

#endif