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

#include "bagel/baglib/opt_window.h"
#include "bagel/baglib/bagel.h"
#include "bagel/boflib/sound.h"
#include "bagel/baglib/save_dialog.h"
#include "bagel/baglib/storage_dev_win.h"
#include "bagel/baglib/pan_window.h"

namespace Bagel {

#define USER_OPTIONS "UserOptions"
#if BOF_MAC || BOF_WINMAC
#define WAVE_VOLUME  "SndVolume"
#else
#define WAVE_VOLUME  "WaveVolume"
#endif

#define BROWN_SCROLL_BKGD   "SLIDER.BMP"
#define BROWN_SCROLL_THMB   "THUMB.BMP"
#define BROWN_SCROLL_LFUP   "LEFTUP.BMP"
#define BROWN_SCROLL_RTUP   "RIGHTUP.BMP"
#define BROWN_SCROLL_LFDN   "LEFTDN.BMP"
#define BROWN_SCROLL_RTDN   "RIGHTDN.BMP"


#define VOLUME_MIN      0
#define VOLUME_MAX      12
#define VOLUME_DEFAULT  10

#define CHECKBOX_WIDTH      20
#define CHECKBOX_HEIGHT     20

#define FLYTHROUGHS_LEFT     41
#define FLYTHROUGHS_TOP     151
#define PANIMATIONS_LEFT     41
#define PANIMATIONS_TOP     201

// Button IDs
//
#define SAVE_ID        0
#define RESTORE_ID     1
#define RESTART_ID     2
#define QUIT_ID        3
#define HELP_ID        4
#define CREDITS_ID     5
#define OKAY_ID        6
#define DEFAULTS_ID    7
#define PANSPEED_ID    8
#define CORRECTION_ID  9
#define FLYTHROUGHS_ID 10
#define PAN_CHECK_ID   11
#define MIDI_VOL_ID    12
#define WAVE_VOL_ID    13


// Local variables
//


// Local functions
//
const CHAR *BuildSysDir(const CHAR *pszFile);

typedef struct {
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

} ST_BUTTONS;

static ST_BUTTONS g_stButtons[NUM_SYS_BUTTONS] = {

	{ "Save",     "saveup.bmp",   "savedn.bmp",   "savegr.bmp",   "savegr.bmp",   490,  55, 120, 40, SAVE_ID },
	{ "Restore",  "restorup.bmp", "restordn.bmp", "restorgr.bmp", "restorgr.bmp", 490, 110, 120, 40, RESTORE_ID },
	{ "Restart",  "restarup.bmp", "restardn.bmp", "restarup.bmp", "restarup.bmp", 490, 165, 120, 40, RESTART_ID },
	{ "Quit",     "quitup.bmp",   "quitdn.bmp",   "quitup.bmp",   "quitup.bmp",   490, 220, 120, 40, QUIT_ID },

	{ "Help",     "helpup.bmp",   "helpdn.bmp",   "helpup.bmp",   "helpup.bmp",   490, 275, 120, 40, HELP_ID },
	{ "Credits",  "creditup.bmp", "creditdn.bmp", "creditup.bmp", "creditdn.bmp", 490, 330, 120, 40, CREDITS_ID },
	{ "Okay",     "okayup.bmp",   "okaydn.bmp",   "okayup.bmp",   "okayup.bmp",   490, 385, 120, 40, OKAY_ID },
	{ "Defaults", "dfaultup.bmp", "dfaultdn.bmp", "dfaultup.bmp", "dfaultup.bmp",  72, 360, 120, 40, DEFAULTS_ID }
};

CBagOptWindow::CBagOptWindow() {
	// Inits
	m_pFlythroughs = nullptr;
	m_pPanimations = nullptr;

	m_pMidiVolumeScroll = nullptr;
	m_pWaveVolumeScroll = nullptr;
	m_pCorrectionScroll = nullptr;
	m_pPanSpeedScroll = nullptr;
	m_bDirty = FALSE;

	// CBofDialog Inits
	//
	_pDlgBackground = nullptr;
	_bFirstTime = TRUE;
	_bTempBitmap = FALSE;
	_lFlags = BOFDLG_DEFAULT;
	_bEndDialog = FALSE;

	m_cSystemData.m_bPanimations = TRUE;
	m_cSystemData.m_bFlythroughs = TRUE;
	m_cSystemData.m_nCorrection = 4;
	m_cSystemData.m_nPanSpeed = 1;
	m_cSystemData.m_nMusicVolume = VOLUME_DEFAULT;
	m_cSystemData.m_nSoundVolume = VOLUME_DEFAULT;

	m_cColorScheme.m_cFace = RGB(123, 156, 206);
	m_cColorScheme.m_cHighlight = RGB(255, 255, 255);
	m_cColorScheme.m_cShadow = RGB(255, 255, 255);
	m_cColorScheme.m_cText = RGB(0, 0, 0);
	m_cColorScheme.m_cTextDisabled = RGB(255, 255, 255);
	m_cColorScheme.m_cOutline = RGB(123, 156, 206);

	// Don't need to save the background behind this dialog
	_lFlags &= ~BOFDLG_SAVEBACKGND;
	_nReturnValue = -1;

	for (INT i = 0; i < NUM_SYS_BUTTONS; i++) {
		m_pButtons[i] = nullptr;
	}
}

#if BOF_DEBUG
CBagOptWindow::~CBagOptWindow() {
	Assert(IsValidObject(this));
}
#endif

ERROR_CODE CBagOptWindow::Detach() {
	Assert(IsValidObject(this));

	CBofCursor::Hide();

	// Save any changes that the user made
	SaveOutNewSettings();

	// Destroy all buttons
	for (INT i = 0; i < NUM_SYS_BUTTONS; i++) {
		if (m_pButtons[i] != nullptr) {
			delete m_pButtons[i];
			m_pButtons[i] = nullptr;
		}
	}

	if (m_pFlythroughs != nullptr) {
		delete m_pFlythroughs;
		m_pFlythroughs = nullptr;
	}
	if (m_pPanimations != nullptr) {
		delete m_pPanimations;
		m_pPanimations = nullptr;
	}

	if (m_pMidiVolumeScroll != nullptr) {
		delete m_pMidiVolumeScroll;
		m_pMidiVolumeScroll = nullptr;
	}

	if (m_pWaveVolumeScroll != nullptr) {
		delete m_pWaveVolumeScroll;
		m_pWaveVolumeScroll = nullptr;
	}

	if (m_pCorrectionScroll != nullptr) {
		delete m_pCorrectionScroll;
		m_pCorrectionScroll = nullptr;
	}

	if (m_pPanSpeedScroll != nullptr) {
		delete m_pPanSpeedScroll;
		m_pPanSpeedScroll = nullptr;
	}

	CBofApp::GetApp()->SetPalette(m_pSavePalette);

	return m_errCode;
}

VOID CBagOptWindow::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pRect != nullptr);

	PaintBackdrop(pRect);

	ValidateAnscestors();
}

