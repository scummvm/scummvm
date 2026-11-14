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
#include "common/noncopyable.h"
#include "common/rect.h"
#include "common/str.h"

#include "ripper/media/playback_plan.h"

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

	virtual void beginIavfSegment(uint sequenceId) {}
	virtual void beginIavfSegment(uint sequenceId, uint frameCount,
		uint width, uint height) {
		beginIavfSegment(sequenceId);
	}
	virtual uint16 service(uint frame) = 0;
	virtual bool continueAfterEnd() const { return false; }
	virtual bool ownsInput() const { return false; }
	virtual bool keepsCursorVisible() const { return false; }
	virtual bool managesPalette() const { return false; }
	virtual void transformPalette(byte *palette, uint colorCount) const {}
};


/**
 * Public playback methods that accept a stream pointer take ownership of it,
 * including on failure. Keep borrowed video data alive until the call returns
 * and borrowed audio data alive until playback finishes or is stopped.
 */
class MediaPlayer {
public:
	MediaPlayer(RipperEngine *engine, InputManager *input, Audio::Mixer *mixer);
	~MediaPlayer();

	bool play(const Common::String &path, bool allowEscSpace, int x = -1, int y = -1,
		bool sceneViewport = false,
		const Common::String &presentationText = Common::String());
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
	bool displayInterfacePcx(const Common::String &path);
	bool playBlockingAudio(const Common::String &path, bool showCursor = true,
		const Common::String &presentationText = Common::String(),
		int displayTop = 0);
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
	bool playInteractiveIavf(const Common::String &path,
		MediaSequenceCallback *callback, uint16 *command);
	void fadePalette(bool fadeIn, uint stepCount);
	bool playScene(const Common::String &path, const ScenePlaybackOptions &options);
	void pauseActiveMedia(bool pause);

private:
	enum PlaybackInputResult {
		kContinuePlayback,
		kStopPlayback,
		kQuitPlayback,
		kStopForMouse,
		kSkipToEnd,
		kAdvanceSegment
	};

	struct ActivePlayback {
		Video::SmackerDecoder *decoder;
		Common::String name;
		Audio::SoundHandle *externalAudio;

		ActivePlayback(Video::SmackerDecoder *decoder_, const Common::String &name_,
			Audio::SoundHandle *externalAudio_) : decoder(decoder_), name(name_),
			externalAudio(externalAudio_) {}
	};

	class ActivePlaybackGuard : private Common::NonCopyable {
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
	SmackerPlaybackResult playValidatedSmacker(Common::SeekableReadStream *stream,
		const Common::String &name, const char *description,
		const SmackerPlaybackPlan &plan);
	bool playScaledInteractiveSequence(const Common::String &path,
		const char *description, MediaSequenceCallback *callback, uint16 *command,
		uint loopStartFrame = 0);
	SmackerPlaybackResult playSmacker(Common::SeekableReadStream *stream, const Common::String &name,
		const SmackerPlaybackPlan &plan);
	bool playIavf(Common::SeekableReadStream &stream, const Common::String &name,
		const IavfPlaybackOptions &options);
	PlaybackInputResult servicePlaybackInput(Video::SmackerDecoder &decoder,
		const SmackerInputPolicy &policy, bool &paused,
		Audio::SoundHandle *externalAudio, bool toolbarOwnsInput,
		const Common::String &name);
	void logPlaybackPause(const char *source, bool pause, Video::SmackerDecoder &decoder,
		const Common::String &name, Audio::SoundHandle *externalAudio) const;

	RipperEngine *_engine;
	InputManager *_input;
	Audio::Mixer *_mixer;
	Common::Array<ActivePlayback> _activePlaybacks;
};

} // End of namespace Ripper

#endif // RIPPER_MEDIA_H
