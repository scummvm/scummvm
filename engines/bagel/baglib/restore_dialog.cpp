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

#include "bagel/baglib/restore_dialog.h"
#include "bagel/baglib/save_dialog.h"
#include "bagel/baglib/bagel.h"
#include "bagel/baglib/buttons.h"
#include "bagel/boflib/sound.h"
#include "bagel/baglib/opt_window.h"

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
	const CHAR *m_pszName;
	const CHAR *m_pszUp;
	const CHAR *m_pszDown;
	const CHAR *m_pszFocus;
	const CHAR *m_pszDisabled;
	INT m_nLeft;
	INT m_nTop;
	INT m_nWidth;
	INT m_nHeight;
	INT m_nID;
};

static ST_BUTTONS g_stButtons[NUM_RESTORE_BTNS] = {
	{ "Restore", "restorup.bmp", "restordn.bmp", "restorgr.bmp", "restorgr.bmp", 21, 400, 120, 40, RESTORE_BTN},
	{ "Cancel", "cancelup.bmp", "canceldn.bmp", "cancelup.bmp", "cancelup.bmp", 495, 400, 120, 40, CANCEL_BTN},

	{ "LineUp", "lineupup.bmp", "lineupdn.bmp", "lineupup.bmp", "lineupgr.bmp", 490, 50, 25, 25, LINEUP_BTN},
	{ "LineDn", "linednup.bmp", "linedndn.bmp", "linednup.bmp", "linedngr.bmp", 490, 250, 25, 25, LINEDN_BTN},

	{ "PageUp", "pageupup.bmp", "pageupdn.bmp", "pageupup.bmp", "pageupgr.bmp", 490, 80, 25, 25, PAGEUP_BTN},
	{ "PageDn", "pagednup.bmp", "pagedndn.bmp", "pagednup.bmp", "pagedngr.bmp", 490, 215, 25, 25, PAGEDN_BTN}
};

extern CBofWindow *g_pHackWindow;
extern INT g_nSelectedSlot;


CBagRestoreDialog::CBagRestoreDialog() {
	LogInfo("Constructing CBagRestoreDialog");

	// Inits
	m_pListBox = nullptr;
	m_pText = nullptr;
	m_pScrollBar = nullptr;
	m_pSaveGameFile = nullptr;
	m_nSelectedItem = -1;
	BofMemSet(&m_stGameInfo, 0, sizeof(ST_SAVEDGAME_HEADER));
	m_pSaveBuf = nullptr;
	m_nBufSize = 0;
	m_bRestored = FALSE;
	m_pSavePalette = nullptr;

	for (INT i = 0; i < NUM_RESTORE_BTNS; i++) {
		m_pButtons[i] = nullptr;
	}
}

#if BOF_DEBUG
CBagRestoreDialog::~CBagRestoreDialog() {
	Assert(IsValidObject(this));

	LogInfo("Destructing CBagRestoreDialog");
}
#endif

