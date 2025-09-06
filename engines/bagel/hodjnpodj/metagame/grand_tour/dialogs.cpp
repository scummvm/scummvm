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
#include "bagel/hodjnpodj/metagame/grand_tour/dialogs.h"
#include "bagel/hodjnpodj/metagame/grand_tour/grand_tour.h"
#include "bagel/hodjnpodj/metagame/grand_tour/resource.h"
#include "bagel/hodjnpodj/hnplibs/button.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace GrandTour {

SCORESTRUCT	astScores[10];

CPalette *pTop10Palette;

// Message Map
BEGIN_MESSAGE_MAP(CTop10Dlg, CBmpDialog)
	//{{AFX_MSG_MAP( CMainPokerWindow )
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CTop10Dlg::CTop10Dlg(CWnd *pParent, CPalette *pPalette)
	: CBmpDialog(pParent, pPalette, IDD_TOP10_SCROLL, ".\\ART\\MLSCROLL.BMP", -1, -1, false) {
	// Inits
	m_pOKButton = NULL;
	pTop10Palette = pPalette;
}

bool CTop10Dlg::OnCommand(WPARAM wParam, LPARAM lParam) {
	// What ever button is clicked, end the dialog and send the ID of the button
	// clicked as the return from the dialog
	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {
		case IDC_TOP10_OK:
			ClearDialogImage();
			EndDialog(0);
			return(true);
		}
	}

	return CBmpDialog::OnCommand(wParam, lParam);
}

void CTop10Dlg::OnCancel() {
	ClearDialogImage();
	EndDialog(0);
}

void CTop10Dlg::OnOK() {
	ClearDialogImage();
	EndDialog(0);
}

void CTop10Dlg::ClearDialogImage() {
}

bool CTop10Dlg::OnInitDialog() {
	bool bSuccess;
	CDC *pDC = GetDC();

	CBmpDialog::OnInitDialog();

	GetScores();

	m_pOKButton = new CColorButton();
	ASSERT(m_pOKButton != NULL);
	m_pOKButton->SetPalette(pTop10Palette);
	bSuccess = m_pOKButton->SetControl(IDC_TOP10_OK, this);
	ASSERT(bSuccess);

	ReleaseDC(pDC);
	return(true);
}

void CTop10Dlg::OnPaint() {
	CDC *pDC = NULL;
	char cTemp[64];
	int nLeft = 0;
	int nTop = 0;
	int i;
	CPalette *pOldPalette = NULL;
	int nOldBkMode;
	COLORREF rgbOldColorRef;

	// Call CBmpDialog onpaint, to paint the background
	CBmpDialog::OnPaint();

	pDC = GetDC();
	ASSERT(pDC);
	pOldPalette = pDC->SelectPalette(pTop10Palette, false);
	pDC->RealizePalette();                      // realize game palette
	nOldBkMode = pDC->SetBkMode(TRANSPARENT);

	pDC->TextOut(nLeft + 50, nTop + 50, "GRAND TOUR - TOP TEN SCORES", 27);
	pDC->TextOut(nLeft + 50, nTop + 70, "RANK", 4);

	rgbOldColorRef = pDC->SetTextColor(RGB(128, 0, 128));

	for (i = 0; i < 10; i++) {
		pDC->SetTextColor(RGB(0, 0, 0));
		Common::sprintf_s(cTemp, "%i", (i + 1));
		pDC->TextOut(nLeft + 50, nTop + (i * 20) + 90, cTemp, strlen(cTemp));
		pDC->SetTextColor(RGB(128, 0, 128));
		pDC->TextOut(nLeft + 70, nTop + (i * 20) + 90, astScores[i].acName, strlen(astScores[i].acName));

		if (astScores[i].nSkillLevel == SKILLLEVEL_LOW) {
			pDC->TextOut(nLeft + 350, nTop + (i * 20) + 90, "Easy", 4);
		} else {
			if (astScores[i].nSkillLevel == SKILLLEVEL_MEDIUM) {
				pDC->TextOut(nLeft + 350, nTop + (i * 20) + 90, "Medium", 6);
			} else {
				pDC->TextOut(nLeft + 350, nTop + (i * 20) + 90, "Hard", 4);
			}
		}
		Common::sprintf_s(cTemp, "%i", astScores[i].nScore);
		pDC->TextOut(nLeft + 425, nTop + (i * 20) + 90, cTemp, strlen(cTemp));

	}

	pDC->SetTextColor(rgbOldColorRef);
	pDC->SelectPalette(pOldPalette, false);
	pDC->SetBkMode(nOldBkMode);
	ReleaseDC(pDC);
}

void CTop10Dlg::GetScores() {
	int i;
	char cTemp1[64];

	for (i = 0; i < 10; i++) {
		Common::sprintf_s(cTemp1, "acName%i", i);
		GetPrivateProfileString("GRAND TOUR TOP TEN", cTemp1, ":", astScores[i].acName, 20, INI_FILENAME);

		if (astScores[i].acName[0] == ':') {
			Common::strcpy_s(astScores[i].acName, " ");
		}

		Common::sprintf_s(cTemp1, "nSkillLevel%i", i);
		astScores[i].nSkillLevel = GetPrivateProfileInt("GRAND TOUR TOP TEN", cTemp1, 0, INI_FILENAME);
		Common::sprintf_s(cTemp1, "nScore%i", i);
		astScores[i].nScore = GetPrivateProfileInt("GRAND TOUR TOP TEN", cTemp1, 0, INI_FILENAME);
	}
}

void CTop10Dlg::OnDestroy() {
	if (m_pOKButton != NULL) {
		delete m_pOKButton;
		m_pOKButton = NULL;
	}

	CBmpDialog::OnDestroy();
}

} // namespace GrandTour
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
