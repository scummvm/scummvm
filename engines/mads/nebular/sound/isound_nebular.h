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
 *
 */

#ifndef MADS_NEBULAR_SOUND_ISOUND_NEBULAR_H
#define MADS_NEBULAR_SOUND_ISOUND_NEBULAR_H

#include "mads/nebular/sound/isound.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

struct ISoundCommandSequence {
	uint16 sequenceOffset;
	byte priority;
	bool parameterAtLeast120;
};

class ISoundSection : public ISound {
private:
	const ISoundCommandSequence *_commands;
	uint _commandCount;

protected:
	ISoundSection(Audio::Mixer *mixer, const char *filename,
		const ISoundCommandSequence *commands, uint commandCount);

public:
	int command(int commandId, int param) override;
};

/** ISOUND.001: "Rex IBM Mod1 9-11-92", command IDs 0-41. */
class ISound1 : public ISoundSection {
public:
	ISound1(Audio::Mixer *mixer);
};

/** ISOUND.002: "Rex IBM Mod2 9-11-92", command IDs 0-43. */
class ISound2 : public ISoundSection {
public:
	ISound2(Audio::Mixer *mixer);
};

/** ISOUND.003: "Rex IBM Mod3 9-11-92", command IDs 0-60. */
class ISound3 : public ISoundSection {
protected:
	ISound3(Audio::Mixer *mixer, const char *filename);

public:
	ISound3(Audio::Mixer *mixer);
};

/** ISOUND.004 has the same command and stream layout as ISOUND.003. */
class ISound4 : public ISound3 {
public:
	ISound4(Audio::Mixer *mixer);
};

/** ISOUND.005: "Rex IBM Mod5 9-11-92", command IDs 0-41. */
class ISound5 : public ISoundSection {
public:
	ISound5(Audio::Mixer *mixer);
};

/** ISOUND.006: "Rex IBM Mod6 9-11-92", command IDs 0-29. */
class ISound6 : public ISoundSection {
public:
	ISound6(Audio::Mixer *mixer);
};

/** ISOUND.007: "Rex IBM Mod7 9-11-92", command IDs 0-37. */
class ISound7 : public ISoundSection {
public:
	ISound7(Audio::Mixer *mixer);
};

/** ISOUND.008: "Rex IBM Mod8 9-11-92", command IDs 0-37. */
class ISound8 : public ISoundSection {
public:
	ISound8(Audio::Mixer *mixer);
};

/** ISOUND.009: "Rex IBM Mod9 9-11-92", command IDs 0-50. */
class ISound9 : public ISoundSection {
public:
	ISound9(Audio::Mixer *mixer);
};

} // namespace Sound
} // namespace RexNebular
} // namespace MADS

#endif
