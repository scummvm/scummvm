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

#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/metagame/gtl/store.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define FONT_SIZE			16
#define TEXT_MORE_DX        120                     // offset of "more" indicator from right margin
#define TEXT_MORE_DY        5                       // offset of "more" indicator bottom of scroll
#define MORE_TEXT_BLURB     "[ More ]"              // actual text to display for "more" indicator
#define MORE_TEXT_LENGTH    8                       // # characters in "more" indicator string

#define BUTTON_DY           15                      // offset for Okay button from store base


extern CBfcMgr      *lpMetaGameStruct;


extern "C" {
	LRESULT StoreHookProc(int, WPARAM, LPARAM);  // keyboard hook procedure definition
}

extern  HINSTANCE   hDLLInst;
extern  HINSTANCE   hExeInst;

typedef LRESULT(FAR PASCAL *FPSTOREHOOKPROC)(int, WPARAM, LPARAM);

static  FPSTOREHOOKPROC     lpfnKbdHook = nullptr;         // pointer to hook procedure

static  HHOOK       hKbdHook = nullptr;                // handle for hook procedure

static  CWnd        *pParentWnd = nullptr;             // parent window for this dialog
static  CGeneralStore   *pStoreDialog = nullptr;       // pointer to our store dialog box

static  CColorButton *pOKButton = nullptr;             // OKAY button on scroll

static  CInventory  *pInventory = nullptr;             // inventory to be displayed
static  CInventory  *pGeneralStore = nullptr;          // general store's inventory


//static  CDC         *pStoreDC = nullptr;               // device context for the store bitmap
static  CBitmap     *pStoreBitmap = nullptr;           // bitmap for an entirely blank store
//                     *pStoreBitmapOld = nullptr;        // bitmap previously mapped to the store context

static  CBitmap     *pBackgroundBitmap = nullptr;
static  CPalette    *pBackgroundPalette = nullptr;

static  CDC         *pWorkDC = nullptr;                // context and resources for the offscreen
static  CBitmap     *pWork = nullptr,                  // ... work area (only if memory permits)
                     *pWorkOld = nullptr;
static  CPalette    *pWorkPalOld = nullptr;

static  CText       *pItemText = nullptr;              // item information field
static  CText       *pItemCost = nullptr;              // item cost field
static  CText       *pTitleText = nullptr;             // title information field

static  CFont       *pFont = nullptr;                  // font to use for displaying store text
//static  char        chPathName[128];                // buffer to hold path name of the store file

static  bool        bActiveWindow = false;          // whether our window is active
static  bool        bFirstTime = true;              // flag for first time information is displayed
static  int         nStore_DX, nStore_DY;           // size of useable store background
static  int         nItem_DDX, nItem_DDY;           // space separation between inventory items
static  int         nItemsPerColumn, nItemsPerRow;  // span of items that fit on the background
static  int         nFirstSlot = 0;                 // first item in current inventory page

static  bool        bPlayingHodj = true;            // whether playing Hodj or Podj

/////////////////////////////////////////////////////////////////////////////
// CGeneralStore dialog



bool CGeneralStore::SetupKeyboardHook() {
	pStoreDialog = this;                            // retain pointer to our dialog box

	hKbdHook = SetWindowsHookEx(WH_KEYBOARD, StoreHookProc, hExeInst, GetCurrentTask());
	if (hKbdHook == nullptr)                           // plug in our keyboard hook
		return false;

	return true;
}


void CGeneralStore::RemoveKeyboardHook() {
	if (m_bKeyboardHook)
		UnhookWindowsHookEx(hKbdHook);                  // unhook our keyboard procedure

	pStoreDialog = nullptr;
	lpfnKbdHook = nullptr;
	hKbdHook = nullptr;
	m_bKeyboardHook = false;
}


