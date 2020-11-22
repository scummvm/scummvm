/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <cstdio>
#include "gui/guidialog.h"

#include "ac/common.h"
#include "ac/draw.h"
#include "ac/game.h"
#include "ac/gamesetup.h"
#include "ac/gamesetupstruct.h"
#include "gui/cscidialog.h"
#include <cctype> //isdigit()
#include "gfx/bitmap.h"
#include "gfx/graphicsdriver.h"
#include "debug/debug_log.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

extern IGraphicsDriver *gfxDriver;
extern GameSetup usetup;
extern GameSetupStruct game;

namespace {

// TODO: store drawing surface inside old gui classes instead
int windowPosX, windowPosY, windowPosWidth, windowPosHeight;
Bitmap *windowBuffer;
IDriverDependantBitmap *dialogDDB;

#undef MAXSAVEGAMES
#define MAXSAVEGAMES 20
DisplayProperties dispp;
char *lpTemp, *lpTemp2;
char bufTemp[260], buffer2[260];
int numsaves = 0, toomanygames;
int filenumbers[MAXSAVEGAMES];
unsigned long filedates[MAXSAVEGAMES];

CSCIMessage smes;

char buff[200];
int myscrnwid = 320, myscrnhit = 200;

}

char *get_gui_dialog_buffer() {
	return buffer2;
}

//
// TODO: rewrite the whole thing to work inside the main game update and render loop!
//

Bitmap *prepare_gui_screen(int x, int y, int width, int height, bool opaque) {
	windowPosX = x;
	windowPosY = y;
	windowPosWidth = width;
	windowPosHeight = height;
	if (windowBuffer) {
		windowBuffer = recycle_bitmap(windowBuffer, windowBuffer->GetColorDepth(), windowPosWidth, windowPosHeight, !opaque);
	} else {
		windowBuffer = BitmapHelper::CreateBitmap(windowPosWidth, windowPosHeight, game.GetColorDepth());
		windowBuffer = ReplaceBitmapWithSupportedFormat(windowBuffer);
	}
	dialogDDB = recycle_ddb_bitmap(dialogDDB, windowBuffer, false, opaque);
	return windowBuffer;
}

Bitmap *get_gui_screen() {
	return windowBuffer;
}

void clear_gui_screen() {
	if (dialogDDB)
		gfxDriver->DestroyDDB(dialogDDB);
	dialogDDB = nullptr;
	delete windowBuffer;
	windowBuffer = nullptr;
}

void refresh_gui_screen() {
	gfxDriver->UpdateDDBFromBitmap(dialogDDB, windowBuffer, false);
	render_graphics(dialogDDB, windowPosX, windowPosY);
}

int loadgamedialog() {
	const int wnd_width = 200;
	const int wnd_height = 120;
	const int boxleft = myscrnwid / 2 - wnd_width / 2;
	const int boxtop = myscrnhit / 2 - wnd_height / 2;
	const int buttonhit = usetup.textheight + 5;

	int handl = CSCIDrawWindow(boxleft, boxtop, wnd_width, wnd_height);
	int ctrlok =
		CSCICreateControl(CNT_PUSHBUTTON | CNF_DEFAULT, 135, 5, 60, 10, get_global_message(MSG_RESTORE));
	int ctrlcancel =
		CSCICreateControl(CNT_PUSHBUTTON | CNF_CANCEL, 135, 5 + buttonhit, 60, 10,
			get_global_message(MSG_CANCEL));
	int ctrllist = CSCICreateControl(CNT_LISTBOX, 10, 30, 120, 80, nullptr);
	int ctrltex1 = CSCICreateControl(CNT_LABEL, 10, 5, 120, 0, get_global_message(MSG_SELECTLOAD));
	CSCISendControlMessage(ctrllist, CLB_CLEAR, 0, 0);

	preparesavegamelist(ctrllist);
	CSCIMessage mes;
	lpTemp = nullptr;
	int toret = -1;
	while (1) {
		CSCIWaitMessage(&mes);      //printf("mess: %d, id %d ",mes.code,mes.id);
		if (mes.code == CM_COMMAND) {
			if (mes.id == ctrlok) {
				int cursel = CSCISendControlMessage(ctrllist, CLB_GETCURSEL, 0, 0);
				if ((cursel >= numsaves) | (cursel < 0))
					lpTemp = nullptr;
				else {
					toret = filenumbers[cursel];
					String path = get_save_game_path(toret);
					strcpy(bufTemp, path);
					lpTemp = &bufTemp[0];
				}
			} else if (mes.id == ctrlcancel) {
				lpTemp = nullptr;
			}

			break;
		}
	}

	CSCIDeleteControl(ctrltex1);
	CSCIDeleteControl(ctrllist);
	CSCIDeleteControl(ctrlok);
	CSCIDeleteControl(ctrlcancel);
	CSCIEraseWindow(handl);
	return toret;
}

