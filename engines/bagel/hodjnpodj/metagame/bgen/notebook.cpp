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
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/metagame/bgen/notebook.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define TEXT_MORE_DX        120                     // offset of "more" indicator from right margin
#define TEXT_MORE_DY        5                       // offset of "more" indicator bottom of scroll
#define MORE_TEXT_BLURB     "[ More ]"              // actual text to display for "more" indicator
#define MORE_TEXT_LENGTH    8                       // # characters in "more" indicator string

extern CBfcMgr *lpMetaGameStruct;

LRESULT NotebookHookProc(int, WPARAM, LPARAM); // keyboard hook procedure definition

extern  HINSTANCE   hDLLInst;
extern  HINSTANCE   hExeInst;

typedef LRESULT(*FPNOTEHOOKPROC)(int, WPARAM, LPARAM);

static  FPNOTEHOOKPROC  lpfnKbdHook = nullptr;         // pointer to hook procedure

static  HHOOK       hKbdHook = nullptr;                // handle for hook procedure

static  CWnd *pParentWnd = nullptr;             // parent window pointer

static  CNotebook *pNotebookDialog = nullptr;        // pointer to our notebook dialog box

static  CColorButton *pOKButton = nullptr;             // OKAY button on scroll

//static  CDC *pNotebookDC = nullptr;            // device context for the notebook bitmap
static  CBitmap *pNotebookBitmap = nullptr;        // bitmap for an entirely blank notebook
//*pNotebookBitmapOld = nullptr;     // bitmap previously mapped to the notebook context

static  CBitmap *pBackgroundBitmap = nullptr;
static  CPalette *pBackgroundPalette = nullptr;

static  CDC *pWorkDC = nullptr;                // context and resources for the offscreen
static  CBitmap *pWork = nullptr,                  // ... work area (only if memory permits)
*pWorkOld = nullptr;
static  CPalette *pWorkPalOld = nullptr;

/*
static  CText       *pItemText = nullptr;              // item information field
static  CText       *pTitleText = nullptr;             // title information field
*/

static  const char *lpsPersonSoundSpec = nullptr;     // spec for person sound file
static  const char *lpsPlaceSoundSpec = nullptr;      // spec for place sound file

static  CFont *pNoteFont = nullptr;              // font to use for displaying notebook text
//static  char        chPathName[128];                // buffer to hold path name of the notebook file
//static  CFile *pInfoFile = nullptr;              // the notebook file
//static  uint32       nInfoFileSize = 0;              // size of notebook file
//static  bool        bInfoEOF = false;               // whether end-of-file has been reached

static  bool        bActiveWindow = false;          // whether our window is active
static  bool        bFirstTime = true;              // flag for first time information is displayed
//static  int         nNotebook_DX, nNotebook_DY;     // size of useable notebook background
//static  int         nItem_DDX, nItem_DDY;           // space separation between inventory items
//static  int         nItemsPerColumn, nItemsPerRow;  // span of items that fit on the background
//static  int         nFirstSlot = 0;                 // first item in current inventory page

static  CNote *pNoteList = nullptr;              // pointer to notebook note list
static  CNote *pKeyNote = nullptr;               // single note to be shown

/////////////////////////////////////////////////////////////////////////////
// CNotebook dialog



bool CNotebook::SetupKeyboardHook() {
	pNotebookDialog = this;                         // retain pointer to our dialog box

	hKbdHook = SetWindowsHookEx(WH_KEYBOARD, NotebookHookProc, hExeInst, GetCurrentTask());
	if (hKbdHook == nullptr)                           // plug in our keyboard hook
		return false;

	return true;
}


void CNotebook::RemoveKeyboardHook() {
	if (m_bKeyboardHook)
		UnhookWindowsHookEx(hKbdHook);                  // unhook our keyboard procedure

	pNotebookDialog = nullptr;
	lpfnKbdHook = nullptr;
	hKbdHook = nullptr;
	m_bKeyboardHook = false;
}


