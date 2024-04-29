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

#include "bagel/dialogs/restore_dialog.h"
#include "bagel/dialogs/save_dialog.h"
#include "bagel/baglib/bagel.h"
#include "bagel/baglib/button_object.h"
#include "bagel/boflib/sound.h"
#include "bagel/dialogs/opt_window.h"
#include "bagel/bagel.h"
#include "bagel/boflib/log.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {

#define USE_CBAGDIALOG 0

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
	const char *m_pszName;
	const char *m_pszUp;
	const char *m_pszDown;
	const char *m_pszFocus;
	const char *m_pszDisabled;
	int m_nLeft;
	int m_nTop;
	int m_nWidth;
	int m_nHeight;
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
	LogInfo("Constructing CBagRestoreDialog");

	Common::fill(m_pButtons, m_pButtons + NUM_RESTORE_BTNS, (CBofBmpButton *)nullptr);
	g_nSelectedSlot = -1;
}

ErrorCode CBagRestoreDialog::attach() {
	Assert(IsValidObject(this));

	m_bRestored = false;

	m_nSelectedItem = g_nSelectedSlot;

	// Save off the current game's palette
	m_pSavePalette = CBofApp::GetApp()->getPalette();

	// Insert ours
	CBofPalette *pPal = _pBackdrop->getPalette();
	CBofApp::GetApp()->setPalette(pPal);

	// Paint the SaveList Box onto the background
	if (_pBackdrop != nullptr) {
		CBofBitmap cBmp(BuildSysDir("SAVELIST.BMP"), pPal);
		cBmp.paint(_pBackdrop, 153, 50);
	}
	if (_pBackdrop != nullptr) {
		CBofBitmap cBmp(BuildSysDir("TYPESAVE.BMP"), pPal);
		cBmp.paint(_pBackdrop, 152, 400);
	}

	// Build all our buttons
	for (int i = 0; i < NUM_RESTORE_BTNS; i++) {
		Assert(m_pButtons[i] == nullptr);

		if ((m_pButtons[i] = new CBofBmpButton) != nullptr) {
			CBofBitmap *pUp, *pDown, *pFocus, *pDis;

			pUp = loadBitmap(BuildSysDir(g_stButtons[i].m_pszUp), pPal);
			pDown = loadBitmap(BuildSysDir(g_stButtons[i].m_pszDown), pPal);
			pFocus = loadBitmap(BuildSysDir(g_stButtons[i].m_pszFocus), pPal);
			pDis = loadBitmap(BuildSysDir(g_stButtons[i].m_pszDisabled), pPal);

			m_pButtons[i]->loadBitmaps(pUp, pDown, pFocus, pDis);

			m_pButtons[i]->create(g_stButtons[i].m_pszName, g_stButtons[i].m_nLeft, g_stButtons[i].m_nTop, g_stButtons[i].m_nWidth, g_stButtons[i].m_nHeight, this, g_stButtons[i]._nID);
			m_pButtons[i]->show();
		} else {
			ReportError(ERR_MEMORY);
			break;
		}
	}

	if (m_nSelectedItem == -1) {
		m_pButtons[0]->setState(BUTTON_DISABLED);
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
	Assert(m_pListBox == nullptr);

	// Create a list box for the user to choose the slot to save into
	CBofRect cRect(LIST_X, LIST_Y, LIST_X + LIST_DX - 1, LIST_Y + LIST_DY - 1);
	if ((m_pListBox = new CBofListBox()) != nullptr) {
		m_pListBox->create("SaveGameList", &cRect, this);

		m_pListBox->setPointSize(LIST_FONT_SIZE);
		m_pListBox->setItemHeight(LIST_TEXT_DY);

		// Set a color for selection highlighting
		if (_pBackdrop != nullptr) {
			CBofPalette *pPal2;

			pPal2 = _pBackdrop->getPalette();

			byte iPalIdx = pPal2->GetNearestIndex(RGB(255, 0, 0));

			m_pListBox->setHighlightColor(pPal2->getColor(iPalIdx));
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

			m_pListBox->addToTail(desc.c_str(), false);
		}

		// Must be a valid item by now
		if (m_nSelectedItem != -1) {
			m_pListBox->setSelectedItem(m_nSelectedItem, false);
		}

		m_pListBox->show();
		m_pListBox->updateWindow();

	} else {
		ReportError(ERR_MEMORY);
	}

	if (!ErrorOccurred()) {
		// There could not already be a text field
		Assert(m_pText == nullptr);

		if ((m_pText = new CBofText) != nullptr) {
			cRect.SetRect(170, 405, 470, 435);
			m_pText->SetupText(&cRect, JUSTIFY_LEFT, FORMAT_CENTER_LEFT);
			m_pText->SetSize(16);
			m_pText->setWeight(TEXT_BOLD);

			// Set initial selected item
			if (m_pListBox != nullptr && m_nSelectedItem != -1) {
				m_pText->setText(m_pListBox->getText(m_nSelectedItem));

			} else {
				m_pText->setText("");
			}

		} else {
			ReportError(ERR_MEMORY, "Could not allocate a CBofText for the Restore Dialog");
		}
	}

	CBagCursor::showSystemCursor();

	return _errCode;
}

ErrorCode CBagRestoreDialog::Detach() {
	Assert(IsValidObject(this));

	CBagCursor::hideSystemCursor();

	if (m_pText != nullptr) {
		delete m_pText;
		m_pText = nullptr;
	}

	if (m_pScrollBar != nullptr) {
		delete m_pScrollBar;
		m_pScrollBar = nullptr;
	}

	if (m_pListBox != nullptr) {
		delete m_pListBox;
		m_pListBox = nullptr;
	}

	// Destroy all buttons
	for (int i = 0; i < NUM_RESTORE_BTNS; i++) {

		if (m_pButtons[i] != nullptr) {
			delete m_pButtons[i];
			m_pButtons[i] = nullptr;
		}
	}

	m_nSelectedItem = -1;

	CBofApp::GetApp()->setPalette(m_pSavePalette);

	return _errCode;
}

void CBagRestoreDialog::onPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

	paintBackdrop(pRect);

	if (m_pListBox != nullptr) {
		m_pListBox->repaintAll();
	}

	if (m_pText != nullptr) {
		m_pText->display(this);
	}

	validateAnscestors();
}

