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
#include "bagel/boflib/stdinc.h"

namespace Bagel {
namespace SpaceBar {

#define PLAY_INC    (0.1)
#define FF_INC      (1.0)
#define REW_INC     (-1.0)

#define START_TIME1 174000
#define END_TIME1   225000

#define CLOSEUPVIDEODIR     "$SBARDIR\\BAR\\CLOSEUP\\BRCC2\\"

#define NUM_FRAME_TYPES 11

struct ST_FRAMES {
	INT m_nFrame;
	double m_fStart;
	double m_fEnd;
	INT m_nUseDisc;
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
const CHAR *BuildVidDir(const CHAR *pszFile);

SBarVidWnd::SBarVidWnd() {
	m_pTimerVar = nullptr;
	m_pPlayingVar = nullptr;
	m_pDiscVar = nullptr;

	m_pMovie = nullptr;
	m_fTimerDiff = 0;

	m_fTimer = 0.0;
	m_nStartTime = 186000 - 180;
}


SBarVidWnd::~SBarVidWnd() {
	Assert(IsValidObject(this));

	Detach();
}

ERROR_CODE SBarVidWnd::Attach() {
	Assert(IsValidObject(this));

	CBagVar *pVar;

	if (CMainWindow::Attach() == ERR_NONE) {

		m_pDiscVar = VARMNGR->GetVariable("CUR_VDISC");

		m_pTimerVar = VARMNGR->GetVariable("CUR_VTIME");

		// What time does the murder occurr?
		//
		pVar = VARMNGR->GetVariable("VDISC_EVTIME");
		if (pVar != nullptr) {
			m_nStartTime = pVar->GetNumValue();
			m_nStartTime -= 180;
		}

		if (m_pTimerVar != nullptr) {
			m_fTimer = m_pTimerVar->GetNumValue();
		}
		if (m_pMovie != nullptr) {
			m_pMovie->Detach();
			delete m_pMovie;
			m_pMovie = nullptr;
		}
		if ((m_pMovie = new CBagCharacterObject) != nullptr) {
			m_pMovie->SetFileName(BuildVidDir("BRNL.SMK"));
			m_pMovie->SetPosition(CBofPoint(209, 10));
			m_pMovie->Attach();

		} else {
			ReportError(ERR_MEMORY);
		}

		m_fTimerDiff = 0;

		if ((m_pPlayingVar = VARMNGR->GetVariable("VDISC_PLAYING")) != nullptr) {
			INT nMode;
			nMode = m_pPlayingVar->GetNumValue();

			switch (nMode) {
			case 1:
				m_fTimerDiff = PLAY_INC;
				break;

			case 2:
				m_fTimerDiff = FF_INC;
				break;

			case 3:
				m_fTimerDiff = REW_INC;
				break;

			default:
				m_fTimerDiff = 0;
				break;
			}

		}

	}

	return m_errCode;
}


ERROR_CODE SBarVidWnd::Detach() {
	Assert(IsValidObject(this));

	if (m_pMovie != nullptr) {
		m_pMovie->Detach();
		delete m_pMovie;
		m_pMovie = nullptr;
	}

	CBagVar *pTimerVar;

	if ((pTimerVar = VARMNGR->GetVariable("CUR_VTIME")) != nullptr) {
		pTimerVar->SetValue((INT)m_fTimer);
	}

	m_pTimerVar = nullptr;
	m_pPlayingVar = nullptr;
	m_pDiscVar = nullptr;

	CMainWindow::Detach();

	return m_errCode;
}


VOID SBarVidWnd::SetPlayMode(INT nMode) {
	Assert(IsValidObject(this));

	if (m_pPlayingVar != nullptr) {
		m_pPlayingVar->SetValue(nMode);
	}

	// If user is playing the disk with the death scene on it, then
	// reflect that in the script.
	//
	if (nMode != 0 && m_pDiscVar != nullptr) {
		if (m_pDiscVar->GetNumValue() == 2) {
			CBagVar *pVar;

			pVar = VARMNGR->GetVariable("VIDDISC_SEEN");
			if (pVar != nullptr) {
				pVar->SetValue(1);
			}
		}
	}
}


BOOL SBarVidWnd::HasDisc() {
	Assert(IsValidObject(this));

	BOOL bHaveDisc;

	// If either disk is in the vid player

	bHaveDisc = FALSE;
	if (m_pDiscVar != nullptr) {

		if (m_pDiscVar->GetNumValue() != 0) {
			bHaveDisc = TRUE;
		}
	}

	return bHaveDisc;
}



INT SBarVidWnd::GetFrame(double fTime, INT nUseDisc) {
	INT i, nFrame;

	nFrame = 0;

	fTime -= m_nStartTime;

	for (i = 0; i < NUM_FRAME_TYPES; i++) {

		if (g_stFrames[i].m_nUseDisc == nUseDisc || (g_stFrames[i].m_nUseDisc == 0)) {

			if ((fTime >= g_stFrames[i].m_fStart) && (fTime < g_stFrames[i].m_fEnd)) {

				nFrame = g_stFrames[i].m_nFrame;

				if (nFrame == -1) {
					nFrame = INT(8 + (fTime - 180) * 10);
				}
				break;
			}
		}
	}
	return nFrame;
}


ERROR_CODE SBarVidWnd::OnRender(CBofBitmap *pBmp, CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);

