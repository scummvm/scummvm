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
#include "bagel/spacebar/dialogs/opt_window.h"
#include "bagel/spacebar/dialogs/save_dialog.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/boflib/sound.h"
#include "bagel/spacebar/baglib/storage_dev_win.h"
#include "bagel/spacebar/baglib/pan_window.h"
#include "bagel/spacebar/boflib/std_keys.h"
#include "bagel/bagel.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"

namespace Bagel {
namespace SpaceBar {

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
	_pFlythroughs = nullptr;
	_pPanimations = nullptr;

	_pMidiVolumeScroll = nullptr;
	_pWaveVolumeScroll = nullptr;
	_pCorrectionScroll = nullptr;
	_pPanSpeedScroll = nullptr;
	_bDirty = false;

	// CBofDialog Inits
	_pDlgBackground = nullptr;
	_bFirstTime = true;
	_bTempBitmap = false;
	_lFlags = BOFDLG_DEFAULT;
	_bEndDialog = false;

	_cSystemData._bPanimations = true;
	_cSystemData._bFlythroughs = true;
	_cSystemData._nCorrection = 4;
	_cSystemData._nPanSpeed = 1;
	_cSystemData._nMusicVolume = VOLUME_DEFAULT;
	_cSystemData._nSoundVolume = VOLUME_DEFAULT;

	_cColorScheme._cFace = RGB(123, 156, 206);
	_cColorScheme._cHighlight = RGB(255, 255, 255);
	_cColorScheme._cShadow = RGB(255, 255, 255);
	_cColorScheme._cText = RGB(0, 0, 0);
	_cColorScheme._cTextDisabled = CTEXT_WHITE;
	_cColorScheme._cOutline = RGB(123, 156, 206);

	// Don't need to save the background behind this dialog
	_lFlags &= ~BOFDLG_SAVEBACKGND;
	_nReturnValue = -1;

	for (int i = 0; i < NUM_SYS_BUTTONS; i++) {
		_pButtons[i] = nullptr;
	}

