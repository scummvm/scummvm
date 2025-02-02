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
		setActive();
		_isShowing = true;
	}

	if (_header->_frameRate == 0) {
		// It seems that the frame rate is 10 if it's not set in the asset
		// header, or even if it's set to zero.
		_header->_frameRate = 10;
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
		assert(args.empty());
		spatialShow();
		return Operand();
	}

	case kSpatialHideMethod: {
		assert(args.empty());
		spatialHide();
		return Operand();
	}

	case kTimePlayMethod: {
		assert(args.empty());
		timePlay();
		return Operand();
	}

	case kTimeStopMethod: {
		assert(args.empty());
		timeStop();
		return Operand();
	}

	case kMovieResetMethod: {
		assert(args.empty());
		movieReset();
		return Operand();
	}

	case kSetCurrentClipMethod: {
		assert(args.empty());
		setCurrentClip();
		return Operand();
	}

	case kIsPlayingMethod: {
		Operand returnValue(kOperandTypeLiteral1);
		returnValue.putInteger(static_cast<int>(_isPlaying));
		return returnValue;
	}

	default: {
		error("Sprite::callMethod(): Got unimplemented method ID %d", methodId);
	}
	}
}

void Sprite::spatialShow() {
	if (_isShowing) {
		warning("Sprite::spatialShow(): (%d) Attempted to spatialShow when already showing", _header->_id);
		return;
	}
	showFrame(_frames[0]);

	setActive();
	_isShowing = true;
	_isPlaying = false;
}

void Sprite::spatialHide() {
	if (!_isShowing) {
		warning("Sprite::spatialHide(): (%d) Attempted to spatialHide when not showing", _header->_id);
		return;
	}
	showFrame(nullptr);

	setInactive();
	_isShowing = false;
	_isPlaying = false;
}

void Sprite::timePlay() {
	if (!_isShowing) {
		warning("Sprite::timePlay(): (%d) Attempted to timePlay when not showing", _header->_id);
		return;
	} else if (_isPlaying) {
		warning("Sprite::timePlay(): (%d) Attempted to timePlay when already playing", _header->_id);
		return;
	}

	setActive();
	_isPlaying = true;
	_nextFrameTime = 0;

	runEventHandlerIfExists(kMovieBeginEvent);
}

void Sprite::timeStop() {
	if (!_isShowing) {
		warning("Sprite::timeStop(): (%d) Attempted to timeStop when not showing", _header->_id);
		return;
	} else if (!_isPlaying) {
		warning("Sprite::timeStop(): (%d) Attempted to timeStop when not playing", _header->_id);
		return;
	}

	_isPlaying = false;
	// TODO: Find the right event handler to run here.
}

void Sprite::movieReset() {
	setActive();
	if (_isShowing) {
		showFrame(_frames[0]);
	} else {
		showFrame(nullptr);
	}
	_isPlaying = false;
	_startTime = 0;
	_currentFrameIndex = 0;
	_nextFrameTime = 0;
	_lastProcessedTime = 0;
}

void Sprite::setCurrentClip() {
	if (_currentFrameIndex < _frames.size()) {
		showFrame(_frames[_currentFrameIndex++]);
	} else {
		warning("Sprite::setCurrentClip(): (%d) Attempted to increment past number of frames", _header->_id);
	}
}

void Sprite::process() {
	updateFrameState();
	// Sprites don't have time event handlers, separate timers do time handling.
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
	if (!_isActive) {
		return;
	}

	if (!_isPlaying) {
		if (_activeFrame != nullptr) {
			debugC(6, kDebugGraphics, "Sprite::updateFrameState(): (%d): Not playing. Persistent frame %d (%d x %d) @ (%d, %d)", 
				_header->_id, _activeFrame->index(), _activeFrame->width(), _activeFrame->height(), _activeFrame->left(), _activeFrame->top());
		} else {
			debugC(6, kDebugGraphics, "Sprite::updateFrameState(): (%d): Not playing, no persistent frame", _header->_id);
		}
		return;
	}

	debugC(5, kDebugGraphics, "Sprite::updateFrameState(): (%d) Frame %d (%d x %d) @ (%d, %d)",
		_header->_id, _activeFrame->index(), _activeFrame->width(), _activeFrame->height(), _activeFrame->left(), _activeFrame->top());

	uint currentTime = g_system->getMillis() - _startTime;
	bool drawNextFrame = currentTime >= _nextFrameTime;
	if (!drawNextFrame) {
		return;
	}

	showFrame(_frames[_currentFrameIndex]);

	uint frameDuration = 1000 / _header->_frameRate;
	_nextFrameTime = ++_currentFrameIndex * frameDuration;

	bool spriteFinishedPlaying = (_currentFrameIndex == _frames.size());
	if (spriteFinishedPlaying) {
		// Sprites always keep their last frame showing until they are hidden
		// with spatialHide.
		showFrame(_frames[_currentFrameIndex - 1]);
		_isPlaying = false;

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
	if (_activeFrame == nullptr || !_isShowing) {
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

void Sprite::showFrame(SpriteFrame *frame) {
	// Erase the previous frame.
	if (_activeFrame != nullptr) {
		g_engine->_dirtyRects.push_back(getActiveFrameBoundingBox());
	}
	
	// Show the next frame.
	_activeFrame = frame;
	if (frame != nullptr) {
		g_engine->_dirtyRects.push_back(getActiveFrameBoundingBox());
	}
}

Common::Rect Sprite::getActiveFrameBoundingBox() {
	// The frame dimensions are relative to those of the sprite movie.
	// So we must get the absolute coordinates.
	Common::Rect bbox = _activeFrame->boundingBox();
	bbox.translate(_header->_boundingBox->left, _header->_boundingBox->top);
	return bbox;
}

} // End of namespace MediaStation
