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
#include "mtropolis/miniscript.h"
#include "mtropolis/render.h"

#include "video/video_decoder.h"
#include "video/qt_decoder.h"

#include "common/substream.h"

#include "graphics/macgui/macfontmanager.h"
#include "graphics/fontman.h"
#include "graphics/font.h"
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
	_volume = data.volume;

	_runtime = context.runtime;

	return true;
}

bool MovieElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "range") {
		result.setIntRange(_playRange);
		return true;
	}
	if (attrib == "timevalue") {
		result.setInt(_currentTimestamp);
		return true;
	}

	return VisualElement::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome MovieElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "range") {
		DynamicValueWriteOrRefAttribFuncHelper<MovieElement, &MovieElement::scriptSetRange, &MovieElement::scriptRangeWriteRefAttribute>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "volume") {
		DynamicValueWriteFuncHelper<MovieElement, &MovieElement::scriptSetVolume>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "timevalue") {
		DynamicValueWriteFuncHelper<MovieElement, &MovieElement::scriptSetTimestamp>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return VisualElement::writeRefAttribute(thread, result, attrib);
}

VThreadState MovieElement::consumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (Event::create(EventIDs::kPlay, 0).respondsTo(msg->getEvent())) {
		// These reverse order
		{
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(EventIDs::kPlay, 0), DynamicValue(), getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
			runtime->sendMessageOnVThread(dispatch);
		}

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
	qtDecoder->setVolume(_volume * 255 / 100);

	_videoDecoder.reset(qtDecoder);

	Common::SafeSeekableSubReadStream *movieDataStream = new Common::SafeSeekableSubReadStream(stream, movieAsset->getMovieDataPos(), movieAsset->getMovieDataPos() + movieAsset->getMovieDataSize(), DisposeAfterUse::NO);

	if (!_videoDecoder->loadStream(movieDataStream))
		_videoDecoder.reset();

	_timeScale = qtDecoder->getTimeScale();

	_unloadSignaller = project->notifyOnSegmentUnload(segmentIndex, this);
	_playMediaSignaller = project->notifyOnPlayMedia(this);

	_maxTimestamp = qtDecoder->getDuration().convertToFramerate(qtDecoder->getTimeScale()).totalNumberOfFrames();
	_playRange = IntRange::create(0, _maxTimestamp);
	_currentTimestamp = 0;

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
	if (_needsReset) {
		_videoDecoder->setReverse(_reversed);
		_videoDecoder->seek(Audio::Timestamp(0, _timeScale).addFrames(_currentTimestamp));
		_videoDecoder->setEndTime(Audio::Timestamp(0, _timeScale).addFrames(_reversed ? _playRange.min : _playRange.max));
		const Graphics::Surface *decodedFrame = _videoDecoder->decodeNextFrame();
		if (decodedFrame)
			_displayFrame = decodedFrame;

		_needsReset = false;
	}

	if (_displayFrame) {
		Graphics::ManagedSurface *target = window->getSurface().get();
		Common::Rect srcRect(0, 0, _displayFrame->w, _displayFrame->h);
		Common::Rect destRect(_cachedAbsoluteOrigin.x, _cachedAbsoluteOrigin.y, _cachedAbsoluteOrigin.x + _rect.getWidth(), _cachedAbsoluteOrigin.y + _rect.getHeight());
		target->blitFrom(*_displayFrame, srcRect, destRect);
	}
}

