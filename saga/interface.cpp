/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Game interface module
#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/actor.h"
#include "saga/console.h"
#include "saga/font.h"
#include "saga/objectmap.h"
#include "saga/isomap.h"
#include "saga/itedata.h"
#include "saga/puzzle.h"
#include "saga/render.h"
#include "saga/rscfile_mod.h"
#include "saga/scene.h"
#include "saga/script.h"
#include "saga/sprite.h"

#include "saga/interface.h"

#include "common/system.h"
#include "common/timer.h"

namespace Saga {

static int verbTypeToTextStringsIdLUT[kVerbTypesMax] = {
	-1,
	kTextPickUp,
	kTextLookAt,
	kTextWalkTo,
	kTextTalkTo,
	kTextOpen,
	kTextClose,
	kTextGive,
	kTextUse,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1
};

Interface::Interface(SagaEngine *vm) : _vm(vm), _initialized(false) {
	byte *resource;
	size_t resourceLength;
	int result;
	int i;

	if (_initialized) {
		return;
	}
	

	// Load interface module resource file context
	_interfaceContext = _vm->getFileContext(GAME_RESOURCEFILE, 0);
	if (_interfaceContext == NULL) {
		error("Interface::Interface(): unable to load resource");
	}
	
	_mainPanel.buttons = _vm->getDisplayInfo().mainPanelButtons;
	_mainPanel.buttonsCount = _vm->getDisplayInfo().mainPanelButtonsCount;

	for (i = 0; i < kVerbTypesMax; i++) {
		_verbTypeToPanelButton[i] = NULL;
	}
	
	for (i = 0; i < _mainPanel.buttonsCount; i++) {
		if (_mainPanel.buttons[i].type == kPanelButtonVerb) {
			_verbTypeToPanelButton[_mainPanel.buttons[i].id] = &_mainPanel.buttons[i];
		}
	}

	result = RSC_LoadResource(_interfaceContext, _vm->getResourceDescription()->mainPanelResourceId, &resource, &resourceLength);
	if ((result != SUCCESS) || (resourceLength == 0)) {
		error("Interface::Interface(): unable to load mainPanel resource");
	}
	_vm->decodeBGImage(resource, resourceLength, &_mainPanel.image,
		&_mainPanel.imageLength, &_mainPanel.imageWidth, &_mainPanel.imageHeight);
	
	RSC_FreeResource(resource);

	_conversePanel.buttons = _vm->getDisplayInfo().conversePanelButtons;
	_conversePanel.buttonsCount = _vm->getDisplayInfo().conversePanelButtonsCount;

	result = RSC_LoadResource(_interfaceContext, _vm->getResourceDescription()->conversePanelResourceId, &resource, &resourceLength);
	if ((result != SUCCESS) || (resourceLength == 0)) {
		error("Interface::Interface unable to load conversePanel resource");
	}
	_vm->decodeBGImage(resource, resourceLength, &_conversePanel.image,
		&_conversePanel.imageLength, &_conversePanel.imageWidth, &_conversePanel.imageHeight);
	RSC_FreeResource(resource);

	_optionPanel.buttons = _vm->getDisplayInfo().optionPanelButtons;
	_optionPanel.buttonsCount = _vm->getDisplayInfo().optionPanelButtonsCount;

	result = RSC_LoadResource(_interfaceContext, _vm->getResourceDescription()->optionPanelResourceId, &resource, &resourceLength);
	if ((result != SUCCESS) || (resourceLength == 0)) {
		error("Interface::Interface unable to load optionPanel resource");
	}
	_vm->decodeBGImage(resource, resourceLength, &_optionPanel.image,
		&_optionPanel.imageLength, &_optionPanel.imageWidth, &_optionPanel.imageHeight);
	RSC_FreeResource(resource);


	if (_vm->_sprite->loadList(_vm->getResourceDescription()->mainPanelSpritesResourceId, _mainPanel.sprites) != SUCCESS) {
		error("Interface::Interface(): Unable to load sprite list");
	}

	if (_vm->getGameType() == GType_ITE) {
		if (_vm->_sprite->loadList(_vm->getResourceDescription()->defaultPortraitsResourceId, _defPortraits) != SUCCESS) {
			error("Interface::Interface(): Unable to load sprite list");
		}
	} else {
		// TODO
	}

	_mainPanel.x = _vm->getDisplayInfo().mainPanelXOffset;
	_mainPanel.y = _vm->getDisplayInfo().mainPanelYOffset;
	_mainPanel.currentButton = NULL;
	_inventoryUpButton = _mainPanel.getButton(_vm->getDisplayInfo().inventoryUpButtonIndex);
	_inventoryDownButton = _mainPanel.getButton(_vm->getDisplayInfo().inventoryDownButtonIndex);
	

	_conversePanel.x = _vm->getDisplayInfo().conversePanelXOffset;
	_conversePanel.y = _vm->getDisplayInfo().conversePanelYOffset;
	_conversePanel.currentButton = NULL;
	_converseUpButton = _conversePanel.getButton(_vm->getDisplayInfo().converseUpButtonIndex);
	_converseDownButton = _conversePanel.getButton(_vm->getDisplayInfo().converseDownButtonIndex);

	_leftPortrait = 0;
	_rightPortrait = 0;

	_optionPanel.x = _vm->getDisplayInfo().optionPanelXOffset;
	_optionPanel.y = _vm->getDisplayInfo().optionPanelYOffset;
	_optionPanel.currentButton = NULL;
	_optionSaveFileSlider = _optionPanel.getButton(_vm->getDisplayInfo().optionSaveFileSliderIndex);
	_optionSaveFilePanel = _optionPanel.getButton(_vm->getDisplayInfo().optionSaveFilePanelIndex);

	_quitPanel.x = _vm->getDisplayInfo().quitPanelXOffset;
	_quitPanel.y = _vm->getDisplayInfo().quitPanelYOffset;
	_quitPanel.imageWidth = _vm->getDisplayInfo().quitPanelWidth;
	_quitPanel.imageHeight = _vm->getDisplayInfo().quitPanelHeight;
	_quitPanel.buttons = _vm->getDisplayInfo().quitPanelButtons;
	_quitPanel.buttonsCount = _vm->getDisplayInfo().quitPanelButtonsCount;
	_quitPanel.currentButton = NULL;

	_loadPanel.x = _vm->getDisplayInfo().loadPanelXOffset;
	_loadPanel.y = _vm->getDisplayInfo().loadPanelYOffset;
	_loadPanel.imageWidth = _vm->getDisplayInfo().loadPanelWidth;
	_loadPanel.imageHeight = _vm->getDisplayInfo().loadPanelHeight;
	_loadPanel.buttons = _vm->getDisplayInfo().loadPanelButtons;
	_loadPanel.buttonsCount = _vm->getDisplayInfo().loadPanelButtonsCount;
	_loadPanel.currentButton = NULL;

	_savePanel.x = _vm->getDisplayInfo().savePanelXOffset;
	_savePanel.y = _vm->getDisplayInfo().savePanelYOffset;
	_savePanel.imageWidth = _vm->getDisplayInfo().savePanelWidth;
	_savePanel.imageHeight = _vm->getDisplayInfo().savePanelHeight;
	_savePanel.buttons = _vm->getDisplayInfo().savePanelButtons;
	_savePanel.buttonsCount = _vm->getDisplayInfo().savePanelButtonsCount;
	_saveEdit = _savePanel.getButton(_vm->getDisplayInfo().saveEditIndex);
	_savePanel.currentButton = NULL;

	_active = true;
	_panelMode = _lockedMode = kPanelNull;
	_savedMode = -1;
	_fadeMode = kNoFade;
	_inMainMode = false;
	*_statusText = 0;
	_statusOnceColor = -1;

	_inventoryCount = 0;
	_inventoryPos = 0;
	_inventoryStart = 0;
	_inventoryEnd = 0;
	_inventoryBox = 0;
	_inventorySize = ITE_INVENTORY_SIZE;
	_saveReminderState = 0;

	_optionSaveFileTop = 0;
	_optionSaveFileTitleNumber = 0;

	_inventory = (uint16 *)calloc(_inventorySize, sizeof(uint16));
	if (_inventory == NULL) {
		error("Interface::Interface(): not enough memory");
	}

	_textInputRepeatPhase = 0;
	_textInput = false;
	_statusTextInput = false;
	_statusTextInputState = kStatusTextInputFirstRun;

	_initialized = true;
}

Interface::~Interface(void) {
	free(_inventory);
	 
	_mainPanel.sprites.freeMem();
	_defPortraits.freeMem();
	_scenePortraits.freeMem();
	_initialized = false;
}

int Interface::activate() {
	if (!_active) {
		_active = true;
		_vm->_script->_skipSpeeches = false;
		_vm->_actor->_protagonist->targetObject = ID_NOTHING;
		_vm->_gfx->showCursor(true);
		unlockMode();
		if (_panelMode == kPanelMain){
			_saveReminderState = 1;
		}
		draw();
	}

	return SUCCESS;
}

int Interface::deactivate() {
	if (_active) {
		_active = false;
		_vm->_gfx->showCursor(false);
		lockMode();
		setMode(kPanelNull);
	}

	return SUCCESS;
}

void Interface::rememberMode() {
	assert (_savedMode == -1);

	_savedMode = _panelMode; 
}

void Interface::restoreMode() {
	assert (_savedMode != -1);

	_panelMode = _savedMode;
	_savedMode = -1;

	draw();
}

void Interface::setMode(int mode) {
	debug(5, "Interface::setMode %i", mode);
	if (mode == kPanelMain) {
		_inMainMode = true;
		_saveReminderState = 1; //TODO: blinking timeout
	} else {
		if (mode == kPanelConverse) {
			_inMainMode = false;
		}		
		_saveReminderState = 0;
	}

	_panelMode = mode;
	
	switch (_panelMode) {
	case kPanelMain:
		_mainPanel.currentButton = NULL;
		break;
	case kPanelConverse:
		_conversePanel.currentButton = NULL;
		converseDisplayText();
		break;
	case kPanelOption:
		_optionPanel.currentButton = NULL;
		_vm->fillSaveList();
		calcOptionSaveSlider();
		if (_optionSaveFileTitleNumber >= _vm->getDisplayInfo().optionSaveFileVisible) {
			_optionSaveFileTitleNumber = _vm->getDisplayInfo().optionSaveFileVisible - 1;
		}
		break;
	case kPanelLoad:
		_loadPanel.currentButton = NULL;
		break;
	case kPanelQuit:
		_quitPanel.currentButton = NULL;
		break;
	case kPanelSave:
		_savePanel.currentButton = NULL;
		_textInputMaxWidth = _saveEdit->width - 10;
		_textInput = true;
		_textInputStringLength = strlen(_textInputString);
		_textInputPos = _textInputStringLength + 1;
		_textInputRepeatPhase = 0;
		break;
	case kPanelMap:
		mapPanelShow();
		break;
	}

	draw();
}

bool Interface::processAscii(uint16 ascii, bool synthetic) {
	int i;
	PanelButton *panelButton;
	if (!synthetic)
		_textInputRepeatPhase = 0;
	if (_statusTextInput) {
		processStatusTextInput(ascii);
		return true;
	}
	switch (_panelMode) {
	case kPanelNull:
		if (ascii == 27) {// Esc
			if (_vm->_scene->isInDemo()) {
				_vm->_scene->skipScene();
			} else {
				_vm->_actor->abortAllSpeeches();
			}
			return true;
		}
		break;
	case kPanelOption:
		//TODO: check input dialog keys
		if (ascii == 27) {// Esc
			ascii = 'c'; //continue
		}
		for (i = 0; i < _optionPanel.buttonsCount; i++) {
			panelButton = &_optionPanel.buttons[i];
			if (panelButton->type == kPanelButtonOption) {
				if (panelButton->ascii == ascii) {
					setOption(panelButton);				
					return true;
				}
			}
		}
		break;
	case kPanelSave:
		if (_textInput) {
			processTextInput(ascii);
			return true;
		} else {
			if (ascii == 27) {// Esc
				ascii = 'c'; //cancel
			}
			for (i = 0; i < _savePanel.buttonsCount; i++) {
				panelButton = &_savePanel.buttons[i];
				if (panelButton->type == kPanelButtonSave) {
					if (panelButton->ascii == ascii) {
						setSave(panelButton);				
						return true;
					}
				}
			}
		}
		break;
	case kPanelQuit:
		if (ascii == 27) {// Esc
			ascii = 'c'; //cancel
		}
		for (i = 0; i < _quitPanel.buttonsCount; i++) {
			panelButton = &_quitPanel.buttons[i];
			if (panelButton->type == kPanelButtonQuit) {
				if (panelButton->ascii == ascii) {
					setQuit(panelButton);				
					return true;
				}
			}
		}
		break;
	case kPanelLoad:
		for (i = 0; i < _loadPanel.buttonsCount; i++) {
			panelButton = &_loadPanel.buttons[i];
			if (panelButton->type == kPanelButtonLoad) {
				if (panelButton->ascii == ascii) {
					setLoad(panelButton);				
					return true;
				}
			}
		}
		break;
	case kPanelMain:
		for (i = 0; i < _mainPanel.buttonsCount; i++) {
			panelButton = &_mainPanel.buttons[i];
			if (panelButton->ascii == ascii) {
				if (panelButton->type == kPanelButtonVerb) {
					_vm->_script->setVerb(panelButton->id);
				}			
				if (panelButton->type == kPanelButtonArrow) {
					inventoryChangePos(panelButton->id);
				}			
				return true;
			}
		}
		if (ascii == 15) // ctrl-o
		{
			if (_saveReminderState > 0) {
				setMode(kPanelOption);
				return true;
			}
		}
		break;
	case kPanelConverse:
		switch (ascii) {
		case 'x':
			setMode(kPanelMain);
			if (_vm->_puzzle->isActive())
				_vm->_puzzle->exitPuzzle();
			break;

		case 'u':
			converseChangePos(-1);
			break;

		case 'd':
			converseChangePos(1);
			break;

		case '1':
		case '2':
		case '3':
		case '4':
			converseSetPos(ascii);
			break;

		}
		break;
	case kPanelMap:
		mapPanelClean();
		break;
	}
	return false;
}

#define KEYBOARD_REPEAT_DELAY1 300000L
#define KEYBOARD_REPEAT_DELAY2 50000L

void Interface::textInputRepeatCallback(void *refCon) {
	((Interface *)refCon)->textInputRepeat();
}                                                                               

void Interface::textInputStartRepeat(uint16 ascii) {
	if (!_textInputRepeatPhase) {
		_textInputRepeatPhase = 1;
		Common::g_timer->removeTimerProc(&textInputRepeatCallback);
		Common::g_timer->installTimerProc(&textInputRepeatCallback, KEYBOARD_REPEAT_DELAY1, this);
	}

	_textInputRepeatChar = ascii;
}

void Interface::textInputRepeat() {
	if (_textInputRepeatPhase == 1) {
		_textInputRepeatPhase = 2;
		Common::g_timer->removeTimerProc(&textInputRepeatCallback);
		Common::g_timer->installTimerProc(&textInputRepeatCallback, KEYBOARD_REPEAT_DELAY2, this);
	} else if (_textInputRepeatPhase == 2) {
		processAscii(_textInputRepeatChar, true);
	}
}

void Interface::processKeyUp(uint16 ascii) {
	if (_textInputRepeatPhase) {
		Common::g_timer->removeTimerProc(&textInputRepeatCallback);
		_textInputRepeatPhase = 0;
	}
}

void Interface::setStatusText(const char *text, int statusColor) {
	assert(text != NULL);
	assert(strlen(text) < STATUS_TEXT_LEN);

	strncpy(_statusText, text, STATUS_TEXT_LEN);
	_statusOnceColor = statusColor;
	drawStatusBar();	
}

int Interface::loadScenePortraits(int resourceId) {
	_scenePortraits.freeMem();

	return _vm->_sprite->loadList(resourceId, _scenePortraits);
}

void Interface::drawVerbPanel(SURFACE *backBuffer, PanelButton* panelButton) {
	PanelButton * rightButtonVerbPanelButton;
	PanelButton * currentVerbPanelButton;
	int textColor;
	int spriteNumber;
	Point point;

	rightButtonVerbPanelButton = getPanelButtonByVerbType(_vm->_script->getRightButtonVerb());
	currentVerbPanelButton = getPanelButtonByVerbType(_vm->_script->getCurrentVerb());

	if (panelButton->state) {
		textColor = _vm->getDisplayInfo().verbTextActiveColor;
	} else {
		if (panelButton == rightButtonVerbPanelButton) {
			textColor = _vm->getDisplayInfo().verbTextActiveColor;
		} else {
			textColor = _vm->getDisplayInfo().verbTextColor;
		}
	}

	if (panelButton == currentVerbPanelButton) {
		spriteNumber = panelButton->downSpriteNumber;
	} else {
		spriteNumber = panelButton->upSpriteNumber;
	}
	point.x = _mainPanel.x + panelButton->xOffset;
	point.y = _mainPanel.y + panelButton->yOffset;

	_vm->_sprite->draw(backBuffer, _vm->getDisplayClip(), _mainPanel.sprites, spriteNumber, point, 256);

	drawVerbPanelText(backBuffer, panelButton, textColor, _vm->getDisplayInfo().verbTextShadowColor);
}

void Interface::draw() {
	SURFACE *backBuffer;
	int i;

	Point leftPortraitPoint;
	Point rightPortraitPoint;	
	Point origin;

	backBuffer = _vm->_gfx->getBackBuffer();

	if (_vm->_scene->isInDemo() || _fadeMode == kFadeOut)
		return;


	drawStatusBar();

	if (_panelMode == kPanelMain) {

		origin.x = _mainPanel.x;
		origin.y = _mainPanel.y;

		bufToSurface(backBuffer, _mainPanel.image, _mainPanel.imageWidth, _mainPanel.imageHeight, NULL, &origin);
		for (i = 0; i < kVerbTypesMax; i++) {
			if (_verbTypeToPanelButton[i] != NULL) {
				drawVerbPanel(backBuffer, _verbTypeToPanelButton[i]);
			}
		}
	} else {
		if (_panelMode == kPanelConverse) {	

			origin.x = _conversePanel.x;
			origin.y = _conversePanel.y;

			bufToSurface(backBuffer, _conversePanel.image, _conversePanel.imageWidth,
				_conversePanel.imageHeight, NULL, &origin);

			converseDisplayTextLines(backBuffer);
		}
	}

	if (_panelMode == kPanelMain || _panelMode == kPanelConverse ||
		_lockedMode == kPanelMain || _lockedMode == kPanelConverse) {
		leftPortraitPoint.x = _mainPanel.x + _vm->getDisplayInfo().leftPortraitXOffset;
		leftPortraitPoint.y = _mainPanel.y + _vm->getDisplayInfo().leftPortraitYOffset;
		_vm->_sprite->draw(backBuffer, _vm->getDisplayClip(), _defPortraits, _leftPortrait, leftPortraitPoint, 256);
	}

	if (!_inMainMode && _vm->getDisplayInfo().rightPortraitXOffset >= 0) { //FIXME: should we change !_inMainMode to _panelMode == kPanelConverse ?
		rightPortraitPoint.x = _mainPanel.x + _vm->getDisplayInfo().rightPortraitXOffset;
		rightPortraitPoint.y = _mainPanel.y + _vm->getDisplayInfo().rightPortraitYOffset;

		// This looks like hack - particularly since it's only done for
		// the right-side portrait - and perhaps it is! But as far as I
		// can tell this is what the original engine does. And it keeps
		// ITE from crashing when entering the Elk King's court.

		if (_rightPortrait >= _scenePortraits.spriteCount)
			_rightPortrait = 0;

		_vm->_sprite->draw(backBuffer, _vm->getDisplayClip(), _scenePortraits, _rightPortrait, rightPortraitPoint, 256);
	}

	drawInventory(backBuffer);
}

void Interface::calcOptionSaveSlider() {
	int totalFiles = _vm->getSaveFilesCount();
	int visibleFiles = _vm->getDisplayInfo().optionSaveFileVisible; 
	int height = _optionSaveFileSlider->height;
	int sliderHeight;
	int pos;

	if (totalFiles < visibleFiles) {
		totalFiles = visibleFiles;
	}

	sliderHeight = visibleFiles * height / totalFiles;
	if (sliderHeight < 7) {
		sliderHeight = 7;
	}

	if (totalFiles - visibleFiles <= 0) {
		pos = 0;
	} else {
		pos = _optionSaveFileTop * (height - sliderHeight) / (totalFiles - visibleFiles);
	}
	_optionPanel.calcPanelButtonRect(_optionSaveFileSlider, _optionSaveRectTop);
	_optionSaveRectBottom = _optionSaveRectSlider = _optionSaveRectTop;

	_optionSaveRectTop.bottom = _optionSaveRectTop.top + pos;
	_optionSaveRectTop.top++;
	_optionSaveRectTop.right--;

	_optionSaveRectSlider.top = _optionSaveRectTop.bottom;
	_optionSaveRectSlider.bottom = _optionSaveRectSlider.top + sliderHeight;

	_optionSaveRectBottom.top = _optionSaveRectSlider.bottom;
	_optionSaveRectBottom.right--;
}

void Interface::drawPanelText(SURFACE *ds, InterfacePanel *panel, PanelButton *panelButton) {
	const char *text;
	int textWidth;
	Rect rect;

	text = _vm->getTextString(panelButton->id);
	panel->calcPanelButtonRect(panelButton, rect);
	if (panelButton->xOffset < 0) {
		textWidth = _vm->_font->getStringWidth(MEDIUM_FONT_ID, text, 0, 0);
		rect.left += 2 + (panel->imageWidth - 1 - textWidth) / 2;
	}

	_vm->_font->draw(MEDIUM_FONT_ID, ds, text, 0, rect.left , rect.top + 1,
		_vm->getDisplayInfo().verbTextColor, _vm->getDisplayInfo().verbTextShadowColor, FONT_SHADOW);
}

void Interface::drawOption() {
	const char *text;
	SURFACE *backBuffer;
	int i;
	int fontHeight;
	uint j, idx;
	int fgColor;
	int bgColor;
	Point origin;
	Rect rect;
	Rect rect2;
	PanelButton *panelButton;

	backBuffer = _vm->_gfx->getBackBuffer();
	origin.x = _vm->getDisplayInfo().optionPanelXOffset;
	origin.y = _vm->getDisplayInfo().optionPanelYOffset;

	bufToSurface(backBuffer, _optionPanel.image, _optionPanel.imageWidth, _optionPanel.imageHeight, NULL, &origin);

	for (i = 0; i < _optionPanel.buttonsCount; i++) {		
		panelButton = &_optionPanel.buttons[i];
		if (panelButton->type == kPanelButtonOption) {
			drawPanelButtonText(backBuffer, &_optionPanel, panelButton);
		}		
		if (panelButton->type == kPanelButtonOptionText) {
			drawPanelText(backBuffer, &_optionPanel, panelButton);			
		}		
	}	

	if (_optionSaveRectTop.height() > 0) {
		drawRect(backBuffer, _optionSaveRectTop, kITEColorDarkGrey);
	}
	
	drawButtonBox(backBuffer, _optionSaveRectSlider, kSlider, _optionSaveFileSlider->state > 0);

	if (_optionSaveRectBottom.height() > 0) {
		drawRect(backBuffer, _optionSaveRectBottom, kITEColorDarkGrey);
	}

	_optionPanel.calcPanelButtonRect(_optionSaveFilePanel, rect);
	rect.top++;
	rect2 = rect;
	fontHeight = _vm->_font->getHeight(SMALL_FONT_ID);
	for (j = 0; j < _vm->getDisplayInfo().optionSaveFileVisible; j++) {
		bgColor = kITEColorDarkGrey0C;
		fgColor = kITEColorBrightWhite;
		
		idx = j + _optionSaveFileTop;
		if (idx == _optionSaveFileTitleNumber) {
			SWAP(bgColor, fgColor);
		}
		if (idx < _vm->getSaveFilesCount()) {
			rect2.top = rect.top + j * (fontHeight + 1);
			rect2.bottom = rect2.top + fontHeight;
			backBuffer->fillRect(rect2, bgColor);
			text = _vm->getSaveFile(idx)->name;
			_vm->_font->draw(SMALL_FONT_ID, backBuffer, text, 0,
				 rect.left + 1, rect2.top, fgColor, 0, 0);
		}
	}

}

void Interface::drawQuit() {
	SURFACE *backBuffer;
	Rect rect;
	int i;
	PanelButton *panelButton;

	backBuffer = _vm->_gfx->getBackBuffer();

	_quitPanel.getRect(rect);
	drawButtonBox(backBuffer, rect, kButton, false);
	for (i = 0; i < _quitPanel.buttonsCount; i++) {		
		panelButton = &_quitPanel.buttons[i];
		if (panelButton->type == kPanelButtonQuit) {
			drawPanelButtonText(backBuffer, &_quitPanel, panelButton);
		}		
		if (panelButton->type == kPanelButtonQuitText) {
			drawPanelText(backBuffer, &_quitPanel, panelButton);
		}		
	}
}

void Interface::handleQuitUpdate(const Point& mousePoint) {
	bool releasedButton;

	_quitPanel.currentButton = quitHitTest(mousePoint);	
	releasedButton = (_quitPanel.currentButton != NULL) && (_quitPanel.currentButton->state > 0) && (!_vm->mouseButtonPressed());

	if (!_vm->mouseButtonPressed()) {
		_quitPanel.zeroAllButtonState();
	}

	if (releasedButton) {
		setQuit(_quitPanel.currentButton); 
	}
}

void Interface::handleQuitClick(const Point& mousePoint) {
	_quitPanel.currentButton = quitHitTest(mousePoint);

	_quitPanel.zeroAllButtonState();

	if (_quitPanel.currentButton == NULL) {
		return;
	}

	_quitPanel.currentButton->state = 1;
}

void Interface::setQuit(PanelButton *panelButton) {
	_quitPanel.currentButton = NULL;
	switch (panelButton->id) {
		case kTextCancel:
			setMode(kPanelOption);
			break;
		case kTextQuit:
			_vm->shutDown();
			break;
	}
}

void Interface::drawLoad() {
	SURFACE *backBuffer;
	Rect rect;
	int i;
	PanelButton *panelButton;

	backBuffer = _vm->_gfx->getBackBuffer();

	_loadPanel.getRect(rect);
	drawButtonBox(backBuffer, rect, kButton, false);
	for (i = 0; i < _loadPanel.buttonsCount; i++) {		
		panelButton = &_loadPanel.buttons[i];
		if (panelButton->type == kPanelButtonLoad) {
			drawPanelButtonText(backBuffer, &_loadPanel, panelButton);
		}		
		if (panelButton->type == kPanelButtonLoadText) {
			drawPanelText(backBuffer, &_loadPanel, panelButton);
		}		
	}
}

void Interface::handleLoadUpdate(const Point& mousePoint) {
	bool releasedButton;

	_loadPanel.currentButton = loadHitTest(mousePoint);	
	releasedButton = (_loadPanel.currentButton != NULL) && (_loadPanel.currentButton->state > 0) && (!_vm->mouseButtonPressed());

	if (!_vm->mouseButtonPressed()) {
		_loadPanel.zeroAllButtonState();
	}

	if (releasedButton) {
		setLoad(_loadPanel.currentButton); 
	}
}

void Interface::handleLoadClick(const Point& mousePoint) {
	_loadPanel.currentButton = loadHitTest(mousePoint);

	_loadPanel.zeroAllButtonState();

	if (_loadPanel.currentButton == NULL) {
		return;
	}

	_loadPanel.currentButton->state = 1;
}

void Interface::setLoad(PanelButton *panelButton) {
	_loadPanel.currentButton = NULL;
	switch (panelButton->id) {
		case kTextOK:			
			setMode(kPanelMain);
			break;
	}
}

void Interface::processStatusTextInput(uint16 ascii) {

	textInputStartRepeat(ascii);
	switch (ascii) {
	case(27): // esc
		_statusTextInputState = kStatusTextInputAborted;
		_statusTextInput = false;
		_vm->_script->wakeUpThreads(kWaitTypeStatusTextInput);
		break;
	case(13): // return
		_statusTextInputState = kStatusTextInputEntered;
		_statusTextInput = false;
		_vm->_script->wakeUpThreads(kWaitTypeStatusTextInput);
		break;
	case(8): // backspace
		if (_statusTextInputPos == 0) {
			break;
		}
		_statusTextInputPos--;
		_statusTextInputString[_statusTextInputPos] = 0;
	default:
		if (_statusTextInputPos >= STATUS_TEXT_INPUT_MAX) {
			break;
		}
		if (((ascii >= 'a') && (ascii <='z')) || 
			((ascii >= '0') && (ascii <='9')) ||
			((ascii >= 'A') && (ascii <='Z')) ||
			(ascii == ' ')) {
			_statusTextInputString[_statusTextInputPos++] = ascii;
			_statusTextInputString[_statusTextInputPos] = 0;
		}
	}
	setStatusText(_statusTextInputString);
}

void Interface::processTextInput(uint16 ascii) {
	char ch[2];
	char tempString[SAVE_TITLE_SIZE];
	uint tempWidth;
	memset(tempString, 0, SAVE_TITLE_SIZE);
	ch[1] = 0;

	textInputStartRepeat(ascii);

	switch (ascii) {
	case(27): // esc
		_textInput = false;
		break;
	case(8): // backspace
		if (_textInputPos <= 1) {
			break;
		}
		_textInputPos--;
	case(127): // del
		if (_textInputPos <= _textInputStringLength) {
			if (_textInputPos != 1) {
				strncpy(tempString, _textInputString, _textInputPos - 1);							
			}
			if (_textInputPos != _textInputStringLength) {
				strncat(tempString, &_textInputString[_textInputPos], _textInputStringLength - _textInputPos);
			}
			strcpy(_textInputString, tempString);
			_textInputStringLength = strlen(_textInputString);
		}
		break;
	case(276): // left
		if (_textInputPos > 1) {
			_textInputPos--;
		}
		break;
	case(275): // right
		if (_textInputPos <= _textInputStringLength) {
			_textInputPos++;
		}
		break;
	default:
		if (((ascii >= 'a') && (ascii <='z')) || 
			((ascii >= '0') && (ascii <='9')) ||
			((ascii >= 'A') && (ascii <='Z')) ||
			(ascii == ' ')) {
			if (_textInputStringLength < SAVE_TITLE_SIZE - 1) {
				ch[0] = ascii;
				tempWidth = _vm->_font->getStringWidth(SMALL_FONT_ID, ch, 0, 0);
				tempWidth += _vm->_font->getStringWidth(SMALL_FONT_ID, _textInputString, 0, 0);
				if (tempWidth > _textInputMaxWidth) {
									break;
				}
				if (_textInputPos != 1) {
					strncpy(tempString, _textInputString, _textInputPos - 1);
					strcat(tempString, ch);
				}
				if ((_textInputStringLength == 0) || (_textInputPos == 1)) {
					strcpy(tempString, ch);
				}
				if ((_textInputStringLength != 0) && (_textInputPos != _textInputStringLength)) {
					strncat(tempString, &_textInputString[_textInputPos - 1], _textInputStringLength - _textInputPos + 1);
				}
				
				strcpy(_textInputString, tempString);
				_textInputStringLength = strlen(_textInputString);
				_textInputPos++;
			}
		}
		break;
	}
}

void Interface::drawTextInput(SURFACE *ds, InterfacePanel *panel, PanelButton *panelButton) {
	Rect rect;
	char ch[2];
	int fgColor;
	uint i;

	ch[1] = 0;
	panel->calcPanelButtonRect(panelButton, rect);
	drawButtonBox(ds, rect, kEdit, _textInput);
	rect.left += 4; 
	rect.top += 4;
	rect.setHeight(_vm->_font->getHeight(SMALL_FONT_ID));

	i = 0;	
	while ((ch[0] = _textInputString[i++]) != 0) {
		rect.setWidth(_vm->_font->getStringWidth(SMALL_FONT_ID, ch, 0, 0));
		if ((i == _textInputPos) && _textInput) {
			fgColor = kITEColorBlack;	
			ds->fillRect(rect, kITEColorWhite);
		} else {
			fgColor = kITEColorWhite;	
		}
		_vm->_font->draw(SMALL_FONT_ID, ds, ch, 0, rect.left, 
			rect.top + 1, fgColor, 0, 0); 
		rect.left += rect.width();
	}
	if (_textInput && (_textInputPos >= i)) {
		ch[0] = ' ';
		rect.setWidth(_vm->_font->getStringWidth(SMALL_FONT_ID, ch, 0, 0));
		ds->fillRect(rect, kITEColorWhite);
	}
}

void Interface::drawSave() {
	SURFACE *backBuffer;
	Rect rect;
	int i;
	PanelButton *panelButton;

	backBuffer = _vm->_gfx->getBackBuffer();

	_savePanel.getRect(rect);
	drawButtonBox(backBuffer, rect, kButton, false);
	for (i = 0; i < _savePanel.buttonsCount; i++) {		
		panelButton = &_savePanel.buttons[i];
		if (panelButton->type == kPanelButtonSave) {
			drawPanelButtonText(backBuffer, &_savePanel, panelButton);
		}		
		if (panelButton->type == kPanelButtonSaveText) {
			drawPanelText(backBuffer, &_savePanel, panelButton);
		}		
	}

	drawTextInput(backBuffer, &_savePanel, _saveEdit);
}

void Interface::handleSaveUpdate(const Point& mousePoint) {
	bool releasedButton;

	_savePanel.currentButton = saveHitTest(mousePoint);	

	validateSaveButtons();

	releasedButton = (_savePanel.currentButton != NULL) && 
		(_savePanel.currentButton->state > 0) && (!_vm->mouseButtonPressed());

	if (!_vm->mouseButtonPressed()) {
		_savePanel.zeroAllButtonState();
	}

	if (releasedButton) {
		setSave(_savePanel.currentButton); 
	}
}

void Interface::handleSaveClick(const Point& mousePoint) {
	_savePanel.currentButton = saveHitTest(mousePoint);
	
	validateSaveButtons();

	_savePanel.zeroAllButtonState();	
	
	if (_savePanel.currentButton == NULL) {
		_textInput = false;
		return;
	}

	_savePanel.currentButton->state = 1;
	if (_savePanel.currentButton == _saveEdit) {
		_textInput = true;
	}
}

void Interface::setSave(PanelButton *panelButton) {
	_savePanel.currentButton = NULL;
	uint titleNumber;
	char *fileName;
	switch (panelButton->id) {
		case kTextSave:
			if (_textInputStringLength == 0 ) {
				break;
			}
			if (!_vm->isSaveListFull() && (_optionSaveFileTitleNumber == 0)) {
				if (_vm->locateSaveFile(_textInputString, titleNumber)) {
					fileName = _vm->calcSaveFileName(_vm->getSaveFile(titleNumber)->slotNumber);
					_vm->save(fileName, _textInputString);
					_optionSaveFileTitleNumber = titleNumber;
				} else {
					fileName = _vm->calcSaveFileName(_vm->getNewSaveSlotNumber());
					_vm->save(fileName, _textInputString);
					_vm->fillSaveList();
					calcOptionSaveSlider();
				}
			} else {
				fileName = _vm->calcSaveFileName(_vm->getSaveFile(_optionSaveFileTitleNumber)->slotNumber);
				_vm->save(fileName, _textInputString);
			}
			setMode(kPanelOption);
			break;
		case kTextCancel:
			setMode(kPanelOption);
			break;
	}
}

void Interface::handleOptionUpdate(const Point& mousePoint) {
	int16 mouseY;
	Rect rect;	
	int totalFiles = _vm->getSaveFilesCount();
	int visibleFiles = _vm->getDisplayInfo().optionSaveFileVisible; 
	bool releasedButton;
	
	if (_vm->mouseButtonPressed()) {
		if (_optionSaveFileSlider->state > 0) {
			_optionPanel.calcPanelButtonRect(_optionSaveFileSlider, rect);

			mouseY = mousePoint.y - rect.top -_optionSaveFileMouseOff;

			if (totalFiles - visibleFiles <= 0) {
				_optionSaveFileTop = 0;
			} else {
				_optionSaveFileTop = mouseY * (totalFiles - visibleFiles) /
					(_optionSaveFileSlider->height - _optionSaveRectSlider.height());
			}

			_optionSaveFileTop = clamp(0, _optionSaveFileTop, _vm->getSaveFilesCount() - _vm->getDisplayInfo().optionSaveFileVisible);
			calcOptionSaveSlider();
		}
	}

	_optionPanel.currentButton = optionHitTest(mousePoint);	

	validateOptionButtons();

	releasedButton = (_optionPanel.currentButton != NULL) && (_optionPanel.currentButton->state > 0) && (!_vm->mouseButtonPressed());

	if (!_vm->mouseButtonPressed()) {
		_optionPanel.zeroAllButtonState();
	}

	if (releasedButton) {
		setOption(_optionPanel.currentButton); 
	}
}


void Interface::handleOptionClick(const Point& mousePoint) {
	Rect rect;
	_optionPanel.currentButton = optionHitTest(mousePoint);

	validateOptionButtons();

	_optionPanel.zeroAllButtonState();

	if (_optionPanel.currentButton == NULL) {
		return;
	}
	
	if (_optionPanel.currentButton == _optionSaveFileSlider) {		
		if ((_optionSaveRectTop.height() > 0) && (mousePoint.y < _optionSaveRectTop.bottom)) {
			_optionSaveFileTop -= _vm->getDisplayInfo().optionSaveFileVisible;
		} else {
			if ((_optionSaveRectBottom.height() > 0) && (mousePoint.y >= _optionSaveRectBottom.top)) {
				_optionSaveFileTop += _vm->getDisplayInfo().optionSaveFileVisible;
			} else {
				if (_vm->getDisplayInfo().optionSaveFileVisible < _vm->getSaveFilesCount()) {
					_optionSaveFileMouseOff = mousePoint.y - _optionSaveRectSlider.top;
					_optionPanel.currentButton->state = 1;
				}
			}
		}

		_optionSaveFileTop = clamp(0, _optionSaveFileTop, _vm->getSaveFilesCount() - _vm->getDisplayInfo().optionSaveFileVisible);
		calcOptionSaveSlider();
	} else {
		if (_optionPanel.currentButton == _optionSaveFilePanel) {
			_optionPanel.calcPanelButtonRect(_optionSaveFilePanel, rect);
			_optionSaveFileTitleNumber = (mousePoint.y - rect.top) / (_vm->_font->getHeight(SMALL_FONT_ID) + 1);
			
			if (_optionSaveFileTitleNumber >= _vm->getDisplayInfo().optionSaveFileVisible) {
				_optionSaveFileTitleNumber = _vm->getDisplayInfo().optionSaveFileVisible - 1;
			}
			_optionSaveFileTitleNumber += _optionSaveFileTop;
			if (_optionSaveFileTitleNumber >= _vm->getSaveFilesCount()) {
				_optionSaveFileTitleNumber = _vm->getSaveFilesCount() - 1;
			}
		} else {
			_optionPanel.currentButton->state = 1;
		}
	}
}


void Interface::setOption(PanelButton *panelButton) {
	char * fileName;
	_optionPanel.currentButton = NULL;
	switch (panelButton->id) {
		case kTextContinuePlaying:
			setMode(kPanelMain);
			break;
		case kTextQuitGame:
			setMode(kPanelQuit);
			break;
		case kTextLoad:			
			if (_vm->getSaveFilesCount() > 0) {
				if (_vm->isSaveListFull() || (_optionSaveFileTitleNumber > 0)) {
					fileName = _vm->calcSaveFileName(_vm->getSaveFile(_optionSaveFileTitleNumber)->slotNumber);
					setMode(kPanelMain);
					_vm->load(fileName);
				}
			}
			break;
		case kTextSave:
			if (!_vm->isSaveListFull() && (_optionSaveFileTitleNumber == 0)) {
				_textInputString[0] = 0;
			} else {
				strcpy(_textInputString, _vm->getSaveFile(_optionSaveFileTitleNumber)->name);
			}
			setMode(kPanelSave);
			break;
	}
}

void Interface::update(const Point& mousePoint, int updateFlag) {
	
	if (_vm->_scene->isInDemo() || _fadeMode == kFadeOut || !_active) {
		return;
	}

	if (_statusTextInput) {
		return;
	}

	if (_panelMode == kPanelMain) {
		if (updateFlag & UPDATE_MOUSEMOVE) {
			bool lastWasPlayfield = _lastMousePoint.y < _vm->getSceneHeight();
			if (mousePoint.y < _vm->getSceneHeight()) {
				if (!lastWasPlayfield) {
					handleMainUpdate(mousePoint);
				}
				_vm->_script->whichObject(mousePoint);
			} else {
				if (lastWasPlayfield) {
					_vm->_script->setNonPlayfieldVerb();
				}
				handleMainUpdate(mousePoint);
			}

		} else {

			if (updateFlag & UPDATE_MOUSECLICK) {
				if (mousePoint.y < _vm->getSceneHeight()) {
					_vm->_script->playfieldClick(mousePoint, (updateFlag & UPDATE_LEFTBUTTONCLICK) != 0);										
				} else {
					handleMainClick(mousePoint);
				}
			}
		}
	}

	if (_panelMode == kPanelConverse) {
		if (updateFlag & UPDATE_MOUSEMOVE) {

			handleConverseUpdate(mousePoint);

		} else {
			if (updateFlag & UPDATE_MOUSECLICK) {
				handleConverseClick(mousePoint);
			}
			if (updateFlag & UPDATE_WHEELUP) {
				converseChangePos(-1);
			}
			if (updateFlag & UPDATE_WHEELDOWN) {
				converseChangePos(1);
			}

			if (_vm->_puzzle->isActive()) {
				_vm->_puzzle->handleClick(mousePoint);
			}
		}
	}

	if (_panelMode == kPanelOption) {
		if (updateFlag & UPDATE_MOUSEMOVE) {

			handleOptionUpdate(mousePoint);

		} else {
			if (updateFlag & UPDATE_MOUSECLICK) {
				handleOptionClick(mousePoint);
			}
			if (updateFlag & UPDATE_WHEELUP) {
				if (_optionSaveFileTop)
					_optionSaveFileTop--;
				calcOptionSaveSlider();
			}
			if (updateFlag & UPDATE_WHEELDOWN) {
				if (_optionSaveFileTop < _vm->getSaveFilesCount() - _vm->getDisplayInfo().optionSaveFileVisible)
					_optionSaveFileTop++;
				calcOptionSaveSlider();
			}
		}
	}

	if (_panelMode == kPanelQuit) {
		if (updateFlag & UPDATE_MOUSEMOVE) {

			handleQuitUpdate(mousePoint);

		} else {
			if (updateFlag & UPDATE_MOUSECLICK) {
				handleQuitClick(mousePoint);
			}
		}
	}

	if (_panelMode == kPanelLoad) {
		if (updateFlag & UPDATE_MOUSEMOVE) {

			handleLoadUpdate(mousePoint);

		} else {
			if (updateFlag & UPDATE_MOUSECLICK) {
				handleLoadClick(mousePoint);
			}
		}
	}

	if (_panelMode == kPanelSave) {
		if (updateFlag & UPDATE_MOUSEMOVE) {

			handleSaveUpdate(mousePoint);

		} else {
			if (updateFlag & UPDATE_MOUSECLICK) {
				handleSaveClick(mousePoint);
			}
		}
	}

	if (_panelMode == kPanelMap) {
		if (updateFlag & UPDATE_MOUSECLICK)
			mapPanelClean();
	}

	_lastMousePoint = mousePoint;
}

void Interface::drawStatusBar() {
	SURFACE *backBuffer;
	Rect rect;

	int string_w;
	int color;

	backBuffer = _vm->_gfx->getBackBuffer();

	// Disable this for IHNM for now, since that game uses the full screen
	// in some cases.

	if (_vm->getGameType() == GType_IHNM) {
		return;
	}


	// Erase background of status bar
	rect.left = _vm->getDisplayInfo().statusXOffset;
	rect.top = _vm->getDisplayInfo().statusYOffset;
	rect.right = rect.left + _vm->getDisplayWidth();
	rect.bottom = rect.top + _vm->getDisplayInfo().statusHeight;

	drawRect(backBuffer, rect, _vm->getDisplayInfo().statusBGColor);

	string_w = _vm->_font->getStringWidth(SMALL_FONT_ID, _statusText, 0, 0);

	if (_statusOnceColor == -1)
		color = _vm->getDisplayInfo().statusTextColor;
	else
		color = _statusOnceColor;

	_vm->_font->draw(SMALL_FONT_ID, backBuffer, _statusText, 0, _vm->getDisplayInfo().statusXOffset + (_vm->getDisplayInfo().statusWidth - string_w) / 2,
			_vm->getDisplayInfo().statusYOffset + _vm->getDisplayInfo().statusTextY, color, 0, 0);

	if (_saveReminderState > 0) {
		rect.left = _vm->getDisplayInfo().saveReminderXOffset;
		rect.top = _vm->getDisplayInfo().saveReminderYOffset;
	
		rect.right = rect.left + _vm->getDisplayInfo().saveReminderWidth;
		rect.bottom = rect.top + _vm->getDisplayInfo().saveReminderHeight;
		_vm->_sprite->draw(backBuffer, _vm->getDisplayClip(),  _vm->_sprite->_mainSprites, 
			_saveReminderState == 1 ? _vm->getDisplayInfo().saveReminderFirstSpriteNumber : _vm->getDisplayInfo().saveReminderSecondSpriteNumber,
			rect, 256);

	}
}

void Interface::handleMainClick(const Point& mousePoint) {

	PanelButton *panelButton;

	panelButton = verbHitTest(mousePoint);
	if (panelButton) {
		_vm->_script->setVerb(panelButton->id);
		return;
	}

	panelButton = _mainPanel.hitTest(mousePoint, kPanelAllButtons);

	if (panelButton != NULL) {
		if (panelButton->type == kPanelButtonArrow) {
			panelButton->state = 1;
			converseChangePos(panelButton->id);
		}

		if (panelButton->type == kPanelButtonInventory) {
			if (_vm->_script->_pointerObject != ID_NOTHING) {
				_vm->_script->hitObject(_vm->leftMouseButtonPressed());
			}
			if (_vm->_script->_pendingVerb) {
				_vm->_actor->_protagonist->currentAction = kActionWait;
				_vm->_script->doVerb();
			}
		}		
	} else {
		if (_saveReminderState > 0) {
			Rect rect;
			rect.left = _vm->getDisplayInfo().saveReminderXOffset;
			rect.top = _vm->getDisplayInfo().saveReminderYOffset;

			rect.right = rect.left + _vm->getDisplayInfo().saveReminderWidth;
			rect.bottom = rect.top + _vm->getDisplayInfo().saveReminderHeight;
			if (rect.contains(mousePoint)) {
				setMode(kPanelOption);
			}
		}
	}
}

void Interface::handleMainUpdate(const Point& mousePoint) {
	PanelButton *panelButton;

	panelButton = verbHitTest(mousePoint);
	if (_mainPanel.currentButton != panelButton) {
		if (_mainPanel.currentButton) {
			if (_mainPanel.currentButton->type == kPanelButtonVerb) {
				setVerbState(_mainPanel.currentButton->id, 0);
			}
		}
		if (panelButton) {
			setVerbState(panelButton->id, 1);			
		}
	}
	
	if (panelButton) {
		_mainPanel.currentButton = panelButton;
		return;
	}


	if (!_vm->mouseButtonPressed()) {			// remove pressed flag
		if (_inventoryUpButton) {
			_inventoryUpButton->state = 0;
			_inventoryDownButton->state = 0;
		}
	}

	panelButton = _mainPanel.hitTest(mousePoint, kPanelAllButtons);
	
	bool changed = false;

	if ((panelButton != NULL) && (panelButton->type == kPanelButtonArrow)) {
		if (panelButton->state == 1) {
			//TODO: insert timeout catchup
			inventoryChangePos(panelButton->id);
		}
		changed = true;
	} else {
		_vm->_script->whichObject(mousePoint);						
	}

	changed = changed || (panelButton != _mainPanel.currentButton);
	_mainPanel.currentButton = panelButton;
	if (changed) {
		draw();
	}		

}

//inventory stuff
void Interface::inventoryChangePos(int chg) {
	if ((chg < 0 && _inventoryStart + chg >= 0) ||
		(chg > 0 && _inventoryStart  < _inventoryEnd)) {
			_inventoryStart += chg;
			draw();
		}
}

void Interface::inventorySetPos(int key) {
	_inventoryBox = key - '1';
	_inventoryPos = _inventoryStart + _inventoryBox;
	if (_inventoryPos >= _inventoryCount)
		_inventoryPos = -1;	
}

void Interface::updateInventory(int pos) {
	int cols = _vm->getDisplayInfo().inventoryColumns;
	if (pos >= _inventoryCount) {
		pos = _inventoryCount - 1;
	}
	if (pos < 0) {
		pos = 0;
	}
	_inventoryStart = (pos - cols) / cols * cols;
	if (_inventoryStart < 0) {
		_inventoryStart = 0;
	}
	
	_inventoryEnd = (_inventoryCount - 1  - cols) / cols * cols;
	if (_inventoryEnd < 0) {
		_inventoryEnd = 0;
	}	
}

void Interface::addToInventory(int objectId) {

	if (_inventoryCount >= _inventorySize) {
		return;
	}
		
	for (int i = _inventoryCount; i > 0; i--) {
		_inventory[i] = _inventory[i - 1];
	}

	_inventory[0] = objectId;
	_inventoryCount++;

	_inventoryPos = 0;
	updateInventory(0);

	draw();	
}

void Interface::removeFromInventory(int objectId) {
	int j = inventoryItemPosition(objectId);
	if (j == -1) {
		return;
	}

	int i;

	for (i = j; i < _inventoryCount - 1; i++) {
		_inventory[i] = _inventory[i + 1];
	}

	--_inventoryCount;
	_inventory[_inventoryCount] = 0;
	updateInventory(j);
	draw();
}

void Interface::clearInventory() {
	for (int i = 0; i < _inventoryCount; i++)
		_inventory[i] = 0;

	_inventoryCount = 0;
	updateInventory(0);
}

int Interface::inventoryItemPosition(int objectId) {
	for (int i = 0; i < _inventoryCount; i++)
		if (_inventory[i] == objectId)
			return i;

	return -1;
}

void Interface::drawInventory(SURFACE *backBuffer) {
	if (_panelMode != kPanelMain)
		return;
	int i;
	Rect rect;
	int ci;
	ObjectData *obj;
	Point point;
	ci = _inventoryStart;
	if (_inventoryStart != 0) {
		drawPanelButtonArrow(backBuffer, &_mainPanel, _inventoryUpButton);		
	}
	if (_inventoryStart != _inventoryEnd) {
		drawPanelButtonArrow(backBuffer, &_mainPanel, _inventoryDownButton);		
	}

	for (i = 0; i < _mainPanel.buttonsCount; i++) {
		if (ci >= _inventoryCount) {
			break;
		}
		if (_mainPanel.buttons[i].type != kPanelButtonInventory) {
			continue;
		}
		_mainPanel.calcPanelButtonRect(&_mainPanel.buttons[i], rect);
		
//4debug		drawRect(backBuffer, rect, kITEColorWhite); 
		point.x = rect.left;
		point.y = rect.top;
		obj = _vm->_actor->getObj(_inventory[ci]);		
		_vm->_sprite->draw(backBuffer, _vm->getDisplayClip(), _vm->_sprite->_mainSprites, obj->spriteListResourceId, rect, 256);
		
		ci++;
	}
}

void Interface::setVerbState(int verb, int state) {
	PanelButton * panelButton = getPanelButtonByVerbType(verb);
	if (state == 2) {
		state = (_mainPanel.currentButton == panelButton) ? 1 : 0;
	} 
	panelButton->state = state;
	draw();
}

void Interface::drawButtonBox(SURFACE *ds, const Rect& rect, ButtonKind kind, bool down) {	
	byte cornerColor;
	byte frameColor;
	byte fillColor;
	byte solidColor;
	byte odl, our, idl, iur;

	switch (kind ) {
		case( kSlider):
			cornerColor = 0x8b;
			frameColor = kITEColorBlack;
			fillColor = kITEColorLightBlue96;
			odl = kITEColorDarkBlue8a;
			our = kITEColorLightBlue92;
			idl = 0x89;
			iur = 0x94;
			solidColor = down ? kITEColorLightBlue94 : kITEColorLightBlue96;
			break;
		case( kEdit):
			cornerColor = kITEColorLightBlue96;
			frameColor = kITEColorLightBlue96;
			fillColor = kITEColorLightBlue96;
			our = kITEColorDarkBlue8a;
			odl = kITEColorLightBlue94;
			iur = 0x97;
			idl = 0x95;
			if (down) {
				solidColor = kITEColorBlue;
			} else {
				solidColor = kITEColorDarkGrey0C;
			}
			break;
		default:
			cornerColor = 0x8b;
			frameColor = kITEColorBlack;
			solidColor = fillColor = kITEColorLightBlue96;
			odl = kITEColorDarkBlue8a;
			our = kITEColorLightBlue94;
			idl = 0x97;
			iur = 0x95;
			if (down) {
				SWAP(odl, our);
				SWAP(idl, iur);
			}
			break;
	}

	int x = rect.left;
	int y = rect.top;
	int w = rect.width();
	int h = rect.height();
	int xe = rect.right - 1;
	int ye = rect.bottom - 1;

	((byte *)ds->getBasePtr(x, y))[0] = cornerColor;
	((byte *)ds->getBasePtr(x, ye))[0] = cornerColor;
	((byte *)ds->getBasePtr(xe, y))[0] = cornerColor;
	((byte *)ds->getBasePtr(xe, ye))[0] = cornerColor;
	ds->hLine(x + 1, y, x + 1 + w - 2, frameColor);
	ds->hLine(x + 1, ye, x + 1 + w - 2, frameColor);
	ds->vLine(x, y + 1, y + 1 + h - 2, frameColor);
	ds->vLine(xe, y + 1, y + 1 + h - 2, frameColor);

	x++;
	y++;
	xe--;
	ye--;
	w -= 2;
	h -= 2;
	ds->vLine(x, y, y + h - 1, odl);
	ds->hLine(x, ye, x + w - 1, odl);
	ds->vLine(xe, y, y + h - 1, our);
	ds->hLine(x + 1, y, x + 1 + w - 2, our);

	x++;
	y++;
	xe--;
	ye--;
	w -= 2;
	h -= 2;
	((byte *)ds->getBasePtr(x, y))[0] = fillColor;
	((byte *)ds->getBasePtr(xe, ye))[0] = fillColor;
	ds->vLine(x, y + 1, y + 1 + h - 2, idl);
	ds->hLine(x + 1, ye, x + 1 + w - 2, idl);
	ds->vLine(xe, y, y + h - 1, iur);
	ds->hLine(x + 1, y, x + 1 + w - 2, iur);

	x++; y++;
	w -= 2; h -= 2;

	Common::Rect fill(x, y, x + w, y + h);
	ds->fillRect(fill, solidColor);
}

void Interface::drawPanelButtonText(SURFACE *ds, InterfacePanel *panel, PanelButton *panelButton) {
	const char *text;
	int textId;
	int textWidth;
	int textHeight;
	Point point;
	int textColor;
	Rect rect;

	textId = panelButton->id;
	switch(panelButton->id) {
		case(kTextReadingSpeed):
			textId = kTextFast;
			break;			
		case(kTextMusic):
			textId = kTextOn;
			break;			
		case(kTextSound):
			textId = kTextOn;
			break;			
	}
	text = _vm->getTextString(textId);

	textWidth = _vm->_font->getStringWidth(MEDIUM_FONT_ID, text, 0, 0);
	textHeight = _vm->_font->getHeight(MEDIUM_FONT_ID);

	point.x = panel->x + panelButton->xOffset + (panelButton->width / 2) - (textWidth / 2);
	point.y = panel->y + panelButton->yOffset + (panelButton->height / 2) - (textHeight / 2);

	if (panelButton == panel->currentButton) {
		textColor = _vm->getDisplayInfo().verbTextActiveColor;
	} else {
		textColor = _vm->getDisplayInfo().verbTextColor;
	}

	panel->calcPanelButtonRect(panelButton, rect);
	drawButtonBox(ds, rect, kButton, panelButton->state > 0);

	_vm->_font->draw(MEDIUM_FONT_ID, ds, text, 0, point.x , point.y, 
		textColor, _vm->getDisplayInfo().verbTextShadowColor, FONT_SHADOW);
}

void Interface::drawPanelButtonArrow(SURFACE *ds, InterfacePanel *panel, PanelButton *panelButton) {
	Point point;
	int spriteNumber;

	if (panel->currentButton == panelButton) {
		if (panelButton->state != 0) {
			spriteNumber = panelButton->downSpriteNumber;
		} else {
			spriteNumber = panelButton->overSpriteNumber;
		}
	} else {
		spriteNumber = panelButton->upSpriteNumber;
	}
		
	point.x = panel->x + panelButton->xOffset;
	point.y = panel->y + panelButton->yOffset;

	_vm->_sprite->draw(ds, _vm->getDisplayClip(), _vm->_sprite->_mainSprites, spriteNumber, point, 256);
}

void Interface::drawVerbPanelText(SURFACE *ds, PanelButton *panelButton, int textColor, int textShadowColor) {
	const char *text;
	int textWidth;
	Point point;
	int textId;
	
	textId = verbTypeToTextStringsIdLUT[panelButton->id];
	
	if (textId == -1)
		error("textId == -1");

	text = _vm->getTextString(textId);
	
	textWidth = _vm->_font->getStringWidth(SMALL_FONT_ID, text, 0, 0);

	point.x = _mainPanel.x + panelButton->xOffset + 1 + (panelButton->width - 1 - textWidth) / 2;
	point.y = _mainPanel.y + panelButton->yOffset + 1;

	_vm->_font->draw(SMALL_FONT_ID, ds, text, 0, point.x , point.y, textColor, textShadowColor, (textShadowColor != 0) ? FONT_SHADOW : 0);
}
	

// Converse stuff
void Interface::converseInit(void) {
	for (int i = 0; i < CONVERSE_MAX_TEXTS; i++)
		_converseText[i].text = NULL;
	converseClear();
}

void Interface::converseClear(void) {
	for (int i = 0; i < CONVERSE_MAX_TEXTS; i++) {
		if (_converseText[i].text != NULL) {
			free(_converseText[i].text);
			_converseText[i].text = NULL;
		}
		_converseText[i].stringNum = -1;
		_converseText[i].replyId = 0;
		_converseText[i].replyFlags = 0;
		_converseText[i].replyBit = 0;
	}

	_converseTextCount = 0;
	_converseStrCount = 0;
	_converseStartPos = 0;
	_converseEndPos = 0;
	_conversePos = -1;
}

bool Interface::converseAddText(const char *text, int replyId, byte replyFlags, int replyBit) {
	int count = 0;         // count how many pieces of text per string
	int i;
	int len;
	byte c;

	assert(strlen(text) < CONVERSE_MAX_WORK_STRING);

	strncpy(_converseWorkString, text, CONVERSE_MAX_WORK_STRING);

	while (1) {
		len = strlen(_converseWorkString);

		for (i = len; i >= 0; i--) {
			c = _converseWorkString[i];

			if ((c == ' ' || c == '\0')
				&& _vm->_font->getStringWidth(SMALL_FONT_ID, _converseWorkString, i, 0) 
					<= CONVERSE_MAX_TEXT_WIDTH)
				break;
		}
		if (i < 0) {
			return true;
		}

		if (_converseTextCount == CONVERSE_MAX_TEXTS) {
			return true;
		}

		_converseText[_converseTextCount].text = (char *)malloc(i + 1);
		strncpy(_converseText[_converseTextCount].text, _converseWorkString, i);

		_converseText[_converseTextCount].text[i] = 0;
		_converseText[_converseTextCount].textNum = count;
		_converseText[_converseTextCount].stringNum =  _converseStrCount;
		_converseText[_converseTextCount].replyId =  replyId;
		_converseText[_converseTextCount].replyFlags =  replyFlags;
		_converseText[_converseTextCount].replyBit =  replyBit;

		_converseTextCount++;
		count++;

		if (len == i) 
			break;

		strncpy(_converseWorkString, &_converseWorkString[i + 1], len - i);
	}

	_converseStrCount++;

	return false;
}

void Interface::converseDisplayText() {
	int end;	

	_converseStartPos = 0;

	end = _converseTextCount - CONVERSE_TEXT_LINES;

	if (end < 0)
		end = 0;

	_converseEndPos = end;
	draw();
}


void Interface::converseSetTextLines(int row) {
	int pos = row + _converseStartPos;
	if (pos >= _converseTextCount)
		pos = -1;
	if (pos != _conversePos) {
		_conversePos = pos;
		draw();
	}
}

void Interface::converseDisplayTextLines(SURFACE *ds) {
	int relPos;
	byte foregnd;
	byte backgnd;
	byte bulletForegnd;
	byte bulletBackgnd;
	const char *str;
	char bullet[2] = {
		(char)0xb7, 0 
	};
	Rect rect(8, CONVERSE_TEXT_LINES * CONVERSE_TEXT_HEIGHT);
	
	assert(_conversePanel.buttonsCount >= 6);

	bulletForegnd = kITEColorGreen;
	bulletBackgnd = kITEColorBlack;
	
	rect.moveTo(_conversePanel.x + _conversePanel.buttons[0].xOffset, 
		_conversePanel.y + _conversePanel.buttons[0].yOffset);

	drawRect(ds, rect, kITEColorDarkGrey); //fill bullet place
	
	for (int i = 0; i < CONVERSE_TEXT_LINES; i++) {
		relPos = _converseStartPos + i;

		if (_converseTextCount <= relPos) {
			break;
		}

		if (_conversePos >= 0 && _converseText[_conversePos].stringNum == _converseText[relPos].stringNum) {
			foregnd = kITEColorBrightWhite;
			backgnd = (!_vm->leftMouseButtonPressed()) ? kITEColorDarkGrey : kITEColorGrey;
		} else {
			foregnd = kITEColorBlue;
			backgnd = kITEColorDarkGrey;
		}

		_conversePanel.calcPanelButtonRect(&_conversePanel.buttons[i], rect);
		rect.left += 8;
		drawRect(ds, rect, backgnd);

		str = _converseText[relPos].text;

		if (_converseText[relPos].textNum == 0) { // first entry
			_vm->_font->draw(SMALL_FONT_ID, ds, bullet, 1,
				rect.left - 6, rect.top, bulletForegnd, bulletBackgnd, FONT_SHADOW | FONT_DONTMAP);
		}
		_vm->_font->draw(SMALL_FONT_ID, ds, str, strlen(str),
			rect.left + 1, rect.top, foregnd, kITEColorBlack, FONT_SHADOW);
	}

	if (_converseStartPos != 0) {
		drawPanelButtonArrow(ds, &_conversePanel, _converseUpButton);
	}

	if (_converseStartPos != _converseEndPos) {
		drawPanelButtonArrow(ds, &_conversePanel, _converseDownButton);
	}
}

void Interface::converseChangePos(int chg) {
	if ((chg < 0 && _converseStartPos + chg >= 0) ||
		(chg > 0 && _converseStartPos  < _converseEndPos)) {
		_converseStartPos += chg;
		draw();
	}
}

void Interface::converseSetPos(int key) {
	Converse *ct;
	int selection = key - '1';

	if (selection >= _converseTextCount)
		return;

	converseSetTextLines(selection);

	ct = &_converseText[_conversePos];

	_vm->_script->finishDialog(ct->replyId, ct->replyFlags, ct->replyBit);

	if (_vm->_puzzle->isActive())
		_vm->_puzzle->handleReply(ct->replyId);

	_conversePos = -1;
}


void Interface::handleConverseUpdate(const Point& mousePoint) {
	bool changed;

	PanelButton *last = _conversePanel.currentButton;
	
	if (!_vm->mouseButtonPressed()) {			// remove pressed flag
		if (_converseUpButton) {
			_converseUpButton->state = 0;
			_converseDownButton->state = 0;
		}
	}

	_conversePanel.currentButton = converseHitTest(mousePoint);
	changed = last != _conversePanel.currentButton;
	

	if (_conversePanel.currentButton == NULL) {
		_conversePos = -1;
		if (changed) {
			draw();
		}
		return;
	}

	if (_conversePanel.currentButton->type == kPanelButtonConverseText) {
		converseSetTextLines(_conversePanel.currentButton->id);
	}
	
	if (_conversePanel.currentButton->type == kPanelButtonArrow) {
		if (_conversePanel.currentButton->state == 1) {
			//TODO: insert timeout catchup
			converseChangePos(_conversePanel.currentButton->id);
		}
		draw();
	}	
}


void Interface::handleConverseClick(const Point& mousePoint) {
	_conversePanel.currentButton = converseHitTest(mousePoint);

	if (_conversePanel.currentButton == NULL) {
		return;
	}

	if (_conversePanel.currentButton->type == kPanelButtonConverseText) {
		converseSetPos(_conversePanel.currentButton->ascii);
	}

	if (_conversePanel.currentButton->type == kPanelButtonArrow) {
		_conversePanel.currentButton->state = 1;
		converseChangePos(_conversePanel.currentButton->id);
	}	

}

void Interface::saveState(Common::OutSaveFile *out) {
	out->writeUint16LE(_inventoryCount);

	for (int i = 0; i < _inventoryCount; i++) {
		out->writeUint16LE(_inventory[i]);
	}
}

void Interface::loadState(Common::InSaveFile *in) {
	_inventoryCount = in->readUint16LE();

	for (int i = 0; i < _inventoryCount; i++) {
		_inventory[i] = in->readUint16LE();
	}
	
	updateInventory(0);
}

void Interface::mapPanelShow() {
	byte *resource;
	size_t resourceLength, imageLength;
	SURFACE *backBuffer;
	Point origin;
	byte *image;
	int imageWidth, imageHeight;
	int result;
	const byte *pal;
	PALENTRY cPal[PAL_ENTRIES];

	_vm->_gfx->showCursor(false);

	backBuffer = _vm->_gfx->getBackBuffer();

	origin.x = 0;
	origin.y = 0;

	result = RSC_LoadResource(_interfaceContext, RID_ITE_TYCHO_MAP, &resource, &resourceLength);
	if ((result != SUCCESS) || (resourceLength == 0)) {
		error("Interface::mapPanelShow(): unable to load Tycho map resource");
	}

	_vm->_gfx->getCurrentPal(_mapSavedPal);

	for (int i = 0; i < 6 ; i++) {
		_vm->_gfx->palToBlack(_mapSavedPal, 0.2 * i);
		_vm->_render->drawScene();
		_vm->_system->delayMillis(5);
	}

	_vm->_render->setFlag(RF_MAP);

	_vm->decodeBGImage(resource, resourceLength, &image, &imageLength, &imageWidth, &imageHeight);
	pal = _vm->getImagePal(resource, resourceLength);

	for (int i = 0; i < PAL_ENTRIES; i++) {
		cPal[i].red = *pal++;
		cPal[i].green = *pal++;
		cPal[i].blue = *pal++;
	}

	bufToSurface(backBuffer, image, imageWidth, imageHeight, NULL, &origin);

	// Evil Evil
	for (int i = 0; i < 6 ; i++) {
		_vm->_gfx->blackToPal(cPal, 0.2 * i);
		_vm->_render->drawScene();
		_vm->_system->delayMillis(5);
	}

	RSC_FreeResource(resource);
	free(image);

	setSaveReminderState(false);

	_mapPanelCrossHairState = true;
}

void Interface::mapPanelClean() {
	PALENTRY pal[PAL_ENTRIES];

	_vm->_gfx->getCurrentPal(pal);

	for (int i = 0; i < 6 ; i++) {
		_vm->_gfx->palToBlack(pal, 0.2 * i);
		_vm->_render->drawScene();
		_vm->_system->delayMillis(5);
	}

	_vm->_render->clearFlag(RF_MAP);
	setMode(kPanelMain);

	_vm->_gfx->showCursor(true);
	_vm->_render->drawScene();

	for (int i = 0; i < 6 ; i++) {
		_vm->_gfx->blackToPal(_mapSavedPal, 0.2 * i);
		_vm->_render->drawScene();
		_vm->_system->delayMillis(5);
	}
}

void Interface::mapPanelDrawCrossHair() {
	SURFACE *backBuffer;

	backBuffer = _vm->_gfx->getBackBuffer();
	_mapPanelCrossHairState = !_mapPanelCrossHairState;

	Point mapPosition = _vm->_isoMap->getMapPosition();
	Rect screen(_vm->getDisplayWidth(),  _vm->getSceneHeight());

	if (screen.contains(mapPosition)) {
		_vm->_sprite->draw(backBuffer, _vm->getDisplayClip(), _vm->_sprite->_mainSprites,
						   _mapPanelCrossHairState? RID_ITE_SPR_XHAIR1 : RID_ITE_SPR_XHAIR2,
						   mapPosition, 256);
	}
}

} // End of namespace Saga
