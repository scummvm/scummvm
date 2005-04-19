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


	_mainPanel.x = 0;		//TODO: move constant to DisplayInfo
	_mainPanel.y = 149;
	_mainPanel.currentButton = NULL;

	_conversePanel.x = 0;	//TODO: move constant to DisplayInfo
	_conversePanel.y = 149;
	_conversePanel.currentButton = NULL;

	_leftPortrait = 0;
	_rightPortrait = 0;

	_active = false;
	_panelMode = _lockedMode = kPanelNull;
	_savedMode = -1;
	_inMainMode = false;
	*_statusText = 0;
	_statusOnceColor = -1;

	_inventoryCount = 0;
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
		for (i = 0; i < kVerbTypesMax; i++) {
			if (_verbTypeToPanelButton[i] != NULL) {
				if (_verbTypeToPanelButton[i]->keyChar == keyCode) {
					_vm->_script->setVerb(_verbTypeToPanelButton[i]->id);
					return true;
				}
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
		//here we will draw verbs
		for (i = 0; i < kVerbTypesMax; i++) {
			if (_verbTypeToPanelButton[i] != NULL) {
				drawPanelButtonText(backBuffer, &_mainPanel, _verbTypeToPanelButton[i], _vm->getDisplayInfo().verbTextColor, _vm->getDisplayInfo().verbTextShadowColor);
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

		_vm->_sprite->draw(backBuffer, _scenePortraits, _rightPortrait, rightPortraitPoint, 256);
	}

	if (_inMainMode) {
		drawInventory();
	}
	return SUCCESS;
}

int Interface::update(const Point& mousePoint, int updateFlag) {
	
	if (_vm->_scene->isInDemo() || _panelMode == kPanelFade)
		return SUCCESS;

	if (_panelMode == kPanelMain) {
		if (updateFlag & UPDATE_MOUSEMOVE) {
	
			if (mousePoint.y < _vm->getSceneHeight()) {
				_vm->_script->whichObject(mousePoint);
			} else {
				if (_lastMousePoint.y < _vm->getSceneHeight()) {
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
	rect.left = 0;
	rect.top = _vm->getDisplayInfo().statusY;
	rect.right = _vm->getDisplayWidth();
	rect.bottom = _vm->getDisplayInfo().statusY + _vm->getDisplayInfo().statusHeight;

	drawRect(backBuffer, &rect, _vm->getDisplayInfo().statusBGColor);

	string_w = _vm->_font->getStringWidth(SMALL_FONT_ID, _statusText, 0, 0);

	if (_statusOnceColor == -1)
		color = _vm->getDisplayInfo().statusTextColor;
	else
		color = _statusOnceColor;

	_vm->_font->draw(SMALL_FONT_ID, backBuffer, _statusText, 0, (_vm->getDisplayInfo().statusWidth / 2) - (string_w / 2),
			_vm->getDisplayInfo().statusY + _vm->getDisplayInfo().statusTextY, color, 0, 0);

}

void Interface::handleCommandClick(const Point& mousePoint) {

	PanelButton *panelButton;

	panelButton = verbHitTest(mousePoint);
	if (panelButton) {
		_vm->_script->setVerb(panelButton->id);
		return;
	}
}

void Interface::handleCommandUpdate(const Point& mousePoint) {
	PanelButton *panelButton;

	panelButton = verbHitTest(mousePoint);
	if (_mainPanel.currentButton != panelButton) {
		if (_mainPanel.currentButton) {
			drawVerb(_mainPanel.currentButton->id, 0);
		}
		if (panelButton) {
			drawVerb(panelButton->id, 1);			
		}
	}
	_mainPanel.currentButton = panelButton;
	if (panelButton) {
		return;
	}
/*	hit_button = inventoryTest(imousePointer, &ibutton_num);

	if (hit_button == SUCCESS) {
		// Hovering over an inventory object
		return SUCCESS;
	}*/

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

void Interface::addToInventory(int sprite) {
	if (_inventoryCount < _inventorySize) {
		for (int i = _inventoryCount; i > 0; i--) {
			_inventory[i] = _inventory[i - 1];
		}

		_inventory[0] = sprite;
		_inventoryCount++;
		draw();
	}
}

void Interface::removeFromInventory(int sprite) {
	for (int i = 0; i < _inventoryCount; i++) {
		if (_inventory[i] == sprite) {
			int j;

			for (j = i; i < _inventoryCount; j++) {
				_inventory[j] = _inventory[j + 1];
			}

			_inventory[j] = 0;
			_inventoryCount--;
			draw();
			return;
		}
	}
}

void Interface::drawInventory() {
	if (_panelMode != kPanelMain)
		return;
	SURFACE *back_buf = _vm->_gfx->getBackBuffer();

	// TODO: Inventory scrolling

	int row = 0;
	int col = 0;

	int x = _vm->getDisplayInfo().inventoryX + _vm->getDisplayInfo().inventoryIconXOffset;
	int y = _vm->getDisplayInfo().inventoryY + _vm->getDisplayInfo().inventoryIconYOffset;
	int width = _vm->getDisplayInfo().inventoryIconWidth + _vm->getDisplayInfo().inventoryXSpacing;
	int height = _vm->getDisplayInfo().inventoryIconHeight + _vm->getDisplayInfo().inventoryYSpacing;
	Point drawPoint;

	for (int i = 0; i < _inventoryCount; i++) {
		if (!_vm->_actor->validObjId(_vm->_actor->objIndexToId(_inventory[i]))) {
			continue;
		}
		drawPoint.x = x + col * width;
		drawPoint.y = y + row * height;

		_vm->_sprite->draw(back_buf, _vm->_sprite->_mainSprites,
			_vm->_actor->getObj(_vm->_actor->objIndexToId(_inventory[i]))->spriteListResourceId,
			drawPoint, 256);

		if (++col >= _vm->getDisplayInfo().inventoryColumns) {
			if (++row >= _vm->getDisplayInfo().inventoryRows) {
				break;
			}
			col = 0;
		}
	}
}

int Interface::inventoryTest(const Point& imousePt, int *ibutton) {
	int row = 0;
	int col = 0;

	int xbase = _vm->getDisplayInfo().inventoryX;
	int ybase = _vm->getDisplayInfo().inventoryY;
	int width = _vm->getDisplayInfo().inventoryIconWidth + _vm->getDisplayInfo().inventoryXSpacing;
	int height = _vm->getDisplayInfo().inventoryIconHeight + _vm->getDisplayInfo().inventoryYSpacing;

	for (int i = 0; i < _inventoryCount; i++) {
		int x = xbase + col * width;
		int y = ybase + row * height;

		if (imousePt.x >= x && imousePt.x < x + _vm->getDisplayInfo().inventoryIconWidth && imousePt.y >= y && imousePt.y < y + _vm->getDisplayInfo().inventoryIconHeight) {
			*ibutton = i;
			return SUCCESS;
		}

		if (++col >= _vm->getDisplayInfo().inventoryColumns) {
			if (++row >= _vm->getDisplayInfo().inventoryRows) {
				break;
			}
			col = 0;
		}
	}

	return FAILURE;
}


void Interface::drawVerb(int verb, int state) {
	SURFACE *backBuffer;
	PanelButton * panelButton;
	PanelButton * rightButtonVerbPanelButton;
	PanelButton * currentVerbPanelButton;
	int textColor;
	int spriteNumber;
	Point point;

	backBuffer = _vm->_gfx->getBackBuffer();

	panelButton = getPanelButtonByVerbType(verb);
	rightButtonVerbPanelButton = getPanelButtonByVerbType(_vm->_script->getRightButtonVerb());
	currentVerbPanelButton = getPanelButtonByVerbType(_vm->_script->getCurrentVerb());

	if (panelButton == NULL) {
		warning("panelButton == NULL");
		return;
	}
	if (state == 2) {
		state = (_mainPanel.currentButton == panelButton) ? 1 : 0;
	}

	if (state) {
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

void Interface::drawPanelButtonArrow(SURFACE *ds, InterfacePanel *panel, PanelButton *panelButton) {
	Point point;
	int spriteNumber;

	if (panel->currentButton == panelButton) {
		if (panelButton->flag != 0) {
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

	drawRect(ds, &rect, kITEColorDarkGrey); //fill bullet place
	
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
		drawRect(ds, &rect, backgnd);

		str = _converseText[relPos].text;

		if (_converseText[relPos].textNum == 0) { // first entry
			_vm->_font->draw(SMALL_FONT_ID, ds, bullet, 1,
				rect.left - 6, rect.top, bulletForegnd, bulletBackgnd, FONT_SHADOW | FONT_DONTMAP);
		}
		_vm->_font->draw(SMALL_FONT_ID, ds, str, strlen(str),
			rect.left + 1, rect.top, foregnd, kITEColorBlack, FONT_SHADOW);
	}

	if (_converseStartPos != 0) {
		drawPanelButtonArrow(ds, &_conversePanel, &_conversePanel.buttons[4]);
	}

	if (_converseStartPos != _converseEndPos) {
		drawPanelButtonArrow(ds, &_conversePanel, &_conversePanel.buttons[5]);
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

PanelButton *Interface::converseHitTest(const Point& mousePoint) {
	PanelButton *panelButton;
	Rect rect;
	int i;
	for (i = 0; i < _conversePanel.buttonsCount; i++) {
		panelButton = &_conversePanel.buttons[i];
		if (panelButton != NULL) {
			_conversePanel.calcPanelButtonRect(panelButton, rect);
			if (rect.contains(mousePoint)) {
				return panelButton;
			}
		}
	}

	return NULL;
}

void Interface::handleConverseUpdate(const Point& mousePoint) {
	bool changed;

	PanelButton *last = _conversePanel.currentButton;
	
	if (!_vm->mouseButtonPressed()) {			// remove pressed flag
		_conversePanel.buttons[4].flag = 0;
		_conversePanel.buttons[5].flag = 0;
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
		if (_conversePanel.currentButton->flag == 1) {
			//TODO: insert timeout catchup
			converseChangePos((_conversePanel.currentButton->id == 0) ? -1 : 1);
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
		_conversePanel.currentButton->flag = 1;
		converseChangePos((_conversePanel.currentButton->id == 0) ? -1 : 1);
	}	

}


} // End of namespace Saga
