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
#include "common/ptr.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
class Serializer;
}

namespace Video {
class SmackerDecoder;
}

namespace Ripper {

class InputManager;
class RipperEngine;
class SceneAudioManager;

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

struct SmackerPlaybackRequest {
	bool allowEscSpace;
	int x;
	int y;
	Audio::SoundHandle *externalAudio;
	bool *stoppedByUser;
	const Common::Array<uint32> *frameAudioOffsets;
	uint32 audioByteRate;
	uint32 timelineStartMillis;
	uint displayScale;
	bool patchInterfacePalette;
	uint frameLimit;
	int originY;
	bool patchWacMediaPalette;
	bool serviceSceneUi;
	bool repeatedLoopPass;
	bool *advanceSegment;
	uint loopStartFrame;
	MediaSequenceCallback *sequenceCallback;
	uint16 *sequenceCommand;
	Common::Array<byte> *sourcePalette;
	bool rememberVideoPalette;
	uint firstFrame;
	uint lastFrame;
	uint boundedLoopStartFrame;
	bool transparentFirstPixel;

	SmackerPlaybackRequest() : allowEscSpace(false), x(-1), y(-1),
		externalAudio(nullptr), stoppedByUser(nullptr), frameAudioOffsets(nullptr),
		audioByteRate(0), timelineStartMillis(0), displayScale(1),
		patchInterfacePalette(true), frameLimit(0), originY(0),
		patchWacMediaPalette(false), serviceSceneUi(false), repeatedLoopPass(false),
		advanceSegment(nullptr), loopStartFrame(0), sequenceCallback(nullptr),
		sequenceCommand(nullptr), sourcePalette(nullptr), rememberVideoPalette(true),
		firstFrame(0), lastFrame(0xffffffff), boundedLoopStartFrame(0xffffffff),
		transparentFirstPixel(false) {}
};

class MediaPlayer {
public:
	MediaPlayer(RipperEngine *engine, InputManager *input, Audio::Mixer *mixer);
	~MediaPlayer();

	bool play(const Common::String &path, bool allowEscSpace, int x = -1, int y = -1,
		bool sceneViewport = false);
	bool playWacMedia(const Common::String &path, int x, int y);
	bool playWacInterfaceSequence(const Common::String &path, int x, int y,
		uint loopStartFrame, MediaSequenceCallback *callback, uint16 *command);
	bool playInterfaceSequence(const Common::String &path, int x, int y,
		Common::Array<byte> &sourcePalette);
	bool playBlockingAudio(const Common::String &path);
	bool playSoundEffect(const Common::String &path, Audio::SoundHandle &handle,
		uint volumePercent = 100, bool loop = false);
	bool playVoiceClip(const Common::String &path, Audio::SoundHandle &handle,
		uint volumePercent = 100);
	bool isSoundEffectActive(const Audio::SoundHandle &handle) const;
	bool stopSoundEffect(Audio::SoundHandle &handle);
	void setSoundEffectVolume(Audio::SoundHandle &handle, uint volumePercent);
	bool playPuzzleSequence(const Common::String &path, uint loopStartFrame,
		MediaSequenceCallback *callback, uint16 *command = nullptr);
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
	bool loadAudio(const Common::String &path, bool preserve);
	bool configureAudio(const Common::String &key, uint volumePercent, uint triggerFrame,
		byte control);
	void clearAudio(const Common::String &key);
	void stopAudio(const Common::String &key);
	void setAudioVolume(const Common::String &key, uint targetVolumePercent,
		uint startFrame, uint timing);
	void serviceSceneAudio(uint frame);
	void resetSceneAudioTriggers();
	void clearSceneAudio(bool includePreserved);
	bool isSceneAudioActive() const;
	bool syncGame(Common::Serializer &serializer);

private:
	bool playAudioClip(const Common::String &path, Audio::SoundHandle &handle,
		Audio::Mixer::SoundType soundType, uint volumePercent, bool loop,
		const char *description);
	bool playScaledInteractiveSequence(const Common::String &path,
		const char *description, MediaSequenceCallback *callback, uint16 *command,
		uint loopStartFrame = 0);
	bool playSmacker(Common::SeekableReadStream *stream, const Common::String &name,
		const SmackerPlaybackRequest &request);
	bool playIavf(Common::SeekableReadStream &stream, const Common::String &name,
		bool allowEscSpace, int overrideX = -1, int overrideY = -1,
		int overrideOriginY = 0, bool serviceSceneUi = false);
	bool servicePlaybackInput(Video::SmackerDecoder &decoder, bool allowEscSpace,
		bool allowSegmentAdvance, bool &paused, bool toolbarPaused, bool &skipToEnd,
		bool &advanceSegment,
		Audio::SoundHandle *externalAudio, bool suppressSceneMouseStop, bool allowSceneHelp);

	RipperEngine *_engine;
	InputManager *_input;
	Audio::Mixer *_mixer;
	Common::ScopedPtr<SceneAudioManager> _sceneAudio;
	bool _stopSceneOnMouse;
};

} // End of namespace Ripper

#endif // RIPPER_MEDIA_H
