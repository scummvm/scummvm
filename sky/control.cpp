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
		_oldScreen = (uint8*)malloc(PAN_CHAR_HEIGHT * PAN_LINE_WIDTH);
		_oldY = 0;
		_oldX = GAME_SCREEN_WIDTH;
}

SkyTextResource::~SkyTextResource(void) {
	free(_oldScreen);
}

void SkyTextResource::drawToScreen(bool doMask) {
	
	doMask = true;
	uint16 cnty, cntx, cpWidth;
	if ((_oldX == _x) && (_oldY == _y) && (_spriteData)) return;
	if (_oldX < GAME_SCREEN_WIDTH) {
		cpWidth = (PAN_LINE_WIDTH > (GAME_SCREEN_WIDTH - _oldX))?(GAME_SCREEN_WIDTH - _oldX):(PAN_LINE_WIDTH);
		for (cnty = 0; cnty < PAN_CHAR_HEIGHT; cnty++)
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
	uint8 *screenPos = _screen + _y * GAME_SCREEN_WIDTH + _x;
	uint8 *copyDest = _oldScreen;
	uint8 *copySrc = ((uint8*)_spriteData) + sizeof(dataFileHeader);
	for (cnty = 0; cnty < PAN_CHAR_HEIGHT; cnty++) {
		memcpy(copyDest, screenPos, cpWidth);
		for (cntx = 0; cntx < PAN_LINE_WIDTH; cntx++)
			if (copySrc[cntx]) screenPos[cntx] = copySrc[cntx];
		copySrc += _spriteData->s_width;
		copyDest += PAN_LINE_WIDTH;
		screenPos += GAME_SCREEN_WIDTH;
	}
	_system->copy_rect(_screen + _y * GAME_SCREEN_WIDTH + _x, GAME_SCREEN_WIDTH, _x, _y, cpWidth, PAN_CHAR_HEIGHT);
}

SkyControl::SkyControl(SkyScreen *screen, SkyDisk *disk, SkyMouse *mouse, SkyText *text, SkyMusicBase *music, OSystem *system, const char *savePath) {

	_skyScreen = screen;
	_skyDisk = disk;
	_skyMouse = mouse;
	_skyText = text;
	_skyMusic = music;
	_system = system;
	_savePath = savePath;
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

	_sprites.controlPanel	= _skyDisk->loadFile(60500, NULL);
	_sprites.button			= _skyDisk->loadFile(60501, NULL);
	_sprites.buttonDown		= _skyDisk->loadFile(60502, NULL);
	_sprites.savePanel		= _skyDisk->loadFile(60503, NULL);
	_sprites.yesNo			= _skyDisk->loadFile(60504, NULL);
	_sprites.slide			= _skyDisk->loadFile(60505, NULL);
	_sprites.slode			= _skyDisk->loadFile(60506, NULL);
	_sprites.slode2			= _skyDisk->loadFile(60507, NULL);
	_sprites.slide2			= _skyDisk->loadFile(60508, NULL);
	_sprites.musicBodge		= _skyDisk->loadFile(60509, NULL);
    
	//Main control panel:                                            X    Y Text       OnClick
	_controlPanel     = createResource(_sprites.controlPanel, 1, 0,  0,   0,  0,      DO_NOTHING, MAINPANEL);
	_exitButton       = createResource(      _sprites.button, 3, 0, 16, 125, 50,            EXIT, MAINPANEL);
	_slide            = createResource(      _sprites.slide2, 1, 0, 19,  99, 95,     SPEED_SLIDE, MAINPANEL);
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
	_musicPanButton   = createResource(      _sprites.button, 3, 0, 58, 119, 35,       TOGGLE_MS, MAINPANEL);
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

void SkyControl::animClick(SkyConResource *pButton) {

	if (pButton->_curSprite != pButton->_numSprites -1) {
		pButton->_curSprite++;
		pButton->drawToScreen(NO_MASK);
		_system->update_screen();
		delay(150);
		pButton->_curSprite--;
		pButton->drawToScreen(NO_MASK);
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
}

void SkyControl::doControlPanel(void) {

	if (SkyState::isDemo() && (!SkyState::isCDVersion())) {
		return ;
	}
	initPanel();

	_skyScreen->clearScreen();
	_skyScreen->setPalette(60510);
	
	drawMainPanel();
	
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
					buttonControl(NULL);
					_text->drawToScreen(WITH_MASK); // flush text restore buffer
					drawMainPanel();
					if (clickRes == QUIT_PANEL) quitPanel = true;
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
}

uint16 SkyControl::handleClick(SkyConResource *pButton) {

	switch(pButton->_onClick) {
		case DO_NOTHING:
			return 0;

		case REST_GAME_PANEL:
			animClick(pButton);
			return saveRestorePanel(false); // texts can't be edited

		case SAVE_GAME_PANEL:
			animClick(pButton);
			return saveRestorePanel(true); // texts can be edited

		case SAVE_A_GAME:
			animClick(pButton);
			return GAME_SAVED;

		case RESTORE_A_GAME:
			animClick(pButton);
			return GAME_RESTORED;

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
			return 0;

		case MUSIC_SLIDE:
			_mouseClicked = true;
			return doMusicSlide();

		case TOGGLE_FX:
			return toggleFx(pButton);

		case TOGGLE_MS:
			return 0;

		case EXIT:
			animClick(pButton);
			return QUIT_PANEL;

		case RESTART:
			animClick(pButton);
			return 0;

		case QUIT_TO_DOS:
			animClick(pButton);
			return 0;

		default: 
			error("SkyControl::handleClick: unknown routine: %X\n",pButton->_onClick);
	}
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

uint16 SkyControl::saveRestorePanel(bool allowEdit) {

	buttonControl(NULL);
	_text->drawToScreen(WITH_MASK); // flush text restore buffer

	SkyConResource **lookList;
	uint16 cnt;
	if (allowEdit) lookList = _savePanLookList;
	else lookList = _restorePanLookList;

	uint8 *saveGameTexts = (uint8*)malloc(MAX_SAVE_GAMES * MAX_TEXT_LEN);
	dataFileHeader *textSprites[MAX_ON_SCREEN];
	_firstText = 0;

	_savePanel->drawToScreen(NO_MASK);
	_quitButton->drawToScreen(NO_MASK);
	
	loadSaveDescriptions(saveGameTexts);
	setUpGameSprites(saveGameTexts, textSprites, _firstText);
		
	uint16 selectedGame = 0;

	bool quitPanel = false;
	bool refreshNames = true;
	uint16 clickRes;
	while (!quitPanel) {
		if (refreshNames) {
			showSprites(textSprites);
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
		}
		bool haveButton = false;
		for (uint16 cnt = 0; cnt < 6; cnt++)
			if (lookList[cnt]->isMouseOver(_mouseX, _mouseY)) {
				buttonControl(lookList[cnt]);
				haveButton = true;

				if (_mouseClicked && lookList[cnt]->_onClick) {
					_mouseClicked = false;
					
					clickRes = handleClick(lookList[cnt]);
			        if ((clickRes == CANCEL_PRESSED) || (clickRes == GAME_SAVED) || 
						(clickRes == GAME_RESTORED) || (clickRes == NO_DISK_SPACE))
						quitPanel = true;
					if (clickRes == SHIFTED) {
						setUpGameSprites(saveGameTexts, textSprites, _firstText);
						refreshNames = true;
					}
				}
			}
		if (_mouseClicked) {
			if ((_mouseX >= GAME_NAME_X) && (_mouseX <= GAME_NAME_X + PAN_LINE_WIDTH) &&
				(_mouseY >= GAME_NAME_Y) && (_mouseY <= GAME_NAME_Y + PAN_CHAR_HEIGHT * MAX_ON_SCREEN)) {

					selectedGame = (_mouseY - GAME_NAME_Y) / PAN_CHAR_HEIGHT + _firstText;
			}
		}
		if (!haveButton) buttonControl(NULL);

	}

	for (cnt = 0; cnt < MAX_ON_SCREEN; cnt++)
		free(textSprites[cnt]);

	free(saveGameTexts);

    return clickRes;
}

void SkyControl::setUpGameSprites(uint8 *nameBuf, dataFileHeader **nameSprites, uint16 firstNum) {

	nameBuf += firstNum * MAX_TEXT_LEN;

	for (uint16 cnt = 0; cnt < MAX_ON_SCREEN; cnt++) {
		displayText_t textSpr = _skyText->displayText((char*)nameBuf, NULL, false, PAN_LINE_WIDTH, 37);
		nameBuf += MAX_TEXT_LEN;
		nameSprites[cnt] = (dataFileHeader*)textSpr.textData;
	}
}

void SkyControl::showSprites(dataFileHeader **nameSprites) {

	SkyConResource *drawResource = new SkyConResource(NULL, 1, 0, 0, 0, 0, 0, _system, _screenBuf);
	for (uint16 cnt = 0; cnt < MAX_ON_SCREEN; cnt++) {
		drawResource->setSprite(nameSprites[cnt]);
		drawResource->setXY(GAME_NAME_X, GAME_NAME_Y + cnt * PAN_CHAR_HEIGHT);
		drawResource->drawToScreen(NO_MASK);
	}
	delete drawResource;
}

void SkyControl::loadSaveDescriptions(uint8 *destBuf) {

	memset(destBuf, 0, MAX_SAVE_GAMES * MAX_TEXT_LEN);

	File *inf = new File();
	inf->open("SKY.SAV",_savePath);
	if (inf->isOpen()) {
		uint8 *tmpBuf = (uint8*)malloc(inf->size());
		inf->read(tmpBuf, inf->size());
		uint8 *destPos = destBuf;
		uint8 *inPos = tmpBuf;
		for (uint16 cnt = 0; cnt < MAX_SAVE_GAMES; cnt++) {
			sprintf((char*)destPos,"%3d: ", cnt + 1);
			uint8 nameCnt = 0;
			while (destPos[nameCnt + 5] = inPos[nameCnt]) nameCnt++;
			destPos += MAX_TEXT_LEN;
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
