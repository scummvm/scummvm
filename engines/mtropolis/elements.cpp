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

#include "mtropolis/elements.h"
#include "mtropolis/assets.h"
#include "mtropolis/element_factory.h"
#include "mtropolis/render.h"

#include "video/video_decoder.h"
#include "video/qt_decoder.h"

#include "common/substream.h"
#include "graphics/managed_surface.h"

namespace MTropolis {

GraphicElement::GraphicElement() : _cacheBitmap(false) {
}

GraphicElement::~GraphicElement() {
}

bool GraphicElement::load(ElementLoaderContext &context, const Data::GraphicElement &data) {
	if (!loadCommon(data.name, data.guid, data.rect1, data.elementFlags, data.layer, data.streamLocator, data.sectionID))
		return false;

	_cacheBitmap = ((data.elementFlags & Data::ElementFlags::kCacheBitmap) != 0);

	return true;
}

void GraphicElement::render(Window *window) {
	// todo
}

MovieElement::MovieElement()
	: _cacheBitmap(false), _paused(false)
	, _loop(false), _alternate(false), _playEveryFrame(false), _assetID(0), _runtime(nullptr) {
}

MovieElement::~MovieElement() {
	if (_unloadSignaller)
		_unloadSignaller->removeReceiver(this);
}

bool MovieElement::load(ElementLoaderContext &context, const Data::MovieElement &data) {
	if (!loadCommon(data.name, data.guid, data.rect1, data.elementFlags, data.layer, data.streamLocator, data.sectionID))
		return false;

	_cacheBitmap = ((data.elementFlags & Data::ElementFlags::kCacheBitmap) != 0);
	_paused = ((data.elementFlags & Data::ElementFlags::kPaused) != 0);
	_loop = ((data.animationFlags & Data::AnimationFlags::kLoop) != 0);
	_alternate = ((data.animationFlags & Data::AnimationFlags::kAlternate) != 0);
	_playEveryFrame = ((data.animationFlags & Data::AnimationFlags::kPlayEveryFrame) != 0);
	_assetID = data.assetID;

	_runtime = context.runtime;

	return true;
}

bool MovieElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "paused") {
		result.setBool(_paused);
		return true;
	}

	return VisualElement::readAttribute(thread, result, attrib);
}

bool MovieElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {

	if (attrib == "paused") {
		writeProxy = DynamicValueWriteFuncHelper<MovieElement, &MovieElement::scriptSetPaused>::create(this);
		return true;
	}
	
	return VisualElement::writeRefAttribute(thread, writeProxy, attrib);
}

VThreadState MovieElement::consumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (Event::create(EventIDs::kPlay, 0).respondsTo(msg->getEvent())) {
		StartPlayingTaskData *startPlayingTaskData = runtime->getVThread().pushTask(this, &MovieElement::startPlayingTask);
		startPlayingTaskData->runtime = runtime;

		ChangeFlagTaskData *becomeVisibleTaskData = runtime->getVThread().pushTask(static_cast<VisualElement *>(this), &MovieElement::changeVisibilityTask);
		becomeVisibleTaskData->desiredFlag = true;
		becomeVisibleTaskData->runtime = runtime;
	}

	return kVThreadReturn;
}


void MovieElement::activate() {
	Project *project = _runtime->getProject();
	Common::SharedPtr<Asset> asset = project->getAssetByID(_assetID).lock();

	if (!asset) {
		warning("Movie element references asset %i but the asset isn't loaded!", _assetID);
		return;
	}

	if (asset->getAssetType() != kAssetTypeMovie) {
		warning("Movie element assigned an asset that isn't a movie");
		return;
	}

	MovieAsset *movieAsset = static_cast<MovieAsset *>(asset.get());
	size_t streamIndex = movieAsset->getStreamIndex();
	int segmentIndex = project->getSegmentForStreamIndex(streamIndex);
	project->openSegmentStream(segmentIndex);
	Common::SeekableReadStream *stream = project->getStreamForSegment(segmentIndex);

	if (!stream) {
		warning("Movie element stream could not be opened");
		return;
	}


	Video::QuickTimeDecoder *qtDecoder = new Video::QuickTimeDecoder();
	qtDecoder->setChunkBeginOffset(movieAsset->getMovieDataPos());

	_videoDecoder.reset(qtDecoder);

	Common::SafeSeekableSubReadStream *movieDataStream = new Common::SafeSeekableSubReadStream(stream, movieAsset->getMovieDataPos(), movieAsset->getMovieDataPos() + movieAsset->getMovieDataSize(), DisposeAfterUse::NO);

	if (!_videoDecoder->loadStream(movieDataStream))
		_videoDecoder.reset();

	_unloadSignaller = project->notifyOnSegmentUnload(segmentIndex, this);
}

void MovieElement::deactivate() {
	if (_unloadSignaller) {
		_unloadSignaller->removeReceiver(this);
		_unloadSignaller.reset();
	}

	_videoDecoder.reset();
}

void MovieElement::render(Window *window) {
	const Graphics::Surface *videoSurface = nullptr;
	while (_videoDecoder->needsUpdate()) {
		videoSurface = _videoDecoder->decodeNextFrame();
		if (_playEveryFrame)
			break;
	}

	if (videoSurface) {
		Graphics::ManagedSurface *target = window->getSurface().get();
		Common::Rect srcRect(0, 0, videoSurface->w, videoSurface->h);
		Common::Rect destRect(_rect.left, _rect.top, _rect.right, _rect.bottom);
		target->blitFrom(*videoSurface, srcRect, destRect);
	}
}

bool MovieElement::scriptSetPaused(const DynamicValue& dest) {
	if (dest.getType() == DynamicValueTypes::kBoolean) {
		_paused = dest.getBool();
		return true;
	}
	return false;
}

void MovieElement::onSegmentUnloaded(int segmentIndex) {
	_videoDecoder.reset();
}

VThreadState MovieElement::startPlayingTask(const StartPlayingTaskData &taskData) {
	if (_videoDecoder && !_videoDecoder->isPlaying()) {
		EventIDs::EventID eventToSend = EventIDs::kPlay;
		if (_paused) {
			_paused = false;
			eventToSend = EventIDs::kUnpause;
		}

		_videoDecoder->start();

		Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(eventToSend, 0), DynamicValue(), getSelfReference()));
		Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
		taskData.runtime->sendMessageOnVThread(dispatch);
	}

	return kVThreadReturn;
}

} // End of namespace MTropolis