extern "C"
LRESULT StoreHookProc(int code, WPARAM wParam, LPARAM lParam) {
	CDC *pDC = nullptr;

	if (code < 0)                                       // required to punt to next hook
		return (CallNextHookEx((HHOOK) lpfnKbdHook, code, wParam, lParam));

	if (lParam & 0xA0000000)                            // ignore ALT and key release
		return false;

	if (bActiveWindow)
		switch (wParam) {                               // process only the keys we are looking for
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
		case VK_DOWN:                               // go to next page of text
		case VK_NUMPAD2:
		case VK_NEXT:
			if (nFirstSlot + (nItemsPerRow * nItemsPerColumn) < (*pGeneralStore).ItemCount()) {
				nFirstSlot += (nItemsPerRow * nItemsPerColumn);
				pDC = (*pStoreDialog).GetDC();
			}
			break;
		case VK_HOME:                               // go to first page of text
			if (nFirstSlot != 0) {
				nFirstSlot = 0;
				pDC = (*pStoreDialog).GetDC();
			}
			break;
		case VK_END:                                // go to last page of text
			nFirstSlot = (*pGeneralStore).ItemCount() - (nItemsPerRow * nItemsPerColumn);
			if (nFirstSlot < 0)
				nFirstSlot = 0;
			pDC = (*pStoreDialog).GetDC();
		}

	if (pDC != nullptr) {                                  // update the inventory page if required
		CGeneralStore::UpdatePage(pDC);
		(*pStoreDialog).ReleaseDC(pDC);
		return true;
	}

	return false;
}