void MovieElement::playMedia(Runtime *runtime, Project *project) {
	// If this isn't visible, then it wasn't rendered
	if (!_visible)
		return;

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
				if (!_videoDecoder->isPlaying())
					_videoDecoder->start();
				if (_videoDecoder->isPaused())
					_videoDecoder->pauseVideo(false);

				_currentPlayState = kMediaStatePlaying;
			}
		} else {
			// Goal state is stopped
			if (_videoDecoder->isPlaying())
				_videoDecoder->stop();

			_currentPlayState = kMediaStateStopped;
		}

		uint32 minTS = _playRange.min;
		uint32 maxTS = _playRange.max;
		uint32 targetTS = _currentTimestamp;

		int framesDecodedThisFrame = 0;
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

		if (_currentPlayState == kMediaStatePlaying) {
			if (_videoDecoder->endOfVideo())
				targetTS = _reversed ? _playRange.min : _playRange.max;
			else
				targetTS = (_videoDecoder->getTime() * _timeScale + 500) / 1000;
		}

		if (framesDecodedThisFrame > 1)
			debug(1, "Perf warning: %i video frames decoded in one frame", framesDecodedThisFrame);

		if (targetTS < _playRange.min)
			targetTS = _playRange.min;
		if (targetTS > _playRange.max)
			targetTS = _playRange.max;

		// Sync TS to the end of video if we hit the end

		if (targetTS != _currentTimestamp) {
			assert(!_paused);

			_currentTimestamp = targetTS;

			if (_currentTimestamp == maxTS) {
				Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(EventIDs::kAtLastCel, 0), DynamicValue(), getSelfReference()));
				Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
				runtime->queueMessage(dispatch);
			}

			if (_currentTimestamp == minTS) {
				Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(EventIDs::kAtFirstCel, 0), DynamicValue(), getSelfReference()));
				Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
				runtime->queueMessage(dispatch);
			}
		}

		if (_currentPlayState == kMediaStatePlaying && _videoDecoder->endOfVideo()) {
			if (_alternate) {
				_reversed = !_reversed;
				if (!_videoDecoder->setReverse(_reversed)) {
					warning("Failed to reverse video decoder, disabling it");
					_videoDecoder.reset();
				}

				uint32 endTS = _reversed ? _playRange.min : _playRange.max;
				_videoDecoder->setEndTime(Audio::Timestamp(0, _timeScale).addFrames(endTS));
			} else {
				// It doesn't look like movies fire any events upon reaching the end, just At Last Cel and At First Cel
				_videoDecoder->stop();
				_currentPlayState = kMediaStateStopped;
			}
		}
	}
}

void MovieElement::onSegmentUnloaded(int segmentIndex) {
	_videoDecoder.reset();
}

MiniscriptInstructionOutcome MovieElement::scriptSetRange(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kIntegerRange) {
		thread->error("Wrong type for movie element range");
		return kMiniscriptInstructionOutcomeFailed;
	}

	return scriptSetRangeTyped(thread, value.getIntRange());
}

MiniscriptInstructionOutcome MovieElement::scriptSetVolume(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger)) {
		thread->error("Wrong type for movie element range");
		return kMiniscriptInstructionOutcomeFailed;
	}

	if (asInteger < 0)
		asInteger = 0;
	else if (asInteger > 100)
		asInteger = 100;

	_volume = asInteger;
	if (_videoDecoder)
		_videoDecoder->setVolume(_volume * 255 / 100);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome MovieElement::scriptSetTimestamp(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger)) {
		thread->error("Wrong type for movie element timevalue");
		return kMiniscriptInstructionOutcomeFailed;
	}

	if (asInteger < _playRange.min)
		asInteger = _playRange.min;
	else if (asInteger > _playRange.max)
		asInteger = _playRange.max;

	if (asInteger != _currentTimestamp) {
		SeekToTimeTaskData *taskData = thread->getRuntime()->getVThread().pushTask("MovieElement::seekToTimeTask", this, &MovieElement::seekToTimeTask);
		taskData->runtime = _runtime;
		taskData->timestamp = asInteger;

		return kMiniscriptInstructionOutcomeYieldToVThreadNoRetry;
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome MovieElement::scriptSetRangeStart(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger)) {
		thread->error("Couldn't set movie element range start");
		return kMiniscriptInstructionOutcomeFailed;
	}

	return scriptSetRangeTyped(thread, IntRange::create(asInteger, _playRange.max));
}

MiniscriptInstructionOutcome MovieElement::scriptSetRangeEnd(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger)) {
		thread->error("Couldn't set movie element range end");
		return kMiniscriptInstructionOutcomeFailed;
	}

	return scriptSetRangeTyped(thread, IntRange::create(_playRange.min, asInteger));
}

