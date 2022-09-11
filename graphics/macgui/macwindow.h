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

#ifndef GRAPHICS_MACGUI_MACWINDOW_H
#define GRAPHICS_MACGUI_MACWINDOW_H

#include "common/stream.h"

#include "graphics/managed_surface.h"
#include "graphics/transparent_surface.h"
#include "graphics/nine_patch.h"
#include "graphics/palette.h"
#include "graphics/font.h"

#include "graphics/macgui/macwidget.h"
#include "graphics/macgui/macwindowborder.h"

namespace Graphics {

class MacWindowManager;
class MacWindowBorder;
class MacWidget;

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

enum {
	kWindowModeDynamicScrollbar = 1 << 0
};
}
using namespace MacWindowConstants;

/**
 * Abstract class that defines common functionality for all window classes.
 * It supports event callbacks and drawing.
 */
class BaseMacWindow : public MacWidget {
public:
	/**
	 * Base constructor.
	 * @param id ID of the window.
	 * @param editable True if the window is editable.
	 * @param wm Pointer to the MacWindowManager that owns the window.
	 */
	BaseMacWindow(int id, bool editable, MacWindowManager *wm);
	virtual ~BaseMacWindow() {}

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
	 * Mutator to change the visible state of the window.
	 * @param visible Target state.
	 */
	virtual void setVisible(bool visible, bool silent = false);
	/**
	 * Accessor to determine whether a window is active.
	 * @return True if the window is active.
	 */
	bool isVisible();

	/**
	 * Method to access the entire interior surface of the window (e.g. to draw an image).
	 * @return A pointer to the entire interior surface of the window.
	 */
	ManagedSurface *getWindowSurface() { return _composeSurface; }

	/**
	 * Method to access the border surface of the window.
	 * @return A pointer to the border surface of the window.
	 */
	virtual ManagedSurface *getBorderSurface() = 0;

	/**
	 * Accessor to retrieve the dimensions of the inner surface of the window
	 * (i.e. without taking borders into account).
	 * Note that the returned dimensions' position is relative to the WM's
	 * screen, just like in getDimensions().
	 * @return The inner dimensions of the window.
	 */
	virtual const Common::Rect &getInnerDimensions() = 0;

	/**
	 * Method called to internally draw the window. This relies on the window
	 * being marked as dirty unless otherwise specified.
	 * @param forceRedraw Its behavior depends on the subclass.
	 */
	virtual bool draw(bool forceRedraw = false) = 0;

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

	/**
	 * Method that checks if the window is needs redrawing.
	 */
	virtual bool isDirty() = 0;

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

	bool (*_callback)(WindowClick, Common::Event &, void *);
	void *_dataPtr;

	bool _visible;
};

/**
 * An implementation of an ordinary window in the Mac interface.
 * It supports custom resizing, scrolling, borders, etc.
 */
class MacWindow : public BaseMacWindow {
public:
	/**
	 * Construct a simple window, with the default settings.
	 * Note that the scroll must be implemented in the event handling,
	 * even if the scrollable flag is set to true.
	 * @param id See BaseMacWindow.
	 * @param scrollable True if the window can be scrolled.
	 * @param resizable True if the window can be resized.
	 * @param editable See BaseMacWindow.
	 * @param wm See BaseMacWindow.
	 */
	MacWindow(int id, bool scrollable, bool resizable, bool editable, MacWindowManager *wm);
	virtual ~MacWindow() {}

	/**
	 * Change the window's location to fixed coordinates (not delta).
	 * @param x New left position of the window relative to the WM's screen.
	 * @param y New top position of the window relative to the WM's screen.
	 */
	void move(int x, int y);

	/*
	 * Change the width and the height of the window.
	 * @param w New width of the window.
	 * @param h New height of the window.
	 * @param inner True to set the inner dimensions.
	 */
	virtual void resize(int w, int h, bool inner = false);

	/**
	 * Change the dimensions of the window ([0, 0, 0, 0] by default).
	 * Note that this can be used to update both the position and the size
	 * of the window, although move() and resize() might be more comfortable.
	 * @param r The desired dimensions of the window.
	 */
	void setDimensions(const Common::Rect &r) override;

	/**
	 * Set a background pattern for the window.
	 * @param pattern
	 */
	void setBackgroundPattern(int pattern);

	/**
	 * Similar to that described in BaseMacWindow.
	 * @param g See BaseMacWindow.
	 * @param forceRedraw If true, the borders are guarranteed to redraw.
	 */
	bool draw(ManagedSurface *g, bool forceRedraw = false) override;

	bool draw(bool forceRedraw = false) override;
	void blit(ManagedSurface *g, Common::Rect &dest) override;

	const Common::Rect &getInnerDimensions() override { return _innerDims; }
	ManagedSurface *getBorderSurface() override { return &_borderSurface; }

