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
#include "graphics/screen.h"
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
 * Base MADS surface class. This derivces from Graphics::Screen
 * because it has logic we'll need for our own Screen class that
 * derives from this one
 */
class BaseSurface : public Graphics::Screen {
private:
	/**
	 * Helper method for calculating new dimensions when scaling a sprite
	 */
	int scaleValue(int value, int scale, int err);
protected:
	static MADSEngine *_vm;
public:
	/**
	 * Sets the engine reference used all surfaces
	 */
	static void setVm(MADSEngine *vm) { _vm = vm; }

	/**
	* Base method for descendents to load their contents
	*/
	virtual void load(const Common::String &resName) {}
public:
	/**
	 * Basic constructor
	 */
	BaseSurface() : Graphics::Screen(0, 0) {
		free();		// Free the 0x0 surface allocated by Graphics::Screen
	}

	/**
	 * Constructor for a surface with fixed dimensions
	 */
	BaseSurface(int width, int height) : Graphics::Screen(width, height) {}

	/**
	 * Destructor
	 */
	~BaseSurface() override {}

	/**
	 * Return a rect containing the bounds of the surface
	 */
	Common::Rect getBounds() { return Common::Rect(0, 0, this->w, this->h); }

	/**
	 * Return the pixels for the surface
	 */
	inline byte *getPixels() { return (byte *)Graphics::ManagedSurface::getPixels(); }

	/**
	 * Return the pixels for the surface
	 */
	inline const void *getPixels() const { return (const byte *)Graphics::ManagedSurface::getPixels(); }

	/**
	 * Return a pointer to a given position on the surface
	 */
	byte *getBasePtr(int x, int y) { return (byte *)Graphics::ManagedSurface::getBasePtr(x, y); }

	/**
	 * Return a pointer to a given position on the surface
	 */
	inline const byte *getBasePtr(int x, int y) const { return (const byte *)Graphics::ManagedSurface::getBasePtr(x, y); }

	/**
	 * Draws a sprite
	 * @param pt		Position to draw sprite at
	 * @param info		General sprite details
	 * @param clipRect	Clipping rectangle to constrain sprite drawing within
	 */
	void drawSprite(const Common::Point &pt, SpriteInfo &info, const Common::Rect &clipRect);

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
	BaseSurface *flipHorizontal() const;

	/**
	 * Copy an area from one surface to another, translating it using a palette
	 * map as it's done
	 */
	void copyRectTranslate(BaseSurface &srcSurface, const byte *paletteMap,
		const Common::Point &destPos, const Common::Rect &srcRect);

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
	void copyFrom(BaseSurface &src, const Common::Point &destPos, int depth, DepthSurface *depthSurface,
		int scale, bool flipped, int transparentColor = -1);
};

class MSurface : public BaseSurface {
protected:
	/**
	 * Override the addDirtyRect from Graphics::Screen, since for standard
	 * surfaces we don't need dirty rects to be tracked
	 */
	void addDirtyRect(const Common::Rect &r) override {}
public:
	MSurface() : BaseSurface() {}
	MSurface(int width, int height) : BaseSurface(width, height) {}
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
	DepthSurface() : MSurface(), _depthStyle(0) {}

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
