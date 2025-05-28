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

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/metagame/frame/movytmpl.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#ifdef TODO
#include <mciavi.h>
#include <digitalv.h>
#include <vfw.h>
#endif

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Frame {

#define WINDOW_WIDTH	640
#define WINDOW_HEIGHT	480

#define STRLEN  		129         
#define	SCROLL_BITMAP	"..\\video\\vscroll.bmp"

#pragma warning(disable: 4706)

static int 		videoID;
static BOOL		bPaintScroll;


/*****************************************************************
 *
 * CMovieWindow::CMovieWindow()
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *		 opens driver, creates a dummy window object
 *		 and sets up default values.
 *   
 * FORMAL PARAMETERS:
 *
 *      n/a
 *   
 * RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
CMovieWindow::CMovieWindow(void){
#ifdef TODO
	MCI_DGV_OPEN_PARMS	mciOpen;                 
	
	m_dwErrorFlag=MAKELPARAM(0,(WPARAM)PLAY_SUCCESSFUL);				//clear Error Flag.
		 
    /* set up the open parameters */
    mciOpen.dwCallback = nullptr;
    mciOpen.wDeviceID = mciOpen.wReserved0 =
			 mciOpen.wReserved1 = 0;
    mciOpen.lpstrDeviceType = "avivideo";
    mciOpen.lpstrElementName = nullptr;
    mciOpen.lpstrAlias = nullptr;
    mciOpen.dwStyle = 0;
    mciOpen.hWndParent = nullptr;
		 
   /* try to open the driver */
    mciSendCommand(0, MCI_OPEN, (DWORD)(MCI_OPEN_TYPE), 
                         (DWORD)(LPMCI_DGV_OPEN_PARMS)&mciOpen);
	
	videoID=-1;
	hWndParent=(HWND)nullptr;
	hWndMovie=(HWND)nullptr;
	pDum=new CDumWnd;
#endif
}                                                                                                           



/*****************************************************************
 *
 * CMovieWindow::BlowWindow(CWnd*, LPCSTR, int, int, int, int)
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *		 opens up a movie window.
 *   
 * FORMAL PARAMETERS:
 *      pParent 	pointer to owner class object.
 *      AviMovee 	the name of movie, complete with the path.
 *		x, y, theMOVIE_WIDTH, theMOVIE_HEIGHT the coordinates/dimesnsions
 *																						 of	the movie window.
 *   
 * RETURN VALUE:
 *
 *     True if movie's playable, else FALSE.
 *
 ****************************************************************/	
BOOL CMovieWindow::BlowWindow(CWnd* pParent,BOOL bScroll,LPCSTR AviMovee, int x, int y, int theMOVIE_WIDTH,int theMOVIE_HEIGHT){
	BOOL b;

	bPaintScroll = bScroll;
	
	MovieRect.SetRect(x, y, ( x + theMOVIE_WIDTH ), ( y + theMOVIE_HEIGHT ));

	hWndParent=pParent->m_hWnd;   
	if(!hWndParent){
#ifdef _DEBUG
		MessageBox("null hwndParent");
#endif
		return(FALSE);
	}
	lpszAviMovie=(LPSTR) AviMovee;
    pOwner=pParent;

    b=PlayMovie();
    return(b);
}                                                                


/*****************************************************************
 *
 * CMovieWindow::PlayMovie(void)
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *		plays movie, brings up error message boxes if needed.
    
 * FORMAL PARAMETERS:
 *		n/a
 *   
 * RETURN VALUE:
 *
 *     True if movie's playable; FALSE, if any error.
 *
 ****************************************************************/
