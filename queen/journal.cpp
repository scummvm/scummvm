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

#include "stdafx.h"
#include "queen/journal.h"
#include "queen/display.h"
#include "queen/graphics.h"
#include "queen/logic.h"
#include "queen/sound.h"

namespace Queen {

// TODO
// - misc
//   * better Journal integration in Logic
//   * get rid of g_engine global
// - save/load code related
//   * new format (SCVM [ver]32 [flags]32) all BE
//   * move code from Resource to Journal
//     - Logic::prepareSaveData(uint8 *dstBuf);
//     - Logic::parseSaveData(const uint8 *srcBuf);
//     - Logic::readSaveDataDescription(char *desc);
//     - tmpbamflag -> carbam, final fight


Journal::Journal(Logic *l, Graphics *g, Display *d, Sound *s, GameConfig *c)
	: _logic(l), _graphics(g), _display(d), _sound(s), _cfg(c) {
	_savePath = g_engine->getSavePath();
	_currentSavePage = 0;
	_currentSaveSlot = 0;
}


void Journal::use() {

	BobSlot *joe = _graphics->bob(0);
	_prevJoeX = joe->x;
	_prevJoeY = joe->y;
	debug(0, "Journal::prepare() - Joe.pos = (%d,%d)", _prevJoeX, _prevJoeY);

	_edit.enable = false;
	_mode = M_NORMAL;

	findSaveDescriptions();

	_panelTextCount = 0;
	_display->palFadeOut(0, 255, JOURNAL_ROOM);
	prepare();
	redraw();
	update();
	_display->palFadeIn(0, 255, JOURNAL_ROOM, false, 0, 0);

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
			case OSystem::EVENT_QUIT:
				system->quit();
				break;
			default:
				break;
			}
		}
		g_system->delay_msecs(20);
	}

	_graphics->textClear(0, GAME_SCREEN_HEIGHT - 1);
	_graphics->cameraBob(0);
	if (_quitCleanly) {
		restore();
	}
}


void Journal::prepare() {

	_display->horizontalScroll(0);
	_display->fullscreen(true);

	_graphics->cameraBob(-1);
	_graphics->bobClearAll();
	_graphics->textClear(0, GAME_SCREEN_HEIGHT - 1);
	_graphics->frameEraseAll(false);

	int i;
	_logic->zoneClearAll(ZONE_ROOM);
	for (i = 0; i < 4; ++i) { // left panel
		_logic->zoneSet(ZONE_ROOM, i + 1, 32, 8 + i * 48, 96, 40 + i * 48);
	}
	_logic->zoneSet(ZONE_ROOM, ZN_TEXT_SPEED, 136, 169, 265, 176);
    _logic->zoneSet(ZONE_ROOM, ZN_SFX_TOGGLE, 221 - 24, 155, 231, 164);
    _logic->zoneSet(ZONE_ROOM, ZN_MUSIC_VOLUME, 136, 182, 265, 189);
	for (i = 0; i < 10; ++i) { // right panel
        _logic->zoneSet(ZONE_ROOM, ZN_DESC_FIRST + i, 131, 7 + i * 13, 290, 18 + i * 13);
        _logic->zoneSet(ZONE_ROOM, ZN_PAGE_FIRST + i, 300, 4 + i * 15, 319, 17 + i * 15);
	}
    _logic->zoneSet(ZONE_ROOM, ZN_INFO_BOX, 273, 146, 295, 189);
    _logic->zoneSet(ZONE_ROOM, ZN_MUSIC_TOGGLE, 125 - 16, 181, 135, 190);
    _logic->zoneSet(ZONE_ROOM, ZN_VOICE_TOGGLE, 158 - 24, 155, 168, 164);
    _logic->zoneSet(ZONE_ROOM, ZN_TEXT_TOGGLE, 125 - 16, 168, 135, 177);

	_graphics->loadBackdrop("journal.PCX", JOURNAL_ROOM);
	_graphics->bankLoad("journal.BBK", JOURNAL_BANK);
	for (i = 1; i <= 20; ++i) {
		int frameNum = JOURNAL_FRAMES + i;
		_graphics->bankUnpack(i, frameNum, JOURNAL_BANK);
		BobFrame *bf = _graphics->frame(frameNum);
		bf->xhotspot = 0;
		bf->yhotspot = 0;
		if (i == FRAME_INFO_BOX) { // adjust info box hot spot to put it on top always
			bf->yhotspot = 200;
		}
	}
	_graphics->bankErase(JOURNAL_BANK);

	_graphics->textCurrentColor(INK_JOURNAL);
}


