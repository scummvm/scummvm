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

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/audiocfg.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/menures.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {

#define ID_SCROLL1   105
#define ID_MUSIC     109
#define ID_SOUND     110


static  CColorButton *pOKButton = nullptr;                     // OKAY button on scroll
static  CColorButton *pCancelButton = nullptr;                 // Cancel button on scroll


CAudioCfgDlg::CAudioCfgDlg(CWnd *pParent, CPalette *pPalette, unsigned int nID)
	: CBmpDialog(pParent, pPalette, IDD_AUDIOCFG, ".\\ART\\SSCROLL.BMP") {
	m_pScrollBar1 = nullptr;

	DoModal();
}

void CAudioCfgDlg::DoDataExchange(CDataExchange *pDX) {
	CBmpDialog::DoDataExchange(pDX);
}


void CAudioCfgDlg::PutDlgData() {
	m_pScrollBar1->SetScrollPos(m_nVolume);

	m_pMusicButton->SetCheck(m_bMusic);
	m_pSoundButton->SetCheck(m_bSound);
}


void CAudioCfgDlg::GetDlgData() {
	m_nVolume   = m_pScrollBar1->GetScrollPos();

	m_bMusic = false;
	if (m_pMusicButton->GetCheck() == 1) {
		m_bMusic = true;
	}

	m_bSound = false;
	if (m_pSoundButton->GetCheck() == 1) {
		m_bSound = true;
	}
}


bool CAudioCfgDlg::OnCommand(WPARAM wParam, LPARAM lParam) {
	/*
	* respond to audio
	*/
	if (HIWORD(lParam) == BN_CLICKED) {

		switch (wParam) {

		case IDOK:
			SaveIniSettings();
			PostMessage(WM_CLOSE, 0, 0);
			return false;

		case IDCANCEL:
			PostMessage(WM_CLOSE, 0, 0);
			return false;

		case ID_MUSIC:
			m_bMusic = !m_bMusic;
			PutDlgData();
			break;

		case ID_SOUND:
			m_bSound = !m_bSound;
			PutDlgData();
			break;

		default:
			break;
		}
	}

	return CBmpDialog::OnCommand(wParam, lParam);
}

void CAudioCfgDlg::OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar *pScroll) {
	int nMin = 0, nMax = 0, nVal = 0;

	if (pScroll == m_pScrollBar1) {

		nMin = LEVEL_MIN;
		nMax = LEVEL_MAX;
		nVal = m_nVolume;
	}

	switch (nSBCode) {

	case SB_LEFT:
		nVal = nMin;
		break;

	case SB_PAGELEFT:
		nVal -= PAGE_SIZE;
		break;

	case SB_LINELEFT:
		if (nVal > nMin)
			nVal--;
		break;

	case SB_RIGHT:
		nVal = nMax;
		break;

	case SB_PAGERIGHT:
		nVal += PAGE_SIZE;
		break;

	case SB_LINERIGHT:
		if (nVal < nMax)
			nVal++;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nVal = nPos;
		break;

	default:
		break;
	}

	if (nVal < nMin)
		nVal = nMin;
	if (nVal > nMax)
		nVal = nMax;

	pScroll->SetScrollPos(nVal);

	if (pScroll == m_pScrollBar1) {
		m_nVolume = nVal;
		UpdateOptions();
	} else
		assert(0);
}


bool CAudioCfgDlg::OnInitDialog() {
	CRect tmpRect;
	CDC *pDC;

	CBmpDialog::OnInitDialog();

	if ((pDC = GetDC()) != nullptr) {

		tmpRect.SetRect(54, 85, 168, 98);
		if ((m_pTxtVolume = new CText) != nullptr) {
			m_pTxtVolume->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
		}

		tmpRect.SetRect(54, 100, 168, 118);
		if ((m_pScrollBar1 = new CScrollBar) != nullptr) {
			m_pScrollBar1->Create(WS_VISIBLE | WS_CHILD | SBS_HORZ | SBS_BOTTOMALIGN, tmpRect, this, ID_SCROLL1);
			m_pScrollBar1->SetScrollRange(LEVEL_MIN, LEVEL_MAX, true);
		}

		ReleaseDC(pDC);
	}

	if ((pOKButton = new CColorButton) != nullptr) {           // build a color QUIT button to let us exit
		pOKButton->SetPalette(m_pPalette);                  // set the palette to use
		pOKButton->SetControl(IDOK, this);                  // tie to the dialog control
	}

	if ((pCancelButton = new CColorButton) != nullptr) {       // build a color QUIT button to let us exit
		pCancelButton->SetPalette(m_pPalette);              // set the palette to use
		pCancelButton->SetControl(IDCANCEL, this);          // tie to the dialog control
	}

	if ((m_pMusicButton = new CCheckButton) != nullptr) {
		m_pMusicButton->SetPalette(m_pPalette);
		m_pMusicButton->SetControl(ID_MUSIC, this);
		if (!CSound::MidiAvailable())
			m_pMusicButton->EnableWindow(false);
	}

	if ((m_pSoundButton = new CCheckButton) != nullptr) {
		m_pSoundButton->SetPalette(m_pPalette);
		m_pSoundButton->SetControl(ID_SOUND, this);
		if (!CSound::SoundAvailable())
			m_pSoundButton->EnableWindow(false);
	}

	LoadIniSettings();

	PutDlgData();

	if (!CSound::SoundVolumeAvailable() && !CSound::MidiVolumeAvailable())
		m_pScrollBar1->EnableWindow(false);

	return true;
}


