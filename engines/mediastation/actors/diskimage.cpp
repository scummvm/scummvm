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

#include "mediastation/actors/diskimage.h"
#include "mediastation/debugchannels.h"
#include "mediastation/graphics.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

DiskImageActor::~DiskImageActor() {
	if (isVisible()) {
		invalidateLocalBounds();
	}

	unregisterWithStreamManager();
	purge();
}

void DiskImageActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderChannelIdent:
		_channelIdent = chunk.readTypedChannelIdent();
		registerWithStreamManager();
		break;

	case kActorHeaderStartup:
		_isVisible = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderDiscardAfterUse:
		// The original just reads this and throws it away.
		chunk.readTypedUint16();
		break;

	case kActorHeaderLoadType:
		_shouldDecompressInPlace = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderInstallType:
		// In the original, this controls behavior if the files are NOT installed. But since
		// the "installation" is just copying from the CD-ROM, we can treat the game as always
		// installed. So just throw away this value.
		chunk.readTypedByte();
		break;

	case kActorHeaderDiskImageMaxStrips:
		_maxStripsInMemory = chunk.readTypedUint16();
		if (_maxStripsInMemory == 0) {
			_maxStripsInMemory = 1;
		}
		break;

	case kActorHeaderDiskImageStripWidth:
		_stripThickness = chunk.readTypedUint16();
		if (_stripThickness == 0) {
			_stripThickness = MediaStationEngine::SCREEN_WIDTH;
		}
		break;

	case kActorHeaderDiskImageUnk1:
		_useVerticalStrips = (static_cast<bool>(chunk.readTypedByte()));
		break;

	case kActorHeaderDiskImageMaxImages:
		_maxImagesInMemory = chunk.readTypedUint16();
		if (_maxImagesInMemory == 0) {
			_maxImagesInMemory = 1;
		}
		break;

	case kActorHeaderDiskImageStripInfo:
		setStripInfo(chunk);
		break;

	case kActorHeaderDiskImageUnkRect:
		// The original reads this rect and then immediately throws it away.
		chunk.readTypedRect();
		break;

	default:
		SpatialEntity::readParameter(chunk, paramType);
	}
}

ScriptValue DiskImageActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;
	switch (methodId) {
	case kSpatialShowMethod:
		ARGCOUNTCHECK(0);
		if (_isVisible == false) {
			_isVisible = true;
			invalidateLocalBounds();
		}
		break;

	case kSpatialHideMethod:
		ARGCOUNTCHECK(0);
		if (_isVisible == true) {
			_isVisible = false;
			invalidateLocalBounds();
		}
		break;

	// setDissolveFactor is handled elsewhere in our reimplementation.

	case kPreloadMethod: {
		ARGCOUNTCHECK(4);
		int16 left = static_cast<int16>(args[0].asFloat());
		int16 top = static_cast<int16>(args[1].asFloat());
		int16 width = static_cast<int16>(args[2].asFloat());
		int16 height = static_cast<int16>(args[3].asFloat());
		Common::Rect rectToPreload(Common::Point(left, top), width, height);
		Common::Point boundsOrigin = getBbox().origin();
		rectToPreload.translate(boundsOrigin.x, boundsOrigin.y);
		preload(rectToPreload);
		break;
	}

	case kPurgeMethod:
		ARGCOUNTCHECK(0);
		_isVisible = false;
		invalidateLocalBounds();
		purge();
		break;

	case kStopLoadMethod:
		ARGCOUNTCHECK(0);
		stopLoad();
		break;

	case kIsLoadingMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToBool(_isLoading);
		break;

	case kIsRectInMemoryMethod: {
		ARGCOUNTCHECK(4);
		int16 left = static_cast<int16>(args[0].asFloat());
		int16 top = static_cast<int16>(args[1].asFloat());
		int16 width = static_cast<int16>(args[2].asFloat());
		int16 height = static_cast<int16>(args[3].asFloat());
		Common::Rect rectToCheck(Common::Point(left, top), width, height);
		bool rectIsInMemory = isRectInMemory(rectToCheck);
		returnValue.setToBool(rectIsInMemory);
		break;
	}

	default:
		returnValue = SpatialEntity::callMethod(methodId, args);
	}

	return returnValue;
}

void DiskImageActor::onEvent(const ActorEvent &event) {
	switch (event.type) {
	case kCachingStartedEvent:
	case kCachingEndedEvent:
	case kCachingFailureEvent:
		// Caching-related events are not implemented, but they can be implemented
		// if the original CD-ROM streaming/caching logic is reimplemented.
		Actor::onEvent(event);
		break;

	default:
		runScriptResponseIfExists(event.type);
	}
}

