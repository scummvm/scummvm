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

#include "bagel/spacebar/vid_wnd.h"
#include "bagel/spacebar/main_window.h"
#include "bagel/baglib/bagel.h"
#include "bagel/baglib/master_win.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/stdinc.h"

namespace Bagel {
namespace SpaceBar {

#define PLAY_INC    (0.1)
#define FF_INC      (1.0)
#define REW_INC     (-1.0)

#define START_TIME1 174000
#define END_TIME1   225000

#define CLOSEUP_VIDEO_DIR     "$SBARDIR\\BAR\\CLOSEUP\\BRCC2\\"

#define NUM_FRAME_TYPES 11

struct ST_FRAMES {
	int _nFrame;
	double _fStart;
	double _fEnd;
	int _nUseDisc;
};

ST_FRAMES g_stFrames[NUM_FRAME_TYPES] = {
	{1, 0, 30, 0},
	{2, 30, 60, 0},
	{3, 60, 90, 0},
	{4, 90, 120, 0},
	{5, 120, 150, 0},
	{6, 150, 180, 0},
	{7, 180, 210, 1},
	{-1, 180, 202.2, 2},
	{229, 202.2, 210, 2},
	{230, 210, 240, 0}
};

// Local functions
const char *BuildVidDir(const char *pszFile);

SBarVidWnd::SBarVidWnd() {
	_pTimerVar = nullptr;
	_pPlayingVar = nullptr;
	_pDiscVar = nullptr;

	_pMovie = nullptr;
	_fTimerDiff = 0;

	_fTimer = 0.0;
	_nStartTime = 186000 - 180;
}


SBarVidWnd::~SBarVidWnd() {
	assert(isValidObject(this));

	SBarVidWnd::detach();
}

ErrorCode SBarVidWnd::attach() {
	assert(isValidObject(this));

	if (CMainWindow::attach() == ERR_NONE) {
		_pDiscVar = g_VarManager->getVariable("CUR_VDISC");
		_pTimerVar = g_VarManager->getVariable("CUR_VTIME");

		// What time does the murder occur?
		CBagVar *pVar = g_VarManager->getVariable("VDISC_EVTIME");
		if (pVar != nullptr) {
			_nStartTime = pVar->getNumValue();
			_nStartTime -= 180;
		}

		if (_pTimerVar != nullptr) {
			_fTimer = _pTimerVar->getNumValue();
		}

		if (_pMovie != nullptr) {
			_pMovie->detach();
			delete _pMovie;
			_pMovie = nullptr;
		}

		_pMovie = new CBagCharacterObject;
		_pMovie->setFileName(BuildVidDir("BRNL.SMK"));
		_pMovie->setPosition(CBofPoint(209, 10));
		_pMovie->attach();

		_fTimerDiff = 0;

		_pPlayingVar = g_VarManager->getVariable("VDISC_PLAYING");
		if (_pPlayingVar != nullptr) {
			int nMode = _pPlayingVar->getNumValue();

			switch (nMode) {
			case 1:
				_fTimerDiff = PLAY_INC;
				break;

			case 2:
				_fTimerDiff = FF_INC;
				break;

			case 3:
				_fTimerDiff = REW_INC;
				break;

			default:
				_fTimerDiff = 0;
				break;
			}
		}
	}

	return _errCode;
}


ErrorCode SBarVidWnd::detach() {
	assert(isValidObject(this));

	if (_pMovie != nullptr) {
		_pMovie->detach();
		delete _pMovie;
		_pMovie = nullptr;
	}

	CBagVar *pTimerVar = g_VarManager->getVariable("CUR_VTIME");
	if (pTimerVar != nullptr) {
		pTimerVar->setValue((int)_fTimer);
	}

	_pTimerVar = nullptr;
	_pPlayingVar = nullptr;
	_pDiscVar = nullptr;

	CMainWindow::detach();

	return _errCode;
}


void SBarVidWnd::setPlayMode(int nMode) {
	assert(isValidObject(this));

	if (_pPlayingVar != nullptr) {
		_pPlayingVar->setValue(nMode);
	}

	// If user is playing the disk with the death scene on it, then
	// reflect that in the script.
	if (nMode != 0 && _pDiscVar != nullptr && _pDiscVar->getNumValue() == 2) {
		CBagVar *pVar = g_VarManager->getVariable("VIDDISC_SEEN");
		if (pVar != nullptr) {
			pVar->setValue(1);
		}
	}
}


bool SBarVidWnd::hasDisc() {
	assert(isValidObject(this));

	// If either disk is in the vid player
	bool bHaveDisc = false;
	if ((_pDiscVar != nullptr) && (_pDiscVar->getNumValue() != 0)) {
		bHaveDisc = true;
	}

	return bHaveDisc;
}

int SBarVidWnd::getFrame(double fTime, int nUseDisc) {
	int nFrame = 0;

	fTime -= _nStartTime;
	for (int i = 0; i < NUM_FRAME_TYPES; i++) {

		if (g_stFrames[i]._nUseDisc == nUseDisc || (g_stFrames[i]._nUseDisc == 0)) {

			if ((fTime >= g_stFrames[i]._fStart) && (fTime < g_stFrames[i]._fEnd)) {
				nFrame = g_stFrames[i]._nFrame;
				if (nFrame == -1) {
					nFrame = int(8 + (fTime - 180) * 10);
				}
				break;
			}
		}
	}

	return nFrame;
}


ErrorCode SBarVidWnd::onRender(CBofBitmap *pBmp, CBofRect *pRect) {
	assert(isValidObject(this));
	assert(pBmp != nullptr);

	setPreFilterPan(true);

	CMainWindow::onRender(pBmp, pRect);

	// If the disc is in Play, FastForward, or Rewind mode
	if (hasDisc() && _pPlayingVar != nullptr && _pPlayingVar->getNumValue() != 0) {
		static uint32 nLastTime = 0;

		if (getTimer() >= nLastTime + 100) {

			_fTimer += _fTimerDiff;

			if (_fTimer < _nStartTime) {
				_fTimer = _nStartTime;

			} else if (_fTimer > _nStartTime + 240) {
				_fTimer = _nStartTime + 240;
			}
			nLastTime = getTimer();
		}

		// Keep BAGEL up to date with this info
		if (_pTimerVar != nullptr) {
			_pTimerVar->setValue((int)_fTimer);
		}

		CBofRect cRect(344, 195, 462, 210);
		char szBuf[12];

		int nDisc = 1;
		if (_pDiscVar != nullptr) {
			nDisc = _pDiscVar->getNumValue();
			assert(nDisc != 0);
		}

		//
		// Show current image of video (based on time)
		//

		int nFrame = getFrame(_fTimer, nDisc);

		_pMovie->setCurrentFrame(nFrame);
		_pMovie->update(pBmp, _pMovie->getPosition());

		//
		// Display current time for that image
		//
		int nTimer = (int)_fTimer;
		int nHr = nTimer / 10000;
		int nMn = (nTimer - (nHr * 10000)) / 100;
		int nSc = nTimer - ((nHr * 10000) + (nMn * 100));

		Common::sprintf_s(szBuf, "%02d:%02d:%02d", nHr, nMn, nSc);

		paintText(pBmp, &cRect, szBuf, 14, TEXT_NORMAL, CTEXT_WHITE, JUSTIFY_WRAP, FORMAT_DEFAULT, FONT_MONO);
	}

	return _errCode;
}

CBagObject *SBarVidWnd::onNewButtonObject(const CBofString &) {
	assert(isValidObject(this));

	SBarVidBut *pVidButtObj = new SBarVidBut();

	return pVidButtObj;
}


const char *BuildVidDir(const char *pszFile) {
	return formPath(CLOSEUP_VIDEO_DIR, pszFile);
}


void SBarVidBut::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *pInfo) {
	switch (getRefId()) {

	case VID_PLAY_BUT: {
		SBarVidWnd *vidWnd = (SBarVidWnd *)pInfo;
		if (vidWnd->hasDisc()) {
			vidWnd->setPlayMode(1);
			vidWnd->setInc(PLAY_INC);
		}
		} break;

	case VID_FF_BUT: {
		SBarVidWnd *vidWnd = (SBarVidWnd *)pInfo;
		if (vidWnd->hasDisc()) {
			vidWnd->setPlayMode(2);
			vidWnd->setInc(FF_INC);
		}
		}
		break;

	case VID_REW_BUT: {
		SBarVidWnd *vidWnd = (SBarVidWnd *)pInfo;
		if (vidWnd->hasDisc()) {
			vidWnd->setPlayMode(3);
			vidWnd->setInc(REW_INC);
		}
		}
		break;

	case VID_STOP_BUT: {
		SBarVidWnd *vidWnd = (SBarVidWnd *)pInfo;
		vidWnd->setPlayMode(0);
		vidWnd->setInc(0);
		}
		break;

	default:
		break;
	}

	CBagButtonObject::onLButtonUp(nFlags, xPoint, pInfo);
}

} // namespace SpaceBar
} // namespace Bagel
