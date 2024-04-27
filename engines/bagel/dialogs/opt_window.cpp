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

#include "common/config-manager.h"
#include "bagel/dialogs/opt_window.h"
#include "bagel/dialogs/save_dialog.h"
#include "bagel/baglib/bagel.h"
#include "bagel/boflib/sound.h"
#include "bagel/baglib/storage_dev_win.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/bagel.h"
#include "bagel/boflib/log.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {

#define USER_OPTIONS "UserOptions"

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
	int m_nID;
};

static const ST_BUTTONS g_stButtons[NUM_SYS_BUTTONS] = {
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
	m_bDirty = false;

	// CBofDialog Inits
	_pDlgBackground = nullptr;
	_bFirstTime = true;
	_bTempBitmap = false;
	_lFlags = BOFDLG_DEFAULT;
	_bEndDialog = false;

	m_cSystemData.m_bPanimations = true;
	m_cSystemData.m_bFlythroughs = true;
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

	for (int i = 0; i < NUM_SYS_BUTTONS; i++) {
		m_pButtons[i] = nullptr;
	}
}

ErrorCode CBagOptWindow::Detach() {
	Assert(IsValidObject(this));

	CBagCursor::HideSystemCursor();

	// Save any changes that the user made
	SaveOutNewSettings();

	// Destroy all buttons
	for (int i = 0; i < NUM_SYS_BUTTONS; i++) {
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

void CBagOptWindow::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pRect != nullptr);

	PaintBackdrop(pRect);

	ValidateAnscestors();
}

