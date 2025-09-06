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
#include "bagel/hodjnpodj/metagame/gtl/rules.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define BUILD_FOR_DLL       false
#define COLOR_BUTTONS       true

#define SCROLL_PIECES       6                       // number of mid-scroll segments
#define SCROLL_SPEC     ".\\art\\lscroll.bmp"       // path for scroll DIB on disk
#define SCROLL_TOP_SPEC ".\\art\\lscrollt.bmp"      // path for scroll's top section DIB on disk
#define SCROLL_BOT_SPEC ".\\art\\lscrollb.bmp"      // path for scroll's bottom section DIB on disk
#define SCROLL_MID_SPEC ".\\art\\lscrollm.bmp"      // path for scroll's middle section DIB on disk

#define BUTTON_DY           15                      // offset for Okay button from scroll base

#define SCROLL_STRIP_WIDTH  10                      // width of scroll middle to reveal per interval 
#define SCROLL_STRIP_DELAY  1000                    // delay to wait after each partial scroll unfurling

#define TEXT_BUFFER_SIZE    512                     // # characters in the text input buffer
#define TEXT_LEFT_MARGIN    55                      // left margin offset for display of text
#define TEXT_TOP_MARGIN     5                       // top margin offset for display of text
#define TEXT_BOTTOM_MARGIN  20                      // bottom margin offset for display of text
#define TEXT_WIDTH          435                     // width of text display area
#define TEXT_MORE_DX        120                     // offset of "more" indicator from right margin
#define TEXT_MORE_DY        10                      // offset of "more" indicator bottom of scroll
#define MORE_TEXT_BLURB     "[ More ]"              // actual text to display for "more" indicator
#define MORE_TEXT_LENGTH    8                       // # characters in "more" indicator string
#define TEXT_NEWLINE        '\\'                    // character that indicates enforced line break


LRESULT PrefHookProc(int, WPARAM, LPARAM);   // keyboard hook procedure definition
static  FARPROC     pKbdHook = nullptr;                // pointer to hook procedure

static  HHOOK       hKbdHook = nullptr;                // handle for hook procedure

static  CRules      *pRulesDialog = nullptr;           // pointer to our rules dialog box
static  CWnd        *pParentWnd = nullptr;             // parent window pointer

static  CColorButton *pOKButton = nullptr;             // OKAY button on scroll

static  CDibDoc *pScrollTopDIB = nullptr,          // DIB for scroll top section
                 *pScrollMidDIB = nullptr,          // DIB for scroll mid section
                  *pScrollBotDIB = nullptr;          // DIB for scroll bottom section

static  CBitmap     *pScrollTopBitmap = nullptr,       // bitmap for scroll top section
                     *pScrollTopBitmapOld = nullptr,    // bitmap previously mapped to top section context
                      *pScrollMidBitmap = nullptr,       // bitmap for scroll mid section
                       *pScrollMidBitmapOld = nullptr,    // bitmap previously mapped to mid section context
                        *pScrollBotBitmap = nullptr,       // bitmap for scroll bottom section
                         *pScrollBotBitmapOld = nullptr;    // bitmap previously mapped to bottom section context

static  CBitmap *pScrollTopMask = nullptr,         // mask for scroll top section bitmap
                 *pScrollTopMaskOld = nullptr,      // bitmap previously mapped to top mask context
                  *pScrollMidMask = nullptr,         // mask for scroll mid section bitmap
                   *pScrollMidMaskOld = nullptr,      // bitmap previously mapped to mid mask context
                    *pScrollBotMask = nullptr,         // mask for scroll bottom section bitmap
                     *pScrollBotMaskOld = nullptr;      // bitmap previously mapped to bottom mask context

static  CBitmap *pScrollBitmap = nullptr,          // bitmap for an entirely blank scroll
                 *pScrollBitmapOld = nullptr,       // bitmap previously mapped to the scroll context
                  *pBackgroundBitmap = nullptr,      // bitmap containing the background for the scroll
                   *pBackgroundBitmapOld = nullptr,   // bitmap previously mapped to the background context
                    *pWorkBitmap = nullptr,            // bitmap containing the work area for the scroll
                     *pWorkBitmapOld = nullptr;         // bitmap previously mapped to the work area context

static  CPalette *pScrollPalette = nullptr,         // palette used for the scroll
                  *pScrollPalOld = nullptr,          // previous palette mapped to scroll context
                   *pBackgroundPalOld = nullptr,      // previous palette mapped to background context
                    *pScrollTopPalOld = nullptr,       // previous palette mapped to top context
                     *pScrollMidPalOld = nullptr,       // previous palette mapped to middle context
                      *pScrollBotPalOld = nullptr,       // previous palette mapped to bottom context
                       *pScrollTopMaskPalOld = nullptr,   // previous palette mapped to top mask context
                        *pScrollMidMaskPalOld = nullptr,   // previous palette mapped to middle mask context
                         *pScrollBotMaskPalOld = nullptr,   // previous palette mapped to bottom mask context
                          *pWorkPalOld = nullptr;            // previous palette mapped to work area context

static  CDC *pScrollDC = nullptr,              // device context for the scroll bitmap
             *pBackgroundDC = nullptr,          // device context for the background bitmap
              *pScrollTopDC = nullptr,           // device context for the top section bitmap
               *pScrollMidDC = nullptr,           // device context for the middle section bitmap
                *pScrollBotDC = nullptr,           // device context for the bottom section bitmap
                 *pScrollTopMaskDC = nullptr,       // device context for the top mask bitmap
                  *pScrollMidMaskDC = nullptr,       // device context for the middle section bitmap
                   *pScrollBotMaskDC = nullptr,       // device context for the bottom section bitmap
                    *pWorkDC = nullptr;                // device context for the work area bitmap

static  CFont       *pFont = nullptr;                  // font to use for displaying rules text
static  char        chPathName[128];                // buffer to hold path name of the rules file
static  CFile       *pHelpFile = nullptr;              // the rules file
static  uint32       nHelpFileSize = 0;              // size of rules file
static  int         nHelpPage = 0;                  // current page of rules text
static  uint32       dwHelpPagePosition[100];        // position of each page (# chars from file start)
static  bool        dwHelpPageEOL[100];             // whether page starts with enforced line break
static  bool        bHelpEOF = false;               // whether end-of-file has been reached
static  int         tabstop = 20 + TEXT_LEFT_MARGIN;// pixels per tab stop

static  char        *pSoundPath = nullptr;             // path spec for rules narration
static  CSound      *pNarrative = nullptr;             // sound object