ERROR_CODE CBagRestoreDialog::Attach() {
	Assert(IsValidObject(this));

	CHAR szFileName[MAX_DIRPATH];
	CBagel *pApp;
	LONG nNumSavedGames;
	CBofPalette *pPal;
	INT i;

	m_bRestored = FALSE;

	m_nSelectedItem = g_nSelectedSlot;

	// Save off the current game's palette
	m_pSavePalette = CBofApp::GetApp()->GetPalette();

	// Insert ours
	pPal = m_pBackdrop->GetPalette();
	CBofApp::GetApp()->SetPalette(pPal);

#if USE_CBAGDIALOG
	CBagStorageDevWnd::Attach();
#endif

	// Paint the SaveList Box onto the background
	if (m_pBackdrop != nullptr) {
		CBofBitmap cBmp(BuildSysDir("SAVELIST.BMP"), pPal);
		cBmp.Paint(m_pBackdrop, 153, 50);
	}
	if (m_pBackdrop != nullptr) {
		CBofBitmap cBmp(BuildSysDir("TYPESAVE.BMP"), pPal);
		cBmp.Paint(m_pBackdrop, 152, 400);
	}

#if BOF_MAC

	// please to call explicitly
	if (GetBackdrop()) {
		PaintBackdrop();
	}
#endif

	// Build all our buttons
	for (i = 0; i < NUM_RESTORE_BTNS; i++) {
		Assert(m_pButtons[i] == nullptr);

		if ((m_pButtons[i] = new CBofBmpButton) != nullptr) {
			CBofBitmap *pUp, *pDown, *pFocus, *pDis;

			pUp = LoadBitmap(BuildSysDir(g_stButtons[i].m_pszUp), pPal);
			pDown = LoadBitmap(BuildSysDir(g_stButtons[i].m_pszDown), pPal);
			pFocus = LoadBitmap(BuildSysDir(g_stButtons[i].m_pszFocus), pPal);
			pDis = LoadBitmap(BuildSysDir(g_stButtons[i].m_pszDisabled), pPal);

			m_pButtons[i]->LoadBitmaps(pUp, pDown, pFocus, pDis);

#if BOF_MAC
			// jwl 09.20.96 make this our own custom window such that no frame is drawn
			// around the window/button
			m_pButtons[i]->SetCustomWindow(true);
#endif
			m_pButtons[i]->Create(g_stButtons[i].m_pszName, g_stButtons[i].m_nLeft, g_stButtons[i].m_nTop, g_stButtons[i].m_nWidth, g_stButtons[i].m_nHeight, this, g_stButtons[i].m_nID);
			m_pButtons[i]->Show();
		} else {
			ReportError(ERR_MEMORY);
			break;
		}
	}

	if (m_nSelectedItem == -1) {
		m_pButtons[0]->SetState(BUTTON_DISABLED);
	}

	// The save game object must not be currently allocated
	Assert(m_pSaveGameFile == nullptr);

	if ((pApp = CBagel::GetBagApp()) != nullptr) {
		Common::strcpy_s(szFileName, pApp->GetSaveGameFileName());
	}

	// Load the save game file
	nNumSavedGames = 0;
	if ((m_pSaveGameFile = new CBagSaveGameFile(szFileName)) != nullptr) {

		nNumSavedGames = m_pSaveGameFile->GetNumSavedGames();
	} else {
		ReportError(ERR_MEMORY);
	}

	// The list box must not be currently allocated
	Assert(m_pListBox == nullptr);

	// Create a list box for the user to choose the slot to save into
	CBofRect cRect(LIST_X, LIST_Y, LIST_X + LIST_DX - 1, LIST_Y + LIST_DY - 1);
	if ((m_pListBox = new CBofListBox) != nullptr) {
		ST_SAVEDGAME_HEADER stGameInfo;

#if BOF_MAC
		// Make this our own custom window such that no frame is drawn
		// around the window/button
		m_pListBox->SetCustomWindow(true);
#endif
		m_pListBox->Create("SaveGameList", &cRect, this);

		m_pListBox->SetPointSize(LIST_FONT_SIZE);
		m_pListBox->SetItemHeight(LIST_TEXT_DY);

		// Set a color for selection highlighting
		if (m_pBackdrop != nullptr) {
			CBofPalette *pPal2;

			pPal2 = m_pBackdrop->GetPalette();

			UBYTE iPalIdx = pPal2->GetNearestIndex(RGB(255, 0, 0));

			m_pListBox->SetHighlightColor(pPal2->GetColor(iPalIdx));
		}

		// Fill the list box with save game entries
		for (i = 0; i < nNumSavedGames; i++) {
			if (m_pSaveGameFile != nullptr) {
				// Just read the title, not the whole damn thing
				m_pSaveGameFile->ReadTitleOnly(i, stGameInfo.m_szTitle);

				m_pListBox->AddToTail(stGameInfo.m_szTitle, FALSE);
			}
		}

		// Must be a valid item by now
		if (m_nSelectedItem != -1) {
			m_pListBox->SetSelectedItem(m_nSelectedItem, FALSE);
		}

		m_pListBox->Show();
		m_pListBox->UpdateWindow();

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
			m_pText->SetWeight(TEXT_BOLD);

			// Set initial selected item
			if (m_pListBox != nullptr && m_nSelectedItem != -1) {
				m_pText->SetText(m_pListBox->GetText(m_nSelectedItem));

			} else {
				m_pText->SetText("");
			}

		} else {
			ReportError(ERR_MEMORY, "Could not allocate a CBofText for the Restore Dialog");
		}
	}

	CBofCursor::Show();

	return m_errCode;
}

ERROR_CODE CBagRestoreDialog::Detach() {
	Assert(IsValidObject(this));

	CBofCursor::Hide();

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
	if (m_pSaveGameFile != nullptr) {
		delete m_pSaveGameFile;
		m_pSaveGameFile = nullptr;
	}

	// Destroy all buttons
	for (INT i = 0; i < NUM_RESTORE_BTNS; i++) {

		if (m_pButtons[i] != nullptr) {
			delete m_pButtons[i];
			m_pButtons[i] = nullptr;
		}
	}

	m_nSelectedItem = -1;

#if USE_CBAGDIALOG
	CBagStorageDevWnd::Detach();
#endif

	CBofApp::GetApp()->SetPalette(m_pSavePalette);

	return m_errCode;
}

VOID CBagRestoreDialog::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

