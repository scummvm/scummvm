#ifndef GUESS_H
#define GUESS_H

#include <afxwin.h>
#include <afxext.h>
#include <cbofdlg.h>

#define IDD_GUESS_DLG 102

/////////////////////////////////////////////////////////////////////////////
// CGuessDlg dialog

class CGuessDlg : public CBmpDialog
{
    public:

        CGuessDlg(CWnd *, CPalette *);    // standard constructor

        CString text;

    protected:
        virtual void DoDataExchange(CDataExchange*);    // DDX/DDV support
        BOOL OnInitDialog(void);
        void OnTimer(UINT);
        void OnPaint(VOID);
        void OnSetFocus(CWnd *);
        void OnActivate(UINT, CWnd *, BOOL);
        BOOL OnEraseBkgnd(CDC *);

        DECLARE_MESSAGE_MAP()
};

#endif // GUESS_H
