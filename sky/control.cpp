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

#include "sky/control.h"
#include "sky/skydefs.h"
#include "sky/sky.h"
#include "common/file.h"
#include "common/gameDetector.h"

SkyConResource::SkyConResource(void *pSpData, uint32 pNSprites, uint32 pCurSprite, uint16 pX, uint16 pY, uint32 pText, uint8 pOnClick, OSystem *system, uint8 *screen) {

	_spriteData = (dataFileHeader*)pSpData;
	_numSprites = pNSprites;
	_curSprite = pCurSprite;
	_x = pX;
	_y = pY;
	_text = pText;
	_onClick = pOnClick;
	_system = system;
	_screen = screen;
}

bool SkyConResource::isMouseOver(uint32 mouseX, uint32 mouseY) {

	if ((mouseX >= _x) && (mouseY >= _y) && ((uint16)mouseX <= _x + _spriteData->s_width) && ((uint16)mouseY <= _y + _spriteData->s_height))
		return true;
	else
		return false;
}

void SkyConResource::drawToScreen(bool doMask) {

	uint8 *screenPos = _y * GAME_SCREEN_WIDTH + _x + _screen;
	uint8 *updatePos = screenPos;

	if (!_spriteData) return;
	uint8 *spriteData = ((uint8*)_spriteData) + sizeof(dataFileHeader);
	spriteData += _spriteData->s_sp_size * _curSprite;
	if (doMask) {
		for (uint16 cnty = 0; cnty < _spriteData->s_height; cnty++) {
			for (uint16 cntx = 0; cntx < _spriteData->s_width; cntx++) {
				if (spriteData[cntx]) screenPos[cntx] = spriteData[cntx];
			}
			screenPos += GAME_SCREEN_WIDTH;
			spriteData += _spriteData->s_width;
		}
	} else {
		for (uint16 cnty = 0; cnty < _spriteData->s_height; cnty++) {
			memcpy(screenPos, spriteData, _spriteData->s_width);
			screenPos += GAME_SCREEN_WIDTH;
			spriteData += _spriteData->s_width;
		}
	}
	_system->copy_rect(updatePos, GAME_SCREEN_WIDTH, _x, _y, _spriteData->s_width, _spriteData->s_height);
}

SkyTextResource::SkyTextResource(void *pSpData, uint32 pNSprites, uint32 pCurSprite, uint16 pX, uint16 pY, uint32 pText, uint8 pOnClick, OSystem *system, uint8 *screen) :
	SkyConResource(pSpData, pNSprites, pCurSprite, pX, pY, pText, pOnClick, system, screen) {
		_oldScreen = (uint8*)malloc(PAN_CHAR_HEIGHT * 3 * PAN_LINE_WIDTH);
		_oldY = 0;
		_oldX = GAME_SCREEN_WIDTH;
}

SkyTextResource::~SkyTextResource(void) {
	free(_oldScreen);
}

void SkyTextResource::flushForRedraw(void) {
    
	if (_oldX < GAME_SCREEN_WIDTH) {
		uint16 cpWidth = (PAN_LINE_WIDTH > (GAME_SCREEN_WIDTH - _oldX))?(GAME_SCREEN_WIDTH - _oldX):(PAN_LINE_WIDTH);
		for (uint8 cnty = 0; cnty < PAN_CHAR_HEIGHT; cnty++)
			memcpy(_screen + (cnty + _oldY) * GAME_SCREEN_WIDTH + _oldX, _oldScreen + cnty * PAN_LINE_WIDTH, cpWidth);
	}
	_oldX = GAME_SCREEN_WIDTH;
}

void SkyTextResource::drawToScreen(bool doMask) {
	
	doMask = true;
	uint16 cnty, cntx, cpWidth, cpHeight;
	if ((_oldX == _x) && (_oldY == _y) && (_spriteData)) return;
	if (_oldX < GAME_SCREEN_WIDTH) {
		cpWidth = (PAN_LINE_WIDTH > (GAME_SCREEN_WIDTH - _oldX))?(GAME_SCREEN_WIDTH - _oldX):(PAN_LINE_WIDTH);
		if (_spriteData && (cpWidth > _spriteData->s_width))
			cpWidth = _spriteData->s_width;
		if (_spriteData)
			cpHeight = (_spriteData->s_height > (GAME_SCREEN_HEIGHT - _oldY))?(GAME_SCREEN_HEIGHT - _oldY):(_spriteData->s_height);
		else
			cpHeight = PAN_CHAR_HEIGHT;
		for (cnty = 0; cnty < cpHeight; cnty++)
			memcpy(_screen + (cnty + _oldY) * GAME_SCREEN_WIDTH + _oldX, _oldScreen + cnty * PAN_LINE_WIDTH, cpWidth);
		_system->copy_rect(_screen + _oldY * GAME_SCREEN_WIDTH + _oldX, GAME_SCREEN_WIDTH, _oldX, _oldY, cpWidth, PAN_CHAR_HEIGHT);
	}
	if (!_spriteData) {
		_oldX = GAME_SCREEN_WIDTH;
		return;
	}
	_oldX = _x;
	_oldY = _y;
	cpWidth = (PAN_LINE_WIDTH > (GAME_SCREEN_WIDTH - _x))?(GAME_SCREEN_WIDTH - _x):(PAN_LINE_WIDTH);
	if (_spriteData && (cpWidth > _spriteData->s_width))
		cpWidth = _spriteData->s_width;
	if (_spriteData)
		cpHeight = (_spriteData->s_height > (GAME_SCREEN_HEIGHT - _y))?(GAME_SCREEN_HEIGHT - _y):(_spriteData->s_height);
	else
		cpHeight = PAN_CHAR_HEIGHT;
	uint8 *screenPos = _screen + _y * GAME_SCREEN_WIDTH + _x;
	uint8 *copyDest = _oldScreen;
	uint8 *copySrc = ((uint8*)_spriteData) + sizeof(dataFileHeader);
	for (cnty = 0; cnty < cpHeight; cnty++) {
		memcpy(copyDest, screenPos, cpWidth);
		for (cntx = 0; cntx < PAN_LINE_WIDTH; cntx++)
			if (copySrc[cntx]) screenPos[cntx] = copySrc[cntx];
		copySrc += _spriteData->s_width;
		copyDest += PAN_LINE_WIDTH;
		screenPos += GAME_SCREEN_WIDTH;
	}
	_system->copy_rect(_screen + _y * GAME_SCREEN_WIDTH + _x, GAME_SCREEN_WIDTH, _x, _y, cpWidth, cpHeight);
}

SkyControl::SkyControl(SkyScreen *screen, SkyDisk *disk, SkyMouse *mouse, SkyText *text, SkyMusicBase *music, SkyLogic *logic, SkySound *sound, OSystem *system, const char *savePath) {

	_skyScreen = screen;
	_skyDisk = disk;
	_skyMouse = mouse;
	_skyText = text;
	_skyMusic = music;
	_skyLogic = logic;
	_skySound = sound;
	_system = system;
	_savePath = savePath;
	_memListRoot = NULL;
}

SkyConResource *SkyControl::createResource(void *pSpData, uint32 pNSprites, uint32 pCurSprite, int16 pX, int16 pY, uint32 pText, uint8 pOnClick, uint8 panelType) {

	if (pText) pText += 0x7000;
	if (panelType == MAINPANEL) {
		pX += MPNL_X;
		pY += MPNL_Y;
	} else {
		pX += SPNL_X;
		pY += SPNL_Y;
	}
	return new SkyConResource(pSpData, pNSprites, pCurSprite, pX, pY, pText, pOnClick, _system, _screenBuf);
}

void SkyControl::removePanel(void) {
	
	free(_screenBuf);
	free(_sprites.controlPanel);	free(_sprites.button);
	free(_sprites.buttonDown);		free(_sprites.savePanel);
	free(_sprites.yesNo);			free(_sprites.slide);
	free(_sprites.slide2);			free(_sprites.slode);
	free(_sprites.slode2);			free(_sprites.musicBodge);
	delete _controlPanel;			delete _exitButton;
	delete _slide;					delete _slide2;
	delete _slode;					delete _restorePanButton;
	delete _savePanButton;			delete _dosPanButton;
	delete _restartPanButton;		delete _fxPanButton;
	delete _musicPanButton;			delete _bodge;
	delete _yesNo;					delete _text;
}