int savegamedialog() {
	char okbuttontext[50];
	strcpy(okbuttontext, get_global_message(MSG_SAVEBUTTON));
	char labeltext[200];
	strcpy(labeltext, get_global_message(MSG_SAVEDIALOG));
	const int wnd_width = 200;
	const int wnd_height = 120;
	const int boxleft = myscrnwid / 2 - wnd_width / 2;
	const int boxtop = myscrnhit / 2 - wnd_height / 2;
	const int buttonhit = usetup.textheight + 5;
	int labeltop = 5;

	int handl = CSCIDrawWindow(boxleft, boxtop, wnd_width, wnd_height);
	int ctrlcancel =
		CSCICreateControl(CNT_PUSHBUTTON | CNF_CANCEL, 135, 5 + buttonhit, 60, 10,
			get_global_message(MSG_CANCEL));
	int ctrllist = CSCICreateControl(CNT_LISTBOX, 10, 40, 120, 80, nullptr);
	int ctrltbox = 0;

	CSCISendControlMessage(ctrllist, CLB_CLEAR, 0, 0);    // clear the list box
	preparesavegamelist(ctrllist);
	if (toomanygames) {
		strcpy(okbuttontext, get_global_message(MSG_REPLACE));
		strcpy(labeltext, get_global_message(MSG_MUSTREPLACE));
		labeltop = 2;
	} else
		ctrltbox = CSCICreateControl(CNT_TEXTBOX, 10, 29, 120, 0, nullptr);

	int ctrlok = CSCICreateControl(CNT_PUSHBUTTON | CNF_DEFAULT, 135, 5, 60, 10, okbuttontext);
	int ctrltex1 = CSCICreateControl(CNT_LABEL, 10, labeltop, 120, 0, labeltext);
	CSCIMessage mes;

	lpTemp = nullptr;
	if (numsaves > 0)
		CSCISendControlMessage(ctrllist, CLB_GETTEXT, 0, (long)&buffer2[0]);
	else
		buffer2[0] = 0;

	CSCISendControlMessage(ctrltbox, CTB_SETTEXT, 0, (long)&buffer2[0]);

	int toret = -1;
	while (1) {
		CSCIWaitMessage(&mes);      //printf("mess: %d, id %d ",mes.code,mes.id);
		if (mes.code == CM_COMMAND) {
			if (mes.id == ctrlok) {
				int cursell = CSCISendControlMessage(ctrllist, CLB_GETCURSEL, 0, 0);
				CSCISendControlMessage(ctrltbox, CTB_GETTEXT, 0, (long)&buffer2[0]);

				if (numsaves > 0)
					CSCISendControlMessage(ctrllist, CLB_GETTEXT, cursell, (long)&bufTemp[0]);
				else
					strcpy(bufTemp, "_NOSAVEGAMENAME");

				if (toomanygames) {
					int nwhand = CSCIDrawWindow(boxleft + 5, boxtop + 20, 190, 65);
					int lbl1 =
						CSCICreateControl(CNT_LABEL, 15, 5, 160, 0, get_global_message(MSG_REPLACEWITH1));
					int lbl2 = CSCICreateControl(CNT_LABEL, 25, 14, 160, 0, bufTemp);
					int lbl3 =
						CSCICreateControl(CNT_LABEL, 15, 25, 160, 0, get_global_message(MSG_REPLACEWITH2));
					int txt1 = CSCICreateControl(CNT_TEXTBOX, 15, 35, 160, 0, bufTemp);
					int btnOk =
						CSCICreateControl(CNT_PUSHBUTTON | CNF_DEFAULT, 25, 50, 60, 10,
							get_global_message(MSG_REPLACE));
					int btnCancel =
						CSCICreateControl(CNT_PUSHBUTTON | CNF_CANCEL, 95, 50, 60, 10,
							get_global_message(MSG_CANCEL));

					CSCIMessage cmes;
					do {
						CSCIWaitMessage(&cmes);
					} while (cmes.code != CM_COMMAND);

					CSCISendControlMessage(txt1, CTB_GETTEXT, 0, (long)&buffer2[0]);
					CSCIDeleteControl(btnCancel);
					CSCIDeleteControl(btnOk);
					CSCIDeleteControl(txt1);
					CSCIDeleteControl(lbl3);
					CSCIDeleteControl(lbl2);
					CSCIDeleteControl(lbl1);
					CSCIEraseWindow(nwhand);
					bufTemp[0] = 0;

					if (cmes.id == btnCancel) {
						lpTemp = nullptr;
						break;
					} else
						toret = filenumbers[cursell];

				} else if (strcmp(buffer2, bufTemp) != 0) {   // create a new game (description different)
					int highestnum = 0;
					for (int pp = 0; pp < numsaves; pp++) {
						if (filenumbers[pp] > highestnum)
							highestnum = filenumbers[pp];
					}

					if (highestnum > 90)
						quit("Save game directory overflow");

					toret = highestnum + 1;
					String path = get_save_game_path(toret);
					strcpy(bufTemp, path);
				} else {
					toret = filenumbers[cursell];
					bufTemp[0] = 0;
				}

				if (bufTemp[0] == 0) {
					String path = get_save_game_path(toret);
					strcpy(bufTemp, path);
				}

				lpTemp = &bufTemp[0];
				lpTemp2 = &buffer2[0];
			} else if (mes.id == ctrlcancel) {
				lpTemp = nullptr;
			}
			break;
		} else if (mes.code == CM_SELCHANGE) {
			int cursel = CSCISendControlMessage(ctrllist, CLB_GETCURSEL, 0, 0);
			if (cursel >= 0) {
				CSCISendControlMessage(ctrllist, CLB_GETTEXT, cursel, (long)&buffer2[0]);
				CSCISendControlMessage(ctrltbox, CTB_SETTEXT, 0, (long)&buffer2[0]);
			}
		}
	}

	CSCIDeleteControl(ctrltbox);
	CSCIDeleteControl(ctrltex1);
	CSCIDeleteControl(ctrllist);
	CSCIDeleteControl(ctrlok);
	CSCIDeleteControl(ctrlcancel);
	CSCIEraseWindow(handl);
	return toret;
}

