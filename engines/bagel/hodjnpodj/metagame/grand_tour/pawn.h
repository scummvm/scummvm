/*****************************************************************
 *
 *  pawn.h
 *
 *  Copyright (c) 1994 by Ledge Multimedia, All Rights Reserved
 *
 *  HISTORY
 *
 *      1.0      06/15/94     EDS     refer to pawn.cpp for details
 *
 *  MODULE DESCRIPTION:
 *
 *      Class definitions for CPawnShop.
 *   
 *  RELEVANT DOCUMENTATION:
 *
 *      n/a
 *   
 ****************************************************************/


#ifndef _INC_PAWN
#define _INC_PAWN

#include <time.h>
#include "bagel/hodjnpodj/metagame/bgen/invent.h"
#include "bagel/hodjnpodj/metagame/bgen/item.h"

#define PAWN_SPEC	".\\art\\mlscroll.bmp"      // path for pawn's DIB on disk
#define PAWN_DX			502
#define PAWN_DY			395
#define	PAWN_CURL_DX		500
#define	PAWN_CURL_DY		50
#define PAWN_BORDER_DX		60
#define PAWN_BORDER_DY		60

#define PAWN_TITLEZONE_DX	100
#define PAWN_TITLEZONE_DY	40
#define PAWN_TITLEZONE_DDY	-10

#define PAWN_TEXTZONE_DX	75
#define PAWN_TEXTZONE_DY	40
#define PAWN_TEXTZONE_DDY	20
#define PAWN_TEXTZONE_DDDY	5

#define PAWN_COSTZONE_DX	75
#define PAWN_COSTZONE_DY	20
#define PAWN_COSTZONE_DDY	20
#define PAWN_COSTZONE_DDDY	5

#define PAWN_BITMAP_DX		59
#define PAWN_BITMAP_DY		59
#define PAWN_BITMAP_DDX		5
#define PAWN_BITMAP_DDY		5

#define IDD_PAWN_DIALOG		940
#define IDC_PAWN_OKAY	    940

#define IDC_PAWN_DOLLAR   	941

#define PAWN_TEXT_COLOR		RGB(128,0,128)
#define PAWN_MORE_COLOR		RGB(0,0,0)
#define PAWN_BLURB_COLOR	RGB(0,0,255)

class CPawnShop : public CDialog
{
// Construction
public:
	CPawnShop(CWnd* pParent, CPalette *pPalette, CInventory *pGeneralStore, CInventory *pInventory);

	int SelectedItem(CPoint point);

static	void UpdatePawn(CDC *pDC);
static	void UpdateContent(CDC *pDC);
static	void UpdateItem(CDC *pDC, CItem *pItem, int nX, int nY);
static	void UpdatePage(CDC *pDC);
static	void UpdateCrowns(CDC *pDC);
static	void ClearDialogImage(void);
static	void RefreshBackground(void);
static	void DoWaitCursor(void);
static	void DoArrowCursor(void);

private:
	BOOL SetupKeyboardHook(void);
	void RemoveKeyboardHook(void);

	BOOL CreateWorkAreas(CDC *pDC);

private:
	BOOL	m_bKeyboardHook;				// whether keyboard hook present

// Dialog Data
	//{{AFX_DATA(CPawnShop)
	enum { IDD = IDD_PAWN_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CPawnShop)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog(void);
	afx_msg	BOOL OnEraseBkgnd(CDC *pDC);
	virtual void OnOK(void);
	virtual void OnCancel(void);
	afx_msg void OnDestroy(void);
	afx_msg void OnPaint(void);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif //!_INC_PAWN