BOOL CMovieWindow::PlayMovie(void){
#ifdef TODO
    MCI_DGV_WINDOW_PARMS    mciWindow;
	MCI_DGV_OPEN_PARMS mciopen;
	MCI_DGV_PLAY_PARMS mciPlay;
	MCI_GENERIC_PARMS mcigen;                
	MCI_STATUS_PARMS mciStatus;

	DWORD RtnVal; 
	                                                                        
	char Str[STRLEN+1];                                                                                          

	if(videoID != -1) {
		mcigen.dwCallback=MAKELPARAM((HWND)hWndParent,0);
		mciSendCommand(videoID,MCI_STOP,0,(DWORD)(LPVOID)&mcigen);
		videoID=-1;
	}                                 
	
	mciopen.dwCallback = nullptr;
	mciopen.wDeviceID = mciopen.wReserved0 =
	mciopen.wReserved1 = 0;
	mciopen.lpstrDeviceType = nullptr;
	mciopen.lpstrElementName = lpszAviMovie;
	mciopen.lpstrAlias = nullptr;
	mciopen.dwStyle = WS_CHILD;
	mciopen.hWndParent = hWndParent;
    
	if(RtnVal=mciSendCommand(0,MCI_OPEN,MCI_DGV_OPEN_PARENT | MCI_OPEN_ELEMENT| MCI_DGV_OPEN_WS ,(DWORD)(LPMCI_DGV_OPEN_PARMS)&mciopen)) {
#ifdef _DEBUG
		mciGetErrorString(RtnVal,(LPSTR)Str,STRLEN);
	    MessageBox(Str,"",MB_ICONEXCLAMATION| MB_OK);
#endif
		return(FALSE);
	}                      

	videoID=mciopen.wDeviceID;       
                                                                                                                                                           

	mciWindow.dwCallback = nullptr;
	mciWindow.hWnd = nullptr;
	mciWindow.wReserved1 = mciWindow.wReserved2 = 0;
	mciWindow.nCmdShow = SW_SHOW;
	mciWindow.lpstrText = (LPSTR)nullptr;
	mciSendCommand(videoID, MCI_WINDOW, \
												MCI_DGV_WINDOW_STATE, \
			 										(DWORD)(LPMCI_DGV_WINDOW_PARMS)&mciWindow);

		 /* get the window handle */
	mciStatus.dwItem = MCI_DGV_STATUS_HWND;
	mciSendCommand(videoID, \
						 				MCI_STATUS, MCI_STATUS_ITEM, \
			 									(DWORD)(LPMCI_STATUS_PARMS)&mciStatus);
	hWndMovie = (HWND)mciStatus.dwReturn;     
	if(!hWndMovie){
#ifdef _DEBUG
		MessageBox("Null Movie Window");
#endif
		return(FALSE);
	}
	                                 
   	::MoveWindow(hWndMovie,MovieRect.left,MovieRect.top,MovieRect.Width(), MovieRect.Height(), TRUE);

	if(!(pDum->CreateDum(hWndParent, hWndMovie,  pOwner, this, MovieRect.left - ((WINDOW_WIDTH - MovieRect.Width()) >> 1), MovieRect.top - ((WINDOW_HEIGHT - MovieRect.Height()) >> 1)))) {
		mciSendCommand(videoID, MCI_CLOSE, 0L, nullptr);
    	videoID=-1;
		mciSendCommand(mciGetDeviceID("avivideo"),MCI_CLOSE, MCI_WAIT, nullptr);
	}
   	
	mciPlay.dwCallback=MAKELPARAM((HWND)pDum->m_hWnd,0);       //notify the Parent window upon end_of_movie.
	mciPlay.dwFrom = mciPlay.dwTo = 0;
	if(RtnVal=mciSendCommand(videoID,MCI_PLAY,MCI_NOTIFY,(DWORD)(LPVOID)&mciPlay)) {
		mciSendCommand(videoID,MCI_CLOSE,0,nullptr);
#ifdef _DEBUG
		mciGetErrorString(RtnVal,(LPSTR)Str,STRLEN);
        MessageBox(Str,"",MB_ICONEXCLAMATION| MB_OK);
#endif
		return(FALSE);
	} 
	  	                     
	RtnVal=MAKELPARAM(::SetFocus(pDum->m_hWnd),0);
#endif
    return TRUE;
}


