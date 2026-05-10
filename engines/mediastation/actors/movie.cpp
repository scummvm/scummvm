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

#include "mediastation/actors/movie.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

StreamMovieProxy::StreamMovieProxy(Chunk &chunk, StreamMovieActor *parent) : SpatialEntity(kActorTypeStreamMovieProxy) {
	_layerId = chunk.readTypedUint32();
	_scriptId = chunk.readTypedUint16();
	uint zIndex = chunk.readTypedSint16();
	setZIndex(zIndex);
	setBounds(parent->getBbox());
	debugC(5, kDebugLoading, "[%s] %s: layerId: %d; scriptId: %d; zIndex: %d",
		parent->debugName(), __func__, _layerId, _scriptId, zIndex);
	_isVisible = true;
	_parent = parent;
}

void StreamMovieProxy::draw(DisplayContext &displayContext) {
	if (_parent != nullptr) {
		_parent->drawLayer(displayContext, _layerId);
	} else {
		warning("[%s] %s: Stream movie proxy has no parent", debugName(), __func__);
	}
}

bool StreamMovieProxy::isVisible() const {
	if (_isVisible) {
		if (_parentStage != nullptr) {
			return _parentStage->isVisible();
		}
	}
	return false;
}

MovieFrame::MovieFrame(Chunk &chunk) {
	if (g_engine->getImtGod()->isFirstGenerationEngine()) {
		blitType = static_cast<MovieBlitType>(chunk.readTypedUint16());
		startInMilliseconds = chunk.readTypedUint32();
		endInMilliseconds = chunk.readTypedUint32();
		// These are unsigned in the data files but ScummVM expects signed.
		leftTop.x = static_cast<int16>(chunk.readTypedUint16());
		leftTop.y = static_cast<int16>(chunk.readTypedUint16());
		index = chunk.readTypedUint32();
		keyframeIndex = chunk.readTypedUint32();
		keepAfterEnd = chunk.readTypedByte();
	} else {
		layerId = chunk.readTypedUint32();
		blitType = static_cast<MovieBlitType>(chunk.readTypedUint16());
		startInMilliseconds = chunk.readTypedUint32();
		endInMilliseconds = chunk.readTypedUint32();
		// These are unsigned in the data files but ScummVM expects signed.
		leftTop.x = static_cast<int16>(chunk.readTypedUint16());
		leftTop.y = static_cast<int16>(chunk.readTypedUint16());
		zIndex = chunk.readTypedSint16();
		// This represents the difference between the left-top coordinate of the
		// keyframe (if applicable) and the left coordinate of this frame. Zero
		// if there is no keyframe.
		diffBetweenKeyframeAndFrame.x = chunk.readTypedSint16();
		diffBetweenKeyframeAndFrame.y = chunk.readTypedSint16();
		index = chunk.readTypedUint32();
		keyframeIndex = chunk.readTypedUint32();
		keepAfterEnd = chunk.readTypedByte();
	}
}

MovieFrameImage::MovieFrameImage(Chunk &chunk, uint index, uint keyframeEndInMilliseconds, const ImageInfo &imageInfo) :
	PixMapImage(chunk, imageInfo), _index(index), _keyframeEndInMilliseconds(keyframeEndInMilliseconds) {
	debugC(5, kDebugLoading, "%s: frame 0x%x", __func__, _index);
}

StreamMovieActor::~StreamMovieActor() {
	unregisterWithStreamManager();
	unregisterForSyncCalls();
	if (_streamFeed != nullptr) {
		g_engine->getStreamFeedManager()->closeStreamFeed(_streamFeed);
		_streamFeed = nullptr;
	}

	delete _streamFrames;
	_streamFrames = nullptr;

	delete _streamSound;
	_streamSound = nullptr;

	for (StreamMovieProxy *proxy : _proxies) {
		delete proxy;
	}
	_proxies.clear();
}

void StreamMovieActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderActorId: {
		// We already have this actor's ID, so we will just verify it is the same
		// as the ID we have already read.
		uint32 duplicateActorId = chunk.readTypedUint16();
		if (duplicateActorId != _id) {
			warning("[%s] %s: Duplicate actor ID %d does not match original ID %d", debugName(), __func__, duplicateActorId, _id);
		}
		break;
	}

	case kActorHeaderChannelIdent:
		_channelIdent = chunk.readTypedChannelIdent();
		registerWithStreamManager();
		break;


	case kActorHeaderStartup:
		_isVisible = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderDiscardAfterUse:
		// The original just reads this and throws it away.
		chunk.readTypedByte();
		break;

	case kActorHeaderMovieAudioChannelIdent: {
		ChannelIdent soundChannelIdent = chunk.readTypedChannelIdent();
		_streamSound->setChannelIdent(soundChannelIdent);
		_streamSound->registerWithStreamManager();
		break;
	}

	case kActorHeaderCachingEnabled:
		_shouldCache = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderInstallType:
		// In the original, this controls behavior if the files are NOT installed. But since
		// the "installation" is just copying from the CD-ROM, we can treat the game as always
		// installed. So just throw away this value.
		chunk.readTypedByte();
		break;

	case kActorHeaderMovieAnimationChannelIdent: {
		ChannelIdent framesChannelIdent = chunk.readTypedChannelIdent();
		_streamFrames->setChannelIdent(framesChannelIdent);
		_streamFrames->registerWithStreamManager();
		break;
	}

	case kActorHeaderSoundInfo:
		_streamSound->getAudioSequence().readParameters(chunk);
		break;

	case kStreamMovieProxyInfo: {
		StreamMovieProxy *proxy = new StreamMovieProxy(chunk, this);
		_proxies.push_back(proxy);
		break;
	}

	default:
		SpatialEntity::readParameter(chunk, paramType);
	}
}

ScriptValue StreamMovieActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (methodId) {
	case kSpatialShowMethod: {
		if (args.empty()) {
			// Set our visibility directly.
			setVisibility(true);
			updateFrameState();
		} else {
			// Set the visibility of a proxy.
			uint scriptId = args[0].asParamToken();
			StreamMovieProxy *proxy = proxyOfScriptId(scriptId);
			if (proxy != nullptr) {
				if (!proxy->isVisible()) {
					proxy->_isVisible = true;
				}
			} else {
				warning("[%s] %s: Stream movie proxy with script ID %s doesn't exist",
					debugName(), __func__, g_engine->formatParamTokenName(scriptId).c_str());
			}
		}
		break;
	}

	case kSpatialHideMethod: {
		if (args.empty()) {
			// Set our visibility directly.
			setVisibility(false);
			updateFrameState();
		} else {
			// Set the visibility of a proxy.
			uint scriptId = args[0].asParamToken();
			StreamMovieProxy *proxy = proxyOfScriptId(scriptId);
			if (proxy != nullptr) {
				if (proxy->isVisible()) {
					proxy->_isVisible = false;
				}
			} else {
				warning("[%s] %s: Stream movie proxy with script ID %s doesn't exist",
					debugName(), __func__, g_engine->formatParamTokenName(scriptId).c_str());
			}
		}
		break;
	}

	case kTimePlayMethod: {
		ARGCOUNTCHECK(0);
		timePlay();
		break;
	}

	case kTimeStopMethod: {
		ARGCOUNTCHECK(0);
		timeStop(false);
		break;
	}

	case kStreamMovieSetProxyZIndex: {
		ARGCOUNTCHECK(2);
		uint scriptId = args[0].asParamToken();
		int zIndex = static_cast<int>(args[1].asFloat());
		StreamMovieProxy *proxy = proxyOfScriptId(scriptId);
		if (proxy != nullptr) {
			proxy->setZIndex(zIndex);
		} else {
			warning("[%s] %s: Stream movie proxy with script ID %s doesn't exist",
				debugName(), __func__, g_engine->formatParamTokenName(scriptId).c_str());
		}
		break;
	}

	case kStreamMovieGetProxyZIndex: {
		ARGCOUNTCHECK(2);
		uint scriptId = args[0].asParamToken();
		StreamMovieProxy *proxy = proxyOfScriptId(scriptId);
		if (proxy != nullptr) {
			returnValue.setToFloat(proxy->zIndex());
		} else {
			warning("[%s] %s: Stream movie proxy with script ID %s doesn't exist",
				debugName(), __func__, g_engine->formatParamTokenName(scriptId).c_str());
		}
		break;
	}

	case kIsPlayingMethod: {
		ARGCOUNTCHECK(0);
		returnValue.setToBool(_isPlaying);
		break;
	}

	case kGetLeftXMethod: {
		ARGCOUNTCHECK(0);
		double left = static_cast<double>(_boundingBox.left);
		returnValue.setToFloat(left);
		break;
	}

	case kGetTopYMethod: {
		ARGCOUNTCHECK(0);
		double top = static_cast<double>(_boundingBox.top);
		returnValue.setToFloat(top);
		break;
	}

	case kStreamMovieMoveProxyToStageMethod: {
		ARGCOUNTCHECK(2);
		uint scriptId = args[0].asParamToken();
		uint targetStageId = args[1].asActorId();
		StreamMovieProxy *proxy = proxyOfScriptId(scriptId);
		if (proxy == nullptr) {
			warning("[%s] %s: Stream movie proxy with script ID %s doesn't exist",
				debugName(), __func__, g_engine->formatParamTokenName(scriptId).c_str());
			break;
		}
		StageActor *parentStage = static_cast<StageActor *>(g_engine->getImtGod()->getActorByIdAndType(targetStageId, kActorTypeStage));
		if (parentStage == nullptr) {
			warning("[%s] %s: Stream movie proxy with script ID %s has null parent stage",
				debugName(), __func__, g_engine->formatParamTokenName(scriptId).c_str());
			break;
		}

		proxy->getParentStage()->removeChildSpatialEntity(proxy);
		parentStage->addChildSpatialEntity(proxy);
		break;
	}

	case kStreamMovieMoveProxyToRootStageMethod: {
		ARGCOUNTCHECK(2);
		uint scriptId = args[0].asParamToken();
		uint sourceStageId = args[1].asActorId();
		StreamMovieProxy *proxy = proxyOfScriptId(scriptId);
		if (proxy == nullptr) {
			warning("[%s] %s: Stream movie proxy with script ID %s doesn't exist",
				debugName(), __func__, g_engine->formatParamTokenName(scriptId).c_str());
			break;
		}

		RootStage *rootStage = g_engine->getRootStage();
		StageActor *sourceStage = static_cast<StageActor *>(g_engine->getImtGod()->getActorByIdAndType(sourceStageId, kActorTypeStage));
		if (sourceStage == nullptr) {
			warning("[%s] %s: Stream movie proxy with script ID %s has null parent stage",
				debugName(), __func__, g_engine->formatParamTokenName(scriptId).c_str());
			break;
		}

		sourceStage->removeChildSpatialEntity(proxy);
		rootStage->addChildSpatialEntity(proxy);
		break;
	}

	default:
		returnValue = SpatialEntity::callMethod(methodId, args);
	}
	return returnValue;
}

