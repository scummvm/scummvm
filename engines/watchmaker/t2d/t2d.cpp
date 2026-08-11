/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_strcat
#define FORBIDDEN_SYMBOL_EXCEPTION_strcpy
#define FORBIDDEN_SYMBOL_EXCEPTION_sprintf

#include "watchmaker/t2d/t2d.h"
#include "watchmaker/globvar.h"
#include "watchmaker/message.h"
#include "watchmaker/schedule.h"
#include "watchmaker/ll/ll_string.h"
#include "watchmaker/define.h"
#include "watchmaker/t2d/t2d_internal.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/ll/ll_diary.h"
#include "watchmaker/extraLS.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/t3d_body.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/ll/ll_mouse.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/classes/do_sound.h"
#include "watchmaker/classes/do_string.h"
#include "watchmaker/main.h"
#include "watchmaker/ll/ll_regen.h"
#include "watchmaker/classes/do_player.h"
#include "watchmaker/ll/ll_sound.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/renderer.h"
#include "common/system.h"

namespace Watchmaker {

#define T2D_SCANNER_BUTTON_DELAY                            FRAME_PER_SECOND/2
#define T2D_SCANNER_SCANNING_STEPS                          24
#define T2D_SCANNER_SCANNING_DX                             9
#define T2D_SCANNER_SCANNING_DELAY                          FRAME_PER_SECOND/7

#define T2D_SCANNER_PREVIEWING                              1
#define T2D_SCANNER_SCANNING                                2
#define T2D_COMPUTER_CONNECTING                             3
#define T2D_PDA_TONE_AQUIRING                               4
#define T2D_PDA_TONE_PROCESS                                5
#define T2D_CURSOR                                          6

#define T2D_COMPUTER_BUTTON_DELAY                           FRAME_PER_SECOND/2
#define T2D_DIALUP_BUTTON_DELAY                             FRAME_PER_SECOND/5
#define T2D_WRITE_DELAY                                     FRAME_PER_SECOND
#define T2D_PDA_BUTTON_DELAY                                FRAME_PER_SECOND/2

#define T2D_KBD_MAX_ENTRIES                                 10

#define T2D_DOC_SCROLL_ADD                                  10

#define T2D_COMPUTER_CONNECTING_SPEED               1200

#define T2D_PDA_LOG_DATA_XI                                 55
#define T2D_PDA_LOG_TEXT_XI                                 150
#define T2D_PDA_LOG_YI                                      46
#define T2D_PDA_LOG_DATA_XF                                 140
#define T2D_PDA_LOG_TEXT_XF                                 690
#define T2D_PDA_LOG_YF                                      345
#define T2D_PDA_LOG_LINES                                   20

#define T2D_PDA_LOG_SB_STARTY                               59
#define T2D_PDA_LOG_SB_TOTY                                 259
#define T2D_PDA_LOG_SB_PIRGY                                14

#define T2D_INDENT_COEFF                                    15

#define REFRESH                                             -12345
#define RESTARTALL                                          12345

#define T2D_PDA_TONE_XI                                     2
#define T2D_PDA_TONE_XF                                     664
#define T2D_PDA_TONE_REDLINE_DX                             2
#define T2D_PDA_TONE_BARRA_DX                               165
#define T2D_PDA_TONE_SPEED                                  3

#define T2D_PDA_HELP_TEXT_XI                                20
#define T2D_PDA_HELP_TEXT_YI                                50
#define T2D_PDA_HELP_TEXT_XF                                320
#define T2D_PDA_HELP_TEXT_YF                                330
#define T2D_PDA_HELP_WIN_LOWER                              330

#define T2D_TEMP_SAVE_FILE_NAME                             "SaveTemp.tga"

#define T2D_OPTIONS_MAX_SAVES                               98

#define T2D_OPTIONS_MAX_DESCRIPT                            16

#define T2D_CURSOR_SPEED                                    300

#define PDA_MENU_OPEN                                       1024

#define STOP_T2D__LAUNCH_LOAD                           1
#define STOP_T2D__LAUNCH_NEWGAME                        2
#define STOP_T2D__LAUNCH_MAINMENU                       3

void delSpaces(Common::String &str) {
	Common::String result;
	for (auto it = str.begin(); it != str.end(); ++it) {
		if (*it != ' ') {
			result += *it;
		}
	}
	str = Common::move(result);
}

t2dWINDOW t2dWin[T2D_MAX_WINDOWS];
int16 WinActive[T2D_MAX_WINDOWS];
int32 CurButton, CurButtonWin;
int32 CurButtonWin_Override = -1;
int32 CurButton_Override = -1;
int32 LinkWinToMouse, LinkWinX, LinkWinY;
int32 LinkBtToMouse, LinkBmToMouse;
int32 ExtraLSId = 0;
struct SRect LinkLim;

//Gestione scrolling con mouse
int32 ScrollWithMouse;
int32 ScrollWithMouseX, ScrollWithMouseY;

//Gestione Tastiera
char t2dKeyTable[T2D_KBD_MAX_ENTRIES];
unsigned int t2dKeyTableH = 0;
unsigned int t2dKeyTableT = 0;

//Variabili Scanner
bool SelectorFlag;

//Variabili Computer
#define COMPUTER_MAX_NUM_DIALUP                                 12
char computerNumeroTel[COMPUTER_MAX_NUM_DIALUP];
const char computerNumeroEsatto[] = "231074";
char computerConnectStr[20] = "";
bool AlphaOrCase;
uint8 WhatText;
#define COMPUTER_SEARCH_MAX_NAME                                13
#define COMPUTER_SEARCH_MAX_SURNAME                             13
#define COMPUTER_SEARCH_MAX_CASENAME                            13
#define COMPUTER_SEARCH_MAX_YEAR                                7
Common::String searchName;
Common::String searchSurName;
Common::String searchCaseName;
Common::String searchYear;

int32 computerCurrentDocument;
int32 computerCurrentEMail;

int32 computerDocList[T2D_TEXT_COMPUTER_DOCLIST_LINES_END - T2D_TEXT_COMPUTER_DOCLIST_LINES_START];

//Variabili Diario
int32 diarioCurrentPage;

//Varabili PDA
int32  StartAcquiring;
//Funzioni e Variabili  PDAHelp
void (*HelpFunc)();
//Funzioni e Variabili PDALog
extern int32   t3dCurTime;
void SetPDALogItem(Init &init, int32 log_item);
int32  CurrentLogPos;
int32  CurrentLogMenu;
struct  SPDALog *CurrentPDALogs[20];
PDALogS PDALogSorted[MAX_PDALOGS];
int     MenuLinks[10];
bool    LogsElaborati = false;
int     NumPDALogSorted = 0;
int     NumMenus = 0;
int     MainTitle = 0;
int     PDAScrollPos = 0;
int     PDAScrollLine = 0;
int     LogTotLines = 0;
int     LogCurAbsLine = 0;
int     LogSBPos = 0;
double  LogSBDy = 0;

//Variabili OPTIONS
int32  optionsCurrent = -1;
int16  optionsCurPos = 0;
int16  optionsSlot = 0;
int16  optionsWhat = 0;
char    optionsSaves[9][MAX_PATH];
bool    optionsSaveDescriptActive = false;
SavesS  Saves[100];
int     NSaves = 0;

//Variabili Cursore
bool    CursorActive = false;
bool    CursorVisible = false;
char    *CursorString;

//Variabili GameOver
#define GAMEOVER_WINDOW_NUM_MSECONDS        2000
uint32 GameOverWindow_StartTime;
uint8 LoadWindow_WhichBkg = 0;  // 0->null      1->MainBkg      2->GameOverBkg


int16 PiroloGrab = 0;
uint8 Settati_PDA_items_durante_t2d = 0;

const char *ComputerCIAFileNames[T2D_BM_COMPUTER_DOCUMENT_LAST - T2D_BM_COMPUTER_DOCUMENT_FIRST + 1] = {
	"t2d\\Computer_Document_AntonioCorona.tga",
	"t2d\\Computer_Document_JacobKrenn.tga",
	"t2d\\Computer_Document_PeterDukes.tga",
	"t2d\\Computer_Document_VictorValencia.tga",
	"t2d\\Computer_Document_Mercury_1969_1.tga",
	"t2d\\Computer_Document_Mercury_1969_2.tga",
	"t2d\\Computer_Document_Mercury_1969_3.tga",
	"t2d\\Computer_Document_Mercury_1969_4.tga",
	"t2d\\Computer_Document_Confidence_1942_1.tga",
	"t2d\\Computer_Document_Confidence_1942_2.tga",
	"t2d\\Computer_Document_Confidence_1942_3.tga",
	"t2d\\Computer_Document_Arl_1998_1.tga",
	"t2d\\Computer_Document_Conv_Confidence.tga",
	"t2d\\Computer_Document_Conv_Mercury.tga",
	"t2d\\Computer_Document_GregorMem.tga",
	"t2d\\noi_benna.tga",
	"t2d\\noi_andrea.tga",
	"t2d\\noi_bellonds.tga",
	"t2d\\noi_daniele.tga",
	"t2d\\noi_edo.tga",
	"t2d\\noi_fede.tga",
	"t2d\\noi_fiabe.tga",
	"t2d\\noi_fox.tga",
	"t2d\\noi_giusti.tga",
	"t2d\\noi_krizia.tga",
	"t2d\\noi_nonji.tga",
	"t2d\\noi_sparteco.tga",
	"t2d\\noi_tag.tga",
	"t2d\\noi_tiz.tga",
	"t2d\\noi_walter.tga",
	"t2d\\noi_vicky.tga",
	"t2d\\noi_pietro.tga"
};

void RefreshUpdate(Init &init) {
	int i, j, NearMenu;

	//Aggiornamento degli update
	for (i = 0; i < MAX_PDALOGS; i++)
		if (init.PDALog[i].flags & PDA_MENU) init.PDALog[i].flags &= ~PDA_UPDATE;

	for (i = 0; i < MAX_PDALOGS; i++) {
		if (init.PDALog[i].text[0] == 0) continue;

		if ((!(init.PDALog[i].flags & PDA_MENU)) && (init.PDALog[i].flags & PDA_UPDATE)) {
			NearMenu = i;
			while (init.PDALog[NearMenu].menu_appartenenza != 0) {
				for (j = 0; j < MAX_PDALOGS; j++) {
					if (init.PDALog[j].text[0] == 0) continue;

					if (init.PDALog[j].menu_creato == init.PDALog[NearMenu].menu_appartenenza) {
						NearMenu = j;
						init.PDALog[NearMenu].flags |= PDA_UPDATE | PDA_ON;
						break;
					}
				}
			}
		}
	}
}

void ScrollLog(Init &init, int Add) {
	int Sign, i;

	if (Add == 0) return;

	Sign = abs(Add) / Add;

	for (i = 0; i < abs(Add); i++) {
		switch (Sign) {
		case +1:
			if (PDAScrollLine + 1 <= init.PDALog[PDALogSorted[PDAScrollPos].PDALogInd].lines) {
				PDAScrollLine++;
			} else {
				PDAScrollLine = 0;
				PDAScrollPos++;
			}
			break;
		case -1:
			if (PDAScrollLine - 1 >= 0) {
				PDAScrollLine--;
			} else {
				if (PDAScrollPos != 0) {
					PDAScrollPos--;
					PDAScrollLine = init.PDALog[PDALogSorted[PDAScrollPos].PDALogInd].lines;
				}
			}
			break;

		}
	}
}


void WriteLog(Init &init, int i, int *CurDate, int *CurLine, int *RealLine, int _PDAScrollLine, int IndentX) {
	t2dWINDOW *w = &t2dWin[T2D_WIN_PDA_LOG];
	int j;

	//Data
	if ((!(init.PDALog[i].flags & PDA_MENU)) && ((i != CurrentLogPos) || (_PDAScrollLine == 0))) {
		strcpy(w->text[T2D_TEXT_PDA_LOG_DATA_START + (*CurDate)].text, init.PDALog[i].info.c_str());
		w->bm[T2D_BM_PDA_LOG_DATA_START + (*CurDate)].py = T2D_PDA_LOG_YI + (*RealLine) * 15;
		w->bm[T2D_BM_PDA_LOG_DATA_START + (*CurDate)].tnum &= ~T2D_BM_OFF;
	} else    w->bm[T2D_BM_PDA_LOG_DATA_START + (*CurDate)].tnum |= T2D_BM_OFF;
	(*CurDate)++;

	//Testo
	for (j = 0; j <= init.PDALog[i].lines; j++) {
		if ((i == CurrentLogPos) && (j < _PDAScrollLine)) continue;

		if (j == init.PDALog[i].lines) {
			(*RealLine)++;
			break;
		}

		strcpy(w->text[T2D_TEXT_PDA_LOG_TEXT_START + (*CurLine)].text, init.PDALog[i].formatted[j].c_str());
		w->bm[T2D_BM_PDA_LOG_TEXT_START + (*CurLine)].px = T2D_PDA_LOG_TEXT_XI + IndentX;
		w->bm[T2D_BM_PDA_LOG_TEXT_START + (*CurLine)].py = T2D_PDA_LOG_YI + (*RealLine) * 15;
		w->bm[T2D_BM_PDA_LOG_TEXT_START + (*CurLine)].tnum &= ~T2D_BM_OFF;

		//Controlla se la scritta rimanda a un menu, se si la scrive in Verde
		if (init.PDALog[i].flags & PDA_MENU) {
			if (init.PDALog[i].flags & PDA_UPDATE)
				w->text[T2D_TEXT_PDA_LOG_TEXT_START + (*CurLine)].color = RED_FONT;
			else    w->text[T2D_TEXT_PDA_LOG_TEXT_START + (*CurLine)].color = GREEN_FONT;
		} else {
			if (init.PDALog[i].flags & PDA_UPDATE)
				w->text[T2D_TEXT_PDA_LOG_TEXT_START + (*CurLine)].color = BLUE_FONT;
			else    w->text[T2D_TEXT_PDA_LOG_TEXT_START + (*CurLine)].color = GRAY_FONT;
		}

		(*CurLine)++;
		(*RealLine)++;

		if ((*RealLine) >= T2D_PDA_LOG_LINES) return;
	}
}

#define PDA_ERROR       0
#define PDA_PRESTOP     1
#define PDA_LAST_LINE   2


int RefreshLogMenu(Init &init, int *Log, int *NumLog, int _PDAScrollLine) {
	int i;
	int CurDate = 0, CurLine = 0, RealLine = 0;
	t2dWINDOW *w = &t2dWin[T2D_WIN_PDA_LOG];

	while (1) {
		if (*Log >= NumPDALogSorted) break;

		CurrentPDALogs[(*NumLog)++] = &init.PDALog[PDALogSorted[*Log].PDALogInd];

		WriteLog(init, PDALogSorted[*Log].PDALogInd, &CurDate, &CurLine, &RealLine, _PDAScrollLine, PDALogSorted[*Log].IndentX);

		if (!(init.PDALog[PDALogSorted[*Log].PDALogInd].flags & PDA_MENU)) init.PDALog[PDALogSorted[*Log].PDALogInd].flags &= ~PDA_UPDATE;

		if (RealLine >= T2D_PDA_LOG_LINES) break;

		(*Log)++;
	}

	CurrentPDALogs[*NumLog] = nullptr;

	for (; CurDate < T2D_BM_PDA_LOG_DATA_END - T2D_BM_PDA_LOG_DATA_START; CurDate++)
		w->bm[T2D_BM_PDA_LOG_DATA_START + CurDate].tnum |= T2D_BM_OFF;

	for (; CurLine < T2D_BM_PDA_LOG_TEXT_END - T2D_BM_PDA_LOG_TEXT_START; CurLine++)
		w->bm[T2D_BM_PDA_LOG_TEXT_START + CurLine].tnum |= T2D_BM_OFF;

	for (i = *NumLog; i < 20; i++) CurrentPDALogs[i] = nullptr;

	if (RealLine >= T2D_PDA_LOG_LINES)
		return PDA_LAST_LINE;
	else    return PDA_PRESTOP;
}

void CloseAllLogMenus(Init &init) {
	int i;

	for (i = 0; i < MAX_PDALOGS; i++) {
		if (init.PDALog[i].text[0] == 0) continue;

		if (init.PDALog[i].flags & PDA_MENU) {
			init.PDALog[i].flags &= ~PDA_MENU_OPEN;
		}
	}
}

int GetNextPDALog(Init &init, int CurrentLog, int Menu) {
	int i;
	int NearMenu = -1, NearTime = -1;

	for (i = 1; i < MAX_PDALOGS; i++) {
		if (i == CurrentLog) continue;
		if ((init.PDALog[i].flags & PDA_ON) == FALSE) continue;
		if (init.PDALog[i].text[0] == 0) continue;
		//if ((PDALog[i].menu_appartenenza!=0) && (!IsMenuOpen(PDALog[i].menu_appartenenza))) continue;
		if (init.PDALog[i].menu_appartenenza != Menu) continue;

		if (CurrentLog == -1) {
			if (init.PDALog[i].flags & PDA_MENU) {
				if (i <= CurrentLog) continue;
				if ((NearMenu == -1) || (i < NearMenu)) NearMenu = i;
			} else {
				if ((NearTime == -1) || (init.PDALog[i].time < init.PDALog[NearTime].time)) NearTime = i;
			}
		} else if (init.PDALog[CurrentLog].flags & PDA_MENU) {
			if (init.PDALog[i].flags & PDA_MENU) {
				if (i <= CurrentLog) continue;
				if ((NearMenu == -1) || (i < NearMenu)) NearMenu = i;
			} else {
				if ((NearTime == -1) || (init.PDALog[i].time < NearTime)) NearTime = i;
			}
		} else {
			if ((init.PDALog[i].flags & PDA_MENU) == FALSE) {
				if (init.PDALog[i].time < init.PDALog[CurrentLog].time) continue;
				if ((init.PDALog[i].time == init.PDALog[CurrentLog].time) && (i <= CurrentLog)) continue;

				if ((init.PDALog[i].time < init.PDALog[NearTime].time) || (NearTime == -1)) {
					NearTime = i;
				} else if (init.PDALog[i].time == init.PDALog[NearTime].time) {
					if (i >= NearTime) continue;
					NearTime = i;
				}
			}
		}
	}

	if (NearMenu != -1) {
		if (NearMenu != CurrentLog) return NearMenu;
		return -1;
	} else {
		if (NearTime != CurrentLog) return NearTime;
		return -1;
	}
}


bool UpdateLogList(Init &init, int Menu, int IndentX, int *NLog) {
	int i = -1;

	if ((i = GetNextPDALog(init, i, Menu)) == -1) {
		return TRUE;
	}

	while (1) {
		PDALogSorted[(*NLog)].PDALogInd = i;
		PDALogSorted[(*NLog)++].IndentX = IndentX;

		if ((init.PDALog[i].flags & PDA_MENU) && (init.PDALog[i].flags & PDA_MENU_OPEN)) {
			UpdateLogList(init, init.PDALog[i].menu_creato, IndentX + 20, NLog);
		}

		if ((i = GetNextPDALog(init, i, Menu)) == -1) {
			return TRUE;
		}
	}

	return TRUE;
}

bool ComputerSearch(WGame &game) {
	int i;
	struct SRect t;
	int OldDoc = computerCurrentDocument;
	computerCurrentDocument = -1;
	Init &init = game.init;
	Renderer &renderer = *game._renderer;

	_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_DOCLIST, 0, FALSE, nullptr, nullptr, nullptr);

	//Elimina spazi finali
	delSpaces(searchName);
	delSpaces(searchSurName);
	delSpaces(searchCaseName);
	delSpaces(searchYear);

	if (AlphaOrCase) {
		//Ricerca tramite nome
		if (searchName.equalsIgnoreCase("ANTONIO") && searchSurName.equalsIgnoreCase("CORONA")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_ANTONIOCORONA;

			SetPDALogItem(init, lPDA3_MENU11_CORONA_ITEM1);
			SetPDALogItem(init, lPDA3_MENU11_CORONA_ITEM2);
			SetPDALogItem(init, lPDA3_MENU11_CORONA_ITEM3);
		} else if (searchName.equalsIgnoreCase("GREGOR") && searchSurName.equalsIgnoreCase("MOORE")) {
			//Abilita finestra DOCLIST
			_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_DOCLIST, 0, TRUE, nullptr, nullptr, nullptr);
			computerCurrentDocument = OldDoc;
			strcpy(t2dWin[T2D_WIN_COMPUTER_DOCLIST].text[T2D_TEXT_COMPUTER_DOCLIST_LINES_START].text, "Gregor Memorandum");
			strcpy(t2dWin[T2D_WIN_COMPUTER_DOCLIST].text[T2D_TEXT_COMPUTER_DOCLIST_LINES_START + 1].text, "Conv. Log: Confidence");
			strcpy(t2dWin[T2D_WIN_COMPUTER_DOCLIST].text[T2D_TEXT_COMPUTER_DOCLIST_LINES_START + 2].text, "Conv. Log: Mercury");
			for (i = T2D_TEXT_COMPUTER_DOCLIST_LINES_START + 3; i < T2D_TEXT_COMPUTER_DOCLIST_LINES_END; i++)
				strcpy(t2dWin[T2D_WIN_COMPUTER_DOCLIST].text[i].text, "");
			computerDocList[0] = T2D_BM_COMPUTER_DOCUMENT_GREGOR_MEM;
			computerDocList[1] = T2D_BM_COMPUTER_DOCUMENT_CONV_CONFIDENCE;
			computerDocList[2] = T2D_BM_COMPUTER_DOCUMENT_CONV_MERCURY;
			computerDocList[3] = -1;
			return TRUE;
		} else if (searchName.equalsIgnoreCase("JACOB") && searchSurName.equalsIgnoreCase("KRENN")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_JACOBKRENN;

			SetPDALogItem(init, lPDA3_MENU13_KRENN_ITEM1);
			SetPDALogItem(init, lPDA3_MENU13_KRENN_ITEM2);
		} else if (searchName.equalsIgnoreCase("PETER") && searchSurName.equalsIgnoreCase("DUKES")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_PETERDUKES;

			SetPDALogItem(init, lPDA3_MENU12_DUKES_ITEM1);
			SetPDALogItem(init, lPDA3_MENU12_DUKES_ITEM2);
		} else if (searchName.equalsIgnoreCase("VICTOR") && searchSurName.equalsIgnoreCase("VALENCIA")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_VICTORVALENCIA;

			SetPDALogItem(init, lPDA3_MENU14_VALENCIA_ITEM1);
			SetPDALogItem(init, lPDA3_MENU14_VALENCIA_ITEM2);
		} else if (searchName.equalsIgnoreCase("PIER TOMMASO") && searchSurName.equalsIgnoreCase("BENNATI")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_BENNA;
		} else if (searchName.equalsIgnoreCase("ANDREA") && searchSurName.equalsIgnoreCase("CORDELLA")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_ANDREA;
		} else if (searchName.equalsIgnoreCase("ALESSANDRO") && searchSurName.equalsIgnoreCase("BELLONDI")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_BELLONDS;
		} else if (searchName.equalsIgnoreCase("DANIELE") && searchSurName.equalsIgnoreCase("ANTONA")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_DANIELE;
		} else if (searchName.equalsIgnoreCase("EDOARDO") && searchSurName.equalsIgnoreCase("GERVINO")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_EDO;
		} else if (searchName.equalsIgnoreCase("FEDERICO") && searchSurName.equalsIgnoreCase("FASCE")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_FEDE;
		} else if (searchName.equalsIgnoreCase("FABIO") && searchSurName.equalsIgnoreCase("CORICA")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_FIABE;
		} else if (searchName.equalsIgnoreCase("MARCO") && searchSurName.equalsIgnoreCase("CASTRUCCI")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_FOX;
		} else if (searchName.equalsIgnoreCase("ALESSANDRO") && searchSurName.equalsIgnoreCase("GIUSTI")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_GIUSTI;
		} else if (searchName.equalsIgnoreCase("CHRISTIAN") && searchSurName.equalsIgnoreCase("CANTAMESSA")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_KRIZIA;
		} else if (searchName.equalsIgnoreCase("STEFANO") && searchSurName.equalsIgnoreCase("MARIANI")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_NONJI;
		} else if (searchName.equalsIgnoreCase("STEFANO") && searchSurName.equalsIgnoreCase("PROSPERI")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_SPARTECO;
		} else if (searchName.equalsIgnoreCase("FABRIZIO") && searchSurName.equalsIgnoreCase("LAGORIO")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_TAG;
		} else if (searchName.equalsIgnoreCase("TIZIANO") && searchSurName.equalsIgnoreCase("SARDONE")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_TIZ;
		} else if (searchName.equalsIgnoreCase("WALTER") && searchSurName.equalsIgnoreCase("BERGAMINO")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_WALTER;
		} else if (searchName.equalsIgnoreCase("VICTORIA") && searchSurName.equalsIgnoreCase("HEWARD")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_VICKY;
		} else if (searchName.equalsIgnoreCase("PIETRO") && searchSurName.equalsIgnoreCase("MONTELATICI")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_PIETRO;
		}

	} else {
		//Ricerca tramite caso
		if (searchCaseName.equalsIgnoreCase("MERCURY") && searchYear.equalsIgnoreCase("1969")) {
			//Abilita finestra DOCLIST
			_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_DOCLIST, 0, TRUE, nullptr, nullptr, nullptr);
			computerCurrentDocument = OldDoc;
			strcpy(t2dWin[T2D_WIN_COMPUTER_DOCLIST].text[T2D_TEXT_COMPUTER_DOCLIST_LINES_START].text, "Mercury 1969-1");
			strcpy(t2dWin[T2D_WIN_COMPUTER_DOCLIST].text[T2D_TEXT_COMPUTER_DOCLIST_LINES_START + 1].text, "Mercury 1969-2");
			strcpy(t2dWin[T2D_WIN_COMPUTER_DOCLIST].text[T2D_TEXT_COMPUTER_DOCLIST_LINES_START + 2].text, "Mercury 1969-3");
			strcpy(t2dWin[T2D_WIN_COMPUTER_DOCLIST].text[T2D_TEXT_COMPUTER_DOCLIST_LINES_START + 3].text, "Mercury 1969-4");
			for (i = T2D_TEXT_COMPUTER_DOCLIST_LINES_START + 4; i < T2D_TEXT_COMPUTER_DOCLIST_LINES_END; i++)
				strcpy(t2dWin[T2D_WIN_COMPUTER_DOCLIST].text[i].text, "");
			computerDocList[0] = T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_1;
			computerDocList[1] = T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_2;
			computerDocList[2] = T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_3;
			computerDocList[3] = T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_4;
			computerDocList[4] = -1;
			return TRUE;
		} else if (searchCaseName.equalsIgnoreCase("MERCURY") && searchYear.equalsIgnoreCase("1969-1")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_1;
		} else if (searchCaseName.equalsIgnoreCase("MERCURY") && searchYear.equalsIgnoreCase("1969-2")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_2;
		} else if (searchCaseName.equalsIgnoreCase("MERCURY") && searchYear.equalsIgnoreCase("1969-3")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_3;
		} else if (searchCaseName.equalsIgnoreCase("MERCURY") && searchYear.equalsIgnoreCase("1969-4")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_4;
		} else if (searchCaseName.equalsIgnoreCase("CONFIDENCE") && searchYear.equalsIgnoreCase("1942")) {
			//Abilita finestra DOCLIST
			_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_DOCLIST, 0, TRUE, nullptr, nullptr, nullptr);
			computerCurrentDocument = OldDoc;
			strcpy(t2dWin[T2D_WIN_COMPUTER_DOCLIST].text[T2D_TEXT_COMPUTER_DOCLIST_LINES_START].text, "Confidence 1942-1");
			strcpy(t2dWin[T2D_WIN_COMPUTER_DOCLIST].text[T2D_TEXT_COMPUTER_DOCLIST_LINES_START + 1].text, "Mercury 1942-2");
			strcpy(t2dWin[T2D_WIN_COMPUTER_DOCLIST].text[T2D_TEXT_COMPUTER_DOCLIST_LINES_START + 2].text, "Mercury 1942-3");
			for (i = T2D_TEXT_COMPUTER_DOCLIST_LINES_START + 3; i < T2D_TEXT_COMPUTER_DOCLIST_LINES_END; i++)
				strcpy(t2dWin[T2D_WIN_COMPUTER_DOCLIST].text[i].text, "");
			computerDocList[0] = T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_1;
			computerDocList[1] = T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_2;
			computerDocList[2] = T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_3;
			computerDocList[3] = -1;
			return TRUE;
		} else if (searchCaseName.equalsIgnoreCase("CONFIDENCE") && searchYear.equalsIgnoreCase("1942-1")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_1;
		} else if (searchCaseName.equalsIgnoreCase("CONFIDENCE") && searchYear.equalsIgnoreCase("1942-2")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_2;
		} else if (searchCaseName.equalsIgnoreCase("CONFIDENCE") && searchYear.equalsIgnoreCase("1942-3")) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_3;
		} else if (searchCaseName.equalsIgnoreCase("ARL") && (searchYear.equalsIgnoreCase("1998") || searchYear.equalsIgnoreCase("1998-1"))) {
			computerCurrentDocument = T2D_BM_COMPUTER_DOCUMENT_ARL_1998_1;

			SetPDALogItem(init, lPDA4_MENU17_ITEM1);
		}
	}

	if (computerCurrentDocument >= 0) {
		if (computerCurrentDocument == OldDoc) return TRUE;

		//Disattiva Bitmap del documento precedente
		if (OldDoc >= 0) t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[OldDoc].tnum |= T2D_BM_OFF;

		//Attiva nuovo documento
		t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].tnum &= ~T2D_BM_OFF;
		if (!t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].tnum)
			t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].tnum  = LoadDDBitmap(game, ComputerCIAFileNames[computerCurrentDocument - T2D_BM_COMPUTER_DOCUMENT_FIRST], 0);

		if (computerCurrentDocument == T2D_BM_COMPUTER_DOCUMENT_ARL_1998_1)
			game.init.Obj[o25FORNOAP].flags |= EXTRA2;

		//Setta paramentri del nuovo documento
		GetDDBitmapExtends(renderer, &t, &t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument]);
		t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].px = game._renderer->rInvFitX(26);
		t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].py = game._renderer->rInvFitY(66);
		t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].ox = 0;
		t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].oy = 0;
		t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].dx = 0;
		t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].dy = 0;
		if (t.y2 - t.y1 > 407) {
			t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].dy = game._renderer->rInvFitY(407);
		} else {
			t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].dy = game._renderer->rInvFitY(t.y2 - t.y1);
		}

		//Attiva la finestra Document
		_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_DOCUMENT, 0, TRUE, nullptr, nullptr, nullptr);
		return TRUE;
	} else if (computerCurrentDocument != -2) {
		//Search Error
		t2dWin[T2D_WIN_COMPUTER_ERROR].bm[T2D_BM_COMPUTER_SEARCHERROR_WIN].tnum &= ~T2D_BM_OFF;
		t2dWin[T2D_WIN_COMPUTER_ERROR].bt[T2D_BT_COMPUTER_ERROR_OK].on = TRUE;
		t2dWin[T2D_WIN_COMPUTER_ERROR].bt[T2D_BT_SearchError_Win_TGA].on = TRUE;
		t2dWin[T2D_WIN_COMPUTER_ERROR].sy = 3;
		_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_ERROR, 0, TRUE, nullptr, nullptr, nullptr);
		return FALSE;
	}

	return FALSE;
}


int CalcLogTotLines(Init &init) {
	int i;

	LogTotLines = 0;
	LogCurAbsLine = 0;

	for (i = 0; i < NumPDALogSorted; i++) {
		if (i == PDAScrollPos) LogCurAbsLine = LogTotLines + PDAScrollLine;
		LogTotLines += init.PDALog[PDALogSorted[i].PDALogInd].lines + 1;
	}

	return LogTotLines;
}

void AggiornaLogSB(Init &init, int Flags) {
	t2dWINDOW *w = &t2dWin[T2D_WIN_PDA_LOG];

	CalcLogTotLines(init);

	if (PiroloGrab != 0) return;

	switch (Flags) {
	case RESTARTALL:
		LogSBPos = 0;
		LogSBDy = 0;
		if (LogTotLines <= T2D_PDA_LOG_LINES) {
			w->bm[T2D_BM_PDA_LOG_SB].tnum |= T2D_BM_OFF;
			w->bt[T2D_BT_PDA_LOG_SB].on = FALSE;
			return;
		}
		break;
	case REFRESH:
		if ((PDAScrollPos == 0) && (PDAScrollLine == 0) && (LogTotLines <= T2D_PDA_LOG_LINES)) {
			w->bm[T2D_BM_PDA_LOG_SB].tnum |= T2D_BM_OFF;
			w->bt[T2D_BT_PDA_LOG_SB].on = FALSE;
			return;
		}
		break;
	}

	LogSBDy = (double)(T2D_PDA_LOG_SB_TOTY) / (double)(LogTotLines - T2D_PDA_LOG_LINES);
	LogSBPos = T2D_PDA_LOG_SB_STARTY + (int)(LogCurAbsLine * LogSBDy);

	w->bm[T2D_BM_PDA_LOG_SB].tnum &= ~T2D_BM_OFF;
	w->bm[T2D_BM_PDA_LOG_SB].py = LogSBPos;
	w->bt[T2D_BT_PDA_LOG_SB].on = TRUE;
	w->bt[T2D_BT_PDA_LOG_SB].lim.y1 = LogSBPos;
	w->bt[T2D_BT_PDA_LOG_SB].lim.y2 = LogSBPos + T2D_PDA_LOG_SB_PIRGY;
}

void FormattaLogs(WGame &game) {
	Renderer &renderer = *game._renderer;
	Init &init = game.init;
	int i, j, t, CurInit, Dx;

	for (i = 0; i < NumPDALogSorted; i++) {
		j = 0;
		for (t = 0; t < MAX_PDA_INFO; t++) {
			if (init.PDALog[PDALogSorted[i].PDALogInd].text[t] == 0) break;

			game._fonts.getTextDim(Sentence[init.PDALog[PDALogSorted[i].PDALogInd].text[t]], FontKind::Standard, &init.PDALog[PDALogSorted[i].PDALogInd].dx, &init.PDALog[PDALogSorted[i].PDALogInd].dy);
			Dx = init.PDALog[PDALogSorted[i].PDALogInd].dx;
			CurInit = 0;
			while (Dx + PDALogSorted[i].IndentX > renderer.rFitX(T2D_PDA_LOG_TEXT_XF - T2D_PDA_LOG_TEXT_XI)) {
				for (Dx = (signed)strlen((char *)(Sentence[init.PDALog[PDALogSorted[i].PDALogInd].text[t]])) - 1; Dx >= 0; Dx--) {
					if ((Sentence[init.PDALog[PDALogSorted[i].PDALogInd].text[t]][CurInit + Dx] == ' ') && (TextLen(game._fonts, (char *)(Sentence[init.PDALog[PDALogSorted[i].PDALogInd].text[t]] + CurInit), (int16)Dx) + PDALogSorted[i].IndentX < renderer.rFitX(T2D_PDA_LOG_TEXT_XF - T2D_PDA_LOG_TEXT_XI))) {
						Sentence[init.PDALog[PDALogSorted[i].PDALogInd].text[t]][CurInit + Dx] = 0;
						strcpy(init.PDALog[PDALogSorted[i].PDALogInd].formatted[j].c_str(), (char *)(Sentence[init.PDALog[PDALogSorted[i].PDALogInd].text[t]] + CurInit));
						Sentence[init.PDALog[PDALogSorted[i].PDALogInd].text[t]][CurInit + Dx] = ' ';
						j++;
						CurInit += Dx + 1;
						break;
					}
				}

				game._fonts.getTextDim((char *)(Sentence[init.PDALog[PDALogSorted[i].PDALogInd].text[t]] + CurInit), FontKind::Standard, &Dx, &init.PDALog[PDALogSorted[i].PDALogInd].dy);
			}

			strcpy(init.PDALog[PDALogSorted[i].PDALogInd].formatted[j].c_str(), (char *)(Sentence[init.PDALog[PDALogSorted[i].PDALogInd].text[t]] + CurInit));

			j++;
		}

		init.PDALog[PDALogSorted[i].PDALogInd].lines = j;
		init.PDALog[PDALogSorted[i].PDALogInd].dy = 15 * init.PDALog[PDALogSorted[i].PDALogInd].lines;

		if (((init.PDALog[PDALogSorted[i].PDALogInd].flags & PDA_MENU) == FALSE) && (init.PDALog[PDALogSorted[i].PDALogInd].time >= 0)) {
			snprintf(init.PDALog[PDALogSorted[i].PDALogInd].info.c_str(), 6, "%04d", init.PDALog[PDALogSorted[i].PDALogInd].time);
			init.PDALog[PDALogSorted[i].PDALogInd].info.c_str()[5] = 0;
			init.PDALog[PDALogSorted[i].PDALogInd].info.c_str()[4] = init.PDALog[PDALogSorted[i].PDALogInd].info.c_str()[3];
			init.PDALog[PDALogSorted[i].PDALogInd].info.c_str()[3] = init.PDALog[PDALogSorted[i].PDALogInd].info.c_str()[2];
			init.PDALog[PDALogSorted[i].PDALogInd].info.c_str()[2] = ':';
		}
	}
}