LRESULT NotebookHookProc(int code, WPARAM wParam, LPARAM lParam) {
	CDC *pDC = nullptr;

	if (code < 0)                                       // required to punt to next hook
		return (CallNextHookEx((HHOOK)lpfnKbdHook, code, wParam, lParam));

	if (lParam & 0xA0000000)                            // ignore ALT and key release
		return false;

	if (bActiveWindow)
		switch (wParam) {                               // process only the keys we are looking for
		case VK_UP:                                 // ... letting the default dialog procedure
		case VK_NUMPAD8:                            // ... deal with all the rest
		case VK_PRIOR:                              // go to previous note
			if ((pKeyNote == nullptr) &&
				(pNoteList != nullptr) &&
				((*pNoteList).m_pPrev != nullptr)) {   // ... scroll up if not at first item
				pNoteList = (*pNoteList).m_pPrev;
				pDC = (*pNotebookDialog).GetDC();
			}
			break;
		case VK_DOWN:                               // go to next note
		case VK_NUMPAD2:
		case VK_NEXT:
			if ((pKeyNote == nullptr) &&
				(pNoteList != nullptr) &&
				((*pNoteList).m_pNext != nullptr)) {
				pNoteList = (*pNoteList).m_pNext;
				pDC = (*pNotebookDialog).GetDC();
			}
			break;
		case VK_HOME:                               // go to first note
			if ((pKeyNote == nullptr) &&
				(pNoteList != nullptr) &&
				((*pNoteList).m_pPrev != nullptr)) {   // ... scroll up if not at first item
				while ((*pNoteList).m_pPrev != nullptr)
					pNoteList = (*pNoteList).m_pPrev;
				pDC = (*pNotebookDialog).GetDC();
			}
			break;
		case VK_END:                                // go to last note
			if ((pKeyNote == nullptr) &&
				(pNoteList != nullptr) &&
				((*pNoteList).m_pNext != nullptr)) {
				while ((*pNoteList).m_pNext != nullptr)
					pNoteList = (*pNoteList).m_pNext;
				pDC = (*pNotebookDialog).GetDC();
			}
		}

	if (pDC != nullptr) {                                  // update the inventory page if required
		CNotebook::UpdateNote(pDC);
		(*pNotebookDialog).ReleaseDC(pDC);
		return true;
	}

	return false;
}


