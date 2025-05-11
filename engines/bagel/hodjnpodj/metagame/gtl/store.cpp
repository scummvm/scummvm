/*****************************************************************
 * Copyright (c) 1994 by Ledge Multimedia, All Rights Reserved.
 *
 *
 * store.cpp
 *
 * HISTORY
 *
 *	  1.0	06/15/94	EDS		first release
 *
 * MODULE DESCRIPTION:
 *
 *     tbs
 *
 * CONSTRUCTORS:
 *
 *	CGeneralStore				create a store object and initialize elements
 *
 * DESTRUCTORS:
 *
 *	~CGeneralStore				destroy a store object and release its resources
 *
 * PUBLIC:
 *
 *      n/a
 *
 * PUBLIC GLOBAL:
 *
 *      n/a
 *
 * PROTECTED:
 *
 *      n/a
 *
 * PRIVATE:
 *
 *      n/a
 *
 * PRIVATE GLOBAL:
 *
 *      n/a
 *
 *
 * MEMBERS:
 *
 *      refer to store.h
 *   
 * RELEVANT DOCUMENTATION:
 *
 *      n/a
 *   
 ****************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "bfc.h"
#include "bitmaps.h"
#include "text.h"
#include "rules.h"
#include "button.h"
#include "sound.h"
#include "store.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define TEXT_MORE_DX		120						// offset of "more" indicator from right margin
#define TEXT_MORE_DY		5                       // offset of "more" indicator bottom of scroll
#define MORE_TEXT_BLURB		"[ More ]"				// actual text to display for "more" indicator
#define MORE_TEXT_LENGTH	8                       // # characters in "more" indicator string

#define BUTTON_DY			15						// offset for Okay button from store base


extern CBfcMgr      *lpMetaGameStruct;


extern "C" {
LRESULT FAR PASCAL StoreHookProc(int,WORD,LONG);	// keyboard hook procedure definition
}

extern 	HINSTANCE	hDLLInst;
extern  HINSTANCE 	hExeInst;

typedef LRESULT (FAR PASCAL *FPSTOREHOOKPROC) ( int, WORD, LONG );

static	FPSTOREHOOKPROC 	lpfnKbdHook = NULL;			// pointer to hook procedure

static	HHOOK		hKbdHook = NULL;                // handle for hook procedure

static	CWnd		*pParentWnd = NULL;				// parent window for this dialog
static	CGeneralStore	*pStoreDialog = NULL;		// pointer to our store dialog box

static	CColorButton *pOKButton = NULL;				// OKAY button on scroll
static	CRect		OkayRect;						// rectangle bounding the OKAY button

static	CInventory	*pInventory = NULL;				// inventory to be displayed
static	CInventory	*pGeneralStore = NULL;			// general store's inventory

static	CRect		StoreRect;						// x/y (left/right) and dx/dy (right/bottom) for the store window
static	CRect		ScrollTopRect,					// area spanned by upper scroll curl
					ScrollBotRect;                  // area spanned by lower scroll curl
					
static	CDC			*pStoreDC = NULL;				// device context for the store bitmap
static	CBitmap		*pStoreBitmap = NULL,			// bitmap for an entirely blank store
					*pStoreBitmapOld = NULL;     	// bitmap previously mapped to the store context

static	CBitmap		*pBackgroundBitmap = NULL;
static	CPalette	*pBackgroundPalette = NULL;

static	CDC			*pWorkDC = NULL;				// context and resources for the offscreen
static	CBitmap		*pWork = NULL,                  // ... work area (only if memory permits)
					*pWorkOld = NULL;
static	CPalette	*pWorkPalOld = NULL;

static	CText		*pItemText = NULL;				// item information field
static	CText		*pItemCost = NULL;				// item cost field
static	CText		*pTitleText = NULL;				// title information field

static	CFont		*pFont = NULL;					// font to use for displaying store text
static	char		chPathName[128];                // buffer to hold path name of the store file

static	BOOL		bActiveWindow = FALSE;			// whether our window is active
static	BOOL		bFirstTime = TRUE;				// flag for first time information is displayed
static	int			nStore_DX, nStore_DY;			// size of useable store background
static	int			nItem_DDX, nItem_DDY;           // space separation between inventory items
static	int			nItemsPerColumn, nItemsPerRow;	// span of items that fit on the background
static	int			nFirstSlot = 0;					// first item in current inventory page

static	BOOL		bPlayingHodj = TRUE;			// whether playing Hodj or Podj

/////////////////////////////////////////////////////////////////////////////
// CGeneralStore dialog



BOOL CGeneralStore::SetupKeyboardHook(void)
{
	pStoreDialog = this;							// retain pointer to our dialog box

	lpfnKbdHook = (FPSTOREHOOKPROC)::GetProcAddress( hDLLInst, "StoreHookProc");
	if (lpfnKbdHook == NULL)                           // setup pointer to our procedure
		return(FALSE);
	
	hKbdHook = SetWindowsHookEx(WH_KEYBOARD,(HOOKPROC) lpfnKbdHook, hExeInst, GetCurrentTask());
	if (hKbdHook == NULL)                           // plug in our keyboard hook
		return(FALSE);
	
	return(TRUE);
}


void CGeneralStore::RemoveKeyboardHook(void)
{
	if (m_bKeyboardHook)
		UnhookWindowsHookEx(hKbdHook);					// unhook our keyboard procedure

	pStoreDialog = NULL;
	lpfnKbdHook = NULL;
	hKbdHook = NULL;
	m_bKeyboardHook = FALSE;
}


extern "C" 
LRESULT FAR PASCAL StoreHookProc(int code, WORD wParam, LONG lParam)
{
CDC	*pDC = NULL;

	if (code < 0)										// required to punt to next hook
		return(CallNextHookEx((HHOOK) lpfnKbdHook,code,wParam,lParam));
	
	if (lParam & 0xA0000000)							// ignore ALT and key release
		return(FALSE);

	if (bActiveWindow)
		switch(wParam) {								// process only the keys we are looking for
			case VK_UP:                                 // ... letting the default dialog procedure
			case VK_NUMPAD8:                            // ... deal with all the rest
			case VK_PRIOR:                              // go to previous page of text
				if (nFirstSlot > 0) {                   // ... scroll up if not at first item
					nFirstSlot -= (nItemsPerRow * nItemsPerColumn);
					if (nFirstSlot < 0)
						nFirstSlot = 0;
					pDC = (*pStoreDialog).GetDC();
				}
				break;
			case VK_DOWN:								// go to next page of text
			case VK_NUMPAD2:
			case VK_NEXT:
				if (nFirstSlot + (nItemsPerRow * nItemsPerColumn) < (*pGeneralStore).ItemCount()) {
					nFirstSlot += (nItemsPerRow * nItemsPerColumn);
					pDC = (*pStoreDialog).GetDC();
				}
				break;
			case VK_HOME:								// go to first page of text
				if (nFirstSlot != 0) {
					nFirstSlot = 0;
					pDC = (*pStoreDialog).GetDC();
				}
				break;
			case VK_END:								// go to last page of text
				nFirstSlot = (*pGeneralStore).ItemCount() - (nItemsPerRow * nItemsPerColumn);
				if (nFirstSlot < 0)
					nFirstSlot = 0;
				pDC = (*pStoreDialog).GetDC();
		}

	if (pDC != NULL) {                              	// update the inventory page if required
		CGeneralStore::UpdatePage(pDC);
		(*pStoreDialog).ReleaseDC(pDC);
		return(TRUE);
	}
	
	return(FALSE);
}


CGeneralStore::CGeneralStore(CWnd* pParent,CPalette *pPalette, CInventory *pStore, CInventory *pInvent)
	: CDialog(CGeneralStore::IDD, pParent)
{
	pBackgroundPalette = pPalette;                      // retain palette to be used
	pGeneralStore = pStore;								// retain store inventory
	pInventory = pInvent;                               // retain inventory to be displayed
	pParentWnd = pParent;                               // retain pointer to parent window
	
	//{{AFX_DATA_INIT(CGeneralStore)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGeneralStore::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGeneralStore)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGeneralStore, CDialog)
	//{{AFX_MSG_MAP(CGeneralStore)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_NCMOUSEMOVE()
	ON_WM_LBUTTONDOWN()
    ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGeneralStore message handlers

void CGeneralStore::OnOK()
{
	ClearDialogImage();
	CDialog::EndDialog(IDC_STORE_OKAY);
}


void CGeneralStore::OnCancel(void)
{
	ClearDialogImage();
    CDialog::OnCancel();
}


void CGeneralStore::OnDestroy()
{
BOOL	bUpdateNeeded;

	if (pFont != NULL)
		delete pFont;								// release the font file

	if (pOKButton != NULL)                          // release the button
		delete pOKButton;

	if (m_bKeyboardHook)								// remove keyboard hook, if present
		RemoveKeyboardHook();

	if (pWorkOld != NULL) {
		(void) (*pWorkDC).SelectObject(pWorkOld);
		pWorkOld = NULL;
	}
	if (pWorkPalOld != NULL) {
		(void) (*pWorkDC).SelectPalette(pWorkPalOld,FALSE);
		pWorkPalOld = NULL;
	}
	if (pWork != NULL) {
		delete pWork;
		pWork = NULL;
	}
	if (pWorkDC != NULL) {
		delete pWorkDC;
		pWorkDC = NULL;
	}
	
	if (pStoreBitmap != NULL) {
		delete pStoreBitmap;
		pStoreBitmap = NULL;
	}
	
	if (pBackgroundBitmap != NULL) {
		delete pBackgroundBitmap;
		pBackgroundBitmap = NULL;
		bUpdateNeeded = (*pParentWnd).GetUpdateRect(NULL,FALSE);
		if (bUpdateNeeded)
	    	(*pParentWnd).ValidateRect(NULL);
	}
	
	if (pItemText != NULL) {
		delete pItemText;
		pItemText = NULL;
	}
	if (pItemCost != NULL) {
		delete pItemCost;
		pItemCost = NULL;
	}
	if (pTitleText != NULL) {
		delete pTitleText;
		pTitleText = NULL;
	}
	
    CDialog::OnDestroy();
}


BOOL CGeneralStore::OnInitDialog()
{
BOOL	bSuccess;
CWnd	*pButton;												// pointer to the OKAY button
CRect	myRect;                                    				// rectangle that holds the button location
int		x, y, dx, dy;                              				// used for calculating positioning info

	CDialog::OnInitDialog();									// do basic dialog initialization

	if (m_pParentWnd == NULL)									// get our parent window
		m_pParentWnd = ((CWnd *) this)->GetParent();			// ... as passed to us or inquired about
		
	(*m_pParentWnd).GetWindowRect(&myRect);
	x = myRect.left + (((myRect.right - myRect.left) - STORE_DX) >> 1);
	y = myRect.top + (((myRect.bottom - myRect.top) - STORE_DY) >> 1);
	StoreRect.SetRect(0,0,STORE_DX,STORE_DY);
	MoveWindow(x,y,STORE_DX,STORE_DY);							// center the dialog box on the parent
	
	pButton = GetDlgItem((int) GetDefID());						// get the window for the okay button
	ASSERT(pButton != NULL);                        			// ... and verify we have it
	(*pButton).GetWindowRect(&myRect);              			// get the button's position and size

	dx = myRect.right - myRect.left;	            			// calculate where to place the button
	x = (StoreRect.right - dx) >> 1;               				// ... centered at the bottom edge
	dy = myRect.bottom - myRect.top;
	y = StoreRect.bottom - dy - BUTTON_DY;
	
	(*pButton).MoveWindow(x,y,dx,dy);               			// reposition the button
	OkayRect.SetRect(x,y,x + dx,y + dy);

	pOKButton = new CColorButton;								// build a color QUIT button to let us exit
	ASSERT(pOKButton != NULL);
	(*pOKButton).SetPalette(pBackgroundPalette);				// set the palette to use
	bSuccess = (*pOKButton).SetControl((int) GetDefID(),this);	// tie to the dialog control
    ASSERT(bSuccess);

	ScrollTopRect.SetRect(0,0,STORE_DX,STORE_CURL_DY);			// setup rectangles for scrolling areas
	ScrollBotRect.SetRect(0,STORE_DY - STORE_CURL_DY,STORE_DX,STORE_DY);

	m_bKeyboardHook = SetupKeyboardHook();			// establish keyboard hook

	bFirstTime = TRUE;
    nFirstSlot = 0;
    
	if ((*pInventory).FindItem(MG_OBJ_HODJ_NOTEBOOK) != NULL)	// see who is playing
		bPlayingHodj = TRUE;
	else
		bPlayingHodj = FALSE;
				
	return(TRUE);  												// return TRUE  unless focused on a control
}


void CGeneralStore::OnActivate(UINT nState, CWnd *, BOOL /*bMinimized*/)
{
BOOL	bUpdateNeeded;

switch(nState) {
	case WA_INACTIVE:
		bActiveWindow = FALSE;
		break;
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		bActiveWindow = TRUE;
		bUpdateNeeded = GetUpdateRect(NULL,FALSE);
		if (bUpdateNeeded)
			InvalidateRect(NULL,FALSE);
	}
}