void Journal::restore() {

	_display->fullscreen(false);

	_logic->joeX(_prevJoeX);
	_logic->joeY(_prevJoeY);
	
	_logic->joeCutFacing(_logic->joeFacing());

	_logic->oldRoom(_logic->currentRoom());
	_logic->roomDisplay(_logic->roomName(_logic->currentRoom()), RDM_FADE_JOE, 0, 0, false);
}


void Journal::redraw() {

	drawNormalPanel();
	drawConfigPanel();
	drawSaveSlot();
	drawSaveDescriptions();
}


void Journal::update() {

	_graphics->update(JOURNAL_ROOM);
	if (_edit.enable) {
		int16 x = 136 + _edit.posCursor;
		int16 y = 9 + _currentSaveSlot * 13 + 8;
		_display->drawBox(x, y, x + 6, y, INK_JOURNAL);
	}
	_display->update(false, 0, 0);
}


void Journal::showBob(int bobNum, int16 x, int16 y, int frameNum) {

	BobSlot *bob = _graphics->bob(bobNum);
	bob->active = true;
	bob->x = x;
	bob->y = y;
	bob->frameNum = JOURNAL_FRAMES + frameNum;
}


void Journal::hideBob(int bobNum) {

	_graphics->bob(bobNum)->active = false;
}


void Journal::findSaveDescriptions() {

	SaveFileManager *mgr = OSystem::instance()->get_savefile_manager();
	char filename[256];
	makeSavegameName(filename);
	bool marks[MAX_SAVE_DESC_NUM];
	mgr->list_savefiles(filename, _savePath, marks, MAX_SAVE_DESC_NUM);

	memset(_saveDescriptions, 0, sizeof(_saveDescriptions));
	int i;
	for (i = 0; i < MAX_SAVE_DESC_NUM; ++i) {
		if (marks[i]) {
			makeSavegameName(filename, i);
			SaveFile *f = mgr->open_savefile(filename, _savePath, false);
			if (f) {
				f->read(_saveDescriptions[i], MAX_SAVE_DESC_LEN);
				debug(0, "Journal::findSaveDescriptions() - %d %s desc=%s", i, filename, _saveDescriptions[i]);
				delete f;
			}
		}
	}
	delete mgr;
}


