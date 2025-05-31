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

#ifndef BAGEL_METAGAME_FRAME_MOVYTMPL_H
#define BAGEL_METAGAME_FRAME_MOVYTMPL_H

#include "bagel/afxwin.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Frame {

#define INTRO_WIDTH     640
#define INTRO_HEIGHT    480
#define MOVIE_WIDTH     320
#define MOVIE_HEIGHT    240
#define MOVIE_OVER      1000         //this value may be changed if compiler detects a duplicate case error. 

#define     PLAY_SUCCESSFUL     MCI_NOTIFY_SUCCESSFUL
#define     PLAY_SUPERSEDED     MCI_NOTIFY_SUPERSEDED
#define     PLAY_ABORTED        MCI_NOTIFY_ABORTED
#define     PLAY_FAILED         MCI_NOTIFY_FAILURE



class CMovieWindow;

class CDumWnd: public CWnd {
public:
	CDumWnd();  //constructor.
	BOOL CreateDum(HWND hWndTheParent, HWND hWndTheMovie, CWnd* pTheOwner, CMovieWindow* TheMovie, int x, int y);      //creates an invisible (dummy) child window, owned by pOwner.
protected:
	long OnMCINotify(WPARAM wParam, LPARAM lParam);
	void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	HWND hOwnr;            //the parent (owner) window.
	HWND hChild;             //the child (movie) window.
	HWND hDum;              //==this->m_hWnd;
	CRect CDumRect;
	char *WndClass;       // the name of window class
	CWnd *pOwnr;           //the owner class object.
	CMovieWindow *pChild;  //the movie class object.

	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};


class CMovieWindow: public CWnd {
public:
	CDumWnd *pDum;      //the class of the dummy window object
	HWND hWndDum;       //the dummy window
	DWORD m_dwErrorFlag;  //the LOWORD indicates an Error condition, i.e.  TRUE=error, FALSE=no error.
	//the HIWORD gives the Error description:
	//  .   The command was successful-PLAY_SUCCESSFUL   (no error).
	//  .   The current command was superseded-PLAY_SUPERSEDED
	//  .   The current command was canceled-PLAY_ABORTED
	//  .   The command fails-PLAY_FAILURE

	CMovieWindow();


	/**
	 * Opens up a movie window.
	 * @param pParent	Pointer to owner class object.
	 * @param bScroll	scroll flag
	 * @param AviMovee	The name of movie, complete with the path.
	 * @param x	X position
	 * @param y	Y position
	 * @param w	Width
	 * @param h	Height
	 * @return		True if movie's playable, else FALSE.
	 */
	BOOL BlowWindow(CWnd *pParentWnd, BOOL bScroll, LPCSTR lpszAviMovie,
		int x = 0, int y = 0, int w = MOVIE_WIDTH, int h = MOVIE_HEIGHT);

protected:
	/**
	 * Plays a movie
	 * @return	True if movie was playable; FALSE, if any error.
	 */
	BOOL PlayMovie();

	LPSTR WndClass = nullptr;		// The name of windw-class
	CWnd *pOwner = nullptr;			// The class of the owner(pparent) window object.
	HWND hWndParent = nullptr;		// The owner window, (from the calling program).
	HWND hWndMovie = nullptr;		// The child window where the movie'll be played.
	LPSTR lpszAviMovie = nullptr;	// The name of movie.
	CRect MovieRect;
public:
	int nMovieId = -1;
};

} // namespace Frame
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