/*****************************************************************
 *
 * CDumWnd::CDumWnd()
 *
 * FUNCTIONAL DESCRIPTION:
 *       CONSTRUCTOR for the DUMMY object class.
 *		 registers a window class to be used in creation
 *		 of DUMMY window and sets up default values.
 *   
 * FORMAL PARAMETERS:
 *
 *      n/a
 *   
 * RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
CDumWnd::CDumWnd(){                                                                    

		/*need a window class which shares the parent-dc for optimization, in order to create a DUMMY window
		*/
	WndClass = (LPSTR)AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_PARENTDC, nullptr, nullptr, nullptr);    
	CDumRect.SetRect(0,0,30,30);                                                        //arbitrary because it's gonna be invisible anyways.
}       


/*****************************************************************
 *
 * CDumWnd::OnMCINotify(WPARAM, LPARAM)
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *		handles the MM_MCINOTIFY message, that is sent 
 *		upon the completion of movie. It cleans up desktop
 *		by destroying the movie window and itself before 
 *		restoring focus to the parent window. It also sends 
 *		a WM_COMMAND message to the parent window with
 *		the following parameters:
 *		wParam :	MOVIE_OVER
 *		lParam	:	pointer to current object (ie. this)
 *   
 * FORMAL PARAMETERS:
 *
 *      wParam, lParam : refer to MCI doc.
 *   
 * RETURN VALUE:
 *
 *      long : 1L if wParam==MCI_NOTIFY_SUCCESSFUL,
 *					0L otherwise.
 *
 ****************************************************************/																													
long CDumWnd::OnMCINotify(WPARAM wParam, LPARAM lParam){              
#ifdef TODO
	BOOL d;   
	MCI_GENERIC_PARMS mciGeneric;
	
	bPaintScroll = FALSE;
	
    if(wParam==MCI_NOTIFY_SUCCESSFUL){
    	mciSendCommand(videoID, MCI_CLOSE, 0L, 
                 (DWORD)(LPMCI_GENERIC_PARMS)&mciGeneric);
    	videoID=-1;
	}                                           
	mciSendCommand(mciGetDeviceID("avivideo"),MCI_CLOSE, MCI_WAIT, nullptr);
	if(d=IsWindow(hChild)){
		d=::DestroyWindow(hChild);
	}

	::InvalidateRect( hOwnr, nullptr, TRUE);           
	::UpdateWindow(hOwnr);
	
	d=DestroyWindow();                                                                                                                              
	
	pChild->m_dwErrorFlag=MAKELPARAM((WORD)(!(wParam==MCI_NOTIFY_SUCCESSFUL)|| !d), wParam);  //set error flag, if error.         
	
	::PostMessage(hOwnr, WM_COMMAND, MOVIE_OVER, (LPARAM)(LPVOID)this);
	::SetActiveWindow(hOwnr);
	::SetFocus(hOwnr);

	(void)lParam;    
	if(wParam==MCI_NOTIFY_SUCCESSFUL) return(1L);
#endif
	return 0L;
}


/*****************************************************************
 *
 * CDumWnd::CreateDum(HWND, HWND, CWnd*, CWnd*)
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *		creates a new window with appropriate ownerships
 *		and sets up the protected members of this class. 
 *   
 * FORMAL PARAMETERS:
 *
 *      hWndPar: parent window from application.
 *		hWndmov: child window to play movie.
 *		pPar:	class of parent window 
 *		pMov:	class of MovieWindow.
 *   
 * RETURN VALUE:
 *
 *      long : 1L if wParam==MCI_NOTIFY_SUCCESSFUL,
 *					0L otherwise.
 *
 ****************************************************************/
