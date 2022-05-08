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
	: _cacheBitmap(false), _reversed(false), _haveFiredAtFirstCel(false), _haveFiredAtLastCel(false)
	, _alternate(false), _playEveryFrame(false), _assetID(0), _runtime(nullptr) {
}

MovieElement::~MovieElement() {
	if (_unloadSignaller)
		_unloadSignaller->removeReceiver(this);
	if (_postRenderSignaller)
		_postRenderSignaller->removeReceiver(this);
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

VThreadState MovieElement::consumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (Event::create(EventIDs::kPlay, 0).respondsTo(msg->getEvent())) {
		StartPlayingTaskData *startPlayingTaskData = runtime->getVThread().pushTask("MovieElement::startPlayingTask", this, &MovieElement::startPlayingTask);
		startPlayingTaskData->runtime = runtime;

		ChangeFlagTaskData *becomeVisibleTaskData = runtime->getVThread().pushTask("MovieElement::changeVisibilityTask", static_cast<VisualElement *>(this), &MovieElement::changeVisibilityTask);
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
	_postRenderSignaller = project->notifyOnPostRender(this);

	if (!_paused && _visible) {
		StartPlayingTaskData *startPlayingTaskData = _runtime->getVThread().pushTask("MovieElement::startPlayingTask", this, &MovieElement::startPlayingTask);
		startPlayingTaskData->runtime = _runtime;
	}
}

void MovieElement::deactivate() {
	if (_unloadSignaller) {
		_unloadSignaller->removeReceiver(this);
		_unloadSignaller.reset();
	}
	if (_postRenderSignaller) {
		_postRenderSignaller->removeReceiver(this);
		_postRenderSignaller.reset();
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
		Common::Rect destRect(_cachedAbsoluteOrigin.x, _cachedAbsoluteOrigin.y, _cachedAbsoluteOrigin.x + _rect.getWidth(), _cachedAbsoluteOrigin.y + _rect.getHeight());
		target->blitFrom(*videoSurface, srcRect, destRect);
	}
}

void MovieElement::onPostRender(Runtime *runtime, Project *project) {
	if (_videoDecoder) {
		if (_videoDecoder->isPlaying() && _videoDecoder->endOfVideo()) {
			if (_alternate) {
				_reversed = !_reversed;
				if (!_videoDecoder->setReverse(_reversed)) {
					warning("Failed to reverse video decoder, disabling it");
					_videoDecoder.reset();
				}
			} else {
				_videoDecoder->stop();
			}

			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(_reversed ? EventIDs::kAtFirstCel : EventIDs::kAtLastCel, 0), DynamicValue(), getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
			runtime->queueMessage(dispatch);
		}
	}
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

ImageElement::ImageElement() : _cacheBitmap(false), _runtime(nullptr) {
}

ImageElement::~ImageElement() {
}

bool ImageElement::load(ElementLoaderContext &context, const Data::ImageElement &data) {
	if (!VisualElement::loadCommon(data.name, data.guid, data.rect1, data.elementFlags, data.layer, data.streamLocator, data.sectionID))
		return false;

	_cacheBitmap = ((data.elementFlags & Data::ElementFlags::kCacheBitmap) != 0);
	_runtime = context.runtime;
	_assetID = data.imageAssetID;

	return true;
}

bool ImageElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	return VisualElement::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome ImageElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	return VisualElement::writeRefAttribute(thread, writeProxy, attrib);
}

void ImageElement::activate() {
	Project *project = _runtime->getProject();
	Common::SharedPtr<Asset> asset = project->getAssetByID(_assetID).lock();

	if (!asset) {
		warning("Image element references asset %i but the asset isn't loaded!", _assetID);
		return;
	}

	if (asset->getAssetType() != kAssetTypeImage) {
		warning("Image element assigned an asset that isn't an image");
		return;
	}

	_cachedImage = static_cast<ImageAsset *>(asset.get())->loadAndCacheImage(_runtime);
}

void ImageElement::deactivate() {
	_cachedImage.reset();
}

void ImageElement::render(Window *window) {
	if (_cachedImage) {
		Common::SharedPtr<Graphics::Surface> optimized = _cachedImage->optimize(_runtime);
		Common::Rect srcRect(optimized->w, optimized->h);
		Common::Rect destRect(_cachedAbsoluteOrigin.x, _cachedAbsoluteOrigin.y, _cachedAbsoluteOrigin.x + _rect.getWidth(), _cachedAbsoluteOrigin.y + _rect.getHeight());
		window->getSurface()->blitFrom(*optimized, srcRect, destRect);
	}
}

MToonElement::MToonElement() : _cel1Based(1) {
}

MToonElement::~MToonElement() {
}

bool MToonElement::load(ElementLoaderContext &context, const Data::MToonElement &data) {
	if (!loadCommon(data.name, data.guid, data.rect1, data.elementFlags, data.layer, data.streamLocator, data.sectionID))
		return false;

	_cacheBitmap = ((data.elementFlags & Data::ElementFlags::kCacheBitmap) != 0);
	_paused = ((data.elementFlags & Data::ElementFlags::kPaused) != 0);
	_loop = ((data.animationFlags & Data::AnimationFlags::kLoop) != 0);
	_maintainRate = ((data.elementFlags & Data::AnimationFlags::kPlayEveryFrame) == 0);	// NOTE: Inverted intentionally
	_assetID = data.assetID;
	_runtime = context.runtime;
	_rateTimes10000 = data.rateTimes10000;

	return true;
}

bool MToonElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "cel") {
		result.setInt(_cel1Based);
		return true;
	}

	return VisualElement::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome MToonElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "cel") {
		// TODO proper support
		DynamicValueWriteIntegerHelper<uint32>::create(&_cel1Based, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return VisualElement::writeRefAttribute(thread, result, attrib);
}

void MToonElement::activate() {
}

void MToonElement::deactivate() {
	_renderSurface.reset();
}

void MToonElement::render(Window *window) {
	if (_renderSurface) {
		Common::Rect srcRect(_renderSurface->w, _renderSurface->h);
		Common::Rect destRect(_cachedAbsoluteOrigin.x, _cachedAbsoluteOrigin.y, _cachedAbsoluteOrigin.x + _rect.getWidth(), _cachedAbsoluteOrigin.y + _rect.getHeight());
		window->getSurface()->blitFrom(*_renderSurface, srcRect, destRect);
	}
}


TextLabelElement::TextLabelElement() : _needsRender(false), _isBitmap(false) {
}

TextLabelElement::~TextLabelElement() {
}

bool TextLabelElement::load(ElementLoaderContext &context, const Data::TextLabelElement &data) {
	if (!loadCommon(data.name, data.guid, data.rect1, data.elementFlags, data.layer, 0, data.sectionID))
		return false;

	_cacheBitmap = ((data.elementFlags & Data::ElementFlags::kCacheBitmap) != 0);
	_assetID = data.assetID;
	_runtime = context.runtime;

	return true;
}

bool TextLabelElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	return VisualElement::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome TextLabelElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	return VisualElement::writeRefAttribute(thread, writeProxy, attrib);
}

