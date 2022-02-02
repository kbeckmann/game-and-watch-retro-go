/*
*********************************************************
*                Warning!!!!!!!                         *
*  This file must be saved with Windows 1252 Encoding   *
*********************************************************
*/
#pragma once
//Stand Portuguese

//Core\Src\porting\gb\main_gb.c =======================================
#define s_Palette               "Paleta"
//=====================================================================

//Core\Src\porting\nes\main_nes.c =====================================
//#define s_Palette "Palette" dul
#define s_Default               "Padrão"
//=====================================================================

//Core\Src\porting\gw\main_gw.c =======================================
#define s_copy_RTC_to_GW_time   "copiar RTC para hora G&W"
#define s_copy_GW_time_to_RTC   "copiar hora G&W para RTC"
#define s_LCD_filter            "Filtro LCD"
#define s_Display_RAM           "Mostrar RAM"
#define s_Press_ACL             "Pressione ACL ou reset"
#define s_Press_TIME            "Pressione TIME [B+TIME]"
#define s_Press_ALARM           "Pressione ALARM [B+GAME]"
#define s_filter_0_none         "0-nenhum"
#define s_filter_1_medium       "1-médio"
#define s_filter_2_high         "2-alto"
//=====================================================================


//Core\Src\porting\odroid_overlay.c ===================================
#define s_Full                  0x07
#define s_Fill                  0x08

#define s_No_Cover              "sem capa"

#define s_Yes                   "Sim"
#define s_No                    "Não"
#define s_PlsChose              "Pergunta"
#define s_OK                    "OK"
#define s_Confirm               "Confirmar"
#define s_Brightness            "Brilho"
#define s_Volume                "Volume"
#define s_OptionsTit            "Opções"
#define s_FPS                   "FPS"
#define s_BUSY                  "OCUPADO"
#define s_Scaling               "Escala"
#define s_SCalingOff            "Desligado"
#define s_SCalingFit            "Ajustar"
#define s_SCalingFull           "Completo"
#define s_SCalingCustom         "Personalizado"
#define s_Filtering             "Filtro"
#define s_FilteringNone         "Nenhum"
#define s_FilteringOff          "Desligado"
#define s_FilteringSharp        "Sharp"
#define s_FilteringSoft         "Suave"
#define s_Speed                 "Velocidade"
#define s_Speed_Unit            "x"
#define s_Save_Cont             "Gravar & Continuar"
#define s_Save_Quit             "Gravar & Sair"
#define s_Reload                "Recarregar"
#define s_Options               "Opções"
#define s_Power_off             "Desligar"
#define s_Quit_to_menu          "Sair para o menu"
#define s_Retro_Go_options      "Retro-Go"

#define s_Colors                "Cores"

#define s_Theme_Title           "Tema UI"
#define s_Theme_sList           "Lista Simplificada"
#define s_Theme_CoverV          "Coverflow V"
#define s_Theme_CoverH          "Coverflow H"
#define s_Theme_CoverLightV     "CoverLight V"
#define s_Theme_CoverLightH     "CoverLight H"
//=====================================================================

//Core\Src\retro-go\rg_emulators.c ====================================
#define s_Title_Date_Format               "%02d-%02d %s %02d:%02d:%02d"
#define fmt_Title_Date_Format(outstr,datefmt,day,month,weekday,hour,minutes,seconds) sprintf(outstr,datefmt,day,month,weekday,hour,minutes,seconds) 

#define s_File                  "Ficheiro"
#define s_Type                  "Tipo"
#define s_Size                  "Tamanho"
#define s_ImgSize               "Tamanho da Imagem"
#define s_Close                 "Fechar"
#define s_GameProp              "Propriedades"
#define s_Resume_game           "Resumir jogo"
#define s_New_game              "Novo jogo"
#define s_Del_favorite          "Apagar favorito"
#define s_Add_favorite          "Adicionar favorito"
#define s_Delete_save           "Apagar save"
#define s_Confiem_del_save      "Apagar ficheiro save?"
//=====================================================================

//Core\Src\retro-go\rg_main.c =========================================
#define s_Second_Unit               "s"
#define s_Version                   "Ver.    :"
#define s_Author                    "Por     :"
#define s_Author_                   "        :"
#define s_UI_Mod                    "UI Mod  :"
#define s_Lang                      "Português"
#define s_LangAuthor                "DefKorns"
#define s_Debug_menu                "Menu Depuração"
#define s_Reset_settings            "Reiniciar configurações"
//#define s_Close                   "Fechar"
#define s_Retro_Go                  "Sobre Retro-Go"
#define s_Confirm_Reset_settings    "Reiniciar todas as configurações?"

#define s_Flash_JEDEC_ID            "Flash JEDEC ID"
#define s_Flash_Name                "Flash Name"
#define s_Flash_SR                  "Flash SR"
#define s_Flash_CR                  "Flash CR"
#define s_Smallest_erase            "Menor apagamento"
#define s_DBGMCU_IDCODE             "DBGMCU IDCODE"
#define s_Enable_DBGMCU_CK          "Ativar DBGMCU CK"
#define s_Disable_DBGMCU_CK         "Desativar DBGMCU CK"
//#define s_Close                   "Fechar"
#define s_Debug_Title               "Depuração"
#define s_Idle_power_off            "Desligamento inativo"
#define s_Splash_Option             "Animação inicial"
#define s_Splash_On                 "LIGADO"
#define s_Splash_Off                "DESLIGADO"

#define s_Time                      "Horas"
#define s_Date                      "Data"
#define s_Time_Title                "HORAS"
#define s_Hour                      "Hora"
#define s_Minute                    "Minuto"
#define s_Second                    "Segundos"
#define s_Time_setup                "Acertar horas"

#define s_Day                       "Dia"
#define s_Month                     "Mês"
#define s_Year                      "Ano"
#define s_Weekday                   "Dia da semana"
#define s_Date_setup                "Acertar Data"

#define s_Weekday_Mon                "Seg"
#define s_Weekday_Tue                "Ter"
#define s_Weekday_Wed                "Qua"
#define s_Weekday_Thu                "Qui"
#define s_Weekday_Fri                "Sex"
#define s_Weekday_Sat                "Sáb"
#define s_Weekday_Sun                "Dom"

#define s_Date_Format               "%02d.%02d.20%02d %s"
#define fmtDate(outstr,datefmt,day,month,year,weekday) sprintf(outstr,datefmt,day,month,year,weekday) 

//=====================================================================
//           ------------ end ---------------