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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_RENDEROBJECT_H
#define NANCY_RENDEROBJECT_H

#include "common/path.h"
#include "graphics/managed_surface.h"

namespace Nancy {

class NancyEngine;
class GraphicsManager;

// Nancy21 can zoom into the viewport, with everything in some of the streams
// of scenes: what is drawn gets scaled around the top left of the viewport, then
// moved by the offset, in viewport pixels
struct ViewTransform {
	double scaleX = 1.0;
	double scaleY = 1.0;
	double offsetX = 0.0;
	double offsetY = 0.0;

	bool isIdentity() const { return scaleX == 1.0 && scaleY == 1.0 && offsetX == 0.0 && offsetY == 0.0; }
};

// Loosely equivalent to the original engine's ZRenderStructs.
// A subclass of this will be automatically updated and drawn from the graphics manager,
// but initialization needs to be done manually.
class RenderObject {
public:
	RenderObject(uint16 zOrder);
	RenderObject(uint16 zOrder, Graphics::ManagedSurface &surface, const Common::Rect &srcBounds, const Common::Rect &destBounds);

	virtual ~RenderObject();

	RenderObject(RenderObject &&) = default;

	virtual void init(); // Does not get called automatically
	virtual void registerGraphics(); // Does not get called automatically
	virtual void updateGraphics() {}

	void moveTo(const Common::Point &position);
	void moveTo(const Common::Rect &bounds);
	// Without forceRedraw, an object that stays shown or hidden isn't drawn again
	void setVisible(bool visible, bool forceRedraw = true);
	void setTransparent(bool isTransparent);
	bool isVisible() const { return _isVisible; }

	// Lights, Camera, Curses draws some things into the stencil buffer only:
	// a stencil object shows nothing itself, but a masked object drawn after it
	// leaves out the pixels the stencil object isn't transparent at
	void setStencil(bool isStencil);
	bool isStencil() const { return _isStencil; }
	void setMasked(bool isMasked);
	bool isMasked() const { return _isMasked; }

	// Nancy26: the part of the screen the object gets drawn in; empty for all of it
	void setClipRect(const Common::Rect &clip);
	const Common::Rect &getClipRect() const { return _clipRect; }

	// Where on the screen the object was the last time it got drawn; empty when it
	// wasn't shown. The viewport can scroll since, which moves where the object
	// would be now, but not what is on the screen.
	const Common::Rect &getDrawnScreenPosition() const { return _drawnScreenPosition; }
	void setDrawnScreenPosition(const Common::Rect &position) { _drawnScreenPosition = position; }

	bool needsRedraw() const { return _needsRedraw; }
	void setNeedsRedraw(bool needsRedraw) { _needsRedraw = needsRedraw; }

	void setHasMoved(bool hasMoved) { _hasMoved = hasMoved; }

	// Nancy16 scales the game up to fit the screen; these objects get drawn at the
	// screen's own resolution instead, and are positioned in screen coordinates
	bool isNativeResolution() const { return _isNativeResolution; }
	void setNativeResolution(bool isNative) { _isNativeResolution = isNative; }

	// Only used by The Vampire Diaries
	void grabPalette(byte *colors, uint paletteStart = 0, uint paletteSize = 256);
	void setPalette(const Common::Path &paletteName, uint paletteStart = 0, uint paletteSize = 256);
	void setPalette(const byte *colors, uint paletteStart = 0, uint paletteSize = 256);

	bool hasMoved() const { return _previousScreenPosition != _screenPosition; }
	void updatePreviousScreenPosition() { _previousScreenPosition = _screenPosition; }

	Common::Rect getScreenPosition() const;
	Common::Rect getScreenPositionRaw() const { return _screenPosition; }
	Common::Rect getPreviousScreenPosition() const;

	// Given a screen-space rect, convert it to the _drawSurface's local space
	Common::Rect convertToLocal(const Common::Rect &screen) const;
	// Given a local (to the _drawSurface) space rect, convert it to screen space
	Common::Rect convertToScreen(const Common::Rect &rect) const;

	Common::Rect getBounds() const { return Common::Rect(_screenPosition.width(), _screenPosition.height()); }
	uint16 getZOrder() const { return _z; }

	// What decides between objects of the same z order: the records of the
	// scripts draw in the order they were made. 0 for everything else.
	uint32 getTieOrder() const { return _tieOrder; }
	void setTieOrder(uint32 order) { _tieOrder = order; }

	// What comes before the z order: Nancy19 to Nancy20 draw each stream in a
	// layer of its own. The UI is in front of all of them.
	static const int32 kUILayer = 0x7FFFFFFF;
	int32 getLayer() const { return _layer; }
	void setLayer(int32 layer);

	// Nancy19 to Nancy20: the z of a record of the scripts, which tells apart the
	// records the z order can't, e.g. the many farther than the background.
	// Everything else is farther than all records of its z order.
	static const double kNoExactZ;
	double getExactZ() const { return _exactZ; }
	void setExactZ(double z);

	// Also moves the object in the draw order, so changing the z of a registered
	// object takes effect without re-registering it
	void setZOrder(uint16 z);

	Graphics::ManagedSurface _drawSurface;

	// Needed for proper handling of objects inside the viewport
	virtual bool isViewportRelative() const { return false; }

	// Whether an object in the viewport moves with its vertical scroll. The
	// records of the scripts are placed by their transforms instead.
	virtual bool isMovedByViewportScroll() const { return true; }

	// The zoom the object gets drawn with, if any. It belongs to whoever set it.
	const ViewTransform *getViewTransform() const { return _viewTransform; }
	void setViewTransform(const ViewTransform *viewTransform) { _viewTransform = viewTransform; }
	bool isZoomed() const { return _viewTransform && !_viewTransform->isIdentity(); }

protected:
	bool _needsRedraw;
	bool _isVisible;
	bool _hasMoved;
	bool _isNativeResolution = false;
	bool _isStencil = false;
	bool _isMasked = false;
	uint16 _z;
	uint32 _tieOrder = 0;
	int32 _layer = 0;
	double _exactZ = kNoExactZ;
	Common::Rect _drawnScreenPosition;
	Common::Rect _previousScreenPosition;
	Common::Rect _screenPosition;
	Common::Rect _clipRect;
	const ViewTransform *_viewTransform = nullptr;

private:
	Common::Rect convertPositionToScreen(const Common::Rect &position) const;
};

} // End of namespace Nancy

#endif // NANCY_RENDEROBJECT_H
