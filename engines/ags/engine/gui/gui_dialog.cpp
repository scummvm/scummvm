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

#include "common/std/algorithm.h"
#include "ags/lib/allegro.h" // find files
#include "ags/engine/gui/gui_dialog.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/gui/csci_dialog.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/util/path.h"
#include "ags/ags.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

#undef MAXSAVEGAMES
#define MAXSAVEGAMES 20

char *get_gui_dialog_buffer() {
	return _G(buffer2);
}

//
// TODO: rewrite the whole thing to work inside the main game update and render loop!
//

Bitmap *prepare_gui_screen(int x, int y, int width, int height, bool opaque) {
	_G(windowPosX) = x;
	_G(windowPosY) = y;
	_G(windowPosWidth) = width;
	_G(windowPosHeight) = height;
	if (_G(windowBuffer)) {
		_G(windowBuffer) = recycle_bitmap(_G(windowBuffer), _G(windowBuffer)->GetColorDepth(), _G(windowPosWidth), _G(windowPosHeight), !opaque);
	} else {
		_G(windowBuffer) = CreateCompatBitmap(_G(windowPosWidth), _G(windowPosHeight));
	}
	_G(dialogDDB) = recycle_ddb_bitmap(_G(dialogDDB), _G(windowBuffer), false, opaque);
	return _G(windowBuffer);
}

Bitmap *get_gui_screen() {
	return _G(windowBuffer);
}

void clear_gui_screen() {
	if (_G(dialogDDB))
		_G(gfxDriver)->DestroyDDB(_G(dialogDDB));
	_G(dialogDDB) = nullptr;
	delete _G(windowBuffer);
	_G(windowBuffer) = nullptr;
}

void refresh_gui_screen() {
	_G(gfxDriver)->UpdateDDBFromBitmap(_G(dialogDDB), _G(windowBuffer), false);
	UpdateCursorAndDrawables();
	render_graphics(_G(dialogDDB), _G(windowPosX), _G(windowPosY));
}

