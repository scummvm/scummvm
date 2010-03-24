/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "m4/m4_views.h"
#include "m4/dialogs.h"
#include "m4/events.h"
#include "m4/font.h"
#include "m4/globals.h"
#include "m4/mads_menus.h"
#include "m4/m4.h"
#include "m4/staticres.h"

namespace M4 {

static const int INV_ANIM_FRAME_SPEED = 8;
static const int INVENTORY_X = 160;
static const int INVENTORY_Y = 159;
static const int SCROLLER_DELAY = 200;

//--------------------------------------------------------------------------

MadsTextDisplay::MadsTextDisplay() {
	for (int i = 0; i < TEXT_DISPLAY_SIZE; ++i) {
		MadsTextDisplayEntry rec;
		rec.active = false;
		_entries.push_back(rec);
	}
}

int MadsTextDisplay::add(int xp, int yp, uint fontColour, int charSpacing, const char *msg, Font *font) {
	int usedSlot = -1;

	for (int idx = 0; idx < TEXT_DISPLAY_SIZE; ++idx) {
		if (!_entries[idx].active) {
			usedSlot = idx;

			_entries[idx].bounds.left = xp;
			_entries[idx].bounds.top = yp;
			_entries[idx].font = font;
			_entries[idx].msg = msg;
			_entries[idx].bounds.setWidth(font->getWidth(msg, charSpacing));
			_entries[idx].bounds.setHeight(font->getHeight());
			_entries[idx].colour1 = fontColour & 0xff;
			_entries[idx].colour2 = fontColour >> 8;
			_entries[idx].spacing = charSpacing;
			_entries[idx].expire = 1;
			_entries[idx].active = true;
			break;
		}
	}

	return usedSlot;
}

void MadsTextDisplay::draw(View *view) {
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		if (_entries[idx].active && (_entries[idx].expire >= 0)) {
			_entries[idx].font->setColours(_entries[idx].colour1, 0xFF,
				(_entries[idx].colour2 == 0) ? _entries[idx].colour1 : _entries[idx].colour2);
			_entries[idx].font->writeString(view, _entries[idx].msg, 
				_entries[idx].bounds.left, _entries[idx].bounds.top, _entries[idx].bounds.width(),
				_entries[idx].spacing);
		}
	}

	// Clear up any now text display entries that are to be expired
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		if (_entries[idx].expire < 0) {
			_entries[idx].active = false;
			_entries[idx].expire = 0;
		}
	}
}

//--------------------------------------------------------------------------

/**
 * Clears the entries list
 */
void ScreenObjects::clear() {
	_entries.clear();
}

/**
 * Adds a new entry to the list of screen objects
 */
void ScreenObjects::add(const Common::Rect &bounds, int layer, int idx, int category) {
	ScreenObjectEntry rec;
	rec.bounds = bounds;
	rec.layer = layer;
	rec.index = idx;
	rec.category = category;
	rec.active = true;

	_entries.push_back(rec);
}

/**
 * Scans the list for an element that contains the given mode. The result will be 1 based for a match,
 * with 0 indicating no entry was found
 */
int ScreenObjects::scan(int xp, int yp, int layer) {
	for (uint i = 0; i < _entries.size(); ++i) {
		if (_entries[i].active && _entries[i].bounds.contains(xp, yp) && (_entries[i].layer == layer))
			return i + 1;
	}

	// Entry not found
	return 0;
}

int ScreenObjects::scanBackwards(int xp, int yp, int layer) {
	for (uint i = _entries.size() - 1; ; --i) {
		if (_entries[i].active && _entries[i].bounds.contains(xp, yp) && (_entries[i].layer == layer))
			return i + 1;

		if (i == 0)
			break;
	}

	// Entry not found
	return 0;
}

void ScreenObjects::setActive(int category, int idx, bool active) {
	for (uint i = 0; i < _entries.size(); ++i) {
		if (_entries[i].active && (_entries[i].category == category) && (_entries[i].index == idx))
			_entries[i].active = active;
	}
}

//--------------------------------------------------------------------------

MadsView::MadsView(MadsM4Engine *vm, const Common::Rect &viewBounds, bool transparent): View(vm, viewBounds, transparent) {
	_spriteSlotsStart = 0;
}

