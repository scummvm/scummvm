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

#ifndef MADS_CORE_SOUND_MANAGER_H
#define MADS_CORE_SOUND_MANAGER_H

#include "common/array.h"
#include "common/memstream.h"
#include "common/mutex.h"
#include "common/queue.h"

namespace Audio {
class Mixer;
}

namespace MADS {

#define CALLBACKS_PER_SECOND 60

class SoundDriver {
protected:
	Audio::Mixer *_mixer;
	Common::Array<byte> _soundData;
	Common::Mutex _driverMutex;

	/**
	 * Gets a stream starting at a given offset in the loaded sound data
	 */
	Common::MemoryReadStream getDataStream(int offset) const {
		return Common::MemoryReadStream(&_soundData[offset], _soundData.size() - offset);
	}

	explicit SoundDriver(Audio::Mixer *mixer) : _mixer(mixer) {}

public:
	/**
	 * Loads a driver data block from an absolute file offset.
	 *
	 * For DOS MZ overlays, dataOffset includes the executable header. Offsets
	 * passed to getDataStream() and stored in sequence data are relative to
	 * the loaded block instead.
	 */
	SoundDriver(Audio::Mixer *mixer, const Common::Path &filename,
		int dataOffset, int dataSize);
	virtual ~SoundDriver() {}

	/**
	 * Execute a player command. Most commands represent sounds to play, but some
	 * low number commands also provide control operations.
	 * @param commandId		Player ommand to execute.
	 * @param param			Optional parameter used by a few commands
	 */
	virtual int command(int commandId, int param) = 0;

	/**
	 * Stop all currently playing sounds
	 */
	virtual int stop() = 0;

	/**
	 * Main poll method to allow sounds to progress
	 */
	virtual int poll() = 0;

	/**
	 * General noise/note output
	 */
	virtual void noise() = 0;

	/**
	 * Set the volume
	 */
	virtual void setVolume(int volume) = 0;
};

class SoundManager {
protected:
	struct QueuedCommand {
		int _commandId;
		int _param;
	};

	enum DriverType { SOUND_ADLIB, SOUND_MT32, SOUND_GM, SOUND_PCSPEAKER, SOUND_PAS };
	Audio::Mixer *_mixer;
	DriverType _driverType;
	bool &_soundFlag;
	SoundDriver *_driver = nullptr;
	bool _newSoundsPaused = false;
	Common::Queue<QueuedCommand> _queuedCommands;
	int _masterVolume = 255;

protected:
	/**
	 * Load the particular section sound handler
	 * @param sectionNum	Section number
	 */
	virtual void loadDriver(int sectionNum) = 0;

public:
	SoundManager(Audio::Mixer *mixer, bool &soundFlag,
			bool supportsGeneralMidi = false);
	virtual ~SoundManager();

	/**
	 * Validate the sound driver files needed for data
	 */
	virtual void validate() = 0;

	/**
	 * Initializes the sound driver for a given game section
	 */
	void init(int sectionNumber);

	/**
	 * Returns true if a driver is loaded
	 */
	bool isLoaded() const {
		return _driver != nullptr;
	}

	/**
	 * Returns whether the current driver reports an active sound.
	 */
	bool isDriverActive();

	/**
	 * Returns whether the selected driver family uses PC speaker data.
	 */
	bool usesPCSpeaker() const {
		return _driverType == SOUND_PCSPEAKER;
	}

	/**
	 * Stop any currently active sound and remove the driver
	 */
	void closeDriver();

	/**
	 * Remove the driver
	 */
	void removeDriver();

	/**
	 * Temporarily pause the playback of any new sound commands
	 */
	void pauseNewCommands();

	/**
	 * Stop queueing sound commands, and execute any previously queued ones
	 */
	void startQueuedCommands();

	/**
	 * Set the master volume
	 */
	void setVolume(int volume);

	//@{
	/**
	 * Executes a command on the sound driver
	 * @param commandid		Command Id to execute
	 * @param param			Optional paramater specific to a few commands
	 */
	int command(int commandId, int param = 0);

	/**
	 * Stops any currently playing sound
	 */
	void stop();

	/**
	 * Noise
	 * Some sort of random noise generation?
	 */
	void noise();

	//@}
};

} // namespace MADS

#endif
