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

#ifndef RIPPER_SCENE_AUDIO_H
#define RIPPER_SCENE_AUDIO_H

#include "audio/mixer.h"
#include "common/noncopyable.h"
#include "common/ptr.h"
#include "common/str.h"

namespace Audio {
class AudioStream;
}

namespace Common {
class Serializer;
class SeekableReadStream;
}

namespace Ripper {

class RipperEngine;

class SceneAudioManager {
	struct Slot {
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

		Slot();
	};

	static const uint kSlotCount = 20;

public:
	SceneAudioManager(RipperEngine *engine, Audio::Mixer *mixer);
	~SceneAudioManager();

	bool load(const Common::String &path, bool preserve);
	bool configure(const Common::String &key, uint volumePercent, uint triggerFrame,
		byte control);
	void clear(const Common::String &key);
	void stop(const Common::String &key);
	void setVolume(const Common::String &key, uint targetVolumePercent,
		uint startFrame, uint timing);
	void service(uint frame);
	void resetTriggers();
	void clearAll(bool includePreserved);
	bool isActive() const;

	struct Snapshot : private Common::NonCopyable {
		Snapshot();
		~Snapshot();

		Slot slots[kSlotCount];
		bool active[kSlotCount];
		Common::ScopedPtr<Audio::AudioStream> streams[kSlotCount];
	};

	void captureSnapshot(Snapshot &state) const;
	bool restoreSnapshot(Snapshot &state);
	bool syncGame(Common::Serializer &serializer);
	bool syncGame(Common::Serializer &serializer, Snapshot &state);
	void applySnapshot(Snapshot &state);

private:
	static Common::String keyFromPath(const Common::String &path);
	Slot *find(const Common::String &key);
	const Slot *find(const Common::String &key) const;
	Audio::AudioStream *prepareStream(const Slot &slot) const;
	bool prepareSnapshot(Snapshot &state) const;
	bool start(Slot &slot);
	void start(Slot &slot, Audio::AudioStream *stream);
	void clearSlot(Slot &slot);
	void applyVolume(Slot &slot);
	Common::String describeSlots() const;

	RipperEngine *_engine;
	Audio::Mixer *_mixer;
	Slot _slots[kSlotCount];
};

} // End of namespace Ripper

#endif // RIPPER_SCENE_AUDIO_H
