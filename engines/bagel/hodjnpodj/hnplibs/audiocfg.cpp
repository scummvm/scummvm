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
#define ID_MUSIC	 109
#define ID_SOUND	 110


static	CColorButton *pOKButton = NULL;						// OKAY button on scroll
static	CColorButton *pCancelButton = NULL;					// Cancel button on scroll


CAudioCfgDlg::CAudioCfgDlg(CWnd *pParent, CPalette *pPalette, UINT nID)
        : CBmpDialog(pParent, pPalette, IDD_AUDIOCFG, ".\\ART\\SSCROLL.BMP")
{
    m_pScrollBar1 = NULL;

    DoModal();
}

void CAudioCfgDlg::DoDataExchange(CDataExchange *pDX)
{
    CBmpDialog::DoDataExchange(pDX);
}


void CAudioCfgDlg::PutDlgData()
{
    m_pScrollBar1->SetScrollPos(m_nVolume);

    m_pMusicButton->SetCheck(m_bMusic);
    m_pSoundButton->SetCheck(m_bSound);
}


void CAudioCfgDlg::GetDlgData()
{
    m_nVolume   = m_pScrollBar1->GetScrollPos();

    m_bMusic = FALSE;
    if (m_pMusicButton->GetCheck() == 1) {
        m_bMusic = TRUE;
    }

    m_bSound = FALSE;
    if (m_pSoundButton->GetCheck() == 1) {
        m_bSound = TRUE;
    }
}


BOOL CAudioCfgDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
    /*
    * respond to audio
    */
    if (HIWORD(lParam) == BN_CLICKED) {

        switch (wParam) {

            case IDOK:
				SaveIniSettings();
                PostMessage(WM_CLOSE, 0, 0);
                return(FALSE);

            case IDCANCEL:
                PostMessage(WM_CLOSE, 0, 0);
                return(FALSE);

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

VOID CAudioCfgDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScroll)
{
    INT 	nMin, nMax, nVal;
    
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
    }
    else 
        assert(0);
}


BOOL CAudioCfgDlg::OnInitDialog(void)
{
    CRect tmpRect;
    CDC *pDC;

    CBmpDialog::OnInitDialog();

    if ((pDC = GetDC()) != NULL) {

        tmpRect.SetRect(54, 85, 168, 98);
        if ((m_pTxtVolume = new CText) != NULL) {
            m_pTxtVolume->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
        }

        tmpRect.SetRect(54, 100, 168, 118);
        if ((m_pScrollBar1 = new CScrollBar) != NULL) {
            m_pScrollBar1->Create(WS_VISIBLE | WS_CHILD | SBS_HORZ | SBS_BOTTOMALIGN, tmpRect, this, ID_SCROLL1);
            m_pScrollBar1->SetScrollRange(LEVEL_MIN, LEVEL_MAX, TRUE);
        }

        ReleaseDC(pDC);
    }

    if ((pOKButton = new CColorButton) != NULL) {           // build a color QUIT button to let us exit
        pOKButton->SetPalette(m_pPalette);                  // set the palette to use
        pOKButton->SetControl(IDOK, this);                  // tie to the dialog control
	}
	
    if ((pCancelButton = new CColorButton) != NULL) {       // build a color QUIT button to let us exit
        pCancelButton->SetPalette(m_pPalette);              // set the palette to use
        pCancelButton->SetControl(IDCANCEL, this);          // tie to the dialog control
    }

    if ((m_pMusicButton = new CCheckButton) != NULL) {
        m_pMusicButton->SetPalette(m_pPalette);
        m_pMusicButton->SetControl(ID_MUSIC, this);
		if (!CSound::MidiAvailable())
			m_pMusicButton->EnableWindow(FALSE);
    }

    if ((m_pSoundButton = new CCheckButton) != NULL) {
        m_pSoundButton->SetPalette(m_pPalette);
        m_pSoundButton->SetControl(ID_SOUND, this);
		if (!CSound::SoundAvailable())
			m_pSoundButton->EnableWindow(FALSE);
    }

    LoadIniSettings();

    PutDlgData();
    
	if (!CSound::SoundVolumeAvailable() && !CSound::MidiVolumeAvailable())
		m_pScrollBar1->EnableWindow(FALSE);

    return(TRUE);
}