	_pSavePalette = nullptr;
}

ErrorCode CBagOptWindow::detach() {
	assert(isValidObject(this));

	CBagCursor::hideSystemCursor();

	// Save any changes that the user made
	saveOutNewSettings();

	// Destroy all buttons
	for (int i = 0; i < NUM_SYS_BUTTONS; i++) {
		delete _pButtons[i];
		_pButtons[i] = nullptr;
	}

	delete _pFlythroughs;
	_pFlythroughs = nullptr;

	delete _pPanimations;
	_pPanimations = nullptr;

	delete _pMidiVolumeScroll;
	_pMidiVolumeScroll = nullptr;

	delete _pWaveVolumeScroll;
	_pWaveVolumeScroll = nullptr;

	delete _pCorrectionScroll;
	_pCorrectionScroll = nullptr;

	delete _pPanSpeedScroll;
	_pPanSpeedScroll = nullptr;

	CBofApp::getApp()->setPalette(_pSavePalette);

	return _errCode;
}

void CBagOptWindow::onPaint(CBofRect *pRect) {
	assert(isValidObject(this));
	assert(pRect != nullptr);

	paintBackdrop(pRect);

	validateAnscestors();
}

ErrorCode CBagOptWindow::attach() {
	// Save off the current game's palette
	_pSavePalette = CBofApp::getApp()->getPalette();
	CBofPalette *pPal = nullptr;

	// Insert ours
	if (_pBackdrop != nullptr) {
		pPal = _pBackdrop->getPalette();
		CBofApp::getApp()->setPalette(pPal);
	}

	// Paint stuff
	if (_pBackdrop != nullptr) {
		CBofBitmap cBmp(buildSysDir("MUSICVOL.BMP"), pPal);
		cBmp.paint(_pBackdrop, 30, 30);
	}
	if (_pBackdrop != nullptr) {
		CBofBitmap cBmp(buildSysDir("AUDIOVOL.BMP"), pPal);
		cBmp.paint(_pBackdrop, 30, 80);
	}
	if (_pBackdrop != nullptr) {
		CBofBitmap cBmp(buildSysDir("FLYTHRUS.BMP"), pPal);
		cBmp.paint(_pBackdrop, 30, 140);
	}
	if (_pBackdrop != nullptr) {
		CBofBitmap cBmp(buildSysDir("PANIMATS.BMP"), pPal);
		cBmp.paint(_pBackdrop, 30, 190);
	}
	if (_pBackdrop != nullptr) {
		CBofBitmap cBmp(buildSysDir("CORRECTN.BMP"), pPal);
		cBmp.paint(_pBackdrop, 30, 250);
	}
	if (_pBackdrop != nullptr) {
		CBofBitmap cBmp(buildSysDir("PANSPEED.BMP"), pPal);
		cBmp.paint(_pBackdrop, 30, 300);
	}

	// Build all our buttons
	for (int i = 0; i < NUM_SYS_BUTTONS; i++) {
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

	char szBuf1[MAX_DIRPATH];
	char szBuf2[MAX_DIRPATH];
	char szBuf3[MAX_DIRPATH];
	char szBuf4[MAX_DIRPATH];
	char szBuf5[MAX_DIRPATH];
	char szBuf6[MAX_DIRPATH];

	Common::strcpy_s(szBuf1, buildSysDir(BROWN_SCROLL_BKGD));
	Common::strcpy_s(szBuf2, buildSysDir(BROWN_SCROLL_THMB));
	Common::strcpy_s(szBuf3, buildSysDir(BROWN_SCROLL_LFUP));
	Common::strcpy_s(szBuf4, buildSysDir(BROWN_SCROLL_RTUP));
	Common::strcpy_s(szBuf5, buildSysDir(BROWN_SCROLL_LFDN));
	Common::strcpy_s(szBuf6, buildSysDir(BROWN_SCROLL_RTDN));

	// Midi volume control
	CBofRect cRect;
	cRect.setRect(73, 48, 73 + 120 - 1, 48 + 20 - 1);
	_pMidiVolumeScroll = new CBofScrollBar;
	_pMidiVolumeScroll->create("", &cRect, this, MIDI_VOL_ID);
	_pMidiVolumeScroll->loadBitmaps(szBuf1, szBuf2, szBuf3, szBuf4, szBuf5, szBuf6);
	_pMidiVolumeScroll->setScrollRange(VOLUME_MIN, VOLUME_MAX, true);
	_pMidiVolumeScroll->show();

	// Digital Audio volume control
	cRect.setRect(73, 98, 73 + 120 - 1, 98 + 20 - 1);
	_pWaveVolumeScroll = new CBofScrollBar;
	_pWaveVolumeScroll->create("", &cRect, this, WAVE_VOL_ID);
	_pWaveVolumeScroll->loadBitmaps(szBuf1, szBuf2, szBuf3, szBuf4, szBuf5, szBuf6);
	_pWaveVolumeScroll->setScrollRange(VOLUME_MIN, VOLUME_MAX, true);
	_pWaveVolumeScroll->show();

	// Pan Correction control
	cRect.setRect(73, 268, 73 + 120 - 1, 268 + 20 - 1);
	_pCorrectionScroll = new CBofScrollBar;
	_pCorrectionScroll->create("", &cRect, this, CORRECTION_ID);
	_pCorrectionScroll->loadBitmaps(szBuf1, szBuf2, szBuf3, szBuf4, szBuf5, szBuf6);
	_pCorrectionScroll->setScrollRange(0, 6, true);
	_pCorrectionScroll->show();

	// Pan Speed control
	cRect.setRect(73, 318, 73 + 120 - 1, 318 + 20 - 1);
	_pPanSpeedScroll = new CBofScrollBar;
	_pPanSpeedScroll->create("", &cRect, this, PANSPEED_ID);
	_pPanSpeedScroll->loadBitmaps(szBuf1, szBuf2, szBuf3, szBuf4, szBuf5, szBuf6);
	_pPanSpeedScroll->setScrollRange(0, 5, true);
	_pPanSpeedScroll->show();

	cRect.setRect(FLYTHROUGHS_LEFT, FLYTHROUGHS_TOP, FLYTHROUGHS_LEFT + CHECKBOX_WIDTH, FLYTHROUGHS_TOP + CHECKBOX_HEIGHT);
	_pFlythroughs = new CBofCheckButton();
	_pFlythroughs->loadColorScheme(&_cColorScheme);
	_errCode = _pFlythroughs->create("", &cRect, this, FLYTHROUGHS_ID);
	_pFlythroughs->show();

	cRect.setRect(PANIMATIONS_LEFT, PANIMATIONS_TOP, PANIMATIONS_LEFT + CHECKBOX_WIDTH, PANIMATIONS_TOP + CHECKBOX_HEIGHT);
	_pPanimations = new CBofCheckButton();
	_pPanimations->loadColorScheme(&_cColorScheme);
	_errCode = _pPanimations->create("", &cRect, this, PAN_CHECK_ID);
	_pPanimations->show();

	loadIniSettings();
	putDialogData();
	CBagCursor::showSystemCursor();

	return _errCode;
}

void CBagOptWindow::putDialogData() {
	assert(isValidObject(this));

	if (_pMidiVolumeScroll != nullptr)
		_pMidiVolumeScroll->setPos(_cSystemData._nMusicVolume, true, true);

	if (_pWaveVolumeScroll != nullptr)
		_pWaveVolumeScroll->setPos(_cSystemData._nSoundVolume, true, true);

	if (_pCorrectionScroll != nullptr)
		_pCorrectionScroll->setPos(_cSystemData._nCorrection, true, true);

	if (_pPanSpeedScroll != nullptr)
		_pPanSpeedScroll->setPos(_cSystemData._nPanSpeed, true, true);

	if (_pFlythroughs != nullptr)
		_pFlythroughs->SetCheck(_cSystemData._bFlythroughs);

	if (_pPanimations != nullptr)
		_pPanimations->SetCheck(_cSystemData._bPanimations);
}

void CBagOptWindow::getDialogData() {
	assert(isValidObject(this));

	if (_pMidiVolumeScroll != nullptr) {
		_cSystemData._nMusicVolume = _pMidiVolumeScroll->getPos();
	}

	if (_pWaveVolumeScroll != nullptr) {
		_cSystemData._nSoundVolume = _pWaveVolumeScroll->getPos();
	}

	if (_pCorrectionScroll != nullptr) {
		_cSystemData._nCorrection = _pCorrectionScroll->getPos();
	}

	if (_pPanSpeedScroll != nullptr) {
		_cSystemData._nPanSpeed = _pPanSpeedScroll->getPos();
	}

	if (_pPanimations != nullptr) {
		_cSystemData._bPanimations = _pPanimations->GetCheck();
	}

	if (_pFlythroughs != nullptr) {
		_cSystemData._bFlythroughs = _pFlythroughs->GetCheck();
	}
}

void CBagOptWindow::onBofButton(CBofObject *pObject, int nState) {
	assert(isValidObject(this));
	assert(pObject != nullptr);

	if (nState == BUTTON_CLICKED) {
		CBofButton *pButton = (CBofButton *)pObject;

		CBagMasterWin *pWin;
		CBagel *pApp;
		switch (pButton->getControlID()) {
		case HELP_ID:
			pApp = CBagel::getBagApp();
			if (pApp != nullptr) {
				pWin = pApp->getMasterWnd();
				if ((pWin != nullptr) && pWin->getCurrentGameWindow() != nullptr) {
					pWin->onHelp(((CBagStorageDevWnd *)pWin->getCurrentGameWindow())->getHelpFilename(), false, this);
				}
			}
			break;

		case CREDITS_ID:
			pApp = CBagel::getBagApp();
			if (pApp != nullptr) {
				pWin = pApp->getMasterWnd();
				if (pWin != nullptr) {
					pWin->showCreditsDialog(this);
				}
			}
			break;

		case OKAY_ID:
			close();
			break;

		case DEFAULTS_ID:
			returnToDefaults();
			break;

		case QUIT_ID: {
			pApp = CBagel::getBagApp();
			if (pApp != nullptr) {
				pWin = pApp->getMasterWnd();
				if ((pWin != nullptr) && pWin->showQuitDialog(this)) {
					_nReturnValue = 0;
					killBackground();
					close();
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
					pWin->showSaveDialog(this);
				}
			}

			break;
		}

		case RESTORE_ID: {
			pApp = CBagel::getBagApp();
			if (pApp != nullptr) {
				pWin = pApp->getMasterWnd();
				if ((pWin != nullptr) && pWin->showRestoreDialog(this)) {
					// Can't restore a deleted palette
					_pSavePalette = nullptr;

					killBackground();
					close();
				}
			}
			break;
		}

		case RESTART_ID: {
			pApp = CBagel::getBagApp();
			if (pApp != nullptr) {
				pWin = pApp->getMasterWnd();
				if ((pWin != nullptr) && pWin->showRestartDialog(this)) {
					// Can't restore a deleted palette
					_pSavePalette = nullptr;

					killBackground();
					close();
				}
			}
			break;
		}

		default:
			logWarning("Unknown button pressed");
			break;

		}

	} else {
		CBofButton *pButton = (CBofButton *)pObject;

		switch (pButton->getControlID()) {
		case FLYTHROUGHS_ID:
			_cSystemData._bFlythroughs = (pButton->getState() == BUTTON_CHECKED);
			_bDirty = true;
			break;

		case PAN_CHECK_ID:
			_cSystemData._bPanimations = (pButton->getState() == BUTTON_CHECKED);
			_bDirty = true;
			break;

		default:
			break;
		}
	}
}

void CBagOptWindow::onBofScrollBar(CBofObject *pObj, int nPos) {
	assert(isValidObject(this));

	CBofScrollBar *pScroll = (CBofScrollBar *)pObj;

	if (pScroll == _pMidiVolumeScroll) {
		_cSystemData._nMusicVolume = nPos;
		CBofSound::setVolume(_cSystemData._nMusicVolume, _cSystemData._nSoundVolume);

	} else if (pScroll == _pWaveVolumeScroll) {
		_cSystemData._nSoundVolume = nPos;
		CBofSound::setVolume(_cSystemData._nMusicVolume, _cSystemData._nSoundVolume);

	} else if (pScroll == _pCorrectionScroll) {
		_cSystemData._nCorrection = nPos;

	} else if (pScroll == _pPanSpeedScroll) {
		_cSystemData._nPanSpeed = nPos;
	}
}

void CBagOptWindow::saveOutNewSettings() {
	assert(isValidObject(this));

	CBagel *pApp = CBagel::getBagApp();

	// Write out current system settings
	ConfMan.setBool("Panimations", _cSystemData._bPanimations);
	ConfMan.setBool("FlyThroughs", _cSystemData._bFlythroughs);

	ConfMan.setInt("Correction", 6 - _cSystemData._nCorrection);
	ConfMan.setInt("PanSpeed", _cSystemData._nPanSpeed);
	ConfMan.setInt("music_volume", VOLUME_SVM(_cSystemData._nMusicVolume));
	ConfMan.setInt("sfx_volume", VOLUME_SVM(_cSystemData._nSoundVolume));
	ConfMan.flushToDisk();

	if (pApp) {
		CBagMasterWin *pWin = pApp->getMasterWnd();

		// Set current Pan correction
		if (pWin != nullptr) {

			int n = pWin->getCorrection();
			CBagStorageDevWnd *pSDev = pWin->getCurrentStorageDev();
			if ((pSDev != nullptr) && pSDev->getDeviceType() == SDEV_GAMEWIN) {
				CBagPanWindow *pPan = (CBagPanWindow *)pSDev;

				CBagPanWindow::setRealCorrection(n);
				if (pPan->getSlideBitmap() != nullptr) {
					pPan->getSlideBitmap()->setCorrWidth(n);
				}
			}
		}
	}

	CBofSound::setVolume(_cSystemData._nMusicVolume, _cSystemData._nSoundVolume);
	CBagPanWindow::setPanSpeed(_cSystemData._nPanSpeed);

	_bDirty = false;
}

void CBagOptWindow::loadIniSettings() {
	assert(isValidObject(this));

	ConfMan.registerDefault("Panimations", true);
	ConfMan.registerDefault("FlyThroughs", true);
	ConfMan.registerDefault("Correction", 2);
	ConfMan.registerDefault("PanSpeed", 1);

	// Read in current system settings
	_cSystemData._bPanimations = ConfMan.getBool("Panimations");
	_cSystemData._bFlythroughs = ConfMan.getBool("FlyThroughs");

	int nTemp = ConfMan.getInt("Correction");
	if (nTemp < 0 || nTemp > 6)
		nTemp = 2;
	_cSystemData._nCorrection = 6 - nTemp;

	// Pan speed
	nTemp = ConfMan.getInt("PanSpeed");
	if (nTemp < 0 || nTemp > 5)
		nTemp = 1;
	_cSystemData._nPanSpeed = nTemp;

	// Midi Volume
	int musVolume = ConfMan.getBool("music_mute") ? 0 : CLIP(ConfMan.getInt("music_volume"), 0, 255);
	_cSystemData._nMusicVolume = SVM_VOLUME(musVolume);

	// Digital Audio Volume
	int sfxVolume = ConfMan.getBool("sfx_mute") ? 0 : CLIP(ConfMan.getInt("sfx_volume"), 0, 255);
	_cSystemData._nSoundVolume = SVM_VOLUME(sfxVolume);
}

void CBagOptWindow::returnToDefaults() {
	assert(isValidObject(this));

	_cSystemData._bPanimations = true;
	_cSystemData._bFlythroughs = true;
	_cSystemData._nCorrection = 4;
	_cSystemData._nPanSpeed = 1;
	_cSystemData._nMusicVolume = VOLUME_DEFAULT;
	_cSystemData._nSoundVolume = VOLUME_DEFAULT;

	putDialogData();

	_bDirty = true;
}

void CBagOptWindow::onInitDialog() {
	assert(isValidObject(this));

	CBofDialog::onInitDialog();

	attach();
}

void CBagOptWindow::onKeyHit(uint32 lKey, uint32 lRepCount) {
	assert(isValidObject(this));

	CBagMasterWin *pWin;
	CBagel *pApp;

	switch (lKey) {
	// Help
	case BKEY_F1:
		pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			pWin = pApp->getMasterWnd();
			if ((pWin != nullptr) && (pWin->getCurrentGameWindow() != nullptr)) {
				pWin->onHelp(((CBagStorageDevWnd *)pWin->getCurrentGameWindow())->getHelpFilename(), false, this);
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
				pWin->showSaveDialog(this);
			}
		}
		break;

	// Restore
	case BKEY_ALT_r:
	case BKEY_RESTORE:
		pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			pWin = pApp->getMasterWnd();
			if ((pWin != nullptr) && pWin->showRestoreDialog(this)) {
				// Can't restore a deleted palette
				_pSavePalette = nullptr;

				killBackground();
				close();
			}
		}
		break;

	// Restart
	case BKEY_F12:
		pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			pWin = pApp->getMasterWnd();
			if ((pWin != nullptr) && pWin->showRestartDialog(this)) {
				// Can't restore a deleted palette
				_pSavePalette = nullptr;

				killBackground();
				close();
			}
		}
		break;

	// Quit
	case BKEY_ALT_F4:
	case BKEY_ALT_q: {
		pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			pWin = pApp->getMasterWnd();
			if ((pWin != nullptr) && pWin->showQuitDialog(this)) {
				_nReturnValue = 0;
				killBackground();
				close();
			}
		}

		break;
	}

	// Close
	case BKEY_ESC:
		close();
		break;

	default:
		CBofDialog::onKeyHit(lKey, lRepCount);
		break;
	}
}

const char *buildSysDir(const char *pszFile) {
	assert(pszFile != nullptr);
	static char szBuf[MAX_DIRPATH];

	// Get the path to the system directory
	Common::sprintf_s(szBuf, "$SBARDIR%sGENERAL%sSYSTEM%s%s", PATH_DELIMETER, PATH_DELIMETER, PATH_DELIMETER, pszFile);

	CBofString cTemp(szBuf, MAX_DIRPATH);
	fixPathName(cTemp);

	return &szBuf[0];
}

} // namespace SpaceBar
} // namespace Bagel
