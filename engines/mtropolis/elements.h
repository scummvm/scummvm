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

#ifndef MTROPOLIS_ELEMENTS_H
#define MTROPOLIS_ELEMENTS_H

#include "graphics/fontman.h"

#include "mtropolis/data.h"
#include "mtropolis/runtime.h"
#include "mtropolis/render.h"

namespace Video {

class VideoDecoder;

} // End of namespace Video

namespace MTropolis {

class AudioPlayer;
class CachedAudio;
class CachedImage;
class CachedMToon;
struct AudioMetadata;
struct ElementLoaderContext;
struct MToonMetadata;

enum MediaState {
	kMediaStatePlaying,
	kMediaStatePlayingLastFrame,
	kMediaStateStopped,
	kMediaStatePaused,
};

class GraphicElement : public VisualElement {
public:
	GraphicElement();
	~GraphicElement();

	bool load(ElementLoaderContext &context, const Data::GraphicElement &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

	void render(Window *window) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Graphic Element"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	bool _cacheBitmap;

	Common::SharedPtr<Graphics::ManagedSurface> _mask;
};

class MovieResizeFilter {
public:
	virtual ~MovieResizeFilter();

	virtual Common::SharedPtr<Graphics::Surface> scaleFrame(const Graphics::Surface &surface, uint32 timestamp) const = 0;
};

class MovieElement : public VisualElement, public ISegmentUnloadSignalReceiver, public IPlayMediaSignalReceiver {
public:
	MovieElement();
	~MovieElement();