MiniscriptInstructionOutcome MovieElement::scriptRangeWriteRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "start") {
		DynamicValueWriteFuncHelper<MovieElement, &MovieElement::scriptSetRangeStart>::create(this, result);
		return kMiniscriptInstructionOutcomeFailed;
	}
	if (attrib == "end") {
		DynamicValueWriteFuncHelper<MovieElement, &MovieElement::scriptSetRangeStart>::create(this, result);
		return kMiniscriptInstructionOutcomeFailed;
	}

	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome MovieElement::scriptSetRangeTyped(MiniscriptThread *thread, const IntRange &range) {
	_playRange = range;

	if (_playRange.min < 0)
		_playRange.min = 0;
	else if (_playRange.min > _maxTimestamp)
		_playRange.min = _maxTimestamp;

	if (_playRange.max > _maxTimestamp)
		_playRange.max = _maxTimestamp;

	if (_playRange.max < _playRange.min)
		_playRange.max = _playRange.min;

	uint32 minTS = _playRange.min;
	uint32 maxTS = _playRange.max;
	uint32 targetTS = _currentTimestamp;

	if (targetTS < minTS)
		targetTS = minTS;
	if (targetTS > maxTS)
		targetTS = maxTS;

	if (targetTS != _currentTimestamp) {
		SeekToTimeTaskData *taskData = thread->getRuntime()->getVThread().pushTask("MovieElement::seekToTimeTask", this, &MovieElement::seekToTimeTask);
		taskData->runtime = _runtime;
		taskData->timestamp = targetTS;

		return kMiniscriptInstructionOutcomeYieldToVThreadNoRetry;
	}

	return kMiniscriptInstructionOutcomeContinue;
}

VThreadState MovieElement::startPlayingTask(const StartPlayingTaskData &taskData) {
	if (_videoDecoder) {
		_videoDecoder->stop();
		_currentPlayState = kMediaStateStopped;
		_needsReset = true;

		_shouldPlayIfNotPaused = true;
		_paused = false;
	}

	return kVThreadReturn;
}

VThreadState MovieElement::seekToTimeTask(const SeekToTimeTaskData &taskData) {
	uint32 minTS = _playRange.min;
	uint32 maxTS = _playRange.max;

	uint32 targetTS = taskData.timestamp;

	if (targetTS < minTS)
		targetTS = minTS;
	if (targetTS > maxTS)
		targetTS = maxTS;

	if (targetTS == _currentTimestamp)
		return kVThreadReturn;

	_currentTimestamp = targetTS;
	if (_videoDecoder) {
		_videoDecoder->stop();
		_currentPlayState = kMediaStateStopped;
	}
	_needsReset = true;

	if (_currentTimestamp == minTS) {
		Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(EventIDs::kAtFirstCel, 0), DynamicValue(), getSelfReference()));
		Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
		taskData.runtime->sendMessageOnVThread(dispatch);
	}

	if (_currentTimestamp == maxTS) {
		Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(EventIDs::kAtLastCel, 0), DynamicValue(), getSelfReference()));
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

MToonElement::MToonElement() : _frame(0), _renderedFrame(0), _flushPriority(0), _celStartTimeMSec(0), _isPlaying(false), _playRange(IntRange::create(1, 1)) {
}

MToonElement::~MToonElement() {
	if (_playMediaSignaller)
		_playMediaSignaller->removeReceiver(this);
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
	_rateTimes100000 = data.rateTimes100000;

	return true;
}