void CGeneralStore::OnPaint()
{
BOOL		bSuccess;
CPaintDC	dc(this); 											// device context for painting

	if (bFirstTime) {											// acquire resources and if first time
		bFirstTime = FALSE;                         		
		bSuccess = CreateWorkAreas(&dc);
		if (!bSuccess)
			CDialog::OnCancel();
	 	}
	
	UpdateStore(&dc);
}


void CGeneralStore::UpdateStore(CDC *pDC)
{
CPalette	*pPalOld;

	DoWaitCursor();												// put up the hourglass cursor
		
	pPalOld = (*pDC).SelectPalette(pBackgroundPalette,FALSE);		// setup the proper palette
	(*pDC).RealizePalette();

	if (pWorkDC == NULL) {                                      // if we don't have a work area
		RefreshBackground();                                    // ... then update the screen directly
		PaintMaskedBitmap(pDC,pBackgroundPalette,pStoreBitmap,0,0,STORE_DX,STORE_DY);
		UpdateContent(pDC);
		if (pTitleText != NULL)
			(*pTitleText).DisplayString(pDC, (*pGeneralStore).GetTitle(), 32, TEXT_HEAVY, STORE_TEXT_COLOR);
	}
	else {
		if (pBackgroundBitmap != NULL)                                                      // ... otherwise revise work area
			PaintBitmap(pWorkDC,pBackgroundPalette,pBackgroundBitmap,0,0,STORE_DX,STORE_DY);
		PaintMaskedBitmap(pWorkDC,pBackgroundPalette,pStoreBitmap,0,0,STORE_DX,STORE_DY);
		UpdateContent(pWorkDC);                                 // ... then zap it to the screen
		if (pTitleText != NULL)
			(*pTitleText).DisplayString(pWorkDC, (*pGeneralStore).GetTitle(), 32, TEXT_HEAVY, STORE_TEXT_COLOR);
		(*pDC).BitBlt(0,0,STORE_DX,STORE_DY,pWorkDC,0,0,SRCCOPY);
	}

	(*pDC).SelectPalette(pPalOld,FALSE);                        // reset the palette
	
	DoArrowCursor();											// return to an arrow cursor
}


