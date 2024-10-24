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

#include "common/file.h"
#include "common/util.h"
#include "common/savefile.h"
#include "common/events.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/memstream.h"

#include "backends/keymapper/keymapper.h"

#include "graphics/thumbnail.h"
#include "gui/message.h"

#include "sword1/control.h"
#include "sword1/logic.h"
#include "sword1/mouse.h"
#include "sword1/objectman.h"
#include "sword1/resman.h"
#include "sword1/sound.h"
#include "sword1/sword1.h"
#include "sword1/sworddefs.h"
#include "sword1/swordres.h"
#include "sword1/screen.h"

namespace Sword1 {

enum LangStrings {
	STR_PAUSED = 0,
	STR_INSERT_CD_A,
	STR_INSERT_CD_B,
	STR_INCORRECT_CD,
	STR_SAVE,
	STR_RESTORE,
	STR_RESTART,
	STR_START,
	STR_QUIT,
	STR_SPEED,
	STR_VOLUME,
	STR_TEXT,
	STR_DONE,
	STR_OK,
	STR_CANCEL,
	STR_MUSIC,
	STR_SPEECH,
	STR_FX,
	STR_THE_END,
	STR_DRIVE_FULL
};

const Button Control::panelButtons[8] = {
	{ 145, 188, 165, 214 },
	{ 145, 224, 165, 250 },
	{ 145, 260, 165, 286 },
	{ 145, 296, 165, 322 },
	{ 475, 188, 495, 214 },
	{ 475, 224, 495, 250 },
	{ 475, 260, 495, 286 },
	{ 475, 332, 495, 358 }
};

const Button Control::deathButtons[8] = {
	{   0,   0,   0,   0 },
	{ 250, 224, 270, 250 },
	{ 250, 260, 270, 286 },
	{ 250, 296, 270, 322 },

	{   0,   0,   0,   0 },
	{   0,   0,   0,   0 },
	{   0,   0,   0,   0 },
	{   0,   0,   0,   0 }
};

const Button Control::confirmButtons[2] = {
	{ 260, 192, 284, 216 },
	{ 260, 256, 284, 280 }
};

const Button Control::speedButtons[3] = {
	{ 240, 136, 264, 160 },
	{ 240, 200, 264, 224 },
	{ 380, 256, 404, 280 }
};

const Button Control::saveButtons[SAVEBUTTONS] = {
	{ 114,  32, 490,  67 },
	{ 114,  68, 490, 103 },
	{ 114, 104, 490, 139 },
	{ 114, 140, 490, 175 },
	{ 114, 176, 490, 211 },
	{ 114, 212, 490, 247 },
	{ 114, 248, 490, 283 },
	{ 114, 284, 490, 319 },

	{ 516,  25, 532,  40 },
	{ 516,  45, 532,  60 },
	{ 516, 289, 532, 305 },
	{ 516, 310, 532, 325 },

	{ 125, 338, 149, 366 },
	{ 462, 338, 485, 366 }
};

const Button Control::restoreButtons[SAVEBUTTONS] = {
	{ 110, 100, SCREEN_FULL_DEPTH, 140 },

	{ 516,  45, 532,  60 },
	{ 516,  25, 532,  40 },
	{ 516, 289, 532, 305 },
	{ 516, 310, 532, 325 },

	{ 125, 338, 149, 366 },
	{ 462, 338, 485, 366 },

	{   0,   0,   0,   0 },
	{   0,   0,   0,   0 },
	{   0,   0,   0,   0 },
	{   0,   0,   0,   0 },
	{   0,   0,   0,   0 },
	{   0,   0,   0,   0 },
	{   0,   0,   0,   0 }
};

const Button Control::volumeButtons[25] = {

	{ VD1X + 30,      VDY, VD1X + 60, VDY + 30 },
	{ VD1X + 60, VDY + 10, VD1X + 80, VDY + 30 },
	{ VD1X + 60, VDY + 30, VD1X + 90, VDY + 60 },
	{ VD1X + 60, VDY + 60, VD1X + 80, VDY + 80 },
	{ VD1X + 30, VDY + 60, VD1X + 60, VDY + 90 },
	{ VD1X + 10, VDY + 60, VD1X + 30, VDY + 80 },
	{ VD1X +  0, VDY + 30, VD1X + 30, VDY + 60 },
	{ VD1X + 10, VDY + 10, VD1X + 30, VDY + 30 },

	{ VD2X + 30,      VDY, VD2X + 60, VDY + 30 },
	{ VD2X + 60, VDY + 10, VD2X + 80, VDY + 30 },
	{ VD2X + 60, VDY + 30, VD2X + 90, VDY + 60 },
	{ VD2X + 60, VDY + 60, VD2X + 80, VDY + 80 },
	{ VD2X + 30, VDY + 60, VD2X + 60, VDY + 90 },
	{ VD2X + 10, VDY + 60, VD2X + 30, VDY + 80 },
	{ VD2X +  0, VDY + 30, VD2X + 30, VDY + 60 },
	{ VD2X + 10, VDY + 10, VD2X + 30, VDY + 30 },

	{ VD3X + 30,      VDY, VD3X + 60, VDY + 30 },
	{ VD3X + 60, VDY + 10, VD3X + 80, VDY + 30 },
	{ VD3X + 60, VDY + 30, VD3X + 90, VDY + 60 },
	{ VD3X + 60, VDY + 60, VD3X + 80, VDY + 80 },
	{ VD3X + 30, VDY + 60, VD3X + 60, VDY + 90 },
	{ VD3X + 10, VDY + 60, VD3X + 30, VDY + 80 },
	{ VD3X +  0, VDY + 30, VD3X + 30, VDY + 60 },
	{ VD3X + 10, VDY + 10, VD3X + 30, VDY + 30 },

	{       472,      340,       496,      364 }

};

Control::Control(SwordEngine *vm, Common::SaveFileManager *saveFileMan, ResMan *pResMan, ObjectMan *pObjMan, OSystem *system, Mouse *pMouse, Sound *pSound, Screen *pScreen, Logic *pLogic) {
	_vm = vm;
	_saveFileMan = saveFileMan;
	_resMan = pResMan;
	_objMan = pObjMan;
	_system = system;
	_mouse = pMouse;
	_sound = pSound;
	_screen = pScreen;
	_logic = pLogic;

	if (gameVersionIsAkella()) {
		_lStrings = _akellaLanguageStrings;
	} else if (gameVersionIsMediaHouse()) {
		_lStrings = _mediaHouseLanguageStrings;
	} else if (vm->_systemVars.realLanguage == Common::Language::PL_POL) {
		_lStrings = _polishTranslationLanguageStrings;
	} else {
		_lStrings = loadCustomStrings("strings.txt") ? _customStrings : _languageStrings + SwordEngine::_systemVars.language * 20;
	}

	_selectedButton = 255;
	_panelShown = false;
	_tempThumbnail = 0;

	for (int i = 0; i < ARRAYSIZE(_slabs); i++) {
		_slabs[i] = nullptr;
	}
}

bool Control::savegamesExist() {
	Common::String pattern = "sword1.???";
	Common::StringArray saveNames = _saveFileMan->listSavefiles(pattern);
	return saveNames.size() > 0;
}

bool Control::isPanelShown() {
	return _panelShown;
}

void Control::getPlayerOptions() {
	debug(1, "Control::getPlayerOptions(): Entering Control Panel");
	_panelShown = true;

	_screenBuf = (uint8 *)malloc(SCREEN_WIDTH * SCREEN_FULL_DEPTH);
	// Make a thumbnail of the screen before displaying the menu
	// in case we want to save the game from the menu.
	_tempThumbnail = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
	Graphics::saveThumbnail(*_tempThumbnail);

	// Reset because it was previously used when 'F5' or 'ESC' were pressed
	SwordEngine::_systemVars.saveGameFlag = SGF_DONE;

	_logic->fnWipeHands(nullptr, 0, 0, 0, 0, 0, 0, 0);
	_logic->fnEndMenu(nullptr, 0, 0, 0, 0, 0, 0, 0);

	int safeCurrentMusic = Logic::_scriptVars[CURRENT_MUSIC];

	// Play music only if we actually have a music file to run..
	if (!SwordEngine::_systemVars.runningFromCd || SwordEngine::_systemVars.currentCD)
		_logic->fnPlayMusic(nullptr, 0, 61, LOOPED, 0, 0, 0, 0); // Control panel music ("2m29")

	Logic::_scriptVars[CURRENT_MUSIC] = safeCurrentMusic;

	_vm->startFadePaletteDown(1);
	_vm->waitForFade();
	_sound->clearAllFx();
	_keyPressed.reset();
	_customType = kActionNone;

	while (SwordEngine::_systemVars.snrStatus != SNR_BLANK && !Engine::shouldQuit()) {
		delay(DEFAULT_FRAME_TIME / 2);

		_sound->setCrossFadeIncrement();

		_mouse->animate();
		_sound->updateMusicStreaming();
		saveRestoreScreen();
	}

	_keyPressed.reset();
	_customType = kActionNone;

	saveRestoreScreen();

	debug(1, "Control::getPlayerOptions(): Returning from Control Panel: saveGameFlag = %d",
		SwordEngine::_systemVars.saveGameFlag);

	// Stop music now, if we're not returning to any in-game music...
	bool wontReloadMusic =
		SwordEngine::_systemVars.saveGameFlag == SGF_RESTORE ||
		SwordEngine::_systemVars.saveGameFlag == SGF_RESTART ||
		SwordEngine::_systemVars.saveGameFlag == SGF_QUIT;

	if (wontReloadMusic || (Logic::_scriptVars[CURRENT_MUSIC] == 0)) {
		_logic->fnStopMusic(nullptr, 0, 0, 0, 0, 0, 0, 0);
	}

	_vm->startFadePaletteDown(1);
	_vm->waitForFade();

	_logic->fnNormalMouse(nullptr, 0, 0, 0, 0, 0, 0, 0);
	Logic::_scriptVars[NEW_PALETTE] = 1;

	if (SwordEngine::_systemVars.saveGameFlag == SGF_SAVE) {
		saveGame();
	} else if (SwordEngine::_systemVars.saveGameFlag == SGF_QUIT) {
		_sound->fadeMusicDown(1);

		Engine::quitGame();
	}

	// Reset again (because it may have been set in fnDeathScreen())
	SwordEngine::_systemVars.controlPanelMode = CP_NORMAL;

	// If DONE or SAVE was selected, try restoring audio...
	if (SwordEngine::_systemVars.saveGameFlag == SGF_DONE || SwordEngine::_systemVars.saveGameFlag == SGF_SAVE) {
		// Restore sound effects...
		for (int j = 0; j < TOTAL_FX_PER_ROOM; j++) {
			if (int32 fxNo = Sound::_roomsFixedFx[Logic::_scriptVars[SCREEN]][j]) { // search the room's fixed fx list (see 'fx_list.c')
				if (Sound::_fxList[fxNo].type == FX_LOOP)
					_logic->fnPlayFx(nullptr, 0, fxNo, 0, 0, 0, 0, 0);
			} else {
				break; // Drop out as soon as we come across a zero, rather than searching the whole list...
			}
		}
		// Restore in-game music...
		if (Logic::_scriptVars[CURRENT_MUSIC]) {
			_logic->fnPlayMusic(nullptr, 0, Logic::_scriptVars[CURRENT_MUSIC], LOOPED, 0, 0, 0, 0);
		}
	}

	_screen->clearScreen();
	free(_screenBuf);

	debug(1, "Control::getPlayerOptions(): Finished getPlayerOptions()");
	_panelShown = false;

	// Was being set to true in initialiseControlPanel(),
	// but we have to wait until here to set it to false
	_mouse->controlPanel(false);

	// Delete the temporary thumbnail
	delete _tempThumbnail;
	_tempThumbnail = nullptr;
}

void Control::askForCdMessage(uint32 needCD, bool incorrectCDPhase) {
	uint8 buf[255];
	_screenBuf = (uint8 *)malloc(SCREEN_WIDTH * SCREEN_FULL_DEPTH);
	if (!_screenBuf)
		return;

	if (!incorrectCDPhase) {
		//ScreenOpen(640 / XBLOCKSIZE, 400 / YBLOCKSIZE); // sets up global structure screenDef
		memset(_screenBuf, 0, SCREEN_WIDTH * SCREEN_DEPTH);

		Common::sprintf_s(buf, "%s%d", _lStrings[STR_INSERT_CD_A], needCD);
		renderText(buf, (640 - getTextLength(buf, true)) / 2, 190, true);

		Common::sprintf_s(buf, "%s", _lStrings[STR_INSERT_CD_B]);
		renderText(buf, (640 - getTextLength(buf, true)) / 2, 210, true);

		_system->copyRectToScreen(_screenBuf, SCREEN_WIDTH, 0, 40, SCREEN_WIDTH, SCREEN_DEPTH);
	} else {
		//if (displayInfo.backBuffer == 0) {
		memset(_screenBuf, 0, SCREEN_WIDTH * SCREEN_DEPTH);
			_system->copyRectToScreen(_screenBuf, SCREEN_WIDTH, 0, 40, SCREEN_WIDTH, SCREEN_DEPTH);
		//}

		memset(_screenBuf, 0, SCREEN_WIDTH * SCREEN_DEPTH);

		Common::sprintf_s(buf, "%s", _lStrings[STR_INCORRECT_CD]);
		renderText(buf, (640 - getTextLength(buf, true)) / 2, 160, true);

		Common::sprintf_s(buf, "%s%d", _lStrings[STR_INSERT_CD_A], needCD);
		renderText(buf, (640 - getTextLength(buf, true)) / 2, 190, true);

		Common::sprintf_s(buf, "%s", _lStrings[STR_INSERT_CD_B]);
		renderText(buf, (640 - getTextLength(buf, true)) / 2, 210, true);

		_system->copyRectToScreen(_screenBuf, SCREEN_WIDTH, 0, 40, SCREEN_WIDTH, SCREEN_DEPTH);
	}
	free(_screenBuf);
}

void Control::saveRestoreScreen() {
	int32 rv;

	if (_oldSnrStatus != SwordEngine::_systemVars.snrStatus) {
		// Tidy up after snr section
		if (_oldSnrStatus == SNR_BLANK) {
			memset(_screenBuf, 0, SCREEN_WIDTH * SCREEN_FULL_DEPTH);
			_newPal = true;
		}

		switch (_oldSnrStatus) {
		case SNR_BLANK:
			memset(_screenBuf, 0, SCREEN_WIDTH * SCREEN_FULL_DEPTH);
			break;
		case SNR_MAINPANEL:
			removeControlPanel();
			_sound->setVolumes();
			break;
		case SNR_SAVE:
			removeSave();
			break;
		case SNR_RESTORE:
			removeRestore();
			break;
		case SNR_RESTART:
		case SNR_QUIT:
			removeConfirmation();
			break;
		case SNR_SPEED:
			removeSpeed();
			break;
		case SNR_VOLUME:
			removeVolume();
			_sound->setVolumes();
			break;
		case SNR_DRIVEFULL:
			removeConfirmation();
			break;
		}

		// Initialise new snr section
		switch (SwordEngine::_systemVars.snrStatus) {
		case SNR_BLANK:
			releaseResources();
			break;
		case SNR_MAINPANEL:
			if (_oldSnrStatus == SNR_BLANK) {
				initialiseResources();
			}

			_sound->getVolumes();
			initialiseControlPanel();
			break;
		case SNR_SAVE:
			initialiseSave();
			break;
		case SNR_RESTORE:
			initialiseRestore();
			break;
		case SNR_RESTART:
			if (SwordEngine::_systemVars.controlPanelMode == CP_NEWGAME) {
				SwordEngine::_systemVars.snrStatus = SNR_BLANK;
				_system->copyRectToScreen(_screenBuf, SCREEN_WIDTH, 0, 40, SCREEN_WIDTH, SCREEN_DEPTH);
			} else {
				initialiseConfirmation(_lStrings[STR_RESTART]);
			}

			break;
		case SNR_QUIT:
			initialiseConfirmation(_lStrings[STR_QUIT]);
			break;
		case SNR_SPEED:
			initialiseSpeed();
			break;
		case SNR_VOLUME:
			_sound->getVolumes();
			initialiseVolume();
			break;
		case SNR_DRIVEFULL:
			initialiseConfirmation(_lStrings[STR_DRIVE_FULL]);
			break;
		}

		_oldSnrStatus = SwordEngine::_systemVars.snrStatus;
	}

	// Implement snr section

	switch (SwordEngine::_systemVars.snrStatus) {
	case SNR_BLANK:
		break;
	case SNR_MAINPANEL:
		implementControlPanel();
		_system->copyRectToScreen(_screenBuf, SCREEN_WIDTH, 0, 40, SCREEN_WIDTH, SCREEN_DEPTH);

		if (_newPal) {
			_newPal = false;
			_vm->startFadePaletteUp(1);
		}

		break;
	case SNR_SAVE:
		implementSave();
		_system->copyRectToScreen(_screenBuf, SCREEN_WIDTH, 0, 40, SCREEN_WIDTH, SCREEN_DEPTH);

		break;
	case SNR_RESTORE:
		implementRestore();
		_system->copyRectToScreen(_screenBuf, SCREEN_WIDTH, 0, 40, SCREEN_WIDTH, SCREEN_DEPTH);

		break;
	case SNR_RESTART:
		rv = implementConfirmation();
		if (rv) {
			if (rv == 1) {
				SwordEngine::_systemVars.saveGameFlag = SGF_RESTART;
				SwordEngine::_systemVars.snrStatus = SNR_BLANK;
			} else {
				SwordEngine::_systemVars.snrStatus = SNR_MAINPANEL;
			}
		}

		_system->copyRectToScreen(_screenBuf, SCREEN_WIDTH, 0, 40, SCREEN_WIDTH, SCREEN_DEPTH);

		break;
	case SNR_QUIT:
		rv = implementConfirmation();
		if (rv) {
			if (rv == 1) {
				SwordEngine::_systemVars.saveGameFlag = SGF_QUIT;
				SwordEngine::_systemVars.snrStatus = SNR_BLANK;
			} else {
				SwordEngine::_systemVars.snrStatus = SNR_MAINPANEL;
			}
		}

		_system->copyRectToScreen(_screenBuf, SCREEN_WIDTH, 0, 40, SCREEN_WIDTH, SCREEN_DEPTH);

		break;
	case SNR_SPEED:
		implementSpeed();
		_system->copyRectToScreen(_screenBuf, SCREEN_WIDTH, 0, 40, SCREEN_WIDTH, SCREEN_DEPTH);
		break;
	case SNR_VOLUME:
		implementVolume();
		_system->copyRectToScreen(_screenBuf, SCREEN_WIDTH, 0, 40, SCREEN_WIDTH, SCREEN_DEPTH);
		break;
	case SNR_SUBTITLES:
		SwordEngine::_systemVars.snrStatus = SNR_MAINPANEL;
		break;
	case SNR_DONE:
		SwordEngine::_systemVars.snrStatus = SNR_BLANK;
		break;
	case SNR_DRIVEFULL:
		rv = implementConfirmation();

		if (rv == 1) {
			SwordEngine::_systemVars.snrStatus = SNR_SAVE;
		}

		_system->copyRectToScreen(_screenBuf, SCREEN_WIDTH, 0, 40, SCREEN_WIDTH, SCREEN_DEPTH);
		break;
	}
}

void Control::renderSlab(int32 start, int32 i) {
	uint8 *src, *dst;
	FrameHeader *f;

	if (start + 1 == _slabSelected) {
		f = (FrameHeader *)((uint8 *)_slabs[start] + _resMan->getUint32(_slabs[start]->spriteOffset[1]));
	} else {
		f = (FrameHeader *)((uint8 *)_slabs[start] + _resMan->getUint32(_slabs[start]->spriteOffset[0]));
	}

	src = (uint8 *)f + sizeof(FrameHeader);

	if (start + 1 == _slabSelected) {
		dst = _screenBuf + saveButtons[i].x1 + SCREEN_WIDTH * (saveButtons[i].y1 - 1);
	} else {
		dst = _screenBuf + saveButtons[i].x1 + SCREEN_WIDTH * saveButtons[i].y1;
	}

	if (SwordEngine::isPsx()) {
		drawPsxComponent(PSX_SLAB, src, dst, f);
	} else {
		for (int j = 0; j < _resMan->getUint16(f->height); j++) {
			memcpy(dst, src, _resMan->getUint16(f->width));
			src += _resMan->getUint16(f->width);
			dst += SCREEN_WIDTH;
		}
	}
}

void Control::renderSlabs() {
	uint8 *src, *dst;
	int32 start;
	FrameHeader *f;

	start = _firstDescription;
	while (start >= (SAVEBUTTONS - 6))
		start -= (SAVEBUTTONS - 6);

	for (int i = 0; i < SAVEBUTTONS - 6; i++) {
		if (i + 1 == _slabSelected)
			continue;

		f = (FrameHeader *)((uint8 *)_slabs[start] + _resMan->getUint32(_slabs[start]->spriteOffset[0]));
		src = (uint8 *)f + sizeof(FrameHeader);
		dst = _screenBuf + saveButtons[i].x1 + SCREEN_WIDTH * saveButtons[i].y1;

		if (SwordEngine::isPsx()) {
			drawPsxComponent(PSX_SLAB, src, dst, f);
		} else {
			for (int j = 0; j < _resMan->getUint16(f->height); j++) {
				memcpy(dst, src, _resMan->getUint16(f->width));
				src += _resMan->getUint16(f->width);
				dst += SCREEN_WIDTH;
			}
		}

		start += 1;
		if (start == SAVEBUTTONS - 6)
			start = 0;
	}

	if (_slabSelected) {
		start = _firstDescription;
		while (start >= (SAVEBUTTONS - 6))
			start -= (SAVEBUTTONS - 6);

		start += (_slabSelected - 1);
		if (start >= (SAVEBUTTONS - 6))
			start -= (SAVEBUTTONS - 6);

		f = (FrameHeader *)((uint8 *)_slabs[start] + _resMan->getUint32(_slabs[start]->spriteOffset[1]));
		src = (uint8 *)f + sizeof(FrameHeader);
		dst = _screenBuf + saveButtons[_slabSelected - 1].x1 + SCREEN_WIDTH * (saveButtons[_slabSelected - 1].y1 - 1);

		if (SwordEngine::isPsx()) {
			drawPsxComponent(PSX_SLAB, src, dst, f);
		} else {
			for (int j = 0; j < _resMan->getUint16(f->height); j++) {
				memcpy(dst, src, _resMan->getUint16(f->width));
				src += _resMan->getUint16(f->width);
				dst += SCREEN_WIDTH;
			}
		}
	}
}

void Control::renderText(const uint8 *str, int32 x, int32 y, bool useSpeechFont) {
	uint8 *src, *dst;
	int32 i, l;
	FrameHeader *f;
	Sprite *srFont;

	if (useSpeechFont) {
		if (SwordEngine::_systemVars.language == BS1_CZECH) {
			srFont = (Sprite *)_resMan->fetchRes(CZECH_GAME_FONT);
		} else {
			srFont = (Sprite *)_resMan->fetchRes(GAME_FONT);
		}
	} else {
		if (SwordEngine::_systemVars.controlPanelMode == CP_DEATHSCREEN) {
			if (SwordEngine::_systemVars.language == BS1_CZECH) {
				srFont = (Sprite *)_resMan->fetchRes(CZECH_SR_DEATHFONT);
			} else {
				srFont = (Sprite *)_resMan->fetchRes(_resMan->getDeathFontId());
			}
		} else {
			if (SwordEngine::_systemVars.language == BS1_CZECH) {
				srFont = (Sprite *)_resMan->fetchRes(CZECH_SR_FONT);
			} else {
				srFont = (Sprite *)_resMan->fetchRes(SR_FONT);
			}
		}
	}

	i = 0;
	l = 0;

	while (str[i] != 0) {
		f = (FrameHeader *)((uint8 *)srFont + _resMan->getUint32(srFont->spriteOffset[str[i] - 32]));
		dst = _screenBuf + SCREEN_WIDTH * y + x + l;
		src = (uint8 *)f + sizeof(FrameHeader);

		if (SwordEngine::isPsx()) {
			src = decompressPsxGfx(src, f);
			uint8 *initialPtr = src;

			for (int k = 0; k < _resMan->getUint16(f->height); k++) {
				for (int j = 0; j < _resMan->getUint16(f->width); j++) {
					if (src[j])
						dst[j] = src[j];
				}

				// On PSX version we need to double horizontal lines
				if (SwordEngine::isPsx()) {
					dst += SCREEN_WIDTH;
					for (int j = 0; j < _resMan->getUint16(f->width); j++)
						if (src[j])
							dst[j] = src[j];
				}

				src += _resMan->getUint16(f->width);
				dst += SCREEN_WIDTH;
			}

			free(initialPtr);
		} else {
			// Copy the data onto the sprite
			for (int k = 0; k < _resMan->getUint16(f->height); k++) {
				for (int j = 0; j < _resMan->getUint16(f->width); j++) {
					if (*src) {
						*dst = *src;
					}

					dst += 1;
					src += 1;
				}

				dst += SCREEN_WIDTH - _resMan->getUint16(f->width);
			}
		}

		l += _resMan->getUint16(f->width);

		if (!SwordEngine::_systemVars.isDemo)
			l -= useSpeechFont ? SP_OVERLAP : OVERLAP;

		i += 1;
	}
}

void Control::renderRedText(const uint8 *str, int32 x, int32 y) {
	uint8 *src, *dst;
	int32 i, l;
	FrameHeader *f;
	Sprite *srRedfont;

	if (SwordEngine::_systemVars.language == BS1_CZECH) {
		srRedfont = (Sprite *)_resMan->fetchRes(CZECH_SR_REDFONT);
	} else {
		srRedfont = (Sprite *)_resMan->fetchRes(SR_REDFONT);
	}

	i = 0;
	l = 0;
	while (str[i] != 0) {
		f = (FrameHeader *)((uint8 *)srRedfont + _resMan->getUint32(srRedfont->spriteOffset[str[i] - 32]));
		dst = _screenBuf + SCREEN_WIDTH * y + x + l;
		src = (uint8 *)f + sizeof(FrameHeader);

		if (SwordEngine::isPsx()) {
			drawPsxComponent(PSX_TEXT, src, dst, f);
		} else {
			// Copy the data onto the sprite
			for (int k = 0; k < _resMan->getUint16(f->height); k++) {
				for (int j = 0; j < _resMan->getUint16(f->width); j++) {
					if (*src) {
						*dst = *src;
					}
					dst += 1;
					src += 1;
				}
				dst += SCREEN_WIDTH - _resMan->getUint16(f->width);
			}
		}

		l += _resMan->getUint16(f->width);

		if (!SwordEngine::_systemVars.isDemo)
			l -= OVERLAP;

		i += 1;
	}
}

void Control::renderTexts() {
	char string[40];

	for (int i = 0; i < SAVEBUTTONS - 6; i++) {
		Common::sprintf_s(string, "%d", _firstDescription + i + 1);
		if (_slabSelected == i + 1) {
			renderRedText((const uint8 *)string, saveButtons[i].x1 + 12, saveButtons[i].y1 + 5);
			renderRedText((const uint8 *)_fileDescriptions[_firstDescription + i], saveButtons[i].x1 + 42, saveButtons[i].y1 + 5);
		} else {
			renderText((const uint8 *)string, saveButtons[i].x1 + 12, saveButtons[i].y1 + 5);
			renderText((const uint8 *)_fileDescriptions[_firstDescription + i], saveButtons[i].x1 + 42, saveButtons[i].y1 + 5);
		}
	}
}

int32 Control::getTextLength(const uint8 *str, bool useSpeechFont) {
	int32 i;
	int32 l;
	FrameHeader *f;
	Sprite *srFont;

	if (useSpeechFont) {
		if (SwordEngine::_systemVars.language == BS1_CZECH) {
			srFont = (Sprite *)_resMan->fetchRes(CZECH_GAME_FONT);
		} else {
			srFont = (Sprite *)_resMan->fetchRes(GAME_FONT);
		}
	} else {
		if (SwordEngine::_systemVars.controlPanelMode == CP_DEATHSCREEN) {
			if (SwordEngine::_systemVars.language == BS1_CZECH) {
				srFont = (Sprite *)_resMan->fetchRes(CZECH_SR_DEATHFONT);
			} else {
				srFont = (Sprite *)_resMan->fetchRes(_resMan->getDeathFontId());
			}
		} else {
			if (SwordEngine::_systemVars.language == BS1_CZECH) {
				srFont = (Sprite *)_resMan->fetchRes(CZECH_SR_FONT);
			} else {
				srFont = (Sprite *)_resMan->fetchRes(SR_FONT);
			}
		}
	}

	i = 0;
	l = 0;

	while (str[i] != 0) {
		f = (FrameHeader *)((uint8 *)srFont +  _resMan->getUint32(srFont->spriteOffset[str[i] - 32]));
		l += _resMan->getUint16(f->width);

		if (!SwordEngine::_systemVars.isDemo)
			l -= useSpeechFont ? SP_OVERLAP : OVERLAP;

		i += 1;
	}

	return l;
}

void Control::putButton(int32 x, int32 y, int32 index) {
	uint8 *src, *dst;
	FrameHeader *f;
	Sprite *srButton;

	srButton = (Sprite *)_resMan->fetchRes(SR_BUTTON);

	f = (FrameHeader *)((uint8 *)srButton + _resMan->getUint32(srButton->spriteOffset[index]));
	src = (uint8 *)f + sizeof(FrameHeader);
	dst = _screenBuf + SCREEN_WIDTH * y + x;

	if (SwordEngine::isPsx()) {
		drawPsxComponent(PSX_BUTTON, src, dst, f);
	} else {
		for (int i = 0; i < _resMan->getUint16(f->height); i++) {
			for (int j = 0; j < _resMan->getUint16(f->width); j++)
				if (*(src + j))
					*(dst + j) = *(src + j);

			src += _resMan->getUint16(f->width);
			dst += SCREEN_WIDTH;
		}
	}
}

void Control::putSpriteButton(Sprite *spr, int32 x, int32 y, int32 index) {
	uint8 *src, *dst;
	FrameHeader *f;

	f = (FrameHeader *)((uint8 *)spr + _resMan->getUint32(spr->spriteOffset[index]));
	src = (uint8 *)f + sizeof(FrameHeader);
	dst = _screenBuf + SCREEN_WIDTH * y + x;

	if (SwordEngine::isPsx()) {
		drawPsxComponent(PSX_BUTTON, src, dst, f);
	} else {
		for (int i = 0; i < _resMan->getUint16(f->height); i++) {
			for (int j = 0; j < _resMan->getUint16(f->width); j++)
				if (*(src + j))
					*(dst + j) = *(src + j);

			src += _resMan->getUint16(f->width);
			dst += SCREEN_WIDTH;
		}
	}
}

void Control::putTextButton(int32 index) {
	uint8 *src, *dst;
	int32 x, y;
	FrameHeader *f;
	Sprite *srTextButton;

	srTextButton = (Sprite *)_resMan->fetchRes(SR_TEXT_BUTTON);

	x = 475;
	y = 260;

	f = (FrameHeader *)((uint8 *)srTextButton + _resMan->getUint32(srTextButton->spriteOffset[index]));
	src = (uint8 *)f + sizeof(FrameHeader);
	dst = _screenBuf + SCREEN_WIDTH * y + x;
	if (SwordEngine::isPsx()) {
		drawPsxComponent(PSX_BUTTON, src, dst, f);
	} else {
		for (int i = 0; i < _resMan->getUint16(f->height); i++) {
			memcpy(dst, src, _resMan->getUint16(f->width));
			src += _resMan->getUint16(f->width);
			dst += SCREEN_WIDTH;
		}
	}
}

int32 Control::getCurrentButton(const Button b[]) {
	for (int i = 0; i < _numButtons; i++) {
		if ((_mouseCoord.x > b[i].x1) && (_mouseCoord.y - 40 > b[i].y1) &&
			(_mouseCoord.x < b[i].x2) && (_mouseCoord.y - 40 < b[i].y2)) {
			return i + 1;
		}
	}

	return 0;
}

void Control::initialiseConfirmation(const uint8 *title) {
	uint8 *src, *dst;
	FrameHeader *f;
	Sprite *srConfirm;

	srConfirm = (Sprite *)_resMan->openFetchRes(SR_CONFIRM);

	f = (FrameHeader *)((uint8 *)srConfirm + _resMan->getUint32(srConfirm->spriteOffset[0]));
	src = (uint8 *)f + sizeof(FrameHeader);
	dst = _screenBuf +
		((SCREEN_WIDTH - _resMan->getUint16(f->width)) / 2) +
		(SCREEN_WIDTH * ((SCREEN_DEPTH - _resMan->getUint16(f->height)) / 2));

	if (SwordEngine::isPsx()) {
		drawPsxComponent(PSX_CONFIRM, src, dst, f);
	} else {
		for (int i = 0; i < _resMan->getUint16(f->height); i++) {
			memcpy(dst, src, _resMan->getUint16(f->width));
			src += _resMan->getUint16(f->width);
			dst += SCREEN_WIDTH;
		}
	}

	_resMan->resClose(SR_CONFIRM);

	renderText(title, (SCREEN_WIDTH - getTextLength(title)) / 2, 120);

	if (SwordEngine::_systemVars.snrStatus == SNR_DRIVEFULL) {
		_numButtons = 1; // only top (OK) button used
	} else {
		_numButtons = 2;
	}

	for (int i = 0; i < _numButtons; i++)
		putButton(confirmButtons[i].x1, confirmButtons[i].y1, 0);

	renderText(_lStrings[STR_OK], SCREEN_WIDTH - confirmButtons[0].x1 - getTextLength(_lStrings[STR_OK]), confirmButtons[0].y1);

	if (SwordEngine::_systemVars.snrStatus != SNR_DRIVEFULL)
		renderText(_lStrings[STR_CANCEL], SCREEN_WIDTH - confirmButtons[1].x1 - getTextLength(_lStrings[STR_CANCEL]), confirmButtons[1].y1);
}

int32 Control::implementConfirmation() {
	_currentButton = getCurrentButton(&confirmButtons[0]);

	if ((_buttonPressed) && (!_currentButton)) {
		// Reset button pressed
		putButton(confirmButtons[_buttonPressed - 1].x1, confirmButtons[_buttonPressed - 1].y1, 0);
		_buttonPressed = 0;
	}

	if (_mouseState != 0) {
		if ((_mouseState & BS1L_BUTTON_DOWN) && (_currentButton)) {
			// Set button pressed
			_buttonPressed = _currentButton;
			putButton(confirmButtons[_buttonPressed - 1].x1, confirmButtons[_buttonPressed - 1].y1, 1);
		}

		if ((_mouseState & BS1L_BUTTON_UP) && (_buttonPressed)) {
			if (_buttonPressed == 1) {
				return 1;
			} else {
				return -1;
			}
		}
	}

	return 0;
}

void Control::removeConfirmation() {
	// Dummy in the original
}

void Control::renderVolumeLight(int32 i) {
	uint8 *src, *dst;
	uint8 vol[2] = { 0, 0 };
	int32 x;
	FrameHeader *f;
	Sprite *srVlight;

	switch (i) {
	case 0:
		vol[0] = _sound->_volMusic[0];
		vol[1] = _sound->_volMusic[1];
		x = 158;
		break;
	case 1:
		vol[0] = _sound->_volSpeech[0];
		vol[1] = _sound->_volSpeech[1];
		x = 291;
		break;
	case 2:
		vol[0] = _sound->_volFX[0];
		vol[1] = _sound->_volFX[1];
		x = 424;
		break;
	default:
		x = 0;
		break;
	}

	srVlight = (Sprite *)_resMan->fetchRes(SR_VLIGHT);

	// Render left light
	f = (FrameHeader *)((uint8 *)srVlight + _resMan->getUint32(srVlight->spriteOffset[vol[0]]));
	src = (uint8 *)f + sizeof(FrameHeader);
	dst = _screenBuf + x + 211 * SCREEN_WIDTH;

	if (SwordEngine::isPsx()) {
		drawPsxComponent(PSX_BUTTON, src, dst, f);
	} else {
		for (int y = 0; y < _resMan->getUint16(f->height); y++) {
			memcpy(dst, src, _resMan->getUint16(f->width));
			src += _resMan->getUint16(f->width);
			dst += SCREEN_WIDTH;
		}
	}

	// Render right light
	f = (FrameHeader *)((uint8 *)srVlight + _resMan->getUint32(srVlight->spriteOffset[vol[1]]));
	src = (uint8 *)f + sizeof(FrameHeader);
	dst = _screenBuf + x + 32 + 211 * SCREEN_WIDTH;

	if (SwordEngine::isPsx()) {
		drawPsxComponent(PSX_BUTTON, src, dst, f);
	} else {
		for (int y = 0; y < _resMan->getUint16(f->height); y++) {
			memcpy(dst, src, _resMan->getUint16(f->width));
			src += _resMan->getUint16(f->width);
			dst += SCREEN_WIDTH;
		}
	}
}

void Control::volUp(int32 i, int32 j) {
	uint32 *vol = nullptr;

	switch (i) {
	case 0:
		vol = &_sound->_volMusic[j];
		break;
	case 1:
		vol = &_sound->_volSpeech[j];
		break;
	case 2:
		vol = &_sound->_volFX[j];
		break;
	}

	if (vol && *vol < 16)
		*vol += 1;
}

void Control::volDown(int32 i, int32 j) {
	uint32 *vol = nullptr;

	switch (i) {
	case 0:
		vol = &_sound->_volMusic[j];
		break;
	case 1:
		vol = &_sound->_volSpeech[j];
		break;
	case 2:
		vol = &_sound->_volFX[j];
		break;
	}

	if (vol && *vol > 0)
		*vol -= 1;
}

void Control::renderVolumeDisc(int32 i, int32 j) {
	uint8 *src, *dst;
	int32 x = 0;
	FrameHeader *f;
	Sprite *srVnob;

	switch (i) {
	case 0:
		x = VD1X;
		break;
	case 1:
		x = VD2X;
		break;
	case 2:
		x = VD3X;
		break;
	default:
		break;
	}

	srVnob = (Sprite *)_resMan->fetchRes(SR_VKNOB);

	// Render the disc
	f = (FrameHeader *)((uint8 *)srVnob + _resMan->getUint32(srVnob->spriteOffset[j]));
	src = (uint8 *)f + sizeof(FrameHeader);
	dst = _screenBuf + x + VDY * SCREEN_WIDTH;

	if (SwordEngine::isPsx()) {
		drawPsxComponent(PSX_BUTTON, src, dst, f);
	} else {
		for (int y = 0; y < _resMan->getUint16(f->height); y++) {
			for (int z = 0; z < _resMan->getUint16(f->width); z++)
				if (*(src + z))
					*(dst + z) = *(src + z);
			src += _resMan->getUint16(f->width);
			dst += SCREEN_WIDTH;
		}
	}
}

void Control::initialiseVolume() {
	uint8 *src, *dst;
	FrameHeader *f;
	Sprite *srVolume;

	_resMan->resOpen(SR_VLIGHT);
	_resMan->resOpen(SR_VKNOB);

	srVolume = (Sprite *)_resMan->openFetchRes(SR_VOLUME);
	f = (FrameHeader *)((uint8 *)srVolume + _resMan->getUint32(srVolume->spriteOffset[0]));
	src = (uint8 *)f + sizeof(FrameHeader);
	dst = _screenBuf +
		((SCREEN_WIDTH - _resMan->getUint16(f->width)) / 2) +
		(SCREEN_WIDTH * ((SCREEN_DEPTH - _resMan->getUint16(f->height)) / 2));

	if (SwordEngine::isPsx()) {
		drawPsxComponent(PSX_PANEL, src, dst, f);
	} else {
		for (int i = 0; i < _resMan->getUint16(f->height); i++) {
			memcpy(dst, src, _resMan->getUint16(f->width));
			src += _resMan->getUint16(f->width);
			dst += SCREEN_WIDTH;
		}
	}

	_resMan->resClose(SR_VOLUME);

	renderText(_lStrings[STR_MUSIC], 149, 39);
	renderText(_lStrings[STR_SPEECH], (SCREEN_WIDTH - getTextLength(_lStrings[STR_SPEECH])) / 2, 39);
	renderText(_lStrings[STR_FX], 438, 39);
	_numButtons = 25;
	putButton(volumeButtons[24].x1, volumeButtons[24].y1, 0);

	renderText(_lStrings[STR_DONE], (volumeButtons[24].x1 - getTextLength(_lStrings[STR_DONE]) - 20), volumeButtons[24].y1);

	renderVolumeLight(0);
	renderVolumeLight(1);
	renderVolumeLight(2);

	renderVolumeDisc(0, 0);
	renderVolumeDisc(1, 0);
	renderVolumeDisc(2, 0);
}

void Control::implementVolume() {
	int32 i, j;

	_currentButton = getCurrentButton(&volumeButtons[0]);

	if (_buttonPressed) {
		if (!_currentButton) {
			// Reset button pressed
			if (_buttonPressed == 25)
				putButton(volumeButtons[_buttonPressed - 1].x1, volumeButtons[_buttonPressed - 1].y1, 0);

			_buttonPressed = 0;
		} else {
			if (_buttonPressed != 25) {
				if (--_buttonHold == 0) {
					_buttonHold = 4;
					i = (_buttonPressed - 1) / 8;
					j = _buttonPressed - i * 8;

					switch (j) {
					case 1:
						volUp(i, 0);
						volUp(i, 1);
						break;
					case 2:
						volUp(i, 1);
						break;
					case 3:
						volDown(i, 0);
						volUp(i, 1);
						break;
					case 4:
						volDown(i, 1);
						break;
					case 5:
						volDown(i, 0);
						volDown(i, 1);
						break;
					case 6:
						volDown(i, 0);
						break;
					case 7:
						volUp(i, 0);
						volDown(i, 1);
						break;
					case 8:
						volUp(i, 0);
						break;
					default:
						break;
					}

					renderVolumeLight(i);
				}
			}
		}
	}

	if (_mouseState) {
		if ((_mouseState & BS1L_BUTTON_DOWN) && (_currentButton)) {
			// Set button pressed
			_buttonPressed = _currentButton;
			_buttonHold = 15;

			if (_buttonPressed == 25) {
				putButton(volumeButtons[_buttonPressed - 1].x1, volumeButtons[_buttonPressed - 1].y1, 1);
			} else {
				i = (_buttonPressed - 1) / 8;
				j = _buttonPressed - i * 8;
				renderVolumeDisc(i, _buttonPressed - i * 8);

				switch (j) {
				case 1:
					volUp(i, 0);
					volUp(i, 1);
					break;
				case 2:
					volUp(i, 1);
					break;
				case 3:
					volDown(i, 0);
					volUp(i, 1);
					break;
				case 4:
					volDown(i, 1);
					break;
				case 5:
					volDown(i, 0);
					volDown(i, 1);
					break;
				case 6:
					volDown(i, 0);
					break;
				case 7:
					volUp(i, 0);
					volDown(i, 1);
					break;
				case 8:
					volUp(i, 0);
					break;
				default:
					break;
				}

				renderVolumeLight(i);
			}
		}

		if (_mouseState & BS1L_BUTTON_UP) {
			if (_buttonPressed) {
				if (_buttonPressed == 25) {
					SwordEngine::_systemVars.snrStatus = SNR_MAINPANEL;
				} else {
					i = (_buttonPressed - 1) / 8;
					renderVolumeDisc(i, 0);
				}

				_buttonPressed = 0;
			} else {
				// Avoid stuck volume discs
				renderVolumeDisc(0, 0);
				renderVolumeDisc(1, 0);
				renderVolumeDisc(2, 0);
			}
		}
	}
}

void Control::removeVolume() {
	_resMan->resClose(SR_VLIGHT);
	_resMan->resClose(SR_VKNOB);
}

void Control::renderScrolls() {
	uint8 *src, *dst;
	FrameHeader *f;
	Sprite *srScroll1, *srScroll2;

	srScroll1 = (Sprite *)_resMan->fetchRes(SR_SCROLL1);
	srScroll2 = (Sprite *)_resMan->fetchRes(SR_SCROLL2);

	f = (FrameHeader *)((uint8 *)srScroll1 + _resMan->getUint32(srScroll1->spriteOffset[_scrollIndex[0]]));
	src = (uint8 *)f + sizeof(FrameHeader);
	dst = _screenBuf + SCROLL1X + SCREEN_WIDTH * SCROLL1Y;

	if (SwordEngine::isPsx()) {
		drawPsxComponent(PSX_CONFIRM, src, dst, f);
	} else {
		for (int i = 0; i < _resMan->getUint16(f->height); i++) {
			memcpy(dst, src, _resMan->getUint16(f->width));
			src += _resMan->getUint16(f->width);
			dst += SCREEN_WIDTH;
		}
	}

	f = (FrameHeader *)((uint8 *)srScroll2 + _resMan->getUint32(srScroll2->spriteOffset[_scrollIndex[1]]));
	src = (uint8 *)f + sizeof(FrameHeader);
	dst = _screenBuf + SCROLL2X + SCREEN_WIDTH * SCROLL2Y;

	if (SwordEngine::isPsx()) {
		drawPsxComponent(PSX_CONFIRM, src, dst, f);
	} else {
		for (int i = 0; i < _resMan->getUint16(f->height); i++) {
			memcpy(dst, src, _resMan->getUint16(f->width));
			src += _resMan->getUint16(f->width);
			dst += SCREEN_WIDTH;
		}
	}
}

void Control::initialiseSpeed() {
	uint8 *src, *dst;
	FrameHeader *f;
	Sprite *srSpeed;

	_resMan->resOpen(SR_SCROLL1);
	_resMan->resOpen(SR_SCROLL2);

	srSpeed = (Sprite *)_resMan->openFetchRes(SR_SPEED);

	f = (FrameHeader *)((uint8 *)srSpeed + _resMan->getUint32(srSpeed->spriteOffset[0]));
	src = (uint8 *)f + sizeof(FrameHeader);
	dst = _screenBuf +
		((SCREEN_WIDTH - _resMan->getUint16(f->width)) / 2) +
		(SCREEN_WIDTH * ((SCREEN_DEPTH - _resMan->getUint16(f->height)) / 2));

	if (SwordEngine::isPsx()) {
		drawPsxComponent(PSX_PANEL, src, dst, f);
	} else {
		for (int i = 0; i < _resMan->getUint16(f->height); i++) {
			memcpy(dst, src, _resMan->getUint16(f->width));
			src += _resMan->getUint16(f->width);
			dst += SCREEN_WIDTH;
		}
	}

	_resMan->resClose(SR_SPEED);

	_scrollIndex[0] = 0;
	_scrollIndex[1] = 0;

	renderText(_lStrings[STR_SPEED], 216, 100);
	_numButtons = 3;
	putButton(speedButtons[0].x1, speedButtons[0].y1, _speedFlag);
	putButton(speedButtons[1].x1, speedButtons[1].y1, 1 - _speedFlag);
	putButton(speedButtons[2].x1, speedButtons[2].y1, 0);

	renderText(_lStrings[STR_DONE], (speedButtons[2].x1 - getTextLength(_lStrings[STR_DONE]) - 20), speedButtons[2].y1);
	renderScrolls();
}

void Control::implementSpeed() {
	Sprite *srScroll1, *srScroll2;

	srScroll1 = (Sprite *)_resMan->fetchRes(SR_SCROLL1);
	srScroll2 = (Sprite *)_resMan->fetchRes(SR_SCROLL2);

	if (++_scrollIndex[0] == (int32)_resMan->getUint32(srScroll1->totalSprites))
		_scrollIndex[0] = 0;
	if (++_scrollIndex[1] == (int32)_resMan->getUint32(srScroll2->totalSprites))
		_scrollIndex[1] = 0;

	renderScrolls();

	_currentButton = getCurrentButton(&speedButtons[0]);

	if ((_buttonPressed == 3) && (!_currentButton)) {
		//Reset button pressed
		putButton(speedButtons[_buttonPressed - 1].x1, speedButtons[_buttonPressed - 1].y1, 0);
		_buttonPressed = 0;
	}

	if (_mouseState) {
		if ((_mouseState & BS1L_BUTTON_DOWN) && (_currentButton)) {
			//Set button pressed
			_buttonPressed = _currentButton;
			if (_buttonPressed == 3) {
				putButton(speedButtons[2].x1, speedButtons[2].y1, 1);
			} else {
				if (_speedFlag == _buttonPressed - 1) {
					_speedFlag = 2 - _buttonPressed;
					putButton(speedButtons[0].x1, speedButtons[0].y1, _speedFlag);
					putButton(speedButtons[1].x1, speedButtons[1].y1, 1 - _speedFlag);
				}
			}
		}

		if ((_mouseState & BS1L_BUTTON_UP) && (_buttonPressed)) {
			if (_buttonPressed == 3) {
				SwordEngine::_systemVars.snrStatus = SNR_MAINPANEL;
			}

			_buttonPressed = 0;
		}
	}
}

void Control::removeSpeed() {
	_resMan->resClose(SR_SCROLL1);
	_resMan->resClose(SR_SCROLL2);

	SwordEngine::_systemVars.parallaxOn = 1 - _speedFlag;
}

int16 Control::readFileDescriptions() {
	char saveName[40];
	Common::String pattern = "sword1.???";
	Common::StringArray filenames = _saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end()); // Sort (hopefully ensuring we are sorted numerically...)

