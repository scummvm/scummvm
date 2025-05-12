/*****************************************************************
 * Copyright (c) 1994 by Ledge Multimedia, Inc., All Rights Reserved
 *
 *
 * movytmpl.cpp
 *
 * HISTORY
 *
 *  1.0 07/13/94 Nish Kamat     
 *
 * MODULE DESCRIPTION:
  	Declares a class CMovieWindow to be used for playing AVI movies. 
  	
  	It creates a child window to be owned by a parent from the calling program. 
  	The movie is played within the child window by calling
 		BlowWindow(CWnd* pParent,(LPCSTR)lpszAviMovie, int x=0, y=0, dx=640, dy=480);
	
	When the movie's over or when the user terminates it using the ESC or ALT_F4 key(s)
 	the module automatically destroys the child window and a dummy window that it creates 
 	to make MCI calls; but it does not delete the class objects. In any case the focus and the 
 	active window are intrinsically restored to the parent window.
 	
 	Hence: Oncethe movie's over, it sends back to the parent window a message
 	WM_COMMAND  (  wParam=MOVIE_OVER       ;    lParam=(LPARAM)pObject_to_delete ) .  
 	
 	Various errors can be brought up. There are two kinds of MCI errors:
 	a) Errors that crop up just before the child window can be setup and the movie played.
 		In this case BlowWindow(...) returns a FALSE.
 	b) Errors that crop up after the child window is set up and the movie has commenced playing.
 		In this case a public  Error Flag in the CMovieWindow object is set.  
 		
 		Please refer to movytmpl.h  for more detailed description of the error flag.
 	
 	
 	In the calling program's WM_COMMAND message handler,
 	typeCast the lParam to (CDumWnd*) before deleting. 
 	
 	A Typical example of  its usage in the calling program is:
 					
 					
 					static CMovieWindow* gpVid;			//globally declared.                                                                      
 					               :
 					               :
 					               : 
 					            {			//where you want the movie to be played.                                                                                     
 					LPCSTR lpszMovieName="c:\\boffo.avi";
 					CWnd* pParent;								                                                                         
 					BOOL bPlaying;
 					               
 					if (!gpVid) gpVid=new CMovieWindow();
 					if(gpVid)	bPlaying=gpVid->BlowWindow(pParent, lpszMovieName, 0,0, 320,240);
 												//bPlaying is set to TRUE if movie can be played. In that case, 
 												//a child window playing the movie is brought up. If there are 
 												//errors while playing the movie the public error flag in gpVid is set.
 					if(!bPlaying && gpVid){ delete(gpVid); gpVid=NULL;}
 								}
 								   :
 					               :
 					               :
 					            {           //OnCommand message handler.
 					BOOL bErrorCondition=FALSE;
 					
					switch(wParam){
 					       	case MOVIE_OVER: 	  if(lParam) delete((CDumWnd*)lParam);
 					       										if(gpVid){
 					       												if(LOWORD(gpVid->m_dwErrorFlag)){
 					       													 bErrorCondition=TRUE;
 					       													 MessageBox("MOVIE PLAY UNSUCCESSFUL");
 					       													 //you may also process the HIWORD of the m_dwError flag,
 					       													 // if necessary.
 					       												}
 					           											delete(gpVid);
 					           											gpVid=NULL;
 					       										}
 					       										
 					}
 								}//end OnCommand handler.										
 					                                                                             
 					                 :
 					                 :
 					                 :
 					                 :
 					                 :
 					                   	
	Important: 
	This template provides no direct capability to stop the playing of a movie. An error will 
	result if you try to play another movie before the first is over, (i.e. before you get the MOVIE_OVER
	message). You can of course send to the dummy window, a WM_KEYDOWN message (wparam=VK_ESCAPE) 
	and stop the movie if desired.
	
*
*  CONSTRUCTORS:
*
*       CMovieWindow::CMovieWindow                     Constructs  
*		CDumWnd::CDumWnd                             Constructs
*
*  PUBLIC:
*
*		CMovieWindow::m_dwErrorFlag					Indicates an Error condition.
*       CMovieWindow::BlowWindow                     Plays an AVI MOVIE in a child Window 
*
*  PROTECTED:
*       CMovieWindow::PlayMovie							Plays the movie.
*
*       CDumWnd::OnSysKeyDown                    Handles WM_SYSKEYDOWN messages for the ALT_F4 combination.      //
*       CDumWnd::OnKeyDown                    Handles WM_SYSKEYDOWN messages for the ESCAPE character code.
*		CDumWnd::OnMCINotify						Handles MM_MCINOTIFY mci message.  
*		CDumWnd::CreateDum						Creates an invisible dummy window (to take care of MCI calls and user interruptions.)
*
*  PRIVATE:
*
*
*
*  MEMBERS:
*
* RELEVANT DOCUMENTATION:
*
*	\vfwdk\inc should be in the path      
*
* FILES USED:
*                             
 ****************************************************************/

#include "bagel/afxwin.h"                     
     
#include <stdlib.h>

