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


// Audio player, this does not support requeueing.  If the sound exhausts, then you must create a
// new audio player.  In particular, since time is being tracked separately, if the loop status
// changes when the timer thinks the sound should still be playing, but the sound has actually
// exhausted, then the sound needs to be requeued.
class AudioPlayer : public Audio::AudioStream {
public:
	AudioPlayer(Audio::Mixer *mixer, byte volume, int8 balance, const Common::SharedPtr<AudioMetadata> &metadata, const Common::SharedPtr<CachedAudio> &audio, bool isLooping, size_t currentPos, size_t startPos, size_t endPos);
	~AudioPlayer();
	
	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override;
	int getRate() const override;
	bool endOfData() const override;

	void sendToMixer(Audio::Mixer *mixer, byte volume, int8 balance);
	void stop();

private:
	Common::Mutex _mutex;

	Common::SharedPtr<AudioMetadata> _metadata;
	Common::SharedPtr<CachedAudio> _audio;
	Audio::SoundHandle _handle;
	bool _isLooping;
	bool _exhausted;
	size_t _currentPos;
	size_t _startPos;
	size_t _endPos;
	Audio::Mixer *_mixer;
};

AudioPlayer::AudioPlayer(Audio::Mixer *mixer, byte volume, int8 balance, const Common::SharedPtr<AudioMetadata> &metadata, const Common::SharedPtr<CachedAudio> &audio, bool isLooping, size_t currentPos, size_t startPos, size_t endPos)
	: _metadata(metadata), _audio(audio), _isLooping(isLooping), _currentPos(currentPos), _startPos(startPos), _endPos(endPos), _exhausted(false), _mixer(nullptr) {
	if (_startPos >= _endPos) {
		// ???
		_exhausted = true;
		_isLooping = false;
	}
	if (_currentPos < _startPos)
		_currentPos = _startPos;

	if (!_exhausted) {
		_mixer = mixer;
		mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, this, -1, volume, balance, DisposeAfterUse::NO);
	}
}

AudioPlayer::~AudioPlayer() {
	stop();
}

int AudioPlayer::readBuffer(int16 *buffer, const int numSamplesTimesChannelCount) {
	Common::StackLock lock(_mutex);

	int samplesRead = 0;
	if (_exhausted)
		return 0;

	uint8 numChannels = _metadata->channels;

	size_t numSamples = numSamplesTimesChannelCount / numChannels;

	while (numSamples > 0) {
		size_t samplesAvailable = _endPos - _currentPos;
		if (samplesAvailable == 0) {
			if (_isLooping) {
				_currentPos = _startPos;
				continue;
			} else {
				_exhausted = true;
				break;
			}
		}

		size_t numSamplesThisIteration = numSamples;
		if (numSamplesThisIteration > samplesAvailable)
			numSamplesThisIteration = samplesAvailable;

		size_t numSampleValues = numSamplesThisIteration * numChannels;
		// TODO: Support more formats
		if (_metadata->bitsPerSample == 8 && _metadata->encoding == AudioMetadata::kEncodingUncompressed) {
			const uint8 *inSamples = static_cast<const uint8 *>(_audio->getData()) + _currentPos * numChannels;
			for (int i = 0; i < numSampleValues; i++)
				buffer[i] = (inSamples[i] - 0x80) * 256;
		} else if (_metadata->bitsPerSample == 16 && _metadata->encoding == AudioMetadata::kEncodingUncompressed) {
			const int16 *inSamples = static_cast<const int16 *>(_audio->getData()) + _currentPos * numChannels;
			memcpy(buffer, inSamples, sizeof(int16) * numSampleValues);
		}

		buffer += numSampleValues;
		numSamples -= numSamplesThisIteration;

		samplesRead += numSamplesThisIteration * numChannels;
		_currentPos += numSamplesThisIteration;
	}

	return samplesRead;
}

bool AudioPlayer::isStereo() const {
	return _metadata->channels == 2;
}

int AudioPlayer::getRate() const {
	return _metadata->sampleRate;
}

bool AudioPlayer::endOfData() const {
	return _exhausted;
}

void AudioPlayer::sendToMixer(Audio::Mixer *mixer, byte volume, int8 balance) {
	mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, this, -1, volume, balance, DisposeAfterUse::NO);
}

void AudioPlayer::stop() {
	if (_mixer)
		_mixer->stopHandle(_handle);

	_exhausted = true;
	_mixer = nullptr;
}

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
	, _alternate(false), _playEveryFrame(false), _assetID(0), _runtime(nullptr), _displayFrame(nullptr)
	, _shouldPlayIfNotPaused(true), _needsReset(true), _currentPlayState(kMediaStateStopped) {
}