	int16 totalFiles = 0;
	int slotNum = 0;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNum = atoi(file->c_str() + file->size() - 3);

		while (totalFiles < slotNum) {
			Common::strcpy_s((char *)_fileDescriptions[totalFiles], 1, "");
			totalFiles++;
		}

		if (slotNum >= 0 && slotNum < MAXSAVEGAMES) {
			Common::InSaveFile *in = _saveFileMan->openForLoading(*file);
			if (in) {
				in->readUint32LE(); // header
				in->read(saveName, 40);
				Common::strcpy_s((char *)_fileDescriptions[totalFiles], sizeof(_fileDescriptions[totalFiles]), saveName);
				delete in;
			}

			totalFiles++;
		}
	}

	for (int i = totalFiles; i < MAXSAVEGAMES; i++)
		Common::strcpy_s((char *)_fileDescriptions[i], 1, "");

	return totalFiles;
}

void Control::setEditDescription(int32 line) {
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

	_editingDescription = line;
	_textCursor = 0;
	_curCharCount = 1;

	Common::strcpy_s(_oldString, sizeof(_oldString), (const char *)_fileDescriptions[_editingDescription + _firstDescription - 1]);
	_slabSelected = line;
}

bool Control::driveSpaceAvailable() {
	char fName[15];
	int slot = _firstDescription + _editingDescription - 1;
	Common::sprintf_s(fName, "sword1.%03d", slot);

	// We are going to write a fake savegame file, to see if
	// we are actually able to write anything with that size.
	// Why are we even doing this? Can't we save right now?
	// Turns out: no; we need to save at the end of getPlayerOptions(),
	// after some clean-up operations are carried out. Right now,
	// we only want to know if we are even able to save.
	Common::OutSaveFile *outf;
	outf = _saveFileMan->openForSaving(fName, false);
	if (!outf) {
		return false;
	}

	int sizeOfFakeSavegame = 30000;
	outf->writeUint32LE(SAVEGAME_HEADER);
	outf->write(_fileDescriptions[slot], 40);
	outf->writeByte(SAVEGAME_VERSION);

	for (int i = 0; i < sizeOfFakeSavegame; i++)
		outf->writeByte(i % 256);

	outf->finalize();

	if (outf->err())
		return false;

	return true;
}

