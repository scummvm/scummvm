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

#define TEXT_MORE_DX		120						// offset of "more" indicator from right margin
#define TEXT_MORE_DY		5                       // offset of "more" indicator bottom of scroll
#define MORE_TEXT_BLURB		"[ More ]"				// actual text to display for "more" indicator
#define MORE_TEXT_LENGTH	8                       // # characters in "more" indicator string

extern CBfcMgr      *lpMetaGameStruct;

extern "C" {
LRESULT FAR PASCAL NotebookHookProc(int,WORD,LONG);	// keyboard hook procedure definition
}

extern 	HINSTANCE	hDLLInst;
extern  HINSTANCE 	hExeInst;

typedef LRESULT (FAR PASCAL *FPNOTEHOOKPROC) ( int, WORD, LONG );

static	FPNOTEHOOKPROC 	lpfnKbdHook = NULL;			// pointer to hook procedure

static	HHOOK		hKbdHook = NULL;                // handle for hook procedure

static	CWnd		*pParentWnd = NULL;				// parent window pointer

static	CNotebook	*pNotebookDialog = NULL;		// pointer to our notebook dialog box

static	CColorButton *pOKButton = NULL;				// OKAY button on scroll
static	CRect		OkayRect;						// rectangle bounding the OKAY button

static	CRect		NotebookRect;					// x/y (left/right) and dx/dy (right/bottom) for the notebook window
static	CRect		PersonRect,						// bounding rectangle for person bitmap
					PlaceRect;						// bounding rectangle for place bitmap
static	CRect		ScrollTopRect,					// area spanned by upper scroll curl
					ScrollBotRect;                  // area spanned by lower scroll curl
					
static	CDC			*pNotebookDC = NULL;			// device context for the notebook bitmap
static	CBitmap		*pNotebookBitmap = NULL,		// bitmap for an entirely blank notebook
					*pNotebookBitmapOld = NULL;     // bitmap previously mapped to the notebook context

static	CBitmap		*pBackgroundBitmap = NULL;
static	CPalette	*pBackgroundPalette = NULL;

static	CDC			*pWorkDC = NULL;				// context and resources for the offscreen
static	CBitmap		*pWork = NULL,                  // ... work area (only if memory permits)
					*pWorkOld = NULL;
static	CPalette	*pWorkPalOld = NULL;

/*
static	CText		*pItemText = NULL;				// item information field
static	CText		*pTitleText = NULL;				// title information field
*/

static	char		*lpsPersonSoundSpec = NULL;		// spec for person sound file
static	char		*lpsPlaceSoundSpec = NULL;      // spec for place sound file

static	CFont		*pNoteFont = NULL;				// font to use for displaying notebook text
static	char		chPathName[128];                // buffer to hold path name of the notebook file
static	CFile		*pInfoFile = NULL;              // the notebook file
static	DWORD		nInfoFileSize = 0;              // size of notebook file
static	BOOL		bInfoEOF = FALSE;               // whether end-of-file has been reached

static	BOOL		bActiveWindow = FALSE;			// whether our window is active
static	BOOL		bFirstTime = TRUE;				// flag for first time information is displayed
static	int			nNotebook_DX, nNotebook_DY;		// size of useable notebook background
static	int			nItem_DDX, nItem_DDY;           // space separation between inventory items
static	int			nItemsPerColumn, nItemsPerRow;	// span of items that fit on the background
static	int			nFirstSlot = 0;					// first item in current inventory page

static	CNote		*pNoteList = NULL;				// pointer to notebook note list
static	CNote		*pKeyNote = NULL;				// single note to be shown

/////////////////////////////////////////////////////////////////////////////
// CNotebook dialog