MovieElement::~MovieElement() {
	if (_unloadSignaller)
		_unloadSignaller->removeReceiver(this);
	if (_playMediaSignaller)
		_playMediaSignaller->removeReceiver(this);
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

		return kVThreadReturn;
	}

	return Structural::consumeCommand(runtime, msg);
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
	_playMediaSignaller = project->notifyOnPlayMedia(this);

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
	if (_playMediaSignaller) {
		_playMediaSignaller->removeReceiver(this);
		_playMediaSignaller.reset();
	}

	_videoDecoder.reset();
}

void MovieElement::render(Window *window) {
	int framesDecodedThisFrame = 0;

	if (_needsReset) {
		// TODO: Seek elsewhere
		_videoDecoder->seekToFrame(0);
		const Graphics::Surface *decodedFrame = _videoDecoder->decodeNextFrame();
		if (decodedFrame) {
			_displayFrame = decodedFrame;
			framesDecodedThisFrame++;
		}

		_needsReset = false;
	}

	while (_videoDecoder->needsUpdate()) {
		if (_playEveryFrame && framesDecodedThisFrame > 0)
			break;

		const Graphics::Surface *decodedFrame = _videoDecoder->decodeNextFrame();

		// GNARLY HACK: QuickTimeDecoder doesn't return true for endOfVideo or false for needsUpdate until it
		// tries decoding past the end, so we're assuming that the decoded frame memory stays valid until we
		// actually have a new frame and continuing to use it.
		if (decodedFrame) {
			framesDecodedThisFrame++;
			_displayFrame = decodedFrame;
			if (_playEveryFrame)
				break;
		}
	}

	if (framesDecodedThisFrame > 1)
		debug(1, "Perf warning: %i video frames decoded in one frame", framesDecodedThisFrame);

	if (_displayFrame) {
		Graphics::ManagedSurface *target = window->getSurface().get();
		Common::Rect srcRect(0, 0, _displayFrame->w, _displayFrame->h);
		Common::Rect destRect(_cachedAbsoluteOrigin.x, _cachedAbsoluteOrigin.y, _cachedAbsoluteOrigin.x + _rect.getWidth(), _cachedAbsoluteOrigin.y + _rect.getHeight());
		target->blitFrom(*_displayFrame, srcRect, destRect);
	}
}