void RefreshLog(WGame &game, int Add) {
	int NumLogInWin = 0;
	int NumLog = 0;
	int Ris;
	Init &init = game.init;

	if (Add == RESTARTALL) {
		CurrentLogPos = 0;
		while ((!(init.PDALog[CurrentLogPos].flags & ON)) && (CurrentLogPos < MAX_PDALOGS))
			CurrentLogPos++;
		NumMenus = 0;
		CurrentLogMenu = 0;
		PDAScrollLine = 0;
		NumPDALogSorted = 0;
		MainTitle = 0;
		PDAScrollPos = 0;
		CloseAllLogMenus(init);
		return;
	} else if (Add == REFRESH) {
		RefreshUpdate(init);
		NumPDALogSorted = 0;
		UpdateLogList(init, 0, 0, &NumPDALogSorted);
		CurrentLogPos = PDALogSorted[PDAScrollPos].PDALogInd;
	} else {
		ScrollLog(init, Add);
		CurrentLogPos = PDALogSorted[PDAScrollPos].PDALogInd;
	}

	CalcLogTotLines(init);
	if ((LogCurAbsLine != 0) && (LogTotLines - LogCurAbsLine < T2D_PDA_LOG_LINES)) {
		RefreshLog(game, -(T2D_PDA_LOG_LINES - (LogTotLines - LogCurAbsLine)));
		return;
	}

	NumLog = PDAScrollPos;

	FormattaLogs(game);

	//RefreshLogMenu(0, 0, &NumLog, &NumLogInWin, &CurDate, &CurLine, &RealLine, PDAScrollLine);
	Ris = RefreshLogMenu(init, &NumLog, &NumLogInWin, PDAScrollLine);
	AggiornaLogSB(init, REFRESH);
	switch (Ris) {
	case PDA_PRESTOP:
		if (Add == REFRESH) return;
		if (Add < 0) return;
		RefreshLog(game, -1);
		break;
	case PDA_LAST_LINE:
		break;
	}
}

/* -----------------16/01/99 10.24-------------------
 *                      PaintT2D
 * --------------------------------------------------*/
void PaintT2D(Renderer &renderer) {
	t2dWINDOW *w;
	int32 a, i, j;

	if (!bT2DActive) return;
//	Ridisegna tutte le bitmaps da visualizzare
	for (a = 0; a < T2D_MAX_WINDOWS; a++) {
		if (!WinActive[a]) continue;

		w = &t2dWin[WinActive[a]];
		for (i = 0; i < T2D_MAX_BITMAPS_IN_WIN; i++) {
			if (!w->bm[i].tnum) continue;
			if (w->bm[i].tnum & T2D_BM_OFF) continue;

			if (w->bm[i].tnum & T2D_BM2TEXT_MASK) {
				j = ((w->bm[i].tnum & T2D_BM2TEXT_MASK) >> T2D_BM2TEXT_SHIFT);
				renderer._2dStuff.displayDDText(w->text[j].text, w->text[j].font, w->text[j].color,
				              w->bm[i].px + w->px, w->bm[i].py + w->py, w->bm[i].ox, w->bm[i].oy, w->bm[i].dx, w->bm[i].dy);
			} else
				renderer._2dStuff.displayDDBitmap(w->bm[i].tnum, w->bm[i].px + w->px, w->bm[i].py + w->py, w->bm[i].ox, w->bm[i].oy, w->bm[i].dx, w->bm[i].dy);
		}
	}

	CheckExtraLocalizationStrings(renderer, ExtraLSId);
}

//Restituisce il puntatore alla finestra corrente (cioe quella piu vicina)
int T2DActualWindow(t2dWINDOW **w) {
	int i;
	for (i = T2D_MAX_WINDOWS - 1; i >= 0; i--)
		if (WinActive[i]) {
			if (w != NULL) *w = &t2dWin[WinActive[i]];
			return WinActive[i];
		}

	return -1;
}

bool CheckRect2(struct SRect p, int32 cmx, int32 cmy) {
	return ((cmx >= p.x1) && (cmx < p.x2) && (cmy >= p.y1) && (cmy < p.y2));
}


//Gestione Stringhe
void AddKeyToString(char Key, char *String, int MaxChars) {
	int Pos;

	if (((Key >= '0') & (Key <= '9')) | ((Key >= 'A') & (Key <= 'Z')) |
	        ((Key >= 'a') & (Key <= 'z')) | (Key == '-') | (Key == VK_SPACE)) {
		if (strcmp(String, "") != 0)
			Pos = strlen(String);
		else    Pos = 0;
		if (Pos < MaxChars) {
			if (CursorVisible) {
				Pos = Pos - 1;
				CursorVisible = false;
			}
			String[Pos] = Key;
			String[Pos + 1] = 0;
		}
	} else if (Key == VK_BACK) {
		Pos = strlen(String) - 1;
		if (CursorVisible) {
			Pos = Pos - 1;
			CursorVisible = false;
		}
		if (Pos >= 0) String[Pos] = 0;
		else        String[0] = 0;
	}
}

//Gestione Tastiera
void T2DResetKeyboard() {
	t2dKeyTableT = 0;
	t2dKeyTableH = 0;
}

void T2DAddKeyInput(char key) {
	if (!bT2DActive) return;

	t2dKeyTable[t2dKeyTableT] = key;

	if ((t2dKeyTableT + 1) % T2D_KBD_MAX_ENTRIES == t2dKeyTableH) return; //La testa � stata raggiunta (situazione quasi impossibile)
	t2dKeyTableT = (t2dKeyTableT + 1) % T2D_KBD_MAX_ENTRIES;
}


//Gestione delle barre nel menu delle opzioni
//Se percent!=-1 il pirolo viene posizionato in base alla percentuale
//Il valore restituito � la percentuale
int GestioneBarra(struct SDDBitmap *Barra, struct SDDBitmap *Pirolo, int MouseX, int Percent) {
#define BARRA_START_X 20
#define BARRA_END_X 175
#define BARRA_CENTRO_Y  13
#define PIROLO_CENTRO_X 3
#define PIROLO_CENTRO_Y 10

	int PiroloX;

	if (Percent == -1)
		PiroloX = MouseX - Barra->px;
	else    PiroloX = (int)(BARRA_START_X + (float)((BARRA_END_X - BARRA_START_X) / 100.0) * Percent);
	//PiroloX=(int)(BARRA_START_X+(float)((BARRA_END_X-BARRA_START_X)/100.0)*Percent);

	if (PiroloX < BARRA_START_X) PiroloX = BARRA_START_X;
	if (PiroloX > BARRA_END_X) PiroloX = BARRA_END_X;

	Pirolo->px = Barra->px + PiroloX - PIROLO_CENTRO_X;
	Pirolo->py = Barra->py + BARRA_CENTRO_Y - PIROLO_CENTRO_Y;

	if (Percent == -1)
		return (int)(((float)(PiroloX - BARRA_START_X) * 100.0) / (float)(BARRA_END_X - BARRA_START_X));
	else    return Percent;
}

void ActivateCursor(bool Active, char *String) {
	int Pos;

	if (Active) {
		CursorString = String;
	} else {
		if (CursorVisible) {
			Pos = strlen(CursorString) - 1;
			if (Pos >= 0) CursorString[Pos] = 0;
			else        CursorString[0] = 0;
		}
	}

	CursorVisible = false;
	CursorActive = Active;
	_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DCONTINUE, MP_DEFAULT, 0, 0, T2D_CURSOR, nullptr, nullptr, nullptr);
}


void doT2DKeyboard(WGame &game) {
	char Key;
	int ActualWindow;
	t2dWINDOW *w;
	int Pos;
	int Len;
// char Name[MAX_PATH];
	Init &init = game.init;

	if (!bT2DActive) return;

	while (t2dKeyTableH != t2dKeyTableT) {
		Key = t2dKeyTable[t2dKeyTableH];
		t2dKeyTableH = (t2dKeyTableH + 1) % T2D_KBD_MAX_ENTRIES;

		switch (bT2DActive) {
		case tCOMPUTER:
			ActualWindow = T2DActualWindow(&w);

			switch (ActualWindow) {
			case T2D_WIN_COMPUTER_BACKGROUND:
				break;

			case T2D_WIN_COMPUTER_DIALUP:
				if (Key == VK_BACK) {
					//Suono
					StartSound(game, wCIAPULSANTE);

					Len = strlen(computerNumeroTel) - 1;
					if (Len >= 0) computerNumeroTel[Len] = 0;
					strcpy(t2dWin[T2D_WIN_COMPUTER_DIALUP].text[T2D_TEXT_COMPUTER_DIALUP_NUMBER].text, computerNumeroTel);
				}

				if (Key == VK_DELETE) {
					//Suono
					StartSound(game, wCIAPULSANTE);

					strcpy(computerNumeroTel, "");
					strcpy(t2dWin[T2D_WIN_COMPUTER_DIALUP].text[T2D_TEXT_COMPUTER_DIALUP_NUMBER].text, computerNumeroTel);
				}

				if ((Key >= '0') & (Key <= '9')) {
					//Suono
					StartSound(game, wCIAPULSANTE);

					Pos = strlen(computerNumeroTel);
					if (Pos < COMPUTER_MAX_NUM_DIALUP - 1) {
						computerNumeroTel[Pos] = Key;
						computerNumeroTel[Pos + 1] = 0;
						strcpy(t2dWin[T2D_WIN_COMPUTER_DIALUP].text[T2D_TEXT_COMPUTER_DIALUP_NUMBER].text, computerNumeroTel);
					}
				}

				if (Key == VK_RETURN) {
					//Suono
					StartSound(game, wCIAPULSANTE);

					//Riabilita DInput
					//ResetDIKbd();
					//bIngnoreDIKeyboard = FALSE;

					if (LoaderFlags & T3D_DEBUGMODE) {
						if (strcmp(computerNumeroTel, "123") == 0) {
							strcpy(computerNumeroTel, computerNumeroEsatto);
							init.InvObj[i28WALKMANOK].flags |= EXTRA2;
						}
					}

					/*                        #ifdef _DEBUG
					                        InvObj[i28WALKMANOK].flags|=EXTRA2;
					                        #endif*/

					if (init.InvObj[i28WALKMANOK].flags & EXTRA2) {  //solo se ho sentito il numero
						if (strcmp(computerNumeroTel, computerNumeroEsatto) == 0) {
							if (!(init.Obj[o17COMPUTER].flags & EXTRA2)) {
								IncCurTime(game, 20);
								init.Obj[o17COMPUTER].flags |= EXTRA2;
							}
							//Fa partire sequenza di connessione
							StartSound(game, wMODEMFULL);
							_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DCONTINUE, MP_DEFAULT, 0, 0, T2D_COMPUTER_CONNECTING, nullptr, nullptr, nullptr);
							/*//Disattiva finestra Dialup
							Event( EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_DIALUP, 0, FALSE, NULL, NULL, NULL );
							t2dWin[T2D_WIN_COMPUTER_BACKGROUND].bt[T2D_BT_COMPUTER_BACKGROUND_EMAIL].on=TRUE;
							t2dWin[T2D_WIN_COMPUTER_BACKGROUND].bt[T2D_BT_COMPUTER_BACKGROUND_SEARCH].on=TRUE;*/
						} else {
							//Disattiva finestra DialUp
							_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_PDA_BUTTON_DELAY, 0, EFFECT_WAIT, nullptr, nullptr, nullptr);
							_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_DIALUP, 0, FALSE, nullptr, nullptr, nullptr);
							//Dial Error
							t2dWin[T2D_WIN_COMPUTER_ERROR].bm[T2D_BM_COMPUTER_DIALERROR_WIN].tnum &= ~T2D_BM_OFF;
							t2dWin[T2D_WIN_COMPUTER_ERROR].bt[T2D_BT_COMPUTER_ERROR_DIALUP].on = TRUE;
							t2dWin[T2D_WIN_COMPUTER_ERROR].bt[T2D_BT_COMPUTER_ERROR_DIALQUIT].on = TRUE;
							t2dWin[T2D_WIN_COMPUTER_ERROR].bt[T2D_BT_DialError_Win_TGA].on = TRUE;
							t2dWin[T2D_WIN_COMPUTER_ERROR].sy = 2;
							_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_ERROR, 0, TRUE, nullptr, nullptr, nullptr);
							//Suono
							StartSound(game, wCIAERRORE);
						}
					} else {
						//Disattiva finestra DialUp
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_DIALUP, 0, FALSE, nullptr, nullptr, nullptr);
						//Com Error
						t2dWin[T2D_WIN_COMPUTER_ERROR].bm[T2D_BM_COMPUTER_COMERROR_WIN].tnum &= ~T2D_BM_OFF;
						t2dWin[T2D_WIN_COMPUTER_ERROR].bt[T2D_BT_COMPUTER_ERROR_COMQUIT].on = TRUE;
						t2dWin[T2D_WIN_COMPUTER_ERROR].bt[T2D_BT_ComError_Win_TGA].on = TRUE;
						t2dWin[T2D_WIN_COMPUTER_ERROR].sy = 1;
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_ERROR, 0, TRUE, nullptr, nullptr, nullptr);
						//Suono
						StartSound(game, wCIAERRORE);
					}
				}
				break;

			case T2D_WIN_COMPUTER_SEARCH: {
				Common::String *String = nullptr;
				int Max = 0;
				switch (WhatText) {
				case  0:
					break;
				case  1:
					String = &searchName;
					Max = COMPUTER_SEARCH_MAX_NAME;
					break;
				case  2:
					String = &searchSurName;
					Max = COMPUTER_SEARCH_MAX_SURNAME;
					break;
				case  3:
					String = &searchCaseName;
					Max = COMPUTER_SEARCH_MAX_CASENAME;
					break;
				case  4:
					String = &searchYear;
					Max = COMPUTER_SEARCH_MAX_YEAR;
					break;
				}

				if (Key == VK_TAB) {
					w->bm[T2D_BM_COMPUTER_SEARCH_NAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_YEAR_ON].tnum |= T2D_BM_OFF;

					ActivateCursor(FALSE, NULL);

					switch (WhatText) {
					case T2D_TEXT_COMPUTER_SEARCH_NAME:
						WhatText = T2D_TEXT_COMPUTER_SEARCH_SURNAME;
						w->bm[T2D_BM_COMPUTER_SEARCH_NAME_ON].tnum |= T2D_BM_OFF;
						w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME_ON].tnum &= ~T2D_BM_OFF;

						strcpy(w->text[WhatText].text, "");
						searchSurName = "";
						break;
					case T2D_TEXT_COMPUTER_SEARCH_SURNAME:
						WhatText = T2D_TEXT_COMPUTER_SEARCH_NAME;
						w->bm[T2D_BM_COMPUTER_SEARCH_NAME_ON].tnum &= ~T2D_BM_OFF;
						w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME_ON].tnum |= T2D_BM_OFF;

						strcpy(w->text[WhatText].text, "");
						searchName = "";
						break;
					case T2D_TEXT_COMPUTER_SEARCH_CASENAME:
						WhatText = T2D_TEXT_COMPUTER_SEARCH_YEAR;
						w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_ON].tnum |= T2D_BM_OFF;
						w->bm[T2D_BM_COMPUTER_SEARCH_YEAR_ON].tnum &= ~T2D_BM_OFF;

						strcpy(w->text[WhatText].text, "");
						searchYear = "";
						break;
					case T2D_TEXT_COMPUTER_SEARCH_YEAR:
						WhatText = T2D_TEXT_COMPUTER_SEARCH_CASENAME;
						w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_ON].tnum &= ~T2D_BM_OFF;
						w->bm[T2D_BM_COMPUTER_SEARCH_YEAR_ON].tnum |= T2D_BM_OFF;

						strcpy(w->text[WhatText].text, "");
						searchCaseName = "";
						break;
					}

					ActivateCursor(TRUE, w->text[WhatText].text);
				}

				if (Key == VK_RETURN) {
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_SEARCH_SEARCH_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_COMPUTER_BUTTON_DELAY, 0, EFFECT_WAIT, nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_SEARCH, T2D_BM_COMPUTER_SEARCH_SEARCH_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);

					ComputerSearch(game);
				}

				if (WhatText == 0) break;

				/*if (Key==VK_BACK)
				{
				    Pos=strlen(String)-1;
				    if (Pos>=0) String[Pos]=0;
				    strcpy( w->text[WhatText].text, String );
				}

				if (((Key>='0') & (Key<='9')) | ((Key>='A') & (Key<='Z')) |
				    ((Key>='a') & (Key<='z')) | (Key=='-') | (Key==VK_SPACE))
				{
				    Pos=strlen(String);
				    if (Pos<Max-1)
				    {
				        String[Pos]=Key;
				        String[Pos+1]=0;
				        strcpy( w->text[WhatText].text, String );
				    }
				}*/
				if (((Key >= '0') & (Key <= '9')) | ((Key >= 'A') & (Key <= 'Z')) |
				        ((Key >= 'a') & (Key <= 'z')) | (Key == '-') | (Key == VK_SPACE) |
				        (Key == VK_BACK)) {
					AddKeyToString(Key, w->text[WhatText].text, Max);
					warning("HACK: AddKeyToString really should work with Common::String, not a hacked iterator");
					AddKeyToString(Key, String->begin(), Max);
				}

				break;
			}
			}
			break;

			/*case tOPTIONS:
			    if (optionsSaveDescriptActive)
			    {
			        w = &t2dWin[T2D_WIN_OPTIONS_MAIN];
			        if (((Key>='0') & (Key<='9')) | ((Key>='A') & (Key<='Z')) |
			        ((Key>='a') & (Key<='z')) | (Key=='-') | (Key==VK_SPACE) | (Key==VK_BACK))
			        {
			            AddKeyToString(Key, w->text[T2D_TEXT_OPTIONS_SAVE_START+optionsWhat].text, T2D_OPTIONS_MAX_DESCRIPT);
			        }
			        else if (Key==VK_RETURN)
			        {
			            if (strlen(w->text[T2D_TEXT_OPTIONS_SAVE_START+optionsWhat].text)==0) break;

			            if (CursorVisible)
			            {
			                Pos=strlen(w->text[T2D_TEXT_OPTIONS_SAVE_START+optionsWhat].text)-1;
			                w->text[T2D_TEXT_OPTIONS_SAVE_START+optionsWhat].text[Pos]=0;
			            }

			            DataSave(w->text[T2D_TEXT_OPTIONS_SAVE_START+optionsWhat].text, optionsSlot);
			            sprintf(Name, "%sWmSav%02d.tga", WmGameDir, optionsSlot);
			            CopyFile(T2D_TEMP_SAVE_FILE_NAME, Name, FALSE);
			            RefreshSaveImg(optionsCurPos, 0);

			            optionsSaveDescriptActive=false;

			            ActivateCursor(FALSE, NULL);

			            //Esce dalle Opzioni
			            Event( EventClass::MC_T2D, ME_T2DSTOP, MP_DEFAULT, 0, 0, tOPTIONS, NULL, NULL, NULL );
			        }
			    }
			    break;*/
		}
	}
}

void doT2DMouseMainMenu(WGame &game, t2dWINDOW *w) {
	w->bm[T2D_BM_MAINMENU_PLAY_ON].tnum |= T2D_BM_OFF;
	w->bm[T2D_BM_MAINMENU_LOAD_ON].tnum |= T2D_BM_OFF;
	w->bm[T2D_BM_MAINMENU_EXIT_ON].tnum |= T2D_BM_OFF;
	w->bm[T2D_BM_MAINMENU_PLAY_OFF].tnum &= ~T2D_BM_OFF;
	w->bm[T2D_BM_MAINMENU_LOAD_OFF].tnum &= ~T2D_BM_OFF;
	w->bm[T2D_BM_MAINMENU_EXIT_OFF].tnum &= ~T2D_BM_OFF;

	if (CurButtonWin == T2D_WIN_MAINMENU_MAIN) {
		if (CurButton == T2D_BT_MAINMENU_PLAY) {
			if (TheMessage->event == ME_MOUSEUPDATE) {
				w->bm[T2D_BM_MAINMENU_PLAY_ON].tnum &= ~T2D_BM_OFF;
				w->bm[T2D_BM_MAINMENU_PLAY_OFF].tnum |= T2D_BM_OFF;
			} else {
				//Inizia il gioco
				_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTOP, MP_DEFAULT, STOP_T2D__LAUNCH_NEWGAME, 0, tMAINMENU,
						nullptr, nullptr, nullptr);
			}
		} else if (CurButton == T2D_BT_MAINMENU_LOAD) {
			if (TheMessage->event == ME_MOUSEUPDATE) {
				w->bm[T2D_BM_MAINMENU_LOAD_ON].tnum &= ~T2D_BM_OFF;
				w->bm[T2D_BM_MAINMENU_LOAD_OFF].tnum |= T2D_BM_OFF;
			} else {
				//Va alla schermata di caricamento
				LoadWindow_WhichBkg = 1;
				_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTOP, MP_DEFAULT, STOP_T2D__LAUNCH_LOAD, 0, tMAINMENU, nullptr,
						nullptr, nullptr);
			}

		} else if (CurButton == T2D_BT_MAINMENU_EXIT) {
			if (TheMessage->event == ME_MOUSEUPDATE) {
				w->bm[T2D_BM_MAINMENU_EXIT_ON].tnum &= ~T2D_BM_OFF;
				w->bm[T2D_BM_MAINMENU_EXIT_OFF].tnum |= T2D_BM_OFF;
			} else {
				//Esce a windows
				CloseSys(game);
			}
		}
	}
}

/* -----------------16/01/99 11.16-------------------
 *                      doT2DMouse
 * --------------------------------------------------*/
