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

Sprite::~Sprite() {
	_frames.clear();
}

Operand Sprite::callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) {
	switch (methodId) {
	case kSpatialShowMethod: {
		assert(args.size() == 0);
		spatialShow();
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

	// Persist the first frame.
	// TODO: Is there anything that says what the persisted frame should be?
	SpriteFrame *firstFrame = _frames[0];
	for (SpriteFrame *frame : _frames) {
		if (frame->index() < firstFrame->index()) {
			firstFrame = frame;
		}
	}
	_persistFrame = firstFrame;
}

void Sprite::timePlay() {
	debugC(5, kDebugScript, "Called Sprite::timePlay");
	_isActive = true;
	_persistFrame = nullptr;
	_startTime = g_system->getMillis();
	_lastProcessedTime = 0;
	_nextFrameTime = 0;
	g_engine->addPlayingAsset(this);

	if (_header->_frameRate == 0) {
		_header->_frameRate = 10;
	}

	runEventHandlerIfExists(kMovieBeginEvent);
}

void Sprite::movieReset() {
	debugC(5, kDebugScript, "Called Sprite::movieReset");
	_isActive = true;
	// We do NOT reset the persisting frame, because it should keep showing!
	_startTime = 0;
	_currentFrameIndex = 0;
	_nextFrameTime = 0;
	_lastProcessedTime = 0;
}

void Sprite::process() {
	drawNextFrame();

	// TODO: I don't think sprites support time-based event handlers. Because we
	// have a separate timer for restarting the sprite when it expires.
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

void Sprite::drawNextFrame() {
	// TODO: With a dirty rect-based system, we would only need to redraw the frame
	// when it NEEDS to be redrawn. But since the whole screen is currently redrawn
	// every time, the persisting frame needs to be redrawn too.
	bool redrawPersistentFrame = _persistFrame != nullptr;
	if (redrawPersistentFrame) {
		debugC(5, kDebugGraphics, "GRAPHICS (Sprite %d): Drawing persistent frame %d", _header->_id, _persistFrame->index());
		drawFrame(_persistFrame);
		return;
	}

	uint currentTime = g_system->getMillis() - _startTime;
	bool redrawCurrentFrame = currentTime <= _nextFrameTime;
	if (redrawCurrentFrame) {
		// Just redraw the current frame in case it was covered over.
		// See TODO above.
		SpriteFrame *currentFrame = _frames[_currentFrameIndex];
		debugC(5, kDebugGraphics, "GRAPHICS (Sprite %d): Re-drawing current frame %d", _header->_id, currentFrame->index());
		drawFrame(currentFrame);
		return;
	}

	SpriteFrame *nextFrame = _frames[_currentFrameIndex];
	debugC(5, kDebugGraphics, "GRAPHICS (Sprite %d): Drawing next frame %d (@%d)", _header->_id, nextFrame->index(), _nextFrameTime);
	uint frameDuration = 1000 / _header->_frameRate;
	_nextFrameTime = _currentFrameIndex * frameDuration;
	drawFrame(nextFrame);

	bool spriteFinishedPlaying = (++_currentFrameIndex == _frames.size());
	if (spriteFinishedPlaying) {
		// Sprites always keep their last frame showing until they are hidden
		// with spatialHide.
		_persistFrame = _frames[_currentFrameIndex - 1];
		_isActive = true;

		// But otherwise, the sprite's params should be reset.
		_startTime = 0;
		_lastProcessedTime = 0;
		_currentFrameIndex = 0;
		_nextFrameTime = 0;

		runEventHandlerIfExists(kSpriteMovieEndEvent);
	}
}

void Sprite::drawFrame(SpriteFrame *frame) {
	uint frameLeft = frame->left() + _header->_boundingBox->left;
	uint frameTop = frame->top() + _header->_boundingBox->top;
	debugC(5, kDebugGraphics, "    Sprite frame %d (%d x %d) @ (%d, %d)", frame->index(), frame->width(), frame->height(), frameLeft, frameTop);
	g_engine->_screen->simpleBlitFrom(frame->_surface, Common::Point(frameLeft, frameTop));
}

} // End of namespace MediaStation