void StreamMovieActor::onEvent(const ActorEvent &event) {
	switch (event.type) {
	case kMovieEndEvent:
		triggerRemainingTimerEvents();
		break;

	case kCachingStartedEvent:
	case kCachingEndedEvent:
	case kCachingFailureEvent:
		// Caching-related events are not implemented, but they can be implemented
		// if the original CD-ROM streaming/caching logic is reimplemented.
		Actor::onEvent(event);
		break;

	case kMovieStoppedEvent:
	case kMovieAbortEvent:
	case kMovieFailureEvent:
		_isPlaying = false;
		break;

	default:
		break;
	}

	runScriptResponseIfExists(event.type);
}

void StreamMovieActor::timerEvent(const TimerEvent &event) {
	Actor::processTimeScriptResponses();

	g_engine->getTimerService()->stopTimer(_timer);
	setupNextScriptResponseTimer();
}

void StreamMovieActor::timePlay() {
	if (_streamFeed == nullptr) {
		_streamFeed = g_engine->getStreamFeedManager()->openStreamFeed(_id);
		_streamFeed->readData();
	}

	if (_isPlaying) {
		return;
	}

	setupNextScriptResponseTimer();
	registerForSyncCalls();
	_streamSound->getAudioSequence().start();
	_framesNotYetShown = _streamFrames->_frames;
	_framesOnScreen.clear();
	_isPlaying = true;
	_startTime = g_engine->getTotalPlayTime();
	_lastProcessedTime = 0;

	ActorEvent actorEvent(_id, kMovieBeginEvent);
	g_engine->getEventLoop()->queueEvent(actorEvent);

	if (_disableScreenAutoUpdateToken == 0) {
		_disableScreenAutoUpdateToken = g_engine->getDisplayUpdateManager()->disableAutoUpdate();

		g_engine->getDisplayUpdateManager()->enableAutoUpdate(_disableScreenAutoUpdateToken);
		_disableScreenAutoUpdateToken = 0;
	}

	updateFrameState();
}

