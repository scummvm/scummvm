// optnwin.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "optn.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


static	CColorButton *pOKButton = NULL;						// OKAY button on scroll
static	CColorButton *pCancelButton = NULL;					// Cancel button on scroll
/////////////////////////////////////////////////////////////////////////////
// COptn dialog


COptn::COptn(
			CWnd* pParent /*=NULL*/,
			CPalette *pPalette /*=NULL*/,
			int nLttrsSlvd /*LSLVD_DEFAULT*/,
			int nTime /*TIME_DEFAULT*/
			)
		:CBmpDialog(pParent, pPalette, IDD_USER_OPTIONS, ".\\ART\\SSCROLL.BMP")

{
	//{{AFX_DATA_INIT(COptn)
	m_nTimeIndex[0] = 15;
	m_nTimeIndex[1] = 20;
	m_nTimeIndex[2] = 25;
	m_nTimeIndex[3] = 30;
	m_nTimeIndex[4] = 35;
	m_nTimeIndex[5] = 40;
	m_nTimeIndex[6] = 45;
	m_nTimeIndex[7] = 50;
	m_nTimeIndex[8] = 60;
	m_nTimeIndex[9] = 70;
	m_nTimeIndex[10] = 80;
	m_nTimeIndex[11] = 90;
	m_nTimeIndex[12] = 120;
	m_nTimeIndex[13] = 150;
	m_nTimeIndex[14] = 180;
	m_nTimeIndex[15] = 240;
	m_nTimeIndex[16] = 300;
	m_nTimeIndex[17] = 589;
	m_nTimeIndex[18] = 601;

	pGamePalette = pPalette;

	m_nTime = TimeToIndex(nTime);
	m_nLttrsSlvd = nLttrsSlvd;

	m_pLttrsSlvd = NULL;
	m_pTime = NULL;
	//}}AFX_DATA_INIT
}

COptn::~COptn()
{
	if ( m_pLttrsSlvd != NULL ) {
		delete m_pLttrsSlvd;
		m_pLttrsSlvd = NULL;
	}

	if ( m_pTime != NULL ) {
		delete m_pTime;
		m_pLttrsSlvd = NULL;
	}

	CBmpDialog::OnDestroy();
}

int COptn::TimeToIndex(int nTime)
{
	int nLow;

	#ifdef REVEAL
	int nMid;
	int nTmp;
	/****************************************
	* Seaches ordered array in log(n) time. *
	****************************************/
	for ( nMid = (int) (TIME_TABLE / 2) + ((TIME_TABLE / 2) % 2), nLow = 0; TRUE ; ) {
		if ( m_nTimeIndex[nMid] == nTime )
			return ((int) nMid);
		if ( m_nTimeIndex[nMid] > nTime ) {
			nMid -= (int) ((nMid - nLow) / 2) + ((nMid - nLow) % 2);
		} else {
			nTmp = nLow;
			nLow = nMid;
			nMid += (int) ((nMid - nTmp) / 2) + ((nMid - nLow) % 2);
		}
	}
	#else
	/***********************************
	* Seaches ordered array in n time. *
	***********************************/
	for ( nLow = 0; nLow < TIME_TABLE ; nLow++) {
		if ( m_nTimeIndex[nLow] == nTime )
			return ((int) nLow);
	}

	return FALSE;
	#endif
}

void COptn::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptn)
	DDX_Control(pDX, IDC_LTTRSSLVD, m_LttrsSlvd);
	DDX_Control(pDX, IDC_TIME, m_Time);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptn, CDialog)
	//{{AFX_MSG_MAP(COptn)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_HSCROLL()
    ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptn message handlers