void doT2DMouse(WGame &game) {
	struct SRect t;
	struct SDDBitmap *d, *d1, *d2;
	t2dWINDOW *w = nullptr;
	t2dBUTTON *b = nullptr;
	int32 a = 0, i = 0;
	char Name[MAX_PATH];
	//Variabili per gestione scrolling
	int32 StartY = 0, DimY = 0;
	char Text[1000];
	int16 mouse_x, mouse_y;
	Init &init = game.init;
	Renderer &renderer = *game._renderer;

	mouse_x = renderer.rInvFitX(TheMessage->wparam1);
	mouse_y = renderer.rInvFitY(TheMessage->wparam2);

	if (!bT2DActive) return;

	if (!bLPressed && !bRPressed) {
		LinkWinToMouse = 0;
		LinkBtToMouse = 0;
		LinkBmToMouse = 0;
		LinkWinX = 0;
		LinkWinY = 0;

		ScrollWithMouse = FALSE;
		PiroloGrab = 0;
	}
	//if (( LinkWinToMouse ) && (!ScrollWithMouse) && (LinkWinToMouse==T2DActualWindow(NULL)))
	if ((LinkWinToMouse) && (!ScrollWithMouse)) {
		w = &t2dWin[LinkWinToMouse];
		if (LinkBmToMouse && LinkBtToMouse) {
			b = &w->bt[LinkBtToMouse];
			d = &w->bm[LinkBmToMouse];

			d->px += (mouse_x - LinkWinX);
			d->py += (mouse_y - LinkWinY);
			if (d->px < LinkLim.x1) d->px = LinkLim.x1;
			if (d->py < LinkLim.y1) d->py = LinkLim.y1;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[LinkBmToMouse]);
			if (b->lim.x2 >= LinkLim.x2) d->px = LinkLim.x2 - (b->lim.x2 - b->lim.x1);
			if (b->lim.y2 >= LinkLim.y2) d->py = LinkLim.y2 - (b->lim.y2 - b->lim.y1);
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[LinkBmToMouse]);
		} else {
			w->px += (mouse_x - LinkWinX);
			w->py += (mouse_y - LinkWinY);
		}
		LinkWinX = mouse_x;
		LinkWinY = mouse_y;
	}

	//Gestione dello scrolling dei documenti tramite mouse
	if (ScrollWithMouse) {
		if (T2DActualWindow(nullptr) == T2D_WIN_COMPUTER_DOCUMENT) {
			w = &t2dWin[T2D_WIN_COMPUTER_DOCUMENT];
			a = computerCurrentDocument;
			StartY = renderer.rInvFitY(66);
			DimY = 407;
		} else if (T2DActualWindow(nullptr) == T2D_WIN_COMPUTER_EMAIL_VIEWER) {
			w = &t2dWin[T2D_WIN_COMPUTER_EMAIL_VIEWER];
			a = computerCurrentEMail;
			StartY = renderer.rInvFitY(59);
			DimY = 182;
		} else if (T2DActualWindow(nullptr) == T2D_WIN_PDA_LOG) {
			w = &t2dWin[T2D_WIN_PDA_LOG];

			if (PiroloGrab != 0) {
				int NewScrollPos;
				LogSBPos = (mouse_y - w->py) - PiroloGrab;
				if (LogSBPos < T2D_PDA_LOG_SB_STARTY) LogSBPos = T2D_PDA_LOG_SB_STARTY;
				if (LogSBPos > T2D_PDA_LOG_SB_STARTY + T2D_PDA_LOG_SB_TOTY) LogSBPos = T2D_PDA_LOG_SB_STARTY + T2D_PDA_LOG_SB_TOTY;
				NewScrollPos = (int)((double)(LogSBPos - T2D_PDA_LOG_SB_STARTY) / (double)LogSBDy);
				while (NewScrollPos != LogCurAbsLine) {
					if (NewScrollPos < LogCurAbsLine) RefreshLog(game, -1);
					if (NewScrollPos > LogCurAbsLine) RefreshLog(game, +1);
				}
				w->bm[T2D_BM_PDA_LOG_SB].py = LogSBPos;
				w->bt[T2D_BT_PDA_LOG_SB].lim.y1 = LogSBPos;
				w->bt[T2D_BT_PDA_LOG_SB].lim.y2 = LogSBPos + T2D_PDA_LOG_SB_PIRGY;
			}
			/*else if (-(mouse_y-ScrollWithMouseY)/15!=0)
			{
			    RefreshLog(-(mouse_y-ScrollWithMouseY)/15);
			    ScrollWithMouseX = mouse_x;
			    ScrollWithMouseY = mouse_y;
			}*/
		}

		if ((T2DActualWindow(nullptr) == T2D_WIN_COMPUTER_DOCUMENT) ||
		        (T2DActualWindow(nullptr) == T2D_WIN_COMPUTER_EMAIL_VIEWER)) {
			GetDDBitmapExtends(renderer, &t, &w->bm[a]);

			if (t.y2 - t.y1 > DimY) {
				if (w->bm[a].oy - renderer.rInvFitY(mouse_y - ScrollWithMouseY) < 0) {
					w->bm[a].oy = 0;
					w->bm[a].py = StartY;
				} else if (w->bm[a].oy + renderer.rInvFitY(DimY) -
				           renderer.rInvFitY(mouse_y - ScrollWithMouseY) >= renderer.rInvFitY(t.y2 - t.y1) - 1) {
					w->bm[a].oy = renderer.rInvFitY(t.y2 - t.y1) - w->bm[a].dy;
					while (w->bm[a].oy + w->bm[a].dy >= renderer.rInvFitY(t.y2 - t.y1)) w->bm[a].oy--;
					w->bm[a].py = StartY - w->bm[a].oy;
				} else {
					w->bm[a].oy -= (mouse_y - ScrollWithMouseY);
					w->bm[a].py += (mouse_y - ScrollWithMouseY);
				}
			}

			ScrollWithMouseX = mouse_x;
			ScrollWithMouseY = mouse_y;
		}
	}

	switch (TheMessage->event) {
	case ME_MRIGHT:
	case ME_MLEFT:
	case ME_MOUSEUPDATE:
//			Prima controllo se rimango sulla stessa finestra
		for (a = 0; a < T2D_MAX_WINDOWS; a++) {
			w = &t2dWin[WinActive[a]];
			if (CurButtonWin && (WinActive[a] == CurButtonWin) && (b = &w->bt[CurButton]) && (b->on)) {
				memcpy(&t, &b->lim, sizeof(t));
				t.x1 += w->px;
				t.y1 += w->py;
				t.x2 += w->px;
				t.y2 += w->py;
				if (CheckRect2(t, mouse_x, mouse_y))
					break;
			}
			if ((LinkWinToMouse) && (bLPressed || bRPressed))
				break;
		}
//			Se non rimango sulla stessa finestra
		if ((a >= T2D_MAX_WINDOWS) || (!bLPressed && !bRPressed)) {
			CurButton = 0;
			CurButtonWin = 0;
			ExtraLSId = 0;
			for (a = T2D_MAX_WINDOWS - 1; a >= 0; a--) {
				if (!WinActive[a] || CurButtonWin) continue;

				w = &t2dWin[WinActive[a]];
				for (i = T2D_MAX_BUTTONS_IN_WIN - 1; i >= 0; i--) {
					b = &w->bt[i];
					if (!b->on || CurButtonWin) continue;

					memcpy(&t, &b->lim, sizeof(t));
					if (w->NOTSTRETCHED == TRUE) {
						t.x2 = t.x1 + renderer.rInvFitX(t.x2 - t.x1);
						t.y2 = t.y1 + renderer.rInvFitY(t.y2 - t.y1);
					}
					t.x1 += w->px;
					t.y1 += w->py;
					t.x2 += w->px;
					t.y2 += w->py;
					if (CheckRect2(t, mouse_x, mouse_y)) {
						CurButton = i;
						CurButtonWin = WinActive[a];
						if (b->tnum > 0)
							ExtraLSId = b->tnum;
						break;
					}
				}
			}
		}

		//Gestione posizione Z finestre
		if ((TheMessage->event != ME_MOUSEUPDATE) && (bT2DActive != tPDA)) {
			for (a = T2D_MAX_WINDOWS - 1; a >= 0; a--) {
				if (WinActive[a] == 0) continue;

				if (CurButtonWin != WinActive[a]) {
					for (i = 1; i < a; i++)
						if (WinActive[i] == CurButtonWin) break;
					if (i == a) break;
					for (; i < a; i++) {
						int32 temp = WinActive[i];
						WinActive[i] = WinActive[i + 1];
						WinActive[i + 1] = temp;
					}
					ResetScreenBuffer();
					return;
				}
				break;
			}
		}

		CurMousePointer = MousePointerDefault;
		//if( !CurButtonWin || ( ( TheMessage->event == ME_MOUSEUPDATE ) && ( !bLPressed && !bRPressed ) ) ) return;
		//if( !CurButtonWin ) return;

		if ((CurButtonWin_Override >= 0) || (CurButton_Override >= 0)) {
			CurButtonWin = CurButtonWin_Override;
			CurButton = CurButton_Override;

			CurButtonWin_Override = -1;
			CurButton_Override = -1;
			TheMessage->event &= ~ME_MOUSEUPDATE;
		}

		switch (bT2DActive) {
		case tSCANNER: {
			if (CurButtonWin == T2D_WIN_SCANNER_BACKGROUND) {
				if ((CurButton == T2D_BT_SCANNER_BACKGROUND_EXIT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTOP, MP_DEFAULT, 0, 0, tSCANNER, nullptr, nullptr, nullptr);
				} else if ((CurButton == T2D_BT_SCANNER_BACKGROUND_SELECTOR) && (bLPressed || bRPressed)) {
					LinkWinToMouse = CurButtonWin;
					LinkBtToMouse = T2D_BT_SCANNER_BACKGROUND_SELECTOR;
					LinkBmToMouse = T2D_BM_SCANNER_BACKGROUND_SELECTOR;
					LinkWinX = mouse_x;
					LinkWinY = mouse_y;
					CurMousePointer = 0;
					GetDDBitmapExtends(renderer, &LinkLim, &w->bm[T2D_BM_SCANNER_BACKGROUND_PIC_LOW]);
				}
			} else if (CurButtonWin == T2D_WIN_SCANNER_TOOLBAR) {
				if ((CurButton == T2D_BT_SCANNER_TOOLBAR_MOVE) && (bLPressed || bRPressed)) {
					LinkWinToMouse = CurButtonWin;
					LinkWinX = mouse_x;
					LinkWinY = mouse_y;
				} else if ((CurButton == T2D_BT_SCANNER_TOOLBAR_EXIT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTOP, MP_DEFAULT, 0, 0, tSCANNER, nullptr, nullptr, nullptr);
				} else if ((CurButton == T2D_BT_SCANNER_TOOLBAR_PREVIEW_OFF) && (TheMessage->event != ME_MOUSEUPDATE)) {
					// Spegne per un attimo bottone di preview
					w->bm[T2D_BM_SCANNER_TOOLBAR_PREVIEW_OFF].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_SCANNER_TOOLBAR_PREVIEW_ON].tnum &= ~T2D_BM_OFF;
					// Aspetta 1 secondo
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_SCANNER_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					// Riaccede tasto preview
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_SCANNER_TOOLBAR,
							T2D_BM_SCANNER_TOOLBAR_PREVIEW_OFF, TRUE, nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_SCANNER_TOOLBAR,
							T2D_BM_SCANNER_TOOLBAR_PREVIEW_ON, FALSE, nullptr, nullptr, nullptr);
					// Spegne pic di preview
					t2dWin[T2D_WIN_SCANNER_BACKGROUND].bm[T2D_BM_SCANNER_BACKGROUND_PIC_LOW].tnum |= T2D_BM_OFF;
					// Spegne selzionatoredi area
					t2dWin[T2D_WIN_SCANNER_BACKGROUND].bm[T2D_BM_SCANNER_BACKGROUND_SELECTOR].tnum |= T2D_BM_OFF;
					t2dWin[T2D_WIN_SCANNER_BACKGROUND].bt[T2D_BT_SCANNER_BACKGROUND_SELECTOR].on = FALSE;
					// Spegne bottone per scan
					w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_OFF].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_FREEZE].tnum &= ~T2D_BM_OFF;
					// Accende finestra progress
					WinActive[2] = T2D_WIN_SCANNER_PROGRESSBAR;
					// Attiva gestione progress bar
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DCONTINUE, MP_WAITA, T2D_SCANNER_SCANNING_DELAY,
							T2D_SCANNER_SCANNING_STEPS, T2D_SCANNER_PREVIEWING, nullptr, nullptr, nullptr);
					// Spegne tutta la toolbar
					w->bt[T2D_BT_SCANNER_TOOLBAR_MOVE].on = FALSE;
					w->bt[T2D_BT_SCANNER_TOOLBAR_EXIT].on = FALSE;
					w->bt[T2D_BT_SCANNER_TOOLBAR_PREVIEW_OFF].on = FALSE;
					w->bt[T2D_BT_SCANNER_TOOLBAR_SELECTAREA_OFF].on = FALSE;
					w->bt[T2D_BT_SCANNER_TOOLBAR_SCAN_OFF].on = FALSE;
					//Suono
					StartSound(game, wSCANPULSANTE);
				} else if ((CurButton == T2D_BT_SCANNER_TOOLBAR_SELECTAREA_OFF) &&
				           (TheMessage->event != ME_MOUSEUPDATE)) {
					// Spegne per un attimo bottone di select area
					b->on = FALSE;
					w->bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_OFF].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_ON].tnum &= ~T2D_BM_OFF;
					// Aspetta 1 secondo
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_SCANNER_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					// Riaccede tasto di select area
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBUTTON, MP_WAITA, T2D_WIN_SCANNER_TOOLBAR,
							T2D_BT_SCANNER_TOOLBAR_SELECTAREA_OFF, TRUE, nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_SCANNER_TOOLBAR,
							T2D_BM_SCANNER_TOOLBAR_SELECTAREA_OFF, TRUE, nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_SCANNER_TOOLBAR,
							T2D_BM_SCANNER_TOOLBAR_SELECTAREA_ON, FALSE, nullptr, nullptr, nullptr);
					// Accende selezionatore di area
					t2dWin[T2D_WIN_SCANNER_BACKGROUND].bm[T2D_BM_SCANNER_BACKGROUND_SELECTOR].tnum &= ~T2D_BM_OFF;
					t2dWin[T2D_WIN_SCANNER_BACKGROUND].bt[T2D_BT_SCANNER_BACKGROUND_SELECTOR].on = TRUE;
					// Accende tasto di scansione
					w = &t2dWin[T2D_WIN_SCANNER_TOOLBAR];
					w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_FREEZE].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_OFF].tnum &= ~T2D_BM_OFF;
					w->bt[T2D_BT_SCANNER_TOOLBAR_SCAN_OFF].on = TRUE;
					//Suono
					StartSound(game, wSCANPULSANTE);
				} else if ((CurButton == T2D_BT_SCANNER_TOOLBAR_SCAN_OFF) && (TheMessage->event != ME_MOUSEUPDATE)) {
					w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_OFF].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_SCANNER_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_SCANNER_TOOLBAR,
							T2D_BM_SCANNER_TOOLBAR_SCAN_FREEZE, TRUE, nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_SCANNER_TOOLBAR,
							T2D_BM_SCANNER_TOOLBAR_SCAN_ON, FALSE, nullptr, nullptr, nullptr);

					w = &t2dWin[T2D_WIN_SCANNER_BACKGROUND];
					d = &w->bm[T2D_BM_SCANNER_BACKGROUND_SELECTOR];
					d1 = &w->bm[T2D_BM_SCANNER_BACKGROUND_PIC_LOW];
					d2 = &w->bm[T2D_BM_SCANNER_BACKGROUND_PIC_HI];

					d2->ox = ((d->px - d1->px) * renderer.getBitmapRealDimX(d2->tnum & ~T2D_BM_OFF)) /
					         renderer.getBitmapRealDimX(d1->tnum & ~T2D_BM_OFF);
					d2->oy = ((d->py - d1->py) * renderer.getBitmapRealDimY(d2->tnum & ~T2D_BM_OFF)) /
					         renderer.getBitmapRealDimY(d1->tnum & ~T2D_BM_OFF);
					d2->dx = renderer.getBitmapRealDimX(d1->tnum & ~T2D_BM_OFF);
					d2->px = d1->px - d2->ox;
					d2->py = d1->py - d2->oy;
					d1->tnum |= T2D_BM_OFF;

					WinActive[2] = T2D_WIN_SCANNER_PROGRESSBAR;
					// Disattiva il select area
					t2dWin[T2D_WIN_SCANNER_TOOLBAR].bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_OFF].tnum |= T2D_BM_OFF;
					t2dWin[T2D_WIN_SCANNER_TOOLBAR].bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_FREEZE].tnum &= ~T2D_BM_OFF;
					// Disattiva il selettore
					t2dWin[T2D_WIN_SCANNER_BACKGROUND].bm[T2D_BM_SCANNER_BACKGROUND_SELECTOR].tnum |= T2D_BM_OFF;
					t2dWin[T2D_WIN_SCANNER_BACKGROUND].bt[T2D_BT_SCANNER_BACKGROUND_SELECTOR].on = FALSE;
					// Attiva gestione progress bar
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DCONTINUE, MP_DEFAULT, T2D_SCANNER_SCANNING_DELAY,
							T2D_SCANNER_SCANNING_STEPS, T2D_SCANNER_SCANNING, nullptr, nullptr, nullptr);
					// Spegne tutta la toolbar
					w = &t2dWin[T2D_WIN_SCANNER_TOOLBAR];
					w->bt[T2D_BT_SCANNER_TOOLBAR_MOVE].on = FALSE;
					w->bt[T2D_BT_SCANNER_TOOLBAR_EXIT].on = FALSE;
					w->bt[T2D_BT_SCANNER_TOOLBAR_PREVIEW_OFF].on = FALSE;
					w->bt[T2D_BT_SCANNER_TOOLBAR_SELECTAREA_OFF].on = FALSE;
					w->bt[T2D_BT_SCANNER_TOOLBAR_SCAN_OFF].on = FALSE;

					//Controllo posizione selettore per quest
					GetDDBitmapExtends(renderer, &t, &t2dWin[T2D_WIN_SCANNER_BACKGROUND].bm[T2D_BM_SCANNER_BACKGROUND_SELECTOR]);
					t.x1 -= 108;
					t.x2 -= 108;
					t.y1 -= 118;
					t.y2 -= 118;
					if ((t.x1 < 180) && (t.y1 < 70) && (t.x2 > 280) && (t.y2 > 200))
						SelectorFlag = TRUE;
					else SelectorFlag = false;

					//Suono
					StartSound(game, wSCANPULSANTE);
				}
			}
			break;
		}
		case tCOMPUTER: {
			if (CurButtonWin == T2D_WIN_COMPUTER_BACKGROUND) {
				if ((CurButton == T2D_BT_COMPUTER_BACKGROUND_EMAIL) && (TheMessage->event != ME_MOUSEUPDATE)) {
					if (!(w->bm[T2D_BM_COMPUTER_EMAIL_ON].tnum & T2D_BM_OFF)) {
						//Pressione Tasto
						w->bm[T2D_BM_COMPUTER_EMAIL_ON].tnum |= T2D_BM_OFF;
						//Suono
						StartSound(game, wCIAPULSANTE);
						//Apre finestra EMail
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_DEFAULT, T2D_WIN_COMPUTER_EMAIL, 0, FALSE,
								nullptr, nullptr, nullptr);
						//Resetta dati
						computerCurrentEMail = -1;
					} else {
						//Pressione Tasto
						w->bm[T2D_BM_COMPUTER_EMAIL_ON].tnum &= ~T2D_BM_OFF;
						//Suono
						StartSound(game, wCIAPULSANTE);
						//Chiude finestra EMail
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_DEFAULT, T2D_WIN_COMPUTER_EMAIL, 0, TRUE, nullptr,
								nullptr, nullptr);
					}
				} else if ((CurButton == T2D_BT_COMPUTER_BACKGROUND_SEARCH) && (TheMessage->event != ME_MOUSEUPDATE)) {
					if (!(w->bm[T2D_BM_COMPUTER_SEARCH_ON].tnum & T2D_BM_OFF)) {
						//Pressione Tasto
						w->bm[T2D_BM_COMPUTER_SEARCH_ON].tnum |= T2D_BM_OFF;
						//Suono
						StartSound(game, wCIAPULSANTE);
						//Apre finestra Search
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_DEFAULT, T2D_WIN_COMPUTER_SEARCH, 0, FALSE,
								nullptr, nullptr, nullptr);

						//Riattiva DirectInput
						//ResetDIKbd();
						//bIngnoreDIKeyboard = FALSE;

						//Resetta Dati
						computerCurrentDocument = -1;
						AlphaOrCase = 0;
						WhatText = 0;
						searchName.clear();
						searchSurName.clear();
						searchCaseName.clear();
						searchYear.clear();
						w->text[T2D_TEXT_COMPUTER_SEARCH_NAME].text[0] = 0;
						w->text[T2D_TEXT_COMPUTER_SEARCH_SURNAME].text[0] = 0;
						w->text[T2D_TEXT_COMPUTER_SEARCH_CASENAME].text[0] = 0;
						w->text[T2D_TEXT_COMPUTER_SEARCH_YEAR].text[0] = 0;
					} else {
						//Pressione Tasto
						w->bm[T2D_BM_COMPUTER_SEARCH_ON].tnum &= ~T2D_BM_OFF;
						//Suono
						StartSound(game, wCIAPULSANTE);
						//Chiude finestra Search
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_DEFAULT, T2D_WIN_COMPUTER_SEARCH, 0, TRUE,
								nullptr, nullptr, nullptr);
					}
				} else if ((CurButton == T2D_BT_COMPUTER_BACKGROUND_QUIT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_QUIT_ON].tnum &= ~T2D_BM_OFF;
					//Suono
					StartSound(game, wCIAPULSANTE);
					//Quitta
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_COMPUTER_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_BACKGROUND,
							T2D_BM_COMPUTER_QUIT_ON, FALSE, nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTOP, MP_WAITA, 0, 0, tCOMPUTER, nullptr, nullptr, nullptr);
				}

			} else if (CurButtonWin == T2D_WIN_COMPUTER_ERROR) {
				if ((CurButton == T2D_BT_COMPUTER_ERROR_MOVE) && (bLPressed || bRPressed)) {
					LinkWinToMouse = CurButtonWin;
					LinkWinX = mouse_x;
					LinkWinY = mouse_y;
				} else if ((CurButton == T2D_BT_COMPUTER_ERROR_DIALUP) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_ERROR_DIALUP_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_COMPUTER_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_ERROR,
							T2D_BM_COMPUTER_ERROR_DIALUP_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
					//Disattiva finestra errore
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_ERROR, 0, FALSE, nullptr,
							nullptr, nullptr);
					//Attiva finestra DialUp
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_DIALUP, 0, TRUE, nullptr,
							nullptr, nullptr);
					//Disattiva DirectInput
					//bIngnoreDIKeyboard = TRUE;
					//Azzera Numero Telefonico
					strcpy(computerNumeroTel, "");
					strcpy(t2dWin[T2D_WIN_COMPUTER_DIALUP].text[T2D_TEXT_COMPUTER_DIALUP_NUMBER].text, "");

					//Disattiva bitmap e pulsanti finestra errore
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_ERROR,
							T2D_BM_COMPUTER_DIALERROR_WIN, FALSE, nullptr, nullptr, nullptr);
					t2dWin[T2D_WIN_COMPUTER_ERROR].bt[T2D_BT_COMPUTER_ERROR_DIALUP].on = FALSE;
					t2dWin[T2D_WIN_COMPUTER_ERROR].bt[T2D_BT_COMPUTER_ERROR_DIALQUIT].on = FALSE;
					t2dWin[T2D_WIN_COMPUTER_ERROR].bt[T2D_BT_DialError_Win_TGA].on = FALSE;
					t2dWin[T2D_WIN_COMPUTER_ERROR].sy = 0;
				} else if ((CurButton == T2D_BT_COMPUTER_ERROR_DIALQUIT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_ERROR_DIALQUIT_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_COMPUTER_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_ERROR,
							T2D_BM_COMPUTER_ERROR_DIALQUIT_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
					//Spenge il computer
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTOP, MP_WAITA, 0, 0, tCOMPUTER, nullptr, nullptr, nullptr);
				} else if ((CurButton == T2D_BT_COMPUTER_ERROR_COMQUIT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_ERROR_COMQUIT_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_COMPUTER_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_ERROR,
							T2D_BM_COMPUTER_ERROR_COMQUIT_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
					//Spenge il computer
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTOP, MP_WAITA, 0, 0, tCOMPUTER, nullptr, nullptr, nullptr);
				} else if ((CurButton == T2D_BT_COMPUTER_ERROR_OK) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_ERROR_OK_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_COMPUTER_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_ERROR,
							T2D_BM_COMPUTER_ERROR_OK_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
					//Distattiva finestra errore
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_ERROR, 0, FALSE, nullptr,
							nullptr, nullptr);

					//Distattiva bitmap e tasti finestra errore
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_ERROR,
							T2D_BM_COMPUTER_SEARCHERROR_WIN, FALSE, nullptr, nullptr, nullptr);
					t2dWin[T2D_WIN_COMPUTER_ERROR].bt[T2D_BT_COMPUTER_ERROR_OK].on = FALSE;
					t2dWin[T2D_WIN_COMPUTER_ERROR].bt[T2D_BT_SearchError_Win_TGA].on = FALSE;
					t2dWin[T2D_WIN_COMPUTER_ERROR].sy = 3;
				}
			} else if (CurButtonWin == T2D_WIN_COMPUTER_DIALUP) {
				if ((CurButton == T2D_BT_COMPUTER_DIALUP_MOVE) && (bLPressed || bRPressed)) {
					LinkWinToMouse = CurButtonWin;
					LinkWinX = mouse_x;
					LinkWinY = mouse_y;
				} else if ((CurButton == T2D_BT_COMPUTER_DIALUP_1) && (TheMessage->event != ME_MOUSEUPDATE)) {
					T2DAddKeyInput('1');
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_DIALUP_1_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DIALUP,
							T2D_BM_COMPUTER_DIALUP_1_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
				} else if ((CurButton == T2D_BT_COMPUTER_DIALUP_2) && (TheMessage->event != ME_MOUSEUPDATE)) {
					T2DAddKeyInput('2');
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_DIALUP_2_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DIALUP,
							T2D_BM_COMPUTER_DIALUP_2_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
				} else if ((CurButton == T2D_BT_COMPUTER_DIALUP_3) && (TheMessage->event != ME_MOUSEUPDATE)) {
					T2DAddKeyInput('3');
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_DIALUP_3_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DIALUP,
							T2D_BM_COMPUTER_DIALUP_3_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
				} else if ((CurButton == T2D_BT_COMPUTER_DIALUP_4) && (TheMessage->event != ME_MOUSEUPDATE)) {
					T2DAddKeyInput('4');
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_DIALUP_4_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DIALUP,
							T2D_BM_COMPUTER_DIALUP_4_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
				} else if ((CurButton == T2D_BT_COMPUTER_DIALUP_5) && (TheMessage->event != ME_MOUSEUPDATE)) {
					T2DAddKeyInput('5');
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_DIALUP_5_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DIALUP,
							T2D_BM_COMPUTER_DIALUP_5_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
				} else if ((CurButton == T2D_BT_COMPUTER_DIALUP_6) && (TheMessage->event != ME_MOUSEUPDATE)) {
					T2DAddKeyInput('6');
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_DIALUP_6_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DIALUP,
							T2D_BM_COMPUTER_DIALUP_6_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
				} else if ((CurButton == T2D_BT_COMPUTER_DIALUP_7) && (TheMessage->event != ME_MOUSEUPDATE)) {
					T2DAddKeyInput('7');
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_DIALUP_7_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DIALUP,
							T2D_BM_COMPUTER_DIALUP_7_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
				} else if ((CurButton == T2D_BT_COMPUTER_DIALUP_8) && (TheMessage->event != ME_MOUSEUPDATE)) {
					T2DAddKeyInput('8');
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_DIALUP_8_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DIALUP,
							T2D_BM_COMPUTER_DIALUP_8_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
				} else if ((CurButton == T2D_BT_COMPUTER_DIALUP_9) && (TheMessage->event != ME_MOUSEUPDATE)) {
					T2DAddKeyInput('9');
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_DIALUP_9_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DIALUP,
							T2D_BM_COMPUTER_DIALUP_9_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
				} else if ((CurButton == T2D_BT_COMPUTER_DIALUP_0) && (TheMessage->event != ME_MOUSEUPDATE)) {
					T2DAddKeyInput('0');
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_DIALUP_0_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DIALUP,
							T2D_BM_COMPUTER_DIALUP_0_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
				} else if ((CurButton == T2D_BT_COMPUTER_DIALUP_ENTER) && (TheMessage->event != ME_MOUSEUPDATE)) {
					T2DAddKeyInput(VK_DELETE);
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_DIALUP_ENTER_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DIALUP,
							T2D_BM_COMPUTER_DIALUP_ENTER_ON, FALSE, nullptr, nullptr, nullptr);
				} else if ((CurButton == T2D_BT_COMPUTER_DIALUP_C) && (TheMessage->event != ME_MOUSEUPDATE)) {
					T2DAddKeyInput(VK_BACK);
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_DIALUP_C_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DIALUP,
							T2D_BM_COMPUTER_DIALUP_C_ON, FALSE, nullptr, nullptr, nullptr);
				} else if ((CurButton == T2D_BT_COMPUTER_DIALUP_OK) && (TheMessage->event != ME_MOUSEUPDATE)) {
					T2DAddKeyInput(VK_RETURN);
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_DIALUP_OK_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_COMPUTER_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DIALUP,
							T2D_BM_COMPUTER_DIALUP_OK_ON, FALSE, nullptr, nullptr, nullptr);
				} else if ((CurButton == T2D_BT_COMPUTER_DIALUP_EXIT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_DIALUP_EXIT_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_COMPUTER_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTOP, MP_WAITA, 0, 0, tCOMPUTER, nullptr, nullptr, nullptr);
				}
			} else if (CurButtonWin == T2D_WIN_COMPUTER_SEARCH) {
				if ((CurButton == T2D_BT_COMPUTER_SEARCH_MOVE) && (bLPressed || bRPressed)) {
					LinkWinToMouse = CurButtonWin;
					LinkWinX = mouse_x;
					LinkWinY = mouse_y;
				} else if ((CurButton == T2D_BT_COMPUTER_SEARCH_SEARCH) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione Tasto
					w->bm[T2D_BM_COMPUTER_SEARCH_SEARCH_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_COMPUTER_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_SEARCH,
							T2D_BM_COMPUTER_SEARCH_SEARCH_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);

					ComputerSearch(game);
				} else if ((CurButton == T2D_BT_COMPUTER_SEARCH_ALPHABETICAL) &&
				           (TheMessage->event != ME_MOUSEUPDATE)) {
					//Suono
					StartSound(game, wCIAPULSANTE);
					//Setta parametri della finestra Search per visualizzare la parte Alphabetical
					w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_WIN].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_ALPHABETICAL_WIN].tnum &= ~T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_YEAR].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_NAME].tnum &= ~T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME].tnum &= ~T2D_BM_OFF;
					w->bt[T2D_BT_COMPUTER_SEARCH_NAME_TEXT].on = TRUE;
					w->bt[T2D_BT_COMPUTER_SEARCH_SURNAME_TEXT].on = TRUE;
					w->bt[T2D_BT_COMPUTER_SEARCH_CASENAME_TEXT].on = FALSE;
					w->bt[T2D_BT_COMPUTER_SEARCH_YEAR_TEXT].on = FALSE;
					w->bm[T2D_BM_COMPUTER_SEARCH_NAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_YEAR_ON].tnum |= T2D_BM_OFF;
					w->bt[T2D_BT_Search_Alphabetical_TGA__name].on = TRUE;
					w->bt[T2D_BT_Search_Alphabetical_TGA__surname].on = TRUE;
					w->bt[T2D_BT_Search_Alphabetical_TGA__casename].on = FALSE;
					w->bt[T2D_BT_Search_Alphabetical_TGA__year].on = FALSE;
					WhatText = 0;
					AlphaOrCase = 1;
					ActivateCursor(FALSE, NULL);
				} else if ((CurButton == T2D_BT_COMPUTER_SEARCH_CASENAME) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Suono
					StartSound(game, wCIAPULSANTE);
					//Setta parametri della finestra Search per visualizzare la parte Casename
					w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_WIN].tnum &= ~T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_ALPHABETICAL_WIN].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME].tnum &= ~T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_YEAR].tnum &= ~T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_NAME].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME].tnum |= T2D_BM_OFF;
					w->bt[T2D_BT_COMPUTER_SEARCH_NAME_TEXT].on = FALSE;
					w->bt[T2D_BT_COMPUTER_SEARCH_SURNAME_TEXT].on = FALSE;
					w->bt[T2D_BT_COMPUTER_SEARCH_CASENAME_TEXT].on = TRUE;
					w->bt[T2D_BT_COMPUTER_SEARCH_YEAR_TEXT].on = TRUE;
					w->bm[T2D_BM_COMPUTER_SEARCH_NAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_YEAR_ON].tnum |= T2D_BM_OFF;
					w->bt[T2D_BT_Search_Alphabetical_TGA__name].on = FALSE;
					w->bt[T2D_BT_Search_Alphabetical_TGA__surname].on = FALSE;
					w->bt[T2D_BT_Search_Alphabetical_TGA__casename].on = TRUE;
					w->bt[T2D_BT_Search_Alphabetical_TGA__year].on = TRUE;
					WhatText = 0;
					AlphaOrCase = 0;
					ActivateCursor(FALSE, NULL);
				} else if ((CurButton == T2D_BT_COMPUTER_SEARCH_NAME_TEXT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pone il cursore sulla casella testo Name
					ActivateCursor(FALSE, NULL);
					WhatText = T2D_TEXT_COMPUTER_SEARCH_NAME;
					w->bm[T2D_BM_COMPUTER_SEARCH_NAME_ON].tnum &= ~T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_YEAR_ON].tnum |= T2D_BM_OFF;
					ActivateCursor(TRUE, w->text[WhatText].text);
					//bIngnoreDIKeyboard = TRUE;

					strcpy(w->text[WhatText].text, "");
					searchName = "";
				} else if ((CurButton == T2D_BT_COMPUTER_SEARCH_SURNAME_TEXT) &&
				           (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pone il cursore sulla casella testo Surname
					ActivateCursor(FALSE, NULL);
					WhatText = T2D_TEXT_COMPUTER_SEARCH_SURNAME;
					w->bm[T2D_BM_COMPUTER_SEARCH_NAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME_ON].tnum &= ~T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_YEAR_ON].tnum |= T2D_BM_OFF;
					ActivateCursor(TRUE, w->text[WhatText].text);
					////bIngnoreDIKeyboard = TRUE;

					strcpy(w->text[WhatText].text, "");
					searchSurName = "";
				} else if ((CurButton == T2D_BT_COMPUTER_SEARCH_CASENAME_TEXT) &&
				           (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pone il cursore sulla casella testo Casename
					ActivateCursor(FALSE, NULL);
					WhatText = T2D_TEXT_COMPUTER_SEARCH_CASENAME;
					w->bm[T2D_BM_COMPUTER_SEARCH_NAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_ON].tnum &= ~T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_YEAR_ON].tnum |= T2D_BM_OFF;
					ActivateCursor(TRUE, w->text[WhatText].text);
					//bIngnoreDIKeyboard = TRUE;

					strcpy(w->text[WhatText].text, "");
					searchCaseName = "";
				} else if ((CurButton == T2D_BT_COMPUTER_SEARCH_YEAR_TEXT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pone il cursore sulla casella testo Year
					ActivateCursor(FALSE, NULL);
					WhatText = T2D_TEXT_COMPUTER_SEARCH_YEAR;
					w->bm[T2D_BM_COMPUTER_SEARCH_NAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_ON].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_COMPUTER_SEARCH_YEAR_ON].tnum &= ~T2D_BM_OFF;
					ActivateCursor(TRUE, w->text[WhatText].text);
					//bIngnoreDIKeyboard = TRUE;

					strcpy(w->text[WhatText].text, "");
					searchYear = "";
				} else if ((CurButton == T2D_BT_COMPUTER_SEARCH_EXIT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione tasto
					w->bm[T2D_BM_COMPUTER_SEARCH_EXIT_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_COMPUTER_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_SEARCH,
							T2D_BM_COMPUTER_SEARCH_EXIT_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
					//Elimina finestra search
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_SEARCH, 0, FALSE, nullptr,
							nullptr, nullptr);

					//Disattiva tasto search
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_BACKGROUND,
							T2D_BM_COMPUTER_SEARCH_ON, FALSE, nullptr, nullptr, nullptr);
				}
			} else if (CurButtonWin == T2D_WIN_COMPUTER_DOCLIST) {
				if ((CurButton == T2D_BT_COMPUTER_DOCLIST_MOVE) && (bLPressed || bRPressed)) {
					LinkWinToMouse = CurButtonWin;
					LinkWinX = mouse_x;
					LinkWinY = mouse_y;
				} else if ((CurButton == T2D_BT_COMPUTER_DOCLIST_SCROLLUP) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione tasto
					w->bm[T2D_BM_COMPUTER_DOCLIST_SCROLLUP_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DOCLIST,
							T2D_BM_COMPUTER_DOCLIST_SCROLLUP_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
				} else if ((CurButton == T2D_BT_COMPUTER_DOCLIST_SCROLLDOWN) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione tasto
					w->bm[T2D_BM_COMPUTER_DOCLIST_SCROLLDOWN_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DOCLIST,
							T2D_BM_COMPUTER_DOCLIST_SCROLLDOWN_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
				} else if (((CurButton >= T2D_BT_COMPUTER_DOCLIST_LINES_START) &&
				            (CurButton <= T2D_BT_COMPUTER_DOCLIST_LINES_END)) &&
				           (TheMessage->event != ME_MOUSEUPDATE)) {
					StartY = computerCurrentDocument;
					computerCurrentDocument = -1;

					//Controlla se la linea premuta contiene il collegamento a un documento
					for (i = 0; i < T2D_BT_COMPUTER_DOCLIST_LINES_END - T2D_BT_COMPUTER_DOCLIST_LINES_START; i++)
						if (computerDocList[i] == -1) break;
					if (CurButton - T2D_BT_COMPUTER_DOCLIST_LINES_START >= i) break;

					computerCurrentDocument = computerDocList[CurButton - T2D_BT_COMPUTER_DOCLIST_LINES_START];

					if (computerCurrentDocument != -1) {
						//Disattiva vecchi documento
						if (StartY >= 0) t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[StartY].tnum |= T2D_BM_OFF;

						//Attiva nuovo documento
						t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].tnum &= ~T2D_BM_OFF;
						if (!t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].tnum)
							t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].tnum = LoadDDBitmap(game,
																											  ComputerCIAFileNames[
																													  computerCurrentDocument -
																													  T2D_BM_COMPUTER_DOCUMENT_FIRST],
																											  0);

						//Setta parametri nuovo documento
						GetDDBitmapExtends(renderer, &t, &t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument]);
						t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].px = renderer.rInvFitX(26);
						t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].py = renderer.rInvFitY(66);
						t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].ox = 0;
						t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].oy = 0;
						t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].dx = 0;
						t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].dy = 0;
						if (t.y2 - t.y1 > 407) {
							t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].dy = renderer.rInvFitY(407);
						} else {
							t2dWin[T2D_WIN_COMPUTER_DOCUMENT].bm[computerCurrentDocument].dy = renderer.rInvFitY(
									t.y2 - t.y1);
						}

						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_DEFAULT, T2D_WIN_COMPUTER_DOCUMENT, 0, TRUE,
								nullptr, nullptr, nullptr);
					}
				} else if ((CurButton == T2D_BT_COMPUTER_DOCLIST_EXIT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione tasto
					w->bm[T2D_BM_COMPUTER_DOCLIST_EXIT_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_COMPUTER_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DOCLIST,
							T2D_BM_COMPUTER_DOCLIST_EXIT_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
					//Disattiva finestra doclist
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_DOCLIST, 0, FALSE, nullptr,
							nullptr, nullptr);
				}
			} else if (CurButtonWin == T2D_WIN_COMPUTER_EMAIL) {
				if ((CurButton == T2D_BT_COMPUTER_EMAIL_MOVE) && (bLPressed || bRPressed)) {
					LinkWinToMouse = CurButtonWin;
					LinkWinX = mouse_x;
					LinkWinY = mouse_y;
				} else if ((CurButton == T2D_BT_COMPUTER_EMAIL_SCROLLUP) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Scroll UP
					w->sy -= 10;
					if (w->sy < 0) w->sy = 0;
				} else if ((CurButton == T2D_BT_COMPUTER_EMAIL_SCROLLDOWN) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Scroll Down
					w->sy += 10;
				} else if (
				    ((CurButton == T2D_BT_COMPUTER_EMAIL_ICON_1) || (CurButton == T2D_BT_COMPUTER_EMAIL_ICON_2)) &&
				    (TheMessage->event != ME_MOUSEUPDATE)) {
					//Disattiva vecchia email
					if (computerCurrentEMail != -1)
						t2dWin[T2D_WIN_COMPUTER_EMAIL_VIEWER].bm[computerCurrentEMail].tnum |= T2D_BM_OFF;

					if (CurButton == T2D_BT_COMPUTER_EMAIL_ICON_1)
						computerCurrentEMail = T2D_BM_COMPUTER_EMAIL_1;
					else if (CurButton == T2D_BT_COMPUTER_EMAIL_ICON_2)
						computerCurrentEMail = T2D_BM_COMPUTER_EMAIL_2;

					//Attiva nuova email
					t2dWin[T2D_WIN_COMPUTER_EMAIL_VIEWER].bm[computerCurrentEMail].tnum &= ~T2D_BM_OFF;

					//Setta parametri
					GetDDBitmapExtends(renderer, &t, &t2dWin[T2D_WIN_COMPUTER_EMAIL_VIEWER].bm[computerCurrentEMail]);
					t2dWin[T2D_WIN_COMPUTER_EMAIL_VIEWER].bm[computerCurrentEMail].px = renderer.rInvFitX(35);
					t2dWin[T2D_WIN_COMPUTER_EMAIL_VIEWER].bm[computerCurrentEMail].py = renderer.rInvFitY(59);
					t2dWin[T2D_WIN_COMPUTER_EMAIL_VIEWER].bm[computerCurrentEMail].ox = 0;
					t2dWin[T2D_WIN_COMPUTER_EMAIL_VIEWER].bm[computerCurrentEMail].oy = 0;
					t2dWin[T2D_WIN_COMPUTER_EMAIL_VIEWER].bm[computerCurrentEMail].dx = 0;
					t2dWin[T2D_WIN_COMPUTER_EMAIL_VIEWER].bm[computerCurrentEMail].dy = 0;
					if (t.y2 - t.y1 > 182)
						t2dWin[T2D_WIN_COMPUTER_EMAIL_VIEWER].bm[computerCurrentEMail].dy = renderer.rInvFitY(182);
					//else  t2dWin[T2D_WIN_COMPUTER_EMAIL_VIEWER].bm[computerCurrentEMail].dy = rInvFitY(t.y2-t.y1);
					else t2dWin[T2D_WIN_COMPUTER_EMAIL_VIEWER].bm[computerCurrentEMail].dy = 0;

					//Apre finestra EMail_Viewer
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_DEFAULT, T2D_WIN_COMPUTER_EMAIL_VIEWER, 0, TRUE,
							nullptr, nullptr, nullptr);
				} else if ((CurButton == T2D_BT_COMPUTER_EMAIL_EXIT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione tasto
					w->bm[T2D_BM_COMPUTER_EMAIL_EXIT_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_COMPUTER_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_EMAIL,
							T2D_BM_COMPUTER_EMAIL_EXIT_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
					//Disattiva finestra email
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_EMAIL, 0, FALSE, nullptr,
							nullptr, nullptr);

					//Disattiva tasto email
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_BACKGROUND,
							T2D_BM_COMPUTER_EMAIL_ON, FALSE, nullptr, nullptr, nullptr);

					//ResetDIKbd();
					//bIngnoreDIKeyboard = FALSE;
				}
			} else if (CurButtonWin == T2D_WIN_COMPUTER_DOCUMENT) {
				if ((CurButton == T2D_BT_COMPUTER_DOCUMENT_MOVE) && (bLPressed || bRPressed)) {
					LinkWinToMouse = CurButtonWin;
					LinkWinX = mouse_x;
					LinkWinY = mouse_y;
				} else if ((CurButton == T2D_BT_COMPUTER_DOCUMENT_SCROLL) && (bLPressed || bRPressed)) {
					//Paramentri per lo scrolling
					ScrollWithMouse = CurButtonWin;
					ScrollWithMouseX = mouse_x;
					ScrollWithMouseY = mouse_y;
				} else if ((CurButton == T2D_BT_COMPUTER_DOCUMENT_SCROLLUP) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione tasto
					w->bm[T2D_BM_COMPUTER_DOCUMENT_SCROLLUP_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DOCUMENT,
							T2D_BM_COMPUTER_DOCUMENT_SCROLLUP_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);

					GetDDBitmapExtends(renderer, &t, &w->bm[computerCurrentDocument]);

					if (t.y2 - t.y1 > 407) {
						if (w->bm[computerCurrentDocument].oy - T2D_DOC_SCROLL_ADD < 0) {
							w->bm[computerCurrentDocument].oy = 0;
							w->bm[computerCurrentDocument].py = 66;
						} else {
							w->bm[computerCurrentDocument].oy -= T2D_DOC_SCROLL_ADD;
							w->bm[computerCurrentDocument].py += T2D_DOC_SCROLL_ADD;
						}
					}
				} else if ((CurButton == T2D_BT_COMPUTER_DOCUMENT_SCROLLDOWN) &&
				           (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione tasto
					w->bm[T2D_BM_COMPUTER_DOCUMENT_SCROLLDOWN_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DOCUMENT,
							T2D_BM_COMPUTER_DOCUMENT_SCROLLDOWN_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);

					GetDDBitmapExtends(renderer, &t, &w->bm[computerCurrentDocument]);

					if (t.y2 - t.y1 > 407) {
						if (w->bm[computerCurrentDocument].oy + w->bm[computerCurrentDocument].dy +
						        T2D_DOC_SCROLL_ADD >= renderer.rInvFitY(t.y2 - t.y1)) {
							w->bm[computerCurrentDocument].oy =
							    renderer.rInvFitY(t.y2 - t.y1) - w->bm[computerCurrentDocument].dy;
							while (w->bm[computerCurrentDocument].oy + w->bm[computerCurrentDocument].dy >=
							        renderer.rInvFitY(t.y2 - t.y1))
								w->bm[computerCurrentDocument].oy--;
							w->bm[computerCurrentDocument].py =
							    renderer.rInvFitY(66) - w->bm[computerCurrentDocument].oy;
						} else {
							w->bm[computerCurrentDocument].oy += T2D_DOC_SCROLL_ADD;
							w->bm[computerCurrentDocument].py -= T2D_DOC_SCROLL_ADD;
						}
					}

				} else if ((CurButton == T2D_BT_COMPUTER_DOCUMENT_EXIT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione tasto
					w->bm[T2D_BM_COMPUTER_DOCUMENT_EXIT_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_COMPUTER_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DOCUMENT,
							T2D_BM_COMPUTER_DOCUMENT_EXIT_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
					//Disattiva documento
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_DOCUMENT,
							(short)computerCurrentDocument, FALSE, nullptr, nullptr, nullptr);
					//Disattiva finestra Document
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_DOCUMENT, 0, FALSE, nullptr,
							nullptr, nullptr);

					computerCurrentDocument = -1;
				}
			} else if (CurButtonWin == T2D_WIN_COMPUTER_EMAIL_VIEWER) {
				if ((CurButton == T2D_BT_COMPUTER_EMAIL_VIEWER_MOVE) && (bLPressed || bRPressed)) {
					LinkWinToMouse = CurButtonWin;
					LinkWinX = mouse_x;
					LinkWinY = mouse_y;
				} else if ((CurButton == T2D_BT_COMPUTER_EMAIL_VIEWER_SCROLL) && (bLPressed || bRPressed)) {
					//Parametri scrolling
					ScrollWithMouse = CurButtonWin;
					ScrollWithMouseX = mouse_x;
					ScrollWithMouseY = mouse_y;
				} else if ((CurButton == T2D_BT_COMPUTER_EMAIL_VIEWER_SCROLLUP) &&
				           (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione tasto
					w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_SCROLLUP_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_EMAIL_VIEWER,
							T2D_BM_COMPUTER_EMAIL_VIEWER_SCROLLUP_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);

					GetDDBitmapExtends(renderer, &t, &w->bm[computerCurrentEMail]);

					if (t.y2 - t.y1 > 182) {
						if (w->bm[computerCurrentEMail].oy - T2D_DOC_SCROLL_ADD < 0) {
							w->bm[computerCurrentEMail].oy = 0;
							w->bm[computerCurrentEMail].py = renderer.rInvFitY(59);
						} else {
							w->bm[computerCurrentEMail].oy -= T2D_DOC_SCROLL_ADD;
							w->bm[computerCurrentEMail].py += T2D_DOC_SCROLL_ADD;
						}
					}
				} else if ((CurButton == T2D_BT_COMPUTER_EMAIL_VIEWER_SCROLLDOWN) &&
				           (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione tasto
					w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_SCROLLDOWN_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_EMAIL_VIEWER,
							T2D_BM_COMPUTER_EMAIL_VIEWER_SCROLLDOWN_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);

					GetDDBitmapExtends(renderer, &t, &w->bm[computerCurrentEMail]);

					if (t.y2 - t.y1 > 182) {
						if (w->bm[computerCurrentEMail].oy + w->bm[computerCurrentEMail].dy +
						        T2D_DOC_SCROLL_ADD >= renderer.rInvFitY(t.y2 - t.y1)) {
							w->bm[computerCurrentEMail].oy =
							    renderer.rInvFitY(t.y2 - t.y1) - w->bm[computerCurrentEMail].dy;
							while (w->bm[computerCurrentEMail].oy + w->bm[computerCurrentEMail].dy >=
							        renderer.rInvFitY(t.y2 - t.y1))
								w->bm[computerCurrentEMail].oy--;
							w->bm[computerCurrentEMail].py = renderer.rInvFitY(59) - w->bm[computerCurrentEMail].oy;
						} else {
							w->bm[computerCurrentEMail].oy += T2D_DOC_SCROLL_ADD;
							w->bm[computerCurrentEMail].py -= T2D_DOC_SCROLL_ADD;
						}
					}

				} else if ((CurButton == T2D_BT_COMPUTER_EMAIL_VIEWER_EXIT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione tasto
					w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_EXIT_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_COMPUTER_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_EMAIL_VIEWER,
							T2D_BM_COMPUTER_EMAIL_VIEWER_EXIT_ON, FALSE, nullptr, nullptr, nullptr);
					//Suono
					StartSound(game, wCIAPULSANTE);
					//Disattiva finestra Email_Viewer
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_EMAIL_VIEWER, 0, FALSE,
							nullptr, nullptr, nullptr);
				}
			}
			break;
		}
		case tDIARIO: {
			if (CurButtonWin == T2D_WIN_DIARIO) {
				//Azzera parti selezionate
				if (TheMessage->event == ME_MOUSEUPDATE) {
					switch (diarioCurrentPage) {
					case 1:
						for (i = T2D_BT_DIARIO_PAGE_1_TEXT_1; i <= T2D_BT_DIARIO_PAGE_1_TEXT_6; i++) {
							if (!(w->bm[i].tnum & T2D_BM_OFF))
								w->bm[i].tnum |= T2D_BM_OFF;
						}
						break;
					case 2:
						for (i = T2D_BT_DIARIO_PAGE_2_TEXT_1; i <= T2D_BT_DIARIO_PAGE_2_TEXT_6; i++) {
							if (!(w->bm[i].tnum & T2D_BM_OFF))
								w->bm[i].tnum |= T2D_BM_OFF;
						}
						break;
					case 3:
						for (i = T2D_BT_DIARIO_PAGE_3_TEXT_1; i <= T2D_BT_DIARIO_PAGE_3_TEXT_4; i++) {
							if (!(w->bm[i].tnum & T2D_BM_OFF))
								w->bm[i].tnum |= T2D_BM_OFF;
						}
						break;
					case 4:
						for (i = T2D_BT_DIARIO_PAGE_4_TEXT_1; i <= T2D_BT_DIARIO_PAGE_4_TEXT_12; i++) {
							if (!(w->bm[i].tnum & T2D_BM_OFF))
								w->bm[i].tnum |= T2D_BM_OFF;
						}
						break;
					}
				}

				if ((bSomeOneSpeak) && ((TheMessage->event == ME_MLEFT) || (TheMessage->event == ME_MRIGHT))) {
					bSkipTalk = TRUE;
					ClearText();
					_vm->_messageSystem.deleteWaitingMsgs(MP_WAIT_LINK);
					return;
				}

				if ((CurButton == T2D_BT_DIARIO_PAGE_LEFT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione tasto
					w->bm[T2D_BM_DIARIO_PAGE_LEFT_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_DIARIO, T2D_BM_DIARIO_PAGE_LEFT_ON,
							FALSE, nullptr, nullptr, nullptr);

					switch (diarioCurrentPage) {
					case 1: //Pagina Iniziale, non puo andare a pagina precedente
						break;
					case 2:
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_DIARIO, T2D_BM_DIARIO_PAGE_1,
								TRUE, nullptr, nullptr, nullptr);
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_DIARIO, T2D_BM_DIARIO_PAGE_2,
								FALSE, nullptr, nullptr, nullptr);
						w->bt[T2D_BT_DIARIO_PAGE_LEFT].on = false;
						for (i = T2D_BT_DIARIO_PAGE_1_TEXT_1; i <= T2D_BT_DIARIO_PAGE_1_TEXT_6; i++)
							w->bt[i].on = TRUE;
						for (i = T2D_BT_DIARIO_PAGE_2_TEXT_1; i <= T2D_BT_DIARIO_PAGE_2_TEXT_6; i++)
							w->bt[i].on = false;
						diarioCurrentPage = 1;
						break;
					case 3:
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_DIARIO, T2D_BM_DIARIO_PAGE_2,
								TRUE, nullptr, nullptr, nullptr);
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_DIARIO, T2D_BM_DIARIO_PAGE_3,
								FALSE, nullptr, nullptr, nullptr);
						for (i = T2D_BT_DIARIO_PAGE_2_TEXT_1; i <= T2D_BT_DIARIO_PAGE_2_TEXT_6; i++)
							w->bt[i].on = TRUE;
						for (i = T2D_BT_DIARIO_PAGE_3_TEXT_1; i <= T2D_BT_DIARIO_PAGE_3_TEXT_4; i++)
							w->bt[i].on = false;
						diarioCurrentPage = 2;
						break;
					case 4:
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_DIARIO, T2D_BM_DIARIO_PAGE_3,
								TRUE, nullptr, nullptr, nullptr);
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_DIARIO, T2D_BM_DIARIO_PAGE_4,
								FALSE, nullptr, nullptr, nullptr);
						w->bt[T2D_BT_DIARIO_PAGE_RIGHT].on = TRUE;
						for (i = T2D_BT_DIARIO_PAGE_3_TEXT_1; i <= T2D_BT_DIARIO_PAGE_3_TEXT_4; i++)
							w->bt[i].on = TRUE;
						for (i = T2D_BT_DIARIO_PAGE_4_TEXT_1; i <= T2D_BT_DIARIO_PAGE_4_TEXT_12; i++)
							w->bt[i].on = false;
						diarioCurrentPage = 3;
						break;
					}
				} else if ((CurButton == T2D_BT_DIARIO_PAGE_RIGHT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione tasto
					w->bm[T2D_BM_DIARIO_PAGE_RIGHT_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_DIARIO, T2D_BM_DIARIO_PAGE_RIGHT_ON,
							FALSE, nullptr, nullptr, nullptr);

					switch (diarioCurrentPage) {
					case 1:
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_DIARIO, T2D_BM_DIARIO_PAGE_1,
								FALSE, nullptr, nullptr, nullptr);
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_DIARIO, T2D_BM_DIARIO_PAGE_2,
								TRUE, nullptr, nullptr, nullptr);
						w->bt[T2D_BT_DIARIO_PAGE_LEFT].on = TRUE;
						for (i = T2D_BT_DIARIO_PAGE_1_TEXT_1; i <= T2D_BT_DIARIO_PAGE_1_TEXT_6; i++)
							w->bt[i].on = false;
						for (i = T2D_BT_DIARIO_PAGE_2_TEXT_1; i <= T2D_BT_DIARIO_PAGE_2_TEXT_6; i++)
							w->bt[i].on = TRUE;
						diarioCurrentPage = 2;
						break;
					case 2:
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_DIARIO, T2D_BM_DIARIO_PAGE_2,
								FALSE, nullptr, nullptr, nullptr);
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_DIARIO, T2D_BM_DIARIO_PAGE_3,
								TRUE, nullptr, nullptr, nullptr);
						for (i = T2D_BT_DIARIO_PAGE_2_TEXT_1; i <= T2D_BT_DIARIO_PAGE_2_TEXT_6; i++)
							w->bt[i].on = false;
						for (i = T2D_BT_DIARIO_PAGE_3_TEXT_1; i <= T2D_BT_DIARIO_PAGE_3_TEXT_4; i++)
							w->bt[i].on = TRUE;
						diarioCurrentPage = 3;
						break;
					case 3:
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_DIARIO, T2D_BM_DIARIO_PAGE_3,
								FALSE, nullptr, nullptr, nullptr);
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_DIARIO, T2D_BM_DIARIO_PAGE_4,
								TRUE, nullptr, nullptr, nullptr);
						w->bt[T2D_BT_DIARIO_PAGE_RIGHT].on = false;
						for (i = T2D_BT_DIARIO_PAGE_3_TEXT_1; i <= T2D_BT_DIARIO_PAGE_3_TEXT_4; i++)
							w->bt[i].on = false;
						for (i = T2D_BT_DIARIO_PAGE_4_TEXT_1; i <= T2D_BT_DIARIO_PAGE_4_TEXT_12; i++)
							w->bt[i].on = TRUE;
						diarioCurrentPage = 4;
						break;
					case 4: //Ultima Pagina
						break;
					}
				} else if ((CurButton == T2D_BT_DIARIO_EXIT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione tasto
					w->bm[T2D_BM_DIARIO_EXIT_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_DIALUP_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_DIARIO, T2D_BM_DIARIO_EXIT_ON, FALSE,
							nullptr, nullptr, nullptr);

					//Uscita
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTOP, MP_WAITA, 0, 0, tDIARIO, nullptr, nullptr, nullptr);
				} else if ((CurButton >= T2D_BT_DIARIO_PAGE_1_TEXT_1) && (CurButton <= T2D_BT_DIARIO_PAGE_1_TEXT_6) &&
				           (diarioCurrentPage == 1)) {
					if (TheMessage->event == ME_MOUSEUPDATE) {
						w->bm[CurButton].tnum &= ~T2D_BM_OFF;
					} else {
						if (CurPlayer == DARRELL)
							PlayerSpeak(game, init.Obj[o34DIARIOAP].action[CurPlayer]);
						else PlayerSpeak(game, init.Obj[oDIARIOPAG1A +
							                                (CurButton - T2D_BT_DIARIO_PAGE_1_TEXT_1)].examine[CurPlayer]);
					}
				} else if ((CurButton >= T2D_BT_DIARIO_PAGE_2_TEXT_1) && (CurButton <= T2D_BT_DIARIO_PAGE_2_TEXT_6) &&
				           (diarioCurrentPage == 2)) {
					if (TheMessage->event == ME_MOUSEUPDATE) {
						w->bm[CurButton].tnum &= ~T2D_BM_OFF;
					} else {
						if (CurPlayer == DARRELL)
							PlayerSpeak(game, init.Obj[o34DIARIOAP].action[CurPlayer]);
						else PlayerSpeak(game, init.Obj[oDIARIOPAG2A +
							                                (CurButton - T2D_BT_DIARIO_PAGE_2_TEXT_1)].examine[CurPlayer]);
					}
				} else if ((CurButton >= T2D_BT_DIARIO_PAGE_3_TEXT_1) && (CurButton <= T2D_BT_DIARIO_PAGE_3_TEXT_4) &&
				           (diarioCurrentPage == 3)) {
					if (TheMessage->event == ME_MOUSEUPDATE) {
						w->bm[CurButton].tnum &= ~T2D_BM_OFF;
					} else {
						if (CurPlayer == DARRELL)
							PlayerSpeak(game, init.Obj[o34DIARIOAP].action[CurPlayer]);
						else PlayerSpeak(game, init.Obj[oDIARIOPAG3A +
							                                (CurButton - T2D_BT_DIARIO_PAGE_3_TEXT_1)].examine[CurPlayer]);
					}
				} else if ((CurButton >= T2D_BT_DIARIO_PAGE_4_TEXT_1) && (CurButton <= T2D_BT_DIARIO_PAGE_4_TEXT_12) &&
				           (diarioCurrentPage == 4)) {
					if (TheMessage->event == ME_MOUSEUPDATE) {
						w->bm[CurButton].tnum &= ~T2D_BM_OFF;
					} else {
						if (CurPlayer == DARRELL)
							PlayerSpeak(game, init.Obj[o34DIARIOAP].action[CurPlayer]);
						else {
							PlayerSpeak(game, init.Obj[oDIARIOPAG4A +
							                           (CurButton - T2D_BT_DIARIO_PAGE_4_TEXT_1)].examine[CurPlayer]);
							_vm->_messageSystem.doEvent(EventClass::MC_STRING, ME_PLAYERSPEAK, MP_WAIT_LINK,
									init.Obj[oDIARIOPAG4A + (CurButton - T2D_BT_DIARIO_PAGE_4_TEXT_1)].text[0], 0, 0,
									nullptr, nullptr, nullptr);
						}
					}
				}
			}
			break;
		}
		case tPDA: {
			if (CurButtonWin == T2D_WIN_PDA_MAIN) {
				if ((CurButton == T2D_BT_PDA_MAIN_TONE) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione Tasto
					StartSound(game, wPDAPULSANTE);
					w->bm[T2D_BM_PDA_MAIN_TONE_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_PDA_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_PDA_MAIN, T2D_BM_PDA_MAIN_TONE_ON,
							FALSE, nullptr, nullptr, nullptr);
					//Apre il Tone Sequencer
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_PDA_TONE, 0, TRUE, nullptr, nullptr,
							nullptr);
				} else if ((CurButton == T2D_BT_PDA_MAIN_LOG) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione Tasto
					StartSound(game, wPDAPULSANTE);
					w->bm[T2D_BM_PDA_MAIN_LOG_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_PDA_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_PDA_MAIN, T2D_BM_PDA_MAIN_LOG_ON,
							FALSE, nullptr, nullptr, nullptr);
					//Apre il Log Viewer
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_PDA_LOG, 0, TRUE, nullptr, nullptr,
							nullptr);
					//Resetta variabili
					RefreshLog(game, RESTARTALL);
					RefreshLog(game, REFRESH);
				} else if ((CurButton == T2D_BT_PDA_MAIN_QUIT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione Tasto
					StartSound(game, wPDAOFF);
					w->bm[T2D_BM_PDA_MAIN_QUIT_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_COMPUTER_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_PDA_MAIN, T2D_BM_PDA_MAIN_QUIT_ON,
							FALSE, nullptr, nullptr, nullptr);
					//Esce dal PDA
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTOP, MP_WAITA, 0, 0, tPDA, nullptr, nullptr, nullptr);
				}
			} else if (CurButtonWin == T2D_WIN_PDA_LOG) {
				if ((CurButton == T2D_BT_PDA_LOG_SCROLL) && (bLPressed || bRPressed)) {
					if (ScrollWithMouse == 0) {
						//Inizializza scrolling
						ScrollWithMouse = CurButtonWin;
						ScrollWithMouseX = mouse_x;
						ScrollWithMouseY = mouse_y;
					}
				} else if ((CurButton == T2D_BT_PDA_LOG_SCROLL) && (TheMessage->event != ME_MOUSEUPDATE)) {
					if (TheMessage->event == ME_MRIGHT) {
						for (i = 0; i < MAX_PDALOGS; i++)
							//if ((strcmp(PDALog[i].info, "MENU")==0) && (CurrentLogMenu==PDALog[i].time))
							if ((init.PDALog[i].flags & PDA_MENU) && (init.PDALog[i].menu_creato == CurrentLogMenu)) {
								CurrentLogMenu = init.PDALog[i].menu_appartenenza;
								NumMenus--;

								RefreshLog(game, REFRESH);
							}

						break;
					}

					StartY = (int)((double)(mouse_y - w->bt[T2D_BT_PDA_LOG_SCROLL].lim.y1 - w->px) /
					               ((double)(w->bt[T2D_BT_PDA_LOG_SCROLL].lim.y2 -
					                         w->bt[T2D_BT_PDA_LOG_SCROLL].lim.y1) / 20.0));

					/*if (NumMenus>0)   StartY-=NumMenus+2;

					if (StartY<0)
					{
					    CurrentLogMenu=MenuLinks[NumMenus+1+StartY];
					    NumMenus=NumMenus+2+StartY;
					    RefreshLog(init, REFRESH);
					    break;
					}*/

					i = 0;
					a = 0;
					while (CurrentPDALogs[i] != nullptr) {
						if ((StartY >= a) && (StartY < a + CurrentPDALogs[i]->lines))
							break;

						a += CurrentPDALogs[i]->lines + 1;
						if (i == 0) a -= PDAScrollLine;
						i++;
					}

					if (CurrentPDALogs[i] == nullptr) break;

					if (CurrentPDALogs[i]->flags & PDA_MENU) {
						//CurrentLogMenu=CurrentPDALogs[i]->menu_creato;
						//NumMenus++;
						if (CurrentPDALogs[i]->flags & PDA_MENU_OPEN)
							CurrentPDALogs[i]->flags &= ~PDA_MENU_OPEN;
						else CurrentPDALogs[i]->flags |= PDA_MENU_OPEN;
						RefreshLog(game, REFRESH);
					}
				} else if ((CurButton == T2D_BT_PDA_LOG_SCROLLUP) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Gestione tasto
					w->bm[T2D_BM_PDA_LOG_SCROLLUP_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_PDA_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_PDA_LOG, T2D_BM_PDA_LOG_SCROLLUP_ON,
							FALSE, nullptr, nullptr, nullptr);

					//Riposiziona il log
					RefreshLog(game, -1);
				} else if ((CurButton == T2D_BT_PDA_LOG_SCROLLDOWN) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Gestione tasto
					w->bm[T2D_BM_PDA_LOG_SCROLLDOWN_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_PDA_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_PDA_LOG, T2D_BM_PDA_LOG_SCROLLDOWN_ON,
							FALSE, nullptr, nullptr, nullptr);

					//Riposiziona il log
					RefreshLog(game, +1);
				} else if (CurButton == T2D_BT_PDA_LOG_SB) {
					if ((PiroloGrab == 0) && bLPressed) {
						PiroloGrab = (mouse_y - w->py) - w->bt[T2D_BT_PDA_LOG_SB].lim.y1;
						ScrollWithMouse = TRUE;
					}
					/*if (PiroloGrab!=0)
					{
					    int NewScrollPos;
					    LogSBPos=(mouse_y-w->py)-PiroloGrab;
					    if (LogSBPos<T2D_PDA_LOG_SB_STARTY) LogSBPos=T2D_PDA_LOG_SB_STARTY;
					    if (LogSBPos>T2D_PDA_LOG_SB_STARTY+T2D_PDA_LOG_SB_TOTY) LogSBPos=T2D_PDA_LOG_SB_STARTY+T2D_PDA_LOG_SB_TOTY;
					    NewScrollPos=(double)LogSBPos/(double)LogSBDy;
					    while(NewScrollPos!=LogCurAbsLine)
					    {
					        if (NewScrollPos<LogCurAbsLine) RefreshLog(-1);
					        if (NewScrollPos>LogCurAbsLine) RefreshLog(+1);
					    }
					    w->bm[T2D_BM_PDA_LOG_SB].py = LogSBPos;
					    w->bt[T2D_BT_PDA_LOG_SB].lim.y1 = LogSBPos;
					    w->bt[T2D_BT_PDA_LOG_SB].lim.y2 = LogSBPos + T2D_PDA_LOG_SB_PIRGY;
					    //game.gameOptions.sound_volume=GestioneBarra(&w->bm[T2D_BM_OPTIONS_SOUND_BARRA], &w->bm[T2D_BM_OPTIONS_SOUND_PIROLO], mouse_x, -1);
					}*/
				} else if ((CurButton == T2D_BT_PDA_LOG_BARRA) && (TheMessage->event != ME_MOUSEUPDATE)) {
					if (mouse_y < LogSBPos)
						RefreshLog(game, -1);
					else RefreshLog(game, +1);
				} else if ((CurButton == T2D_BT_PDA_LOG_EXIT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione Tasto
					StartSound(game, wPDAPULSANTE);
					w->bm[T2D_BM_PDA_LOG_EXIT_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_PDA_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_PDA_LOG, T2D_BM_PDA_LOG_EXIT_ON, FALSE,
							nullptr, nullptr, nullptr);
					//Chiude finestra help se aperta
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_DEFAULT, T2D_WIN_PDA_HELP, 0, FALSE, nullptr, nullptr,
							nullptr);
					//Chiude finestra Log Viewer
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_PDA_LOG, 0, FALSE, nullptr, nullptr,
							nullptr);
				}
			} else if (CurButtonWin == T2D_WIN_PDA_TONE) {
				if ((CurButton == T2D_BT_PDA_TONE_HELP) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione Tasto
					StartSound(game, wPDAPULSANTE);
					w->bm[T2D_BM_PDA_TONE_HELP_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_PDA_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_PDA_TONE, T2D_BM_PDA_TONE_HELP_ON,
							FALSE, nullptr, nullptr, nullptr);
					//Setta e Apre finestra Help
					SetHelpWin(game._fonts, nullptr, 5, Sentence[init.Obj[oTONERHELP].text[0]],
					           Sentence[init.Obj[oTONERHELP].text[1]], Sentence[init.Obj[oTONERHELP].text[2]],
					           Sentence[init.Obj[oTONERHELP].text[3]], Sentence[init.Obj[oTONERHELP].text[4]]);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_PDA_HELP, 0, TRUE, nullptr, nullptr,
							nullptr);
				} else if ((CurButton == T2D_BT_PDA_TONE_EXIT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione Tasto
					StartSound(game, wPDAPULSANTE);
					w->bm[T2D_BM_PDA_TONE_EXIT_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_PDA_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_PDA_TONE, T2D_BM_PDA_TONE_EXIT_ON,
							FALSE, nullptr, nullptr, nullptr);
					//Chiude finestra help
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_DEFAULT, T2D_WIN_PDA_HELP, 0, FALSE, nullptr, nullptr,
							nullptr);
					//Chiude finestra Tone
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_PDA_TONE, 0, FALSE, nullptr, nullptr,
							nullptr);
				} else if ((CurButton == T2D_BT_PDA_TONE_ACQUIRE) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Disattiva tasto
					w->bt[T2D_BT_PDA_TONE_ACQUIRE].on = FALSE;
					//Pressione Tasto
					StartSound(game, wPDAPULSANTE);
					w->bm[T2D_BM_PDA_TONE_ACQUIRE_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_PDA_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_PDA_TONE, T2D_BM_PDA_TONE_ACQUIRE_ON,
							FALSE, nullptr, nullptr, nullptr);
					//Fa partire sequenza di aquiring
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DCONTINUE, MP_WAITA, 0, 0, T2D_PDA_TONE_AQUIRING, nullptr, nullptr,
							nullptr);
				} else if ((CurButton == T2D_BT_PDA_TONE_PROCESS) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Disattiva tasto
					w->bt[T2D_BT_PDA_TONE_PROCESS].on = FALSE;
					//Pressione Tasto
					StartSound(game, wPDAPULSANTE);
					w->bm[T2D_BM_PDA_TONE_PROCESS_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_PDA_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_PDA_TONE, T2D_BM_PDA_TONE_PROCESS_ON,
							FALSE, nullptr, nullptr, nullptr);
					//Fa partire sequenza di processing
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DCONTINUE, MP_WAITA, 0, 0, T2D_PDA_TONE_PROCESS, nullptr, nullptr,
							nullptr);
					if (!(init.InvObj[i28WALKMANOK].flags & EXTRA2))
						IncCurTime(game, 5);
					init.InvObj[i28WALKMANOK].flags |= EXTRA2;
				}

			} else if (CurButtonWin == T2D_WIN_PDA_HELP) {
				if ((CurButton == T2D_BT_PDA_HELP_MOVE) && (bLPressed || bRPressed)) {
					LinkWinToMouse = CurButtonWin;
					LinkWinX = mouse_x;
					LinkWinY = mouse_y;
				} else if ((CurButton == T2D_BT_PDA_HELP_EXIT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione Tasto
					StartSound(game, wPDAPULSANTE);
					w->bm[T2D_BM_PDA_HELP_EXIT_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_PDA_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_PDA_HELP, T2D_BM_PDA_HELP_EXIT_ON,
							FALSE, nullptr, nullptr, nullptr);
					//Chiude finestra Tone
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_PDA_HELP, 0, FALSE, nullptr, nullptr,
							nullptr);
					if (HelpFunc != NULL) HelpFunc();
				}
			}
			break;
		}
		case tOPTIONS: {
			w->bm[T2D_BM_OPTIONS_MAINMENU_OFF].tnum &= ~T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_MAINMENU_ON].tnum |= T2D_BM_OFF;

			if (optionsSaveDescriptActive) break;
			if (CurButtonWin == T2D_WIN_OPTIONS_MAIN) {
				if ((
				            ((CurButton == T2D_BT_OPTIONS_SAVE) && PlayerCanSave()) ||
				            (CurButton == T2D_BT_OPTIONS_LOAD) ||
				            (CurButton == T2D_BT_OPTIONS_OPTIONS)
				        ) &&
				        (TheMessage->event != ME_MOUSEUPDATE)) {
					//Cambia finestra
					if (optionsCurrent != CurButton) {
						switch (optionsCurrent) {
						case T2D_BT_OPTIONS_SAVE:
							w->bm[T2D_BM_OPTIONS_CONTORNO].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_SAVE].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_GRIGLIA].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_FRECCIA_SU_OFF].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_OFF].tnum |= T2D_BM_OFF;
							w->bt[T2D_BT_OPTIONS_FRECCIA_SU].on = FALSE;
							w->bt[T2D_BT_OPTIONS_FRECCIA_GIU].on = FALSE;
							break;
						case T2D_BT_OPTIONS_LOAD:
							w->bm[T2D_BM_OPTIONS_CONTORNO].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_LOAD].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_GRIGLIA].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_FRECCIA_SU_OFF].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_OFF].tnum |= T2D_BM_OFF;
							w->bt[T2D_BT_OPTIONS_FRECCIA_SU].on = FALSE;
							w->bt[T2D_BT_OPTIONS_FRECCIA_GIU].on = FALSE;
							break;
						case T2D_BT_OPTIONS_OPTIONS:
							w->bm[T2D_BM_OPTIONS_CONTORNO].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_SOUND_ON].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_MUSIC_ON].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_SPEECH_ON].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_SUBTITLES_ON].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_SOUND_OFF].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_MUSIC_OFF].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_SPEECH_OFF].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_SUBTITLES_OFF].tnum |= T2D_BM_OFF;
							w->bt[T2D_BT_OPTIONS_SOUND].on = false;
							w->bt[T2D_BT_OPTIONS_MUSIC].on = false;
							w->bt[T2D_BT_OPTIONS_SPEECH].on = false;
							w->bt[T2D_BT_OPTIONS_SUBTITLES].on = false;
							w->bm[T2D_BM_OPTIONS_SOUND_BARRA].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_SOUND_PIROLO].tnum |= T2D_BM_OFF;
							w->bt[T2D_BT_OPTIONS_SOUND_BARRA].on = false;
							w->bm[T2D_BM_OPTIONS_MUSIC_BARRA].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_MUSIC_PIROLO].tnum |= T2D_BM_OFF;
							w->bt[T2D_BT_OPTIONS_MUSIC_BARRA].on = false;
							w->bm[T2D_BM_OPTIONS_SPEECH_BARRA].tnum |= T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_SPEECH_PIROLO].tnum |= T2D_BM_OFF;
							w->bt[T2D_BT_OPTIONS_SPEECH_BARRA].on = false;
							break;
						}

						switch (CurButton) {
						case T2D_BT_OPTIONS_SAVE:
							w->bm[T2D_BM_OPTIONS_CONTORNO].tnum &= ~T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_SAVE].tnum &= ~T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_GRIGLIA].tnum &= ~T2D_BM_OFF;
							RefreshSaveImg(game, 0, 0);
							break;
						case T2D_BT_OPTIONS_LOAD:
							w->bm[T2D_BM_OPTIONS_CONTORNO].tnum &= ~T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_LOAD].tnum &= ~T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_GRIGLIA].tnum &= ~T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_FRECCIA_SU_OFF].tnum &= ~T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_OFF].tnum &= ~T2D_BM_OFF;
							w->bt[T2D_BT_OPTIONS_FRECCIA_SU].on = TRUE;
							w->bt[T2D_BT_OPTIONS_FRECCIA_GIU].on = TRUE;
							RefreshSaveImg(game, 0, 1);
							break;
						case T2D_BT_OPTIONS_OPTIONS:
							w->bm[T2D_BM_OPTIONS_CONTORNO].tnum &= ~T2D_BM_OFF;
							if (game.gameOptions.sound_on)
								w->bm[T2D_BM_OPTIONS_SOUND_ON].tnum &= ~T2D_BM_OFF;
							else w->bm[T2D_BM_OPTIONS_SOUND_OFF].tnum &= ~T2D_BM_OFF;
							if (game.gameOptions.music_on)
								w->bm[T2D_BM_OPTIONS_MUSIC_ON].tnum &= ~T2D_BM_OFF;
							else w->bm[T2D_BM_OPTIONS_MUSIC_OFF].tnum &= ~T2D_BM_OFF;
							if (game.gameOptions.speech_on)
								w->bm[T2D_BM_OPTIONS_SPEECH_ON].tnum &= ~T2D_BM_OFF;
							else w->bm[T2D_BM_OPTIONS_SPEECH_OFF].tnum &= ~T2D_BM_OFF;
							if (game.gameOptions.subtitles_on)
								w->bm[T2D_BM_OPTIONS_SUBTITLES_ON].tnum &= ~T2D_BM_OFF;
							else w->bm[T2D_BM_OPTIONS_SUBTITLES_OFF].tnum &= ~T2D_BM_OFF;
							w->bt[T2D_BT_OPTIONS_SOUND].on = TRUE;
							w->bt[T2D_BT_OPTIONS_MUSIC].on = TRUE;
							w->bt[T2D_BT_OPTIONS_SPEECH].on = TRUE;
							w->bt[T2D_BT_OPTIONS_SUBTITLES].on = TRUE;
							w->bm[T2D_BM_OPTIONS_SOUND_BARRA].tnum &= ~T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_SOUND_PIROLO].tnum &= ~T2D_BM_OFF;
							w->bt[T2D_BT_OPTIONS_SOUND_BARRA].on = TRUE;
							w->bm[T2D_BM_OPTIONS_MUSIC_BARRA].tnum &= ~T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_MUSIC_PIROLO].tnum &= ~T2D_BM_OFF;
							w->bt[T2D_BT_OPTIONS_MUSIC_BARRA].on = TRUE;
							w->bm[T2D_BM_OPTIONS_SPEECH_BARRA].tnum &= ~T2D_BM_OFF;
							w->bm[T2D_BM_OPTIONS_SPEECH_PIROLO].tnum &= ~T2D_BM_OFF;
							w->bt[T2D_BT_OPTIONS_SPEECH_BARRA].on = TRUE;
							GestioneBarra(&w->bm[T2D_BM_OPTIONS_SOUND_BARRA], &w->bm[T2D_BM_OPTIONS_SOUND_PIROLO],
							              -1, game.gameOptions.sound_volume);
							GestioneBarra(&w->bm[T2D_BM_OPTIONS_MUSIC_BARRA], &w->bm[T2D_BM_OPTIONS_MUSIC_PIROLO],
							              -1, game.gameOptions.music_volume);
							GestioneBarra(&w->bm[T2D_BM_OPTIONS_SPEECH_BARRA], &w->bm[T2D_BM_OPTIONS_SPEECH_PIROLO],
							              -1, game.gameOptions.speech_volume);
							RefreshSaveImg(game, 0, 2);
							break;
						}

						if ((CurButton == T2D_BT_OPTIONS_SAVE) || (CurButton == T2D_BT_OPTIONS_LOAD)) {
							if (optionsCurPos > 0) {
								w->bm[T2D_BM_OPTIONS_FRECCIA_SU_OFF].tnum &= ~T2D_BM_OFF;
								w->bt[T2D_BT_OPTIONS_FRECCIA_SU].on = TRUE;
							} else {
								w->bm[T2D_BM_OPTIONS_FRECCIA_SU_OFF].tnum |= T2D_BM_OFF;
								w->bt[T2D_BT_OPTIONS_FRECCIA_SU].on = FALSE;
							}
							if (optionsCurPos + 6 < NSaves) {
								w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_OFF].tnum &= ~T2D_BM_OFF;
								w->bt[T2D_BT_OPTIONS_FRECCIA_GIU].on = TRUE;
							} else {
								w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_OFF].tnum |= T2D_BM_OFF;
								w->bt[T2D_BT_OPTIONS_FRECCIA_GIU].on = FALSE;
							}
						}

						optionsCurrent = CurButton;
					}
				} else if ((CurButton >= T2D_BT_OPTIONS_SAVE_START) && (CurButton <= T2D_BT_OPTIONS_SAVE_END) &&
				           (TheMessage->event != ME_MOUSEUPDATE)) {
					optionsWhat = CurButton - T2D_BT_OPTIONS_SAVE_START;
					if (optionsSaves[optionsWhat][0] == 0) break;

					switch (optionsCurrent) {
					case T2D_BT_OPTIONS_SAVE:
						optionsSlot = -1;

						if (optionsCurPos + optionsWhat == 0) {
							for (i = 0; i < T2D_OPTIONS_MAX_SAVES; i++) {
								int afret;

								snprintf(Name, MAX_PATH, "%sWm%02d.sav", game.workDirs._savesDir.c_str(), i + 1);

								t3dForceNOFastFile(1);
								afret = t3dAccessFile(Name);
								t3dForceNOFastFile(0);

								if (!afret) {
									optionsSlot = i + 1;
									break;
								}
							}
						} else optionsSlot = Saves[optionsCurPos + optionsWhat - 1].NFile;

						if (optionsSlot == -1) break; //Spazi finiti

						{
							TimeDate sysTime;
							g_system->getTimeAndDate(sysTime);
							snprintf(Text, 1000, "%02d:%02d.%02d %02d/%02d/%02d", sysTime.tm_hour, sysTime.tm_min,
									sysTime.tm_sec, sysTime.tm_mday, sysTime.tm_mon, sysTime.tm_year);
						}
						if (DataSave(Text, (uint8) optionsSlot)) {
							snprintf(Text, 1000, "%stemp.tmp", game.workDirs._gameDir.c_str());
							snprintf(Name, MAX_PATH, "%sWmSav%02d.tga", game.workDirs._savesDir.c_str(), optionsSlot);
							CopyFile(Text, Name, FALSE);
						} else {
							DebugLogFile("DataSave(slot %d) Failed. Quitting ...", optionsSlot);
							CloseSys(game);
						}

						CaricaSaves(game);
						SortSaves();
						RefreshSaveImg(game, optionsCurPos, 0);
						break;
					case T2D_BT_OPTIONS_LOAD:
						bShowOnlyLoadWindow = 0;
						i = Saves[optionsCurPos + optionsWhat].NFile;
						snprintf(Name, MAX_PATH, "%sWm%02d.sav", game.workDirs._savesDir.c_str(), i);
						if (!DataLoad(game, "", (uint8) i)) {
							DebugLogFile("DataLoad(slot %d) Failed. Quitting ...", i);
							CloseSys(game);
						}
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTOP, MP_DEFAULT, 0, 0, tOPTIONS, nullptr, nullptr,
								nullptr);
						break;
					}
				} else if ((CurButton == T2D_BT_OPTIONS_FRECCIA_SU) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione Tasto
					w->bm[T2D_BM_OPTIONS_FRECCIA_SU_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_PDA_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_OPTIONS_MAIN,
							T2D_BM_OPTIONS_FRECCIA_SU_ON, FALSE, nullptr, nullptr, nullptr);
					//Scrolling Up
					if (optionsCurPos > 0) {
						optionsCurPos -= 3;
						if (optionsCurPos < 0) optionsCurPos = 0;
						switch (optionsCurrent) {
						case T2D_BT_OPTIONS_SAVE:
							RefreshSaveImg(game, optionsCurPos, 0);
							break;
						case T2D_BT_OPTIONS_LOAD:
							RefreshSaveImg(game, optionsCurPos, 1);
							break;
						}
					}

					if (optionsCurPos > 0) {
						w->bm[T2D_BM_OPTIONS_FRECCIA_SU_OFF].tnum &= ~T2D_BM_OFF;
						w->bt[T2D_BT_OPTIONS_FRECCIA_SU].on = TRUE;
					} else {
						w->bm[T2D_BM_OPTIONS_FRECCIA_SU_OFF].tnum |= T2D_BM_OFF;
						w->bt[T2D_BT_OPTIONS_FRECCIA_SU].on = FALSE;
					}
					if (optionsCurPos + 6 < NSaves) {
						w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_OFF].tnum &= ~T2D_BM_OFF;
						w->bt[T2D_BT_OPTIONS_FRECCIA_GIU].on = TRUE;
					} else {
						w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_OFF].tnum |= T2D_BM_OFF;
						w->bt[T2D_BT_OPTIONS_FRECCIA_GIU].on = FALSE;
					}
				} else if ((CurButton == T2D_BT_OPTIONS_FRECCIA_GIU) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Pressione Tasto
					w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_ON].tnum &= ~T2D_BM_OFF;
					_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_PDA_BUTTON_DELAY, 0, EFFECT_WAIT,
							nullptr, nullptr, nullptr);
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_OPTIONS_MAIN,
							T2D_BM_OPTIONS_FRECCIA_GIU_ON, FALSE, nullptr, nullptr, nullptr);
					if (optionsCurPos + 6 > NSaves) break;
					if (optionsCurPos + 3 >= T2D_OPTIONS_MAX_SAVES) break;
					//Scrolling Down
					if (optionsCurPos == 0)
						optionsCurPos += 2;
					else optionsCurPos += 3;
					switch (optionsCurrent) {
					case T2D_BT_OPTIONS_SAVE:
						RefreshSaveImg(game, optionsCurPos, 0);
						break;
					case T2D_BT_OPTIONS_LOAD:
						RefreshSaveImg(game, optionsCurPos, 1);
						break;
					}

					if (optionsCurPos > 0) {
						w->bm[T2D_BM_OPTIONS_FRECCIA_SU_OFF].tnum &= ~T2D_BM_OFF;
						w->bt[T2D_BT_OPTIONS_FRECCIA_SU].on = TRUE;
					} else {
						w->bm[T2D_BM_OPTIONS_FRECCIA_SU_OFF].tnum |= T2D_BM_OFF;
						w->bt[T2D_BT_OPTIONS_FRECCIA_SU].on = FALSE;
					}
					if (optionsCurPos + 6 < NSaves) {
						w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_OFF].tnum &= ~T2D_BM_OFF;
						w->bt[T2D_BT_OPTIONS_FRECCIA_GIU].on = TRUE;
					} else {
						w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_OFF].tnum |= T2D_BM_OFF;
						w->bt[T2D_BT_OPTIONS_FRECCIA_GIU].on = FALSE;
					}
				} else if ((CurButton == T2D_BT_OPTIONS_QUIT) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Esce dalle Opzioni
					CloseSys(game);
				} else if ((CurButton == T2D_BT_OPTIONS_CLOSE) && (TheMessage->event != ME_MOUSEUPDATE)) {
					//Esce dalle Opzioni
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTOP, MP_DEFAULT, 0, 0, tOPTIONS, nullptr, nullptr, nullptr);
				} else if (CurButton == T2D_BT_OPTIONS_MAINMENU) {
					if (TheMessage->event != ME_MOUSEUPDATE) {
						bShowOnlyLoadWindow = 0;
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTOP, MP_DEFAULT, STOP_T2D__LAUNCH_MAINMENU, 0, tOPTIONS,
								nullptr, nullptr, nullptr);
					} else {
						w->bm[T2D_BM_OPTIONS_MAINMENU_OFF].tnum |= T2D_BM_OFF;
						w->bm[T2D_BM_OPTIONS_MAINMENU_ON].tnum &= ~T2D_BM_OFF;
					}
				} else if ((CurButton == T2D_BT_OPTIONS_SOUND) && (TheMessage->event != ME_MOUSEUPDATE)) {
					game.gameOptions.sound_on = !game.gameOptions.sound_on;
					if (game.gameOptions.sound_on) {
						//Fa partire gli effetti BackGround
						for (i = 0; i < MAX_ROOMS; i++) {
							if (init.Room[i].flags & ROOM_VISIBLE) {
								for (a = 0; a < MAX_SOUNDS_IN_ROOM; a++)
									if ((init.Room[i].sounds[a]) &&
									        (init.Sound[init.Room[i].sounds[a]].flags & SOUND_ON))
										StartSound(game, init.Room[i].sounds[a]);
							}
						}

						w->bm[T2D_BM_OPTIONS_SOUND_ON].tnum &= ~T2D_BM_OFF;
						w->bm[T2D_BM_OPTIONS_SOUND_OFF].tnum |= T2D_BM_OFF;
					} else {
						sStopAllSounds();
						w->bm[T2D_BM_OPTIONS_SOUND_ON].tnum |= T2D_BM_OFF;
						w->bm[T2D_BM_OPTIONS_SOUND_OFF].tnum &= ~T2D_BM_OFF;
					}
				} else if ((CurButton == T2D_BT_OPTIONS_MUSIC) && (TheMessage->event != ME_MOUSEUPDATE)) {
					game.gameOptions.music_on = !game.gameOptions.music_on;
					if (game.gameOptions.music_on) {
						PlayMusic(init.Room[getRoomFromStr(init, t3dCurRoom->name)].music, 3000, 3000);
						w->bm[T2D_BM_OPTIONS_MUSIC_ON].tnum &= ~T2D_BM_OFF;
						w->bm[T2D_BM_OPTIONS_MUSIC_OFF].tnum |= T2D_BM_OFF;
					} else {
						StopMusic();
						w->bm[T2D_BM_OPTIONS_MUSIC_ON].tnum |= T2D_BM_OFF;
						w->bm[T2D_BM_OPTIONS_MUSIC_OFF].tnum &= ~T2D_BM_OFF;
					}
				} else if ((CurButton == T2D_BT_OPTIONS_SPEECH) && (TheMessage->event != ME_MOUSEUPDATE)) {
					game.gameOptions.speech_on = !game.gameOptions.speech_on;
					if (game.gameOptions.speech_on) {
						w->bm[T2D_BM_OPTIONS_SPEECH_ON].tnum &= ~T2D_BM_OFF;
						w->bm[T2D_BM_OPTIONS_SPEECH_OFF].tnum |= T2D_BM_OFF;
					} else {
						//turn Speech off
						w->bm[T2D_BM_OPTIONS_SPEECH_ON].tnum |= T2D_BM_OFF;
						w->bm[T2D_BM_OPTIONS_SPEECH_OFF].tnum &= ~T2D_BM_OFF;

						//forzo i sottotitoli a on se non c'e' parlato
						game.gameOptions.subtitles_on = 1;
						w->bm[T2D_BM_OPTIONS_SUBTITLES_ON].tnum &= ~T2D_BM_OFF;
						w->bm[T2D_BM_OPTIONS_SUBTITLES_OFF].tnum |= T2D_BM_OFF;
					}
				} else if ((CurButton == T2D_BT_OPTIONS_SUBTITLES) && (TheMessage->event != ME_MOUSEUPDATE)) {
					// se il parlato e' spento i sottotitoli devono rimanere attivi
					if (!game.gameOptions.speech_on && game.gameOptions.subtitles_on) break;

					game.gameOptions.subtitles_on = !game.gameOptions.subtitles_on;
					if (game.gameOptions.subtitles_on) {
						w->bm[T2D_BM_OPTIONS_SUBTITLES_ON].tnum &= ~T2D_BM_OFF;
						w->bm[T2D_BM_OPTIONS_SUBTITLES_OFF].tnum |= T2D_BM_OFF;
					} else {
						w->bm[T2D_BM_OPTIONS_SUBTITLES_ON].tnum |= T2D_BM_OFF;
						w->bm[T2D_BM_OPTIONS_SUBTITLES_OFF].tnum &= ~T2D_BM_OFF;
					}
				} else if (CurButton == T2D_BT_OPTIONS_SOUND_BARRA) {
					if (bLPressed || bRPressed || (TheMessage->event == ME_MLEFT) ||
					        (TheMessage->event == ME_MRIGHT))
						PiroloGrab = 1;
					if (PiroloGrab == 1) {
						game.gameOptions.sound_volume = GestioneBarra(&w->bm[T2D_BM_OPTIONS_SOUND_BARRA],
																	  &w->bm[T2D_BM_OPTIONS_SOUND_PIROLO], mouse_x, -1);
						sSetAllSoundsVolume(game.gameOptions.sound_volume);
					}
				} else if (CurButton == T2D_BT_OPTIONS_MUSIC_BARRA) {
					if (bLPressed || bRPressed || (TheMessage->event == ME_MLEFT) ||
					        (TheMessage->event == ME_MRIGHT))
						PiroloGrab = 2;
					if (PiroloGrab == 2) {
						game.gameOptions.music_volume = GestioneBarra(&w->bm[T2D_BM_OPTIONS_MUSIC_BARRA],
						                                &w->bm[T2D_BM_OPTIONS_MUSIC_PIROLO], mouse_x, -1);
						mSetAllVolume(game.gameOptions.music_volume);
					}
				} else if (CurButton == T2D_BT_OPTIONS_SPEECH_BARRA) {
					if (bLPressed || bRPressed || (TheMessage->event == ME_MLEFT) ||
					        (TheMessage->event == ME_MRIGHT))
						PiroloGrab = 3;
					if (PiroloGrab == 3) {
						game.gameOptions.speech_volume = GestioneBarra(&w->bm[T2D_BM_OPTIONS_SPEECH_BARRA],
																	   &w->bm[T2D_BM_OPTIONS_SPEECH_PIROLO], mouse_x,
																	   -1);
						sSetAllSpeechVolume(game.gameOptions.speech_volume);
					}
				}
			}
			break;
		}
		case tMAINMENU: {
			doT2DMouseMainMenu(game, w);
			break;
		}
		case tGAMEOVER: {
			if (timeGetTime() >= (GameOverWindow_StartTime + GAMEOVER_WINDOW_NUM_MSECONDS)) {
				LoadWindow_WhichBkg = 2;
				_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTOP, MP_DEFAULT, STOP_T2D__LAUNCH_LOAD, 0, tMAINMENU, nullptr, nullptr,
						nullptr);
			}
			break;
		}
		}
		break;
	}
}