BOOL CNotebook::SetupKeyboardHook(void)
{
	pNotebookDialog = this;							// retain pointer to our dialog box

	lpfnKbdHook = (FPNOTEHOOKPROC)::GetProcAddress( hDLLInst, "NotebookHookProc");
	if (lpfnKbdHook == NULL)                           // setup pointer to our procedure
		return(FALSE);
	
	hKbdHook = SetWindowsHookEx(WH_KEYBOARD,(HOOKPROC) lpfnKbdHook, hExeInst, GetCurrentTask());
	if (hKbdHook == NULL)                           // plug in our keyboard hook
		return(FALSE);
	
	return(TRUE);
}


void CNotebook::RemoveKeyboardHook(void)
{
	if (m_bKeyboardHook)
		UnhookWindowsHookEx(hKbdHook);					// unhook our keyboard procedure

	pNotebookDialog = NULL;
	lpfnKbdHook = NULL;
	hKbdHook = NULL;
	m_bKeyboardHook = FALSE;
}


extern "C" 
LRESULT FAR PASCAL NotebookHookProc(int code, WORD wParam, LONG lParam)
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
			case VK_PRIOR:                              // go to previous note
				if ((pKeyNote == NULL) &&
					(pNoteList != NULL) &&
					((*pNoteList).m_pPrev != NULL)) {   // ... scroll up if not at first item
					pNoteList = (*pNoteList).m_pPrev;
					pDC = (*pNotebookDialog).GetDC();
				}
				break;
			case VK_DOWN:								// go to next note
			case VK_NUMPAD2:
			case VK_NEXT:
				if ((pKeyNote == NULL) &&
					(pNoteList != NULL) &&
					((*pNoteList).m_pNext != NULL)) {
					pNoteList = (*pNoteList).m_pNext;
					pDC = (*pNotebookDialog).GetDC();
				}
				break;
			case VK_HOME:								// go to first note
				if ((pKeyNote == NULL) &&
					(pNoteList != NULL) &&
					((*pNoteList).m_pPrev != NULL)) {   // ... scroll up if not at first item
					while ((*pNoteList).m_pPrev != NULL)
						pNoteList = (*pNoteList).m_pPrev;
					pDC = (*pNotebookDialog).GetDC();
				}
				break;
			case VK_END:								// go to last note
				if ((pKeyNote == NULL) &&
					(pNoteList != NULL) &&
					((*pNoteList).m_pNext != NULL)) {
					while ((*pNoteList).m_pNext != NULL)
						pNoteList = (*pNoteList).m_pNext;
					pDC = (*pNotebookDialog).GetDC();
				}
		}

	if (pDC != NULL) {                              	// update the inventory page if required
		CNotebook::UpdateNote(pDC);
		(*pNotebookDialog).ReleaseDC(pDC);
		return(TRUE);
	}
	
	return(FALSE);
}


