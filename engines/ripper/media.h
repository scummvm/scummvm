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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_MEDIA_H
#define RIPPER_MEDIA_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace Video {
class SmackerDecoder;
}

namespace Ripper {

class InputManager;
class RipperEngine;

enum MediaFormat {
	kMediaFormatUnknown,
	kMediaFormatSmacker,
	kMediaFormatIavf
};

MediaFormat detectMediaFormat(Common::SeekableReadStream &stream);
const char *mediaFormatName(MediaFormat format);

class MediaSequenceCallback {
public:
	// A nested presentation may replace the active VGA palette while the outer
	// Smacker stays paused. Resume it after restoring its decoder palette.
	static const uint16 kContinueRefreshPalette = 0xffff;

	virtual ~MediaSequenceCallback() {}

	virtual uint16 service(uint frame) = 0;
	virtual bool continueAfterEnd() const { return false; }
	virtual bool ownsInput() const { return false; }
	virtual bool managesPalette() const { return false; }
	virtual void transformPalette(byte *palette, uint colorCount) const {}
};

struct SmackerPlacementPolicy {
	int x;
	int y;
	int originY;
	uint displayScale;
	Common::Rect centerBounds;

	SmackerPlacementPolicy() : x(-1), y(-1), originY(0), displayScale(1),
		centerBounds() {}
};

struct SmackerInputPolicy {
	bool allowEscSpace;
	bool serviceSceneUi;
	bool *stoppedByUser;
	bool *advanceSegment;

	SmackerInputPolicy() : allowEscSpace(false), serviceSceneUi(false),
		stoppedByUser(nullptr), advanceSegment(nullptr) {}
};

struct SmackerTimelinePolicy {
	Audio::SoundHandle *externalAudio;
	const Common::Array<uint32> *frameAudioOffsets;
	uint32 audioByteRate;
	uint32 timelineStartMillis;

	SmackerTimelinePolicy() : externalAudio(nullptr), frameAudioOffsets(nullptr),
		audioByteRate(0), timelineStartMillis(0) {}
};

struct SmackerPalettePolicy {
	bool patchInterfacePalette;
	bool patchWacMediaPalette;
	bool preserveDisplayPalette;
	Common::Array<byte> *sourcePalette;
	bool rememberVideoPalette;

	SmackerPalettePolicy() : patchInterfacePalette(true),
		patchWacMediaPalette(false), preserveDisplayPalette(false), sourcePalette(nullptr),
		rememberVideoPalette(true) {}
};

struct SmackerFramePolicy {
	uint frameLimit;
	uint firstFrame;
	uint lastFrame;

	SmackerFramePolicy() : frameLimit(0), firstFrame(0), lastFrame(0xffffffff) {}
};

struct SmackerLoopPolicy {
	bool loopFromStart;
	uint loopStartFrame;
	uint boundedLoopStartFrame;

	SmackerLoopPolicy() : loopFromStart(false), loopStartFrame(0),
		boundedLoopStartFrame(0xffffffff) {}
};

struct SmackerCallbackPolicy {
	MediaSequenceCallback *sequenceCallback;
	uint16 *sequenceCommand;

	SmackerCallbackPolicy() : sequenceCallback(nullptr), sequenceCommand(nullptr) {}
};

struct SmackerRenderingPolicy {
	bool transparentFirstPixel;
	bool retainFinalTransparentFrame;

	SmackerRenderingPolicy() : transparentFirstPixel(false), retainFinalTransparentFrame(false) {}
};

struct SmackerPlaybackPlan {
	SmackerPlacementPolicy placement;
	SmackerInputPolicy input;
	SmackerTimelinePolicy timeline;
	SmackerPalettePolicy palette;
	SmackerFramePolicy frames;
	SmackerLoopPolicy loop;
	SmackerCallbackPolicy callback;
	SmackerRenderingPolicy rendering;
	const char *retailRoute;

	SmackerPlaybackPlan() : retailRoute("RunMediaSequence@0x1e516") {}
};

Common::String describeSmackerPlaybackPlan(const SmackerPlaybackPlan &plan);

class MediaPlayer {
public:
	MediaPlayer(RipperEngine *engine, InputManager *input, Audio::Mixer *mixer);
	~MediaPlayer();

