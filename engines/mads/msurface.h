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
public:
	static MADSEngine *_vm;

	/**
	 * Sets the engine reference
	 */
	static void setVm(MADSEngine *vm) { _vm = vm; }

	/**
	 * Create a new surface.
	 */
	static MSurface *init();

	/**
	 * Create a surface
	 */
	static MSurface *init(int width, int height);
protected:
	/**
	 * Basic constructor
	 */
	MSurface();

	/**
	 * Constructor for a surface with fixed dimensions
	 */
	MSurface(int width, int height);
public:
	/**
	 * Helper method for calculating new dimensions when scaling a sprite
	 */
	static int scaleValue(int value, int scale, int err);	
public:
	/**
	 * Destructor
	 */
	virtual ~MSurface();

	/**
	 * Reinitialises a surface to have a given set of dimensions
	 */
	void setSize(int width, int height);

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
	 * Clears the surface
	 */
	void empty();

	/**
	 * Updates the screen with the contents of the surface
	 */
	void updateScreen();

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
	 * Translates the data of a surface using a specified RGBList translation matrix.
	 */
	void translate(RGBList *list, bool isTransparent = false);

	// Base virtual methods
	/**
	 * Loads a background by scene name
	 */
	virtual void loadBackground(const Common::String &sceneName) {}

	/**
	 * Load background by room number
	 */
	virtual void loadBackground(int roomNumber, RGBList **palData) = 0;

	/**
	 * Load background from a passed stream
	 */
	virtual void loadBackground(Common::SeekableReadStream *source, RGBList **palData) {}

	/**
	 * Load scene codes from a passed stream
	 */
	virtual void loadCodes(Common::SeekableReadStream *source) = 0;

	/**
	 * Load a given user interface by index
	 */
	virtual void loadInterface(int index, RGBList **palData) {}
};

class MSurfaceMADS: public MSurface {
	friend class MSurface;
protected:
	MSurfaceMADS(): MSurface() {}
	MSurfaceMADS(int width, int height): MSurface(width, height) {}
public:
	virtual void loadCodes(Common::SeekableReadStream *source);
	virtual void loadBackground(const Common::String &sceneName) {}
	virtual void loadBackground(int roomNumber, RGBList **palData);
	virtual void loadInterface(int index, RGBList **palData);
};

class MSurfaceNebular: public MSurfaceMADS {
	friend class MSurface;
protected:
	MSurfaceNebular(): MSurfaceMADS() {}
	MSurfaceNebular(int width, int height): MSurfaceMADS(width, height) {}
private:
	void loadBackgroundStream(Common::SeekableReadStream *source, RGBList **palData);
public:
	virtual void loadBackground(int roomNumber, RGBList **palData);
};

} // End of namespace MADS

#endif /* MADS_MSURFACE_H */
