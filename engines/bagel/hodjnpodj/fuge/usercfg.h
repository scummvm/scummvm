/*****************************************************************
*
*  Copyright (c) 1994 by Boffo Games, All Rights Reserved
*
*
*  usercfg.h -
*
*  HISTORY
*
*       1.00        05/09/94    BCW     Created this file
*
*  MODULE DESCRIPTION:
*
*       Header for user configuration dialog
*
*  RELEVANT DOCUMENTATION:
*
*
*
****************************************************************/
#ifndef USERCFG_H
#define USERCFG_H

#include <afxwin.h>
#include <afxext.h>
#include <cbofdlg.h>
#include <stdinc.h>

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

#define SIZE_MIN     0
#define SIZE_DEF     2
#define SIZE_MAX     2

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

#endif // USERCFG_H
