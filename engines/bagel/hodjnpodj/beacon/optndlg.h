// optndlg.h : header file
//
#include "resource.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"                    
#include "bagel/hodjnpodj/hnplibs/text.h"

#define LEFT_SIDE		 30 
#define	OPTIONS_COLOR	RGB(0, 0, 0)	// Color of the stats info CText
#define NUM_SPEEDS		3	
                    
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
	CScrollBar m_ScrollSweeps;
	CScrollBar m_ScrollSpeed;
	BOOL	m_bAutomatic;
	BOOL	m_bChangeAtTwelve;
	UINT	m_nSweeps;
	UINT	m_nSpeed;
	UINT	nSweepSets;
	CString mSpeedTable [NUM_SPEEDS];
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(COptnDlg)
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg	BOOL OnEraseBkgnd(CDC *pDC);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPaint();
	afx_msg void OnAutomatic();
	afx_msg void OnChangeAtTwelve();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
