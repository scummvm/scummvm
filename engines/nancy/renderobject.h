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

#include "graphics/managed_surface.h"

namespace Nancy {

class NancyEngine;
class GraphicsManager;

// Loosely equivalent to the original engine's ZRenderStructs.
// A subclass of this will be automatically updated and drawn from the graphics manager,
// but initialization needs to be done manually.
class RenderObject {
	friend class GraphicsManager;
public:
	RenderObject(uint16 zOrder);
	RenderObject(uint16 zOrder, Graphics::ManagedSurface &surface, const Common::Rect &srcBounds, const Common::Rect &destBounds);

	virtual ~RenderObject();

	virtual void init(); // Does not get called automatically
	virtual void registerGraphics(); // Does not get called automatically
	virtual void updateGraphics() {}

	void moveTo(const Common::Point &position);
	void moveTo(const Common::Rect &bounds);
	void setVisible(bool visible);
	void setTransparent(bool isTransparent);

	// Only used by The Vampire Diaries
	void grabPalette(byte *colors, uint paletteStart = 0, uint paletteSize = 256);
	void setPalette(const Common::String &paletteName, uint paletteStart = 0, uint paletteSize = 256);
	void setPalette(const byte *colors, uint paletteStart = 0, uint paletteSize = 256);

	bool hasMoved() const { return _previousScreenPosition != _screenPosition; }
	Common::Rect getScreenPosition() const;
	Common::Rect getPreviousScreenPosition() const;

	// Given a screen-space rect, convert it to the _drawSurface's local space
	Common::Rect convertToLocal(const Common::Rect &screen) const;
	// Given a local (to the _drawSurface) space rect, convert it to screen space
	Common::Rect convertToScreen(const Common::Rect &rect) const;

	Common::Rect getBounds() const { return Common::Rect(_screenPosition.width(), _screenPosition.height()); }

	Graphics::ManagedSurface _drawSurface;

protected:
	// Z order and blit type are extracted directly from the corresponding
	// ZRenderStruct from the original engine
	uint16 getZOrder() const { return _z; }

	// Needed for proper handling of objects inside the viewport
	virtual bool isViewportRelative() const { return false; }

	bool _needsRedraw;
	bool _isVisible;
	uint16 _z;
	Common::Rect _previousScreenPosition;
	Common::Rect _screenPosition;
};

} // End of namespace Nancy

#endif // NANCY_RENDEROBJECT_H
