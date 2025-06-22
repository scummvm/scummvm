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

#include "mediastation/assets/sprite.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

SpriteFrameHeader::SpriteFrameHeader(Chunk &chunk) : BitmapHeader(chunk) {
	_index = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "SpriteFrameHeader::SpriteFrameHeader(): _index = 0x%x (@0x%llx)", _index, static_cast<long long int>(chunk.pos()));
	_boundingBox = chunk.readTypedPoint();
	debugC(5, kDebugLoading, "SpriteFrameHeader::SpriteFrameHeader(): _boundingBox (@0x%llx)", static_cast<long long int>(chunk.pos()));
}

SpriteFrame::SpriteFrame(Chunk &chunk, SpriteFrameHeader *header) : Bitmap(chunk, header) {
	_bitmapHeader = header;
}

SpriteFrame::~SpriteFrame() {
	// The base class destructor takes care of deleting the bitmap header.
}

uint32 SpriteFrame::left() {
	return _bitmapHeader->_boundingBox.x;
}

uint32 SpriteFrame::top() {
	return _bitmapHeader->_boundingBox.y;
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
	// If we're just referencing another asset's frames,
	// don't delete those frames.
	if (_assetReference == 0) {
		for (SpriteFrame *frame : _frames) {
			delete frame;
		}
	}
	_frames.clear();
}

void Sprite::readParameter(Chunk &chunk, AssetHeaderSectionType paramType) {
	switch (paramType) {
	case kAssetHeaderChunkReference:
		_chunkReference = chunk.readTypedChunkReference();
		break;

	case kAssetHeaderFrameRate:
		_frameRate = static_cast<uint32>(chunk.readTypedDouble());
		break;

	case kAssetHeaderLoadType:
		_loadType = chunk.readTypedByte();
		break;

	case kAssetHeaderStartup:
		_isVisible = static_cast<bool>(chunk.readTypedByte());
		break;

	case kAssetHeaderSpriteChunkCount: {
		_frameCount = chunk.readTypedUint16();

		// Set the default clip.
		SpriteClip clip;
		clip.id = DEFAULT_CLIP_ID;
		clip.firstFrameIndex = 0;
		clip.lastFrameIndex = _frameCount - 1;
		_clips.setVal(clip.id, clip);
		setCurrentClip(clip.id);
		break;
	}

	case kAssetHeaderSpriteClip: {
		SpriteClip spriteClip;
		spriteClip.id = chunk.readTypedUint16();
		spriteClip.firstFrameIndex = chunk.readTypedUint16();
		spriteClip.lastFrameIndex = chunk.readTypedUint16();
		_clips.setVal(spriteClip.id, spriteClip);
		break;
	}

	case kAssetHeaderCurrentSpriteClip: {
		uint clipId = chunk.readTypedUint16();
		setCurrentClip(clipId);
		break;
	}

	default:
		SpatialEntity::readParameter(chunk, paramType);
	}
}

ScriptValue Sprite::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (methodId) {
	case kSpatialShowMethod: {
		assert(args.empty());
		setVisibility(true);
		return returnValue;
	}

	case kSpatialHideMethod: {
		assert(args.empty());
		setVisibility(false);
		return returnValue;
	}

	case kTimePlayMethod: {
		assert(args.empty());
		play();
		return returnValue;
	}

	case kTimeStopMethod: {
		assert(args.empty());
		stop();
		return returnValue;
	}

	case kMovieResetMethod: {
		assert(args.empty());
		setCurrentFrameToInitial();
		return returnValue;
	}

	case kSetCurrentClipMethod: {
		assert(args.size() <= 1);
		uint clipId = DEFAULT_CLIP_ID;
		if (args.size() == 1) {
			clipId = args[0].asParamToken();
		}
		setCurrentClip(clipId);
		return returnValue;
	}

	case kIncrementFrameMethod: {
		assert(args.size() <= 1);
		bool loopAround = false;
		if (args.size() == 1) {
			loopAround = args[0].asBool();
		}

		bool moreFrames = activateNextFrame();
		if (!moreFrames) {
			if (loopAround) {
				setCurrentFrameToInitial();
			}
		}
		return returnValue;
	}

	case kDecrementFrameMethod: {
		bool shouldSetCurrentFrameToFinal = false;
		if (args.size() == 1) {
			shouldSetCurrentFrameToFinal = args[0].asBool();
		}

		bool moreFrames = activatePreviousFrame();
		if (!moreFrames) {
			if (shouldSetCurrentFrameToFinal) {
				setCurrentFrameToFinal();
			}
		}
		return returnValue;
	}

	case kGetCurrentClipIdMethod: {
		returnValue.setToParamToken(_activeClip.id);
		return returnValue;
	}

	case kIsPlayingMethod: {
		returnValue.setToBool(_isPlaying);
		return returnValue;
	}

	default:
		return SpatialEntity::callMethod(methodId, args);
	}
}