BOOL CDumWnd::CreateDum(HWND hWndPar, HWND hWndMov, CWnd* pPar, CMovieWindow* pMov, int x, int y){
#ifdef TODO
	BOOL b;
	CRect	cWndRect;

    hOwnr=hWndPar;
	hChild=hWndMov;                                                                                                                 
	
	pOwnr=pPar;
	pChild=pMov;
	
	cWndRect.SetRect(x,y,x + WINDOW_WIDTH,y + WINDOW_HEIGHT);
	
	b=Create(WndClass,"DUMMY", WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CS_BYTEALIGNWINDOW | CS_OWNDC, cWndRect,  pOwnr, nullptr); 
    if(b){
    	hDum=(HWND)(this->m_hWnd);
    	MFC::ShowWindow(hDum,SW_SHOW);
	}
	return b;
#endif
	return false;
}                


void CDumWnd::OnPaint()
{
	CPaintDC 	dc(this);
	CBitmap		*pBitmap;
	CPalette	*pPalette;

    if (bPaintScroll) {
		pBitmap = FetchBitmap(&dc, &pPalette, SCROLL_BITMAP);
		PaintBitmap(&dc, pPalette, pBitmap);
		delete pBitmap;
		delete pPalette;
	}
}

                                                                    
 /*****************************************************************
 *
 * CDumWnd::OnSysKeyDown(UINT, UINT, UINT)
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *		handles the wm_SYSKEYDOWN message; it intercepts just
 *		the ALT_F4 combination and performs clean-up.
 *		Other keys are transmitted down  to the default handler.
 *   
 * FORMAL PARAMETERS:
 *
 *      refer to WindowsSDK / MFC library doc.
 *   
 * RETURN VALUE:
 *
 *	void
 *
 ****************************************************************/                                                                   
void CDumWnd::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
#ifdef TODO
	BOOL d;   
	MCI_GENERIC_PARMS mciGeneric;
	
    switch (nChar) {

        // User has hit ALT_F4 so close down the MCI calls and this dummy window.
        //
        case VK_F4:
        	bPaintScroll = FALSE;
         
	    	mciSendCommand(videoID, MCI_CLOSE, 0L, 
	                 (DWORD)(LPMCI_GENERIC_PARMS)&mciGeneric);
   			videoID=-1;
			mciSendCommand(mciGetDeviceID("avivideo"),MCI_CLOSE, MCI_WAIT, nullptr);

			if(d=IsWindow(hChild)){
				d=::DestroyWindow(hChild);
			}   

			::InvalidateRect( hOwnr, nullptr, TRUE);           
			::UpdateWindow(hOwnr);
	
			::SendMessage(hOwnr, WM_COMMAND, MOVIE_OVER, (LPARAM)(LPVOID)this);
    		PostMessage(WM_CLOSE, 0, 0);
    		break;

        default:
            CWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
            break;
    }
#endif
}                         



 /*****************************************************************
 *
 * CDumWnd::OnKeyDown(UINT, UINT, UINT)
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *		handles the wm_KEYDOWN message; it intercepts just
 *		the ESC key and performs clean-up.
 *		Other keys are transmitted down  to the default handler.
 *   
 * FORMAL PARAMETERS:
 *
 *      refer to WindowsSDK / MFC library doc.
 *   
 * RETURN VALUE:
 *
 *	void
 *
 ****************************************************************/
void CDumWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags){
    switch (nChar) {

        // User has hit ESCAPE so close down the MCI calls and this dummy window.
        //
        case VK_ESCAPE: 
        	OnSysKeyDown(VK_F4, nRepCnt, nFlags);
    		break;

        default:
            CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
            break;
    }
}

BEGIN_MESSAGE_MAP(CDumWnd, CWnd)
	ON_MESSAGE(MM_MCINOTIFY,  OnMCINotify)   //map the MM_MCINOTIFY message.
	ON_WM_SYSKEYDOWN()
	ON_WM_KEYDOWN()
    ON_WM_PAINT()
END_MESSAGE_MAP()   

} // namespace Frame
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
