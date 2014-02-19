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

struct SpriteInfo {
	MSprite *sprite;
	int hotX, hotY;
	int width, height;
	int scaleX, scaleY;
	uint8 encoding;
	byte *inverseColorTable;
	RGB8 *palette;
};

class MSurface : public Graphics::Surface {
public:
	static MADSEngine *_vm;

	/**
	 * Sets the engine reference
	 */
	static void setVm(MADSEngine *vm) { _vm = vm; }

	/**
	 * Create a new surface the same size as the screen.
	 * @param isScreen		Set to true for the screen surface
	 */
	static MSurface *init(bool isScreen = false);

	/**
	 * Create a surface
	 */
	static MSurface *init(int w, int h);
private:
	byte _color;
	bool _isScreen;
protected:
	MSurface(bool isScreen = false);
	MSurface(int w, int h);
public:
	virtual ~MSurface() {}

	void create(int w, int h) {
		Graphics::Surface::create(w, h, Graphics::PixelFormat::createFormatCLUT8());
	}

	void setColor(byte value) { _color = value; }
	byte getColor() { return _color; }
	void vLine(int x, int y1, int y2);
	void hLine(int x1, int x2, int y);
	void vLineXor(int x, int y1, int y2);
	void hLineXor(int x1, int x2, int y);
	void line(int x1, int y1, int x2, int y2, byte color);
	void frameRect(int x1, int y1, int x2, int y2);
	void fillRect(int x1, int y1, int x2, int y2);

	static int scaleValue(int value, int scale, int err);	
	void drawSprite(int x, int y, SpriteInfo &info, const Common::Rect &clipRect);

	// Surface methods
	int width() { return w; }
	int height() { return h; }
	void setSize(int sizeX, int sizeY);
	byte *getData();
	byte *getBasePtr(int x, int y);
	void freeData();
	void empty();
	void frameRect(const Common::Rect &r, uint8 color);
	void fillRect(const Common::Rect &r, uint8 color);
	void copyFrom(MSurface *src, const Common::Rect &srcBounds, int destX, int destY,
		int transparentColor = -1);

	void update() { 
		if (_isScreen) {
			g_system->copyRectToScreen((const byte *)pixels, pitch, 0, 0, w, h);
			g_system->updateScreen(); 
		}
	}

	// copyTo methods
	void copyTo(MSurface *dest, int transparentColor = -1) { 
		dest->copyFrom(this, Common::Rect(width(), height()), 0, 0, transparentColor);		
	}
	void copyTo(MSurface *dest, int x, int y, int transparentColor = -1) {
		dest->copyFrom(this, Common::Rect(width(), height()), x, y, transparentColor);
	}
	void copyTo(MSurface *dest, const Common::Rect &srcBounds, int destX, int destY,
				int transparentColor = -1) {
		dest->copyFrom(this, srcBounds, destX, destY, transparentColor);
	}

	void translate(RGBList *list, bool isTransparent = false);

	// Base virtual methods
	virtual void loadBackground(const Common::String &sceneName) {}
	virtual void loadBackground(int roomNumber, RGBList **palData) = 0;
	virtual void loadBackground(Common::SeekableReadStream *source, RGBList **palData) {}
	virtual void loadCodes(Common::SeekableReadStream *source) = 0;
	virtual void loadInterface(int index, RGBList **palData) {}
};

class MSurfaceMADS: public MSurface {
	friend class MSurface;
protected:
	MSurfaceMADS(bool isScreen = false): MSurface(isScreen) {}
	MSurfaceMADS(int w, int h): MSurface(w, h) {}
public:
	virtual void loadCodes(Common::SeekableReadStream *source);
	virtual void loadBackground(const Common::String &sceneName) {}
	virtual void loadBackground(int roomNumber, RGBList **palData);
	virtual void loadInterface(int index, RGBList **palData);
};

class MSurfaceNebular: public MSurfaceMADS {
	friend class MSurface;
protected:
	MSurfaceNebular(bool isScreen = false): MSurfaceMADS(isScreen) {}
	MSurfaceNebular(int w, int h): MSurfaceMADS(w, h) {}
private:
	void loadBackgroundStream(Common::SeekableReadStream *source, RGBList **palData);
public:
	virtual void loadBackground(int roomNumber, RGBList **palData);
};

class MSurfaceM4: public MSurface {
	friend class MSurface;
protected:
	MSurfaceM4(bool isScreen = false): MSurface(isScreen) {}
	MSurfaceM4(int widthVal, int heightVal): MSurface(widthVal, heightVal) {}

	void loadBackgroundStream(Common::SeekableReadStream *source);
public:
	virtual void loadCodes(Common::SeekableReadStream *source);
	virtual void loadBackground(int roomNumber, RGBList **palData);
};

class MSurfaceRiddle: public MSurfaceM4 {
	friend class MSurface;
protected:
	MSurfaceRiddle(bool isScreen = false): MSurfaceM4(isScreen) {}
	MSurfaceRiddle(int widthVal, int heightVal): MSurfaceM4(widthVal, heightVal) {}
public:
	virtual void loadBackground(const Common::String &sceneName);
};

} // End of namespace MADS

#endif /* MADS_MSURFACE_H */