bool Sprite::activateNextFrame() {
	if (_currentFrameIndex < _activeClip.lastFrameIndex) {
		_currentFrameIndex++;
		dirtyIfVisible();
		return true;
	}
	return false;
}

bool Sprite::activatePreviousFrame() {
	if (_currentFrameIndex > _activeClip.firstFrameIndex) {
		_currentFrameIndex--;
		dirtyIfVisible();
		return true;
	}
	return false;
}

void Sprite::dirtyIfVisible() {
	if (_isVisible) {
		invalidateLocalBounds();
	}
}

void Sprite::setVisibility(bool visibility) {
	if (_isVisible != visibility) {
		_isVisible = visibility;
		invalidateLocalBounds();
	}
}

void Sprite::play() {
	_isPlaying = true;
	_startTime = g_system->getMillis();
	_lastProcessedTime = 0;
	_nextFrameTime = 0;

	scheduleNextFrame();
}

void Sprite::stop() {
	_nextFrameTime = 0;
	_isPlaying = false;
}

void Sprite::setCurrentClip(uint clipId) {
	if (_activeClip.id != clipId) {
		if (_clips.contains(clipId)) {
			_activeClip = _clips.getVal(clipId);
		} else {
			_activeClip.id = clipId;
			warning("Sprite clip %d not found in sprite %d", clipId, _id);
		}
	}

	setCurrentFrameToInitial();
}

void Sprite::setCurrentFrameToInitial() {
	if (_currentFrameIndex != _activeClip.firstFrameIndex) {
		_currentFrameIndex = _activeClip.firstFrameIndex;
		dirtyIfVisible();
	}
}

void Sprite::setCurrentFrameToFinal() {
	if (_currentFrameIndex != _activeClip.lastFrameIndex) {
		_currentFrameIndex = _activeClip.lastFrameIndex;
		dirtyIfVisible();
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
	Common::sort(_frames.begin(), _frames.end(), [](SpriteFrame *a, SpriteFrame *b) {
		return a->index() < b->index();
	});
}

void Sprite::scheduleNextFrame() {
	if (!_isPlaying) {
		return;
	}

	if (_currentFrameIndex < _activeClip.lastFrameIndex) {
		scheduleNextTimerEvent();
	} else {
		stop();
	}
}

void Sprite::scheduleNextTimerEvent() {
	uint frameDuration = 1000 / _frameRate;
	_nextFrameTime += frameDuration;
}

void Sprite::updateFrameState() {
	if (!_isPlaying) {
		return;
	}

	uint currentTime = g_system->getMillis() - _startTime;
	bool drawNextFrame = currentTime >= _nextFrameTime;
	debugC(kDebugGraphics, "nextFrameTime: %d; startTime: %d, currentTime: %d", _nextFrameTime, _startTime, currentTime);
	if (drawNextFrame) {
		timerEvent();
	}
}

void Sprite::timerEvent() {
	if (!_isPlaying) {
		error("Attempt to activate sprite frame when sprite is not playing");
		return;
	}

	bool result = activateNextFrame();
	if (!result) {
		stop();
	} else {
		postMovieEndEventIfNecessary();
		scheduleNextFrame();
	}
}

void Sprite::postMovieEndEventIfNecessary() {
	if (_currentFrameIndex != _activeClip.lastFrameIndex) {
		return;
	}

	_isPlaying = false;
	_startTime = 0;
	_nextFrameTime = 0;

	ScriptValue value;
	value.setToParamToken(_activeClip.id);
	runEventHandlerIfExists(kSpriteMovieEndEvent, value);
}

void Sprite::draw(const Common::Array<Common::Rect> &dirtyRegion) {
	SpriteFrame *activeFrame = _frames[_currentFrameIndex];
	if (_isVisible) {
		Common::Rect frameBbox = activeFrame->boundingBox();
		frameBbox.translate(_boundingBox.left, _boundingBox.top);
		g_engine->getDisplayManager()->imageBlit(frameBbox.origin(), activeFrame, _dissolveFactor, dirtyRegion);
	}
}

} // End of namespace MediaStation
