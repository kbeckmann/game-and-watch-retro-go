#pragma once
//Stand �c�^����

//Core\Src\porting\gb\main_gb.c
#define s_Palette               "�զ�O"

//Core\Src\porting\nes\main_nes.c
//#define s_Palette "�զ�O" dul
#define s_Default               "�q�{"

//Core\Src\porting\odroid_overlay.c
#define s_Full                  0x07
#define s_Fill                  0x08

#define s_No_Cover              "�L�ʭ�"

#define s_Yes                   "�� �O"
#define s_No                    "�� �_"
#define s_PlsChose              "�п�ܡG"
#define s_OK                    "�� �T�w"
#define s_Confirm               "��T�T�{"
#define s_Brightness            "�G��"
#define s_Volume                "���q"
#define s_OptionsTit            "�t�γ]�m"
#define s_FPS                   "�V�v"
#define s_BUSY                  "�t���]CPU�^"
#define s_Scaling               "�Y��"
#define s_SCalingFull           "����"
#define s_Filtering             "�L�o"
#define s_FilteringNone         "����"
#define s_Speed                 "�t��"
#define s_Speed_Unit            "��"
#define s_Save_Cont             "�� �O�s�i��"
#define s_Save_Quit             "�� �O�s�h�X"
#define s_Reload                "�� ���s���J"
#define s_Options               "�� �C���]�m"
#define s_Power_off             "�s ������v"
#define s_Quit_to_menu          "�� �h�X�C��"
#define s_Retro_Go_options      "�C���ﶵ"

#define s_Theme_Title           "����"
#define s_Theme_sList           "²��C��"
#define s_Theme_CoverV          "��������"
#define s_Theme_CoverH          "���Ǻu��"
#define s_Theme_CoverLight      "CoverLight "
#define s_Theme_CoverLightV     "CoverLightV"

//Core\Src\retro-go\rg_emulators.c

#define s_Title_Date_Format               "%02d-%02d �P%s %02d:%02d:%02d"
#define fmt_Title_Date_Format(outstr,datefmt,day,month,weekday,hour,minutes,seconds) sprintf(outstr,datefmt,month,day,weekday,hour,minutes,seconds) 

#define s_File                  "�W��: "
#define s_Type                  "����: "
#define s_Size                  "�j�p: "
#define s_ImgSize               "�Ϲ�: "
#define s_Close                 "�� ����"
#define s_GameProp              "�C���ɮ��ݩ�"
#define s_Resume_game           "�� �~��C��"
#define s_New_game              "�� �}�l�C��"
#define s_Del_favorite          "�� ��������"
#define s_Add_favorite          "�� �K�[����"
#define s_Delete_save           "�� �R���i��"
#define s_Confiem_del_save      "�z�T�{�n�R���w�O�s���C���i�סH"

//Core\Src\retro-go\rg_main.c
#define s_Second_Unit               "��"
#define s_Version                   "��    ���G "
#define s_Author                    "�S�O�^�m�G "
#define s_Author_                   "        �G "
#define s_UI_Mod                    "�������ơG "
#define s_Lang                      "�c�餤��� "
#define s_LangAuthor                "����ּ�"
#define s_Debug_menu                "�� �ոիH��"
#define s_Reset_settings            "�� ���m�]�w"
//#define s_Close                   "Close"
#define s_Retro_Go                  "���� Retro-Go"
#define s_Confirm_Reset_settings    "�z�T�w�n���m�Ҧ��]�w��T�H"

#define s_Flash_JEDEC_ID            "�s�x JEDEC ID"
#define s_Flash_Name                "�s�x����"
#define s_Flash_SR                  "�s�x���A"
#define s_Flash_CR                  "�s�x�t�m"
#define s_Smallest_erase            "�̤p���"
#define s_DBGMCU_IDCODE             "DBGMCU IDCODE"
#define s_Enable_DBGMCU_CK          "�ҥ� DBGMCU CK"
#define s_Disable_DBGMCU_CK         "�T�� DBGMCU CK"
//#define s_Close                   "Close"
#define s_Debug_Title               "�ոտﶵ"
#define s_Idle_power_off            "�Ŷ��ݾ�"

#define s_Time                      "�ɶ��G"
#define s_Date                      "����G"
#define s_Time_Title                "�ɶ�"
#define s_Hour                      "�ɡG"
#define s_Minute                    "���G"
#define s_Second                    "���G"
#define s_Time_setup                "�ɶ��]�w"

#define s_Day                       "��  �G"
#define s_Month                     "��  �G"
#define s_Year                      "�~  �G"
#define s_Weekday                   "�P���G"
#define s_Date_setup                "����]�w"

#define s_Weekday_Mon                "�@"
#define s_Weekday_Tue                "�G"
#define s_Weekday_Wed                "�T"
#define s_Weekday_Thu                "�|"
#define s_Weekday_Fri                "��"
#define s_Weekday_Sat                "��"
#define s_Weekday_Sun                "��"

#define s_Date_Format               "20%02d�~%02d��%02d�� �P%s"
#define fmtDate(x,fmt,d,m,y,w) sprintf(x,fmt,y,m,d,w) 
