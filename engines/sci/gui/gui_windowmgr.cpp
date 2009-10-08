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
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_gfx.h"
#include "sci/gui/gui_windowmgr.h"

namespace Sci {

// window styles
enum {
	kTransparent = (1 << 0),
	kNoFrame     = (1 << 1),
	kTitle       = (1 << 2),
	kTopmost     = (1 << 3),
	kUser        = (1 << 7)
};

SciGuiWindowMgr::SciGuiWindowMgr(EngineState *state, SciGuiScreen *screen, SciGuiGfx *gfx)
	: _s(state), _screen(screen), _gfx(gfx) {

	_wmgrPort = new GuiPort(1);
	_windowsById.resize(2);
	_windowsById[0] = 0;
	_windowsById[1] = _wmgrPort;

	int16 offTop = 20;

	_gfx->OpenPort(_wmgrPort);
	_gfx->SetPort(_wmgrPort);
	_gfx->SetOrigin(0, offTop);
	_wmgrPort->rect.bottom = 200 - offTop;
	_wmgrPort->rect.right = 320;
	_wmgrPort->rect.moveTo(0, 0);
	_wmgrPort->curTop = 0;
	_wmgrPort->curLeft = 0;
	_windowList.push_front(_wmgrPort);

	_picWind = NewWindow(Common::Rect(0, 10, _screen->_width, _screen->_height), 0, 0, kTransparent | kNoFrame, 0, true);
}

SciGuiWindowMgr::~SciGuiWindowMgr() {
	// TODO: Clear _windowList and delete all stuff in it?
}

int16 SciGuiWindowMgr::isFrontWindow(GuiWindow *pWnd) {
	return _windowList.back() == pWnd;
}

void SciGuiWindowMgr::BeginUpdate(GuiWindow *wnd) {
	GuiPort *oldPort = _gfx->SetPort(_wmgrPort);
	PortList::iterator it = _windowList.reverse_begin();
	const PortList::iterator end = Common::find(_windowList.begin(), _windowList.end(), wnd);
	while (it != end) {
		// FIXME: We also store GuiPort objects in the window list.
		// We should add a check that we really only pass windows here...
		UpdateWindow((GuiWindow *)*it);
		--it;
	}
	_gfx->SetPort(oldPort);
}

void SciGuiWindowMgr::EndUpdate(GuiWindow *wnd) {
	GuiPort *oldPort = _gfx->SetPort(_wmgrPort);
	const PortList::iterator end = _windowList.end();
	PortList::iterator it = Common::find(_windowList.begin(), end, wnd);
	while (it != end) {
		++it;
		// FIXME: We also store GuiPort objects in the window list.
		// We should add a check that we really only pass windows here...
		UpdateWindow((GuiWindow *)*it);
	}

	_gfx->SetPort(oldPort);
}

GuiWindow *SciGuiWindowMgr::NewWindow(const Common::Rect &dims, const Common::Rect *restoreRect, const char *title, uint16 style, uint16 arg8, bool draw) {
	// Find an unused window/port id
	uint id = 1;
	while (id < _windowsById.size() && _windowsById[id]) {
		++id;
	}
	if (id == _windowsById.size())
		_windowsById.push_back(0);
	assert(0 < id && id < 0xFFFF);

	GuiWindow *pwnd = new GuiWindow(id);
	Common::Rect r;

	if (!pwnd) {
		warning("Can't open window!");
		return 0;
	}

	_windowsById[id] = pwnd;
	if (style & kTopmost)
		_windowList.push_front(pwnd);
	else
		_windowList.push_back(pwnd);
	_gfx->OpenPort(pwnd);
	r = dims;
	pwnd->rect = dims;
	if (restoreRect)
		pwnd->restoreRect = *restoreRect;
	
	pwnd->wndStyle = style;
	pwnd->hSaved1 = pwnd->hSaved2 = NULL_REG;
	pwnd->bDrawn = false;
	if ((style & kTransparent) == 0)
		pwnd->uSaveFlag = (arg8 == 0xFFFF ? 1 : 3);
	
	if (title && (style & kTitle)) {
		pwnd->title = title;
	}
	
	r = dims;
	if (style == kUser || !(style & kNoFrame)) {
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

	// CHECKME: Is this 'kTransparent' check necessary?
	// The shadow is already drawn if !(wndStyle & (kUser | kNoFrame)).
	if (!(pwnd->wndStyle & (kTransparent | kUser | kNoFrame))) {
		// The shadow is drawn slightly outside the window.
		// Enlarge restoreRect to cover that.
		pwnd->restoreRect.bottom++;
		pwnd->restoreRect.right++;
	}
	
	if (draw)
		DrawWindow(pwnd);
	_gfx->SetPort((GuiPort *)pwnd);
	_gfx->SetOrigin(pwnd->rect.left, pwnd->rect.top + _wmgrPort->top);
	pwnd->rect.moveTo(0, 0);
	return pwnd;
}

void SciGuiWindowMgr::DrawWindow(GuiWindow *pWnd) {
	if (pWnd->bDrawn)
		return;
	Common::Rect r;
	int16 wndStyle = pWnd->wndStyle;

	pWnd->bDrawn = true;
	GuiPort *oldport = _gfx->SetPort(_wmgrPort);
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
	if (!(wndStyle & kUser)) {
		r = pWnd->dims;
		if (!(wndStyle & kNoFrame)) {
			r.translate(1, 1);
			_gfx->FrameRect(r);// shadow
			r.translate(-1, -1);
			_gfx->FrameRect(r);// window frame

			if (wndStyle & kTitle) {
				_gfx->FrameRect(r);
				r.grow(-1);
				_gfx->FillRect(r, 1, 0);
				if (!pWnd->title.empty()) {
					int16 oldcolor = _gfx->GetPort()->penClr;
					_gfx->PenColor(255);
					_gfx->TextBox(pWnd->title.c_str(), 1, r, 1, 0);
					_gfx->PenColor(oldcolor);
				}
				
				r = pWnd->dims;
				r.top += 9;
			}
			
			r.grow(-1);
		}
		
		if (!(wndStyle & kTransparent))
			_gfx->FillRect(r, 1, pWnd->backClr);

		_gfx->ShowBits(pWnd->dims, 1);
	}
	_gfx->SetPort(oldport);
}

void SciGuiWindowMgr::DisposeWindow(GuiWindow *pWnd, int16 arg2) {
	_gfx->SetPort(_wmgrPort);
	_gfx->RestoreBits(pWnd->hSaved1);
	_gfx->RestoreBits(pWnd->hSaved2);
	if (arg2)
		_gfx->ShowBits(pWnd->restoreRect, 1);
//	else
//		g_sci->ReAnimate(&pwnd->dims);
	_windowList.remove(pWnd);
	_gfx->SetPort(_windowList.back());
	_windowsById[pWnd->id] = 0;
	delete pWnd;
}

void SciGuiWindowMgr::UpdateWindow(GuiWindow *wnd) {
	GuiMemoryHandle handle;

	if (wnd->uSaveFlag && wnd->bDrawn) {
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

} // End of namespace Sci
