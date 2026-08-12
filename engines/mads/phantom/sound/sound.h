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

#ifndef MADS_PHANTOM_SOUND_SOUND_H
#define MADS_PHANTOM_SOUND_SOUND_H

#include "mads/core/sound_manager.h"

namespace MADS {
namespace Phantom {
namespace Sound {

class PhantomSoundManager : public SoundManager {
private:
	bool _isDemo;

protected:
	/**
	 * Load the appropriate sound driver for the current section of the game.
	 */
	void loadDriver(int sectionNum) override;

public:
	PhantomSoundManager(Audio::Mixer *mixer, bool &soundFlag, bool usePas,
			bool isDemo);
	~PhantomSoundManager() override {}

	/**
	 * Validate the sound driver files needed for data
	 */
	void validate() override;
};

} // namespace Sound
} // namespace Phantom
} // namespace MADS

#endif
