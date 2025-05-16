#include "bagel/afxwin.h"

#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"

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