void SetPDALogItem(Init &init, int32 log_item) {
	if (log_item == lNULL) return;

	if (!(init.PDALog[log_item].flags & PDA_ON)) {
		init.PDALog[log_item].flags |= (PDA_ON | PDA_UPDATE);
		init.PDALog[log_item].time = t3dCurTime;
		Settati_PDA_items_durante_t2d = 1;
	}
}

/* -----------------15/01/99 16.29-------------------
 *                      doT2D
 * --------------------------------------------------*/
void doT2D(WGame &game) {
	struct SDDBitmap *d, *d1, *d2, *d3, *d4;
	t2dWINDOW *w;
	t2dBUTTON *b;
	int32 i, j;
	struct SRect tmp_rect;
	static uint32 LastTime = 0, CurTime = 0;
	Init &init = game.init;
	Renderer &renderer = *game._renderer;

	switch (TheMessage->event) {
	case ME_T2DSTART:

		Settati_PDA_items_durante_t2d = 0;

		bIngnoreDIKeyboard = true;

		if (InvStatus & INV_ON)
			_vm->_messageSystem.doEvent(EventClass::MC_INVENTORY, ME_INVSWITCH, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);

		StopDiary(game, 0, 0, 0);
		ClearText();

		bT2DActive = TheMessage->bparam;
		bPlayerInAnim = true;
		bPauseAllAnims = true;
		memset(WinActive, 0, sizeof(WinActive));
		switch (bT2DActive) {
		case tSCANNER:
			w = &t2dWin[T2D_WIN_SCANNER_BACKGROUND];
			w->NOTSTRETCHED = false;
			w->px = 0;
			w->py = 0;
			w->bm[T2D_BM_SCANNER_BACKGROUND_WIN].tnum           = LoadDDBitmap(game, "t2d\\Scanner_BackgroundWin.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_BACKGROUND_EXIT].tnum          = LoadDDBitmap(game, "t2d\\Scanner_Exit.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_BACKGROUND_PIC_LOW].tnum       = LoadDDBitmap(game, "t2d\\Scanner_Pic_Low.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_BACKGROUND_PIC_HI].tnum        = LoadDDBitmap(game, "t2d\\Scanner_Pic_Hi.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_BACKGROUND_SELECTOR].tnum      = LoadDDBitmap(game, "t2d\\Scanner_Selector.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_BACKGROUND_WIN].px = 0;
			w->bm[T2D_BM_SCANNER_BACKGROUND_WIN].py = 0;
			w->bm[T2D_BM_SCANNER_BACKGROUND_EXIT].px = 772;
			w->bm[T2D_BM_SCANNER_BACKGROUND_EXIT].py = 10;
			w->bm[T2D_BM_SCANNER_BACKGROUND_PIC_LOW].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_SCANNER_BACKGROUND_PIC_LOW].px = 108;
			w->bm[T2D_BM_SCANNER_BACKGROUND_PIC_LOW].py = 118;
			w->bm[T2D_BM_SCANNER_BACKGROUND_PIC_HI].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_SCANNER_BACKGROUND_PIC_HI].px = 108;
			w->bm[T2D_BM_SCANNER_BACKGROUND_PIC_HI].py = 118;
			w->bm[T2D_BM_SCANNER_BACKGROUND_SELECTOR].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_SCANNER_BACKGROUND_SELECTOR].px = 108;
			w->bm[T2D_BM_SCANNER_BACKGROUND_SELECTOR].py = 118;

			b = &w->bt[T2D_BT_SCANNER_BACKGROUND_EXIT];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_SCANNER_BACKGROUND_EXIT]);
			b->tnum = EXTRALS_EXIT;
			b = &w->bt[T2D_BT_SCANNER_BACKGROUND_SELECTOR];
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_SCANNER_BACKGROUND_SELECTOR]);

			w = &t2dWin[T2D_WIN_SCANNER_TOOLBAR];
			w->NOTSTRETCHED = false;
			w->px = 621;
			w->py = 184;
			w->bm[T2D_BM_SCANNER_TOOLBAR_WIN].tnum              = LoadDDBitmap(game, "t2d\\Scanner_ToolbarWin.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_TOOLBAR_EXIT].tnum             = LoadDDBitmap(game, "t2d\\Scanner_Exit.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_TOOLBAR_PREVIEW_FREEZE].tnum   = LoadDDBitmap(game, "t2d\\Scanner_Preview_Freeze.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_TOOLBAR_PREVIEW_OFF].tnum      = LoadDDBitmap(game, "t2d\\Scanner_Preview_Off.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_TOOLBAR_PREVIEW_ON].tnum       = LoadDDBitmap(game, "t2d\\Scanner_Preview_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_FREEZE].tnum = LoadDDBitmap(game, "t2d\\Scanner_SelectArea_Freeze.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_OFF].tnum   = LoadDDBitmap(game, "t2d\\Scanner_SelectArea_Off.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_ON].tnum    = LoadDDBitmap(game, "t2d\\Scanner_SelectArea_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_FREEZE].tnum      = LoadDDBitmap(game, "t2d\\Scanner_Scan_Freeze.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_OFF].tnum         = LoadDDBitmap(game, "t2d\\Scanner_Scan_Off.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_ON].tnum          = LoadDDBitmap(game, "t2d\\Scanner_Scan_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_TOOLBAR_WIN].px = 0;
			w->bm[T2D_BM_SCANNER_TOOLBAR_WIN].py = 0;
			w->bm[T2D_BM_SCANNER_TOOLBAR_EXIT].px = 90;
			w->bm[T2D_BM_SCANNER_TOOLBAR_EXIT].py = 10;
			w->bm[T2D_BM_SCANNER_TOOLBAR_PREVIEW_FREEZE].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_SCANNER_TOOLBAR_PREVIEW_FREEZE].px = 10;
			w->bm[T2D_BM_SCANNER_TOOLBAR_PREVIEW_FREEZE].py = 32;
			w->bm[T2D_BM_SCANNER_TOOLBAR_PREVIEW_OFF].px = 10;
			w->bm[T2D_BM_SCANNER_TOOLBAR_PREVIEW_OFF].py = 32;
			w->bm[T2D_BM_SCANNER_TOOLBAR_PREVIEW_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_SCANNER_TOOLBAR_PREVIEW_ON].px = 10;
			w->bm[T2D_BM_SCANNER_TOOLBAR_PREVIEW_ON].py = 32;
			w->bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_FREEZE].px = 10;
			w->bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_FREEZE].py = 67;
			w->bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_OFF].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_OFF].px = 10;
			w->bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_OFF].py = 67;
			w->bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_ON].px = 10;
			w->bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_ON].py = 67;
			w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_FREEZE].px = 10;
			w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_FREEZE].py = 102;
			w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_OFF].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_OFF].px = 10;
			w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_OFF].py = 102;
			w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_ON].px = 10;
			w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_ON].py = 102;

			b = &w->bt[T2D_BT_SCANNER_TOOLBAR_MOVE];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_SCANNER_TOOLBAR_WIN]);
			b->lim.y2 = 29;
			b->tnum = EXTRALS_SCANPROGRAM;
			b = &w->bt[T2D_BT_SCANNER_TOOLBAR_EXIT];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_SCANNER_TOOLBAR_EXIT]);
			b->tnum = EXTRALS_EXIT;
			b = &w->bt[T2D_BT_SCANNER_TOOLBAR_PREVIEW_OFF];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_SCANNER_TOOLBAR_PREVIEW_OFF]);
			b->tnum = EXTRALS_PREVIEW;
			b = &w->bt[T2D_BT_SCANNER_TOOLBAR_SELECTAREA_OFF];
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_OFF]);
			b->tnum = EXTRALS_SELECTAREA;
			b = &w->bt[T2D_BT_SCANNER_TOOLBAR_SCAN_OFF];
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_SCANNER_TOOLBAR_SCAN_OFF]);
			b->tnum = EXTRALS_SCAN;

			w = &t2dWin[T2D_WIN_SCANNER_PROGRESSBAR];
			w->NOTSTRETCHED = false;
			w->px = 280;
			w->py = 300;
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_WIN].tnum          = LoadDDBitmap(game, "t2d\\Scanner_ProgressWin.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_EXIT].tnum         = LoadDDBitmap(game, "t2d\\Scanner_Exit.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_PROGRESS_OFF].tnum = LoadDDBitmap(game, "t2d\\Scanner_ProgressBar_Off.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_PROGRESS_ON].tnum  = LoadDDBitmap(game, "t2d\\Scanner_ProgressBar_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_SCANNING_OFF].tnum = LoadDDBitmap(game, "t2d\\Scanner_Scanning_Off.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_SCANNING_ON].tnum  = LoadDDBitmap(game, "t2d\\Scanner_Scanning_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_WIN].px = 0;
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_WIN].py = 0;
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_EXIT].px = 221;
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_EXIT].py = 10;
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_PROGRESS_OFF].px = 15;
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_PROGRESS_OFF].py = 67;
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_PROGRESS_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_PROGRESS_ON].px = 15;
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_PROGRESS_ON].py = 67;
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_SCANNING_OFF].px = 86;
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_SCANNING_OFF].py = 52;
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_SCANNING_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_SCANNING_ON].px = 86;
			w->bm[T2D_BM_SCANNER_PROGRESSBAR_SCANNING_ON].py = 52;

			WinActive[0] = T2D_WIN_SCANNER_BACKGROUND;
			WinActive[1] = T2D_WIN_SCANNER_TOOLBAR;
			CurButton = 0;
			CurButtonWin = 0;
			LinkWinToMouse = 0;
			LinkWinX = 0;
			LinkWinY = 0;

			SelectorFlag = false;
			break;

		case tCOMPUTER:
			//Inizializzazione finestra BackGround
			w = &t2dWin[T2D_WIN_COMPUTER_BACKGROUND];
			w->NOTSTRETCHED = false;
			w->px = 0;
			w->py = 0;
			w->bm[T2D_BM_COMPUTER_BACKGROUND_WIN].tnum          = LoadDDBitmap(game, "t2d\\Computer_BackgroundWin.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_EMAIL_ON].tnum                = LoadDDBitmap(game, "t2d\\Computer_EMail_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_SEARCH_ON].tnum               = LoadDDBitmap(game, "t2d\\Computer_Search_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_QUIT_ON].tnum                 = LoadDDBitmap(game, "t2d\\Computer_Quit_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_BACKGROUND_WIN].px = 0;
			w->bm[T2D_BM_COMPUTER_BACKGROUND_WIN].py = 0;
			w->bm[T2D_BM_COMPUTER_EMAIL_ON].px = 97;
			w->bm[T2D_BM_COMPUTER_EMAIL_ON].py = 445;
			w->bm[T2D_BM_COMPUTER_EMAIL_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_SEARCH_ON].px = 615;
			w->bm[T2D_BM_COMPUTER_SEARCH_ON].py = 444;
			w->bm[T2D_BM_COMPUTER_SEARCH_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_QUIT_ON].px = 678;
			w->bm[T2D_BM_COMPUTER_QUIT_ON].py = 525;
			w->bm[T2D_BM_COMPUTER_QUIT_ON].tnum |= T2D_BM_OFF;

			b = &w->bt[T2D_BT_COMPUTER_BACKGROUND_EMAIL];
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_EMAIL_ON]);
			b->tnum = EXTRALS_NEWMAIL;

			b = &w->bt[T2D_BT_COMPUTER_BACKGROUND_SEARCH];
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_SEARCH_ON]);
			b->tnum = EXTRALS_SEARCH;

			b = &w->bt[T2D_BT_COMPUTER_BACKGROUND_QUIT];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_QUIT_ON]);
			b->tnum = EXTRALS_QUIT;

			//Gestione scritte
			w->bm[T2D_BM_COMPUTER_SCRITTE_1].tnum = (T2D_TEXT_COMPUTER_SCRITTE_1 << T2D_BM2TEXT_SHIFT);
			w->bm[T2D_BM_COMPUTER_SCRITTE_1].px = 50;
			w->bm[T2D_BM_COMPUTER_SCRITTE_1].py = 100;
			w->bm[T2D_BM_COMPUTER_SCRITTE_1].tnum |= T2D_BM_OFF;
			strcpy(w->text[T2D_TEXT_COMPUTER_SCRITTE_1].text, "");
			w->text[T2D_TEXT_COMPUTER_SCRITTE_1].font = FontKind::Standard;
			w->text[T2D_TEXT_COMPUTER_SCRITTE_1].color = WHITE_FONT;
			w->bm[T2D_BM_COMPUTER_SCRITTE_2].tnum = (T2D_TEXT_COMPUTER_SCRITTE_2 << T2D_BM2TEXT_SHIFT);
			w->bm[T2D_BM_COMPUTER_SCRITTE_2].px = 50;
			w->bm[T2D_BM_COMPUTER_SCRITTE_2].py = 130;
			w->bm[T2D_BM_COMPUTER_SCRITTE_2].tnum |= T2D_BM_OFF;
			strcpy(w->text[T2D_TEXT_COMPUTER_SCRITTE_2].text, "");
			w->text[T2D_TEXT_COMPUTER_SCRITTE_2].font = FontKind::Standard;
			w->text[T2D_TEXT_COMPUTER_SCRITTE_2].color = WHITE_FONT;
			w->bm[T2D_BM_COMPUTER_SCRITTE_3].tnum = (T2D_TEXT_COMPUTER_SCRITTE_3 << T2D_BM2TEXT_SHIFT);
			w->bm[T2D_BM_COMPUTER_SCRITTE_3].px = 50;
			w->bm[T2D_BM_COMPUTER_SCRITTE_3].py = 160;
			w->bm[T2D_BM_COMPUTER_SCRITTE_3].tnum |= T2D_BM_OFF;
			strcpy(w->text[T2D_TEXT_COMPUTER_SCRITTE_3].text, "");
			w->text[T2D_TEXT_COMPUTER_SCRITTE_3].font = FontKind::Standard;
			w->text[T2D_TEXT_COMPUTER_SCRITTE_3].color = WHITE_FONT;
			w->bm[T2D_BM_COMPUTER_SCRITTE_4].tnum = (T2D_TEXT_COMPUTER_SCRITTE_4 << T2D_BM2TEXT_SHIFT);
			w->bm[T2D_BM_COMPUTER_SCRITTE_4].px = 50;
			w->bm[T2D_BM_COMPUTER_SCRITTE_4].py = 190;
			w->bm[T2D_BM_COMPUTER_SCRITTE_4].tnum |= T2D_BM_OFF;
			strcpy(w->text[T2D_TEXT_COMPUTER_SCRITTE_4].text, "");
			w->text[T2D_TEXT_COMPUTER_SCRITTE_4].font = FontKind::Standard;
			w->text[T2D_TEXT_COMPUTER_SCRITTE_4].color = WHITE_FONT;

			//ExtraLS
			b = &w->bt[T2D_BT_BackgroundWin_TGA__cia1];
			b->lim.x1 = 66;
			b->lim.y1 = 33;
			b->lim.x2 = 212;
			b->lim.y2 = 47;
			b->tnum = EXTRALS_CIAMAINTERMINAL;
			b->on = true;

			b = &w->bt[T2D_BT_BackgroundWin_TGA__cia2];
			b->lim.x1 = 427;
			b->lim.y1 = 89;
			b->lim.x2 = 732;
			b->lim.y2 = 112;
			b->tnum = EXTRALS_CIAMAINTERMINAL;
			b->on = true;
			//Fine Inizializzazione finestra BackGround

			//Inizializzazione finestra Error
			//La variabile sy della finestra verra usata per indicare la finestra di errore
			//0: Niente
			//1: ComError
			//2: DialError
			//3: SearchError
			w = &t2dWin[T2D_WIN_COMPUTER_ERROR];
			w->NOTSTRETCHED = false;
			w->px = 300;
			w->py = 100;
			w->bm[T2D_BM_COMPUTER_COMERROR_WIN].tnum            = LoadDDBitmap(game, "t2d\\Computer_ComError_Win.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DIALERROR_WIN].tnum           = LoadDDBitmap(game, "t2d\\Computer_DialError_Win.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_SEARCHERROR_WIN].tnum         = LoadDDBitmap(game, "t2d\\Computer_SearchError_Win.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_ERROR_COMQUIT_ON].tnum        = LoadDDBitmap(game, "t2d\\Computer_Error_Quit_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_ERROR_DIALQUIT_ON].tnum       = LoadDDBitmap(game, "t2d\\Computer_Error_Quit_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_ERROR_DIALUP_ON].tnum         = LoadDDBitmap(game, "t2d\\Computer_Error_DialUp_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_ERROR_OK_ON].tnum             = LoadDDBitmap(game, "t2d\\Computer_Error_Ok_On.tga", rSURFACESTRETCH);

			w->bm[T2D_BM_COMPUTER_COMERROR_WIN].px = 0;
			w->bm[T2D_BM_COMPUTER_COMERROR_WIN].py = 0;
			w->bm[T2D_BM_COMPUTER_COMERROR_WIN].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DIALERROR_WIN].px = 0;
			w->bm[T2D_BM_COMPUTER_DIALERROR_WIN].py = 0;
			w->bm[T2D_BM_COMPUTER_DIALERROR_WIN].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_SEARCHERROR_WIN].px = 0;
			w->bm[T2D_BM_COMPUTER_SEARCHERROR_WIN].py = 0;
			w->bm[T2D_BM_COMPUTER_SEARCHERROR_WIN].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_ERROR_COMQUIT_ON].px = 146;
			w->bm[T2D_BM_COMPUTER_ERROR_COMQUIT_ON].py = 67;
			w->bm[T2D_BM_COMPUTER_ERROR_COMQUIT_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_ERROR_DIALQUIT_ON].px = 227;
			w->bm[T2D_BM_COMPUTER_ERROR_DIALQUIT_ON].py = 70;
			w->bm[T2D_BM_COMPUTER_ERROR_DIALQUIT_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_ERROR_DIALUP_ON].px = 76;
			w->bm[T2D_BM_COMPUTER_ERROR_DIALUP_ON].py = 70;
			w->bm[T2D_BM_COMPUTER_ERROR_DIALUP_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_ERROR_OK_ON].px = 149;
			w->bm[T2D_BM_COMPUTER_ERROR_OK_ON].py = 71;
			w->bm[T2D_BM_COMPUTER_ERROR_OK_ON].tnum |= T2D_BM_OFF;

			b = &w->bt[T2D_BT_COMPUTER_ERROR_WIN];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_COMERROR_WIN]);
			b = &w->bt[T2D_BT_COMPUTER_ERROR_MOVE];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_COMERROR_WIN]);
			b->lim.y2 = 30;
			b = &w->bt[T2D_BT_COMPUTER_ERROR_COMQUIT];
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_ERROR_COMQUIT_ON]);
			b->tnum = EXTRALS_QUIT;
			b = &w->bt[T2D_BT_COMPUTER_ERROR_DIALQUIT];
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_ERROR_DIALQUIT_ON]);
			b->tnum = EXTRALS_QUIT;
			b = &w->bt[T2D_BT_COMPUTER_ERROR_DIALUP];
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_ERROR_DIALUP_ON]);
			b->tnum = EXTRALS_DIAL;
			b = &w->bt[T2D_BT_COMPUTER_ERROR_OK];
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_ERROR_OK_ON]);
			b->tnum = EXTRALS_OK;

			//ExtraLS
			b = &w->bt[T2D_BT_ComError_Win_TGA];
			b->lim.x1 = 108;
			b->lim.y1 = 29;
			b->lim.x2 = 274;
			b->lim.y2 = 47;
			b->tnum = EXTRALS_NOCONNECTION;

			b = &w->bt[T2D_BT_DialError_Win_TGA];
			b->lim.x1 = 79;
			b->lim.y1 = 29;
			b->lim.x2 = 319;
			b->lim.y2 = 48;
			b->tnum = EXTRALS_INVALIDNUMBER;

			b = &w->bt[T2D_BT_SearchError_Win_TGA];
			b->lim.x1 = 121;
			b->lim.y1 = 33;
			b->lim.x2 = 266;
			b->lim.y2 = 49;
			b->tnum = EXTRALS_NODOCUMENTS;

			w->sy = 0;
			//Fine Inizializzazione finestra Error

			//Inizializzazione finestra DialUp
			w = &t2dWin[T2D_WIN_COMPUTER_DIALUP];
			w->NOTSTRETCHED = false;
			w->px = 300;
			w->py = 100;
			w->bm[T2D_BM_COMPUTER_DIALUP_WIN].tnum          = LoadDDBitmap(game, "t2d\\Computer_DialUpWin.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DIALUP_1_ON].tnum         = LoadDDBitmap(game, "t2d\\Computer_DialUp_1_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DIALUP_2_ON].tnum         = LoadDDBitmap(game, "t2d\\Computer_DialUp_2_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DIALUP_3_ON].tnum         = LoadDDBitmap(game, "t2d\\Computer_DialUp_3_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DIALUP_4_ON].tnum         = LoadDDBitmap(game, "t2d\\Computer_DialUp_4_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DIALUP_5_ON].tnum         = LoadDDBitmap(game, "t2d\\Computer_DialUp_5_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DIALUP_6_ON].tnum         = LoadDDBitmap(game, "t2d\\Computer_DialUp_6_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DIALUP_7_ON].tnum         = LoadDDBitmap(game, "t2d\\Computer_DialUp_7_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DIALUP_8_ON].tnum         = LoadDDBitmap(game, "t2d\\Computer_DialUp_8_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DIALUP_9_ON].tnum         = LoadDDBitmap(game, "t2d\\Computer_DialUp_9_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DIALUP_0_ON].tnum         = LoadDDBitmap(game, "t2d\\Computer_DialUp_0_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DIALUP_ENTER_ON].tnum     = LoadDDBitmap(game, "t2d\\Computer_DialUp_Enter_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DIALUP_C_ON].tnum         = LoadDDBitmap(game, "t2d\\Computer_DialUp_C_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DIALUP_OK_ON].tnum        = LoadDDBitmap(game, "t2d\\Computer_DialUp_Ok_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DIALUP_EXIT_ON].tnum      = LoadDDBitmap(game, "t2d\\Computer_Exit_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DIALUP_1_ON].px = 32;
			w->bm[T2D_BM_COMPUTER_DIALUP_1_ON].py = 124;
			w->bm[T2D_BM_COMPUTER_DIALUP_1_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DIALUP_2_ON].px = 86;
			w->bm[T2D_BM_COMPUTER_DIALUP_2_ON].py = 124;
			w->bm[T2D_BM_COMPUTER_DIALUP_2_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DIALUP_3_ON].px = 140;
			w->bm[T2D_BM_COMPUTER_DIALUP_3_ON].py = 124;
			w->bm[T2D_BM_COMPUTER_DIALUP_3_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DIALUP_4_ON].px = 32;
			w->bm[T2D_BM_COMPUTER_DIALUP_4_ON].py = 178;
			w->bm[T2D_BM_COMPUTER_DIALUP_4_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DIALUP_5_ON].px = 86;
			w->bm[T2D_BM_COMPUTER_DIALUP_5_ON].py = 178;
			w->bm[T2D_BM_COMPUTER_DIALUP_5_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DIALUP_6_ON].px = 140;
			w->bm[T2D_BM_COMPUTER_DIALUP_6_ON].py = 178;
			w->bm[T2D_BM_COMPUTER_DIALUP_6_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DIALUP_7_ON].px = 32;
			w->bm[T2D_BM_COMPUTER_DIALUP_7_ON].py = 232;
			w->bm[T2D_BM_COMPUTER_DIALUP_7_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DIALUP_8_ON].px = 86;
			w->bm[T2D_BM_COMPUTER_DIALUP_8_ON].py = 232;
			w->bm[T2D_BM_COMPUTER_DIALUP_8_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DIALUP_9_ON].px = 140;
			w->bm[T2D_BM_COMPUTER_DIALUP_9_ON].py = 232;
			w->bm[T2D_BM_COMPUTER_DIALUP_9_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DIALUP_0_ON].px = 86;
			w->bm[T2D_BM_COMPUTER_DIALUP_0_ON].py = 286;
			w->bm[T2D_BM_COMPUTER_DIALUP_0_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DIALUP_ENTER_ON].px = 32;
			w->bm[T2D_BM_COMPUTER_DIALUP_ENTER_ON].py = 286;
			w->bm[T2D_BM_COMPUTER_DIALUP_ENTER_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DIALUP_C_ON].px = 140;
			w->bm[T2D_BM_COMPUTER_DIALUP_C_ON].py = 286;
			w->bm[T2D_BM_COMPUTER_DIALUP_C_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DIALUP_OK_ON].px = 46;
			w->bm[T2D_BM_COMPUTER_DIALUP_OK_ON].py = 338;
			w->bm[T2D_BM_COMPUTER_DIALUP_OK_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DIALUP_EXIT_ON].px = 176;
			w->bm[T2D_BM_COMPUTER_DIALUP_EXIT_ON].py = 9;
			w->bm[T2D_BM_COMPUTER_DIALUP_EXIT_ON].tnum |= T2D_BM_OFF;

			b = &w->bt[T2D_BT_COMPUTER_DIALUP_EXIT];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DIALUP_EXIT_ON]);
			b = &w->bt[T2D_BT_COMPUTER_DIALUP_1];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DIALUP_1_ON]);
			b = &w->bt[T2D_BT_COMPUTER_DIALUP_2];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DIALUP_2_ON]);
			b = &w->bt[T2D_BT_COMPUTER_DIALUP_3];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DIALUP_3_ON]);
			b = &w->bt[T2D_BT_COMPUTER_DIALUP_4];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DIALUP_4_ON]);
			b = &w->bt[T2D_BT_COMPUTER_DIALUP_5];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DIALUP_5_ON]);
			b = &w->bt[T2D_BT_COMPUTER_DIALUP_6];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DIALUP_6_ON]);
			b = &w->bt[T2D_BT_COMPUTER_DIALUP_7];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DIALUP_7_ON]);
			b = &w->bt[T2D_BT_COMPUTER_DIALUP_8];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DIALUP_8_ON]);
			b = &w->bt[T2D_BT_COMPUTER_DIALUP_9];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DIALUP_9_ON]);
			b = &w->bt[T2D_BT_COMPUTER_DIALUP_0];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DIALUP_0_ON]);
			b = &w->bt[T2D_BT_COMPUTER_DIALUP_ENTER];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DIALUP_ENTER_ON]);
			b->tnum = EXTRALS_CLEAR;
			b = &w->bt[T2D_BT_COMPUTER_DIALUP_C];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DIALUP_C_ON]);
			b->tnum = EXTRALS_CANCEL;
			b = &w->bt[T2D_BT_COMPUTER_DIALUP_OK];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DIALUP_OK_ON]);
			b->tnum = EXTRALS_OK;
			b = &w->bt[T2D_BT_COMPUTER_DIALUP_MOVE];
			b->on = true;
			b->lim.x1 = 0;
			b->lim.y1 = 0;
			b->lim.x2 = 203;
			b->lim.y2 = 40;

			//Gestione Scritte
			w->bm[T2D_BM_COMPUTER_DIALUP_NUMBER].tnum = (T2D_TEXT_COMPUTER_DIALUP_NUMBER << T2D_BM2TEXT_SHIFT);
			w->bm[T2D_BM_COMPUTER_DIALUP_NUMBER].px = 50;
			w->bm[T2D_BM_COMPUTER_DIALUP_NUMBER].py = 76;
			strcpy(w->text[T2D_TEXT_COMPUTER_DIALUP_NUMBER].text, "");
			w->text[T2D_TEXT_COMPUTER_DIALUP_NUMBER].font = FontKind::Computer;
			w->text[T2D_TEXT_COMPUTER_DIALUP_NUMBER].color = BLACK_FONT;

			//Gestione variabili
			strcpy(computerNumeroTel, "");

			//ExtraLS
			b = &w->bt[T2D_BT_DialUpWin_TGA__Dial];
			b->lim.x1 = 81;
			b->lim.y1 = 7;
			b->lim.x2 = 124;
			b->lim.y2 = 22;
			b->tnum = EXTRALS_DIAL;
			b->on = true;

			b = &w->bt[T2D_BT_DialUpWin_TGA__Enter];
			b->lim.x1 = 22;
			b->lim.y1 = 48;
			b->lim.x2 = 184;
			b->lim.y2 = 67;
			b->tnum = EXTRALS_ENTERUSERNUMBER;
			b->on = true;

			b = &w->bt[T2D_BT_DialUpWin_TGA__Dialling];
			b->lim.x1 = 40;
			b->lim.y1 = 74;
			b->lim.x2 = 167;
			b->lim.y2 = 92;
			b->tnum = EXTRALS_DIALLING;
			b->on = false;

			//Fine Inizializzazione finestra DialUp

			//Inizializzazione finestra Search
			w = &t2dWin[T2D_WIN_COMPUTER_SEARCH];
			w->NOTSTRETCHED = false;
			w->px = 100;
			w->py = 100;
			w->bm[T2D_BM_COMPUTER_SEARCH_ALPHABETICAL_WIN].tnum         = LoadDDBitmap(game, "t2d\\Computer_Search_Alphabetical.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_WIN].tnum             = LoadDDBitmap(game, "t2d\\Computer_Search_CaseName.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_SEARCH_SEARCH_ON].tnum                = LoadDDBitmap(game, "t2d\\Computer_Search_Search_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_SEARCH_NAME_ON].tnum                  = LoadDDBitmap(game, "t2d\\Computer_Search_Name_Text_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME_ON].tnum               = LoadDDBitmap(game, "t2d\\Computer_Search_SurName_Text_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_ON].tnum              = LoadDDBitmap(game, "t2d\\Computer_Search_CaseName_Text_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_SEARCH_YEAR_ON].tnum                  = LoadDDBitmap(game, "t2d\\Computer_Search_Year_Text_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_SEARCH_EXIT_ON].tnum                  = LoadDDBitmap(game, "t2d\\Computer_Exit_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_SEARCH_ALPHABETICAL_WIN].px = 0;
			w->bm[T2D_BM_COMPUTER_SEARCH_ALPHABETICAL_WIN].py = 0;
			w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_WIN].px = 0;
			w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_WIN].py = 0;
			w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_WIN].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_SEARCH_SEARCH_ON].px = 282;
			w->bm[T2D_BM_COMPUTER_SEARCH_SEARCH_ON].py = 257;
			w->bm[T2D_BM_COMPUTER_SEARCH_SEARCH_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_SEARCH_NAME_ON].px = 130;
			w->bm[T2D_BM_COMPUTER_SEARCH_NAME_ON].py = 147;
			w->bm[T2D_BM_COMPUTER_SEARCH_NAME_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME_ON].px = 406;
			w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME_ON].py = 147;
			w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_ON].px = 184;
			w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_ON].py = 147;
			w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_SEARCH_YEAR_ON].px = 502;
			w->bm[T2D_BM_COMPUTER_SEARCH_YEAR_ON].py = 147;
			w->bm[T2D_BM_COMPUTER_SEARCH_YEAR_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_SEARCH_EXIT_ON].px = 653;
			w->bm[T2D_BM_COMPUTER_SEARCH_EXIT_ON].py = 7;
			w->bm[T2D_BM_COMPUTER_SEARCH_EXIT_ON].tnum |= T2D_BM_OFF;

			b = &w->bt[T2D_BT_COMPUTER_SEARCH_WIN];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_SEARCH_ALPHABETICAL_WIN]);
			b = &w->bt[T2D_BT_COMPUTER_SEARCH_MOVE];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_SEARCH_ALPHABETICAL_WIN]);
			b->lim.y2 = 30;
			b = &w->bt[T2D_BT_COMPUTER_SEARCH_SEARCH];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_SEARCH_SEARCH_ON]);
			b->tnum = EXTRALS_SEARCH;
			b = &w->bt[T2D_BT_COMPUTER_SEARCH_ALPHABETICAL];
			b->on = true;
			b->lim.x1 = 38;
			b->lim.y1 = 51;
			b->lim.x2 = 168;
			b->lim.y2 = 83;
			b->tnum = EXTRALS_NAME;
			b = &w->bt[T2D_BT_COMPUTER_SEARCH_CASENAME];
			b->on = true;
			b->lim.x1 = 166;
			b->lim.y1 = 51;
			b->lim.x2 = 291;
			b->lim.y2 = 83;
			b->tnum = EXTRALS_SUBJECT;
			b = &w->bt[T2D_BT_COMPUTER_SEARCH_NAME_TEXT];
			b->on = true;
			b->on = true;
			b->lim.x1 = 133;
			b->lim.y1 = 150;
			b->lim.x2 = 298;
			b->lim.y2 = 172;
			b = &w->bt[T2D_BT_COMPUTER_SEARCH_SURNAME_TEXT];
			b->on = true;
			b->on = true;
			b->lim.x1 = 409;
			b->lim.y1 = 150;
			b->lim.x2 = 574;
			b->lim.y2 = 172;
			b = &w->bt[T2D_BT_COMPUTER_SEARCH_CASENAME_TEXT];
			b->lim.x1 = 187;
			b->lim.y1 = 150;
			b->lim.x2 = 352;
			b->lim.y2 = 172;
			b = &w->bt[T2D_BT_COMPUTER_SEARCH_YEAR_TEXT];
			b->lim.x1 = 505;
			b->lim.y1 = 150;
			b->lim.x2 = 593;
			b->lim.y2 = 172;
			b = &w->bt[T2D_BT_COMPUTER_SEARCH_EXIT];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_SEARCH_EXIT_ON]);

			//Gestione Scritte
			w->bm[T2D_BM_COMPUTER_SEARCH_NAME].tnum = (T2D_TEXT_COMPUTER_SEARCH_NAME << T2D_BM2TEXT_SHIFT);
			w->bm[T2D_BM_COMPUTER_SEARCH_NAME].px = 140;
			w->bm[T2D_BM_COMPUTER_SEARCH_NAME].py = 154;
			strcpy(w->text[T2D_TEXT_COMPUTER_SEARCH_NAME].text, "");
			w->text[T2D_TEXT_COMPUTER_SEARCH_NAME].font = FontKind::Computer;
			w->text[T2D_TEXT_COMPUTER_SEARCH_NAME].color = BLACK_FONT;

			w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME].tnum = (T2D_TEXT_COMPUTER_SEARCH_SURNAME << T2D_BM2TEXT_SHIFT);
			w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME].px = 420;
			w->bm[T2D_BM_COMPUTER_SEARCH_SURNAME].py = 154;
			strcpy(w->text[T2D_TEXT_COMPUTER_SEARCH_SURNAME].text, "");
			w->text[T2D_TEXT_COMPUTER_SEARCH_SURNAME].font = FontKind::Computer;
			w->text[T2D_TEXT_COMPUTER_SEARCH_SURNAME].color = BLACK_FONT;

			w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME].tnum = (T2D_TEXT_COMPUTER_SEARCH_CASENAME << T2D_BM2TEXT_SHIFT);
			w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME].px = 200;
			w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME].py = 154;
			w->bm[T2D_BM_COMPUTER_SEARCH_CASENAME].tnum |= T2D_BM_OFF;
			strcpy(w->text[T2D_TEXT_COMPUTER_SEARCH_CASENAME].text, "");
			w->text[T2D_TEXT_COMPUTER_SEARCH_CASENAME].font = FontKind::Computer;
			w->text[T2D_TEXT_COMPUTER_SEARCH_CASENAME].color = BLACK_FONT;

			w->bm[T2D_BM_COMPUTER_SEARCH_YEAR].tnum = (T2D_TEXT_COMPUTER_SEARCH_YEAR << T2D_BM2TEXT_SHIFT);
			w->bm[T2D_BM_COMPUTER_SEARCH_YEAR].px = 515;
			w->bm[T2D_BM_COMPUTER_SEARCH_YEAR].py = 154;
			w->bm[T2D_BM_COMPUTER_SEARCH_YEAR].tnum |= T2D_BM_OFF;
			strcpy(w->text[T2D_TEXT_COMPUTER_SEARCH_YEAR].text, "");
			w->text[T2D_TEXT_COMPUTER_SEARCH_YEAR].font = FontKind::Computer;
			w->text[T2D_TEXT_COMPUTER_SEARCH_YEAR].color = BLACK_FONT;

			//Gestione Variabili
			AlphaOrCase = 1;
			WhatText = 0;

			//ExtraLS
			b = &w->bt[T2D_BT_Search_Alphabetical_TGA__search];
			b->lim.x1 = 319;
			b->lim.y1 = 9;
			b->lim.x2 = 370;
			b->lim.y2 = 23;
			b->tnum = EXTRALS_SEARCH;
			b->on = true;

			b = &w->bt[T2D_BT_Search_Alphabetical_TGA__findby];
			b->lim.x1 = 36;
			b->lim.y1 = 32;
			b->lim.x2 = 98;
			b->lim.y2 = 48;
			b->tnum = EXTRALS_FINDBY;
			b->on = true;

			b = &w->bt[T2D_BT_Search_Alphabetical_TGA__name];
			b->lim.x1 = 72;
			b->lim.y1 = 152;
			b->lim.x2 = 121;
			b->lim.y2 = 170;
			b->tnum = EXTRALS_NAME;
			b->on = true;

			b = &w->bt[T2D_BT_Search_Alphabetical_TGA__surname];
			b->lim.x1 = 318;
			b->lim.y1 = 152;
			b->lim.x2 = 398;
			b->lim.y2 = 170;
			b->tnum = EXTRALS_SURNAME;
			b->on = true;

			b = &w->bt[T2D_BT_Search_Alphabetical_TGA__casename];
			b->lim.x1 = 72;
			b->lim.y1 = 152;
			b->lim.x2 = 175;
			b->lim.y2 = 169;
			b->tnum = EXTRALS_CASENAME;
			b->on = false;

			b = &w->bt[T2D_BT_Search_Alphabetical_TGA__year];
			b->lim.x1 = 448;
			b->lim.y1 = 152;
			b->lim.x2 = 496;
			b->lim.y2 = 169;
			b->tnum = EXTRALS_YEAR;
			b->on = false;
			//Fine Inizializzazione finestra Search

			//Inizializzazione finestra EMail
			w = &t2dWin[T2D_WIN_COMPUTER_EMAIL];
			w->NOTSTRETCHED = false;
			w->px = 50;
			w->py = 100;
			w->bm[T2D_BM_COMPUTER_EMAIL_WIN].tnum           = LoadDDBitmap(game, "t2d\\Computer_EMail_Win.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_EMAIL_EXIT_ON].tnum       = LoadDDBitmap(game, "t2d\\Computer_Exit_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_EMAIL_ICON_1].tnum        = LoadDDBitmap(game, "t2d\\Computer_EMail_Icon.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_EMAIL_ICON_2].tnum        = LoadDDBitmap(game, "t2d\\Computer_EMail_Icon.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_EMAIL_WIN].px = 0;
			w->bm[T2D_BM_COMPUTER_EMAIL_WIN].py = 0;
			w->bm[T2D_BM_COMPUTER_EMAIL_EXIT_ON].px = 654;
			w->bm[T2D_BM_COMPUTER_EMAIL_EXIT_ON].py = 8;
			w->bm[T2D_BM_COMPUTER_EMAIL_EXIT_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_EMAIL_ICON_1].px = 60;
			w->bm[T2D_BM_COMPUTER_EMAIL_ICON_1].py = 110;
			w->bm[T2D_BM_COMPUTER_EMAIL_ICON_2].px = 60;
			w->bm[T2D_BM_COMPUTER_EMAIL_ICON_2].py = 170;

			//Gestione Tasti
			b = &w->bt[T2D_BT_COMPUTER_EMAIL_WIN];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_EMAIL_WIN]);
			b = &w->bt[T2D_BT_COMPUTER_EMAIL_MOVE];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_EMAIL_WIN]);
			b->lim.y2 = 30;
			b = &w->bt[T2D_BT_COMPUTER_EMAIL_EXIT];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_EMAIL_EXIT_ON]);
			b = &w->bt[T2D_BT_COMPUTER_EMAIL_SCROLLUP];
			b->on = true;
			b->lim.x1 = 612;
			b->lim.y1 = 82;
			b->lim.x2 = 627;
			b->lim.y2 = 98;
			b = &w->bt[T2D_BT_COMPUTER_EMAIL_SCROLLDOWN];
			b->on = true;
			b->lim.x1 = 612;
			b->lim.y1 = 248;
			b->lim.x2 = 627;
			b->lim.y2 = 263;
			b = &w->bt[T2D_BT_COMPUTER_EMAIL_ICON_1];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_EMAIL_ICON_1]);
			b = &w->bt[T2D_BT_COMPUTER_EMAIL_ICON_2];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_EMAIL_ICON_2]);

			//Gestione Scritte
			w->bm[T2D_BM_COMPUTER_EMAIL_TEXT_1].tnum = (T2D_TEXT_COMPUTER_EMAIL_1 << T2D_BM2TEXT_SHIFT);
			w->bm[T2D_BM_COMPUTER_EMAIL_TEXT_1].px = 150;
			w->bm[T2D_BM_COMPUTER_EMAIL_TEXT_1].py = 125;
			strcpy(w->text[T2D_TEXT_COMPUTER_EMAIL_1].text, Sentence[init.Obj[oCOMPUTEREMAIL1].text[0]]);
			w->text[T2D_TEXT_COMPUTER_EMAIL_1].font = FontKind::Computer;
			w->text[T2D_TEXT_COMPUTER_EMAIL_1].color = BLACK_FONT;

			w->bm[T2D_BM_COMPUTER_EMAIL_TEXT_2].tnum = (T2D_TEXT_COMPUTER_EMAIL_2 << T2D_BM2TEXT_SHIFT);
			w->bm[T2D_BM_COMPUTER_EMAIL_TEXT_2].px = 150;
			w->bm[T2D_BM_COMPUTER_EMAIL_TEXT_2].py = 185;
			strcpy(w->text[T2D_TEXT_COMPUTER_EMAIL_2].text, Sentence[init.Obj[oCOMPUTEREMAIL2].text[0]]);
			w->text[T2D_TEXT_COMPUTER_EMAIL_2].font = FontKind::Computer;
			w->text[T2D_TEXT_COMPUTER_EMAIL_2].color = BLACK_FONT;

			//ExtraLS
			b = &w->bt[T2D_BT_EMail_Win_TGA__email];
			b->lim.x1 = 313;
			b->lim.y1 = 10;
			b->lim.x2 = 371;
			b->lim.y2 = 24;
			b->tnum = EXTRALS_EMAIL;
			b->on = true;

			b = &w->bt[T2D_BT_EMail_Win_TGA__avail];
			b->lim.x1 = 26;
			b->lim.y1 = 51;
			b->lim.x2 = 187;
			b->lim.y2 = 67;
			b->tnum = EXTRALS_AVAILABLEEMAIL;
			b->on = true;
			//Fine Inizializzazione finestra EMail

			//Inizializzazione finestra Document
			w = &t2dWin[T2D_WIN_COMPUTER_DOCUMENT];
			w->NOTSTRETCHED = true;
			w->px = 100;
			w->py = 100;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_WIN].tnum                = LoadDDBitmap(game, "t2d\\Computer_Document_Win.tga", 0);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_SCROLLUP_ON].tnum        = LoadDDBitmap(game, "t2d\\Computer_Document_ScrollUp_On.tga", 0);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_SCROLLDOWN_ON].tnum      = LoadDDBitmap(game, "t2d\\Computer_Document_ScrollDown_On.tga", 0);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_EXIT_ON].tnum            = LoadDDBitmap(game, "t2d\\Computer_Exit_On.tga", 0);

			/*                  w->bm[T2D_BM_COMPUTER_DOCUMENT_ANTONIOCORONA].tnum      = LoadDDBitmap(game, "t2d\\Computer_Document_AntonioCorona.tga", 0 );
			                    w->bm[T2D_BM_COMPUTER_DOCUMENT_JACOBKRENN].tnum         = LoadDDBitmap(game, "t2d\\Computer_Document_JacobKrenn.tga", 0 );
			                    w->bm[T2D_BM_COMPUTER_DOCUMENT_PETERDUKES].tnum         = LoadDDBitmap(game, "t2d\\Computer_Document_PeterDukes.tga", 0 );
			                    w->bm[T2D_BM_COMPUTER_DOCUMENT_VICTORVALENCIA].tnum     = LoadDDBitmap(game, "t2d\\Computer_Document_VictorValencia.tga", 0 );
			                    w->bm[T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_1].tnum     = LoadDDBitmap(game, "t2d\\Computer_Document_Mercury_1969_1.tga", 0 );
			                    w->bm[T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_2].tnum     = LoadDDBitmap(game, "t2d\\Computer_Document_Mercury_1969_2.tga", 0 );
			                    w->bm[T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_3].tnum     = LoadDDBitmap(game, "t2d\\Computer_Document_Mercury_1969_3.tga", 0 );
			                    w->bm[T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_4].tnum     = LoadDDBitmap(game, "t2d\\Computer_Document_Mercury_1969_4.tga", 0 );
			                    w->bm[T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_1].tnum  = LoadDDBitmap(game, "t2d\\Computer_Document_Confidence_1942_1.tga", 0 );
			                    w->bm[T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_2].tnum  = LoadDDBitmap(game, "t2d\\Computer_Document_Confidence_1942_2.tga", 0 );
			                    w->bm[T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_3].tnum  = LoadDDBitmap(game, "t2d\\Computer_Document_Confidence_1942_3.tga", 0 );
			                    w->bm[T2D_BM_COMPUTER_DOCUMENT_ARL_1998_1].tnum         = LoadDDBitmap(game, "t2d\\Computer_Document_Arl_1998_1.tga", 0 );
			                    w->bm[T2D_BM_COMPUTER_DOCUMENT_CONV_CONFIDENCE].tnum    = LoadDDBitmap(game, "t2d\\Computer_Document_Conv_Confidence.tga", 0 );
			                    w->bm[T2D_BM_COMPUTER_DOCUMENT_CONV_MERCURY].tnum       = LoadDDBitmap(game, "t2d\\Computer_Document_Conv_Mercury.tga", 0 );
			                    w->bm[T2D_BM_COMPUTER_DOCUMENT_GREGOR_MEM].tnum         = LoadDDBitmap(game, "t2d\\Computer_Document_GregorMem.tga", 0 );
			*/
			w->bm[T2D_BM_COMPUTER_DOCUMENT_WIN].px = renderer.rInvFitX(0);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_WIN].py = renderer.rInvFitY(0);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_SCROLLUP_ON].px = renderer.rInvFitX(461);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_SCROLLUP_ON].py = renderer.rInvFitY(57);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_SCROLLUP_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_SCROLLDOWN_ON].px = renderer.rInvFitX(461);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_SCROLLDOWN_ON].py = renderer.rInvFitY(437);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_SCROLLDOWN_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_EXIT_ON].px = renderer.rInvFitX(478);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_EXIT_ON].py = renderer.rInvFitY(9);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_EXIT_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_ANTONIOCORONA].px = renderer.rInvFitX(26);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_ANTONIOCORONA].py = renderer.rInvFitY(66);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_ANTONIOCORONA].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_JACOBKRENN].px = renderer.rInvFitX(26);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_JACOBKRENN].py = renderer.rInvFitY(66);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_JACOBKRENN].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_PETERDUKES].px = renderer.rInvFitX(26);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_PETERDUKES].py = renderer.rInvFitY(66);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_PETERDUKES].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_VICTORVALENCIA].px = renderer.rInvFitX(26);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_VICTORVALENCIA].py = renderer.rInvFitY(66);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_VICTORVALENCIA].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_1].px = renderer.rInvFitX(26);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_1].py = renderer.rInvFitY(66);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_1].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_2].px = renderer.rInvFitX(26);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_2].py = renderer.rInvFitY(66);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_2].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_3].px = renderer.rInvFitX(26);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_3].py = renderer.rInvFitY(66);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_3].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_4].px = renderer.rInvFitX(26);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_4].py = renderer.rInvFitY(66);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_4].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_1].px = renderer.rInvFitX(26);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_1].py = renderer.rInvFitY(66);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_1].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_2].px = renderer.rInvFitX(26);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_2].py = renderer.rInvFitY(66);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_2].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_3].px = renderer.rInvFitX(26);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_3].py = renderer.rInvFitY(66);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_3].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_ARL_1998_1].px = renderer.rInvFitX(26);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_ARL_1998_1].py = renderer.rInvFitY(66);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_ARL_1998_1].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_CONV_CONFIDENCE].px = renderer.rInvFitX(26);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_CONV_CONFIDENCE].py = renderer.rInvFitY(66);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_CONV_CONFIDENCE].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_CONV_MERCURY].px = renderer.rInvFitX(26);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_CONV_MERCURY].py = renderer.rInvFitY(66);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_CONV_MERCURY].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCUMENT_GREGOR_MEM].px = renderer.rInvFitX(26);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_GREGOR_MEM].py = renderer.rInvFitY(66);
			w->bm[T2D_BM_COMPUTER_DOCUMENT_GREGOR_MEM].tnum |= T2D_BM_OFF;

			b = &w->bt[T2D_BT_COMPUTER_DOCUMENT_WIN];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DOCUMENT_WIN]);
			b = &w->bt[T2D_BT_COMPUTER_DOCUMENT_MOVE];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DOCUMENT_WIN]);
			b->lim.y2 = 30;
			b = &w->bt[T2D_BT_COMPUTER_DOCUMENT_SCROLL];
			b->on = true;
			b->lim.x1 = renderer.rInvFitX(26);
			b->lim.y1 = renderer.rInvFitY(66);
			b->lim.x2 = renderer.rInvFitX(456);
			b->lim.y2 = renderer.rInvFitY(473);
			b = &w->bt[T2D_BT_COMPUTER_DOCUMENT_EXIT];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DOCUMENT_EXIT_ON]);
			b = &w->bt[T2D_BT_COMPUTER_DOCUMENT_SCROLLUP];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DOCUMENT_SCROLLUP_ON]);
			b = &w->bt[T2D_BT_COMPUTER_DOCUMENT_SCROLLDOWN];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DOCUMENT_SCROLLDOWN_ON]);

			//ExtraLS
			b = &w->bt[T2D_BT_Document_Win_TGA];
			b->lim.x1 = 176;
			b->lim.y1 = 12;
			b->lim.x2 = 304;
			b->lim.y2 = 29;
			b->tnum = EXTRALS_DOCUMENTVIEWER;
			b->on = true;
			//Fine Inizializzazione finestra Document

			//Inizializzazione finestra DocList
			w = &t2dWin[T2D_WIN_COMPUTER_DOCLIST];
			w->NOTSTRETCHED = false;
			w->px = 100;
			w->py = 150;
			w->bm[T2D_BM_COMPUTER_DOCLIST_WIN].tnum             = LoadDDBitmap(game, "t2d\\Computer_DocList_Win.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DOCLIST_EXIT_ON].tnum         = LoadDDBitmap(game, "t2d\\Computer_Exit_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DOCLIST_SCROLLUP_ON].tnum     = LoadDDBitmap(game, "t2d\\Computer_Document_ScrollUp_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DOCLIST_SCROLLDOWN_ON].tnum   = LoadDDBitmap(game, "t2d\\Computer_Document_ScrollDown_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_COMPUTER_DOCLIST_WIN].px = 0;
			w->bm[T2D_BM_COMPUTER_DOCLIST_WIN].py = 0;
			w->bm[T2D_BM_COMPUTER_DOCLIST_EXIT_ON].px = 274;
			w->bm[T2D_BM_COMPUTER_DOCLIST_EXIT_ON].py = 9;
			w->bm[T2D_BM_COMPUTER_DOCLIST_EXIT_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCLIST_SCROLLUP_ON].px = 245;
			w->bm[T2D_BM_COMPUTER_DOCLIST_SCROLLUP_ON].py = 52;
			w->bm[T2D_BM_COMPUTER_DOCLIST_SCROLLUP_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_DOCLIST_SCROLLDOWN_ON].px = 245;
			w->bm[T2D_BM_COMPUTER_DOCLIST_SCROLLDOWN_ON].py = 209;
			w->bm[T2D_BM_COMPUTER_DOCLIST_SCROLLDOWN_ON].tnum |= T2D_BM_OFF;

			//Tasti
			b = &w->bt[T2D_BT_COMPUTER_DOCLIST_WIN];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DOCLIST_WIN]);
			b = &w->bt[T2D_BT_COMPUTER_DOCLIST_MOVE];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DOCLIST_WIN]);
			b->lim.y2 = 30;
			b = &w->bt[T2D_BT_COMPUTER_DOCLIST_EXIT];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DOCLIST_EXIT_ON]);
			b = &w->bt[T2D_BT_COMPUTER_DOCLIST_SCROLLUP];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DOCLIST_SCROLLUP_ON]);
			b = &w->bt[T2D_BT_COMPUTER_DOCLIST_SCROLLDOWN];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_DOCLIST_SCROLLDOWN_ON]);

			for (i = 0; i <= T2D_TEXT_COMPUTER_DOCLIST_LINES_END - T2D_TEXT_COMPUTER_DOCLIST_LINES_START; i++) {
				b = &w->bt[T2D_BT_COMPUTER_DOCLIST_LINES_START + i];
				b->on = true;
				b->lim.x1 = 37;
				b->lim.y1 = 60 + 20 * i;
				b->lim.x2 = 235;
				b->lim.y2 = 80 + 20 * i;

				w->bm[T2D_BM_COMPUTER_DOCLIST_LINES_START + i].tnum = ((T2D_TEXT_COMPUTER_DOCLIST_LINES_START + i) << T2D_BM2TEXT_SHIFT);
				w->bm[T2D_BM_COMPUTER_DOCLIST_LINES_START + i].px = 45;
				w->bm[T2D_BM_COMPUTER_DOCLIST_LINES_START + i].py = 65 + 20 * i;

				strcpy(w->text[T2D_TEXT_COMPUTER_DOCLIST_LINES_START + i].text, "");
				w->text[T2D_TEXT_COMPUTER_DOCLIST_LINES_START + i].font = FontKind::PDA;
				w->text[T2D_TEXT_COMPUTER_DOCLIST_LINES_START + i].color = BLUE_FONT;
			}

			//ExtraLS
			b = &w->bt[T2D_BT_DocList_Win_TGA];
			b->lim.x1 = 32;
			b->lim.y1 = 29;
			b->lim.x2 = 113;
			b->lim.y2 = 45;
			b->tnum = EXTRALS_DOCUMENTS;
			b->on = true;
			//Fine Inizializzazione finestra DocList

			//Inizializzazione finestra EMail Viewer
			w = &t2dWin[T2D_WIN_COMPUTER_EMAIL_VIEWER];
			w->NOTSTRETCHED = true;
			w->px = 100;
			w->py = 100;
			w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_WIN].tnum                = LoadDDBitmap(game, "t2d\\Computer_EMail_Viewer_Win.tga", 0);
			w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_SCROLLUP_ON].tnum        = LoadDDBitmap(game, "t2d\\Computer_Document_ScrollUp_On.tga", 0);
			w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_SCROLLDOWN_ON].tnum      = LoadDDBitmap(game, "t2d\\Computer_Document_ScrollDown_On.tga", 0);
			w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_EXIT_ON].tnum            = LoadDDBitmap(game, "t2d\\Computer_Exit_On.tga", 0);
			w->bm[T2D_BM_COMPUTER_EMAIL_1].tnum                         = LoadDDBitmap(game, "t2d\\Computer_EMail_1.tga", 0);
			w->bm[T2D_BM_COMPUTER_EMAIL_2].tnum                         = LoadDDBitmap(game, "t2d\\Computer_EMail_2.tga", 0);
			w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_SCROLLUP_ON].px = renderer.rInvFitX(486);
			w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_SCROLLUP_ON].py = renderer.rInvFitY(46);
			w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_SCROLLUP_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_SCROLLDOWN_ON].px = renderer.rInvFitX(486);
			w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_SCROLLDOWN_ON].py = renderer.rInvFitY(209);
			w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_SCROLLDOWN_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_EXIT_ON].px = renderer.rInvFitX(500);
			w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_EXIT_ON].py = renderer.rInvFitY(7);
			w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_EXIT_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_EMAIL_1].px = renderer.rInvFitX(35);
			w->bm[T2D_BM_COMPUTER_EMAIL_1].py = renderer.rInvFitY(59);
			w->bm[T2D_BM_COMPUTER_EMAIL_1].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_EMAIL_2].px = renderer.rInvFitX(35);
			w->bm[T2D_BM_COMPUTER_EMAIL_2].py = renderer.rInvFitY(59);
			w->bm[T2D_BM_COMPUTER_EMAIL_2].tnum |= T2D_BM_OFF;

			b = &w->bt[T2D_BT_COMPUTER_EMAIL_VIEWER_WIN];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_WIN]);
			b = &w->bt[T2D_BT_COMPUTER_EMAIL_VIEWER_MOVE];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_WIN]);
			b->lim.y2 = 30;
			b = &w->bt[T2D_BT_COMPUTER_EMAIL_VIEWER_SCROLL];
			b->on = true;
			b->lim.x1 = 35;
			b->lim.y1 = 59;
			b->lim.x2 = 465;
			b->lim.y2 = 240;
			b = &w->bt[T2D_BT_COMPUTER_EMAIL_VIEWER_SCROLLUP];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_SCROLLUP_ON]);
			b = &w->bt[T2D_BT_COMPUTER_EMAIL_VIEWER_SCROLLDOWN];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_SCROLLDOWN_ON]);
			b = &w->bt[T2D_BT_COMPUTER_EMAIL_VIEWER_EXIT];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_COMPUTER_EMAIL_VIEWER_EXIT_ON]);

			//ExtraLS
			b = &w->bt[T2D_BT_EMail_Viewer_Win_TGA];
			b->lim.x1 = 185;
			b->lim.y1 = 9;
			b->lim.x2 = 307;
			b->lim.y2 = 25;
			b->tnum = EXTRALS_EMAILVIEWER;
			b->on = true;
			//Fine Inizializzazione finestra EMail Viewer

			WinActive[0] = T2D_WIN_COMPUTER_BACKGROUND;
			WinActive[1] = T2D_WIN_COMPUTER_DIALUP;
			//bIngnoreDIKeyboard = true;
			CurButton = 0;
			CurButtonWin = 0;
			LinkWinToMouse = 0;
			LinkWinX = 0;
			LinkWinY = 0;
			ScrollWithMouse = 0;
			computerCurrentDocument = -1;
			computerCurrentEMail = -1;
			computerNumeroTel[0] = 0;
			computerConnectStr[0] = 0;
			searchName.clear();
			searchSurName.clear();
			searchCaseName.clear();
			searchYear.clear();


			/*//Scritte Iniziali
			w = &t2dWin[T2D_WIN_COMPUTER_BACKGROUND];

			strcpy( w->text[T2D_TEXT_COMPUTER_SCRITTE_1].text, "Loading CIAOS" );
			strcpy( w->text[T2D_TEXT_COMPUTER_SCRITTE_2].text, "Loading Dialup" );
			strcpy( w->text[T2D_TEXT_COMPUTER_SCRITTE_3].text, "Insert Telephone Number" );

			Event( EventClass::MC_T2D, ME_T2DSETBITMAP, MP_DEFAULT, T2D_WIN_COMPUTER_BACKGROUND, T2D_BM_COMPUTER_SCRITTE_1, TRUE, NULL, NULL, NULL );
			Event( EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_WRITE_DELAY, 0, EFFECT_WAIT, NULL, NULL, NULL );
			Event( EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_BACKGROUND, T2D_BM_COMPUTER_SCRITTE_2, TRUE, NULL, NULL, NULL );
			Event( EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_WRITE_DELAY, 0, EFFECT_WAIT, NULL, NULL, NULL );
			Event( EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_BACKGROUND, T2D_BM_COMPUTER_SCRITTE_3, TRUE, NULL, NULL, NULL );
			Event( EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_WRITE_DELAY, 0, EFFECT_WAIT, NULL, NULL, NULL );
			Event( EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_BACKGROUND, T2D_BM_COMPUTER_SCRITTE_1, FALSE, NULL, NULL, NULL );
			Event( EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_BACKGROUND, T2D_BM_COMPUTER_SCRITTE_2, FALSE, NULL, NULL, NULL );
			Event( EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_BACKGROUND, T2D_BM_COMPUTER_SCRITTE_3, FALSE, NULL, NULL, NULL );

			w->bm[T2D_BM_COMPUTER_SCRITTE_1].tnum &= ~T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_SCRITTE_2].tnum &= ~T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_SCRITTE_3].tnum &= ~T2D_BM_OFF;

			Event( EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_WRITE_DELAY, 0, EFFECT_WAIT, NULL, NULL, NULL );

			w->bm[T2D_BM_COMPUTER_SCRITTE_1].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_SCRITTE_2].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_COMPUTER_SCRITTE_3].tnum |= T2D_BM_OFF;

			Event( EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_BACKGROUND, T2D_BM_COMPUTER_SCRITTE_1, FALSE, NULL, NULL, NULL );
			Event( EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_BACKGROUND, T2D_BM_COMPUTER_SCRITTE_2, FALSE, NULL, NULL, NULL );
			Event( EventClass::MC_T2D, ME_T2DSETBITMAP, MP_WAITA, T2D_WIN_COMPUTER_BACKGROUND, T2D_BM_COMPUTER_SCRITTE_3, FALSE, NULL, NULL, NULL );

			Event( EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_DIALUP, 0, TRUE, NULL, NULL, NULL );*/

			break;

		case tDIARIO:
			w = &t2dWin[T2D_WIN_DIARIO];
			w->NOTSTRETCHED = false;
			w->px = 0;
			w->py = 0;
			w->bm[T2D_BM_DIARIO_PAGE_1].tnum            = LoadDDBitmap(game, "t2d\\Diario_Page1.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_2].tnum            = LoadDDBitmap(game, "t2d\\Diario_Page2.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_3].tnum            = LoadDDBitmap(game, "t2d\\Diario_Page3.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_4].tnum            = LoadDDBitmap(game, "t2d\\Diario_Page4.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_LEFT_ON].tnum      = LoadDDBitmap(game, "t2d\\Diario_Page_Left_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_RIGHT_ON].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page_Right_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_EXIT_ON].tnum           = LoadDDBitmap(game, "t2d\\Diario_Exit_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_1].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page1_Text1.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_2].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page1_Text2.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_3].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page1_Text3.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_4].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page1_Text4.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_5].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page1_Text5.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_6].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page1_Text6.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_1].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page2_Text1.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_2].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page2_Text2.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_3].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page2_Text3.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_4].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page2_Text4.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_5].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page2_Text5.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_6].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page2_Text6.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_3_TEXT_1].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page3_Text1.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_3_TEXT_2].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page3_Text2.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_3_TEXT_3].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page3_Text3.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_3_TEXT_4].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page3_Text4.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_1].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page4_Text1.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_2].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page4_Text2.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_3].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page4_Text3.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_4].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page4_Text4.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_5].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page4_Text5.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_6].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page4_Text6.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_7].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page4_Text7.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_8].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page4_Text8.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_9].tnum     = LoadDDBitmap(game, "t2d\\Diario_Page4_Text9.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_10].tnum    = LoadDDBitmap(game, "t2d\\Diario_Page4_Text10.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_11].tnum    = LoadDDBitmap(game, "t2d\\Diario_Page4_Text11.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_12].tnum    = LoadDDBitmap(game, "t2d\\Diario_Page4_Text12.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_DIARIO_PAGE_1].px = 0;
			w->bm[T2D_BM_DIARIO_PAGE_1].py = 0;
			w->bm[T2D_BM_DIARIO_PAGE_1].tnum &= ~T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_2].px = 0;
			w->bm[T2D_BM_DIARIO_PAGE_2].py = 0;
			w->bm[T2D_BM_DIARIO_PAGE_2].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_3].px = 0;
			w->bm[T2D_BM_DIARIO_PAGE_3].py = 0;
			w->bm[T2D_BM_DIARIO_PAGE_3].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_4].px = 0;
			w->bm[T2D_BM_DIARIO_PAGE_4].py = 0;
			w->bm[T2D_BM_DIARIO_PAGE_4].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_LEFT_ON].px = 17;
			w->bm[T2D_BM_DIARIO_PAGE_LEFT_ON].py = 549;
			w->bm[T2D_BM_DIARIO_PAGE_LEFT_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_RIGHT_ON].px = 739;
			w->bm[T2D_BM_DIARIO_PAGE_RIGHT_ON].py = 551;
			w->bm[T2D_BM_DIARIO_PAGE_RIGHT_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_EXIT_ON].px = 369;
			w->bm[T2D_BM_DIARIO_EXIT_ON].py = 20;
			w->bm[T2D_BM_DIARIO_EXIT_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_1].px = 65;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_1].py = 76;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_1].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_2].px = 157;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_2].py = 227;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_2].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_3].px = 52;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_3].py = 322;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_3].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_4].px = 427;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_4].py = 83;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_4].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_5].px = 527;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_5].py = 173;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_5].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_6].px = 441;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_6].py = 336;
			w->bm[T2D_BM_DIARIO_PAGE_1_TEXT_6].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_1].px = 51;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_1].py = 52;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_1].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_2].px = 178;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_2].py = 172;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_2].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_3].px = 24;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_3].py = 312;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_3].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_4].px = 424;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_4].py = 67;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_4].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_5].px = 574;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_5].py = 221;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_5].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_6].px = 442;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_6].py = 437;
			w->bm[T2D_BM_DIARIO_PAGE_2_TEXT_6].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_3_TEXT_1].px = 55;
			w->bm[T2D_BM_DIARIO_PAGE_3_TEXT_1].py = 49;
			w->bm[T2D_BM_DIARIO_PAGE_3_TEXT_1].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_3_TEXT_2].px = 83;
			w->bm[T2D_BM_DIARIO_PAGE_3_TEXT_2].py = 286;
			w->bm[T2D_BM_DIARIO_PAGE_3_TEXT_2].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_3_TEXT_3].px = 462;
			w->bm[T2D_BM_DIARIO_PAGE_3_TEXT_3].py = 78;
			w->bm[T2D_BM_DIARIO_PAGE_3_TEXT_3].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_3_TEXT_4].px = 466;
			w->bm[T2D_BM_DIARIO_PAGE_3_TEXT_4].py = 319;
			w->bm[T2D_BM_DIARIO_PAGE_3_TEXT_4].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_1].px = 76;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_1].py = 78;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_1].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_2].px = 72;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_2].py = 106;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_2].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_3].px = 72;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_3].py = 140;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_3].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_4].px = 69;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_4].py = 168;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_4].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_5].px = 71;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_5].py = 201;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_5].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_6].px = 69;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_6].py = 233;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_6].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_7].px = 66;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_7].py = 264;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_7].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_8].px = 64;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_8].py = 296;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_8].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_9].px = 62;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_9].py = 324;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_9].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_10].px = 58;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_10].py = 357;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_10].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_11].px = 58;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_11].py = 390;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_11].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_12].px = 57;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_12].py = 421;
			w->bm[T2D_BM_DIARIO_PAGE_4_TEXT_12].tnum |= T2D_BM_OFF;

			b = &w->bt[T2D_BT_DIARIO_PAGE_LEFT];
			b->on = false;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_DIARIO_PAGE_LEFT_ON]);
			b = &w->bt[T2D_BT_DIARIO_PAGE_RIGHT];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_DIARIO_PAGE_RIGHT_ON]);
			b = &w->bt[T2D_BT_DIARIO_EXIT];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_DIARIO_EXIT_ON]);
			b->tnum = EXTRALS_EXIT;
			for (i = T2D_BT_DIARIO_PAGE_1_TEXT_1; i <= T2D_BT_DIARIO_PAGE_1_TEXT_6; i++) {
				b = &w->bt[i];
				b->on = true;
				GetDDBitmapExtends(renderer, &b->lim, &w->bm[i]);
			}
			for (i = T2D_BT_DIARIO_PAGE_2_TEXT_1; i <= T2D_BT_DIARIO_PAGE_2_TEXT_6; i++) {
				b = &w->bt[i];
				b->on = false;
				GetDDBitmapExtends(renderer, &b->lim, &w->bm[i]);
			}
			for (i = T2D_BT_DIARIO_PAGE_3_TEXT_1; i <= T2D_BT_DIARIO_PAGE_3_TEXT_4; i++) {
				b = &w->bt[i];
				b->on = false;
				GetDDBitmapExtends(renderer, &b->lim, &w->bm[i]);
			}
			for (i = T2D_BT_DIARIO_PAGE_4_TEXT_1; i <= T2D_BT_DIARIO_PAGE_4_TEXT_12; i++) {
				b = &w->bt[i];
				b->on = false;
				GetDDBitmapExtends(renderer, &b->lim, &w->bm[i]);
			}

			PlayerSpeak(game, init.Obj[o34DIARIOAP].action[CurPlayer]);
			//else  PlayerSpeak( init.Obj[oDIARIOAP+(CurButton-T2D_BT_DIARIO_PAGE_2_TEXT_1)].examine[CurPlayer] );

			WinActive[0] = T2D_WIN_DIARIO;
			CurButton = 0;
			CurButtonWin = 0;
			LinkWinToMouse = 0;
			LinkWinX = 0;
			LinkWinY = 0;

			diarioCurrentPage = 1;
			break;

		case tPDA:
			//Inizializzazione finestra Main
			w = &t2dWin[T2D_WIN_PDA_MAIN];
			w->NOTSTRETCHED = false;
			w->px = 0;
			w->py = 0;
			w->bm[T2D_BM_PDA_MAIN_WIN].tnum         = LoadDDBitmap(game, "t2d\\PDA_Main_Win.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_MAIN_TONE_ON].tnum     = LoadDDBitmap(game, "t2d\\PDA_Main_Tone_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_MAIN_LOG_ON].tnum      = LoadDDBitmap(game, "t2d\\PDA_Main_Log_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_MAIN_QUIT_ON].tnum     = LoadDDBitmap(game, "t2d\\PDA_Main_Quit_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_MAIN_WIN].px = 0;
			w->bm[T2D_BM_PDA_MAIN_WIN].py = 0;
			w->bm[T2D_BM_PDA_MAIN_WIN].tnum &= ~T2D_BM_OFF;
			w->bm[T2D_BM_PDA_MAIN_TONE_ON].px = 68;
			w->bm[T2D_BM_PDA_MAIN_TONE_ON].py = 79;
			w->bm[T2D_BM_PDA_MAIN_TONE_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_PDA_MAIN_LOG_ON].px = 99;
			w->bm[T2D_BM_PDA_MAIN_LOG_ON].py = 227;
			w->bm[T2D_BM_PDA_MAIN_LOG_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_PDA_MAIN_TIME].px = 200;
			w->bm[T2D_BM_PDA_MAIN_TIME].py = 200;
			w->bm[T2D_BM_PDA_MAIN_TIME].tnum &= ~T2D_BM_OFF;
			w->bm[T2D_BM_PDA_MAIN_QUIT_ON].px = 719;
			w->bm[T2D_BM_PDA_MAIN_QUIT_ON].py = 503;
			w->bm[T2D_BM_PDA_MAIN_QUIT_ON].tnum |= T2D_BM_OFF;

			b = &w->bt[T2D_BT_PDA_MAIN_TONE];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_PDA_MAIN_TONE_ON]);
			b->tnum = EXTRALS_TONESEQUENCER;
			b = &w->bt[T2D_BT_PDA_MAIN_LOG];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_PDA_MAIN_LOG_ON]);
			b->tnum = EXTRALS_LOG;
			b = &w->bt[T2D_BT_PDA_MAIN_QUIT];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_PDA_MAIN_QUIT_ON]);
			b->tnum = EXTRALS_QUIT;


			w->bm[T2D_BM_PDA_MAIN_TIME].tnum |= ((T2D_TEXT_PDA_MAIN_TIME) << T2D_BM2TEXT_SHIFT);
			w->bm[T2D_BM_PDA_MAIN_TIME].px = 690;
			w->bm[T2D_BM_PDA_MAIN_TIME].py = 360;
			w->text[T2D_TEXT_PDA_MAIN_TIME].font = FontKind::PDA;
			w->text[T2D_TEXT_PDA_MAIN_TIME].color = GREEN_FONT;
			if (t3dCurTime >= 1300) {
				snprintf(w->text[T2D_TEXT_PDA_MAIN_TIME].text, T2D_MAX_TEXTS_IN_WIN, "%04d", t3dCurTime - 1200);
				w->text[T2D_TEXT_PDA_MAIN_TIME].text[5] = 0;
				w->text[T2D_TEXT_PDA_MAIN_TIME].text[4] = w->text[T2D_TEXT_PDA_MAIN_TIME].text[3];
				w->text[T2D_TEXT_PDA_MAIN_TIME].text[3] = w->text[T2D_TEXT_PDA_MAIN_TIME].text[2];
				w->text[T2D_TEXT_PDA_MAIN_TIME].text[2] = ':';
				strcat(w->text[T2D_TEXT_PDA_MAIN_TIME].text, " PM");
			} else {
				snprintf(w->text[T2D_TEXT_PDA_MAIN_TIME].text, T2D_MAX_TEXTS_IN_WIN, "%04d", t3dCurTime);
				w->text[T2D_TEXT_PDA_MAIN_TIME].text[5] = 0;
				w->text[T2D_TEXT_PDA_MAIN_TIME].text[4] = w->text[T2D_TEXT_PDA_MAIN_TIME].text[3];
				w->text[T2D_TEXT_PDA_MAIN_TIME].text[3] = w->text[T2D_TEXT_PDA_MAIN_TIME].text[2];
				w->text[T2D_TEXT_PDA_MAIN_TIME].text[2] = ':';
				strcat(w->text[T2D_TEXT_PDA_MAIN_TIME].text, " AM");
			}
			//Fine Inizializzazione finestra Main

			//Inizializzazione finestra Log Viewer
			w = &t2dWin[T2D_WIN_PDA_LOG];
			w->NOTSTRETCHED = false;
			w->px = 30;
			w->py = 31;
			w->bm[T2D_BM_PDA_LOG_WIN].tnum                  = LoadDDBitmap(game, "t2d\\PDA_Log_Win.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_LOG_SCROLLUP_ON].tnum          = LoadDDBitmap(game, "t2d\\PDA_ScrollUp_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_LOG_SCROLLDOWN_ON].tnum        = LoadDDBitmap(game, "t2d\\PDA_ScrollDown_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_LOG_SB].tnum                   = LoadDDBitmap(game, "t2d\\PDA_Log_SB.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_LOG_EXIT_ON].tnum              = LoadDDBitmap(game, "t2d\\PDA_Exit_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_LOG_WIN].px = 0;
			w->bm[T2D_BM_PDA_LOG_WIN].py = 0;
			w->bm[T2D_BM_PDA_LOG_SCROLLUP_ON].px = 696;
			w->bm[T2D_BM_PDA_LOG_SCROLLUP_ON].py = 45;
			w->bm[T2D_BM_PDA_LOG_SCROLLUP_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_PDA_LOG_SCROLLDOWN_ON].px = 696;
			w->bm[T2D_BM_PDA_LOG_SCROLLDOWN_ON].py = 333;
			w->bm[T2D_BM_PDA_LOG_SCROLLDOWN_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_PDA_LOG_SB].px = 696;
			w->bm[T2D_BM_PDA_LOG_SB].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_PDA_LOG_EXIT_ON].px = 720;
			w->bm[T2D_BM_PDA_LOG_EXIT_ON].py = 6;
			w->bm[T2D_BM_PDA_LOG_EXIT_ON].tnum |= T2D_BM_OFF;


			b = &w->bt[T2D_BT_PDA_LOG_WIN];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_PDA_LOG_WIN]);
			b = &w->bt[T2D_BT_PDA_LOG_SCROLL];
			b->on = true;
			b->lim.x1 = 23;
			b->lim.y1 = 47;
			b->lim.x2 = 692;
			b->lim.y2 = 345;
			b = &w->bt[T2D_BT_PDA_LOG_SCROLLUP];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_PDA_LOG_SCROLLUP_ON]);
			b = &w->bt[T2D_BT_PDA_LOG_SCROLLDOWN];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_PDA_LOG_SCROLLDOWN_ON]);
			b = &w->bt[T2D_BT_PDA_LOG_SB];
			b->on = false;
			b->lim.x1 = 696;
			b->lim.y1 = 0;
			b->lim.x2 = 710;
			b->lim.y2 = 0;
			b = &w->bt[T2D_BT_PDA_LOG_BARRA];
			b->on = true;
			b->lim.x1 = 696;
			b->lim.y1 = 58;
			b->lim.x2 = 711;
			b->lim.y2 = 333;
			b = &w->bt[T2D_BT_PDA_LOG_EXIT];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_PDA_LOG_EXIT_ON]);

			for (i = 0; i < T2D_BM_PDA_LOG_DATA_END - T2D_BM_PDA_LOG_DATA_START; i++) {
				w->bm[T2D_BM_PDA_LOG_DATA_START + i].tnum = ((T2D_TEXT_PDA_LOG_DATA_START + i) << T2D_BM2TEXT_SHIFT);
				w->bm[T2D_BM_PDA_LOG_DATA_START + i].px = T2D_PDA_LOG_DATA_XI;
				w->bm[T2D_BM_PDA_LOG_DATA_START + i].py = 0;
				strcpy(w->text[T2D_TEXT_PDA_LOG_DATA_START + i].text, "");
				w->text[T2D_TEXT_PDA_LOG_DATA_START + i].font = FontKind::PDA;
				w->text[T2D_TEXT_PDA_LOG_DATA_START + i].color = GRAY_FONT;
			}

			for (i = 0; i < T2D_BM_PDA_LOG_TEXT_END - T2D_BM_PDA_LOG_TEXT_START; i++) {
				w->bm[T2D_BM_PDA_LOG_TEXT_START + i].tnum = ((T2D_TEXT_PDA_LOG_TEXT_START + i) << T2D_BM2TEXT_SHIFT);
				w->bm[T2D_BM_PDA_LOG_TEXT_START + i].px = T2D_PDA_LOG_TEXT_XI;
				w->bm[T2D_BM_PDA_LOG_TEXT_START + i].py = 0;
				w->bm[T2D_BM_PDA_LOG_TEXT_START].tnum |= T2D_BM_OFF;
				strcpy(w->text[T2D_TEXT_PDA_LOG_TEXT_START + i].text, "");
				w->text[T2D_TEXT_PDA_LOG_TEXT_START + i].font = FontKind::PDA;
			}

			//ExtraLS
			b = &w->bt[T2D_BT_PDA_Log_Win_TGA];
			b->lim.x1 = 16;
			b->lim.y1 = 5;
			b->lim.x2 = 84;
			b->lim.y2 = 19;
			b->tnum = EXTRALS_LOGVIEWER;
			b->on = true;
			//Fine Inizializzazione Log Viewer

			//Inizializzazione finestra Tone Sequencer
			w = &t2dWin[T2D_WIN_PDA_TONE];
			w->NOTSTRETCHED = false;
			w->px = 30;
			w->py = 31;
			w->bm[T2D_BM_PDA_TONE_WIN].tnum                 = LoadDDBitmap(game, "t2d\\PDA_Tone_Win.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_TONE_HELP_ON].tnum             = LoadDDBitmap(game, "t2d\\PDA_Help_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_TONE_EXIT_ON].tnum             = LoadDDBitmap(game, "t2d\\PDA_Exit_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_TONE_WAVE_1].tnum              = LoadDDBitmap(game, "t2d\\PDA_Tone_Seq_231073.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_TONE_WAVE_2].tnum              = LoadDDBitmap(game, "t2d\\PDA_Tone_Seq_231074.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_TONE_REDLINE].tnum             = LoadDDBitmap(game, "t2d\\PDA_Tone_RedLine.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_TONE_BARRA_1].tnum             = LoadDDBitmap(game, "t2d\\PDA_Tone_Barra.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_TONE_BARRA_2].tnum             = LoadDDBitmap(game, "t2d\\PDA_Tone_Barra.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_TONE_ACQUIRE_ON].tnum          = LoadDDBitmap(game, "t2d\\PDA_Tone_Acquire_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_TONE_PROCESS_ON].tnum          = LoadDDBitmap(game, "t2d\\PDA_Tone_Process_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_TONE_RISULTATO].tnum           = LoadDDBitmap(game, "t2d\\PDA_Tone_Risultato.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_TONE_WIN].px = 0;
			w->bm[T2D_BM_PDA_TONE_WIN].py = 0;
			w->bm[T2D_BM_PDA_TONE_HELP_ON].px = 719;
			w->bm[T2D_BM_PDA_TONE_HELP_ON].py = 24;
			w->bm[T2D_BM_PDA_TONE_HELP_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_PDA_TONE_EXIT_ON].px = 719;
			w->bm[T2D_BM_PDA_TONE_EXIT_ON].py = 6;
			w->bm[T2D_BM_PDA_TONE_EXIT_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_PDA_TONE_WAVE_1].px = T2D_PDA_TONE_XI;
			w->bm[T2D_BM_PDA_TONE_WAVE_1].py = 46;
			w->bm[T2D_BM_PDA_TONE_WAVE_1].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_PDA_TONE_WAVE_2].px = T2D_PDA_TONE_XI;
			w->bm[T2D_BM_PDA_TONE_WAVE_2].py = 169;
			w->bm[T2D_BM_PDA_TONE_WAVE_2].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_PDA_TONE_REDLINE].px = T2D_PDA_TONE_XI;
			w->bm[T2D_BM_PDA_TONE_REDLINE].py = 46;
			w->bm[T2D_BM_PDA_TONE_REDLINE].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_PDA_TONE_BARRA_1].px = 126;
			w->bm[T2D_BM_PDA_TONE_BARRA_1].py = 307;
			w->bm[T2D_BM_PDA_TONE_BARRA_1].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_PDA_TONE_BARRA_2].px = 126;
			w->bm[T2D_BM_PDA_TONE_BARRA_2].py = 339;
			w->bm[T2D_BM_PDA_TONE_BARRA_2].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_PDA_TONE_ACQUIRE_ON].px = 37;
			w->bm[T2D_BM_PDA_TONE_ACQUIRE_ON].py = 305;
			w->bm[T2D_BM_PDA_TONE_ACQUIRE_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_PDA_TONE_PROCESS_ON].px = 37;
			w->bm[T2D_BM_PDA_TONE_PROCESS_ON].py = 335;
			w->bm[T2D_BM_PDA_TONE_PROCESS_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_PDA_TONE_RISULTATO].px = 510;
			w->bm[T2D_BM_PDA_TONE_RISULTATO].py = 320;
			w->bm[T2D_BM_PDA_TONE_RISULTATO].tnum |= T2D_BM_OFF;

			b = &w->bt[T2D_BT_PDA_TONE_WIN];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_PDA_TONE_WIN]);
			b = &w->bt[T2D_BT_PDA_TONE_HELP];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_PDA_TONE_HELP_ON]);
			b = &w->bt[T2D_BT_PDA_TONE_EXIT];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_PDA_TONE_EXIT_ON]);
			b = &w->bt[T2D_BT_PDA_TONE_ACQUIRE];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_PDA_TONE_ACQUIRE_ON]);
			b->tnum = EXTRALS_ACQUIRE;
			b = &w->bt[T2D_BT_PDA_TONE_PROCESS];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_PDA_TONE_PROCESS_ON]);
			b->tnum = EXTRALS_PROCESS;

			//ExtraLS
			b = &w->bt[T2D_BT_PDA_Tone_Win_TGA];
			b->lim.x1 = 16;
			b->lim.y1 = 5;
			b->lim.x2 = 84;
			b->lim.y2 = 19;
			b->tnum = EXTRALS_TONESEQUENCER;
			b->on = true;
			//Fine Inizializzazione finestra Tone Sequencer

			//Inizializzazione finestra Help
			w = &t2dWin[T2D_WIN_PDA_HELP];
			w->NOTSTRETCHED = false;
			w->px = 30;
			w->py = 31;
			w->bm[T2D_BM_PDA_HELP_WIN].tnum                 = LoadDDBitmap(game, "t2d\\PDA_Help_Win.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_HELP_WIN2].tnum                = LoadDDBitmap(game, "t2d\\PDA_Help_Win.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_HELP_SCROLLUP_ON].tnum         = LoadDDBitmap(game, "t2d\\PDA_ScrollUp_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_HELP_SCROLLDOWN_ON].tnum       = LoadDDBitmap(game, "t2d\\PDA_ScrollDown_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_HELP_EXIT_ON].tnum             = LoadDDBitmap(game, "t2d\\PDA_Exit_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_PDA_HELP_WIN].px = 0;
			w->bm[T2D_BM_PDA_HELP_WIN].py = 0;
			w->bm[T2D_BM_PDA_HELP_WIN2].px = 0;
			w->bm[T2D_BM_PDA_HELP_WIN2].py = 0;
			w->bm[T2D_BM_PDA_HELP_SCROLLUP_ON].px = 327;
			w->bm[T2D_BM_PDA_HELP_SCROLLUP_ON].py = 39;
			w->bm[T2D_BM_PDA_HELP_SCROLLUP_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_PDA_HELP_SCROLLDOWN_ON].px = 327;
			w->bm[T2D_BM_PDA_HELP_SCROLLDOWN_ON].py = 324;
			w->bm[T2D_BM_PDA_HELP_SCROLLDOWN_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_PDA_HELP_EXIT_ON].px = 330;
			w->bm[T2D_BM_PDA_HELP_EXIT_ON].py = 6;
			w->bm[T2D_BM_PDA_HELP_EXIT_ON].tnum |= T2D_BM_OFF;

			b = &w->bt[T2D_BT_PDA_HELP_WIN];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_PDA_LOG_WIN]);
			b = &w->bt[T2D_BT_PDA_HELP_MOVE];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_PDA_LOG_WIN]);
			b->lim.y2 = 20;
			b = &w->bt[T2D_BT_PDA_HELP_SCROLL];
			b->on = true;
			b->lim.x1 = 13;
			b->lim.y1 = 40;
			b->lim.x2 = 322;
			b->lim.y2 = 335;
			b = &w->bt[T2D_BT_PDA_HELP_EXIT];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_PDA_HELP_EXIT_ON]);

			for (i = 0; i < T2D_BM_PDA_HELP_TEXT_END - T2D_BM_PDA_HELP_TEXT_START; i++) {
				w->bm[T2D_BM_PDA_HELP_TEXT_START + i].tnum = ((T2D_TEXT_PDA_HELP_START + i) << T2D_BM2TEXT_SHIFT);
				w->bm[T2D_BM_PDA_HELP_TEXT_START + i].px = T2D_PDA_HELP_TEXT_XI;
				w->bm[T2D_BM_PDA_HELP_TEXT_START + i].py = T2D_PDA_HELP_TEXT_YI + 20 * i;
				w->bm[T2D_BM_PDA_HELP_TEXT_START].tnum |= T2D_BM_OFF;
				strcpy(w->text[T2D_TEXT_PDA_HELP_START + i].text, "");
				w->text[T2D_TEXT_PDA_HELP_START + i].font = FontKind::PDA;
				w->text[T2D_TEXT_PDA_HELP_START + i].color = GREEN_FONT;
			}

			//ExtraLS
			b = &w->bt[T2D_BT_Help_Win_TGA];
			b->lim.x1 = 19;
			b->lim.y1 = 5;
			b->lim.x2 = 49;
			b->lim.y2 = 20;
			b->tnum = EXTRALS_HELP;
			b->on = true;
			//Fine Inizializzazione finestra Help


			WinActive[0] = T2D_WIN_PDA_MAIN;
			//bIngnoreDIKeyboard = true;
			CurButton = 0;
			CurButtonWin = 0;
			LinkWinToMouse = 0;
			LinkWinX = 0;
			LinkWinY = 0;
			ScrollWithMouse = 0;
			StartAcquiring = TheMessage->lparam[0]; // i28WALKMANOK
			if (StartAcquiring == i28WALKMANOK) {
				_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_DEFAULT, T2D_WIN_PDA_TONE, 0, true, nullptr, nullptr, nullptr);
				_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DCONTINUE, MP_DEFAULT, 0, 0, T2D_PDA_TONE_AQUIRING, nullptr, nullptr, nullptr);
			}

			//Cerca il MainTitle
			for (i = 0; i < MAX_PDALOGS; i++)
				if (init.PDALog[i].flags & PDA_MAIN) MainTitle = i;

			CurrentLogPos = 0;
			while ((!(init.PDALog[CurrentLogPos].flags & ON)) && (CurrentLogPos < MAX_PDALOGS))
				CurrentLogPos++;
			NumMenus = 0;
			CurrentLogMenu = 0;
			PDAScrollLine = 0;

			RefreshLog(game, RESTARTALL);
			RefreshLog(game, REFRESH);
			break;

		case tOPTIONS: {
			int32 LoadWindow_OffsetX = 0;
			int32 LoadWindow_OffsetY = 0;

			LoadWindow_OffsetX = -100;
			if (bShowOnlyLoadWindow) {
				/*LoadWindow_OffsetX = -100; */ LoadWindow_OffsetY = -50;
				bStartMenuActive = 1;
			}

			//Inizializzazione finestra Main
			w = &t2dWin[T2D_WIN_OPTIONS_MAIN];
			w->NOTSTRETCHED = false;
			w->px = 0;
			w->py = 0;
			//Bitmaps
			w->bm[T2D_BM_OPTIONS_CONTORNO].tnum         = LoadDDBitmap(game, "t2d\\OPTIONS_Contorno.tga",      rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_SAVE].tnum             = LoadDDBitmap(game, "t2d\\OPTIONS_Save.tga",          rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_LOAD].tnum             = LoadDDBitmap(game, "t2d\\OPTIONS_Load.tga",          rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_GRIGLIA].tnum          = LoadDDBitmap(game, "t2d\\OPTIONS_Griglia.tga",           rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_FRECCIA_SU_OFF].tnum   = LoadDDBitmap(game, "t2d\\OPTIONS_Freccia_Su_Off.tga",    rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_FRECCIA_SU_ON].tnum    = LoadDDBitmap(game, "t2d\\OPTIONS_Freccia_Su_On.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_OFF].tnum  = LoadDDBitmap(game, "t2d\\OPTIONS_Freccia_Giu_Off.tga",   rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_ON].tnum   = LoadDDBitmap(game, "t2d\\OPTIONS_Freccia_Giu_On.tga",    rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_SOUND_ON].tnum         = LoadDDBitmap(game, "t2d\\OPTIONS_Sound_On.tga",      rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_MUSIC_ON].tnum         = LoadDDBitmap(game, "t2d\\OPTIONS_Music_On.tga",      rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_SPEECH_ON].tnum        = LoadDDBitmap(game, "t2d\\OPTIONS_Speech_On.tga",     rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_SUBTITLES_ON].tnum     = LoadDDBitmap(game, "t2d\\OPTIONS_Subtitles_On.tga",  rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_SOUND_OFF].tnum        = LoadDDBitmap(game, "t2d\\OPTIONS_Sound_Off.tga",     rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_MUSIC_OFF].tnum        = LoadDDBitmap(game, "t2d\\OPTIONS_Music_Off.tga",     rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_SPEECH_OFF].tnum       = LoadDDBitmap(game, "t2d\\OPTIONS_Speech_Off.tga",        rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_SUBTITLES_OFF].tnum    = LoadDDBitmap(game, "t2d\\OPTIONS_Subtitles_Off.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_SOUND_BARRA].tnum      = LoadDDBitmap(game, "t2d\\OPTIONS_Barra.tga",         rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_SOUND_PIROLO].tnum     = LoadDDBitmap(game, "t2d\\OPTIONS_Pirolo.tga",            rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_MUSIC_BARRA].tnum      = LoadDDBitmap(game, "t2d\\OPTIONS_Barra.tga",         rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_MUSIC_PIROLO].tnum     = LoadDDBitmap(game, "t2d\\OPTIONS_Pirolo.tga",            rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_SPEECH_BARRA].tnum     = LoadDDBitmap(game, "t2d\\OPTIONS_Barra.tga",         rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_SPEECH_PIROLO].tnum    = LoadDDBitmap(game, "t2d\\OPTIONS_Pirolo.tga",            rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_CONTORNO].px = LoadWindow_OffsetX + 223;
			w->bm[T2D_BM_OPTIONS_CONTORNO].py = LoadWindow_OffsetY + 59;
			w->bm[T2D_BM_OPTIONS_CONTORNO].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_SAVE].px = w->bm[T2D_BM_OPTIONS_CONTORNO].px + 506;
			w->bm[T2D_BM_OPTIONS_SAVE].py = w->bm[T2D_BM_OPTIONS_CONTORNO].py + 0;
			w->bm[T2D_BM_OPTIONS_SAVE].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_LOAD].px = w->bm[T2D_BM_OPTIONS_CONTORNO].px + 508;
			w->bm[T2D_BM_OPTIONS_LOAD].py = w->bm[T2D_BM_OPTIONS_CONTORNO].py + 0;
			w->bm[T2D_BM_OPTIONS_LOAD].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_GRIGLIA].px = w->bm[T2D_BM_OPTIONS_CONTORNO].px + 37;
			w->bm[T2D_BM_OPTIONS_GRIGLIA].py = w->bm[T2D_BM_OPTIONS_CONTORNO].py + 75;
			w->bm[T2D_BM_OPTIONS_GRIGLIA].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_FRECCIA_SU_OFF].px = w->bm[T2D_BM_OPTIONS_CONTORNO].px + 519;
			w->bm[T2D_BM_OPTIONS_FRECCIA_SU_OFF].py = w->bm[T2D_BM_OPTIONS_CONTORNO].py + 85;
			w->bm[T2D_BM_OPTIONS_FRECCIA_SU_OFF].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_FRECCIA_SU_ON].px = w->bm[T2D_BM_OPTIONS_CONTORNO].px + 519;
			w->bm[T2D_BM_OPTIONS_FRECCIA_SU_ON].py = w->bm[T2D_BM_OPTIONS_CONTORNO].py + 85;
			w->bm[T2D_BM_OPTIONS_FRECCIA_SU_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_OFF].px = w->bm[T2D_BM_OPTIONS_CONTORNO].px + 519;
			w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_OFF].py = w->bm[T2D_BM_OPTIONS_CONTORNO].py + 420;
			w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_OFF].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_ON].px = w->bm[T2D_BM_OPTIONS_CONTORNO].px + 519;
			w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_ON].py = w->bm[T2D_BM_OPTIONS_CONTORNO].py + 420;
			w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_SOUND_ON].px = w->bm[T2D_BM_OPTIONS_CONTORNO].px + 50;
			w->bm[T2D_BM_OPTIONS_SOUND_ON].py = w->bm[T2D_BM_OPTIONS_CONTORNO].py + 60;
			w->bm[T2D_BM_OPTIONS_SOUND_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_MUSIC_ON].px = w->bm[T2D_BM_OPTIONS_CONTORNO].px + 44;
			w->bm[T2D_BM_OPTIONS_MUSIC_ON].py = w->bm[T2D_BM_OPTIONS_CONTORNO].py + 140;
			w->bm[T2D_BM_OPTIONS_MUSIC_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_SPEECH_ON].px = w->bm[T2D_BM_OPTIONS_CONTORNO].px + 50;
			w->bm[T2D_BM_OPTIONS_SPEECH_ON].py = w->bm[T2D_BM_OPTIONS_CONTORNO].py + 252;
			w->bm[T2D_BM_OPTIONS_SPEECH_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_SUBTITLES_ON].px = w->bm[T2D_BM_OPTIONS_CONTORNO].px + 40;
			w->bm[T2D_BM_OPTIONS_SUBTITLES_ON].py = w->bm[T2D_BM_OPTIONS_CONTORNO].py + 348;
			w->bm[T2D_BM_OPTIONS_SUBTITLES_ON].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_SOUND_OFF].px = w->bm[T2D_BM_OPTIONS_SOUND_ON].px;
			w->bm[T2D_BM_OPTIONS_SOUND_OFF].py = w->bm[T2D_BM_OPTIONS_SOUND_ON].py - 4;
			w->bm[T2D_BM_OPTIONS_SOUND_OFF].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_MUSIC_OFF].px = w->bm[T2D_BM_OPTIONS_MUSIC_ON].px - 2;
			w->bm[T2D_BM_OPTIONS_MUSIC_OFF].py = w->bm[T2D_BM_OPTIONS_MUSIC_ON].py + 1;
			w->bm[T2D_BM_OPTIONS_MUSIC_OFF].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_SPEECH_OFF].px = w->bm[T2D_BM_OPTIONS_SPEECH_ON].px - 4;
			w->bm[T2D_BM_OPTIONS_SPEECH_OFF].py = w->bm[T2D_BM_OPTIONS_SPEECH_ON].py - 9;
			w->bm[T2D_BM_OPTIONS_SPEECH_OFF].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_SUBTITLES_OFF].px = w->bm[T2D_BM_OPTIONS_SUBTITLES_ON].px;
			w->bm[T2D_BM_OPTIONS_SUBTITLES_OFF].py = w->bm[T2D_BM_OPTIONS_SUBTITLES_ON].py;
			w->bm[T2D_BM_OPTIONS_SUBTITLES_OFF].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_SOUND_BARRA].px = w->bm[T2D_BM_OPTIONS_CONTORNO].px + 200;
			w->bm[T2D_BM_OPTIONS_SOUND_BARRA].py = w->bm[T2D_BM_OPTIONS_CONTORNO].py + 70;
			w->bm[T2D_BM_OPTIONS_SOUND_BARRA].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_SOUND_PIROLO].px = 0;
			w->bm[T2D_BM_OPTIONS_SOUND_PIROLO].py = 0;
			w->bm[T2D_BM_OPTIONS_SOUND_PIROLO].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_MUSIC_BARRA].px = w->bm[T2D_BM_OPTIONS_CONTORNO].px + 200;
			w->bm[T2D_BM_OPTIONS_MUSIC_BARRA].py = w->bm[T2D_BM_OPTIONS_CONTORNO].py + 164;
			w->bm[T2D_BM_OPTIONS_MUSIC_BARRA].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_MUSIC_PIROLO].px = 0;
			w->bm[T2D_BM_OPTIONS_MUSIC_PIROLO].py = 0;
			w->bm[T2D_BM_OPTIONS_MUSIC_PIROLO].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_SPEECH_BARRA].px = w->bm[T2D_BM_OPTIONS_CONTORNO].px + 200;
			w->bm[T2D_BM_OPTIONS_SPEECH_BARRA].py = w->bm[T2D_BM_OPTIONS_CONTORNO].py + 262;
			w->bm[T2D_BM_OPTIONS_SPEECH_BARRA].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_SPEECH_PIROLO].px = 0;
			w->bm[T2D_BM_OPTIONS_SPEECH_PIROLO].py = 0;
			w->bm[T2D_BM_OPTIONS_SPEECH_PIROLO].tnum |= T2D_BM_OFF;
			if (bShowOnlyLoadWindow) {
				w->bm[T2D_BM_OPTIONS_MAINMENU_OFF].tnum = LoadDDBitmap(game, "t2d\\OPTIONS_MainMenu_Off.tga",          rSURFACESTRETCH);
				w->bm[T2D_BM_OPTIONS_MAINMENU_ON].tnum  = LoadDDBitmap(game, "t2d\\OPTIONS_MainMenu_On.tga",               rSURFACESTRETCH);

				w->bm[T2D_BM_OPTIONS_MAINMENU_OFF].px = 330;
				w->bm[T2D_BM_OPTIONS_MAINMENU_OFF].py = 536;
				w->bm[T2D_BM_OPTIONS_MAINMENU_OFF].tnum &= ~T2D_BM_OFF;

				w->bm[T2D_BM_OPTIONS_MAINMENU_ON].px = 330;
				w->bm[T2D_BM_OPTIONS_MAINMENU_ON].py = 536;
				w->bm[T2D_BM_OPTIONS_MAINMENU_ON].tnum |= T2D_BM_OFF;
			}

			w->bm[T2D_BM_OPTIONS_WIN].px = 0;
			w->bm[T2D_BM_OPTIONS_WIN].py = 0;
			switch (LoadWindow_WhichBkg) {
			case 0: {
				w->bm[T2D_BM_OPTIONS_WIN].tnum = T2D_BM_OFF;
			}
			break;

			case 1: {
				w->bm[T2D_BM_OPTIONS_WIN].tnum              = LoadDDBitmap(game, "t2d\\Extra_MainBkg.tga",         rSURFACESTRETCH);
				w->bm[T2D_BM_OPTIONS_WIN].tnum &= ~T2D_BM_OFF;
			}
			break;

			case 2: {
				w->bm[T2D_BM_OPTIONS_WIN].tnum              = LoadDDBitmap(game, "t2d\\Extra_GameOverBkg.tga",     rSURFACESTRETCH);
				w->bm[T2D_BM_OPTIONS_WIN].tnum &= ~T2D_BM_OFF;
			}
			break;
			}
			LoadWindow_WhichBkg = 0;


			//Bitmaps salvataggi
			for (j = 0; j < 3; j++)
				for (i = 0; i < 3; i++) {
					w->bm[T2D_BM_OPTIONS_SAVE_START + j * 3 + i].px = w->bm[T2D_BM_OPTIONS_CONTORNO].px + 37 + 170 * i;
					w->bm[T2D_BM_OPTIONS_SAVE_START + j * 3 + i].py = w->bm[T2D_BM_OPTIONS_CONTORNO].py + 71 + 140 * j;
					w->bm[T2D_BM_OPTIONS_SAVE_START + j * 3 + i].tnum |= T2D_BM_OFF;
				}

			if (bShowOnlyLoadWindow) {
				b = &w->bt[T2D_BT_OPTIONS_MAINMENU];
				b->on = true;
				GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_OPTIONS_MAINMENU_OFF]);
				b->tnum = EXTRALS_MAINMENU;
			} else {
				//Tasti
				b = &w->bt[T2D_BT_OPTIONS_SAVE];
				b->on = true;
				b->tnum = EXTRALS_SAVE;
				b->lim.x1 = 154;
				b->lim.y1 = 22;
				b->lim.x2 = 208;
				b->lim.y2 = 44;
				b = &w->bt[T2D_BT_OPTIONS_LOAD];
				b->on = true;
				b->tnum = EXTRALS_LOAD;
				b->lim.x1 = 268;
				b->lim.y1 = 22;
				b->lim.x2 = 322;
				b->lim.y2 = 44;
				b = &w->bt[T2D_BT_OPTIONS_OPTIONS];
				b->on = true;
				b->tnum = EXTRALS_OPTIONS;
				b->lim.x1 = 372;
				b->lim.y1 = 22;
				b->lim.x2 = 454;
				b->lim.y2 = 44;
				b = &w->bt[T2D_BT_OPTIONS_QUIT];
				b->on = true;
				b->tnum = EXTRALS_QUIT;
				b->lim.x1 = 494;
				b->lim.y1 = 22;
				b->lim.x2 = 540;
				b->lim.y2 = 44;
				b = &w->bt[T2D_BT_OPTIONS_CLOSE];
				b->on = true;
				b->tnum = EXTRALS_CLOSE;
				b->lim.x1 = 592;
				b->lim.y1 = 22;
				b->lim.x2 = 652;
				b->lim.y2 = 44;
			}
			b = &w->bt[T2D_BT_OPTIONS_FRECCIA_SU];
			b->on = false;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_OPTIONS_FRECCIA_SU_ON]);
			b = &w->bt[T2D_BT_OPTIONS_FRECCIA_GIU];
			b->on = false;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_ON]);
			for (j = 0; j < 3; j++)
				for (i = 0; i < 3; i++) {
					b = &w->bt[T2D_BT_OPTIONS_SAVE_START + j * 3 + i];
					b->on = true;
					b->lim.x1 = w->bm[T2D_BM_OPTIONS_CONTORNO].px + 52 + 170 * i;
					b->lim.y1 = w->bm[T2D_BM_OPTIONS_CONTORNO].py + 66 + 140 * j;
					b->lim.x2 = b->lim.x1 + 128;
					b->lim.y2 = b->lim.y1 + 96;
				}
			b = &w->bt[T2D_BT_OPTIONS_SOUND];
			b->on = false;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_OPTIONS_SOUND_OFF]);
			b->tnum = EXTRALS_SOUND;
			b = &w->bt[T2D_BT_OPTIONS_MUSIC];
			b->on = false;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_OPTIONS_MUSIC_OFF]);
			b->tnum = EXTRALS_MUSIC;
			b = &w->bt[T2D_BT_OPTIONS_SPEECH];
			b->on = false;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_OPTIONS_SPEECH_OFF]);
			b->tnum = EXTRALS_SPEECH;
			b = &w->bt[T2D_BT_OPTIONS_SUBTITLES];
			b->on = false;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_OPTIONS_SUBTITLES_OFF]);
			b->tnum = EXTRALS_SUBTITLES;

			b = &w->bt[T2D_BT_OPTIONS_SOUND_BARRA];
			b->on = false;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_OPTIONS_SOUND_BARRA]);
			b = &w->bt[T2D_BT_OPTIONS_MUSIC_BARRA];
			b->on = false;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_OPTIONS_MUSIC_BARRA]);
			b = &w->bt[T2D_BT_OPTIONS_SPEECH_BARRA];
			b->on = false;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_OPTIONS_SPEECH_BARRA]);

			//Gestione Scritte
			for (j = 0; j < 3; j++)
				for (i = 0; i < 3; i++) {
					w->bm[T2D_BM_OPTIONS_TEXT_SAVE_START + j * 3 + i].tnum = ((T2D_TEXT_OPTIONS_SAVE_START + j * 3 + i) << T2D_BM2TEXT_SHIFT);
					w->bm[T2D_BM_OPTIONS_TEXT_SAVE_START + j * 3 + i].px = w->bm[T2D_BM_OPTIONS_SAVE_START + j * 3 + i].px;
					w->bm[T2D_BM_OPTIONS_TEXT_SAVE_START + j * 3 + i].py = w->bm[T2D_BM_OPTIONS_SAVE_START + j * 3 + i].py + 105;
					w->bm[T2D_BM_OPTIONS_TEXT_SAVE_START + j * 3 + i].tnum |= T2D_BM_OFF;
					strcpy(w->text[T2D_TEXT_OPTIONS_SAVE_START + j * 3 + i].text, "");
					w->text[T2D_TEXT_OPTIONS_SAVE_START + j * 3 + i].font = FontKind::Standard;
					w->text[T2D_TEXT_OPTIONS_SAVE_START + j * 3 + i].color = WHITE_FONT;
				}

			//Settaggi iniziali
			WinActive[0] = T2D_WIN_OPTIONS_MAIN;
			CurButton = 0;
			CurButtonWin = 0;
			LinkWinToMouse = 0;
			LinkWinX = 0;
			LinkWinY = 0;
			ScrollWithMouse = 0;
			optionsCurrent = -1;
			optionsCurPos = 0;
			optionsSlot = 0;
			optionsWhat = 0;
			optionsSaveDescriptActive = false;
			NSaves = 0;
			RefreshSaveImg(game, 0, 2);

			CaricaSaves(game);
			SortSaves();

			if (TheMessage->wparam1 == MPX_START_T2D_SAVE) {
				CurButtonWin_Override = T2D_WIN_OPTIONS_MAIN;
				CurButton_Override = T2D_BT_OPTIONS_SAVE;
			}
			if (TheMessage->wparam1 == MPX_START_T2D_LOAD) {
				CurButtonWin_Override = T2D_WIN_OPTIONS_MAIN;
				CurButton_Override = T2D_BT_OPTIONS_LOAD;
			}
			if (TheMessage->wparam1 == MPX_START_T2D_OPTIONS) {
				CurButtonWin_Override = T2D_WIN_OPTIONS_MAIN;
				CurButton_Override = T2D_BT_OPTIONS_OPTIONS;
			}
			break;
		}
		case tMAINMENU:
			//Inizializzazione finestra Main
			w = &t2dWin[T2D_WIN_MAINMENU_MAIN];
			w->NOTSTRETCHED = false;
			w->px = 0;
			w->py = 0;
			//Bitmaps
			w->bm[T2D_BM_MAINMENU_WIN].tnum             = LoadDDBitmap(game, "t2d\\MainMenu_Win.tga",      rSURFACESTRETCH);
			w->bm[T2D_BM_MAINMENU_PLAY_ON].tnum         = LoadDDBitmap(game, "t2d\\MainMenu_Play_On.tga",  rSURFACESTRETCH);
			w->bm[T2D_BM_MAINMENU_PLAY_OFF].tnum        = LoadDDBitmap(game, "t2d\\MainMenu_Play_Off.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_MAINMENU_LOAD_ON].tnum         = LoadDDBitmap(game, "t2d\\MainMenu_Load_On.tga",  rSURFACESTRETCH);
			w->bm[T2D_BM_MAINMENU_LOAD_OFF].tnum        = LoadDDBitmap(game, "t2d\\MainMenu_Load_Off.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_MAINMENU_EXIT_ON].tnum         = LoadDDBitmap(game, "t2d\\MainMenu_Exit_On.tga",  rSURFACESTRETCH);
			w->bm[T2D_BM_MAINMENU_EXIT_OFF].tnum        = LoadDDBitmap(game, "t2d\\MainMenu_Exit_Off.tga", rSURFACESTRETCH);
			w->bm[T2D_BM_MAINMENU_WIN].px = 0;
			w->bm[T2D_BM_MAINMENU_WIN].py = 0;
			w->bm[T2D_BM_MAINMENU_WIN].tnum &= ~T2D_BM_OFF;

			GetDDBitmapExtends(renderer, &tmp_rect, &w->bm[T2D_BM_MAINMENU_PLAY_ON]);
			w->bm[T2D_BM_MAINMENU_PLAY_ON].px = w->bm[T2D_BM_MAINMENU_PLAY_OFF].px = (800 - (tmp_rect.x2 - tmp_rect.x1)) / 2;
			w->bm[T2D_BM_MAINMENU_PLAY_ON].py = w->bm[T2D_BM_MAINMENU_PLAY_OFF].py = 146;
			w->bm[T2D_BM_MAINMENU_PLAY_ON].tnum |= T2D_BM_OFF;

			GetDDBitmapExtends(renderer, &tmp_rect, &w->bm[T2D_BM_MAINMENU_LOAD_ON]);
			w->bm[T2D_BM_MAINMENU_LOAD_ON].px = w->bm[T2D_BM_MAINMENU_LOAD_OFF].px = (800 - (tmp_rect.x2 - tmp_rect.x1)) / 2;
			w->bm[T2D_BM_MAINMENU_LOAD_ON].py = w->bm[T2D_BM_MAINMENU_LOAD_OFF].py = 278;
			w->bm[T2D_BM_MAINMENU_LOAD_ON].tnum |= T2D_BM_OFF;

			GetDDBitmapExtends(renderer, &tmp_rect, &w->bm[T2D_BM_MAINMENU_EXIT_ON]);
			w->bm[T2D_BM_MAINMENU_EXIT_ON].px = w->bm[T2D_BM_MAINMENU_EXIT_OFF].px = (800 - (tmp_rect.x2 - tmp_rect.x1)) / 2;
			w->bm[T2D_BM_MAINMENU_EXIT_ON].py = w->bm[T2D_BM_MAINMENU_EXIT_OFF].py = 414;
			w->bm[T2D_BM_MAINMENU_EXIT_ON].tnum |= T2D_BM_OFF;

			/*                  w->bm[T2D_BM_MAINMENU_START].px = 192;
			                    w->bm[T2D_BM_MAINMENU_START].py = 98;
			                    w->bm[T2D_BM_MAINMENU_START].tnum |= T2D_BM_OFF;
			                    w->bm[T2D_BM_MAINMENU_LOAD].px = 228;
			                    w->bm[T2D_BM_MAINMENU_LOAD].py = 239;
			                    w->bm[T2D_BM_MAINMENU_LOAD].tnum |= T2D_BM_OFF;
			                    w->bm[T2D_BM_MAINMENU_QUIT].px = 302;
			                    w->bm[T2D_BM_MAINMENU_QUIT].py = 376;
			                    w->bm[T2D_BM_MAINMENU_QUIT].tnum |= T2D_BM_OFF;

			                    //Tasti
			                    b = &w->bt[T2D_BT_MAINMENU_START]; b->on = true;
			                    GetDDBitmapExtends( &b->lim, &w->bm[T2D_BM_MAINMENU_START] );
			                    b = &w->bt[T2D_BT_MAINMENU_LOAD]; b->on = true;
			                    GetDDBitmapExtends( &b->lim, &w->bm[T2D_BM_MAINMENU_LOAD] );
			                    b = &w->bt[T2D_BT_MAINMENU_QUIT]; b->on = true;
			                    GetDDBitmapExtends( &b->lim, &w->bm[T2D_BM_MAINMENU_QUIT] );
			*/
			//Tasti
			b = &w->bt[T2D_BT_MAINMENU_PLAY];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_MAINMENU_PLAY_ON]);
			b->tnum = EXTRALS_PLAYTHEGAME;
			b = &w->bt[T2D_BT_MAINMENU_LOAD];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_MAINMENU_LOAD_ON]);
			b->tnum = EXTRALS_LOADAGAME;
			b = &w->bt[T2D_BT_MAINMENU_EXIT];
			b->on = true;
			GetDDBitmapExtends(renderer, &b->lim, &w->bm[T2D_BM_MAINMENU_EXIT_ON]);
			b->tnum = EXTRALS_EXIT;

			//Settaggi iniziali
			WinActive[0] = T2D_WIN_MAINMENU_MAIN;
			CurButton = 0;
			CurButtonWin = 0;
			LinkWinToMouse = 0;
			LinkWinX = 0;
			LinkWinY = 0;
			ScrollWithMouse = 0;

			bStartMenuActive = 1;
			break;

		case tGAMEOVER:
			//Inizializzazione finestra Main
			w = &t2dWin[T2D_WIN_GAMEOVER_MAIN];
			w->NOTSTRETCHED = false;
			w->px = 0;
			w->py = 0;
			//Bitmaps
			w->bm[T2D_BM_GAMEOVER_WIN].tnum             = LoadDDBitmap(game, "t2d\\GAMEOVER_Win.tga",      rSURFACESTRETCH);
			w->bm[T2D_BM_GAMEOVER_WIN].px = 0;
			w->bm[T2D_BM_GAMEOVER_WIN].py = 0;
			w->bm[T2D_BM_GAMEOVER_WIN].tnum &= ~T2D_BM_OFF;

			//ExtraLS
			b = &w->bt[T2D_BT_GameOver_Win_TGA];
			b->lim.x1 = 206;
			b->lim.y1 = 264;
			b->lim.x2 = 604;
			b->lim.y2 = 332;
			b->tnum = EXTRALS_GAMEOVER;
			b->on = true;

			GameOverWindow_StartTime = timeGetTime();

			//Settaggi iniziali
			WinActive[0] = T2D_WIN_GAMEOVER_MAIN;
			CurButton = 0;
			CurButtonWin = 0;
			LinkWinToMouse = 0;
			LinkWinX = 0;
			LinkWinY = 0;
			ScrollWithMouse = 0;
			break;
		}
		doT2DMouse(game);
		break;

	case ME_T2DSTOP:
		bPlayerInAnim = false;
		bPauseAllAnims = false;

		for (i = 0; i < T2D_MAX_WINDOWS; i++) {
			for (j = 0; j < T2D_MAX_BITMAPS_IN_WIN; j++) {
				t2dWin[i].bm[j].tnum |= T2D_BM_OFF;
				if (t2dWin[i].bm[j].tnum > 0) {
					rReleaseBitmap(t2dWin[i].bm[j].tnum);
					t2dWin[i].bm[j].tnum = 0;
				}
			}
			/*
			                for( j=0; j< T2D_MAX_BUTTONS_IN_WIN; j++ )
			                    if( t2dWin[i].bt[j].tnum > 0 )
			                    {
			                        rReleaseBitmap( t2dWin[i].bt[j].tnum );
			                        t2dWin[i].bt[j].tnum = 0;
			                    }*/
		}
		for (uint k = 0; k < ARRAYSIZE(t2dWin); k++) t2dWin[k].reset();
		memset(WinActive, 0, sizeof(WinActive));

		switch (bT2DActive) {
		case tSCANNER:
			if (SelectorFlag)
				StartAnim(game, a297);
			break;

		case tCOMPUTER:
			if ((game._gameVars.getCurRoomId() == r17) && (init.Obj[o17COMPUTER].flags & EXTRA))
				StartAnim(game, a1711);
			init.Obj[o17COMPUTER].flags &= ~EXTRA;
			break;

		case tOPTIONS:
			game._renderer->setCurCameraViewport(t3dCurCamera->Fov, bSuperView);
			bStartMenuActive = 0;
			break;

		case tMAINMENU:
			bStartMenuActive = 0;
			break;

		}
		bT2DActive = 0;

		if (TheMessage->wparam1 == STOP_T2D__LAUNCH_NEWGAME) {
			game.StartPlayingGame("r91.t3d");

//				start intro
			if (!(LoaderFlags & T3D_DEBUGMODE))
				_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR000, 0, 0, nullptr, nullptr, nullptr);
		}
		if (TheMessage->wparam1 == STOP_T2D__LAUNCH_LOAD) {
			bShowOnlyLoadWindow = 1;
			_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, MPX_START_T2D_LOAD, 0, tOPTIONS, nullptr, nullptr, nullptr);
		}
		if (TheMessage->wparam1 == STOP_T2D__LAUNCH_MAINMENU) {
			StopPlayingGame(game);
			_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, 0, 0, tMAINMENU, nullptr, nullptr, nullptr);
		}

		//Riabilita DInput
		ResetDIKbd();
		bIngnoreDIKeyboard = false;
		StartDiary(game, game._gameVars.getCurRoomId(), nullptr);

		if (Settati_PDA_items_durante_t2d) {
			_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, FRAME_PER_SECOND * 3, 0, EFFECT_DISPLAY_NEWLOGIMG, nullptr, nullptr, nullptr);
			Settati_PDA_items_durante_t2d = 0;
		}
		break;

	case ME_T2DSETBITMAP:
		if (TheMessage->bparam)
			t2dWin[TheMessage->wparam1].bm[TheMessage->wparam2].tnum &= ~T2D_BM_OFF;
		else
			t2dWin[TheMessage->wparam1].bm[TheMessage->wparam2].tnum |= T2D_BM_OFF;
		break;

	case ME_T2DSETBUTTON:
		t2dWin[TheMessage->wparam1].bt[TheMessage->wparam2].on = TheMessage->bparam;
		break;

	case ME_T2DSETWINDOW:
		ResetScreenBuffer();
		if (TheMessage->bparam) {
			j = -1;
			for (i = 0; i < T2D_MAX_WINDOWS; i++) {
				if (WinActive[i] == TheMessage->wparam1) j = i;
				if (!WinActive[i]) {
					if (j == -1) {
						WinActive[i] = TheMessage->wparam1;
					} else {
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSWAPWINDOWS, MP_DEFAULT, WinActive[i - 1], WinActive[j], 0, nullptr, nullptr, nullptr);
					}
					break;
				}
			}
		} else {
			for (i = 0; i < T2D_MAX_WINDOWS; i++)
				if (WinActive[i] == TheMessage->wparam1) {
					for (j = i + 1; j < T2D_MAX_WINDOWS; j++)
						WinActive[j - 1] = WinActive[j];
					WinActive[T2D_MAX_WINDOWS - 1] = 0;
					break;
				}
		}
		break;

	case ME_T2DSWAPWINDOWS:
		j = -1;
		for (i = 0; i < T2D_MAX_WINDOWS; i++) {
			if (WinActive[i] == TheMessage->wparam1) {
				if (j < 0)
					j = i;
				else {
					WinActive[i] = TheMessage->wparam2;
					WinActive[j] = TheMessage->wparam1;
					break;
				}
			} else if (WinActive[i] == TheMessage->wparam2) {
				if (j < 0)
					j = i;
				else {
					WinActive[i] = TheMessage->wparam1;
					WinActive[j] = TheMessage->wparam2;
					break;
				}
			}
		}
		ResetScreenBuffer();
		break;

	case ME_T2DCONTINUE:
		if (!bT2DActive) return;
		switch (TheMessage->bparam) {
		case T2D_SCANNER_PREVIEWING:
		case T2D_SCANNER_SCANNING:
			if (TheMessage->wparam1-- <= 0) {
				t2dWin[T2D_WIN_SCANNER_BACKGROUND].bm[T2D_BM_SCANNER_BACKGROUND_PIC_LOW].tnum |= T2D_BM_OFF;
				t2dWin[T2D_WIN_SCANNER_BACKGROUND].bm[T2D_BM_SCANNER_BACKGROUND_PIC_HI].tnum |= T2D_BM_OFF;
				j = renderer.getBitmapRealDimY(t2dWin[T2D_WIN_SCANNER_BACKGROUND].bm[T2D_BM_SCANNER_BACKGROUND_PIC_LOW].tnum);
				if (TheMessage->bparam == T2D_SCANNER_PREVIEWING)
					d = &t2dWin[T2D_WIN_SCANNER_BACKGROUND].bm[T2D_BM_SCANNER_BACKGROUND_PIC_LOW];
				else
					d = &t2dWin[T2D_WIN_SCANNER_BACKGROUND].bm[T2D_BM_SCANNER_BACKGROUND_PIC_HI];

				d1 = &t2dWin[T2D_WIN_SCANNER_PROGRESSBAR].bm[T2D_BM_SCANNER_PROGRESSBAR_PROGRESS_OFF];
				d2 = &t2dWin[T2D_WIN_SCANNER_PROGRESSBAR].bm[T2D_BM_SCANNER_PROGRESSBAR_PROGRESS_ON];
				if (TheMessage->wparam2 & 1) {
					d3 = &t2dWin[T2D_WIN_SCANNER_PROGRESSBAR].bm[T2D_BM_SCANNER_PROGRESSBAR_SCANNING_OFF];
					d4 = &t2dWin[T2D_WIN_SCANNER_PROGRESSBAR].bm[T2D_BM_SCANNER_PROGRESSBAR_SCANNING_ON];
				} else {
					d3 = &t2dWin[T2D_WIN_SCANNER_PROGRESSBAR].bm[T2D_BM_SCANNER_PROGRESSBAR_SCANNING_ON];
					d4 = &t2dWin[T2D_WIN_SCANNER_PROGRESSBAR].bm[T2D_BM_SCANNER_PROGRESSBAR_SCANNING_OFF];
				}

				d->tnum &= ~T2D_BM_OFF;                                                 // Pic di fondo
				d1->tnum &= ~T2D_BM_OFF;                                                // Barra spenta
				d2->tnum &= ~T2D_BM_OFF;                                                // Barra accesa
				d3->tnum &= ~T2D_BM_OFF;
				d4->tnum |= T2D_BM_OFF;

				TheMessage->wparam2 --;
				d->dy = (j * (T2D_SCANNER_SCANNING_STEPS - TheMessage->wparam2)) / T2D_SCANNER_SCANNING_STEPS;
				d1->ox = (T2D_SCANNER_SCANNING_DX * (T2D_SCANNER_SCANNING_STEPS - TheMessage->wparam2));
				d2->dx = (T2D_SCANNER_SCANNING_DX * (T2D_SCANNER_SCANNING_STEPS - TheMessage->wparam2));

				TheMessage->wparam1 = T2D_SCANNER_SCANNING_DELAY;
				if (TheMessage->wparam2 <= 0) {
					// Punto giusto 320, 503 di scansione
					d3 = &t2dWin[T2D_WIN_SCANNER_PROGRESSBAR].bm[T2D_BM_SCANNER_PROGRESSBAR_SCANNING_OFF];
					d3->tnum &= ~T2D_BM_OFF;
					d4 = &t2dWin[T2D_WIN_SCANNER_PROGRESSBAR].bm[T2D_BM_SCANNER_PROGRESSBAR_SCANNING_ON];
					d4->tnum |= T2D_BM_OFF;
					d1->tnum &= ~T2D_BM_OFF;                                            // Barra spenta
					d1->ox = d1->oy = d1->dx = d1->dy = 0;
					d2->tnum |= T2D_BM_OFF;                                             // Barra spenta
					d2->ox = d2->oy = d2->dx = d2->dy = 0;
					if (TheMessage->bparam == T2D_SCANNER_PREVIEWING) {
						d->ox = d->oy = d->dx = d->dy = 0;
						// Attiva bottone per selezionare l'area
						t2dWin[T2D_WIN_SCANNER_TOOLBAR].bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_OFF].tnum &= ~T2D_BM_OFF;
						t2dWin[T2D_WIN_SCANNER_TOOLBAR].bm[T2D_BM_SCANNER_TOOLBAR_SELECTAREA_FREEZE].tnum |= T2D_BM_OFF;
						// Riattiva i bottoni della toolbar che servono
						t2dWin[T2D_WIN_SCANNER_TOOLBAR].bt[T2D_BT_SCANNER_TOOLBAR_MOVE].on = true;
						t2dWin[T2D_WIN_SCANNER_TOOLBAR].bt[T2D_BT_SCANNER_TOOLBAR_EXIT].on = true;
						t2dWin[T2D_WIN_SCANNER_TOOLBAR].bt[T2D_BT_SCANNER_TOOLBAR_PREVIEW_OFF].on = true;
						t2dWin[T2D_WIN_SCANNER_TOOLBAR].bt[T2D_BT_SCANNER_TOOLBAR_SELECTAREA_OFF].on = true;
					} else {
						// Riattiva i bottoni della toolbar che servono
						t2dWin[T2D_WIN_SCANNER_TOOLBAR].bt[T2D_BT_SCANNER_TOOLBAR_MOVE].on = true;
						t2dWin[T2D_WIN_SCANNER_TOOLBAR].bt[T2D_BT_SCANNER_TOOLBAR_EXIT].on = true;
						t2dWin[T2D_WIN_SCANNER_TOOLBAR].bt[T2D_BT_SCANNER_TOOLBAR_PREVIEW_OFF].on = true;
					}
					// Spegne finestra progress
					_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_DEFAULT, T2D_WIN_SCANNER_PROGRESSBAR, 0, false, nullptr, nullptr, nullptr);
				}
			}
			TheMessage->flags |= MP_WAIT_RETRACE;
			if (TheMessage->wparam2 > 0) ReEvent();
			break;

		case T2D_COMPUTER_CONNECTING:
			if (computerConnectStr[0] == 0) {
				//Partenza
				strcpy(computerConnectStr, Sentence[init.Obj[oCOMPUTERDIAL].text[0]]);
				strcpy(t2dWin[T2D_WIN_COMPUTER_DIALUP].text[T2D_TEXT_COMPUTER_DIALUP_NUMBER].text, computerConnectStr);
				t2dWin[T2D_WIN_COMPUTER_DIALUP].bt[T2D_BT_DialUpWin_TGA__Dialling].on = true;
				LastTime = timeGetTime();
			} else {
				CurTime = timeGetTime();
				if (CurTime - LastTime > T2D_COMPUTER_CONNECTING_SPEED) {
					strcat(computerConnectStr, ".");
					strcpy(t2dWin[T2D_WIN_COMPUTER_DIALUP].text[T2D_TEXT_COMPUTER_DIALUP_NUMBER].text, computerConnectStr);
					LastTime = CurTime;

					if (strlen(computerConnectStr) > 12) {
						//Disattiva finestra Dialup
						strcpy(computerConnectStr, Sentence[init.Obj[oCOMPUTERDIAL].text[1]]);
						strcpy(t2dWin[T2D_WIN_COMPUTER_DIALUP].text[T2D_TEXT_COMPUTER_DIALUP_NUMBER].text, computerConnectStr);
						_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_COMPUTER_BUTTON_DELAY, 0, EFFECT_WAIT, nullptr, nullptr, nullptr);
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_COMPUTER_DIALUP, 0, false, nullptr, nullptr, nullptr);
						t2dWin[T2D_WIN_COMPUTER_BACKGROUND].bt[T2D_BT_COMPUTER_BACKGROUND_EMAIL].on = true;
						t2dWin[T2D_WIN_COMPUTER_BACKGROUND].bt[T2D_BT_COMPUTER_BACKGROUND_SEARCH].on = true;

						t2dWin[T2D_WIN_COMPUTER_DIALUP].bt[T2D_BT_DialUpWin_TGA__Dialling].on = false;
						break;
					}
				}
			}
			TheMessage->flags |= MP_WAIT_RETRACE;
			ReEvent();
			break;

		case T2D_PDA_TONE_AQUIRING:
			w = &t2dWin[T2D_WIN_PDA_TONE];
			if (w->bm[T2D_BM_PDA_TONE_BARRA_1].tnum & T2D_BM_OFF) {
				//Attiva le due onde
				if (StartAcquiring == i28WALKMANOK) {
					w->bm[T2D_BM_PDA_TONE_WAVE_1].tnum &= ~T2D_BM_OFF;
					w->bm[T2D_BM_PDA_TONE_WAVE_2].tnum &= ~T2D_BM_OFF;
				}

				w->bm[T2D_BM_PDA_TONE_WAVE_1].dx = 1;
				w->bm[T2D_BM_PDA_TONE_WAVE_2].dx = 1;

				//Attiva la barra
				w->bm[T2D_BM_PDA_TONE_BARRA_1].tnum &= ~T2D_BM_OFF;
				w->bm[T2D_BM_PDA_TONE_BARRA_1].dx = 1;

				//Disattiva i tasti Acquire e Process
				w->bt[T2D_BT_PDA_TONE_ACQUIRE].on = false;
				w->bt[T2D_BT_PDA_TONE_PROCESS].on = false;
			} else {
				if (w->bm[T2D_BM_PDA_TONE_WAVE_1].dx + 1 >= (T2D_PDA_TONE_XF - T2D_PDA_TONE_XI)) {
					//Mette a zero le Dx delle onde e della barra, che avranno cosi la dimensione totale
					w->bm[T2D_BM_PDA_TONE_WAVE_1].dx = 0;
					w->bm[T2D_BM_PDA_TONE_WAVE_2].dx = 0;
					w->bm[T2D_BM_PDA_TONE_BARRA_1].dx = 0;

					//Riattiva il tasto Process
					w->bt[T2D_BT_PDA_TONE_ACQUIRE].on = true;
					w->bt[T2D_BT_PDA_TONE_PROCESS].on = true;

					//Suono
					StartSound(game, wPDAALERT);

					if (StartAcquiring != i28WALKMANOK) {
						t2dBUTTON *button = nullptr;

						button = &w->bt[T2D_BT_PDA_TONE_WIN];
						button->on = false;
						button = &w->bt[T2D_BT_PDA_TONE_HELP];
						button->on = false;
						button = &w->bt[T2D_BT_PDA_TONE_EXIT];
						button->on = false;
						button = &w->bt[T2D_BT_PDA_TONE_ACQUIRE];
						button->on = false;
						button = &w->bt[T2D_BT_PDA_TONE_PROCESS];
						button->on = false;

						SetHelpWin(game._fonts, &PDA_Tone_Acquire_Help_Func, 2, Sentence[init.Obj[oTONERHELP].text[4]], " ");
						_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_DEFAULT, T2D_WIN_PDA_HELP, 0, true, nullptr, nullptr, nullptr);
					}
					break;
				}

				//Onde
				w->bm[T2D_BM_PDA_TONE_WAVE_1].dx += T2D_PDA_TONE_SPEED;
				w->bm[T2D_BM_PDA_TONE_WAVE_2].dx += T2D_PDA_TONE_SPEED;

				//Barra
				w->bm[T2D_BM_PDA_TONE_BARRA_1].dx = (w->bm[T2D_BM_PDA_TONE_WAVE_1].dx * T2D_PDA_TONE_BARRA_DX) / (T2D_PDA_TONE_XF - T2D_PDA_TONE_XI);
				if (w->bm[T2D_BM_PDA_TONE_BARRA_1].dx == 0) w->bm[T2D_BM_PDA_TONE_BARRA_1].dx = 1;
			}

			TheMessage->flags |= MP_WAIT_RETRACE;
			ReEvent();
			break;

		case T2D_PDA_TONE_PROCESS:
			w = &t2dWin[T2D_WIN_PDA_TONE];

			if (w->bm[T2D_BM_PDA_TONE_REDLINE].tnum & T2D_BM_OFF) {
				//Attiva la linea rossa...
				w->bm[T2D_BM_PDA_TONE_REDLINE].tnum &= ~T2D_BM_OFF;
				w->bm[T2D_BM_PDA_TONE_REDLINE].px = T2D_PDA_TONE_XI;

				//... e la barra
				w->bm[T2D_BM_PDA_TONE_BARRA_2].tnum &= ~T2D_BM_OFF;
				w->bm[T2D_BM_PDA_TONE_BARRA_2].dx = 1;

				//Disattiva il tasto process
				w->bt[T2D_BT_PDA_TONE_PROCESS].on = false;
			} else {
				//Linea
				w->bm[T2D_BM_PDA_TONE_REDLINE].px += T2D_PDA_TONE_SPEED;

				//Barra
				w->bm[T2D_BM_PDA_TONE_BARRA_2].dx = ((w->bm[T2D_BM_PDA_TONE_REDLINE].px - T2D_PDA_TONE_XI) * T2D_PDA_TONE_BARRA_DX) / (T2D_PDA_TONE_XF - T2D_PDA_TONE_XI);
				if (w->bm[T2D_BM_PDA_TONE_BARRA_2].dx == 0) w->bm[T2D_BM_PDA_TONE_BARRA_2].dx = 1;

				//Controlla se la linea � arrivata alla fine
				if (w->bm[T2D_BM_PDA_TONE_REDLINE].px + T2D_PDA_TONE_REDLINE_DX + 1 >= T2D_PDA_TONE_XF) {
					//Azzera linea e barra
					w->bm[T2D_BM_PDA_TONE_REDLINE].tnum |= T2D_BM_OFF;
					w->bm[T2D_BM_PDA_TONE_BARRA_2].tnum |= T2D_BM_OFF;
					//Riattiva il tasto Process
					w->bt[T2D_BT_PDA_TONE_PROCESS].on = true;
					//Attiva il testo del risultato
					if (StartAcquiring == i28WALKMANOK)
						w->bm[T2D_BM_PDA_TONE_RISULTATO].tnum &= ~T2D_BM_OFF;

					//Suono
					StartSound(game, wPDAALERT);
					break;
				}
			}

			TheMessage->flags |= MP_WAIT_RETRACE;
			ReEvent();
			break;

		case T2D_CURSOR:
			if (CursorActive == false) break;
			CurTime = timeGetTime();
			if (CurTime > LastTime + T2D_CURSOR_SPEED) {
				LastTime = CurTime;
				CursorVisible = !CursorVisible;
				w = &t2dWin[T2D_WIN_OPTIONS_MAIN];
				if (CursorVisible) {
					i = strlen(CursorString);
					CursorString[i] = '|';
					CursorString[i + 1] = 0;
				} else {
					i = strlen(CursorString);
					CursorString[i - 1] = 0;
				}
			}
			TheMessage->flags |= MP_WAIT_RETRACE;
			ReEvent();
			break;
		}
		break;
	}
}

