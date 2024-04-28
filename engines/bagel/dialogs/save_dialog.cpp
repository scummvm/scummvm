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

#define USE_CBAGDIALOG 0

const char *BuildSysDir(const char *pszFile);

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
	LogInfo("Constructing CBagSaveDialog");
	g_nSelectedSlot = -1;

	m_pListBox = nullptr;
	m_pEditText = nullptr;
	m_pScrollBar = nullptr;
	m_nSelectedItem = -1;
	m_pSaveBuf = nullptr;
	m_nBufSize = 0;
	m_pSavePalette = nullptr;

	for (int i = 0; i < NUM_BUTTONS; i++) {
		m_pButtons[i] = nullptr;
	}
}

ErrorCode CBagSaveDialog::attach() {
	Assert(IsValidObject(this));

	// Save off the current game's palette
	m_pSavePalette = CBofApp::GetApp()->GetPalette();

	// Insert ours
	CBofPalette *pPal = m_pBackdrop->GetPalette();
	CBofApp::GetApp()->SetPalette(pPal);

	// Paint the SaveList Box onto the background
	if (m_pBackdrop != nullptr) {
		CBofBitmap cBmp(BuildSysDir("SAVELIST.BMP"), pPal);
		cBmp.Paint(m_pBackdrop, 153, 50);
	}
	if (m_pBackdrop != nullptr) {
		CBofBitmap cBmp(BuildSysDir("TYPESAVE.BMP"), pPal);
		cBmp.Paint(m_pBackdrop, 152, 400);
	}

	// Build all our buttons
	for (int i = 0; i < NUM_BUTTONS; i++) {
		Assert(m_pButtons[i] == nullptr);

		if ((m_pButtons[i] = new CBofBmpButton) != nullptr) {
			CBofBitmap *pUp, *pDown, *pFocus, *pDis;

			pUp = LoadBitmap(BuildSysDir(g_stButtons[i]._up), pPal);
			pDown = LoadBitmap(BuildSysDir(g_stButtons[i]._down), pPal);
			pFocus = LoadBitmap(BuildSysDir(g_stButtons[i]._focus), pPal);
			pDis = LoadBitmap(BuildSysDir(g_stButtons[i]._disabled), pPal);

			m_pButtons[i]->LoadBitmaps(pUp, pDown, pFocus, pDis);

			m_pButtons[i]->Create(g_stButtons[i]._name, g_stButtons[i]._left, g_stButtons[i]._top, g_stButtons[i]._width, g_stButtons[i]._height, this, g_stButtons[i]._id);
			m_pButtons[i]->Show();
		} else {
			ReportError(ERR_MEMORY);
			break;
		}
	}

	// The edit text control must not be currently allocated
	Assert(m_pEditText == nullptr);

	if ((m_pEditText = new CBofEditText("", EDIT_X, EDIT_Y, EDIT_DX, EDIT_DY, this)) != nullptr) {
		m_pEditText->SetText("");
		m_pEditText->Show();
	}

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
	Assert(m_pListBox == nullptr);

	// Create a list box for the user to choose the slot to save into
	if ((m_pListBox = new CBofListBox()) != nullptr) {
		CBofRect cRect(LIST_X, LIST_Y, LIST_X + LIST_DX - 1, LIST_Y + LIST_DY - 1);

		m_pListBox->Create("SaveGameList", &cRect, this);

		m_pListBox->SetPointSize(LIST_FONT_SIZE);
		m_pListBox->SetItemHeight(LIST_TEXT_DY);

		// Set a color for selection highlighting
		if (m_pBackdrop != nullptr) {
			CBofPalette *pPal2 = m_pBackdrop->GetPalette();
			byte iPalIdx = pPal2->GetNearestIndex(RGB(255, 0, 0));

			m_pListBox->SetHighlightColor(pPal2->GetColor(iPalIdx));
		}

		// Fill the list box with save game entries
		for (int i = 0; i < MAX_SAVEDGAMES; i++) {
			char title[MAX_SAVETITLE];
			Common::strcpy_s(title, "Empty");		// Default empty string

			for (const auto &entry : _savesList) {
				if (entry.getSaveSlot() == (i + 1)) {
					Common::String desc = entry.getDescription();
					Common::strcpy_s(title, desc.c_str());

					if (m_nSelectedItem == -1) {
						m_nSelectedItem = i;
						m_pEditText->SetText(desc.c_str());
					}
					break;
				}
			}

			m_pListBox->AddToTail(title, false);
		}

		m_pListBox->Show();
		m_pListBox->UpdateWindow();

	} else {
		ReportError(ERR_MEMORY);
	}

	if (m_nSelectedItem != -1) {
		if (m_pEditText != nullptr) {
			m_pEditText->SetFocus();
		}

		if (m_pListBox != nullptr) {
			m_pListBox->SetSelectedItem(m_nSelectedItem, false);

			if (m_nSelectedItem >= 9) {

				m_pListBox->ScrollTo(m_nSelectedItem - 8);
			}
		}
	} else if (m_pButtons[0] != nullptr) {
		m_pButtons[0]->SetState(BUTTON_DISABLED);
	}

	CBagCursor::showSystemCursor();

	return m_errCode;
}

