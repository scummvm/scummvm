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

#ifndef GRAPHICS_MACGUI_MACWINDOW_H
#define GRAPHICS_MACGUI_MACWINDOW_H

#include "graphics/managed_surface.h"
#include "graphics/transparent_surface.h"
#include "graphics/nine_patch.h"
#include "graphics/palette.h"

#include "graphics/macgui/macwindowborder.h"

namespace Graphics {

class MacWindowManager;
class MacWindowBorder;

namespace MacWindowConstants {
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
	kBorderScrollLeft,
	kBorderScrollRight,
	kBorderCloseButton,
	kBorderInner,
	kBorderBorder,
	kBorderResizeButton
};
}
using namespace MacWindowConstants;

/**
 * Abstract class that defines common functionality for all window classes.
 * It supports event callbacks and drawing.
 */
class BaseMacWindow {
public:
	/**
	 * Base constructor.
	 * @param id ID of the window.
	 * @param editable True if the window is editable (e.g. for resizing).
	 * @param wm Pointer to the MacWindowManager that owns the window.
	 */
	BaseMacWindow(int id, bool editable, MacWindowManager *wm);
	virtual ~BaseMacWindow() {}

	/**
	 * Accessor method for the complete dimensions of the window.
	 * @return Dimensions of the window (including border) relative to the WM's screen.
	 */
	const Common::Rect &getDimensions() { return _dims; }

	/**
	 * Accessor method to the id of the window.
	 * @return The id set in the constructor.
	 */
	int getId() { return _id; }

	/**
	 * Accessor method to the type of window.
	 * Each subclass must indicate it's type.
	 * @return The type of the window.
	 */
	WindowType getType() { return _type; }

	/**
	 * Accessor method to check whether the window is editable (e.g. for resizing).
	 * @return True if the window is editable as indicated in the constructor.
	 */
	bool isEditable() { return _editable; }

	/**
	 * Method to access the entire surface of the window (e.g. to draw an image).
	 * @return A pointer to the entire surface of the window.
	 */
	ManagedSurface *getSurface() { return &_surface; }

	/**
	 * Abstract method for indicating whether the window is active or inactive.
	 * Used by the WM to handle focus on windows, etc.
	 * @param active Desired state of the window.
	 */
	virtual void setActive(bool active) = 0;

	/**
	 * Method for marking the window for redraw.
	 * @param dirty True if the window needs to be redrawn.
	 */
	void setDirty(bool dirty) { _contentIsDirty = dirty; }

	/**
	 * Method called to draw the window into the target surface.
	 * This method is most often called by the WM, and relies on
	 * the window being marked as dirty unless otherwise specified.
	 * @param g Surface on which to draw the window.
	 * @param forceRedraw It's behavior depends on the subclass.
	 */
	virtual bool draw(ManagedSurface *g, bool forceRedraw = false) = 0;

	/**
	 * Method called by the WM when there is an event concerning the window.
	 * Note that depending on the subclass of the window, it might not be called
	 * if the window is not active.
	 * @param event Event to be processed.
	 * @return true If the event was successfully consumed and processed.
	 */
	virtual bool processEvent(Common::Event &event) = 0;

	virtual bool hasAllFocus() = 0;

	/**
	 * Set the callback that will be used when an event needs to be processed.
	 * @param callback A function pointer to a function that accepts:
	 *					- A WindowClick, the pert of the window that was clicked.
	 *					- The event to be processed.
	 *					- Any additional required data (e.g. the engine's GUI).
	 */
	void setCallback(bool (*callback)(WindowClick, Common::Event &, void *), void *data) { _callback = callback; _dataPtr = data; }

protected:
	int _id;
	WindowType _type;

	bool _editable;

	ManagedSurface _surface;
	bool _contentIsDirty;

	Common::Rect _dims;

	bool (*_callback)(WindowClick, Common::Event &, void *);
	void *_dataPtr;

	MacWindowManager *_wm;
};

class MacWindow : public BaseMacWindow {
public:
	MacWindow(int id, bool scrollable, bool resizable, bool editable, MacWindowManager *wm);
	virtual ~MacWindow();
	void move(int x, int y);
	void resize(int w, int h);
	void setDimensions(const Common::Rect &r);
	const Common::Rect &getInnerDimensions() { return _innerDims; }

	bool draw(ManagedSurface *g, bool forceRedraw = false);

	void setActive(bool active);
	bool isActive();

	void setTitle(Common::String &title) { _title = title; }
	void setHighlight(WindowClick highlightedPart);
	void setScroll(float scrollPos, float scrollSize);
	bool processEvent(Common::Event &event);
	bool hasAllFocus() { return _beingDragged || _beingResized; }

	void loadBorder(Common::SeekableReadStream &file, bool active, int lo, int ro, int to, int bo);
	//void setBorder(TransparentSurface &border, bool active);

	void setCloseable(bool closeable);

private:
	void drawBorder();
	void prepareBorderSurface(ManagedSurface *g);
	void drawSimpleBorder(ManagedSurface *g);
	void drawBorderFromSurface(ManagedSurface *g);
	void drawBox(ManagedSurface *g, int x, int y, int w, int h);
	void fillRect(ManagedSurface *g, int x, int y, int w, int h, int color);
	const Font *getTitleFont();
	void updateInnerDims();
	WindowClick isInBorder(int x, int y);

	bool isInCloseButton(int x, int y);
	bool isInResizeButton(int x, int y);
	WindowClick isInScroll(int x, int y);

private:
	ManagedSurface _borderSurface;
	ManagedSurface _composeSurface;

	MacWindowBorder _macBorder;

	bool _scrollable;
	bool _resizable;
	bool _active;
	bool _borderIsDirty;

	bool _closeable;

	int _borderWidth;

	bool _beingDragged, _beingResized;
	int _draggedX, _draggedY;

	WindowClick _highlightedPart;
	float _scrollPos, _scrollSize;

	Common::Rect _innerDims;

	Common::String _title;
};



} // End of namespace Graphics

#endif