void CGeneralStore::UpdatePage(CDC *pDC)
{
CPalette	*pPalOld;

	if (pWorkDC == NULL) 										// update everything if no work area
		(*pStoreDialog).InvalidateRect(NULL,FALSE);
	else {                                                      // otherwise just update central area
		DoWaitCursor();											// put up the hourglass cursor
		pPalOld = (*pDC).SelectPalette(pBackgroundPalette,FALSE);// setup the proper palette
		(*pDC).RealizePalette();
		if (pBackgroundBitmap != NULL)
			PaintBitmap(pWorkDC,pBackgroundPalette,pBackgroundBitmap,0,0,STORE_DX,STORE_DY);
		PaintMaskedBitmap(pWorkDC,pBackgroundPalette,pStoreBitmap,0,0,STORE_DX,STORE_DY);
		UpdateContent(pWorkDC);                                 // zap it to the screen
		(*pDC).BitBlt(
					0,
					STORE_BORDER_DY + STORE_TITLEZONE_DY,
					STORE_DX,
					STORE_DY - (STORE_BORDER_DY << 1) - STORE_TITLEZONE_DY - STORE_TEXTZONE_DY,
					pWorkDC,
					0,
					STORE_BORDER_DY + STORE_TITLEZONE_DY,
					SRCCOPY);
		(*pDC).BitBlt(
					STORE_DX - TEXT_MORE_DX,
					STORE_DY - STORE_CURL_DY,
					TEXT_MORE_DX,
					STORE_CURL_DY,
					pWorkDC,
					STORE_DX - TEXT_MORE_DX,
					STORE_DY - STORE_CURL_DY,
					SRCCOPY);
		(*pDC).SelectPalette(pPalOld,FALSE);                    // reset the palette
		DoArrowCursor();
	}															// return to an arrow cursor
}


