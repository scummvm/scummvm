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
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/button.h"

#ifndef HODJNPODJ_PDQ_USERCFG_H
#define HODJNPODJ_PDQ_USERCFG_H

namespace Bagel {
namespace HodjNPodj {
namespace PDQ {

#define IDD_USERCFG 100

#define SPEED_MIN     1
#define SPEED_DEF     7
#define SPEED_MAX    10
#define SHOWN_MIN     1
#define SHOWN_DEF     3
#define SHOWN_MAX     9

class CUserCfgDlg : public CBmpDialog
{
    public:
        CUserCfgDlg(CWnd* pParent = NULL, CPalette *pPalette = NULL, UINT = IDD_USERCFG);

    protected:

        virtual void DoDataExchange(CDataExchange*);
        virtual BOOL OnCommand(WPARAM, LPARAM);
        virtual BOOL OnInitDialog(void);
        void PutDlgData(void);
        void GetDlgData(void);
        void DispSpeed(void);
        void DispShown(void);
		void ClearDialogImage(void);


        void OnHScroll(UINT, UINT, CScrollBar *);
        void OnPaint();
        void OnDestroy();
        DECLARE_MESSAGE_MAP()

        CScrollBar   *m_pSpeedScroll;
        CScrollBar   *m_pShownScroll;
        CCheckButton *m_pNamesButton;

        /*
        * user preference data
        */
        UINT        m_nShown;               // initial number of letters to be revealed
        UINT        m_nGameSpeed;           // game speed 1..5 (1 = fast, 5 = slow)
        BOOL        m_bRandomLetters;       // TRUE if letters to be revealed in random order
        BOOL        m_bShowNames;           // TRUE if we shouldm show category names
        BOOL        m_bShouldSave;          // TRUE if we should save theses values
};

} // namespace PDQ
} // namespace HodjNPodj
} // namespace Bagel

#endif