ErrorCode CBagSaveDialog::Detach() {
	Assert(IsValidObject(this));

	CBagCursor::hideSystemCursor();

	if (m_pScrollBar != nullptr) {
		delete m_pScrollBar;
		m_pScrollBar = nullptr;
	}

	if (m_pEditText != nullptr) {
		delete m_pEditText;
		m_pEditText = nullptr;
	}

	if (m_pListBox != nullptr) {
		delete m_pListBox;
		m_pListBox = nullptr;
	}

	// Destroy all buttons
	for (int i = 0; i < NUM_BUTTONS; i++) {
		if (m_pButtons[i] != nullptr) {
			delete m_pButtons[i];
			m_pButtons[i] = nullptr;
		}
	}

	m_nSelectedItem = -1;

	// Restore the saved palette
	CBofApp::GetApp()->SetPalette(m_pSavePalette);

	return m_errCode;
}

void CBagSaveDialog::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

	PaintBackdrop(pRect);

	if (m_pListBox != nullptr) {
		m_pListBox->RepaintAll();
	}

	ValidateAnscestors();
}

void CBagSaveDialog::SaveAndClose() {
	Assert(IsValidObject(this));

	// We should not be able to access the save button if we
	// have not yet chosen a slot to save into.
	Assert(m_nSelectedItem != -1);
	if (m_nSelectedItem != -1) {
		LogInfo(BuildString("Saving into slot #%d", m_nSelectedItem));

		g_nSelectedSlot = m_nSelectedItem;

		// Save the game
		g_engine->saveGameState(m_nSelectedItem + 1,
			m_pEditText->GetText().GetBuffer());

		Close();
		SetReturnValue(SAVE_BTN);
	}
}


void CBagSaveDialog::onKeyHit(uint32 lKey, uint32 nRepCount) {
	Assert(IsValidObject(this));

	switch (lKey) {
	case BKEY_UP:
		if (m_pListBox != nullptr) {
			m_pListBox->LineUp();
		}
		break;

	case BKEY_DOWN:
		if (m_pListBox != nullptr) {
			m_pListBox->LineDown();
		}
		break;

	case BKEY_PAGEUP:
		if (m_pListBox != nullptr) {
			m_pListBox->PageUp();
		}
		break;

	case BKEY_PAGEDOWN:
		if (m_pListBox != nullptr) {
			m_pListBox->PageDown();
		}
		break;

	// Save into current slot, and exit
	case BKEY_ENTER:
		SaveAndClose();
		break;

	// Cancel without saving
	case BKEY_ESC:
		SetReturnValue(CANCEL_BTN);
		Close();
		break;

	default:
		CBofDialog::onKeyHit(lKey, nRepCount);
		break;
	}
}


void CBagSaveDialog::OnBofButton(CBofObject *pObject, int nFlags) {
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

	if (nFlags != BUTTON_CLICKED)
		return;
	
	CBofBmpButton *pButton = (CBofBmpButton *)pObject;

	switch (pButton->GetControlID()) {
	// Do actual save
	case SAVE_BTN:
		SetReturnValue(SAVE_BTN);
		SaveAndClose();
		break;

	// Cancel without saving
	case CANCEL_BTN:
		SetReturnValue(CANCEL_BTN);
		Close();
		break;

	case LINEUP_BTN:
		if (m_pListBox != nullptr) {
			m_pListBox->LineUp();
		}
		break;

	case LINEDN_BTN:
		if (m_pListBox != nullptr) {
			m_pListBox->LineDown();
		}
		break;

	case PAGEUP_BTN:
		if (m_pListBox != nullptr) {
			m_pListBox->PageUp();
		}
		break;

	case PAGEDN_BTN:
		if (m_pListBox != nullptr) {
			m_pListBox->PageDown();
		}
		break;

	default:
		LogWarning(BuildString("Save/Restore: Unknown button: %d", pButton->GetControlID()));
		break;
	}
}


void CBagSaveDialog::OnBofListBox(CBofObject * /*pObject*/, int nItemIndex) {
	Assert(IsValidObject(this));

	// Reset the focus away from the text field if set
	ReleaseFocus();

	// There is only one list box on this dialog
	if (m_pListBox != nullptr) {
		// Force item to be highlighted
		m_pListBox->RepaintAll();

		// Show selected item in the Edit control
		if (m_pEditText != nullptr) {
			m_pEditText->SetFocus();
			m_pEditText->SetText(m_pListBox->GetText(nItemIndex));
		}

		m_nSelectedItem = nItemIndex;
	}

	if ((m_nSelectedItem != -1) && (m_pButtons[0] != nullptr) && (m_pButtons[0]->GetState() == BUTTON_DISABLED)) {
		m_pButtons[0]->SetState(BUTTON_UP, true);
	}
}

void CBagSaveDialog::OnInitDialog() {
	Assert(IsValidObject(this));

	CBofDialog::OnInitDialog();
	attach();
}

} // namespace Bagel
