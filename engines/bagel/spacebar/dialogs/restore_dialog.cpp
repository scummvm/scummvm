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

#include "bagel/spacebar/dialogs/restore_dialog.h"
#include "bagel/spacebar/dialogs/save_dialog.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/spacebar/baglib/button_object.h"
#include "bagel/boflib/sound.h"
#include "bagel/spacebar/dialogs/opt_window.h"
#include "bagel/bagel.h"
#include "bagel/boflib/log.h"
#include "bagel/spacebar/boflib/std_keys.h"

namespace Bagel {
namespace SpaceBar {

#define DIALOG_WIDTH    640
#define DIALOG_HEIGHT   480

// Edit Text control placement
//
#define EDIT_DX         330
#define EDIT_DY         20
#define EDIT_X          154 //((DIALOG_WIDTH - EDIT_DX) / 2)
#define EDIT_Y          405

// List box placement
//
#define LIST_DX         EDIT_DX
#define LIST_DY         200 //248
#define LIST_X          155 //((DIALOG_WIDTH - LIST_DX) / 2)
#define LIST_Y          72
#define LIST_TEXT_DY    EDIT_DY
#define LIST_FONT_SIZE  12

struct ST_BUTTONS {
	const char *_pszName;
	const char *_pszUp;
	const char *_pszDown;
	const char *_pszFocus;
	const char *_pszDisabled;
	int _nLeft;
	int _nTop;
	int _nWidth;
	int _nHeight;
	int _nID;
};

static const ST_BUTTONS g_stButtons[NUM_RESTORE_BTNS] = {
	{ "Restore", "restorup.bmp", "restordn.bmp", "restorgr.bmp", "restorgr.bmp", 21, 400, 120, 40, RESTORE_BTN},
	{ "Cancel", "cancelup.bmp", "canceldn.bmp", "cancelup.bmp", "cancelup.bmp", 495, 400, 120, 40, CANCEL_BTN},

	{ "LineUp", "lineupup.bmp", "lineupdn.bmp", "lineupup.bmp", "lineupgr.bmp", 490, 50, 25, 25, LINEUP_BTN},
	{ "LineDn", "linednup.bmp", "linedndn.bmp", "linednup.bmp", "linedngr.bmp", 490, 250, 25, 25, LINEDN_BTN},

	{ "PageUp", "pageupup.bmp", "pageupdn.bmp", "pageupup.bmp", "pageupgr.bmp", 490, 80, 25, 25, PAGEUP_BTN},
	{ "PageDn", "pagednup.bmp", "pagedndn.bmp", "pagednup.bmp", "pagedngr.bmp", 490, 215, 25, 25, PAGEDN_BTN}
};

extern CBofWindow *g_hackWindow;
extern int g_nSelectedSlot;


CBagRestoreDialog::CBagRestoreDialog() {
	logInfo("Constructing CBagRestoreDialog");

	Common::fill(_pButtons, _pButtons + NUM_RESTORE_BTNS, (CBofBmpButton *)nullptr);
	g_nSelectedSlot = -1;
}

ErrorCode CBagRestoreDialog::attach() {
	assert(isValidObject(this));

	_bRestored = false;

	_nSelectedItem = g_nSelectedSlot;

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
	for (int i = 0; i < NUM_RESTORE_BTNS; i++) {
		assert(_pButtons[i] == nullptr);

		_pButtons[i] = new CBofBmpButton;

		CBofBitmap *pUp = loadBitmap(buildSysDir(g_stButtons[i]._pszUp), pPal);
		CBofBitmap *pDown = loadBitmap(buildSysDir(g_stButtons[i]._pszDown), pPal);
		CBofBitmap *pFocus = loadBitmap(buildSysDir(g_stButtons[i]._pszFocus), pPal);
		CBofBitmap *pDis = loadBitmap(buildSysDir(g_stButtons[i]._pszDisabled), pPal);

		_pButtons[i]->loadBitmaps(pUp, pDown, pFocus, pDis);
		_pButtons[i]->create(g_stButtons[i]._pszName, g_stButtons[i]._nLeft, g_stButtons[i]._nTop, g_stButtons[i]._nWidth, g_stButtons[i]._nHeight, this, g_stButtons[i]._nID);
		_pButtons[i]->show();
	}

	if (_nSelectedItem == -1) {
		_pButtons[0]->setState(BUTTON_DISABLED);
	}

	// Get a list of saves, and filter out the autosave entry if present
	// (we don't show the autosave slot in the original UI)
	_savesList = g_engine->listSaves();
	int nNumSavedGames = 0;
	for (SaveStateList::iterator it = _savesList.begin(); it != _savesList.end();) {
		if (it->isAutosave()) {
			it = _savesList.erase(it);
		} else {
			nNumSavedGames = MAX(nNumSavedGames, it->getSaveSlot());
			it++;
		}
	}

	// The list box must not be currently allocated
	if (_pListBox != nullptr)
		fatalError(ERR_UNKNOWN, "Unexpected value found in _pListBox");

	// Create a list box for the user to choose the slot to save into
	CBofRect cRect(LIST_X, LIST_Y, LIST_X + LIST_DX - 1, LIST_Y + LIST_DY - 1);

	_pListBox = new CBofListBox();
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
	for (int i = 0; i < nNumSavedGames; i++) {
		Common::String desc = "Empty";

		for (const auto &entry : _savesList) {
			if (entry.getSaveSlot() == (i + 1)) {
				desc = entry.getDescription();
				break;
			}
		}

		_pListBox->addToTail(desc.c_str(), false);
	}

	// Must be a valid item by now
	if (_nSelectedItem != -1) {
		_pListBox->setSelectedItem(_nSelectedItem, false);
	}

	_pListBox->show();
	_pListBox->updateWindow();

	if (!errorOccurred()) {
		// There could not already be a text field
		assert(_pText == nullptr);

		_pText = new CBofText;

		cRect.setRect(170, 405, 470, 435);
		_pText->setupText(&cRect, JUSTIFY_LEFT, FORMAT_CENTER_LEFT);
		_pText->SetSize(16);
		_pText->setWeight(TEXT_BOLD);

		// Set initial selected item
		if (_nSelectedItem != -1) {
			_pText->setText(_pListBox->getText(_nSelectedItem));
		} else {
			_pText->setText("");
		}
	}

	CBagCursor::showSystemCursor();

	return _errCode;
}

ErrorCode CBagRestoreDialog::detach() {
	assert(isValidObject(this));

	CBagCursor::hideSystemCursor();

	delete _pText;
	_pText = nullptr;

	delete _pScrollBar;
	_pScrollBar = nullptr;

	delete _pListBox;
	_pListBox = nullptr;

	// Destroy all buttons
	for (int i = 0; i < NUM_RESTORE_BTNS; i++) {
		delete _pButtons[i];
		_pButtons[i] = nullptr;
	}

	_nSelectedItem = -1;

	CBofApp::getApp()->setPalette(_pSavePalette);

	return _errCode;
}

void CBagRestoreDialog::onPaint(CBofRect *pRect) {
	assert(isValidObject(this));

	paintBackdrop(pRect);

	if (_pListBox != nullptr) {
		_pListBox->repaintAll();
	}

	if (_pText != nullptr) {
		_pText->display(this);
	}

	validateAnscestors();
}

ErrorCode CBagRestoreDialog::RestoreAndclose() {
	assert(isValidObject(this));

	if (!errorOccurred()) {
		// We should not be able to access the save button if we
		// have not yet chosen a slot to save into.
		assert(_nSelectedItem != -1);
		if (_nSelectedItem != -1) {
			logInfo(buildString("Restoring from slot #%d", _nSelectedItem));

			g_nSelectedSlot = _nSelectedItem;

			// If we are restoring a game, then we don't need to repaint
			// the background, because the screen is changing to a restored state.
			killBackground();
			close();

			// Restore
			if (g_engine->loadGameState(_nSelectedItem + 1).getCode() == Common::kNoError) {
				_bRestored = true;
			}
		}
	}

	return _errCode;
}

void CBagRestoreDialog::onKeyHit(uint32 lKey, uint32 nRepCount) {
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
		if (_nSelectedItem != -1)
			RestoreAndclose();
		break;


	// Cancel without saving
	case BKEY_ESC:
		close();
		break;

	default:
		CBofDialog::onKeyHit(lKey, nRepCount);
		break;
	}
}

void CBagRestoreDialog::onBofButton(CBofObject *pObject, int nFlags) {
	assert(isValidObject(this));
	assert(pObject != nullptr);

	if (nFlags != BUTTON_CLICKED)
		return;

	CBofBmpButton *pButton = (CBofBmpButton *)pObject;

	switch (pButton->getControlID()) {

	// Do actual save
	case RESTORE_BTN:
		RestoreAndclose();
		break;

	// Cancel without saving
	case CANCEL_BTN:
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

void CBagRestoreDialog::onBofListBox(CBofObject *pObject, int nItemIndex) {
	assert(isValidObject(this));
	assert(pObject != nullptr);

	CBofListBox *pListBox = (CBofListBox *)pObject;

	// There is only one list box on this dialog
	if (_pListBox != nullptr) {
		// Force item to be highlighted
		_pListBox->repaintAll();

		// Show selected item in the Edit control
		if (_pText != nullptr) {
			_pText->setText(_pListBox->getText(nItemIndex));
			_pText->display(this);
		}

		_nSelectedItem = nItemIndex;
	}

	if (_nSelectedItem != -1) {
		if ((_pButtons[0] != nullptr) && (_pButtons[0]->getState() == BUTTON_DISABLED)) {
			_pButtons[0]->setState(BUTTON_UP, true);
		}

		// If user double-clicked on this entry, then just restore it now
		if (pListBox->getState() == LISTBOX_USENOW) {
			RestoreAndclose();
		}

	} else if ((_pButtons[0] != nullptr) && (_pButtons[0]->getState() != BUTTON_DISABLED)) {
		_pButtons[0]->setState(BUTTON_DISABLED, true);
	}
}

void CBagRestoreDialog::onInitDialog() {
	assert(isValidObject(this));

	CBofDialog::onInitDialog();

	attach();
}

} // namespace SpaceBar
} // namespace Bagel