void StreamMovieActor::timeStop(bool isMovieEnd) {
	if (!_isPlaying) {
		return;
	}

	_isPlaying = false;
	_streamSound->getAudioSequence().stop();
	g_engine->getTimerService()->stopTimer(_timer);
	unregisterForSyncCalls();

	_framesNotYetShown.clear();
	if (_hasStill) {
		_framesNotYetShown = _streamFrames->_frames;
	}
	_framesOnScreen.clear();

	EventType eventType = isMovieEnd ? kMovieEndEvent : kMovieStoppedEvent;
	ActorEvent actorEvent(_id, eventType);
	g_engine->getEventLoop()->queueEvent(actorEvent);

	if (_disableScreenAutoUpdateToken == 0) {
		_disableScreenAutoUpdateToken = g_engine->getDisplayUpdateManager()->disableAutoUpdate();

		DisplayEvent event(kDisplayEnableAutoUpdateEvent, _disableScreenAutoUpdateToken);
		g_engine->getEventLoop()->queueEvent(event);
		_disableScreenAutoUpdateToken = 0;
	}

	updateFrameState();
}

PreDisplaySyncState StreamMovieActor::preDisplaySync() {
	if (!_isPlaying) {
		return kPreDisplaySyncNoScreenUpdateRequested;
	}

	// Update frame state if visible.
	if (_isVisible) {
		updateFrameState();
		return kPreDisplaySyncForceScreenUpdate;
	} else {
		// Request display update when playing.
		return kPreDisplaySyncNoScreenUpdateRequested;
	}
}

void StreamMovieActor::setVisibility(bool visibility) {
	if (visibility != _isVisible) {
		_isVisible = visibility;
		invalidateLocalBounds();
	}
}

void StreamMovieActor::updateFrameState() {
	uint movieTime = 0;
	if (_isPlaying) {
		uint currentTime = g_engine->getTotalPlayTime();
		movieTime = currentTime - _startTime;
	}
	debugC(7, kDebugGraphics, "[%s] %s: Starting update (movie time: %d)", debugName(), __func__, movieTime);

	// This complexity is necessary becuase movies can have more than one frame
	// showing at the same time - for instance, a movie background and an
	// animation on that background are a part of the saem movie and are on
	// screen at the same time, it's just the starting and ending times of one
	// can be different from the starting and ending times of another.
	//
	// We can rely on the frames being ordered in order of their start. First,
	// see if there are any new frames to show.
	for (auto it = _framesNotYetShown.begin(); it != _framesNotYetShown.end();) {
		MovieFrame *frame = *it;
		bool isAfterStart = movieTime >= frame->startInMilliseconds;
		if (isAfterStart) {
			_framesOnScreen.insert(frame);
			invalidateRect(getFrameBoundingBox(frame));

			// We don't need ++it because we will either have another frame
			// that needs to be drawn, or we have reached the end of the new
			// frames.
			it = _framesNotYetShown.erase(it);
		} else {
			// We've hit a frame that shouldn't yet be shown.
			// Rely on the ordering to not bother with any further frames.
			break;
		}
	}

	// Now see if there are any old frames that no longer need to be shown.
	for (auto it = _framesOnScreen.begin(); it != _framesOnScreen.end();) {
		MovieFrame *frame = *it;
		bool isAfterEnd = movieTime >= frame->endInMilliseconds;
		if (isAfterEnd) {
			invalidateRect(getFrameBoundingBox(frame));
			it = _framesOnScreen.erase(it);

			if (_framesOnScreen.empty() && movieTime >= _fullTime) {
				timeStop(true);
				break;
			}
		} else {
			++it;
		}
	}

	// Show the frames that are currently active, for debugging purposes.
	for (MovieFrame *frame : _framesOnScreen) {
		debugC(8, kDebugGraphics, "[%s] %s: (time: %d ms) Frame %d (%d x %d) @ (%d, %d); start: %d ms, end: %d ms, zIndex = %d",
			debugName(), __func__, movieTime, frame->index, frame->image->width(), frame->image->height(), frame->leftTop.x, frame->leftTop.y, frame->startInMilliseconds, frame->endInMilliseconds, frame->zIndex);
	}
}

void StreamMovieActor::draw(DisplayContext &displayContext) {
	const uint DEFAULT_LAYER_ID = 0;
	drawLayer(displayContext, DEFAULT_LAYER_ID);
}

