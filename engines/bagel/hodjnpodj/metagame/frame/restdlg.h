#include <afxwin.h>
#include <afxext.h>
#include <text.h>
#include <cbofdlg.h>
#include <button.h>
#include "bfc.h"

class CRestoreDlg: public CBmpDialog {
    public:
        CRestoreDlg(CHAR *[], CWnd *, CPalette *);

    protected:
        void ClearDialogImage(void);

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
        CColorButton *m_pQuitButton;
        CText *m_pTxtRestore;
        CHAR *m_pszDescriptions[MAX_SAVEGAMES];

};
