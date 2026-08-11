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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/audiocfg.h"
#include "bagel/hodjnpodj/peggle/resource.h"
#include "bagel/hodjnpodj/peggle/options.h"
#include "bagel/hodjnpodj/peggle/optndlg.h"
#include "bagel/hodjnpodj/peggle/game.h"

namespace Bagel {
namespace HodjNPodj {
namespace Peggle {

extern  LPGAMESTRUCT    pGameInfo;

extern CPalette *pOptionsPalette;


void COptions::OnLButtonDown(unsigned int nFlags, CPoint point) {

	CDialog::OnLButtonDown(nFlags, point);
}


void COptions::OnMouseMove(unsigned int nFlags, CPoint point) {

	CDialog::OnMouseMove(nFlags, point);
}

void COptions::OnClickedRules() {
	CWnd    *pControl = nullptr;

	CRules  RulesDlg(this, RULESSPEC, pOptionsPalette, ((*pGameInfo).bSoundEffectsEnabled ? NARRATIVESPEC : nullptr));
	RulesDlg.DoModal();                      // invoke the help dialog box
	SetDefID(IDC_OPTIONS_OPTIONS);
	SetDefID(IDC_OPTIONS_RETURN);
	pControl = GetDlgItem(IDC_OPTIONS_RETURN);
	GotoDlgCtrl(pControl);
}


void COptions::OnClickedNewgame() {
	ClearDialogImage();
	EndDialog(IDC_NEWGAME);

}


void COptions::OnClickedOptions() {
	CWnd    *pControl = nullptr;

	COptnDlg OptionsDlg(this, pOptionsPalette);      // Call Specific Game

	if (OptionsDlg.DoModal() == IDOK)  {     // save values set in dialog box
		ClearDialogImage();
		EndDialog(IDC_RESTART) ;
	} else {
		SetDefID(IDC_OPTIONS_RULES);
		SetDefID(IDC_OPTIONS_RETURN);
		pControl = GetDlgItem(IDC_OPTIONS_RETURN);
		GotoDlgCtrl(pControl);
	}
}


void COptions::OnClickedAudio() {
	CWnd    *pControl = nullptr;

	CAudioCfgDlg dlgAudioCfg(this, pOptionsPalette, IDD_AUDIOCFG);

	if (pGameInfo != nullptr) {
		pGameInfo->bMusicEnabled = GetPrivateProfileInt("Meta", "Music", true, "HODJPODJ.INI");
		pGameInfo->bSoundEffectsEnabled = GetPrivateProfileInt("Meta", "SoundEffects", true, "HODJPODJ.INI");
	}

	SetDefID(IDC_OPTIONS_RULES);
	SetDefID(IDC_OPTIONS_RETURN);
	pControl = GetDlgItem(IDC_OPTIONS_RETURN);
	GotoDlgCtrl(pControl);
}


void COptions::OnClickedReturn() {
	ClearDialogImage();
	EndDialog(IDC_RETURN) ;          // successful completion
}

void COptions::OnClickedQuit() {
	ClearDialogImage();
	EndDialog(IDC_QUIT) ;            // unsuccessful completion
}

} // namespace Peggle
} // namespace HodjNPodj
} // namespace Bagel