void DiskImageActor::readChunk(Chunk &chunk) {
	uint index = chunk.readTypedUint16();
	ImageInfo imageInfo(chunk);

	if (!_stripInfoNodes.contains(index)) {
		error("[%s] %s: Strip index %d not found", debugName(), __func__, index);
	}
	StripInfoNode &stripInfoNode = _stripInfoNodes.getVal(index);

	if (_useVerticalStrips) {
		// Vertical strips. Arranged horizontally, so the left edge varies by strip index.
		Common::Point stripOrigin(
			_originalBoundingBox.left + (_stripThickness * index),
			_originalBoundingBox.top
		);
		stripInfoNode.rect = Common::Rect(stripOrigin, imageInfo._dimensions.x, imageInfo._dimensions.y);

	} else {
		// Horizontal strips. Arranged vertically, so the top edge varies by strip index.
		Common::Point stripOrigin(
			_originalBoundingBox.left,
			_originalBoundingBox.top + (_stripThickness * index)
		);
		stripInfoNode.rect = Common::Rect(stripOrigin, imageInfo._dimensions.x, imageInfo._dimensions.y);
	}
	stripInfoNode.isLoaded = true;

	PixMapImage *stripImage = new PixMapImage(chunk, imageInfo, _shouldDecompressInPlace);
	StripImageNode stripImageNode;
	stripImageNode.image = stripImage;
	// In the original, this does indeed get the absolute time, not gameplay time.
	stripImageNode.lastDrawTime = g_system->getMillis();
	stripImageNode.isLoaded = true;
	_stripImageNodes.setVal(index, stripImageNode);

	// This was in releaseBuffer in the original, but it's here since
	// we read data synchronously for now in the reimplementation.
	if (_isLoadingStrips) {
		if (isRectInMemory(_rectToLoad)) {
			if (_firePreloadEvent) {
				ActorEvent event(_id, kDiskImageActorStepEvent);
				g_engine->getEventLoop()->queueEvent(event);
			}
			stopLoad();
		} else {
			// The timer is meant to fire immediately.
			g_engine->getTimerService()->startTimer(_timer, (uint32)0);
			unregisterWithStreamManager();
		}
	}
}

void DiskImageActor::draw(DisplayContext &displayContext) {
	if (!_isVisible || !isReadyToDraw(displayContext)) {
		// Only draw if all required strips are ready.
		return;
	}

	// Draw each loaded strip that intersects the clip region.
	for (auto it = _stripInfoNodes.begin(); it != _stripInfoNodes.end(); ++it) {
		uint stripIndex = it->_key;
		const StripInfoNode &stripInfo = it->_value;

		if (stripInfo.isLoaded) {
			bool rectInClipRegion = displayContext.rectIsInClip(stripInfo.rect);
			if (rectInClipRegion) {
				// Update the last access time for cache management.
				if (!_stripImageNodes.contains(stripIndex)) {
					error("[%s] %s: Strip index %d marked as loaded but image node not found", debugName(), __func__, stripIndex);
				}
				StripImageNode &imageNode = _stripImageNodes.getVal(stripIndex);
				// In the original, this does indeed get the absolute time, not gameplay time.
				imageNode.lastDrawTime = g_system->getMillis();

				// Draw the strip image at its designated position.
				g_engine->getDisplayManager()->imageBlit(
					stripInfo.rect.origin(), imageNode.image, _dissolveFactor, &displayContext
				);
			}
		}
	}
}

void DiskImageActor::preload(const Common::Rect &rect, bool fireStepEvent) {
	// Check whether the area is already in memory.
	debugCN(5, kDebugGraphics, "[%s] %s: (%d, %d, %d, %d)", debugName(), __func__, PRINT_RECT(rect));
	Common::Rect rectToLoad = rect.findIntersectingRect(getBbox());
	bool rectIsAlreadyInMemory = isRectInMemory(rectToLoad);

	if (!rectIsAlreadyInMemory) {
		debugC(5, kDebugGraphics, " (not loaded)");
		setStripsToLoad(rectToLoad);
		_isLoading = true;
		_firePreloadEvent = fireStepEvent;
		_rectToLoad = rectToLoad;

		// The timer is meant to fire immediately.
		g_engine->getTimerService()->startTimer(_timer, (uint32)0);

	} else if (fireStepEvent) {
		debugC(5, kDebugGraphics, " (loaded)");
		ActorEvent event(_id, kDiskImageActorStepEvent);
		g_engine->getEventLoop()->queueEvent(event);
	}
}