void CGeneralStore::UpdateContent(CDC *pDC)
{
CRect	myRect;
CItem	*pItem;
int		i, x, y, dx, dy;
CFont	*pFontOld;
CSize   textInfo;                       						// font info about the text to be displayed
TEXTMETRIC	fontMetrics;

	if ((*pGeneralStore).ItemCount() <= 0)
		return;
		
	nStore_DX = STORE_DX - (STORE_BORDER_DX << 1);				// calculate the horizontal space we have available
	nItemsPerRow = nStore_DX / STORE_BITMAP_DX;               // estimate number of items that will fit
	while(TRUE) {
		nItem_DDX = (nStore_DX - (nItemsPerRow * STORE_BITMAP_DX)) / (nItemsPerRow - 1);	// now evaluate the distance that would occur between
		if (nItem_DDX >= STORE_BITMAP_DDX)                       // ... items, and if is less than the minimum allowed
			break;                                              // ... then reduce the count of items per row
		nItemsPerRow -= 1;
	}
	
	nStore_DY = STORE_DY - STORE_TEXTZONE_DY - (STORE_BORDER_DY << 1) - STORE_TITLEZONE_DY;	// calculate the vertical space we have available
	nItemsPerColumn = nStore_DY / STORE_BITMAP_DY;            // estimate number of items that will fit
	while(TRUE) {
		nItem_DDY = (nStore_DY - (nItemsPerColumn * STORE_BITMAP_DY)) / (nItemsPerColumn - 1);    // now evaluate the distance that would occur between
		if (nItem_DDY >= STORE_BITMAP_DDY)                       // ... items, and if is less than the minimum allowed
			break;                                              // ... then reduce the count of items per column
		nItemsPerColumn -= 1;
	}

	pItem = (*pGeneralStore).FetchItem(nFirstSlot);                // get first item on this page
	for (i = 0; (i < (nItemsPerRow * nItemsPerColumn)) && (pItem != NULL); i++) {							// will thumb through all of them
		x = (i % nItemsPerRow);                                 // calculate its horizontal position
		x *= (STORE_BITMAP_DX + nItem_DDX);                      // ... allowing proper spacing between items
		y = (i / nItemsPerRow);                                 // calculate its vertical position
		y *= (STORE_BITMAP_DY + nItem_DDY);                      // ... allowing proper spacing between items
		UpdateItem(pDC,pItem,x + STORE_BORDER_DX,y + STORE_BORDER_DY + STORE_TITLEZONE_DY);		// now show the item
		pItem = (*pItem).GetNext();
	}

	if (pStoreBitmap != NULL) {
		pFontOld = (*pDC).SelectObject(pFont);  				// select it into our context
		(*pDC).SetBkMode(TRANSPARENT);            					// make the text overlay transparently
		(*pDC).GetTextMetrics(&fontMetrics);						// show whether there are more notes
		x = STORE_DX - TEXT_MORE_DX;	                            // ... that can be scrolled through
		y = STORE_DY -
			STORE_CURL_DY +
			((STORE_CURL_DY - fontMetrics.tmHeight) >> 1) -
			TEXT_MORE_DY;
		textInfo = (*pDC).GetTextExtent(MORE_TEXT_BLURB,MORE_TEXT_LENGTH);
		dx = textInfo.cx;
	    dy = fontMetrics.tmHeight;
	
		if (pItem == NULL) {
			myRect.SetRect(x,y,x + dx,y + dy);
			BltBitmap(pDC, pBackgroundPalette, pStoreBitmap, &myRect, &myRect, SRCCOPY);
		}
		else {
		    (*pDC).SetTextColor(STORE_MORE_COLOR);
			(*pDC).TextOut(x,y,MORE_TEXT_BLURB,MORE_TEXT_LENGTH);
		}
		(void) (*pDC).SelectObject(pFontOld);         					// map out the font
	}
}