bool MToonElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "cel") {
		result.setInt(_frame + 1);
		return true;
	} else if (attrib == "flushpriority") {
		result.setInt(_flushPriority);
		return true;
	} else if (attrib == "rate") {
		result.setFloat(_rateTimes100000 / 100000.0);
		return true;
	} else if (attrib == "range") {
		result.setIntRange(_playRange);
		return true;
	}

	return VisualElement::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome MToonElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "cel") {
		// TODO proper support
		DynamicValueWriteFuncHelper<MToonElement, &MToonElement::scriptSetCel>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "flushpriority") {
		DynamicValueWriteIntegerHelper<int32>::create(&_flushPriority, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "maintainrate") {
		DynamicValueWriteBoolHelper::create(&_maintainRate, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "rate") {
		DynamicValueWriteFuncHelper<MToonElement, &MToonElement::scriptSetRate>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "range") {
		DynamicValueWriteFuncHelper<MToonElement, &MToonElement::scriptSetRange>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return VisualElement::writeRefAttribute(thread, result, attrib);
}

VThreadState MToonElement::consumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties>& msg) {
	if (Event::create(EventIDs::kPlay, 0).respondsTo(msg->getEvent())) {
		StartPlayingTaskData *startPlayingTaskData = runtime->getVThread().pushTask("MToonElement::startPlayingTask", this, &MToonElement::startPlayingTask);
		startPlayingTaskData->runtime = runtime;

		ChangeFlagTaskData *becomeVisibleTaskData = runtime->getVThread().pushTask("MToonElement::changeVisibilityTask", static_cast<VisualElement *>(this), &MToonElement::changeVisibilityTask);
		becomeVisibleTaskData->desiredFlag = true;
		becomeVisibleTaskData->runtime = runtime;

		return kVThreadReturn;
	}
	if (Event::create(EventIDs::kStop, 0).respondsTo(msg->getEvent())) {
		// Works differently from movies: Needs to hide the element and pause
		warning("mToon element stops are not implemented");
		return kVThreadReturn;
	}

	return kVThreadReturn;
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

	_playMediaSignaller = project->notifyOnPlayMedia(this);
	_playRange = IntRange::create(1, _metadata->frames.size());
}

void MToonElement::deactivate() {
	if (_playMediaSignaller) {
		_playMediaSignaller->removeReceiver(this);
		_playMediaSignaller.reset();
	}

	_renderSurface.reset();
}

void MToonElement::render(Window *window) {
	if (_cachedMToon) {
		_cachedMToon->optimize(_runtime);

		_cachedMToon->getOrRenderFrame(_renderedFrame, _frame, _renderSurface);
		_renderedFrame = _frame;

		Rect16 frameRect = _metadata->frames[_frame].rect;

		if (_renderSurface) {
			Common::Rect srcRect;
			Common::Rect destRect;

			if (frameRect.getWidth() == _renderSurface->w && frameRect.getHeight() == _renderSurface->h) {
				// Frame rect is the size of the render surface, meaning the frame rect is an offset
				srcRect = Common::Rect(0, 0, frameRect.getWidth(), frameRect.getHeight());
			} else {
				// Frame rect is a sub-area of the rendered rect
				srcRect = Common::Rect(frameRect.left, frameRect.top, frameRect.right, frameRect.bottom);
			}
			destRect = Common::Rect(_cachedAbsoluteOrigin.x + frameRect.left, _cachedAbsoluteOrigin.y + frameRect.top, _cachedAbsoluteOrigin.x + frameRect.right, _cachedAbsoluteOrigin.y + frameRect.bottom);

			window->getSurface()->blitFrom(*_renderSurface, srcRect, destRect);
		}
	}
}

VThreadState MToonElement::startPlayingTask(const StartPlayingTaskData &taskData) {
	_frame = _playRange.min;
	_paused = false;
	_isPlaying = false;	// Reset play state, it starts for real in playMedia

	Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(EventIDs::kPlay, 0), DynamicValue(), getSelfReference()));
	Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
	taskData.runtime->sendMessageOnVThread(dispatch);

	return kVThreadReturn;
}

VThreadState MToonElement::changeFrameTask(const ChangeFrameTaskData &taskData) {
	if (taskData.frame == _frame)
		return kVThreadReturn;

	uint32 minFrame = _playRange.min;
	uint32 maxFrame = _playRange.max;
	_frame = taskData.frame;

	if (_frame == minFrame) {
		Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(EventIDs::kAtFirstCel, 0), DynamicValue(), getSelfReference()));
		Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
		taskData.runtime->sendMessageOnVThread(dispatch);
	}

	if (_frame == maxFrame) {
		Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(EventIDs::kAtLastCel, 0), DynamicValue(), getSelfReference()));
		Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
		taskData.runtime->sendMessageOnVThread(dispatch);
	}

	return kVThreadReturn;
}

