// cmessbox.h : header file
//

#define	TEXT_COLOR				RGB(0, 0, 0)	// displayed text color
#define	ITEMDLG_FONT_SIZE		-14
#define	ITEMDLG_TEXT_COLOR		RGB(128,0,128)
#define	ITEMDLG_BLURB_COLOR		RGB(0,0,255)

#define	ITEMDLG_COL			16				// first message box positioning
#define	ITEMDLG_ROW     	24

#define	ITEMDLG_WIDTH		186				// TEXT dims
#define	ITEMDLG_HEIGHT		20

#define	ITEMDLG_DX			113				// Width of Dialog box
#define	ITEMDLG_DY			110
#define	ITEMDLG_BITMAP_DX	60
#define	ITEMDLG_BITMAP_DY	60													

#define IDD_TURNDLG			203

#include <afxwin.h>
#include <afxext.h>
#include <cbofdlg.h>
#include <text.h>
#include <button.h>

class CTurnDialog : public CBmpDialog
{
    private:
        CColorButton *m_pOKButton;      // OKAY button on scroll
        CText        *m_pTextMessage;
        BOOL          m_bHodj;
        BOOL          m_bGain;
        BOOL          m_bTurn;          // True: win/lose turn, False: win/lose Game

    public:

        // standard constructor
        CTurnDialog(CWnd *pParent, CPalette *pPalette, BOOL bHodj, BOOL bGain, BOOL bTurn );
        void ClearDialogImage(void);

        // Dialog Data
        //{{AFX_DATA(CTurnDialog)
        enum { IDD = IDD_TURNDLG };
            // NOTE: the ClassWizard will add data members here
        //}}AFX_DATA

    // Implementation
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        // Generated message map functions
        //{{AFX_MSG(CTurnDialog)
        afx_msg void OnPaint();
        virtual void OnOK();
        virtual void OnCancel();
        virtual BOOL OnInitDialog();
        afx_msg void OnDestroy();
        afx_msg BOOL OnEraseBkgnd(CDC *pDC);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};