	bool load(ElementLoaderContext &context, const Data::MovieElement &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

	VThreadState consumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void activate() override;
	void deactivate() override;

	bool canAutoPlay() const override;
	void queueAutoPlayEvents(Runtime *runtime, bool isAutoPlaying) override;

	void render(Window *window) override;
	void playMedia(Runtime *runtime, Project *project) override;

	void setResizeFilter(const Common::SharedPtr<MovieResizeFilter> &filter);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Movie Element"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

protected:
	void onPauseStateChanged() override;
	void onSegmentUnloaded(int segmentIndex) override;

private:
	IntRange computeRealRange() const;

	void stopSubtitles();

	MiniscriptInstructionOutcome scriptSetRange(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetRangeStart(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetRangeEnd(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptRangeWriteRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib);
	MiniscriptInstructionOutcome scriptSetVolume(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetTimestamp(MiniscriptThread *thread, const DynamicValue &value);

	MiniscriptInstructionOutcome scriptSetRangeTyped(MiniscriptThread *thread, const IntRange &range);

	struct StartPlayingTaskData {
		StartPlayingTaskData() : runtime(nullptr) {}

		Runtime *runtime;
	};

	struct SeekToTimeTaskData {
		SeekToTimeTaskData() : runtime(nullptr), timestamp(0) {}

		Runtime *runtime;
		uint32 timestamp;
	};

	VThreadState startPlayingTask(const StartPlayingTaskData &taskData);
	VThreadState seekToTimeTask(const SeekToTimeTaskData &taskData);

	bool _cacheBitmap;
	bool _alternate;
	bool _playEveryFrame;
	bool _reversed;
	//bool _haveFiredAtLastCel;
	//bool _haveFiredAtFirstCel;
	bool _shouldPlayIfNotPaused;
	bool _needsReset;	// If true, then the video position was reset by a seek or stop and decoding must be restarted even if the target state is the same as the play state.
	MediaState _currentPlayState;
	uint32 _assetID;

	Common::SharedPtr<Video::VideoDecoder> _videoDecoder;
	uint32 _maxTimestamp;
	uint32 _timeScale;
	uint32 _currentTimestamp;
	int32 _volume;
	IntRange _playRange;

	const Graphics::Surface *_displayFrame;
	Common::SharedPtr<Graphics::Surface> _scaledFrame;
	Common::SharedPtr<MovieResizeFilter> _resizeFilter;

	Common::SharedPtr<SegmentUnloadSignaller> _unloadSignaller;
	Common::SharedPtr<PlayMediaSignaller> _playMediaSignaller;

	Common::SharedPtr<SubtitlePlayer> _subtitles;

	Common::Array<int> _damagedFrames;

	Runtime *_runtime;
};

class ImageElement : public VisualElement {
public:
	ImageElement();
	~ImageElement();

	bool load(ElementLoaderContext &context, const Data::ImageElement &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) override;

	void activate() override;
	void deactivate() override;

	void render(Window *window) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Image Element"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	MiniscriptInstructionOutcome scriptSetFlushPriority(MiniscriptThread *thread, const DynamicValue &value);

	bool _cacheBitmap;
	uint32 _assetID;

	Common::SharedPtr<CachedImage> _cachedImage;

	Common::String _text;	// ...???

	Runtime *_runtime;
};

class MToonElement : public VisualElement, public IPlayMediaSignalReceiver {
public:
	MToonElement();
	~MToonElement();

	bool load(ElementLoaderContext &context, const Data::MToonElement &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

	VThreadState consumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void activate() override;
	void deactivate() override;

	bool canAutoPlay() const override;

	void render(Window *window) override;

	bool isMouseCollisionAtPoint(int32 relativeX, int32 relativeY) const override;

	Common::Rect getRelativeCollisionRect() const override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "mToon Element"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	struct StartPlayingTaskData {
		StartPlayingTaskData() : runtime(nullptr) {}

		Runtime *runtime;
	};

	struct StopPlayingTaskData {
		StopPlayingTaskData() : runtime(nullptr) {}

		Runtime *runtime;
	};

	struct ChangeFrameTaskData {
		ChangeFrameTaskData() : runtime(nullptr), frame(0) {}

		Runtime *runtime;
		uint32 frame;
	};

	VThreadState startPlayingTask(const StartPlayingTaskData &taskData);
	VThreadState stopPlayingTask(const StopPlayingTaskData &taskData);

	void playMedia(Runtime *runtime, Project *project) override;
	MiniscriptInstructionOutcome scriptSetRate(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetCel(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetRange(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetRangeStart(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetRangeEnd(MiniscriptThread *thread, const DynamicValue &value);

	MiniscriptInstructionOutcome scriptRangeWriteRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib);
	MiniscriptInstructionOutcome scriptSetRangeTyped(MiniscriptThread *thread, const IntRange &value);

	void onPauseStateChanged() override;

	bool _cacheBitmap;

	// If set, then carry over residual frame time and display at the desired rate.  If not set, reset residual each frame for smoother animation.
	bool _maintainRate;

	uint32 _assetID;
	int32 _rateTimes100000;
	int32 _flushPriority;
	uint32 _celStartTimeMSec;
	bool _isPlaying;	// Is actually rolling media, this is only set by playMedia because it needs to start after scene transition

	Runtime *_runtime;
	Common::SharedPtr<Graphics::Surface> _renderSurface;
	uint32 _renderedFrame;

	Common::SharedPtr<MToonMetadata> _metadata;
	Common::SharedPtr<CachedMToon> _cachedMToon;
	Common::SharedPtr<PlayMediaSignaller> _playMediaSignaller;

	// NOTE: To produce proper behavior, these are not sanitized until playMedia.  render must tolerate invalid values without changing them.
	IntRange _playRange;
	int32 _cel;
};

class TextLabelElement : public VisualElement {
public:
	TextLabelElement();
	~TextLabelElement();

	bool isTextLabel() const override;

	bool load(ElementLoaderContext &context, const Data::TextLabelElement &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	bool readAttributeIndexed(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib, const DynamicValue &index) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttributeIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib, const DynamicValue &index) override;

	void activate() override;
	void deactivate() override;

	void render(Window *window) override;

	void setTextStyle(uint16 macFontID, const Common::String &fontFamilyName, uint size, TextAlignment alignment, const TextStyleFlags &styleFlags);

	Graphics::FontManager::FontUsage getDefaultUsageForMacFont(uint16 macFontID, uint size);
	Graphics::FontManager::FontUsage getDefaultUsageForNamedFont(const Common::String &fontFamilyName, uint size);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Text Label Element"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusPartial; }
#endif

private:
	struct TextLabelLineWriteInterface {
		static MiniscriptInstructionOutcome write(MiniscriptThread *thread, const DynamicValue &dest, void *objectRef, uintptr ptrOrOffset);
		static MiniscriptInstructionOutcome refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib);
		static MiniscriptInstructionOutcome refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index);
	};

	MiniscriptInstructionOutcome scriptSetText(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetLine(MiniscriptThread *thread, size_t lineIndex, const DynamicValue &value);

	bool findLineRange(size_t lineNum, uint32 &outStartPos, uint32 &outEndPos) const;
	size_t countLines() const;

	bool _cacheBitmap;
	bool _needsRender;

	//bool _isBitmap;
	uint32 _assetID;

	Common::String _text;
	uint16 _macFontID;
	Common::String _fontFamilyName;
	uint _size;
	TextAlignment _alignment;
	TextStyleFlags _styleFlags;

	Common::Array<MacFormattingSpan> _macFormattingSpans;

	// NOTE: This may be a surface loaded from data, so it must not be altered.
	// If you need to render again, recreate the surface.  If you want to change
	// this behavior, please add a flag indicating that it is from the asset.
	Common::SharedPtr<Graphics::ManagedSurface> _renderedText;

	Runtime *_runtime;
};

class SoundElement : public NonVisualElement, public IPlayMediaSignalReceiver {
public:
	SoundElement();
	~SoundElement();

	bool load(ElementLoaderContext &context, const Data::SoundElement &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) override;

	VThreadState consumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void activate() override;
	void deactivate() override;

	bool canAutoPlay() const override;

	void playMedia(Runtime *runtime, Project *project) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Sound Element"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	void stopPlayer();

	MiniscriptInstructionOutcome scriptSetLoop(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetVolume(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetBalance(MiniscriptThread *thread, const DynamicValue &value);

	struct StartPlayingTaskData {
		StartPlayingTaskData() : runtime(nullptr) {}

		Runtime *runtime;
	};

	VThreadState startPlayingTask(const StartPlayingTaskData &taskData);
	VThreadState stopPlayingTask(const StartPlayingTaskData &taskData);

	void setLoop(bool loop);
	void setVolume(uint16 volume);
	void setBalance(int16 balance);

	uint16 _leftVolume;
	uint16 _rightVolume;
	int16 _balance;
	uint32 _assetID;

	Common::SharedPtr<CachedAudio> _cachedAudio;
	Common::SharedPtr<AudioMetadata> _metadata;
	Common::SharedPtr<AudioPlayer> _player;
	uint64 _startTime;
	uint64 _finishTime;
	uint64 _startTimestamp;	// Time in the sound corresponding to the start time
	uint64 _cueCheckTime;
	bool _shouldPlayIfNotPaused;
	bool _needsReset;

	Common::SharedPtr<PlayMediaSignaller> _playMediaSignaller;

	Common::SharedPtr<SubtitlePlayer> _subtitlePlayer;

	Runtime *_runtime;
};

} // End of namespace MTropolis

#endif