CGeneralStore::CGeneralStore(CWnd* pParent, CPalette *pPalette, CInventory *pStore, CInventory *pInvent)
	: CDialog(CGeneralStore::IDD, pParent) {
	pBackgroundPalette = pPalette;                      // retain palette to be used
	pGeneralStore = pStore;                             // retain store inventory
	pInventory = pInvent;                               // retain inventory to be displayed
	pParentWnd = pParent;                               // retain pointer to parent window

	//{{AFX_DATA_INIT(CGeneralStore)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGeneralStore::DoDataExchange(CDataExchange* pDX) {
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

void CGeneralStore::OnOK() {
	ClearDialogImage();
	CDialog::EndDialog(IDC_STORE_OKAY);
}


void CGeneralStore::OnCancel() {
	ClearDialogImage();
	CDialog::OnCancel();
}


void CGeneralStore::OnDestroy() {
	bool    bUpdateNeeded;

	if (pFont != nullptr)
		delete pFont;                               // release the font file

	if (pOKButton != nullptr)                          // release the button
		delete pOKButton;

	if (m_bKeyboardHook)                                // remove keyboard hook, if present
		RemoveKeyboardHook();

	if (pWorkOld != nullptr) {
		(*pWorkDC).SelectObject(pWorkOld);
		pWorkOld = nullptr;
	}
	if (pWorkPalOld != nullptr) {
		(*pWorkDC).SelectPalette(pWorkPalOld, false);
		pWorkPalOld = nullptr;
	}
	if (pWork != nullptr) {
		delete pWork;
		pWork = nullptr;
	}
	if (pWorkDC != nullptr) {
		delete pWorkDC;
		pWorkDC = nullptr;
	}

	if (pStoreBitmap != nullptr) {
		delete pStoreBitmap;
		pStoreBitmap = nullptr;
	}

	if (pBackgroundBitmap != nullptr) {
		delete pBackgroundBitmap;
		pBackgroundBitmap = nullptr;
		bUpdateNeeded = (*pParentWnd).GetUpdateRect(nullptr, false);
		if (bUpdateNeeded)
			(*pParentWnd).ValidateRect(nullptr);
	}

	if (pItemText != nullptr) {
		delete pItemText;
		pItemText = nullptr;
	}
	if (pItemCost != nullptr) {
		delete pItemCost;
		pItemCost = nullptr;
	}
	if (pTitleText != nullptr) {
		delete pTitleText;
		pTitleText = nullptr;
	}

	CDialog::OnDestroy();
}


bool CGeneralStore::OnInitDialog() {
	bool    bSuccess;
	CWnd    *pButton;                                               // pointer to the OKAY button
	CRect   myRect;                                                 // rectangle that holds the button location
	int     x, y, dx, dy;                                           // used for calculating positioning info

	CDialog::OnInitDialog();                                    // do basic dialog initialization

	if (m_pParentWnd == nullptr)                                   // get our parent window
		m_pParentWnd = ((CWnd *) this)->GetParent();            // ... as passed to us or inquired about

	(*m_pParentWnd).GetWindowRect(&myRect);
	x = myRect.left + (((myRect.right - myRect.left) - STORE_DX) >> 1);
	y = myRect.top + (((myRect.bottom - myRect.top) - STORE_DY) >> 1);
	StoreRect.SetRect(0, 0, STORE_DX, STORE_DY);
	MoveWindow(x, y, STORE_DX, STORE_DY);                       // center the dialog box on the parent

	pButton = GetDlgItem((int) GetDefID());                     // get the window for the okay button
	ASSERT(pButton != nullptr);                                    // ... and verify we have it
	(*pButton).GetWindowRect(&myRect);                          // get the button's position and size

	dx = myRect.right - myRect.left;                            // calculate where to place the button
	x = (StoreRect.right - dx) >> 1;                            // ... centered at the bottom edge
	dy = myRect.bottom - myRect.top;
	y = StoreRect.bottom - dy - BUTTON_DY;

	(*pButton).MoveWindow(x, y, dx, dy);                        // reposition the button
	OkayRect.SetRect(x, y, x + dx, y + dy);

	pOKButton = new CColorButton();                               // build a color QUIT button to let us exit
	ASSERT(pOKButton != nullptr);
	(*pOKButton).SetPalette(pBackgroundPalette);                // set the palette to use
	bSuccess = (*pOKButton).SetControl((int) GetDefID(), this); // tie to the dialog control
	ASSERT(bSuccess);

	ScrollTopRect.SetRect(0, 0, STORE_DX, STORE_CURL_DY);       // setup rectangles for scrolling areas
	ScrollBotRect.SetRect(0, STORE_DY - STORE_CURL_DY, STORE_DX, STORE_DY);

	m_bKeyboardHook = SetupKeyboardHook();          // establish keyboard hook

	bFirstTime = true;
	nFirstSlot = 0;

	if ((*pInventory).FindItem(MG_OBJ_HODJ_NOTEBOOK) != nullptr)   // see who is playing
		bPlayingHodj = true;
	else
		bPlayingHodj = false;

	return true;                                              // return true  unless focused on a control
}


void CGeneralStore::OnActivate(unsigned int nState, CWnd *, bool /*bMinimized*/) {
	bool    bUpdateNeeded;

	switch (nState) {
	case WA_INACTIVE:
		bActiveWindow = false;
		break;
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		bActiveWindow = true;
		bUpdateNeeded = GetUpdateRect(nullptr, false);
		if (bUpdateNeeded)
			InvalidateRect(nullptr, false);
	}
}


void CGeneralStore::OnPaint() {
	bool        bSuccess;
	CPaintDC    dc(this);                                           // device context for painting

	if (bFirstTime) {                                           // acquire resources and if first time
		bFirstTime = false;
		bSuccess = CreateWorkAreas(&dc);
		if (!bSuccess)
			CDialog::OnCancel();
	}

	UpdateStore(&dc);
}


void CGeneralStore::UpdateStore(CDC *pDC) {
	CPalette    *pPalOld;

	DoWaitCursor();                                             // put up the hourglass cursor

	pPalOld = (*pDC).SelectPalette(pBackgroundPalette, false);      // setup the proper palette
	(*pDC).RealizePalette();

	if (pWorkDC == nullptr) {                                      // if we don't have a work area
		RefreshBackground();                                    // ... then update the screen directly
		PaintMaskedBitmap(pDC, pBackgroundPalette, pStoreBitmap, 0, 0, STORE_DX, STORE_DY);
		UpdateContent(pDC);
		if (pTitleText != nullptr)
			(*pTitleText).DisplayString(pDC, (*pGeneralStore).GetTitle(), 32, TEXT_HEAVY, STORE_TEXT_COLOR);
	} else {
		if (pBackgroundBitmap != nullptr)                                                      // ... otherwise revise work area
			PaintBitmap(pWorkDC, pBackgroundPalette, pBackgroundBitmap, 0, 0, STORE_DX, STORE_DY);
		PaintMaskedBitmap(pWorkDC, pBackgroundPalette, pStoreBitmap, 0, 0, STORE_DX, STORE_DY);
		UpdateContent(pWorkDC);                                 // ... then zap it to the screen
		if (pTitleText != nullptr)
			(*pTitleText).DisplayString(pWorkDC, (*pGeneralStore).GetTitle(), 32, TEXT_HEAVY, STORE_TEXT_COLOR);
		(*pDC).BitBlt(0, 0, STORE_DX, STORE_DY, pWorkDC, 0, 0, SRCCOPY);
	}

	(*pDC).SelectPalette(pPalOld, false);                       // reset the palette

	DoArrowCursor();                                            // return to an arrow cursor
}


void CGeneralStore::UpdatePage(CDC *pDC) {
	CPalette    *pPalOld;

	if (pWorkDC == nullptr)                                        // update everything if no work area
		(*pStoreDialog).InvalidateRect(nullptr, false);
	else {                                                      // otherwise just update central area
		DoWaitCursor();                                         // put up the hourglass cursor
		pPalOld = (*pDC).SelectPalette(pBackgroundPalette, false); // setup the proper palette
		(*pDC).RealizePalette();
		if (pBackgroundBitmap != nullptr)
			PaintBitmap(pWorkDC, pBackgroundPalette, pBackgroundBitmap, 0, 0, STORE_DX, STORE_DY);
		PaintMaskedBitmap(pWorkDC, pBackgroundPalette, pStoreBitmap, 0, 0, STORE_DX, STORE_DY);
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
		(*pDC).SelectPalette(pPalOld, false);                   // reset the palette
		DoArrowCursor();
	}                                                           // return to an arrow cursor
}


void CGeneralStore::UpdateContent(CDC *pDC) {
	CRect   myRect;
	CItem   *pItem;
	int     i, x, y, dx, dy;
	CFont   *pFontOld;
	CSize   textInfo;                                               // font info about the text to be displayed
	TEXTMETRIC  fontMetrics;

	if ((*pGeneralStore).ItemCount() <= 0)
		return;

	nStore_DX = STORE_DX - (STORE_BORDER_DX << 1);              // calculate the horizontal space we have available
	nItemsPerRow = nStore_DX / STORE_BITMAP_DX;               // estimate number of items that will fit
	while (true) {
		nItem_DDX = (nStore_DX - (nItemsPerRow * STORE_BITMAP_DX)) / (nItemsPerRow - 1);    // now evaluate the distance that would occur between
		if (nItem_DDX >= STORE_BITMAP_DDX)                       // ... items, and if is less than the minimum allowed
			break;                                              // ... then reduce the count of items per row
		nItemsPerRow -= 1;
	}

	nStore_DY = STORE_DY - STORE_TEXTZONE_DY - (STORE_BORDER_DY << 1) - STORE_TITLEZONE_DY; // calculate the vertical space we have available
	nItemsPerColumn = nStore_DY / STORE_BITMAP_DY;            // estimate number of items that will fit
	while (true) {
		nItem_DDY = (nStore_DY - (nItemsPerColumn * STORE_BITMAP_DY)) / (nItemsPerColumn - 1);    // now evaluate the distance that would occur between
		if (nItem_DDY >= STORE_BITMAP_DDY)                       // ... items, and if is less than the minimum allowed
			break;                                              // ... then reduce the count of items per column
		nItemsPerColumn -= 1;
	}

	pItem = (*pGeneralStore).FetchItem(nFirstSlot);                // get first item on this page
	for (i = 0; (i < (nItemsPerRow * nItemsPerColumn)) && (pItem != nullptr); i++) {                           // will thumb through all of them
		x = (i % nItemsPerRow);                                 // calculate its horizontal position
		x *= (STORE_BITMAP_DX + nItem_DDX);                      // ... allowing proper spacing between items
		y = (i / nItemsPerRow);                                 // calculate its vertical position
		y *= (STORE_BITMAP_DY + nItem_DDY);                      // ... allowing proper spacing between items
		UpdateItem(pDC, pItem, x + STORE_BORDER_DX, y + STORE_BORDER_DY + STORE_TITLEZONE_DY);  // now show the item
		pItem = (*pItem).GetNext();
	}

	if (pStoreBitmap != nullptr) {
		pFontOld = (*pDC).SelectObject(pFont);                  // select it into our context
		(*pDC).SetBkMode(TRANSPARENT);                              // make the text overlay transparently
		(*pDC).GetTextMetrics(&fontMetrics);                        // show whether there are more notes
		x = STORE_DX - TEXT_MORE_DX;                                // ... that can be scrolled through
		y = STORE_DY -
		    STORE_CURL_DY +
		    ((STORE_CURL_DY - fontMetrics.tmHeight) >> 1) -
		    TEXT_MORE_DY;
		textInfo = (*pDC).GetTextExtent(MORE_TEXT_BLURB, MORE_TEXT_LENGTH);
		dx = textInfo.cx;
		dy = fontMetrics.tmHeight;

		if (pItem == nullptr) {
			myRect.SetRect(x, y, x + dx, y + dy);
			BltBitmap(pDC, pBackgroundPalette, pStoreBitmap, &myRect, &myRect, SRCCOPY);
		} else {
			(*pDC).SetTextColor(STORE_MORE_COLOR);
			(*pDC).TextOut(x, y, MORE_TEXT_BLURB, MORE_TEXT_LENGTH);
		}
		(*pDC).SelectObject(pFontOld);                            // map out the font
	}
}


void CGeneralStore::UpdateItem(CDC *pDC, CItem *pItem, int nX, int nY) {
	bool bSuccess = true;
	const char *pArtSpec;

	pArtSpec = (*pItem).GetArtSpec();
	if (pArtSpec != nullptr)
		bSuccess = PaintMaskedDIB(pDC, pBackgroundPalette, pArtSpec, nX, nY, STORE_BITMAP_DX, STORE_BITMAP_DY);

	(void)bSuccess; // suppress unused variable warning
}


void CGeneralStore::UpdateCrowns(CDC *pDC) {
	CItem   *pItem;
	char    chBuffer[128];

	pItem = (*pInventory).FindItem(MG_OBJ_CROWN);
	if ((pItem == nullptr) ||
	        ((*pItem).GetQuantity() < 1)) {
		if (bPlayingHodj)
			Common::strcpy_s(chBuffer, "Hodj has no Crowns");
		else
			Common::strcpy_s(chBuffer, "Podj has no Crowns");
	} else {
		if (bPlayingHodj)
			Common::sprintf_s(chBuffer, "Hodj has %ld Crowns", (*pItem).GetQuantity());
		else
			Common::sprintf_s(chBuffer, "Podj has %ld Crowns", (*pItem).GetQuantity());
	}
	(*pItemCost).DisplayString(pDC, chBuffer, FONT_SIZE, TEXT_BOLD, STORE_TEXT_COLOR);
}


bool CGeneralStore::OnEraseBkgnd(CDC *) {
	return true;                                                  // do not automatically erase background to white
}


void CGeneralStore::ClearDialogImage() {
	if (pBackgroundBitmap != nullptr) {
		delete pOKButton;
		pOKButton = nullptr;
		(*pStoreDialog).ValidateRect(nullptr);
		RefreshBackground();
	}
}


void CGeneralStore::RefreshBackground() {
	CDC *pDC;

	if (pBackgroundBitmap != nullptr) {
		pDC = (*pStoreDialog).GetDC();                      // get a context for our window
		PaintBitmap(pDC, pBackgroundPalette, pBackgroundBitmap, 0, 0, STORE_DX, STORE_DY);
		(*pStoreDialog).ReleaseDC(pDC);                     // release the context
	}
}


void CGeneralStore::OnShowWindow(bool bShow, unsigned int nStatus) {
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here

}


void CGeneralStore::OnSize(unsigned int nType, int cx, int cy) {
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

}


int CGeneralStore::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	bool    bSuccess;

	AddFontResource("msserif.fon");
	pFont = new CFont();
	ASSERT(pFont != nullptr);
	bSuccess = (*pFont).CreateFont(-14, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FF_ROMAN, "MS Sans Serif");
	ASSERT(bSuccess);

	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}


