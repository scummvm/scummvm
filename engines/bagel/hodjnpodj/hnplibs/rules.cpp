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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/boflib/sound.h"
//

namespace Bagel {
namespace HodjNPodj {

#ifdef _WINDLL
	#define BUILD_FOR_DLL       TRUE
#else
	#define BUILD_FOR_DLL       FALSE
#endif

#define COLOR_BUTTONS       TRUE

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


#if BUILD_FOR_DLL

extern "C" {
	LRESULT FAR PASCAL KeyboardHookProc(int, WORD, LONG);   // keyboard hook procedure definition
}
typedef LRESULT(FAR PASCAL *FPKBDHOOKPROC)(int, WORD, LONG);
extern  HINSTANCE   hDLLInst;
extern  HINSTANCE   hExeInst;
static  FPKBDHOOKPROC   lpfnKbdHook = NULL;         // pointer to hook procedure

#else

LRESULT FAR PASCAL PrefHookProc(int, WORD, LONG);   // keyboard hook procedure definition
static  FARPROC     pKbdHook = NULL;                // pointer to hook procedure

#endif

static  HHOOK       hKbdHook = NULL;                // handle for hook procedure

static  CRules      *pRulesDialog = NULL;           // pointer to our rules dialog box
static  CWnd        *pParentWnd = NULL;             // parent window pointer

static  CColorButton *pOKButton = NULL;             // OKAY button on scroll
static  CRect       OkayRect;                       // rectangle bounding the OKAY button

static  CDibDoc     *pScrollTopDIB = NULL,          // DIB for scroll top section
                     *pScrollMidDIB = NULL,          // DIB for scroll mid section
                      *pScrollBotDIB = NULL;          // DIB for scroll bottom section

static  CBitmap     *pScrollTopBitmap = NULL,       // bitmap for scroll top section
                     *pScrollTopBitmapOld = NULL,    // bitmap previously mapped to top section context
                      *pScrollMidBitmap = NULL,       // bitmap for scroll mid section
                       *pScrollMidBitmapOld = NULL,    // bitmap previously mapped to mid section context
                        *pScrollBotBitmap = NULL,       // bitmap for scroll bottom section
                         *pScrollBotBitmapOld = NULL;    // bitmap previously mapped to bottom section context

static  CBitmap     *pScrollTopMask = NULL,         // mask for scroll top section bitmap
                     *pScrollTopMaskOld = NULL,      // bitmap previously mapped to top mask context
                      *pScrollMidMask = NULL,         // mask for scroll mid section bitmap
                       *pScrollMidMaskOld = NULL,      // bitmap previously mapped to mid mask context
                        *pScrollBotMask = NULL,         // mask for scroll bottom section bitmap
                         *pScrollBotMaskOld = NULL;      // bitmap previously mapped to bottom mask context

static  CBitmap     *pScrollBitmap = NULL,          // bitmap for an entirely blank scroll
                     *pScrollBitmapOld = NULL,       // bitmap previously mapped to the scroll context
                      *pBackgroundBitmap = NULL,      // bitmap containing the background for the scroll
                       *pBackgroundBitmapOld = NULL,   // bitmap previously mapped to the background context
                        *pWorkBitmap = NULL,            // bitmap containing the work area for the scroll
                         *pWorkBitmapOld = NULL;         // bitmap previously mapped to the work area context

static  CRect       ScrollRect,                     // x/y (left/right) and dx/dy (right/bottom) for the scroll window
        ScrollTopRect,                  // rectangle bounding the scroll top section
        ScrollBotRect,                  // rectangle bounding the scroll bottom section
        ScrollMidRect;                  // rectangle bounding the scroll middle section

static  CRect       ScrollTopCurlRect,              // current location of top curl for mouse clicks
        ScrollBotCurlRect;              // current location of bottom curl for mouse clicks

static  CPalette    *pScrollPalette = NULL,         // palette used for the scroll
                     *pScrollPalOld = NULL,          // previous palette mapped to scroll context
                      *pBackgroundPalOld = NULL,      // previous palette mapped to background context
                       *pScrollTopPalOld = NULL,       // previous palette mapped to top context
                        *pScrollMidPalOld = NULL,       // previous palette mapped to middle context
                         *pScrollBotPalOld = NULL,       // previous palette mapped to bottom context
                          *pScrollTopMaskPalOld = NULL,   // previous palette mapped to top mask context
                           *pScrollMidMaskPalOld = NULL,   // previous palette mapped to middle mask context
                            *pScrollBotMaskPalOld = NULL,   // previous palette mapped to bottom mask context
                             *pWorkPalOld = NULL;            // previous palette mapped to work area context

static  CDC         *pScrollDC = NULL,              // device context for the scroll bitmap
                     *pBackgroundDC = NULL,          // device context for the background bitmap
                      *pScrollTopDC = NULL,           // device context for the top section bitmap
                       *pScrollMidDC = NULL,           // device context for the middle section bitmap
                        *pScrollBotDC = NULL,           // device context for the bottom section bitmap
                         *pScrollTopMaskDC = NULL,       // device context for the top mask bitmap
                          *pScrollMidMaskDC = NULL,       // device context for the middle section bitmap
                           *pScrollBotMaskDC = NULL,       // device context for the bottom section bitmap
                            *pWorkDC = NULL;                // device context for the work area bitmap

static  CFont       *pFont = NULL;                  // font to use for displaying rules text
static  char        chPathName[128];                // buffer to hold path name of the rules file
static  CFile       *pHelpFile = NULL;              // the rules file
static  DWORD       nHelpFileSize = 0;              // size of rules file
static  int         nHelpPage = 0;                  // current page of rules text
static  DWORD       dwHelpPagePosition[100];        // position of each page (# chars from file start)
static  BOOL        dwHelpPageEOL[100];             // whether page starts with enforced line break
static  BOOL        bHelpEOF = FALSE;               // whether end-of-file has been reached
static  int         tabstop = 20 + TEXT_LEFT_MARGIN;// pixels per tab stop

static  const char  *pSoundPath = NULL;             // path spec for rules narration
static  CSound      *pNarrative = NULL;             // sound object

static  BOOL        first_time = TRUE;
static  BOOL        bActiveWindow = FALSE;          // whether our window is active
static  BOOL        bBruteForce = FALSE;            // whether we can be clever


/////////////////////////////////////////////////////////////////////////////
// CRules dialog



BOOL CRules::SetupKeyboardHook(void) {
	#if BUILD_FOR_DLL
	pRulesDialog = this;                            // retain pointer to our dialog box

	lpfnKbdHook = (FPKBDHOOKPROC)::GetProcAddress(hDLLInst, "KeyboardHookProc");
	if (lpfnKbdHook == NULL)                           // setup pointer to our procedure
		return (FALSE);

	hKbdHook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC) lpfnKbdHook, hExeInst, GetCurrentTask());
	#else
	HINSTANCE   hInst;