ERROR_CODE CBagOptWindow::Attach() {
	CBofRect cRect;
	CBofPalette *pPal;

	// Save off the current game's palette
	m_pSavePalette = CBofApp::GetApp()->GetPalette();

	// Insert ours
	pPal = m_pBackdrop->GetPalette();
	CBofApp::GetApp()->SetPalette(pPal);

#if BOF_MAC
	// painting a backdrop is a really good idea right now...
	if (GetBackdrop()) {
		PaintBackdrop();
	}
#endif

	// Paint stuff
	if (m_pBackdrop != nullptr) {
		CBofBitmap cBmp(BuildSysDir("MUSICVOL.BMP"), pPal);
		cBmp.Paint(m_pBackdrop, 30, 30);
	}
	if (m_pBackdrop != nullptr) {
		CBofBitmap cBmp(BuildSysDir("AUDIOVOL.BMP"), pPal);
		cBmp.Paint(m_pBackdrop, 30, 80);
	}
	if (m_pBackdrop != nullptr) {
		CBofBitmap cBmp(BuildSysDir("FLYTHRUS.BMP"), pPal);
		cBmp.Paint(m_pBackdrop, 30, 140);
	}
	if (m_pBackdrop != nullptr) {
		CBofBitmap cBmp(BuildSysDir("PANIMATS.BMP"), pPal);
		cBmp.Paint(m_pBackdrop, 30, 190);
	}
	if (m_pBackdrop != nullptr) {
		CBofBitmap cBmp(BuildSysDir("CORRECTN.BMP"), pPal);
		cBmp.Paint(m_pBackdrop, 30, 250);
	}
	if (m_pBackdrop != nullptr) {
		CBofBitmap cBmp(BuildSysDir("PANSPEED.BMP"), pPal);
		cBmp.Paint(m_pBackdrop, 30, 300);
	}

	// Build all our buttons
	for (INT i = 0; i < NUM_SYS_BUTTONS; i++) {
		Assert(m_pButtons[i] == nullptr);

		if ((m_pButtons[i] = new CBofBmpButton) != nullptr) {
			CBofBitmap *pUp, *pDown, *pFocus, *pDis;

			pUp = LoadBitmap(BuildSysDir(g_stButtons[i].m_pszUp), pPal);
			pDown = LoadBitmap(BuildSysDir(g_stButtons[i].m_pszDown), pPal);
			pFocus = LoadBitmap(BuildSysDir(g_stButtons[i].m_pszFocus), pPal);
			pDis = LoadBitmap(BuildSysDir(g_stButtons[i].m_pszDisabled), pPal);

			m_pButtons[i]->LoadBitmaps(pUp, pDown, pFocus, pDis);

#if BOF_MAC
			// make this our own custom window such that no frame is drawn
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

	CHAR szBuf1[MAX_DIRPATH];
	CHAR szBuf2[MAX_DIRPATH];
	CHAR szBuf3[MAX_DIRPATH];
	CHAR szBuf4[MAX_DIRPATH];
	CHAR szBuf5[MAX_DIRPATH];
	CHAR szBuf6[MAX_DIRPATH];

	Common::strcpy_s(szBuf1, BuildSysDir(BROWN_SCROLL_BKGD));
	Common::strcpy_s(szBuf2, BuildSysDir(BROWN_SCROLL_THMB));
	Common::strcpy_s(szBuf3, BuildSysDir(BROWN_SCROLL_LFUP));
	Common::strcpy_s(szBuf4, BuildSysDir(BROWN_SCROLL_RTUP));
	Common::strcpy_s(szBuf5, BuildSysDir(BROWN_SCROLL_LFDN));
	Common::strcpy_s(szBuf6, BuildSysDir(BROWN_SCROLL_RTDN));

	// Midi volume control
	//
	cRect.SetRect(73, 48, 73 + 120 - 1, 48 + 20 - 1);
	if ((m_pMidiVolumeScroll = new CBofScrollBar) != nullptr) {
		m_pMidiVolumeScroll->Create("", &cRect, this, MIDI_VOL_ID);

		m_pMidiVolumeScroll->LoadBitmaps(szBuf1, szBuf2, szBuf3, szBuf4, szBuf5, szBuf6);
		m_pMidiVolumeScroll->SetScrollRange(VOLUME_MIN, VOLUME_MAX, TRUE);
		m_pMidiVolumeScroll->Show();

	} else {
		ReportError(ERR_MEMORY, "Could not allocate the Midi Volume Scroll Bar");
	}

	// Digital Audio volume control
	cRect.SetRect(73, 98, 73 + 120 - 1, 98 + 20 - 1);
	if ((m_pWaveVolumeScroll = new CBofScrollBar) != nullptr) {
		m_pWaveVolumeScroll->Create("", &cRect, this, WAVE_VOL_ID);

		m_pWaveVolumeScroll->LoadBitmaps(szBuf1, szBuf2, szBuf3, szBuf4, szBuf5, szBuf6);
		m_pWaveVolumeScroll->SetScrollRange(VOLUME_MIN, VOLUME_MAX, TRUE);
		m_pWaveVolumeScroll->Show();

	} else {
		ReportError(ERR_MEMORY, "Could not allocate the Wave Volume Scroll Bar");
	}

	// Pan Correction control
	cRect.SetRect(73, 268, 73 + 120 - 1, 268 + 20 - 1);
	if ((m_pCorrectionScroll = new CBofScrollBar) != nullptr) {
		m_pCorrectionScroll->Create("", &cRect, this, CORRECTION_ID);

		m_pCorrectionScroll->LoadBitmaps(szBuf1, szBuf2, szBuf3, szBuf4, szBuf5, szBuf6);
		m_pCorrectionScroll->SetScrollRange(0, 6, TRUE);
		m_pCorrectionScroll->Show();

	} else {
		ReportError(ERR_MEMORY, "Could not allocate the Pan Correction Scroll Bar");
	}

	// Pan Speed control
	cRect.SetRect(73, 318, 73 + 120 - 1, 318 + 20 - 1);
	if ((m_pPanSpeedScroll = new CBofScrollBar) != nullptr) {
		m_pPanSpeedScroll->Create("", &cRect, this, PANSPEED_ID);

		m_pPanSpeedScroll->LoadBitmaps(szBuf1, szBuf2, szBuf3, szBuf4, szBuf5, szBuf6);
		m_pPanSpeedScroll->SetScrollRange(0, 5, TRUE);
		m_pPanSpeedScroll->Show();

	} else {
		ReportError(ERR_MEMORY, "Could not allocate the Pan Speed Scroll Bar");
	}

	cRect.SetRect(FLYTHROUGHS_LEFT, FLYTHROUGHS_TOP, FLYTHROUGHS_LEFT + CHECKBOX_WIDTH, FLYTHROUGHS_TOP + CHECKBOX_HEIGHT);
	if ((m_pFlythroughs = new CBofCheckButton()) != nullptr) {
		m_pFlythroughs->LoadColorScheme(&m_cColorScheme);
		m_errCode = m_pFlythroughs->Create("", &cRect, this, FLYTHROUGHS_ID);
		m_pFlythroughs->Show();
	}

	cRect.SetRect(PANIMATIONS_LEFT, PANIMATIONS_TOP, PANIMATIONS_LEFT + CHECKBOX_WIDTH, PANIMATIONS_TOP + CHECKBOX_HEIGHT);
	if ((m_pPanimations = new CBofCheckButton()) != nullptr) {
		m_pPanimations->LoadColorScheme(&m_cColorScheme);
		m_errCode = m_pPanimations->Create("", &cRect, this, PAN_CHECK_ID);
		m_pPanimations->Show();
	}

	LoadIniSettings();

	PutDialogData();

	CBofCursor::Show();

	return m_errCode;
}

VOID CBagOptWindow::PutDialogData() {
	Assert(IsValidObject(this));

	if (m_pMidiVolumeScroll != nullptr)
		m_pMidiVolumeScroll->SetPos(m_cSystemData.m_nMusicVolume);

	if (m_pWaveVolumeScroll != nullptr)
		m_pWaveVolumeScroll->SetPos(m_cSystemData.m_nSoundVolume);

	if (m_pCorrectionScroll != nullptr)
		m_pCorrectionScroll->SetPos(m_cSystemData.m_nCorrection);

	if (m_pPanSpeedScroll != nullptr)
		m_pPanSpeedScroll->SetPos(m_cSystemData.m_nPanSpeed);

	if (m_pFlythroughs != nullptr)
		m_pFlythroughs->SetCheck(m_cSystemData.m_bFlythroughs);

	if (m_pPanimations != nullptr)
		m_pPanimations->SetCheck(m_cSystemData.m_bPanimations);
}

VOID CBagOptWindow::GetDialogData() {
	Assert(IsValidObject(this));

	if (m_pMidiVolumeScroll != nullptr) {
		m_cSystemData.m_nMusicVolume = m_pMidiVolumeScroll->GetPos();
	}

	if (m_pWaveVolumeScroll != nullptr) {
		m_cSystemData.m_nSoundVolume = m_pWaveVolumeScroll->GetPos();
	}

	if (m_pCorrectionScroll != nullptr) {
		m_cSystemData.m_nCorrection = m_pCorrectionScroll->GetPos();
	}

	if (m_pPanSpeedScroll != nullptr) {
		m_cSystemData.m_nPanSpeed = m_pPanSpeedScroll->GetPos();
	}

	if (m_pPanimations != nullptr) {
		m_cSystemData.m_bPanimations = m_pPanimations->GetCheck();
	}

	if (m_pFlythroughs != nullptr) {
		m_cSystemData.m_bFlythroughs = m_pFlythroughs->GetCheck();
	}
}

VOID CBagOptWindow::OnBofButton(CBofObject *pObject, INT nState) {
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

	if (nState == BUTTON_CLICKED) {

		CBagMasterWin *pWin;
		CBagel *pApp;
		CBofButton *pButton;
		pButton = (CBofButton *)pObject;

		switch (pButton->GetControlID()) {

		case HELP_ID:
			if ((pApp = CBagel::GetBagApp()) != nullptr) {
				if ((pWin = pApp->GetMasterWnd()) != nullptr) {
					if (pWin->GetCurrentGameWindow() != nullptr) {
						pWin->OnHelp(((CBagStorageDevWnd *)pWin->GetCurrentGameWindow())->GetHelpFilename(), FALSE, this);
					}
				}
			}
			break;

		case CREDITS_ID:
			if ((pApp = CBagel::GetBagApp()) != nullptr) {
				if ((pWin = pApp->GetMasterWnd()) != nullptr) {
					pWin->ShowCreditsDialog(this);
				}
			}
			break;

		case OKAY_ID:
			Close();
			break;

		case DEFAULTS_ID:
			ReturnToDefaults();
			break;

		case QUIT_ID: {

			if ((pApp = CBagel::GetBagApp()) != nullptr) {
				if ((pWin = pApp->GetMasterWnd()) != nullptr) {
					if (pWin->ShowQuitDialog(this)) {
						_nReturnValue = 0;
						KillBackground();
						Close();
					}
				}
			}

			break;
		}

		case SAVE_ID: {
			if ((pApp = CBagel::GetBagApp()) != nullptr) {
				if ((pWin = pApp->GetMasterWnd()) != nullptr) {
					pWin->ShowSaveDialog(this);
				}
			}

			break;
		}

		case RESTORE_ID: {
			if ((pApp = CBagel::GetBagApp()) != nullptr) {
				if ((pWin = pApp->GetMasterWnd()) != nullptr) {
					if (pWin->ShowRestoreDialog(this)) {

						// Can't restore a deleted palette
						m_pSavePalette = nullptr;

						KillBackground();
						Close();
					}
				}
			}
			break;
		}

		case RESTART_ID: {
			if ((pApp = CBagel::GetBagApp()) != nullptr) {
				if ((pWin = pApp->GetMasterWnd()) != nullptr) {

					if (pWin->ShowRestartDialog(this)) {

						// Can't restore a deleted palette
						m_pSavePalette = nullptr;

						KillBackground();
						Close();
					}
				}
			}
			break;
		}

		default:
			LogWarning("Unknown button pressed");
			break;

		}

	} else {
		CBofButton *pButton;
		pButton = (CBofButton *)pObject;

		switch (pButton->GetControlID()) {
		case FLYTHROUGHS_ID:
			m_cSystemData.m_bFlythroughs = (pButton->GetState() == BUTTON_CHECKED);
			m_bDirty = TRUE;
			break;

		case PAN_CHECK_ID:
			m_cSystemData.m_bPanimations = (pButton->GetState() == BUTTON_CHECKED);
			m_bDirty = TRUE;
			break;

		default:
			break;
		}
	}
}

VOID CBagOptWindow::OnBofScrollBar(CBofObject *pObj, INT nPos) {
	Assert(IsValidObject(this));

	CBofScrollBar *pScroll;
	pScroll = (CBofScrollBar *)pObj;

	if (pScroll == m_pMidiVolumeScroll) {
		m_cSystemData.m_nMusicVolume = nPos;
		CBofSound::SetVolume(m_cSystemData.m_nMusicVolume, m_cSystemData.m_nSoundVolume);

	} else if (pScroll == m_pWaveVolumeScroll) {
		m_cSystemData.m_nSoundVolume = nPos;
		CBofSound::SetVolume(m_cSystemData.m_nMusicVolume, m_cSystemData.m_nSoundVolume);

	} else if (pScroll == m_pCorrectionScroll) {
		m_cSystemData.m_nCorrection = nPos;

	} else if (pScroll == m_pPanSpeedScroll) {
		m_cSystemData.m_nPanSpeed = nPos;
	}
}

VOID CBagOptWindow::SaveOutNewSettings() {
	Assert(IsValidObject(this));

	CBagel *pApp;

	// Write out current system settings
	//
	if ((pApp = CBagel::GetBagApp()) != nullptr) {

		pApp->SetOption(USER_OPTIONS, "Panimations", m_cSystemData.m_bPanimations);
		pApp->SetOption(USER_OPTIONS, "FlyThroughs", m_cSystemData.m_bFlythroughs);

		pApp->SetOption(USER_OPTIONS, "Correction", 6 - m_cSystemData.m_nCorrection);
		pApp->SetOption(USER_OPTIONS, "PanSpeed", m_cSystemData.m_nPanSpeed);
		pApp->SetOption(USER_OPTIONS, "MidiVolume", m_cSystemData.m_nMusicVolume);
		pApp->SetOption(USER_OPTIONS, WAVE_VOLUME, m_cSystemData.m_nSoundVolume);

		CBagMasterWin *pWin;

		// Set current Pan correction
		if ((pWin = pApp->GetMasterWnd()) != nullptr) {
			CBagStorageDevWnd *pSDev;
			CBagPanWindow *pPan;
			INT n;

			n = pWin->GetCorrection();
			if ((pSDev = pWin->GetCurrentStorageDev()) != nullptr) {
				if (pSDev->GetDeviceType() == SDEV_GAMEWIN) {
					pPan = (CBagPanWindow *)pSDev;

					CBagPanWindow::SetRealCorrection(n);
					if (pPan->GetSlideBitmap() != nullptr) {
						pPan->GetSlideBitmap()->SetCorrWidth(n);
					}
				}
			}
		}
	}

	CBofSound::SetVolume(m_cSystemData.m_nMusicVolume, m_cSystemData.m_nSoundVolume);
	CBagPanWindow::SetPanSpeed(m_cSystemData.m_nPanSpeed);

#if BOF_MAC || BOF_WINMAC
	pApp->Commit();
#endif

	m_bDirty = FALSE;
}

VOID CBagOptWindow::LoadIniSettings() {
	Assert(IsValidObject(this));

	CBagel *pApp;

	// Read in current system settings
	if ((pApp = CBagel::GetBagApp()) != nullptr) {
		INT nTemp;

		pApp->GetOption(USER_OPTIONS, "Panimations", &m_cSystemData.m_bPanimations, TRUE);
		pApp->GetOption(USER_OPTIONS, "FlyThroughs", &m_cSystemData.m_bFlythroughs, TRUE);

		// Correction
		pApp->GetOption(USER_OPTIONS, "Correction", &nTemp, 2);

		if (nTemp < 0 || nTemp > 6)
			nTemp = 2;

		m_cSystemData.m_nCorrection = 6 - nTemp;

		// Pan speed
		pApp->GetOption(USER_OPTIONS, "PanSpeed", &nTemp, 1);

		if (nTemp < 0 || nTemp > 5)
			nTemp = 1;
		m_cSystemData.m_nPanSpeed = nTemp;

		// Midi Volume
		pApp->GetOption(USER_OPTIONS, "MidiVolume", &nTemp, VOLUME_DEFAULT);

		if ((nTemp < VOLUME_MIN) || (nTemp > VOLUME_MAX)) {
			nTemp = VOLUME_DEFAULT;
		}
		m_cSystemData.m_nMusicVolume = nTemp;

		// Digital Audio Volume
		pApp->GetOption(USER_OPTIONS, WAVE_VOLUME, &nTemp, VOLUME_DEFAULT);

		if ((nTemp < VOLUME_MIN) || (nTemp > VOLUME_MAX)) {
			nTemp = VOLUME_DEFAULT;
		}

		m_cSystemData.m_nSoundVolume = nTemp;
	}
}

VOID CBagOptWindow::ReturnToDefaults() {
	Assert(IsValidObject(this));

	m_cSystemData.m_bPanimations = TRUE;
	m_cSystemData.m_bFlythroughs = TRUE;
	m_cSystemData.m_nCorrection = 4;
	m_cSystemData.m_nPanSpeed = 1;
	m_cSystemData.m_nMusicVolume = VOLUME_DEFAULT;
	m_cSystemData.m_nSoundVolume = VOLUME_DEFAULT;

	PutDialogData();

	m_bDirty = TRUE;
}

VOID CBagOptWindow::OnInitDialog() {
	Assert(IsValidObject(this));

	CBofDialog::OnInitDialog();

	Attach();
}

VOID CBagOptWindow::OnKeyHit(ULONG lKey, ULONG lRepCount) {
	Assert(IsValidObject(this));

	CBagMasterWin *pWin;
	CBagel *pApp;

	switch (lKey) {
	// Help
	//
	case BKEY_F1:
		if ((pApp = CBagel::GetBagApp()) != nullptr) {
			if ((pWin = pApp->GetMasterWnd()) != nullptr) {
				if (pWin->GetCurrentGameWindow() != nullptr) {
					pWin->OnHelp(((CBagStorageDevWnd *)pWin->GetCurrentGameWindow())->GetHelpFilename(), FALSE, this);
				}
			}
		}
		break;

	// Save
	//
	case BKEY_ALT_S:
	case BKEY_ALT_s:
	case BKEY_F2:
	case BKEY_SAVE:
		if ((pApp = CBagel::GetBagApp()) != nullptr) {
			if ((pWin = pApp->GetMasterWnd()) != nullptr) {
				pWin->ShowSaveDialog(this);
			}
		}
		break;

	// Restore
	//
	case BKEY_ALT_R:
	case BKEY_ALT_r:
	case BKEY_RESTORE:
		if ((pApp = CBagel::GetBagApp()) != nullptr) {
			if ((pWin = pApp->GetMasterWnd()) != nullptr) {
				if (pWin->ShowRestoreDialog(this)) {

					// Can't restore a deleted palette
					m_pSavePalette = nullptr;

					KillBackground();
					Close();
				}
			}
		}
		break;

	// Restart
	//
	case BKEY_F12:
		if ((pApp = CBagel::GetBagApp()) != nullptr) {
			if ((pWin = pApp->GetMasterWnd()) != nullptr) {

				if (pWin->ShowRestartDialog(this)) {

					// Can't restore a deleted palette
					m_pSavePalette = nullptr;

					KillBackground();
					Close();
				}
			}
		}
		break;

	// Quit
	//
	case BKEY_ALT_F4:
	case BKEY_ALT_Q:
	case BKEY_ALT_q: {
		if ((pApp = CBagel::GetBagApp()) != nullptr) {
			if ((pWin = pApp->GetMasterWnd()) != nullptr) {
				if (pWin->ShowQuitDialog(this)) {
					_nReturnValue = 0;
					KillBackground();
					Close();
				}
			}
		}

		break;
	}

	// Close
	//
	//case BKEY_ENTER:
	case BKEY_ESC:
		Close();
		break;

	default:
		CBofDialog::OnKeyHit(lKey, lRepCount);
		break;
	}
}

const CHAR *BuildSysDir(const CHAR *pszFile) {
	Assert(pszFile != nullptr);
	static CHAR szBuf[MAX_DIRPATH];

	// Get the path to the system directory
	Common::sprintf_s(szBuf, "$SBARDIR%sGENERAL%sSYSTEM%s%s", PATH_DELIMETER, PATH_DELIMETER, PATH_DELIMETER, pszFile);

	CBofString cTemp(szBuf, MAX_DIRPATH);
	MACROREPLACE(cTemp);

	return &szBuf[0];
}

} // namespace Bagel