void Journal::drawSaveDescriptions() {

	int i;
	for (i = 0; i < SAVE_PER_PAGE; ++i) {
		int n = _currentSavePage * 10 + i;
		char nb[4];
		sprintf(nb, "%d", n + 1);
		int y = 9 + i * 13;
		_graphics->textSet(136, y, _saveDescriptions[n], false);
		_graphics->textSet(109, y + 1, nb, false);
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
	}
	else if (zoneNum == ZN_MAKE_ENTRY) {
		initEditBuffer(_saveDescriptions[_currentSavePage * 10 + _currentSaveSlot]);
		enterYesNoMode(zoneNum, TXT_MAKE_ENTRY);
	}
	else if (zoneNum == ZN_CLOSE) {
		_quit = true;
	}
	else if (zoneNum == ZN_GIVEUP) {
		enterYesNoMode(zoneNum, TXT_GIVE_UP);
	}
	if (zoneNum == ZN_TEXT_SPEED) {
		_cfg->talkSpeed = (x - 136) / 4;
		if (_cfg->talkSpeed < 1) {
			_cfg->talkSpeed = 1;
		}
		drawConfigPanel();
	}
	else if (zoneNum == ZN_SFX_TOGGLE) {
		_sound->toggleSfx();
		drawConfigPanel();
	}
	else if (zoneNum == ZN_MUSIC_VOLUME) {
		_cfg->musicVolume = (x - 136) * 100 / 130;
		if (_cfg->musicVolume < 4) {
			_cfg->musicVolume = 4;
		}
		else if (_cfg->musicVolume > 95) {
			_cfg->musicVolume = 100;
		}
		// XXX alter_current_volume();
		drawConfigPanel();
	}
	else if (zoneNum >= ZN_DESC_FIRST && zoneNum <= ZN_DESC_LAST) {
		_currentSaveSlot = zoneNum - ZN_DESC_FIRST;
		drawSaveSlot();
	}
	else if (zoneNum >= ZN_PAGE_FIRST && zoneNum <= ZN_PAGE_LAST) {
		_currentSavePage = zoneNum - ZN_PAGE_FIRST;
		drawSaveDescriptions();
	}
	else if (zoneNum == ZN_INFO_BOX) {
		_mode = M_INFO_BOX;
		showInformationBox();
	}
	else if (zoneNum == ZN_MUSIC_TOGGLE) {
		_sound->toggleMusic();
		if (_sound->musicOn()) {
			// XXX playsong(lastoverride);
		}
		else {
			// XXX playsong(-1);
		}
		drawConfigPanel();
	}
	else if (zoneNum == ZN_VOICE_TOGGLE) {
		_sound->toggleSpeech();
		if (!_sound->speechOn()) {
			// ensure text is always on when voice is off
			_cfg->textToggle = true;
		}
		drawConfigPanel();
	}
	else if (zoneNum == ZN_TEXT_TOGGLE) {
		// only allow change on CD-ROM version
		if (_logic->resource()->JASVersion()[0] == 'C') {
			_cfg->textToggle = !_cfg->textToggle;
			if (!_sound->speechOn()) {
				// ensure text is always on when voice is off
				_cfg->textToggle = true;
			}
			drawConfigPanel();
		}
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
				_display->palFadeOut(0, 223, JOURNAL_ROOM);
				loadState(currentSlot);
				_graphics->textClear(0, GAME_SCREEN_HEIGHT - 1);
				// XXX panelflag=1;
				// XXX walkgameload=1;
				_quit = true;
				_quitCleanly = false;
			}
			else {
				exitYesNoMode();
			}
			break;
		case ZN_MAKE_ENTRY:
			if (_edit.text[0]) {
				saveState(currentSlot, _edit.text);
				_quit = true;
			}
			else {
				exitYesNoMode();
			}
			break;
		case ZN_GIVEUP:
			OSystem::instance()->quit();
			break;
		}
	}
	else if (zoneNum == ZN_NO) {
		exitYesNoMode();
	}
}


void Journal::handleMouseDown(int x, int y) {

	int16 zone = _logic->zoneIn(ZONE_ROOM, x, y);
	if (_mode == M_INFO_BOX) {
		handleInfoBoxMode(_mode);
	}
	else if (_mode == M_YES_NO) {
		handleYesNoMode(zone);
	}
	else if (_mode == M_NORMAL) {
		handleNormalMode(zone, x);
	}
	update();
}


void Journal::handleKeyDown(uint16 ascii, int keycode) {

	if (_mode == M_YES_NO) {
		if (keycode == 27) { // escape
			handleYesNoMode(ZN_NO);
		}
		else if (_edit.enable) {
			updateEditBuffer(ascii, keycode);
		}
	}
	else if (_mode == M_NORMAL) {
		handleNormalMode(ZN_CLOSE, 0);
	}
}


void Journal::clearPanelTexts() {

	int i;
	for (i = 0; i < _panelTextCount; ++i) {
		_graphics->textClear(_panelTextY[i], _panelTextY[i]);
	}
}


void Journal::drawPanelText(int y, const char *text) {

	char s[80];
	strcpy(s, text);
	char *p = strchr(s, ' ');
	if (p == NULL) {
		int x = (128 - _graphics->textWidth(s)) / 2;
		_graphics->textSet(x, y, s, false);
		_panelTextY[_panelTextCount++] = y;
	}
	else {
		*p++ = '\0';
		drawPanelText(y - 5, s);
		drawPanelText(y + 5, p);
	}
}


