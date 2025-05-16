/*****************************************************************
 *
 *  store.h
 *
 *  Copyright (c) 1994 by Ledge Multimedia, All Rights Reserved
 *
 *  HISTORY
 *
 *      1.0      06/15/94     EDS     refer to store.cpp for details
 *
 *  MODULE DESCRIPTION:
 *
 *      Class definitions for CGeneralStore.
 *
 *  RELEVANT DOCUMENTATION:
 *
 *      n/a
 *
 ****************************************************************/


#ifndef _INC_STORE
#define _INC_STORE

#include <time.h>
#include "bagel/hodjnpodj/metagame/bgen/invent.h"
#include "bagel/hodjnpodj/metagame/bgen/item.h"

#define STORE_SPEC  ".\\art\\mlscroll.bmp"      // path for store's DIB on disk
#define STORE_DX            502
#define STORE_DY            395
#define STORE_CURL_DX       500
#define STORE_CURL_DY       50
#define STORE_BORDER_DX     60
#define STORE_BORDER_DY     60

#define STORE_TITLEZONE_DX  100
#define STORE_TITLEZONE_DY  40
#define STORE_TITLEZONE_DDY -10

#define STORE_TEXTZONE_DX   75
#define STORE_TEXTZONE_DY   40
#define STORE_TEXTZONE_DDY  20
#define STORE_TEXTZONE_DDDY 5

#define STORE_COSTZONE_DX   75
#define STORE_COSTZONE_DY   20
#define STORE_COSTZONE_DDY  20
#define STORE_COSTZONE_DDDY 5

#define STORE_BITMAP_DX     59
#define STORE_BITMAP_DY     59
#define STORE_BITMAP_DDX    5
#define STORE_BITMAP_DDY    5

#define IDD_STORE_DIALOG    930
#define IDC_STORE_OKAY      930

#define IDC_STORE_DOLLAR    931

#define STORE_TEXT_COLOR    RGB(128,0,128)
#define STORE_MORE_COLOR    RGB(0,0,0)
#define STORE_BLURB_COLOR   RGB(0,0,255)


class CGeneralStore : public CDialog {
// Construction
public:
	CGeneralStore(CWnd* pParent, CPalette *pPalette, CInventory *pStore, CInventory *pInventory);

	int SelectedItem(CPoint point);

	static  void UpdateStore(CDC *pDC);
	static  void UpdateContent(CDC *pDC);
	static  void UpdateItem(CDC *pDC, CItem *pItem, int nX, int nY);
	static  void UpdatePage(CDC *pDC);
	static  void UpdateCrowns(CDC *pDC);
	static  void ClearDialogImage(void);
	static  void RefreshBackground(void);
	static  void DoWaitCursor(void);
	static  void DoArrowCursor(void);

private:
	BOOL SetupKeyboardHook(void);
	void RemoveKeyboardHook(void);

	BOOL CreateWorkAreas(CDC *pDC);

private:
	BOOL    m_bKeyboardHook;                // whether keyboard hook present

// Dialog Data
	//{{AFX_DATA(CGeneralStore)
	enum { IDD = IDD_STORE_DIALOG };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CGeneralStore)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog(void);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
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

#endif //!_INC_STORE
