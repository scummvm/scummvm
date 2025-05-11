// optndlg.h : header file
//
#define LEFT_SIDE		 30 
#define	OPTIONS_COLOR	RGB(0, 0, 0)	// Color of the stats info CText

#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"                    
#include "mod.h"
#include "bagel/hodjnpodj/hnplibs/text.h"

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
	CScrollBar m_ScrollTime;
	CScrollBar m_ScrollDifficulty;
	int _difficulty;
	int _time;
	int _seconds;
	int _minutes;
	CString mDifficultyTable [MAX_DIFFICULTY];
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(COptnDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg	BOOL OnEraseBkgnd(CDC *pDC);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
