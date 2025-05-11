// optn.h : header file
//

//#include <options.h>
#include "resource.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"

#define MIN_LTTRSSLVD 		0				// Preferences
#define	MAX_LTTRSSLVD		15

#define TIME_TABLE			19
#define MIN_INDEX_TIME 		0
#define MAX_INDEX_TIME 		18

#define	OPTIONS_COLOR		RGB(0, 0, 0)	// Color of the stats info CText
#define FONT_SIZE			18

#define STAT_BOX_COL		20
#define STAT_BOX_ROW		20
#define	STAT_BOX_WIDTH	 	200
#define STAT_BOX_HEIGHT	 	20

#define BOX_ROW_OFFSET		48

/////////////////////////////////////////////////////////////////////////////
// COptn dialog

class COptn : public CBmpDialog
{
private:
	CString		m_chTime[TIME_TABLE];
	int			m_nTimeIndex[TIME_TABLE];
	CText		*m_pLttrsSlvd, *m_pTime;
	CPalette	*pGamePalette;

	int 		TimeToIndex(int nTime);

public:
	COptn(CWnd* pParent = NULL, CPalette *pGamePalette = NULL, int nLttrsSolvd = FALSE, int nTime = FALSE);	// standard constructor
	~COptn();
	void ClearDialogImage(void);

	int 		m_nLttrsSlvd, m_nTime;
// Dialog Data
	//{{AFX_DATA(COptn)
	enum { IDD = IDD_USER_OPTIONS };
	CScrollBar	m_LttrsSlvd;
	CScrollBar	m_Time;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(COptn)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg	BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