void SkyControl::initPanel(void) {

	_screenBuf = (uint8*)malloc(GAME_SCREEN_WIDTH * FULL_SCREEN_HEIGHT);
	memset(_screenBuf, 0, GAME_SCREEN_WIDTH * FULL_SCREEN_HEIGHT);

	uint16 volY = (127 - _skyMusic->giveVolume()) / 4 + 59 - MPNL_Y; // volume slider's Y coordinate
	uint16 spdY = 12 - (SkyState::_systemVars.gameSpeed / SPEED_MULTIPLY);
	spdY += MPNL_Y + 93; // speed slider's initial position

	_sprites.controlPanel	= _skyDisk->loadFile(60500, NULL);
	_sprites.button			= _skyDisk->loadFile(60501, NULL);
	_sprites.buttonDown		= _skyDisk->loadFile(60502, NULL);
	_sprites.savePanel		= _skyDisk->loadFile(60503, NULL);
	_sprites.yesNo			= _skyDisk->loadFile(60504, NULL);
	_sprites.slide			= _skyDisk->loadFile(60505, NULL);
	_sprites.slode			= _skyDisk->loadFile(60506, NULL);
	_sprites.slode2			= _skyDisk->loadFile(60507, NULL);
	_sprites.slide2			= _skyDisk->loadFile(60508, NULL);
	if (SkyState::_systemVars.gameVersion < 368) 
		_sprites.musicBodge = NULL;
	else
		_sprites.musicBodge = _skyDisk->loadFile(60509, NULL);
    
	//Main control panel:                                            X    Y Text       OnClick
	_controlPanel     = createResource(_sprites.controlPanel, 1, 0,  0,   0,  0,      DO_NOTHING, MAINPANEL);
	_exitButton       = createResource(      _sprites.button, 3, 0, 16, 125, 50,            EXIT, MAINPANEL);
	_slide            = createResource(      _sprites.slide2, 1, 0, 19,spdY, 95,     SPEED_SLIDE, MAINPANEL);
	_slide2           = createResource(      _sprites.slide2, 1, 0, 19,volY, 14,     MUSIC_SLIDE, MAINPANEL);
	_slode            = createResource(      _sprites.slode2, 1, 0,  9,  49,  0,      DO_NOTHING, MAINPANEL);
	_restorePanButton = createResource(      _sprites.button, 3, 0, 58,  19, 51, REST_GAME_PANEL, MAINPANEL);
	_savePanButton    = createResource(      _sprites.button, 3, 0, 58,  39, 48, SAVE_GAME_PANEL, MAINPANEL);
	_dosPanButton     = createResource(      _sprites.button, 3, 0, 58,  59, 93,     QUIT_TO_DOS, MAINPANEL);
	_restartPanButton = createResource(      _sprites.button, 3, 0, 58,  79, 94,         RESTART, MAINPANEL);
	if (SkyState::_systemVars.systemFlags & SF_FX_OFF)
		_fxPanButton  = createResource(      _sprites.button, 3, 0, 58,  99, 86,       TOGGLE_FX, MAINPANEL);
	else
		_fxPanButton  = createResource(      _sprites.button, 3, 2, 58,  99, 87,       TOGGLE_FX, MAINPANEL);

	if (SkyState::isCDVersion()) { // CD Version: Toggle text/speech
	  _musicPanButton = createResource(      _sprites.button, 3, 0, 58, 119, 52,     TOGGLE_TEXT, MAINPANEL);
	} else {                       // disk version: toggle music on/off
	  _musicPanButton = createResource(      _sprites.button, 3, 0, 58, 119, 91,       TOGGLE_MS, MAINPANEL);
	}
	_bodge            = createResource(  _sprites.musicBodge, 2, 1, 98, 115,  0,      DO_NOTHING, MAINPANEL);
	_yesNo            = createResource(       _sprites.yesNo, 1, 0, -2,  40,  0,      DO_NOTHING, MAINPANEL);

	_text = new SkyTextResource(NULL, 1, 0, 15, 137, 0, DO_NOTHING, _system, _screenBuf);
	_controlPanLookList[0] = _exitButton;
	_controlPanLookList[1] = _restorePanButton;
	_controlPanLookList[2] = _savePanButton;
	_controlPanLookList[3] = _dosPanButton;
	_controlPanLookList[4] = _restartPanButton;
	_controlPanLookList[5] = _fxPanButton;
	_controlPanLookList[6] = _musicPanButton;
	_controlPanLookList[7] = _slide;
	_controlPanLookList[8] = _slide2;

	// save/restore panel
	_savePanel      = createResource( _sprites.savePanel, 1, 0,   0,   0,  0,      DO_NOTHING, SAVEPANEL);
	_saveButton     = createResource(    _sprites.button, 3, 0,  29, 129, 48,     SAVE_A_GAME, SAVEPANEL);
	_downFastButton = createResource(_sprites.buttonDown, 1, 0, 212, 104,  0, SHIFT_DOWN_FAST, SAVEPANEL);
	_downSlowButton = createResource(_sprites.buttonDown, 1, 0, 212, 114,  0, SHIFT_DOWN_SLOW, SAVEPANEL);
	_upFastButton   = createResource(_sprites.buttonDown, 1, 0, 212,  21,  0,   SHIFT_UP_FAST, SAVEPANEL);
	_upSlowButton   = createResource(_sprites.buttonDown, 1, 0, 212,  10,  0,   SHIFT_UP_SLOW, SAVEPANEL);
	_quitButton     = createResource(    _sprites.button, 3, 0,  72, 129, 49,       SP_CANCEL, SAVEPANEL);
	_restoreButton  = createResource(    _sprites.button, 3, 0,  29, 129, 51,  RESTORE_A_GAME, SAVEPANEL);
	
	_savePanLookList[0] = _saveButton;
	_restorePanLookList[0] = _restoreButton;
	_restorePanLookList[1] = _savePanLookList[1] = _downSlowButton;
	_restorePanLookList[2] = _savePanLookList[2] = _downFastButton;
	_restorePanLookList[3] = _savePanLookList[3] = _upFastButton;
	_restorePanLookList[4] = _savePanLookList[4] = _upSlowButton;
	_restorePanLookList[5] = _savePanLookList[5] = _quitButton;
}

void SkyControl::buttonControl(SkyConResource *pButton) {

	if (pButton == NULL) {
		if (_textSprite) free(_textSprite);
		_textSprite = NULL;
		_curButtonText = 0;
		_text->setSprite(NULL);
		return ;
	}
	if (_curButtonText != pButton->_text) {
		if (_textSprite) free(_textSprite);
		_textSprite = NULL;
		_curButtonText = pButton->_text;
		if (pButton->_text) {
			_skyText->getText(pButton->_text);
			displayText_t textRes;
			textRes = _skyText->displayText(NULL, false, PAN_LINE_WIDTH, 255);
			_textSprite = (dataFileHeader*)textRes.textData;
			_text->setSprite(_textSprite);
		} else _text->setSprite(NULL);
	}
	_text->setXY(_mouseX + 12, _mouseY - 16);
}

void SkyControl::drawTextCross(uint32 flags) {
	
	_bodge->drawToScreen(NO_MASK);
	if (!(flags & SF_ALLOW_SPEECH))
		drawCross(151, 124);
	if (!(flags & SF_ALLOW_TEXT))
		drawCross(173, 124);
}

void SkyControl::drawCross(uint16 x, uint16 y) {

	_text->flushForRedraw();
	uint8 *bufPos, *crossPos;
	bufPos = _screenBuf + y * GAME_SCREEN_WIDTH + x;
	crossPos = _crossImg;
	for (uint16 cnty = 0; cnty < CROSS_SZ_Y; cnty++) {
		for (uint16 cntx = 0; cntx < CROSS_SZ_X; cntx++)
			if (crossPos[cntx] != 0xFF)
				bufPos[cntx] = crossPos[cntx];
		bufPos += GAME_SCREEN_WIDTH;
		crossPos += CROSS_SZ_X;
	}
	bufPos = _screenBuf + y * GAME_SCREEN_WIDTH + x;
	_system->copy_rect(bufPos, GAME_SCREEN_WIDTH, x, y, CROSS_SZ_X, CROSS_SZ_Y);
	_text->drawToScreen(WITH_MASK);
}

