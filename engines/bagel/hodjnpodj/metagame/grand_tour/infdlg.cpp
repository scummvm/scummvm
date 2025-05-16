/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace GrandTour {

#ifdef NODEEDIT

#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "gtl.h"

#include "gtldoc.h"
#include "gtlview.h"
#include "gtlfrm.h"
#include "infdlg.h"

///DEFS infdlg.h

#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInfDlg dialog

CInfDlg::CInfDlg(CWnd *pParent /*=NULL*/)
	: CDialog(CInfDlg::IDD, pParent) {
	TRACECONSTRUCTOR(CInfDlg) ;
	// note: parent is same as frame, but GetParentFrame returns NULL.
	m_xpFrameWnd = (CGtlFrame *)pParent ;

	//{{AFX_DATA_INIT(CInfDlg)
	m_iXCoord = 0;
	m_iYCoord = 0;
	m_iLogScrollPosX = 0;
	m_iLogScrollPosY = 0;
	m_iTotalSizeX = 0;
	m_iTotalSizeY = 0;
	m_iScrollRangeX = 0;
	m_iViewSizeX = 0;
	m_iScrollRangeY = 0;
	m_iViewSizeY = 0;
	m_iScrollCount = 0;
	//}}AFX_DATA_INIT
}

void CInfDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInfDlg)
	DDX_Text(pDX, IDC_EDIT_XCOORD, m_iXCoord);
	DDX_Text(pDX, IDC_EDIT_YCOORD, m_iYCoord);
	DDX_Text(pDX, IDC_EDIT_XLSCRPOS, m_iLogScrollPosX);
	DDX_Text(pDX, IDC_EDIT_YLSCRPOS, m_iLogScrollPosY);
	DDX_Text(pDX, IDC_EDIT_XTOTAL, m_iTotalSizeX);
	DDX_Text(pDX, IDC_EDIT_YTOTAL, m_iTotalSizeY);
	DDX_Text(pDX, IDC_EDIT_XSRANGE, m_iScrollRangeX);
	DDX_Text(pDX, IDC_EDIT_XVIEW, m_iViewSizeX);
	DDX_Text(pDX, IDC_EDIT_YSRANGE, m_iScrollRangeY);
	DDX_Text(pDX, IDC_EDIT_YVIEW, m_iViewSizeY);
	DDX_Text(pDX, IDC_EDIT_SCRCOUNT, m_iScrollCount);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CInfDlg, CDialog)
	//{{AFX_MSG_MAP(CInfDlg)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInfDlg message handlers

void CInfDlg::OnSetFocus(CWnd *) {
}


BOOL CInfDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	m_bClose = FALSE ;
	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CInfDlg::OnClose() {
	DestroyWindow() ;
}

void CInfDlg::OnCancel() {
	DestroyWindow() ;
}

void CInfDlg::OnDestroy() {
	CDialog::OnDestroy();
}

void CInfDlg::PostNcDestroy() {
	if (!m_bClose) {
		m_bClose = TRUE ;   // set flag to close dialog box
		m_xpGtlData->UpdateDialogs(TRUE) ;  // handle close
	}
	if (m_xpDocument)
		m_xpDocument->m_xpcInfDlg = NULL ;
	delete this;
}

#ifdef OLD_CODE

void CInfDlg::OnClose() {
	// TODO: Add your message handler code here and/or call default
	CDialog::OnClose();

//    DestroyWindow();
}

void CInfDlg::PostNcDestroy() {
	if (m_xpFrameWnd && m_xpFrameWnd->GetMenu())
		m_xpFrameWnd->GetMenu()->CheckMenuItem(ID_VIEW_INFO_DLG,
		                                       MF_UNCHECKED) ;
	// uncheck menu item in frame window
	m_xpDocument->m_xpcInfDlg = NULL ;
	delete this;
}
#endif


/////////////////////////////////////////////////////////////////////////////
// CCtlDlg message handlers
/////////////////////////////////////////////////////////////////////////////
// CCtlDlg dialog

CCtlDlg::CCtlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCtlDlg::IDD, pParent) {
	TRACECONSTRUCTOR(CCtlDlg) ;
	// note: parent is same as frame, but GetParentFrame returns NULL.
	m_xpFrameWnd = (CGtlFrame *)pParent ;
//    m_xpFrameWnd = (CGtlFrame *)GetParentFrame() ;


	//{{AFX_DATA_INIT(CCtlDlg)
	m_bPaintBackground = FALSE;
	m_bShowNodes = FALSE;
	m_bMetaGame = FALSE;
	m_bShowSensitivity = FALSE;
	//}}AFX_DATA_INIT
}

void CCtlDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCtlDlg)
	DDX_Check(pDX, IDC_CTL_PAINTBKG, m_bPaintBackground);
	DDX_Check(pDX, IDC_CTL_SHOWNODES, m_bShowNodes);
	DDX_Check(pDX, IDC_METAGAME, m_bMetaGame);
	DDX_Check(pDX, IDC_CTL_SHOW_SENSITIVITY, m_bShowSensitivity);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCtlDlg, CDialog)
	//{{AFX_MSG_MAP(CCtlDlg)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_CTL_PAINTBKG, OnClickedCtlPaintBkg)
	ON_BN_CLICKED(IDC_CTL_SHOWNODES, OnClickedCtlShowNodes)
	ON_BN_CLICKED(IDC_CTL_METAGAME, OnClickedCtlMetaGame)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CTL_SHOW_SENSITIVITY, OnClickedCtlShowSensitivity)
	ON_BN_CLICKED(IDC_CTL_DUMP, OnCtlDump)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCtlDlg message handlers

BOOL CCtlDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	if (m_xpGtlData && m_xpDocument && ((m_xpView = m_xpDocument->m_xpcLastFocusView) != NULL)) {
		m_cDlgBsuSet.InitDlgBsuSet(this, &m_xpView->m_cViewBsuSet) ;
		m_cDlgBsuSet.AddBarToSet(IDC_CTL_HSCROLL, SB_HORZ) ;
		m_cDlgBsuSet.AddBarToSet(IDC_CTL_VSCROLL, SB_VERT) ;
		m_cDlgBsuSet.LinkWndBsuSet() ;
	}

	m_bClose = FALSE ;
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCtlDlg::OnClickedCtlPaintBkg() {
	// TODO: Add your control notification handler code here
//  ::dbgWindowDump(NULL) ;
	EffectControl() ;

}

void CCtlDlg::OnClickedCtlShowNodes() {
	// TODO: Add your control notification handler code here

	EffectControl() ;
}

void CCtlDlg::OnClickedCtlShowSensitivity() {
	// TODO: Add your control notification handler code here
	EffectControl() ;

}

void CCtlDlg::OnClickedCtlMetaGame() {
	// TODO: Add your control notification handler code here

	EffectControl() ;
}

void CCtlDlg::OnCtlDump() {
	// TODO: Add your control notification handler code here
	::dbgWindowDump(NULL) ;
}

void CCtlDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* xpScrollBar) {
	// TODO: Add your message handler code here and/or call default

	m_cDlgBsuSet.OnScroll(nSBCode, nPos, xpScrollBar, BSCT_HORZ) ;
}

void CCtlDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* xpScrollBar) {
	// TODO: Add your message handler code here and/or call default

	m_cDlgBsuSet.OnScroll(nSBCode, nPos, xpScrollBar, BSCT_VERT) ;
}

void CCtlDlg::OnClose() {
	// TODO: Add your message handler code here and/or call default
//	CDialog::OnClose();
	DestroyWindow() ;
}

void CCtlDlg::OnCancel() {
	DestroyWindow() ;
}

void CCtlDlg::OnDestroy() {
	EffectControl() ;
	CDialog::OnDestroy();

	// TODO: Add your message handler code here

}

void CCtlDlg::PostNcDestroy() {
	if (!m_bClose) {
		m_bClose = TRUE ;   // set flag to close dialog box
		EffectControl(FALSE) ;
	}

	if (m_xpDocument)
		m_xpDocument->m_xpcCtlDlg = NULL ;
	delete this;
}

//* CCtlDlg::EffectControl -- effect control command on document
BOOL CCtlDlg::EffectControl(BOOL bUpdate)
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CCtlDlg::EffectControl) ;
	int iError = 0 ;        // error code

	if (bUpdate)
		UpdateData(TRUE) ;  // update variables from dialog box

	m_xpGtlData->UpdateDialogs(TRUE) ;  // copy data from
	// dialog box back to document

// cleanup:

	JXELEAVE(CCtlDlg::EffectControl) ;
	RETURN(iError != 0) ;
}

//* CCtlDlg::InitDialogBox -- set scroll bars
BOOL CCtlDlg::InitDialogBox(void)
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CCtlDlg::InitDialogBox) ;
	int iError = 0 ;        // error code


// cleanup:

	JXELEAVE(CCtlDlg::InitDialogBox) ;
	RETURN(iError != 0) ;
}



/////////////////////////////////////////////////////////////////////////////
// CNodeDlg dialog

CNodeDlg::CNodeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNodeDlg::IDD, pParent) {
	TRACECONSTRUCTOR(CNodeDlg) ;
	// note: parent is same as frame, but GetParentFrame returns NULL.
	m_xpFrameWnd = (CGtlFrame *)pParent ;

	//{{AFX_DATA_INIT(CNodeDlg)
	m_stSector = "";
	m_stLoc = "";
	m_stName = "";
	m_iX = 0;
	m_iY = 0;
	m_iNodeIndex = 0;
	m_bRelocatable = FALSE;
	m_iWeight = 0;
	m_iSensitivity = 0;
//	m_iSector = 0;
	m_iSectorButton = -1;
	//}}AFX_DATA_INIT
}

void CNodeDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNodeDlg)
	DDX_Text(pDX, IDC_NODE_SECTOR, m_stSector);
	DDV_MaxChars(pDX, m_stSector, 11);
	DDX_Text(pDX, IDC_NODE_LOC, m_stLoc);
	DDV_MaxChars(pDX, m_stLoc, 8);
	DDX_Text(pDX, IDC_NODE_NAME, m_stName);
	DDV_MaxChars(pDX, m_stName, 11);
	DDX_Text(pDX, IDC_NODE_X, m_iX);
	DDX_Text(pDX, IDC_NODE_Y, m_iY);
	DDX_Text(pDX, IDC_NODE_INDEX, m_iNodeIndex);
	DDX_Check(pDX, IDC_NODE_RELOC, m_bRelocatable);
	DDX_Text(pDX, IDC_NODE_WGT, m_iWeight);
	DDX_Text(pDX, IDC_NODE_SENSITIVITY, m_iSensitivity);
//	DDX_Text(pDX, IDC_NODE_SECTOR, m_iSector);
	DDX_Radio(pDX, IDC_SECT_BUT0, m_iSectorButton);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNodeDlg, CDialog)
	//{{AFX_MSG_MAP(CNodeDlg)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
//	ON_BN_CLICKED(IDC_NODE_RELOC, OnNodeReloc)
	ON_BN_CLICKED(IDC_NODE_RESET, OnNodeReset)
	ON_BN_CLICKED(IDC_NODE_APPLY, OnNodeApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNodeDlg message handlers

BOOL CNodeDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	m_bApply = FALSE ;
	m_bClose = FALSE ;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

#if OLDCODE

void CNodeDlg::OnKillfocusNodeName() {
	// TODO: Add your control notification handler code here
	EffectControl() ;

}

void CNodeDlg::OnKillfocusNodeSector() {
	// TODO: Add your control notification handler code here
	EffectControl() ;
}

void CNodeDlg::OnKillfocusNodeIndex() {
	// TODO: Add your control notification handler code here
	EffectControl() ;

}

void CNodeDlg::OnKillfocusNodeLoc() {
	// TODO: Add your control notification handler code here
	EffectControl() ;

}

void CNodeDlg::OnKillfocusNodeX() {
	// TODO: Add your control notification handler code here
	EffectControl() ;

}

void CNodeDlg::OnKillfocusNodeY() {
	// TODO: Add your control notification handler code here
	EffectControl() ;

}

void CNodeDlg::OnNodePassThru() {
	// TODO: Add your control notification handler code here
	EffectControl() ;

}

void CNodeDlg::OnNodeReloc() {
	// TODO: Add your control notification handler code here

}

#endif /* OLDCODE */

void CNodeDlg::OnNodeReset() {
	// TODO: Add your control notification handler code here
	m_bApply = FALSE ;  // don't apply changes
	m_xpGtlData->UpdateDialogs(FALSE) ; // copy data from document
	// back to dialog box
	UpdateData(FALSE) ; // update dialog box from variables

}

void CNodeDlg::OnNodeApply() {
	// TODO: Add your control notification handler code here

	UpdateData(TRUE) ;  // update variables from dialog box
	m_bApply = TRUE ;   // apply button pressed
	m_xpGtlData->UpdateDialogs(TRUE) ;  // copy data from
	// dialog box back to document
}

void CNodeDlg::OnClose() {
	// TODO: Add your message handler code here and/or call default

	CDialog::OnClose();
}

void CNodeDlg::OnCancel() {
	DestroyWindow() ;
}

void CNodeDlg::OnDestroy() {
//  EffectControl() ;
	CDialog::OnDestroy();

	// TODO: Add your message handler code here

}


void CNodeDlg::PostNcDestroy() {

	if (!m_bClose) {
		m_bApply = FALSE ;  // don't apply changes
		m_bClose = TRUE ;   // set flag to close dialog box
		if (m_xpGtlData)
			m_xpGtlData->UpdateDialogs(TRUE) ;  // process close
	}

	if (m_xpDocument)
		m_xpDocument->m_xpcNodeDlg = NULL ;
	delete this;
}

#if OLD_CODE

// CNodeDlg::EffectControl -- effect control command on document
BOOL CNodeDlg::EffectControl(BOOL bUpdate)
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CNodeDlg::EffectControl) ;
	int iError = 0 ;        // error code

	if (bUpdate)
		UpdateData(TRUE) ;  // update variables from dialog box

	m_xpGtlData->UpdateDialogs(TRUE) ;  // copy data from
	// dialog box back to document