	SetPreFilterPan(TRUE);

	CMainWindow::OnRender(pBmp, pRect);

	// If the disc is in Play, FastForward, or Rewind mode
	//
	if (HasDisc() && m_pPlayingVar != nullptr && m_pPlayingVar->GetNumValue() != 0) {

		static uint32 nLastTime = 0;

		if (GetTimer() >= nLastTime + 100) {

			m_fTimer += m_fTimerDiff;

			if (m_fTimer < m_nStartTime) {
				m_fTimer = m_nStartTime;

			} else if (m_fTimer > m_nStartTime + 240) {
				m_fTimer = m_nStartTime + 240;
			}
			nLastTime = GetTimer();
		}

		// Keep BAGEL up to date with this info
		//
		if (m_pTimerVar != nullptr) {
			m_pTimerVar->SetValue((INT)m_fTimer);
		}

		CBofRect cRect(344, 195, 462, 210);
		CHAR szBuf[12];
		INT nHr, nMn, nSc, nTimer;
		INT nFrame, nDisc;

		nDisc = 1;
		if (m_pDiscVar != nullptr) {
			nDisc = m_pDiscVar->GetNumValue();
			Assert(nDisc != 0);
		}

		//
		// Show current image of video (based on time)
		//

		nFrame = GetFrame(m_fTimer, nDisc);

		m_pMovie->SetCurrentFrame(nFrame);
		m_pMovie->Update(pBmp, m_pMovie->GetPosition());

		//
		// Display current time for that image
		//
		nTimer = (INT)m_fTimer;
		nHr = nTimer / 10000;
		nMn = (nTimer - (nHr * 10000)) / 100;
		nSc = nTimer - ((nHr * 10000) + (nMn * 100));

		Common::sprintf_s(szBuf, "%02d:%02d:%02d", nHr, nMn, nSc);

		PaintText(pBmp, &cRect, szBuf, 14, TEXT_NORMAL, RGB(255, 255, 255), JUSTIFY_WRAP, FORMAT_DEFAULT, FONT_MONO);
	}

	return m_errCode;
}

CBagObject *SBarVidWnd::OnNewButtonObject(const CBofString &) {
	Assert(IsValidObject(this));

	SBarVidBut *pVidButtObj;

	pVidButtObj = new SBarVidBut();

	return pVidButtObj;
}


const CHAR *BuildVidDir(const CHAR *pszFile) {
	Assert(pszFile != nullptr);

	static CHAR szBuf[MAX_DIRPATH];

	Common::sprintf_s(szBuf, "%s%s%s", CLOSEUPVIDEODIR, PATH_DELIMETER, pszFile);

	CBofString cStr(szBuf, MAX_DIRPATH);
	MACROREPLACE(cStr);

	return &szBuf[0];
}


void SBarVidBut::OnLButtonUp(uint32 nFlags, CBofPoint *xPoint, VOID *pInfo) {
	switch (GetRefId()) {

	case VID_PLAY_BUT:

		if (((SBarVidWnd *)pInfo)->HasDisc()) {
			((SBarVidWnd *)pInfo)->SetPlayMode(1);
			((SBarVidWnd *)pInfo)->SetInc(PLAY_INC);
		}
		break;

	case VID_FF_BUT:
		if (((SBarVidWnd *)pInfo)->HasDisc()) {
			((SBarVidWnd *)pInfo)->SetPlayMode(2);
			((SBarVidWnd *)pInfo)->SetInc(FF_INC);
		}
		break;

	case VID_REW_BUT:
		if (((SBarVidWnd *)pInfo)->HasDisc()) {
			((SBarVidWnd *)pInfo)->SetPlayMode(3);
			((SBarVidWnd *)pInfo)->SetInc(REW_INC);
		}
		break;

	case VID_STOP_BUT:
		((SBarVidWnd *)pInfo)->SetPlayMode(0);
		((SBarVidWnd *)pInfo)->SetInc(0);
		break;

	default:
		break;
	}

	CBagButtonObject::OnLButtonUp(nFlags, xPoint, pInfo);
}

} // namespace SpaceBar
} // namespace Bagel
