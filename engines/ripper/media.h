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

class MediaPlayer {
public:
	MediaPlayer(RipperEngine *engine, InputManager *input, Audio::Mixer *mixer);
	~MediaPlayer();

	bool play(const Common::String &path, bool allowEscSpace, int x = -1, int y = -1,
		bool sceneViewport = false);
	bool playWacMedia(const Common::String &path, int x, int y);
	bool playBlockingAudio(const Common::String &path);
	bool playSoundEffect(const Common::String &path, Audio::SoundHandle &handle,
		uint volumePercent = 100);
	bool stopSoundEffect(Audio::SoundHandle &handle);
	bool playScene(const Common::String &path, int x, int y, bool firstFrameOnly,
		bool loopUntilInput = false, bool allowEscSpace = false);
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
	// InitializeAudioTriggerSlotFromPath at 0x37174 and the scene handlers at
	// 0x15e48..0x160cc operate on g_audioTriggerSlots[20]. Keep the table intact
	// across script callbacks so later scenes can address preserved resources.
	struct AudioSlot {
		Common::String path;
		Common::String key;
		Audio::SoundHandle handle;
		uint volumePercent;
		uint targetVolumePercent;
		uint triggerFrame;
		uint volumeStartFrame;
		uint volumeTiming;
		uint volumeRampStep;
		uint volumeRampProgress;
		int volumeRampDirection;
		byte control;
		bool occupied;
		bool preserve;
		bool volumeRampPending;
		bool sparseVolumeRamp;

		AudioSlot();
	};

	static const uint kAudioSlotCount = 20;
	AudioSlot *findAudioSlot(const Common::String &key);
	const AudioSlot *findAudioSlot(const Common::String &key) const;
	bool startAudioSlot(AudioSlot &slot);
	void clearAudioSlot(AudioSlot &slot);
	void applyAudioSlotVolume(AudioSlot &slot);
	Common::String describeAudioSlots() const;
	bool playSmacker(Common::SeekableReadStream *stream, const Common::String &name,
		bool allowEscSpace, int x, int y, Audio::SoundHandle *externalAudio = nullptr,
		bool *stoppedByUser = nullptr, const Common::Array<uint32> *frameAudioOffsets = nullptr,
		uint32 audioByteRate = 0, uint32 timelineStartMillis = 0, uint displayScale = 1,
		bool patchInterfacePalette = true, uint frameLimit = 0, int originY = 0,
		bool presentFinalFrameOnEsc = false, bool patchWacMediaPalette = false,
		bool serviceSceneUi = false, bool repeatedLoopPass = false,
		bool *advanceSegment = nullptr);
	bool playIavf(Common::SeekableReadStream &stream, const Common::String &name,
		bool allowEscSpace, bool serviceSceneUi = false);
	bool servicePlaybackInput(Video::SmackerDecoder &decoder, bool allowEscSpace,
		bool allowSegmentAdvance, bool &paused, bool toolbarPaused, bool &skipToEnd,
		bool &advanceSegment,
		Audio::SoundHandle *externalAudio, bool suppressSceneMouseStop);

	RipperEngine *_engine;
	InputManager *_input;
	Audio::Mixer *_mixer;
	AudioSlot _audioSlots[kAudioSlotCount];
	bool _stopSceneOnMouse;
};

} // End of namespace Ripper

#endif // RIPPER_MEDIA_H
