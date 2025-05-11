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

#define ITEMDLG2_ROW_OFFSET ( ITEMDLG_DY - ITEMDLG_HEIGHT )	// Row offset from first 
													// message box

#define IDD_ITEMMOVE		202
#define	BUTTON_DY			10

#include "bagel/afxwin.h"

#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "button.h"
#include "item.h"

class CItemDialog : public CBmpDialog
{
    private:
        CText          *m_pTextDescription;
        CText          *m_pTextMessage;
        CColorButton   *m_pOKButton;          // OKAY button on scroll
        CItem          *m_pItem;
        BOOL            m_bHodj;
        BOOL            m_bGain;
        long            m_lAmount;

    public:

        // standard constructor
        CItemDialog(CWnd *pParent, CPalette *pPalette, CItem *pItem, BOOL bHodj, BOOL bGain, long lAmount);

        //  static  void UpdateItem(CDC *pDC, CItem *pItem, int nX, int nY);

        // Dialog Data
        //{{AFX_DATA(CItemDialog)
        enum { IDD = IDD_ITEMMOVE };
            // NOTE: the ClassWizard will add data members here
        //}}AFX_DATA

    private:
        void ClearDialogImage(void);

    // Implementation
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        // Generated message map functions
        //{{AFX_MSG(CItemDialog)
        afx_msg void OnPaint();
        virtual void OnOK();
        virtual void OnCancel();
        virtual BOOL OnInitDialog();
        afx_msg void OnDestroy();
        afx_msg BOOL OnEraseBkgnd(CDC *pDC);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};