bool CGeneralStore::CreateWorkAreas(CDC *pDC) {
	bool        bSuccess = false;
	//CPalette    *pPalOld;
	CRect       myRect;

	pStoreBitmap = FetchBitmap(pDC, nullptr, STORE_SPEC);
	if (pStoreBitmap == nullptr)
		return false;

	if ((GetFreeSpace(0) >= (unsigned long) 500000) &&
	        (GlobalCompact((unsigned long) 500000) >= (unsigned long) 400000))
		pBackgroundBitmap = FetchScreenBitmap(pDC, pBackgroundPalette, 0, 0, STORE_DX, STORE_DY);
	else
		pBackgroundBitmap = nullptr;

	(*pDC).SelectPalette(pBackgroundPalette, false);
	(*pDC).RealizePalette();

	if ((GetFreeSpace(0) >= (unsigned long) 1000000) &&
	        (GlobalCompact((unsigned long) 500000) >= (unsigned long) 450000)) {
		pWork = new CBitmap();
		if ((*pWork).CreateCompatibleBitmap(pDC, STORE_DX, STORE_DY)) {
			pWorkDC = new CDC();
			if ((pWorkDC != nullptr) &&
			        (*pWorkDC).CreateCompatibleDC(pDC)) {
				pWorkPalOld = (*pWorkDC).SelectPalette(pBackgroundPalette, false);
				(*pWorkDC).RealizePalette();
				pWorkOld = (*pWorkDC).SelectObject(pWork);
				if (pWorkOld != nullptr)
					bSuccess = true;
			}
		}
	} else {
		pWork = nullptr;
		pWorkDC = nullptr;
		bSuccess = true;
	}

	if (!bSuccess) {
		if (pWorkPalOld != nullptr) {
			(*pWorkDC).SelectPalette(pWorkPalOld, false);
			pWorkPalOld = nullptr;
		}
		if (pWork != nullptr) {
			delete pWork;
			pWork = nullptr;
		}
		delete pWorkDC;
		pWorkDC = nullptr;
		bSuccess = true;
	}

	(*pDC).SelectPalette(pWorkPalOld, false);

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

	return true;
}


