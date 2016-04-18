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

namespace Wage {

enum WindowType {
	kWindowScene,
	kWindowConsole
};

enum {
	kBorderWidth = 17
};

enum BorderHighlight {
	kBorderNone = 0,
	kBorderScrollUp,
	kBorderScrollDown,
	kBorderCloseButton
};

class MacWindow {
public:
	MacWindow(bool scrollable);
	~MacWindow();
	void move(int x, int y);
	void resize(int w, int h);
	void setDimensions(const Common::Rect &r);
	const Common::Rect &getDimensions() { return _dims; }
	bool draw(Graphics::ManagedSurface *g, bool forceRedraw = false);
	void setActive(bool active);
	Graphics::ManagedSurface *getSurface() { return &_surface; }
	void setTitle(Common::String &title) { _title = title; }
	void setHighlight(BorderHighlight highlightedPart) { _highlightedPart = highlightedPart; }
	void setScroll(float scrollPos, float scrollSize) { _scrollPos = scrollPos; _scrollSize = scrollSize; }
	void setDirty(bool dirty) { _contentIsDirty = dirty; }

private:
	void drawBorder();
	void drawBox(Graphics::ManagedSurface *g, int x, int y, int w, int h);
	void fillRect(Graphics::ManagedSurface *g, int x, int y, int w, int h, int color = kColorBlack);
	const Graphics::Font *getTitleFont();
	bool builtInFonts();

private:
	Graphics::ManagedSurface _surface;
	Graphics::ManagedSurface _borderSurface;
	Graphics::ManagedSurface _composeSurface;
	bool _scrollable;
	bool _active;
	bool _borderIsDirty;
	bool _contentIsDirty;

	BorderHighlight _highlightedPart;
	float _scrollPos, _scrollSize;

	Common::Rect _dims;

	Common::String _title;
};

} // End of namespace Wage

#endif
