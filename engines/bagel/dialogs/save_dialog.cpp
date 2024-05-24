/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bagel/dialogs/save_dialog.h"
#include "bagel/baglib/bagel.h"
#include "bagel/baglib/button_object.h"
#include "bagel/bagel.h"
#include "bagel/boflib/log.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {

const char *buildSysDir(const char *pszFile);

#define DIALOG_WIDTH    640
#define DIALOG_HEIGHT   480


// Edit Text control placement
//
#define EDIT_DX         326
#define EDIT_DY         20
#define EDIT_X          156 //((DIALOG_WIDTH - EDIT_DX) / 2)
#define EDIT_Y          409

// List box placement
//
#define LIST_DX         EDIT_DX
#define LIST_DY         200 //248
#define LIST_X          155 //((DIALOG_WIDTH - LIST_DX) / 2)
#define LIST_Y          72
#define LIST_TEXT_DY    EDIT_DY
#define LIST_FONT_SIZE  12

struct ST_BUTTONS {
	const char *_name;
	const char *_up;
	const char *_down;
	const char *_focus;
	const char *_disabled;
	int _left;
	int _top;
	int _width;
	int _height;
	int _id;
};

static const ST_BUTTONS g_stButtons[NUM_BUTTONS] = {
	{ "Save", "saveup.bmp", "savedn.bmp", "savegr.bmp", "savegr.bmp", 21, 400, 120, 40, SAVE_BTN },
	{ "Cancel", "cancelup.bmp", "canceldn.bmp", "cancelup.bmp", "cancelup.bmp", 495, 400, 120, 40, CANCEL_BTN },

	{ "LineUp", "lineupup.bmp", "lineupdn.bmp", "lineupup.bmp", "lineupgr.bmp", 490, 50, 25, 25, LINEUP_BTN },
	{ "LineDn", "linednup.bmp", "linedndn.bmp", "linednup.bmp", "linedngr.bmp", 490, 250, 25, 25, LINEDN_BTN },

	{ "PageUp", "pageupup.bmp", "pageupdn.bmp", "pageupup.bmp", "pageupgr.bmp", 490, 80, 25, 25, PAGEUP_BTN },
	{ "PageDn", "pagednup.bmp", "pagedndn.bmp", "pagednup.bmp", "pagedngr.bmp", 490, 215, 25, 25, PAGEDN_BTN }
};

// Globals
int g_nSelectedSlot;

CBagSaveDialog::CBagSaveDialog() {
	logInfo("Constructing CBagSaveDialog");
	g_nSelectedSlot = -1;

	_pListBox = nullptr;
	_pEditText = nullptr;
	_pScrollBar = nullptr;
	_nSelectedItem = -1;
	_pSaveBuf = nullptr;
	_nBufSize = 0;
	_pSavePalette = nullptr;

	for (int i = 0; i < NUM_BUTTONS; i++) {
		_pButtons[i] = nullptr;
	}
}