void SkyControl::animClick(SkyConResource *pButton) {

	if (pButton->_curSprite != pButton->_numSprites -1) {
		pButton->_curSprite++;
		_text->flushForRedraw();
		pButton->drawToScreen(NO_MASK);
		_text->drawToScreen(WITH_MASK);
		_system->update_screen();
		delay(150);
		pButton->_curSprite--;
		_text->flushForRedraw();
		pButton->drawToScreen(NO_MASK);
		_text->drawToScreen(WITH_MASK);
		_system->update_screen();
	}
}

void SkyControl::drawMainPanel(void) {

	memset(_screenBuf, 0, GAME_SCREEN_WIDTH * FULL_SCREEN_HEIGHT);
	_system->copy_rect(_screenBuf, GAME_SCREEN_WIDTH, 0, 0, GAME_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
	_controlPanel->drawToScreen(NO_MASK);
	_exitButton->drawToScreen(NO_MASK);
	_savePanButton->drawToScreen(NO_MASK);
	_restorePanButton->drawToScreen(NO_MASK);
	_dosPanButton->drawToScreen(NO_MASK);
	_restartPanButton->drawToScreen(NO_MASK);
	_fxPanButton->drawToScreen(NO_MASK);
	_musicPanButton->drawToScreen(NO_MASK);
	_slode->drawToScreen(WITH_MASK);
	_slide->drawToScreen(WITH_MASK);
	_slide2->drawToScreen(WITH_MASK);
	_bodge->drawToScreen(WITH_MASK);
	if (SkyState::isCDVersion())
		drawTextCross(SkyState::_systemVars.systemFlags & TEXT_FLAG_MASK);
}

void SkyControl::restartGame(void) {
	if (SkyState::isDemo() && (!SkyState::isCDVersion()))
		return; // I don't think this can happen

	/*parseSaveData(_restartData);

	_skyScreen->forceRefresh();
	_skyScreen->setPalette((uint8*)SkyState::fetchCompact(SkyState::_systemVars.currentPalette));
	_skyMouse->spriteMouse(_savedMouse, 0, 0);
	SkyState::_systemVars.pastIntro = true;*/
}

void SkyControl::doLoadSavePanel(void) {
	if (SkyState::isDemo())
		return; // I don't think this can even happen
	initPanel();
	_skyScreen->clearScreen();
	if ((!SkyState::isCDVersion()) && (SkyState::_systemVars.gameVersion != 348)) 
		_skyScreen->setPalette(60509);
	else
		_skyScreen->setPalette(60510);

	_savedMouse = _skyMouse->giveCurrentMouseType();
	_skyMouse->spriteMouse(MOUSE_NORMAL,0,0);
	_lastButton = -1;
	_curButtonText = 0;
	_textSprite = NULL;

	saveRestorePanel(false);

	memset(_screenBuf, 0, GAME_SCREEN_WIDTH * FULL_SCREEN_HEIGHT);
	_system->copy_rect(_screenBuf, GAME_SCREEN_WIDTH, 0, 0, GAME_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
	_system->update_screen();
	_skyScreen->forceRefresh();
	_skyScreen->setPalette((uint8*)SkyState::fetchCompact(SkyState::_systemVars.currentPalette));
	removePanel();
	_skyMouse->spriteMouse(_savedMouse, 0, 0);
}

void SkyControl::doControlPanel(void) {

	if (SkyState::isDemo()) {
		return ;
	}
	initPanel();

	_skyScreen->clearScreen();
	if ((!SkyState::isCDVersion()) && (SkyState::_systemVars.gameVersion != 348)) 
		_skyScreen->setPalette(60509);
	else
		_skyScreen->setPalette(60510);
	
	drawMainPanel();

	_savedMouse = _skyMouse->giveCurrentMouseType();
	
	_skyMouse->spriteMouse(MOUSE_NORMAL,0,0);
	bool quitPanel = false;
	_lastButton = -1;
	_curButtonText = 0;
	_textSprite = NULL;
	uint16 clickRes = 0;

	while (!quitPanel) {
		_text->drawToScreen(WITH_MASK);
		_system->update_screen();
		_mouseClicked = false;
		delay(50);
		if (_keyPressed == 27) { // escape pressed
			_mouseClicked = false;
			quitPanel = true;
		}
		bool haveButton = false;
		for (uint8 lookCnt = 0; lookCnt < 9; lookCnt++) {
			if (_controlPanLookList[lookCnt]->isMouseOver(_mouseX, _mouseY)) {
				haveButton = true;
				buttonControl(_controlPanLookList[lookCnt]);
				if (_mouseClicked && _controlPanLookList[lookCnt]->_onClick) {
					clickRes = handleClick(_controlPanLookList[lookCnt]);
					_text->flushForRedraw();
					drawMainPanel();
					_text->drawToScreen(WITH_MASK);
					if ((clickRes == QUIT_PANEL) || (clickRes == GAME_SAVED) ||
						(clickRes == GAME_RESTORED))
						quitPanel = true;
				}
				_mouseClicked = false;
			}
		}
		if (!haveButton) buttonControl(NULL);

	}
	memset(_screenBuf, 0, GAME_SCREEN_WIDTH * FULL_SCREEN_HEIGHT);
	_system->copy_rect(_screenBuf, GAME_SCREEN_WIDTH, 0, 0, GAME_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
	_system->update_screen();
	_skyScreen->forceRefresh();
	_skyScreen->setPalette((uint8*)SkyState::fetchCompact(SkyState::_systemVars.currentPalette));
	removePanel();
	_skyMouse->spriteMouse(_savedMouse, 0, 0);
}

uint16 SkyControl::handleClick(SkyConResource *pButton) {

	char quitDos[] = "Quit to DOS?";

	switch(pButton->_onClick) {
		case DO_NOTHING:
			return 0;

		case REST_GAME_PANEL:
			if (SkyState::_systemVars.systemFlags & SF_CHOOSING)
				return CANCEL_PRESSED; // can't save/restore while choosing
			animClick(pButton);
			return saveRestorePanel(false); // texts can't be edited

		case SAVE_GAME_PANEL:
			if (SkyState::_systemVars.systemFlags & SF_CHOOSING)
				return CANCEL_PRESSED; // can't save/restore while choosing
			animClick(pButton);
			return saveRestorePanel(true); // texts can be edited

		case SAVE_A_GAME:
			animClick(pButton);
			return saveGameToFile();

		case RESTORE_A_GAME:
			animClick(pButton);
			return restoreGameFromFile();

		case SP_CANCEL:
			animClick(pButton);
			return CANCEL_PRESSED;

		case SHIFT_DOWN_FAST:
			animClick(pButton);
			return shiftDown(FAST);

		case SHIFT_DOWN_SLOW:
			animClick(pButton);
			return shiftDown(SLOW);

		case SHIFT_UP_FAST:
			animClick(pButton);
			return shiftUp(FAST);

		case SHIFT_UP_SLOW:
			animClick(pButton);
			return shiftUp(SLOW);

		case SPEED_SLIDE:
			_mouseClicked = true;
            return doSpeedSlide();

		case MUSIC_SLIDE:
			_mouseClicked = true;
			return doMusicSlide();

		case TOGGLE_FX:
			return toggleFx(pButton);

		case TOGGLE_MS:
			return 0;

		case TOGGLE_TEXT:
			animClick(pButton);
			return toggleText();

		case EXIT:
			animClick(pButton);
			return QUIT_PANEL;

		case RESTART:
			animClick(pButton);
			return 0;

		case QUIT_TO_DOS:
			animClick(pButton);
			if (getYesNo(quitDos)) {
				showGameQuitMsg(false);
				delay(1500);
				_system->quit();
			}
			return 0;

		default: 
			error("SkyControl::handleClick: unknown routine: %X\n",pButton->_onClick);
	}
}

bool SkyControl::getYesNo(char *text) {

	bool retVal = false;
	bool quitPanel = false;
	uint8 mouseType = MOUSE_NORMAL;
	uint8 wantMouse = MOUSE_NORMAL;
	dataFileHeader *dlgTextDat;
	uint16 textY = MPNL_Y;

	_yesNo->drawToScreen(WITH_MASK);
	if (text) {
		displayText_t dlgLtm = _skyText->displayText(text, NULL, true, _yesNo->_spriteData->s_width - 8, 37);
		dlgTextDat = (dataFileHeader*)dlgLtm.textData;
		textY = MPNL_Y + 44 + (28 - dlgTextDat->s_height) / 2;
	} else
		dlgTextDat = NULL;
	
	SkyTextResource *dlgText = new SkyTextResource(dlgTextDat, 1, 0, MPNL_X+2, textY, 0, DO_NOTHING, _system, _screenBuf);
	dlgText->drawToScreen(WITH_MASK);

	while (!quitPanel) {
		if (mouseType != wantMouse) {
			mouseType = wantMouse;
			_skyMouse->spriteMouse(mouseType, 0, 0);
		}
		_system->update_screen();
		delay(50);
		if ((_mouseY >= 83) && (_mouseY <= 110)) {
			if ((_mouseX >= 77) && (_mouseX <= 114)) { // over 'yes'
				wantMouse = MOUSE_CROSS;
				if (_mouseClicked) {
                    quitPanel = true;
                    retVal = true;
				}
			} else if ((_mouseX >= 156) && (_mouseX <= 193)) { // over 'no'
				wantMouse = MOUSE_CROSS;
				if (_mouseClicked) {
                    quitPanel = true;
                    retVal = false;
				}
			} else
				wantMouse = MOUSE_NORMAL;
		} else
			wantMouse = MOUSE_NORMAL;
	}
	_mouseClicked = false;
	if (dlgTextDat)
		free(dlgTextDat);
	delete dlgText;
	return retVal;
}

uint16 SkyControl::doMusicSlide(void) {

	int ofsY = _slide2->_y - _mouseY;
	uint8 volume;
	while (_mouseClicked) {
		delay(50);
		int newY = ofsY + _mouseY;
		if (newY < 59) newY = 59;
		if (newY > 91) newY = 91;
		if (newY != _slide2->_y) {
			_slode->drawToScreen(NO_MASK);
			_slide2->setXY(_slide2->_x, (uint16)newY);
			_slide2->drawToScreen(WITH_MASK);
			_slide->drawToScreen(WITH_MASK);
			volume = (newY - 59) * 4;
			if (volume >= 128) volume = 0;
			else volume = 127 - volume;
			_skyMusic->setVolume(volume);
		}
		buttonControl(_slide2);
		_text->drawToScreen(WITH_MASK);
		_system->update_screen();
	}
	return 0;
}

uint16 SkyControl::doSpeedSlide(void) {

	/*int ofsY = _slide->_y - _mouseY;
	uint16 speedDelay = 12 - (_slide->_y - (MPNL_Y + 93));
	speedDelay *= SPEED_MULTIPLY;
	while (_mouseClicked) {
		delay(50);
		int newY = ofsY + _mouseY;
		if (newY < MPNL_Y + 93) newY = MPNL_Y + 93;
		if (newY > MPNL_Y + 104) newY = MPNL_Y + 104;
		if (newY != _slide->_y) {
			_slode->drawToScreen(NO_MASK);
			_slide->setXY(_slide->_x, (uint16)newY);
			_slide->drawToScreen(WITH_MASK);
			_slide2->drawToScreen(WITH_MASK);
			speedDelay = 12 - (newY - (MPNL_Y + 93));
			speedDelay *= SPEED_MULTIPLY;
		}
		buttonControl(_slide);
		_text->drawToScreen(WITH_MASK);
		_system->update_screen();
	}
	SkyState::_systemVars.gameSpeed = speedDelay;
	printf("New delay: %d\n",speedDelay);*/
	return SPEED_CHANGED;
}

uint16 SkyControl::toggleFx(SkyConResource *pButton) {

	SkyState::_systemVars.systemFlags ^= SF_FX_OFF;
	if (SkyState::_systemVars.systemFlags & SF_FX_OFF) {
		pButton->_curSprite = 0;
		pButton->_text = 0x7000 + 86;
	} else {
		pButton->_curSprite = 2;
		pButton->_text = 0x7000 + 87;
	}
	pButton->drawToScreen(WITH_MASK);
	buttonControl(pButton);
	_system->update_screen();
	return TOGGLED;
}

uint16 SkyControl::toggleText(void) {

	uint32 flags = SkyState::_systemVars.systemFlags & TEXT_FLAG_MASK;
	SkyState::_systemVars.systemFlags &= ~TEXT_FLAG_MASK;


	if (flags == SF_ALLOW_TEXT)
		flags = SF_ALLOW_SPEECH;
	else if (flags == SF_ALLOW_SPEECH)
		flags = SF_ALLOW_SPEECH | SF_ALLOW_TEXT;
	else
		flags = SF_ALLOW_TEXT;

	SkyState::_systemVars.systemFlags |= flags;

	drawTextCross(flags);

	_system->update_screen();
	return TOGGLED;
}

uint16 SkyControl::shiftDown(uint8 speed) {

	if (speed == SLOW) {
		if (_firstText >= MAX_SAVE_GAMES - MAX_ON_SCREEN) return 0;
		_firstText++;
	} else {
		if (_firstText <= MAX_SAVE_GAMES - 2 * MAX_ON_SCREEN)
			_firstText += MAX_ON_SCREEN;
		else if (_firstText < MAX_SAVE_GAMES - MAX_ON_SCREEN)
			_firstText = MAX_SAVE_GAMES - MAX_ON_SCREEN;
		else return 0;
	}
	return SHIFTED;
}

uint16 SkyControl::shiftUp(uint8 speed) {

	if (speed == SLOW) {
		if (_firstText > 0) _firstText--;
		else return 0;
	} else {
		if (_firstText >= MAX_ON_SCREEN) _firstText -= MAX_ON_SCREEN;
		else if (_firstText > 0) _firstText = 0;
		else return 0;
	}
	return SHIFTED;
}

uint16 SkyControl::saveRestorePanel(bool allowSave) {

	_keyPressed = 0;
	buttonControl(NULL);
	_text->drawToScreen(WITH_MASK); // flush text restore buffer

	SkyConResource **lookList;
	uint16 cnt;
	if (allowSave) lookList = _savePanLookList;
	else lookList = _restorePanLookList;

	uint8 *saveGameTexts = (uint8*)malloc(MAX_SAVE_GAMES * MAX_TEXT_LEN);
	dataFileHeader *textSprites[MAX_ON_SCREEN + 1];
	textSprites[MAX_ON_SCREEN] = NULL;
	_firstText = 0;
	
	loadDescriptions(saveGameTexts);
	_selectedGame = 0;

	bool quitPanel = false;
	bool refreshNames = true;
	bool refreshAll = true;
	uint16 clickRes = 0;
	while (!quitPanel) {
		if (refreshNames || refreshAll) {
			if (refreshAll) {
				_savePanel->drawToScreen(NO_MASK);
				_quitButton->drawToScreen(NO_MASK);
			}
			setUpGameSprites(saveGameTexts, textSprites, _firstText, _selectedGame);
			showSprites(textSprites, allowSave);
			refreshNames = false;
		}

		_text->drawToScreen(WITH_MASK);
		_system->update_screen();
		_mouseClicked = false;
		delay(50);
		if (_keyPressed == 27) { // escape pressed
			_mouseClicked = false;
			clickRes = CANCEL_PRESSED;
			quitPanel = true;
		} else if ((_keyPressed == 13) || (_keyPressed == 15)) {
			clickRes = handleClick(lookList[0]);
			if (clickRes == GAME_SAVED)
				saveDescriptions(saveGameTexts);
			quitPanel = true;
			_mouseClicked = false;
			_keyPressed = 0;
		} if (allowSave && _keyPressed) {
			handleKeyPress(_keyPressed, _selectedGame * MAX_TEXT_LEN + saveGameTexts);
			refreshNames = true;
			_keyPressed = 0;
		}

		bool haveButton = false;
		for (cnt = 0; cnt < 6; cnt++)
			if (lookList[cnt]->isMouseOver(_mouseX, _mouseY)) {
				buttonControl(lookList[cnt]);
				haveButton = true;

				if (_mouseClicked && lookList[cnt]->_onClick) {
					_mouseClicked = false;
					
					clickRes = handleClick(lookList[cnt]);

			        if (clickRes == SHIFTED) {
						_selectedGame = _firstText;
						refreshNames = true;
					}
					if ((clickRes == CANCEL_PRESSED) || (clickRes == NO_DISK_SPACE) || 
						(clickRes == GAME_RESTORED))
						quitPanel = true;

					if (clickRes == GAME_SAVED) {
						saveDescriptions(saveGameTexts);
						quitPanel = true;
					}
					if (clickRes == RESTORE_FAILED)
 						refreshAll = true;
				}
			}

		if (_mouseClicked) {
			if ((_mouseX >= GAME_NAME_X) && (_mouseX <= GAME_NAME_X + PAN_LINE_WIDTH) &&
				(_mouseY >= GAME_NAME_Y) && (_mouseY <= GAME_NAME_Y + PAN_CHAR_HEIGHT * MAX_ON_SCREEN)) {

					_selectedGame = (_mouseY - GAME_NAME_Y) / PAN_CHAR_HEIGHT + _firstText;
					refreshNames = true;
			}
		}
		if (!haveButton) buttonControl(NULL);
	}

	for (cnt = 0; cnt < MAX_ON_SCREEN + 1; cnt++)
		free(textSprites[cnt]);

	free(saveGameTexts);

    return clickRes;
}

bool SkyControl::checkKeyList(uint8 key) {
	static const uint8 charList[14] = " ,().='-&+!?\"";
	for (uint chCnt = 0; chCnt < ARRAYSIZE(charList); chCnt++)
		if (charList[chCnt] == key) return true;
	return false;
}

void SkyControl::handleKeyPress(uint8 key, uint8 *textBuf) {

	if (key == 8) { // backspace
		for (uint8 cnt = 0; cnt < 6; cnt++)
			if (!textBuf[cnt]) return;

		while (textBuf[1])
			textBuf++;
		textBuf[0] = 0;
	} else {
		if (_enteredTextWidth >= PAN_LINE_WIDTH - 10)
			return;
        if (((key >= 'A') && (key <= 'Z')) || ((key >= 'a') && (key <= 'z')) ||
			((key >= '0') && (key <= '9')) || checkKeyList(key)) {
				uint8 strLen = 0;
				while (textBuf[0]) {
					textBuf++;
					strLen++;
				}
				if (strLen < MAX_TEXT_LEN) {
                    textBuf[0] = key;
					textBuf[1] = 0;
				}
		}
	}
}

void SkyControl::setUpGameSprites(uint8 *nameBuf, dataFileHeader **nameSprites, uint16 firstNum, uint16 selectedGame) {

	char cursorChar[2] = "-";
	nameBuf += firstNum * MAX_TEXT_LEN;
	displayText_t textSpr;
	if (!nameSprites[MAX_ON_SCREEN]) {
		textSpr = _skyText->displayText(cursorChar, NULL, false, 15, 0);
		nameSprites[MAX_ON_SCREEN] = (dataFileHeader*)textSpr.textData;
	}
	for (uint16 cnt = 0; cnt < MAX_ON_SCREEN; cnt++) {
		if (firstNum + cnt == selectedGame)
			textSpr = _skyText->displayText((char*)nameBuf, NULL, false, PAN_LINE_WIDTH, 0);
		else
			textSpr = _skyText->displayText((char*)nameBuf, NULL, false, PAN_LINE_WIDTH, 37);
		nameBuf += MAX_TEXT_LEN;
		nameSprites[cnt] = (dataFileHeader*)textSpr.textData;
		if (firstNum + cnt == selectedGame) {
			nameSprites[cnt]->flag = 1;
			_enteredTextWidth = (uint16)textSpr.textWidth;
		} else
			nameSprites[cnt]->flag = 0;
	}
}

void SkyControl::showSprites(dataFileHeader **nameSprites, bool allowSave) {

	SkyConResource *drawResource = new SkyConResource(NULL, 1, 0, 0, 0, 0, 0, _system, _screenBuf);
	for (uint16 cnt = 0; cnt < MAX_ON_SCREEN; cnt++) {
		drawResource->setSprite(nameSprites[cnt]);
		drawResource->setXY(GAME_NAME_X, GAME_NAME_Y + cnt * PAN_CHAR_HEIGHT);
		if (nameSprites[cnt]->flag) { // name is highlighted
			for (uint16 cnty = GAME_NAME_Y + cnt * PAN_CHAR_HEIGHT; cnty < GAME_NAME_Y + (cnt + 1) * PAN_CHAR_HEIGHT - 1; cnty++)
				memset(_screenBuf + cnty * GAME_SCREEN_WIDTH + GAME_NAME_X, 37, PAN_LINE_WIDTH);
			drawResource->drawToScreen(WITH_MASK);
			if (allowSave) {
				drawResource->setSprite(nameSprites[MAX_ON_SCREEN]);
				drawResource->setXY(GAME_NAME_X + _enteredTextWidth + 1, GAME_NAME_Y + cnt * PAN_CHAR_HEIGHT + 4);
				drawResource->drawToScreen(WITH_MASK);
			}
			_system->copy_rect(_screenBuf + (GAME_NAME_Y + cnt * PAN_CHAR_HEIGHT) * GAME_SCREEN_WIDTH + GAME_NAME_X, GAME_SCREEN_WIDTH, GAME_NAME_X, GAME_NAME_Y + cnt * PAN_CHAR_HEIGHT, PAN_LINE_WIDTH, PAN_CHAR_HEIGHT);
		} else 
			drawResource->drawToScreen(NO_MASK);
	}
	delete drawResource;
}

void SkyControl::loadDescriptions(uint8 *destBuf) {

	memset(destBuf, 0, MAX_SAVE_GAMES * MAX_TEXT_LEN);

	File *inf = new File();
	inf->open("SKY-VM.SAV",_savePath);
	if (inf->isOpen()) {
		uint8 *tmpBuf = (uint8*)malloc(inf->size());
		inf->read(tmpBuf, inf->size());
		uint8 *destPos = destBuf;
		uint8 *inPos = tmpBuf;
		for (uint16 cnt = 0; cnt < MAX_SAVE_GAMES; cnt++) {
			sprintf((char*)destPos,"%3d: ", cnt + 1);
			uint8 nameCnt = 0;
			while ((destPos[nameCnt + 5] = inPos[nameCnt]))
				nameCnt++;
			destPos += MAX_TEXT_LEN;
			inPos += nameCnt + 1;
		}
		free(tmpBuf);
		inf->close();
	} else {
		uint8 *destPos = destBuf;
		for (uint16 cnt = 0; cnt < MAX_SAVE_GAMES; cnt++) {
			sprintf((char*)destPos,"%3d: ", cnt + 1);
			destPos += MAX_TEXT_LEN;
		}
	}
}

void SkyControl::saveDescriptions(uint8 *srcBuf) {

	uint8 *tmpBuf = (uint8*)malloc(MAX_SAVE_GAMES * MAX_TEXT_LEN);
	uint8 *tmpPos = tmpBuf;
	uint8 *srcPos = srcBuf;
	for (uint16 cnt = 0; cnt < MAX_SAVE_GAMES; cnt++) {
		uint8 namePos = 5;
		while (srcPos[namePos]) {
			if (srcPos[namePos] != '_') {
				*tmpPos = srcPos[namePos];
				tmpPos++;
			}
			namePos++;
		}
		*tmpPos = 0;
		tmpPos++;
		srcPos += MAX_TEXT_LEN;
	}
	File *outf = new File();
	outf->open("SKY-VM.SAV", _savePath, File::kFileWriteMode);
	outf->write(tmpBuf, tmpPos - tmpBuf);
	outf->close();
	free(tmpBuf);	
}

uint16 SkyControl::saveGameToFile(void) {

	char fName[20];
	sprintf(fName,"SKY-VM.%03d", _selectedGame);
	File *outf = new File();
	if (!outf->open(fName, _savePath, File::kFileWriteMode)) {
		delete outf;
		return NO_DISK_SPACE;
	}

	uint8 *saveData = (uint8*)malloc(0x50000);
	uint32 fSize = prepareSaveData(saveData);

	if (outf->write(saveData, fSize) != fSize) {
		free(saveData);
		delete outf;
		return NO_DISK_SPACE;
	}
	outf->close();
	delete outf;
	free(saveData);
	return GAME_SAVED;
}

#define STOSD(ptr, val) { *(uint32*)(ptr) = TO_LE_32(val); (ptr) += 4; }
#define STOSW(ptr, val) { *(uint16*)(ptr) = TO_LE_16(val); (ptr) += 2; }

void SkyControl::stosMegaSet(uint8 **destPos, MegaSet *mega) {
	STOSW(*destPos, mega->gridWidth);
	STOSW(*destPos, mega->colOffset);
	STOSW(*destPos, mega->colWidth);
	STOSW(*destPos, mega->lastChr);
	// anims, stands, turnTable
}

void SkyControl::stosStr(uint8 **destPos, uint16 *src, bool isGraf) {
	uint16 strLen = 0;
	if (isGraf) {
		while (src[strLen] || src[strLen+2])
			strLen++;
		strLen += 3;
	} else {
		while (src[strLen])
			strLen++;
		strLen++;
	}
	STOSW(*destPos, strLen);
	for (uint16 cnt = 0; cnt < strLen; cnt++) {
		STOSW(*destPos, src[cnt]);
	}
}

void SkyControl::stosCompact(uint8 **destPos, Compact *cpt) {
	uint16 saveType = 0;
	if (cpt->extCompact) {
		saveType |= SAVE_EXT;
		if (cpt->extCompact->megaSet0) saveType |= SAVE_MEGA0;
		if (cpt->extCompact->megaSet1) saveType |= SAVE_MEGA1;
		if (cpt->extCompact->megaSet2) saveType |= SAVE_MEGA2;
		if (cpt->extCompact->megaSet3) saveType |= SAVE_MEGA3;
		if (cpt->extCompact->turnProg) saveType |= SAVE_TURNP;
	}
	if (cpt->grafixProg) saveType |= SAVE_GRAFX;

	STOSW(*destPos, saveType);

	if (saveType & SAVE_GRAFX)
		stosStr(destPos, cpt->grafixProg, true);
	if (saveType & SAVE_TURNP)
		stosStr(destPos, cpt->extCompact->turnProg, false);

	STOSW(*destPos, cpt->logic);
	STOSW(*destPos, cpt->status);
	STOSW(*destPos, cpt->sync);
	STOSW(*destPos, cpt->screen);
	STOSW(*destPos, cpt->place);
	// getToTable
	STOSW(*destPos, cpt->xcood);
	STOSW(*destPos, cpt->ycood);
	STOSW(*destPos, cpt->frame);
	STOSW(*destPos, cpt->cursorText);
	STOSW(*destPos, cpt->mouseOn);
	STOSW(*destPos, cpt->mouseOff);
	STOSW(*destPos, cpt->mouseClick);
	STOSW(*destPos, cpt->mouseRelX);
	STOSW(*destPos, cpt->mouseRelY);
	STOSW(*destPos, cpt->mouseSizeX);
	STOSW(*destPos, cpt->mouseSizeY);
	STOSW(*destPos, cpt->actionScript);
	STOSW(*destPos, cpt->upFlag);
	STOSW(*destPos, cpt->downFlag);
	STOSW(*destPos, cpt->getToFlag);
	STOSW(*destPos, cpt->flag);
	STOSW(*destPos, cpt->mood);
	// grafixProg
	STOSW(*destPos, cpt->offset);
	STOSW(*destPos, cpt->mode);
	STOSW(*destPos, cpt->baseSub);
	STOSW(*destPos, cpt->baseSub_off);
	if (cpt->extCompact) {
		STOSW(*destPos, cpt->extCompact->actionSub);
		STOSW(*destPos, cpt->extCompact->actionSub_off);
		STOSW(*destPos, cpt->extCompact->getToSub);
		STOSW(*destPos, cpt->extCompact->getToSub_off);
		STOSW(*destPos, cpt->extCompact->extraSub);
		STOSW(*destPos, cpt->extCompact->extraSub_off);
		STOSW(*destPos, cpt->extCompact->dir);
		STOSW(*destPos, cpt->extCompact->stopScript);
		STOSW(*destPos, cpt->extCompact->miniBump);
		STOSW(*destPos, cpt->extCompact->leaving);
		STOSW(*destPos, cpt->extCompact->atWatch);
		STOSW(*destPos, cpt->extCompact->atWas);
		STOSW(*destPos, cpt->extCompact->alt);
		STOSW(*destPos, cpt->extCompact->request);
		STOSW(*destPos, cpt->extCompact->spWidth_xx);
		STOSW(*destPos, cpt->extCompact->spColour);
		STOSW(*destPos, cpt->extCompact->spTextId);
		STOSW(*destPos, cpt->extCompact->spTime);
		STOSW(*destPos, cpt->extCompact->arAnimIndex);
		// turnProg
		STOSW(*destPos, cpt->extCompact->waitingFor);
		STOSW(*destPos, cpt->extCompact->arTargetX);
		STOSW(*destPos, cpt->extCompact->arTargetY);
		// animScratch
		STOSW(*destPos, cpt->extCompact->megaSet);

		if (cpt->extCompact->megaSet0)
			stosMegaSet(destPos, cpt->extCompact->megaSet0);
		if (cpt->extCompact->megaSet1)
			stosMegaSet(destPos, cpt->extCompact->megaSet1);
		if (cpt->extCompact->megaSet2)
			stosMegaSet(destPos, cpt->extCompact->megaSet2);
		if (cpt->extCompact->megaSet3)
			stosMegaSet(destPos, cpt->extCompact->megaSet3);
	}
}

uint32 SkyControl::prepareSaveData(uint8 *destBuf) {

	uint32 cnt;
	memset(destBuf, 0, 4); // space for data size
	uint8 *destPos = destBuf + 4;
	STOSD(destPos, SAVE_FILE_REVISION);

	STOSD(destPos, SkyState::_systemVars.gameVersion);
	STOSW(destPos, _skySound->_saveSounds[0]);
	STOSW(destPos, _skySound->_saveSounds[1]);

    STOSD(destPos, _skyMusic->giveCurrentMusic());
	STOSD(destPos, _skyText->giveCurrentCharSet());
	STOSD(destPos, _savedMouse);
	STOSD(destPos, SkyState::_systemVars.currentPalette);
	for (cnt = 0; cnt < 838; cnt++)
		STOSD(destPos, SkyLogic::_scriptVariables[cnt]);
	uint32 *loadedFilesList = _skyDisk->giveLoadedFilesList();

	for (cnt = 0; cnt < 60; cnt++)
		STOSD(destPos, loadedFilesList[cnt]);

	for (cnt = 0; cnt < ARRAYSIZE(_saveLoadCpts); cnt++)
		stosCompact(&destPos, _saveLoadCpts[cnt]);

	for (cnt = 0; cnt < 3; cnt++)
		STOSW(destPos, SkyCompact::park_table[cnt]);

	for (cnt = 0; cnt < 13; cnt++)
		STOSW(destPos, SkyCompact::high_floor_table[cnt]);

	*(uint32*)destBuf = TO_LE_32(destPos - destBuf); // save size
	return destPos - destBuf;
}

#undef STOSD
#undef STOSW

void SkyControl::appendMemList(uint16 *pMem) {
	AllocedMem *newMem = new AllocedMem;
	newMem->mem = pMem;
	newMem->next = _memListRoot;
	_memListRoot = newMem;
}

void SkyControl::freeMemList(void) {
	AllocedMem *block = _memListRoot;
	AllocedMem *temp;
	while (block) {
		temp = block;
		free(block->mem);
		block = block->next;
		delete temp;
	}
	_memListRoot = NULL;
}


#define LODSD(strPtr, val) { val = READ_LE_UINT32(strPtr); (strPtr) += 4; }
#define LODSW(strPtr, val) { val = READ_LE_UINT16(strPtr); (strPtr) += 2; }

void SkyControl::lodsMegaSet(uint8 **srcPos, MegaSet *mega) {
	LODSW(*srcPos, mega->gridWidth);
	LODSW(*srcPos, mega->colOffset);
	LODSW(*srcPos, mega->colWidth);
	LODSW(*srcPos, mega->lastChr);
	// anims, stands, turnTable
}

void SkyControl::lodsCompact(uint8 **srcPos, Compact *cpt) {

	uint16 saveType, cnt;
	LODSW(*srcPos, saveType);
	if ((saveType & (SAVE_EXT | SAVE_TURNP)) && (cpt->extCompact == NULL))
		error("Can't restore! SaveData is SAVE_EXT for Compact");
	if ((saveType & SAVE_MEGA0) && (cpt->extCompact->megaSet0 == NULL))
		error("Can't restore! SaveData is SAVE_MEGA0 for Compact");
	if ((saveType & SAVE_MEGA1) && (cpt->extCompact->megaSet1 == NULL))
		error("Can't restore! SaveData is SAVE_MEGA1 for Compact");
	if ((saveType & SAVE_MEGA2) && (cpt->extCompact->megaSet2 == NULL))
		error("Can't restore! SaveData is SAVE_MEGA2 for Compact");
	if ((saveType & SAVE_MEGA3) && (cpt->extCompact->megaSet3 == NULL))
		error("Can't restore! SaveData is SAVE_MEGA3 for Compact");

	if (saveType & SAVE_GRAFX) {
		uint16 grafxLen;
		LODSW(*srcPos, grafxLen);
		cpt->grafixProg = (uint16*)malloc(grafxLen << 1);
		appendMemList(cpt->grafixProg);
		for (cnt = 0; cnt < grafxLen; cnt++) {
			LODSW(*srcPos, cpt->grafixProg[cnt]);
		}
	} else
		cpt->grafixProg = NULL;

	if (saveType & SAVE_TURNP) {
		uint16 turnLen;
		LODSW(*srcPos, turnLen);
		cpt->extCompact->turnProg = (uint16*)malloc(turnLen << 1);
		appendMemList(cpt->extCompact->turnProg);
		for (cnt = 0; cnt < turnLen; cnt++)
			LODSW(*srcPos, cpt->extCompact->turnProg[cnt]);
	} else if (cpt->extCompact)
		cpt->extCompact->turnProg = NULL;

	LODSW(*srcPos, cpt->logic);
	LODSW(*srcPos, cpt->status);
	LODSW(*srcPos, cpt->sync);
	LODSW(*srcPos, cpt->screen);
	LODSW(*srcPos, cpt->place);
	// getToTable
	LODSW(*srcPos, cpt->xcood);
	LODSW(*srcPos, cpt->ycood);
	LODSW(*srcPos, cpt->frame);
	LODSW(*srcPos, cpt->cursorText);
	LODSW(*srcPos, cpt->mouseOn);
	LODSW(*srcPos, cpt->mouseOff);
	LODSW(*srcPos, cpt->mouseClick);
	LODSW(*srcPos, cpt->mouseRelX);
	LODSW(*srcPos, cpt->mouseRelY);
	LODSW(*srcPos, cpt->mouseSizeX);
	LODSW(*srcPos, cpt->mouseSizeY);
	LODSW(*srcPos, cpt->actionScript);
	LODSW(*srcPos, cpt->upFlag);
	LODSW(*srcPos, cpt->downFlag);
	LODSW(*srcPos, cpt->getToFlag);
	LODSW(*srcPos, cpt->flag);
	LODSW(*srcPos, cpt->mood);
	// grafixProg
	LODSW(*srcPos, cpt->offset);
	LODSW(*srcPos, cpt->mode);
	LODSW(*srcPos, cpt->baseSub);
	LODSW(*srcPos, cpt->baseSub_off);
	if (saveType & SAVE_EXT) {
		LODSW(*srcPos, cpt->extCompact->actionSub);
		LODSW(*srcPos, cpt->extCompact->actionSub_off);
		LODSW(*srcPos, cpt->extCompact->getToSub);
		LODSW(*srcPos, cpt->extCompact->getToSub_off);
		LODSW(*srcPos, cpt->extCompact->extraSub);
		LODSW(*srcPos, cpt->extCompact->extraSub_off);
		LODSW(*srcPos, cpt->extCompact->dir);
		LODSW(*srcPos, cpt->extCompact->stopScript);
		LODSW(*srcPos, cpt->extCompact->miniBump);
		LODSW(*srcPos, cpt->extCompact->leaving);
		LODSW(*srcPos, cpt->extCompact->atWatch);
		LODSW(*srcPos, cpt->extCompact->atWas);
		LODSW(*srcPos, cpt->extCompact->alt);
		LODSW(*srcPos, cpt->extCompact->request);
		LODSW(*srcPos, cpt->extCompact->spWidth_xx);
		LODSW(*srcPos, cpt->extCompact->spColour);
		LODSW(*srcPos, cpt->extCompact->spTextId);
		LODSW(*srcPos, cpt->extCompact->spTime);
		LODSW(*srcPos, cpt->extCompact->arAnimIndex);
		// turnProg
		LODSW(*srcPos, cpt->extCompact->waitingFor);
		LODSW(*srcPos, cpt->extCompact->arTargetX);
		LODSW(*srcPos, cpt->extCompact->arTargetY);
		// animScratch
		LODSW(*srcPos, cpt->extCompact->megaSet);

		if (saveType & SAVE_MEGA0)
			lodsMegaSet(srcPos, cpt->extCompact->megaSet0);
		if (saveType & SAVE_MEGA1)
			lodsMegaSet(srcPos, cpt->extCompact->megaSet1);
		if (saveType & SAVE_MEGA2)
			lodsMegaSet(srcPos, cpt->extCompact->megaSet2);
		if (saveType & SAVE_MEGA3)
			lodsMegaSet(srcPos, cpt->extCompact->megaSet3);
	}
}

uint16 SkyControl::parseSaveData(uint8 *srcBuf) {

	char loadText[] = "Savegame has an old version. Loading it can lead to errors.";
	uint32 reloadList[60];
	uint32 oldSection = SkyLogic::_scriptVariables[CUR_SECTION];
	
	uint32 cnt;
	uint8 *srcPos = srcBuf;
	uint32 size;
	uint32 saveRev;

	LODSD(srcPos, size);
	LODSD(srcPos, saveRev);
	if (saveRev > SAVE_FILE_REVISION) {
		warning("Unknown save file revision (%d)",saveRev);
		return RESTORE_FAILED;
	}

	uint32 music, charSet, mouseType, palette, gameVersion;
	
	if (saveRev >= 3) {
		LODSD(srcPos, gameVersion);
		if (gameVersion != SkyState::_systemVars.gameVersion) {
			printf("This savegame was created by Beneath a Steel Sky v0.0%03d\n", gameVersion);
			printf("It cannot be loaded by this version (v0.0%3d)\n", SkyState::_systemVars.gameVersion);
			return RESTORE_FAILED;
		}
		LODSW(srcPos, _skySound->_saveSounds[0]);
		LODSW(srcPos, _skySound->_saveSounds[1]);
		_skySound->restoreSfx();
	} else {
		if(!getYesNo(loadText))
			return RESTORE_FAILED;
	}
	freeMemList(); // memory from last restore isn't needed anymore
	LODSD(srcPos, music);
	LODSD(srcPos, charSet);
	LODSD(srcPos, mouseType);
	LODSD(srcPos, palette);

	for (cnt = 0; cnt < 838; cnt++)
		LODSD(srcPos, SkyLogic::_scriptVariables[cnt]);

	for (cnt = 0; cnt < 60; cnt++)
		LODSD(srcPos, reloadList[cnt]);

	for (cnt = 0; cnt < ARRAYSIZE(_saveLoadCpts); cnt++)
		lodsCompact(&srcPos, _saveLoadCpts[cnt]);

	for (cnt = 0; cnt < 3; cnt++)
		LODSW(srcPos, SkyCompact::park_table[cnt]);

	for (cnt = 0; cnt < 13; cnt++)
		LODSW(srcPos, SkyCompact::high_floor_table[cnt]);

	if (srcPos - srcBuf != (int32)size)
		error("Restore failed! Savegame data = %d bytes. Expected size: %d.\n", srcPos-srcBuf, size);

	_skyLogic->fnLeaveSection(oldSection, 0, 0);
	_skyLogic->fnEnterSection(SkyLogic::_scriptVariables[CUR_SECTION], 0, 0);
	_skyDisk->refreshFilesList(reloadList);
	_skyMusic->startMusic((uint16)music);
	_skyText->fnSetFont(charSet);
	_savedMouse = (uint16)mouseType;
	SkyState::_systemVars.currentPalette = palette; // will be set when doControlPanel ends
	SkyState::_systemVars.systemFlags |= SF_GAME_RESTORED; // what's that for?

	return GAME_RESTORED;
}

#undef LODSD
#undef LODSW

uint16 SkyControl::restoreGameFromFile(void) {
	
	char fName[20];
	sprintf(fName,"SKY-VM.%03d", _selectedGame);
	File *inf = new File();
	if (!inf->open(fName, _savePath)) {
		delete inf;
		return RESTORE_FAILED;
	}

	uint32 fSize = inf->size();
	uint8 *saveData = (uint8*)malloc(fSize);
	uint32 infSize = inf->readUint32LE();
	inf->seek(0, SEEK_SET);

	if (fSize != infSize) {
		warning("File size doesn't match expected data size!");
		delete inf;
		free(saveData);
		return RESTORE_FAILED;
	}
	if (inf->read(saveData, fSize) != fSize) {
		warning("Can't read from file!");
		delete inf;
		free(saveData);
		return RESTORE_FAILED;
	}

	uint16 res = parseSaveData(saveData);
	inf->close();
	delete inf;
	free(saveData);
	return res;
}

void SkyControl::delay(unsigned int amount) {

	OSystem::Event event;

	uint32 start = _system->get_msecs();
	uint32 cur = start;
	_keyPressed = 0;	//reset

	do {
		while (_system->poll_event(&event)) {
			switch (event.event_code) {
				case OSystem::EVENT_KEYDOWN:
					// Make sure backspace works right (this fixes a small issue on OS X)
					if (event.kbd.keycode == 8)
						_keyPressed = 8;
					else
						_keyPressed = (byte)event.kbd.ascii;
					break;

				case OSystem::EVENT_MOUSEMOVE:
					_mouseX = event.mouse.x;
					_mouseY = event.mouse.y;
					_system->set_mouse_pos(_mouseX, _mouseY);
					break;

				case OSystem::EVENT_LBUTTONDOWN:
					_mouseClicked = true;
#ifdef _WIN32_WCE
					_mouseX = event.mouse.x;
					_mouseY = event.mouse.y;
#endif
					break;

				case OSystem::EVENT_LBUTTONUP:
                    _mouseClicked = false;
					break;

				case OSystem::EVENT_RBUTTONDOWN:
					break;

				case OSystem::EVENT_QUIT:
					showGameQuitMsg(false);
					delay(1500);
					_system->quit();
					break;

				default:
					break;
			}
		}

		uint this_delay = 20; // 1?
		if (this_delay > amount)
			this_delay = amount;

		if (this_delay > 0)	_system->delay_msecs(this_delay);

		cur = _system->get_msecs();
	} while (cur < start + amount);
}

void SkyControl::showGameQuitMsg(bool useScreen) {

	uint8 *textBuf1 = (uint8*)malloc(GAME_SCREEN_WIDTH * 14 + sizeof(dataFileHeader));
	uint8 *textBuf2 = (uint8*)malloc(GAME_SCREEN_WIDTH * 14 + sizeof(dataFileHeader));
	uint8 textNum;
	uint8 *screenData;
	if (useScreen) {
		if (_skyScreen->sequenceRunning())
			_skyScreen->stopSequence();

		screenData = _skyScreen->giveCurrent();
	} else
		screenData = _screenBuf;
	switch (SkyState::_systemVars.language) {
		case DE_DEU: textNum = 1; break;
		case FR_FRA: textNum = 2; break;
		case IT_ITA: textNum = 4; break;
		case PT_BRA: textNum = 5; break;
		default: textNum = 0; break;
	}
	_skyText->displayText(_quitTexts[textNum * 2 + 0], textBuf1, true, 320, 255);
	_skyText->displayText(_quitTexts[textNum * 2 + 1], textBuf2, true, 320, 255);
	uint8 *curLine1 = textBuf1 + sizeof(dataFileHeader);
	uint8 *curLine2 = textBuf2 + sizeof(dataFileHeader);
	uint8 *targetLine = screenData + GAME_SCREEN_WIDTH * 80;
	for (uint8 cnty = 0; cnty < PAN_CHAR_HEIGHT; cnty++) {
		for (uint16 cntx = 0; cntx < GAME_SCREEN_WIDTH; cntx++) {
			if (curLine1[cntx])
				targetLine[cntx] = curLine1[cntx];
			if (curLine2[cntx])
				(targetLine + 24 * GAME_SCREEN_WIDTH)[cntx] = curLine2[cntx];
		}
		curLine1 += GAME_SCREEN_WIDTH;
		curLine2 += GAME_SCREEN_WIDTH;
		targetLine += GAME_SCREEN_WIDTH;
	}
	_skyScreen->halvePalette();
	_skyScreen->showScreen(screenData);
	free(textBuf1); free(textBuf2);
	delay(1500);
	_system->quit();
}

char SkyControl::_quitTexts[12][30] = {
	"Game over player one",
	"BE VIGILANT",
	"Das Spiel ist aus.",
	"SEI WACHSAM",
	"Game over joueur 1",
	"SOYEZ VIGILANTS",
	"Spelet r slut, Agent 1.",
	"VAR VAKSAM",
	"Game over giocatore 1",
	"SIATE VIGILANTI",
	"Fim de jogo para o jogador um",
	"BE VIGILANT"
};

uint8 SkyControl::_crossImg[594] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0B, 0x61, 0xFF, 0xFF, 0xFF, 0xFF, 0x4F, 0x4D, 0x61, 
    0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0x08, 0x4E, 0x53, 0x50, 0x4F, 0x0C, 0x4D, 0x4E, 0x51, 0x58, 0x58, 0x54, 0x4E, 0x08, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4E, 0x54, 0x58, 0x50, 0x4E, 0xFF, 
    0xFF, 0xFF, 0xFF, 0x50, 0x4E, 0x54, 0x58, 0x58, 0x54, 0x4E, 0x0C, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0x61, 0x53, 0x58, 0x54, 0x4E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0x50, 0x4E, 0x55, 0x58, 0x58, 0x53, 0x4E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x05, 0x51, 0x58, 0x58, 
    0x51, 0x50, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4F, 0x51, 0x58, 
    0x59, 0x58, 0x51, 0x61, 0xFF, 0xFF, 0x61, 0x54, 0x58, 0x58, 0x4F, 0x52, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4E, 0x55, 0x58, 0x58, 0x57, 0x4E, 
    0x4F, 0x56, 0x58, 0x57, 0x61, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4F, 0x51, 0x58, 0x58, 0x58, 0x58, 0x58, 0x54, 0x4E, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0x6A, 0x4F, 0x58, 0x58, 0x58, 0x58, 0x52, 0x06, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x04, 0x54, 0x58, 
    0x58, 0x58, 0x58, 0x57, 0x53, 0x61, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x04, 0x09, 0x58, 0x58, 0x58, 0x57, 0x56, 0x58, 0x58, 0x58, 
    0x57, 0x4F, 0x0A, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0x61, 0x55, 0x58, 0x58, 0x58, 0x58, 0x4E, 0x64, 0x57, 0x58, 0x58, 0x58, 0x58, 0x53, 0x61, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x61, 0x57, 0x58, 0x58, 0x58, 0x58, 
    0x50, 0xFF, 0xFF, 0x4E, 0x57, 0x58, 0x58, 0x58, 0x58, 0x56, 0x61, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0x61, 0x58, 0x58, 0x58, 0x58, 0x58, 0x53, 0x09, 0xFF, 0xFF, 0xFF, 0x4E, 
    0x57, 0x58, 0x58, 0x58, 0x58, 0x58, 0x0B, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x61, 0x57, 
    0x58, 0x58, 0x58, 0x58, 0x56, 0x4E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x61, 0x58, 0x58, 0x58, 0x58, 
    0x58, 0x57, 0x61, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x04, 0x55, 0x58, 0x58, 0x58, 0x58, 0x58, 0x4E, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4F, 0x58, 0x58, 0x58, 0x58, 0x4E, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0x06, 0x58, 0x58, 0x58, 0x58, 0x58, 0x52, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0x0C, 0x52, 0x58, 0x58, 0x51, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x61, 0x56, 0x58, 
    0x58, 0x58, 0x58, 0x56, 0x61, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x61, 0x56, 
    0x58, 0x61, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x4D, 0x4D, 0x51, 0x56, 0x58, 0x58, 0x50, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4F, 0x54, 0x09, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4E, 0x50, 0x54, 0x61, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x06, 0x50, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0x61, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0x61, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x61, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x61, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF
};

