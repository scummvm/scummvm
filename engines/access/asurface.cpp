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

const int TRANSPARENCY = 0;

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
	Common::fill(data, data + w * h, TRANSPARENCY);

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

int BaseSurface::_clipWidth;
int BaseSurface::_clipHeight;

BaseSurface::BaseSurface(): Graphics::Screen(0, 0) {
	free();		// Free the 0x0 surface allocated by Graphics::Screen
	_leftSkip = _rightSkip = 0;
	_topSkip = _bottomSkip = 0;
	_lastBoundsX = _lastBoundsY = 0;
	_lastBoundsW = _lastBoundsH = 0;
	_orgX1 = _orgY1 = 0;
	_orgX2 = _orgY2 = 0;
	_lColor = 0;
	_maxChars = 0;
}

BaseSurface::~BaseSurface() {
	_savedBlock.free();
}

void BaseSurface::clearBuffer() {
	byte *pSrc = (byte *)getPixels();
	Common::fill(pSrc, pSrc + w * h, 0);
}

void BaseSurface::plotImage(SpriteResource *sprite, int frameNum, const Common::Point &pt) {
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

void BaseSurface::copyBuffer(Graphics::ManagedSurface *src) {
	blitFrom(*src);
}

void BaseSurface::plotF(SpriteFrame *frame, const Common::Point &pt) {
	sPlotF(frame, Common::Rect(pt.x, pt.y, pt.x + frame->w, pt.y + frame->h));
}

void BaseSurface::plotB(SpriteFrame *frame, const Common::Point &pt) {
	sPlotB(frame, Common::Rect(pt.x, pt.y, pt.x + frame->w, pt.y + frame->h));
}

void BaseSurface::sPlotF(SpriteFrame *frame, const Common::Rect &bounds) {
	transBlitFrom(*frame, Common::Rect(0, 0, frame->w, frame->h), bounds, TRANSPARENCY, false);
}

void BaseSurface::sPlotB(SpriteFrame *frame, const Common::Rect &bounds) {
	transBlitFrom(*frame, Common::Rect(0, 0, frame->w, frame->h), bounds, TRANSPARENCY, true);
}

void BaseSurface::copyBlock(BaseSurface *src, const Common::Rect &bounds) {
	copyRectToSurface(*src, bounds.left, bounds.top, bounds);
}

void BaseSurface::copyTo(BaseSurface *dest) {
	if (dest->empty())
		dest->create(this->w, this->h);

	dest->blitFrom(*this);
}

void BaseSurface::saveBlock(const Common::Rect &bounds) {
	_savedBounds = bounds;
	_savedBounds.clip(Common::Rect(0, 0, this->w, this->h));

	_savedBlock.free();
	_savedBlock.create(bounds.width(), bounds.height(),
		Graphics::PixelFormat::createFormatCLUT8());
	_savedBlock.copyRectToSurface(*this, 0, 0, _savedBounds);
}

void BaseSurface::restoreBlock() {
	if (!_savedBounds.isEmpty()) {
		copyRectToSurface(_savedBlock, _savedBounds.left, _savedBounds.top,
			Common::Rect(0, 0, _savedBlock.w, _savedBlock.h));

		_savedBlock.free();
		_savedBounds = Common::Rect(0, 0, 0, 0);
	}
}

void BaseSurface::drawRect() {
	Graphics::ManagedSurface::fillRect(Common::Rect(_orgX1, _orgY1, _orgX2, _orgY2), _lColor);
}

void BaseSurface::drawLine(int x1, int y1, int x2, int y2, int col) {
	Graphics::ManagedSurface::drawLine(x1, y1, x2, y2, col);
}

void BaseSurface::drawLine() {
	Graphics::ManagedSurface::drawLine(_orgX1, _orgY1, _orgX2, _orgY1, _lColor);
}

void BaseSurface::drawBox() {
	Graphics::ManagedSurface::drawLine(_orgX1, _orgY1, _orgX2, _orgY1, _lColor);
	Graphics::ManagedSurface::drawLine(_orgX1, _orgY2, _orgX2, _orgY2, _lColor);
	Graphics::ManagedSurface::drawLine(_orgX2, _orgY1, _orgX2, _orgY1, _lColor);
	Graphics::ManagedSurface::drawLine(_orgX2, _orgY2, _orgX2, _orgY2, _lColor);
}

void BaseSurface::flipHorizontal(BaseSurface &dest) {
	dest.create(this->w, this->h);
	for (int y = 0; y < h; ++y) {
		const byte *pSrc = (const byte *)getBasePtr(this->w - 1, y);
		byte *pDest = (byte *)dest.getBasePtr(0, y);

		for (int x = 0; x < w; ++x, --pSrc, ++pDest)
			*pDest = *pSrc;
	}
}

void BaseSurface::moveBufferLeft() {
	byte *p = (byte *)getPixels();
	Common::copy(p + TILE_WIDTH, p + (w * h), p);
}

void BaseSurface::moveBufferRight() {
	byte *p = (byte *)getPixels();
	Common::copy_backward(p, p + (pitch * h) - TILE_WIDTH, p + (pitch * h));
}

void BaseSurface::moveBufferUp() {
	byte *p = (byte *)getPixels();
	Common::copy(p + (pitch * TILE_HEIGHT), p + (pitch * h), p);
}

void BaseSurface::moveBufferDown() {
	byte *p = (byte *)getPixels();
	Common::copy_backward(p, p + (pitch * (h - TILE_HEIGHT)), p + (pitch * h));
}

bool BaseSurface::clip(Common::Rect &r) {
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

} // End of namespace Access