CNotebook::CNotebook(CWnd *pParent, CPalette *pPalette, CNote *pNotes, CNote *pNote)
	: CDialog(CNotebook::IDD, pParent) {
	pParentWnd = pParent;                           // retain pointer to parent window
	pBackgroundPalette = pPalette;                  // retain palette to be used
	pNoteList = pNotes;                             // retain pointer to note list
	pKeyNote = pNote;                               // retain single note to be shown

	//{{AFX_DATA_INIT(CNotebook)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNotebook::DoDataExchange(CDataExchange *pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNotebook)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNotebook, CDialog)
	//{{AFX_MSG_MAP(CNotebook)
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
// CNotebook message handlers

void CNotebook::OnOK() {
	ClearDialogImage();                             // pre-clear dialog buttons
	CDialog::EndDialog(IDC_NOTEBOOK_OKAY);          // ... and restore background
}


void CNotebook::OnCancel() {
	ClearDialogImage();                             // pre-clear dialog buttons
	CDialog::OnCancel();                            // ... and restore background
}


void CNotebook::OnDestroy() {
	bool    bUpdateNeeded;

	if (pNoteFont != nullptr)
		delete pNoteFont;                           // release the font file

	if (pOKButton != nullptr)                          // release the button if not gone already
		delete pOKButton;

	//	(*pInfoFile).Close();                           // close and release the notebook file
	//	delete pInfoFile;

	if (m_bKeyboardHook)                                // remove keyboard hook, if present
		RemoveKeyboardHook();

	if (pWorkOld != nullptr) {                         // release the various contexts, palettes
		(*pWorkDC).SelectObject(pWorkOld);    // ... and bitmaps that we used
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

	if (pNotebookBitmap != nullptr) {
		delete pNotebookBitmap;
		pNotebookBitmap = nullptr;
	}

	if (pBackgroundBitmap != nullptr) {                // if we had a background to refresh
		delete pBackgroundBitmap;                   // ... then validate the parent window
		pBackgroundBitmap = nullptr;
		bUpdateNeeded = (*pParentWnd).GetUpdateRect(nullptr, false);
		if (bUpdateNeeded)
			(*pParentWnd).ValidateRect(nullptr);
	}

	/*
		if (pTitleText != nullptr) {
			delete pTitleText;
			pTitleText = nullptr;
		}
		if (pItemText != nullptr) {
			delete pItemText;
			pItemText = nullptr;
		}
	*/

	CDialog::OnDestroy();
}


bool CNotebook::OnInitDialog() {
	bool    bSuccess;
	CWnd *pButton;                                               // pointer to the OKAY button
	CRect   myRect;                                                 // rectangle that holds the button location
	int     x, y, dx, dy;                                           // used for calculating positioning info

	CDialog::OnInitDialog();                                    // do basic dialog initialization

	if (pParentWnd == nullptr)                                     // get our parent window
		pParentWnd = ((CWnd *)this)->GetParent();              // ... as passed to us or inquired about

	(*pParentWnd).GetWindowRect(&myRect);

	x = myRect.left + (((myRect.right - myRect.left) - NOTEBOOK_DX) >> 1);
	y = myRect.top + (((myRect.bottom - myRect.top) - NOTEBOOK_DY) >> 1);

	NotebookRect.SetRect(x, y, x + NOTEBOOK_DX, y + NOTEBOOK_DY);
	MoveWindow(x, y, NOTEBOOK_DX, NOTEBOOK_DY);                 // center the dialog box on the parent

	pButton = GetDlgItem((int)GetDefID());                     // get the window for the okay button
	ASSERT(pButton != nullptr);                                    // ... and verify we have it
	(*pButton).GetWindowRect(&myRect);                          // get the button's position and size

	dx = myRect.right - myRect.left;                            // calculate where to place the button
	x = (NOTEBOOK_DX - dx) >> 1;                                // ... centered at the bottom edge
	dy = myRect.bottom - myRect.top;
	y = NOTEBOOK_DY - dy - NOTE_BUTTON_DY;

	(*pButton).MoveWindow(x, y, dx, dy);                        // reposition the button
	OkayRect.SetRect(x, y, x + dx, y + dy);

	pOKButton = new CColorButton();                               // build a color OKAY button to let us exit
	ASSERT(pOKButton != nullptr);
	(*pOKButton).SetPalette(pBackgroundPalette);                // set the palette to use
	bSuccess = (*pOKButton).SetControl((int)GetDefID(), this); // tie to the dialog control
	ASSERT(bSuccess);

	ScrollTopRect.SetRect(0, 0, NOTEBOOK_DX, NOTEBOOK_CURL_DY); // setup rectangles for scrolling areas
	ScrollBotRect.SetRect(0, NOTEBOOK_DY - NOTEBOOK_CURL_DY, NOTEBOOK_DX, NOTEBOOK_DY);

	PersonRect.SetRect(NOTE_PERSON_DX, NOTE_PERSON_DY, NOTE_PERSON_DX + NOTE_BITMAP_DX, NOTE_PERSON_DY + NOTE_BITMAP_DY);
	PlaceRect.SetRect(NOTE_PLACE_DX, NOTE_PLACE_DY, NOTE_PLACE_DX + NOTE_BITMAP_DX, NOTE_PLACE_DY + NOTE_BITMAP_DY);

	m_bKeyboardHook = SetupKeyboardHook();          // establish keyboard hook

	bFirstTime = true;

	return true;                                              // return true  unless focused on a control
}


void CNotebook::OnActivate(unsigned int nState, CWnd *, bool /*bMinimized*/) {
	bool    bUpdateNeeded;

	switch (nState) {                                               // force a repaint if activated
	case WA_INACTIVE:                                           // .. by switching from some other
		bActiveWindow = false;                                  // ... application
		break;
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		bActiveWindow = true;
		bUpdateNeeded = GetUpdateRect(nullptr, false);
		if (bUpdateNeeded)
			InvalidateRect(nullptr, false);
	}
}


void CNotebook::OnPaint() {
	bool        bSuccess;
	CPaintDC    dc(this);                                           // device context for painting

	if (bFirstTime) {                                           // acquire resources and if first time
		bFirstTime = false;
		bSuccess = CreateWorkAreas(&dc);
		if (!bSuccess)
			CDialog::OnCancel();
	}

	UpdateNotebook(&dc);                                        // update the notebook display
}


void CNotebook::UpdateNotebook(CDC *pDC) {
	CPalette *pPalOld;

	DoWaitCursor();                                             // put up the hourglass cursor

	pPalOld = (*pDC).SelectPalette(pBackgroundPalette, false);  // setup the proper palette
	(*pDC).RealizePalette();

	if (pWorkDC == nullptr) {                                      // if we don't have a work area
		RefreshBackground();                                    // ... then update the screen directly
		PaintMaskedBitmap(pDC, pBackgroundPalette, pNotebookBitmap, 0, 0, NOTEBOOK_DX, NOTEBOOK_DY);
		UpdateContent(pDC);
		//		if (pTitleText != nullptr)
		//			(*pTitleText).DisplayString(pDC, "Log Entries", 32, TEXT_HEAVY, RGB(128,0,128));
	} else {
		if (pBackgroundBitmap != nullptr)                                                      // ... otherwise revise work area
			PaintBitmap(pWorkDC, pBackgroundPalette, pBackgroundBitmap, 0, 0, NOTEBOOK_DX, NOTEBOOK_DY);
		PaintMaskedBitmap(pWorkDC, pBackgroundPalette, pNotebookBitmap, 0, 0, NOTEBOOK_DX, NOTEBOOK_DY);
		UpdateContent(pWorkDC);                                 // ... then zap it to the screen
		//		if (pTitleText != nullptr)
		//			(*pTitleText).DisplayString(pWorkDC, "Log Entries", 32, TEXT_HEAVY, RGB(128,0,128));
		(*pDC).BitBlt(0, 0, NOTEBOOK_DX, NOTEBOOK_DY, pWorkDC, 0, 0, SRCCOPY);
	}

	(*pDC).SelectPalette(pPalOld, false);                       // reset the palette

	DoArrowCursor();                                            // return to an arrow cursor
}


void CNotebook::UpdateNote(CDC *pDC) {
	CPalette *pPalOld;

	if (pWorkDC == nullptr)                                        // update everything if no work area
		(*pNotebookDialog).InvalidateRect(nullptr, false);
	else {                                                      // otherwise just update central area
		DoWaitCursor();                                         // put up the hourglass cursor
		pPalOld = (*pDC).SelectPalette(pBackgroundPalette, false); // setup the proper palette
		(*pDC).RealizePalette();
		PaintMaskedBitmap(pWorkDC, pBackgroundPalette, pNotebookBitmap, 0, 0, NOTEBOOK_DX, NOTEBOOK_DY);
		UpdateContent(pWorkDC);                                 // zap it to the screen
		(*pDC).BitBlt(
			0,
			NOTEBOOK_CURL_DY,
			NOTEBOOK_DX,
			NOTEBOOK_DY - (NOTEBOOK_CURL_DY << 1),
			pWorkDC,
			0,
			NOTEBOOK_CURL_DY,
			SRCCOPY);
		(*pDC).BitBlt(
			NOTEBOOK_DX - TEXT_MORE_DX,
			NOTEBOOK_DY - NOTEBOOK_CURL_DY,
			TEXT_MORE_DX,
			NOTEBOOK_CURL_DY,
			pWorkDC,
			NOTEBOOK_DX - TEXT_MORE_DX,
			NOTEBOOK_DY - NOTEBOOK_CURL_DY,
			SRCCOPY);
		(*pDC).SelectPalette(pPalOld, false);                   // reset the palette
		DoArrowCursor();
	}                                                           // return to an arrow cursor
}


void CNotebook::UpdateContent(CDC *pDC) {
	int     i, x, y, dx, dy;
	const char *pFileSpec;
	CFont *pFontOld;
	CNote *pNote;
	CRect   myRect;
	CText *pText;
	CSize   textInfo;                                               // font info about the text to be displayed
	TEXTMETRIC  fontMetrics;
	int     nDeltaX, nDeltaY;

	if (pNoteList == nullptr) {                                    // empty notebook
		if (pKeyNote == nullptr) {                                 // ... so just say so and leave
			pText = new CText();
			myRect.SetRect(NOTE_TEXT_DX,                        // calculate where to put it
				NOTE_TEXT_DY,
				NOTE_TEXT_DX + NOTE_TEXT_DDX,
				NOTE_TEXT_DY + NOTE_TEXT_DDY);
			pText = new CText(pDC, pBackgroundPalette, &myRect, JUSTIFY_CENTER);
			(*pText).DisplayString(pDC, "The log is empty ...", 32, TEXT_BOLD, RGB(128, 0, 128));
			delete pText;
			return;
		} else
			pNote = pKeyNote;                                   // show the requested note
	} else
		pNote = pNoteList;                                      // show the one note list points to

	lpsPersonSoundSpec = (*pNote).GetPersonSoundSpec();
	lpsPlaceSoundSpec = (*pNote).GetPlaceSoundSpec();

	pFileSpec = (*pNote).GetPersonArtSpec();
	if (pFileSpec != nullptr)
		PaintMaskedDIB(pDC, pBackgroundPalette, pFileSpec,
			NOTE_PERSON_DX, NOTE_PERSON_DY, NOTE_BITMAP_DX, NOTE_BITMAP_DY);
	else {
		MessageBox(nullptr, "Missing Person Artwork", "Internal Problem", MB_ICONEXCLAMATION);
		ShowClue(pNote);
	}

	pFileSpec = (*pNote).GetPlaceArtSpec();
	if (pFileSpec != nullptr)
		PaintMaskedDIB(pDC, pBackgroundPalette, pFileSpec,
			NOTE_PLACE_DX, NOTE_PLACE_DY, NOTE_BITMAP_DX, NOTE_BITMAP_DY);
	else {
		MessageBox(nullptr, "Missing Place Artwork", "Internal Problem", MB_ICONEXCLAMATION);
		ShowClue(pNote);
	}

	pFileSpec = (*pNote).GetClueArtSpec();
	if (pFileSpec != nullptr) {
		if ((*pNote).GetRepeatCount() <= NOTE_BITMAPS_PER_LINE) {   // shrink the bitmaps a little if
			nDeltaX = NOTE_BITMAP_DX;                               // ... the number of repetitions
			nDeltaY = NOTE_BITMAP_DY;                               // ... would force them off the page
		} else {
			nDeltaX = NOTE_SMALL_BITMAP_DX;
			nDeltaY = NOTE_SMALL_BITMAP_DY;
		}
		dx = ((*pNote).GetRepeatCount() * nDeltaX) + (((*pNote).GetRepeatCount() - 1) * (NOTE_BITMAP_DDX / (*pNote).GetRepeatCount()));
		x = ((NOTEBOOK_DX - (NOTEBOOK_BORDER_DX << 1)) - dx) >> 1;  // establish left most position
		for (i = 0; i < (*pNote).GetRepeatCount(); i++)             // loop till all icons displayed
			PaintMaskedDIB(pDC, pBackgroundPalette, pFileSpec,
				x + (i * (nDeltaX + (NOTE_BITMAP_DDX / (*pNote).GetRepeatCount()))) + NOTEBOOK_BORDER_DX,
				NOTE_ICON_DY,
				nDeltaX,
				nDeltaY);
	} else {
		MessageBox(nullptr, "Missing Clue Artwork", "Internal Problem", MB_ICONEXCLAMATION);
		ShowClue(pNote);
	}

	pFontOld = (*pDC).SelectObject(pNoteFont);                  // select it into our context
	(*pDC).SetBkMode(TRANSPARENT);                              // make the text overlay transparently
	(*pDC).SetTextColor(NOTE_TEXT_COLOR);                       // set the color of the text

	if ((*pNote).GetDescription() != nullptr) {
		textInfo = (*pDC).GetTextExtent((*pNote).GetDescription(), strlen((*pNote).GetDescription())); // get the area spanned by the text
		dy = textInfo.cy * (strlen((*pNote).GetDescription()) / NOTE_TEXT_CHARSPERLINE);
		if (dy < NOTE_TEXT_DDY)                                     // use the estimated number of lines
			myRect.SetRect(NOTE_TEXT_DX,                            // ... of text to see if we can fit into
				NOTE_TEXT_DY + ((NOTE_TEXT_DDY - dy) >> 1),  // ... a smaller rectangle, and thus
				NOTE_TEXT_DX + NOTE_TEXT_DDX,                // ... center the text vertically as well
				NOTE_TEXT_DY + ((NOTE_TEXT_DDY - dy) >> 1) + dy);
		else
			myRect.SetRect(NOTE_TEXT_DX,                            // nope, so just use the default rectangle
				NOTE_TEXT_DY,
				NOTE_TEXT_DX + NOTE_TEXT_DDX,
				NOTE_TEXT_DY + NOTE_TEXT_DDY);
		(*pDC).DrawText(                              // zap the text to the work area
			(const char *)(*pNote).GetDescription(),
			-1,
			&myRect,
			DT_CENTER | DT_WORDBREAK);
	} else {
		MessageBox(nullptr, "Missing Clue Description", "Internal Problem", MB_ICONEXCLAMATION);
		ShowClue(pNote);
	}

	if (pNotebookBitmap != nullptr) {
		(*pDC).GetTextMetrics(&fontMetrics);                        // show whether there are more notes
		x = NOTEBOOK_DX - TEXT_MORE_DX;                             // ... that can be scrolled through
		y = NOTEBOOK_DY -
			NOTEBOOK_CURL_DY +
			((NOTEBOOK_CURL_DY - fontMetrics.tmHeight) >> 1) -
			TEXT_MORE_DY;
		textInfo = (*pDC).GetTextExtent(MORE_TEXT_BLURB, MORE_TEXT_LENGTH);
		dx = textInfo.cx;
		dy = fontMetrics.tmHeight;

		if ((*pNote).m_pNext == nullptr) {
			myRect.SetRect(x, y, x + dx, y + dy);
			BltBitmap(pDC, pBackgroundPalette, pNotebookBitmap, &myRect, &myRect, SRCCOPY);
		} else {
			(*pDC).SetTextColor(NOTE_MORE_COLOR);
			(*pDC).TextOut(x, y, MORE_TEXT_BLURB, MORE_TEXT_LENGTH);
		}
	}

	(*pDC).SelectObject(pFontOld);                            // map out the font
}


void CNotebook::ShowClue(CNote *pNote) {
	char    blurb[128];

	Common::sprintf_s(blurb, "Id=%d  Clue=%d  Person=%d  Place=%d", (*pNote).GetID(), (*pNote).GetClueID(), (*pNote).GetPersonID(), (*pNote).GetPlaceID());
	MessageBox(nullptr, blurb, "Internal Problem", MB_ICONINFORMATION);
}


bool CNotebook::OnEraseBkgnd(CDC *) {
	return true;                                                  // do not automatically erase background to white
}


void CNotebook::ClearDialogImage() {
	if (pBackgroundBitmap != nullptr) {                            // release the dialog button
		delete pOKButton;                                       // ... validate our window to avoid refresh
		pOKButton = nullptr;                                       // ... and restore the background
		(*pNotebookDialog).ValidateRect(nullptr);
		RefreshBackground();
	}
}


void CNotebook::RefreshBackground() {
	CDC *pDC;

	if (pBackgroundBitmap != nullptr) {
		pDC = (*pNotebookDialog).GetDC();                       // get a context for our window
		PaintBitmap(pDC, pBackgroundPalette, pBackgroundBitmap, 0, 0, NOTEBOOK_DX, NOTEBOOK_DY);
		(*pNotebookDialog).ReleaseDC(pDC);                      // release the context
	}
}


void CNotebook::OnShowWindow(bool bShow, unsigned int nStatus) {
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here

}


void CNotebook::OnSize(unsigned int nType, int cx, int cy) {
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

}


int CNotebook::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	bool    bSuccess;

	AddFontResource("msserif.fon");                       // create the text font we'll use
	pNoteFont = new CFont();
	ASSERT(pNoteFont != nullptr);
	bSuccess = (*pNoteFont).CreateFont(NOTE_FONT_SIZE, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FF_ROMAN, "MS Sans Serif");
	ASSERT(bSuccess);

	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}


bool CNotebook::CreateWorkAreas(CDC *pDC) {
	bool        bSuccess = false;
	//CPalette *pPalOld;
	CRect       myRect;

	pNotebookBitmap = FetchBitmap(pDC, nullptr, NOTEBOOK_SPEC); // fetch the notebook's bitmap
	if (pNotebookBitmap == nullptr)                            // ... and punt if not successful
		return false;
	// get the background bitmap
	if ((GetFreeSpace(0) >= (unsigned long)500000) &&
		(GlobalCompact((unsigned long)500000) >= (unsigned long)400000))
		pBackgroundBitmap = FetchScreenBitmap(pDC, pBackgroundPalette, 0, 0, NOTEBOOK_DX, NOTEBOOK_DY);
	else
		pBackgroundBitmap = nullptr;

	(*pDC).SelectPalette(pBackgroundPalette, false); // create an offscreen bitmap that
	(*pDC).RealizePalette();                           // ... we can use to construct note
	// ... entries to avoid flashes
	if ((GetFreeSpace(0) >= (unsigned long)1000000) &&
		(GlobalCompact((unsigned long)500000) >= (unsigned long)450000)) {
		pWork = new CBitmap();                                   // will paint directly if not successful
		if ((*pWork).CreateCompatibleBitmap(pDC, NOTEBOOK_DX, NOTEBOOK_DY)) {
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

	(*pDC).SelectPalette(pWorkPalOld, false);

	if (!bSuccess) {                                        // not successful, so tear down
		if (pWorkPalOld != nullptr) {                          // ... the work area
			(*pWorkDC).SelectPalette(pWorkPalOld, false);
			pWorkPalOld = nullptr;
		}
		if (pWork != nullptr) {
			delete pWork;
			pWork = nullptr;
		}
		delete pWorkDC;
		pWorkDC = nullptr;
	}

	/*
		myRect.SetRect(NOTEBOOK_TEXTZONE_DX,
						 NOTEBOOK_BORDER_DY + NOTEBOOK_TITLEZONE_DDY,
						 NOTEBOOK_DX - NOTEBOOK_TEXTZONE_DX,
						 NOTEBOOK_BORDER_DY + NOTEBOOK_TITLEZONE_DDY + NOTEBOOK_TITLEZONE_DY);
		pTitleText = new CText(pDC, pBackgroundPalette, &myRect, JUSTIFY_CENTER);
	*/

	return true;
}


void CNotebook::OnMouseMove(unsigned int nFlags, CPoint point) {
	HCURSOR hNewCursor = nullptr;
	CWinApp *pMyApp = nullptr;

	if (!bActiveWindow)                             // punt if window not active
		return;

	pMyApp = AfxGetApp();

	if (OkayRect.PtInRect(point))                   // use standard arrow in buttons
		hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);
	else if (ScrollTopRect.PtInRect(point)) {           // set cursor to scolling up okay or invalid
		if ((pKeyNote != nullptr) ||                   // ... depending on note status
			((pNoteList != nullptr) &&
				((*pNoteList).m_pPrev == nullptr)))
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_INVALID);
		else
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_ARROWUP);
	} else if (ScrollBotRect.PtInRect(point)) {         // set cursor to scrolling down okay or invalid
		if ((pKeyNote != nullptr) ||                   // ... depending on note status
			((pNoteList != nullptr) &&
				((*pNoteList).m_pNext == nullptr)))
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_INVALID);
		else
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_ARROWDN);
	} else if (PersonRect.PtInRect(point) &&
		(lpsPersonSoundSpec != nullptr))
		hNewCursor = (*pMyApp).LoadCursor(IDC_NOTEBOOK_SOUND);
	else if (PlaceRect.PtInRect(point) &&
		(lpsPlaceSoundSpec != nullptr))
		hNewCursor = (*pMyApp).LoadCursor(IDC_NOTEBOOK_SOUND);

	if (hNewCursor == nullptr)                         // use default cursor if not specified
		hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);

	ASSERT(hNewCursor != nullptr);                     // force the cursor change
	MFC::SetCursor(hNewCursor);

	CDialog::OnMouseMove(nFlags, point);            // do standard mouse move behavior
}