bool DiskImageActor::isReadyToDraw(DisplayContext &displayContext) {
	for (auto it = _stripInfoNodes.begin(); it != _stripInfoNodes.end(); ++it) {
		const StripInfoNode &stripInfo = it->_value;
		bool stripRequiredButMissing = displayContext.rectIsInClip(stripInfo.rect) && !stripInfo.isLoaded;
		if (stripRequiredButMissing) {
			debugC(7, kDebugGraphics, "[%s] %s: Strip %d not loaded but in clip region", debugName(), __func__, it->_key);
			return false;
		}
	}

	return true;
}

bool DiskImageActor::isRectInMemory(const Common::Rect &rectToCheck) {
	// Check whether all strips that intersect with the specified rectangle are loaded.
	for (auto it = _stripInfoNodes.begin(); it != _stripInfoNodes.end(); ++it) {
		const StripInfoNode &stripInfo = it->_value;
		bool stripNotReady = !stripInfo.isLoaded || stripInfo.isLoadScheduled;
		bool stripNeededButNotReady = stripInfo.rect.intersects(rectToCheck) && stripNotReady;
		if (stripNeededButNotReady) {
			debugC(5, kDebugGraphics, "[%s] %s: Strip %d not loaded but in clip region", debugName(), __func__, it->_key);
			return false;
		}
	}

	return true;
}

void DiskImageActor::setStripInfo(Chunk &chunk) {
	StripInfoNode stripInfo;
	uint index = chunk.readTypedUint16();
	stripInfo.streamId = chunk.readTypedUint16();
	stripInfo.lengthInBytes = chunk.readTypedUint32();

	if (_useVerticalStrips) {
		// Calculate the left edge of this strip by offsetting from the original origin.
		Common::Point stripOrigin(
			_originalBoundingBox.left + (_stripThickness * index),
			_originalBoundingBox.top
		);

		// Each strip spans the full height of the original bounds.
		stripInfo.rect = Common::Rect(stripOrigin, _stripThickness, _originalBoundingBox.height());

	} else {
		// Calculate the top edge of this strip by offsetting from the original origin.
		Common::Point stripOrigin(
			_originalBoundingBox.left,
			_originalBoundingBox.top + (_stripThickness * index)
		);

		// Each strip spans the full width of the original bounds.
		stripInfo.rect = Common::Rect(stripOrigin, _originalBoundingBox.width(), _stripThickness);
	}

	_stripInfoNodes.setVal(index, stripInfo);
}

void DiskImageActor::setStripsToLoad(const Common::Rect &rectToLoad) {
	// Identify strips that intersect with the specified region and mark them for loading.
	for (auto it = _stripInfoNodes.begin(); it != _stripInfoNodes.end(); ++it) {
		StripInfoNode &stripInfo = it->_value;

		// Only consider strips that are not already loaded.
		if (!stripInfo.isLoaded) {
			bool stripIntersectsRectToLoad = stripInfo.rect.intersects(rectToLoad);
			if (stripIntersectsRectToLoad) {
				stripInfo.isLoadScheduled = true;
			}
		}
	}
}

void DiskImageActor::unloadLeastRecentlyDrawnStrip() {
	uint stripToUnload = UINT_MAX;
	uint oldestTime = UINT_MAX; // Maximum value, will be replaced by any valid strip
	for (auto it = _stripImageNodes.begin(); it != _stripImageNodes.end(); ++it) {
		uint stripIndex = it->_key;
		const StripImageNode &imageNode = it->_value;
		if (!imageNode.isLoaded) {
			continue;
		}

		// Find the oldest strip that doesn't intersect the viewport.
		const StripInfoNode &stripInfo = _stripInfoNodes.getVal(stripIndex);
		bool stripIntersectsViewport = stripInfo.rect.intersects(getBbox());
		bool stripDrawnBeforeCurrentOldest = imageNode.lastDrawTime < oldestTime;
		bool stripIsEligibleForUnload = !stripIntersectsViewport && stripDrawnBeforeCurrentOldest;
		if (stripIsEligibleForUnload) {
			oldestTime = imageNode.lastDrawTime;
			stripToUnload = stripIndex;
		}
	}

	// Unload the strip if it was found.
	if (stripToUnload != UINT_MAX) {
		StripImageNode &imageNode = _stripImageNodes.getVal(stripToUnload);
		delete imageNode.image;
		imageNode.image = nullptr;
		imageNode.isLoaded = false;
		_stripImageNodes.erase(stripToUnload);

		StripInfoNode &stripInfo = _stripInfoNodes.getVal(stripToUnload);
		stripInfo.isLoaded = false;
	}
}

