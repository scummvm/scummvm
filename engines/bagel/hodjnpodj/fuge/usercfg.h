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

#ifndef HODJNPODJ_FUGE_USERCFG_H
#define HODJNPODJ_FUGE_USERCFG_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Fuge {

#define IDD_USERCFG 100

#define BALLS_MIN     1
#define BALLS_DEF     5
#define BALLS_MAX     5

#define LEVEL_MIN     1
#define LEVEL_DEF     6
#define LEVEL_MAX     6

#define SPEED_MIN     1
#define SPEED_DEF     6
#define SPEED_MAX     10

#define PSIZE_MIN     0
#define PSIZE_DEF     2
#define PSIZE_MAX     2

#define GFORCE_MIN     0
#define GFORCE_DEF     0
#define GFORCE_MAX     20

class CUserCfgDlg : public CBmpDialog
{
    public:
        CUserCfgDlg(CWnd *pParent, CPalette *pPalette, UINT);

    protected:

        virtual VOID DoDataExchange(CDataExchange *);
        virtual BOOL OnCommand(WPARAM, LPARAM);
        virtual BOOL OnInitDialog(VOID);
        VOID PutDlgData(VOID);
        VOID GetDlgData(VOID);
        VOID LoadIniSettings(VOID);
        VOID SaveIniSettings(VOID);
        VOID UpdateOptions(VOID);
        VOID ClearDialogImage(VOID);                               
                               
        afx_msg BOOL OnEraseBkgnd(CDC *);
        VOID OnHScroll(UINT, UINT, CScrollBar *);
        VOID OnClose();
        VOID OnDestroy();
        VOID OnPaint();
        DECLARE_MESSAGE_MAP()

        CText        *m_pTxtNumBalls;
        CText        *m_pTxtStartLevel;
        CText        *m_pTxtBallSpeed;
        CText        *m_pTxtPaddleSize;
        CScrollBar   *m_pScrollBar1;
        CScrollBar   *m_pScrollBar2;
        CScrollBar   *m_pScrollBar3;
        CScrollBar   *m_pScrollBar4;
        CCheckButton *m_pWallButton;

        INT           m_nNumBalls;
        INT           m_nStartLevel;
        INT           m_nBallSpeed;
        INT           m_nPaddleSize;
        BOOL          m_bOutterWall;

        BOOL        m_bSave;                    // True if should save theses values
};

} // namespace Fuge
} // namespace HodjNPodj
} // namespace Bagel

#endif
