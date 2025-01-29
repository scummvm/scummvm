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

#include "mediastation/datum.h"
#include "mediastation/assets/sprite.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

SpriteFrameHeader::SpriteFrameHeader(Chunk &chunk) : BitmapHeader(chunk) {
	_index = Datum(chunk).u.i;
	debugC(5, kDebugLoading, "SpriteFrameHeader::SpriteFrameHeader(): _index = 0x%x (@0x%llx)", _index, static_cast<long long int>(chunk.pos()));
	_boundingBox = Datum(chunk, kDatumTypePoint2).u.point;
	debugC(5, kDebugLoading, "SpriteFrameHeader::SpriteFrameHeader(): _boundingBox (@0x%llx)", static_cast<long long int>(chunk.pos()));
}

SpriteFrameHeader::~SpriteFrameHeader() {
	delete _boundingBox;
	_boundingBox = nullptr;
}

SpriteFrame::SpriteFrame(Chunk &chunk, SpriteFrameHeader *header) : Bitmap(chunk, header) {
	_bitmapHeader = header;
}

SpriteFrame::~SpriteFrame() {
	// The base class destructor takes care of deleting the bitmap header.
}

uint32 SpriteFrame::left() {
	return _bitmapHeader->_boundingBox->x;
}

uint32 SpriteFrame::top() {
	return _bitmapHeader->_boundingBox->y;
}

Common::Point SpriteFrame::topLeft() {
	return Common::Point(left(), top());
}

Common::Rect SpriteFrame::boundingBox() {
	return Common::Rect(topLeft(), width(), height());
}

uint32 SpriteFrame::index() {
	return _bitmapHeader->_index;
}

Sprite::Sprite(AssetHeader *header) : Asset(header) {
	if (header->_startup == kAssetStartupActive) {
		_isActive = true;
	}
}

Sprite::~Sprite() {
	for (SpriteFrame *frame : _frames) {
		delete frame;
	}
	_frames.clear();
}

Operand Sprite::callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) {
	switch (methodId) {
	case kSpatialShowMethod: {
		assert(args.size() == 0);
		spatialShow();
		return Operand();
	}

	case kSpatialHideMethod: {
		assert(args.empty());
		_isActive = false;
		return Operand();
	}

	case kTimeStopMethod: {
		assert(args.empty());
		_isActive = false;
		return Operand();
	}

	case kTimePlayMethod: {
		assert(args.size() == 0);
		timePlay();
		return Operand();
	}

	case kMovieResetMethod: {
		assert(args.size() == 0);
		movieReset();
		return Operand();
	}

	default: {
		error("Sprite::callMethod(): Got unimplemented method ID %d", methodId);
	}
	}
}

void Sprite::spatialShow() {
	debugC(5, kDebugScript, "Called Sprite::spatialShow");
	_isActive = true;
	g_engine->addPlayingAsset(this);

	_isPaused = true;
	if (_activeFrame == nullptr) {
		showFrame(0);
	}
}

void Sprite::timePlay() {
	_isActive = true;
	_isPaused = false;
	_startTime = g_system->getMillis();
	_lastProcessedTime = 0;
	_nextFrameTime = 0;
	g_engine->addPlayingAsset(this);

	if (_header->_frameRate == 0) {
		// It seems that the frame rate is 10 if it's not set in the asset
		// header, or even if it's set to zero.
		_header->_frameRate = 10;
	}

	runEventHandlerIfExists(kMovieBeginEvent);
}

void Sprite::movieReset() {
	_isActive = true;
	// We do NOT reset the persisting frame, because it should keep showing!
	_isPaused = true;
	_startTime = 0;
	_currentFrameIndex = 0;
	_nextFrameTime = 0;
	_lastProcessedTime = 0;
}

void Sprite::process() {
	updateFrameState();
}

void Sprite::readChunk(Chunk &chunk) {
	// Reads one frame from the sprite.
	debugC(5, kDebugLoading, "Sprite::readFrame(): Reading sprite frame (@0x%llx)", static_cast<long long int>(chunk.pos()));
	SpriteFrameHeader *header = new SpriteFrameHeader(chunk);
	SpriteFrame *frame = new SpriteFrame(chunk, header);
	_frames.push_back(frame);

	// TODO: Are these in exactly reverse order? If we can just reverse the
	// whole thing once.
	Common::sort(_frames.begin(), _frames.end(), [](SpriteFrame * a, SpriteFrame * b) {
		return a->index() < b->index();
	});
}

void Sprite::updateFrameState() {
	if (_isPaused) {
		return;
	}

	uint currentTime = g_system->getMillis() - _startTime;
	bool drawNextFrame = currentTime >= _nextFrameTime;
	if (!drawNextFrame) {
		return;
	}

	showFrame(_currentFrameIndex);

	uint frameDuration = 1000 / _header->_frameRate;
	_nextFrameTime = ++_currentFrameIndex * frameDuration;

	bool spriteFinishedPlaying = (_currentFrameIndex == _frames.size());
	if (spriteFinishedPlaying) {
		// Sprites always keep their last frame showing until they are hidden
		// with spatialHide.
		showFrame(_currentFrameIndex - 1);
		_isPaused = true;

		// But otherwise, the sprite's params should be reset.
		_isActive = true;
		_startTime = 0;
		_lastProcessedTime = 0;
		_currentFrameIndex = 0;
		_nextFrameTime = 0;

		runEventHandlerIfExists(kSpriteMovieEndEvent);
	}
}

void Sprite::redraw(Common::Rect &rect) {
	if (_activeFrame == nullptr) {
		return;
	}

	Common::Rect bbox = getActiveFrameBoundingBox();
	Common::Rect areaToRedraw = bbox.findIntersectingRect(rect);
	if (!areaToRedraw.isEmpty()) {
		Common::Point originOnScreen(areaToRedraw.left, areaToRedraw.top);
		areaToRedraw.translate(-_activeFrame->left() - _header->_boundingBox->left, -_activeFrame->top() - _header->_boundingBox->top);
		g_engine->_screen->simpleBlitFrom(_activeFrame->_surface, areaToRedraw, originOnScreen);
	}
}

void Sprite::showFrame(uint frameIndex) {
	// Erase the previous frame.
	if (_activeFrame != nullptr) {
		g_engine->_dirtyRects.push_back(getActiveFrameBoundingBox());
	}
	
	// Show the next frame.
	_activeFrame = _frames[frameIndex];
	g_engine->_dirtyRects.push_back(getActiveFrameBoundingBox());
}

Common::Rect Sprite::getActiveFrameBoundingBox() {
	// The frame dimensions are relative to those of the sprite movie.
	// So we must get the absolute coordinates.
	Common::Rect bbox = _activeFrame->boundingBox();
	bbox.translate(_header->_boundingBox->left, _header->_boundingBox->top);
	return bbox;
}

} // End of namespace MediaStation