MadsView::MadsView(MadsM4Engine *vm, int x, int y, bool transparent): View(vm, x, y, transparent) {
	_spriteSlotsStart = 0;
}

int MadsView::getSpriteSlotsIndex() {
	return _spriteSlotsStart++;
}

void MadsView::onRefresh(RectList *rects, M4Surface *destSurface) {
	// Draw text elements onto the view
	_textDisplay.draw(this);

	View::onRefresh(rects, destSurface);
}

/*--------------------------------------------------------------------------
 * MadsInterfaceView handles the user interface section at the bottom of
 * game screens in MADS games
 *--------------------------------------------------------------------------
 */

MadsInterfaceView::MadsInterfaceView(MadsM4Engine *vm): GameInterfaceView(vm, 
		Common::Rect(0, MADS_SURFACE_HEIGHT, vm->_screen->width(), vm->_screen->height())) {
	_screenType = VIEWID_INTERFACE;
	_highlightedElement = -1;
	_topIndex = 0;
	_selectedObject = -1;
	_cheatKeyCtr = 0;

	_objectSprites = NULL;
	_objectPalData = NULL;

	/* Set up the rect list for screen elements */
	// Actions
	for (int i = 0; i < 10; ++i)
		_screenObjects.addRect((i / 5) * 32 + 1, (i % 5) * 8 + MADS_SURFACE_HEIGHT + 2,
			((i / 5) + 1) * 32 + 3, ((i % 5) + 1) * 8 + MADS_SURFACE_HEIGHT + 2);

	// Scroller elements (up arrow, scroller, down arrow)
	_screenObjects.addRect(73, 160, 82, 167);
	_screenObjects.addRect(73, 168, 82, 190);
	_screenObjects.addRect(73, 191, 82, 198);

	// Inventory object names
	for (int i = 0; i < 5; ++i)
		_screenObjects.addRect(89, 158 + i * 8, 160, 166 + i * 8);

	// Full rectangle area for all vocab actions
	for (int i = 0; i < 5; ++i)
		_screenObjects.addRect(239, 158 + i * 8, 320, 166 + i * 8);
}

MadsInterfaceView::~MadsInterfaceView() {
	delete _objectSprites;
}

void MadsInterfaceView::setFontMode(InterfaceFontMode newMode) {
	switch (newMode) {
	case ITEM_NORMAL:
		_vm->_font->setColors(4, 4, 0xff);
		break;
	case ITEM_HIGHLIGHTED:
		_vm->_font->setColors(5, 5, 0xff);
		break;
	case ITEM_SELECTED:
		_vm->_font->setColors(6, 6, 0xff);
		break;
	}
}

void MadsInterfaceView::initialise() {
	// Build up the inventory list
	_inventoryList.clear();

	for (uint i = 0; i < _madsVm->globals()->getObjectsSize(); ++i) {
		MadsObject *obj = _madsVm->globals()->getObject(i);
		if (obj->roomNumber == PLAYER_INVENTORY)
			_inventoryList.push_back(i);
	}

	// If the inventory has at least one object, select it
	if (_inventoryList.size() > 0)
		setSelectedObject(_inventoryList[0]);
}

void MadsInterfaceView::setSelectedObject(int objectNumber) {
	char resName[80];

	// Load inventory resource
	if (_objectSprites) {
		_vm->_palette->deleteRange(_objectPalData);
		delete _objectSprites;
	}

	// Check to make sure the object is in the inventory, and also visible on-screen
	int idx = _inventoryList.indexOf(objectNumber);
	if (idx == -1) {
		// Object wasn't found, so return
		_selectedObject = -1;
		return;
	}

	// Found the object
	if (idx < _topIndex)
		_topIndex = idx;
	else if (idx >= (_topIndex + 5))
		_topIndex = MAX(0, idx - 4);

	_selectedObject = objectNumber;
	sprintf(resName, "*OB%.3dI.SS", objectNumber);

	Common::SeekableReadStream *data = _vm->res()->get(resName);
	_objectSprites = new SpriteAsset(_vm, data, data->size(), resName);
	_vm->res()->toss(resName);

	// Slot it into available palette space
	_objectPalData = _objectSprites->getRgbList();
	_vm->_palette->addRange(_objectPalData);
	_objectSprites->translate(_objectPalData, true);

	_objectFrameNumber = 0;
}

