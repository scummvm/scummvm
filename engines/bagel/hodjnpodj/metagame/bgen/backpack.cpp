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
#include "bagel/hodjnpodj/metagame/bgen/notebook.h"
#include "bagel/hodjnpodj/metagame/bgen/backpack.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define TEXT_MORE_DX        120                     // offset of "more" indicator from right margin
#define TEXT_MORE_DY        5                       // offset of "more" indicator bottom of scroll
#define MORE_TEXT_BLURB     "[ More ]"              // actual text to display for "more" indicator
#define MORE_TEXT_LENGTH    8                       // # characters in "more" indicator string

#define BUTTON_DY           15                      // offset for Okay button from backpack base

extern CBfcMgr *lpMetaGameStruct;

LRESULT BackpackHookProc(int, WPARAM, LPARAM); // keyboard hook procedure definition

extern  HINSTANCE   hDLLInst;
extern  HINSTANCE   hExeInst;

typedef LRESULT(*FPPACKHOOKPROC)(int, WPARAM, LPARAM);

static  FPPACKHOOKPROC  lpfnKbdHook = nullptr;         // pointer to hook procedure

static  HHOOK       hKbdHook = nullptr;                // handle for hook procedure

static  CBackpack *pBackpackDialog = nullptr;        // pointer to our backpack dialog box

static  CWnd *pParentWnd = nullptr;             // parent window for this dialog

static  CColorButton *pOKButton = nullptr;             // OKAY button on scroll

static  CInventory *pInventory = nullptr;         // inventory to be displayed

//static  CDC *pBackpackDC = nullptr;            // device context for the backpack bitmap
static  CBitmap *pBackpackBitmap = nullptr;        // bitmap for an entirely blank backpack

static  CBitmap *pBackgroundBitmap = nullptr;
static  CPalette *pBackgroundPalette = nullptr;

static  CDC *pWorkDC = nullptr;                // context and resources for the offscreen
static  CBitmap *pWork = nullptr,                  // ... work area (only if memory permits)
*pWorkOld = nullptr;
static  CPalette *pWorkPalOld = nullptr;

static  CText *pItemText = nullptr;              // item information field
static  CText *pTitleText = nullptr;             // title information field

static  CFont *pFont = nullptr;                  // font to use for displaying backpack text
//static  char        chPathName[128];                // buffer to hold path name of the backpack file

static  bool        bActiveWindow = false;          // whether our window is active
static  bool        bFirstTime = true;              // flag for first time information is displayed
static  int         nBackpack_DX, nBackpack_DY;     // size of useable backpack background
static  int         nItem_DDX, nItem_DDY;           // space separation between inventory items
static  int         nItemsPerColumn, nItemsPerRow;  // span of items that fit on the background
static  int         nFirstSlot = 0;                 // first item in current inventory page


/////////////////////////////////////////////////////////////////////////////
// CBackpack dialog



bool CBackpack::SetupKeyboardHook() {
	pBackpackDialog = this;                         // retain pointer to our dialog box

	lpfnKbdHook = &BackpackHookProc;
	hKbdHook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)lpfnKbdHook, hExeInst, GetCurrentTask());
	if (hKbdHook == nullptr)                           // plug in our keyboard hook
		return false;

	return true;
}


void CBackpack::RemoveKeyboardHook() {
	if (m_bKeyboardHook)
		UnhookWindowsHookEx(hKbdHook);                  // unhook our keyboard procedure

	pBackpackDialog = nullptr;
	lpfnKbdHook = nullptr;
	hKbdHook = nullptr;
	m_bKeyboardHook = false;
}


