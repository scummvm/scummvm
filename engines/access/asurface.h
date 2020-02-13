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

#ifndef ACCESS_ASURFACE_H
#define ACCESS_ASURFACE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/memstream.h"
#include "common/rect.h"
#include "graphics/screen.h"
#include "access/data.h"

namespace Access {

class SpriteResource;
class SpriteFrame;

/**
 * Base Access surface class. This derivces from Graphics::Screen
 * because it has logic we'll need for our own Screen class that
 * derives from this one
 */
class BaseSurface : virtual public Graphics::Screen {
private:
	Graphics::Surface _savedBlock;

	void flipHorizontal(BaseSurface &dest);
protected:
	Common::Rect _savedBounds;
public:
	int _leftSkip, _rightSkip;
	int _topSkip, _bottomSkip;
	int _lastBoundsX, _lastBoundsY;
	int _lastBoundsW, _lastBoundsH;
	int _orgX1, _orgY1;
	int _orgX2, _orgY2;
	int _lColor;

	Common::Point _printOrg;
	Common::Point _printStart;
	int _maxChars;
public:
	static int _clipWidth, _clipHeight;
public:
	BaseSurface();

	~BaseSurface() override;

	void clearBuffer();

	void plotImage(SpriteResource *sprite, int frameNum, const Common::Point &pt);

	/**
	 * Scaled draw frame in forward orientation
	 */
	void sPlotF(SpriteFrame *frame, const Common::Rect &bounds);

	/**
	 * Scaled draw frame in backwards orientation
	 */
	void sPlotB(SpriteFrame *frame, const Common::Rect &bounds);

	/**
	 * Draw an image full-size in forward orientation
	 */
	void plotF(SpriteFrame *frame, const Common::Point &pt);

	/**
	 * Draw an image full-size in backwards orientation
	 */
	void plotB(SpriteFrame *frame, const Common::Point &pt);

	virtual void copyBlock(BaseSurface *src, const Common::Rect &bounds);

	virtual void restoreBlock();

	virtual void drawRect();

	virtual void drawLine(int x1, int y1, int x2, int y2, int col);

	virtual void drawLine();

	virtual void drawBox();

	virtual void copyBuffer(Graphics::ManagedSurface *src);

	void copyTo(BaseSurface *dest);

	void saveBlock(const Common::Rect &bounds);

	void moveBufferLeft();

	void moveBufferRight();

	void moveBufferUp();

	void moveBufferDown();

	bool clip(Common::Rect &r);
};

class ASurface : public BaseSurface {
protected:
	/**
	 * Override the addDirtyRect from Graphics::Screen, since for standard
	 * surfaces we don't need dirty rects to be tracked
	 */
	void addDirtyRect(const Common::Rect &r) override {}
public:
	ASurface() : BaseSurface() {}
};

class SpriteFrame : public ASurface {
public:
	SpriteFrame(AccessEngine *vm, Common::SeekableReadStream *stream, int frameSize);
	~SpriteFrame() override;
};

class SpriteResource {
public:
	Common::Array<SpriteFrame *> _frames;
public:
	SpriteResource(AccessEngine *vm, Resource *res);
	~SpriteResource();

	int getCount() { return _frames.size(); }

	SpriteFrame *getFrame(int idx) { return _frames[idx]; }
};

enum ImageFlag {
	IMGFLAG_CROPPED = 1,
	IMGFLAG_BACKWARDS = 2,
	IMGFLAG_DRAWN = 4,
	IMGFLAG_UNSCALED = 8
};

class ImageEntry {
public:
	int _frameNumber;
	SpriteResource *_spritesPtr;
	int _offsetY;
	Common::Point _position;
	int _flags;
public:
	ImageEntry();
};

class ImageEntryList : public Common::Array<ImageEntry> {
public:
	void addToList(ImageEntry &ie);
};

} // End of namespace Access

#endif /* ACCESS_ASURFACE_H */
