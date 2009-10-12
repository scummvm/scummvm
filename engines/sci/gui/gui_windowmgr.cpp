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
	SCI_WINDOWMGR_STYLE_TRANSPARENT = (1 << 0),
	SCI_WINDOWMGR_STYLE_NOFRAME     = (1 << 1),
	SCI_WINDOWMGR_STYLE_TITLE       = (1 << 2),
	SCI_WINDOWMGR_STYLE_TOPMOST     = (1 << 3),
	SCI_WINDOWMGR_STYLE_USER        = (1 << 7)
};

SciGuiWindowMgr::SciGuiWindowMgr(SciGuiScreen *screen, SciGuiGfx *gfx)
	: _screen(screen), _gfx(gfx) {

	_wmgrPort = new GuiPort(1);
	_windowsById.resize(2);
	_windowsById[0] = 0;
	_windowsById[1] = _wmgrPort;

	int16 offTop = 10;

	_gfx->OpenPort(_wmgrPort);
	_gfx->SetPort(_wmgrPort);
	_gfx->SetOrigin(0, offTop);
	_wmgrPort->rect.bottom = 200 - offTop;
	_wmgrPort->rect.right = 320;
	_wmgrPort->rect.moveTo(0, 0);
	_wmgrPort->curTop = 0;
	_wmgrPort->curLeft = 0;
	_windowList.push_front(_wmgrPort);

	_picWind = NewWindow(Common::Rect(0, 10, _screen->_width, _screen->_height), 0, 0, SCI_WINDOWMGR_STYLE_TRANSPARENT | SCI_WINDOWMGR_STYLE_NOFRAME, 0, true);
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

	// wnd has to be in _windowList
	assert(it != end);

	while (++it != end) {
		// FIXME: We also store GuiPort objects in the window list.
		// We should add a check that we really only pass windows here...
		UpdateWindow((GuiWindow *)*it);
	}

	_gfx->SetPort(oldPort);
}

GuiWindow *SciGuiWindowMgr::NewWindow(const Common::Rect &dims, const Common::Rect *restoreRect, const char *title, uint16 style, int16 priority, bool draw) {
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
	if (style & SCI_WINDOWMGR_STYLE_TOPMOST)
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
	if ((style & SCI_WINDOWMGR_STYLE_TRANSPARENT) == 0)
		pwnd->uSaveFlag = (priority == -1 ? SCI_SCREEN_MASK_VISUAL : SCI_SCREEN_MASK_VISUAL | SCI_SCREEN_MASK_PRIORITY);
	
	if (title && (style & SCI_WINDOWMGR_STYLE_TITLE)) {
		pwnd->title = title;
	}
	
	r = dims;
	if (style == SCI_WINDOWMGR_STYLE_USER || !(style & SCI_WINDOWMGR_STYLE_NOFRAME)) {
		r.grow(1);
		if (style & SCI_WINDOWMGR_STYLE_TITLE) {
			r.top -= 10;
			r.bottom++;
		}
	}

	// FIXME: it seems as if shadows may result in the window getting moved one upwards
	//         so that the shadow is visible (lsl5)
	
	pwnd->dims = r;
	const Common::Rect *wmprect = &_wmgrPort->rect;
	int16 oldtop = pwnd->dims.top;
	int16 oldleft = pwnd->dims.left;
	if (wmprect->top > pwnd->dims.top)
		pwnd->dims.moveTo(pwnd->dims.left, wmprect->top);
	
	if (wmprect->bottom < pwnd->dims.bottom)
		pwnd->dims.moveTo(pwnd->dims.left, wmprect->bottom - pwnd->dims.bottom + pwnd->dims.top);
	
	if (wmprect->right < pwnd->dims.right)
		pwnd->dims.moveTo(wmprect->right + pwnd->dims.left - pwnd->dims.right, pwnd->dims.top);
	
	if (wmprect->left > pwnd->dims.left)
		pwnd->dims.moveTo(wmprect->left, pwnd->dims.top);
	
	pwnd->rect.moveTo(pwnd->rect.left + pwnd->dims.left - oldleft, pwnd->rect.top + pwnd->dims.top - oldtop);
	if (restoreRect == 0)
		pwnd->restoreRect = pwnd->dims;

	if (!(pwnd->wndStyle & (SCI_WINDOWMGR_STYLE_USER | SCI_WINDOWMGR_STYLE_NOFRAME))) {
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
	if ((wndStyle & SCI_WINDOWMGR_STYLE_TRANSPARENT) == 0) {
		pWnd->hSaved1 = _gfx->BitsSave(pWnd->restoreRect, SCI_SCREEN_MASK_VISUAL);
		if (pWnd->uSaveFlag & 2) {
			pWnd->hSaved2 = _gfx->BitsSave(pWnd->restoreRect, 2);
			if ((wndStyle & SCI_WINDOWMGR_STYLE_USER) == 0)
				_gfx->FillRect(pWnd->restoreRect, SCI_SCREEN_MASK_PRIORITY, 0, 15);
		}
	}
	
	// drawing frame,shadow and title
	if (!(wndStyle & SCI_WINDOWMGR_STYLE_USER)) {
		r = pWnd->dims;
		if (!(wndStyle & SCI_WINDOWMGR_STYLE_NOFRAME)) {
			r.translate(1, 1);
			_gfx->FrameRect(r);// shadow
			r.translate(-1, -1);
			_gfx->FrameRect(r);// window frame

			if (wndStyle & SCI_WINDOWMGR_STYLE_TITLE) {
				_gfx->FrameRect(r);
				r.grow(-1);
				_gfx->FillRect(r, SCI_SCREEN_MASK_VISUAL, 0);
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
		
		if (!(wndStyle & SCI_WINDOWMGR_STYLE_TRANSPARENT))
			_gfx->FillRect(r, SCI_SCREEN_MASK_VISUAL, pWnd->backClr);

		_gfx->BitsShow(pWnd->restoreRect);
	}
	_gfx->SetPort(oldport);
}

void SciGuiWindowMgr::DisposeWindow(GuiWindow *pWnd, int16 arg2) {
	_gfx->SetPort(_wmgrPort);
	_gfx->BitsRestore(pWnd->hSaved1);
	_gfx->BitsRestore(pWnd->hSaved2);
	if (arg2)
		_gfx->BitsShow(pWnd->restoreRect);
	else
		_gfx->BitsShow(pWnd->restoreRect); // FIXME: dummy, should be ReAnimate(&pwnd->dims);
	_windowList.remove(pWnd);
	_gfx->SetPort(_windowList.back());
	_windowsById[pWnd->id] = 0;
	delete pWnd;
}

void SciGuiWindowMgr::UpdateWindow(GuiWindow *wnd) {
	GuiMemoryHandle handle;

	if (wnd->uSaveFlag && wnd->bDrawn) {
		handle = _gfx->BitsSave(wnd->restoreRect, SCI_SCREEN_MASK_VISUAL);
		_gfx->BitsRestore(wnd->hSaved1);
		wnd->hSaved1 = handle;
		if (wnd->uSaveFlag & SCI_SCREEN_MASK_PRIORITY) {
			handle = _gfx->BitsSave(wnd->restoreRect, SCI_SCREEN_MASK_PRIORITY);
			_gfx->BitsRestore(wnd->hSaved2);
			wnd->hSaved2 = handle;
		}
	}
}

} // End of namespace Sci
