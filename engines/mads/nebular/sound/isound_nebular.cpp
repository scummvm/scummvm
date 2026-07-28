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

#include "mads/nebular/sound/isound_nebular.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

namespace {

const ISoundCommandSequence kSection1Commands[42] = {
	{ 0x0000, 0x00, false }, // 0: reset
	{ 0x0000, 0x00, false }, // 1: reset
	{ 0x0000, 0x00, false }, // 2: reset
	{ 0x0000, 0x00, false }, // 3: reset
	{ 0x0000, 0x00, false }, // 4: reset
	{ 0x0000, 0x00, false }, // 5: reset
	{ 0x0000, 0x00, false }, // 6: pause
	{ 0x0000, 0x00, false }, // 7: resume
	{ 0x0000, 0x00, false }, // 8: active duration
	{ 0x01c8, 0x40, false },
	{ 0x0000, 0x00, false }, // 10: no-op
	{ 0x0000, 0x00, false }, // 11: no-op
	{ 0x0000, 0x00, false }, // 12: no-op
	{ 0x0000, 0x00, false }, // 13: no-op
	{ 0x01dc, 0x40, false },
	{ 0x0000, 0x00, false }, // 15: no-op
	{ 0x0226, 0x40, false },
	{ 0x0236, 0x40, false },
	{ 0x0242, 0x40, false },
	{ 0x024e, 0x40, false },
	{ 0x025a, 0x40, false },
	{ 0x0266, 0x40, false },
	{ 0x0272, 0x40, false },
	{ 0x027e, 0x40, false },
	{ 0x028a, 0x40, false },
	{ 0x0296, 0x40, true  },
	{ 0x02a2, 0x40, true  },
	{ 0x02ae, 0x44, true  },
	{ 0x02ba, 0x44, true  },
	{ 0x02c6, 0x40, true  },
	{ 0x02d0, 0x40, true  },
	{ 0x02dc, 0x44, false },
	{ 0x02e8, 0x40, true  },
	{ 0x02f4, 0x40, false },
	{ 0x0300, 0x40, false },
	{ 0x030c, 0x40, false },
	{ 0x031a, 0x40, false },
	{ 0x0326, 0x40, false },
	{ 0x0332, 0x40, false },
	{ 0x033e, 0x40, false },
	{ 0x0340, 0x40, false },
	{ 0x034c, 0x40, false }
};

const ISoundCommandSequence kSection2Commands[44] = {
	{ 0x0000, 0x00, false }, // 0: reset
	{ 0x0000, 0x00, false }, // 1: reset
	{ 0x0000, 0x00, false }, // 2: reset
	{ 0x0000, 0x00, false }, // 3: reset
	{ 0x0000, 0x00, false }, // 4: reset
	{ 0x0000, 0x00, false }, // 5: reset
	{ 0x0000, 0x00, false }, // 6: pause
	{ 0x0000, 0x00, false }, // 7: resume
	{ 0x0000, 0x00, false }, // 8: active duration
	{ 0x0000, 0x00, false }, // 9: no-op
	{ 0x0000, 0x00, false }, // 10: no-op
	{ 0x0000, 0x00, false }, // 11: no-op
	{ 0x01ce, 0x40, false },
	{ 0x01da, 0x40, false },
	{ 0x01e6, 0x40, false },
	{ 0x0000, 0x00, false }, // 15: no-op
	{ 0x0000, 0x00, false }, // 16: no-op
	{ 0x0000, 0x00, false }, // 17: no-op
	{ 0x020c, 0x40, false },
	{ 0x0222, 0x40, false },
	{ 0x0238, 0x40, false },
	{ 0x0244, 0x40, false },
	{ 0x0274, 0x40, false },
	{ 0x0280, 0x40, false },
	{ 0x028c, 0x40, false },
	{ 0x0298, 0x40, false },
	{ 0x02a4, 0x40, false },
	{ 0x02b0, 0x40, false },
	{ 0x02bc, 0x40, false },
	{ 0x02cc, 0x40, false },
	{ 0x02d8, 0x40, false },
	{ 0x0334, 0x40, false },
	{ 0x0390, 0x40, false },
	{ 0x039c, 0x40, false },
	{ 0x03a8, 0x40, false },
	{ 0x03be, 0x40, false },
	{ 0x0000, 0x00, false }, // 36: no-op
	{ 0x03e4, 0x40, false },
	{ 0x03f0, 0x40, false },
	{ 0x0400, 0x40, false },
	{ 0x040c, 0x40, false },
	{ 0x0418, 0x40, false },
	{ 0x03d8, 0x40, false },
	{ 0x0424, 0x40, false }
};

const ISoundCommandSequence kSection3Commands[61] = {
	{ 0x0000, 0x00, false }, // 0: reset
	{ 0x0000, 0x00, false }, // 1: reset
	{ 0x0000, 0x00, false }, // 2: reset
	{ 0x0000, 0x00, false }, // 3: reset
	{ 0x0000, 0x00, false }, // 4: reset
	{ 0x0000, 0x00, false }, // 5: reset
	{ 0x0000, 0x00, false }, // 6: pause
	{ 0x0000, 0x00, false }, // 7: resume
	{ 0x0000, 0x00, false }, // 8: active duration
	{ 0x01c8, 0x40, false },
	{ 0x01ca, 0x40, false },
	{ 0x01cc, 0x40, false },
	{ 0x01ce, 0x40, false },
	{ 0x01d0, 0x40, false },
	{ 0x022c, 0x40, false },
	{ 0x0288, 0x40, false },
	{ 0x028a, 0x40, false },
	{ 0x028c, 0x40, false },
	{ 0x0000, 0x00, false }, // 18: no-op
	{ 0x0290, 0x40, false },
	{ 0x029c, 0x40, false },
	{ 0x02a8, 0x40, false },
	{ 0x02b8, 0x40, false },
	{ 0x02c2, 0x40, false },
	{ 0x02cc, 0x40, false },
	{ 0x02d8, 0x40, false },
	{ 0x02ee, 0x40, false },
	{ 0x0484, 0x40, false },
	{ 0x02fc, 0x40, false },
	{ 0x0308, 0x40, false },
	{ 0x0314, 0x40, false },
	{ 0x0370, 0x40, false },
	{ 0x037c, 0x40, false },
	{ 0x0388, 0x40, false },
	{ 0x0394, 0x40, false },
	{ 0x03aa, 0x40, false },
	{ 0x0000, 0x00, false }, // 36: no-op
	{ 0x03d0, 0x40, false },
	{ 0x03dc, 0x40, false },
	{ 0x03e8, 0x40, false },
	{ 0x03f4, 0x40, false },
	{ 0x0400, 0x40, false },
	{ 0x0410, 0x40, false },
	{ 0x041c, 0x40, false },
	{ 0x0428, 0x40, false },
	{ 0x0434, 0x40, false },
	{ 0x0440, 0x40, false },
	{ 0x0442, 0x40, false },
	{ 0x0444, 0x40, false },
	{ 0x0446, 0x40, false },
	{ 0x0448, 0x40, false },
	{ 0x044a, 0x40, false },
	{ 0x0456, 0x40, false },
	{ 0x0458, 0x40, false },
	{ 0x045a, 0x40, false },
	{ 0x045c, 0x40, false },
	{ 0x045e, 0x40, false },
	{ 0x0460, 0x40, false },
	{ 0x046c, 0x40, false },
	{ 0x03c4, 0x40, false },
	{ 0x0478, 0x40, false }
};

const ISoundCommandSequence kSection5Commands[42] = {
	{ 0x0000, 0x00, false }, // 0: reset
	{ 0x0000, 0x00, false }, // 1: reset
	{ 0x0000, 0x00, false }, // 2: reset
	{ 0x0000, 0x00, false }, // 3: reset
	{ 0x0000, 0x00, false }, // 4: reset
	{ 0x0000, 0x00, false }, // 5: reset
	{ 0x0000, 0x00, false }, // 6: pause
	{ 0x0000, 0x00, false }, // 7: resume
	{ 0x0000, 0x00, false }, // 8: active duration
	{ 0x01c8, 0x40, false },
	{ 0x01d4, 0x40, false },
	{ 0x01e0, 0x40, false },
	{ 0x01ec, 0x40, false },
	{ 0x01f8, 0x40, false },
	{ 0x0204, 0x40, false },
	{ 0x0214, 0x40, false },
	{ 0x0224, 0x40, false },
	{ 0x0234, 0x40, false },
	{ 0x0258, 0x40, false },
	{ 0x0268, 0x40, false },
	{ 0x0274, 0x40, false },
	{ 0x0280, 0x40, false },
	{ 0x028c, 0x40, false },
	{ 0x0298, 0x40, false },
	{ 0x02c8, 0x40, false },
	{ 0x02d4, 0x40, false },
	{ 0x02e0, 0x40, false },
	{ 0x02ec, 0x40, false },
	{ 0x02f8, 0x40, false },
	{ 0x0000, 0x00, false }, // 29: no-op
	{ 0x0306, 0x40, false },
	{ 0x0362, 0x40, false },
	{ 0x036e, 0x40, false },
	{ 0x037a, 0x40, false },
	{ 0x0386, 0x40, false },
	{ 0x039c, 0x40, false },
	{ 0x0000, 0x00, false }, // 36: no-op
	{ 0x03b8, 0x40, false },
	{ 0x0000, 0x00, false }, // 38: no-op
	{ 0x03c6, 0x40, false },
	{ 0x03d6, 0x40, false },
	{ 0x0000, 0x00, false }  // 41: no-op
};

const ISoundCommandSequence kSection6Commands[30] = {
	{ 0x0000, 0x00, false }, // 0: reset
	{ 0x0000, 0x00, false }, // 1: reset
	{ 0x0000, 0x00, false }, // 2: reset
	{ 0x0000, 0x00, false }, // 3: reset
	{ 0x0000, 0x00, false }, // 4: reset
	{ 0x0000, 0x00, false }, // 5: reset
	{ 0x0000, 0x00, false }, // 6: pause
	{ 0x0000, 0x00, false }, // 7: resume
	{ 0x0000, 0x00, false }, // 8: active duration
	{ 0x01c8, 0x40, false },
	{ 0x01d4, 0x40, false },
	{ 0x01e0, 0x40, false },
	{ 0x01f0, 0x40, false },
	{ 0x01fc, 0x40, false },
	{ 0x0208, 0x40, false },
	{ 0x0214, 0x40, false },
	{ 0x0220, 0x40, false },
	{ 0x022c, 0x40, false },
	{ 0x0238, 0x40, false },
	{ 0x0244, 0x40, false },
	{ 0x0250, 0x40, false },
	{ 0x0266, 0x40, false },
	{ 0x0276, 0x40, false },
	{ 0x0282, 0x40, false },
	{ 0x0000, 0x00, false }, // 24: no-op
	{ 0x0290, 0x40, false },
	{ 0x0000, 0x00, false }, // 26: no-op
	{ 0x0000, 0x00, false }, // 27: no-op
	{ 0x0000, 0x00, false }, // 28: no-op
	{ 0x0000, 0x00, false }  // 29: no-op
};

const ISoundCommandSequence kSection7Commands[38] = {
	{ 0x0000, 0x00, false }, // 0: reset
	{ 0x0000, 0x00, false }, // 1: reset
	{ 0x0000, 0x00, false }, // 2: reset
	{ 0x0000, 0x00, false }, // 3: reset
	{ 0x0000, 0x00, false }, // 4: reset
	{ 0x0000, 0x00, false }, // 5: reset
	{ 0x0000, 0x00, false }, // 6: pause
	{ 0x0000, 0x00, false }, // 7: resume
	{ 0x0000, 0x00, false }, // 8: active duration
	{ 0x024a, 0x40, false },
	{ 0x024a, 0x40, false },
	{ 0x024a, 0x40, false },
	{ 0x024a, 0x40, false },
	{ 0x024a, 0x40, false },
	{ 0x024a, 0x40, false },
	{ 0x01c8, 0x40, false },
	{ 0x01d4, 0x40, false },
	{ 0x01e0, 0x40, false },
	{ 0x01ec, 0x40, false },
	{ 0x01fc, 0x40, false },
	{ 0x020c, 0x40, false },
	{ 0x0220, 0x40, false },
	{ 0x022c, 0x40, false },
	{ 0x0238, 0x40, false },
	{ 0x0244, 0x40, false },
	{ 0x0246, 0x40, false },
	{ 0x0248, 0x40, false },
	{ 0x0304, 0x40, false },
	{ 0x0312, 0x40, false },
	{ 0x024e, 0x40, false },
	{ 0x0250, 0x40, false },
	{ 0x02ac, 0x40, false },
	{ 0x02ae, 0x40, false },
	{ 0x02ba, 0x40, false },
	{ 0x02c6, 0x40, false },
	{ 0x02dc, 0x40, false },
	{ 0x0000, 0x00, false }, // 36: no-op
	{ 0x02f8, 0x40, false }
};

const ISoundCommandSequence kSection8Commands[38] = {
	{ 0x0000, 0x00, false }, // 0: reset
	{ 0x0000, 0x00, false }, // 1: reset
	{ 0x0000, 0x00, false }, // 2: reset
	{ 0x0000, 0x00, false }, // 3: reset
	{ 0x0000, 0x00, false }, // 4: reset
	{ 0x0000, 0x00, false }, // 5: reset
	{ 0x0000, 0x00, false }, // 6: pause
	{ 0x0000, 0x00, false }, // 7: resume
	{ 0x0000, 0x00, false }, // 8: active duration
	{ 0x01c8, 0x40, false },
	{ 0x0000, 0x00, false }, // 10: no-op
	{ 0x01d6, 0x40, false },
	{ 0x01e2, 0x40, false },
	{ 0x01ee, 0x40, false },
	{ 0x01fa, 0x40, false },
	{ 0x020a, 0x40, false },
	{ 0x0216, 0x40, false },
	{ 0x0222, 0x40, false },
	{ 0x022e, 0x40, false },
	{ 0x023a, 0x40, false },
	{ 0x0246, 0x40, false },
	{ 0x0252, 0x40, false },
	{ 0x025e, 0x40, false },
	{ 0x026a, 0x40, false },
	{ 0x026c, 0x40, false },
	{ 0x0278, 0x40, false },
	{ 0x0290, 0x40, false },
	{ 0x029c, 0x40, false },
	{ 0x0000, 0x00, false }, // 28: no-op
	{ 0x0000, 0x00, false }, // 29: no-op
	{ 0x02ac, 0x40, false },
	{ 0x0360, 0x40, false },
	{ 0x030a, 0x40, false },
	{ 0x0316, 0x40, false },
	{ 0x0322, 0x40, false },
	{ 0x0338, 0x40, false },
	{ 0x0000, 0x00, false }, // 36: no-op
	{ 0x0354, 0x40, false }
};

const ISoundCommandSequence kSection9Commands[51] = {
	{ 0x0000, 0x00, false }, // 0: reset
	{ 0x0000, 0x00, false }, // 1: reset
	{ 0x0000, 0x00, false }, // 2: reset
	{ 0x0000, 0x00, false }, // 3: reset
	{ 0x0000, 0x00, false }, // 4: reset
	{ 0x0000, 0x00, false }, // 5: reset
	{ 0x0000, 0x00, false }, // 6: pause
	{ 0x0000, 0x00, false }, // 7: resume
	{ 0x0000, 0x00, false }, // 8: active duration
	{ 0x0000, 0x00, false }, // 9: no-op
	{ 0x0000, 0x00, false }, // 10: no-op
	{ 0x01cc, 0x40, false },
	{ 0x01d8, 0x40, false },
	{ 0x01e4, 0x40, false },
	{ 0x01f0, 0x40, false },
	{ 0x01fc, 0x40, false },
	{ 0x0210, 0x40, false },
	{ 0x0220, 0x40, false },
	{ 0x0230, 0x40, false },
	{ 0x0256, 0x40, false },
	{ 0x0274, 0x40, false },
	{ 0x0000, 0x00, false }, // 21: no-op
	{ 0x0000, 0x00, false }, // 22: no-op
	{ 0x02b6, 0x40, false },
	{ 0x02d2, 0x40, false },
	{ 0x02de, 0x40, false },
	{ 0x02ea, 0x40, false },
	{ 0x02f6, 0x40, false },
	{ 0x0302, 0x40, false },
	{ 0x030e, 0x40, false },
	{ 0x031a, 0x40, false },
	{ 0x0336, 0x40, false },
	{ 0x034c, 0x40, false },
	{ 0x0358, 0x40, false },
	{ 0x0000, 0x00, false }, // 34: no-op
	{ 0x036a, 0x40, false },
	{ 0x037e, 0x40, false },
	{ 0x038a, 0x40, false },
	{ 0x0000, 0x00, false }, // 38: no-op
	{ 0x0000, 0x00, false }, // 39: no-op
	{ 0x0000, 0x00, false }, // 40: no-op
	{ 0x0000, 0x00, false }, // 41: no-op
	{ 0x0000, 0x00, false }, // 42: no-op
	{ 0x0000, 0x00, false }, // 43: no-op
	{ 0x0000, 0x00, false }, // 44: no-op
	{ 0x0000, 0x00, false }, // 45: no-op
	{ 0x0000, 0x00, false }, // 46: no-op
	{ 0x0000, 0x00, false }, // 47: no-op
	{ 0x039a, 0x40, false },
	{ 0x0000, 0x00, false }, // 49: no-op
	{ 0x0000, 0x00, false }  // 50: no-op
};

} // namespace