bool Control::attemptSave() {
	// If this slot already has a savegame, or there's enough space for a new savegame
	if (strlen(_oldString) || driveSpaceAvailable()) {
		_selectedSavegame = _firstDescription + _editingDescription - 1;
		if (_firstDescription + _editingDescription > _gamesSaved)
			_gamesSaved = _firstDescription + _editingDescription;

		uneditDescription();

		SwordEngine::_systemVars.saveGameFlag = SGF_SAVE;
		SwordEngine::_systemVars.snrStatus = SNR_BLANK;

		return true;
	} else {
		return false;
	}
}

void Control::editDescription() {
	char string[40];
	int32 len;
	int32 index;
	if (_keyPressed.keycode) {
		uint16 ch = _keyPressed.ascii;
		_keyPressed.reset();
		_customType = kActionNone;

		index = _editingDescription + _firstDescription - 1;
		len = Common::strnlen((char *)_fileDescriptions[index], sizeof(_fileDescriptions[index]));
		if ((ch != CR) && (ch != ESCAPE)) {
			if ((ch >= FIRSTFONTCHAR) && (ch <= LASTFONTCHAR)) {
				if ((len < 32) && (getTextLength((const uint8 *)_fileDescriptions[index]) < 310)) {
					_fileDescriptions[index][len] = ch;
					_fileDescriptions[index][len + 1] = '\0';
					_curCharCount = 1;
				}
			}
		}

		if (ch == BACKSPACE) {
			if (len) {
				_fileDescriptions[index][len - 1] = '\0';
				_curCharCount = 1;
			}
		}

		if ((ch == ESCAPE) || (ch == CR)) {
			if ((ch == ESCAPE) || ((ch == CR) && (len == 0))) {
				_textCursor = 1;
				_curCharCount = 10;
				Common::strcpy_s((char *)_fileDescriptions[index], sizeof(_fileDescriptions[index]), _oldString);
				uneditDescription();
			} else {
				if (!attemptSave())
					SwordEngine::_systemVars.snrStatus = SNR_DRIVEFULL;
			}
		}
	}

	if (SwordEngine::_systemVars.snrStatus != SNR_BLANK) {
		if (--_curCharCount == 0) {
			_curCharCount = 10;
			_textCursor ^= 1;
			if (_textCursor) {
				// Add cursor to line
				Common::strcpy_s(string, sizeof(string), (const char *)_fileDescriptions[_editingDescription + _firstDescription - 1]);
				len = strlen(string);
				string[len] = '_';
				string[len + 1] = '\0';
			} else {
				// Remove cursor from line
				Common::strcpy_s(string, sizeof(string), (const char *)_fileDescriptions[_editingDescription + _firstDescription - 1]);
			}

			renderSlab(_slabSelected - 1, _editingDescription - 1);
			renderRedText((const uint8 *)string, saveButtons[_editingDescription - 1].x1 + 42, saveButtons[_editingDescription - 1].y1 + 5);
			Common::sprintf_s(string, sizeof(string), "%d", _firstDescription + _editingDescription);
			renderRedText((const uint8 *)string, saveButtons[_editingDescription - 1].x1 + 12, saveButtons[_editingDescription - 1].y1 + 5);
		}
	}
}