void CGeneralStore::OnMouseMove(unsigned int nFlags, CPoint point) {
	HCURSOR hNewCursor = nullptr;
	CWinApp *pMyApp = nullptr;
	CRect   testRect;
	int     i;
	CItem   *pItem;
	int     nPrice;
	CDC     *pDC;
	char    chBuffer[128];

	if (!bActiveWindow)                             // punt if window not active
		return;

	pMyApp = AfxGetApp();

	if (OkayRect.PtInRect(point))                   // use standard arrow in buttons
		hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);
	else if (ScrollTopRect.PtInRect(point)) {           // set cursor to scolling up okay or invalid
		if (nFirstSlot == 0)                        // ... depending on current slot for page
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_INVALID);
		else
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_ARROWUP);
	} else if (ScrollBotRect.PtInRect(point)) {         // set cursor to scrolling down okay or invalid
		if (nFirstSlot + (nItemsPerRow * nItemsPerColumn) >= (*pGeneralStore).ItemCount())
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_INVALID);
		else
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_ARROWDN);
	} else {                                        // see if cursor is on an inventory item
		i = SelectedItem(point);                    // ... and if so, then show the text blurb
		if ((i >= 0) &&                             // ... for it at the base of the scroll
		        ((i + nFirstSlot) < (*pGeneralStore).ItemCount())) {
			pItem = (*pGeneralStore).FetchItem(i + nFirstSlot);
			if (pItem != nullptr) {
				hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);
				pDC = GetDC();
				nPrice = (*pItem).GetValue();
				if ((*pItem).GetQuantity() == 1) {
					if (nPrice == 1)
						Common::strcpy_s(chBuffer, "It can be bought for 1 Crown");
					else
						Common::sprintf_s(chBuffer, "It can be bought for %d Crowns", nPrice);
				} else if (nPrice == 1)
					Common::strcpy_s(chBuffer, "One can be bought for 1 Crown");
				else
					Common::sprintf_s(chBuffer, "One can be bought for %d Crowns", nPrice);
				(*pItemText).DisplayString(pDC, (*pItem).GetDescription(), FONT_SIZE, TEXT_BOLD, STORE_TEXT_COLOR);
				(*pItemCost).DisplayString(pDC, chBuffer, FONT_SIZE, TEXT_BOLD, STORE_TEXT_COLOR);
				ReleaseDC(pDC);
				hNewCursor = (*pMyApp).LoadCursor(IDC_STORE_DOLLAR);
			}
		}
	}

	if (hNewCursor == nullptr) {                       // use default cursor if not specified
		hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);
		pDC = GetDC();
		(*pItemText).DisplayString(pDC, "", FONT_SIZE, TEXT_BOLD, STORE_TEXT_COLOR);
		(*pItemText).RestoreBackground(pDC);        // clear any extant item text
		UpdateCrowns(pDC);                          // clear any extant item text
		ReleaseDC(pDC);
	}

	ASSERT(hNewCursor != nullptr);                     // force the cursor change
	MFC::SetCursor(hNewCursor);

	CDialog::OnMouseMove(nFlags, point);            // do standard mouse move behavior
}