static  bool        first_time = true;
static  bool        bActiveWindow = false;          // whether our window is active
static  bool        bBruteForce = false;            // whether we can be clever


/////////////////////////////////////////////////////////////////////////////
// CRules dialog



bool CRules::SetupKeyboardHook() {
	HINSTANCE   hInst;

	pRulesDialog = this;                            // retain pointer to our dialog box

	hInst = AfxGetInstanceHandle();                 // get our application instance

	hKbdHook = SetWindowsHookEx(WH_KEYBOARD, PrefHookProc, hInst, GetCurrentTask());

	if (hKbdHook == nullptr)                           // plug in our keyboard hook
		return false;

	return true;
}


void CRules::RemoveKeyboardHook() {
	#if BUILD_FOR_DLL
	if (m_bKeyboardHook)
		UnhookWindowsHookEx(hKbdHook);                  // unhook our keyboard procedure

	lpfnKbdHook = nullptr;
	#else
	if (m_bKeyboardHook) {
		UnhookWindowsHookEx(hKbdHook);                  // unhook our keyboard procedure
		FreeProcInstance(pKbdHook);                     // release our procedure pointer
	}

	pKbdHook = nullptr;
	#endif

	pRulesDialog = nullptr;
	hKbdHook = nullptr;
	m_bKeyboardHook = false;
}