ErrorCode CBagSaveDialog::attach() {
	assert(isValidObject(this));

	g_engine->enableKeymapper(false);

	// Save off the current game's palette
	_pSavePalette = CBofApp::getApp()->getPalette();

	CBofPalette *pPal = nullptr;
	// Insert ours
	if (_pBackdrop != nullptr) {
		pPal = _pBackdrop->getPalette();
		CBofApp::getApp()->setPalette(pPal);
	}
	
	// Paint the SaveList Box onto the background
	if (_pBackdrop != nullptr) {
		CBofBitmap cBmp(buildSysDir("SAVELIST.BMP"), pPal);
		cBmp.paint(_pBackdrop, 153, 50);
	}
	if (_pBackdrop != nullptr) {
		CBofBitmap cBmp(buildSysDir("TYPESAVE.BMP"), pPal);
		cBmp.paint(_pBackdrop, 152, 400);
	}

	// Build all our buttons
	for (int i = 0; i < NUM_BUTTONS; i++) {
		assert(_pButtons[i] == nullptr);

		_pButtons[i] = new CBofBmpButton;

		CBofBitmap *pUp = loadBitmap(buildSysDir(g_stButtons[i]._up), pPal);
		CBofBitmap *pDown = loadBitmap(buildSysDir(g_stButtons[i]._down), pPal);
		CBofBitmap *pFocus = loadBitmap(buildSysDir(g_stButtons[i]._focus), pPal);
		CBofBitmap *pDis = loadBitmap(buildSysDir(g_stButtons[i]._disabled), pPal);

		_pButtons[i]->loadBitmaps(pUp, pDown, pFocus, pDis);
		_pButtons[i]->create(g_stButtons[i]._name, g_stButtons[i]._left, g_stButtons[i]._top, g_stButtons[i]._width, g_stButtons[i]._height, this, g_stButtons[i]._id);
		_pButtons[i]->show();
	}

	// The edit text control must not be currently allocated
	assert(_pEditText == nullptr);

	_pEditText = new CBofEditText("", EDIT_X, EDIT_Y, EDIT_DX, EDIT_DY, this);
	_pEditText->setText("");
	_pEditText->show();

	// Get a list of saves, and filter out the autosave entry if present
	// (we don't show the autosave slot in the original UI)
	_savesList = g_engine->listSaves();
	for (SaveStateList::iterator it = _savesList.begin(); it != _savesList.end(); ++it) {
		if (it->isAutosave()) {
			_savesList.erase(it);
			break;
		}
	}

	// The list box must not be currently allocated
	assert(_pListBox == nullptr);

	// Create a list box for the user to choose the slot to save into
	_pListBox = new CBofListBox();

	CBofRect cRect(LIST_X, LIST_Y, LIST_X + LIST_DX - 1, LIST_Y + LIST_DY - 1);

	_pListBox->create("SaveGameList", &cRect, this);
	_pListBox->setPointSize(LIST_FONT_SIZE);
	_pListBox->setItemHeight(LIST_TEXT_DY);

	// Set a color for selection highlighting
	if (_pBackdrop != nullptr) {
		CBofPalette *pPal2 = _pBackdrop->getPalette();
		byte iPalIdx = pPal2->getNearestIndex(RGB(255, 0, 0));

		_pListBox->setHighlightColor(pPal2->getColor(iPalIdx));
	}

	// Fill the list box with save game entries
	for (int i = 0; i < MAX_SAVED_GAMES; i++) {
		char title[MAX_SAVE_TITLE];
		Common::strcpy_s(title, "Empty");		// Default empty string

		for (const auto &entry : _savesList) {
			if (entry.getSaveSlot() == (i + 1)) {
				Common::String desc = entry.getDescription();
				Common::strcpy_s(title, desc.c_str());

				if (_nSelectedItem == -1) {
					_nSelectedItem = i;
					_pEditText->setText(desc.c_str());
				}
				break;
			}
		}

		_pListBox->addToTail(title, false);
	}

	_pListBox->show();
	_pListBox->updateWindow();

	if (_nSelectedItem != -1) {
		_pEditText->setFocus();
		_pListBox->setSelectedItem(_nSelectedItem, false);

		if (_nSelectedItem >= 9) {
			_pListBox->scrollTo(_nSelectedItem - 8);
		}
	} else if (_pButtons[0] != nullptr) {
		_pButtons[0]->setState(BUTTON_DISABLED);
	}

	CBagCursor::showSystemCursor();

	return _errCode;
}

ErrorCode CBagSaveDialog::detach() {
	assert(isValidObject(this));

	g_engine->enableKeymapper(true);

	CBagCursor::hideSystemCursor();

	delete _pScrollBar;
	_pScrollBar = nullptr;

	delete _pEditText;
	_pEditText = nullptr;

	delete _pListBox;
	_pListBox = nullptr;

	// Destroy all buttons
	for (int i = 0; i < NUM_BUTTONS; i++) {
		delete _pButtons[i];
		_pButtons[i] = nullptr;
	}

	_nSelectedItem = -1;

	// Restore the saved palette
	CBofApp::getApp()->setPalette(_pSavePalette);

	return _errCode;
}