	pRulesDialog = this;                            // retain pointer to our dialog box

	hInst = AfxGetInstanceHandle();                 // get our application instance

	pKbdHook = MakeProcInstance((FARPROC) PrefHookProc, hInst);
	if (pKbdHook == NULL)                           // setup pointer to our procedure
		return (FALSE);

	hKbdHook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC) pKbdHook, hInst, GetCurrentTask());
	#endif

	if (hKbdHook == NULL)                           // plug in our keyboard hook
		return (FALSE);

	return (TRUE);
}


void CRules::RemoveKeyboardHook(void) {
	#if BUILD_FOR_DLL
	if (m_bKeyboardHook)
		UnhookWindowsHookEx(hKbdHook);                  // unhook our keyboard procedure

	lpfnKbdHook = NULL;
	#else
	if (m_bKeyboardHook) {
		UnhookWindowsHookEx(hKbdHook);                  // unhook our keyboard procedure
		FreeProcInstance(pKbdHook);                     // release our procedure pointer
	}

	pKbdHook = NULL;
	#endif

	pRulesDialog = NULL;
	hKbdHook = NULL;
	m_bKeyboardHook = FALSE;
}


#if BUILD_FOR_DLL
extern "C"
LRESULT FAR PASCAL KeyboardHookProc(int code, WORD wParam, LONG lParam) {
	if (code < 0)                                   // required to punt to next hook
		return (CallNextHookEx((HHOOK) lpfnKbdHook, code, wParam, lParam));
#else
LRESULT FAR PASCAL PrefHookProc(int code, WORD wParam, LONG lParam) {
	if (code < 0)                                   // required to punt to next hook
		return (CallNextHookEx((HHOOK) pKbdHook, code, wParam, lParam));
#endif

	if (lParam & 0xA0000000)                        // ignore ALT and key release
		return (FALSE);

	if (bActiveWindow)
		switch (wParam) {                               // process only the keys we are looking for
		case VK_UP:                                 // ... letting the default dialog procedure
		case VK_NUMPAD8:                            // ... deal with all the rest
		case VK_PRIOR:                              // go to previous page of text
			if (nHelpPage > 0)
				(*pRulesDialog).UpdateScroll(nHelpPage - 1);
			return (TRUE);
		case VK_DOWN:                               // go to next page of text
		case VK_NUMPAD2:
		case VK_NEXT:
			if (!bHelpEOF)
				(*pRulesDialog).UpdateScroll(nHelpPage + 1);
			return (TRUE);
		case VK_HOME:                               // go to first page of text
			if (nHelpPage > 0)
				(*pRulesDialog).UpdateScroll(0);
			return (TRUE);
		case VK_END:                                // go to last page of text
			while (!bHelpEOF) {
				(*pRulesDialog).UpdateScroll(nHelpPage + 1);
			}
			return (TRUE);
		}

	return (FALSE);
}


CRules::CRules(CWnd *pParent, const char *pszPathName,
               CPalette *pPalette, const char *pszSoundPath)
	: CDialog(CRules::IDD, pParent) {
	Common::strcpy_s(chPathName, pszPathName);                  // retain path to rules file on disk
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


void CRules::OnCancel(void) {
	ClearDialogImage();
	CDialog::OnCancel();
}


void CRules::OnDestroy() {
	BOOL    bUpdateNeeded;
	HCURSOR hNewCursor = NULL;
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();
	hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);
	//if (hNewCursor != NULL);
	SetCursor(hNewCursor);

	if (pNarrative != NULL)                         // end the narration
		delete pNarrative;

	if (pOKButton != NULL)                          // release the button
		delete pOKButton;

	delete pFont;                                   // release the font file

	(*pHelpFile).Close();                           // close and release the rules file
	delete pHelpFile;

	if (pBackgroundBitmap != NULL) {
		bUpdateNeeded = (*pParentWnd).GetUpdateRect(NULL, FALSE);
		if (bUpdateNeeded)
			(*pParentWnd).ValidateRect(NULL);
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


BOOL CRules::OnInitDialog() {
	CWnd    *pButton;                                   // pointer to the OKAY button
	CRect   myRect;                                     // rectangle that holds the button location
	int     x, y, dx, dy;                               // used for calculating positioning info
	BOOL    bSuccess;

	CDialog::OnInitDialog();                        // do basic dialog initialization

	if (pParentWnd == NULL)                         // get our parent window
		pParentWnd = ((CWnd *) this)->GetParent();  // ... as passed to us or inquired about

	(*pParentWnd).GetWindowRect(&myRect);
	x = myRect.left + (((myRect.right - myRect.left) - ScrollRect.right) >> 1);
	y = myRect.top + (((myRect.bottom - myRect.top) - ScrollRect.bottom) >> 1);
	MoveWindow(x, y, ScrollRect.right, ScrollRect.bottom);      // center the dialog box on the parent

	pButton = GetDlgItem((int) GetDefID());         // get the window for the okay button
	ASSERT(pButton != NULL);                        // ... and verify we have it
	(*pButton).GetWindowRect(&myRect);              // get the button's position and size

	dx = myRect.right - myRect.left;                // calculate where to place the button
	x = (ScrollRect.right - dx) >> 1;               // ... centered at the bottom edge
	dy = myRect.bottom - myRect.top;
	y = ScrollRect.bottom - dy - BUTTON_DY;

	(*pButton).MoveWindow(x, y, dx, dy);            // reposition the button
	OkayRect.SetRect(x, y, x + dx, y + dy);

	#if COLOR_BUTTONS
	pOKButton = new CColorButton;                   // build a color QUIT button to let us exit
	ASSERT(pOKButton != NULL);
	(*pOKButton).SetPalette(pScrollPalette);        // set the palette to use
	bSuccess = (*pOKButton).SetControl((int) GetDefID(), this); // tie to the dialog control
	ASSERT(bSuccess);
	#endif

	if (pSoundPath != NULL)
		pNarrative = new CSound(this, pSoundPath, SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);

	m_bKeyboardHook = SetupKeyboardHook();          // establish keyboard hook

	first_time = TRUE;

	return (TRUE);                                  // return TRUE  unless focused on a control
}


void CRules::OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized) {
	BOOL    bUpdateNeeded;

	switch (nState) {
	case WA_INACTIVE:
		bActiveWindow = FALSE;
		break;
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		bActiveWindow = TRUE;
		bUpdateNeeded = GetUpdateRect(NULL, FALSE);
		if (bUpdateNeeded)
			InvalidateRect(NULL, FALSE);
	}
}


void CRules::OnPaint() {
	CPaintDC    dc(this);                               // device context for painting
	CPalette    *pPalOld = NULL;
	CDibDoc     *pDibDoc;

	DoWaitCursor();                                 // put up the hourglass cursor

	if (pScrollPalette != NULL) {                   // map in our palette
		pPalOld = dc.SelectPalette(pScrollPalette, FALSE);
		(void) dc.RealizePalette();                 // .. and make the system use it
	}

	if (first_time) {                               // unfurl scroll visually
		first_time = FALSE;                         // ... but only if this is the first time
		UnfurlScroll(&dc);                          // ... we are updating the screen
		ReleaseCompatibleContext(pScrollTopDC, pScrollTopBitmap, pScrollTopBitmapOld, pScrollTopPalOld);
		ReleaseCompatibleContext(pScrollBotDC, pScrollBotBitmap, pScrollBotBitmapOld, pScrollBotPalOld);
		ReleaseCompatibleContext(pScrollTopMaskDC, pScrollTopMask, pScrollTopMaskOld, pScrollTopMaskPalOld);
		ReleaseCompatibleContext(pScrollBotMaskDC, pScrollBotMask, pScrollBotMaskOld, pScrollBotMaskPalOld);
		if (pNarrative != NULL)
			(*pNarrative).play();                   // play the narration
	} else if (bBruteForce) {                           // need to paint directly to screen
		pDibDoc = LoadScrollDIB(SCROLL_SPEC, NULL);
		if (pDibDoc != NULL) {
			PaintScrollDIB(&dc, pDibDoc);
			delete pDibDoc;
		}
		WritePage(&dc, nHelpPage);
		UpdateMore(&dc);                            // update the "more" indicator
	} else
		(void) dc.BitBlt(        // update the screen with just the
		    0, 0,                           // ... current page of text
		    ScrollRect.right,
		    ScrollRect.bottom,
		    pScrollDC,
		    0, 0,
		    SRCCOPY);

	if (pScrollPalette != NULL)                     // map out our palette
		(void) dc.SelectPalette(pPalOld, FALSE);

	DoArrowCursor();                                // return to an arrow cursor
}


BOOL CRules::OnEraseBkgnd(CDC *pDC) {
	return (TRUE);                                      // do not automatically erase background to white
}


void CRules::ClearDialogImage(void) {
	delete pOKButton;
	pOKButton = NULL;
	if (pBackgroundBitmap != NULL)
		ValidateRect(NULL);
	RefreshBackground();
}


void CRules::RefreshBackground(void) {
	CDC         *pDC;
	CPalette    *pPalOld = NULL;

	if (pBackgroundBitmap == NULL)
		return;

	pDC = GetDC();                                  // get a context for our window

	if (pScrollPalette != NULL) {                   // map in our palette
		pPalOld = (*pDC).SelectPalette(pScrollPalette, FALSE);
		(void)(*pDC).RealizePalette();              // .. and make the system use it
	}

	(void)(*pDC).BitBlt(        // repaint the background as it was
	    0,
	    0,
	    ScrollRect.right,
	    ScrollRect.bottom,
	    pBackgroundDC,
	    0,
	    0,
	    SRCCOPY);

	if (pScrollPalette != NULL)                     // map out our palette
		(void)(*pDC).SelectPalette(pPalOld, FALSE);

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
		pDibDoc = LoadScrollDIB(SCROLL_SPEC, NULL);
		if (pDibDoc != NULL) {
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

	(void)(*pBackgroundDC).BitBlt(      // save the entire background
	    0, 0,
	    ScrollRect.right,
	    ScrollRect.bottom,
	    pDC,
	    0, 0,
	    SRCCOPY);

	(void)(*pScrollDC).BitBlt(       // copy background to scroll context
	    0, 0,                           // ... so we can construct the scroll
	    ScrollRect.right,               // ... on top of it in a masked manner
	    ScrollRect.bottom,
	    pDC,
	    0, 0,
	    SRCCOPY);

// setup the top part of the scroll

	ScrollTopCurlRect = ScrollTopRect;              // record where we're putting the top curl

	(void)(*pScrollDC).BitBlt(       // mask out where the top curl goes
	    0, 0,
	    ScrollTopRect.right,
	    ScrollTopRect.bottom,
	    pScrollTopMaskDC,
	    0, 0,
	    SRCAND);

	(void)(*pScrollTopMaskDC).BitBlt(     // invert the top curl mask
	    0, 0,
	    ScrollTopRect.right,
	    ScrollTopRect.bottom,
	    NULL,
	    0, 0,
	    DSTINVERT);

	(void)(*pScrollTopDC).BitBlt(      // remove transparent area from top curl
	    0, 0,
	    ScrollTopRect.right,
	    ScrollTopRect.bottom,
	    pScrollTopMaskDC,
	    0, 0,
	    SRCAND);

	(void)(*pScrollTopMaskDC).BitBlt(     // restore top curl mask
	    0, 0,
	    ScrollTopRect.right,
	    ScrollTopRect.bottom,
	    NULL,
	    0, 0,
	    DSTINVERT);

	(void)(*pScrollDC).BitBlt(       // paint top curl into the scroll
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

	(void)(*pScrollMidMaskDC).BitBlt(     // invert the mid section mask
	    0, 0,
	    ScrollMidRect.right,
	    ScrollMidRect.bottom,
	    NULL,
	    0, 0,
	    DSTINVERT);

	(void)(*pScrollMidDC).BitBlt(      // mask out transparent part of mid section
	    0, 0,
	    ScrollMidRect.right,
	    ScrollMidRect.bottom,
	    pScrollMidMaskDC,
	    0, 0,
	    SRCAND);

	(void)(*pScrollMidMaskDC).BitBlt(     // reset the mid section mask
	    0, 0,
	    ScrollMidRect.right,
	    ScrollMidRect.bottom,
	    NULL,
	    0, 0,
	    DSTINVERT);

	for (i = 0; i < SCROLL_PIECES; i++) {           // build the scroll's mid section srip-wise
		(void)(*pScrollDC).BitBlt(
		    dstRect.left,               // mask out where the image will go
		    dstRect.top + ScrollMidRect.bottom * i,
		    ScrollMidRect.right,
		    ScrollMidRect.bottom,
		    pScrollMidMaskDC,
		    0, 0,
		    SRCAND);

		(void)(*pScrollDC).BitBlt(      // paint in the mid section image
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

	(void)(*pScrollDC).BitBlt(       // mask out where the bottom curl image goes
	    dstRect.left,
	    dstRect.top,
	    ScrollBotRect.right,
	    ScrollBotRect.bottom,
	    pScrollBotMaskDC,
	    0, 0,
	    SRCAND);

	(void)(*pScrollBotMaskDC).BitBlt(     // invert the bottom curl mask
	    0, 0,
	    ScrollBotRect.right,
	    ScrollBotRect.bottom,
	    NULL,
	    0, 0,
	    DSTINVERT);

	(void)(*pScrollBotDC).BitBlt(      // mask out transparent part of curl
	    0, 0,
	    ScrollBotRect.right,
	    ScrollBotRect.bottom,
	    pScrollBotMaskDC,
	    0, 0,
	    SRCAND);

	(void)(*pScrollBotMaskDC).BitBlt(     // reset the mask
	    0, 0,
	    ScrollBotRect.right,
	    ScrollBotRect.bottom,
	    NULL,
	    0, 0,
	    DSTINVERT);

	(void)(*pScrollDC).BitBlt(       // paint in the bottom curl
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

		(void)(*pWorkDC).BitBlt(      // splat in the background for top curl
		    0, 0,
		    ScrollTopRect.right,
		    ScrollTopRect.bottom,
		    pBackgroundDC,
		    dst2Rect.left,
		    dst2Rect.top,
		    SRCCOPY);

		(void)(*pWorkDC).BitBlt(      // mask out where the top curl goes
		    0, 0,
		    ScrollTopRect.right,
		    ScrollTopRect.bottom,
		    pScrollTopMaskDC,
		    0, 0,
		    SRCAND);

		(void)(*pWorkDC).BitBlt(      // insert the top curl background
		    0, 0,
		    ScrollTopRect.right,
		    ScrollTopRect.bottom,
		    pScrollTopDC,
		    0, 0,
		    SRCPAINT);

		if (i == 0) {                               // if first time, just paint curl to display
			(void)(*pDC).BitBlt(
			    dst2Rect.left,
			    dst2Rect.top,
			    ScrollTopRect.right,
			    ScrollTopRect.bottom,
			    pWorkDC,
			    0, 0,
			    SRCCOPY);
		} else {
			(void)(*pWorkDC).BitBlt(     // ... otherwise paint in a new piece of
			    dst2Rect.left,          // ... the middle section
			    ScrollTopRect.bottom,
			    dst2Rect.right,
			    scroll_delta,
			    pScrollDC,
			    dst2Rect.left,
			    dst2Rect.top + ScrollTopRect.bottom,
			    SRCCOPY);
			(void)(*pDC).BitBlt(      // ... then paint it and the curl, thus
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
			(void)(*pWorkDC).BitBlt(                // ... just paint in the bottom curl
			    0, 0,
			    dst2Rect.right,
			    scroll_delta,
			    pDC,
			    dst2Rect.left,
			    dst2Rect.top,
			    SRCCOPY);
		} else {                                    //
			(void)(*pWorkDC).BitBlt(
			    0, 0,
			    dst2Rect.right,
			    scroll_delta,
			    pScrollDC,
			    dst2Rect.left,
			    dst2Rect.top,
			    SRCCOPY);
		}

		(void)(*pWorkDC).BitBlt(
		    0,
		    scroll_delta,
		    ScrollBotRect.right,
		    ScrollBotRect.bottom,
		    pBackgroundDC,
		    ScrollBotRect.left,
		    dst2Rect.top + scroll_delta,
		    SRCCOPY);

		(void)(*pWorkDC).BitBlt(
		    0,
		    scroll_delta,
		    ScrollBotRect.right,
		    ScrollBotRect.bottom,
		    pScrollBotMaskDC,
		    0, 0,
		    SRCAND);

		(void)(*pWorkDC).BitBlt(
		    0,
		    scroll_delta,
		    ScrollBotRect.right,
		    ScrollBotRect.bottom,
		    pScrollBotDC,
		    0, 0,
		    SRCPAINT);

		(void)(*pDC).BitBlt(
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
	CPalette    *pPalOld = NULL;

	if (bBruteForce) {
		nHelpPage = nPage;
		InvalidateRect(NULL, FALSE);
		return;
	}

	pDC = GetDC();

	if (pScrollPalette != NULL) {                   // map in our palette
		pPalOld = (*pDC).SelectPalette(pScrollPalette, FALSE);
		(void)(*pDC).RealizePalette();              // .. and make the system use it
	}

	dstRect = ScrollMidRect;                        // setup initial destination for blts
	dstRect.top += ScrollTopRect.bottom;
	dstRect.bottom += ScrollTopRect.bottom;

	for (i = 0; i < SCROLL_PIECES; i++) {           // repaint the scroll midsection artwork
		(void)(*pScrollDC).BitBlt(      // ... mask out where artwork goes
		    dstRect.left,
		    dstRect.top + ScrollMidRect.bottom * i,
		    ScrollMidRect.right,
		    ScrollMidRect.bottom,
		    pScrollMidMaskDC,
		    0, 0,
		    SRCAND);

		(void)(*pScrollDC).BitBlt(      // ... paint in the scroll artwork
		    dstRect.left,
		    dstRect.top + ScrollMidRect.bottom * i,
		    ScrollMidRect.right,
		    ScrollMidRect.bottom,
		    pScrollMidDC,
		    0, 0,
		    SRCPAINT);
	}

	WritePage(pScrollDC, nPage);                    // construct the new page of text

	(void)(*pDC).BitBlt(        // paint the result to the sceen
	    TEXT_LEFT_MARGIN,
	    ScrollTopRect.bottom + TEXT_TOP_MARGIN,
	    TEXT_WIDTH,
	    (SCROLL_PIECES * ScrollMidRect.bottom) - TEXT_TOP_MARGIN - 1,
	    pScrollDC,
	    TEXT_LEFT_MARGIN,
	    ScrollTopRect.bottom + TEXT_TOP_MARGIN,
	    SRCCOPY);

	UpdateMore(pDC);                                // update the "more" indicator

	if (pScrollPalette != NULL)                     // map out our palette
		(void)(*pDC).SelectPalette(pPalOld, FALSE);

	ReleaseDC(pDC);
}


void CRules::WritePage(CDC *pDC, int nPage) {
	int         i, n, x, y;
	UINT        nCropped = 0;
	UINT        nCount = 0;
	char        chInBuf[TEXT_BUFFER_SIZE];
	CFont       *pFontOld = NULL;
	CSize       textInfo;
	BOOL        bEOL, bEOF;
	TEXTMETRIC  fontMetrics;

	nHelpPage = nPage;
	bEOF = bHelpEOF = FALSE;

	(*pHelpFile).SeekToBegin();

	if ((nHelpPage == 0) && (nPage == 0)) {
		dwHelpPagePosition[nHelpPage] = (*pHelpFile).GetPosition();
		bEOL = dwHelpPageEOL[nHelpPage] = TRUE;
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

	while (TRUE) {
try_again:
		if (i >= (int) nCount) {
			if (bEOF)
				break;
			dwHelpPagePosition[nHelpPage + 1] += i + nCropped;
			nCount = (*pHelpFile).Read(&chInBuf, TEXT_BUFFER_SIZE);
			if (nCount < TEXT_BUFFER_SIZE)
				bEOF = TRUE;
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

		bEOL = FALSE;

		while (TRUE) {
			if (i + n > (int) nCount) {
				n = nCount - i;
				if (bEOF)
					break;
				dwHelpPagePosition[nHelpPage + 1] += i + nCropped;
				strncpy(&chInBuf[0], &chInBuf[i], n);
				nCount = (*pHelpFile).Read(&chInBuf[n], TEXT_BUFFER_SIZE - n);
				nCount += n;
				if (nCount < TEXT_BUFFER_SIZE)
					bEOF = TRUE;
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
				bEOL = TRUE;
				n -= 1;
				break;
			}

			textInfo = (*pDC).GetTextExtent(&chInBuf[i], n);
			if (textInfo.cx > TEXT_WIDTH - TEXT_LEFT_MARGIN) {
				if ((chInBuf[i + n - 1] != ' ') &&
				        (chInBuf[i + n - 1] != '\t'))
					while (n > 0) {
						n -= 1;
						if (strchr(" \t,;:-", chInBuf[i + n - 1]) != NULL)
							break;
					}
				break;
			}
			n += 1;
		}
//		(*pDC).TextOut(x,y,(LPCSTR) &chInBuf[i],n);
		(*pDC).TabbedTextOut(x, y, (LPCSTR) &chInBuf[i], n, 1, &tabstop, 0);
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

	(void)(*pDC).SelectObject(pFontOld);

	dwHelpPagePosition[nPage + 1] += i + nCropped;
	dwHelpPageEOL[nPage + 1] = bEOL;

	while (TRUE) {
		if (i >= (int) nCount) {
			if (bEOF)
				break;
			nCount = (*pHelpFile).Read(&chInBuf, TEXT_BUFFER_SIZE);
			if (nCount < TEXT_BUFFER_SIZE)
				bEOF = TRUE;
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
		bHelpEOF = TRUE;

	(void)(*pDC).SelectObject(pFontOld);

}


void CRules::UpdateMore(CDC *pDC) {
	int         x, y, dx, dy;
	CFont       *pFontOld = NULL;
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
		(void)(*pDC).BitBlt(
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
		(void)(*pDC).BitBlt(
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

	(void)(*pDC).SelectObject(pFontOld);
}


void CRules::OnShowWindow(BOOL bShow, UINT nStatus) {
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here

}


void CRules::OnSize(UINT nType, int cx, int cy) {
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

}


int CRules::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	BOOL    bSuccess;
	int     xDIB, yDIB, dxDIB, dyDIB;

	pHelpFile = new CFile();
	ASSERT(pHelpFile != NULL);
	bSuccess = (*pHelpFile).Open(chPathName, CFile::modeRead | CFile::typeBinary, NULL);
	ASSERT(bSuccess);
	if (!bSuccess)
		return (-1);
	nHelpFileSize = (*pHelpFile).GetLength();

	AddFontResource("msserif.fon");
	pFont = new CFont();
	ASSERT(pFont != NULL);
	bSuccess = (*pFont).CreateFont(16, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FF_ROMAN, "MS Sans Serif");
	ASSERT(bSuccess);

	if ((GetFreeSpace(0) >= (unsigned long) 1000000) &&
	        (GlobalCompact((unsigned long) 1000000) >= (unsigned long) 750000)) {
		pScrollTopDIB = LoadScrollDIB(SCROLL_TOP_SPEC, &ScrollTopRect);
		pScrollMidDIB = LoadScrollDIB(SCROLL_MID_SPEC, &ScrollMidRect);
		pScrollBotDIB = LoadScrollDIB(SCROLL_BOT_SPEC, &ScrollBotRect);
		if ((pScrollTopDIB == NULL) ||
		        (pScrollMidDIB == NULL) ||
		        (pScrollBotDIB == NULL))
			bBruteForce = TRUE;
		else
			bBruteForce = FALSE;
		if (bBruteForce) {
			if (pScrollTopDIB != NULL)
				delete pScrollTopDIB;
			if (pScrollMidDIB != NULL)
				delete pScrollMidDIB;
			if (pScrollBotDIB != NULL)
				delete pScrollBotDIB;
			pScrollTopDIB = NULL;
			pScrollMidDIB = NULL;
			pScrollBotDIB = NULL;
		}
	} else
		bBruteForce = TRUE;

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
		return (-1);

	return (0);
}


BOOL CRules::CreateWorkAreas(CDC *pDC) {
	BOOL    bSuccess;
	CRect   WorkRect;

	pScrollTopBitmap = CreateScrollBitmap(pDC, pScrollTopDIB, pScrollPalette);
	delete pScrollTopDIB;
	pScrollMidBitmap = CreateScrollBitmap(pDC, pScrollMidDIB, pScrollPalette);
	delete pScrollMidDIB;
	pScrollBotBitmap = CreateScrollBitmap(pDC, pScrollBotDIB, pScrollPalette);
	delete pScrollBotDIB;
	if ((pScrollTopBitmap == NULL) ||
	        (pScrollMidBitmap == NULL) ||
	        (pScrollBotBitmap == NULL))
		return (FALSE);

	pScrollTopDC = SetupCompatibleContext(pDC, pScrollTopBitmap, pScrollTopBitmapOld, pScrollPalette, pScrollTopPalOld);
	pScrollMidDC = SetupCompatibleContext(pDC, pScrollMidBitmap, pScrollMidBitmapOld, pScrollPalette, pScrollMidPalOld);
	pScrollBotDC = SetupCompatibleContext(pDC, pScrollBotBitmap, pScrollBotBitmapOld, pScrollPalette, pScrollBotPalOld);
	if ((pScrollTopDC == NULL) ||
	        (pScrollMidDC == NULL) ||
	        (pScrollBotDC == NULL))
		return (FALSE);

	pScrollBitmap = new CBitmap();
	if (pScrollBitmap == NULL)
		return (FALSE);
	bSuccess = (*pScrollBitmap).CreateCompatibleBitmap(pDC, ScrollRect.right, ScrollRect.bottom);
	if (!bSuccess)
		return (FALSE);
	pScrollDC = SetupCompatibleContext(pDC, pScrollBitmap, pScrollBitmapOld, pScrollPalette, pScrollPalOld);
	if (pScrollDC == NULL)
		return (FALSE);

	pBackgroundBitmap = new CBitmap();
	if (pBackgroundBitmap == NULL)
		return (FALSE);
	bSuccess = (*pBackgroundBitmap).CreateCompatibleBitmap(pDC, ScrollRect.right, ScrollRect.bottom);
	if (!bSuccess)
		return (FALSE);
	pBackgroundDC = SetupCompatibleContext(pDC, pBackgroundBitmap, pBackgroundBitmapOld, pScrollPalette, pBackgroundPalOld);
	if (pBackgroundDC == NULL)
		return (FALSE);

	WorkRect.UnionRect(&ScrollTopRect, &ScrollMidRect);
	WorkRect.UnionRect(&WorkRect, &ScrollBotRect);
	WorkRect.bottom += SCROLL_STRIP_WIDTH;

	pWorkBitmap = new CBitmap();
	if (pWorkBitmap == NULL)
		return (FALSE);
	bSuccess = (*pWorkBitmap).CreateCompatibleBitmap(pDC, WorkRect.right, WorkRect.bottom);
	if (!bSuccess)
		return (FALSE);
	pWorkDC = SetupCompatibleContext(pDC, pWorkBitmap, pWorkBitmapOld, pScrollPalette, pWorkPalOld);
	if (pWorkDC == NULL)
		return (FALSE);

	pScrollTopMask = new CBitmap();
	pScrollMidMask = new CBitmap();
	pScrollBotMask = new CBitmap();
	if ((pScrollTopMask == NULL) ||
	        (pScrollMidMask == NULL) ||
	        (pScrollBotMask == NULL))
		return (FALSE);
	pScrollTopMaskDC = SetupMask(pDC, pScrollTopDC, pScrollTopMask, pScrollTopMaskOld, &ScrollTopRect);
	pScrollMidMaskDC = SetupMask(pDC, pScrollMidDC, pScrollMidMask, pScrollMidMaskOld, &ScrollMidRect);
	pScrollBotMaskDC = SetupMask(pDC, pScrollBotDC, pScrollBotMask, pScrollBotMaskOld, &ScrollBotRect);
	if ((pScrollTopMaskDC == NULL) ||
	        (pScrollMidMaskDC == NULL) ||
	        (pScrollBotMaskDC == NULL))
		return (FALSE);

	return (TRUE);
}


CDC *CRules::SetupMask(CDC *pDC, CDC *pBitmapDC, CBitmap *pMask, CBitmap * &pMaskOld, CRect *pRect) {
	CDC *pNewDC = NULL;

	(*pMask).CreateBitmap((*pRect).right, (*pRect).bottom, 1, 1, NULL);

	pNewDC = new CDC();

	if ((pNewDC != NULL) &&
	        (*pNewDC).CreateCompatibleDC(pDC)) {
		pMaskOld = (*pNewDC).SelectObject(pMask);
		if (pMaskOld != NULL) {
			(void)(*pNewDC).BitBlt(
			    0, 0,
			    (*pRect).right,
			    (*pRect).bottom,
			    pBitmapDC,
			    0, 0,
			    SRCCOPY);
			return (pNewDC);
		}
	}

	return (NULL);
}


CDC *CRules::SetupCompatibleContext(CDC *pDC, CBitmap *pBitmap, CBitmap * &pBitmapOld, CPalette *pPalette, CPalette * &pPalOld) {
	CDC *pNewDC = NULL;

	pNewDC = new CDC();

	if ((pDC != NULL) &&
	        (pNewDC != NULL) &&
	        (*pNewDC).CreateCompatibleDC(pDC)) {
		if (pPalette)
			pPalOld = (*pNewDC).SelectPalette(pPalette, FALSE);
		else
			pPalOld = NULL;
		(void)(*pNewDC).RealizePalette();
		pBitmapOld = (*pNewDC).SelectObject(pBitmap);
		if (pBitmapOld != NULL)
			return (pNewDC);
	}

	return (NULL);
}


void CRules::ReleaseCompatibleContext(CDC *&pDC, CBitmap * &pBitmap, CBitmap *pBitmapOld, CPalette *pPalOld) {
	if (pBitmapOld != NULL) {
		(void)(*pDC).SelectObject(pBitmapOld);
		pBitmapOld = NULL;
	}

	if (pPalOld != NULL) {
		(void)(*pDC).SelectPalette(pPalOld, FALSE);
		pPalOld = NULL;
	}

	if (pBitmap != NULL) {
		(*pBitmap).DeleteObject();
		delete pBitmap;
		pBitmap = NULL;
	}

	if (pDC != NULL) {
		(*pDC).DeleteDC();
		delete pDC;
		pDC = NULL;
	}
}


CDibDoc *CRules::LoadScrollDIB(const char *pszPathName, CRect *pRect) {
	BOOL        bSuccess;
	HDIB        hDIB;
	LPSTR       lpDIB;
	int         dxDIB, dyDIB;
	CDibDoc     *pDibDoc = NULL;
//	CPalette *pPalette = NULL;
//	CBitmap      *pBitmap = NULL;

	pDibDoc = new CDibDoc();
	ASSERT(pDibDoc != NULL);

	bSuccess = (*pDibDoc).OpenDocument(pszPathName);
	ASSERT(bSuccess);
	hDIB = (*pDibDoc).GetHDIB();
	ASSERT(hDIB != NULL);
	lpDIB = (LPSTR) GlobalLock((HGLOBAL) hDIB);
	dxDIB = (int) DIBWidth(lpDIB);
	dyDIB = (int) DIBHeight(lpDIB);
	GlobalUnlock((HGLOBAL) hDIB);

	if (pRect != NULL)
		(*pRect).SetRect(0, 0, dxDIB, dyDIB);

	return (pDibDoc);
}


BOOL CRules::PaintScrollDIB(CDC *pDC, CDibDoc *pDibDoc) {
	BOOL        bSuccess;
	HDIB        hDIB;
	LPSTR       lpDIB;
	int         dxDIB, dyDIB;
	CRect       myRect;

	hDIB = (*pDibDoc).GetHDIB();
	ASSERT(hDIB != NULL);
	lpDIB = (LPSTR) GlobalLock((HGLOBAL) hDIB);
	dxDIB = (int) DIBWidth(lpDIB);
	dyDIB = (int) DIBHeight(lpDIB);
	GlobalUnlock((HGLOBAL) hDIB);

	myRect.SetRect(0, 0, dxDIB, dyDIB);

	bSuccess = PaintDIB((*pDC).m_hDC, myRect, hDIB, myRect, pScrollPalette);

	return (bSuccess);
}


CBitmap *CRules::CreateScrollBitmap(CDC *pDC, CDibDoc *pDibDoc, CPalette *pPalette) {
	HDIB        hDIB;
	CBitmap     *pBitmap = NULL;

	hDIB = (*pDibDoc).GetHDIB();
	ASSERT(hDIB != NULL);

	pBitmap = ConvertDIB(pDC, hDIB, pPalette);

	return (pBitmap);
}


void CRules::OnLButtonDown(UINT nFlags, CPoint point) {
	if (!bActiveWindow)
		return;

	if (ScrollTopCurlRect.PtInRect(point) && (nHelpPage != 0))
		UpdateScroll(nHelpPage - 1);
	else if (ScrollBotCurlRect.PtInRect(point) && !bHelpEOF)
		UpdateScroll(nHelpPage + 1);

	OnMouseMove(nFlags, point);

	CDialog::OnLButtonDown(nFlags, point);
}


void CRules::OnMouseMove(UINT nFlags, CPoint point) {
	HCURSOR hNewCursor = NULL;
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

	//if (hNewCursor != NULL);
	SetCursor(hNewCursor);

	CDialog::OnMouseMove(nFlags, point);
}


BOOL CRules::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message) {
	if ((*pWnd).m_hWnd == (*this).m_hWnd)
		return (TRUE);
	else
		return (FALSE);
}


void CRules::DoWaitCursor(void) {
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();

	(void)(*pMyApp).BeginWaitCursor();
}


void CRules::DoArrowCursor(void) {
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();

	(void)(*pMyApp).EndWaitCursor();
}

} // namespace HodjNPodj
} // namespace Bagel