LRESULT BackpackHookProc(int code, WPARAM wParam, LPARAM lParam) {
	CDC *pDC = nullptr;

	if (code < 0)                                       // required to punt to next hook
		return CallNextHookEx((HHOOK)lpfnKbdHook, code, wParam, lParam);

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
				pDC = (*pBackpackDialog).GetDC();
			}
			break;
		case VK_DOWN:                               // go to next page of text
		case VK_NUMPAD2:
		case VK_NEXT:
			if (nFirstSlot + (nItemsPerRow * nItemsPerColumn) < (*pInventory).ItemCount()) {
				nFirstSlot += (nItemsPerRow * nItemsPerColumn);
				pDC = (*pBackpackDialog).GetDC();
			}
			break;
		case VK_HOME:                               // go to first page of text
			if (nFirstSlot != 0) {
				nFirstSlot = 0;
				pDC = (*pBackpackDialog).GetDC();
			}
			break;
		case VK_END:                                // go to last page of text
			nFirstSlot = (*pInventory).ItemCount() - (nItemsPerRow * nItemsPerColumn);
			if (nFirstSlot < 0)
				nFirstSlot = 0;
			pDC = (*pBackpackDialog).GetDC();
		}

	if (pDC != nullptr) {                                  // update the inventory page if required
		CBackpack::UpdatePage(pDC);
		(*pBackpackDialog).ReleaseDC(pDC);
		return true;
	}

	return false;
}