#if BUILD_FOR_DLL
extern "C"
LRESULT KeyboardHookProc(int code, WPARAM wParam, LPARAM lParam) {
	if (code < 0)                                   // required to punt to next hook
		return (CallNextHookEx((HHOOK) lpfnKbdHook, code, wParam, lParam));
#else
LRESULT PrefHookProc(int code, WPARAM wParam, LPARAM lParam) {
	if (code < 0)                                   // required to punt to next hook
		return (CallNextHookEx((HHOOK) pKbdHook, code, wParam, lParam));
#endif

	if (lParam & 0xA0000000)                        // ignore ALT and key release
		return false;

	if (bActiveWindow)
		switch (wParam) {                               // process only the keys we are looking for
		case VK_UP:                                 // ... letting the default dialog procedure
		case VK_NUMPAD8:                            // ... deal with all the rest
		case VK_PRIOR:                              // go to previous page of text
			if (nHelpPage > 0)
				(*pRulesDialog).UpdateScroll(nHelpPage - 1);
			return true;
		case VK_DOWN:                               // go to next page of text
		case VK_NUMPAD2:
		case VK_NEXT:
			if (!bHelpEOF)
				(*pRulesDialog).UpdateScroll(nHelpPage + 1);
			return true;
		case VK_HOME:                               // go to first page of text
			if (nHelpPage > 0)
				(*pRulesDialog).UpdateScroll(0);
			return true;
		case VK_END:                                // go to last page of text
			while (!bHelpEOF) {
				(*pRulesDialog).UpdateScroll(nHelpPage + 1);
			}
			return true;
		}

	return false;
}


CRules::CRules(CWnd* pParent, char *pszPathName, CPalette *pPalette, char *pszSoundPath)
	: CDialog(CRules::IDD, pParent) {
	Common::strcpy_s(chPathName, pszPathName);                // retain path to rules file on disk
	pScrollPalette = pPalette;                      // retain palette to be used
	pParentWnd = pParent;                           // retain parent window pointer
	pSoundPath = pszSoundPath;              // retain path to sound file on disk

	//{{AFX_DATA_INIT(CRules)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRules::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRules)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRules, CDialog)
	//{{AFX_MSG_MAP(CRules)
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
// CRules message handlers

void CRules::OnOK() {
	ClearDialogImage();
	CDialog::EndDialog(IDC_RULES_OKAY);
}


void CRules::OnCancel() {
	ClearDialogImage();
	CDialog::OnCancel();
}


void CRules::OnDestroy() {
	bool    bUpdateNeeded;
	HCURSOR hNewCursor = nullptr;
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();
	hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);
	//if (hNewCursor != nullptr);
	MFC::SetCursor(hNewCursor);

	if (pNarrative != nullptr)                         // end the narration
		delete pNarrative;

	if (pOKButton != nullptr)                          // release the button
		delete pOKButton;

	delete pFont;                                   // release the font file

	(*pHelpFile).Close();                           // close and release the rules file
	delete pHelpFile;

	if (pBackgroundBitmap != nullptr) {
		bUpdateNeeded = (*pParentWnd).GetUpdateRect(nullptr, false);
		if (bUpdateNeeded)
			(*pParentWnd).ValidateRect(nullptr);
	}

	if (!bBruteForce) {
		ReleaseCompatibleContext(pWorkDC, pWorkBitmap, pWorkBitmapOld, pWorkPalOld);
		ReleaseCompatibleContext(pScrollDC, pScrollBitmap, pScrollBitmapOld, pScrollPalOld);
		ReleaseCompatibleContext(pBackgroundDC, pBackgroundBitmap, pBackgroundBitmapOld, pBackgroundPalOld);
		ReleaseCompatibleContext(pScrollMidDC, pScrollMidBitmap, pScrollMidBitmapOld, pScrollMidPalOld);
		ReleaseCompatibleContext(pScrollMidMaskDC, pScrollMidMask, pScrollMidMaskOld, pScrollMidMaskPalOld);
	}

	if (m_bKeyboardHook)                            // remove keyboard hook, if present
		RemoveKeyboardHook();

	CDialog::OnDestroy();
}


bool CRules::OnInitDialog() {
	CWnd    *pButton;                                   // pointer to the OKAY button
	CRect   myRect;                                     // rectangle that holds the button location
	int     x, y, dx, dy;                               // used for calculating positioning info
	bool    bSuccess;

	CDialog::OnInitDialog();                        // do basic dialog initialization

	if (pParentWnd == nullptr)                         // get our parent window
		pParentWnd = ((CWnd *) this)->GetParent();  // ... as passed to us or inquired about

	(*pParentWnd).GetWindowRect(&myRect);
	x = myRect.left + (((myRect.right - myRect.left) - ScrollRect.right) >> 1);
	y = myRect.top + (((myRect.bottom - myRect.top) - ScrollRect.bottom) >> 1);
	MoveWindow(x, y, ScrollRect.right, ScrollRect.bottom);      // center the dialog box on the parent

	pButton = GetDlgItem((int) GetDefID());         // get the window for the okay button
	ASSERT(pButton != nullptr);                        // ... and verify we have it
	(*pButton).GetWindowRect(&myRect);              // get the button's position and size

	dx = myRect.right - myRect.left;                // calculate where to place the button
	x = (ScrollRect.right - dx) >> 1;               // ... centered at the bottom edge
	dy = myRect.bottom - myRect.top;
	y = ScrollRect.bottom - dy - BUTTON_DY;

	(*pButton).MoveWindow(x, y, dx, dy);            // reposition the button
	OkayRect.SetRect(x, y, x + dx, y + dy);

	#if COLOR_BUTTONS
	pOKButton = new CColorButton();                   // build a color QUIT button to let us exit
	ASSERT(pOKButton != nullptr);
	(*pOKButton).SetPalette(pScrollPalette);        // set the palette to use
	bSuccess = (*pOKButton).SetControl((int) GetDefID(), this); // tie to the dialog control
	ASSERT(bSuccess);
	#endif

	if (pSoundPath != nullptr)
		pNarrative = new CSound(this, pSoundPath, SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);

	m_bKeyboardHook = SetupKeyboardHook();          // establish keyboard hook

	first_time = true;

	return true;                                  // return true  unless focused on a control
}


void CRules::OnActivate(unsigned int nState, CWnd *pWndOther, bool bMinimized) {
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


void CRules::OnPaint() {
	CPaintDC    dc(this);                               // device context for painting
	CPalette    *pPalOld = nullptr;
	CDibDoc     *pDibDoc;

	DoWaitCursor();                                 // put up the hourglass cursor

	if (pScrollPalette != nullptr) {                   // map in our palette
		pPalOld = dc.SelectPalette(pScrollPalette, false);
		dc.RealizePalette();                 // .. and make the system use it
	}

	if (first_time) {                               // unfurl scroll visually
		first_time = false;                         // ... but only if this is the first time
		UnfurlScroll(&dc);                          // ... we are updating the screen
		ReleaseCompatibleContext(pScrollTopDC, pScrollTopBitmap, pScrollTopBitmapOld, pScrollTopPalOld);
		ReleaseCompatibleContext(pScrollBotDC, pScrollBotBitmap, pScrollBotBitmapOld, pScrollBotPalOld);
		ReleaseCompatibleContext(pScrollTopMaskDC, pScrollTopMask, pScrollTopMaskOld, pScrollTopMaskPalOld);
		ReleaseCompatibleContext(pScrollBotMaskDC, pScrollBotMask, pScrollBotMaskOld, pScrollBotMaskPalOld);
		if (pNarrative != nullptr)
			(*pNarrative).play();                   // play the narration
	} else if (bBruteForce) {                           // need to paint directly to screen
		pDibDoc = LoadScrollDIB(SCROLL_SPEC, nullptr);
		if (pDibDoc != nullptr) {
			PaintScrollDIB(&dc, pDibDoc);
			delete pDibDoc;
		}
		WritePage(&dc, nHelpPage);
		UpdateMore(&dc);                            // update the "more" indicator
	} else
		dc.BitBlt(        // update the screen with just the
		    0, 0,                           // ... current page of text
		    ScrollRect.right,
		    ScrollRect.bottom,
		    pScrollDC,
		    0, 0,
		    SRCCOPY);

	if (pScrollPalette != nullptr)                     // map out our palette
		dc.SelectPalette(pPalOld, false);

	DoArrowCursor();                                // return to an arrow cursor
}


bool CRules::OnEraseBkgnd(CDC *pDC) {
	return true;                                      // do not automatically erase background to white
}


void CRules::ClearDialogImage() {
	delete pOKButton;
	pOKButton = nullptr;
	if (pBackgroundBitmap != nullptr)
		ValidateRect(nullptr);
	RefreshBackground();
}


void CRules::RefreshBackground() {
	CDC         *pDC;
	CPalette    *pPalOld = nullptr;

	if (pBackgroundBitmap == nullptr)
		return;

	pDC = GetDC();                                  // get a context for our window

	if (pScrollPalette != nullptr) {                   // map in our palette
		pPalOld = (*pDC).SelectPalette(pScrollPalette, false);
		(*pDC).RealizePalette();              // .. and make the system use it
	}

	(*pDC).BitBlt(        // repaint the background as it was
	    0,
	    0,
	    ScrollRect.right,
	    ScrollRect.bottom,
	    pBackgroundDC,
	    0,
	    0,
	    SRCCOPY);

	if (pScrollPalette != nullptr)                     // map out our palette
		(*pDC).SelectPalette(pPalOld, false);

	ReleaseDC(pDC);                                 // release the context
}


void CRules::UnfurlScroll(CDC *pDC) {
	int     i, j, k, h;
	int     scroll_delta;
	CRect   dstRect, dst2Rect;
	CDibDoc *pDibDoc;

	if (!bBruteForce)
		bBruteForce = !CreateWorkAreas(pDC);            // create our work areas
	if (bBruteForce) {
		ReleaseCompatibleContext(pWorkDC, pWorkBitmap, pWorkBitmapOld, pWorkPalOld);
		ReleaseCompatibleContext(pScrollDC, pScrollBitmap, pScrollBitmapOld, pScrollPalOld);
		ReleaseCompatibleContext(pBackgroundDC, pBackgroundBitmap, pBackgroundBitmapOld, pBackgroundPalOld);
		ReleaseCompatibleContext(pScrollMidDC, pScrollMidBitmap, pScrollMidBitmapOld, pScrollMidPalOld);
		ReleaseCompatibleContext(pScrollMidMaskDC, pScrollMidMask, pScrollMidMaskOld, pScrollMidMaskPalOld);
		pDibDoc = LoadScrollDIB(SCROLL_SPEC, nullptr);
		if (pDibDoc != nullptr) {
			PaintScrollDIB(pDC, pDibDoc);
			delete pDibDoc;
		}
		WritePage(pDC, 0);
		UpdateMore(pDC);
		ScrollTopCurlRect = ScrollTopRect;
		ScrollBotCurlRect = ScrollBotRect;
		ScrollBotCurlRect.top += ScrollTopCurlRect.bottom + ScrollMidRect.bottom * SCROLL_PIECES;
		ScrollBotCurlRect.bottom += ScrollTopCurlRect.bottom + ScrollMidRect.bottom * SCROLL_PIECES;
		return;
	}

	(*pBackgroundDC).BitBlt(      // save the entire background
	    0, 0,
	    ScrollRect.right,
	    ScrollRect.bottom,
	    pDC,
	    0, 0,
	    SRCCOPY);

	(*pScrollDC).BitBlt(       // copy background to scroll context
	    0, 0,                           // ... so we can construct the scroll
	    ScrollRect.right,               // ... on top of it in a masked manner
	    ScrollRect.bottom,
	    pDC,
	    0, 0,
	    SRCCOPY);

// setup the top part of the scroll

	ScrollTopCurlRect = ScrollTopRect;              // record where we're putting the top curl

	(*pScrollDC).BitBlt(       // mask out where the top curl goes
	    0, 0,
	    ScrollTopRect.right,
	    ScrollTopRect.bottom,
	    pScrollTopMaskDC,
	    0, 0,
	    SRCAND);

	(*pScrollTopMaskDC).BitBlt(     // invert the top curl mask
	    0, 0,
	    ScrollTopRect.right,
	    ScrollTopRect.bottom,
	    nullptr,
	    0, 0,
	    DSTINVERT);

	(*pScrollTopDC).BitBlt(      // remove transparent area from top curl
	    0, 0,
	    ScrollTopRect.right,
	    ScrollTopRect.bottom,
	    pScrollTopMaskDC,
	    0, 0,
	    SRCAND);

	(*pScrollTopMaskDC).BitBlt(     // restore top curl mask
	    0, 0,
	    ScrollTopRect.right,
	    ScrollTopRect.bottom,
	    nullptr,
	    0, 0,
	    DSTINVERT);

	(*pScrollDC).BitBlt(       // paint top curl into the scroll
	    0, 0,
	    ScrollTopRect.right,
	    ScrollTopRect.bottom,
	    pScrollTopDC,
	    0, 0,
	    SRCPAINT);

// setup the middle portion of the scroll

	dstRect = ScrollMidRect;                        // setup initial destination for a
	dstRect.top += ScrollTopRect.bottom;            // ... mid section strip of the scroll
	dstRect.bottom += ScrollTopRect.bottom;

	(*pScrollMidMaskDC).BitBlt(     // invert the mid section mask
	    0, 0,
	    ScrollMidRect.right,
	    ScrollMidRect.bottom,
	    nullptr,
	    0, 0,
	    DSTINVERT);

	(*pScrollMidDC).BitBlt(      // mask out transparent part of mid section
	    0, 0,
	    ScrollMidRect.right,
	    ScrollMidRect.bottom,
	    pScrollMidMaskDC,
	    0, 0,
	    SRCAND);

	(*pScrollMidMaskDC).BitBlt(     // reset the mid section mask
	    0, 0,
	    ScrollMidRect.right,
	    ScrollMidRect.bottom,
	    nullptr,
	    0, 0,
	    DSTINVERT);

	for (i = 0; i < SCROLL_PIECES; i++) {           // build the scroll's mid section srip-wise
		(*pScrollDC).BitBlt(
		    dstRect.left,               // mask out where the image will go
		    dstRect.top + ScrollMidRect.bottom * i,
		    ScrollMidRect.right,
		    ScrollMidRect.bottom,
		    pScrollMidMaskDC,
		    0, 0,
		    SRCAND);

		(*pScrollDC).BitBlt(      // paint in the mid section image
		    dstRect.left,
		    dstRect.top + ScrollMidRect.bottom * i,
		    ScrollMidRect.right,
		    ScrollMidRect.bottom,
		    pScrollMidDC,
		    0, 0,
		    SRCPAINT);
	}

// setup the bottom part of the scroll

	dstRect.top += ScrollMidRect.bottom * SCROLL_PIECES;    // record where the bottom curl goes
	dstRect.bottom = dstRect.top + ScrollBotRect.bottom;
	ScrollBotCurlRect = dstRect;

	(*pScrollDC).BitBlt(       // mask out where the bottom curl image goes
	    dstRect.left,
	    dstRect.top,
	    ScrollBotRect.right,
	    ScrollBotRect.bottom,
	    pScrollBotMaskDC,
	    0, 0,
	    SRCAND);

	(*pScrollBotMaskDC).BitBlt(     // invert the bottom curl mask
	    0, 0,
	    ScrollBotRect.right,
	    ScrollBotRect.bottom,
	    nullptr,
	    0, 0,
	    DSTINVERT);

	(*pScrollBotDC).BitBlt(      // mask out transparent part of curl
	    0, 0,
	    ScrollBotRect.right,
	    ScrollBotRect.bottom,
	    pScrollBotMaskDC,
	    0, 0,
	    SRCAND);

	(*pScrollBotMaskDC).BitBlt(     // reset the mask
	    0, 0,
	    ScrollBotRect.right,
	    ScrollBotRect.bottom,
	    nullptr,
	    0, 0,
	    DSTINVERT);

	(*pScrollDC).BitBlt(       // paint in the bottom curl
	    dstRect.left,
	    dstRect.top,
	    ScrollBotRect.right,
	    ScrollBotRect.bottom,
	    pScrollBotDC,
	    0, 0,
	    SRCPAINT);

// write the first page of text

	WritePage(pScrollDC, 0);                        // construct first page of text

// now display what we built, making the scroll look like it is unfurlinf from top
// ... and bottom, by increasing the display of the middle section and moving the
// ... top and bottom curls further away

	j = (ScrollMidRect.bottom * SCROLL_PIECES) / SCROLL_STRIP_WIDTH;    // number of times to update display
	k = (ScrollMidRect.bottom * SCROLL_PIECES) % SCROLL_STRIP_WIDTH;    // residual not displayed after main loop
	h = j & 0x1;                                                        // whether there is a dangling odd pixel row

	j = (j >> 1) + h + ((k > 0) ? 1 : 0);                               // revise loop count based on residual & dangle

	scroll_delta = SCROLL_STRIP_WIDTH;                                  // current width of the strip to display

	dstRect.left = 0;
	dstRect.right = ScrollRect.right;
	dstRect.top = (ScrollRect.bottom - (ScrollTopRect.bottom + ScrollBotRect.bottom)) >> 1;
	dstRect.bottom = dstRect.top + ScrollTopRect.bottom + ScrollBotRect.bottom;

	for (i = 0; i <= j; i++) {                      // do each segment in turn

		dst2Rect = dstRect;

		(*pWorkDC).BitBlt(      // splat in the background for top curl
		    0, 0,
		    ScrollTopRect.right,
		    ScrollTopRect.bottom,
		    pBackgroundDC,
		    dst2Rect.left,
		    dst2Rect.top,
		    SRCCOPY);

		(*pWorkDC).BitBlt(      // mask out where the top curl goes
		    0, 0,
		    ScrollTopRect.right,
		    ScrollTopRect.bottom,
		    pScrollTopMaskDC,
		    0, 0,
		    SRCAND);

		(*pWorkDC).BitBlt(      // insert the top curl background
		    0, 0,
		    ScrollTopRect.right,
		    ScrollTopRect.bottom,
		    pScrollTopDC,
		    0, 0,
		    SRCPAINT);

		if (i == 0) {                               // if first time, just paint curl to display
			(*pDC).BitBlt(
			    dst2Rect.left,
			    dst2Rect.top,
			    ScrollTopRect.right,
			    ScrollTopRect.bottom,
			    pWorkDC,
			    0, 0,
			    SRCCOPY);
		} else {
			(*pWorkDC).BitBlt(     // ... otherwise paint in a new piece of
			    dst2Rect.left,          // ... the middle section
			    ScrollTopRect.bottom,
			    dst2Rect.right,
			    scroll_delta,
			    pScrollDC,
			    dst2Rect.left,
			    dst2Rect.top + ScrollTopRect.bottom,
			    SRCCOPY);
			(*pDC).BitBlt(      // ... then paint it and the curl, thus
			    dst2Rect.left,          // ... causing the top to seemingly unfurl
			    dst2Rect.top,           // ... by one more strip
			    ScrollTopRect.right,
			    ScrollTopRect.bottom + scroll_delta,
			    pWorkDC,
			    0, 0,
			    SRCCOPY);
		}

		dst2Rect.top = dstRect.bottom - ScrollBotRect.bottom - scroll_delta;
		dst2Rect.bottom = dstRect.bottom;

		if ((i == 0) || ((i == 1) && (h == 1))) {   // if first time or doing last iteration
			(*pWorkDC).BitBlt(                // ... just paint in the bottom curl
			    0, 0,
			    dst2Rect.right,
			    scroll_delta,
			    pDC,
			    dst2Rect.left,
			    dst2Rect.top,
			    SRCCOPY);
		} else {                                    //
			(*pWorkDC).BitBlt(
			    0, 0,
			    dst2Rect.right,
			    scroll_delta,
			    pScrollDC,
			    dst2Rect.left,
			    dst2Rect.top,
			    SRCCOPY);
		}

		(*pWorkDC).BitBlt(
		    0,
		    scroll_delta,
		    ScrollBotRect.right,
		    ScrollBotRect.bottom,
		    pBackgroundDC,
		    ScrollBotRect.left,
		    dst2Rect.top + scroll_delta,
		    SRCCOPY);

		(*pWorkDC).BitBlt(
		    0,
		    scroll_delta,
		    ScrollBotRect.right,
		    ScrollBotRect.bottom,
		    pScrollBotMaskDC,
		    0, 0,
		    SRCAND);

		(*pWorkDC).BitBlt(
		    0,
		    scroll_delta,
		    ScrollBotRect.right,
		    ScrollBotRect.bottom,
		    pScrollBotDC,
		    0, 0,
		    SRCPAINT);

		(*pDC).BitBlt(
		    ScrollBotRect.left,
		    dst2Rect.top,
		    ScrollBotRect.right,
		    ScrollBotRect.bottom + scroll_delta,
		    pWorkDC,
		    0, 0,
		    SRCCOPY);

		if ((j == 0) && (h == 1)) {
			dstRect.top -= scroll_delta >> 1;
			dstRect.bottom += scroll_delta >> 1;
		} else {
			dstRect.top -= scroll_delta;
			dstRect.bottom += scroll_delta;
		}

		if ((i == j - 1) && (k != 0)) {
			scroll_delta = k >> 1;
			dstRect.top += SCROLL_STRIP_WIDTH - scroll_delta;
			dstRect.bottom -= SCROLL_STRIP_WIDTH - scroll_delta;

		}
	}

	UpdateMore(pDC);
}


void CRules::UpdateScroll(int nPage) {
	int         i;
	CRect       dstRect;
	CDC         *pDC;
	CPalette    *pPalOld = nullptr;

	if (bBruteForce) {
		nHelpPage = nPage;
		InvalidateRect(nullptr, false);
		return;
	}

	pDC = GetDC();

	if (pScrollPalette != nullptr) {                   // map in our palette
		pPalOld = (*pDC).SelectPalette(pScrollPalette, false);
		(*pDC).RealizePalette();              // .. and make the system use it
	}

	dstRect = ScrollMidRect;                        // setup initial destination for blts
	dstRect.top += ScrollTopRect.bottom;
	dstRect.bottom += ScrollTopRect.bottom;

	for (i = 0; i < SCROLL_PIECES; i++) {           // repaint the scroll midsection artwork
		(*pScrollDC).BitBlt(      // ... mask out where artwork goes
		    dstRect.left,
		    dstRect.top + ScrollMidRect.bottom * i,
		    ScrollMidRect.right,
		    ScrollMidRect.bottom,
		    pScrollMidMaskDC,
		    0, 0,
		    SRCAND);

		(*pScrollDC).BitBlt(      // ... paint in the scroll artwork
		    dstRect.left,
		    dstRect.top + ScrollMidRect.bottom * i,
		    ScrollMidRect.right,
		    ScrollMidRect.bottom,
		    pScrollMidDC,
		    0, 0,
		    SRCPAINT);
	}

	WritePage(pScrollDC, nPage);                    // construct the new page of text

	(*pDC).BitBlt(        // paint the result to the sceen
	    TEXT_LEFT_MARGIN,
	    ScrollTopRect.bottom + TEXT_TOP_MARGIN,
	    TEXT_WIDTH,
	    (SCROLL_PIECES * ScrollMidRect.bottom) - TEXT_TOP_MARGIN - 1,
	    pScrollDC,
	    TEXT_LEFT_MARGIN,
	    ScrollTopRect.bottom + TEXT_TOP_MARGIN,
	    SRCCOPY);

	UpdateMore(pDC);                                // update the "more" indicator

	if (pScrollPalette != nullptr)                     // map out our palette
		(*pDC).SelectPalette(pPalOld, false);

	ReleaseDC(pDC);
}


void CRules::WritePage(CDC *pDC, int nPage) {
	int         i, n, x, y;
	unsigned int        nCropped = 0;
	unsigned int        nCount = 0;
	char        chInBuf[TEXT_BUFFER_SIZE];
	CFont       *pFontOld = nullptr;
	CSize       textInfo;
	bool        bEOL, bEOF;
	TEXTMETRIC  fontMetrics;

	nHelpPage = nPage;
	bEOF = bHelpEOF = false;

	(*pHelpFile).SeekToBegin();

	if ((nHelpPage == 0) && (nPage == 0)) {
		dwHelpPagePosition[nHelpPage] = (*pHelpFile).GetPosition();
		bEOL = dwHelpPageEOL[nHelpPage] = true;
	} else {
		(*pHelpFile).Seek(dwHelpPagePosition[nPage], CFile::begin);
		bEOL = dwHelpPageEOL[nHelpPage];
	}

	dwHelpPagePosition[nHelpPage + 1] = dwHelpPagePosition[nHelpPage];

	pFontOld = (*pDC).SelectObject(pFont);

	(*pDC).GetTextMetrics(&fontMetrics);

	(*pDC).SetBkMode(TRANSPARENT);

	i = 0;
	n = 1;
	x = TEXT_LEFT_MARGIN;
	y = ScrollTopRect.bottom + TEXT_TOP_MARGIN;

	while (true) {
try_again:
		if (i >= (int) nCount) {
			if (bEOF)
				break;
			dwHelpPagePosition[nHelpPage + 1] += i + nCropped;
			nCount = (*pHelpFile).Read(&chInBuf, TEXT_BUFFER_SIZE);
			if (nCount < TEXT_BUFFER_SIZE)
				bEOF = true;
			i = 0;
			n = 1;
			nCropped = 0;
		}

		if (bEOL &&
		        ((chInBuf[i] == '\r') ||
		         (chInBuf[i] == '\n'))) {
			i += 1;
			goto try_again;
		}

		if (!bEOL &&
		        ((chInBuf[i] == ' ') ||
		         (chInBuf[i] == '\t'))) {
			i += 1;
			goto try_again;
		}

		bEOL = false;

		while (true) {
			if (i + n > (int) nCount) {
				n = nCount - i;
				if (bEOF)
					break;
				dwHelpPagePosition[nHelpPage + 1] += i + nCropped;
				strncpy(&chInBuf[0], &chInBuf[i], n);
				nCount = (*pHelpFile).Read(&chInBuf[n], TEXT_BUFFER_SIZE - n);
				nCount += n;
				if (nCount < TEXT_BUFFER_SIZE)
					bEOF = true;
				i = 0;
				n = 1;
				nCropped = 0;
			}

			if (chInBuf[i + n - 1] == '\r') {
crop_byte:
				strncpy(&chInBuf[i + n - 1], &chInBuf[i + n], nCount - i - n);
				nCount -= 1;
				nCropped += 1;
				continue;
			}

			if (chInBuf[i + n - 1] == '\n') {
				if ((n > 1) &&
				        ((chInBuf[i + n - 2] == ' ') ||
				         (chInBuf[i + n - 2] == '-')))
					goto crop_byte;
				else
					chInBuf[i + n - 1] = ' ';
			}

			if (chInBuf[i + n - 1] == TEXT_NEWLINE) {
				bEOL = true;
				n -= 1;
				break;
			}

			textInfo = (*pDC).GetTextExtent(&chInBuf[i], n);
			if (textInfo.cx > TEXT_WIDTH - TEXT_LEFT_MARGIN) {
				if ((chInBuf[i + n - 1] != ' ') &&
				        (chInBuf[i + n - 1] != '\t'))
					while (n > 0) {
						n -= 1;
						if (strchr(" \t,;:-", chInBuf[i + n - 1]) != nullptr)
							break;
					}
				break;
			}
			n += 1;
		}
//		(*pDC).TextOut(x,y,(const char *) &chInBuf[i],n);
		(*pDC).TabbedTextOut(x, y, (const char *) &chInBuf[i], n, 1, &tabstop, 0);
		if (chInBuf[i + n] == '\r')
			i += 2;
		else if (chInBuf[i + n] == TEXT_NEWLINE)
			i += 1;
		i += n;
		n = 1;
		y += fontMetrics.tmHeight;
		if (y - ScrollTopRect.bottom > (SCROLL_PIECES * ScrollMidRect.bottom) - TEXT_BOTTOM_MARGIN)
			break;
	}

	(*pDC).SelectObject(pFontOld);

	dwHelpPagePosition[nPage + 1] += i + nCropped;
	dwHelpPageEOL[nPage + 1] = bEOL;

	while (true) {
		if (i >= (int) nCount) {
			if (bEOF)
				break;
			nCount = (*pHelpFile).Read(&chInBuf, TEXT_BUFFER_SIZE);
			if (nCount < TEXT_BUFFER_SIZE)
				bEOF = true;
			i = 0;
			n = 1;
		}

		if (bEOL &&
		        ((chInBuf[i] == '\r') ||
		         (chInBuf[i] == '\n') ||
		         (chInBuf[i] == TEXT_NEWLINE))) {
			i += 1;
			dwHelpPagePosition[nHelpPage + 1] += 1;
		} else
			break;
	}

	if (bEOF && (i >= (int) nCount))
		bHelpEOF = true;

	(*pDC).SelectObject(pFontOld);

}


void CRules::UpdateMore(CDC *pDC) {
	int         x, y, dx, dy;
	CFont       *pFontOld = nullptr;
	TEXTMETRIC  fontMetrics;
	CSize       textInfo;

	pFontOld = (*pDC).SelectObject(pFont);

	(*pDC).GetTextMetrics(&fontMetrics);

	(*pDC).SetBkMode(TRANSPARENT);

	x = ScrollRect.right - TEXT_MORE_DX;
	y = ((ScrollTopRect.bottom - fontMetrics.tmHeight) >> 1) +
	    TEXT_MORE_DY - 3;
	textInfo = (*pDC).GetTextExtent(MORE_TEXT_BLURB, MORE_TEXT_LENGTH);
	dx = textInfo.cx;
	dy = fontMetrics.tmHeight;

	if (nHelpPage == 0)
		(*pDC).BitBlt(
		    x,
		    y,
		    dx,
		    dy,
		    pScrollDC,
		    x,
		    y,
		    SRCCOPY);
	else
		(*pDC).TextOut(x, y, MORE_TEXT_BLURB, MORE_TEXT_LENGTH);

	y = ScrollRect.bottom -
	    ScrollBotRect.bottom +
	    ((ScrollBotRect.bottom - fontMetrics.tmHeight) >> 1) -
	    TEXT_MORE_DY;

	if (bHelpEOF)
		(*pDC).BitBlt(
		    x,
		    y,
		    dx,
		    dy,
		    pScrollDC,
		    x,
		    y,
		    SRCCOPY);
	else
		(*pDC).TextOut(x, y, MORE_TEXT_BLURB, MORE_TEXT_LENGTH);

	(*pDC).SelectObject(pFontOld);
}


void CRules::OnShowWindow(bool bShow, unsigned int nStatus) {
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here

}


void CRules::OnSize(unsigned int nType, int cx, int cy) {
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

}


int CRules::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	bool    bSuccess;
	int     xDIB, yDIB, dxDIB, dyDIB;

	pHelpFile = new CFile();
	ASSERT(pHelpFile != nullptr);
	bSuccess = (*pHelpFile).Open(chPathName, CFile::modeRead | CFile::typeBinary, nullptr);
	ASSERT(bSuccess);
	if (!bSuccess)
		return -1;
	nHelpFileSize = (*pHelpFile).GetLength();

	AddFontResource("msserif.fon");
	pFont = new CFont();
	ASSERT(pFont != nullptr);
	bSuccess = (*pFont).CreateFont(16, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FF_ROMAN, "MS Sans Serif");
	ASSERT(bSuccess);

	if ((GetFreeSpace(0) >= (unsigned long) 1000000) &&
	        (GlobalCompact((unsigned long) 1000000) >= (unsigned long) 750000)) {
		pScrollTopDIB = LoadScrollDIB(SCROLL_TOP_SPEC, &ScrollTopRect);
		pScrollMidDIB = LoadScrollDIB(SCROLL_MID_SPEC, &ScrollMidRect);
		pScrollBotDIB = LoadScrollDIB(SCROLL_BOT_SPEC, &ScrollBotRect);
		if ((pScrollTopDIB == nullptr) ||
		        (pScrollMidDIB == nullptr) ||
		        (pScrollBotDIB == nullptr))
			bBruteForce = true;
		else
			bBruteForce = false;
		if (bBruteForce) {
			if (pScrollTopDIB != nullptr)
				delete pScrollTopDIB;
			if (pScrollMidDIB != nullptr)
				delete pScrollMidDIB;
			if (pScrollBotDIB != nullptr)
				delete pScrollBotDIB;
			pScrollTopDIB = nullptr;
			pScrollMidDIB = nullptr;
			pScrollBotDIB = nullptr;
		}
	} else
		bBruteForce = true;

	if (bBruteForce) {
		dxDIB = 501;
		dyDIB = 395;
		ScrollTopRect.SetRect(0, 0, 501, 48);
		ScrollMidRect.SetRect(0, 0, 501, 50);
		ScrollBotRect.SetRect(0, 0, 501, 47);
	} else {
		dxDIB = ScrollTopRect.right;
		dyDIB = ScrollTopRect.bottom +
		        (ScrollMidRect.bottom * SCROLL_PIECES) +
		        ScrollBotRect.bottom;
	}

	xDIB = (GAME_WIDTH - dxDIB) >> 1;
	yDIB = (GAME_HEIGHT - dyDIB) >> 1;

	ScrollRect.SetRect(xDIB, yDIB, dxDIB, dyDIB);
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}


bool CRules::CreateWorkAreas(CDC *pDC) {
	bool    bSuccess;
	CRect   WorkRect;

	pScrollTopBitmap = CreateScrollBitmap(pDC, pScrollTopDIB, pScrollPalette);
	delete pScrollTopDIB;
	pScrollMidBitmap = CreateScrollBitmap(pDC, pScrollMidDIB, pScrollPalette);
	delete pScrollMidDIB;
	pScrollBotBitmap = CreateScrollBitmap(pDC, pScrollBotDIB, pScrollPalette);
	delete pScrollBotDIB;
	if ((pScrollTopBitmap == nullptr) ||
	        (pScrollMidBitmap == nullptr) ||
	        (pScrollBotBitmap == nullptr))
		return false;

	pScrollTopDC = SetupCompatibleContext(pDC, pScrollTopBitmap, pScrollTopBitmapOld, pScrollPalette, pScrollTopPalOld);
	pScrollMidDC = SetupCompatibleContext(pDC, pScrollMidBitmap, pScrollMidBitmapOld, pScrollPalette, pScrollMidPalOld);
	pScrollBotDC = SetupCompatibleContext(pDC, pScrollBotBitmap, pScrollBotBitmapOld, pScrollPalette, pScrollBotPalOld);
	if ((pScrollTopDC == nullptr) ||
	        (pScrollMidDC == nullptr) ||
	        (pScrollBotDC == nullptr))
		return false;

	pScrollBitmap = new CBitmap();
	if (pScrollBitmap == nullptr)
		return false;
	bSuccess = (*pScrollBitmap).CreateCompatibleBitmap(pDC, ScrollRect.right, ScrollRect.bottom);
	if (!bSuccess)
		return false;
	pScrollDC = SetupCompatibleContext(pDC, pScrollBitmap, pScrollBitmapOld, pScrollPalette, pScrollPalOld);
	if (pScrollDC == nullptr)
		return false;

	pBackgroundBitmap = new CBitmap();
	if (pBackgroundBitmap == nullptr)
		return false;
	bSuccess = (*pBackgroundBitmap).CreateCompatibleBitmap(pDC, ScrollRect.right, ScrollRect.bottom);
	if (!bSuccess)
		return false;
	pBackgroundDC = SetupCompatibleContext(pDC, pBackgroundBitmap, pBackgroundBitmapOld, pScrollPalette, pBackgroundPalOld);
	if (pBackgroundDC == nullptr)
		return false;

	WorkRect.UnionRect(&ScrollTopRect, &ScrollMidRect);
	WorkRect.UnionRect(&WorkRect, &ScrollBotRect);
	WorkRect.bottom += SCROLL_STRIP_WIDTH;

	pWorkBitmap = new CBitmap();
	if (pWorkBitmap == nullptr)
		return false;
	bSuccess = (*pWorkBitmap).CreateCompatibleBitmap(pDC, WorkRect.right, WorkRect.bottom);
	if (!bSuccess)
		return false;
	pWorkDC = SetupCompatibleContext(pDC, pWorkBitmap, pWorkBitmapOld, pScrollPalette, pWorkPalOld);
	if (pWorkDC == nullptr)
		return false;

	pScrollTopMask = new CBitmap();
	pScrollMidMask = new CBitmap();
	pScrollBotMask = new CBitmap();
	if ((pScrollTopMask == nullptr) ||
	        (pScrollMidMask == nullptr) ||
	        (pScrollBotMask == nullptr))
		return false;
	pScrollTopMaskDC = SetupMask(pDC, pScrollTopDC, pScrollTopMask, pScrollTopMaskOld, &ScrollTopRect);
	pScrollMidMaskDC = SetupMask(pDC, pScrollMidDC, pScrollMidMask, pScrollMidMaskOld, &ScrollMidRect);
	pScrollBotMaskDC = SetupMask(pDC, pScrollBotDC, pScrollBotMask, pScrollBotMaskOld, &ScrollBotRect);
	if ((pScrollTopMaskDC == nullptr) ||
	        (pScrollMidMaskDC == nullptr) ||
	        (pScrollBotMaskDC == nullptr))
		return false;

	return true;
}


CDC *CRules::SetupMask(CDC *pDC, CDC *pBitmapDC, CBitmap *pMask, CBitmap * &pMaskOld, CRect *pRect) {
	CDC *pNewDC = nullptr;

	(*pMask).CreateBitmap((*pRect).right, (*pRect).bottom, 1, 1, nullptr);

	pNewDC = new CDC();

	if ((pNewDC != nullptr) &&
	        (*pNewDC).CreateCompatibleDC(pDC)) {
		pMaskOld = (*pNewDC).SelectObject(pMask);
		if (pMaskOld != nullptr) {
			(*pNewDC).BitBlt(
			    0, 0,
			    (*pRect).right,
			    (*pRect).bottom,
			    pBitmapDC,
			    0, 0,
			    SRCCOPY);
			return (pNewDC);
		}
	}

	return nullptr;
}


CDC *CRules::SetupCompatibleContext(CDC *pDC, CBitmap *pBitmap, CBitmap * &pBitmapOld, CPalette *pPalette, CPalette * &pPalOld) {
	CDC *pNewDC = nullptr;

	pNewDC = new CDC();

	if ((pDC != nullptr) &&
	        (pNewDC != nullptr) &&
	        (*pNewDC).CreateCompatibleDC(pDC)) {
		if (pPalette)
			pPalOld = (*pNewDC).SelectPalette(pPalette, false);
		else
			pPalOld = nullptr;
		(*pNewDC).RealizePalette();
		pBitmapOld = (*pNewDC).SelectObject(pBitmap);
		if (pBitmapOld != nullptr)
			return (pNewDC);
	}

	return nullptr;
}


void CRules::ReleaseCompatibleContext(CDC *&pDC, CBitmap * &pBitmap, CBitmap *pBitmapOld, CPalette *pPalOld) {
	if (pBitmapOld != nullptr) {
		(*pDC).SelectObject(pBitmapOld);
		pBitmapOld = nullptr;
	}

	if (pPalOld != nullptr) {
		(*pDC).SelectPalette(pPalOld, false);
		pPalOld = nullptr;
	}

	if (pBitmap != nullptr) {
		(*pBitmap).DeleteObject();
		delete pBitmap;
		pBitmap = nullptr;
	}

	if (pDC != nullptr) {
		(*pDC).DeleteDC();
		delete pDC;
		pDC = nullptr;
	}
}


CDibDoc *CRules::LoadScrollDIB(const char *pszPathName, CRect *pRect) {
	bool        bSuccess;
	HDIB        hDIB;
	int         dxDIB, dyDIB;
	CDibDoc     *pDibDoc = nullptr;

	pDibDoc = new CDibDoc();
	ASSERT(pDibDoc != nullptr);

	bSuccess = (*pDibDoc).OpenDocument(pszPathName);
	ASSERT(bSuccess);
	hDIB = (*pDibDoc).GetHDIB();
	ASSERT(hDIB != nullptr);
	
	dxDIB = (int) DIBWidth(hDIB);
	dyDIB = (int) DIBHeight(hDIB);
	

	if (pRect != nullptr)
		(*pRect).SetRect(0, 0, dxDIB, dyDIB);

	return (pDibDoc);
}


bool CRules::PaintScrollDIB(CDC *pDC, CDibDoc *pDibDoc) {
	bool        bSuccess;
	HDIB        hDIB;
	int         dxDIB, dyDIB;
	CRect       myRect;

	hDIB = (*pDibDoc).GetHDIB();
	ASSERT(hDIB != nullptr);
	
	dxDIB = (int) DIBWidth(hDIB);
	dyDIB = (int) DIBHeight(hDIB);
	

	myRect.SetRect(0, 0, dxDIB, dyDIB);

	bSuccess = PaintDIB((*pDC).m_hDC, myRect, hDIB, myRect, pScrollPalette);

	return bSuccess;
}


CBitmap *CRules::CreateScrollBitmap(CDC *pDC, CDibDoc *pDibDoc, CPalette *pPalette) {
	HDIB        hDIB;
	CBitmap     *pBitmap = nullptr;

	hDIB = (*pDibDoc).GetHDIB();
	ASSERT(hDIB != nullptr);

	pBitmap = ConvertDIB(pDC, hDIB, pPalette);

	return pBitmap;
}


void CRules::OnLButtonDown(unsigned int nFlags, CPoint point) {
	if (!bActiveWindow)
		return;

	if (ScrollTopCurlRect.PtInRect(point) && (nHelpPage != 0))
		UpdateScroll(nHelpPage - 1);
	else if (ScrollBotCurlRect.PtInRect(point) && !bHelpEOF)
		UpdateScroll(nHelpPage + 1);

	OnMouseMove(nFlags, point);

	CDialog::OnLButtonDown(nFlags, point);
}


void CRules::OnMouseMove(unsigned int nFlags, CPoint point) {
	HCURSOR hNewCursor = nullptr;
	CWinApp *pMyApp;

	if (!bActiveWindow)
		return;

	pMyApp = AfxGetApp();

	if (OkayRect.PtInRect(point))
		hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);
	else if (ScrollTopCurlRect.PtInRect(point)) {
		if (nHelpPage == 0)
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_INVALID);
		else
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_ARROWUP);
	} else if (ScrollBotCurlRect.PtInRect(point)) {
		if (bHelpEOF)
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_INVALID);
		else
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_ARROWDN);
	} else
		hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);

	//if (hNewCursor != nullptr);
	MFC::SetCursor(hNewCursor);

	CDialog::OnMouseMove(nFlags, point);
}


bool CRules::OnSetCursor(CWnd *pWnd, unsigned int nHitTest, unsigned int message) {
	if ((*pWnd).m_hWnd == (*this).m_hWnd)
		return true;
	else
		return false;
}


void CRules::DoWaitCursor() {
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();

	(*pMyApp).BeginWaitCursor();
}


void CRules::DoArrowCursor() {
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();

	(*pMyApp).EndWaitCursor();
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
