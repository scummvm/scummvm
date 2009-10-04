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

#include "common/util.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/tools.h"
#include "sci/gui/gui_gfx.h"
#include "sci/gui/gui_windowmgr.h"
#include "sci/gui/gui_memmgr.h"

namespace Sci {

namespace {
const Common::Rect s_picRect(0, 10, 320, 200);
} // end of anonymous namespace

// window styles
enum {
	kTransparent = (1 << 0),
	kNoFrame     = (1 << 1),
	kTitle       = (1 << 2),
	kTopmost     = (1 << 3),
	kUser        = (1 << 7)
};

SciGUIwindowMgr::SciGUIwindowMgr(EngineState *state, SciGUIgfx *gfx)
	: _s(state), _gfx(gfx) {

	// FIXME: remove memmgr
	InitMem(0x320);

	HEAPHANDLE wmgrPortH = heapNewPtr(sizeof(sciPort), kDataPort, "wmgrPort");
	heapClearPtr(wmgrPortH);
	_wmgrPort = (sciPort *)heap2Ptr(wmgrPortH);

	int16 offTop = 20;

	_gfx->OpenPort(_wmgrPort);
	_gfx->SetPort(_wmgrPort);
	_gfx->SetOrigin(0, offTop);
	_wmgrPort->rect.bottom = 200 - offTop;
	_wmgrPort->rect.right = 320;
	_wmgrPort->rect.moveTo(0, 0);
	_wmgrPort->curTop = 0;
	_wmgrPort->curLeft = 0;

	windowList.AddToFront(wmgrPortH);

	_picWind = NewWindow(s_picRect, 0, 0, kTransparent | kNoFrame, 0, 1);
}

SciGUIwindowMgr::~SciGUIwindowMgr() {
}

int16 SciGUIwindowMgr::isFrontWindow(sciWnd *pWnd) {
	if (heap2Ptr(windowList.getLast()) == (byte *)pWnd)
		return 1;
	return 0;
}

void SciGUIwindowMgr::SelectWindow(HEAPHANDLE hh) {
	sciPort *port = (sciPort *)heap2Ptr(hh);
	_gfx->SetPort(port);
	if (hh != windowList.getLast()) { // selecting not topmost window
		sciWnd *prevwnd = (sciWnd *)heap2Ptr(port->node.prev);
		BeginUpdate(prevwnd);
		windowList.MoveToEnd(hh);
		EndUpdate(prevwnd);
	}
	_gfx->SetPort(port);
}

void SciGUIwindowMgr::BeginUpdate(sciWnd *wnd) {
	sciPort *oldPort = _gfx->SetPort(_wmgrPort);
	sciWnd *node = (sciWnd *)heap2Ptr(windowList.getLast());
	while (node != wnd) {
		UpdateWindow(node);
		node = (sciWnd *)heap2Ptr(node->node.prev);
	}
	_gfx->SetPort(oldPort);
}

void SciGUIwindowMgr::EndUpdate(sciWnd *wnd) {
	sciPort *oldPort = _gfx->SetPort(_wmgrPort);
	sciWnd *last = (sciWnd *)heap2Ptr(windowList.getLast());
	while (wnd != last) {
		wnd = (sciWnd *)heap2Ptr(wnd->node.next);
		UpdateWindow(wnd);
	}
	_gfx->SetPort(oldPort);
}

sciWnd *SciGUIwindowMgr::NewWindow(const Common::Rect &dims, const Common::Rect *restoreRect, const char *title, uint16 style, uint16 arg8, uint16 argA) {
	HEAPHANDLE hWnd = heapNewPtr(sizeof(sciWnd), kDataWindow, title);
	Common::Rect r;

	if (!hWnd) {
		warning("Can't open window!");
		return 0;
	}
	heapClearPtr(hWnd);
	if (style & kTopmost)
		windowList.AddToFront(hWnd);
	else
		windowList.AddToEnd(hWnd);
	sciWnd *pwnd = (sciWnd *)heap2Ptr(hWnd);
	_gfx->OpenPort((sciPort *)pwnd);
	r = dims;
	pwnd->rect = dims;
	if (restoreRect)
		pwnd->restoreRect = *restoreRect;
	
	pwnd->wndStyle = style;
	pwnd->hSaved1 = pwnd->hSaved2 = NULL_REG;
	pwnd->bDrawed = false;
	if ((style & kTransparent) == 0)
		pwnd->uSaveFlag = (arg8 == 0xFFFF ? 1 : 3);
	
	if (title && (style & kTitle)) {
		HEAPHANDLE hTitle = heapNewPtr((uint16)strlen(title) + 1, kDataString, title);
		if (!hTitle) {
			warning("Can't open window!");
			return 0;
		}
		pwnd->hTitle = hTitle;
		strcpy((char *)heap2Ptr(hTitle), title);
	}
	
	r = dims;
	if (style == kUser || (style & kNoFrame) == 0) {
		r.grow(1);
		if (style & kTitle) {
			r.top -= 10;
			r.bottom++;
		}
	}
	
	pwnd->dims = r;
	const Common::Rect *wmprect = &_wmgrPort->rect;
	int16 oldtop = pwnd->dims.top;
	int16 oldleft = pwnd->dims.left;
	if (wmprect->top > pwnd->dims.top)
		pwnd->dims.moveTo(pwnd->dims.left, wmprect->top);
	
	if (wmprect->bottom < pwnd->dims.bottom)
		pwnd->dims.moveTo(pwnd->dims.left, wmprect->bottom
				- pwnd->dims.bottom + pwnd->dims.top);
	
	if (wmprect->right < pwnd->dims.right)
		pwnd->dims.moveTo(wmprect->right + pwnd->dims.left
				- pwnd->dims.right, pwnd->dims.top);
	
	if (wmprect->left > pwnd->dims.left)
		pwnd->dims.moveTo(wmprect->left, pwnd->dims.top);
	
	pwnd->rect.moveTo(pwnd->rect.left + pwnd->dims.left - oldleft,
			pwnd->rect.top + pwnd->dims.top - oldtop);
	if (restoreRect == 0)
		pwnd->restoreRect = pwnd->dims;
	
	if (argA)
		DrawWindow(pwnd);
	_gfx->SetPort((sciPort *)pwnd);
	_gfx->SetOrigin(pwnd->rect.left, pwnd->rect.top + _wmgrPort->top);
	pwnd->rect.moveTo(0, 0);
	return pwnd;
}

void SciGUIwindowMgr::DrawWindow(sciWnd *pWnd) {
	if (pWnd->bDrawed)
		return;
	Common::Rect r;
	int16 wndStyle = pWnd->wndStyle;

	pWnd->bDrawed = true;
	sciPort *oldport = _gfx->SetPort(_wmgrPort);
	_gfx->PenColor(0);
	if ((wndStyle & kTransparent) == 0) {
		pWnd->hSaved1 = _gfx->SaveBits(pWnd->restoreRect, 1);
		if (pWnd->uSaveFlag & 2) {
			pWnd->hSaved2 = _gfx->SaveBits(pWnd->restoreRect, 2);
			if ((wndStyle & kUser) == 0)
				_gfx->FillRect(pWnd->restoreRect, 2, 0, 0xF);
		}
	}
	
	// drawing frame,shadow and title
	if ((wndStyle & kUser) == 0) {
		r = pWnd->dims;
		if ((wndStyle & kNoFrame) == 0) {
			r.translate(1, 1);
			_gfx->FrameRect(r);// shadow
			r.translate(-1, -1);
			_gfx->FrameRect(r);// window frame
			if (wndStyle & kTitle) {
				_gfx->FrameRect(r);
				r.grow(-1);
				_gfx->FillRect(r, 1, 0);
				if (pWnd->hTitle) {
					int16 oldcolor = _gfx->GetPort()->penClr;
					_gfx->PenColor(255);
					_gfx->TextBox((char *)heap2Ptr(pWnd->hTitle), 1, r, 1, 0);
					_gfx->PenColor(oldcolor);
				}
				
				r = pWnd->dims;
				r.top += 9;
			}
			
			r.grow(-1);
		}
		
		if ((wndStyle & kTransparent) == 0)
			_gfx->FillRect(r, 1, pWnd->backClr);
		_gfx->ShowBits(pWnd->dims, 1);
	}
	_gfx->SetPort(oldport);
}

void SciGUIwindowMgr::DisposeWindow(sciWnd *pWnd, int16 arg2) {
	_gfx->SetPort(_wmgrPort);
	_gfx->RestoreBits(pWnd->hSaved1);
	_gfx->RestoreBits(pWnd->hSaved2);
	if (arg2)
		_gfx->ShowBits(pWnd->restoreRect, 1);
//	else
//		g_sci->ReAnimate(&pwnd->dims);
	HEAPHANDLE hh = ptr2heap((byte *)pWnd);
	windowList.DeleteNode(hh);
	SelectWindow(windowList.getLast());
	if (pWnd->hTitle)
		heapDisposePtr(pWnd->hTitle);
	heapDisposePtr(hh);
}

void SciGUIwindowMgr::UpdateWindow(sciWnd *wnd) {
	sciMemoryHandle handle;

	if (wnd->uSaveFlag && wnd->bDrawed) {
		handle = _gfx->SaveBits(wnd->restoreRect, 1);
		_gfx->RestoreBits(wnd->hSaved1);
		wnd->hSaved1 = handle;
		if (wnd->uSaveFlag & 2) {
			handle = _gfx->SaveBits(wnd->restoreRect, 2);
			_gfx->RestoreBits(wnd->hSaved2);
			wnd->hSaved2 = handle;
		}
	}
}

} // end of namespace Sci