ISoundSection::ISoundSection(Audio::Mixer *mixer, const char *filename,
		const ISoundCommandSequence *commands, uint commandCount) :
		ISound(mixer, filename),
	_commands(commands),
	_commandCount(commandCount) {
}

int ISoundSection::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	if (commandId < 0 || (uint)commandId >= _commandCount)
		return 0;

	beginCommand(param);
	if (commandId <= 8)
		return executeCommonCommand(commandId);

	const ISoundCommandSequence &entry = _commands[commandId];
	if (entry.parameterAtLeast120 && _commandParam < 0x78)
		return 0;

	if (entry.sequenceOffset)
		playSequence(entry.sequenceOffset, entry.priority);

	return 0;
}

ISound1::ISound1(Audio::Mixer *mixer) :
	ISoundSection(mixer, "ISOUND.001", kSection1Commands,
		ARRAYSIZE(kSection1Commands)) {
}

ISound2::ISound2(Audio::Mixer *mixer) :
	ISoundSection(mixer, "ISOUND.002", kSection2Commands,
		ARRAYSIZE(kSection2Commands)) {
}

ISound3::ISound3(Audio::Mixer *mixer) :
	ISound3(mixer, "ISOUND.003") {
}

ISound3::ISound3(Audio::Mixer *mixer, const char *filename) :
	ISoundSection(mixer, filename, kSection3Commands,
		ARRAYSIZE(kSection3Commands)) {
}

ISound4::ISound4(Audio::Mixer *mixer) :
	ISound3(mixer, "ISOUND.004") {
}

ISound5::ISound5(Audio::Mixer *mixer) :
	ISoundSection(mixer, "ISOUND.005", kSection5Commands,
		ARRAYSIZE(kSection5Commands)) {
}

ISound6::ISound6(Audio::Mixer *mixer) :
	ISoundSection(mixer, "ISOUND.006", kSection6Commands,
		ARRAYSIZE(kSection6Commands)) {
}

ISound7::ISound7(Audio::Mixer *mixer) :
	ISoundSection(mixer, "ISOUND.007", kSection7Commands,
		ARRAYSIZE(kSection7Commands)) {
}

ISound8::ISound8(Audio::Mixer *mixer) :
	ISoundSection(mixer, "ISOUND.008", kSection8Commands,
		ARRAYSIZE(kSection8Commands)) {
}

ISound9::ISound9(Audio::Mixer *mixer) :
	ISoundSection(mixer, "ISOUND.009", kSection9Commands,
		ARRAYSIZE(kSection9Commands)) {
}

} // namespace Sound
} // namespace RexNebular
} // namespace MADS