void Journal::drawCheckBox(bool active, int bobNum, int16 x, int16 y, int frameNum) {

	if (active) {
		showBob(bobNum, x, y, frameNum);
	}
	else {
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
		drawPanelText(y + 12, _logic->joeResponse(*titles++));
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

	drawSlideBar(_cfg->talkSpeed,     4,   1, BOB_TALK_SPEED,   136 - 4, 164, FRAME_BLUE_PIN);
	drawSlideBar(_cfg->musicVolume, 130, 100, BOB_MUSIC_VOLUME, 136 - 4, 177, FRAME_GREEN_PIN);

	drawCheckBox(_sound->sfxOn(),    BOB_SFX_TOGGLE,    221, 155, FRAME_CHECK_BOX);
	drawCheckBox(_sound->speechOn(), BOB_SPEECH_TOGGLE, 158, 155, FRAME_CHECK_BOX);
	drawCheckBox(_cfg->textToggle,   BOB_TEXT_TOGGLE,   125, 167, FRAME_CHECK_BOX);
	drawCheckBox(_sound->musicOn(),  BOB_MUSIC_TOGGLE,  125, 181, FRAME_CHECK_BOX);
}


void Journal::showInformationBox() {

	_graphics->textClear(0, GAME_SCREEN_HEIGHT - 1);
	showBob(BOB_INFO_BOX, 72, 221, FRAME_INFO_BOX);

	const char *ver = _logic->resource()->JASVersion();
	switch (ver[0]) {
	case 'P':
		_graphics->textSetCentered(132, "PC Hard Drive", false);
		break;
	case 'C':
		_graphics->textSetCentered(132, "PC CD-ROM", false);
		break;
	case 'a':
	case 'A':
	case 'c':
		_graphics->textSetCentered(132, "Amiga", false);
		break;
	}
	switch (ver[1]) {
	case 'E':
		_graphics->textSetCentered(144, "English", false);
		break;
	case 'G':
		_graphics->textSetCentered(144, "Deutsch", false);
		break;
	case 'I':
		_graphics->textSetCentered(144, "Italiano", false);
		break;
	case 'F' :
		_graphics->textSetCentered(144, "Fran\x87""ais", false);
		break;
	}
	char versionId[12];
	sprintf(versionId, "Version %c.%c%c", ver[2], ver[3], ver[4]);
	_graphics->textSetCentered(156, versionId, false);
}


void Journal::hideInformationBox() {

	_graphics->textClear(0, GAME_SCREEN_HEIGHT - 1);
	hideBob(BOB_INFO_BOX);
	redraw();
}


void Journal::initEditBuffer(const char *desc) {

	_edit.enable = true;
	_edit.posCursor = _graphics->textWidth(desc);
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
			_graphics->textWidth(_edit.text) < 146) {
			_edit.text[_edit.textCharsCount] = (char)ascii;
			++_edit.textCharsCount;
			dirty = true;
		}
		break;
	}
	if (dirty) {
		_graphics->textSet(136, 9 + _currentSaveSlot * 13, _edit.text, false);
		_edit.posCursor = _graphics->textWidth(_edit.text);
		update();
	}
}


void Journal::makeSavegameName(char *buf, int slot) {

	if (slot >= 0) {
		sprintf(buf, "queensav.%03d", slot); // "queen.s%02d"
	}
	else {
		sprintf(buf, "queensav."); // "queen.s"
	}
}


void Journal::saveState(int slot, const char *desc) {

	warning("Journal::saveState(%d, %s)", slot, desc);
	_logic->gameSave(slot, desc);
}


void Journal::loadState(int slot) {

	warning("Journal::loadState(%d)", slot);
	_logic->gameLoad(slot);
	// maybe allow loading others formats too :
	// - loadState_DOS();   size = 24613, LE
	// - loadState_Amiga(); size = 24607, BE
	// - loadState_Joost(); size = 24622, BE
}


} // End of namespace Queen