int DiskImageActor::getStripToLoad() {
	if (_stripImageNodes.size() >= _maxImagesInMemory) {
		unloadLeastRecentlyDrawnStrip();
	}

	for (auto it = _stripInfoNodes.begin(); it != _stripInfoNodes.end(); ++it) {
		uint currentStripIndex = it->_key;
		const StripInfoNode &stripInfo = it->_value;

		// Find a strip that is scheduled to load but not yet loaded.
		if (stripInfo.isLoadScheduled && !stripInfo.isLoaded) {
			return currentStripIndex;
		}
	}
	return -1;
}

void DiskImageActor::startStripLoad(uint stripIndex) {
	// Retrieve the stream ID for the specified strip.
	registerWithStreamManager();
	StripInfoNode &stripInfo = _stripInfoNodes.getVal(stripIndex);
	stripInfo.isLoadScheduled = false;
	uint streamId = stripInfo.streamId;

	// Open a stream feed for this strip.
	_isLoadingStrips = true;
	ImtStreamFeed *streamFeed = g_engine->getStreamFeedManager()->openStreamFeed(streamId);
	streamFeed->readData();
	g_engine->getStreamFeedManager()->closeStreamFeed(streamFeed);
	_isLoadingStrips = false;
}

void DiskImageActor::timerEvent(const TimerEvent &event) {
	int stripId = getStripToLoad();
	if (stripId != -1) {
		startStripLoad(stripId);
	} else {
		if (_firePreloadEvent) {
			ActorEvent actorEvent(_id, kDiskImageActorEndEvent);
			g_engine->getEventLoop()->queueEvent(actorEvent);
		}
		_isLoading = false;
	}
}

void DiskImageActor::purge() {
	// Delete all loaded images.
	debugPrintNodes();
	stopLoad();
	for (auto it = _stripImageNodes.begin(); it != _stripImageNodes.end(); ++it) {
		StripImageNode &imageNode = it->_value;
		delete imageNode.image;
		imageNode.image = nullptr;
	}
	_stripImageNodes.clear();

	// Mark all strips as not loaded.
	for (auto it = _stripInfoNodes.begin(); it != _stripInfoNodes.end(); ++it) {
		StripInfoNode &stripInfo = it->_value;
		stripInfo.isLoaded = false;
		stripInfo.isLoadScheduled = false;
	}

	debugPrintNodes();
}

void DiskImageActor::stopLoad() {
	_isLoading = false;
	_firePreloadEvent = false;
	_rectToLoad = Common::Rect();

	unregisterWithStreamManager();
}

void DiskImageActor::debugPrintNodes() {
	debugC(5, kDebugGraphics, "[%s] %s: STRIP NODES [", debugName(), __func__);
	for (auto it = _stripInfoNodes.begin(); it != _stripInfoNodes.end(); ++it) {
		uint stripIndex = it->_key;
		const StripInfoNode &stripInfo = it->_value;
		bool hasImage = _stripImageNodes.contains(stripIndex) && _stripImageNodes.getVal(stripIndex).image != nullptr;
		debugC(5, kDebugGraphics, "\t\tStrip %d: loaded=%s, scheduled=%s, hasImage=%s, rect=(%d,%d,%d,%d)",
			stripIndex,
			stripInfo.isLoaded ? "yes" : "no",
			stripInfo.isLoadScheduled ? "yes" : "no",
			hasImage ? "yes" : "no",
			stripInfo.rect.left, stripInfo.rect.top, stripInfo.rect.right, stripInfo.rect.bottom);
	}
	debugC(5, kDebugGraphics, "]");
}

void DiskImageActor::setAdjustedBounds(CylindricalWrapMode wrapMode) {
	SpatialEntity::setAdjustedBounds(wrapMode);
	Common::Point boundsOffset = _boundingBox.origin() - _originalBoundingBox.origin();

	for (auto it = _stripInfoNodes.begin(); it != _stripInfoNodes.end(); ++it) {
		uint stripIndex = it->_key;
		StripInfoNode &stripInfo = it->_value;
		Common::Point adjustedOrigin;

		if (_useVerticalStrips) {
			// Strips arranged left to right.
			adjustedOrigin.x = _originalBoundingBox.left + (stripIndex * _stripThickness) + boundsOffset.x;
			adjustedOrigin.y = _originalBoundingBox.top + boundsOffset.y;
		} else {
			// Strips arranged top to bottom.
			adjustedOrigin.x = _originalBoundingBox.left + boundsOffset.x;
			adjustedOrigin.y = _originalBoundingBox.top + (stripIndex * _stripThickness) + boundsOffset.y;
		}

		// Update the strip's origin whilst preserving its dimensions.
		stripInfo.rect.moveTo(adjustedOrigin.x, adjustedOrigin.y);
	}
}

} // End of namespace MediaStation