void CGeneralStore::UpdateItem(CDC *pDC, CItem *pItem, int nX, int nY)
{
BOOL	bSuccess = TRUE;
char	*pArtSpec;

	pArtSpec = (*pItem).GetArtSpec();
	if (pArtSpec != NULL)
		bSuccess = PaintMaskedDIB(pDC,pBackgroundPalette,pArtSpec,nX,nY,STORE_BITMAP_DX,STORE_BITMAP_DY);
}


void CGeneralStore::UpdateCrowns(CDC *pDC)
{
CItem	*pItem;
char	chBuffer[128];

	pItem = (*pInventory).FindItem(MG_OBJ_CROWN);
	if ((pItem == NULL) ||
		((*pItem).GetQuantity() < 1)) {
		if (bPlayingHodj)
			strcpy(chBuffer,"Podj has no Crowns");
		else
			strcpy(chBuffer,"Podj has no Crowns");
		}
	else {
		if (bPlayingHodj)
			sprintf(chBuffer,"Hodj has %ld Crowns",(*pItem).GetQuantity());
		else
			sprintf(chBuffer,"Podj has %ld Crowns",(*pItem).GetQuantity());
	}
	(*pItemCost).DisplayString(pDC, chBuffer, 18, TEXT_BOLD, STORE_TEXT_COLOR);
}


BOOL CGeneralStore::OnEraseBkgnd(CDC *)
{
return(TRUE);													// do not automatically erase background to white
}