CNotebook::CNotebook(CWnd* pParent,CPalette *pPalette, CNote *pNotes, CNote *pNote)
	: CDialog(CNotebook::IDD, pParent)
{
	pParentWnd = pParent;							// retain pointer to parent window
	pBackgroundPalette = pPalette;                  // retain palette to be used
	pNoteList = pNotes;								// retain pointer to note list
	pKeyNote = pNote;								// retain single note to be shown
	
	//{{AFX_DATA_INIT(CNotebook)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNotebook::DoDataExchange(CDataExchange* pDX)
{
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

void CNotebook::OnOK()
{
	ClearDialogImage();								// pre-clear dialog buttons
	CDialog::EndDialog(IDC_NOTEBOOK_OKAY);          // ... and restore background
}


void CNotebook::OnCancel(void)
{
	ClearDialogImage();								// pre-clear dialog buttons
    CDialog::OnCancel();                            // ... and restore background
}


void CNotebook::OnDestroy()
{
BOOL	bUpdateNeeded;
	
	if (pNoteFont != NULL)
		delete pNoteFont;							// release the font file

	if (pOKButton != NULL)                          // release the button if not gone already
		delete pOKButton;
	
//	(*pInfoFile).Close();                           // close and release the notebook file
//	delete pInfoFile;

	if (m_bKeyboardHook)								// remove keyboard hook, if present
		RemoveKeyboardHook();

	if (pWorkOld != NULL) {							// release the various contexts, palettes
		(void) (*pWorkDC).SelectObject(pWorkOld);   // ... and bitmaps that we used
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
	
	if (pNotebookBitmap != NULL) {
		delete pNotebookBitmap;
		pNotebookBitmap = NULL;
	}
	
	if (pBackgroundBitmap != NULL) {				// if we had a background to refresh
		delete pBackgroundBitmap;                   // ... then validate the parent window
		pBackgroundBitmap = NULL;
		bUpdateNeeded = (*pParentWnd).GetUpdateRect(NULL,FALSE);
		if (bUpdateNeeded)
	    	(*pParentWnd).ValidateRect(NULL);
	}

/*	
	if (pTitleText != NULL) {
		delete pTitleText;
		pTitleText = NULL;
	}
	if (pItemText != NULL) {
		delete pItemText;
		pItemText = NULL;
	}
*/
	
    CDialog::OnDestroy();
}


BOOL CNotebook::OnInitDialog()
{
BOOL	bSuccess;
CWnd	*pButton;												// pointer to the OKAY button
CRect	myRect;                                    				// rectangle that holds the button location
int		x, y, dx, dy;                              				// used for calculating positioning info

	CDialog::OnInitDialog();									// do basic dialog initialization

	if (pParentWnd == NULL)										// get our parent window
		pParentWnd = ((CWnd *) this)->GetParent();				// ... as passed to us or inquired about

	(*pParentWnd).GetWindowRect(&myRect);

	x = myRect.left + (((myRect.right - myRect.left) - NOTEBOOK_DX) >> 1);
	y = myRect.top + (((myRect.bottom - myRect.top) - NOTEBOOK_DY) >> 1);
	
	NotebookRect.SetRect(x,y,x + NOTEBOOK_DX,y + NOTEBOOK_DY);
	MoveWindow(x,y,NOTEBOOK_DX,NOTEBOOK_DY);					// center the dialog box on the parent
	
	pButton = GetDlgItem((int) GetDefID());						// get the window for the okay button
	ASSERT(pButton != NULL);                        			// ... and verify we have it
	(*pButton).GetWindowRect(&myRect);              			// get the button's position and size

	dx = myRect.right - myRect.left;	            			// calculate where to place the button
	x = (NOTEBOOK_DX - dx) >> 1;               					// ... centered at the bottom edge
	dy = myRect.bottom - myRect.top;
	y = NOTEBOOK_DY - dy - NOTE_BUTTON_DY;
	
	(*pButton).MoveWindow(x,y,dx,dy);               			// reposition the button
	OkayRect.SetRect(x,y,x + dx,y + dy);

	pOKButton = new CColorButton;								// build a color OKAY button to let us exit
	ASSERT(pOKButton != NULL);
	(*pOKButton).SetPalette(pBackgroundPalette);				// set the palette to use
	bSuccess = (*pOKButton).SetControl((int) GetDefID(),this);	// tie to the dialog control
    ASSERT(bSuccess);

	ScrollTopRect.SetRect(0,0,NOTEBOOK_DX,NOTEBOOK_CURL_DY);	// setup rectangles for scrolling areas
	ScrollBotRect.SetRect(0,NOTEBOOK_DY - NOTEBOOK_CURL_DY,NOTEBOOK_DX,NOTEBOOK_DY);

	PersonRect.SetRect(NOTE_PERSON_DX,NOTE_PERSON_DY,NOTE_PERSON_DX + NOTE_BITMAP_DX,NOTE_PERSON_DY + NOTE_BITMAP_DY);
	PlaceRect.SetRect(NOTE_PLACE_DX,NOTE_PLACE_DY,NOTE_PLACE_DX + NOTE_BITMAP_DX,NOTE_PLACE_DY + NOTE_BITMAP_DY);

	m_bKeyboardHook = SetupKeyboardHook();			// establish keyboard hook

	bFirstTime = TRUE;
			
	return(TRUE);  												// return TRUE  unless focused on a control
}


void CNotebook::OnActivate(UINT nState, CWnd *, BOOL /*bMinimized*/)
{
BOOL	bUpdateNeeded;

switch(nState) {												// force a repaint if activated
	case WA_INACTIVE:                                           // .. by switching from some other
		bActiveWindow = FALSE;                                  // ... application
		break;
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		bActiveWindow = TRUE;
		bUpdateNeeded = GetUpdateRect(NULL,FALSE);
		if (bUpdateNeeded)
			InvalidateRect(NULL,FALSE);
	}
}


void CNotebook::OnPaint()
{
BOOL		bSuccess;
CPaintDC	dc(this); 											// device context for painting

	if (bFirstTime) {											// acquire resources and if first time
		bFirstTime = FALSE;                         		
		bSuccess = CreateWorkAreas(&dc);
		if (!bSuccess)
			CDialog::OnCancel();
	 	}
	
	UpdateNotebook(&dc);                                        // update the notebook display
}


void CNotebook::UpdateNotebook(CDC *pDC)
{
CPalette	*pPalOld;

	DoWaitCursor();												// put up the hourglass cursor
		
	pPalOld = (*pDC).SelectPalette(pBackgroundPalette,FALSE);	// setup the proper palette
	(*pDC).RealizePalette();

	if (pWorkDC == NULL) {                                      // if we don't have a work area
		RefreshBackground();                                    // ... then update the screen directly
		PaintMaskedBitmap(pDC,pBackgroundPalette,pNotebookBitmap,0,0,NOTEBOOK_DX,NOTEBOOK_DY);
		UpdateContent(pDC);
//		if (pTitleText != NULL)
//			(*pTitleText).DisplayString(pDC, "Log Entries", 32, TEXT_HEAVY, RGB(128,0,128));
	}
	else {
		if (pBackgroundBitmap != NULL)                                                      // ... otherwise revise work area
			PaintBitmap(pWorkDC,pBackgroundPalette,pBackgroundBitmap,0,0,NOTEBOOK_DX,NOTEBOOK_DY);
		PaintMaskedBitmap(pWorkDC,pBackgroundPalette,pNotebookBitmap,0,0,NOTEBOOK_DX,NOTEBOOK_DY);
		UpdateContent(pWorkDC);                                 // ... then zap it to the screen
//		if (pTitleText != NULL)
//			(*pTitleText).DisplayString(pWorkDC, "Log Entries", 32, TEXT_HEAVY, RGB(128,0,128));
		(*pDC).BitBlt(0,0,NOTEBOOK_DX,NOTEBOOK_DY,pWorkDC,0,0,SRCCOPY);
	}

	(*pDC).SelectPalette(pPalOld,FALSE);                        // reset the palette
	
	DoArrowCursor();											// return to an arrow cursor
}


void CNotebook::UpdateNote(CDC *pDC)
{
CPalette	*pPalOld;

	if (pWorkDC == NULL) 										// update everything if no work area
		(*pNotebookDialog).InvalidateRect(NULL,FALSE);
	else {                                                      // otherwise just update central area
		DoWaitCursor();											// put up the hourglass cursor
		pPalOld = (*pDC).SelectPalette(pBackgroundPalette,FALSE);// setup the proper palette
		(*pDC).RealizePalette();
		PaintMaskedBitmap(pWorkDC,pBackgroundPalette,pNotebookBitmap,0,0,NOTEBOOK_DX,NOTEBOOK_DY);
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
		(*pDC).SelectPalette(pPalOld,FALSE);                    // reset the palette
		DoArrowCursor();
	}															// return to an arrow cursor
}


void CNotebook::UpdateContent(CDC *pDC)
{
int		i, x, y, dx, dy;
char	*pFileSpec;
CFont	*pFontOld;
CNote	*pNote;
CRect	myRect;
CText	*pText;
CSize   textInfo;                       						// font info about the text to be displayed
TEXTMETRIC	fontMetrics;
int		nDeltaX, nDeltaY;

	if (pNoteList == NULL) {									// empty notebook
		if (pKeyNote == NULL) {                                 // ... so just say so and leave
			pText = new CText();
			myRect.SetRect(NOTE_TEXT_DX,                        // calculate where to put it
						   NOTE_TEXT_DY,
						   NOTE_TEXT_DX + NOTE_TEXT_DDX,
						   NOTE_TEXT_DY + NOTE_TEXT_DDY);
			pText = new CText(pDC, pBackgroundPalette, &myRect, JUSTIFY_CENTER);
			(*pText).DisplayString(pDC, "The log is empty ...", 32, TEXT_BOLD, RGB(128,0,128));
			delete pText;
			return;
		}
		else
			pNote = pKeyNote;									// show the requested note
		}
	else
		pNote = pNoteList;	                                    // show the one note list points to

	lpsPersonSoundSpec = (*pNote).GetPersonSoundSpec();
	lpsPlaceSoundSpec = (*pNote).GetPlaceSoundSpec();

	pFileSpec = (*pNote).GetPersonArtSpec();
	if (pFileSpec != NULL)
		PaintMaskedDIB(pDC,pBackgroundPalette,pFileSpec,
					   NOTE_PERSON_DX,NOTE_PERSON_DY,NOTE_BITMAP_DX,NOTE_BITMAP_DY);
	else {
		::MessageBox(NULL,"Missing Person Artwork","Internal Problem",MB_ICONEXCLAMATION);
		ShowClue(pNote);
    }
    
	pFileSpec = (*pNote).GetPlaceArtSpec();
	if (pFileSpec != NULL)
		PaintMaskedDIB(pDC,pBackgroundPalette,pFileSpec,
					   NOTE_PLACE_DX,NOTE_PLACE_DY,NOTE_BITMAP_DX,NOTE_BITMAP_DY);
	else {
		::MessageBox(NULL,"Missing Place Artwork","Internal Problem",MB_ICONEXCLAMATION);
		ShowClue(pNote);
    }

	pFileSpec = (*pNote).GetClueArtSpec();
	if (pFileSpec != NULL) {
		if ((*pNote).GetRepeatCount() <= NOTE_BITMAPS_PER_LINE) {	// shrink the bitmaps a little if
			nDeltaX = NOTE_BITMAP_DX;                               // ... the number of repetitions
			nDeltaY = NOTE_BITMAP_DY;                               // ... would force them off the page
		}
		else {
			nDeltaX = NOTE_SMALL_BITMAP_DX;
			nDeltaY = NOTE_SMALL_BITMAP_DY;
		}
		dx = ((*pNote).GetRepeatCount() * nDeltaX) + (((*pNote).GetRepeatCount() - 1) * (NOTE_BITMAP_DDX / (*pNote).GetRepeatCount())); 
		x = ((NOTEBOOK_DX - (NOTEBOOK_BORDER_DX << 1)) - dx) >> 1;  // establish left most position
		for (i = 0; i < (*pNote).GetRepeatCount(); i++)             // loop till all icons displayed
			PaintMaskedDIB(pDC,pBackgroundPalette,pFileSpec,
							x + (i * (nDeltaX + (NOTE_BITMAP_DDX / (*pNote).GetRepeatCount()))) + NOTEBOOK_BORDER_DX,
							NOTE_ICON_DY,
							nDeltaX,
							nDeltaY);
	}
	else {
		::MessageBox(NULL,"Missing Clue Artwork","Internal Problem",MB_ICONEXCLAMATION);
		ShowClue(pNote);
    }

	pFontOld = (*pDC).SelectObject(pNoteFont);  				// select it into our context
	(*pDC).SetBkMode(TRANSPARENT);            					// make the text overlay transparently
	(*pDC).SetTextColor(NOTE_TEXT_COLOR);            			// set the color of the text

	if ((*pNote).GetDescription() != NULL) {
		textInfo = (*pDC).GetTextExtent((*pNote).GetDescription(),strlen((*pNote).GetDescription()));  // get the area spanned by the text
		dy = textInfo.cy * (strlen((*pNote).GetDescription()) / NOTE_TEXT_CHARSPERLINE);
		if (dy < NOTE_TEXT_DDY)                                     // use the estimated number of lines
			myRect.SetRect(NOTE_TEXT_DX,                            // ... of text to see if we can fit into
						   NOTE_TEXT_DY + ((NOTE_TEXT_DDY - dy) >> 1),	// ... a smaller rectangle, and thus
						   NOTE_TEXT_DX + NOTE_TEXT_DDX,                // ... center the text vertically as well
						   NOTE_TEXT_DY + ((NOTE_TEXT_DDY - dy) >> 1) + dy);
		else
			myRect.SetRect(NOTE_TEXT_DX,							// nope, so just use the default rectangle
						   NOTE_TEXT_DY,
						   NOTE_TEXT_DX + NOTE_TEXT_DDX,
						   NOTE_TEXT_DY + NOTE_TEXT_DDY);
		(*pDC).DrawText(                         					// zap the text to the work area
	            (LPCSTR) (*pNote).GetDescription(),
	            -1,
	            &myRect,
	            DT_CENTER | DT_WORDBREAK);
	}
	else {
		::MessageBox(NULL,"Missing Clue Description","Internal Problem",MB_ICONEXCLAMATION);
		ShowClue(pNote);
    }

	if (pNotebookBitmap != NULL) {
		(*pDC).GetTextMetrics(&fontMetrics);						// show whether there are more notes
		x = NOTEBOOK_DX - TEXT_MORE_DX;	                            // ... that can be scrolled through
		y = NOTEBOOK_DY -
			NOTEBOOK_CURL_DY +
			((NOTEBOOK_CURL_DY - fontMetrics.tmHeight) >> 1) -
			TEXT_MORE_DY;
		textInfo = (*pDC).GetTextExtent(MORE_TEXT_BLURB,MORE_TEXT_LENGTH);
		dx = textInfo.cx;
	    dy = fontMetrics.tmHeight;
	
		if ((*pNote).m_pNext == NULL) {
			myRect.SetRect(x,y,x + dx,y + dy);
			BltBitmap(pDC, pBackgroundPalette, pNotebookBitmap, &myRect, &myRect, SRCCOPY);
		}
		else {
		    (*pDC).SetTextColor(NOTE_MORE_COLOR);
			(*pDC).TextOut(x,y,MORE_TEXT_BLURB,MORE_TEXT_LENGTH);
		}
	}
	
	(void) (*pDC).SelectObject(pFontOld);         					// map out the font
}


void CNotebook::ShowClue(CNote *pNote)
{
char	blurb[128];

	sprintf(blurb,"Id=%d  Clue=%d  Person=%d  Place=%d",(*pNote).GetID(),(*pNote).GetClueID(),(*pNote).GetPersonID(),(*pNote).GetPlaceID());
	::MessageBox(NULL,blurb,"Internal Problem",MB_ICONINFORMATION);
}


BOOL CNotebook::OnEraseBkgnd(CDC *)
{
return(TRUE);													// do not automatically erase background to white
}


void CNotebook::ClearDialogImage(void)
{
	if (pBackgroundBitmap != NULL) {							// release the dialog button
		delete pOKButton;                                       // ... validate our window to avoid refresh
		pOKButton = NULL;                                       // ... and restore the background
		(*pNotebookDialog).ValidateRect(NULL);
		RefreshBackground();
	}
}


void CNotebook::RefreshBackground(void)
{
CDC	*pDC;

	if (pBackgroundBitmap != NULL) {
		pDC = (*pNotebookDialog).GetDC();						// get a context for our window
		PaintBitmap(pDC,pBackgroundPalette,pBackgroundBitmap,0,0,NOTEBOOK_DX,NOTEBOOK_DY);
		(*pNotebookDialog).ReleaseDC(pDC);						// release the context
	}
}


void CNotebook::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	
}


void CNotebook::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}


int CNotebook::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
BOOL	bSuccess;
	
	::AddFontResource("msserif.fon");						// create the text font we'll use
	pNoteFont = new CFont();
	ASSERT(pNoteFont != NULL);
	bSuccess = (*pNoteFont).CreateFont(NOTE_FONT_SIZE,0,0,0,FW_BOLD,0,0,0,0,OUT_RASTER_PRECIS,0,PROOF_QUALITY,FF_ROMAN,"MS Sans Serif");
	ASSERT(bSuccess);

	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}