void CBagSaveDialog::onPaint(CBofRect *pRect) {
	assert(isValidObject(this));

	paintBackdrop(pRect);

	if (_pListBox != nullptr) {
		_pListBox->repaintAll();
	}

	validateAnscestors();
}

void CBagSaveDialog::saveAndClose() {
	assert(isValidObject(this));

	// We should not be able to access the save button if we
	// have not yet chosen a slot to save into.
	assert(_nSelectedItem != -1);
	if (_nSelectedItem != -1) {
		logInfo(buildString("Saving into slot #%d", _nSelectedItem));

		g_nSelectedSlot = _nSelectedItem;

		// Save the game
		g_engine->saveGameState(_nSelectedItem + 1,
			_pEditText->getText().getBuffer());

		close();
		setReturnValue(SAVE_BTN);
	}
}


void CBagSaveDialog::onKeyHit(uint32 lKey, uint32 nRepCount) {
	assert(isValidObject(this));

	switch (lKey) {
	case BKEY_UP:
		if (_pListBox != nullptr) {
			_pListBox->lineUp();
		}
		break;

	case BKEY_DOWN:
		if (_pListBox != nullptr) {
			_pListBox->lineDown();
		}
		break;

	case BKEY_PAGEUP:
		if (_pListBox != nullptr) {
			_pListBox->pageUp();
		}
		break;

	case BKEY_PAGEDOWN:
		if (_pListBox != nullptr) {
			_pListBox->pageDown();
		}
		break;

	// Save into current slot, and exit
	case BKEY_ENTER:
		saveAndClose();
		break;

	// Cancel without saving
	case BKEY_ESC:
		setReturnValue(CANCEL_BTN);
		close();
		break;

	default:
		CBofDialog::onKeyHit(lKey, nRepCount);
		break;
	}
}


void CBagSaveDialog::onBofButton(CBofObject *pObject, int nFlags) {
	assert(isValidObject(this));
	assert(pObject != nullptr);

	if (nFlags != BUTTON_CLICKED)
		return;

	CBofBmpButton *pButton = (CBofBmpButton *)pObject;

	switch (pButton->getControlID()) {
	// Do actual save
	case SAVE_BTN:
		setReturnValue(SAVE_BTN);
		saveAndClose();
		break;

	// Cancel without saving
	case CANCEL_BTN:
		setReturnValue(CANCEL_BTN);
		close();
		break;

	case LINEUP_BTN:
		if (_pListBox != nullptr) {
			_pListBox->lineUp();
		}
		break;

	case LINEDN_BTN:
		if (_pListBox != nullptr) {
			_pListBox->lineDown();
		}
		break;

	case PAGEUP_BTN:
		if (_pListBox != nullptr) {
			_pListBox->pageUp();
		}
		break;

	case PAGEDN_BTN:
		if (_pListBox != nullptr) {
			_pListBox->pageDown();
		}
		break;

	default:
		logWarning(buildString("Save/Restore: Unknown button: %d", pButton->getControlID()));
		break;
	}
}


void CBagSaveDialog::onBofListBox(CBofObject * /*pObject*/, int nItemIndex) {
	assert(isValidObject(this));

	// Reset the focus away from the text field if set
	releaseFocus();

	// There is only one list box on this dialog
	if (_pListBox != nullptr) {
		// Force item to be highlighted
		_pListBox->repaintAll();

		// Show selected item in the Edit control
		if (_pEditText != nullptr) {
			_pEditText->setFocus();
			_pEditText->setText(_pListBox->getText(nItemIndex));
		}

		_nSelectedItem = nItemIndex;
	}

	if ((_nSelectedItem != -1) && (_pButtons[0] != nullptr) && (_pButtons[0]->getState() == BUTTON_DISABLED)) {
		_pButtons[0]->setState(BUTTON_UP, true);
	}
}

void CBagSaveDialog::onInitDialog() {
	assert(isValidObject(this));

	CBofDialog::onInitDialog();
	attach();
}

} // namespace Bagel