void MToonElement::playMedia(Runtime *runtime, Project *project) {
	uint32 targetFrame = _frame;

	if (_paused)
		return;

	uint32 minFrame = _playRange.min - 1;
	uint32 maxFrame = _playRange.max - 1;

	uint64 playTime = runtime->getPlayTime();
	if (!_isPlaying) {
		_isPlaying = true;
		_celStartTimeMSec = runtime->getPlayTime();
	}

	const bool isReversed = (_rateTimes100000 < 0);
	uint32 absRateTimes100000;
	if (isReversed)
		absRateTimes100000 = -_rateTimes100000;
	else
		absRateTimes100000 = _rateTimes100000;

	// Might be possible due to drift?
	if (playTime < _celStartTimeMSec)
		return;

	uint64 timeSinceCelStart = playTime - _celStartTimeMSec;
	uint64 framesAdvanced = timeSinceCelStart * static_cast<uint64>(absRateTimes100000) / static_cast<uint64>(100000000);

	if (framesAdvanced > 0) {
		// This needs to be handled correctly: Reaching the last frame triggers At Last Cel or At First Cel,
		// but going PAST the end frame triggers automatic stop and pause. The Obsidian bureau filing cabinets
		// depend on this, since they reset the cel when reaching the last cel but do not unpause.
		bool ranPastEnd = false;

		size_t framesRemainingToOnePastEnd = isReversed ? (_frame - minFrame + 1) : (maxFrame + 1 - _frame);
		if (framesRemainingToOnePastEnd <= framesAdvanced) {
			ranPastEnd = true;
			if (_loop)
				targetFrame = isReversed ? maxFrame : minFrame;
			else
				targetFrame = isReversed ? minFrame : maxFrame;
		} else
			targetFrame = isReversed ? (_frame - framesAdvanced) : (_frame + framesAdvanced);

		if (_frame != targetFrame) {
			_frame = targetFrame;

			if (_frame == maxFrame) {
				Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(EventIDs::kAtLastCel, 0), DynamicValue(), getSelfReference()));
				Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
				runtime->queueMessage(dispatch);
			}

			if (_frame == minFrame) {
				Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(EventIDs::kAtFirstCel, 0), DynamicValue(), getSelfReference()));
				Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
				runtime->queueMessage(dispatch);
			}
		}

		if (ranPastEnd && !_loop) {
			_paused = true;

			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(EventIDs::kPause, 0), DynamicValue(), getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
			runtime->queueMessage(dispatch);
		}

		if (_maintainRate)
			_celStartTimeMSec = playTime;
		else
			_celStartTimeMSec += (static_cast<uint64>(100000000) * framesAdvanced) / absRateTimes100000;
	}
}

MiniscriptInstructionOutcome MToonElement::scriptSetCel(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger)) {
		thread->error("Attempted to set mToon cel to an invalid value");
		return kMiniscriptInstructionOutcomeFailed;
	}

	if (asInteger < _playRange.min)
		asInteger = _playRange.min;
	else if (asInteger > _playRange.max)
		asInteger = _playRange.max;

	uint32 frame = asInteger - 1;
	_celStartTimeMSec = thread->getRuntime()->getPlayTime();

	if (frame != _frame) {
		ChangeFrameTaskData *taskData = thread->getRuntime()->getVThread().pushTask("MToonElement::changeFrameTask", this, &MToonElement::changeFrameTask);
		taskData->runtime = _runtime;
		taskData->frame = frame;

		return kMiniscriptInstructionOutcomeYieldToVThreadNoRetry;
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome MToonElement::scriptSetRange(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kIntegerRange) {
		thread->error("Invalid type for mToon range");
		return kMiniscriptInstructionOutcomeFailed;
	}

	IntRange intRange = value.getIntRange();
	size_t numFrames = _metadata->frames.size();
	if (intRange.min < 1)
		intRange.min = 1;
	else if (intRange.min > numFrames)
		intRange.min = numFrames;

	if (intRange.max > numFrames)
		intRange.max = numFrames;

	if (intRange.max < intRange.min)
		intRange.min = intRange.max;

	_playRange = intRange;

	uint32 targetFrame = _frame;
	uint32 minFrame = intRange.min - 1;
	uint32 maxFrame = intRange.max - 1;
	if (targetFrame < minFrame)
		targetFrame = minFrame;
	else if (targetFrame > maxFrame)
		targetFrame = maxFrame;

	if (targetFrame != _frame) {
		ChangeFrameTaskData *taskData = thread->getRuntime()->getVThread().pushTask("MToonElement::changeFrameTask", this, &MToonElement::changeFrameTask);
		taskData->frame = targetFrame;
		taskData->runtime = _runtime;
		return kMiniscriptInstructionOutcomeYieldToVThreadNoRetry;
	}

	return kMiniscriptInstructionOutcomeContinue;
}