void Control::restoreSelected() {
	if (_keyPressed.keycode) {
		char ch = _keyPressed.ascii;
		_keyPressed.reset();
		_customType = kActionNone;

		if ((ch == ESCAPE) || (ch == CR)) {
			if (ch == ESCAPE) {
				uneditDescription();
			} else {
				// Restore the game here
				_selectedSavegame = _editingDescription + _firstDescription - 1;
				uneditDescription();
				SwordEngine::_systemVars.saveGameFlag = SGF_RESTORE;
				SwordEngine::_systemVars.snrStatus = SNR_BLANK;
			}
		}
	}
}

void Control::uneditDescription(void) {
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);

	_slabSelected = 0;

	renderSlabs();
	renderTexts();
	_textCursor = 0;
	_editingDescription = 0;
}

bool Control::saveGame() {
	if (Common::strnlen((char *)_fileDescriptions[_selectedSavegame], sizeof(_fileDescriptions[_selectedSavegame]) > 0)) {
		saveGameToFile(_selectedSavegame);
		return true;
	}

	return false;
}

void Control::initialiseSave() {
	Common::Keymapper *keymapper = _vm->getEventManager()->getKeymapper();
	keymapper->getKeymap("game-shortcuts")->setEnabled(false);

	uint8 *src, *dst;
	int32 size;
	FrameHeader *f;
	Sprite *srWindow;
	Sprite *srButuf, *srButus, *srButds, *srButdf;
	Sprite *srSlab1, *srSlab2, *srSlab3, *srSlab4;

	if (SwordEngine::_systemVars.language == BS1_CZECH)
		_resMan->resOpen(CZECH_SR_REDFONT);
	else
		_resMan->resOpen(SR_REDFONT);

	srWindow = (Sprite *)_resMan->openFetchRes(SR_WINDOW);

	f = (FrameHeader *)((uint8 *)srWindow + _resMan->getUint32(srWindow->spriteOffset[0]));
	src = (uint8 *)f + sizeof(FrameHeader);
	dst = _screenBuf +
		((SCREEN_WIDTH - _resMan->getUint16(f->width)) / 2) +
		(SCREEN_WIDTH * ((SCREEN_DEPTH - _resMan->getUint16(f->height)) / 2));

	if (SwordEngine::isPsx()) {
		drawPsxComponent(PSX_PANEL, src, dst, f);
	} else {
		for (int i = 0; i < _resMan->getUint16(f->height); i++) {
			memcpy(dst, src, _resMan->getUint16(f->width));
			src += _resMan->getUint16(f->width);
			dst += SCREEN_WIDTH;
		}
	}

	_resMan->resClose(SR_WINDOW);

	_numButtons = SAVEBUTTONS;
	putButton(saveButtons[SAVEBUTTONS - 2].x1, saveButtons[SAVEBUTTONS - 2].y1, 0);
	putButton(saveButtons[SAVEBUTTONS - 1].x1, saveButtons[SAVEBUTTONS - 1].y1, 0);

	srButuf = (Sprite *)_resMan->openFetchRes(SR_BUTUF);
	putSpriteButton(srButuf, saveButtons[SAVEBUTTONS - 6].x1, saveButtons[SAVEBUTTONS - 6].y1, 0);
	_resMan->resClose(SR_BUTUF);

	srButus = (Sprite *)_resMan->openFetchRes(SR_BUTUS);
	putSpriteButton(srButus, saveButtons[SAVEBUTTONS - 5].x1, saveButtons[SAVEBUTTONS - 5].y1, 0);
	_resMan->resClose(SR_BUTUS);

	srButds = (Sprite *)_resMan->openFetchRes(SR_BUTDS);
	putSpriteButton(srButds, saveButtons[SAVEBUTTONS - 4].x1, saveButtons[SAVEBUTTONS - 4].y1, 0);
	_resMan->resClose(SR_BUTDS);

	srButdf = (Sprite *)_resMan->openFetchRes(SR_BUTDF);
	putSpriteButton(srButdf, saveButtons[SAVEBUTTONS - 3].x1, saveButtons[SAVEBUTTONS - 3].y1, 0);
	_resMan->resClose(SR_BUTDF);

	renderText(_lStrings[STR_SAVE], saveButtons[SAVEBUTTONS - 2].x1 + 40, saveButtons[SAVEBUTTONS - 2].y1);
	renderText(_lStrings[STR_CANCEL], saveButtons[SAVEBUTTONS - 1].x1 - 15 - getTextLength(_lStrings[STR_CANCEL]), saveButtons[SAVEBUTTONS - 1].y1);

	_gamesSaved = readFileDescriptions();

	// Create the slabs here
	srSlab1 = (Sprite *)_resMan->openFetchRes(SR_SLAB1);

	if (SwordEngine::isPsx()) {
		f = (FrameHeader *)((uint8 *)srSlab1 + _resMan->getUint32(srSlab1->spriteOffset[0]));
		size = sizeof(Sprite) + sizeof(FrameHeader) + _resMan->getUint16(f->width) / 2 * _resMan->getUint16(f->height);
		f = (FrameHeader *)((uint8 *)srSlab1 + _resMan->getUint32(srSlab1->spriteOffset[1]));
		size += (sizeof(FrameHeader) + _resMan->getUint16(f->width) / 2 * _resMan->getUint16(f->height));
	} else {
		f = (FrameHeader *)((uint8 *)srSlab1 + _resMan->getUint32(srSlab1->spriteOffset[0]));
		size = sizeof(Sprite) + sizeof(FrameHeader) + _resMan->getUint16(f->width) * _resMan->getUint16(f->height);
		f = (FrameHeader *)((uint8 *)srSlab1 + _resMan->getUint32(srSlab1->spriteOffset[1]));
		size += (sizeof(FrameHeader) + _resMan->getUint16(f->width) * _resMan->getUint16(f->height));
	}

	for (int i = 0; i < SAVEBUTTONS - 6; i++) {
		_slabs[i] = (Sprite *)malloc(size);
	}

	memcpy((uint8 *)_slabs[0], (uint8 *)srSlab1, size);
	memcpy((uint8 *)_slabs[4], (uint8 *)srSlab1, size);
	_resMan->resClose(SR_SLAB1);

	srSlab2 = (Sprite *)_resMan->openFetchRes(SR_SLAB2);
	memcpy((uint8 *)_slabs[1], (uint8 *)srSlab2, size);
	memcpy((uint8 *)_slabs[5], (uint8 *)srSlab2, size);
	_resMan->resClose(SR_SLAB2);

	srSlab3 = (Sprite *)_resMan->openFetchRes(SR_SLAB3);
	memcpy((uint8 *)_slabs[2], (uint8 *)srSlab3, size);
	memcpy((uint8 *)_slabs[6], (uint8 *)srSlab3, size);
	_resMan->resClose(SR_SLAB3);

	srSlab4 = (Sprite *)_resMan->openFetchRes(SR_SLAB4);
	memcpy((uint8 *)_slabs[3], (uint8 *)srSlab4, size);
	memcpy((uint8 *)_slabs[7], (uint8 *)srSlab4, size);
	_resMan->resClose(SR_SLAB4);

	renderSlabs();
	renderTexts();
}

void Control::implementSave() {
	Sprite *srButuf, *srButus, *srButds, *srButdf;

	_currentButton = getCurrentButton(&saveButtons[0]);

	if ((_buttonPressed) && (!_currentButton)) {
		// Reset button pressed
		if (_buttonPressed > SAVEBUTTONS - 6) {
			switch (_buttonPressed) {
			case (SAVEBUTTONS - 5):
				srButuf = (Sprite *)_resMan->openFetchRes(SR_BUTUF);
				putSpriteButton(srButuf, saveButtons[SAVEBUTTONS - 6].x1, saveButtons[SAVEBUTTONS - 6].y1, 0);
				_resMan->resClose(SR_BUTUF);
				_scroll = 0;
				break;
			case (SAVEBUTTONS - 4):
				srButus = (Sprite *)_resMan->openFetchRes(SR_BUTUS);
				putSpriteButton(srButus, saveButtons[SAVEBUTTONS - 5].x1, saveButtons[SAVEBUTTONS - 5].y1, 0);
				_resMan->resClose(SR_BUTUS);
				_scroll = 0;
				break;
			case (SAVEBUTTONS - 3):
				srButds = (Sprite *)_resMan->openFetchRes(SR_BUTDS);
				putSpriteButton(srButds, saveButtons[SAVEBUTTONS - 4].x1, saveButtons[SAVEBUTTONS - 4].y1, 0);
				_resMan->resClose(SR_BUTDS);
				_scroll = 0;
				break;
			case (SAVEBUTTONS - 2):
				srButdf = (Sprite *)_resMan->openFetchRes(SR_BUTDF);
				putSpriteButton(srButdf, saveButtons[SAVEBUTTONS - 3].x1, saveButtons[SAVEBUTTONS - 3].y1, 0);
				_resMan->resClose(SR_BUTDF);
				_scroll = 0;
				break;
			case (SAVEBUTTONS - 1):
			case (SAVEBUTTONS):
				putButton(saveButtons[_buttonPressed - 1].x1, saveButtons[_buttonPressed - 1].y1, 0);
				break;
			}
		}

		_buttonPressed = 0;
	}

	// Enhancement! Scrollable savegames list! :-)
	bool useWheelScroll = false;
	if (_mouseState && _currentButton >= 1 && _currentButton <= 8) {
		if (_mouseState & BS1_WHEEL_DOWN) {
			_buttonPressed = (SAVEBUTTONS - 3);
			useWheelScroll = true;
		} else if (_mouseState & BS1_WHEEL_UP) {
			_buttonPressed = (SAVEBUTTONS - 4);
			useWheelScroll = true;
		}
	}

	if (_mouseState) {
		if ((useWheelScroll || (_mouseState & BS1L_BUTTON_DOWN)) && (_currentButton)) {
			// Set button pressed
			if (!useWheelScroll)
				_buttonPressed = _currentButton;

			if (_buttonPressed) {
				if (_buttonPressed > SAVEBUTTONS - 6) {
					switch (_buttonPressed) {
					case (SAVEBUTTONS - 5):
						srButuf = (Sprite *)_resMan->openFetchRes(SR_BUTUF);
						putSpriteButton(srButuf, saveButtons[SAVEBUTTONS - 6].x1, saveButtons[SAVEBUTTONS - 6].y1, 1);
						_resMan->resClose(SR_BUTUF);

						if (!_editingDescription) {
							_scroll = 1;
							_scrollCount = 10;
							if (_firstDescription > 0) {
								if (_firstDescription - (SAVEBUTTONS - 6) < 0)
									_firstDescription = 0;
								else
									_firstDescription -= (SAVEBUTTONS - 6);
								renderSlabs();
								renderTexts();
							}
						}

						break;
					case (SAVEBUTTONS - 4):
						// Don't render the button as pressed when using wheel scrolling
						if (!useWheelScroll) {
							srButus = (Sprite *)_resMan->openFetchRes(SR_BUTUS);
							putSpriteButton(srButus, saveButtons[SAVEBUTTONS - 5].x1, saveButtons[SAVEBUTTONS - 5].y1, 1);
							_resMan->resClose(SR_BUTUS);
						}

						if (!_editingDescription) {
							_scroll = 2;
							_scrollCount = 10;
							if (_firstDescription > 0) {
								_firstDescription -= 1;
								renderSlabs();
								renderTexts();
							}
						}

						break;
					case (SAVEBUTTONS - 3):
						// Don't render the button as pressed when using wheel scrolling
						if (!useWheelScroll) {
							srButds = (Sprite *)_resMan->openFetchRes(SR_BUTDS);
							putSpriteButton(srButds, saveButtons[SAVEBUTTONS - 4].x1, saveButtons[SAVEBUTTONS - 4].y1, 1);
							_resMan->resClose(SR_BUTDS);
						}

						if (!_editingDescription) {
							_scroll = 3;
							_scrollCount = 10;
							if (_firstDescription < MAXSAVEGAMES - (SAVEBUTTONS - 6)) {
								_firstDescription += 1;
								renderSlabs();
								renderTexts();
							}
						}

						break;
					case (SAVEBUTTONS - 2):
						srButdf = (Sprite *)_resMan->openFetchRes(SR_BUTDF);
						putSpriteButton(srButdf, saveButtons[SAVEBUTTONS - 3].x1, saveButtons[SAVEBUTTONS - 3].y1, 1);
						_resMan->resClose(SR_BUTDF);

						if (!_editingDescription) {
							_scroll = 4;
							_scrollCount = 10;
							if (_firstDescription < MAXSAVEGAMES - (SAVEBUTTONS - 6)) {
								if (_firstDescription + (SAVEBUTTONS - 6) > MAXSAVEGAMES - (SAVEBUTTONS - 6))
									_firstDescription = MAXSAVEGAMES - (SAVEBUTTONS - 6);
								else
									_firstDescription += (SAVEBUTTONS - 6);
								renderSlabs();
								renderTexts();
							}
						}

						break;
					case (SAVEBUTTONS - 1):
					case (SAVEBUTTONS):
						putButton(saveButtons[_buttonPressed - 1].x1, saveButtons[_buttonPressed - 1].y1, 1);
						break;
					}
				} else {
					if (_editingDescription) {
						if (_buttonPressed != _editingDescription) {
							Common::strcpy_s(
								(char *)_fileDescriptions[_editingDescription + _firstDescription - 1],
								sizeof(_fileDescriptions[_editingDescription + _firstDescription - 1]),
								_oldString);
							_slabSelected = 0;
							uneditDescription();

							setEditDescription(_buttonPressed);
						}
					} else {
						setEditDescription(_buttonPressed);
					}
				}
			}
		}

		if (useWheelScroll)
			_scroll = 0;

		if ((_mouseState & BS1L_BUTTON_UP) && (_buttonPressed)) {
			if (_buttonPressed > SAVEBUTTONS - 6) {
				switch (_buttonPressed) {
				case (SAVEBUTTONS - 5):
					srButuf = (Sprite *)_resMan->openFetchRes(SR_BUTUF);
					putSpriteButton(srButuf, saveButtons[SAVEBUTTONS - 6].x1, saveButtons[SAVEBUTTONS - 6].y1, 0);
					_resMan->resClose(SR_BUTUF);
					_scroll = 0;
					break;
				case (SAVEBUTTONS - 4):
					srButus = (Sprite *)_resMan->openFetchRes(SR_BUTUS);
					putSpriteButton(srButus, saveButtons[SAVEBUTTONS - 5].x1, saveButtons[SAVEBUTTONS - 5].y1, 0);
					_resMan->resClose(SR_BUTUS);
					_scroll = 0;
					break;
				case (SAVEBUTTONS - 3):
					srButds = (Sprite *)_resMan->openFetchRes(SR_BUTDS);
					putSpriteButton(srButds, saveButtons[SAVEBUTTONS - 4].x1, saveButtons[SAVEBUTTONS - 4].y1, 0);
					_resMan->resClose(SR_BUTDS);
					_scroll = 0;
					break;
				case (SAVEBUTTONS - 2):
					srButdf = (Sprite *)_resMan->openFetchRes(SR_BUTDF);
					putSpriteButton(srButdf, saveButtons[SAVEBUTTONS - 3].x1, saveButtons[SAVEBUTTONS - 3].y1, 0);
					_resMan->resClose(SR_BUTDF);
					_scroll = 0;
					break;
				case (SAVEBUTTONS - 1):
					putButton(saveButtons[SAVEBUTTONS - 2].x1, saveButtons[SAVEBUTTONS - 1].y1, 0);
					if ((_editingDescription) && (strlen((char *)_fileDescriptions[_editingDescription + _firstDescription - 1]))) {
						if (!attemptSave())
							SwordEngine::_systemVars.snrStatus = SNR_DRIVEFULL;
					}
					break;
				case (SAVEBUTTONS):
					putButton(saveButtons[SAVEBUTTONS - 1].x1, saveButtons[SAVEBUTTONS - 1].y1, 0);
					if (_editingDescription)
						uneditDescription();
					SwordEngine::_systemVars.snrStatus = SNR_MAINPANEL;
					break;
				}
			}

			_buttonPressed = 0;
		}
	}

	if (_scroll) {
		if (--_scrollCount == 0) {

			_scrollCount = 2;

			switch (_scroll) {
			case 1:
				if (_firstDescription > 0) {
					if (_firstDescription - (SAVEBUTTONS - 6) < 0)
						_firstDescription = 0;
					else
						_firstDescription -= (SAVEBUTTONS - 6);
					renderSlabs();
					renderTexts();
				}
				break;
			case 2:
				if (_firstDescription > 0) {
					_firstDescription -= 1;
					renderSlabs();
					renderTexts();
				}
				break;
			case 3:
				if (_firstDescription < MAXSAVEGAMES - (SAVEBUTTONS - 6)) {
					_firstDescription += 1;
					renderSlabs();
					renderTexts();
				}
				break;
			case 4:
				if (_firstDescription < MAXSAVEGAMES - (SAVEBUTTONS - 6)) {
					if (_firstDescription + (SAVEBUTTONS - 6) > MAXSAVEGAMES - (SAVEBUTTONS - 6))
						_firstDescription = MAXSAVEGAMES - (SAVEBUTTONS - 6);
					else
						_firstDescription += (SAVEBUTTONS - 6);
					renderSlabs();
					renderTexts();
				}
				break;
			}
		}
	}

	if (_editingDescription)
		editDescription();
}

