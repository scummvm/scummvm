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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/gui/gui_dbllist.h"

namespace Sci {

class SciGUIwindowMgr {
public:
	SciGUIwindowMgr(EngineState *state, SciGUIgfx *gfx);
	~SciGUIwindowMgr();

	int16 isFrontWindow(GUIWindow *wnd);
	void SelectWindow(HEAPHANDLE hh);
	void BeginUpdate(GUIWindow *wnd);
	void EndUpdate(GUIWindow *wnd);
	GUIWindow *NewWindow(const Common::Rect &dims, const Common::Rect *restoreRect, const char *title, uint16 style, uint16 arg8, uint16 argA);
	void DrawWindow(GUIWindow *wnd);
	void DisposeWindow(GUIWindow *pWnd, int16 arg2);
	void UpdateWindow(GUIWindow *wnd);

	GUIPort *_wmgrPort;
	GUIWindow *_picWind;

private:
	EngineState *_s;
	SciGUIgfx *_gfx;

	DblList windowList;
};

} // end of namespace Sci
