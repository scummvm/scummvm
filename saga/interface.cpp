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

static VERB_DATA I_VerbData[] = {
	{I_VERB_WALKTO, "verb_walkto", "Walk to", kVerbWalkTo},
	{I_VERB_LOOKAT, "verb_lookat", "Look at", kVerbLookAt},
	{I_VERB_PICKUP, "verb_pickup", "Pick up", kVerbPickup},
	{I_VERB_TALKTO, "verb_talkto", "Talk to", kVerbSpeakTo},
	{I_VERB_OPEN, "verb_open", "Open", kVerbOpen},
	{I_VERB_CLOSE, "verb_close", "Close", kVerbClose},
	{I_VERB_USE, "verb_use", "Use", kVerbUse},
	{I_VERB_GIVE, "verb_give", "Give", kVerbGive}
};

static InterfaceButton ITEMainPanel[] = {
	{5, 4, 46, 47, "Portrait", 0, 0, BUTTON_NONE, 0}, //TODO: remove?
	// "Walk To" and "Talk To" share button sprites
	{52, 4, 109, 14, "Walk To", 1, 2, BUTTON_VERB, I_VERB_WALKTO},
	{52, 15, 109, 25, "Look At", 3, 4, BUTTON_VERB, I_VERB_LOOKAT},
	{52, 26, 109, 36, "Pick Up", 5, 6, BUTTON_VERB, I_VERB_PICKUP},
	{52, 37, 109, 47, "Talk To", 1, 2, BUTTON_VERB, I_VERB_TALKTO},
	{110, 4, 166, 14, "Open", 7, 8, BUTTON_VERB, I_VERB_OPEN},
	{110, 15, 166, 25, "Close", 9, 10, BUTTON_VERB, I_VERB_CLOSE},
	{110, 26, 166, 36, "Use", 11, 12, BUTTON_VERB, I_VERB_USE},
	{110, 37, 166, 47, "Give", 13, 14, BUTTON_VERB, I_VERB_GIVE},

	{181, 6, 206, 24, "Inv1", 0, 0, BUTTON_NONE, 0},
	{213, 6, 240, 24, "Inv2", 0, 0, BUTTON_NONE, 0},
	{245, 6, 272, 24, "Inv3", 0, 0, BUTTON_NONE, 0},
	{277, 6, 304, 24, "Inv4", 0, 0, BUTTON_NONE, 0},
	{181, 27, 208, 45, "Inv5", 0, 0, BUTTON_NONE, 0},
	{213, 27, 240, 45, "Inv6", 0, 0, BUTTON_NONE, 0},
	{245, 27, 272, 45, "Inv7", 0, 0, BUTTON_NONE, 0},
	{277, 27, 304, 45, "Inv8", 0, 0, BUTTON_NONE, 0},
	{306, 6, 314, 11, "InvUp", 0, 0, BUTTON_NONE, 0},
	{306, 41, 314, 45, "InvDown", 0, 0, BUTTON_NONE, 0}
};


static InterfaceButton IHNMMainPanel[] = {
	{5, 4, 46, 47, "Portrait", 0, 0, 0, 0}
};

int Interface::registerLang(void) {
#if 0
	size_t i;

	for (i = 0; i < ARRAYSIZE(I_VerbData); i++) {
		if (CVAR_Register_S(I_VerbData[i].verb_str,
			I_VerbData[i].verb_cvar,
			NULL, CVAR_CFG, VERB_STRLIMIT) != SUCCESS) {

			return FAILURE;
		}

		assert(CVAR_Find(I_VerbData[i].verb_cvar) != NULL);
	}
#endif

	return SUCCESS;
}

