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

#ifndef MADS_DRAGONSPHERE_SOUND_GSOUND_DRAGONSPHERE_H
#define MADS_DRAGONSPHERE_SOUND_GSOUND_DRAGONSPHERE_H

#include "mads/dragonsphere/sound/gsound.h"

namespace MADS {
namespace Dragonsphere {
namespace Sound {

/**
 * Dragonsphere-specific GSOUND controllers.
 *
 * Their tables are tied to validated retail overlays, and no table may be
 * reused by another MADS game merely because its driver has the same name.
 */
class GSoundDragonsphere : public GSound {
private:
	const GSoundCommandSpec *_commandSpecs;
	uint _commandSpecCount;
	byte _section;

	bool validCommand(int command) const;
	bool runDeferredMusic(int internalCommand, uint16 guard,
			uint16 counter, uint16 period, const GSoundChannelRoot *roots,
			uint rootCount);

protected:
	GSoundDragonsphere(Audio::Mixer *mixer,
			const GSoundDriverData &driverData,
			const GSoundCommandSpec *commandSpecs, uint commandSpecCount);

	const GSoundCommandSpec *findCommandSpec(int command) const override;
	bool executeSpecialCommand(int command, bool fromDeferred) override;
	bool executeNativeCallback(uint16 targetOffset,
			GSoundChannel &channel) override;

public:
	int command(int commandId, int param) override;
};

class GSound1 : public GSoundDragonsphere {
public:
	explicit GSound1(Audio::Mixer *mixer);
};

class GSound2 : public GSoundDragonsphere {
public:
	explicit GSound2(Audio::Mixer *mixer);
};

class GSound3 : public GSoundDragonsphere {
public:
	explicit GSound3(Audio::Mixer *mixer);
};

class GSound4 : public GSoundDragonsphere {
public:
	explicit GSound4(Audio::Mixer *mixer);
};

class GSound5 : public GSoundDragonsphere {
public:
	explicit GSound5(Audio::Mixer *mixer);
};

class GSound6 : public GSoundDragonsphere {
public:
	explicit GSound6(Audio::Mixer *mixer);
};

class GSound9 : public GSoundDragonsphere {
public:
	explicit GSound9(Audio::Mixer *mixer);
};

bool validateDragonsphereGSoundFiles();

} // namespace Sound
} // namespace Dragonsphere
} // namespace MADS

#endif // MADS_DRAGONSPHERE_SOUND_GSOUND_DRAGONSPHERE_H