void Control::removeSave() {
	for (int i = 0; i < ARRAYSIZE(_slabs); i++) {
		free(_slabs[i]);
		_slabs[i] = nullptr;
	}

	if (SwordEngine::_systemVars.language == BS1_CZECH) {
		_resMan->resClose(CZECH_SR_REDFONT);
	} else {
		_resMan->resClose(SR_REDFONT);
	}

	_sound->setVolumes();

	Common::Keymapper *keymapper = _vm->getEventManager()->getKeymapper();
	keymapper->getKeymap("game-shortcuts")->setEnabled(true);
}

bool Control::restoreGame() {
	if (_selectedSavegame < MAXSAVEGAMES) {
		restoreGameFromFile(_selectedSavegame);
		doRestore();

		return true;
	}

	return false;
}

void Control::initialiseRestore() {
	uint8 *src, *dst;
	int32 size;
	FrameHeader *f;
	Sprite *srWindow;
	Sprite *srButuf, *srButus, *srButds, *srButdf;
	Sprite *srSlab1, *srSlab2, *srSlab3, *srSlab4;

	if (SwordEngine::_systemVars.language == BS1_CZECH)
		_resMan->resOpen(CZECH_SR_REDFONT);
	else
		_resMan->resOpen(SR_REDFONT);

	srWindow = (Sprite *)_resMan->openFetchRes(SR_WINDOW);

	f = (FrameHeader *)((uint8 *)srWindow + _resMan->getUint32(srWindow->spriteOffset[0]));
	src = (uint8 *)f + sizeof(FrameHeader);
	dst = _screenBuf +
		((SCREEN_WIDTH - _resMan->getUint16(f->width)) / 2) +
		(SCREEN_WIDTH * ((SCREEN_DEPTH - _resMan->getUint16(f->height)) / 2));

	if (SwordEngine::isPsx()) {
		drawPsxComponent(PSX_PANEL, src, dst, f);
	} else {
		for (int i = 0; i < _resMan->getUint16(f->height); i++) {
			memcpy(dst, src, _resMan->getUint16(f->width));
			src += _resMan->getUint16(f->width);
			dst += SCREEN_WIDTH;
		}
	}

	_resMan->resClose(SR_WINDOW);

	_numButtons = SAVEBUTTONS;
	putButton(saveButtons[SAVEBUTTONS - 2].x1, saveButtons[SAVEBUTTONS - 2].y1, 0);
	putButton(saveButtons[SAVEBUTTONS - 1].x1, saveButtons[SAVEBUTTONS - 1].y1, 0);

	srButuf = (Sprite *)_resMan->openFetchRes(SR_BUTUF);
	putSpriteButton(srButuf, saveButtons[SAVEBUTTONS - 6].x1, saveButtons[SAVEBUTTONS - 6].y1, 0);
	_resMan->resClose(SR_BUTUF);

	srButus = (Sprite *)_resMan->openFetchRes(SR_BUTUS);
	putSpriteButton(srButus, saveButtons[SAVEBUTTONS - 5].x1, saveButtons[SAVEBUTTONS - 5].y1, 0);
	_resMan->resClose(SR_BUTUS);

	srButds = (Sprite *)_resMan->openFetchRes(SR_BUTDS);
	putSpriteButton(srButds, saveButtons[SAVEBUTTONS - 4].x1, saveButtons[SAVEBUTTONS - 4].y1, 0);
	_resMan->resClose(SR_BUTDS);

	srButdf = (Sprite *)_resMan->openFetchRes(SR_BUTDF);
	putSpriteButton(srButdf, saveButtons[SAVEBUTTONS - 3].x1, saveButtons[SAVEBUTTONS - 3].y1, 0);
	_resMan->resClose(SR_BUTDF);

	renderText(_lStrings[STR_RESTORE], saveButtons[SAVEBUTTONS - 2].x1 + 40, saveButtons[SAVEBUTTONS - 2].y1);
	renderText(_lStrings[STR_CANCEL], saveButtons[SAVEBUTTONS - 1].x1 - 15 - getTextLength(_lStrings[STR_CANCEL]), saveButtons[SAVEBUTTONS - 1].y1);

	_gamesSaved = readFileDescriptions();

	// Create the slabs here
	srSlab1 = (Sprite *)_resMan->openFetchRes(SR_SLAB1);

	if (SwordEngine::isPsx()) {
		f = (FrameHeader *)((uint8 *)srSlab1 + _resMan->getUint32(srSlab1->spriteOffset[0]));
		size = sizeof(Sprite) + sizeof(FrameHeader) + _resMan->getUint16(f->width) / 2 * _resMan->getUint16(f->height);
		f = (FrameHeader *)((uint8 *)srSlab1 + _resMan->getUint32(srSlab1->spriteOffset[1]));
		size += (sizeof(FrameHeader) + _resMan->getUint16(f->width) / 2 * _resMan->getUint16(f->height));
	} else {
		f = (FrameHeader *)((uint8 *)srSlab1 + _resMan->getUint32(srSlab1->spriteOffset[0]));
		size = sizeof(Sprite) + sizeof(FrameHeader) + _resMan->getUint16(f->width) * _resMan->getUint16(f->height);
		f = (FrameHeader *)((uint8 *)srSlab1 + _resMan->getUint32(srSlab1->spriteOffset[1]));
		size += (sizeof(FrameHeader) + _resMan->getUint16(f->width) * _resMan->getUint16(f->height));
	}

	for (int i = 0; i < SAVEBUTTONS - 6; i++) {
		_slabs[i] = (Sprite *)malloc(size);
	}

	memcpy((uint8 *)_slabs[0], (uint8 *)srSlab1, size);
	memcpy((uint8 *)_slabs[4], (uint8 *)srSlab1, size);
	_resMan->resClose(SR_SLAB1);

	srSlab2 = (Sprite *)_resMan->openFetchRes(SR_SLAB2);
	memcpy((uint8 *)_slabs[1], (uint8 *)srSlab2, size);
	memcpy((uint8 *)_slabs[5], (uint8 *)srSlab2, size);
	_resMan->resClose(SR_SLAB2);

	srSlab3 = (Sprite *)_resMan->openFetchRes(SR_SLAB3);
	memcpy((uint8 *)_slabs[2], (uint8 *)srSlab3, size);
	memcpy((uint8 *)_slabs[6], (uint8 *)srSlab3, size);
	_resMan->resClose(SR_SLAB3);

	srSlab4 = (Sprite *)_resMan->openFetchRes(SR_SLAB4);
	memcpy((uint8 *)_slabs[3], (uint8 *)srSlab4, size);
	memcpy((uint8 *)_slabs[7], (uint8 *)srSlab4, size);
	_resMan->resClose(SR_SLAB4);

	renderSlabs();
	renderTexts();
}

void Control::implementRestore() {
	char string[40];

	Sprite *srButuf, *srButus, *srButds, *srButdf;

	_currentButton = getCurrentButton(&saveButtons[0]);

	if ((_buttonPressed) && (!_currentButton)) {
		//Reset button pressed
		if (_buttonPressed > SAVEBUTTONS - 6) {
			switch (_buttonPressed) {
			case (SAVEBUTTONS - 5):
				srButuf = (Sprite *)_resMan->openFetchRes(SR_BUTUF);
				putSpriteButton(srButuf, saveButtons[SAVEBUTTONS - 6].x1, saveButtons[SAVEBUTTONS - 6].y1, 0);
				_resMan->resClose(SR_BUTUF);
				_scroll = 0;
				break;
			case (SAVEBUTTONS - 4):
				srButus = (Sprite *)_resMan->openFetchRes(SR_BUTUS);
				putSpriteButton(srButus, saveButtons[SAVEBUTTONS - 5].x1, saveButtons[SAVEBUTTONS - 5].y1, 0);
				_resMan->resClose(SR_BUTUS);
				_scroll = 0;
				break;
			case (SAVEBUTTONS - 3):
				srButds = (Sprite *)_resMan->openFetchRes(SR_BUTDS);
				putSpriteButton(srButds, saveButtons[SAVEBUTTONS - 4].x1, saveButtons[SAVEBUTTONS - 4].y1, 0);
				_resMan->resClose(SR_BUTDS);
				_scroll = 0;
				break;
			case (SAVEBUTTONS - 2):
				srButdf = (Sprite *)_resMan->openFetchRes(SR_BUTDF);
				putSpriteButton(srButdf, saveButtons[SAVEBUTTONS - 3].x1, saveButtons[SAVEBUTTONS - 3].y1, 0);
				_resMan->resClose(SR_BUTDF);
				_scroll = 0;
				break;
			case (SAVEBUTTONS - 1):
			case (SAVEBUTTONS):
				putButton(saveButtons[_buttonPressed - 1].x1, saveButtons[_buttonPressed - 1].y1, 0);
				break;
			}
		}

		_buttonPressed = 0;
	}

	// Enhancement! Scrollable savegames list! :-)
	bool useWheelScroll = false;
	if (_mouseState && _currentButton >= 1 && _currentButton <= 8) {
		if (_mouseState & BS1_WHEEL_DOWN) {
			_buttonPressed = (SAVEBUTTONS - 3);
			useWheelScroll = true;
		} else if (_mouseState & BS1_WHEEL_UP) {
			_buttonPressed = (SAVEBUTTONS - 4);
			useWheelScroll = true;
		}
	}

	if (_mouseState) {
		if ((useWheelScroll || (_mouseState & BS1L_BUTTON_DOWN)) && (_currentButton)) {
			// Set button pressed
			if (!useWheelScroll)
				_buttonPressed = _currentButton;

			if (_buttonPressed) {
				if (_buttonPressed > SAVEBUTTONS - 6) {
					switch (_buttonPressed) {
					case (SAVEBUTTONS - 5):
						srButuf = (Sprite *)_resMan->openFetchRes(SR_BUTUF);
						putSpriteButton(srButuf, saveButtons[SAVEBUTTONS - 6].x1, saveButtons[SAVEBUTTONS - 6].y1, 1);
						_resMan->resClose(SR_BUTUF);

						if (_editingDescription) {
							uneditDescription();
						}

						_scroll = 1;
						_scrollCount = 10;
						if (_firstDescription > 0) {
							if (_firstDescription - (SAVEBUTTONS - 6) < 0)
								_firstDescription = 0;
							else
								_firstDescription -= (SAVEBUTTONS - 6);
							renderSlabs();
							renderTexts();
						}

						break;
					case (SAVEBUTTONS - 4):
						// Don't render the button as pressed when using wheel scrolling
						if (!useWheelScroll) {
							srButus = (Sprite *)_resMan->openFetchRes(SR_BUTUS);
							putSpriteButton(srButus, saveButtons[SAVEBUTTONS - 5].x1, saveButtons[SAVEBUTTONS - 5].y1, 1);
							_resMan->resClose(SR_BUTUS);
						}

						if (_editingDescription) {
							uneditDescription();
						}

						_scroll = 2;
						_scrollCount = 10;
						if (_firstDescription > 0) {
							_firstDescription -= 1;
							renderSlabs();
							renderTexts();
						}

						break;
					case (SAVEBUTTONS - 3):
						// Don't render the button as pressed when using wheel scrolling
						if (!useWheelScroll) {
							srButds = (Sprite *)_resMan->openFetchRes(SR_BUTDS);
							putSpriteButton(srButds, saveButtons[SAVEBUTTONS - 4].x1, saveButtons[SAVEBUTTONS - 4].y1, 1);
							_resMan->resClose(SR_BUTDS);
						}

						if (_editingDescription) {
							uneditDescription();
						}

						_scroll = 3;
						_scrollCount = 10;
						if (_firstDescription < MAXSAVEGAMES - (SAVEBUTTONS - 6)) {
							_firstDescription += 1;
							renderSlabs();
							renderTexts();
						}

						break;
					case (SAVEBUTTONS - 2):
						srButdf = (Sprite *)_resMan->openFetchRes(SR_BUTDF);
						putSpriteButton(srButdf, saveButtons[SAVEBUTTONS - 3].x1, saveButtons[SAVEBUTTONS - 3].y1, 1);
						_resMan->resClose(SR_BUTDF);

						if (_editingDescription) {
							uneditDescription();
						}

						_scroll = 4;
						_scrollCount = 10;
						if (_firstDescription < MAXSAVEGAMES - (SAVEBUTTONS - 6)) {
							if (_firstDescription + (SAVEBUTTONS - 6) > MAXSAVEGAMES - (SAVEBUTTONS - 6))
								_firstDescription = MAXSAVEGAMES - (SAVEBUTTONS - 6);
							else
								_firstDescription += (SAVEBUTTONS - 6);
							renderSlabs();
							renderTexts();
						}

						break;
					case (SAVEBUTTONS - 1):
					case (SAVEBUTTONS):
						putButton(saveButtons[_buttonPressed - 1].x1, saveButtons[_buttonPressed - 1].y1, 1);
						break;
					}
				} else {
					if (_editingDescription) {
						if ((_buttonPressed != _editingDescription) && (strlen((char *)_fileDescriptions[_buttonPressed + _firstDescription - 1]))) {
							Common::strcpy_s(_fileDescriptions[_editingDescription + _firstDescription - 1], _oldString);
							_slabSelected = 0;
							uneditDescription();

							setEditDescription(_buttonPressed);

							renderSlab(_slabSelected - 1, _editingDescription - 1);
							Common::strcpy_s(string, (char *)_fileDescriptions[_editingDescription + _firstDescription - 1]);
							renderRedText((const uint8 *)string, saveButtons[_editingDescription - 1].x1 + 42, saveButtons[_editingDescription - 1].y1 + 5);
							Common::sprintf_s(string, "%d", _firstDescription + _editingDescription);
							renderRedText((const uint8 *)string, saveButtons[_editingDescription - 1].x1 + 12, saveButtons[_editingDescription - 1].y1 + 5);
						}
					} else {
						if (strlen((char *)_fileDescriptions[_buttonPressed + _firstDescription - 1])) {
							setEditDescription(_buttonPressed);
							renderSlab(_slabSelected - 1, _editingDescription - 1);
							Common::strcpy_s(string, (char *)_fileDescriptions[_editingDescription + _firstDescription - 1]);
							renderRedText((const uint8 *)string, saveButtons[_editingDescription - 1].x1 + 42, saveButtons[_editingDescription - 1].y1 + 5);
							Common::sprintf_s(string, "%d", _firstDescription + _editingDescription);
							renderRedText((const uint8 *)string, saveButtons[_editingDescription - 1].x1 + 12, saveButtons[_editingDescription - 1].y1 + 5);
						}
					}
				}
			}
		}

		if (useWheelScroll)
			_scroll = 0;

		if ((_mouseState & BS1L_BUTTON_UP) && (_buttonPressed)) {
			if (_buttonPressed > SAVEBUTTONS - 6) {
				switch (_buttonPressed) {
				case (SAVEBUTTONS - 5):
					srButuf = (Sprite *)_resMan->openFetchRes(SR_BUTUF);
					putSpriteButton(srButuf, saveButtons[SAVEBUTTONS - 6].x1, saveButtons[SAVEBUTTONS - 6].y1, 0);
					_resMan->resClose(SR_BUTUF);
					_scroll = 0;
					break;
				case (SAVEBUTTONS - 4):
					srButus = (Sprite *)_resMan->openFetchRes(SR_BUTUS);
					putSpriteButton(srButus, saveButtons[SAVEBUTTONS - 5].x1, saveButtons[SAVEBUTTONS - 5].y1, 0);
					_resMan->resClose(SR_BUTUS);
					_scroll = 0;
					break;
				case (SAVEBUTTONS - 3):
					srButds = (Sprite *)_resMan->openFetchRes(SR_BUTDS);
					putSpriteButton(srButds, saveButtons[SAVEBUTTONS - 4].x1, saveButtons[SAVEBUTTONS - 4].y1, 0);
					_resMan->resClose(SR_BUTDS);
					_scroll = 0;
					break;
				case (SAVEBUTTONS - 2):
					srButdf = (Sprite *)_resMan->openFetchRes(SR_BUTDF);
					putSpriteButton(srButdf, saveButtons[SAVEBUTTONS - 3].x1, saveButtons[SAVEBUTTONS - 3].y1, 0);
					_resMan->resClose(SR_BUTDF);
					_scroll = 0;
					break;
				case (SAVEBUTTONS - 1):
					putButton(saveButtons[SAVEBUTTONS - 2].x1, saveButtons[SAVEBUTTONS - 1].y1, 0);
					if ((_editingDescription) && (strlen((char *)_fileDescriptions[_editingDescription + _firstDescription - 1]))) {
						//Restore game here
						_selectedSavegame = _firstDescription + _editingDescription - 1;
						//Common::sprintf_s(saveFilename, "savegame.%.3d", _firstDescription + _editingDescription - 1);
						uneditDescription();
						SwordEngine::_systemVars.saveGameFlag = SGF_RESTORE;
						SwordEngine::_systemVars.snrStatus = SNR_BLANK;
					}
					break;
				case (SAVEBUTTONS):
					putButton(saveButtons[SAVEBUTTONS - 1].x1, saveButtons[SAVEBUTTONS - 1].y1, 0);
					if (_editingDescription)
						uneditDescription();
					SwordEngine::_systemVars.snrStatus = SNR_MAINPANEL;
					break;
				}
			}

			_buttonPressed = 0;
		}
	}

	if (_scroll) {
		if (--_scrollCount == 0) {
			_scrollCount = 2;

			switch (_scroll) {
			case 1:
				if (_firstDescription > 0) {
					if (_firstDescription - (SAVEBUTTONS - 6) < 0)
						_firstDescription = 0;
					else
						_firstDescription -= (SAVEBUTTONS - 6);
					renderSlabs();
					renderTexts();
				}
				break;
			case 2:
				if (_firstDescription > 0) {
					_firstDescription -= 1;
					renderSlabs();
					renderTexts();
				}
				break;
			case 3:
				if (_firstDescription < MAXSAVEGAMES - (SAVEBUTTONS - 6)) {
					_firstDescription += 1;
					renderSlabs();
					renderTexts();
				}
				break;
			case 4:
				if (_firstDescription < MAXSAVEGAMES - (SAVEBUTTONS - 6)) {
					if (_firstDescription + (SAVEBUTTONS - 6) > MAXSAVEGAMES - (SAVEBUTTONS - 6))
						_firstDescription = MAXSAVEGAMES - (SAVEBUTTONS - 6);
					else
						_firstDescription += (SAVEBUTTONS - 6);
					renderSlabs();
					renderTexts();
				}
				break;
			}
		}
	}

	if (_editingDescription)
		restoreSelected();
}