BOOL CNotebook::CreateWorkAreas(CDC *pDC)
{
BOOL		bSuccess = FALSE;
CPalette	*pPalOld;
CRect		myRect;

	pNotebookBitmap = FetchBitmap(pDC,NULL,NOTEBOOK_SPEC);	// fetch the notebook's bitmap
	if (pNotebookBitmap == NULL)                            // ... and punt if not successful
		return(FALSE);
                                                            // get the background bitmap
	if ((GetFreeSpace(0) >= (unsigned long) 500000) &&
    	(GlobalCompact((unsigned long) 500000) >= (unsigned long) 400000))
		pBackgroundBitmap = FetchScreenBitmap(pDC,pBackgroundPalette,0,0,NOTEBOOK_DX,NOTEBOOK_DY);
	else
		pBackgroundBitmap = NULL;

	pPalOld = (*pDC).SelectPalette(pBackgroundPalette,FALSE);// create an offscreen bitmap that		
	(void) (*pDC).RealizePalette();                          // ... we can use to construct note
                                                             // ... entries to avoid flashes
	if ((GetFreeSpace(0) >= (unsigned long) 1000000) &&
    	(GlobalCompact((unsigned long) 500000) >= (unsigned long) 450000)) {
		pWork = new CBitmap();                                   // will paint directly if not successful
		if ((*pWork).CreateCompatibleBitmap(pDC,NOTEBOOK_DX,NOTEBOOK_DY)) {
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
	
	(void) (*pDC).SelectPalette(pWorkPalOld,FALSE);

	if (!bSuccess) {                                        // not successful, so tear down
		if (pWorkPalOld != NULL) {							// ... the work area
			(void) (*pWorkDC).SelectPalette(pWorkPalOld,FALSE);
			pWorkPalOld = NULL;
		}
		if (pWork != NULL) {
			delete pWork;
			pWork = NULL;
		}
		delete pWorkDC;
		pWorkDC = NULL;
	}

/*
	myRect.SetRect(NOTEBOOK_TEXTZONE_DX,
					 NOTEBOOK_BORDER_DY + NOTEBOOK_TITLEZONE_DDY,
					 NOTEBOOK_DX - NOTEBOOK_TEXTZONE_DX,
					 NOTEBOOK_BORDER_DY + NOTEBOOK_TITLEZONE_DDY + NOTEBOOK_TITLEZONE_DY);
	pTitleText = new CText(pDC, pBackgroundPalette, &myRect, JUSTIFY_CENTER);
*/
	
	return(TRUE);
}


void CNotebook::OnMouseMove(UINT nFlags, CPoint point)
{
HCURSOR	hNewCursor = NULL;
CWinApp	*pMyApp = NULL;

	if (!bActiveWindow)								// punt if window not active
		return;

	pMyApp = AfxGetApp();

	if (OkayRect.PtInRect(point))					// use standard arrow in buttons
		hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);
	else
	if (ScrollTopRect.PtInRect(point)) {            // set cursor to scolling up okay or invalid
		if ((pKeyNote != NULL) ||                   // ... depending on note status
			((pNoteList != NULL) &&
			 ((*pNoteList).m_pPrev == NULL))) 
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_INVALID);
		else
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_ARROWUP);
	}
	else
	if (ScrollBotRect.PtInRect(point)) {			// set cursor to scrolling down okay or invalid
		if ((pKeyNote != NULL) ||                   // ... depending on note status
			((pNoteList != NULL) &&
			 ((*pNoteList).m_pNext == NULL)))
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_INVALID);
		else
			hNewCursor = (*pMyApp).LoadCursor(IDC_RULES_ARROWDN);
	}
	else
	if (PersonRect.PtInRect(point) &&
		(lpsPersonSoundSpec != NULL))
		hNewCursor = (*pMyApp).LoadCursor(IDC_NOTEBOOK_SOUND);
	else
	if (PlaceRect.PtInRect(point) &&
		(lpsPlaceSoundSpec != NULL))
		hNewCursor = (*pMyApp).LoadCursor(IDC_NOTEBOOK_SOUND);

	if (hNewCursor == NULL) 						// use default cursor if not specified
		hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);

	ASSERT(hNewCursor != NULL);						// force the cursor change
	::SetCursor(hNewCursor);
	
	CDialog::OnMouseMove(nFlags, point);            // do standard mouse move behavior
}