Interface::Interface(SagaEngine *vm) : _vm(vm), _initialized(false) {
	int result;

	if (_initialized) {
		return;
	}

	_iThread = _vm->_script->SThreadCreate();
	if (_iThread == NULL) {
		warning("Interface::Interface(): Error creating script thread for game interface module");
		return;
	}

	// Load interface module resource file context
	_interfaceContext = _vm->getFileContext(GAME_RESOURCEFILE, 0);
	if (_interfaceContext == NULL) {
		return;
	}

	// Initialize interface data by game type
	if (_vm->getGameType() == GType_ITE) {
		// Load Inherit the Earth interface desc
		_mainPanel.buttons = ITEMainPanel;
		_mainPanel.nbuttons = ARRAYSIZE(ITEMainPanel);

	} else if (_vm->getGameType() == GType_IHNM) {
		// Load I Have No Mouth interface desc
		_mainPanel.buttons = IHNMMainPanel;
		_mainPanel.nbuttons = ARRAYSIZE(IHNMMainPanel);
	} else {
		return;
	}

	// Load command panel resource
	result = RSC_LoadResource(_interfaceContext, _vm->getResourceDescription()->command_panel_rn, &_mainPanel.res, &_mainPanel.res_len);
	if (result != SUCCESS) {
		return;
	}

	// Load dialogue panel resource
	result = RSC_LoadResource(_interfaceContext, _vm->getResourceDescription()->dialogue_panel_rn, &_conversePanel.res, &_conversePanel.res_len);
	if (result != SUCCESS) {
		return;
	}

	if (_vm->_sprite->loadList(RID_ITE_COMMAND_BUTTONSPRITES, _mainPanel.sprites) != SUCCESS) {
		error("Unable to load sprite list");
	}
	
	
	if (_vm->_sprite->loadList(RID_ITE_DEFAULT_PORTRAITS, _defPortraits) != SUCCESS) {
		error("Unable to load sprite list");
	}


	_vm->decodeBGImage(_mainPanel.res, _mainPanel.res_len, &_mainPanel.img,
					&_mainPanel.img_len, &_mainPanel.img_w, &_mainPanel.img_h);

	_vm->decodeBGImage(_conversePanel.res, _conversePanel.res_len,
					&_conversePanel.img, &_conversePanel.img_len,
					&_conversePanel.img_w, &_conversePanel.img_h);

	_mainPanel.x = 0;
	_mainPanel.y = 149;

	_conversePanel.x = 0;
	_conversePanel.y = 149;

	_mainPanel.set_button = 1;
	_leftPortrait = 0;
	_rightPortrait = 0;

	_activeVerb = I_VERB_WALKTO;

	_active = false;
	_panelMode = _lockedMode = kPanelNull;
	_savedMode = -1;
	_inMainMode = false;
	*_statusText = 0;

	_inventoryCount = 0;
	_inventorySize = ITE_INVENTORY_SIZE;

	_inventory = (uint16 *)calloc(_inventorySize, sizeof(uint16));
	if (_inventory == NULL) {
		return;
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

	draw();

	return SUCCESS;
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
		origin.y = _vm->getDisplayHeight() - _mainPanel.img_h;

		bufToSurface(backBuffer, _mainPanel.img, _mainPanel.img_w, _mainPanel.img_h, NULL, &origin);
		
	} else {
		base.x = _conversePanel.x;
		base.y = _conversePanel.y;

		origin.x = 0;
		origin.y = _vm->getDisplayHeight() - _mainPanel.img_h;

		bufToSurface(backBuffer, _conversePanel.img, _conversePanel.img_w,
						_conversePanel.img_h, NULL, &origin);
	}

	if (_panelMode == kPanelMain || _panelMode == kPanelConverse ||
		_lockedMode == kPanelMain || _lockedMode == kPanelConverse) {
			leftPortraitPoint.x = base.x + _vm->getDisplayInfo().leftPortraitX;
			leftPortraitPoint.y = base.y + _vm->getDisplayInfo().leftPortraitY;
			_vm->_sprite->draw(backBuffer, _defPortraits, _leftPortrait, leftPortraitPoint, 256);
		}
		

	if (!_inMainMode && _vm->getDisplayInfo().rightPortraitX >= 0) {
		rightPortraitPoint.x = base.x + _vm->getDisplayInfo().rightPortraitX;
		rightPortraitPoint.y = base.y + _vm->getDisplayInfo().rightPortraitY;

		_vm->_sprite->draw(backBuffer, _scenePortraits, _rightPortrait, rightPortraitPoint, 256);
	}

	if (_inMainMode) {
		drawInventory();
	}
	return SUCCESS;
}

