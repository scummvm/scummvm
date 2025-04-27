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

#ifndef HODJNPODJ_METAGAME_GTL_SAVEDLG_H
#define HODJNPODJ_METAGAME_GTL_SAVEDLG_H

#include <afxwin.h>
#include <afxext.h>
#include <text.h>
#include <cbofdlg.h>
#include <button.h>
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

class CSaveDlg: public CBmpDialog {
    public:
        CSaveDlg(CHAR *[], CWnd *, CPalette *);

    protected:
        VOID ClearDialogImage(VOID);
        VOID EditDescription(INT);

        //{{AFX_MSG(COptions)
        virtual BOOL OnCommand(WPARAM, LPARAM);
        virtual void OnCancel(void);
        afx_msg void OnDestroy(void);
        afx_msg void OnPaint(void);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()

    private:
        BOOL OnInitDialog(void);

        CColorButton *m_pSlotButtons[MAX_SAVEGAMES];
        CEdit        *m_pSlotText[MAX_SAVEGAMES];
        CColorButton *m_pQuitButton;
        CColorButton *m_pOkButton;
        CText        *m_pTxtSave;
        CHAR         *m_pszDescriptions[MAX_SAVEGAMES];
        INT           m_nCurSlot;
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