CBackpack::CBackpack(CWnd *pParent, CPalette *pPalette, CInventory *pInvent)
	: CDialog(CBackpack::IDD, pParent) {
	pBackgroundPalette = pPalette;                      // retain palette to be used
	pInventory = pInvent;                               // retain inventory to be displayed
	pParentWnd = pParent;                               // retain pointer to parent window

	//{{AFX_DATA_INIT(CBackpack)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CBackpack::DoDataExchange(CDataExchange *pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBackpack)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBackpack, CDialog)
	//{{AFX_MSG_MAP(CBackpack)
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
// CBackpack message handlers

void CBackpack::OnOK() {
	ClearDialogImage();
	CDialog::EndDialog(IDC_BACKPACK_OKAY);
}


void CBackpack::OnCancel() {
	ClearDialogImage();
	CDialog::OnCancel();
}


void CBackpack::OnDestroy() {
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

	if (pBackpackBitmap != nullptr) {
		delete pBackpackBitmap;
		pBackpackBitmap = nullptr;
	}

	if (pBackgroundBitmap != nullptr) {
		delete pBackgroundBitmap;
		pBackgroundBitmap = nullptr;
		bUpdateNeeded = (*pParentWnd).GetUpdateRect(nullptr, false);
		if (bUpdateNeeded)
			(*pParentWnd).ValidateRect(nullptr);
	}

	if (pTitleText != nullptr) {
		delete pTitleText;
		pTitleText = nullptr;
	}
	if (pItemText != nullptr) {
		delete pItemText;
		pItemText = nullptr;
	}

	CDialog::OnDestroy();
}


bool CBackpack::OnInitDialog() {
	bool    bSuccess;
	CWnd *pButton;                                               // pointer to the OKAY button
	CRect   myRect;                                                 // rectangle that holds the button location
	int     x, y, dx, dy;                                           // used for calculating positioning info

	CDialog::OnInitDialog();                                    // do basic dialog initialization

	if (m_pParentWnd == nullptr)                                   // get our parent window
		m_pParentWnd = ((CWnd *)this)->GetParent();            // ... as passed to us or inquired about
	assert(m_pParentWnd);

	(*m_pParentWnd).GetWindowRect(&myRect);
	x = myRect.left + (((myRect.right - myRect.left) - BACKPACK_DX) >> 1);
	y = myRect.top + (((myRect.bottom - myRect.top) - BACKPACK_DY) >> 1);
	BackpackRect.SetRect(0, 0, BACKPACK_DX, BACKPACK_DY);
	MoveWindow(x, y, BACKPACK_DX, BACKPACK_DY);         // center the dialog box on the parent

	pButton = GetDlgItem((int)GetDefID());                     // get the window for the okay button
	ASSERT(pButton != nullptr);                                    // ... and verify we have it
	(*pButton).GetWindowRect(&myRect);                          // get the button's position and size

	dx = myRect.right - myRect.left;                            // calculate where to place the button
	x = (BackpackRect.right - dx) >> 1;                         // ... centered at the bottom edge
	dy = myRect.bottom - myRect.top;
	y = BackpackRect.bottom - dy - BUTTON_DY;

	(*pButton).MoveWindow(x, y, dx, dy);                        // reposition the button
	OkayRect.SetRect(x, y, x + dx, y + dy);

	pOKButton = new CColorButton();                   // build a color QUIT button to let us exit
	ASSERT(pOKButton != nullptr);
	(*pOKButton).SetPalette(pBackgroundPalette);        // set the palette to use
	bSuccess = (*pOKButton).SetControl((int)GetDefID(), this); // tie to the dialog control
	ASSERT(bSuccess);

	ScrollTopRect.SetRect(0, 0, BACKPACK_DX, BACKPACK_CURL_DY); // setup rectangles for scrolling areas
	ScrollBotRect.SetRect(0, BACKPACK_DY - BACKPACK_CURL_DY, BACKPACK_DX, BACKPACK_DY);

	m_bKeyboardHook = SetupKeyboardHook();          // establish keyboard hook

	bFirstTime = true;
	nFirstSlot = 0;

	return true;                                              // return true  unless focused on a control
}


void CBackpack::OnActivate(unsigned int nState, CWnd *, bool /*bMinimized*/) {
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


void CBackpack::OnPaint() {
	bool        bSuccess;
	CPaintDC    dc(this);                                           // device context for painting

	if (bFirstTime) {                                           // acquire resources and if first time
		bFirstTime = false;
		bSuccess = CreateWorkAreas(&dc);
		if (!bSuccess)
			CDialog::OnCancel();
	}

	UpdateBackpack(&dc);
}


void CBackpack::UpdateBackpack(CDC *pDC) {
	CPalette *pPalOld;

	ShowWaitCursor();                                             // put up the hourglass cursor

	pPalOld = (*pDC).SelectPalette(pBackgroundPalette, false);      // setup the proper palette
	(*pDC).RealizePalette();

	if (pWorkDC == nullptr) {                                      // if we don't have a work area
		RefreshBackground();                                    // ... then update the screen directly
		PaintMaskedBitmap(pDC, pBackgroundPalette, pBackpackBitmap, 0, 0, BACKPACK_DX, BACKPACK_DY);
		UpdateContent(pDC);
		if (pTitleText != nullptr)
			(*pTitleText).DisplayString(pDC, (*pInventory).GetTitle(), 32, TEXT_HEAVY, BACKPACK_TEXT_COLOR);
	} else {
		if (pBackgroundBitmap != nullptr)                                                      // ... otherwise revise work area
			PaintBitmap(pWorkDC, pBackgroundPalette, pBackgroundBitmap, 0, 0, BACKPACK_DX, BACKPACK_DY);
		PaintMaskedBitmap(pWorkDC, pBackgroundPalette, pBackpackBitmap, 0, 0, BACKPACK_DX, BACKPACK_DY);
		UpdateContent(pWorkDC);                                 // ... then zap it to the screen
		if (pTitleText != nullptr)
			(*pTitleText).DisplayString(pWorkDC, (*pInventory).GetTitle(), 32, TEXT_HEAVY, BACKPACK_TEXT_COLOR);
		(*pDC).BitBlt(0, 0, BACKPACK_DX, BACKPACK_DY, pWorkDC, 0, 0, SRCCOPY);
	}

	(*pDC).SelectPalette(pPalOld, false);                       // reset the palette

	DoArrowCursor();                                            // return to an arrow cursor
}


void CBackpack::UpdatePage(CDC *pDC) {
	CPalette *pPalOld;

	if (pWorkDC == nullptr)                                        // update everything if no work area
		(*pBackpackDialog).InvalidateRect(nullptr, false);
	else {                                                      // otherwise just update central area
		ShowWaitCursor();                                         // put up the hourglass cursor
		pPalOld = (*pDC).SelectPalette(pBackgroundPalette, false); // setup the proper palette
		(*pDC).RealizePalette();
		PaintMaskedBitmap(pWorkDC, pBackgroundPalette, pBackpackBitmap, 0, 0, BACKPACK_DX, BACKPACK_DY);
		UpdateContent(pWorkDC);                                 // zap it to the screen
		(*pDC).BitBlt(
			0,
			BACKPACK_BORDER_DY + BACKPACK_TITLEZONE_DY,
			BACKPACK_DX,
			BACKPACK_DY - (BACKPACK_BORDER_DY << 1) - BACKPACK_TITLEZONE_DY - BACKPACK_TEXTZONE_DY,
			pWorkDC,
			0,
			BACKPACK_BORDER_DY + BACKPACK_TITLEZONE_DY,
			SRCCOPY);
		(*pDC).BitBlt(
			BACKPACK_DX - TEXT_MORE_DX,
			BACKPACK_DY - BACKPACK_CURL_DY,
			TEXT_MORE_DX,
			BACKPACK_CURL_DY,
			pWorkDC,
			BACKPACK_DX - TEXT_MORE_DX,
			BACKPACK_DY - BACKPACK_CURL_DY,
			SRCCOPY);
		(*pDC).SelectPalette(pPalOld, false);                   // reset the palette
		DoArrowCursor();
	}                                                           // return to an arrow cursor
}


void CBackpack::UpdateContent(CDC *pDC) {
	CRect   myRect;
	CItem *pItem;
	CFont *pFontOld;
	CSize   textInfo;                                               // font info about the text to be displayed
	TEXTMETRIC  fontMetrics;
	int     i, x, y, dx, dy;

	if ((*pInventory).ItemCount() <= 0)
		return;

	nBackpack_DX = BACKPACK_DX - (BACKPACK_BORDER_DX << 1);     // calculate the horizontal space we have available
	nItemsPerRow = nBackpack_DX / BACKPACK_BITMAP_DX;           // estimate number of items that will fit
	while (true) {
		nItem_DDX = (nBackpack_DX - (nItemsPerRow * BACKPACK_BITMAP_DX)) / (nItemsPerRow - 1);  // now evaluate the distance that would occur between
		if ((nItem_DDX >= BACKPACK_BITMAP_DDX) ||               // ... items, and if is less than the minimum allowed
			(nItemsPerRow == 1))
			break;                                              // ... then reduce the count of items per row
		nItemsPerRow -= 1;
	}

	nBackpack_DY = BACKPACK_DY - BACKPACK_TEXTZONE_DY - (BACKPACK_BORDER_DY << 1) - BACKPACK_TITLEZONE_DY;  // calculate the vertical space we have available
	nItemsPerColumn = nBackpack_DY / BACKPACK_BITMAP_DY;        // estimate number of items that will fit
	while (true) {
		nItem_DDY = (nBackpack_DY - (nItemsPerColumn * BACKPACK_BITMAP_DY)) / (nItemsPerColumn - 1);    // now evaluate the distance that would occur between
		if ((nItem_DDY >= BACKPACK_BITMAP_DDY) ||               // ... items, and if is less than the minimum allowed
			(nItemsPerColumn == 1))
			break;                                              // ... then reduce the count of items per column
		nItemsPerColumn -= 1;
	}

	pItem = (*pInventory).FetchItem(nFirstSlot);                // get first item on this page
	for (i = 0; (i < (nItemsPerRow * nItemsPerColumn)) && (pItem != nullptr); i++) {                           // will thumb through all of them
		x = (i % nItemsPerRow);                                 // calculate its horizontal position
		x *= (BACKPACK_BITMAP_DX + nItem_DDX);                      // ... allowing proper spacing between items
		y = (i / nItemsPerRow);                                 // calculate its vertical position
		y *= (BACKPACK_BITMAP_DY + nItem_DDY);                      // ... allowing proper spacing between items
		UpdateItem(pDC, pItem, x + BACKPACK_BORDER_DX, y + BACKPACK_BORDER_DY + BACKPACK_TITLEZONE_DY);     // now show the item
		pItem = (*pItem).GetNext();
	}

	if (pBackpackBitmap != nullptr) {
		pFontOld = (*pDC).SelectObject(pFont);                  // select it into our context
		(*pDC).SetBkMode(TRANSPARENT);                              // make the text overlay transparently
		(*pDC).GetTextMetrics(&fontMetrics);                        // show whether there are more notes
		x = BACKPACK_DX - TEXT_MORE_DX;                             // ... that can be scrolled through
		y = BACKPACK_DY -
			BACKPACK_CURL_DY +
			((BACKPACK_CURL_DY - fontMetrics.tmHeight) >> 1) -
			TEXT_MORE_DY;
		textInfo = (*pDC).GetTextExtent(MORE_TEXT_BLURB, MORE_TEXT_LENGTH);
		dx = textInfo.cx;
		dy = fontMetrics.tmHeight;
		if (pItem == nullptr) {
			myRect.SetRect(x, y, x + dx, y + dy);
			BltBitmap(pDC, pBackgroundPalette, pBackpackBitmap, &myRect, &myRect, SRCCOPY);
		} else {
			(*pDC).SetTextColor(BACKPACK_MORE_COLOR);
			(*pDC).TextOut(x, y, MORE_TEXT_BLURB, MORE_TEXT_LENGTH);
		}
		(*pDC).SelectObject(pFontOld);                            // map out the font
	}
}


void CBackpack::UpdateItem(CDC *pDC, CItem *pItem, int nX, int nY) {
	CFont *pFontOld = nullptr;               // font that was mapped to the context
	char    chBuffer[32];

	PaintMaskedDIB(pDC, pBackgroundPalette, (*pItem).GetArtSpec(), nX, nY, BACKPACK_BITMAP_DX, BACKPACK_BITMAP_DY);

	if (((*pItem).m_nQuantity == 0) ||
		((*pItem).m_nQuantity > 1)) {
		Common::sprintf_s(chBuffer, "%ld", (*pItem).m_nQuantity);
		pFontOld = (*pDC).SelectObject(pFont);                  // select it into our context
		(*pDC).SetBkMode(TRANSPARENT);                          // make the text overlay transparently
		(*pDC).SetTextColor(BACKPACK_BLURB_COLOR);              // set the color of the text
		(*pDC).TextOut(                             // zap the text to the work area
			nX,
			nY,
			(const char *)chBuffer,
			strlen(chBuffer));
		(*pDC).SelectObject(pFontOld);                     // map out the font
	}
}


bool CBackpack::OnEraseBkgnd(CDC *) {
	return true;                                                  // do not automatically erase background to white
}


void CBackpack::ClearDialogImage() {
	if (pBackgroundBitmap != nullptr) {
		delete pOKButton;
		pOKButton = nullptr;
		(*pBackpackDialog).ValidateRect(nullptr);
		RefreshBackground();
	}
}


void CBackpack::RefreshBackground() {
	CDC *pDC;

	if (pBackgroundBitmap != nullptr) {
		pDC = (*pBackpackDialog).GetDC();                       // get a context for our window
		PaintBitmap(pDC, pBackgroundPalette, pBackgroundBitmap, 0, 0, BACKPACK_DX, BACKPACK_DY);
		(*pBackpackDialog).ReleaseDC(pDC);                      // release the context
	}
}


void CBackpack::OnShowWindow(bool bShow, unsigned int nStatus) {
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here

}


void CBackpack::OnSize(unsigned int nType, int cx, int cy) {
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

}


int CBackpack::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	bool    bSuccess;

	AddFontResource("msserif.fon");
	pFont = new CFont();
	ASSERT(pFont != nullptr);
	bSuccess = (*pFont).CreateFont(BACKPACK_FONT_SIZE, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FF_ROMAN, "MS Sans Serif");
	ASSERT(bSuccess);

	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}


bool CBackpack::CreateWorkAreas(CDC *pDC) {
	bool        bSuccess = false;
	//CPalette *pPalOld;
	CRect       myRect;

	pBackpackBitmap = FetchBitmap(pDC, nullptr, BACKPACK_SPEC);
	if (pBackpackBitmap == nullptr)
		return false;

	pBackgroundBitmap = FetchScreenBitmap(pDC, pBackgroundPalette, 0, 0, BACKPACK_DX, BACKPACK_DY);

	(*pDC).SelectPalette(pBackgroundPalette, false);
	(*pDC).RealizePalette();

	if ((GetFreeSpace(0) >= (unsigned long)1000000) &&
		(GlobalCompact((unsigned long)500000) >= (unsigned long)450000)) {
		pWork = new CBitmap();
		if ((*pWork).CreateCompatibleBitmap(pDC, BACKPACK_DX, BACKPACK_DY)) {
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

	myRect.SetRect(BACKPACK_TEXTZONE_DX,
		BACKPACK_BORDER_DY + BACKPACK_TITLEZONE_DDY,
		BACKPACK_DX - BACKPACK_TEXTZONE_DX,
		BACKPACK_BORDER_DY + BACKPACK_TITLEZONE_DDY + BACKPACK_TITLEZONE_DY);
	pTitleText = new CText(pDC, pBackgroundPalette, &myRect, JUSTIFY_CENTER);

	return true;
}


void CBackpack::OnMouseMove(unsigned int nFlags, CPoint point) {
	HCURSOR hNewCursor = nullptr;
	CWinApp *pMyApp = nullptr;
	CRect   testRect;
	int     i;
	CItem *pItem;
	CDC *pDC;

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
		if (nFirstSlot + (nItemsPerRow * nItemsPerColumn) >= (*pInventory).ItemCount())
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_INVALID);
		else
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_ARROWDN);
	} else {                                        // see if cursor is on an inventory item
		i = SelectedItem(point);                    // ... and if so, then show the text blurb
		if ((i >= 0) &&                             // ... for it at the base of the scroll
			((i + nFirstSlot) < (*pInventory).ItemCount())) {
			pItem = (*pInventory).FetchItem(i + nFirstSlot);
			if (pItem != nullptr) {
				if ((*pItem).m_nActionCode == ITEM_ACTION_NOTEBOOK)
					hNewCursor = (*pMyApp).LoadCursor(IDC_NOTEBOOK_BOOK);
				else if ((*pItem).GetSoundSpec() != nullptr) {
					hNewCursor = (*pMyApp).LoadCursor(IDC_NOTEBOOK_SOUND);
					(*pItem).m_nActionCode = ITEM_ACTION_SOUND;
				} else
					hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);
				pDC = GetDC();
				if (pItemText == nullptr) {
					testRect.SetRect(BACKPACK_TEXTZONE_DX,
						BACKPACK_DY - BACKPACK_BORDER_DY - BACKPACK_TEXTZONE_DY + BACKPACK_TEXTZONE_DDY,
						BACKPACK_DX - BACKPACK_TEXTZONE_DX,
						BACKPACK_DY - BACKPACK_BORDER_DY + BACKPACK_TEXTZONE_DDY);
					pItemText = new CText(pDC, pBackgroundPalette, &testRect, JUSTIFY_CENTER);
				}
				if (pItemText != nullptr)
					(*pItemText).DisplayString(pDC, (*pItem).GetDescription(), 24, TEXT_BOLD, BACKPACK_TEXT_COLOR);
				ReleaseDC(pDC);
			}
		}
	}

	if (hNewCursor == nullptr) {                       // use default cursor if not specified
		hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);
		if (pItemText != nullptr) {
			pDC = GetDC();
			(*pItemText).RestoreBackground(pDC);    // clear any extant item text
			delete pItemText;
			pItemText = nullptr;
			ReleaseDC(pDC);
		}
	}

	ASSERT(hNewCursor != nullptr);                     // force the cursor change
	MFC::SetCursor(hNewCursor);

	CDialog::OnMouseMove(nFlags, point);            // do standard mouse move behavior
}