void CNotebook::OnLButtonDown(unsigned int nFlags, CPoint point) {
	CDC *pDC = nullptr;
	CSound *pSound;

	if (!bActiveWindow)                             // punt if window not active
		return;

	if (ScrollTopRect.PtInRect(point) &&            // if click is in upper curl, then
		(pKeyNote == nullptr) &&
		(pNoteList != nullptr) &&
		((*pNoteList).m_pPrev != nullptr)) {            // ... scroll up if not at first item
		pNoteList = (*pNoteList).m_pPrev;
		pDC = GetDC();
	}
	// if click is in lower curl, then
	// ... scroll down if not show last item
	else if (ScrollBotRect.PtInRect(point) &&
		(pKeyNote == nullptr) &&
		(pNoteList != nullptr) &&
		((*pNoteList).m_pNext != nullptr)) {
		pNoteList = (*pNoteList).m_pNext;
		pDC = GetDC();
	} else if (PersonRect.PtInRect(point)) {            // play the person sound file
		if (lpsPersonSoundSpec != nullptr) {
			pSound = new CSound(this, lpsPersonSoundSpec, SOUND_WAVE | SOUND_QUEUE | SOUND_AUTODELETE);
			(*pSound).setDrivePath(lpMetaGameStruct->m_chCDPath);
			(*pSound).play();
		}
	} else if (PlaceRect.PtInRect(point)) {             // play the place sound file
		if (lpsPlaceSoundSpec != nullptr) {
			pSound = new CSound(this, lpsPlaceSoundSpec, SOUND_WAVE | SOUND_QUEUE | SOUND_AUTODELETE);
			(*pSound).setDrivePath(lpMetaGameStruct->m_chCDPath);
			(*pSound).play();
		}
	}

	if (pDC != nullptr) {                              // update the noteboook entry if required
		UpdateNote(pDC);
		ReleaseDC(pDC);
	}

	OnMouseMove(nFlags, point);                     // do standard mouse movement

	CDialog::OnLButtonDown(nFlags, point);          // do standard mouse clicking
}


bool CNotebook::OnSetCursor(CWnd *pWnd, unsigned int /*nHitTest*/, unsigned int /*message*/) {
	if ((*pWnd).m_hWnd == (*this).m_hWnd)
		return true;
	else
		return false;
}


void CNotebook::DoWaitCursor() {
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();

	(*pMyApp).BeginWaitCursor();
}


void CNotebook::DoArrowCursor() {
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();

	(*pMyApp).EndWaitCursor();
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
