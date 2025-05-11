// optndlg.h : header file
//

#include "options.h"

#define	OPTN_BUTTON_WIDTH		50
#define	OPTN_BUTTON_HEIGHT		47
#define	CROSS_X					40
#define	CROSS_Y					27
#define	TRIANGLE_X				40
#define TRIANGLE_Y				76
#define	CROSSPLUS_X				125
#define	CROSSPLUS_Y				27
#define	TRIANGLEPLUS_X			125
#define TRIANGLEPLUS_Y			76

/////////////////////////////////////////////////////////////////////////////
// COptnDlg dialog

class COptnDlg : public COptions
{
// Construction
public:
	COptnDlg(CWnd* pParent = NULL,CPalette *pPalette = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(COptnDlg)
	enum { IDD = IDD_SUBOPTIONS };
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(COptnDlg)
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg	BOOL OnEraseBkgnd(CDC *pDC);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPaint();
	afx_msg void OnClickedOkay() ;
	afx_msg void OnClickedCancel() ;
	afx_msg void OnClickedCross() ;
	afx_msg void OnClickedCrossPlus() ;
	afx_msg void OnClickedTriangle() ;
	afx_msg void OnClickedTrianglePlus() ;
	afx_msg void OnRandom();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
