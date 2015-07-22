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

#include "common/algorithm.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "access/access.h"
#include "access/asurface.h"

namespace Access {

SpriteResource::SpriteResource(AccessEngine *vm, Resource *res) {
	Common::Array<uint32> offsets;
	int count = res->_stream->readUint16LE();

	for (int i = 0; i < count; i++)
		offsets.push_back(res->_stream->readUint32LE());
	offsets.push_back(res->_size);	// For easier calculations of Noctropolis sizes

	// Build up the frames
	for (int i = 0; i < count; ++i) {
		res->_stream->seek(offsets[i]);
		int frameSize = offsets[i + 1] - offsets[i];

		SpriteFrame *frame = new SpriteFrame(vm, res->_stream, frameSize);
		_frames.push_back(frame);
	}
}

SpriteResource::~SpriteResource() {
	for (uint i = 0; i < _frames.size(); ++i)
		delete _frames[i];
}

SpriteFrame::SpriteFrame(AccessEngine *vm, Common::SeekableReadStream *stream, int frameSize) {
	int xSize = stream->readUint16LE();
	int ySize = stream->readUint16LE();

	if (vm->getGameID() == GType_MartianMemorandum) {
		int size = stream->readUint16LE();
		if (size != frameSize)
			warning("Unexpected file difference: framesize %d - size %d %d - unknown %d", frameSize, xSize, ySize, size);
	}
	create(xSize, ySize);

	// Empty surface
	byte *data = (byte *)getPixels();
	Common::fill(data, data + w * h, 0);

	// Decode the data
	for (int y = 0; y < h; ++y) {
		int offset = stream->readByte();
		int len = stream->readByte();
		assert((offset + len) <= w);

		byte *destP = (byte *)getBasePtr(offset, y);
		stream->read(destP, len);
	}
}

SpriteFrame::~SpriteFrame() {
	free();
}

/*------------------------------------------------------------------------*/

ImageEntry::ImageEntry() {
	_frameNumber = 0;
	_spritesPtr = nullptr;
	_offsetY = 0;
	_flags = 0;
}

/*------------------------------------------------------------------------*/

static bool sortImagesY(const ImageEntry &ie1, const ImageEntry &ie2) {
	int v = (ie1._position.y + ie1._offsetY) - (ie2._position.y + ie2._offsetY);
	return (v < 0) || (v == 0 && ie1._position.y <= ie2._position.y);
}

void ImageEntryList::addToList(ImageEntry &ie) {
	assert(size() < 35);
	push_back(ie);
	Common::sort(begin(), end(), sortImagesY);
}

/*------------------------------------------------------------------------*/

int ASurface::_clipWidth;
int ASurface::_clipHeight;

ASurface::ASurface(): Graphics::Surface() {
	_leftSkip = _rightSkip = 0;
	_topSkip = _bottomSkip = 0;
	_lastBoundsX = _lastBoundsY = 0;
	_lastBoundsW = _lastBoundsH = 0;
	_orgX1 = _orgY1 = 0;
	_orgX2 = _orgY2 = 0;
	_lColor = 0;
	_maxChars = 0;
}

ASurface::~ASurface() {
	free();
	_savedBlock.free();
}

void ASurface::create(uint16 width, uint16 height) {
	Graphics::Surface::create(width, height, Graphics::PixelFormat::createFormatCLUT8());
}

void ASurface::clearBuffer() {
	byte *pSrc = (byte *)getPixels();
	Common::fill(pSrc, pSrc + w * h, 0);
}

bool ASurface::clip(Common::Rect &r) {
	int skip;
	_leftSkip = _rightSkip = 0;
	_topSkip = _bottomSkip = 0;

	if (r.left > _clipWidth || r.left < 0) {
		if (r.left >= 0)
			return true;

		skip = -r.left;
		r.setWidth(r.width() - skip);
		_leftSkip = skip;
		r.moveTo(0, r.top);
	}

	int right = r.right - 1;
	if (right < 0)
		return true;
	else if (right > _clipWidth) {
		skip = right - _clipWidth;
		r.setWidth(r.width() - skip);
		_rightSkip = skip;
	}

	if (r.top > _clipHeight || r.top < 0) {
		if (r.top >= 0)
			return true;

		skip = -r.top;
		r.setHeight(r.height() - skip);
		_topSkip = skip;
		r.moveTo(r.left, 0);
	}

	int bottom = r.bottom - 1;
	if (bottom < 0)
		return true;
	else if (bottom > _clipHeight) {
		skip = bottom - _clipHeight;
		_bottomSkip = skip;
		r.setHeight(r.height() - skip);
	}

	return false;
}

void ASurface::plotImage(SpriteResource *sprite, int frameNum, const Common::Point &pt) {
	SpriteFrame *frame = sprite->getFrame(frameNum);
	Common::Rect r(pt.x, pt.y, pt.x + frame->w, pt.y + frame->h);

	if (!clip(r)) {
		_lastBoundsX = r.left;
		_lastBoundsY = r.top;
		_lastBoundsW = r.width();
		_lastBoundsH = r.height();

		plotF(frame, pt);
	}
}

void ASurface::transBlitFrom(ASurface *src, const Common::Point &destPos) {
	if (getPixels() == nullptr)
		create(w, h);

	for (int yp = 0; yp < src->h; ++yp) {
		const byte *srcP = (const byte *)src->getBasePtr(0, yp);
		byte *destP = (byte *)getBasePtr(destPos.x, destPos.y + yp);

		for (int xp = 0; xp < this->w; ++xp, ++srcP, ++destP) {
			if (*srcP != 0)
				*destP = *srcP;
		}
	}
}

void ASurface::transBlitFrom(ASurface *src, const Common::Rect &bounds) {
	const int SCALE_LIMIT = 0x100;
	int scaleX = SCALE_LIMIT * bounds.width() / src->w;
	int scaleY = SCALE_LIMIT * bounds.height() / src->h;
	int scaleXCtr = 0, scaleYCtr = 0;

	for (int yCtr = 0, destY = bounds.top; yCtr < src->h; ++yCtr) {
		// Handle skipping lines if Y scaling
		scaleYCtr += scaleY;
		if (scaleYCtr < SCALE_LIMIT)
			continue;
		scaleYCtr -= SCALE_LIMIT;

		// Handle off-screen lines
		if (destY >= this->h)
			break;

		if (destY >= 0) {
			// Handle drawing the line
			const byte *pSrc = (const byte *)src->getBasePtr(0, yCtr);
			byte *pDest = (byte *)getBasePtr(bounds.left, destY);
			scaleXCtr = 0;
			int x = bounds.left;

			for (int xCtr = 0; xCtr < src->w; ++xCtr, ++pSrc) {
				// Handle horizontal scaling
				scaleXCtr += scaleX;
				if (scaleXCtr < SCALE_LIMIT)
					continue;
				scaleXCtr -= SCALE_LIMIT;

				// Only handle on-screen pixels
				if (x >= this->w)
					break;
				if (x >= 0 && *pSrc != 0)
					*pDest = *pSrc;

				++pDest;
				++x;
			}
		}

		++destY;
	}
}

void ASurface::transBlitFrom(ASurface &src) {
	blitFrom(src);
}

void ASurface::blitFrom(Graphics::Surface &src) {
	assert(w >= src.w && h >= src.h);
	for (int y = 0; y < src.h; ++y) {
		const byte *srcP = (const byte *)src.getBasePtr(0, y);
		byte *destP = (byte *)getBasePtr(0, y);
		Common::copy(srcP, srcP + src.w, destP);
	}
}

void ASurface::copyBuffer(Graphics::Surface *src) {
	blitFrom(*src);
}

void ASurface::plotF(SpriteFrame *frame, const Common::Point &pt) {
	sPlotF(frame, Common::Rect(pt.x, pt.y, pt.x + frame->w, pt.y + frame->h));
}

void ASurface::plotB(SpriteFrame *frame, const Common::Point &pt) {
	sPlotB(frame, Common::Rect(pt.x, pt.y, pt.x + frame->w, pt.y + frame->h));
}

void ASurface::sPlotF(SpriteFrame *frame, const Common::Rect &bounds) {
	transBlitFrom(frame, bounds);
}

void ASurface::sPlotB(SpriteFrame *frame, const Common::Rect &bounds) {
	ASurface flippedFrame;
	frame->flipHorizontal(flippedFrame);

	transBlitFrom(&flippedFrame, bounds);
}

void ASurface::copyBlock(ASurface *src, const Common::Rect &bounds) {
	copyRectToSurface(*src, bounds.left, bounds.top, bounds);
}

void ASurface::saveBlock(const Common::Rect &bounds) {
	_savedBounds = bounds;
	_savedBounds.clip(Common::Rect(0, 0, this->w, this->h));

	_savedBlock.free();
	_savedBlock.create(bounds.width(), bounds.height(),
		Graphics::PixelFormat::createFormatCLUT8());
	_savedBlock.copyRectToSurface(*this, 0, 0, _savedBounds);
}

void ASurface::restoreBlock() {
	if (!_savedBounds.isEmpty()) {
		copyRectToSurface(_savedBlock, _savedBounds.left, _savedBounds.top,
			Common::Rect(0, 0, _savedBlock.w, _savedBlock.h));

		_savedBlock.free();
		_savedBounds = Common::Rect(0, 0, 0, 0);
	}
}

void ASurface::drawRect() {
	Graphics::Surface::fillRect(Common::Rect(_orgX1, _orgY1, _orgX2, _orgY2), _lColor);
}

void ASurface::drawLine(int x1, int y1, int x2, int y2, int col) {
	Graphics::Surface::drawLine(x1, y1, x2, y2, col);
}

void ASurface::drawLine() {
	Graphics::Surface::drawLine(_orgX1, _orgY1, _orgX2, _orgY1, _lColor);
}

void ASurface::drawBox() {
	Graphics::Surface::drawLine(_orgX1, _orgY1, _orgX2, _orgY1, _lColor);
	Graphics::Surface::drawLine(_orgX1, _orgY2, _orgX2, _orgY2, _lColor);
	Graphics::Surface::drawLine(_orgX2, _orgY1, _orgX2, _orgY1, _lColor);
	Graphics::Surface::drawLine(_orgX2, _orgY2, _orgX2, _orgY2, _lColor);
}

void ASurface::flipHorizontal(ASurface &dest) {
	dest.create(this->w, this->h);
	for (int y = 0; y < h; ++y) {
		const byte *pSrc = (const byte *)getBasePtr(this->w - 1, y);
		byte *pDest = (byte *)dest.getBasePtr(0, y);

		for (int x = 0; x < w; ++x, --pSrc, ++pDest)
			*pDest = *pSrc;
	}
}

void ASurface::moveBufferLeft() {
	byte *p = (byte *)getPixels();
	Common::copy(p + TILE_WIDTH, p + (w * h), p);
}

void ASurface::moveBufferRight() {
	byte *p = (byte *)getPixels();
	Common::copy_backward(p, p + (pitch * h) - TILE_WIDTH, p + (pitch * h));
}

void ASurface::moveBufferUp() {
	byte *p = (byte *)getPixels();
	Common::copy(p + (pitch * TILE_HEIGHT), p + (pitch * h), p);
}

void ASurface::moveBufferDown() {
	byte *p = (byte *)getPixels();
	Common::copy_backward(p, p + (pitch * (h - TILE_HEIGHT)), p + (pitch * h));
}

} // End of namespace Access
