/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TINSEL_SCROLL_H // prevent multiple includes
#define TINSEL_SCROLL_H

namespace Tinsel {

#define MAX_HNOSCROLL 10
#define MAX_VNOSCROLL 10

// These structures defined here so boundaries can be saved
struct NOSCROLLB {
	int ln;
	int c1;
	int c2;
};

struct SCROLLDATA {
	NOSCROLLB NoVScroll[MAX_VNOSCROLL]; // Vertical no-scroll boundaries
	NOSCROLLB NoHScroll[MAX_HNOSCROLL]; // Horizontal no-scroll boundaries
	unsigned NumNoV, NumNoH;            // Counts of no-scroll boundaries
	// DW2 fields
	int xTrigger;
	int xDistance;
	int xSpeed;
	int yTriggerTop;
	int yTriggerBottom;
	int yDistance;
	int ySpeed;
};

class Scroll {
public:
	Scroll();

	void DontScrollCursor();
	void DoScrollCursor();

	void SetNoScroll(int x1, int y1, int x2, int y2);
	void DropScroll();

	void ScrollFocus(int actor);
	int GetScrollFocus();
	void ScrollTo(int x, int y, int xIter, int yIter);

	void KillScroll();

	void GetNoScrollData(SCROLLDATA *ssd);
	void RestoreNoScrollData(SCROLLDATA *ssd);

	void SetScrollParameters(int xTrigger, int xDistance, int xSpeed, int yTriggerTop,
	                         int yTriggerBottom, int yDistance, int ySpeed);

	bool IsScrolling();

	void ScrollImage();
	void MonitorScroll();

	void InitScroll(int width, int height);

private:
	void NeedScroll(int direction);
	void RestoreScrollDefaults();

	int _leftScroll, _downScroll; // Number of iterations outstanding

	int _scrollActor;
	PMOVER _pScrollMover;
	int _oldx, _oldy;

	/** Boundaries and numbers of boundaries */
	SCROLLDATA _scrollData;

	int _imageW, _imageH;

	bool _scrollCursor; // If a TAG or EXIT polygon is clicked on,
	    // the cursor is kept over that polygon
	    // whilst scrolling

	int _scrollPixelsX, _scrollPixelsY;
};

void ScrollProcess(CORO_PARAM, const void *);

} // End of namespace Tinsel

#endif /* TINSEL_SCROLL_H */
