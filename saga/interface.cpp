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
#include "saga/itedata.h"
#include "saga/rscfile_mod.h"
#include "saga/scene.h"
#include "saga/script.h"
#include "saga/sprite.h"

#include "saga/interface.h"

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

	if (_vm->_sprite->loadList(RID_ITE_COMMAND_BUTTONSPRITES, _mainPanel.sprites) != SUCCESS) { //TODO: move constant to ResourceDescription
		error("Interface::Interface(): Unable to load sprite list");
	}
		
	if (_vm->_sprite->loadList(RID_ITE_DEFAULT_PORTRAITS, _defPortraits) != SUCCESS) { //TODO: move constant to ResourceDescription
		error("Interface::Interface(): Unable to load sprite list");
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

	_active = false;
	_panelMode = _lockedMode = kPanelNull;
	_savedMode = -1;
	_inMainMode = false;
	*_statusText = 0;
	_statusOnceColor = -1;

	_inventoryCount = 0;
	_inventoryPos = 0;
	_inventoryStart = 0;
	_inventoryEnd = 0;
	_inventoryBox = 0;
	_inventorySize = ITE_INVENTORY_SIZE;

	_inventory = (uint16 *)calloc(_inventorySize, sizeof(uint16));
	if (_inventory == NULL) {
		error("Interface::Interface(): not enough memory");
	}

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
		if (_panelMode == kPanelMain)
			;// show save reminder
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

int Interface::setMode(int mode, bool force) {
	// TODO: Is this where we should hide/show the mouse cursor?
	int newMode = mode;

	if (mode == kPanelConverse) {
		_inMainMode = false;
	} else {
		if (mode == kPanelInventory) {
			_inMainMode = true;
			newMode = kPanelMain;
		}
	}

	// This lets us to prevents actors to pop up during initial
	// scene fade in.
	if (_savedMode != -1 && !force) {
		_savedMode = newMode;
		debug(0, "Saved mode: %d. my mode is %d", newMode, _panelMode);
	}
	else
		_panelMode = newMode;
	
	if (_panelMode == kPanelMain) {
		_mainPanel.currentButton = NULL;
	} else {
		if (_panelMode == kPanelConverse) {
			_conversePanel.currentButton = NULL;
			converseDisplayText();
		}
	}

	draw();

	return SUCCESS;
}

bool Interface::processKeyCode(int keyCode) {
	int i;
	PanelButton *panelButton;
	switch (_panelMode) {
	case kPanelNull:
		if (keyCode == 27) {// Esc
			if (_vm->_scene->isInDemo()) {
				_vm->_scene->skipScene();
			} else {
				_vm->_actor->abortAllSpeeches();
			}
			return true;
		}
		break;
	case kPanelMain:
		for (i = 0; i < _mainPanel.buttonsCount; i++) {
			panelButton = &_mainPanel.buttons[i];
			if (panelButton->keyChar == keyCode) {
				if (panelButton->type == kPanelButtonVerb) {
					_vm->_script->setVerb(panelButton->id);
				}			
				if (panelButton->type == kPanelButtonArrow) {
					inventoryChangePos(panelButton->id);
				}			
				return true;
			}
		}
		break;
	case kPanelConverse:
		switch (keyCode) {
		case 'x':
			setMode(kPanelInventory);
			// FIXME: puzzle
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
			converseSetPos(keyCode);
			break;

		}
	}
	return false;
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

int Interface::setLeftPortrait(int portrait) {
	_leftPortrait = portrait;
	draw();

	return SUCCESS;
}

int Interface::setRightPortrait(int portrait) {
	_rightPortrait = portrait;
	draw();

	return SUCCESS;
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

	_vm->_sprite->draw(backBuffer, _mainPanel.sprites, spriteNumber, point, 256);

	drawPanelButtonText(backBuffer, &_mainPanel, panelButton, textColor, _vm->getDisplayInfo().verbTextShadowColor);
}

int Interface::draw() {
	SURFACE *backBuffer;
	int i;

	Point leftPortraitPoint;
	Point rightPortraitPoint;	
	Point origin;

	backBuffer = _vm->_gfx->getBackBuffer();

	if (_vm->_scene->isInDemo() || _panelMode == kPanelFade)
		return SUCCESS;


	drawStatusBar();

	if (_panelMode == kPanelMain) {

		origin.x = 0;
		origin.y = _vm->getDisplayHeight() - _mainPanel.imageHeight;

		bufToSurface(backBuffer, _mainPanel.image, _mainPanel.imageWidth, _mainPanel.imageHeight, NULL, &origin);
		for (i = 0; i < kVerbTypesMax; i++) {
			if (_verbTypeToPanelButton[i] != NULL) {
				drawVerbPanel(backBuffer, _verbTypeToPanelButton[i]);
			}
		}
	} else {
		if (_panelMode == kPanelConverse) {	

			origin.x = 0;
			origin.y = _vm->getDisplayHeight() - _mainPanel.imageHeight;

			bufToSurface(backBuffer, _conversePanel.image, _conversePanel.imageWidth,
				_conversePanel.imageHeight, NULL, &origin);

			converseDisplayTextLines(backBuffer);
		}
	}

	if (_panelMode == kPanelMain || _panelMode == kPanelConverse ||
		_lockedMode == kPanelMain || _lockedMode == kPanelConverse) {
			leftPortraitPoint.x = _mainPanel.x + _vm->getDisplayInfo().leftPortraitXOffset;
			leftPortraitPoint.y = _mainPanel.y + _vm->getDisplayInfo().leftPortraitYOffset;
			_vm->_sprite->draw(backBuffer, _defPortraits, _leftPortrait, leftPortraitPoint, 256);
		}
		

	if (!_inMainMode && _vm->getDisplayInfo().rightPortraitXOffset >= 0) {
		rightPortraitPoint.x = _mainPanel.x + _vm->getDisplayInfo().rightPortraitXOffset;
		rightPortraitPoint.y = _mainPanel.y + _vm->getDisplayInfo().rightPortraitYOffset;

		// This looks like hack - particularly since it's only done for
		// the right-side portrait - and perhaps it is! But as far as I
		// can tell this is what the original engine does. And it keeps
		// ITE from crashing when entering the Elk King's court.

		if (_rightPortrait >= _scenePortraits.spriteCount)
			_rightPortrait = 0;

		_vm->_sprite->draw(backBuffer, _scenePortraits, _rightPortrait, rightPortraitPoint, 256);
	}

	drawInventory(backBuffer);
	return SUCCESS;
}

int Interface::update(const Point& mousePoint, int updateFlag) {
	
	if (_vm->_scene->isInDemo() || _panelMode == kPanelFade)
		return SUCCESS;

	if (_panelMode == kPanelMain) {
		if (updateFlag & UPDATE_MOUSEMOVE) {
			bool lastWasPlayfield = _lastMousePoint.y < _vm->getSceneHeight();
			if (mousePoint.y < _vm->getSceneHeight()) {
				if (!lastWasPlayfield) {
					handleCommandUpdate(mousePoint);
				}
				_vm->_script->whichObject(mousePoint);
			} else {
				if (lastWasPlayfield) {
					_vm->_script->setNonPlayfieldVerb();
				}
				handleCommandUpdate(mousePoint);
			}

		} else {

			if (updateFlag & UPDATE_MOUSECLICK) {
				if (mousePoint.y < _vm->getSceneHeight()) {
					_vm->_script->playfieldClick(mousePoint, (updateFlag & UPDATE_LEFTBUTTONCLICK) != 0);										
				} else {
					handleCommandClick(mousePoint);
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
		}
	}

	_lastMousePoint = mousePoint;
	return SUCCESS;
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

	_vm->_font->draw(SMALL_FONT_ID, backBuffer, _statusText, 0, _vm->getDisplayInfo().statusXOffset + (_vm->getDisplayInfo().statusWidth / 2) - (string_w / 2),
			_vm->getDisplayInfo().statusYOffset + _vm->getDisplayInfo().statusTextY, color, 0, 0);

}

void Interface::handleCommandClick(const Point& mousePoint) {

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
	}
}

void Interface::handleCommandUpdate(const Point& mousePoint) {
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
	
	if (panelButton != NULL) {
		if (panelButton->type == kPanelButtonArrow) {
			if (panelButton->state == 1) {
				//TODO: insert timeout catchup
				inventoryChangePos(panelButton->id);
			}
			draw();
		}

		if (panelButton->type == kPanelButtonInventory) {
			_vm->_script->whichObject(mousePoint);
		}		
	}

	bool changed = (panelButton != _mainPanel.currentButton);
	_mainPanel.currentButton = panelButton;
	if (changed) {
		draw();
	}		

}

PanelButton *Interface::verbHitTest(const Point& mousePoint) {
	PanelButton *panelButton;
	Rect rect;
	int i;
	for (i = 0; i < kVerbTypesMax; i++) {
		panelButton = _verbTypeToPanelButton[i];
		if (panelButton != NULL) {
			_mainPanel.calcPanelButtonRect(panelButton, rect);
			if (rect.contains(mousePoint))
				return panelButton;
		}
	}

	return NULL;
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

void Interface::addToInventory(int objectId, int pos) {
	if (pos != -1) { 
		_inventory[pos] = objectId;
		_inventoryCount = MAX(_inventoryCount, pos + 1);
		return;
	}

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
		_vm->_sprite->draw(backBuffer, _vm->_sprite->_mainSprites, obj->spriteListResourceId, rect, 256);
		
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

	_vm->_sprite->draw(ds, _vm->_sprite->_mainSprites, spriteNumber, point, 256);
}

void Interface::drawPanelButtonText(SURFACE *ds, InterfacePanel *panel, PanelButton *panelButton, int textColor, int textShadowColor) {
	const char *text;
	int textWidth;
	Point point;
	int textId;
	
	textId = verbTypeToTextStringsIdLUT[panelButton->id];
	
	if (textId == -1)
		error("textId == -1");

	text = _vm->getTextString(textId);

	
	textWidth = _vm->_font->getStringWidth(SMALL_FONT_ID, text, 0, 0);

	point.x = panel->x + panelButton->xOffset + (panelButton->width / 2) - (textWidth / 2);
	point.y = panel->y + panelButton->yOffset + 1;

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

	// FIXME: TODO: Puzzle

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
		converseSetPos(_conversePanel.currentButton->keyChar);
	}

	if (_conversePanel.currentButton->type == kPanelButtonArrow) {
		_conversePanel.currentButton->state = 1;
		converseChangePos(_conversePanel.currentButton->id);
	}	

}


} // End of namespace Saga
