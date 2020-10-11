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

#ifndef GLK_WINDOW_GRAPHICS_H
#define GLK_WINDOW_GRAPHICS_H

#include "glk/windows.h"
#include "glk/picture.h"

namespace Glk {

/**
 * Graphics window
 */
class GraphicsWindow : public Window {
private:
	void touch();

	void drawPicture(Picture *src, int x0, int y0, int width, int height, uint linkval);
public:
	uint _bgnd;
	bool _dirty;
	uint _w, _h;
	Graphics::ManagedSurface *_surface;
public:
	/**
	 * Constructor
	 */
	GraphicsWindow(Windows *windows, uint rock);

	/**
	 * Destructor
	 */
	~GraphicsWindow() override;

	bool drawPicture(const Common::String &image, int xpos, int ypos, bool scale,
	                   uint imagewidth, uint imageheight);
	void drawPicture(const Graphics::Surface &image, uint transColor, int x0, int y0,
		int width, int height, uint linkval = 0);

	/**
	 * Rearranges the window
	 */
	void rearrange(const Rect &box) override;

	/**
	 * Get window split size within parent pair window
	 */
	uint getSplit(uint size, bool vertical) const override {
		return size;
	}

	/**
	 * Click the window
	 */
	void click(const Point &newPos) override;

	/**
	 * Cancel a mouse event
	 */
	void cancelMouseEvent() override {
		_mouseRequest = false;
	}

	/**
	 * Cancel a hyperlink event
	 */
	void cancelHyperlinkEvent() override {
		_hyperRequest = false;
	}

	void requestMouseEvent() override {
		_mouseRequest = true;
	}

	void requestHyperlinkEvent() override {
		_hyperRequest = true;
	}

	/**
	 * Redraw the window
	 */
	void redraw() override;

	void eraseRect(bool whole, const Rect &box) override;

	/**
	 * Fill an area of the window
	 */
	void fillRect(uint color, const Rect &box) override;

	/**
	 * Clear the window
	 */
	virtual void clear() override;

	/**
	 * Draw a rectangle in the given area
	 */
	void frameRect(uint color, const Rect &box);

	/**
	 * Draws a line between two points
	 */
	void drawLine(uint color, const Point &from, const Point &to);

	void getSize(uint *width, uint *height) const override;

	void setBackgroundColor(uint color) override;
};

} // End of namespace Glk

#endif
