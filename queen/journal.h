/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef QUEENJOURNAL_H
#define QUEENJOURNAL_H

#include "common/util.h"

namespace Queen {

class QueenEngine;

class Journal {
public:

	Journal(QueenEngine *vm);
	void use();
  
	enum {
		JOURNAL_BANK   = 8,
		JOURNAL_FRAMES = 40,
		JOURNAL_ROOM   = 0 // dummy value to make Graphics methods happy
	};

	enum {
		ZN_REVIEW_ENTRY = 1,
		ZN_MAKE_ENTRY   = 2,
		ZN_YES          = ZN_MAKE_ENTRY,
		ZN_CLOSE        = 3,
		ZN_NO           = ZN_CLOSE,
		ZN_GIVEUP       = 4,
		ZN_TEXT_SPEED   = 5,
		ZN_SFX_TOGGLE   = 6,
		ZN_MUSIC_VOLUME = 7,
		ZN_DESC_FIRST   = 8,
		ZN_DESC_LAST    = 17,
		ZN_PAGE_FIRST   = 18,
		ZN_PAGE_LAST    = 27,
		ZN_INFO_BOX     = 28,
		ZN_MUSIC_TOGGLE = 29,
		ZN_VOICE_TOGGLE = 30,
		ZN_TEXT_TOGGLE  = 31
	};

	enum {
		BOB_LEFT_RECT_1   =  1,
		BOB_LEFT_RECT_2   =  2,
		BOB_LEFT_RECT_3   =  3,
		BOB_LEFT_RECT_4   =  4,
		BOB_TALK_SPEED    =  5,
		BOB_SFX_TOGGLE    =  6,
		BOB_MUSIC_VOLUME  =  7,
		BOB_SAVE_DESC     =  8,
		BOB_SAVE_PAGE     =  9,
		BOB_SPEECH_TOGGLE = 10,
		BOB_TEXT_TOGGLE   = 11,
		BOB_MUSIC_TOGGLE  = 12,
		BOB_INFO_BOX      = 13
	};

	enum {
		FRAME_BLUE_1    =  1,
		FRAME_BLUE_2    =  2,
		FRAME_ORANGE    =  3,
		FRAME_GREY      =  5,
		FRAME_CHECK_BOX = 16,
		FRAME_BLUE_PIN  = 18,
		FRAME_GREEN_PIN = 19,
		FRAME_INFO_BOX  = 20
	};

	enum {
		TXT_CLOSE        = 30,
		TXT_GIVE_UP      = 31,
		TXT_MAKE_ENTRY   = 32,
		TXT_REVIEW_ENTRY = 33,
		TXT_YES          = 34,
		TXT_NO           = 35
	};

	enum {
		SAVE_PER_PAGE     = 10,
		MAX_SAVE_DESC_LEN = 32,
		MAX_SAVE_DESC_NUM = 10 * SAVE_PER_PAGE,
		MAX_PANEL_TEXTS   = 4 * 2
	};

	enum Mode {
		M_NORMAL,
		M_INFO_BOX,
		M_YES_NO
	};


private:

	void prepare();
	void restore();
	
	void redraw();
	void update();

	void showBob(int bobNum, int16 x, int16 y, int frameNum);
	void hideBob(int bobNum);

	void findSaveDescriptions();
	void drawSaveDescriptions();
	void drawSaveSlot();

	void enterYesNoMode(int16 zoneNum, int titleNum);
	void exitYesNoMode();

	void handleNormalMode(int16 zoneNum, int mousex);
	void handleInfoBoxMode(int16 zoneNum);
	void handleYesNoMode(int16 zoneNum);

	void handleMouseWheel(int inc);
	void handleMouseDown(int x, int y);
	void handleKeyDown(uint16 ascii, int keycode);

	void clearPanelTexts();
	void drawPanelText(int y, const char *text);
	void drawCheckBox(bool active, int bobNum, int16 x, int16 y, int frameNum);
	void drawSlideBar(int value, int hi, int lo, int bobNum, int16 x, int16 y, int frameNum);
	void drawPanel(const int *frames, const int *titles, int n);
	void drawNormalPanel();
	void drawYesNoPanel(int titleNum);
	void drawConfigPanel();
	
	void showInformationBox();
	void hideInformationBox();

	void initEditBuffer(const char *desc);
	void updateEditBuffer(uint16 ascii, int keycode);

	void makeSavegameName(char *buf, int slot = -1);
	void saveState(int slot, const char *desc);
	void loadState(int slot);


	struct {
		bool enable;
		int posCursor;
		uint textCharsCount;
		char text[MAX_SAVE_DESC_LEN];
	} _edit;

	int _currentSavePage;
	int _currentSaveSlot;

	int _prevJoeX, _prevJoeY;

	int _panelTextCount;
	int _panelTextY[MAX_PANEL_TEXTS];

	uint16 _prevZoneNum;
	char _saveDescriptions[MAX_SAVE_DESC_NUM][MAX_SAVE_DESC_LEN];
	Mode _mode;
	bool _quit;
	bool _quitCleanly;

	QueenEngine *_vm;
};


} // End of namespace Queen

#endif
