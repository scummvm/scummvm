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
 */

#include "common/util.h"
#include "mads/phantom/sound/isound_phantom.h"

namespace MADS {
namespace Phantom {
namespace Sound {

namespace {

const ISound::OverlaySpec kEarlyLayout = { 0x00f4, 0x00ce, 0x00dc };
const ISound::OverlaySpec kLateLayout = { 0x00e0, 0x00ba, 0x00c8 };

struct ISoundSectionDefinition {
	int sectionNumber;
	const char *filename;
	const ISound::OverlaySpec *layout;
};

const ISoundSectionDefinition kSectionDefinitions[] = {
	{ 1, "ISOUND.PH1", &kEarlyLayout },
	{ 2, "ISOUND.PH2", &kEarlyLayout },
	{ 3, "ISOUND.PH3", &kLateLayout },
	{ 4, "ISOUND.PH4", &kLateLayout },
	{ 5, "ISOUND.PH5", &kLateLayout },
	{ 9, "ISOUND.PH9", &kEarlyLayout }
};

struct ISoundCommandSequence {
	byte commandId;
	uint16 sequenceOffset;
	byte priority;
};

struct ISoundCommandTable {
	int sectionNumber;
	const ISoundCommandSequence *commands;
	uint commandCount;
	const byte *malformedCommands;
	uint malformedCommandCount;
};

const ISoundCommandSequence kSection1Commands[] = {
	{ 24, 0x01d0, 0 }, { 25, 0x0201, 2 }, { 26, 0x022b, 4 },
	{ 27, 0x0235, 6 }, { 64, 0x0240, 0 }, { 65, 0x0256, 0 },
	{ 66, 0x0269, 0 }, { 67, 0x027e, 0 }, { 69, 0x0288, 0 },
	{ 70, 0x0292, 0 }, { 73, 0x02a3, 0 }, { 74, 0x02d4, 0 },
	{ 75, 0x02ba, 0 }
};

const ISoundCommandSequence kSection2Commands[] = {
	{ 24, 0x01d0, 0 }, { 25, 0x0201, 2 }, { 26, 0x022b, 4 },
	{ 27, 0x0235, 6 }, { 36, 0x0240, 0 }, { 37, 0x0255, 0 },
	{ 38, 0x025f, 0 }, { 39, 0x026f, 0 }, { 64, 0x0240, 0 },
	{ 65, 0x0255, 0 }, { 66, 0x025f, 0 }, { 67, 0x026f, 0 },
	{ 70, 0x0279, 0 }, { 71, 0x0283, 0 }, { 72, 0x0291, 0 }
};
const byte kSection2MalformedCommands[] = { 76 };

const ISoundCommandSequence kSection3Commands[] = {
	{ 24, 0x01d0, 0 }, { 25, 0x0201, 2 }, { 26, 0x022b, 4 },
	{ 27, 0x0235, 6 }, { 34, 0x02b2, 0 }, { 37, 0x02b2, 0 },
	{ 38, 0x0240, 0 }, { 39, 0x0272, 0 }, { 64, 0x0240, 0 },
	{ 65, 0x0272, 0 }, { 66, 0x027c, 0 }, { 67, 0x02a3, 0 },
	{ 68, 0x02c0, 0 }, { 69, 0x02ce, 0 }, { 72, 0x02e6, 0 },
	{ 73, 0x02dc, 0 }, { 74, 0x02f5, 0 }, { 75, 0x02ff, 0 }
};

const ISoundCommandSequence kSection4Commands[] = {
	{ 24, 0x0240, 0 }, { 25, 0x0240, 0 }, { 26, 0x022b, 4 },
	{ 27, 0x0235, 6 }, { 32, 0x0250, 0 }, { 33, 0x025e, 0 },
	{ 34, 0x026e, 0 }, { 35, 0x0277, 0 }, { 36, 0x0281, 0 },
	{ 37, 0x028b, 0 }, { 38, 0x0295, 0 }, { 64, 0x0250, 0 },
	{ 65, 0x025e, 0 }, { 66, 0x026e, 0 }, { 67, 0x0277, 0 },
	{ 68, 0x0281, 0 }, { 69, 0x028b, 0 }, { 70, 0x0295, 0 }
};
const byte kSection4MalformedCommands[] = { 74, 75, 76 };

const ISoundCommandSequence kSection5Commands[] = {
	{ 24, 0x01d0, 0 }, { 25, 0x0201, 2 }, { 26, 0x022b, 4 },
	{ 27, 0x0235, 6 }, { 64, 0x02c4, 0 }, { 65, 0x0240, 0 },
	{ 66, 0x0242, 0 }, { 67, 0x024c, 0 }, { 68, 0x0256, 0 },
	{ 69, 0x026c, 0 }, { 70, 0x02a0, 0 }, { 71, 0x02ae, 0 },
	{ 72, 0x02b8, 0 }, { 73, 0x02c2, 0 }, { 74, 0x02d3, 0 },
	{ 78, 0x02e4, 0 }
};

const ISoundCommandSequence kSection9Commands[] = {
	{ 24, 0x01d0, 0 }, { 25, 0x0201, 2 }, { 26, 0x022b, 4 },
	{ 27, 0x0235, 6 }, { 64, 0x0240, 0 }, { 65, 0x024e, 2 },
	{ 67, 0x0258, 6 }, { 68, 0x027f, 8 }, { 70, 0x0289, 12 },
	{ 71, 0x02a5, 14 }
};

const ISoundCommandTable kCommandTables[] = {
	{ 1, kSection1Commands, ARRAYSIZE(kSection1Commands), nullptr, 0 },
	{ 2, kSection2Commands, ARRAYSIZE(kSection2Commands),
		kSection2MalformedCommands, ARRAYSIZE(kSection2MalformedCommands) },
	{ 3, kSection3Commands, ARRAYSIZE(kSection3Commands), nullptr, 0 },
	{ 4, kSection4Commands, ARRAYSIZE(kSection4Commands),
		kSection4MalformedCommands, ARRAYSIZE(kSection4MalformedCommands) },
	{ 5, kSection5Commands, ARRAYSIZE(kSection5Commands), nullptr, 0 },
	{ 9, kSection9Commands, ARRAYSIZE(kSection9Commands), nullptr, 0 }
};

const ISoundCommandTable *findCommandTable(int sectionNumber) {
	for (uint index = 0; index < ARRAYSIZE(kCommandTables); ++index) {
		if (kCommandTables[index].sectionNumber == sectionNumber)
			return &kCommandTables[index];
	}
	return nullptr;
}

const ISoundSectionDefinition *findSectionDefinition(int sectionNumber) {
	for (uint index = 0; index < ARRAYSIZE(kSectionDefinitions); ++index) {
		if (kSectionDefinitions[index].sectionNumber == sectionNumber)
			return &kSectionDefinitions[index];
	}
	return nullptr;
}

const ISoundSectionDefinition &getSectionDefinition(int sectionNumber) {
	const ISoundSectionDefinition *definition = findSectionDefinition(sectionNumber);
	assert(definition);
	return *definition;
}

} // namespace

ISoundSection::CommandDisposition ISoundSection::lookupCommand(
		int sectionNumber, byte commandId, uint16 &sequenceOffset,
		byte &priority) {
	const ISoundCommandTable *table = findCommandTable(sectionNumber);
	if (table) {
		for (uint index = 0; index < table->commandCount; ++index) {
			if (table->commands[index].commandId == commandId) {
				sequenceOffset = table->commands[index].sequenceOffset;
				priority = table->commands[index].priority;
				return kCommandPlaySequence;
			}
		}
		for (uint index = 0; index < table->malformedCommandCount; ++index) {
			if (table->malformedCommands[index] == commandId)
				return kCommandMalformed;
		}
	}
	return kCommandUnhandled;
}

bool ISoundSection::validateSectionLayout(int sectionNumber,
		const OverlayLayout &layout, Common::String *reason) {
	const ISoundCommandTable *table = findCommandTable(sectionNumber);
	if (!table) {
		if (reason)
			*reason = "unsupported Phantom ISOUND section";
		return false;
	}

	const bool lateLayout = sectionNumber >= 3 && sectionNumber <= 5;
	const uint16 noteTableOffset = lateLayout ? 0x00e0 : 0x00f4;
	const uint32 noteTableEnd = noteTableOffset + (0x00ba + 1) * 2U;
	if (noteTableEnd > layout.initializedDataSize) {
		if (reason)
			*reason = "overlay data does not contain the selected section layout";
		return false;
	}

	for (uint index = 0; index < table->commandCount; ++index) {
		if ((uint32)table->commands[index].sequenceOffset + 1 >=
				layout.initializedDataSize) {
			if (reason)
				*reason = "overlay data does not contain a mapped command stream";
			return false;
		}
	}
	return true;
}

ISoundSection::ISoundSection(Audio::Mixer *mixer, int sectionNumber) :
	ISound(mixer, getSectionDefinition(sectionNumber).filename,
		*getSectionDefinition(sectionNumber).layout),
	_sectionNumber(sectionNumber) {
}

bool ISoundSection::isOverlaySupported(int sectionNumber,
		Common::String *reason) {
	const ISoundSectionDefinition *definition = findSectionDefinition(sectionNumber);
	if (!definition) {
		if (reason)
			*reason = "unsupported Phantom ISOUND section";
		return false;
	}

	OverlayLayout layout;
	if (!readOverlayLayout(definition->filename, layout, reason))
		return false;
	return validateSectionLayout(sectionNumber, layout, reason);
}

int ISoundSection::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	(void)param;
	if (commandId >= 0 && commandId <= 8)
		return executeCommonCommand(commandId);

	uint16 sequenceOffset = 0;
	byte priority = 0;
	switch (lookupCommand(_sectionNumber, commandId, sequenceOffset, priority)) {
	case kCommandPlaySequence:
		playSequence(sequenceOffset, priority);
		break;
	case kCommandMalformed:
		warning("Phantom ISOUND ignored command %d with a malformed native handler",
			commandId);
		break;
	default:
		break;
	}
	return 0;
}

ISound1::ISound1(Audio::Mixer *mixer) :
	ISoundSection(mixer, 1) {
}

ISound2::ISound2(Audio::Mixer *mixer) :
	ISoundSection(mixer, 2) {
}

ISound3::ISound3(Audio::Mixer *mixer) :
	ISoundSection(mixer, 3) {
}

ISound4::ISound4(Audio::Mixer *mixer) :
	ISoundSection(mixer, 4) {
}

ISound5::ISound5(Audio::Mixer *mixer) :
	ISoundSection(mixer, 5) {
}

ISound9::ISound9(Audio::Mixer *mixer) :
	ISoundSection(mixer, 9) {
}

} // namespace Sound
} // namespace Phantom
} // namespace MADS