void MToonElement::onPauseStateChanged() {
	_celStartTimeMSec = _runtime->getPlayTime();
}

MiniscriptInstructionOutcome MToonElement::scriptSetRate(MiniscriptThread *thread, const DynamicValue &value) {
	switch (value.getType()) {
	case DynamicValueTypes::kFloat:
		_rateTimes100000 = static_cast<int32>(round(value.getFloat()) * 100000.0);
		break;
	case DynamicValueTypes::kInteger:
		_rateTimes100000 = value.getInt() * 100000;
		break;
	default:
		thread->error("Invalid type for Miniscript rate");
		return kMiniscriptInstructionOutcomeFailed;
	}
	_celStartTimeMSec = thread->getRuntime()->getPlayTime();

	return kMiniscriptInstructionOutcomeContinue;
}


TextLabelElement::TextLabelElement() : _needsRender(false), _isBitmap(false), _macFontID(0), _size(12), _alignment(kTextAlignmentLeft) {
}

TextLabelElement::~TextLabelElement() {
}

bool TextLabelElement::isTextLabel() const {
	return true;
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
	if (attrib == "text") {
		result.setString(_text);
		return true;
	}

	return VisualElement::readAttribute(thread, result, attrib);
}

bool TextLabelElement::readAttributeIndexed(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib, const DynamicValue &index) {
	if (attrib == "line") {
		int32 asInteger = 0;
		if (!index.roundToInt(asInteger) || asInteger < 1) {
			thread->error("Invalid text label line index");
			return false;
		}

		size_t lineIndex = asInteger - 1;
		uint32 startPos;
		uint32 endPos;
		if (findLineRange(lineIndex, startPos, endPos))
			result.setString(_text.substr(startPos, endPos - startPos));
		else
			result.setString("");

		return true;
	}

	return VisualElement::readAttributeIndexed(thread, result, attrib, index);
}

MiniscriptInstructionOutcome TextLabelElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "text") {
		DynamicValueWriteFuncHelper<TextLabelElement, &TextLabelElement::scriptSetText>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return VisualElement::writeRefAttribute(thread, writeProxy, attrib);
}

