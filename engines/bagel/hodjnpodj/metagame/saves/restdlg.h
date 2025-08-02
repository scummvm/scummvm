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

#ifndef BAGEL_METAGAME_SAVES_RESTDLG_H
#define BAGEL_METAGAME_SAVES_RESTDLG_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Saves {

class CRestoreDlg: public CBmpDialog {
public:
	CRestoreDlg(CHAR *[], CWnd *, CPalette *);

protected:
	void ClearDialogImage(void);

	//{{AFX_MSG(COptions)
	virtual BOOL OnCommand(WPARAM, LPARAM) override;
	virtual void OnCancel(void) override;
	afx_msg void OnDestroy(void);
	afx_msg void OnPaint(void);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	BOOL OnInitDialog(void) override;

	CColorButton *m_pSlotButtons[MAX_SAVEGAMES];
	CColorButton *m_pQuitButton;
	CText *m_pTxtRestore;
	CHAR *m_pszDescriptions[MAX_SAVEGAMES];

};

} // namespace Saves
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
