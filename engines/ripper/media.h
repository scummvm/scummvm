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

	bool play(const Common::String &path, bool allowEscSpace, int x = -1, int y = -1);
	bool playScene(const Common::String &path, int x, int y, bool firstFrameOnly,
		bool loopUntilInput = false);
	bool loadAudio(const Common::String &path);
	bool startLoadedAudio(const Common::String &key, uint volumePercent, bool loop);
	bool isSceneAudioActive() const;

private:
	bool playSmacker(Common::SeekableReadStream *stream, const Common::String &name,
		bool allowEscSpace, int x, int y, Audio::SoundHandle *externalAudio = nullptr,
		bool *stoppedByUser = nullptr, const Common::Array<uint32> *frameAudioOffsets = nullptr,
		uint32 audioByteRate = 0, uint32 timelineStartMillis = 0, uint displayScale = 1,
		bool patchInterfacePalette = true, uint frameLimit = 0, int originY = 0);
	bool playIavf(Common::SeekableReadStream &stream, const Common::String &name, bool allowEscSpace);
	bool servicePlaybackInput(Video::SmackerDecoder &decoder, bool allowEscSpace,
		bool &paused, Audio::SoundHandle *externalAudio);

	RipperEngine *_engine;
	InputManager *_input;
	Audio::Mixer *_mixer;
	Common::String _loadedAudioPath;
	Common::String _loadedAudioKey;
	Audio::SoundHandle _sceneAudioHandle;
	bool _stopSceneOnMouse;
};

} // End of namespace Ripper

#endif // RIPPER_MEDIA_H
