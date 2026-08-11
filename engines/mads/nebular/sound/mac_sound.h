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

class MacSoundDriver : public SoundDriver {
private:
	MacResourceProvider *_resources;
	int _section;
	Common::Array<Audio::SoundHandle> _handles;
	int _volume = Audio::Mixer::kMaxChannelVolume;
	bool _paused = false;

	void setPaused(bool paused);
	void discardFinishedSounds();

public:
	MacSoundDriver(Audio::Mixer *mixer, MacResourceProvider *resources, int section);

	int command(int commandId, int param) override;
	int stop() override;
	int poll() override { return 0; }
	void noise() override {}
	void setVolume(int volume) override;
};

class MacSoundManager : public SoundManager {
private:
	MacResourceProvider *_resources;

protected:
	void loadDriver(int sectionNum) override;

public:
	MacSoundManager(Audio::Mixer *mixer, bool &soundFlag, MacResourceProvider *resources) :
			SoundManager(mixer, soundFlag), _resources(resources) {}

	void validate() override {}
};

} // namespace Sound
} // namespace RexNebular
} // namespace MADS

#endif