void MadsInterfaceView::addObjectToInventory(int objectNumber) {
	if (_inventoryList.indexOf(objectNumber) == -1) {
		_madsVm->globals()->getObject(objectNumber)->roomNumber = PLAYER_INVENTORY;
		_inventoryList.push_back(objectNumber);
	}

	setSelectedObject(objectNumber);
}

void MadsInterfaceView::onRefresh(RectList *rects, M4Surface *destSurface) {
	_vm->_font->setFont(FONT_INTERFACE_MADS);
	char buffer[100];

	// Check to see if any dialog is currently active
	bool dialogVisible = _vm->_viewManager->getView(LAYER_DIALOG) != NULL;

	// Highlighting logic for action list
	int actionIndex = 0;
	for (int x = 0; x < 2; ++x) {
		for (int y = 0; y < 5; ++y, ++actionIndex) {
			// Determine the font colour depending on whether an item is selected. Note that the first action,
			// 'Look', is always 'selected', even when another action is clicked on
			setFontMode((_highlightedElement == actionIndex) ? ITEM_HIGHLIGHTED :
				((actionIndex == 0) ? ITEM_SELECTED : ITEM_NORMAL));

			// Get the verb action and capitalise it
			const char *verbStr = _madsVm->globals()->getVocab(kVerbLook + actionIndex);
			strcpy(buffer, verbStr);
			if ((buffer[0] >= 'a') && (buffer[0] <= 'z')) buffer[0] -= 'a' - 'A';

			// Display the verb
			const Common::Rect r(_screenObjects[actionIndex]);
			_vm->_font->writeString(destSurface, buffer, r.left, r.top, r.width(), 0);
		}
	}

	// Check for highlighting of the scrollbar controls
	if ((_highlightedElement == SCROLL_UP) || (_highlightedElement == SCROLL_SCROLLER) || (_highlightedElement == SCROLL_DOWN)) {
		// Highlight the control's borders
		const Common::Rect r(_screenObjects[_highlightedElement]);
		destSurface->frameRect(r, 5);
	}

	// Draw the horizontal line in the scroller representing the current top selected
	const Common::Rect scroller(_screenObjects[SCROLL_SCROLLER]);
	int yP = (_inventoryList.size() < 2) ? 0 : (scroller.height() - 5) * _topIndex / (_inventoryList.size() - 1);
	destSurface->setColor(4);
	destSurface->hLine(scroller.left + 2, scroller.right - 3, scroller.top + 2 + yP);

	// List inventory items
	for (uint i = 0; i < 5; ++i) {
		if ((_topIndex + i) >= _inventoryList.size())
			break;

		const char *descStr = _madsVm->globals()->getVocab(_madsVm->globals()->getObject(
			_inventoryList[_topIndex + i])->descId);
		strcpy(buffer, descStr);
		if ((buffer[0] >= 'a') && (buffer[0] <= 'z')) buffer[0] -= 'a' - 'A';

		const Common::Rect r(_screenObjects[INVLIST_START + i]);

		// Set the highlighting of the inventory item
		if (_highlightedElement == (int)(INVLIST_START + i)) setFontMode(ITEM_HIGHLIGHTED);
		else if (_selectedObject == _inventoryList[_topIndex + i]) setFontMode(ITEM_SELECTED);
		else setFontMode(ITEM_NORMAL);

		// Write out it's description
		_vm->_font->writeString(destSurface, buffer, r.left, r.top, r.width(), 0);
	}

	// Handle the display of any currently selected object
	if (_objectSprites) {
		// Display object sprite. Note that the frame number isn't used directly, because it would result
		// in too fast an animation
		M4Sprite *spr = _objectSprites->getFrame(_objectFrameNumber / INV_ANIM_FRAME_SPEED);
		spr->copyTo(destSurface, INVENTORY_X, INVENTORY_Y, 0);

		if (!_madsVm->globals()->invObjectsStill && !dialogVisible) {
			// If objetcs are to animated, move to the next frame
			if (++_objectFrameNumber >= (_objectSprites->getCount() * INV_ANIM_FRAME_SPEED))
				_objectFrameNumber = 0;
		}

		// List the vocab actions for the currently selected object
		MadsObject *obj = _madsVm->globals()->getObject(_selectedObject);
		int yIndex = MIN(_highlightedElement - VOCAB_START, obj->vocabCount - 1);

		for (int i = 0; i < obj->vocabCount; ++i) {
			const Common::Rect r(_screenObjects[VOCAB_START + i]);

			// Get the vocab description and capitalise it
			const char *descStr = _madsVm->globals()->getVocab(obj->vocabList[i].vocabId);
			strcpy(buffer, descStr);
			if ((buffer[0] >= 'a') && (buffer[0] <= 'z')) buffer[0] -= 'a' - 'A';

			// Set the highlighting and display the entry
			setFontMode((i == yIndex) ? ITEM_HIGHLIGHTED : ITEM_NORMAL);
			_vm->_font->writeString(destSurface, buffer, r.left, r.top, r.width(), 0);
		}
	}
}