void CAudioCfgDlg::OnPaint() {
	CBmpDialog::OnPaint();

	UpdateOptions();
}


void CAudioCfgDlg::UpdateOptions() {
	char buf[40];
	CDC *pDC;

	if ((pDC = GetDC()) != nullptr) {
		if (m_pTxtVolume != nullptr) {
			if (CSound::SoundVolumeAvailable() || CSound::MidiVolumeAvailable())
				Common::sprintf_s(buf, "Volume: %d", m_nVolume);
			else
				Common::strcpy_s(buf, "Volume control not available");
			m_pTxtVolume->DisplayString(pDC, buf, 13, TEXT_NORMAL, RGB(0, 0, 0));
		}
		ReleaseDC(pDC);
	}
}


bool CAudioCfgDlg::OnEraseBkgnd(CDC *) {
	return true;
}


void CAudioCfgDlg::OnClose() {
	if (pOKButton != nullptr) {                          // release the button
		delete pOKButton;
		pOKButton = nullptr;
	}

	if (pCancelButton != nullptr) {                        // release the button
		delete pCancelButton;
		pCancelButton = nullptr;
	}

	if (m_pTxtVolume != nullptr) {
		delete m_pTxtVolume;
		m_pTxtVolume = nullptr;
	}

	if (m_pScrollBar1 != nullptr) {
		delete m_pScrollBar1;
		m_pScrollBar1 = nullptr;
	}

	if (m_pMusicButton != nullptr) {
		delete m_pMusicButton;
		m_pMusicButton = nullptr;
	}

	if (m_pSoundButton != nullptr) {
		delete m_pSoundButton;
		m_pSoundButton = nullptr;
	}

	ClearDialogImage();
	EndDialog(0);
}

void CAudioCfgDlg::ClearDialogImage() {
	if (pOKButton != nullptr) {                          // release the button
		delete pOKButton;
		pOKButton = nullptr;
	}

	if (pCancelButton != nullptr) {                        // release the button
		delete pCancelButton;
		pCancelButton = nullptr;
	}

	ValidateRect(nullptr);
}


void CAudioCfgDlg::LoadIniSettings() {
	int nMidiVolume, nWaveVolume;

	m_bMusic = GetPrivateProfileInt("Meta", "Music", true, "HODJPODJ.INI");
	m_bSound = GetPrivateProfileInt("Meta", "SoundEffects", true, "HODJPODJ.INI");

	nMidiVolume = GetPrivateProfileInt("Meta", "MidiVolume", LEVEL_DEF, "HODJPODJ.INI");
	nWaveVolume = GetPrivateProfileInt("Meta", "WaveVolume", LEVEL_DEF, "HODJPODJ.INI");

	if (nWaveVolume > nMidiVolume)
		m_nVolume = nWaveVolume;
	else
		m_nVolume = nMidiVolume;

	if ((m_nVolume < LEVEL_MIN) || (m_nVolume > LEVEL_MAX))
		m_nVolume = LEVEL_DEF;

	CSound::setVolume(m_nVolume, m_nVolume);
}

void CAudioCfgDlg::SaveIniSettings() {
	WritePrivateProfileString("Meta", "Music", Common::String::format("%d", m_bMusic ? 1 : 0).c_str(), "HODJPODJ.INI");
	WritePrivateProfileString("Meta", "SoundEffects", Common::String::format("%d", m_bSound ? 1 : 0).c_str(), "HODJPODJ.INI");
	WritePrivateProfileString("Meta", "WaveVolume", Common::String::format("%d", m_nVolume).c_str(), "HODJPODJ.INI");
	WritePrivateProfileString("Meta", "MidiVolume", Common::String::format("%d", m_nVolume).c_str(), "HODJPODJ.INI");

	CSound::setVolume(m_nVolume, m_nVolume);
}

BEGIN_MESSAGE_MAP(CAudioCfgDlg, CBmpDialog)
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_WM_HSCROLL()
	ON_WM_PAINT()
END_MESSAGE_MAP()

} // namespace HodjNPodj
} // namespace Bagel