void TextLabelElement::activate() {
	Project *project = _runtime->getProject();
	Common::SharedPtr<Asset> asset = project->getAssetByID(_assetID).lock();

	if (!asset) {
		warning("Text element references asset %i but the asset isn't loaded!", _assetID);
		return;
	}

	if (asset->getAssetType() != kAssetTypeText) {
		warning("Text element assigned an asset that isn't text");
		return;
	}

	TextAsset *textAsset = static_cast<TextAsset *>(asset.get());

	if (textAsset->isBitmap()) {
		_renderedText = textAsset->getBitmapSurface();
		_needsRender = false;
	} else {
		_needsRender = true;
		_text = textAsset->getString();
		_macFormattingSpans = textAsset->getMacFormattingSpans();
	}
}

void TextLabelElement::deactivate() {
}


void TextLabelElement::render(Window *window) {
}

SoundElement::SoundElement() {
}

SoundElement::~SoundElement() {
}

bool SoundElement::load(ElementLoaderContext &context, const Data::SoundElement &data) {
	if (!NonVisualElement::loadCommon(data.name, data.guid, data.elementFlags))
		return false;

	_paused = ((data.soundFlags & Data::SoundElement::kPaused) != 0);
	_loop = ((data.soundFlags & Data::SoundElement::kLoop) != 0);
	_leftVolume = data.leftVolume;
	_rightVolume = data.rightVolume;
	_balance = data.balance;
	_assetID = data.assetID;
	_runtime = context.runtime;

	return true;
}

bool SoundElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "loop") {
		result.setBool(_loop);
		return true;
	} else if (attrib == "volume") {
		result.setInt((_leftVolume + _rightVolume) / 2);
		return true;
	}

	return NonVisualElement::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome SoundElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "loop") {
		DynamicValueWriteFuncHelper<SoundElement, &SoundElement::scriptSetLoop>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "volume") {
		DynamicValueWriteFuncHelper<SoundElement, &SoundElement::scriptSetVolume>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "balance") {
		DynamicValueWriteFuncHelper<SoundElement, &SoundElement::scriptSetBalance>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return NonVisualElement::writeRefAttribute(thread, writeProxy, attrib);
}

void SoundElement::activate() {
}

void SoundElement::deactivate() {
}

MiniscriptInstructionOutcome SoundElement::scriptSetLoop(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kBoolean)
		return kMiniscriptInstructionOutcomeFailed;

	setLoop(value.getBool());
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome SoundElement::scriptSetVolume(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	if (asInteger < 0)
		asInteger = 0;
	else if (asInteger > 100)
		asInteger = 100;

	setVolume(asInteger);
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome SoundElement::scriptSetBalance(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	if (asInteger < -100)
		asInteger = -100;
	else if (asInteger > 100)
		asInteger = 100;

	setBalance(asInteger);
	return kMiniscriptInstructionOutcomeContinue;
}

void SoundElement::setLoop(bool loop) {
	_loop = loop;
}

void SoundElement::setVolume(uint16 volume) {
	uint16 fullVolumeLeft = 100 - _balance;

	// Weird math to ensure _leftVolume + _rightVolume stays divisible by 2
	_leftVolume = (volume * fullVolumeLeft + 50) / 100;
	_rightVolume = volume * 2 - _leftVolume;
}

void SoundElement::setBalance(int16 balance) {
	_balance = balance;
	setVolume((_leftVolume + _rightVolume) / 2);
}

} // End of namespace MTropolis