void CGeneralStore::OnLButtonDown(unsigned int nFlags, CPoint point) {
	int     i;
	int     nPrice;
	CDC     *pDC = nullptr;
	CItem   *pItem;
	CItem   *pCrowns;
	bool    bUpdateNeeded = false;
	CSound  *pSound;

	if (!bActiveWindow)                             // punt if window not active
		return;

	if (ScrollTopRect.PtInRect(point) &&            // if click is in upper curl, then
	        (nFirstSlot > 0)) {                         // ... scroll up if not at first item
		nFirstSlot -= (nItemsPerRow * nItemsPerColumn);
		if (nFirstSlot < 0)
			nFirstSlot = 0;
		bUpdateNeeded = true;
	} else                                          // if click is in lower curl, then
		if (ScrollBotRect.PtInRect(point) &&            // ... scroll down if not show last item
		        (nFirstSlot + (nItemsPerRow * nItemsPerColumn) < (*pGeneralStore).ItemCount())) {
			nFirstSlot += (nItemsPerRow * nItemsPerColumn);
			bUpdateNeeded = true;
		} else {                                        // see if cursor is on an inventory item
			i = SelectedItem(point);                    // ... and if so, then show then dispatch
			if ((i >= 0) &&                             // ... on its action code
			        ((i + nFirstSlot) < (*pGeneralStore).ItemCount())) {
				pItem = (*pGeneralStore).FetchItem(i + nFirstSlot);
				if (pItem != nullptr) {
					pDC = GetDC();
					nPrice = (*pItem).GetValue();
					pCrowns = (*pInventory).FindItem(MG_OBJ_CROWN);
					if ((pCrowns == nullptr) ||
					        ((*pCrowns).GetQuantity() < nPrice)) {
						(*pItemText).DisplayString(pDC, "Not have enough crowns to buy that!", FONT_SIZE, TEXT_BOLD, STORE_BLURB_COLOR);
						pSound = new CSound(this, (bPlayingHodj ? ".\\sound\\gsps5.wav" : ".\\sound\\gsps6.wav"), SOUND_WAVE | SOUND_QUEUE | SOUND_AUTODELETE);
						(*pSound).setDrivePath(lpMetaGameStruct->m_chCDPath);
						(*pSound).play();
					} else {
						(*pItemText).DisplayString(pDC, "Thanks for the purchase!", FONT_SIZE, TEXT_BOLD, STORE_BLURB_COLOR);
						if (brand() & 1)
							pSound = new CSound(this, (bPlayingHodj ? ".\\sound\\gsps1.wav" : ".\\sound\\gsps2.wav"), SOUND_WAVE | SOUND_QUEUE | SOUND_AUTODELETE);
						else
							pSound = new CSound(this, (bPlayingHodj ? ".\\sound\\gsps3.wav" : ".\\sound\\gsps4.wav"), SOUND_WAVE | SOUND_QUEUE | SOUND_AUTODELETE);
						(*pSound).setDrivePath(lpMetaGameStruct->m_chCDPath);
						(*pSound).play();
						(*pInventory).DiscardItem(pCrowns, nPrice);
						if ((*pItem).GetQuantity() > 1) {
							(*pGeneralStore).DiscardItem(pItem, 1);
							(*pInventory).AddItem((*pItem).GetID(), 1);
						} else {
							(*pGeneralStore).RemoveItem(pItem);
							(*pInventory).AddItem(pItem);
							bUpdateNeeded = true;
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

	OnMouseMove(nFlags, point);                     // do standard mouse movement

	CDialog::OnLButtonDown(nFlags, point);          // do standard mouse clicking
}


int CGeneralStore::SelectedItem(CPoint point) {
	int     i = -1,
	        x, y, col, row;
	CRect   testRect;

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

	return (i);
}


bool CGeneralStore::OnSetCursor(CWnd *pWnd, unsigned int /*nHitTest*/, unsigned int /*message*/) {
	if ((*pWnd).m_hWnd == (*this).m_hWnd)
		return true;
	else
		return false;
}


void CGeneralStore::DoWaitCursor() {
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();

	(*pMyApp).BeginWaitCursor();
}


void CGeneralStore::DoArrowCursor() {
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();

	(*pMyApp).EndWaitCursor();
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