int loadgamedialog() {
	const int wnd_width = 200;
	const int wnd_height = 120;
	const int boxleft = _G(myscrnwid) / 2 - wnd_width / 2;
	const int boxtop = _G(myscrnhit) / 2 - wnd_height / 2;
	const int buttonhit = _GP(usetup).textheight + 5;

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
	_G(lpTemp) = nullptr;
	int toret = -1;
	while (1) {
		CSCIWaitMessage(&mes);      //printf("mess: %d, id %d ",mes.code,mes.id);
		if (mes.code == CM_COMMAND) {
			if (mes.id == ctrlok) {
				int cursel = CSCISendControlMessage(ctrllist, CLB_GETCURSEL, 0, 0);
				if ((cursel >= _G(numsaves)) | (cursel < 0))
					_G(lpTemp) = nullptr;
				else {
					toret = _G(filenumbers)[cursel];
					String path = get_save_game_path(toret);
					Common::strcpy_s(_G(bufTemp), path.GetCStr());
					_G(lpTemp) = &_G(bufTemp)[0];
				}
			} else if (mes.id == ctrlcancel) {
				_G(lpTemp) = nullptr;
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
	Common::strcpy_s(okbuttontext, get_global_message(MSG_SAVEBUTTON));
	char labeltext[200];
	Common::strcpy_s(labeltext, get_global_message(MSG_SAVEDIALOG));
	const int wnd_width = 200;
	const int wnd_height = 120;
	const int boxleft = _G(myscrnwid) / 2 - wnd_width / 2;
	const int boxtop = _G(myscrnhit) / 2 - wnd_height / 2;
	const int buttonhit = _GP(usetup).textheight + 5;
	int labeltop = 5;

	int handl = CSCIDrawWindow(boxleft, boxtop, wnd_width, wnd_height);
	int ctrlcancel =
	    CSCICreateControl(CNT_PUSHBUTTON | CNF_CANCEL, 135, 5 + buttonhit, 60, 10,
	                      get_global_message(MSG_CANCEL));
	int ctrllist = CSCICreateControl(CNT_LISTBOX, 10, 40, 120, 80, nullptr);
	int ctrltbox = 0;

	CSCISendControlMessage(ctrllist, CLB_CLEAR, 0, 0);    // clear the list box
	preparesavegamelist(ctrllist);
	if (_G(toomanygames)) {
		Common::strcpy_s(okbuttontext, get_global_message(MSG_REPLACE));
		Common::strcpy_s(labeltext, get_global_message(MSG_MUSTREPLACE));
		labeltop = 2;
	} else
		ctrltbox = CSCICreateControl(CNT_TEXTBOX, 10, 29, 120, 0, nullptr);

	int ctrlok = CSCICreateControl(CNT_PUSHBUTTON | CNF_DEFAULT, 135, 5, 60, 10, okbuttontext);
	int ctrltex1 = CSCICreateControl(CNT_LABEL, 10, labeltop, 120, 0, labeltext);
	CSCIMessage mes;

	_G(lpTemp) = nullptr;
	if (_G(numsaves) > 0)
		CSCISendControlMessage(ctrllist, CLB_GETTEXT, 0, &_G(buffer2)[0]);
	else
		_G(buffer2)[0] = 0;

	CSCISendControlMessage(ctrltbox, CTB_SETTEXT, 0, &_G(buffer2)[0]);

	int toret = -1;
	while (1) {
		CSCIWaitMessage(&mes);      //printf("mess: %d, id %d ",mes.code,mes.id);
		if (mes.code == CM_COMMAND) {
			if (mes.id == ctrlok) {
				int cursell = CSCISendControlMessage(ctrllist, CLB_GETCURSEL, 0, 0);
				CSCISendControlMessage(ctrltbox, CTB_GETTEXT, 0, &_G(buffer2)[0]);

				if (_G(numsaves) > 0)
					CSCISendControlMessage(ctrllist, CLB_GETTEXT, cursell, &_G(bufTemp)[0]);
				else
					Common::strcpy_s(_G(bufTemp), "_NOSAVEGAMENAME");

				if (_G(toomanygames)) {
					int nwhand = CSCIDrawWindow(boxleft + 5, boxtop + 20, 190, 65);
					int lbl1 =
					    CSCICreateControl(CNT_LABEL, 15, 5, 160, 0, get_global_message(MSG_REPLACEWITH1));
					int lbl2 = CSCICreateControl(CNT_LABEL, 25, 14, 160, 0, _G(bufTemp));
					int lbl3 =
					    CSCICreateControl(CNT_LABEL, 15, 25, 160, 0, get_global_message(MSG_REPLACEWITH2));
					int txt1 = CSCICreateControl(CNT_TEXTBOX, 15, 35, 160, 0, _G(bufTemp));
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

					CSCISendControlMessage(txt1, CTB_GETTEXT, 0, &_G(buffer2)[0]);
					CSCIDeleteControl(btnCancel);
					CSCIDeleteControl(btnOk);
					CSCIDeleteControl(txt1);
					CSCIDeleteControl(lbl3);
					CSCIDeleteControl(lbl2);
					CSCIDeleteControl(lbl1);
					CSCIEraseWindow(nwhand);
					_G(bufTemp)[0] = 0;

					if (cmes.id == btnCancel) {
						_G(lpTemp) = nullptr;
						break;
					} else
						toret = _G(filenumbers)[cursell];

				} else if (strcmp(_G(buffer2), _G(bufTemp)) != 0) {     // create a new game (description different)
					int highestnum = 0;
					for (int pp = 0; pp < _G(numsaves); pp++) {
						if (_G(filenumbers)[pp] > highestnum)
							highestnum = _G(filenumbers)[pp];
					}

					if (highestnum > 90)
						quit("Save game directory overflow");

					toret = highestnum + 1;
					String path = get_save_game_path(toret);
					Common::strcpy_s(_G(bufTemp), path.GetCStr());
				} else {
					toret = _G(filenumbers)[cursell];
					_G(bufTemp)[0] = 0;
				}

				if (_G(bufTemp)[0] == 0) {
					String path = get_save_game_path(toret);
					Common::strcpy_s(_G(bufTemp), path.GetCStr());
				}

				_G(lpTemp) = &_G(bufTemp)[0];
				_G(lpTemp2) = &_G(buffer2)[0];
			} else if (mes.id == ctrlcancel) {
				_G(lpTemp) = nullptr;
			}
			break;
		} else if (mes.code == CM_SELCHANGE) {
			int cursel = CSCISendControlMessage(ctrllist, CLB_GETCURSEL, 0, 0);
			if (cursel >= 0) {
				CSCISendControlMessage(ctrllist, CLB_GETTEXT, cursel, &_G(buffer2)[0]);
				CSCISendControlMessage(ctrltbox, CTB_SETTEXT, 0, &_G(buffer2)[0]);
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
	// Get a list of savegames
	SaveStateList saveList = ::AGS::g_vm->listSaves();

	// The original AGS sorts the list from most recent to oldest.
	// We don't have the modification date in ScummVM though. We could try to
	// parse the date string, but for now, sort by decreasing slot number, which
	// should work better than the default sort by increasing slot.
	Common::sort(saveList.begin(), saveList.end(),
	[](const SaveStateDescriptor & x, const SaveStateDescriptor & y) {
		return x.getSaveSlot() > y.getSaveSlot();
	});

	// fill in the list box and global savegameindex[] array for backward compatibilty
	for (_G(numsaves) = 0; _G(numsaves) < (int)saveList.size(); ++_G(numsaves)) {
		CSCISendControlMessage(ctrllist, CLB_ADDITEM, 0,
		                       saveList[_G(numsaves)].getDescription().encode().c_str());
		_G(filenumbers)[_G(numsaves)] = saveList[_G(numsaves)].getSaveSlot();
		_G(filedates)[_G(numsaves)] = 0;
		//(long int)saveList[_G(numsaves)].FileTime;
	}
	_G(toomanygames) = (_G(numsaves) >= MAXSAVEGAMES) ? 1 : 0;
	// Select the first item
	CSCISendControlMessage(ctrllist, CLB_SETCURSEL, 0, 0);
}

void enterstringwindow(const char *prompttext, char *dst_buf, size_t dst_sz) {
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
				_G(buffer2)[0] = 0;
			else
				CSCISendControlMessage(ctrltbox, CTB_GETTEXT, 0, &_G(buffer2)[0]);
			break;
		}
	}

	CSCIDeleteControl(ctrltex1);
	CSCIDeleteControl(ctrltbox);
	CSCIDeleteControl(ctrlok);
	if (wantCancel)
		CSCIDeleteControl(ctrlcancel);
	CSCIEraseWindow(handl);
	snprintf(dst_buf, dst_sz, "%s", _G(buffer2));
}

int enternumberwindow(char *prompttext) {
	char ourbuf[200];
	enterstringwindow(prompttext, ourbuf, sizeof(ourbuf));
	if (ourbuf[0] == 0)
		return -9999;
	return atoi(ourbuf);
}

int roomSelectorWindow(int currentRoom, int numRooms,
		const std::vector<int> &roomNumbers, const std::vector<String> &roomNames) {
	char labeltext[200];
	Common::strcpy_s(labeltext, get_global_message(MSG_SAVEDIALOG));
	const int wnd_width = 240;
	const int wnd_height = 160;
	const int boxleft = _G(myscrnwid) / 2 - wnd_width / 2;
	const int boxtop = _G(myscrnhit) / 2 - wnd_height / 2;
	const int labeltop = 5;

	int handl = CSCIDrawWindow(boxleft, boxtop, wnd_width, wnd_height);
	int ctrllist = CSCICreateControl(CNT_LISTBOX, 10, 40, 220, 100, nullptr);
	int ctrlcancel =
	    CSCICreateControl(CNT_PUSHBUTTON | CNF_CANCEL, 80, 145, 60, 10, "Cancel");

	CSCISendControlMessage(ctrllist, CLB_CLEAR, 0, 0);    // clear the list box
	for (int aa = 0; aa < numRooms; aa++) {
		snprintf(_G(buff), sizeof(_G(buff)), "%3d %s", roomNumbers[aa], roomNames[aa].GetCStr());
		CSCISendControlMessage(ctrllist, CLB_ADDITEM, 0, &_G(buff)[0]);
		if (roomNumbers[aa] == currentRoom) {
			CSCISendControlMessage(ctrllist, CLB_SETCURSEL, aa, 0);
		}
	}

	int ctrlok = CSCICreateControl(CNT_PUSHBUTTON | CNF_DEFAULT, 10, 145, 60, 10, "OK");
	int ctrltex1 = CSCICreateControl(CNT_LABEL, 10, labeltop, 180, 0, "Choose which room to go to:");
	CSCIMessage mes;

	_G(lpTemp) = nullptr;
	_G(buffer2)[0] = 0;

	int ctrltbox = CSCICreateControl(CNT_TEXTBOX, 10, 29, 120, 0, nullptr);
	CSCISendControlMessage(ctrltbox, CTB_SETTEXT, 0, &_G(buffer2)[0]);

	int toret = -1;
	while (1) {
		CSCIWaitMessage(&mes);      //printf("mess: %d, id %d ",mes.code,mes.id);
		if (mes.code == CM_COMMAND) {
			if (mes.id == ctrlok) {
				CSCISendControlMessage(ctrltbox, CTB_GETTEXT, 0, &_G(buffer2)[0]);
				if (Common::isDigit(_G(buffer2)[0])) {
					toret = atoi(_G(buffer2));
				}
			} else if (mes.id == ctrlcancel) {
			}
			break;
		} else if (mes.code == CM_SELCHANGE) {
			int cursel = CSCISendControlMessage(ctrllist, CLB_GETCURSEL, 0, 0);
			if (cursel >= 0) {
				snprintf(_G(buffer2), sizeof(_G(buffer2)), "%d", roomNumbers[cursel]);
				CSCISendControlMessage(ctrltbox, CTB_SETTEXT, 0, &_G(buffer2)[0]);
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

	_GP(smes).code = 0;

	do {
		if (SHOULD_QUIT)
			return 1;

		CSCIWaitMessage(&_GP(smes));
	} while (_GP(smes).code != CM_COMMAND);

	if (btnPlay)
		CSCIDeleteControl(btnPlay);

	CSCIDeleteControl(btnQuit);
	CSCIDeleteControl(lbl1);
	CSCIEraseWindow(windl);

	if (_GP(smes).id == btnQuit)
		return 1;

	return 0;
}

int quitdialog() {
	char quitbut[50], playbut[50];
	Common::strcpy_s(quitbut, get_global_message(MSG_QUITBUTTON));
	Common::strcpy_s(playbut, get_global_message(MSG_PLAYBUTTON));
	return myscimessagebox(get_global_message(MSG_QUITDIALOG), quitbut, playbut);
}

} // namespace AGS3
