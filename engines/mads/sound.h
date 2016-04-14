/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MADS_SOUND_H
#define MADS_SOUND_H

#include "common/scummsys.h"
#include "common/queue.h"

namespace Audio {
class Mixer;
}

namespace OPL {
class OPL;
}

namespace MADS {

namespace Nebular {
class ASound;
}

class MADSEngine;

class SoundManager {
private:
	MADSEngine *_vm;
	Audio::Mixer *_mixer;
	OPL::OPL *_opl;
	Nebular::ASound *_driver;
	bool _pollSoundEnabled;
	bool _soundPollFlag;
	bool _newSoundsPaused;
	Common::Queue<int> _queuedCommands;
	int _masterVolume;
public:
	SoundManager(MADSEngine *vm, Audio::Mixer *mixer);
	~SoundManager();

	bool _preferRoland;

	/**
	 * Initializes the sound driver for a given game section
	 */
	void init(int sectionNumber);

	/**
	 * Stop any currently active sound and remove the driver
	 */
	void closeDriver();

	/**
	 * Remove the driver
	 */
	void removeDriver();

	/**
	 * Sets the enabled status of the sound
	 * @flag		True if sound should be enabled
	 */
	void setEnabled(bool flag);

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
	void command(int commandId, int param = 0);

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

} // End of namespace MADS

#endif /* MADS_SOUND_H */
