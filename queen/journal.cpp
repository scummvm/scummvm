/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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

#include "stdafx.h"
#include "queen/journal.h"

#include "queen/bankman.h"
#include "queen/display.h"
#include "queen/graphics.h"
#include "queen/grid.h"
#include "queen/logic.h"
#include "queen/queen.h"
#include "queen/resource.h"
#include "queen/sound.h"

namespace Queen {


Journal::Journal(QueenEngine *vm)
	: _vm(vm) {
	_currentSavePage = 0;
	_currentSaveSlot = 0;
}


void Journal::use() {
	BobSlot *joe = _vm->graphics()->bob(0);
	_prevJoeX = joe->x;
	_prevJoeY = joe->y;

	_edit.enable = false;
	_mode = M_NORMAL;

	memset(_saveDescriptions, 0, sizeof(_saveDescriptions));
	_vm->findGameStateDescriptions(_saveDescriptions);

	_panelTextCount = 0;
	_vm->display()->palFadeOut(0, 255, JOURNAL_ROOM);
	prepare();
	redraw();
	update();
	_vm->display()->palFadeIn(0, 255, JOURNAL_ROOM);

	_quitCleanly = true;
	_quit = false;
	OSystem *system = OSystem::instance();
	while (!_quit) {
		OSystem::Event event;
		while (system->poll_event(&event)) {
			switch (event.event_code) {
			case OSystem::EVENT_KEYDOWN:
				handleKeyDown(event.kbd.ascii, event.kbd.keycode);
				break;
			case OSystem::EVENT_LBUTTONDOWN:
				handleMouseDown(event.mouse.x, event.mouse.y);
				break;
			case OSystem::EVENT_WHEELUP:
				handleMouseWheel(-1);
				break;
			case OSystem::EVENT_WHEELDOWN:
				handleMouseWheel(1);
				break;
			case OSystem::EVENT_QUIT:
				system->quit();
				break;
			default:
				break;
			}
		}
		system->delay_msecs(20);
	}

	_vm->writeOptionSettings();

	_vm->display()->clearTexts(0, GAME_SCREEN_HEIGHT - 1);
	_vm->graphics()->putCameraOnBob(0);
	if (_quitCleanly) {
		restore();
	}
}


void Journal::prepare() {
	_vm->display()->horizontalScroll(0);
	_vm->display()->fullscreen(true);

	_vm->graphics()->putCameraOnBob(-1);
	_vm->graphics()->clearBobs();
	_vm->display()->clearTexts(0, GAME_SCREEN_HEIGHT - 1);
	_vm->bankMan()->eraseFrames(false);
	_vm->display()->textCurrentColor(INK_JOURNAL);

	int i;
	_vm->grid()->clear(GS_ROOM);
	for (i = 0; i < 4; ++i) { // left panel
		_vm->grid()->setZone(GS_ROOM, i + 1, 32, 8 + i * 48, 96, 40 + i * 48);
	}
	_vm->grid()->setZone(GS_ROOM, ZN_TEXT_SPEED, 136, 169, 265, 176);
	_vm->grid()->setZone(GS_ROOM, ZN_SFX_TOGGLE, 221 - 24, 155, 231, 164);
	_vm->grid()->setZone(GS_ROOM, ZN_MUSIC_VOLUME, 136, 182, 265, 189);
	for (i = 0; i < 10; ++i) { // right panel
		_vm->grid()->setZone(GS_ROOM, ZN_DESC_FIRST + i, 131, 7 + i * 13, 290, 18 + i * 13);
		_vm->grid()->setZone(GS_ROOM, ZN_PAGE_FIRST + i, 300, 4 + i * 15, 319, 17 + i * 15);
	}
	_vm->grid()->setZone(GS_ROOM, ZN_INFO_BOX, 273, 146, 295, 189);
	_vm->grid()->setZone(GS_ROOM, ZN_MUSIC_TOGGLE, 125 - 16, 181, 135, 190);
	_vm->grid()->setZone(GS_ROOM, ZN_VOICE_TOGGLE, 158 - 24, 155, 168, 164);
	_vm->grid()->setZone(GS_ROOM, ZN_TEXT_TOGGLE, 125 - 16, 168, 135, 177);

	_vm->display()->setupNewRoom("journal", JOURNAL_ROOM);
	_vm->bankMan()->load("journal.BBK", JOURNAL_BANK);
	for (i = 1; i <= 20; ++i) {
		int frameNum = JOURNAL_FRAMES + i;
		_vm->bankMan()->unpack(i, frameNum, JOURNAL_BANK);
		BobFrame *bf = _vm->bankMan()->fetchFrame(frameNum);
		bf->xhotspot = 0;
		bf->yhotspot = 0;
		if (i == FRAME_INFO_BOX) { // adjust info box hot spot to put it on top always
			bf->yhotspot = 200;
		}
	}
	_vm->bankMan()->close(JOURNAL_BANK);
}


void Journal::restore() {
	_vm->display()->fullscreen(false);
	_vm->display()->forceFullRefresh();

	_vm->logic()->joePos(_prevJoeX, _prevJoeY);
	_vm->logic()->joeCutFacing(_vm->logic()->joeFacing());

	_vm->logic()->oldRoom(_vm->logic()->currentRoom());
	_vm->logic()->displayRoom(_vm->logic()->currentRoom(), RDM_FADE_JOE, 0, 0, false);
}


void Journal::redraw() {
	drawNormalPanel();
	drawConfigPanel();
	drawSaveDescriptions();
	drawSaveSlot();
}


void Journal::update() {
	_vm->graphics()->update(JOURNAL_ROOM);
	if (_edit.enable) {
		int16 x = 136 + _edit.posCursor;
		int16 y = 9 + _currentSaveSlot * 13 + 8;
		_vm->display()->drawBox(x, y, x + 6, y, INK_JOURNAL);
	}
	_vm->display()->forceFullRefresh();
	_vm->display()->update();
}


void Journal::showBob(int bobNum, int16 x, int16 y, int frameNum) {
	BobSlot *bob = _vm->graphics()->bob(bobNum);
	bob->curPos(x, y);
	bob->frameNum = JOURNAL_FRAMES + frameNum;
}


void Journal::hideBob(int bobNum) {
	_vm->graphics()->bob(bobNum)->active = false;
}


void Journal::drawSaveDescriptions() {
	int i;
	for (i = 0; i < SAVE_PER_PAGE; ++i) {
		int n = _currentSavePage * 10 + i;
		char nb[4];
		sprintf(nb, "%d", n + 1);
		int y = 9 + i * 13;
		_vm->display()->setText(136, y, _saveDescriptions[n], false);
		_vm->display()->setText(109, y + 1, nb, false);
	}
	// hightlight current page
	showBob(BOB_SAVE_PAGE, 300, 3 + _currentSavePage * 15, 6 + _currentSavePage);
}


void Journal::drawSaveSlot() {
	showBob(BOB_SAVE_DESC, 130, 6 + _currentSaveSlot * 13, 17);
}


void Journal::enterYesNoMode(int16 zoneNum, int titleNum) {
	_mode = M_YES_NO;
	_prevZoneNum = zoneNum;
	drawYesNoPanel(titleNum);
}


void Journal::exitYesNoMode() {
	_mode = M_NORMAL;
	if (_prevZoneNum == ZN_MAKE_ENTRY) {
		_edit.enable = false;
	}
	redraw();
}


void Journal::handleNormalMode(int16 zoneNum, int x) {
	if (zoneNum == ZN_REVIEW_ENTRY) {
		enterYesNoMode(zoneNum, TXT_REVIEW_ENTRY);
	} else if (zoneNum == ZN_MAKE_ENTRY) {
		initEditBuffer(_saveDescriptions[_currentSavePage * 10 + _currentSaveSlot]);
		enterYesNoMode(zoneNum, TXT_MAKE_ENTRY);
	} else if (zoneNum == ZN_CLOSE) {
		_quit = true;
	} else if (zoneNum == ZN_GIVEUP) {
		enterYesNoMode(zoneNum, TXT_GIVE_UP);
	}
	if (zoneNum == ZN_TEXT_SPEED) {
		_vm->talkSpeed((x - 136) * 100 / 130);
		drawConfigPanel();
	} else if (zoneNum == ZN_SFX_TOGGLE) {
		_vm->sound()->toggleSfx();
		drawConfigPanel();
	} else if (zoneNum == ZN_MUSIC_VOLUME) {
		// int val = (x - 136) * 100 / 130;
		// XXX alter_current_volume();
		drawConfigPanel();
	} else if (zoneNum >= ZN_DESC_FIRST && zoneNum <= ZN_DESC_LAST) {
		_currentSaveSlot = zoneNum - ZN_DESC_FIRST;
		drawSaveSlot();
	} else if (zoneNum >= ZN_PAGE_FIRST && zoneNum <= ZN_PAGE_LAST) {
		_currentSavePage = zoneNum - ZN_PAGE_FIRST;
		drawSaveDescriptions();
	} else if (zoneNum == ZN_INFO_BOX) {
		_mode = M_INFO_BOX;
		showInformationBox();
	} else if (zoneNum == ZN_MUSIC_TOGGLE) {
		_vm->sound()->toggleMusic();
		if (_vm->sound()->musicOn()) {
			// XXX playsong(lastoverride);
		} else {
			// XXX playsong(-1);
		}
		drawConfigPanel();
	} else if (zoneNum == ZN_VOICE_TOGGLE) {
		_vm->sound()->toggleSpeech();
		drawConfigPanel();
	} else if (zoneNum == ZN_TEXT_TOGGLE) {
		_vm->subtitles(!_vm->subtitles());
		drawConfigPanel();
	}
}


void Journal::handleInfoBoxMode(int16 zoneNum) {
	hideInformationBox();
	_mode = M_NORMAL;
}


void Journal::handleYesNoMode(int16 zoneNum) {
	if (zoneNum == ZN_YES) {
		_mode = M_NORMAL;
		int currentSlot = _currentSavePage * 10 + _currentSaveSlot;
		switch (_prevZoneNum) {
		case ZN_REVIEW_ENTRY:
			if (_saveDescriptions[currentSlot][0]) {
				_vm->graphics()->clearBobs();
				_vm->display()->palFadeOut(0, 223, JOURNAL_ROOM);
				_vm->loadGameState(currentSlot);
				_vm->display()->clearTexts(0, GAME_SCREEN_HEIGHT - 1);
				// XXX panelflag=1;
				// XXX walkgameload=1;
				_quit = true;
				_quitCleanly = false;
			} else {
				exitYesNoMode();
			}
			break;
		case ZN_MAKE_ENTRY:
			if (_edit.text[0]) {
				_vm->saveGameState(currentSlot, _edit.text);
				_quit = true;
			} else {
				exitYesNoMode();
			}
			break;
		case ZN_GIVEUP:
			OSystem::instance()->quit();
			break;
		}
	} else if (zoneNum == ZN_NO) {
		exitYesNoMode();
	}
}


void Journal::handleMouseWheel(int inc) {
	if (_mode == M_NORMAL) {
		int curSave = _currentSavePage * SAVE_PER_PAGE + _currentSaveSlot + inc;
		if (curSave >= 0 && curSave < SAVE_PER_PAGE * 10) {
			_currentSavePage = curSave / SAVE_PER_PAGE;
			_currentSaveSlot = curSave % SAVE_PER_PAGE;
			drawSaveDescriptions();
			drawSaveSlot();
			update();
		}
	}
}


void Journal::handleMouseDown(int x, int y) {
	int16 zone = _vm->grid()->findZoneForPos(GS_ROOM, x, y);
	if (_mode == M_INFO_BOX) {
		handleInfoBoxMode(_mode);
	} else if (_mode == M_YES_NO) {
		handleYesNoMode(zone);
	} else if (_mode == M_NORMAL) {
		handleNormalMode(zone, x);
	}
	update();
}


void Journal::handleKeyDown(uint16 ascii, int keycode) {
	if (_mode == M_YES_NO) {
		if (keycode == 27) { // escape
			handleYesNoMode(ZN_NO);
		} else if (_edit.enable) {
			updateEditBuffer(ascii, keycode);
		}
	} else if (_mode == M_NORMAL) {
		handleNormalMode(ZN_CLOSE, 0);
	}
}


void Journal::clearPanelTexts() {
	int i;
	for (i = 0; i < _panelTextCount; ++i) {
		_vm->display()->clearTexts(_panelTextY[i], _panelTextY[i]);
	}
}


void Journal::drawPanelText(int y, const char *text) {
	char s[80];
	strcpy(s, text);
	char *p = strchr(s, ' ');
	if (p == NULL) {
		int x = (128 - _vm->display()->textWidth(s)) / 2;
		_vm->display()->setText(x, y, s, false);
		_panelTextY[_panelTextCount++] = y;
	} else {
		*p++ = '\0';
		drawPanelText(y - 5, s);
		drawPanelText(y + 5, p);
	}
}


void Journal::drawCheckBox(bool active, int bobNum, int16 x, int16 y, int frameNum) {
	if (active) {
		showBob(bobNum, x, y, frameNum);
	} else {
		hideBob(bobNum);
	}
}


void Journal::drawSlideBar(int value, int hi, int lo, int bobNum, int16 x, int16 y, int frameNum) {
	showBob(bobNum, x + value * hi / lo, y, frameNum);
}


void Journal::drawPanel(const int *frames, const int *titles, int n) { 
	clearPanelTexts();
	_panelTextCount = 0;
	int bobNum = 1;
	int y = 8;
	while (n--) {
		showBob(bobNum++, 32, y, *frames++);
		drawPanelText(y + 12, _vm->logic()->joeResponse(*titles++));
		y += 48;
	}
}


void Journal::drawNormalPanel() {
	int frames[] = { FRAME_BLUE_1, FRAME_BLUE_2, FRAME_BLUE_1, FRAME_ORANGE };
	int titles[] = { TXT_REVIEW_ENTRY, TXT_MAKE_ENTRY, TXT_CLOSE, TXT_GIVE_UP };
	drawPanel(frames, titles, 4);
}


void Journal::drawYesNoPanel(int titleNum) {
	int frames[] = { FRAME_GREY, FRAME_BLUE_1, FRAME_BLUE_2 };
	int titles[] = { titleNum, TXT_YES, TXT_NO };
	drawPanel(frames, titles, 3);

	hideBob(BOB_LEFT_RECT_4);
	hideBob(BOB_TALK_SPEED);
	hideBob(BOB_SFX_TOGGLE);
	hideBob(BOB_MUSIC_VOLUME);
	hideBob(BOB_SPEECH_TOGGLE);
	hideBob(BOB_TEXT_TOGGLE);
	hideBob(BOB_MUSIC_TOGGLE);
}


void Journal::drawConfigPanel() {
	_vm->checkOptionSettings();

	drawSlideBar(_vm->talkSpeed(), 130, 100, BOB_TALK_SPEED, 136 - 4, 164, FRAME_BLUE_PIN);
	// XXX music_volume
	drawSlideBar(100, 130, 100, BOB_MUSIC_VOLUME, 136 - 4, 177, FRAME_GREEN_PIN);

	drawCheckBox(_vm->sound()->sfxOn(), BOB_SFX_TOGGLE, 221, 155, FRAME_CHECK_BOX);
	drawCheckBox(_vm->sound()->speechOn(), BOB_SPEECH_TOGGLE, 158, 155, FRAME_CHECK_BOX);
	drawCheckBox(_vm->subtitles(), BOB_TEXT_TOGGLE, 125, 167, FRAME_CHECK_BOX);
	drawCheckBox(_vm->sound()->musicOn(), BOB_MUSIC_TOGGLE, 125, 181, FRAME_CHECK_BOX);
}


void Journal::showInformationBox() {
	_vm->display()->clearTexts(0, GAME_SCREEN_HEIGHT - 1);
	showBob(BOB_INFO_BOX, 72, 221, FRAME_INFO_BOX);

	const char *ver = _vm->resource()->JASVersion();
	switch (ver[0]) {
	case 'P':
		_vm->display()->setTextCentered(132, "PC Hard Drive", false);
		break;
	case 'C':
		_vm->display()->setTextCentered(132, "PC CD-ROM", false);
		break;
	case 'a':
		_vm->display()->setTextCentered(132, "Amiga A500/600", false);
		break;
	case 'A':
		_vm->display()->setTextCentered(132, "Amiga A1200", false);
		break;
	case 'c':
		_vm->display()->setTextCentered(132, "Amiga CD-32", false);
		break;
	}
	switch (ver[1]) {
	case 'E':
		_vm->display()->setTextCentered(144, "English", false);
		break;
	case 'G':
		_vm->display()->setTextCentered(144, "Deutsch", false);
		break;
	case 'I':
		_vm->display()->setTextCentered(144, "Italiano", false);
		break;
	case 'F' :
		_vm->display()->setTextCentered(144, "Fran\x87""ais", false);
		break;
	}
	char versionId[13];
	sprintf(versionId, "Version %c.%c%c", ver[2], ver[3], ver[4]);
	_vm->display()->setTextCentered(156, versionId, false);
}


void Journal::hideInformationBox() {
	_vm->display()->clearTexts(0, GAME_SCREEN_HEIGHT - 1);
	hideBob(BOB_INFO_BOX);
	redraw();
}


void Journal::initEditBuffer(const char *desc) {
	_edit.enable = true;
	_edit.posCursor = _vm->display()->textWidth(desc);
	_edit.textCharsCount = strlen(desc);
	memset(_edit.text, 0, sizeof(_edit.text));
	strcpy(_edit.text, desc);
}


void Journal::updateEditBuffer(uint16 ascii, int keycode) {
	bool dirty = false;
	switch (keycode) {
	case 8: // backspace
		if (_edit.textCharsCount > 0) {
			--_edit.textCharsCount;
			_edit.text[_edit.textCharsCount] = '\0';
			dirty = true;
		}
		break;
	case '\n':
	case '\r':
		handleYesNoMode(ZN_MAKE_ENTRY);
		break;
	default:
		if (isprint((char)ascii) && 
			_edit.textCharsCount < (sizeof(_edit.text) - 1) && 
			_vm->display()->textWidth(_edit.text) < 146) {
			_edit.text[_edit.textCharsCount] = (char)ascii;
			++_edit.textCharsCount;
			dirty = true;
		}
		break;
	}
	if (dirty) {
		_vm->display()->setText(136, 9 + _currentSaveSlot * 13, _edit.text, false);
		_edit.posCursor = _vm->display()->textWidth(_edit.text);
		update();
	}
}

} // End of namespace Queen