void preparesavegamelist(int ctrllist) {
	numsaves = 0;
	toomanygames = 0;
	al_ffblk ffb;
	int bufix = 0;

	String svg_dir = get_save_game_directory();
	String svg_suff = get_save_game_suffix();
	String searchPath = String::FromFormat("%s""agssave.*%s", svg_dir.GetCStr(), svg_suff.GetCStr());

	int don = al_findfirst(searchPath, &ffb, -1);
	while (!don) {
		bufix = 0;
		if (numsaves >= MAXSAVEGAMES) {
			toomanygames = 1;
			break;
		}

		// only list games .000 to .099 (to allow higher slots for other purposes)
		if (strstr(ffb.name, ".0") == nullptr) {
			don = al_findnext(&ffb);
			continue;
		}

		const char *numberExtension = strstr(ffb.name, ".0") + 1;
		int sgNumber = atoi(numberExtension);

		String thisGamePath = get_save_game_path(sgNumber);

		// get description
		String description;
		read_savedgame_description(thisGamePath, description);

		CSCISendControlMessage(ctrllist, CLB_ADDITEM, 0, (long)description.GetCStr());
		// Select the first item
		CSCISendControlMessage(ctrllist, CLB_SETCURSEL, 0, 0);
		filenumbers[numsaves] = sgNumber;
		filedates[numsaves] = (long int)ffb.time;
		numsaves++;
		don = al_findnext(&ffb);
	}

	al_findclose(&ffb);
	if (numsaves >= MAXSAVEGAMES)
		toomanygames = 1;

	for (int nn = 0; nn < numsaves - 1; nn++) {
		for (int kk = 0; kk < numsaves - 1; kk++) { // Date order the games
			if (filedates[kk] < filedates[kk + 1]) {  // swap them round
				CSCISendControlMessage(ctrllist, CLB_GETTEXT, kk, (long)&buff[0]);
				CSCISendControlMessage(ctrllist, CLB_GETTEXT, kk + 1, (long)&buffer2[0]);
				CSCISendControlMessage(ctrllist, CLB_SETTEXT, kk + 1, (long)&buff[0]);
				CSCISendControlMessage(ctrllist, CLB_SETTEXT, kk, (long)&buffer2[0]);
				int numtem = filenumbers[kk];
				filenumbers[kk] = filenumbers[kk + 1];
				filenumbers[kk + 1] = numtem;
				long numted = filedates[kk];
				filedates[kk] = filedates[kk + 1];
				filedates[kk + 1] = numted;
			}
		}
	}
}