void CaricaSaves(WGame &game) {
	warning("STUBBED: CaricaSaves");
#if 0
	int i, j;
	char DataFile[MAX_PATH];
	char GfxFile[MAX_PATH];
	char Temp[10];
	char Data[T3D_NAMELEN + 8];
	FILE *file;

	NSaves = 0;

	t3dForceNOFastFile(1);

	for (i = 0; i < T2D_OPTIONS_MAX_SAVES; i++) {
		snprintf(DataFile, MAX_PATH, "%sWm%02d.sav", game.workDirs._savesDir.c_str(), i + 1);
		snprintf(GfxFile,  MAX_PATH, "%sWmSav%02d.tga", game.workDirs._savesDir.c_str(), i + 1);

//		se non facessi cosi' la funzione t3dOpenFile() non trovando il file sparerebbe mille warning
		if (!t3dAccessFile(DataFile)) continue;

		file = t3dOpenFile(DataFile);
		if (!file) continue;

		Saves[NSaves].NFile = i + 1;

		for (j = 0; j < (T3D_NAMELEN + 2); j++)
			Data[j] = t3dRead8();

		t3dCloseFile();

		//Hour
		Temp[0] = Data[2 + 0];
		Temp[1] = Data[2 + 1];
		Temp[2] = 0;
		Saves[NSaves].Hour = atoi(Temp);
		Temp[0] = Data[2 + 3];
		Temp[1] = Data[2 + 4];
		Temp[2] = 0;
		Saves[NSaves].Min = atoi(Temp);
		Temp[0] = Data[2 + 6];
		Temp[1] = Data[2 + 7];
		Temp[2] = 0;
		Saves[NSaves].Sec = atoi(Temp);
		Temp[0] = Data[2 + 9];
		Temp[1] = Data[2 + 10];
		Temp[2] = 0;
		Saves[NSaves].Day = atoi(Temp);
		Temp[0] = Data[2 + 12];
		Temp[1] = Data[2 + 13];
		Temp[2] = 0;
		Saves[NSaves].Month = atoi(Temp);
		Temp[0] = Data[2 + 15];
		Temp[1] = Data[2 + 16];
		Temp[2] = Data[2 + 17];
		Temp[3] = Data[2 + 18];
		Temp[4] = 0;
		Saves[NSaves].Year = atoi(Temp);

		NSaves++;
	}
#endif
	t3dForceNOFastFile(0);
}