bool MadsInterfaceView::onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents) {
	MadsAction &act = _madsVm->scene()->getAction();

	// If the mouse isn't being held down, then reset the repeated scroll timer
	if (eventType != MEVENT_LEFT_HOLD)
		_nextScrollerTicks = 0;

	// Handle various event types
	switch (eventType) {
	case MEVENT_MOVE:
		// If the cursor isn't in "wait mode", don't do any processing
		if (_vm->_mouse->getCursorNum() == CURSOR_WAIT)
			return true;

		// Ensure the cursor is the standard arrow
		_vm->_mouse->setCursorNum(CURSOR_ARROW);

		// Check if any interface element is currently highlighted
		_highlightedElement = _screenObjects.find(Common::Point(x, y));

		return true;

	case MEVENT_LEFT_CLICK:
		// Left mouse click
		{
			// Check if an inventory object was selected
			if ((_highlightedElement >= INVLIST_START) && (_highlightedElement < (INVLIST_START + 5))) {
				// Ensure there is an inventory item listed in that cell
				uint idx = _highlightedElement - INVLIST_START;
				if ((_topIndex + idx) < _inventoryList.size()) {
					// Set the selected object
					setSelectedObject(_inventoryList[_topIndex + idx]);
				}
			} else if ((_highlightedElement >= ACTIONS_START) && (_highlightedElement < (ACTIONS_START + 10))) {
				// A standard action was selected
				int verbId = kVerbLook + (_highlightedElement - ACTIONS_START);
				warning("Selected action #%d", verbId);
				
			} else if ((_highlightedElement >= VOCAB_START) && (_highlightedElement < (VOCAB_START + 5))) {
				// A vocab action was selected
				MadsObject *obj = _madsVm->globals()->getObject(_selectedObject);
				int vocabIndex = MIN(_highlightedElement - VOCAB_START, obj->vocabCount - 1);
				if (vocabIndex >= 0) {
					act._actionMode = ACTMODE_OBJECT;
					act._actionMode2 = ACTMODE2_2;
					act._flags1 = obj->vocabList[1].flags1;
					act._flags2 = obj->vocabList[1].flags2;

					act._currentHotspot = _selectedObject;
					act._articleNumber = act._flags2;
				}
			}
		}
		return true;

	case MEVENT_LEFT_HOLD:
		// Left mouse hold
		// Handle the scroller - the up/down buttons allow for multiple actions whilst the mouse is held down
		if ((_highlightedElement == SCROLL_UP) || (_highlightedElement == SCROLL_DOWN)) {
			if ((_nextScrollerTicks == 0) || (g_system->getMillis() >= _nextScrollerTicks)) {
				// Handle scroll up/down action
				_nextScrollerTicks = g_system->getMillis() + SCROLLER_DELAY;

				if ((_highlightedElement == SCROLL_UP) && (_topIndex > 0))
					--_topIndex;
				if ((_highlightedElement == SCROLL_DOWN) && (_topIndex < (int)(_inventoryList.size() - 1)))
					++_topIndex;
			}
		}
		return true;

	case MEVENT_LEFT_DRAG:
		// Left mouse drag
		// Handle the the the scroller area that can be dragged to adjust the top displayed index
		if (_highlightedElement == SCROLL_SCROLLER) {
			// Calculate the new top index based on the Y position
			const Common::Rect r(_screenObjects[SCROLL_SCROLLER]);
			_topIndex = CLIP((int)(_inventoryList.size() - 1) * (y - r.top - 2) / (r.height() - 5),
				0, (int)_inventoryList.size() - 1);
		}
		return true;

	case KEVENT_KEY:
		if (_cheatKeyCtr == CHEAT_SEQUENCE_MAX)
			handleCheatKey(param1);
		handleKeypress(param1);
		return true;

	default:
		break;
	}

	return false;
}

