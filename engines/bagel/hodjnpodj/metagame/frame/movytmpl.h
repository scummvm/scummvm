/*****************************************************************
 *
 *  movytmpl.h
 *
 *  Copyright (c) 1994 by Ledge Multimedia, All Rights Reserved
 *
 *  HISTORY
 *
 *      1.0     07/20/94     Nish Kamat  
 *
 *  MODULE DESCRIPTION:
 *
 *      Class definitions for CMovieWindow, to be used by movytmpl.cpp
 *		and your application.cpp.
 *   
 *  RELEVANT DOCUMENTATION:
 *
 *     Windows SDK doc for MCI.
 *   
 ****************************************************************/


#ifndef _INC_MOVYTMPL_H
#define _INC_MOVYTMPL_H

#include "bagel/afxwin.h"                     
     
#include <stdlib.h>
#include "bagel/hodjnpodj/hnplibs/stdinc.h"                 


#define INTRO_WIDTH 	640
#define INTRO_HEIGHT	480  
#define MOVIE_WIDTH 	320
#define MOVIE_HEIGHT	240  
#define MOVIE_OVER   	1000         //this value may be changed if compiler detects a duplicate case error. 

#define     PLAY_SUCCESSFUL		MCI_NOTIFY_SUCCESSFUL
#define 	PLAY_SUPERSEDED 	MCI_NOTIFY_SUPERSEDED
#define 	PLAY_ABORTED		MCI_NOTIFY_ABORTED
#define 	PLAY_FAILED			MCI_NOTIFY_FAILURE



class CMovieWindow; 
 
class CDumWnd: public CWnd{
	public:                   
		CDumWnd();	//constructor. 
   		BOOL CreateDum(HWND hWndTheParent, HWND hWndTheMovie, CWnd* pTheOwner, CMovieWindow* TheMovie, int x, int y);      //creates an invisible (dummy) child window, owned by pOwner.
	protected:
		long OnMCINotify(WPARAM wParam, LPARAM lParam); 
		void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		
		HWND hOwnr;            //the parent (owner) window.
		HWND hChild;             //the child (movie) window.
		HWND hDum;              //==this->m_hWnd;
		CRect CDumRect;                            
		char* WndClass;       // the name of window class
		CWnd* pOwnr;           //the owner class object.
		CMovieWindow* pChild;  //the movie class object.

    	afx_msg void OnPaint();
		DECLARE_MESSAGE_MAP()
};


class CMovieWindow: public CWnd{                                                                                                                                                                                            

	public:                   
		CDumWnd* pDum;		//the class of the dummy window object
		HWND hWndDum;	    //the dummy window
		DWORD m_dwErrorFlag;  //the LOWORD indicates an Error condition, i.e.  TRUE=error, FALSE=no error.
												//the HIWORD gives the Error description:
												//	. 	The command was successful—PLAY_SUCCESSFUL   (no error).
												//	·	The current command was superseded—PLAY_SUPERSEDED
												//	·	The current command was canceled—PLAY_ABORTED
												//	·	The command fails—PLAY_FAILURE
		
		CMovieWindow();
		BOOL BlowWindow(CWnd *pParentWnd, BOOL bScroll, LPCSTR lpszAviMovie, int movie_x=0, int movie_y=0, int movie_width=MOVIE_WIDTH, int movie_height=MOVIE_HEIGHT);
		         //use this func. to play the AVI movie within a child window,
				 // to be owned by the parent window CWnd*. The width
				 // and height of the child window are specified too.      
				 //returns TRUE if no error, else FALSE.


	protected:
		BOOL PlayMovie(void);  //plays movie. 
		

		LPSTR WndClass;       //the name of windw-class      
		CWnd* pOwner;			//the class of the owner(pparent) window object.
		HWND hWndParent;	    //the owner window, (from the calling program).
		HWND hWndMovie;			//the child window where the movie'll be played.
		LPSTR lpszAviMovie;     //the name of movie.
		CRect MovieRect;
	public:
		int	  nMovieId;
};	   
#endif
