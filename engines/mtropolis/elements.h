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

#include "mtropolis/data.h"
#include "mtropolis/runtime.h"
#include "mtropolis/render.h"

#include "audio/mixer.h"

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
	kMediaStateStopped,
	kMediaStatePaused,
};

class GraphicElement : public VisualElement {
public:
	GraphicElement();
	~GraphicElement();

	bool load(ElementLoaderContext &context, const Data::GraphicElement &data);

	void render(Window *window) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Graphic Element"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusPartial; }
#endif

private:
	bool _cacheBitmap;
};

class MovieElement : public VisualElement, public ISegmentUnloadSignalReceiver, public IPlayMediaSignalReceiver {
public:
	MovieElement();
	~MovieElement();

	bool load(ElementLoaderContext &context, const Data::MovieElement &data);

	VThreadState consumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void activate() override;
	void deactivate() override;

	void render(Window *window) override;
	void playMedia(Runtime *runtime, Project *project) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Movie Element"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusPartial; }
#endif

private:
	void onSegmentUnloaded(int segmentIndex) override;

	struct StartPlayingTaskData {
		Runtime *runtime;
	};

	VThreadState startPlayingTask(const StartPlayingTaskData &taskData);

	bool _cacheBitmap;
	bool _alternate;
	bool _playEveryFrame;
	bool _reversed;
	bool _haveFiredAtLastCel;
	bool _haveFiredAtFirstCel;
	bool _shouldPlayIfNotPaused;
	bool _needsReset;	// If true, then the video position was reset by a seek or stop and decoding must be restarted even if the target state is the same as the play state.
	MediaState _currentPlayState;
	uint32 _assetID;

	Common::SharedPtr<Video::VideoDecoder> _videoDecoder;
	const Graphics::Surface *_displayFrame;

	Common::SharedPtr<SegmentUnloadSignaller> _unloadSignaller;
	Common::SharedPtr<PlayMediaSignaller> _playMediaSignaller;

	Runtime *_runtime;
};

class ImageElement : public VisualElement {
public:
	ImageElement();
	~ImageElement();

	bool load(ElementLoaderContext &context, const Data::ImageElement &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib);
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib);

	void activate() override;
	void deactivate() override;

	void render(Window *window) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Image Element"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	bool _cacheBitmap;
	uint32 _assetID;

	Common::SharedPtr<CachedImage> _cachedImage;

	Common::String _text;	// ...???

	Runtime *_runtime;
};

class MToonElement : public VisualElement {
public:
	MToonElement();
	~MToonElement();

	bool load(ElementLoaderContext &context, const Data::MToonElement &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

	void activate() override;
	void deactivate() override;

	void render(Window *window) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "mToon Element"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusPartial; }
#endif

private:
	bool _cacheBitmap;

	// If set, then carry over residual frame time and display at the desired rate.  If not set, reset residual each frame for smoother animation.
	bool _maintainRate;

	uint32 _assetID;
	uint32 _rateTimes10000;
	uint32 _cel1Based;
	int32 _flushPriority;

	Runtime *_runtime;
	Common::SharedPtr<Graphics::Surface> _renderSurface;
	uint32 _renderedFrame;

	Common::SharedPtr<MToonMetadata> _metadata;
	Common::SharedPtr<CachedMToon> _cachedMToon;
};

class TextLabelElement : public VisualElement {
public:
	TextLabelElement();
	~TextLabelElement();

	bool load(ElementLoaderContext &context, const Data::TextLabelElement &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib);
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib);

	void activate() override;
	void deactivate() override;

	void render(Window *window) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Text Label Element"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusPartial; }
#endif

private:
	bool _cacheBitmap;
	bool _needsRender;

	bool _isBitmap;
	uint32 _assetID;

	Common::String _text;
	Common::Array<MacFormattingSpan> _macFormattingSpans;
	Common::SharedPtr<Graphics::Surface> _renderedText;	// NOTE: This may be a pre-rendered instance that is read-only.  Rendering must create a new surface!

	Runtime *_runtime;
};

class SoundElement : public NonVisualElement, public IPlayMediaSignalReceiver {
public:
	SoundElement();
	~SoundElement();

	bool load(ElementLoaderContext &context, const Data::SoundElement &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib);
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib);

	VThreadState consumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg);

	void activate() override;
	void deactivate() override;

	void playMedia(Runtime *runtime, Project *project) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Sound Element"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusPartial; }
#endif

private:
	MiniscriptInstructionOutcome scriptSetLoop(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetVolume(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetBalance(MiniscriptThread *thread, const DynamicValue &value);

	struct StartPlayingTaskData {
		Runtime *runtime;
	};

	VThreadState startPlayingTask(const StartPlayingTaskData &taskData);

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
	uint64 _finishTime;
	bool _shouldPlayIfNotPaused;
	bool _needsReset;

	Common::SharedPtr<PlayMediaSignaller> _playMediaSignaller;

	Runtime *_runtime;
};

} // End of namespace MTropolis

#endif