void SortSaves() {
	warning("STUBBED: SortSaves");
#if 0
	qsort(Saves, NSaves, sizeof(SavesS), CompareSaves);
#endif
}

void RefreshSaveImg(WGame &game, int Pos, uint8 Type) {
	//Type: 0=Save, 1=Load, 2=Nothing
	t2dWINDOW *w = &t2dWin[T2D_WIN_OPTIONS_MAIN];
	int i;
	char DataFileName[MAX_PATH];
	char GfxFileName[MAX_PATH];

	if (Type == 2) {
		for (i = 0; i < 9; i++) {
			optionsSaves[i][0] = 0;
			w->bm[T2D_BM_OPTIONS_SAVE_START + i].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_TEXT_SAVE_START + i].tnum |= T2D_BM_OFF;
		}

		return;
	}

	for (i = 0; i < 9; i++) {
		if ((Type == 0) && (Pos + i == 0)) {
			w->bm[T2D_BM_OPTIONS_SAVE_START].tnum   = LoadDDBitmap(game, "t2d\\OPTIONS_NewSave.tga",    rSURFACESTRETCH);
			w->bm[T2D_BM_OPTIONS_SAVE_START].tnum   &= ~T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_TEXT_SAVE_START].tnum |= T2D_BM_OFF;
			w->text[T2D_TEXT_OPTIONS_SAVE_START].text[0] = 0;
			strcpy(optionsSaves[i], "NewSave");
			continue;
		}

		if (Pos >= NSaves) break;

		snprintf(DataFileName, MAX_PATH, "%sWm%02d.sav", game.workDirs._savesDir.c_str(), Saves[Pos].NFile);
		snprintf(GfxFileName,  MAX_PATH, "%sWmSav%02d.tga", game.workDirs._savesDir.c_str(), Saves[Pos].NFile);

		t3dForceNOFastFile(1);
		if (!t3dAccessFile(DataFileName)) {
			t3dForceNOFastFile(0);
			break;
		}
		t3dForceNOFastFile(0);

		t3dForceNOFastFile(1);
		if (t3dAccessFile(GfxFileName)) {
			w->bm[T2D_BM_OPTIONS_SAVE_START + i].tnum = rLoadBitmapImage(game, GfxFileName, (uint8)(rBITMAPSURFACE | rSURFACEFLIP | rSURFACESTRETCH));
			t3dForceNOFastFile(0);
		} else {
			t3dForceNOFastFile(0);
			w->bm[T2D_BM_OPTIONS_SAVE_START + i].tnum = LoadDDBitmap(game, "t2d\\OPTIONS_NotFound.tga", rSURFACESTRETCH);
		}
		w->bm[T2D_BM_OPTIONS_SAVE_START + i].tnum &= ~T2D_BM_OFF;

		//Aggiorna la descrizione
		snprintf(w->text[T2D_TEXT_OPTIONS_SAVE_START + i].text, T2D_MAX_TEXTS_IN_WIN, "%02d:%02d %02d/%02d/%04d", Saves[Pos].Hour, Saves[Pos].Min, Saves[Pos].Day, Saves[Pos].Month, Saves[Pos].Year);
		w->bm[T2D_BM_OPTIONS_TEXT_SAVE_START + i].tnum &= ~T2D_BM_OFF;
		strcpy(optionsSaves[i], DataFileName);

		Pos++;
	}

	if (i < 9) {
		for (; i < 9; i++) {
			optionsSaves[i][0] = 0;
			w->bm[T2D_BM_OPTIONS_SAVE_START + i].tnum |= T2D_BM_OFF;
			w->bm[T2D_BM_OPTIONS_TEXT_SAVE_START + i].tnum |= T2D_BM_OFF;
			w->text[T2D_TEXT_OPTIONS_SAVE_START + i].text[0] = 0;
		}
	}

	if (optionsCurPos > 0) {
		w->bm[T2D_BM_OPTIONS_FRECCIA_SU_OFF].tnum &= ~T2D_BM_OFF;
		w->bt[T2D_BT_OPTIONS_FRECCIA_SU].on = TRUE;
	} else {
		w->bm[T2D_BM_OPTIONS_FRECCIA_SU_OFF].tnum |= T2D_BM_OFF;
		w->bt[T2D_BT_OPTIONS_FRECCIA_SU].on = FALSE;
	}
	if (optionsCurPos + 6 < NSaves) {
		w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_OFF].tnum &= ~T2D_BM_OFF;
		w->bt[T2D_BT_OPTIONS_FRECCIA_GIU].on = TRUE;
	} else {
		w->bm[T2D_BM_OPTIONS_FRECCIA_GIU_OFF].tnum |= T2D_BM_OFF;
		w->bt[T2D_BT_OPTIONS_FRECCIA_GIU].on = FALSE;
	}
}