void CBackpack::OnLButtonDown(unsigned int nFlags, CPoint point) {
	int         i;
	CDC *pDC = nullptr;
	CItem *pItem;
	CWnd *pButton;
	CSound *pSound;
	CWnd *pControl;

	if (!bActiveWindow)                             // punt if window not active
		return;

	if (ScrollTopRect.PtInRect(point) &&            // if click is in upper curl, then
		(nFirstSlot > 0)) {                         // ... scroll up if not at first item
		nFirstSlot -= (nItemsPerRow * nItemsPerColumn);
		if (nFirstSlot < 0)
			nFirstSlot = 0;
		pDC = GetDC();
	} else                                          // if click is in lower curl, then
		if (ScrollBotRect.PtInRect(point) &&            // ... scroll down if not show last item
			(nFirstSlot + (nItemsPerRow * nItemsPerColumn) < (*pInventory).ItemCount())) {
			nFirstSlot += (nItemsPerRow * nItemsPerColumn);
			pDC = GetDC();
		} else {                                        // see if cursor is on an inventory item
			i = SelectedItem(point);                    // ... and if so, then show then dispatch
			if ((i >= 0) &&                             // ... on its action code
				((i + nFirstSlot) < (*pInventory).ItemCount())) {
				pItem = (*pInventory).FetchItem(i + nFirstSlot);
				if (pItem != nullptr) {
					CNotebook   NotebookDlg(this, pBackgroundPalette, (*pItem).GetFirstNote(), nullptr);
					switch ((*pItem).GetActionCode()) {
					case ITEM_ACTION_NOTEBOOK:      // open the notebook
						pItem = (*pInventory).FindItem(MG_OBJ_HODJ_NOTEBOOK);
						if (pItem == nullptr)
							pItem = (*pInventory).FindItem(MG_OBJ_PODJ_NOTEBOOK);
						if (pItem != nullptr) {
							pButton = GetDlgItem((int)GetDefID()); // get the window for the okay button
							ASSERT(pButton != nullptr);                // ... and verify we have it
							(*pButton).EnableWindow(false);  // disable & enable across note entries
							NotebookDlg.DoModal();   // invoke the notebook dialog box
							(*pButton).EnableWindow(true);
							pControl = GetDlgItem((int)GetDefID());
							GotoDlgCtrl(pControl);
							bActiveWindow = true;
						}
						break;
					case ITEM_ACTION_SOUND:
						pSound = new CSound(this, (*pItem).GetSoundSpec(), SOUND_WAVE | SOUND_QUEUE | SOUND_AUTODELETE);
						(*pSound).setDrivePath(lpMetaGameStruct->m_chCDPath);
						(*pSound).play();
						break;
					}
				}
			}
		}

	if (pDC != nullptr) {                              // update the inventory page if required
		UpdatePage(pDC);
		ReleaseDC(pDC);
	}

	OnMouseMove(nFlags, point);                     // do standard mouse movement

	CDialog::OnLButtonDown(nFlags, point);          // do standard mouse clicking
}