void CGeneralStore::ClearDialogImage(void)
{
	if (pBackgroundBitmap != NULL) {
		delete pOKButton;
		pOKButton = NULL;
		(*pStoreDialog).ValidateRect(NULL);
		RefreshBackground();
	}
}


void CGeneralStore::RefreshBackground(void)
{
CDC	*pDC;

	if (pBackgroundBitmap != NULL) {
		pDC = (*pStoreDialog).GetDC();						// get a context for our window
		PaintBitmap(pDC,pBackgroundPalette,pBackgroundBitmap,0,0,STORE_DX,STORE_DY);
		(*pStoreDialog).ReleaseDC(pDC);						// release the context
	}
}


void CGeneralStore::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	
}


void CGeneralStore::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}


int CGeneralStore::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
BOOL	bSuccess;
	
	::AddFontResource("msserif.fon");
	pFont = new CFont();
	ASSERT(pFont != NULL);
	bSuccess = (*pFont).CreateFont(-14,0,0,0,FW_BOLD,0,0,0,0,OUT_RASTER_PRECIS,0,PROOF_QUALITY,FF_ROMAN,"MS Sans Serif");
	ASSERT(bSuccess);

	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}


BOOL CGeneralStore::CreateWorkAreas(CDC *pDC)
{
BOOL		bSuccess = FALSE;
CPalette	*pPalOld;
CRect		myRect;

	pStoreBitmap = FetchBitmap(pDC,NULL,STORE_SPEC);
	if (pStoreBitmap == NULL)
		return(FALSE);

	if ((GetFreeSpace(0) >= (unsigned long) 500000) &&
    	(GlobalCompact((unsigned long) 500000) >= (unsigned long) 400000))
		pBackgroundBitmap = FetchScreenBitmap(pDC,pBackgroundPalette,0,0,STORE_DX,STORE_DY);
	else
		pBackgroundBitmap = NULL;

	pPalOld = (*pDC).SelectPalette(pBackgroundPalette,FALSE);		
	(void) (*pDC).RealizePalette();

	if ((GetFreeSpace(0) >= (unsigned long) 1000000) &&
    	(GlobalCompact((unsigned long) 500000) >= (unsigned long) 450000)) {
		pWork = new CBitmap();
		if ((*pWork).CreateCompatibleBitmap(pDC,STORE_DX,STORE_DY)) {
			pWorkDC = new CDC();
			if ((pWorkDC != NULL) &&
				(*pWorkDC).CreateCompatibleDC(pDC)) {
				pWorkPalOld = (*pWorkDC).SelectPalette(pBackgroundPalette,FALSE);
				(void) (*pWorkDC).RealizePalette();
				pWorkOld = (*pWorkDC).SelectObject(pWork);
				if (pWorkOld != NULL)
					bSuccess = TRUE;
			}
		}
	}
	else {
		pWork = NULL;
		pWorkDC = NULL;
		bSuccess = TRUE;
	}

	if (!bSuccess) {
		if (pWorkPalOld != NULL) {
			(void) (*pWorkDC).SelectPalette(pWorkPalOld,FALSE);
			pWorkPalOld = NULL;
		}
		if (pWork != NULL) {
			delete pWork;
			pWork = NULL;
		}
		delete pWorkDC;
		pWorkDC = NULL;
		bSuccess = TRUE;
	}

	(void) (*pDC).SelectPalette(pWorkPalOld,FALSE);

	myRect.SetRect(STORE_TEXTZONE_DX,
				   STORE_BORDER_DY + STORE_TITLEZONE_DDY,
				   STORE_DX - STORE_TEXTZONE_DX,
				   STORE_BORDER_DY + STORE_TITLEZONE_DDY + STORE_TITLEZONE_DY);
	pTitleText = new CText(pDC, pBackgroundPalette, &myRect, JUSTIFY_CENTER);

	myRect.SetRect(STORE_TEXTZONE_DX,
					 STORE_DY - STORE_BORDER_DY - STORE_TEXTZONE_DY + STORE_TEXTZONE_DDDY,
					 STORE_DX - STORE_TEXTZONE_DX,
					 STORE_DY - STORE_BORDER_DY - STORE_TEXTZONE_DY + STORE_TEXTZONE_DDY + STORE_TEXTZONE_DDDY);
	pItemText = new CText(pDC, pBackgroundPalette, &myRect, JUSTIFY_CENTER);
	myRect.SetRect(STORE_COSTZONE_DX,
					 STORE_DY - STORE_BORDER_DY - STORE_COSTZONE_DY + STORE_COSTZONE_DDDY,
					 STORE_DX - STORE_COSTZONE_DX,
					 STORE_DY - STORE_BORDER_DY - STORE_COSTZONE_DY + STORE_COSTZONE_DDY + STORE_COSTZONE_DDDY);
	pItemCost = new CText(pDC, pBackgroundPalette, &myRect, JUSTIFY_CENTER);
	
	return(TRUE);
}