MiniscriptInstructionOutcome TextLabelElement::writeRefAttributeIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib, const DynamicValue &index) {
	if (attrib == "line") {
		int32 asInteger = 0;
		if (!index.roundToInt(asInteger) || asInteger < 1) {
			thread->error("Invalid text label line set index");
			return kMiniscriptInstructionOutcomeFailed;
		}

		writeProxy.pod.ifc = &TextLabelLineWriteInterface::_instance;
		writeProxy.pod.objectRef = this;
		writeProxy.pod.ptrOrOffset = asInteger - 1;
		return kMiniscriptInstructionOutcomeContinue;
	}

	return VisualElement::writeRefAttributeIndexed(thread, writeProxy, attrib, index);
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
	if (!_visible)
		return;

	int renderWidth = _rect.getWidth();
	int renderHeight = _rect.getHeight();
	if (_renderedText) {
		if (renderWidth != _renderedText->w || renderHeight != _renderedText->h)
			_needsRender = true;
	}

	if (_needsRender) {
		_needsRender = false;

		_renderedText.reset();
		_renderedText.reset(new Graphics::ManagedSurface());

		_renderedText->create(renderWidth, renderHeight, Graphics::PixelFormat::createFormatCLUT8());
		_renderedText->fillRect(Common::Rect(0, 0, renderWidth, renderHeight), 0);

		const Graphics::Font *font = nullptr;
		if (_fontFamilyName.size() > 0) {
			font = FontMan.getFontByName(_fontFamilyName.c_str());
		} else if (_macFontID != 0) {
			// TODO: Formatting spans
			int slant = 0;
			// FIXME/HACK: These aren't public...
			if (_styleFlags.bold)
				slant |= 1;
			if (_styleFlags.italic)
				slant |= 2;
			if (_styleFlags.underline)
				slant |= 4;
			if (_styleFlags.outline)
				slant |= 8;
			if (_styleFlags.shadow)
				slant |= 16;
			if (_styleFlags.condensed)
				slant |= 32;
			if (_styleFlags.expanded)
				slant |= 64;

			// FIXME/HACK: This is a stupid way to make getFont return null on failure
			font = _runtime->getMacFontManager()->getFont(Graphics::MacFont(_macFontID, _size, slant, static_cast<Graphics::FontManager::FontUsage>(-1)));
		}

		if (!font)
			font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);

		int height = font->getFontHeight();
		int ascent = font->getFontAscent();

		Graphics::TextAlign textAlign = Graphics::kTextAlignLeft;
		switch (_alignment) {
		case kTextAlignmentLeft:
			textAlign = Graphics::kTextAlignLeft;
			break;
		case kTextAlignmentCenter:
			textAlign = Graphics::kTextAlignCenter;
			break;
		case kTextAlignmentRight:
			textAlign = Graphics::kTextAlignRight;
			break;
		default:
			break;
		}

		int line = 0;
		uint32 lineStart = 0;
		while (lineStart < _text.size()) {
			bool noMoreLines = false;
			uint32 lineEndPos = _text.find('\r', lineStart);
			if (lineEndPos == Common::String::npos) {
				lineEndPos = _text.size();
				noMoreLines = true;
			}

			Common::String lineStr;
			if (lineStart != 0 || lineEndPos != _text.size())
				lineStr = _text.substr(lineStart, lineEndPos - lineStart);
			else
				lineStr = _text;

			// Split the line into sublines
			while (lineStr.size() > 0) {
				size_t lineCommitted = 0;
				bool prevWasWhitespace = true;
				for (size_t i = 0; i <= lineStr.size(); i++) {
					bool isWhitespace = (i == lineStr.size() || lineStr[i] < ' ');

					if (isWhitespace) {
						if (!prevWasWhitespace) {
							int width = font->getStringWidth(lineStr.substr(0, i));
							if (width > renderWidth)
								break;
						}
						lineCommitted = i + 1;
					}

					prevWasWhitespace = isWhitespace;
				}

				if (lineCommitted > lineStr.size())
					lineCommitted = lineStr.size();

				// Too little space for anything
				if (lineCommitted == 0) {
					lineCommitted = 1;
					for (size_t i = 2; i <= lineStr.size(); i++) {
						int width = font->getStringWidth(lineStr.substr(0, i));
						if (width > renderWidth)
							break;
						lineCommitted = i;
					}
				}

				font->drawString(_renderedText.get(), lineStr.substr(0, lineCommitted), 0, line * height + (height - ascent) / 2, renderWidth, 1, textAlign, 0, false);

				if (lineCommitted == lineStr.size())
					lineStr.clear();
				else {
					lineStr = lineStr.substr(lineCommitted);
					line++;
				}
			}

			if (noMoreLines)
				break;

			line++;
			lineStart = lineEndPos + 1;
		}
	}

	Graphics::ManagedSurface *target = window->getSurface().get();
	Common::Rect srcRect(0, 0, renderWidth, renderHeight);
	Common::Rect destRect(_cachedAbsoluteOrigin.x, _cachedAbsoluteOrigin.y, _cachedAbsoluteOrigin.x + _rect.getWidth(), _cachedAbsoluteOrigin.y + _rect.getHeight());

	const uint32 opaqueColor = 0xff000000;
	const uint32 drawPalette[2] = {0, opaqueColor};

	if (_renderedText) {
		_renderedText->setPalette(drawPalette, 0, 2);
		target->transBlitFrom(*_renderedText.get(), srcRect, destRect, 0);
	}
}