void Control::removeRestore() {
	for (int i = 0; i < ARRAYSIZE(_slabs); i++) {
		free(_slabs[i]);
		_slabs[i] = nullptr;
	}

	if (SwordEngine::_systemVars.language == BS1_CZECH) {
		_resMan->resClose(CZECH_SR_REDFONT);
	} else {
		_resMan->resClose(SR_REDFONT);
	}

	_sound->setVolumes();
}

void Control::initialiseControlPanel() {
	uint8 *src;
	uint8 *dst;
	FrameHeader *f;
	Sprite *srPanel = nullptr;

	_resMan->resOpen(SR_TEXT_BUTTON);
	_mouse->controlPanel(true);

	// Clear the render buffer and copy the control panel into it
	memset(_screenBuf, 0, SCREEN_WIDTH * SCREEN_FULL_DEPTH);
	_system->copyRectToScreen(_screenBuf, SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_FULL_DEPTH);

	if (SwordEngine::_systemVars.controlPanelMode != CP_DEATHSCREEN) { // NOT THE DEATH SCREEN ie. start game panel, normal control panel, or end of game
		switch (SwordEngine::_systemVars.language) {
		case BS1_ENGLISH:
			if (!SwordEngine::isPsx() && SwordEngine::_systemVars.realLanguage == Common::EN_USA) {
				srPanel = (Sprite *)_resMan->openFetchRes(SR_PANEL_AMERICAN);
			} else {
				srPanel = (Sprite *)_resMan->openFetchRes(SR_PANEL_ENGLISH);
			}

			break;
		case BS1_FRENCH:
			srPanel = (Sprite *)_resMan->openFetchRes(SR_PANEL_FRENCH);
			break;
		case BS1_GERMAN:
			srPanel = (Sprite *)_resMan->openFetchRes(SR_PANEL_GERMAN);
			break;
		case BS1_ITALIAN:
			srPanel = (Sprite *)_resMan->openFetchRes(SR_PANEL_ITALIAN);
			break;
		case BS1_SPANISH:
			srPanel = (Sprite *)_resMan->openFetchRes(SR_PANEL_SPANISH);
			break;
		case BS1_CZECH:
			srPanel = (Sprite *)_resMan->openFetchRes(SR_PANEL_SPANISH);
			break;
		case BS1_PORT:
			srPanel = (Sprite *)_resMan->openFetchRes(SR_PANEL_SPANISH);
			break;
		default:
			break;
		}
	} else {
		srPanel = (Sprite *)_resMan->openFetchRes(SR_DEATHPANEL);
	}

	if (srPanel) {
		f = (FrameHeader *)((uint8 *)srPanel + _resMan->getUint32(srPanel->spriteOffset[0]));
		src = (uint8 *)f + sizeof(FrameHeader);

		if (SwordEngine::isPsx()) {
			int component = SwordEngine::_systemVars.controlPanelMode == CP_DEATHSCREEN ? PSX_DEATHPANEL : PSX_PANEL;
			dst = _screenBuf +
				  SCREEN_WIDTH * (SCREEN_DEPTH - _resMan->getUint16(f->height)) / 2 +
				  (SCREEN_WIDTH - _resMan->getUint16(f->width)) / 2;

			if (component == PSX_DEATHPANEL)
				dst = _screenBuf;

			drawPsxComponent(component, src, dst, f);
		} else {
			dst = _screenBuf + SCREEN_WIDTH * (SCREEN_DEPTH - _resMan->getUint16(f->height)) / 2 + (SCREEN_WIDTH - _resMan->getUint16(f->width)) / 2;
			for (int i = 0; i < _resMan->getUint16(f->height); i++) {
				memcpy(dst, src, _resMan->getUint16(f->width));
				dst += SCREEN_WIDTH;
				src += _resMan->getUint16(f->width);
			}
		}

		_system->copyRectToScreen(_screenBuf, SCREEN_WIDTH, 0, 40, SCREEN_WIDTH, SCREEN_DEPTH);
	}

	if (SwordEngine::_systemVars.controlPanelMode != CP_DEATHSCREEN) {
		switch (SwordEngine::_systemVars.language) {
		case BS1_ENGLISH:
			if (SwordEngine::_systemVars.realLanguage == Common::EN_USA) {
				_resMan->resClose(SR_PANEL_AMERICAN);
			} else {
				_resMan->resClose(SR_PANEL_ENGLISH);
			}

			break;
		case BS1_FRENCH:
			_resMan->resClose(SR_PANEL_FRENCH);
			break;
		case BS1_GERMAN:
			_resMan->resClose(SR_PANEL_GERMAN);
			break;
		case BS1_ITALIAN:
			_resMan->resClose(SR_PANEL_ITALIAN);
			break;
		case BS1_SPANISH:
			_resMan->resClose(SR_PANEL_SPANISH);
			break;
		case BS1_CZECH:
			_resMan->resClose(SR_PANEL_SPANISH);
			break;
		case BS1_PORT:
			_resMan->resClose(SR_PANEL_SPANISH);
			break;
		default:
			break;
		}
	} else {
		_resMan->resClose(SR_DEATHPANEL);
	}

	// Draw the buttons...
	if (SwordEngine::_systemVars.controlPanelMode == CP_NORMAL) {
		_numButtons = 8;

		for (int i = 0; i < _numButtons; i++) {
			putButton(panelButtons[i].x1, panelButtons[i].y1, 0);
		}

		putTextButton(SwordEngine::_systemVars.showText ? 1 : 0);
	} else {
		_numButtons = 4;

		for (int i = 1; i < _numButtons; i++) {
			putButton(deathButtons[i].x1, deathButtons[i].y1, 0);
		}
	}

	// Render text elements...
	if (SwordEngine::_systemVars.controlPanelMode == CP_THEEND)
		renderText(_lStrings[STR_THE_END], (SCREEN_WIDTH - getTextLength(_lStrings[STR_THE_END])) / 2, 188);

	if (SwordEngine::_systemVars.controlPanelMode == CP_NORMAL) {
		renderText(_lStrings[STR_SAVE], 180, 188);
		renderText(_lStrings[STR_DONE], 460 - getTextLength(_lStrings[STR_DONE]), 332);

		renderText(_lStrings[STR_RESTORE], 180, 224);
		renderText(_lStrings[STR_RESTART], 180, 260);
		renderText(_lStrings[STR_QUIT], 180, 296);
	} else { // Start-game panel, death panel & end-game panel
		renderText(_lStrings[STR_RESTORE], 285, 224);

		if (SwordEngine::_systemVars.controlPanelMode == CP_NEWGAME) { // Start-game panel
			renderText(_lStrings[STR_START], 285, 260);
		} else {
			renderText(_lStrings[STR_RESTART], 285, 260);
		}

		renderText(_lStrings[STR_QUIT], 285, 296);
	}

	if (SwordEngine::_systemVars.controlPanelMode == CP_NORMAL) {
		renderText(_lStrings[STR_SPEED], 460 - getTextLength(_lStrings[STR_SPEED]), 188);
		renderText(_lStrings[STR_VOLUME], 460 - getTextLength(_lStrings[STR_VOLUME]), 224);
		renderText(_lStrings[STR_TEXT], 460 - getTextLength(_lStrings[STR_TEXT]), 260);
	}
}

void Control::implementControlPanel() {
	if (SwordEngine::_systemVars.controlPanelMode == CP_NORMAL) {
		_currentButton = getCurrentButton(&panelButtons[0]);
	} else {
		_currentButton = getCurrentButton(&deathButtons[0]);
	}

	if ((_buttonPressed) && (!_currentButton)) {
		// Reset button pressed
		if (_buttonPressed != TEXTBUTTONID) {
			if (SwordEngine::_systemVars.controlPanelMode == CP_NORMAL) {
				putButton(panelButtons[_buttonPressed - 1].x1, panelButtons[_buttonPressed - 1].y1, 0);
			} else {
				putButton(deathButtons[_buttonPressed - 1].x1, deathButtons[_buttonPressed - 1].y1, 0);
			}
		}

		_buttonPressed = 0;
	}

	if (_mouseState) {
		if ((_mouseState & BS1L_BUTTON_DOWN) && (_currentButton)) {
			// Set button pressed
			_buttonPressed = _currentButton;
			if (_buttonPressed != TEXTBUTTONID) {
				if (SwordEngine::_systemVars.controlPanelMode == CP_NORMAL) {
					putButton(panelButtons[_buttonPressed - 1].x1, panelButtons[_buttonPressed - 1].y1, 1);
				} else {
					putButton(deathButtons[_buttonPressed - 1].x1, deathButtons[_buttonPressed - 1].y1, 1);
				}
			} else {
				SwordEngine::_systemVars.showText = !SwordEngine::_systemVars.showText;
				putTextButton(SwordEngine::_systemVars.showText ? 1 : 0);
			}
		}

		if ((_mouseState & BS1L_BUTTON_UP) && (_buttonPressed)) {
			SwordEngine::_systemVars.snrStatus = _buttonPressed + 1;
			_buttonPressed = 0;
		}
	}
}

void Control::removeControlPanel() {
	_resMan->resClose(SR_TEXT_BUTTON);
}

void Control::initialiseResources() {
	if (SwordEngine::_systemVars.controlPanelMode == CP_DEATHSCREEN) {
		if (SwordEngine::_systemVars.language == BS1_CZECH) {
			_resMan->resOpen(CZECH_SR_DEATHFONT);
		} else {
			_resMan->resOpen(_resMan->getDeathFontId());
		}
	} else {
		if (SwordEngine::_systemVars.language == BS1_CZECH) {
			_resMan->resOpen(CZECH_SR_FONT);
		} else {
			_resMan->resOpen(SR_FONT);
		}
	}

	_resMan->resOpen(SR_BUTTON);

	_screen->fnSetFadeTargetPalette(0, 256, SR_PALETTE);
	_screen->fnSetFadeTargetPalette(0, 1, 0, BORDER_BLACK);
}

void Control::releaseResources() {
	_resMan->resClose(SR_BUTTON);

	if (SwordEngine::_systemVars.controlPanelMode == CP_DEATHSCREEN) {
		if (SwordEngine::_systemVars.language == BS1_CZECH) {
			_resMan->resClose(CZECH_SR_DEATHFONT);
		} else {
			_resMan->resClose(_resMan->getDeathFontId());
		}
	} else {
		if (SwordEngine::_systemVars.language == BS1_CZECH) {
			_resMan->resClose(CZECH_SR_FONT);
		} else {
			_resMan->resClose(SR_FONT);
		}
	}
}

void Control::delay(uint32 msecs) {
	Common::Event event;

	uint32 now = _system->getMillis();
	uint32 endTime = now + msecs;
	_keyPressed.reset();
	_customType = kActionNone;
	_mouseState = 0;

	do {
		Common::EventManager *eventMan = _system->getEventManager();
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				_customType = event.customType;
				return;
			case Common::EVENT_KEYDOWN:
				_keyPressed = event.kbd;
				// we skip the rest of the delay and return immediately
				// to handle keyboard input
				return;
			case Common::EVENT_MOUSEMOVE:
				_mouseCoord = event.mouse;
				break;
			case Common::EVENT_LBUTTONDOWN:
				_mouseState |= BS1L_BUTTON_DOWN;
				_mouseCoord = event.mouse;
				break;
			case Common::EVENT_LBUTTONUP:
				_mouseState |= BS1L_BUTTON_UP;
				_mouseCoord = event.mouse;
				break;
			case Common::EVENT_WHEELUP:
				_mouseState |= BS1_WHEEL_UP;
				_mouseCoord = event.mouse;
				break;
			case Common::EVENT_WHEELDOWN:
				_mouseState |= BS1_WHEEL_DOWN;
				break;
			default:
				break;
			}
		}

		_screen->_screenAccessMutex.lock();
		_system->updateScreen();
		_screen->_screenAccessMutex.unlock();

		_system->delayMillis(10);
	} while (_system->getMillis() < endTime);
}

uint8 *Control::decompressPsxGfx(uint8 *src, FrameHeader *f) {
	uint8 *dst = (uint8 *)malloc(_resMan->getUint16(f->height) * _resMan->getUint16(f->width));
	memset(dst, 0, _resMan->getUint16(f->height) * _resMan->getUint16(f->width));
	Screen::decompressHIF(src, dst);
	return dst;
}

void Control::drawPsxComponent(int componentType, uint8 *src, uint8 *dst, FrameHeader *f) {
	src = decompressPsxGfx(src, f);
	uint8 *initialPtr = src;

	switch (componentType) {
	case PSX_PANEL:
	case PSX_SLAB:
		// Draw the source image at double the resolution
		for (int i = 0; i < _resMan->getUint16(f->height); i++) {
			for (int j = 0; j < _resMan->getUint16(f->width) / 2; j++) {
				if (src[j]) {
					dst[j * 2] = src[j];
					dst[j * 2 + 1] = src[j];
				}
			}

			dst += SCREEN_WIDTH;

			for (int j = 0; j < _resMan->getUint16(f->width) / 2; j++) {
				if (src[j]) {
					dst[j * 2] = src[j];
					dst[j * 2 + 1] = src[j];
				}
			}

			dst += SCREEN_WIDTH;

			src += _resMan->getUint16(f->width) / 2;
		}

		break;
	case PSX_DEATHPANEL:
		// The PSX death panel  is 1/3 of the original width, so adjust for that...
		for (int i = 0; i < _resMan->getUint16(f->height) / 2; i++) {
			for (int j = 0; (j < (_resMan->getUint16(f->width)) / 3) && (j < (SCREEN_WIDTH - 3)); j++) {
				if (src[j]) {
					dst[j * 3] = src[j];
					dst[j * 3 + 1] = src[j];
					dst[j * 3 + 2] = src[j];
				}
			}

			dst += SCREEN_WIDTH;

			for (int j = 0; j < (_resMan->getUint16(f->width)) / 3; j++) {
				if (src[j]) {
					dst[j * 3] = src[j];
					dst[j * 3 + 1] = src[j];
					dst[j * 3 + 2] = src[j];
				}
			}

			dst += SCREEN_WIDTH;
			src += _resMan->getUint16(f->width) / 3;
		}

		break;
	case PSX_TEXT:
	case PSX_BUTTON:
	case PSX_CONFIRM:
	case PSX_SCROLL:
		for (int i = 0; i < _resMan->getUint16(f->height); i++) {
			for (int j = 0; j < _resMan->getUint16(f->width); j++) {
				if (src[j])
					dst[j] = src[j];
			}

			dst += SCREEN_WIDTH;

			for (int j = 0; j < _resMan->getUint16(f->width); j++) {
				if (src[j])
					dst[j] = src[j];
			}

			dst += SCREEN_WIDTH;
			src += _resMan->getUint16(f->width);
		}

		break;
	default:
		break;
	};

	free(initialPtr);
}