bool MadsInterfaceView::handleCheatKey(int32 keycode) {
	switch (keycode) {
	case Common::KEYCODE_SPACE:
		// TODO: Move player to current destination
		return true;

	case Common::KEYCODE_t | (Common::KEYCODE_LALT):
	case Common::KEYCODE_t | (Common::KEYCODE_RALT):
	{
		// Teleport to room
		//Scene *sceneView = (Scene *)vm->_viewManager->getView(VIEWID_SCENE);


		return true;
	}

	default:
		break;
	}

	return false;
}

const char *CHEAT_SEQUENCE = "widepipe";

bool MadsInterfaceView::handleKeypress(int32 keycode) {
	int flags = keycode >> 24;
	int kc = keycode & 0xffff;

	// Capitalise the letter if necessary
	if (_cheatKeyCtr < CHEAT_SEQUENCE_MAX) {
		if ((flags & Common::KBD_CTRL) && (kc == CHEAT_SEQUENCE[_cheatKeyCtr])) {
			++_cheatKeyCtr;
			if (_cheatKeyCtr == CHEAT_SEQUENCE_MAX)
				Dialog::display(_vm, 22, cheatingEnabledDesc);
			return true;
		} else {
			_cheatKeyCtr = 0;
		}
	}

	// Handle the various keys
	if ((keycode == Common::KEYCODE_ESCAPE) || (keycode == Common::KEYCODE_F1)) {
		// Game menu
		_madsVm->globals()->dialogType = DIALOG_GAME_MENU;
		leaveScene();
		return false;
	} else if (flags & Common::KBD_CTRL) {
		// Handling of the different control key combinations
		switch (kc) {
		case Common::KEYCODE_i:
			// Mouse to inventory
			warning("TODO: Mouse to inventory");
			break;

		case Common::KEYCODE_k:
			// Toggle hotspots
			warning("TODO: Toggle hotspots");
			break;

		case Common::KEYCODE_p:
			// Player stats
			warning("TODO: Player stats");
			break;

		case Common::KEYCODE_q:
			// Quit game
			break;

		case Common::KEYCODE_s:
			// Activate sound
			warning("TODO: Activate sound");
			break;

		case Common::KEYCODE_u:
			// Rotate player
			warning("TODO: Rotate player");
			break;

		case Common::KEYCODE_v: {
			// Release version
			Dialog *dlg = new Dialog(_vm, GameReleaseInfoStr, GameReleaseTitleStr);
			_vm->_viewManager->addView(dlg);
			_vm->_viewManager->moveToFront(dlg);
			return false;
		}

		default:
			break;
		}
	} else if ((flags & Common::KBD_ALT) && (kc == Common::KEYCODE_q)) {
		// Quit Game

	} else {
		// Standard keypresses
		switch (kc) {
			case Common::KEYCODE_F2:
				// Save game
				_madsVm->globals()->dialogType = DIALOG_SAVE;
				leaveScene();
				break;
			case Common::KEYCODE_F3:
				// Restore game
				_madsVm->globals()->dialogType = DIALOG_RESTORE;
				leaveScene();
				break;
		}
	}
//DIALOG_OPTIONS
	return false;
}

void MadsInterfaceView::leaveScene() {
	// Close the scene
	View *view = _madsVm->_viewManager->getView(VIEWID_SCENE);
	_madsVm->_viewManager->deleteView(view);
}

} // End of namespace M4
