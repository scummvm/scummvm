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

#define MOVIE_WIDTH     640
#define MOVIE_HEIGHT    480
#define MOVIE_OVER      1000         //this value may be changed if compiler detects a duplicate case error. 

#define     PLAY_SUCCESSFUL     MCI_NOTIFY_SUCCESSFUL
#define     PLAY_SUPERSEDED     MCI_NOTIFY_SUPERSEDED
#define     PLAY_ABORTED        MCI_NOTIFY_ABORTED
#define     PLAY_FAILED         MCI_NOTIFY_FAILURE

class CMovieWindow {
public:
	/**
	 * Opens up a movie window.
	 * @param pParent	Pointer to owner class object.
	 * @param bScroll	scroll flag
	 * @param AviMovee	The name of movie, complete with the path.
	 * @param x	X position
	 * @param y	Y position
	 * @param w	Width
	 * @param h	Height
	 * @return		True if movie's playable, else false.
	 */
	bool BlowWindow(CWnd *pParentWnd, bool bScroll, const char *lpszAviMovie,
		int x = 0, int y = 0, int w = MOVIE_WIDTH, int h = MOVIE_HEIGHT);

protected:
	/**
	 * Plays a movie
	 * @return	True if movie was playable; false, if any error.
	 */
	bool PlayMovie();

	CRect MovieRect;
	CWnd *m_pParent = nullptr;
	const char *lpszAviMovie = nullptr;	// The name of movie.

public:
	int nMovieId = -1;
};

} // namespace Frame
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