void CAudioCfgDlg::OnPaint(void)
{
    CBmpDialog::OnPaint();

    UpdateOptions();
}


VOID CAudioCfgDlg::UpdateOptions(VOID)
{
    CHAR buf[40];
    CDC *pDC;

    if ((pDC = GetDC()) != NULL) {
        if (m_pTxtVolume != NULL) {
        	if (CSound::SoundVolumeAvailable() || CSound::MidiVolumeAvailable())
            	Common::sprintf_s(buf, "Volume: %d", m_nVolume);
        	else
            	Common::strcpy_s(buf,"Volume control not available");
            m_pTxtVolume->DisplayString(pDC, buf, 13, TEXT_NORMAL, RGB( 0, 0, 0));
        }
        ReleaseDC(pDC);
    }
}

     
BOOL CAudioCfgDlg::OnEraseBkgnd(CDC *)
{
    return(TRUE);
}     


void CAudioCfgDlg::OnClose()
{
	if (pOKButton != NULL) {                          // release the button
		delete pOKButton;
		pOKButton = NULL;
	}

	if (pCancelButton != NULL) {                     	// release the button
		delete pCancelButton;
		pCancelButton = NULL;
	}

    if (m_pTxtVolume != NULL) {
        delete m_pTxtVolume;
        m_pTxtVolume = NULL;
    }

    if (m_pScrollBar1 != NULL) {
        delete m_pScrollBar1;
        m_pScrollBar1 = NULL;
    }

    if (m_pMusicButton != NULL) {
        delete m_pMusicButton;
        m_pMusicButton = NULL;
    }

    if (m_pSoundButton != NULL) {
        delete m_pSoundButton;
        m_pSoundButton = NULL;
    }

	ClearDialogImage();
    EndDialog(0);
}

void CAudioCfgDlg::ClearDialogImage(void)
{
	if (pOKButton != NULL) {                          // release the button
		delete pOKButton;
		pOKButton = NULL;
	}

	if (pCancelButton != NULL) {                     	// release the button
		delete pCancelButton;
		pCancelButton = NULL;
	}

	ValidateRect(NULL);
}


VOID CAudioCfgDlg::LoadIniSettings(VOID)
{
int	nMidiVolume, nWaveVolume;

    m_bMusic = GetPrivateProfileInt("Meta", "Music", TRUE, "HODJPODJ.INI");
    m_bSound = GetPrivateProfileInt("Meta", "SoundEffects", TRUE, "HODJPODJ.INI");

    nMidiVolume = GetPrivateProfileInt("Meta", "MidiVolume", LEVEL_DEF, "HODJPODJ.INI");
    nWaveVolume = GetPrivateProfileInt("Meta", "WaveVolume", LEVEL_DEF, "HODJPODJ.INI");

	if (nWaveVolume > nMidiVolume)
		m_nVolume = nWaveVolume;
	else
		m_nVolume = nMidiVolume;

    if ((m_nVolume < LEVEL_MIN) || (m_nVolume > LEVEL_MAX))
        m_nVolume = LEVEL_DEF;

	CSound::setVolume(m_nVolume,m_nVolume);
}

VOID CAudioCfgDlg::SaveIniSettings() {
    WritePrivateProfileString("Meta", "Music", Common::String::format("%d", m_bMusic ? 1 : 0).c_str(), "HODJPODJ.INI");
    WritePrivateProfileString("Meta", "SoundEffects", Common::String::format("%d", m_bSound ? 1 : 0).c_str(), "HODJPODJ.INI");
    WritePrivateProfileString("Meta", "WaveVolume", Common::String::format("%d", m_nVolume).c_str(), "HODJPODJ.INI");
    WritePrivateProfileString("Meta", "MidiVolume", Common::String::format("%d", m_nVolume).c_str(), "HODJPODJ.INI");

	CSound::setVolume(m_nVolume,m_nVolume);
}

BEGIN_MESSAGE_MAP(CAudioCfgDlg, CBmpDialog)
    ON_WM_CLOSE()    
    ON_WM_ERASEBKGND()
    ON_WM_HSCROLL()
    ON_WM_PAINT()
END_MESSAGE_MAP()

} // namespace HodjNPodj
} // namespace Bagel