void PDA_Tone_Acquire_Help_Func() {
	t2dBUTTON *b;
	t2dWINDOW *w;

	w = &t2dWin[T2D_WIN_PDA_TONE];

	b = &w->bt[T2D_BT_PDA_TONE_WIN];
	b->on = TRUE;
	b = &w->bt[T2D_BT_PDA_TONE_HELP];
	b->on = TRUE;
	b = &w->bt[T2D_BT_PDA_TONE_EXIT];
	b->on = TRUE;
	b = &w->bt[T2D_BT_PDA_TONE_ACQUIRE];
	b->on = TRUE;
	b = &w->bt[T2D_BT_PDA_TONE_PROCESS];
	b->on = TRUE;

	w->bm[T2D_BM_PDA_TONE_BARRA_1].tnum |= T2D_BM_OFF;

	//Delay
	_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, T2D_PDA_BUTTON_DELAY, 0, EFFECT_WAIT, nullptr, nullptr, nullptr);
	//Chiude finestra help
	_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSETWINDOW, MP_DEFAULT, T2D_WIN_PDA_HELP, 0, FALSE, nullptr, nullptr, nullptr);
	//Chiude finestra Tone
//	Event( EventClass::MC_T2D, ME_T2DSETWINDOW, MP_WAITA, T2D_WIN_PDA_TONE, 0, FALSE, NULL, NULL, NULL );
}

