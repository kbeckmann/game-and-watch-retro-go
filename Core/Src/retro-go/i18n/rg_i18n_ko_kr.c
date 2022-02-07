/*
***************************************************
*                Warning!!!!!!!                   *
*  This file must be saved with EUC-KR Encoding   *
***************************************************
*/
#if !defined (INCLUDED_KO_KR)
#define INCLUDED_KO_KR 0
#endif
#if INCLUDED_KO_KR==1

int ko_kr_fmt_Title_Date_Format(char *outstr, const char *datefmt, uint16_t day, uint16_t month, const char *weekday, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, datefmt, day, month, weekday, hour, minutes, seconds);
};

int ko_kr_fmt_Date(char *outstr, const char *datefmt, uint16_t day, uint16_t month, uint16_t year, const char *weekday)
{
    return sprintf(outstr, datefmt, day, month, year, weekday);
};

int ko_kr_fmt_Time(char *outstr, const char *timefmt, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, timefmt, hour, minutes, seconds);
};

const lang_t lang_ko_kr LANG_DATA = {
    .codepage = 949,
    .extra_font = cjk_ko_kr,
    .s_LangUI = "UI 언어",
    .s_LangTitle = "언어",
    .s_LangName = "Korean",
    //한국어
    // If you can translate, please feed back the translation results to me, thank you
    // translate by  Augen(히힛마스터):

    // Core\Src\porting\gb\main_gb.c =======================================
    .s_Palette = "팔레트",
    //=====================================================================

    // Core\Src\porting\nes\main_nes.c =====================================
    //.s_Palette= "Palette" dul
    .s_Default = "기본",
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

    .s_No_Cover = "커버 없음",

    .s_Yes = "네",
    .s_No = "아니오",
    .s_PlsChose = "선택해 주세요 := ",
    .s_OK = "확인",
    .s_Confirm = "적용",
    .s_Brightness = "밝기",
    .s_Volume = "소리크기",
    .s_OptionsTit = "환경 설정",
    .s_FPS = "FPS",
    .s_BUSY = "클럭(CPU)= ",
    .s_Scaling = "스케일",
    .s_SCalingOff = "Off",
    .s_SCalingFit = "Fit",
    .s_SCalingFull = "전체화면",
    .s_SCalingCustom = "Custom",
    .s_Filtering = "필터링",
    .s_FilteringNone = "필터링 없음",
    .s_FilteringOff = "Off",
    .s_FilteringSharp = "Sharp",
    .s_FilteringSoft = "Soft",
    .s_Speed = "속도(배속)",
    .s_Speed_Unit = "x",
    .s_Save_Cont = "저장 및 계속 하기",
    .s_Save_Quit = "저장 및 종료 하기",
    .s_Reload = "다시 불러오기",
    .s_Options = "설정",
    .s_Power_off = "전원 종료",
    .s_Quit_to_menu = "매뉴로 나가기",
    .s_Retro_Go_options = "Retro-Go",

    .s_Font = "Font",
    .s_Colors = "Colors",
    .s_Theme_Title = "UI 태마",
    .s_Theme_sList = "심플 리스트",
    .s_Theme_CoverV = "커버플로우 세로",
    .s_Theme_CoverH = "커버플로우 가로",
    .s_Theme_CoverLightV = "커버플로우 V",
    .s_Theme_CoverLightH = "커버플로우",

    //=====================================================================

    // Core\Src\retro-go\rg_emulators.c ====================================
    .s_File = "파일",
    .s_Type = "형식",
    .s_Size = "크기",
    .s_ImgSize = "이미지 크기",
    .s_Close = "닫기",
    .s_GameProp = "속성",
    .s_Resume_game = "계속 게임 하기",
    .s_New_game = "새로 게임 시작 하기",
    .s_Del_favorite = "즐겨찾기 삭제",
    .s_Add_favorite = "즐겨찾기 추가",
    .s_Delete_save = "저장데이터 삭제",
    .s_Confiem_del_save = "저장 데이터를 삭제하시겠습니까?",
    //=====================================================================

    // Core\Src\retro-go\rg_main.c =========================================
    .s_Second_Unit = "초",
    .s_Version = "Ver.",
    .s_Author = "By",
    .s_Author_ = "\t\t+",
    .s_UI_Mod = "인터페이스 모드",
    .s_Lang = "한국어",
    .s_LangAuthor = "Augen(히힛마스터)",
    .s_Debug_menu = "디버그 매뉴",
    .s_Reset_settings = "모든 설정 초기화",
    //.s_Close                  = "닫기",
    .s_Retro_Go = "Retro-Go 정보= ",
    .s_Confirm_Reset_settings = "모든 설정을 재설정 하시겠습니까?",

    .s_Flash_JEDEC_ID = "플래시 JEDEC ID",
    .s_Flash_Name = "플래시 이름",
    .s_Flash_SR = "플래시 SR",
    .s_Flash_CR = "플래시 CR",
    .s_Smallest_erase = "Smallest 지우기",
    .s_DBGMCU_IDCODE = "DBGMCU IDCODE",
    .s_Enable_DBGMCU_CK = "DBGMCU CK 활성화",
    .s_Disable_DBGMCU_CK = "DBGMCU CK 비활성화",
    //.s_Close                  = "닫기",
    .s_Debug_Title = "디버그",
    .s_Idle_power_off = "모든 전원 종료",
    .s_Splash_Option = "Splash Animation",
    .s_Splash_On = "ON",
    .s_Splash_Off = "OFF",

    .s_Time = "시간",
    .s_Date = "날싸",
    .s_Time_Title = "시간",
    .s_Hour = "시",
    .s_Minute = "분",
    .s_Second = "초",
    .s_Time_setup = "시간 설정",

    .s_Day = "일",
    .s_Month = "월",
    .s_Year = "년",
    .s_Weekday = "주",
    .s_Date_setup = "날짜 설정",

    .s_Weekday_Mon = "월",
    .s_Weekday_Tue = "화",
    .s_Weekday_Wed = "수",
    .s_Weekday_Thu = "목",
    .s_Weekday_Fri = "금",
    .s_Weekday_Sat = "토",
    .s_Weekday_Sun = "일",

    .s_Title_Date_Format = "%02d-%02d %s %02d:%02d:%02d",
    .s_Date_Format = "%02d.%02d.20%02d %s",
    .s_Time_Format = "%02d:%02d:%02d",
    .fmt_Title_Date_Format = ko_kr_fmt_Title_Date_Format,
    .fmtDate = ko_kr_fmt_Date,
    .fmtTime = ko_kr_fmt_Time,
    //=====================================================================
};
#endif