int Control::displayMessage(const char *altButton, const char *message, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, message);
	vsnprintf(buf, STRINGBUFLEN, message, va);
	va_end(va);

	GUI::MessageDialog dialog(buf, "OK", altButton);
	int result = dialog.runModal();
	_mouse->setPointer(MSE_POINTER, 0);
	return result;
}

void Control::checkForOldSaveGames() {
	Common::InSaveFile *inf = _saveFileMan->openForLoading("SAVEGAME.INF");

	if (!inf)
		return;

	GUI::MessageDialog dialog0(
	    _("ScummVM found that you have old saved games for Broken Sword 1 that should be converted.\n"
	      "The old saved game format is no longer supported, so you will not be able to load your games if you don't convert them.\n\n"
	      "Press OK to convert them now, otherwise you will be asked again the next time you start the game.\n"), _("OK"), _("Cancel"));

	int choice = dialog0.runModal();
	if (choice != GUI::kMessageOK) {
		// user pressed cancel
		return;
	}

	// Convert every save slot we find in the index file to the new format
	uint8 saveName[32];
	uint8 slot = 0;
	uint8 ch;

	memset(saveName, 0, sizeof(saveName));

	do {
		uint8 pos = 0;
		do {
			ch = inf->readByte();
			if (pos < sizeof(saveName) - 1) {
				if ((ch == 10) || (ch == 255) || (inf->eos()))
					saveName[pos++] = '\0';
				else if (ch >= 32)
					saveName[pos++] = ch;
			}
		} while ((ch != 10) && (ch != 255) && (!inf->eos()));

		if (pos > 1)    // if the slot has a description
			convertSaveGame(slot, (char *)saveName);
		slot++;
	} while ((ch != 255) && (!inf->eos()));

	delete inf;

	// Delete index file
	_saveFileMan->removeSavefile("SAVEGAME.INF");
}

void Control::saveGameToFile(uint8 slot) {
	char fName[15];
	uint16 cnt;
	Common::sprintf_s(fName, "sword1.%03d", slot);
	uint16 liveBuf[TOTAL_SECTIONS];
	Common::OutSaveFile *outf;
	outf = _saveFileMan->openForSaving(fName);
	if (!outf) {
		// Display an error message and do nothing
		displayMessage(0, "Unable to create file '%s'. (%s)", fName, _saveFileMan->popErrorDesc().c_str());
		return;
	}

	outf->writeUint32LE(SAVEGAME_HEADER);
	outf->write(_fileDescriptions[slot], 40);
	outf->writeByte(SAVEGAME_VERSION);

	// Saving can occur either delayed from the GMM (in which case the panel is now shown and we can make
	// a thumbnail now) or from the game menu (the panel, in which case we created the _tempThumbnail just
	// before showing the panel).
	if (!isPanelShown())
		Graphics::saveThumbnail(*outf);
	else if (_tempThumbnail)
		outf->write(_tempThumbnail->getData(), _tempThumbnail->size());

	// Date / time
	TimeDate curTime;
	_system->getTimeAndDate(curTime);

	uint32 saveDate = (curTime.tm_mday & 0xFF) << 24 | ((curTime.tm_mon + 1) & 0xFF) << 16 | ((curTime.tm_year + 1900) & 0xFFFF);
	uint16 saveTime = (curTime.tm_hour & 0xFF) << 8 | ((curTime.tm_min) & 0xFF);

	outf->writeUint32BE(saveDate);
	outf->writeUint16BE(saveTime);

	outf->writeUint32BE(g_engine->getTotalPlayTime() / 1000);

	_objMan->saveLiveList(liveBuf);
	for (cnt = 0; cnt < TOTAL_SECTIONS; cnt++)
		outf->writeUint16LE(liveBuf[cnt]);

	Object *cpt = _objMan->fetchObject(PLAYER);
	Logic::_scriptVars[CHANGE_DIR] = cpt->o_dir;
	Logic::_scriptVars[CHANGE_X] = cpt->o_xcoord;
	Logic::_scriptVars[CHANGE_Y] = cpt->o_ycoord;
	Logic::_scriptVars[CHANGE_STANCE] = STAND;
	Logic::_scriptVars[CHANGE_PLACE] = cpt->o_place;

	for (cnt = 0; cnt < NUM_SCRIPT_VARS; cnt++)
		outf->writeUint32LE(Logic::_scriptVars[cnt]);

	uint32 playerSize = (sizeof(Object) - 12000) / 4;
	uint32 *playerRaw = (uint32 *)cpt;
	for (uint32 cnt2 = 0; cnt2 < playerSize; cnt2++)
		outf->writeUint32LE(playerRaw[cnt2]);
	outf->finalize();
	if (outf->err())
		displayMessage(0, "Couldn't write to file '%s'. Device full? (%s)", fName, _saveFileMan->popErrorDesc().c_str());
	delete outf;
}

bool Control::restoreGameFromFile(uint8 slot) {
	char fName[15];
	uint16 cnt;
	Common::sprintf_s(fName, "sword1.%03d", slot);
	Common::InSaveFile *inf;
	inf = _saveFileMan->openForLoading(fName);
	if (!inf) {
		// Display an error message, and do nothing
		displayMessage(0, "Can't open file '%s'. (%s)", fName, _saveFileMan->popErrorDesc().c_str());
		return false;
	}

	uint saveHeader = inf->readUint32LE();
	if (saveHeader != SAVEGAME_HEADER) {
		// Display an error message, and do nothing
		displayMessage(0, "Saved game '%s' is corrupt", fName);
		return false;
	}

	inf->skip(40);      // skip description
	uint8 saveVersion = inf->readByte();

	if (saveVersion > SAVEGAME_VERSION) {
		warning("Different saved game version");
		return false;
	}

	if (saveVersion < 2) // These older version of the savegames used a flag to signal presence of thumbnail
		inf->skip(1);

	Graphics::skipThumbnail(*inf);

	inf->readUint32BE();    // save date
	inf->readUint16BE();    // save time

	if (saveVersion < 2) { // Before version 2 we didn't had play time feature
		g_engine->setTotalPlayTime(0);
	} else {
		g_engine->setTotalPlayTime(inf->readUint32BE() * 1000);
	}

	_restoreBuf = (uint8 *)malloc(
	                  TOTAL_SECTIONS * 2 +
	                  NUM_SCRIPT_VARS * 4 +
	                  (sizeof(Object) - 12000));

	uint16 *liveBuf = (uint16 *)_restoreBuf;
	uint32 *scriptBuf = (uint32 *)(_restoreBuf + 2 * TOTAL_SECTIONS);
	uint32 *playerBuf = (uint32 *)(_restoreBuf + 2 * TOTAL_SECTIONS + 4 * NUM_SCRIPT_VARS);

	for (cnt = 0; cnt < TOTAL_SECTIONS; cnt++)
		liveBuf[cnt] = inf->readUint16LE();

	for (cnt = 0; cnt < NUM_SCRIPT_VARS; cnt++)
		scriptBuf[cnt] = inf->readUint32LE();

	uint32 playerSize = (sizeof(Object) - 12000) / 4;
	for (uint32 cnt2 = 0; cnt2 < playerSize; cnt2++)
		playerBuf[cnt2] = inf->readUint32LE();

	if (inf->err() || inf->eos()) {
		displayMessage(0, "Can't read from file '%s'. (%s)", fName, _saveFileMan->popErrorDesc().c_str());
		delete inf;
		free(_restoreBuf);
		_restoreBuf = nullptr;
		return false;
	}
	delete inf;
	return true;
}

bool Control::convertSaveGame(uint8 slot, char *desc) {
	char oldFileName[15];
	char newFileName[40];
	Common::sprintf_s(oldFileName, "SAVEGAME.%03d", slot);
	Common::sprintf_s(newFileName, "sword1.%03d", slot);
	uint8 *saveData;
	int dataSize;

	// Check if the new file already exists
	Common::InSaveFile *testSave = _saveFileMan->openForLoading(newFileName);

	if (testSave) {
		delete testSave;

		Common::U32String msg = Common::U32String::format(_("Target new saved game already exists!\n"
		                     "Would you like to keep the old saved game (%s) or the new one (%s)?\n"),
		                     oldFileName, newFileName);
		GUI::MessageDialog dialog0(msg, _("Keep the old one"), _("Keep the new one"));

		int choice = dialog0.runModal();
		if (choice == GUI::kMessageAlt) {
			// User chose to keep the new game, so delete the old one
			_saveFileMan->removeSavefile(oldFileName);
			return true;
		}
	}

	Common::InSaveFile *oldSave = _saveFileMan->openForLoading(oldFileName);
	if (!oldSave) {
		// Display a warning message and do nothing
		warning("Can't open file '%s'", oldFileName);
		return false;
	}

	// Read data from old type of save game
	dataSize = oldSave->size();
	saveData = new uint8[dataSize];
	oldSave->read(saveData, dataSize);
	delete oldSave;

	// Now write the save data to a new type of save game
	Common::OutSaveFile *newSave;
	newSave = _saveFileMan->openForSaving(newFileName);
	if (!newSave) {
		// Display a warning message and do nothing
		warning("Unable to create file '%s'. (%s)", newFileName, _saveFileMan->popErrorDesc().c_str());
		delete[] saveData;
		saveData = nullptr;
		return false;
	}

	newSave->writeUint32LE(SAVEGAME_HEADER);
	newSave->write(desc, 40);
	newSave->writeByte(SAVEGAME_VERSION);

	// Date / time
	TimeDate curTime;
	_system->getTimeAndDate(curTime);

	uint32 saveDate = (curTime.tm_mday & 0xFF) << 24 | ((curTime.tm_mon + 1) & 0xFF) << 16 | ((curTime.tm_year + 1900) & 0xFFFF);
	uint16 saveTime = (curTime.tm_hour & 0xFF) << 8 | ((curTime.tm_min) & 0xFF);

	newSave->writeUint32BE(saveDate);
	newSave->writeUint16BE(saveTime);
	newSave->writeUint32BE(0); // We don't have playtime info when converting, so we start from 0.

	newSave->write(saveData, dataSize);

	newSave->finalize();
	if (newSave->err())
		warning("Couldn't write to file '%s'. Device full?", newFileName);
	delete newSave;

	// Delete old save
	_saveFileMan->removeSavefile(oldFileName);

	// Cleanup
	delete[] saveData;
	saveData = nullptr;
	return true;
}

void Control::doRestore() {
	uint8 *bufPos = _restoreBuf;
	_objMan->loadLiveList((uint16 *)bufPos);
	bufPos += TOTAL_SECTIONS * 2;
	for (uint16 cnt = 0; cnt < NUM_SCRIPT_VARS; cnt++) {
		Logic::_scriptVars[cnt] = *(uint32 *)bufPos;
		bufPos += 4;
	}
	uint32 playerSize = (sizeof(Object) - 12000) / 4;
	uint32 *playerRaw = (uint32 *)_objMan->fetchObject(PLAYER);
	Object *cpt = _objMan->fetchObject(PLAYER);
	for (uint32 cnt2 = 0; cnt2 < playerSize; cnt2++) {
		*playerRaw = *(uint32 *)bufPos;
		playerRaw++;
		bufPos += 4;
	}
	free(_restoreBuf);
	Logic::_scriptVars[CHANGE_DIR] = cpt->o_dir;
	Logic::_scriptVars[CHANGE_X] = cpt->o_xcoord;
	Logic::_scriptVars[CHANGE_Y] = cpt->o_ycoord;
	Logic::_scriptVars[CHANGE_STANCE] = STAND;
	Logic::_scriptVars[CHANGE_PLACE] = cpt->o_place;
	SwordEngine::_systemVars.justRestoredGame = 1;
	if (SwordEngine::_systemVars.isDemo)
		Logic::_scriptVars[PLAYINGDEMO] = 1;
}

bool Control::gameVersionIsAkella() {
	if (SwordEngine::_systemVars.realLanguage == Common::RU_RUS) {
		uint32 resId = _resMan->getDeathFontId();
		return resId == SR_DEATHFONT;
	} else {
		return false;
	}
}

bool Control::gameVersionIsMediaHouse() {
	if (SwordEngine::_systemVars.realLanguage == Common::RU_RUS) {
		uint32 resId = _resMan->getDeathFontId();
		return resId == SR_FONT;
	} else {
		return false;
	}
}

bool Control::loadCustomStrings(const char *filename) {
	Common::File f;

	if (f.open(filename)) {
		Common::String line;

		for (int lineNo = 0; lineNo < 20; lineNo++) {
			line = f.readLine();

			if (f.eos())
				return false;

			memset((void*)_customStrings[lineNo], 0, 43);
			strncpy((char*)_customStrings[lineNo], line.c_str(), 42);
		}
		return true;
	}
	return false;
}

const uint8 *Control::getPauseString() {
	if (SwordEngine::isPsx()) {
		switch (SwordEngine::_systemVars.language) {
		case BS1_ENGLISH:
			return _psxPauseStrings[0];
		case BS1_GERMAN:
		case BS1_FRENCH:
			return _psxPauseStrings[1];
		case BS1_ITALIAN:
		case BS1_SPANISH:
			return _psxPauseStrings[2];
		default:
			return _psxPauseStrings[0];
		}
	}

	return _lStrings[STR_PAUSED];
}

const uint8 Control::_languageStrings[8 * 20][43] = {
	// BS1_ENGLISH:
	"PAUSED",
	"PLEASE INSERT CD-",
	"THEN PRESS A KEY",
	"INCORRECT CD",
	"Save",
	"Restore",
	"Restart",
	"Start",
	"Quit",
	"Speed",
	"Volume",
	"Text",
	"Done",
	"OK",
	"Cancel",
	"Music",
	"Speech",
	"Fx",
	"The End",
	"DRIVE FULL!",
// BS1_FRENCH:
	"PAUSE",
	"INS\xC9REZ LE CD-",
	"ET APPUYES SUR UNE TOUCHE",
	"CD INCORRECT",
	"Sauvegarder",
	"Recharger",
	"Recommencer",
	"Commencer",
	"Quitter",
	"Vitesse",
	"Volume",
	"Texte",
	"Termin\xE9",
	"OK",
	"Annuler",
	"Musique",
	"Voix",
	"Fx",
	"Fin",
	"DISQUE PLEIN!",
//BS1_GERMAN:
	"PAUSE",
	"BITTE LEGEN SIE CD-",
	("EIN UND DR\xDC""CKEN SIE EINE BELIEBIGE TASTE"),
	"FALSCHE CD",
	"Speichern",
	"Laden",
	"Neues Spiel",
	"Start",
	"Beenden",
	"Geschwindigkeit",
	"Lautst\xE4rke",
	"Text",
	"Fertig",
	"OK",
	"Abbrechen",
	"Musik",
	"Sprache",
	"Fx",
	"Ende",
	"DRIVE FULL!",
//BS1_ITALIAN:
	"PAUSA",
	"INSERITE IL CD-",
	"E PREMETE UN TASTO",
	"CD ERRATO",
	"Salva",
	"Ripristina",
	"Ricomincia",
	"Inizio",
	"Esci",
	"Velocit\xE0",
	"Volume",
	"Testo",
	"Fatto",
	"OK",
	"Annulla",
	"Musica",
	"Parlato",
	"Fx",
	"Fine",
	"DISCO PIENO!",
//BS1_SPANISH:
	"PAUSA",
	"POR FAVOR INTRODUCE EL CD-",
	"Y PULSA UNA TECLA",
	"CD INCORRECTO",
	"Guardar",
	"Recuperar",
	"Reiniciar",
	"Empezar",
	"Abandonar",
	"Velocidad",
	"Volumen",
	"Texto",
	"Hecho",
	"OK",
	"Cancelar",
	"M\xFAsica",
	"Di\xE1logo",
	"Fx",
	"Fin",
	"DISCO LLENO",
// BS1_CZECH:
	"\xAC\x41S SE ZASTAVIL",
	"VLO\xA6TE DO MECHANIKY CD DISK",
	"PAK STISKN\xB7TE LIBOVOLNOU KL\xB5VESU",
	"TO NEBUDE TO SPR\xB5VN\x90 CD",
	"Ulo\xA7it pozici",
	"Nahr\xA0t pozici",
	"Za\x9F\xA1t znovu",
	"Start",
	"Ukon\x9Fit hru",
	"Rychlost",
	"Hlasitost",
	"Titulky",
	"Souhlas\xA1m",
	"Ano",
	"Ne",
	"Hudba",
	"Mluven, slovo",
	"Zvuky",
	"Konec",
	"Disk pln\xEC",
//BS1_PORTUGESE:
	"PAUSA",
	"FAVOR INSERIR CD",
	"E DIGITAR UMA TECLA",
	"CD INCORRETO",
	"Salvar",
	"Restaurar",
	"Reiniciar",
	"Iniciar",
	"Sair",
	"Velocidade",
	"Volume",
	"Texto",
	"Feito",
	"OK",
	"Cancelar",
	"M\xFAsica",
	"Voz",
	"Efeitos",
	"Fim",
	"UNIDADE CHEIA!",
};