	/**
	 * Centers the window using the dimensions of the parent window manager, or undoes this; does
	 * nothing if WM is null.
	 */
	void center(bool toCenter = true);

	/**
	 * Mutator to change the active state of the window.
	 * Most often called from the WM.
	 * @param active Target state.
	 */
	void setActive(bool active) override;
	/**
	 * Accessor to determine whether a window is active.
	 * @return True if the window is active.
	 */
	bool isActive() const;

	/**
	 * Mutator to change the title of the window.
	 * @param title Target title.
	 */
	void setTitle(const Common::String &title);
	/**
	 * Accessor to get the title of the window.
	 * @return Title.
	 */
	const Common::String &getTitle() const { return _title; };

	/**
	 * Highlight the target part of the window.
	 * Used for the default borders.
	 * @param highlightedPart Part to be highlighted.
	 */
	void setHighlight(WindowClick highlightedPart);
	/**
	 * Set the scroll poisition.
	 * @param scrollPos Target scroll position.
	 * @param scrollSize Size of the scrolling bar.
	 */
	void setScroll(float scrollPos, float scrollSize);
	/**
	 * See BaseMacWindow.
	 */
	bool processEvent(Common::Event &event) override;
	bool hasAllFocus() override { return _beingDragged || _beingResized; }

	/**
	 * Set arbitrary border from a BMP data stream, with custom border offsets.
	 * Note that the BMP has to be 9patch compliant. For examples, go to:
	 * https://github.com/blorente/MacVenture-Extract-Guide/tree/master/borders
	 * @param file The BMP data stream with the desired border.
	 * @param active Whether the border corresponds with the active state of the window.
	 * @param lo Width of the left side of the border, in pixels.
	 * @param ro Width of the right side of the border, in pixels.
	 * @param to Width of the top side of the border, in pixels.
	 * @param bo Width of the bottom side of the border, in pixels.
	 */
	void loadBorder(Common::SeekableReadStream &file, uint32 flags, int lo = -1, int ro = -1, int to = -1, int bo = -1);
	void loadBorder(Common::SeekableReadStream &file, uint32 flags, BorderOffsets offsets);
	void setBorder(Graphics::TransparentSurface *surface, uint32 flags, BorderOffsets offsets);
	void disableBorder();
	void loadWin95Border(const Common::String &filename, uint32 flags);
	/**
	 * we better set this before we load the border
	 * @param scrollbar state
	 */
	void enableScrollbar(bool active) { _hasScrollBar = active; }

	/**
	 * Indicate whether the window can be closed (false by default).
	 * @param closeable True if the window can be closed.
	 */
	void setCloseable(bool closeable);

	/**
	 * Mutator to change the border type.
	 * @param borderType Border type.
	 */
	void setBorderType(int borderType);
	/**
	 * Accessor to get the border type.
	 * @return Border type.
	 */
	int getBorderType() const { return _borderType; };

	/**
	 * We should call this method whenever we need border flags
	 * don't calc border flags yourself
	 * @return Border flags
	 */
	uint32 getBorderFlags() const;

	void addDirtyRect(const Common::Rect &r);
	void markAllDirty();
	void mergeDirtyRects();

	bool isDirty() override { return _borderIsDirty || _contentIsDirty; }

	void setBorderDirty(bool dirty) { _borderIsDirty = true; }
	void resizeBorderSurface();

	void setMode(uint32 mode) { _mode = mode; }
	void setBorderOffsets(BorderOffsets &offsets) { _macBorder.setOffsets(offsets); }

	void updateInnerDims();

private:
	void drawBorderFromSurface(ManagedSurface *g, uint32 flags);
	void drawPattern();
	void drawBox(ManagedSurface *g, int x, int y, int w, int h);
	void fillRect(ManagedSurface *g, int x, int y, int w, int h, int color);
	const Font *getTitleFont();
	void updateOuterDims();

	bool isInCloseButton(int x, int y) const;
	bool isInResizeButton(int x, int y) const;
	WindowClick isInScroll(int x, int y) const;

protected:
	void drawBorder();
	WindowClick isInBorder(int x, int y) const;
	BorderOffsets getBorderOffsets() const { return _macBorder.getOffset(); }

protected:
	ManagedSurface _borderSurface;

	bool _borderIsDirty;
	Common::Rect _innerDims;

	Common::List<Common::Rect> _dirtyRects;
	bool _hasScrollBar;

	uint32 _mode;
private:
	MacWindowBorder _macBorder;

	int _pattern;
	bool _hasPattern;

	bool _scrollable;
	bool _resizable;

	bool _closeable;

	int _borderWidth;

	bool _beingDragged, _beingResized;
	int _draggedX, _draggedY;

	WindowClick _highlightedPart;

	Common::String _title;

	int _borderType;
};

} // End of namespace Graphics

#endif
