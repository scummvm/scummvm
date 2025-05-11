// optndlg.h : header file
//
#define LEFT_SIDE		26 
#define	OPTIONS_COLOR	RGB(0, 0, 0)	// Color of the stats info CText
#define NUM_SPEEDS		12

#include "resource.h"
#include "cbofdlg.h"                    
#include "text.h"

/////////////////////////////////////////////////////////////////////////////
// COptnDlg dialog

class COptnDlg : public CBmpDialog
{
// Construction
public:
	COptnDlg(CWnd* pParent = NULL,CPalette *pPalette = NULL);	// standard constructor
	~COptnDlg();		// destructor
	void UpdateScrollbars();
	void ClearDialogImage(void);

// Dialog Data
	//{{AFX_DATA(COptnDlg)
	enum { IDD = IDD_SUBOPTIONS };
	CScrollBar 	m_ScrollButtons;
	CScrollBar 	m_ScrollSpeed;
	BOOL 		m_bPlayGame;
	int m_nNumButtons;
	int m_nSpeed;
	CString mSpeedTable [NUM_SPEEDS];
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(COptnDlg)
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg	BOOL OnEraseBkgnd(CDC *pDC);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