void SetHelpWin(Fonts &fonts, void (*Func)(), int NArg, ...) {
	va_list vl;
	char *Text = nullptr;
	int i, Dx = 0, Dy = 0;
	t2dWINDOW *w = &t2dWin[T2D_WIN_PDA_HELP];

	va_start(vl, NArg);

	for (i = 0; i < NArg; i++) {
		Text = va_arg(vl, char *);

		if (Text == nullptr) break;

		fonts.getTextDim(Text, FontKind::Standard, &Dx, &Dy);
		w->bm[T2D_BM_PDA_HELP_TEXT_START + i].px = T2D_PDA_HELP_TEXT_XI + (T2D_PDA_HELP_TEXT_XF - T2D_PDA_HELP_TEXT_XI - Dx) / 2;
		w->bm[T2D_BM_PDA_HELP_TEXT_START + i].py = T2D_PDA_HELP_TEXT_YI + (Dy + 5) * i;
		w->bm[T2D_BM_PDA_HELP_TEXT_START + i].tnum &= ~T2D_BM_OFF;

		strcpy(w->text[T2D_TEXT_PDA_HELP_START + i].text, Text);
	}

	HelpFunc = Func;

	w->bm[T2D_BM_PDA_HELP_WIN].dy = T2D_PDA_HELP_TEXT_YI + (Dy + 5) * (i);
	w->bm[T2D_BM_PDA_HELP_WIN2].oy = T2D_PDA_HELP_WIN_LOWER;
	w->bm[T2D_BM_PDA_HELP_WIN2].py = -(355 - (T2D_PDA_HELP_TEXT_YI + (Dy + 5) * (i) + (355 - T2D_PDA_HELP_WIN_LOWER)));

	for (; i < T2D_BM_PDA_HELP_TEXT_END - T2D_BM_PDA_HELP_TEXT_START; i++)
		w->bm[T2D_BM_PDA_HELP_TEXT_START + i].tnum |= T2D_BM_OFF;

	va_end(vl);
}

} // End of namespace Watchmaker