void enterstringwindow(const char *prompttext, char *stouse) {
	const int wnd_width = 200;
	const int wnd_height = 40;
	const int boxleft = 60, boxtop = 80;
	int wantCancel = 0;
	if (prompttext[0] == '!') {
		wantCancel = 1;
		prompttext++;
	}

	int handl = CSCIDrawWindow(boxleft, boxtop, wnd_width, wnd_height);
	int ctrlok = CSCICreateControl(CNT_PUSHBUTTON | CNF_DEFAULT, 135, 5, 60, 10, "OK");
	int ctrlcancel = -1;
	if (wantCancel)
		ctrlcancel = CSCICreateControl(CNT_PUSHBUTTON | CNF_CANCEL, 135, 20, 60, 10, get_global_message(MSG_CANCEL));
	int ctrltbox = CSCICreateControl(CNT_TEXTBOX, 10, 29, 120, 0, nullptr);
	int ctrltex1 = CSCICreateControl(CNT_LABEL, 10, 5, 120, 0, prompttext);
	CSCIMessage mes;

	while (1) {
		CSCIWaitMessage(&mes);
		if (mes.code == CM_COMMAND) {
			if (mes.id == ctrlcancel)
				buffer2[0] = 0;
			else
				CSCISendControlMessage(ctrltbox, CTB_GETTEXT, 0, (long)&buffer2[0]);
			break;
		}
	}

	CSCIDeleteControl(ctrltex1);
	CSCIDeleteControl(ctrltbox);
	CSCIDeleteControl(ctrlok);
	if (wantCancel)
		CSCIDeleteControl(ctrlcancel);
	CSCIEraseWindow(handl);
	strcpy(stouse, buffer2);
}

int enternumberwindow(char *prompttext) {
	char ourbuf[200];
	enterstringwindow(prompttext, ourbuf);
	if (ourbuf[0] == 0)
		return -9999;
	return atoi(ourbuf);
}

