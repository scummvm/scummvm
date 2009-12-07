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

#include "common/algorithm.h"		// for find()
#include "gui/dialog.h"
#include "gui/message.h"

#include "m4/m4_menus.h"
#include "m4/m4_views.h"
#include "m4/woodscript.h"
#include "m4/midi.h"

namespace M4 {

const char *EmptySaveString = "<empty>";

//--------------------------------------------------------------------------
// Callback methods
//
// Following is a set of callback methods used to handle the execution
// of buttons in the various dialogs
//--------------------------------------------------------------------------

// General function which simply closes the active menu

void OrionCallbacks::closeMenuFn(DialogView *view, MenuObject *item) {
	view->close();
}

void OrionCallbacks::closeMenuFn(OrionMenuView *view) {
	closeMenuFn(view, NULL);
}

/* Game menu functions */

void OrionCallbacks::gameOptionsMenuFn(DialogView *view, MenuObject *item) {
	view->vm()->loadMenu(OPTIONS_MENU);
	view->close();
}

void OrionCallbacks::gameSaveGameFn(DialogView *view, MenuObject *item) {
	view->vm()->loadMenu(SAVE_MENU);
	view->close();
}

void OrionCallbacks::gameLoadGameFn(DialogView *view, MenuObject *item) {
	view->vm()->loadMenu(LOAD_MENU);
	view->close();
}

void OrionCallbacks::gameExitFn(DialogView *view, MenuObject *item) {
	view->vm()->_events->quitFlag = true;
	view->close();
}

/* Options menu */

void OrionCallbacks::optionsDigiSliderFn(DialogView *view, MenuObject *item) {
	// Digi volume slider changed
	int percent = ((MenuHorizSlider *) item)->percent();

	view->vm()->_sound->setVolume(percent * 255 / 100);
}

void OrionCallbacks::optionsMidiSliderFn(DialogView *view, MenuObject *item) {
	// Midi volume slider changed
	int percent = ((MenuHorizSlider *) item)->percent();

	view->vm()->midi()->setVolume(percent * 255 / 100);
}

void OrionCallbacks::optionsScrollingFn(DialogView *view, MenuObject *item) {
	// TODO: Change current Digi volume settings here
}

void OrionCallbacks::optionsCancelFn(DialogView *view, MenuObject *item) {
	// TODO: Reset original option settings here
	OrionMenuView *vw = (OrionMenuView *) view;

	vw->vm()->midi()->setVolume(vw->_originalMidiVolume);

	vw->vm()->loadMenu(GAME_MENU);
	vw->close();
}

void OrionCallbacks::optionsDoneFn(DialogView *view, MenuObject *item) {
	view->vm()->loadMenu(GAME_MENU);
	view->close();
}

void OrionCallbacks::optionsReturnFn(OrionMenuView *view) {
	optionsDoneFn(view, NULL);
}

void OrionCallbacks::optionsEscapeFn(OrionMenuView *view) {
	optionsCancelFn(view, NULL);
}

/* Save/Load dialog functions */

// Save the current game

void OrionCallbacks::saveLoadSaveFn(DialogView *view, MenuObject *item) {
	if (view->_selectedSlot == -1)
		return;

	MenuTextField *textItem = (MenuTextField *) view->getItem(SLTAG_TEXTFIELD);
	if (!textItem)
		return;

	textItem->setState(OS_NORMAL);

	// Save the game
	bool succeeded = view->vm()->_saveLoad->save(view->_selectedSlot + 1, textItem->getText());

	if (!succeeded) {
		GUI::MessageDialog dialog("Save game failed!");
		dialog.runModal();
	}

	// Close the menu
	closeMenuFn(view, item);
}

void OrionCallbacks::saveLoadLoadFn(DialogView *view, MenuObject *item) {
	// TODO: load the selected save game
	closeMenuFn(view, item);
}

void OrionCallbacks::saveLoadSlotFn(DialogView *view, MenuObject *item) {
	OrionMenuView *vw = (OrionMenuView *) view;
	MenuSaveLoadText *button = (MenuSaveLoadText *) item;

	view->_selectedSlot = button->getIndex();
	view->_deleteSaveDesc = true;

	// Disable all the slots except the selected one
	for (int index = 0; index < SL_NUM_VISIBLE_SLOTS; ++index) {
		MenuSaveLoadText *currentItem = (MenuSaveLoadText *) view->getItem(SLTAG_SLOTS_START + index);
		if (currentItem->getIndex() != button->getIndex()) {
			currentItem->setState(OS_GREYED);
		}
	}

	// Get a copy of the slot bounds
	Common::Rect slotBounds = button->getBounds();

	if (view->getMenuType() == SAVE_MENU) {
		// Add in a text field for entry of the savegame name
		vw->items().push_back(new MenuTextField(view, SLTAG_TEXTFIELD,
			slotBounds.left, slotBounds.top, slotBounds.width(), slotBounds.height(), false,
			saveLoadSaveFn, (button->getText() == EmptySaveString) ? NULL : button->getText(),
			button->getIndex() + 1));

	} else {
		vw->items().push_back(new MenuTextField(view, SLTAG_TEXTFIELD,
			slotBounds.left, slotBounds.top, slotBounds.width(), slotBounds.height(), true,
			saveLoadLoadFn, button->getText(), button->getIndex() + 1));
	}

	// Hide the existing slot
	button->setVisible(false);

	// Disable the slider

	MenuVertSlider *slider = (MenuVertSlider *) view->getItem(SLTAG_VSLIDER);
	slider->setState(OS_GREYED);

	// Enable the save/load button
	MenuButton *btn = (MenuButton *) view->getItem(SLTAG_SAVELOAD);
	btn->setState(OS_NORMAL);
}

void OrionCallbacks::saveLoadCancelFn(DialogView *view, MenuObject *item) {
	OrionMenuView *vw = (OrionMenuView *) view;

	if (view->_selectedSlot != -1) {
		// Pressed cancel with a save selected, so revert back to no selection

		// Re-enable all the other slots

		for (int index = 0; index < SL_NUM_VISIBLE_SLOTS; ++index) {
			if (index != view->_selectedSlot) {
				MenuSaveLoadText *currentItem = (MenuSaveLoadText *) view->getItem(SLTAG_SLOTS_START + index);
				currentItem->setState(OS_NORMAL);
			}
		}

		// Show the previously hidden slot again
		MenuSaveLoadText *slot = (MenuSaveLoadText *) view->getItem(SLTAG_SLOTS_START + view->_selectedSlot);
		slot->setVisible(true);
		slot->setState(OS_NORMAL);

		// Remove the text selection
		MenuTextField *textField = (MenuTextField *) view->getItem(SLTAG_TEXTFIELD);
		delete textField;
		vw->items().remove(textField);

		// Set button enablement
		MenuButton *btn = (MenuButton *) view->getItem(SLTAG_SAVELOAD);
		btn->setState(OS_GREYED);
		btn = (MenuButton *) view->getItem(SLTAG_CANCEL);
		btn->setState(OS_NORMAL);

		// Re-enable the slider

		MenuVertSlider *slider = (MenuVertSlider *) view->getItem(SLTAG_VSLIDER);
		slider->setState(OS_NORMAL);

		view->_selectedSlot = -1;

	} else {
		// Close the dialog
		if (vw->_loadSaveFromHotkey)
			// Since dialog was called from hotkey, return directly to the game
			closeMenuFn(view, item);
		else {
			// Return to the game menu
			view->vm()->loadMenu(GAME_MENU);
			view->close();
		}
	}
}

void OrionCallbacks::saveLoadSliderFn(DialogView *view, MenuObject *item) {
	OrionMenuView *vw = (OrionMenuView *) view;
	MenuVertSlider *slider = (MenuVertSlider *) item;

	if (slider->sliderState() == VSLIDER_THUMBNAIL) {
		// Callback generated by slider thumb, so set top slot using slider percentage
		vw->setTopSaveSlot(slider->percent() * 89 / 100);

	} else {
		int newIndex = view->_topSaveSlotIndex;

		switch (slider->sliderState()) {
		case VSLIDER_UP:
			if (newIndex > 0)
				--newIndex;
			break;

		case VSLIDER_PAGE_UP:
			if (newIndex > 0)
				newIndex = MAX(newIndex - 10, 0);
			break;

		case VSLIDER_PAGE_DOWN:
			if (newIndex < 89)
				newIndex = MIN(newIndex + 10, 89);
			break;

		case VSLIDER_DOWN:
			if (newIndex < 89)
				++newIndex;
			break;

		default:
			break;
		}

		if (newIndex != view->_topSaveSlotIndex) {
			// Set the new top slot
			vw->setTopSaveSlot(newIndex);

			// Set the new slider position
			slider->setPercentage(newIndex * 100 / 89);
		}
	}
}

void OrionCallbacks::saveLoadEscapeFn(OrionMenuView *view) {
	saveLoadCancelFn(view, NULL);
}

void OrionCallbacks::saveLoadReturnFn(OrionMenuView *view) {
	MenuTextField *textItem = (MenuTextField *) view->getItem(SLTAG_TEXTFIELD);
	if (textItem) {
		if (view->getMenuType() == SAVE_MENU)
			saveLoadSaveFn(view, NULL);
		else
			saveLoadLoadFn(view, NULL);
	}
}

//--------------------------------------------------------------------------

OrionMenuView::OrionMenuView(M4Engine *Vm, int x, int y, MenuType menuType, bool calledFromMainMenu,
		bool loadSaveFromHotkey): DialogView(Vm, x, y, true) {
	_menuType = menuType;
	_screenType = VIEWID_MENU;
	_screenFlags.layer = LAYER_MENU;
	_screenFlags.get = SCREVENT_ALL;
	_screenFlags.blocks = SCREVENT_ALL;
	_screenFlags.immovable = true;
	//_screenFlags.immovable = false;	// uncomment to make menu movable
	_coords.left = x;
	_coords.top = y;
	_currentItem = NULL;
	_escapeHandler = &OrionCallbacks::closeMenuFn;
	_returnHandler = NULL;
	_saveNames = NULL;
	_savegameThumbnail = NULL;
	_deleteSaveDesc = false;
	_closeFlag = false;

	_calledFromMainMenu = calledFromMainMenu;
	_loadSaveFromHotkey = loadSaveFromHotkey;

	_interfaceWasVisible = _vm->_interfaceView->isVisible();
	if (_interfaceWasVisible)
		_vm->_interfaceView->hide();

	_vm->_mouse->setCursorNum(CURSOR_ARROW);

	switch (menuType) {
	case GAME_MENU:
		loadSprites(MENU_GAME);

		// Add menu contents
		_menuObjects.push_back(new MenuButton(this, BTNID_MAIN, 45, 53, 24, 24, &OrionCallbacks::closeMenuFn));
		_menuObjects.push_back(new MenuButton(this, BTNID_OPTIONS, 45, 94, 24, 24, &OrionCallbacks::gameOptionsMenuFn));
		_menuObjects.push_back(new MenuButton(this, BTNID_RESUME, 45, 135, 24, 24, &OrionCallbacks::closeMenuFn));
		_menuObjects.push_back(new MenuButton(this, BTNID_QUIT, 141, 135, 24, 24, &OrionCallbacks::gameExitFn));
		_menuObjects.push_back(new MenuButton(this, BTNID_SAVE, 141, 53, 24, 24, &OrionCallbacks::gameSaveGameFn, _calledFromMainMenu));
		_menuObjects.push_back(new MenuButton(this, BTNID_LOAD, 141, 94, 24, 24, &OrionCallbacks::gameLoadGameFn,
			!_vm->_saveLoad->hasSaves()));

		_escapeHandler = &OrionCallbacks::closeMenuFn;
		_returnHandler = &OrionCallbacks::closeMenuFn;
		break;

	case OPTIONS_MENU:
		loadSprites(MENU_OPTIONS);

		// Store the original settings in case user aborts dialog
		_originalMidiVolume = _vm->midi()->getVolume();

		// Add menu contents
		// TODO: Currently the Digi slider isn't hooked up to anything
		_menuObjects.push_back(new MenuButton(this, OPTIONID_CANCEL, 93, 141, 74, 43,
			&OrionCallbacks::optionsCancelFn, false, false, OBJTYPE_OM_CANCEL));
		_menuObjects.push_back(new MenuButton(this, OPTIONID_DONE, 168, 141, 74, 43,
			&OrionCallbacks::optionsDoneFn, false, false, OBJTYPE_OM_DONE));
		_menuObjects.push_back(new MenuHorizSlider(this, OPTIONID_HSLIDER_MIDI, 47, 64, 212, 24,
			_originalMidiVolume * 100 / 255, &OrionCallbacks::optionsMidiSliderFn, true));
		_menuObjects.push_back(new MenuHorizSlider(this, OPTIONID_HSLIDER_DIGI, 47, 104, 212, 24,
			0, &OrionCallbacks::optionsDigiSliderFn, true));

		_escapeHandler = &OrionCallbacks::optionsEscapeFn;
		_returnHandler = &OrionCallbacks::optionsReturnFn;
		break;

	case SAVE_MENU:
	case LOAD_MENU:
		loadSprites(MENU_SAVELOAD);

		// Set up the defaults for the window
		_topSaveSlotIndex = 0;
		_selectedSlot = -1;
		_highlightedSlot = -1;
		_saveNames = _vm->_saveLoad->getSaves();

		// Set up menu elements
		_menuObjects.push_back(new MenuMessage(this, SLTAG_SAVELOAD_LABEL, 50, 241, 70, 16));
		_menuObjects.push_back(new MenuButton(this, SLTAG_SAVELOAD, 214, 384, 72, 41,
			(menuType == SAVE_MENU) ? &OrionCallbacks::saveLoadSaveFn : &OrionCallbacks::saveLoadLoadFn,
			true, true, (menuType == SAVE_MENU) ? OBJTYPE_SL_SAVE : OBJTYPE_SL_LOAD));
		_menuObjects.push_back(new MenuButton(this, SLTAG_CANCEL, 139, 384, 74, 43,
			&OrionCallbacks::saveLoadCancelFn, false, false, OBJTYPE_SL_CANCEL));
		_menuObjects.push_back(new MenuVertSlider(this, SLTAG_VSLIDER, 291, 255, 23, 127, 0,
			&OrionCallbacks::saveLoadSliderFn));

		if (_menuType == SAVE_MENU)
			_savegameThumbnail = createThumbnail();

		_menuObjects.push_back(new MenuImage(this, SLTAG_THUMBNAIL, 66, 28, 215, 162,
			(_savegameThumbnail == NULL) ? _sprites->getFrame(SL_EMPTY_THUMBNAIL) : _savegameThumbnail));


		{
			SaveGameIterator slot = _saveNames->begin();
			for (uint slotIndex = 0; slotIndex < SL_NUM_VISIBLE_SLOTS; ++slotIndex, ++slot) {
				// Get save slot
				bool isEmpty = (slotIndex >= _saveNames->size()) || (*slot).empty();

				_menuObjects.push_back(new MenuSaveLoadText(this, SLTAG_SLOTS_START + slotIndex,
					50, 256 + slotIndex * 15, 238, 15, &OrionCallbacks::saveLoadSlotFn,
					(menuType == LOAD_MENU) && isEmpty, true, (menuType == LOAD_MENU),
					isEmpty ? EmptySaveString : slot->c_str(), slotIndex + 1));
			}
		}

		_escapeHandler = &OrionCallbacks::saveLoadEscapeFn;
		_returnHandler = &OrionCallbacks::saveLoadReturnFn;
		break;

	default:
		error("Unknown menu type");
		break;
	}

	// Draw all the items onto the background surface
	for (MenuObjectsIterator i = _menuObjects.begin(); i != _menuObjects.end(); ++i)
		(*i)->onRefresh();
}

OrionMenuView::~OrionMenuView() {
	delete _sprites;

	for (MenuObjectList::iterator i = _menuObjects.begin(); i != _menuObjects.end(); ++i)
		delete *i;
	_menuObjects.clear();

	if (_saveNames)
		delete _saveNames;
	if (_savegameThumbnail)
		delete _savegameThumbnail;
}

bool OrionMenuView::loadSprites(const char *seriesName) {
	Common::SeekableReadStream *data = _vm->res()->get(seriesName);
	RGB8 *palette;

	_sprites = new SpriteAsset(_vm, data, data->size(), seriesName);
	palette = _sprites->getPalette();
	_vm->_palette->setPalette(palette, 0, _sprites->getColorCount());

	_vm->res()->toss(seriesName);

	// Update the palette
	//_vm->setPalette((byte *) _menuPalette, 59, 197);

	// The first sprite is the menu background

	M4Sprite *bg = _sprites->getFrame(0);
	this->setSize(bg->width(), bg->height());
	_coords.setWidth(bg->width());
	_coords.setHeight(bg->height());
	bg->copyTo(this);

	return true;
}

// Creates a thumbnail based on the current background screen

M4Surface *OrionMenuView::createThumbnail() {
	M4Surface srcSurface(_vm->_screen->width(), _vm->_screen->height());
	M4Surface *result = new M4Surface(_vm->_screen->width() / 3, _vm->_screen->height() / 3);

	// Translate the scene data

	_vm->_scene->onRefresh(NULL, &srcSurface);
	byte *srcP = srcSurface.getBasePtr(0, 0);
	byte *destP = result->getBasePtr(0, 0);

	for (int yCtr = 0; yCtr < _vm->_scene->height() / 3; ++yCtr, srcP += g_system->getWidth() * 3) {
		byte *src0P = srcP;
		byte *src1P = srcP + _vm->_screen->width();
		byte *src2P = src1P + _vm->_screen->width();

		for (int xCtr = 0; xCtr < result->width(); ++xCtr) {
			*destP = (byte)((uint32)((
				*src0P + *(src0P + 1) + *(src0P + 2) +
				*src1P + *(src1P + 1) + *(src1P + 2) +
				*src2P + *(src2P + 1) + *(src2P + 2)) / 9));
			if (*destP == 0)
				*destP = 21;

			++destP;
			src0P += 3;
			src1P += 3;
			src2P += 3;
		}
	}

	// Translate the game interface view - since it's using standard colors that can't be
	// averaged, simply take the top left pixel of every 3x3 pixel block

	_vm->_interfaceView->onRefresh(NULL, &srcSurface);
	destP = result->getBasePtr(0, 0) + (_vm->_screen->width() / 3) * (_vm->_interfaceView->bounds().top / 3);

	int yStart = _vm->_interfaceView->bounds().top;
	int yEnd = MIN(_vm->_screen->height() - 1, (int) _vm->_interfaceView->bounds().bottom - 1);
	for (int yCtr = yStart; yCtr <= yEnd; yCtr += 3) {
		srcP = (byte *)srcSurface.getBasePtr(0, yCtr) + (yCtr * _vm->_screen->width());

		for (int xCtr = 0; xCtr < result->width(); ++xCtr, srcP += 3)
			*destP++ = *srcP;
	}

	return result;
}

void OrionMenuView::destroyView() {
	M4Engine *engine = _vm;
	bool interfaceVisible = _interfaceWasVisible;
	engine->_viewManager->deleteView(this);

	// Fade the game back in if no menu views are active (such as if a button was pressed in one menu
	// to activate another menu dialog)
	bool fadeIn = engine->_viewManager->getView(VIEWID_MENU) == NULL;

	if (fadeIn) {
		bool fadeToBlack = engine->_events->quitFlag;
		engine->_ws->update();
		engine->_palette->fadeFromGreen(M4_DIALOG_FADE_STEPS, M4_DIALOG_FADE_DELAY, fadeToBlack);

		if (interfaceVisible)
			engine->_interfaceView->show();
	}
}

bool OrionMenuView::onEvent(M4EventType eventType, int param, int x, int y, bool &captureEvents) {
	static Common::Point movingPos(0, 0);
	static bool movingFlag = false;

	bool handledFlag = false;
	int localX, localY;
	MenuObjectsIterator i;

	if (!_screenFlags.visible)
		return false;

	if (!movingFlag)
		captureEvents = false;

	// If the escape key is pressed, then pass onto the Escape handler

	if (eventType == KEVENT_KEY) {
		if ((param == Common::KEYCODE_ESCAPE) && (_escapeHandler != NULL)) {
			// Execute the Escape handler function
			_currentItem = NULL;
			captureEvents = false;
			_escapeHandler(this);
			destroyView();
			return true;
		}

		if (((param == Common::KEYCODE_RETURN) || (param == Common::KEYCODE_KP_ENTER)) &&
			(_returnHandler != NULL)) {
			// Execute the Return handler function
			_currentItem = NULL;
			captureEvents = false;
			_returnHandler(this);
			return true;
		}

		MenuTextField *textItem = (MenuTextField *) getItem(SLTAG_TEXTFIELD);
		if (textItem && textItem->onEvent(KEVENT_KEY, param, x, y, _currentItem))
			return true;
	}

	// Convert the screen position to a relative position within the menu surface
	localX = x - _coords.left;
	localY = y - _coords.top;

	// If there is an active object handling events, pass it on until it releases control

	if (_currentItem) {
		handledFlag = _currentItem->onEvent(eventType, param, localX, localY, _currentItem);

		if (_closeFlag) {
			// Dialog has been flagged to be closed
			captureEvents = false;
			destroyView();
			return true;
		}

		if (_currentItem) {
			captureEvents =
				(Common::find(_menuObjects.begin(), _menuObjects.end(), _currentItem) != _menuObjects.end());
			if (!captureEvents)
				// The menu object is no longer active, so reset current item
				_currentItem = NULL;
		} else {
			captureEvents = false;
		}

		if (handledFlag)
			return true;
	}

	if (eventType == KEVENT_KEY) {
		// Handle keypresses by looping through the item list to see if any of them want it

		for (i = _menuObjects.begin(); (i != _menuObjects.end()) && !handledFlag; ++i) {
			MenuObject *menuObj = *i;
			MenuObject *dummyItem;
			handledFlag = menuObj->onEvent(eventType, param, localX, localY, dummyItem);
		}

		return handledFlag;

	} else {
		// Handle mouse events by scanning the item list to see if the cursor is within any

		for (i = _menuObjects.begin(); (i != _menuObjects.end()) && !handledFlag; ++i) {
			MenuObject *menuObj = *i;

			if (menuObj->isInside(localX, localY)) {
				// Found an item, so pass it the event
				menuObj->onEvent(eventType, param, localX, localY, _currentItem);

				if (_closeFlag) {
					// Dialog has been flagged to be closed
					captureEvents = false;
					destroyView();
					return true;
				}

				if (_currentItem) {
					captureEvents =
						(Common::find(_menuObjects.begin(), _menuObjects.end(), _currentItem) != _menuObjects.end());
					if (!captureEvents)
						// The menu object is no longer active, so reset current item
						_currentItem = NULL;
				} else {
					captureEvents = false;
				}

				return true;
			}
		}
	}

	// None of the items have handled the event, so fall back on menu-wide event handling

	switch (eventType) {
	case MEVENT_LEFT_CLICK:
	case MEVENT_DOUBLECLICK:
		if (!_screenFlags.immovable) {
			// Move the entire dialog
			captureEvents = true;
			movingFlag = true;
			movingPos.x = x;
			movingPos.y = y;
		}
		break;

	case MEVENT_LEFT_DRAG:
	case MEVENT_DOUBLECLICK_DRAG:
		if (movingFlag) {
			moveRelative(x - movingPos.x, y - movingPos.y);
			movingPos.x = x;
			movingPos.y = y;
		}
		break;

	case MEVENT_LEFT_RELEASE:
	case MEVENT_DOUBLECLICK_RELEASE:
		captureEvents = false;
		movingFlag = false;
		break;

	default:
		break;
	}

	return true;
}

MenuObject *OrionMenuView::getItem(int objectId) {
	MenuObjectsIterator i;
	for (i = _menuObjects.begin(); i != _menuObjects.end(); ++i) {
		MenuObject *obj = *i;
		if (obj->getObjectId() == objectId)
			return obj;
	}

	return NULL;
}

void OrionMenuView::setTopSaveSlot(int slotNumber) {
	_topSaveSlotIndex = MAX(MIN(slotNumber, 89), 0);

	// Update the details of the load/save slots

	// Get save slot
	SaveGameIterator slot = _saveNames->begin();
	for (int i = 0; i < _topSaveSlotIndex; ++i)
		++slot;

	for (uint index = 0; index < SL_NUM_VISIBLE_SLOTS; ++index, ++slot) {
		MenuSaveLoadText *item = (MenuSaveLoadText *) getItem(SLTAG_SLOTS_START + index);
		uint slotIndex = _topSaveSlotIndex + index;

		bool isEmpty = (slotIndex >= _saveNames->size()) || slot->empty();
		item->setDisplay(slotIndex + 1, isEmpty ? EmptySaveString : slot->c_str());

		item->setState((_menuType == SAVE_MENU) || !isEmpty ? OS_NORMAL : OS_GREYED);
	}
}

void OrionMenuView::refresh(const Common::Rect &areaRect) {
	// Copy the selected portion of the background
	_sprites->getFrame(0)->copyTo(this, areaRect, areaRect.left, areaRect.top);

	for (MenuObjectsIterator i = _menuObjects.begin(); i != _menuObjects.end(); ++i) {
		MenuObject *obj = *i;
		if (obj->getBounds().intersects(areaRect))
			obj->onRefresh();
	}
}

}
