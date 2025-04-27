// cmapdlg.h : header file
//

#define	TEXT_COLOR				RGB(0, 0, 0)	// displayed text color
#define	ITEMDLG_TEXT_COLOR		RGB(128,0,128)
#define	ITEMDLG_BLURB_COLOR		RGB(0,0,255)

#define	MAP_COL			20				// first message box positioning
#define	MAP_ROW     	10

#define ZOOMMAP_WIDTH		640				// pixel dims
#define	ZOOMMAP_HEIGHT		480
#define BIGMAP_WIDTH		1925			// pixel dims
#define	BIGMAP_HEIGHT		1415

#define	MAP_DX				113				// Width of Dialog box
#define	MAP_DY				110
#define	PLAYER_BITMAP_DX	18
#define	PLAYER_BITMAP_DY	28
#define	PLAYER_OFFSET_X		 9
#define	PLAYER_OFFSET_Y		14

#define	NUM_NODES			20
#define IDD_ZOOMMAP			204
#define	BUTTON_DY			10

#include <afxwin.h>
#include <afxext.h>
#include <cbofdlg.h>
#include <text.h>
#include <button.h>

class CMapDialog : public CBmpDialog
{
    private:
        POINT         m_HodjLoc, m_PodjLoc;
        POINT         m_ptZoomHodj, m_ptZoomPodj;
        CText        *m_pTextMessage;
        CColorButton *m_pOKButton;          // OKAY button on scroll

    public:

        // standard constructor
        CMapDialog( CWnd* pParent, CPalette *pPalette, POINT HodjLoc, POINT PodjLoc );

        void ClearDialogImage(void);

        // Dialog Data
        //{{AFX_DATA(CMapDialog)
        enum { IDD = IDD_ZOOMMAP };
            // NOTE: the ClassWizard will add data members here
        //}}AFX_DATA

    // Implementation
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        // Generated message map functions
        //{{AFX_MSG(CMapDialog)
        afx_msg void OnPaint();
        virtual void OnOK();
        virtual void OnCancel();
        virtual BOOL OnInitDialog();
        afx_msg void OnDestroy();
        afx_msg BOOL OnEraseBkgnd(CDC *pDC);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};
