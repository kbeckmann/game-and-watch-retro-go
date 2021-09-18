#pragma once
//Stand ��������

//Core\Src\porting\gb\main_gb.c
#define s_Palette               "��ɫ��"

//Core\Src\porting\nes\main_nes.c
//#define s_Palette "��ɫ��" dul
#define s_Default               "Ĭ��"

//Core\Src\porting\odroid_overlay.c
#define s_Full                  0x07
#define s_Fill                  0x08

#define s_No_Cover              "�޷���"

#define s_Yes                   "�� ��"
#define s_No                    "�� ��"
#define s_PlsChose              "��ѡ��"
#define s_OK                    "�� ȷ��"
#define s_Confirm               "��Ϣȷ��"
#define s_Brightness            "����"
#define s_Volume                "����"
#define s_OptionsTit            "ϵͳ����"
#define s_FPS                   "֡��"
#define s_BUSY                  "���أ�CPU��"
#define s_Scaling               "����"
#define s_SCalingFull           "ȫ��"
#define s_Filtering             "����"
#define s_FilteringNone         "�ر�"
#define s_Speed                 "�ٶ�"
#define s_Speed_Unit            "��"
#define s_Save_Cont             "�� �������"
#define s_Save_Quit             "�� �����˳�"
#define s_Reload                "�� ���¼���"
#define s_Options               "�� ��Ϸ����"
#define s_Power_off             "�� �ػ�����"
#define s_Quit_to_menu          "�� �˳���Ϸ"
#define s_Retro_Go_options      "��Ϸѡ��"

#define s_Theme_Title           "����"
#define s_Theme_sList           "���б�"
#define s_Theme_CoverV          "��ֱ����"
#define s_Theme_CoverH          "ˮƽ����"
#define s_Theme_CoverLight      "CoverLight "
#define s_Theme_CoverLightV     "CoverLightV"

//Core\Src\retro-go\rg_emulators.c

#define s_Title_Date_Format               "%02d-%02d ��%s %02d:%02d:%02d"
#define fmt_Title_Date_Format(outstr,datefmt,day,month,weekday,hour,minutes,seconds) sprintf(outstr,datefmt,month,day,weekday,hour,minutes,seconds) 

#define s_File                  "����: "
#define s_Type                  "����: "
#define s_Size                  "��С: "
#define s_ImgSize               "ͼ��: "
#define s_Close                 "�� �ر�"
#define s_GameProp              "��Ϸ�ļ�����"
#define s_Resume_game           "�� ������Ϸ"
#define s_New_game              "�� ��ʼ��Ϸ"
#define s_Del_favorite          "�� �Ƴ��ղ�"
#define s_Add_favorite          "�� �����ղ�"
#define s_Delete_save           "�� ɾ������"
#define s_Confiem_del_save      "��ȷ��Ҫɾ���ѱ������Ϸ���ȣ�"

//Core\Src\retro-go\rg_main.c
#define s_Second_Unit               "��"
#define s_Version                   "��    ���� "
#define s_Author                    "�ر��ף� "
#define s_Author_                   "        �� "
#define s_UI_Mod                    "���������� "
#define s_Lang                      "��������"
#define s_LangAuthor                "�ӽ�����"
#define s_Debug_menu                "�� ������Ϣ"
#define s_Reset_settings            "�� �����趨"
//#define s_Close                   "Close"
#define s_Retro_Go                  "���� Retro-Go"
#define s_Confirm_Reset_settings    "��ȷ��Ҫ���������趨��Ϣ��"

#define s_Flash_JEDEC_ID            "�洢 JEDEC ID"
#define s_Flash_Name                "�洢оƬ"
#define s_Flash_SR                  "�洢״̬"
#define s_Flash_CR                  "�洢����"
#define s_Smallest_erase            "��С��λ"
#define s_DBGMCU_IDCODE             "DBGMCU IDCODE"
#define s_Enable_DBGMCU_CK          "���� DBGMCU CK"
#define s_Disable_DBGMCU_CK         "���� DBGMCU CK"
//#define s_Close                   "Close"
#define s_Debug_Title               "����ѡ��"
#define s_Idle_power_off            "���д���"

#define s_Time                      "ʱ�䣺"
#define s_Date                      "���ڣ�"
#define s_Time_Title                "ʱ��"
#define s_Hour                      "ʱ��"
#define s_Minute                    "�֣�"
#define s_Second                    "�룺"
#define s_Time_setup                "ʱ������"

#define s_Day                       "��  ��"
#define s_Month                     "��  ��"
#define s_Year                      "��  ��"
#define s_Weekday                   "���ڣ�"
#define s_Date_setup                "��������"

#define s_Weekday_Mon                "һ"
#define s_Weekday_Tue                "��"
#define s_Weekday_Wed                "��"
#define s_Weekday_Thu                "��"
#define s_Weekday_Fri                "��"
#define s_Weekday_Sat                "��"
#define s_Weekday_Sun                "��"

#define s_Date_Format               "20%02d��%02d��%02d�� ��%s"
#define fmtDate(x,fmt,d,m,y,w) sprintf(x,fmt,y,m,d,w) 

