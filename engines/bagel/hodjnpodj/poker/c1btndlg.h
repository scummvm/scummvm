/*****************************************************************
*
*  Copyright (c) 1994 by Boffo Games, All Rights Reserved
*
*
*  C1btndlg.h -
*
*  HISTORY
*
*       1.00        08/25/94    BCW     Created this file
*
*  MODULE DESCRIPTION:
*
*
*  RELEVANT DOCUMENTATION:
*
*
****************************************************************/
#ifndef C1BTNDLG_H
#define C1BTNDLG_H

#include "bagel/afxwin.h"

#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/button.h"

#define CBUTTON1 IDOK
#define CBUTTON2 IDCANCEL

class C1ButtonDialog : public CBmpDialog
{
    public:

        // standard constructor
        C1ButtonDialog(CWnd *, CPalette *, char *, char *, char *pszText2 = NULL, char *pszText3 = NULL);

    private:
        void ClearDialogImage(void);

        // Dialog Data
        //{{AFX_DATA(C1ButtonDialog)
            // NOTE: the ClassWizard will add data members here
        //}}AFX_DATA

    // Implementation
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        // Generated message map functions
        //{{AFX_MSG(C1ButtonDialog)
        afx_msg void OnPaint();
        virtual void OnOK();
        virtual void OnCancel();
        virtual BOOL OnInitDialog();
        afx_msg void OnDestroy();
        afx_msg BOOL OnEraseBkgnd(CDC *pDC);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()

    private:
        CText        *m_cTextMessage1;
        CText        *m_cTextMessage2;
        CText        *m_cTextMessage3;
        char         *m_pszMessage1;
        char         *m_pszMessage2;
        char         *m_pszMessage3;
        char         *m_pszButton1Text;
        CColorButton *m_pButton1;
};

#endif //C1BTNDLG_H