void CGeneralStore::OnMouseMove(UINT nFlags, CPoint point)
{
HCURSOR	hNewCursor = NULL;
CWinApp	*pMyApp = NULL;
CRect	testRect;
int		i;
CItem	*pItem;
int		nPrice;
CDC		*pDC;
char	chBuffer[128];

	if (!bActiveWindow)								// punt if window not active
		return;
		
	pMyApp = AfxGetApp();

	if (OkayRect.PtInRect(point))					// use standard arrow in buttons
		hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);
	else
	if (ScrollTopRect.PtInRect(point)) {            // set cursor to scolling up okay or invalid
		if (nFirstSlot == 0)                        // ... depending on current slot for page 
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_INVALID);
		else
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_ARROWUP);
	}
	else
	if (ScrollBotRect.PtInRect(point)) {			// set cursor to scrolling down okay or invalid
		if (nFirstSlot + (nItemsPerRow * nItemsPerColumn) >= (*pGeneralStore).ItemCount())
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_INVALID);
		else
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_ARROWDN);
	}
	else {											// see if cursor is on an inventory item
		i = SelectedItem(point);                    // ... and if so, then show the text blurb
		if ((i >= 0) &&                             // ... for it at the base of the scroll
			((i + nFirstSlot) < (*pGeneralStore).ItemCount())) {
			pItem = (*pGeneralStore).FetchItem(i + nFirstSlot);
			if (pItem != NULL) {
				hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);
				pDC = GetDC();
				nPrice = (*pItem).GetValue();
				if ((*pItem).GetQuantity() == 1) {
					if (nPrice == 1)
						strcpy(chBuffer,"It can be bought for 1 Crown");
					else
						sprintf(chBuffer,"It can be bought for %d Crowns",nPrice);
				}
				else
				if (nPrice == 1)
					strcpy(chBuffer,"One can be bought for 1 Crown");
				else
					sprintf(chBuffer,"One can be bought for %d Crowns",nPrice);
				(*pItemText).DisplayString(pDC, (*pItem).GetDescription(), 18, TEXT_BOLD, STORE_TEXT_COLOR);
				(*pItemCost).DisplayString(pDC, chBuffer, 18, TEXT_BOLD, STORE_TEXT_COLOR);
				ReleaseDC(pDC);
				hNewCursor = (*pMyApp).LoadCursor(IDC_STORE_DOLLAR);
			}
		}
	}

	if (hNewCursor == NULL) {						// use default cursor if not specified
		hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);
		pDC = GetDC();
		(*pItemText).DisplayString(pDC, "", 18, TEXT_BOLD, STORE_TEXT_COLOR);
		(*pItemText).RestoreBackground(pDC);    	// clear any extant item text
		UpdateCrowns(pDC);    						// clear any extant item text
		ReleaseDC(pDC);
	}

	ASSERT(hNewCursor != NULL);						// force the cursor change
	MFC::SetCursor(hNewCursor);
	
	CDialog::OnMouseMove(nFlags, point);            // do standard mouse move behavior
}


