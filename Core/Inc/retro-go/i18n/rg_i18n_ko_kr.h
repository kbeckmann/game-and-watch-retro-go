/*
***************************************************
*                Warning!!!!!!!                   *
*  This file must be saved with EUC-KR Encoding   *
***************************************************
*/

#pragma once
//한국어
//If you can translate, please feed back the translation results to me, thank you
//translate by  Augen(히힛마스터): 

//Core\Src\porting\gb\main_gb.c =======================================
#define s_Palette               "팔레트"
//=====================================================================

//Core\Src\porting\nes\main_nes.c =====================================
//#define s_Palette "Palette" dul
#define s_Default               "기본"
//=====================================================================

//Core\Src\porting\gw\main_gw.c =======================================
#define s_copy_RTC_to_GW_time   "copy RTC to G&W time"
#define s_copy_GW_time_to_RTC   "copy G&W time to RTC"
#define s_LCD_filter            "LCD filter"
#define s_Display_RAM           "Display RAM"
#define s_Press_ACL             "Press ACL" 
#define s_filter_0_none         "0-none"
#define s_filter_1_medium       "1-medium"
#define s_filter_2_high         "2-high"
//=====================================================================

//Core\Src\porting\odroid_overlay.c ===================================
#define s_Full                  0x07
#define s_Fill                  0x08

#define s_No_Cover              "커버 없음"

#define s_Yes                   "네"
#define s_No                    "아니오"
#define s_PlsChose              "선택해 주세요 : "
#define s_OK                    "확인"
#define s_Confirm               "적용"
#define s_Brightness            "밝기"
#define s_Volume                "소리크기"
#define s_OptionsTit            "환경 설정"
#define s_FPS                   "FPS"
#define s_BUSY                  "클럭(CPU) "
#define s_Scaling               "스케일"
#define s_SCalingOff            "Off"
#define s_SCalingFit            "Fit"
#define s_SCalingFull           "전체화면"
#define s_SCalingCustom         "Custom"
#define s_Filtering             "필터링"
#define s_FilteringNone         "필터링 없음"
#define s_FilteringOff          "Off"
#define s_FilteringSharp        "Sharp"
#define s_FilteringSoft         "Soft"
#define s_Speed                 "속도(배속)"
#define s_Speed_Unit            "x"
#define s_Save_Cont             "저장 및 계속 하기"
#define s_Save_Quit             "저장 및 종료 하기"
#define s_Reload                "다시 불러오기"
#define s_Options               "설정"
#define s_Power_off             "전원 종료"
#define s_Quit_to_menu          "매뉴로 나가기"
#define s_Retro_Go_options      "Retro-Go"

#define s_Colors                "Colors"

#define s_Theme_Title           "UI 태마"
#define s_Theme_sList           "심플 리스트"
#define s_Theme_CoverV          "커버플로우 세로"
#define s_Theme_CoverH          "커버플로우 가로"
#define s_Theme_CoverLightV     "커버플로우 V"
#define s_Theme_CoverLightH     "커버플로우"

//=====================================================================

//Core\Src\retro-go\rg_emulators.c ====================================

#define s_Title_Date_Format               "%02d-%02d %s %02d:%02d:%02d"
#define fmt_Title_Date_Format(outstr,datefmt,day,month,weekday,hour,minutes,seconds) sprintf(outstr,datefmt,day,month,weekday,hour,minutes,seconds) 


#define s_File                  "파일"
#define s_Type                  "형식"
#define s_Size                  "크기"
#define s_ImgSize               "이미지 크기"
#define s_Close                 "닫기"
#define s_GameProp              "속성"
#define s_Resume_game           "계속 게임 하기"
#define s_New_game              "새로 게임 시작 하기"
#define s_Del_favorite          "즐겨찾기 삭제"
#define s_Add_favorite          "즐겨찾기 추가"
#define s_Delete_save           "저장데이터 삭제"
#define s_Confiem_del_save      "저장 데이터를 삭제하시겠습니까?"
//=====================================================================

//Core\Src\retro-go\rg_main.c =========================================
#define s_Second_Unit               "초"
#define s_Version                   "Ver.   :"
#define s_Author                    "By     :"
#define s_Author_                   "       :"
#define s_UI_Mod                    "인터페이스 모드 :"
#define s_Lang                      "한국어"
#define s_LangAuthor                "Augen(히힛마스터)"
#define s_Debug_menu                "디버그 매뉴"
#define s_Reset_settings            "모든 설정 초기화"
//#define s_Close                   "닫기"
#define s_Retro_Go                  "Retro-Go 정보 "
#define s_Confirm_Reset_settings    "모든 설정을 재설정 하시겠습니까?"

#define s_Flash_JEDEC_ID            "플래시 JEDEC ID"
#define s_Flash_Name                "플래시 이름"
#define s_Flash_SR                  "플래시 SR"
#define s_Flash_CR                  "플래시 CR"
#define s_Smallest_erase            "Smallest 지우기"
#define s_DBGMCU_IDCODE             "DBGMCU IDCODE"
#define s_Enable_DBGMCU_CK          "DBGMCU CK 활성화"
#define s_Disable_DBGMCU_CK         "DBGMCU CK 비활성화"
//#define s_Close                   "닫기"
#define s_Debug_Title               "디버그"
#define s_Idle_power_off            "모든 전원 종료"

#define s_Time                      "시간"
#define s_Date                      "날싸"
#define s_Time_Title                "시간"
#define s_Hour                      "시"
#define s_Minute                    "분"
#define s_Second                    "초"
#define s_Time_setup                "시간 설정"

#define s_Day                       "일"
#define s_Month                     "월"
#define s_Year                      "년"
#define s_Weekday                   "주"
#define s_Date_setup                "날짜 설정"

#define s_Weekday_Mon                "월"
#define s_Weekday_Tue                "화"
#define s_Weekday_Wed                "수"
#define s_Weekday_Thu                "목"
#define s_Weekday_Fri                "금"
#define s_Weekday_Sat                "토"
#define s_Weekday_Sun                "일"

#define s_Date_Format               "%02d.%02d.20%02d %s"
#define fmtDate(outstr,datefmt,day,month,year,weekday) sprintf(outstr,datefmt,day,month,year,weekday) 
//=====================================================================