#include "movytmpl.h"
#include <mciavi.h>              // search path: \vfwdk\inc               ; VideoForWindows SDK
#include <digitalv.h>             // search path: \vfwdk\inc      
#include <vfw.h>
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"      

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
	MCI_DGV_OPEN_PARMS	mciOpen;                 
	
	m_dwErrorFlag=MAKELPARAM(0,(WPARAM)PLAY_SUCCESSFUL);				//clear Error Flag.
		 
    /* set up the open parameters */
    mciOpen.dwCallback = NULL;
    mciOpen.wDeviceID = mciOpen.wReserved0 =
			 mciOpen.wReserved1 = 0;
    mciOpen.lpstrDeviceType = "avivideo";
    mciOpen.lpstrElementName = NULL;
    mciOpen.lpstrAlias = NULL;
    mciOpen.dwStyle = 0;
    mciOpen.hWndParent = NULL;
		 
   /* try to open the driver */
    mciSendCommand(0, MCI_OPEN, (DWORD)(MCI_OPEN_TYPE), 
                         (DWORD)(LPMCI_DGV_OPEN_PARMS)&mciOpen);
	
	videoID=-1;
	hWndParent=(HWND)NULL;
	hWndMovie=(HWND)NULL;        
	pDum=new CDumWnd;            
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
	
	mciopen.dwCallback = NULL;
	mciopen.wDeviceID = mciopen.wReserved0 =
	mciopen.wReserved1 = 0;
	mciopen.lpstrDeviceType = NULL;
	mciopen.lpstrElementName = lpszAviMovie;
	mciopen.lpstrAlias = NULL;
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
                                                                                                                                                           

	mciWindow.dwCallback = NULL;
	mciWindow.hWnd = NULL;
	mciWindow.wReserved1 = mciWindow.wReserved2 = 0;
	mciWindow.nCmdShow = SW_SHOW;
	mciWindow.lpstrText = (LPSTR)NULL;
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
		mciSendCommand(videoID, MCI_CLOSE, 0L, NULL);
    	videoID=-1;
		mciSendCommand(mciGetDeviceID("avivideo"),MCI_CLOSE, MCI_WAIT, NULL);
	}
   	
	mciPlay.dwCallback=MAKELPARAM((HWND)pDum->m_hWnd,0);       //notify the Parent window upon end_of_movie.
	mciPlay.dwFrom = mciPlay.dwTo = 0;
	if(RtnVal=mciSendCommand(videoID,MCI_PLAY,MCI_NOTIFY,(DWORD)(LPVOID)&mciPlay)) {
		mciSendCommand(videoID,MCI_CLOSE,0,NULL);
#ifdef _DEBUG
		mciGetErrorString(RtnVal,(LPSTR)Str,STRLEN);
        MessageBox(Str,"",MB_ICONEXCLAMATION| MB_OK);
#endif
		return(FALSE);
	} 
	  	                     
	RtnVal=MAKELPARAM(::SetFocus(pDum->m_hWnd),0);
    return(TRUE);
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
	WndClass = (LPSTR)AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_PARENTDC, NULL, NULL, NULL);    
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
	BOOL d;   
	MCI_GENERIC_PARMS mciGeneric;
	
	bPaintScroll = FALSE;
	
    if(wParam==MCI_NOTIFY_SUCCESSFUL){
    	mciSendCommand(videoID, MCI_CLOSE, 0L, 
                 (DWORD)(LPMCI_GENERIC_PARMS)&mciGeneric);
    	videoID=-1;
	}                                           
	mciSendCommand(mciGetDeviceID("avivideo"),MCI_CLOSE, MCI_WAIT, NULL);
	if(d=IsWindow(hChild)){
		d=::DestroyWindow(hChild);
	}

	::InvalidateRect( hOwnr, NULL, TRUE);           
	::UpdateWindow(hOwnr);
	
	d=DestroyWindow();                                                                                                                              
	
	pChild->m_dwErrorFlag=MAKELPARAM((WORD)(!(wParam==MCI_NOTIFY_SUCCESSFUL)|| !d), wParam);  //set error flag, if error.         
	
	MFC::PostMessage(hOwnr, WM_COMMAND, MOVIE_OVER, (LPARAM)(LPVOID)this);
	::SetActiveWindow(hOwnr);
	::SetFocus(hOwnr);

	(void)lParam;    
	if(wParam==MCI_NOTIFY_SUCCESSFUL) return(1L);
	return(0L);
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
	BOOL b;
	CRect	cWndRect;

    hOwnr=hWndPar;
	hChild=hWndMov;                                                                                                                 
	
	pOwnr=pPar;
	pChild=pMov;
	
	cWndRect.SetRect(x,y,x + WINDOW_WIDTH,y + WINDOW_HEIGHT);
	
	b=Create(WndClass,"DUMMY", WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CS_BYTEALIGNWINDOW | CS_OWNDC, cWndRect,  pOwnr, NULL); 
    if(b){
    	hDum=(HWND)(this->m_hWnd);
    	::ShowWindow(hDum,SW_SHOW);
	}
	return b;
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
void CDumWnd::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{   
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
			mciSendCommand(mciGetDeviceID("avivideo"),MCI_CLOSE, MCI_WAIT, NULL);

			if(d=IsWindow(hChild)){
				d=::DestroyWindow(hChild);
			}   

			::InvalidateRect( hOwnr, NULL, TRUE);           
			::UpdateWindow(hOwnr);
	
			::SendMessage(hOwnr, WM_COMMAND, MOVIE_OVER, (LPARAM)(LPVOID)this);
    		PostMessage(WM_CLOSE, 0, 0);
    		break;

        default:
            CWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
            break;
    }
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