	bool play(const Common::String &path, bool allowEscSpace, int x = -1, int y = -1,
		bool sceneViewport = false);
	bool playWacMedia(const Common::String &path, int x, int y);
	bool playWacPresentation(const Common::String &path, int x, int y);
	bool playWacInterfaceSequence(const Common::String &path,
		const Common::Rect &centerBounds,
		uint loopStartFrame, MediaSequenceCallback *callback, uint16 *command);
	bool playWacInterfaceSequenceStream(Common::SeekableReadStream *stream,
		const Common::String &name, const Common::Rect &centerBounds,
		uint loopStartFrame, MediaSequenceCallback *callback, uint16 *command);
	bool playInterfaceSequence(const Common::String &path, int x, int y,
		Common::Array<byte> &sourcePalette);
	bool displayScenePcx(const Common::String &path);
	bool playBlockingAudio(const Common::String &path);
	bool playSoundEffect(const Common::String &path, Audio::SoundHandle &handle,
		uint volumePercent = 100, bool loop = false);
	bool playSoundEffectStream(Common::SeekableReadStream *stream,
		const Common::String &name, Audio::SoundHandle &handle,
		uint volumePercent = 100, bool loop = false);
	bool playRawSoundEffect(const Common::Array<byte> &data, uint sampleRate,
		byte flags, Audio::SoundHandle &handle, uint volumePercent = 100);
	bool playVoiceClip(const Common::String &path, Audio::SoundHandle &handle,
		uint volumePercent = 100);
	bool playVoiceClipStream(Common::SeekableReadStream *stream,
		const Common::String &name, Audio::SoundHandle &handle,
		uint volumePercent = 100);
	bool isSoundEffectActive(const Audio::SoundHandle &handle) const;
	uint32 getSoundEffectElapsedTime(const Audio::SoundHandle &handle) const;
	bool stopSoundEffect(Audio::SoundHandle &handle);
	void setSoundEffectVolume(Audio::SoundHandle &handle, uint volumePercent);
	bool playPuzzleSequence(const Common::String &path, uint loopStartFrame,
		MediaSequenceCallback *callback, uint16 *command = nullptr);
	bool playPuzzleSequenceStream(Common::SeekableReadStream *stream,
		const Common::String &name, int x, int y, uint loopStartFrame,
		MediaSequenceCallback *callback, uint16 *command = nullptr);
	bool playSceneStream(Common::SeekableReadStream *stream,
		const Common::String &name, int x, int y, bool allowEscSpace);
	bool playPuzzleSequenceSegment(const Common::String &path, uint firstFrame,
		uint lastFrame, int x, int y, MediaSequenceCallback *callback,
		uint16 *command = nullptr, uint boundedLoopStartFrame = 0xffffffff);
	bool playCombatSequence(const Common::String &path,
		MediaSequenceCallback *callback, uint16 *command = nullptr);
	bool playShootingGallerySequence(const Common::String &path,
		MediaSequenceCallback *callback, uint16 *command = nullptr);
	bool playBlobShooterSequence(const Common::String &path,
		MediaSequenceCallback *callback, uint16 *command = nullptr);
	bool playTransparentSmackerOverlay(const Common::String &path, int x, int y);
	void fadePalette(bool fadeIn, uint stepCount);
	bool playScene(const Common::String &path, int x, int y, bool firstFrameOnly,
		bool loopUntilInput = false, bool allowEscSpace = false,
		MediaSequenceCallback *callback = nullptr, uint16 *command = nullptr);
	void pauseActiveMedia(bool pause);

private:
	enum SourcePolicy {
		kSourceDirectFile,
		kSourceConfiguredPath,
		kSourceInterfaceLibrary,
		kSourceSoundEffect,
		kSourceBlockingAudio
	};

	struct ActivePlayback {
		Video::SmackerDecoder *decoder;
		Common::String name;
		Audio::SoundHandle *externalAudio;

		ActivePlayback(Video::SmackerDecoder *decoder_, const Common::String &name_,
			Audio::SoundHandle *externalAudio_) : decoder(decoder_), name(name_),
			externalAudio(externalAudio_) {}
	};

	class ActivePlaybackGuard {
	public:
		ActivePlaybackGuard(MediaPlayer *player, Video::SmackerDecoder *decoder,
			const Common::String &name, Audio::SoundHandle *externalAudio);
		~ActivePlaybackGuard();

	private:
		MediaPlayer *_player;
	};

	bool playAudioClip(const Common::String &path, Audio::SoundHandle &handle,
		Audio::Mixer::SoundType soundType, uint volumePercent, bool loop,
		const char *description);
	bool playAudioStream(Common::SeekableReadStream *audioStream,
		const Common::String &name, const Common::String &source,
		Audio::SoundHandle &handle, Audio::Mixer::SoundType soundType,
		uint volumePercent, bool loop, const char *description);
	Common::SeekableReadStream *openSource(const Common::String &path,
		SourcePolicy policy, Common::String &source) const;
	bool playValidatedSmacker(Common::SeekableReadStream *stream,
		const Common::String &name, const char *description,
		const SmackerPlaybackPlan &plan);
	bool playScaledInteractiveSequence(const Common::String &path,
		const char *description, MediaSequenceCallback *callback, uint16 *command,
		uint loopStartFrame = 0);
	bool playSmacker(Common::SeekableReadStream *stream, const Common::String &name,
		const SmackerPlaybackPlan &plan);
	bool playIavf(Common::SeekableReadStream &stream, const Common::String &name,
		bool allowEscSpace, int overrideX = -1, int overrideY = -1,
		int overrideOriginY = 0, bool serviceSceneUi = false,
		bool rememberVideoPalette = true, uint displayScale = 0);
	bool servicePlaybackInput(Video::SmackerDecoder &decoder, bool allowEscSpace,
		bool allowSegmentAdvance, bool &paused, bool &skipToEnd,
		bool &advanceSegment,
		Audio::SoundHandle *externalAudio, bool suppressSceneMouseStop, bool allowSceneHelp,
		const Common::String &name);
	void logPlaybackPause(const char *source, bool pause, Video::SmackerDecoder &decoder,
		const Common::String &name, Audio::SoundHandle *externalAudio) const;

	RipperEngine *_engine;
	InputManager *_input;
	Audio::Mixer *_mixer;
	Common::Array<ActivePlayback> _activePlaybacks;
	bool _stopSceneOnMouse;
};

} // End of namespace Ripper

#endif // RIPPER_MEDIA_H