void StreamMovieActor::drawLayer(DisplayContext &displayContext, uint layerId) {
	for (MovieFrame *frame : _framesOnScreen) {
		if (frame->layerId != layerId) {
			continue;
		}

		Common::Rect bbox = getFrameBoundingBox(frame);
		debugC(8, kDebugGraphics, "[%s] %s: layer %d, frame %d (%d, %d, %d, %d)",
			debugName(), __func__, layerId, frame->index, PRINT_RECT(bbox));

		switch (frame->blitType) {
		case kUncompressedMovieBlit:
			g_engine->getDisplayManager()->imageBlit(bbox.origin(), frame->image, _dissolveFactor, &displayContext);
			break;

		case kUncompressedDeltaMovieBlit:
			g_engine->getDisplayManager()->imageDeltaBlit(
				bbox.origin(), frame->diffBetweenKeyframeAndFrame,
				frame->image, frame->keyframeImage, _dissolveFactor, &displayContext);
			break;

		case kCompressedDeltaMovieBlit:
			if (frame->keyframeImage->isCompressed()) {
				decompressIntoAuxImage(frame);
			}
			g_engine->getDisplayManager()->imageDeltaBlit(
				bbox.origin(), frame->diffBetweenKeyframeAndFrame,
				frame->image, frame->keyframeImage, _dissolveFactor, &displayContext);
			break;

		default:
			error("%s: Got unknown movie frame blit type: %d", __func__, frame->blitType);
		}
	}
}

void StreamMovieActor::invalidateLocalBounds() {
	for (StreamMovieProxy *proxy : _proxies) {
		// Our bounds might have changed, so pass that on to the proxies.
		proxy->setBounds(getBbox());
	}
	SpatialEntity::invalidateLocalBounds();
}

Common::Rect StreamMovieActor::getFrameBoundingBox(MovieFrame *frame) {
	// Use _boundingBox directly (which may be temporarily offset by camera rendering)
	// The camera offset is already applied to _boundingBox by pushBoundingBoxOffset()
	Common::Point origin = _boundingBox.origin() + frame->leftTop;
	Common::Rect bbox = Common::Rect(origin, frame->image->width(), frame->image->height());
	return bbox;
}

StreamMovieActorFrames::~StreamMovieActorFrames() {
	unregisterWithStreamManager();

	for (MovieFrame *frame : _frames) {
		delete frame;
	}
	_frames.clear();

	for (MovieFrameImage *image : _images) {
		delete image;
	}
	_images.clear();
}

void StreamMovieActorFrames::readChunk(Chunk &chunk) {
	uint sectionType = chunk.readTypedUint16();
	switch (static_cast<MovieSectionType>(sectionType)) {
	case kMovieImageDataSection:
		readImageData(chunk);
		break;

	case kMovieFrameDataSection:
		readFrameData(chunk);
		break;

	default:
		error("%s: Unknown movie still section type", __func__);
	}

	for (MovieFrame *frame : _frames) {
		if (frame->endInMilliseconds > _parent->_fullTime) {
			_parent->_fullTime = frame->endInMilliseconds;
		}
		if (frame->keepAfterEnd) {
			_parent->_hasStill = true;
		}
	}

	if (_parent->_hasStill) {
		_parent->_framesNotYetShown = _frames;
	}
}

void StreamMovieActor::loadIsComplete() {
	SpatialEntity::loadIsComplete();
	updateFrameState();
}

StreamMovieActorSound::~StreamMovieActorSound() {
	unregisterWithStreamManager();
}

void StreamMovieActorSound::readChunk(Chunk &chunk) {
	_audioSequence.readChunk(chunk);
}

void StreamMovieActorSound::soundPlayStateChanged(SoundPlayState state, SoundStopReason why) {
	if (state == kSoundPlayStateStopped) {
		if (why == kSoundStopForFailure) {
			ActorEvent actorEvent(_parent->_id, kMovieFailureEvent);
			g_engine->getEventLoop()->queueEvent(actorEvent);
		}
	}
}

StreamMovieActor::StreamMovieActor() :
	_framesOnScreen(StreamMovieActor::compareFramesByZIndex), SpatialEntity(kActorTypeMovie) {
	_streamFrames = new StreamMovieActorFrames(this);
	_streamSound = new StreamMovieActorSound(this);
}