void CNotebook::OnLButtonDown(UINT nFlags, CPoint point)
{
CDC		*pDC = NULL;
CSound	*pSound;

	if (!bActiveWindow)								// punt if window not active
		return;

	if (ScrollTopRect.PtInRect(point) &&			// if click is in upper curl, then
		(pKeyNote == NULL) &&
		(pNoteList != NULL) &&
		((*pNoteList).m_pPrev != NULL)) {            // ... scroll up if not at first item
		pNoteList = (*pNoteList).m_pPrev;
		pDC = GetDC();
	}
	else                                            // if click is in lower curl, then
	if (ScrollBotRect.PtInRect(point) &&            // ... scroll down if not show last item
		(pKeyNote == NULL) &&
		(pNoteList != NULL) &&
		((*pNoteList).m_pNext != NULL)) {
		pNoteList = (*pNoteList).m_pNext;
		pDC = GetDC();
	}
	else
	if (PersonRect.PtInRect(point)) {				// play the person sound file
		if (lpsPersonSoundSpec != NULL) {
			pSound = new CSound(this,lpsPersonSoundSpec,SOUND_WAVE | SOUND_QUEUE | SOUND_AUTODELETE);
			(*pSound).SetDrivePath(lpMetaGameStruct->m_chCDPath);
			(*pSound).Play();
		}
	}           
	else
	if (PlaceRect.PtInRect(point)) {                // play the place sound file
		if (lpsPlaceSoundSpec != NULL) {
			pSound = new CSound(this,lpsPlaceSoundSpec,SOUND_WAVE | SOUND_QUEUE | SOUND_AUTODELETE);
			(*pSound).SetDrivePath(lpMetaGameStruct->m_chCDPath);
			(*pSound).Play();
		}
	}           
	

	if (pDC != NULL) {                              // update the noteboook entry if required
		UpdateNote(pDC);
		ReleaseDC(pDC);
	}

	OnMouseMove(nFlags,point);                      // do standard mouse movement
	
	CDialog::OnLButtonDown(nFlags, point);          // do standard mouse clicking
}


BOOL CNotebook::OnSetCursor(CWnd *pWnd, UINT /*nHitTest*/, UINT /*message*/)
{
	if ((*pWnd).m_hWnd == (*this).m_hWnd)
		return(TRUE);
	else
		return(FALSE);
}


void CNotebook::DoWaitCursor(void)
{
CWinApp	*pMyApp;

	pMyApp = AfxGetApp();

	(void) (*pMyApp).BeginWaitCursor();
}


void CNotebook::DoArrowCursor(void)
{
CWinApp	*pMyApp;

	pMyApp = AfxGetApp();

	(void) (*pMyApp).EndWaitCursor();
}