int roomSelectorWindow(int currentRoom, int numRooms, int *roomNumbers, char **roomNames) {
	char labeltext[200];
	strcpy(labeltext, get_global_message(MSG_SAVEDIALOG));
	const int wnd_width = 240;
	const int wnd_height = 160;
	const int boxleft = myscrnwid / 2 - wnd_width / 2;
	const int boxtop = myscrnhit / 2 - wnd_height / 2;
	const int labeltop = 5;

	int handl = CSCIDrawWindow(boxleft, boxtop, wnd_width, wnd_height);
	int ctrllist = CSCICreateControl(CNT_LISTBOX, 10, 40, 220, 100, nullptr);
	int ctrlcancel =
		CSCICreateControl(CNT_PUSHBUTTON | CNF_CANCEL, 80, 145, 60, 10, "Cancel");

	CSCISendControlMessage(ctrllist, CLB_CLEAR, 0, 0);    // clear the list box
	for (int aa = 0; aa < numRooms; aa++) {
		sprintf(buff, "%3d %s", roomNumbers[aa], roomNames[aa]);
		CSCISendControlMessage(ctrllist, CLB_ADDITEM, 0, (long)&buff[0]);
		if (roomNumbers[aa] == currentRoom) {
			CSCISendControlMessage(ctrllist, CLB_SETCURSEL, aa, 0);
		}
	}

	int ctrlok = CSCICreateControl(CNT_PUSHBUTTON | CNF_DEFAULT, 10, 145, 60, 10, "OK");
	int ctrltex1 = CSCICreateControl(CNT_LABEL, 10, labeltop, 180, 0, "Choose which room to go to:");
	CSCIMessage mes;

	lpTemp = nullptr;
	buffer2[0] = 0;

	int ctrltbox = CSCICreateControl(CNT_TEXTBOX, 10, 29, 120, 0, nullptr);
	CSCISendControlMessage(ctrltbox, CTB_SETTEXT, 0, (long)&buffer2[0]);

	int toret = -1;
	while (1) {
		CSCIWaitMessage(&mes);      //printf("mess: %d, id %d ",mes.code,mes.id);
		if (mes.code == CM_COMMAND) {
			if (mes.id == ctrlok) {
				CSCISendControlMessage(ctrltbox, CTB_GETTEXT, 0, (long)&buffer2[0]);
				if (isdigit(buffer2[0])) {
					toret = atoi(buffer2);
				}
			} else if (mes.id == ctrlcancel) {
			}
			break;
		} else if (mes.code == CM_SELCHANGE) {
			int cursel = CSCISendControlMessage(ctrllist, CLB_GETCURSEL, 0, 0);
			if (cursel >= 0) {
				sprintf(buffer2, "%d", roomNumbers[cursel]);
				CSCISendControlMessage(ctrltbox, CTB_SETTEXT, 0, (long)&buffer2[0]);
			}
		}
	}

	CSCIDeleteControl(ctrltbox);
	CSCIDeleteControl(ctrltex1);
	CSCIDeleteControl(ctrllist);
	CSCIDeleteControl(ctrlok);
	CSCIDeleteControl(ctrlcancel);
	CSCIEraseWindow(handl);
	return toret;
}

int myscimessagebox(const char *lpprompt, char *btn1, char *btn2) {
	const int wnd_width = 240 - 80;
	const int wnd_height = 120 - 80;
	const int boxleft = 80;
	const int boxtop = 80;

	int windl = CSCIDrawWindow(boxleft, boxtop, wnd_width, wnd_height);
	int lbl1 = CSCICreateControl(CNT_LABEL, 10, 5, 150, 0, lpprompt);
	int btflag = CNT_PUSHBUTTON;

	if (btn2 == nullptr)
		btflag |= CNF_DEFAULT | CNF_CANCEL;
	else
		btflag |= CNF_DEFAULT;

	int btnQuit = CSCICreateControl(btflag, 10, 25, 60, 10, btn1);
	int btnPlay = 0;

	if (btn2 != nullptr)
		btnPlay = CSCICreateControl(CNT_PUSHBUTTON | CNF_CANCEL, 85, 25, 60, 10, btn2);

	smes.code = 0;

	do {
		CSCIWaitMessage(&smes);
	} while (smes.code != CM_COMMAND);

	if (btnPlay)
		CSCIDeleteControl(btnPlay);

	CSCIDeleteControl(btnQuit);
	CSCIDeleteControl(lbl1);
	CSCIEraseWindow(windl);

	if (smes.id == btnQuit)
		return 1;

	return 0;
}

int quitdialog() {
	char quitbut[50], playbut[50];
	strcpy(quitbut, get_global_message(MSG_QUITBUTTON));
	strcpy(playbut, get_global_message(MSG_PLAYBUTTON));
	return myscimessagebox(get_global_message(MSG_QUITDIALOG), quitbut, playbut);
}

} // namespace AGS3
