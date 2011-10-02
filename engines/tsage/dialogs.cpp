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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/translation.h"

#include "gui/dialog.h"
#include "gui/widget.h"

#include "tsage/tsage.h"
#include "tsage/core.h"
#include "tsage/dialogs.h"
#include "tsage/staticres.h"
#include "tsage/globals.h"
#include "tsage/ringworld/ringworld_logic.h"

namespace TsAGE {

/*--------------------------------------------------------------------------*/

/**
 * This dialog class provides a simple message display with support for either one or two buttons.
 */
MessageDialog::MessageDialog(const Common::String &message, const Common::String &btn1Message,
							 const Common::String &btn2Message) : GfxDialog() {
	// Set up the message
	addElements(&_msg, &_btn1, NULL);

	_msg.set(message, 200, ALIGN_LEFT);
	_msg._bounds.moveTo(0, 0);
	_defaultButton = &_btn1;

	// Set up the first button
	_btn1.setText(btn1Message);
	_btn1._bounds.moveTo(_msg._bounds.right - _btn1._bounds.width(), _msg._bounds.bottom + 2);

	if (!btn2Message.empty()) {
		// Set up the second button
		add(&_btn2);
		_btn2.setText(btn2Message);
		_btn2._bounds.moveTo(_msg._bounds.right - _btn2._bounds.width(), _msg._bounds.bottom + 2);
		_btn1._bounds.translate(-(_btn2._bounds.width() + 4), 0);
	}

	// Do post setup for the dialog
	setDefaults();

	// Set the dialog's center
	setCenter(g_globals->_dialogCenter.x, g_globals->_dialogCenter.y);
}

int MessageDialog::show(const Common::String &message, const Common::String &btn1Message, const Common::String &btn2Message) {
	// Ensure that the cursor is the arrow
	g_globals->_events.setCursor(CURSOR_ARROW);

	int result = show2(message, btn1Message, btn2Message);

	g_globals->_events.setCursorFromFlag();
	return result;
}

int MessageDialog::show2(const Common::String &message, const Common::String &btn1Message, const Common::String &btn2Message) {
	MessageDialog *dlg = new MessageDialog(message, btn1Message, btn2Message);
	dlg->draw();

	GfxButton *defaultButton = !btn2Message.empty() ? &dlg->_btn2 : &dlg->_btn1;
	GfxButton *selectedButton = dlg->execute(defaultButton);
	int result =  (selectedButton == defaultButton) ? 1 : 0;

	delete dlg;
	return result;
}

/*--------------------------------------------------------------------------*/

ConfigDialog::ConfigDialog() : GUI::OptionsDialog("", "GlobalConfig") {
	//
	// Sound controllers
	//

	addVolumeControls(this, "GlobalConfig.");
	setVolumeSettingsState(true); // could disable controls by GUI options

	//
	// Add the buttons
	//

	new GUI::ButtonWidget(this, "GlobalConfig.Ok", _("~O~K"), 0, GUI::kOKCmd);
	new GUI::ButtonWidget(this, "GlobalConfig.Cancel", _("~C~ancel"), 0, GUI::kCloseCmd);
}

/*--------------------------------------------------------------------------*/

void ModalDialog::draw() {
	// Set the palette for use in the dialog
	setPalette();

	// Make a backup copy of the area the dialog will occupy
	Rect tempRect = _bounds;
	tempRect.collapse(-10, -10);
	_savedArea = Surface_getArea(g_globals->_gfxManagerInstance.getSurface(), tempRect);

	_gfxManager.activate();

	// Fill in the contents of the entire dialog
	_gfxManager._bounds = Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	drawFrame();

	// Draw each element in the dialog in order
	GfxElementList::iterator i;
	for (i = _elements.begin(); i != _elements.end(); ++i) {
		(*i)->draw();
	}

	_gfxManager.deactivate();
}

void ModalDialog::drawFrame() {
	Rect origRect = _bounds;
	_bounds.collapse(-10, -10);

	// Fill the dialog area
	g_globals->gfxManager().fillRect(origRect, 54);

	// Draw top line
	GfxSurface surface = surfaceFromRes(8, 1, 7);
	for (int xp = _bounds.left + 10; xp < (_bounds.right - 20); xp += 10)
		surface.draw(Common::Point(xp, _bounds.top));
	surface.draw(Common::Point(_bounds.right - 20, _bounds.top));

	surface = surfaceFromRes(8, 1, 1);
	surface.draw(Common::Point(_bounds.left, _bounds.top));

	surface = surfaceFromRes(8, 1, 4);
	surface.draw(Common::Point(_bounds.right - 10, _bounds.top));

	// Draw vertical edges
	surface = surfaceFromRes(8, 1, 2);
	for (int yp = _bounds.top + 10; yp < (_bounds.bottom - 20); yp += 10)
		surface.draw(Common::Point(_bounds.left, yp));
	surface.draw(Common::Point(_bounds.left, _bounds.bottom - 20));

	surface = surfaceFromRes(8, 1, 5);
	for (int yp = _bounds.top + 10; yp < (_bounds.bottom - 20); yp += 10)
		surface.draw(Common::Point(_bounds.right - 10, yp));
	surface.draw(Common::Point(_bounds.right - 10, _bounds.bottom - 20));

	// Draw bottom line
	surface = surfaceFromRes(8, 1, 8);
	for (int xp = _bounds.left + 10; xp < (_bounds.right - 20); xp += 10)
		surface.draw(Common::Point(xp, _bounds.bottom - 10));
	surface.draw(Common::Point(_bounds.right - 20, _bounds.bottom - 10));

	surface = surfaceFromRes(8, 1, 3);
	surface.draw(Common::Point(_bounds.left, _bounds.bottom - 10));

	surface = surfaceFromRes(8, 1, 6);
	surface.draw(Common::Point(_bounds.right - 10, _bounds.bottom - 10));

	// Set the dialog's manager bounds
	_gfxManager._bounds = origRect;
}

/*--------------------------------------------------------------------------*/

bool GfxInvImage::process(Event &event) {
	if (!event.handled && (event.eventType == EVENT_BUTTON_DOWN)) {
		event.handled = _bounds.contains(event.mousePos);
		return event.handled;
	}

	return false;
}

/*--------------------------------------------------------------------------*/

void InventoryDialog::show() {
	// Determine how many items are in the player's inventory
	int itemCount = 0;
	SynchronizedList<InvObject *>::iterator i;
	for (i = RING_INVENTORY._itemList.begin(); i != RING_INVENTORY._itemList.end(); ++i) {
		if ((*i)->inInventory())
			++itemCount;
	}

	if (itemCount == 0) {
		MessageDialog::show(INV_EMPTY_MSG, OK_BTN_STRING);
		return;
	}

	InventoryDialog *dlg = new InventoryDialog();
	dlg->draw();
	dlg->execute();
	delete dlg;
}

InventoryDialog::InventoryDialog() {
	// Determine the maximum size of the image of any item in the player's inventory
	int imgWidth = 0, imgHeight = 0;

	SynchronizedList<InvObject *>::iterator i;
	for (i = RING_INVENTORY._itemList.begin(); i != RING_INVENTORY._itemList.end(); ++i) {
		InvObject *invObject = *i;
		if (invObject->inInventory()) {
			// Get the image for the item
			GfxSurface itemSurface = surfaceFromRes(invObject->_displayResNum, invObject->_rlbNum, invObject->_cursorNum);

			// Maintain the dimensions of the largest item image
			imgWidth = MAX(imgWidth, (int)itemSurface.getBounds().width());
			imgHeight = MAX(imgHeight, (int)itemSurface.getBounds().height());

			// Add the item to the display list
			GfxInvImage *img = new GfxInvImage();
			_images.push_back(img);
			img->setDetails(invObject->_displayResNum, invObject->_rlbNum, invObject->_cursorNum);
			img->_invObject = invObject;
			add(img);
		}
	}
	assert(_images.size() > 0);

	// Figure out the number of columns/rows to show all the items
	int cellsSize = 3;
	while ((cellsSize * cellsSize) < (int)_images.size())
		++cellsSize;

	// Set the position of each inventory item to be displayed
	int cellX = 0;
	Common::Point pt(0, 0);

	for (uint idx = 0; idx < _images.size(); ++idx) {
		if (cellX == cellsSize) {
			// Move to the start of the next line
			pt.x = 0;
			pt.y += imgHeight + 2;
			cellX = 0;
		}

		_images[idx]->_bounds.moveTo(pt.x, pt.y);

		pt.x += imgWidth + 2;
		++cellX;
	}

	// Set up the buttons
	pt.y += imgHeight + 2;
	_btnOk.setText(OK_BTN_STRING);
	_btnOk._bounds.moveTo((imgWidth + 2) * cellsSize - _btnOk._bounds.width(), pt.y);
	_btnLook.setText(LOOK_BTN_STRING);
	_btnLook._bounds.moveTo(_btnOk._bounds.left - _btnLook._bounds.width() - 2, _btnOk._bounds.top);
	addElements(&_btnLook, &_btnOk, NULL);

	frame();
	setCenter(SCREEN_CENTER_X, SCREEN_CENTER_Y);
}

InventoryDialog::~InventoryDialog() {
	for (uint idx = 0; idx < _images.size(); ++idx)
		delete _images[idx];
}

void InventoryDialog::execute() {
	if ((RING_INVENTORY._selectedItem) && RING_INVENTORY._selectedItem->inInventory())
		RING_INVENTORY._selectedItem->setCursor();

	GfxElement *hiliteObj;
	bool lookFlag = false;
	_gfxManager.activate();

	while (!g_vm->shouldQuit()) {
		// Get events
		Event event;
		while (!g_globals->_events.getEvent(event) && !g_vm->shouldQuit()) {
			g_system->delayMillis(10);
			g_system->updateScreen();
		}
		if (g_vm->shouldQuit())
			break;

		hiliteObj = NULL;
		if ((event.eventType == EVENT_BUTTON_DOWN) && !_bounds.contains(event.mousePos))
			break;

		// Pass event to elements
		event.mousePos.x -= _gfxManager._bounds.left;
		event.mousePos.y -= _gfxManager._bounds.top;

		for (GfxElementList::iterator i = _elements.begin(); i != _elements.end(); ++i) {
			if ((*i)->process(event))
				hiliteObj = *i;
		}

		if (!event.handled && event.eventType == EVENT_KEYPRESS) {
			if ((event.kbd.keycode == Common::KEYCODE_RETURN) || (event.kbd.keycode == Common::KEYCODE_ESCAPE)) {
				// Exit the dialog
				//hiliteObj = &_btnOk;
				break;
			}
		}

		if (hiliteObj == &_btnOk) {
			// Ok button clicked
			if (lookFlag)
				g_globals->_events.setCursor(CURSOR_WALK);
			break;
		} else if (hiliteObj == &_btnLook) {
			// Look button clicked
			if (_btnLook._message == LOOK_BTN_STRING) {
				_btnLook._message = PICK_BTN_STRING;
				lookFlag = 1;
				g_globals->_events.setCursor(CURSOR_LOOK);
			} else {
				_btnLook._message = LOOK_BTN_STRING;
				lookFlag = 0;
				g_globals->_events.setCursor(CURSOR_WALK);
			}

			hiliteObj->draw();
		} else if (hiliteObj) {
			// Inventory item selected
			InvObject *invObject = static_cast<GfxInvImage *>(hiliteObj)->_invObject;
			if (lookFlag) {
				g_globals->_screenSurface.displayText(invObject->_description);
			} else {
				RING_INVENTORY._selectedItem = invObject;
				invObject->setCursor();
			}
		}
	}

	_gfxManager.deactivate();
}

/*--------------------------------------------------------------------------*/

void OptionsDialog::show() {
	OptionsDialog *dlg = new OptionsDialog();
	dlg->draw();

	GfxButton *btn = dlg->execute();

	if (btn == &dlg->_btnQuit) {
		// Quit game
		if (MessageDialog::show(QUIT_CONFIRM_MSG, CANCEL_BTN_STRING, QUIT_BTN_STRING) == 1) {
			g_vm->quitGame();
		}
	} else if (btn == &dlg->_btnRestart) {
		// Restart game
		g_globals->_game->restartGame();
	} else if (btn == &dlg->_btnSound) {
		// Sound dialog
		SoundDialog::execute();
	} else if (btn == &dlg->_btnSave) {
		// Save button
		g_globals->_game->saveGame();
	} else if (btn == &dlg->_btnRestore) {
		// Restore button
		g_globals->_game->restoreGame();
	}

	dlg->remove();
	delete dlg;
}

OptionsDialog::OptionsDialog() {
	// Set the element text
	_gfxMessage.set(OPTIONS_MSG, 140, ALIGN_LEFT);
	_btnRestore.setText(RESTORE_BTN_STRING);
	_btnSave.setText(SAVE_BTN_STRING);
	_btnRestart.setText(RESTART_BTN_STRING);
	_btnQuit.setText(QUIT_BTN_STRING);
	_btnSound.setText(SOUND_BTN_STRING);
	_btnResume.setText(RESUME_BTN_STRING);

	// Set position of the elements
	_gfxMessage._bounds.moveTo(0, 1);
	_btnRestore._bounds.moveTo(0, _gfxMessage._bounds.bottom + 1);
	_btnSave._bounds.moveTo(0, _btnRestore._bounds.bottom + 1);
	_btnRestart._bounds.moveTo(0, _btnSave._bounds.bottom + 1);
	_btnQuit._bounds.moveTo(0, _btnRestart._bounds.bottom + 1);
	_btnSound._bounds.moveTo(0, _btnQuit._bounds.bottom + 1);
	_btnResume._bounds.moveTo(0, _btnSound._bounds.bottom + 1);

	// Set all the buttons to the widest button
	GfxButton *btnList[6] = {&_btnRestore, &_btnSave, &_btnRestart, &_btnQuit, &_btnSound, &_btnResume};
	int16 btnWidth = 0;
	for (int idx = 0; idx < 6; ++idx)
		btnWidth = MAX(btnWidth, btnList[idx]->_bounds.width());
	for (int idx = 0; idx < 6; ++idx)
		btnList[idx]->_bounds.setWidth(btnWidth);

	// Add the items to the dialog
	addElements(&_gfxMessage, &_btnRestore, &_btnSave, &_btnRestart, &_btnQuit, &_btnSound, &_btnResume, NULL);

	// Set the dialog size and position
	frame();
	setCenter(160, 100);
}

/*--------------------------------------------------------------------------*/

void SoundDialog::execute() {
	ConfigDialog *dlg = new ConfigDialog();
	dlg->runModal();
	delete dlg;
	g_globals->_soundManager.syncSounds();
	g_globals->_events.setCursorFromFlag();
}

} // End of namespace TsAGE