// cleanup:

	JXELEAVE(CNodeDlg::EffectControl) ;
	RETURN(iError != 0) ;
}
#endif /* OLD_CODE */

/////////////////////////////////////////////////////////////////////////////
// CActDlg dialog


CActDlg::CActDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CActDlg::IDD, pParent) {
	TRACECONSTRUCTOR(CActDlg) ;
	//{{AFX_DATA_INIT(CActDlg)
	m_stGame = "";
	m_iMoney = 0;
	//}}AFX_DATA_INIT
}

void CActDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CActDlg)
	DDX_Text(pDX, IDC_ACTION_GAME, m_stGame);
	DDX_Text(pDX, IDC_ACTION_MONEY, m_iMoney);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CActDlg, CDialog)
	//{{AFX_MSG_MAP(CActDlg)
	// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CActDlg message handlers


/////////////////////////////////////////////////////////////////////////////
// CMenuDlg dialog


CMenuDlg::CMenuDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMenuDlg::IDD, pParent) {
	TRACECONSTRUCTOR(CMenuDlg) ;
	// note: parent is same as frame, but GetParentFrame returns NULL.
	m_xpFrameWnd = (CGtlFrame *)pParent ;

	//{{AFX_DATA_INIT(CMenuDlg)
	m_bCtlDlg = FALSE;
	m_bInfoDlg = FALSE;
	m_bNodeDlg = FALSE;
	//}}AFX_DATA_INIT
}

void CMenuDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMenuDlg)
	DDX_Check(pDX, IDC_MENU_CTL, m_bCtlDlg);
	DDX_Check(pDX, IDC_MENU_INFO, m_bInfoDlg);
	DDX_Check(pDX, IDC_MENU_NODE, m_bNodeDlg);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMenuDlg, CDialog)
	//{{AFX_MSG_MAP(CMenuDlg)
	ON_BN_CLICKED(IDC_MENU_CTL, OnMenuCtl)
	ON_BN_CLICKED(IDC_MENU_INFO, OnMenuInfo)
	ON_BN_CLICKED(IDC_MENU_NEW, OnMenuNew)
	ON_BN_CLICKED(IDC_MENU_NODE, OnMenuNode)
	ON_BN_CLICKED(IDC_MENU_OPEN, OnMenuOpen)
	ON_BN_CLICKED(IDC_MENU_SAVE, OnMenuSave)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMenuDlg message handlers

BOOL CMenuDlg::OnInitDialog() {

	m_bNew = m_bOpen = m_bSave = FALSE ;
	m_bClose = FALSE ;
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMenuDlg::OnMenuCtl() {
	// TODO: Add your control notification handler code here
	EffectControl() ;

}

void CMenuDlg::OnMenuInfo() {
	// TODO: Add your control notification handler code here
	EffectControl() ;
}

void CMenuDlg::OnMenuNode() {
	// TODO: Add your control notification handler code here
	EffectControl() ;
}

void CMenuDlg::OnMenuNew() {
	m_xpFrameWnd->PostMessage(WM_COMMAND, ID_CALL_NEW) ;
}

void CMenuDlg::OnMenuOpen() {
	m_xpFrameWnd->PostMessage(WM_COMMAND, ID_CALL_OPEN) ;
}

void CMenuDlg::OnMenuSave() {
	if (m_xpDocument)
		m_xpDocument->DoOnFileSaveAs();
}

void CMenuDlg::OnCancel() {
	// TODO: Add extra cleanup here
	DestroyWindow() ;
//	CDialog::OnCancel();
}

void CMenuDlg::OnClose() {
	// TODO: Add your message handler code here and/or call default
	DestroyWindow() ;

//	CDialog::OnClose();
}

void CMenuDlg::OnDestroy() {
	CDialog::OnDestroy();

	// TODO: Add your message handler code here

}

void CMenuDlg::PostNcDestroy() {

	if (!m_bClose) {
		m_bClose = TRUE ;   // set flag to close dialog box
		EffectControl(FALSE) ;
	}

	if (m_xpDocument)
		m_xpDocument->m_xpcMenuDlg = NULL ;
	delete this;
}

////* CMenuDlg::EffectControl -- effect control command on document
BOOL CMenuDlg::EffectControl(BOOL bUpdate)
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CMenuDlg::EffectControl) ;
	int iError = 0 ;        // error code

	if (bUpdate)
		UpdateData(TRUE) ;  // update variables from dialog box

	m_xpGtlData->UpdateDialogs(TRUE) ;  // copy data from
	// dialog box back to document

// cleanup:

	JXELEAVE(CMenuDlg::EffectControl) ;
	RETURN(iError != 0) ;
}
#endif

} // namespace GrandTour
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
