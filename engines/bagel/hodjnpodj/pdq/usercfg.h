/**
*   USERCFG.H   -
*
*
*   Description -
*
*
*
*
*   (c) Copyright 1994 - Boffo Games
*   All rights reserved.
*
*
*   Revision History:
*
*   Version     Date        Author      Comments
*   -------     --------    ------      --------------------------------
*   1.00a       03-18-94     BCW        Created this file
*
**/
#ifndef USERCFG_H
#define USERCFG_H

#include "bagel/afxwin.h"

#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/button.h"

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

#endif // USERCFG_H