ErrorCode CBagRestoreDialog::RestoreAndclose() {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {
		// We should not be able to access the save button if we
		// have not yet chosen a slot to save into.
		Assert(m_nSelectedItem != -1);
		if (m_nSelectedItem != -1) {
			LogInfo(BuildString("Restoring from slot #%d", m_nSelectedItem));

			g_nSelectedSlot = m_nSelectedItem;

			// If we are restoring a game, then we don't need to repaint
			// the background, because the screen is changing to a restored state.
			killBackground();
			close();

			// Restore
			if (g_engine->loadGameState(m_nSelectedItem + 1).getCode() == Common::kNoError) {
				m_bRestored = true;
			}
		}
	}

	return _errCode;
}

void CBagRestoreDialog::onKeyHit(uint32 lKey, uint32 nRepCount) {
	Assert(IsValidObject(this));

	switch (lKey) {
	case BKEY_UP:
		if (m_pListBox != nullptr) {
			m_pListBox->lineUp();
		}
		break;

	case BKEY_DOWN:
		if (m_pListBox != nullptr) {
			m_pListBox->lineDown();
		}
		break;

	case BKEY_PAGEUP:
		if (m_pListBox != nullptr) {
			m_pListBox->pageUp();
		}
		break;

	case BKEY_PAGEDOWN:
		if (m_pListBox != nullptr) {
			m_pListBox->pageDown();
		}
		break;

	// Save into current slot, and exit
	case BKEY_ENTER:
		if (m_nSelectedItem != -1)
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
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

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
		if (m_pListBox != nullptr) {
			m_pListBox->lineUp();
		}
		break;

	case LINEDN_BTN:
		if (m_pListBox != nullptr) {
			m_pListBox->lineDown();
		}
		break;

	case PAGEUP_BTN:
		if (m_pListBox != nullptr) {
			m_pListBox->pageUp();
		}
		break;

	case PAGEDN_BTN:
		if (m_pListBox != nullptr) {
			m_pListBox->pageDown();
		}
		break;

	default:
		LogWarning(BuildString("Save/Restore: Unknown button: %d", pButton->getControlID()));
		break;
	}
}

void CBagRestoreDialog::onBofListBox(CBofObject *pObject, int nItemIndex) {
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

	CBofListBox *pListBox = (CBofListBox *)pObject;

	// There is only one list box on this dialog
	if (m_pListBox != nullptr) {
		// Force item to be highlighted
		m_pListBox->repaintAll();

		// Show selected item in the Edit control
		if (m_pText != nullptr) {
			m_pText->setText(m_pListBox->getText(nItemIndex));
			m_pText->display(this);
		}

		m_nSelectedItem = nItemIndex;
	}

	if (m_nSelectedItem != -1) {
		if ((m_pButtons[0] != nullptr) && (m_pButtons[0]->getState() == BUTTON_DISABLED)) {
			m_pButtons[0]->setState(BUTTON_UP, true);
		}

		// If user double-clicked on this entry, then just restore it now
		if (pListBox->getState() == LISTBOX_USENOW) {
			RestoreAndclose();
		}

	} else if ((m_pButtons[0] != nullptr) && (m_pButtons[0]->getState() != BUTTON_DISABLED)) {
		m_pButtons[0]->setState(BUTTON_DISABLED, true);
	}
}

void CBagRestoreDialog::onInitDialog() {
	Assert(IsValidObject(this));

	CBofDialog::onInitDialog();

	attach();
}

} // namespace Bagel