// Russian versions
const uint8 Control::_akellaLanguageStrings[20][43] = {
	"\x20\x20\x20\x20\x20\x20",                                         // "PAUSED",
	"\x93\x91\x92\x80\x8D\x8E\x82\x88\x92\x85\x20\x84\x88\x91\x8A\x2D", // "PLEASE INSERT CD-",
	"\x45\x4E\x54\x45\x52\x2D\xAF\xE0\xAE\xA4\xAE\xAB\xA6\xA8\xE2\xEC", // "THEN PRESS A KEY",
	"\x8D\x85\x82\x85\x90\x8D\x9B\x89\x20\x43\x44",                     // "INCORRECT CD",
	"\x40",                                                             // "Save",
	"\x7F",                                                             // "Restore",
	"\xD7",                                                             // "Restart",
	"\xD7",                                                             // "Start",
	"\xD8",                                                             // "Quit",
	"\xDD",                                                             // "Speed",
	"\xDE",                                                             // "Volume",
	"\xF7",                                                             // "Text",
	"\xFE",                                                             // "Done",
	"OK",
	"\x8E\xE2\xAC\xA5\xAD\xA0",                                         // "Cancel",
	"\x20\x20\x20\x20\x20",                                             // "Music",
	"\x20\x20\x20\x20\x20",                                             // "Speech",
	"\x20\x20",                                                         // "Fx",
	"\x8A\xAE\xAD\xA5\xE6",                                             // "The End",
	"DRIVE FULL!",
};

const uint8 Control::_mediaHouseLanguageStrings[20][43] = {
	"\x8F\x80\x93\x87\x80",                                                                         // "PAUSED",
	"\x82\x91\x92\x80\x82\x9C\x92\x85\x20\x43\x44\x2D",                                             // "PLEASE INSERT CD-",
	"\x88\x20\x8D\x80\x86\x8C\x88\x92\x85\x20\x8B\x9E\x81\x93\x9E\x20\x8A\x8B\x80\x82\x88\x98\x93", // "THEN PRESS A KEY",
	"\x8D\x85\x20\x92\x8E\x92\x20\x84\x88\x91\x8A",                                                 // "INCORRECT CD",
	"\x91\xAE\xE5\x70\xA0\xAD\xA8\xE2\xEC",                                                         // "Save",
	"\x87\xA0\xA3\x70\xE3\xA7\xA8\xE2\xEC",                                                         // "Restore",
	"\x90\xA5\xE1\xE2\xA0\x70\xE2",                                                                 // "Restart",
	"\x91\xE2\xA0\x70\xE2",                                                                         // "Start",
	"\x82\xEB\xE5\xAE\xA4",                                                                         // "Quit",
	"\x91\xAA\xAE\x70\xAE\xE1\xE2\xEC",                                                             // "Speed",
	"\x83\x70\xAE\xAC\xAA\xAE\xE1\xE2\xEC",                                                         // "Volume",
	"\x92\xA5\xAA\xE1\xE2",                                                                         // "Text",
	"\x82\xE1\xA5",                                                                                 // "Done",
	"\x8E\xAA",                                                                                     // "OK",
	"\x8E\xE2\xAC\xA5\xAD\xA0",                                                                     // "Cancel",
	"\x8C\xE3\xA7\xEB\xAA\xA0",                                                                     // "Music",
	"\x83\xAE\xAB\xAE\xE1",                                                                         // "Speech",
	"\x87\xA2\xE3\xAA",                                                                             // "Fx",
	"\x8A\xAE\xAD\xA5\xE6\x00\x45\x20\x43\x44\x2D\x00",                                             // "The End",
	"DRIVE FULL!",
};

const uint8 Control::_polishTranslationLanguageStrings[20][43] = {
	"GRA ZATRZYMANA",                              // "PAUSED",
	"PROSZ\xCA W\xA3O\xAFY\xC6 P\xA3YT\xCA CD-",   // "PLEASE INSERT CD-",
	"I NACISN\xA1\xC6 DOWOLNY KLAWISZ",            // "THEN PRESS A KEY",
	"NIEPRAWID\xA3OWA P\xA3YTA CD",                // "INCORRECT CD",
	"Zapisz",                                      // "Save",
	"Wczytaj",                                     // "Restore",
	"Nowa gra",                                    // "Restart",
	"Start",                                       // "Start",
	"Wyjd\xBC",                                    // "Quit",
	("Pr\xEA""dko\xB6\xE6"),                       // "Speed", the double pair of "" is to avoid escaping the d after \xEA
	"G\xB3o\xB6no\xB6\xE6",                        // "Volume",
	"Napisy",                                      // "Text",
	"Gotowe",                                      // "Done",
	"OK",                                          // "OK",
	"Anuluj",                                      // "Cancel",
	"Muzyka",                                      // "Music",
	"Mowa",	                                       // "Speech",
	"Efekty",                                      // "Fx",
	"Koniec",                                      // "The End",
	"DYSK PE\xA3NY!"                               // "DRIVE FULL!"
};

/* ---------- PSX CREDITS CODE ---------- */

int32 Control::getCreditsStringLength(uint8 *str, uint8 *font) {
	int32 width = 0;
	FrameHeader *f;

	while (*str) {
		f = (FrameHeader *)_resMan->fetchFrame(font, *str - 32);
		width += f->width;
		str++;

		if (*str)
			width += PSX_CREDITS_SPACING;
	}

	return width;
}

int32 Control::getCreditsFontHeight(uint8 *font) {
	FrameHeader *f;

	f = (FrameHeader *)_resMan->fetchFrame(font, 'A' - 32);
	return (f->height / 2);
}

void Control::createCreditsTextSprite(uint8 *data, int32 pitch, uint8 *str, uint8 *font) {
	uint16 x = 0;
	FrameHeader *f;
	uint8 *src, *dst;

	while (*str) {
		f = (FrameHeader *)_resMan->fetchFrame(font, *str - 32);

		src = (uint8 *)f + sizeof(FrameHeader);
		dst = data + x;

		for (int i = 0; i < f->height / 2; i++) {
			memcpy(dst, src, f->width);
			src += f->width;
			dst += pitch;
		}

		x += (f->width + PSX_CREDITS_SPACING);
		str++;
	}
}

void Control::renderCreditsTextSprite(uint8 *data, uint8 *screenBuf, int16 x, int16 y, int16 width, int16 height) {
	// Clip the coordinates like the PSX code would do
	y = (y + 1) & 0xFFFE;
	x -= 129;
	y -= (128 - 56);

	// Boundary checks
	if (x >= SCREEN_WIDTH || y >= SCREEN_FULL_DEPTH)
		return;

	if (x + width <= 0 || y + height <= 0)
		return;

	// Are there rows outside the screen?
	// Calculate how many doubled rows of the sprite are outside the screen on the top
	int16 skippedDoubledRows = (y < 0) ? -y : 0;
	int16 skippedRowsInData = skippedDoubledRows / 2;

	data += width * skippedRowsInData; // Adjust data pointer based on the number of skipped rows in the sprite
	height -= skippedDoubledRows;      // Adjust height based on the number of skipped doubled rows

	if (y < 0) {
		y = 0;
	}

	uint8 *dst = screenBuf + x + SCREEN_WIDTH * y;

	for (int i = 0; i < height; i += 2) { // Increment by 2 for the doubled height
		// Boundary check for y
		if (y + i >= SCREEN_FULL_DEPTH)
			break;

		// First horizontal line
		for (int j = 0; j < width; j++) {
			// Boundary checks for x
			if (x + j < 0)
				continue;

			if (x + j >= SCREEN_WIDTH)
				break;

			if (data[j])
				dst[j] = data[j];
		}

		dst += SCREEN_WIDTH;

		// Second horizontal line (duplicated)
		for (int j = 0; j < width; j++) {
			// Boundary checks for x
			if (x + j < 0)
				continue;

			if (x + j >= SCREEN_WIDTH)
				break;

			if (data[j])
				dst[j] = data[j];
		}

		dst += SCREEN_WIDTH; // Move to the next line
		data += width;       // Move to the next row of source sprite
	}
}

void Control::psxEndCredits() {
	int16 h;
	int16 nextCredit = PSX_NUM_CREDITS + 1;
	uint8 *creditLine = nullptr;
	uint8 *titleLine = nullptr;
	int32 *creditData = nullptr;
	Common::File creditsFile;

	int32 creditsFileSize = 0;
	int32 totalCreditsNum = 0;

	uint8 *creditSprite[PSX_NUM_CREDITS];
	uint8 *titleSprite[PSX_NUM_CREDITS];
	int16 creditWidth[PSX_NUM_CREDITS];
	int16 titleWidth[PSX_NUM_CREDITS];
	int16 creditsHeight[PSX_NUM_CREDITS] = {
		400, 440, 480, 520, 560, 600, 640,
		680, 720, 760, 800, 840, 880, 920
	};

	for (int i = 0; i < PSX_NUM_CREDITS; i++) {
		creditSprite[i] = nullptr;
		titleSprite[i] = nullptr;
		creditWidth[i] = 0;
		titleWidth[i] = 0;
	}

	// If we're here, the resource is already there, no need to open it
	uint8 *font = (uint8 *)_resMan->fetchRes(GAME_FONT);

	switch (SwordEngine::_systemVars.language) {
	case BS1_ENGLISH:
		totalCreditsNum = 101;
		creditsFileSize = 2798;
		break;
	case BS1_GERMAN:
		totalCreditsNum = 83;
		creditsFileSize = 2382;
		break;
	case BS1_FRENCH:
		totalCreditsNum = 83;
		creditsFileSize = 2382;
		break;
	case BS1_SPANISH:
		totalCreditsNum = 83;
		creditsFileSize = 2412;
		break;
	case BS1_ITALIAN:
		totalCreditsNum = 101;
		creditsFileSize = 2823;
		break;
	default:
		totalCreditsNum = 101;
		creditsFileSize = 2798;
	}

	_sound->clearAllFx();
	_screen->startFadePaletteUp(1);

	for (int i = 0; i < PSX_NUM_CREDITS; i++)
		creditsHeight[i] = 400 + i * 40;

	h = getCreditsFontHeight(font);
	_screen->fnSetFadeTargetPalette(193, 1, 0, TEXT_WHITE);

	_sound->streamMusicFile(101, 1);
	_sound->updateMusicStreaming();

	uint8 *creditsScreenBuf = (uint8 *)malloc(SCREEN_WIDTH * SCREEN_FULL_DEPTH);
	if (!creditsScreenBuf) {
		warning("Control::psxEndCredits(): Couldn't allocate memory for credits screen buffer");
		return;
	}

	memset(creditsScreenBuf, 0, SCREEN_WIDTH * SCREEN_FULL_DEPTH);

	creditData = (int32 *)malloc(creditsFileSize);
	if (!creditData) {
		warning("Control::psxEndCredits(): Couldn't allocate memory for text data");
		free(creditsScreenBuf);
		return;
	}

	if (!creditsFile.exists("CREDITS.DAT")) {
		debug(2, "Control::psxEndCredits(): Couldn't find CREDITS.DAT");
		free(creditsScreenBuf);
		free(creditData);
		return;
	}

	creditsFile.open("CREDITS.DAT");
	if (!creditsFile.isOpen()) {
		debug(2, "Control::psxEndCredits(): Couldn't open CREDITS.DAT");
		free(creditsScreenBuf);
		free(creditData);
		return;
	}

	creditsFile.read(creditData, creditsFileSize);
	creditsFile.close();

	bool allSet = true;

	for (int i = 0; i < PSX_NUM_CREDITS; i++) {
		_sound->updateMusicStreaming();
		_sound->setCrossFadeIncrement();

		creditLine = ((uint8 *)creditData + creditData[i + totalCreditsNum]);
		titleLine = ((uint8 *)creditData + creditData[i]);

		creditWidth[i] = (getCreditsStringLength(creditLine, font) + 1) & 0xFFFE;
		titleWidth[i] = (getCreditsStringLength(titleLine, font) + 1) & 0xFFFE;

		if (creditWidth[i]) {
			creditSprite[i] = (uint8 *)malloc(h * creditWidth[i]);
			if (!creditSprite[i]) {
				warning("Control::psxEndCredits(): Couldn't allocate memory for text sprites");
				allSet = false;
				break; // Break so the clean-up code is executed
			}

			memset(creditSprite[i], 0, h * creditWidth[i]);
		} else {
			creditSprite[i] = nullptr;
		}

		if (titleWidth[i]) {
			titleSprite[i] = (uint8 *)malloc(h * titleWidth[i]);
			if (!titleSprite[i]) {
				warning("Control::psxEndCredits(): Couldn't allocate memory for text sprites");
				allSet = false;
				break; // Break so the clean-up code is executed
			}

			memset(titleSprite[i], 0, h * titleWidth[i]);
		} else {
			titleSprite[i] = nullptr;
		}

		createCreditsTextSprite(creditSprite[i], creditWidth[i], creditLine, font);
		createCreditsTextSprite(titleSprite[i], titleWidth[i], titleLine, font);
	}

	_keyPressed.reset();
	_customType = kActionNone;

	while (allSet && creditsHeight[PSX_NUM_CREDITS - 1] > -120 &&
		!Engine::shouldQuit() &&
		_customType != kActionEscape) {
		memset(creditsScreenBuf, 0, SCREEN_WIDTH * SCREEN_FULL_DEPTH);

		for (int i = 0; i < PSX_NUM_CREDITS; i++) {
			// Name
			renderCreditsTextSprite(
				creditSprite[i],
				creditsScreenBuf,
				PSX_CREDITS_MIDDLE + Logic::_scriptVars[SCROLL_OFFSET_X],
				PSX_CREDITS_OFFSET + creditsHeight[i],
				creditWidth[i],
				h * 2);

			// Role
			renderCreditsTextSprite(
				titleSprite[i],
				creditsScreenBuf,
				PSX_CREDITS_MIDDLE + Logic::_scriptVars[SCROLL_OFFSET_X] - 30 - titleWidth[i],
				PSX_CREDITS_OFFSET + creditsHeight[i],
				titleWidth[i],
				h * 2);

			creditsHeight[i] -= 2;
		}

		_system->copyRectToScreen(creditsScreenBuf, SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_FULL_DEPTH);

		delay(33); // Run credits at about 30 FPS

		// Always remember to update sound :-)
		_sound->updateMusicStreaming();
		_sound->setCrossFadeIncrement();

		// Scroll the credits!
		if (creditsHeight[0] < -120) {
			if (nextCredit <= totalCreditsNum) {
				for (int i = 0; i < PSX_NUM_CREDITS; i++) {
					creditsHeight[i] += 40;
				}

				if (creditSprite[0] != nullptr)
					free(creditSprite[0]);

				if (titleSprite[0] != nullptr)
					free(titleSprite[0]);

				for (int i = 0; i < PSX_NUM_CREDITS - 1; i++) {
					creditSprite[i] = creditSprite[i + 1];
					titleSprite[i] = titleSprite[i + 1];
					creditWidth[i] = creditWidth[i + 1];
					titleWidth[i] = titleWidth[i + 1];
				}

				creditLine = ((uint8 *)creditData + creditData[nextCredit - 1 + totalCreditsNum]);
				titleLine = ((uint8 *)creditData + creditData[nextCredit - 1]);

				creditWidth[PSX_NUM_CREDITS - 1] = (getCreditsStringLength(creditLine, font) + 1) & 0xFFFE;
				titleWidth[PSX_NUM_CREDITS - 1] = (getCreditsStringLength(titleLine, font) + 1) & 0xFFFE;

				if (creditWidth[PSX_NUM_CREDITS - 1]) {
					creditSprite[PSX_NUM_CREDITS - 1] = (uint8 *)malloc(h * creditWidth[PSX_NUM_CREDITS - 1]);
					if (!creditSprite[PSX_NUM_CREDITS - 1]) {
						warning("Control::psxEndCredits(): Couldn't allocate memory for text sprites");
						break;
					}

					memset(creditSprite[PSX_NUM_CREDITS - 1], 0, h * creditWidth[PSX_NUM_CREDITS - 1]);
				} else {
					creditSprite[PSX_NUM_CREDITS - 1] = nullptr;
				}

				if (titleWidth[PSX_NUM_CREDITS - 1]) {
					titleSprite[PSX_NUM_CREDITS - 1] = (uint8 *)malloc(h * titleWidth[PSX_NUM_CREDITS - 1]);
					if (!titleSprite[PSX_NUM_CREDITS - 1]) {
						warning("Control::psxEndCredits(): Couldn't allocate memory for text sprites");
						break;
					}

					memset(titleSprite[PSX_NUM_CREDITS - 1], 0, h * titleWidth[PSX_NUM_CREDITS - 1]);
				} else {
					titleSprite[PSX_NUM_CREDITS - 1] = nullptr;
				}

				createCreditsTextSprite(creditSprite[PSX_NUM_CREDITS - 1], creditWidth[PSX_NUM_CREDITS - 1], creditLine, font);
				createCreditsTextSprite(titleSprite[PSX_NUM_CREDITS - 1], titleWidth[PSX_NUM_CREDITS - 1], titleLine, font);

				nextCredit += 1;
			}
		}
	}

	for (int i = 0; i < PSX_NUM_CREDITS; i++) {
		if (creditSprite[i] != nullptr)
			free(creditSprite[i]);

		if (titleSprite[i] != nullptr)
			free(titleSprite[i]);
	}

	free(creditData);

	_screen->startFadePaletteDown(1);
	_vm->waitForFade();

	memset(creditsScreenBuf, 0, SCREEN_WIDTH * SCREEN_FULL_DEPTH);
	_system->copyRectToScreen(creditsScreenBuf, SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_FULL_DEPTH);
	free(creditsScreenBuf);

	_keyPressed.reset();
	_customType = kActionNone;
}

} // End of namespace Sword1
