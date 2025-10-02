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

#ifndef BAGEL_METAGAME_SAVES_SAVEDLG_H
#define BAGEL_METAGAME_SAVES_SAVEDLG_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Saves {

class CSaveDlg: public CBmpDialog {
public:
	CSaveDlg(char *[], CWnd *, CPalette *);

protected:
	void ClearDialogImage();
	void EditDescription(int);

	//{{AFX_MSG(COptions)
	virtual bool OnCommand(WPARAM, LPARAM) override;
	virtual void OnCancel() override;
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool OnInitDialog() override;

	CColorButton *m_pSlotButtons[MAX_SAVEGAMES] = {};
	CEdit *m_pSlotText[MAX_SAVEGAMES] = {};
	CColorButton *m_pQuitButton = nullptr;
	CColorButton *m_pOkButton = nullptr;
	CText *m_pTxtSave = nullptr;
	char *m_pszDescriptions[MAX_SAVEGAMES] = {};
	int m_nCurSlot = 0;
};

} // namespace Saves
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
