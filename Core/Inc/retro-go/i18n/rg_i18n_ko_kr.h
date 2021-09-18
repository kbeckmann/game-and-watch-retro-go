#pragma once
//�ѱ���
//If you can translate, please feed back the translation results to me, thank you
//translate by  Augen(����������): 

//Core\Src\porting\gb\main_gb.c
#define s_Palette               "�ȷ�Ʈ"

//Core\Src\porting\nes\main_nes.c
//#define s_Palette "Palette" dul
#define s_Default               "�⺻"

//Core\Src\porting\odroid_overlay.c
#define s_Full                  0x07
#define s_Fill                  0x08

#define s_No_Cover              "Ŀ�� ����"

#define s_Yes                   "��"
#define s_No                    "�ƴϿ�"
#define s_PlsChose              "������ �ּ��� : "
#define s_OK                    "Ȯ��"
#define s_Confirm               "����"
#define s_Brightness            "���"
#define s_Volume                "�Ҹ�ũ��"
#define s_OptionsTit            "ȯ�� ����"
#define s_FPS                   "FPS"
#define s_BUSY                  "Ŭ��(CPU) "
#define s_Scaling               "������"
#define s_SCalingFull           "��üȭ��"
#define s_Filtering             "���͸�"
#define s_FilteringNone         "���͸� ����"
#define s_Speed                 "�ӵ�(���)"
#define s_Speed_Unit            "x"
#define s_Save_Cont             "���� �� ��� �ϱ�"
#define s_Save_Quit             "���� �� ���� �ϱ�"
#define s_Reload                "�ٽ� �ҷ�����"
#define s_Options               "����"
#define s_Power_off             "���� ����"
#define s_Quit_to_menu          "�Ŵ��� ������"
#define s_Retro_Go_options      "Retro-Go"

#define s_Theme_Title           "UI �¸�"
#define s_Theme_sList           "���� ����Ʈ"
#define s_Theme_CoverV          "Ŀ���÷ο� ����"
#define s_Theme_CoverH          "Ŀ���÷ο� ����"
#define s_Theme_CoverLight      "Ŀ���÷ο� ����"
#define s_Theme_CoverLightV     "Ŀ���÷ο� ����"

//Core\Src\retro-go\rg_emulators.c

#define s_Title_Date_Format               "%02d-%02d %s %02d:%02d:%02d"
#define fmt_Title_Date_Format(outstr,datefmt,day,month,weekday,hour,minutes,seconds) sprintf(outstr,datefmt,day,month,weekday,hour,minutes,seconds) 


#define s_File                  "����"
#define s_Type                  "����"
#define s_Size                  "ũ��"
#define s_ImgSize               "�̹��� ũ��"
#define s_Close                 "�ݱ�"
#define s_GameProp              "�Ӽ�"
#define s_Resume_game           "��� ���� �ϱ�"
#define s_New_game              "���� ���� ���� �ϱ�"
#define s_Del_favorite          "���ã�� ����"
#define s_Add_favorite          "���ã�� �߰�"
#define s_Delete_save           "���嵥���� ����"
#define s_Confiem_del_save      "���� �����͸� �����Ͻðڽ��ϱ�?"

//Core\Src\retro-go\rg_main.c
#define s_Second_Unit               "��"
#define s_Version                   "Ver.   :"
#define s_Author                    "By     :"
#define s_Author_                   "       :"
#define s_UI_Mod                    "�������̽� ��� :"
#define s_Lang                      "�ѱ���"
#define s_LangAuthor                "Augen(����������)"
#define s_Debug_menu                "����� �Ŵ�"
#define s_Reset_settings            "��� ���� �ʱ�ȭ"
//#define s_Close                   "�ݱ�"
#define s_Retro_Go                  "Retro-Go ���� "
#define s_Confirm_Reset_settings    "��� ������ �缳�� �Ͻðڽ��ϱ�?"

#define s_Flash_JEDEC_ID            "�÷��� JEDEC ID"
#define s_Flash_Name                "�÷��� �̸�"
#define s_Flash_SR                  "�÷��� SR"
#define s_Flash_CR                  "�÷��� CR"
#define s_Smallest_erase            "Smallest �����"
#define s_DBGMCU_IDCODE             "DBGMCU IDCODE"
#define s_Enable_DBGMCU_CK          "DBGMCU CK Ȱ��ȭ"
#define s_Disable_DBGMCU_CK         "DBGMCU CK ��Ȱ��ȭ"
//#define s_Close                   "�ݱ�"
#define s_Debug_Title               "�����"
#define s_Idle_power_off            "��� ���� ����"

#define s_Time                      "�ð�"
#define s_Date                      "����"
#define s_Time_Title                "�ð�"
#define s_Hour                      "��"
#define s_Minute                    "��"
#define s_Second                    "��"
#define s_Time_setup                "�ð� ����"

#define s_Day                       "��"
#define s_Month                     "��"
#define s_Year                      "��"
#define s_Weekday                   "��"
#define s_Date_setup                "��¥ ����"

#define s_Weekday_Mon                "��"
#define s_Weekday_Tue                "ȭ"
#define s_Weekday_Wed                "��"
#define s_Weekday_Thu                "��"
#define s_Weekday_Fri                "��"
#define s_Weekday_Sat                "��"
#define s_Weekday_Sun                "��"

#define s_Date_Format               "%02d.%02d.20%02d %s"
#define fmtDate(outstr,datefmt,day,month,year,weekday) sprintf(outstr,datefmt,day,month,year,weekday) 