#if !BOF_MAC
	PaintBackdrop(pRect);
#endif

	if (m_pListBox != nullptr) {
		m_pListBox->RepaintAll();
	}

#if BOF_MAC
	if (GetBackdrop()) {
		PaintBackdrop();
	}

	for (INT i = 0; i < NUM_RESTORE_BTNS; i++) {
		if (m_pButtons[i] != nullptr) {
			m_pButtons[i]->Paint();
		}
	}
#endif

	if (m_pText != nullptr) {
		m_pText->Display(this);
	}

	ValidateAnscestors();
}

ERROR_CODE CBagRestoreDialog::RestoreAndClose() {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {
		// We should not be able to access the save button if we
		// have not yet chosen a slot to save into.
		Assert(m_nSelectedItem != -1);
		if (m_nSelectedItem != -1) {
			LogInfo(BuildString("Restoring from slot #%d", m_nSelectedItem));

			g_nSelectedSlot = m_nSelectedItem;

			//
			// Restore
			//
			if (m_pSaveGameFile != nullptr) {
				Assert(m_pSaveBuf != nullptr);
				m_pSaveGameFile->ReadSavedGame(m_nSelectedItem, &m_stGameInfo, m_pSaveBuf, m_nBufSize);

				// If no error
				if (!m_pSaveGameFile->ErrorOccurred()) {
					// Make sure this is a valid Save Game record
					if (m_pSaveBuf->m_lStructSize == sizeof(ST_BAGEL_SAVE)) {
						// Successfully restored
						m_bRestored = TRUE;

					} else {
						ReportError(ERR_FTYPE, "Saved Game #%d is from a previous version and cannot be restored.  Please delete the file %s.", m_nSelectedItem, CBagel::GetBagApp()->GetSaveGameFileName());
					}
				}
			}

			// If we are restoring a game, then we don't need to repaint
			// the background, because the screen is changing to a restored state.
			KillBackground();

			Close();
		}
	}

	return m_errCode;
}

VOID CBagRestoreDialog::OnKeyHit(ULONG lKey, ULONG nRepCount) {
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
		if (m_nSelectedItem != -1)
			RestoreAndClose();
		break;


	// Cancel without saving
	case BKEY_ESC:
		Close();
		break;

	default:
		CBofDialog::OnKeyHit(lKey, nRepCount);
		break;
	}
}

VOID CBagRestoreDialog::OnBofButton(CBofObject *pObject, INT nFlags) {
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

	if (nFlags == BUTTON_CLICKED) {

		CBofBmpButton *pButton;

		pButton = (CBofBmpButton *)pObject;

		switch (pButton->GetControlID()) {

		// Do actual save
		case RESTORE_BTN:
			RestoreAndClose();
			break;

		// Cancel without saving
		case CANCEL_BTN:
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
}

VOID CBagRestoreDialog::OnBofListBox(CBofObject *pObject, INT nItemIndex) {
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

	CBofListBox *pListBox;

	pListBox = (CBofListBox *)pObject;

	// There is only one list box on this dialog
	if (m_pListBox != nullptr) {
		ST_SAVEDGAME_HEADER stGameInfo;

		// Force item to be highlighted
		m_pListBox->RepaintAll();

		if (m_pSaveGameFile != nullptr) {
			// Only getting the title causes a crash when the user tries
			// to restore an empty record, so I undid jwl changes.

			// Read entire header for this record, so we can see
			// if this record is a valid saved game or not.
			m_pSaveGameFile->ReadTitle(nItemIndex, &stGameInfo);

			if (stGameInfo.m_bUsed) {
				m_pText->SetText(stGameInfo.m_szTitle);
				m_nSelectedItem = nItemIndex;
			} else {
				m_pText->SetText("");
				m_nSelectedItem = -1;
			}
			m_pText->Display(this);
		}
	}

	if (m_nSelectedItem != -1) {
		if ((m_pButtons[0] != nullptr) && (m_pButtons[0]->GetState() == BUTTON_DISABLED)) {
			m_pButtons[0]->SetState(BUTTON_UP, TRUE);
		}

		// If user double-clicked on this entry, then just restore it now
		if (pListBox->GetState() == LISTBOX_USENOW) {
			RestoreAndClose();
		}

	} else {
		if ((m_pButtons[0] != nullptr) && (m_pButtons[0]->GetState() != BUTTON_DISABLED)) {
			m_pButtons[0]->SetState(BUTTON_DISABLED, TRUE);
		}
	}
}

#if !USE_CBAGDIALOG
VOID CBagRestoreDialog::OnInitDialog() {
	Assert(IsValidObject(this));

	CBofDialog::OnInitDialog();

	Attach();
}
#endif

} // namespace Bagel