void MovieElement::playMedia(Runtime *runtime, Project *project) {
	if (_videoDecoder) {
		if (_shouldPlayIfNotPaused) {
			if (_paused) {
				// Goal state is paused
				if (_videoDecoder->isPlaying()) {
					_videoDecoder->pauseVideo(true);
					_currentPlayState = kMediaStatePaused;
				}
			} else {
				// Goal state is playing
				if (_videoDecoder->isPaused())
					_videoDecoder->pauseVideo(false);
				if (!_videoDecoder->isPlaying())
					_videoDecoder->start();

				_currentPlayState = kMediaStatePlaying;
			}
		} else {
			// Goal state is stopped
			if (_videoDecoder->isPlaying())
				_videoDecoder->stop();

			_currentPlayState = kMediaStateStopped;
		}

		if (_currentPlayState == kMediaStatePlaying && _videoDecoder->endOfVideo()) {
			if (_alternate) {
				_reversed = !_reversed;
				if (!_videoDecoder->setReverse(_reversed)) {
					warning("Failed to reverse video decoder, disabling it");
					_videoDecoder.reset();
				}
			} else {
				_videoDecoder->stop();
				_currentPlayState = kMediaStateStopped;
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
	if (_videoDecoder) {
		// TODO: Frame ranges
		_needsReset = true;
		_shouldPlayIfNotPaused = true;
		_paused = false;

		Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(EventIDs::kPlay, 0), DynamicValue(), getSelfReference()));
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
	if (attrib == "text") {
		// Obsidian accesses this on an image element in the menus, and if it fails, the "save first" warning
		// prompt buttons aren't layered correctly?
		result.setString(_text);
		return true;
	}

	return VisualElement::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome ImageElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "text") {
		DynamicValueWriteStringHelper::create(&_text, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

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

MToonElement::MToonElement() : _cel1Based(1), _renderedFrame(0), _flushPriority(0) {
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
	} else if (attrib == "flushpriority") {
		result.setInt(_flushPriority);
		return true;
	}

	return VisualElement::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome MToonElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "cel") {
		// TODO proper support
		DynamicValueWriteIntegerHelper<uint32>::create(&_cel1Based, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "flushpriority") {
		DynamicValueWriteIntegerHelper<int32>::create(&_flushPriority, result);
		return kMiniscriptInstructionOutcomeContinue;

	}

	return VisualElement::writeRefAttribute(thread, result, attrib);
}

void MToonElement::activate() {
	Project *project = _runtime->getProject();
	Common::SharedPtr<Asset> asset = project->getAssetByID(_assetID).lock();

	if (!asset) {
		warning("mToon element references asset %i but the asset isn't loaded!", _assetID);
		return;
	}

	if (asset->getAssetType() != kAssetTypeMToon) {
		warning("mToon element assigned an asset that isn't an mToon");
		return;
	}

	_cachedMToon = static_cast<MToonAsset *>(asset.get())->loadAndCacheMToon(_runtime);
	_metadata = _cachedMToon->getMetadata();
}

void MToonElement::deactivate() {
	_renderSurface.reset();
}

void MToonElement::render(Window *window) {
	if (_cachedMToon) {
		_cachedMToon->optimize(_runtime);
		uint32 frame = 0;

		if (_cel1Based < 1)
			frame = 0;
		else if (_cel1Based > _metadata->frames.size())
			frame = _metadata->frames.size() - 1;
		else
			frame = _cel1Based - 1;

		_cachedMToon->getOrRenderFrame(_renderedFrame, frame, _renderSurface);
		_renderedFrame = frame;
	}

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

SoundElement::SoundElement() : _finishTime(0), _shouldPlayIfNotPaused(true), _needsReset(true) {
}

SoundElement::~SoundElement() {
	if (_playMediaSignaller)
		_playMediaSignaller->removeReceiver(this);
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

VThreadState SoundElement::consumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (Event::create(EventIDs::kPlay, 0).respondsTo(msg->getEvent())) {
		StartPlayingTaskData *startPlayingTaskData = runtime->getVThread().pushTask("SoundElement::startPlayingTask", this, &SoundElement::startPlayingTask);
		startPlayingTaskData->runtime = runtime;

		return kVThreadReturn;
	}

	return Structural::consumeCommand(runtime, msg);
}

void SoundElement::activate() {
	Project *project = _runtime->getProject();
	Common::SharedPtr<Asset> asset = project->getAssetByID(_assetID).lock();

	if (!asset) {
		warning("Sound element references asset %i but the asset isn't loaded!", _assetID);
		return;
	}

	if (asset->getAssetType() != kAssetTypeAudio) {
		warning("Sound element assigned an asset that isn't audio");
		return;
	}

	_cachedAudio = static_cast<AudioAsset *>(asset.get())->loadAndCacheAudio(_runtime);
	_metadata = static_cast<AudioAsset *>(asset.get())->getMetadata();

	_playMediaSignaller = project->notifyOnPlayMedia(this);

	if (!_paused) {
		StartPlayingTaskData *startPlayingTaskData = _runtime->getVThread().pushTask("SoundElement::startPlayingTask", this, &SoundElement::startPlayingTask);
		startPlayingTaskData->runtime = _runtime;
	}
}


void SoundElement::deactivate() {
	if (_playMediaSignaller) {
		_playMediaSignaller->removeReceiver(this);
		_playMediaSignaller.reset();
	}

	_metadata.reset();
	_cachedAudio.reset();
	_player.reset();
}

void SoundElement::playMedia(Runtime *runtime, Project *project) {
	if (_shouldPlayIfNotPaused) {
		if (_paused) {
			// Goal state is paused
			// TODO: Track pause time
			_player.reset();
		} else {
			// Goal state is playing
			if (_needsReset) {
				// TODO: Reset to start time
				_player.reset();
				_needsReset = false;
			}

			if (!_player) {
				_finishTime = _runtime->getPlayTime() + _metadata->durationMSec;

				_player.reset();

				int normalizedVolume = (_leftVolume + _rightVolume) * 255 / 2;
				int normalizedBalance = _balance * 127 / 100;

				// TODO: Support ranges
				size_t numSamples = _cachedAudio->getNumSamples(*_metadata);
				_player.reset(new AudioPlayer(_runtime->getAudioMixer(), normalizedVolume, normalizedBalance, _metadata, _cachedAudio, _loop, 0, 0, numSamples));
			}

			// TODO: Check cue points and queue them here
			
			if (!_loop && _runtime->getPlayTime() >= _finishTime) {
				// Don't throw out the handle - It can still be playing but we just treat it like it's not.
				// If it has anything left, then we let it finish and avoid clipping the sound, but we need
				// to know that the handle is still here so we can actually stop it if the element is
				// destroyed, since the stream is tied to the CachedAudio.

				Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(EventIDs::kStop, 0), DynamicValue(), getSelfReference()));
				Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
				runtime->queueMessage(dispatch);

				_shouldPlayIfNotPaused = false;
			}
		}
	} else {
		// Goal state is stopped
		_player.reset();
	}
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

VThreadState SoundElement::startPlayingTask(const StartPlayingTaskData &taskData) {
	_paused = false;
	_shouldPlayIfNotPaused = true;
	_needsReset = true;

	Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(EventIDs::kPlay, 0), DynamicValue(), getSelfReference()));
	Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
	taskData.runtime->sendMessageOnVThread(dispatch);

	return kVThreadReturn;
}

} // End of namespace MTropolis
