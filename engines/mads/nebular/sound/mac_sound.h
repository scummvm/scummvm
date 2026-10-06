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

#ifndef MADS_NEBULAR_SOUND_MAC_SOUND_H
#define MADS_NEBULAR_SOUND_MAC_SOUND_H

#include "audio/mixer.h"
#include "mads/core/sound_manager.h"

namespace MADS {
namespace RexNebular {

class MacResourceProvider;

namespace Sound {

enum MacSoundOperation {
	kMacSoundPlay = 1,
	kMacSoundPlayRepeated = 2,
	kMacSoundChangeVolume = 3,
	kMacSoundPlayPriority = 4,
	kMacSoundPlayRepeatedPriority = 5,
	kMacSoundPlayPriorityVolume = 6,
	kMacSoundPlayRepeatedPriorityVolume = 7,
	kMacSoundPlayVolume = 8,
	kMacSoundPlayRepeatedVolume = 9,
	kMacSoundWait = 10,
	kMacSoundHalt = 11,
	kMacSoundHaltAll = 12,
	kMacSoundRampDown = 13
};

struct MacSoundCommand {
	MacSoundOperation operation;
	int resourceID;
	int argument1;
	int argument2;
	int argument3;
};

struct MacSoundVoice {
	Audio::SoundHandle handle;
	int resourceID;
	int volume;
	bool priority;
	bool delayed;
	uint32 delayEndTick;
	MacSoundCommand delayedCommand;

	MacSoundVoice();
};

struct MacSoundQueueEntry {
	bool used;
	MacSoundCommand command;

	MacSoundQueueEntry();
};

class MacSoundDriver : public SoundDriver {
private:
	MacResourceProvider *_resources;
	int _section;
	MacSoundVoice _voices[2];
	MacSoundQueueEntry _queue[4];
	int _volume;
	bool _paused;
	bool _ramping;
	int _rampVolume;
	uint32 _serviceTick;
	uint32 _pausedAtTick;

	void setPaused(bool paused);
	void updateVoice(MacSoundVoice &voice);
	MacSoundVoice *selectVoice();
	int effectiveVolume(int volume) const;
	void setVoiceVolume(MacSoundVoice &voice, int volume);
	void play(const MacSoundCommand &command);
	void wait(const MacSoundCommand &command);
	void halt(int resourceID);
	void enqueue(const MacSoundCommand &command);
	void execute(const MacSoundCommand &command);
	MacSoundCommand mapCommand(int commandId, int param) const;

public:
	MacSoundDriver(Audio::Mixer *mixer, MacResourceProvider *resources, int section);

	int command(int commandId, int param) override;
	int stop() override;
	int poll() override;
	void noise() override {}
	void setVolume(int volume) override;
	void commandMacintosh(const MacSoundCommand &command);
	void queueMacintosh(const MacSoundCommand &command);
	void dispatchQueue();
	void service(uint32 tick);
};

class MacSoundManager : public SoundManager {
private:
	MacResourceProvider *_resources;

protected:
	void loadDriver(int sectionNum) override;
	MacSoundDriver *getMacintoshDriver() const;

public:
	MacSoundManager(Audio::Mixer *mixer, bool &soundFlag, MacResourceProvider *resources) :
			SoundManager(mixer, soundFlag), _resources(resources) {}

	void validate() override {}
	void startQueuedCommands() override;
	void commandMacintosh(const MacSoundCommand &cmd, bool queued);
	void service(uint32 tick);
};

bool commandMacintoshSound(const MacSoundCommand &cmd, bool queued = false);
bool commandMacintoshSound(MacSoundOperation operation, int resourceID,
	int argument1 = 0, int argument2 = 0, int argument3 = 0,
	bool queued = false);

} // namespace Sound
} // namespace RexNebular
} // namespace MADS

#endif
