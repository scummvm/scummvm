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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef WAGE_MACWINDOW_H
#define WAGE_MACWINDOW_H

#include "graphics/managed_surface.h"

namespace Graphics {
	class MacWindowManager;
}

namespace Wage {

enum WindowType {
	kWindowUnknown,
	kWindowWindow,
	kWindowMenu
};

enum {
	kBorderWidth = 17
};

enum WindowClick {
	kBorderNone = 0,
	kBorderScrollUp,
	kBorderScrollDown,
	kBorderCloseButton,
	kBorderInner,
	kBorderBorder,
	kBorderResizeButton
};

class BaseMacWindow {
public:
	BaseMacWindow(int id, bool editable, Graphics::MacWindowManager *wm);
	virtual ~BaseMacWindow() {}

	const Common::Rect &getDimensions() { return _dims; }
	int getId() { return _id; }
	WindowType getType() { return _type; }
	bool isEditable() { return _editable; }
	Graphics::ManagedSurface *getSurface() { return &_surface; }
	virtual void setActive(bool active) = 0;
	void setDirty(bool dirty) { _contentIsDirty = dirty; }

	virtual bool draw(Graphics::ManagedSurface *g, bool forceRedraw = false) = 0;
	virtual bool processEvent(Common::Event &event) = 0;

	virtual bool hasAllFocus() = 0;

	void setCallback(bool (*callback)(WindowClick, Common::Event &, void *), void *data) { _callback = callback; _dataPtr = data; }

protected:
	int _id;
	WindowType _type;

	bool _editable;

	Graphics::ManagedSurface _surface;
	bool _contentIsDirty;

	Common::Rect _dims;

	bool (*_callback)(WindowClick, Common::Event &, void *);
	void *_dataPtr;

	Graphics::MacWindowManager *_wm;
};

class MacWindow : public BaseMacWindow {
public:
	MacWindow(int id, bool scrollable, bool resizable, bool editable, Graphics::MacWindowManager *wm);
	virtual ~MacWindow();
	void move(int x, int y);
	void resize(int w, int h);
	void setDimensions(const Common::Rect &r);
	const Common::Rect &getInnerDimensions() { return _innerDims; }

	bool draw(Graphics::ManagedSurface *g, bool forceRedraw = false);

	void setActive(bool active);
	void setTitle(Common::String &title) { _title = title; }
	void setHighlight(WindowClick highlightedPart);
	void setScroll(float scrollPos, float scrollSize);
	bool processEvent(Common::Event &event);
	bool hasAllFocus() { return _beingDragged || _beingResized; }

private:
	void drawBorder();
	void drawBox(Graphics::ManagedSurface *g, int x, int y, int w, int h);
	void fillRect(Graphics::ManagedSurface *g, int x, int y, int w, int h, int color);
	const Graphics::Font *getTitleFont();
	void updateInnerDims();
	WindowClick isInBorder(int x, int y);

private:
	Graphics::ManagedSurface _borderSurface;
	Graphics::ManagedSurface _composeSurface;
	bool _scrollable;
	bool _resizable;
	bool _active;
	bool _borderIsDirty;

	bool _beingDragged, _beingResized;
	int _draggedX, _draggedY;

	WindowClick _highlightedPart;
	float _scrollPos, _scrollSize;

	Common::Rect _innerDims;

	Common::String _title;
};

} // End of namespace Wage

#endif