int Interface::update(const Point& imousePt, int update_flag) {
	SURFACE *back_buf;

	int imouse_x, imouse_y;

	if (_vm->_scene->isInDemo() || _panelMode == kPanelFade)
		return SUCCESS;

	imouse_x = imousePt.x;
	imouse_y = imousePt.y;

	back_buf = _vm->_gfx->getBackBuffer();


	if (_panelMode == kPanelMain) { // FIXME: HACK
		// Update playfield space ( only if cursor is inside )
		if (imouse_y < _vm->getSceneHeight()) {
			// Mouse is in playfield space
			if (update_flag == UPDATE_MOUSEMOVE) {
				handlePlayfieldUpdate(back_buf, imousePt);
			} else if (update_flag == UPDATE_MOUSECLICK) {
				handlePlayfieldClick(back_buf, imousePt);
			}
		}

		// Update command space
		if (update_flag == UPDATE_MOUSEMOVE) {
			handleCommandUpdate(back_buf, imousePt);
		} else if (update_flag == UPDATE_MOUSECLICK) {
			handleCommandClick(back_buf, imousePt);
		}
	}

	drawStatusBar(back_buf);

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

int Interface::handleCommandClick(SURFACE *ds, const Point& imousePt) {
	int hit_button;
	int ibutton_num;

	int x_base;
	int y_base;

	Point button;

	int old_set_button;
	int set_button;

	hit_button = hitTest(imousePt, &ibutton_num);
	if (hit_button != SUCCESS) {
		// Clicking somewhere other than a button doesn't do anything
		return SUCCESS;
	}

	x_base = _mainPanel.x;
	y_base = _mainPanel.y;

	if (_mainPanel.buttons[ibutton_num].flags & BUTTON_SET) {
		old_set_button = _mainPanel.set_button;
		set_button = ibutton_num;
		_mainPanel.set_button = set_button;

		if (_mainPanel.buttons[set_button].flags & BUTTON_VERB) {
			_activeVerb = _mainPanel.buttons[ibutton_num].data;
		}

		if (_mainPanel.buttons[set_button].flags & BUTTON_BITMAP) {
			button.x = x_base + _mainPanel.buttons[set_button].x1;
			button.y = y_base + _mainPanel.buttons[set_button].y1;

			_vm->_sprite->draw(ds, _mainPanel.sprites, _mainPanel.buttons[set_button].
						active_sprite - 1, button, 256);
		}

		if (_mainPanel.buttons[old_set_button].flags & BUTTON_BITMAP) {
			button.x = x_base + _mainPanel.buttons[old_set_button].x1;
			button.y = y_base + _mainPanel.buttons[old_set_button].y1;

			_vm->_sprite->draw(ds, _mainPanel.sprites, _mainPanel.buttons[old_set_button].
						inactive_sprite - 1, button, 256);
		}
	}

	return SUCCESS;
}

int Interface::handleCommandUpdate(SURFACE *ds, const Point& imousePt) {
	int hit_button;
	int ibutton_num;

//	int button_w = 0;

/*	int verb_idx = 0;
	int string_w = 0;
	int color;
	int i;*/

	hit_button = inventoryTest(imousePt, &ibutton_num);

	if (hit_button == SUCCESS) {
		// Hovering over an inventory object
		return SUCCESS;
	}

/*	hit_button = hitTest(imousePt, &ibutton_num);

	if (hit_button == SUCCESS) {
		// Hovering over a command panel button
		setStatusText(I_VerbData[_activeVerb].verb_str);
	}

	for (i = 0; i < _mainPanel.nbuttons; i++) {
		if (!(_mainPanel.buttons[i].flags & BUTTON_LABEL)) {
			continue;
		}

		button_w = _mainPanel.buttons[i].x2 - _mainPanel.buttons[i].x1;

		verb_idx = _mainPanel.buttons[i].data;

		string_w = _vm->_font->getStringWidth(SMALL_FONT_ID, I_VerbData[verb_idx].verb_str, 0, 0);

		if (i == hit_button) {
			color = _iDesc.cmd_txt_hilitecol;
		} else {
			color = _iDesc.cmd_txt_col;
		}

		button.x = _mainPanel.x + _mainPanel.buttons[i].x1;
		button.y = _mainPanel.y + _mainPanel.buttons[i].y1;

		_vm->_font->draw(SMALL_FONT_ID, ds, I_VerbData[verb_idx].verb_str, 0,
				button.x + ((button_w / 2) - (string_w / 2)), button.y + 1,
				color, _iDesc.cmd_txt_shadowcol, FONT_SHADOW);

		if ((i == _mainPanel.set_button) && (_mainPanel.buttons[i].flags & BUTTON_BITMAP)) {
			_vm->_sprite->draw(ds, _mainPanel.sprites, _mainPanel.buttons[i].active_sprite - 1,
						button, 256);
		}
	}
*/
	return SUCCESS;
}

int Interface::handlePlayfieldClick(SURFACE *ds, const Point& imousePt) {
//	return SUCCESS;

	int objectNum;
	uint16 object_flags = 0;

	int script_num;
	ActorLocation location;

	objectNum = _vm->_scene->_objectMap->hitTest(imousePt);

	if (objectNum == -1) {
		// Player clicked on empty spot - walk here regardless of verb
		location.fromScreenPoint(imousePt);

		_vm->_actor->actorWalkTo(ID_PROTAG, location);
		return SUCCESS;
	}

	object_flags = _vm->_scene->_objectMap->getFlags(objectNum);

	if (object_flags & kHitZoneExit) { // FIXME. This is wrong
		if ((script_num = _vm->_scene->_objectMap->getEPNum(objectNum)) != -1) {
			// Set active verb in script module
			_vm->_script->putWord(4, 4, I_VerbData[_activeVerb].s_verb);

			// Execute object script if present
			if (script_num != 0) {
				_vm->_script->SThreadExecute(_iThread, script_num);
			}
		}
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

int Interface::hitTest(const Point& imousePt, int *ibutton) {
	InterfaceButton *buttons;

	int nbuttons;
	int xbase;
	int ybase;

	int i;

	buttons = _mainPanel.buttons;
	nbuttons = _mainPanel.nbuttons;

	xbase = _mainPanel.x;
	ybase = _mainPanel.y;

	for (i = 0; i < nbuttons; i++) {
		if ((imousePt.x >= (xbase + buttons[i].x1)) && (imousePt.x < (xbase + buttons[i].x2)) &&
			(imousePt.y >= (ybase + buttons[i].y1)) && (imousePt.y < (ybase + buttons[i].y2))) {
			*ibutton = i;
			return SUCCESS;
		}
	}

	*ibutton = -1;
	return FAILURE;
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

	int x = _vm->getDisplayInfo().inventoryX + _vm->getDisplayInfo().inventoryIconX;
	int y = _vm->getDisplayInfo().inventoryY + _vm->getDisplayInfo().inventoryIconY;
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

void Interface::converseSetPos(void) {
	Converse *ct;
	int selection = 1; // = keyStroke - '1'; // FIXME

	if (selection >= _converseTextCount)
		return;

	// FIXME: wait until Andrew defines proper color
	converseSetTextLines(selection, kColorBrightWhite, false);

	ct = &_converseText[_conversePos];
	// FIXME: TODO: finish dialog thread

	// FIXME: TODO: Puzzle

	_conversePos = -1;
}


} // End of namespace Saga