void StreamMovieActor::readChunk(Chunk &chunk) {
	MovieSectionType sectionType = static_cast<MovieSectionType>(chunk.readTypedUint16());
	if (sectionType == kMovieRootSection) {
		parseMovieHeader(chunk);
	} else if (sectionType == kMovieChunkMarkerSection) {
		parseMovieChunkMarker(chunk);
	} else {
		error("%s: Got unused movie chunk header section", __func__);
	}
}

void StreamMovieActor::parseMovieHeader(Chunk &chunk) {
	_chunkCount = chunk.readTypedUint16();
	_frameRate = chunk.readTypedDouble();
	debugC(5, kDebugLoading, "[%s] %s: chunkCount: 0x%x, frameRate: %f", debugName(), __func__, _chunkCount, _frameRate);

	Common::Array<uint> chunkLengths;
	for (uint i = 0; i < _chunkCount; i++) {
		uint chunkLength = chunk.readTypedUint32();
		chunkLengths.push_back(chunkLength);
	}
}

void StreamMovieActor::parseMovieChunkMarker(Chunk &chunk) {
	// TODO: There is no warning here because that would spam with thousands of warnings.
	// This takes care of scheduling stream load and such - it doesn't actually read from the
	// chunk that is passed in. Since we don't need that scheduling since we are currently reading
	// the whole movie at once rather than streaming it from the CD-ROM, we don't currently need
	// to do much here anyway.
}

void StreamMovieActor::invalidateRect(const Common::Rect &rect) {
	invalidateLocalBounds();
}

void StreamMovieActor::decompressIntoAuxImage(MovieFrame *frame) {
	const Common::Point origin(0, 0);
	frame->keyframeImage->_image.create(frame->keyframeImage->width(), frame->keyframeImage->height(), Graphics::PixelFormat::createFormatCLUT8());
	frame->keyframeImage->_image.setTransparentColor(0);
	g_engine->getDisplayManager()->imageBlit(origin, frame->keyframeImage, 1.0, nullptr, &frame->keyframeImage->_image);
	frame->keyframeImage->setCompressionType(kUncompressedBitmap);
}

void StreamMovieActorFrames::readImageData(Chunk &chunk) {
	ImageInfo imageInfo(chunk);
	uint index = chunk.readTypedUint32();
	uint keyframeEndInMilliseconds = chunk.readTypedUint32();
	MovieFrameImage *frame = new MovieFrameImage(chunk, index, keyframeEndInMilliseconds, imageInfo);
	_images.push_back(frame);
}

void StreamMovieActorFrames::readFrameData(Chunk &chunk) {
	uint frameDataToRead = chunk.readTypedUint16();
	for (uint i = 0; i < frameDataToRead; i++) {
		MovieFrame *frame = new MovieFrame(chunk);
		if (!_parent->isLayerInSeparateZPlane(frame->layerId)) {
			frame->layerId = 0;
		}

		// We cannot use a hashmap here because multiple frames can have the
		// same index, and frames are not necessarily in index order. So we'll
		// do a linear search, which is how the original does it.
		for (MovieFrameImage *image : _images) {
			if (image->_index == frame->index) {
				frame->image = image;
				break;
			}
		}

		if (frame->keyframeIndex != 0) {
			for (MovieFrameImage *image : _images) {
				if (image->_index == frame->keyframeIndex) {
					frame->keyframeImage = image;
					break;
				}
			}
		}

		_frames.push_back(frame);
	}
}

StreamMovieProxy *StreamMovieActor::proxyOfId(uint layerId) {
	// TODO: Why can this not be a hashmap?
	for (StreamMovieProxy *proxy : _proxies) {
		if (proxy->_layerId == layerId) {
			return proxy;
		}
	}
	return nullptr;
}

StreamMovieProxy *StreamMovieActor::proxyOfScriptId(uint scriptId) {
	// TODO: Why can this not be a hashmap?
	for (StreamMovieProxy *proxy : _proxies) {
		if (proxy->_scriptId == scriptId) {
			return proxy;
		}
	}
	return nullptr;
}

bool StreamMovieActor::isLayerInSeparateZPlane(uint layerId) {
	bool proxyExistsForLayer = proxyOfId(layerId) != nullptr;
	return proxyExistsForLayer;
}

int StreamMovieActor::compareFramesByZIndex(const MovieFrame *a, const MovieFrame *b) {
	if (b->zIndex > a->zIndex) {
		return 1;
	} else if (a->zIndex > b->zIndex) {
		return -1;
	} else {
		return 0;
	}
}

} // End of namespace MediaStation