ErrorCode CBagOptWindow::attach() {
	// Save off the current game's palette
	m_pSavePalette = CBofApp::GetApp()->GetPalette();

	// Insert ours
	CBofPalette *pPal = m_pBackdrop->GetPalette();
	CBofApp::GetApp()->SetPalette(pPal);

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
	for (int i = 0; i < NUM_SYS_BUTTONS; i++) {
		Assert(m_pButtons[i] == nullptr);

		if ((m_pButtons[i] = new CBofBmpButton) != nullptr) {
			CBofBitmap *pUp, *pDown, *pFocus, *pDis;

			pUp = LoadBitmap(BuildSysDir(g_stButtons[i].m_pszUp), pPal);
			pDown = LoadBitmap(BuildSysDir(g_stButtons[i].m_pszDown), pPal);
			pFocus = LoadBitmap(BuildSysDir(g_stButtons[i].m_pszFocus), pPal);
			pDis = LoadBitmap(BuildSysDir(g_stButtons[i].m_pszDisabled), pPal);

			m_pButtons[i]->LoadBitmaps(pUp, pDown, pFocus, pDis);

			m_pButtons[i]->Create(g_stButtons[i].m_pszName, g_stButtons[i].m_nLeft, g_stButtons[i].m_nTop, g_stButtons[i].m_nWidth, g_stButtons[i].m_nHeight, this, g_stButtons[i].m_nID);
			m_pButtons[i]->Show();
		} else {
			ReportError(ERR_MEMORY);
			break;
		}
	}

	char szBuf1[MAX_DIRPATH];
	char szBuf2[MAX_DIRPATH];
	char szBuf3[MAX_DIRPATH];
	char szBuf4[MAX_DIRPATH];
	char szBuf5[MAX_DIRPATH];
	char szBuf6[MAX_DIRPATH];

	Common::strcpy_s(szBuf1, BuildSysDir(BROWN_SCROLL_BKGD));
	Common::strcpy_s(szBuf2, BuildSysDir(BROWN_SCROLL_THMB));
	Common::strcpy_s(szBuf3, BuildSysDir(BROWN_SCROLL_LFUP));
	Common::strcpy_s(szBuf4, BuildSysDir(BROWN_SCROLL_RTUP));
	Common::strcpy_s(szBuf5, BuildSysDir(BROWN_SCROLL_LFDN));
	Common::strcpy_s(szBuf6, BuildSysDir(BROWN_SCROLL_RTDN));

	// Midi volume control
	CBofRect cRect;
	cRect.SetRect(73, 48, 73 + 120 - 1, 48 + 20 - 1);
	if ((m_pMidiVolumeScroll = new CBofScrollBar) != nullptr) {
		m_pMidiVolumeScroll->Create("", &cRect, this, MIDI_VOL_ID);

		m_pMidiVolumeScroll->LoadBitmaps(szBuf1, szBuf2, szBuf3, szBuf4, szBuf5, szBuf6);
		m_pMidiVolumeScroll->SetScrollRange(VOLUME_MIN, VOLUME_MAX, true);
		m_pMidiVolumeScroll->Show();

	} else {
		ReportError(ERR_MEMORY, "Could not allocate the Midi Volume Scroll Bar");
	}

	// Digital Audio volume control
	cRect.SetRect(73, 98, 73 + 120 - 1, 98 + 20 - 1);
	if ((m_pWaveVolumeScroll = new CBofScrollBar) != nullptr) {
		m_pWaveVolumeScroll->Create("", &cRect, this, WAVE_VOL_ID);

		m_pWaveVolumeScroll->LoadBitmaps(szBuf1, szBuf2, szBuf3, szBuf4, szBuf5, szBuf6);
		m_pWaveVolumeScroll->SetScrollRange(VOLUME_MIN, VOLUME_MAX, true);
		m_pWaveVolumeScroll->Show();

	} else {
		ReportError(ERR_MEMORY, "Could not allocate the Wave Volume Scroll Bar");
	}

	// Pan Correction control
	cRect.SetRect(73, 268, 73 + 120 - 1, 268 + 20 - 1);
	if ((m_pCorrectionScroll = new CBofScrollBar) != nullptr) {
		m_pCorrectionScroll->Create("", &cRect, this, CORRECTION_ID);

		m_pCorrectionScroll->LoadBitmaps(szBuf1, szBuf2, szBuf3, szBuf4, szBuf5, szBuf6);
		m_pCorrectionScroll->SetScrollRange(0, 6, true);
		m_pCorrectionScroll->Show();

	} else {
		ReportError(ERR_MEMORY, "Could not allocate the Pan Correction Scroll Bar");
	}

	// Pan Speed control
	cRect.SetRect(73, 318, 73 + 120 - 1, 318 + 20 - 1);
	if ((m_pPanSpeedScroll = new CBofScrollBar) != nullptr) {
		m_pPanSpeedScroll->Create("", &cRect, this, PANSPEED_ID);

		m_pPanSpeedScroll->LoadBitmaps(szBuf1, szBuf2, szBuf3, szBuf4, szBuf5, szBuf6);
		m_pPanSpeedScroll->SetScrollRange(0, 5, true);
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

	CBagCursor::ShowSystemCursor();

	return m_errCode;
}

void CBagOptWindow::PutDialogData() {
	Assert(IsValidObject(this));

	if (m_pMidiVolumeScroll != nullptr)
		m_pMidiVolumeScroll->SetPos(m_cSystemData.m_nMusicVolume, true, true);

	if (m_pWaveVolumeScroll != nullptr)
		m_pWaveVolumeScroll->SetPos(m_cSystemData.m_nSoundVolume, true, true);

	if (m_pCorrectionScroll != nullptr)
		m_pCorrectionScroll->SetPos(m_cSystemData.m_nCorrection, true, true);

	if (m_pPanSpeedScroll != nullptr)
		m_pPanSpeedScroll->SetPos(m_cSystemData.m_nPanSpeed, true, true);

	if (m_pFlythroughs != nullptr)
		m_pFlythroughs->SetCheck(m_cSystemData.m_bFlythroughs);

	if (m_pPanimations != nullptr)
		m_pPanimations->SetCheck(m_cSystemData.m_bPanimations);
}

void CBagOptWindow::GetDialogData() {
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

void CBagOptWindow::OnBofButton(CBofObject *pObject, int nState) {
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

	if (nState == BUTTON_CLICKED) {
		CBofButton *pButton = (CBofButton *)pObject;

		CBagMasterWin *pWin;
		CBagel *pApp;
		switch (pButton->GetControlID()) {
		case HELP_ID:
			pApp = CBagel::getBagApp();
			if (pApp != nullptr) {
				pWin = pApp->getMasterWnd();
				if ((pWin != nullptr) && pWin->GetCurrentGameWindow() != nullptr) {
					pWin->OnHelp(((CBagStorageDevWnd *)pWin->GetCurrentGameWindow())->GetHelpFilename(), false, this);
				}
			}
			break;

		case CREDITS_ID:
			pApp = CBagel::getBagApp();
			if (pApp != nullptr) {
				pWin = pApp->getMasterWnd();
				if (pWin != nullptr) {
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
			pApp = CBagel::getBagApp();
			if (pApp != nullptr) {
				pWin = pApp->getMasterWnd();
				if ((pWin != nullptr) &&  pWin->ShowQuitDialog(this)) {
					_nReturnValue = 0;
					KillBackground();
					Close();
					g_engine->quitGame();
				}
			}

			break;
		}

		case SAVE_ID: {
			pApp = CBagel::getBagApp();
			if (pApp != nullptr) {
				pWin = pApp->getMasterWnd();
				if (pWin != nullptr) {
					pWin->ShowSaveDialog(this);
				}
			}

			break;
		}

		case RESTORE_ID: {
			pApp = CBagel::getBagApp();
			if (pApp != nullptr) {
				pWin = pApp->getMasterWnd();
				if ((pWin != nullptr) && pWin->ShowRestoreDialog(this)) {
					// Can't restore a deleted palette
					m_pSavePalette = nullptr;

					KillBackground();
					Close();
				}
			}
			break;
		}

		case RESTART_ID: {
			pApp = CBagel::getBagApp();
			if (pApp != nullptr) {
				pWin = pApp->getMasterWnd();
				if ((pWin != nullptr) && pWin->ShowRestartDialog(this)) {
					// Can't restore a deleted palette
					m_pSavePalette = nullptr;

					KillBackground();
					Close();
				}
			}
			break;
		}

		default:
			LogWarning("Unknown button pressed");
			break;

		}

	} else {
		CBofButton *pButton = (CBofButton *)pObject;

		switch (pButton->GetControlID()) {
		case FLYTHROUGHS_ID:
			m_cSystemData.m_bFlythroughs = (pButton->GetState() == BUTTON_CHECKED);
			m_bDirty = true;
			break;

		case PAN_CHECK_ID:
			m_cSystemData.m_bPanimations = (pButton->GetState() == BUTTON_CHECKED);
			m_bDirty = true;
			break;

		default:
			break;
		}
	}
}

void CBagOptWindow::OnBofScrollBar(CBofObject *pObj, int nPos) {
	Assert(IsValidObject(this));

	CBofScrollBar *pScroll = (CBofScrollBar *)pObj;

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

void CBagOptWindow::SaveOutNewSettings() {
	Assert(IsValidObject(this));

	CBagel *pApp = CBagel::getBagApp();

	// Write out current system settings
	ConfMan.setBool("Panimations", m_cSystemData.m_bPanimations);
	ConfMan.setBool("FlyThroughs", m_cSystemData.m_bFlythroughs);

	ConfMan.setInt("Correction", 6 - m_cSystemData.m_nCorrection);
	ConfMan.setInt("PanSpeed", m_cSystemData.m_nPanSpeed);
	ConfMan.setInt("music_volume", VOLUME_SVM(m_cSystemData.m_nMusicVolume));
	ConfMan.setInt("sfx_volume", VOLUME_SVM(m_cSystemData.m_nSoundVolume));
	ConfMan.flushToDisk();

	if (pApp) {
		CBagMasterWin *pWin = pApp->getMasterWnd();

		// Set current Pan correction
		if (pWin != nullptr) {

			int n = pWin->GetCorrection();
			CBagStorageDevWnd *pSDev = pWin->GetCurrentStorageDev();
			if ((pSDev != nullptr) && pSDev->GetDeviceType() == SDEV_GAMEWIN) {
				CBagPanWindow *pPan = (CBagPanWindow *)pSDev;

				CBagPanWindow::SetRealCorrection(n);
				if (pPan->GetSlideBitmap() != nullptr) {
					pPan->GetSlideBitmap()->SetCorrWidth(n);
				}
			}
		}
	}

	CBofSound::SetVolume(m_cSystemData.m_nMusicVolume, m_cSystemData.m_nSoundVolume);
	CBagPanWindow::SetPanSpeed(m_cSystemData.m_nPanSpeed);

	m_bDirty = false;
}

void CBagOptWindow::LoadIniSettings() {
	Assert(IsValidObject(this));
	int nTemp;

	ConfMan.registerDefault("Panimations", true);
	ConfMan.registerDefault("FlyThroughs", true);
	ConfMan.registerDefault("Correction", 2);
	ConfMan.registerDefault("PanSpeed", 1);

	// Read in current system settings
	m_cSystemData.m_bPanimations = ConfMan.getBool("Panimations");
	m_cSystemData.m_bFlythroughs = ConfMan.getBool("FlyThroughs");

	nTemp = ConfMan.getInt("Correction");
	if (nTemp < 0 || nTemp > 6)
		nTemp = 2;
	m_cSystemData.m_nCorrection = 6 - nTemp;

	// Pan speed
	nTemp = ConfMan.getInt("PanSpeed");
	if (nTemp < 0 || nTemp > 5)
		nTemp = 1;
	m_cSystemData.m_nPanSpeed = nTemp;

	// Midi Volume
	int musVolume = ConfMan.getBool("music_mute") ? 0 : CLIP(ConfMan.getInt("music_volume"), 0, 255);
	m_cSystemData.m_nMusicVolume = SVM_VOLUME(musVolume);

	// Digital Audio Volume
	int sfxVolume = ConfMan.getBool("sfx_mute") ? 0 : CLIP(ConfMan.getInt("sfx_volume"), 0, 255);
	m_cSystemData.m_nSoundVolume = SVM_VOLUME(sfxVolume);
}

void CBagOptWindow::ReturnToDefaults() {
	Assert(IsValidObject(this));

	m_cSystemData.m_bPanimations = true;
	m_cSystemData.m_bFlythroughs = true;
	m_cSystemData.m_nCorrection = 4;
	m_cSystemData.m_nPanSpeed = 1;
	m_cSystemData.m_nMusicVolume = VOLUME_DEFAULT;
	m_cSystemData.m_nSoundVolume = VOLUME_DEFAULT;

	PutDialogData();

	m_bDirty = true;
}

void CBagOptWindow::OnInitDialog() {
	Assert(IsValidObject(this));

	CBofDialog::OnInitDialog();

	attach();
}

void CBagOptWindow::OnKeyHit(uint32 lKey, uint32 lRepCount) {
	Assert(IsValidObject(this));

	CBagMasterWin *pWin;
	CBagel *pApp;

	switch (lKey) {
	// Help
	case BKEY_F1:
		pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			pWin = pApp->getMasterWnd();
			if ((pWin != nullptr) && (pWin->GetCurrentGameWindow() != nullptr)) {
				pWin->OnHelp(((CBagStorageDevWnd *)pWin->GetCurrentGameWindow())->GetHelpFilename(), false, this);
			}
		}
		break;

	// Save
	case BKEY_ALT_s:
	case BKEY_F2:
	case BKEY_SAVE:
		pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			pWin = pApp->getMasterWnd();
			if (pWin != nullptr) {
				pWin->ShowSaveDialog(this);
			}
		}
		break;

	// Restore
	case BKEY_ALT_r:
	case BKEY_RESTORE:
		pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			pWin = pApp->getMasterWnd();
			if ((pWin != nullptr) && pWin->ShowRestoreDialog(this)) {
				// Can't restore a deleted palette
				m_pSavePalette = nullptr;

				KillBackground();
				Close();
			}
		}
		break;

	// Restart
	case BKEY_F12:
		pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			pWin = pApp->getMasterWnd();
			if ((pWin != nullptr) && pWin->ShowRestartDialog(this)) {
				// Can't restore a deleted palette
				m_pSavePalette = nullptr;

				KillBackground();
				Close();
			}
		}
		break;

	// Quit
	case BKEY_ALT_F4:
	case BKEY_ALT_q: {
		pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			pWin = pApp->getMasterWnd();
			if ((pWin != nullptr) && pWin->ShowQuitDialog(this)) {
				_nReturnValue = 0;
				KillBackground();
				Close();
			}
		}

		break;
	}

	// Close
	case BKEY_ESC:
		Close();
		break;

	default:
		CBofDialog::OnKeyHit(lKey, lRepCount);
		break;
	}
}

const char *BuildSysDir(const char *pszFile) {
	Assert(pszFile != nullptr);
	static char szBuf[MAX_DIRPATH];

	// Get the path to the system directory
	Common::sprintf_s(szBuf, "$SBARDIR%sGENERAL%sSYSTEM%s%s", PATH_DELIMETER, PATH_DELIMETER, PATH_DELIMETER, pszFile);

	CBofString cTemp(szBuf, MAX_DIRPATH);
	MACROREPLACE(cTemp);

	return &szBuf[0];
}

} // namespace Bagel
