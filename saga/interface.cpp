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
#include "saga/objectdata.h"
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

	_iThread = _vm->_script->createThread();
	if (_iThread == NULL) {
		error("Interface::Interface(): Error creating script thread for game interface module");
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

int Interface::setStatusText(const char *new_txt) {
	assert(new_txt != NULL);

	strncpy(_statusText, new_txt, STATUS_TEXT_LEN);

	return SUCCESS;
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

	Point base;
	Point leftPortraitPoint;
	Point rightPortraitPoint;	
	Point origin;

	backBuffer = _vm->_gfx->getBackBuffer();

	if (_vm->_scene->isInDemo() || _panelMode == kPanelFade)
		return SUCCESS;


	drawStatusBar(backBuffer);

	if (_panelMode == kPanelMain) {
		base.x = _mainPanel.x;
		base.y = _mainPanel.y;

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
		base.x = _conversePanel.x;
		base.y = _conversePanel.y;

		origin.x = 0;
		origin.y = _vm->getDisplayHeight() - _mainPanel.imageHeight;

		bufToSurface(backBuffer, _conversePanel.image, _conversePanel.imageWidth,
						_conversePanel.imageHeight, NULL, &origin);
		converseDisplayText(0);
	}

	if (_panelMode == kPanelMain || _panelMode == kPanelConverse ||
		_lockedMode == kPanelMain || _lockedMode == kPanelConverse) {
			leftPortraitPoint.x = base.x + _vm->getDisplayInfo().leftPortraitXOffset;
			leftPortraitPoint.y = base.y + _vm->getDisplayInfo().leftPortraitYOffset;
			_vm->_sprite->draw(backBuffer, _defPortraits, _leftPortrait, leftPortraitPoint, 256);
		}
		

	if (!_inMainMode && _vm->getDisplayInfo().rightPortraitXOffset >= 0) {
		rightPortraitPoint.x = base.x + _vm->getDisplayInfo().rightPortraitXOffset;
		rightPortraitPoint.y = base.y + _vm->getDisplayInfo().rightPortraitYOffset;

		_vm->_sprite->draw(backBuffer, _scenePortraits, _rightPortrait, rightPortraitPoint, 256);
	}

	if (_inMainMode) {
		drawInventory();
	}
	return SUCCESS;
}

int Interface::update(const Point& mousePoint, int updateFlag) {
	SURFACE *backBuffer;

	if (_vm->_scene->isInDemo() || _panelMode == kPanelFade)
		return SUCCESS;

	
	backBuffer = _vm->_gfx->getBackBuffer();


	if (_panelMode == kPanelMain) {
		if (updateFlag & UPDATE_MOUSEMOVE) {
	
			if (mousePoint.y < _vm->getSceneHeight()) {
				//handlePlayfieldUpdate(backBuffer, imousePointer);
				_vm->_script->whichObject(mousePoint);
			} else {
				if (_lastMousePoint.y < _vm->getSceneHeight()) {
					_vm->_script->setNonPlayfieldVerb();
				}
				handleCommandUpdate(backBuffer, mousePoint);
			}

		} else {

			if (updateFlag & UPDATE_MOUSECLICK) {
				if (mousePoint.y < _vm->getSceneHeight()) {
					//handlePlayfieldClick(backBuffer, mousePoint);
					_vm->_script->playfieldClick(mousePoint, (updateFlag & UPDATE_LEFTBUTTONCLICK) != 0);
										
				} else {
					handleCommandClick(backBuffer, mousePoint);
				}
			}
		}
	}

	drawStatusBar(backBuffer);
	_lastMousePoint = mousePoint;
	return SUCCESS;
}

int Interface::drawStatusBar(SURFACE *ds) {
	Rect rect;

	int string_w;

	// Disable this for IHNM for now, since that game uses the full screen
	// in some cases.

	if (_vm->getGameType() == GType_IHNM) {
		return SUCCESS;
	}


	// Erase background of status bar
	rect.left = 0;
	rect.top = _vm->getDisplayInfo().statusY;
	rect.right = _vm->getDisplayWidth();
	rect.bottom = _vm->getDisplayInfo().statusY + _vm->getDisplayInfo().statusHeight;

	drawRect(ds, &rect, _vm->getDisplayInfo().statusBGColor);

	string_w = _vm->_font->getStringWidth(SMALL_FONT_ID, _statusText, 0, 0);

	_vm->_font->draw(SMALL_FONT_ID, ds, _statusText, 0, (_vm->getDisplayInfo().statusWidth / 2) - (string_w / 2),
			_vm->getDisplayInfo().statusY + _vm->getDisplayInfo().statusTextY, _vm->getDisplayInfo().statusTextColor, 0, 0);

	return SUCCESS;
}

void Interface::handleCommandClick(SURFACE *ds, const Point& mousePoint) {

	PanelButton *panelButton;

	panelButton = verbHitTest(mousePoint);
	if (panelButton) {
		_vm->_script->setVerb(panelButton->id);
		return;
	}
}

void Interface::handleCommandUpdate(SURFACE *ds, const Point& mousePoint) {
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

int Interface::handlePlayfieldClick(SURFACE *ds, const Point& imousePt) {
//	return SUCCESS;

	int objectNum;
	uint16 object_flags = 0;

//	int script_num;
	Location location;

	objectNum = _vm->_scene->_objectMap->hitTest(imousePt);

	if (objectNum == -1) {
		// Player clicked on empty spot - walk here regardless of verb
		location.fromScreenPoint(imousePt);

		_vm->_actor->actorWalkTo(ID_PROTAG, location);
		return SUCCESS;
	}

	object_flags = _vm->_scene->_objectMap->getFlags(objectNum);

	if (object_flags & kHitZoneExit) { // FIXME. This is wrong
/*		if ((script_num = _vm->_scene->_objectMap->getEPNum(objectNum)) != -1) {
			// Set active verb in script module
			_vm->_script->putWord(4, 4, I_VerbData[_activeVerb].s_verb);

			// Execute object script if present
			if (script_num != 0) {
				_vm->_script->SThreadExecute(_iThread, script_num);
			}
		}*/
	} else {
		// Not a normal scene object - walk to it as if it weren't there
		location.fromScreenPoint(imousePt);

		_vm->_actor->actorWalkTo(ID_PROTAG, location);
	}

	return SUCCESS;
}

int Interface::handlePlayfieldUpdate(SURFACE *ds, const Point& imousePt) {
	return SUCCESS;
	/*
	const char *object_name;
	int objectNum;
	uint16 object_flags = 0;

	char new_status[STATUS_TEXT_LEN];

	new_status[0] = 0;

	objectNum = _vm->_scene->_objectMap->hitTest(imousePt);

	if (objectNum == -1) {
		// Cursor over nothing - just display current verb
		setStatusText(I_VerbData[_activeVerb].verb_str);
		return SUCCESS;
	}

	object_flags = _vm->_scene->_objectMap->getFlags(objectNum);
	object_name = _vm->_scene->_objectMap->getName(objectNum);

	if (object_flags & OBJECT_EXIT) { // FIXME. This is wrong
		// Normal scene object - display as subject of verb
		snprintf(new_status, STATUS_TEXT_LEN, "%s %s", I_VerbData[_activeVerb].verb_str, object_name);
	} else {
		// Not normal scene object - override verb as we can only
		// walk to this object
		snprintf(new_status, STATUS_TEXT_LEN, "%s %s", I_VerbData[I_VERB_WALKTO].verb_str, object_name);
	}

	setStatusText(new_status);

	return SUCCESS;
	*/
}

PanelButton *Interface::verbHitTest(const Point& mousePoint) {
	PanelButton *panelButton;
	Rect rect;
	int i;
	for (i = 0; i < kVerbTypesMax; i++) {
		panelButton = _verbTypeToPanelButton[i];
		if (panelButton != NULL) {
			rect.left = _mainPanel.x + panelButton->xOffset;
			rect.right = rect.left + panelButton->width;
			rect.top = _mainPanel.y + panelButton->yOffset;
			rect.bottom = rect.top + panelButton->height;
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
		if (_inventory[i] >= ARRAYSIZE(ObjectTable)) {
			continue;
		}
		drawPoint.x = x + col * width;
		drawPoint.y = y + row * height;

		_vm->_sprite->draw(back_buf, _vm->_sprite->_mainSprites,
			ObjectTable[_inventory[i]].spritelistRn,
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
void Interface::converseClear(void) {
	for (int i = 0; i < CONVERSE_MAX_TEXTS; i++) {
		if (_converseText[i].text)
			free(_converseText[i].text);
		_converseText[i].text = NULL;
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

	for (int i = 0; i < CONVERSE_TEXT_LINES; i++) {
		_converseLastColors[0][i] = 0;
		_converseLastColors[1][i] = 0;
	}
}

bool Interface::converseAddText(const char *text, int replyId, byte replyFlags, int replyBit) {
	int count = 0;         // count how many pieces of text per string
	char temp[128];

	assert(strlen(text) < 128);

	strncpy(temp, text, 128);

	while (1) {
		int i;
		int len = strlen(temp);

		for (i = len; i >= 0; i--) {
			byte c = temp[i];

			if ((c == ' ' || c == '\0')
				&& _vm->_font->getStringWidth(SMALL_FONT_ID, temp, i, 0) 
					<= CONVERSE_MAX_TEXT_WIDTH)
				break;
		}
		if (i < 0) 
			return true;

		if (_converseTextCount == CONVERSE_MAX_TEXTS)
			return true;

		_converseText[_converseTextCount].text = (char *)malloc(i + 1);
		strncpy(_converseText[_converseTextCount].text, temp, i);

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

		strncpy(temp, &temp[i + 1], len - i);
	}

	_converseStrCount++;

	return false;
}

enum converseColors {
	kColorBrightWhite = 0x2,
	kColorGrey = 0xa,
	kColorDarkGrey = 0xb,
	kColorGreen = 0xba,
	kColorBlack = 0xf,
	kColorBlue = 0x93
};

void Interface::converseDisplayText(int pos) {
	int end;

	if (pos >= _converseTextCount)
		pos = _converseTextCount - 1;
	if (pos < 0)
		pos = 0;

	_converseStartPos = pos;

	end = _converseTextCount - CONVERSE_TEXT_LINES;

	if (end < 0)
		end = 0;

	_converseEndPos = end;

	converseDisplayTextLine(kColorBrightWhite, false, true);
}


void Interface::converseSetTextLines(int row, int textcolor, bool btnDown) {
	_conversePos = row + _converseStartPos;
	if (_conversePos >= _converseTextCount)
		_conversePos = -1;

	converseDisplayTextLine(textcolor, btnDown, false);
}

void Interface::converseDisplayTextLine(int textcolor, bool btnDown, bool rebuild) {
	int x = 52; // FIXME: remove hardcoded value
	int y = 6; // FIXME: remove hardcoded value
	int pos = _converseStartPos;
	byte textcolors[2][CONVERSE_TEXT_LINES];
	SURFACE *ds;

	ds = _vm->_gfx->getBackBuffer(); // FIXME: probably best to move this out

	for (int i = 0; i < CONVERSE_TEXT_LINES; i++) {
		int relpos = pos + i;

		if (_conversePos >= 0
			&& _converseText[_conversePos].stringNum
						== _converseText[relpos].stringNum) {
			textcolors[0][i] = textcolor;
			textcolors[1][i] = (!btnDown) ? kColorDarkGrey : kColorGrey;
		} else {
			textcolors[0][i] = kColorBlue;
			textcolors[1][i] = kColorDarkGrey;
		}
	}
		// if no colors have changed, exit
	if (!rebuild && memcmp(textcolors, _converseLastColors, sizeof(textcolors)) == 0)
		return;

	memcpy(_converseLastColors, textcolors, sizeof(textcolors));

	Rect rect(8, CONVERSE_TEXT_LINES * CONVERSE_TEXT_HEIGHT);
	int scrx = _conversePanel.x + x;

	rect.moveTo(_conversePanel.x + x, _conversePanel.y + y);

	drawRect(ds, &rect, kColorDarkGrey);

	rect.top = rect.left = 0;
	rect.right = CONVERSE_MAX_TEXT_WIDTH;
	rect.bottom = CONVERSE_TEXT_HEIGHT;

	for (int i = 0; i < CONVERSE_TEXT_LINES; i++) {
		byte foregnd = textcolors[0][i];
		byte backgnd = textcolors[1][i];
		int relpos = pos + i;

		rect.moveTo(_conversePanel.x + x + 7 + 1, 
					_conversePanel.y + y + i * CONVERSE_TEXT_HEIGHT);

		drawRect(ds, &rect, backgnd);

		if (_converseTextCount > i) {
			const char *str = _converseText[relpos].text;
			char bullet[] = { (char)0xb7, 0 };
			int scry = i * CONVERSE_TEXT_HEIGHT + _conversePanel.y + y;
			byte tcolor, bcolor;

			if (_converseText[relpos].textNum == 0) { // first entry
				tcolor = kColorGreen;
				bcolor = kColorBlack;
				_vm->_font->draw(SMALL_FONT_ID, ds, bullet, strlen(bullet),
								 scrx + 2, scry, tcolor, bcolor, FONT_SHADOW | FONT_DONTMAP);
			}
			_vm->_font->draw(SMALL_FONT_ID, ds, str, strlen(str),
							 scrx + 9, scry, foregnd, kColorBlack, FONT_SHADOW);
		}
	}

	// FIXME: TODO: arrows
}

void Interface::converseChangePos(int chg) {
	if ((chg < 0 && _converseStartPos + chg >= 0) ||
		(chg > 0 && _converseStartPos  < _converseEndPos)) {
		_converseStartPos += chg;
		converseDisplayTextLine(kColorBlue, false, true);
	}
}

void Interface::converseSetPos(int key) {
	Converse *ct;
	int selection = key - '1';

	if (selection >= _converseTextCount)
		return;

	// FIXME: wait until Andrew defines proper color
	converseSetTextLines(selection, kColorBrightWhite, false);

	ct = &_converseText[_conversePos];

	//finishDialog( ct->replyID, ct->replyFlags, ct->replyBit );
	// FIXME: TODO: finish dialog thread

	// FIXME: TODO: Puzzle

	_conversePos = -1;
}


} // End of namespace Saga