int CBackpack::SelectedItem(CPoint point) {
	int     i = -1,
		x, y, col, row;
	CRect   testRect;

	testRect.SetRect(BACKPACK_BORDER_DX,
		BACKPACK_BORDER_DY + BACKPACK_TITLEZONE_DY,
		BACKPACK_DX - BACKPACK_BORDER_DX,
		BACKPACK_DY - BACKPACK_TEXTZONE_DY - BACKPACK_BORDER_DY);
	if (testRect.PtInRect(point)) {
		col = (point.x - BACKPACK_BORDER_DX) / (BACKPACK_BITMAP_DX + nItem_DDX);
		row = (point.y - BACKPACK_BORDER_DY - BACKPACK_TITLEZONE_DY) / (BACKPACK_BITMAP_DY + nItem_DDY);
		x = col * (BACKPACK_BITMAP_DX + nItem_DDX) + BACKPACK_BITMAP_DX + BACKPACK_BORDER_DX;
		y = row * (BACKPACK_BITMAP_DY + nItem_DDY) + BACKPACK_BITMAP_DX + BACKPACK_BORDER_DY + BACKPACK_TITLEZONE_DY;
		if ((point.x < x) &&
			(point.y < y))
			i = (row * nItemsPerRow) + col;
	}

	return i;
}


bool CBackpack::OnSetCursor(CWnd *pWnd, unsigned int /*nHitTest*/, unsigned int /*message*/) {
	if ((*pWnd).m_hWnd == (*this).m_hWnd)
		return true;
	else
		return false;
}


void CBackpack::ShowWaitCursor() {
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();

	(*pMyApp).BeginWaitCursor();
}


void CBackpack::DoArrowCursor() {
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();

	(*pMyApp).EndWaitCursor();
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