void TextLabelElement::setTextStyle(uint16 macFontID, const Common::String &fontFamilyName, uint size, TextAlignment alignment, const TextStyleFlags &styleFlags) {
	_needsRender = true;

	_macFontID = macFontID;
	_fontFamilyName = fontFamilyName;
	_size = size;
	_alignment = alignment;
	_styleFlags = styleFlags;
}

MiniscriptInstructionOutcome TextLabelElement::scriptSetText(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kString) {
		thread->error("Tried to set a text label element's text to something that wasn't a string");
		return kMiniscriptInstructionOutcomeFailed;
	}

	_text = value.getString();
	_needsRender = true;
	_macFormattingSpans.clear();

	return kMiniscriptInstructionOutcomeContinue;
}


MiniscriptInstructionOutcome TextLabelElement::scriptSetLine(MiniscriptThread *thread, size_t lineIndex, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kString) {
		thread->error("Tried to set a text label element's text to something that wasn't a string");
		return kMiniscriptInstructionOutcomeFailed;
	}

	uint32 startPos;
	uint32 endPos;
	if (findLineRange(lineIndex, startPos, endPos))
		_text = _text.substr(0, startPos) + value.getString() + _text.substr(endPos, _text.size() - endPos);
	else {
		size_t numLines = countLines();
		while (numLines <= lineIndex) {
			_text += '\r';
			numLines++;
		}
		_text += value.getString();
	}

	_needsRender = true;
	_macFormattingSpans.clear();
	
	return kMiniscriptInstructionOutcomeContinue;
}

bool TextLabelElement::findLineRange(size_t lineIndex, uint32 &outStartPos, uint32 &outEndPos) const {
	uint32 lineStart = 0;
	uint32 lineEnd = _text.size();
	size_t linesToScan = lineIndex + 1;

	while (linesToScan > 0) {
		linesToScan--;

		lineEnd = _text.find('\r', lineStart);
		if (lineEnd == Common::String::npos) {
			lineEnd = _text.size();
			break;
		}
	}

	if (linesToScan > 0)
		return false;

	outStartPos = lineStart;
	outEndPos = lineEnd;

	return true;
}

size_t TextLabelElement::countLines() const {
	size_t numLines = 1;
	for (char c : _text)
		if (c == '\r')
			numLines++;

	return numLines;
}

MiniscriptInstructionOutcome TextLabelElement::TextLabelLineWriteInterface::write(MiniscriptThread *thread, const DynamicValue &dest, void *objectRef, uintptr ptrOrOffset) const {
	return static_cast<TextLabelElement *>(objectRef)->scriptSetLine(thread, ptrOrOffset, dest);
}

MiniscriptInstructionOutcome TextLabelElement::TextLabelLineWriteInterface::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) const {
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome TextLabelElement::TextLabelLineWriteInterface::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) const {
	return kMiniscriptInstructionOutcomeFailed;
}

TextLabelElement::TextLabelLineWriteInterface TextLabelElement::TextLabelLineWriteInterface::_instance;


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

#ifdef MTROPOLIS_DEBUG_ENABLE
void SoundElement::debugInspect(IDebugInspectionReport *report) const {
	NonVisualElement::debugInspect(report);

	report->declareDynamic("leftVol", Common::String::format("%i", _leftVolume));
	report->declareDynamic("rightVol", Common::String::format("%i", _rightVolume));
	report->declareDynamic("balance", Common::String::format("%i", _balance));
	report->declareDynamic("asset", Common::String::format("%i", _assetID));

	AudioMetadata *metadata = _metadata.get();
	report->declareDynamic("duration", metadata ? Common::String::format("%i", metadata->durationMSec) : Common::String("Unknown"));
	report->declareDynamic("finishTime", Common::String::format("%i", static_cast<int>(_finishTime)));
	report->declareDynamic("shouldPlayIfNotPaused", _shouldPlayIfNotPaused ? "true" : "false");
	report->declareDynamic("paused", _paused ? "true" : "false");
	report->declareDynamic("needsReset", _needsReset ? "true" : "false");
}
#endif

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