BOOL COptn::OnInitDialog()
{
	CRect	statsRect;						// game stats displays
	int 	nStat_col_offset;	  			// game stats placement
	int		nStat_row_offset;
	int 	nStatWidth, nStatHeight;
	BOOL	bAssertCheck;
	CDC		*pDC;

	CBmpDialog::OnInitDialog();

	// TODO: Add extra initialization here
	m_chTime[0] = "15 Seconds";
	m_chTime[1] = "20 Seconds";
	m_chTime[2] = "25 Seconds";
	m_chTime[3] = "30 Seconds";
	m_chTime[4] = "35 Seconds";
	m_chTime[5] = "40 Seconds";
	m_chTime[6] = "45 Seconds";
	m_chTime[7] = "50 Seconds";
	m_chTime[8] = "60 Seconds";
	m_chTime[9] = "70 Seconds";
	m_chTime[10] = "80 Seconds";
	m_chTime[11] = "90 Seconds";
	m_chTime[12] = "120 Seconds";
	m_chTime[13] = "150 Seconds";
	m_chTime[14] = "3 Minutes";
	m_chTime[15] = "4 Minutes";
	m_chTime[16] = "5 Minutes";
	m_chTime[17] = "10 Minutes";
	m_chTime[18] = "None";

	pDC = GetDC();

	nStat_col_offset	= STAT_BOX_COL;		// setup the Letters Solved
	nStat_row_offset	= STAT_BOX_ROW;		//  stat display box
	nStatWidth			= STAT_BOX_WIDTH;
	nStatHeight			= STAT_BOX_HEIGHT;
	statsRect.SetRect	(
						 nStat_col_offset,
						 nStat_row_offset,
						 nStat_col_offset + nStatWidth,
						 nStat_row_offset + nStatHeight
						);

	if ((m_pLttrsSlvd = new CText()) != NULL) {
		bAssertCheck = (*m_pLttrsSlvd).SetupText(pDC, pGamePalette, &statsRect, JUSTIFY_LEFT);
		ASSERT(bAssertCheck);   			// initialize the text objext
	}
	m_LttrsSlvd.SetScrollRange(MIN_LTTRSSLVD, MAX_LTTRSSLVD, FALSE);	// setup Letters Solved defaults
	m_LttrsSlvd.SetScrollPos(m_nLttrsSlvd, TRUE);

	nStat_row_offset += BOX_ROW_OFFSET;		// Time stat display box
	statsRect.SetRect(
						 nStat_col_offset,
						 nStat_row_offset,
						 nStat_col_offset + nStatWidth,
						 nStat_row_offset + nStatHeight
					 );

	if ((m_pTime = new CText()) != NULL) {
		bAssertCheck = (*m_pTime).SetupText(pDC, pGamePalette, &statsRect, JUSTIFY_LEFT);
		ASSERT(bAssertCheck);   			// initialize the text objext
	}

 	m_Time.SetScrollRange(MIN_INDEX_TIME, MAX_INDEX_TIME, FALSE);				// Time scroll bar defaults
	m_Time.SetScrollPos(m_nTime, TRUE);

	if ((pOKButton = new CColorButton) != NULL) {					// build a color QUIT button to let us exit
		(*pOKButton).SetPalette(pGamePalette);						// set the palette to use
		(*pOKButton).SetControl(IDOK,this);				// tie to the dialog control
	}
	
	if ((pCancelButton = new CColorButton) != NULL) {					// build a color QUIT button to let us exit
		(*pCancelButton).SetPalette(pGamePalette);						// set the palette to use
		(*pCancelButton).SetControl(IDCANCEL,this);				// tie to the dialog control
	}
	
	ReleaseDC(pDC);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void COptn::OnDestroy(void)
{
    CBmpDialog::OnDestroy();
}


void COptn::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
	CDC		*pDC;
	BOOL	bAssertCheck;
	char 	msg[64];

	pDC = GetDC();

	if (pScrollBar->GetDlgCtrlID() == IDC_LTTRSSLVD) {
		switch (nSBCode) {

			case SB_LINERIGHT:
				m_nLttrsSlvd++;
				break;

			case SB_PAGERIGHT:
				m_nLttrsSlvd += (int) MAX_LTTRSSLVD/5; // magic # 5 = five divisions
				break;

			case SB_RIGHT:
				m_nLttrsSlvd = MAX_LTTRSSLVD;
				break;

			case SB_LINELEFT:
				m_nLttrsSlvd--;
				break;

			case SB_PAGELEFT:
				m_nLttrsSlvd -= (int) MAX_LTTRSSLVD/5; // magic # 5 = five divisions
				break;

			case SB_LEFT:
				m_nLttrsSlvd = MIN_LTTRSSLVD;
				break;

			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				m_nLttrsSlvd = (int) nPos;
				break;

		} // end switch

		if ( m_nLttrsSlvd < MIN_LTTRSSLVD )
			m_nLttrsSlvd = MIN_LTTRSSLVD;
		if ( m_nLttrsSlvd > MAX_LTTRSSLVD )
			m_nLttrsSlvd = MAX_LTTRSSLVD;

		Common::sprintf_s(msg, "Letters Solved: %d", m_nLttrsSlvd);
		bAssertCheck = (*m_pLttrsSlvd).DisplayString(pDC,msg, FONT_SIZE, FW_BOLD, OPTIONS_COLOR);
		ASSERT(bAssertCheck);
		pScrollBar->SetScrollPos(m_nLttrsSlvd, TRUE);

	} else if (pScrollBar->GetDlgCtrlID()==IDC_TIME) {
		switch (nSBCode) {

			case SB_LINERIGHT:
				m_nTime++;
				break;
			case SB_PAGERIGHT:
				m_nTime += MAX_INDEX_TIME/5;  // want 5 pagerights end to end
				break;
			case SB_RIGHT:
				m_nTime = MAX_INDEX_TIME;
				break;
			case SB_LINELEFT:
				m_nTime--;
				break;
			case SB_PAGELEFT:
				m_nTime -= MAX_INDEX_TIME/5;  // want 5 pagerights end to end
				break;
			case SB_LEFT:
				m_nTime = 0;
				break;
			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				m_nTime = (int) nPos;
				break;
		} // end switch

		if ( m_nTime < MIN_INDEX_TIME )
			m_nTime = MIN_INDEX_TIME;
		if ( m_nTime > MAX_INDEX_TIME )
			m_nTime = MAX_INDEX_TIME;

		Common::sprintf_s(msg, "Time Limit: %s", m_chTime[m_nTime]);

		bAssertCheck = (*m_pTime).DisplayString(pDC,msg, FONT_SIZE, FW_BOLD, OPTIONS_COLOR);
		ASSERT(bAssertCheck);   // paint the text

		pScrollBar->SetScrollPos(m_nTime, TRUE);
	} // end if

	ReleaseDC(pDC);
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void COptn::OnPaint()
{
	CDC		*pDC;
	BOOL	bAssertCheck;
	char	msg[64];

	CBmpDialog::OnPaint();
	// Do not call CDialog::OnPaint() for painting messages

	pDC = GetDC();

	Common::sprintf_s(msg, "Letters Solved: %d", m_nLttrsSlvd);	// Display Letters Solved stats
	bAssertCheck = (*m_pLttrsSlvd).DisplayString(pDC,msg, FONT_SIZE, FW_BOLD, OPTIONS_COLOR);
	ASSERT(bAssertCheck);

	Common::sprintf_s(msg, "Time: %s", m_chTime[m_nTime]);		// Display Time stats
	bAssertCheck = (*m_pTime).DisplayString(pDC,msg, FONT_SIZE, FW_BOLD, OPTIONS_COLOR);
	ASSERT(bAssertCheck);

	ReleaseDC(pDC);
}

BOOL COptn::OnEraseBkgnd(CDC *pDC)
{
	return(TRUE);
}

void COptn::OnOK()
{
	m_nTime = m_nTimeIndex[m_nTime];
	ClearDialogImage();
	EndDialog( IDOK );
//		CDialog::OnOK();
}

void COptn::OnCancel()
{
	ClearDialogImage();
	EndDialog( 0 );
}

void COptn::ClearDialogImage(void)
{
	if (pOKButton != NULL) {                          // release the button
		delete pOKButton;
		pOKButton = NULL;
	}

	if (pCancelButton != NULL) {                     	// release the button
		delete pCancelButton;
		pCancelButton = NULL;
	}

	ValidateRect(NULL);
}