void CGeneralStore::OnLButtonDown(UINT nFlags, CPoint point)
{
int		i;
int		nPrice;
CDC		*pDC = NULL;
CItem	*pItem;
CItem	*pCrowns;
BOOL	bUpdateNeeded = FALSE;
CSound	*pSound;

	if (!bActiveWindow)								// punt if window not active
		return;

	if (ScrollTopRect.PtInRect(point) &&			// if click is in upper curl, then
		(nFirstSlot > 0)) {                         // ... scroll up if not at first item
		nFirstSlot -= (nItemsPerRow * nItemsPerColumn);
		if (nFirstSlot < 0)
			nFirstSlot = 0;
		bUpdateNeeded = TRUE;
	}
	else                                            // if click is in lower curl, then
	if (ScrollBotRect.PtInRect(point) &&            // ... scroll down if not show last item
		(nFirstSlot + (nItemsPerRow * nItemsPerColumn) < (*pGeneralStore).ItemCount())) {
		nFirstSlot += (nItemsPerRow * nItemsPerColumn);
		bUpdateNeeded = TRUE;
	}
	else {											// see if cursor is on an inventory item
		i = SelectedItem(point);                    // ... and if so, then show then dispatch
		if ((i >= 0) &&                             // ... on its action code
			((i + nFirstSlot) < (*pGeneralStore).ItemCount())) {
			pItem = (*pGeneralStore).FetchItem(i + nFirstSlot);
			if (pItem != NULL) {
				pDC = GetDC();
				nPrice = (*pItem).GetValue();
				pCrowns = (*pInventory).FindItem(MG_OBJ_CROWN);
				if ((pCrowns == NULL) ||
					((*pCrowns).GetQuantity() < nPrice)) {
					(*pItemText).DisplayString(pDC, "Not have enough crowns to buy that!", 18, TEXT_BOLD, STORE_BLURB_COLOR);
					pSound = new CSound(this,(bPlayingHodj ? ".\\sound\\gsps5.wav" : ".\\sound\\gsps6.wav"),SOUND_WAVE | SOUND_QUEUE | SOUND_AUTODELETE);
					(*pSound).SetDrivePath(lpMetaGameStruct->m_chCDPath);
					(*pSound).Play();
				}
				else {
					(*pItemText).DisplayString(pDC, "Thanks for the purchase!", 18, TEXT_BOLD, STORE_BLURB_COLOR);
					if (brand() & 1)
						pSound = new CSound(this,(bPlayingHodj ? ".\\sound\\gsps1.wav" : ".\\sound\\gsps2.wav"),SOUND_WAVE | SOUND_QUEUE | SOUND_AUTODELETE);
					else
						pSound = new CSound(this,(bPlayingHodj ? ".\\sound\\gsps3.wav" : ".\\sound\\gsps4.wav"),SOUND_WAVE | SOUND_QUEUE | SOUND_AUTODELETE);
					(*pSound).SetDrivePath(lpMetaGameStruct->m_chCDPath);
					(*pSound).Play();
					(*pInventory).DiscardItem(pCrowns,nPrice);
					if ((*pItem).GetQuantity() > 1) {
						(*pGeneralStore).DiscardItem(pItem,1);
						(*pInventory).AddItem((*pItem).GetID(),1);
					}
					else {
						(*pGeneralStore).RemoveItem(pItem);
						(*pInventory).AddItem(pItem);
						bUpdateNeeded = TRUE;
					}



               	}
				(*pItemCost).RestoreBackground(pDC);    // clear any extant item cost
				ReleaseDC(pDC);
			}
		}
	}

	if (bUpdateNeeded) {                              // update the inventory page if required
		pDC = GetDC();
		UpdatePage(pDC);
		ReleaseDC(pDC);
	}

	OnMouseMove(nFlags,point);                      // do standard mouse movement
	
	CDialog::OnLButtonDown(nFlags, point);          // do standard mouse clicking
}


int CGeneralStore::SelectedItem(CPoint point)
{
int		i = -1,
		x, y, col, row;
CRect	testRect;

	testRect.SetRect(STORE_BORDER_DX,
					 STORE_BORDER_DY + STORE_TITLEZONE_DY,
					 STORE_DX - STORE_BORDER_DX,
					 STORE_DY - STORE_TEXTZONE_DY - STORE_BORDER_DY);
	if (testRect.PtInRect(point)) {
		col = (point.x - STORE_BORDER_DX) / (STORE_BITMAP_DX + nItem_DDX);
		row = (point.y - STORE_BORDER_DY - STORE_TITLEZONE_DY) / (STORE_BITMAP_DY + nItem_DDY);
		x = col * (STORE_BITMAP_DX + nItem_DDX) + STORE_BITMAP_DX + STORE_BORDER_DX;
		y = row * (STORE_BITMAP_DY + nItem_DDY) + STORE_BITMAP_DX + STORE_BORDER_DY + STORE_TITLEZONE_DY;
		if ((point.x < x) &&
			(point.y < y))
			i = (row * nItemsPerRow) + col;
	}
	
	return(i);
}


BOOL CGeneralStore::OnSetCursor(CWnd *pWnd, UINT /*nHitTest*/, UINT /*message*/)
{
	if ((*pWnd).m_hWnd == (*this).m_hWnd)
		return(TRUE);
	else
		return(FALSE);
}


void CGeneralStore::DoWaitCursor(void)
{
CWinApp	*pMyApp;

	pMyApp = AfxGetApp();

	(void) (*pMyApp).BeginWaitCursor();
}


void CGeneralStore::DoArrowCursor(void)
{
CWinApp	*pMyApp;

	pMyApp = AfxGetApp();

	(void) (*pMyApp).EndWaitCursor();
}
