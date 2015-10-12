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

#ifndef MADS_MSURFACE_H
#define MADS_MSURFACE_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "graphics/surface.h"
#include "mads/palette.h"

namespace MADS {

class MADSEngine;
class MSprite;
class DepthSurface;

/**
 * Basic sprite information
 */
struct SpriteInfo {
	MSprite *sprite;
	int hotX, hotY;
	int width, height;
	int scaleX, scaleY;
	uint8 encoding;
	byte *inverseColorTable;
	byte *palette;
};

/*
 * MADS graphics surface
 */
class MSurface : public Graphics::Surface {
protected:
	static MADSEngine *_vm;
	bool _freeFlag;
public:
	/**
	 * Sets the engine refrence used all surfaces
	 */
	static void setVm(MADSEngine *vm) { _vm = vm; }

	/**
	 * Helper method for calculating new dimensions when scaling a sprite
	 */
	static int scaleValue(int value, int scale, int err);

	/**
	* Base method for descendents to load their contents
	*/
	virtual void load(const Common::String &resName) {}
public:
	/**
	 * Basic constructor
	 */
	MSurface();

	/**
	 * Constructor for a surface with fixed dimensions
	 */
	MSurface(int width, int height);

	/**
	 * Destructor
	 */
	virtual ~MSurface();

	/**
	 * Reinitializes a surface to have a given set of dimensions
	 */
	void setSize(int width, int height);

	/**
	 * Sets the pixels the surface is associated with
	 * @remarks		The surface will not free the data block
	 */
	void setPixels(byte *pData, int horizSize, int vertSize);

	/**
	 * Draws an arbitrary line on the screen using a specified color
	 * @param startPos		Starting position
	 * @param endPos		Ending position
	 * @param color			Color to use
	 */
	void line(const Common::Point &startPos, const Common::Point &endPos, byte color);

	/**
	 * Draws a sprite
	 * @param pt		Position to draw sprite at
	 * @param info		General sprite details
	 * @param clipRect	Clipping rectangle to constrain sprite drawing within
	 */
	void drawSprite(const Common::Point &pt, SpriteInfo &info, const Common::Rect &clipRect);

	/**
	 * Returns the width of the surface
	 */
	int getWidth() const { return w; }

	/**
	 * Returns the height of the surface
	 */
	int getHeight() const { return h; }

	/**
	 * Returns the size of the surface as a Rect
	 */
	Common::Rect getBounds() const {
		return Common::Rect(0, 0, w, h);
	}

	/**
	 * Returns a pointer to the surface data
	 */
	byte *getData() { return (byte *)Graphics::Surface::getPixels(); }

	/**
	 * Returns a pointer to a given position within the surface
	 */
	byte *getBasePtr(int x, int y) { return (byte *)Graphics::Surface::getBasePtr(x, y); }

	/**
	* Returns a pointer to a given position within the surface
	*/
	const byte *getBasePtr(int x, int y) const { return (const byte *)Graphics::Surface::getBasePtr(x, y); }

	/**
	 * Clears the surface
	 */
	void empty();

	/**
	 * Copys a sub-section of another surface into the current one.
	 * @param src			Source surface
	 * @param srcBounds		Area of source surface to copy
	 * @param destPos		Destination position to draw in current surface
	 * @param transparentColor	Transparency palette index
	 */
	void copyFrom(MSurface *src, const Common::Rect &srcBounds, const Common::Point &destPos,
		int transparentColor = -1);

	/**
	* Copys a sub-section of another surface into the current one.
	* @param src			Source surface
	* @param destPos		Destination position to draw in current surface
	* @param depth			Depth of sprite
	* @param depthSurface	Depth surface to use with sprite depth
	* @param scale			Scale for image
	* @param flipped		Flag for whether image is to be flipped
	* @param transparentColor	Transparency palette index
	*/
	void copyFrom(MSurface *src, const Common::Point &destPos, int depth, DepthSurface *depthSurface,
		int scale, bool flipped, int transparentColor = -1);

	/**
	 * Copies the surface to a given destination surface
	 */
	void copyTo(MSurface *dest, int transparentColor = -1) {
		dest->copyFrom(this, Common::Rect(w, h), Common::Point(), transparentColor);
	}

	/**
	 * Copies the surface to a given destination surface
	 */
	void copyTo(MSurface *dest, const Common::Point &pt, int transparentColor = -1) {
		dest->copyFrom(this, Common::Rect(w, h), pt, transparentColor);
	}

	/**
	 * Copies the surface to a given destination surface
	 */
	void copyTo(MSurface *dest, const Common::Rect &srcBounds, const Common::Point &destPos,
				int transparentColor = -1) {
		dest->copyFrom(this, srcBounds, destPos, transparentColor);
	}

	/**
	 * Scroll the screen horizontally by a given amount
	 * @param xAmount	Horizontal amount
	 */
	void scrollX(int xAmount);

	/**
	 * Scroll the screen vertically by a given amount
	 * @param yAmount	Vertical amount
	 */
	void scrollY(int yAmount);

	/**
	 * Translates the pixels of an image used the passed palette with RGB mapping
	 */
	void translate(Common::Array<RGB6> &palette);

	/**
	* Translates the pixels of an image used the passed palette with RGB mapping
	*/
	void translate(byte map[PALETTE_COUNT]);

	/**
	 * Create a new surface which is a flipped horizontal copy of the current one
	 */
	MSurface *flipHorizontal() const;

	/**
	 * Copy an area from one surface to another, translating it using a palette
	 * map as it's done
	 */
	void copyRectTranslate(MSurface &srcSurface, const byte *paletteMap,
		const Common::Point &destPos, const Common::Rect &srcRect);
};

class DepthSurface : public MSurface {
public:
	/**
	 * Depth style
	 */
	int _depthStyle;

	/**
	 * Constructor
	 */
	DepthSurface() : _depthStyle(0) {}

	/**
	 * Returns the depth at a given position
	 */
	int getDepth(const Common::Point &pt);

	/**
	 */
	int getDepthHighBit(const Common::Point &pt);
};

} // End of namespace MADS

#endif /* MADS_MSURFACE_H */
