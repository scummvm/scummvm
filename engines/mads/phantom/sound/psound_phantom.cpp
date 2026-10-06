/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT file.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "common/textconsole.h"
#include "common/util.h"
#include "mads/phantom/sound/psound_phantom.h"

namespace MADS {
namespace Phantom {
namespace Sound {

namespace {

const PSoundDriverData kPSound1Data = {
	"PSOUND.PH1", 0x3000, 0x4486, 0x4880, 0x0058, 0x3bfe,
	0x3c06, 34, { 0x00a4, 0x0124, 0x0140, 0x014a, 0x0154 }
};

const PSoundDriverData kPSound2Data = {
	"PSOUND.PH2", 0x2dd0, 0x1c7a, 0x2070, 0x0058, 0x12ec,
	0x147a, 32, { 0x00a4, 0x0124, 0x0140, 0x014a, 0x0154 }
};

const PSoundDriverData kPSound3Data = {
	"PSOUND.PH3", 0x2e50, 0x357a, 0x3970, 0x0058, 0x05ce,
	0x2d7a, 32, { 0x00a4, 0x0124, 0x0140, 0x014a, 0x0154 }
};

const PSoundDriverData kPSound4Data = {
	"PSOUND.PH4", 0x2d20, 0x0a90, 0x0e80, 0x0058, 0x0484,
	0x0510, 22, { 0x00a4, 0x0124, 0x0140, 0x014a, 0x0154 }
};

const PSoundDriverData kPSound5Data = {
	"PSOUND.PH5", 0x2ee0, 0x4eb6, 0x52b0, 0x0058, 0x436c,
	0x4476, 41, { 0x00a4, 0x0124, 0x0140, 0x014a, 0x0154 }
};

const PSoundDriverData kPSound9Data = {
	"PSOUND.PH9", 0x2e50, 0x3016, 0x3410, 0x0058, 0x254c,
	0x2616, 40, { 0x00a4, 0x0124, 0x0140, 0x014a, 0x0154 }
};

const PSoundDriverData kPSoundDemoData = {
	"PSOUND.PHA", 0x2d80, 0x4508, 0x4900, 0x0058, 0x2501,
	0x2508, 128, { 0x009c, 0x011c, 0x0138, 0x0142, 0x014c }
};

struct ValidationEntry {
	const PSoundDriverData *driverData;
	const char *first8192Md5;
};

const ValidationEntry kRetailValidation[] = {
	{ &kPSound1Data, "b314aadd7e0ae7b857cf283740e2e1cc" },
	{ &kPSound2Data, "1de247af9adaa72117c8b4dcb8a72bf9" },
	{ &kPSound3Data, "8747dfc90c45fbab58cb5ec1710e7c37" },
	{ &kPSound4Data, "69f8f318d491e90e5c063708a2f82db4" },
	{ &kPSound5Data, "3d846fc65dece14b29b0495fb5341284" },
	{ &kPSound9Data, "f7d7e85759526098168180d89d5723ae" }
};

const ValidationEntry kDemoValidation = {
	&kPSoundDemoData, "190d286ea8b7227e1ff1a7cd3e980bd3"
};

const ValidationEntry *retailValidationForSection(int section) {
	switch (section) {
	case 1:
		return &kRetailValidation[0];
	case 2:
		return &kRetailValidation[1];
	case 3:
		return &kRetailValidation[2];
	case 4:
		return &kRetailValidation[3];
	case 5:
		return &kRetailValidation[4];
	case 9:
		return &kRetailValidation[5];
	default:
		return nullptr;
	}
}

} // namespace

PhantomPSound::PhantomPSound(Audio::Mixer *mixer,
		const PSoundDriverData &driverData) :
		PSound(mixer, driverData) {
}

int PhantomPSound::dispatchBaseCommand(int commandId) {
	switch (commandId) {
	case 0:
		return command0();
	case 1:
		return command1();
	case 2:
		return command2();
	case 3:
		return command3();
	case 4:
		return command4();
	case 5:
		return command5();
	case 6:
		return command6();
	case 7:
		return command7();
	case 8:
		return command8();
	default:
		return 0;
	}
}

void PhantomPSound::loadFixedChannels(const uint16 *sequences, uint count) {
	assert(count <= kChannelCount);
	for (uint i = 0; i < count; ++i)
		loadChannel(i, sequences[i]);
}

void PhantomPSound::playSounds(const uint16 *sequences, uint count) {
	for (uint i = 0; i < count; ++i)
		playSound(sequences[i]);
}

void PhantomPSound::playMusic(const uint16 *sequences, uint count) {
	for (uint i = 0; i < count; ++i)
		playMusicAny(sequences[i]);
}

bool PhantomPSound::loadFixedIfInactive(uint16 guard,
										const uint16 *sequences, uint count) {
	if (isSoundActive(guard))
		return false;
	loadFixedChannels(sequences, count);
	return true;
}

void PhantomPSound::leaveMalformedChannelSilent(const char *filename,
												byte channel, uint16 sequenceOffset) {
	// These immediates are not MZ-relocated and the original loader performs
	// no bounds check. Reinterpreting them would invent a stream; leave only
	// the affected channel silent while preserving every valid native root.
	if (isDataRangeValid(sequenceOffset, 1))
		error("%s malformed root 0x%04x unexpectedly resolves inside data",
			  filename, sequenceOffset);
	finishChannel(channel);
	warning("%s contains malformed native sequence root 0x%04x for channel %u; "
			"leaving that channel silent",
			filename, sequenceOffset, channel);
}

bool PhantomPSound::callFunction(uint16 targetOffset, Channel &channel) {
	(void)targetOffset;
	(void)channel;
	return false;
}

int PhantomPSound::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	(void)param;
	_frameCounter = 0;
	if (commandId >= 0 && commandId <= 8)
		return dispatchBaseCommand(commandId);
	return executeCommand(commandId);
}

bool validatePhantomPSoundFile(int section, bool isDemo,
							   Common::String *reason) {
	const ValidationEntry *entry = isDemo ? &kDemoValidation : retailValidationForSection(section);
	if (!entry) {
		if (reason)
			*reason = "unsupported section";
		return false;
	}
	return PSound::validateFile(*entry->driverData, entry->first8192Md5,
								reason);
}

PSound1::PSound1(Audio::Mixer *mixer) : PhantomPSound(mixer, kPSound1Data),
		_previousSelector(0xffff), _olderSelector(0xffff) {
}

void PSound1::selectBackgroundMusic() {
	static const uint16 tracks[][kMusicChannelCount] = {
		{0x095d, 0x0984, 0x09f7, 0x0ab8, 0x0972, 0x0b67},
		{0x16ba, 0x1796, 0x184b, 0x1908, 0x191a, 0x1927},
		{0x192a, 0x1b20, 0x1ca5, 0x1e72, 0x1fdd, 0x1ff6},
		{0x1ff8, 0x2285, 0x24c2, 0x24d4, 0x24e6, 0x24f3},
		{0x0cc7, 0x0cec, 0x0df9, 0x0e9e, 0x0cdc, 0x0ea7}};
	static const byte weightedTracks[] = {0, 3, 2, 1, 0, 4, 1, 2};
	static const uint16 guards[] = {0x095d, 0x0cc7, 0x1ff8, 0x16ba, 0x192a};

	if (_channels[0].activeCount) {
		for (uint i = 0; i < ARRAYSIZE(guards); ++i) {
			if (isSoundActive(guards[i]))
				return;
		}
	}

	command1();
	uint16 selector;
	do {
		selector = nextRandom() & 7;
	} while (selector == _previousSelector || selector == _olderSelector);
	_olderSelector = _previousSelector;
	_previousSelector = selector;
	loadFixedChannels(tracks[weightedTracks[selector]], kMusicChannelCount);
}

int PSound1::executeCommand(int commandId) {
	switch (commandId) {
	case 0x10:
		selectBackgroundMusic();
		break;
	case 0x18: {
		static const uint16 sounds[] = {0x3a50, 0x3a5c};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x19: {
		static const uint16 sounds[] = {0x3a6a, 0x3a76};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x1a:
		playSound(0x3a84);
		break;
	case 0x1b:
		playSound(0x3a8c);
		break;
	case 0x20: {
		static const uint16 music[] = {
			0x2e8a, 0x2ec5, 0x2ef9, 0x2f23, 0x2f4f, 0x2f7b};
		loadFixedIfInactive(0x2e8a, music, ARRAYSIZE(music));
		break;
	}
	case 0x21: {
		static const uint16 music[] = {
			0x24f6, 0x272f, 0x28be, 0x2a8b, 0x2bc4, 0x2cf3};
		loadFixedIfInactive(0x24f6, music, ARRAYSIZE(music));
		break;
	}
	case 0x22: {
		static const uint16 music[] = {
			0x0178, 0x03a6, 0x04cf, 0x0600, 0x0701, 0x07df};
		loadFixedIfInactive(0x0178, music, ARRAYSIZE(music));
		break;
	}
	case 0x23: {
		static const uint16 music[] = {
			0x3454, 0x358c, 0x3652, 0x3708, 0x37be};
		loadFixedChannels(music, ARRAYSIZE(music));
		leaveMalformedChannelSilent("PSOUND.PH1", 5, 0x69fe);
		break;
	}
	case 0x24: {
		static const uint16 music[] = {
			0x2fd4, 0x310e, 0x31e2, 0x32a6, 0x336a, 0x336a};
		loadFixedIfInactive(0x2fd4, music, ARRAYSIZE(music));
		break;
	}
	case 0x25: {
		static const uint16 music[] = {
			0x38a6, 0x390b, 0x396b, 0x39bb, 0x3a06, 0x39b2};
		loadFixedChannels(music, ARRAYSIZE(music));
		break;
	}
	case 0x26: {
		static const uint16 music[] = {
			0x095d, 0x0984, 0x09f7, 0x0ab8, 0x0972, 0x0b67};
		command1();
		loadFixedChannels(music, ARRAYSIZE(music));
		break;
	}
	case 0x27: {
		static const uint16 music[] = {
			0x0eaa, 0x1051, 0x116b, 0x1253, 0x1355, 0x1521};
		loadFixedIfInactive(0x0eaa, music, ARRAYSIZE(music));
		break;
	}
	case 0x40:
		playSound(0x3a98);
		break;
	case 0x41: {
		static const uint16 sounds[] = {0x3aa8, 0x3aba};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x42:
		playSound(0x3acc);
		break;
	case 0x43:
		playSound(0x3adc);
		break;
	case 0x44:
		playSound(0x3b7c);
		break;
	case 0x45: {
		static const uint16 sounds[] = {0x3afa, 0x3b17};
		command5();
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x46: {
		static const uint16 sounds[] = {0x3b32, 0x3b3c};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x47:
		playSound(0x3b65);
		break;
	case 0x48:
		playSound(0x3b83);
		break;
	case 0x49:
		playSound(0x3b9a);
		break;
	case 0x4a:
		playSound(0x3bf6);
		break;
	case 0x4b: {
		static const uint16 sounds[] = {0x3bac, 0x3bcc};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x4c:
		playSound(0x3b44);
		break;
	default:
		break;
	}
	return 0;
}

PSound2::PSound2(Audio::Mixer *mixer) : PhantomPSound(mixer, kPSound2Data) {
}

int PSound2::executeCommand(int commandId) {
	switch (commandId) {
	case 0x10: {
		static const uint16 music[] = {0x0e8e, 0x0eed, 0x0f46};
		if (!isSoundActive(0x0e8e)) {
			command1();
			playMusic(music, ARRAYSIZE(music));
		}
		break;
	}
	case 0x18: {
		static const uint16 sounds[] = {0x12ee, 0x12fa};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x19: {
		static const uint16 sounds[] = {0x1308, 0x1314};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x1a:
		playSound(0x1322);
		break;
	case 0x1b:
		playSound(0x132a);
		break;
	case 0x20: {
		static const uint16 music[] = {0x0f9e, 0x105e, 0x11b1};
		command1();
		playMusic(music, ARRAYSIZE(music));
		break;
	}
	case 0x21: {
		static const uint16 music[] = {
			0x1258, 0x1299, 0x12a9, 0x12b9,
			0x12c9, 0x12cb, 0x12cd, 0x12db};
		loadFixedIfInactive(0x1258, music, ARRAYSIZE(music));
		break;
	}
	case 0x22: {
		static const uint16 music[] = {
			0x02b2, 0x0483, 0x06b2, 0x0855, 0x0aa0, 0x0d2f, 0x0e36};
		if (!isSoundActive(0x02b2)) {
			command1();
			loadFixedChannels(music, ARRAYSIZE(music));
		}
		break;
	}
	case 0x23: {
		static const uint16 music[] = {
			0x0178, 0x01f3, 0x020e, 0x0251, 0x027c};
		if (!isSoundActive(0x0178)) {
			command1();
			playMusic(music, ARRAYSIZE(music));
		}
		break;
	}
	case 0x40:
		playSound(0x140d);
		break;
	case 0x41:
		playSound(0x1346);
		break;
	case 0x42:
		playSound(0x133e);
		break;
	case 0x43:
		playSound(0x1336);
		break;
	case 0x44: {
		static const uint16 sounds[] = {0x1346, 0x1387, 0x13ca};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x45:
		playSound(0x1415);
		break;
	case 0x46: {
		static const uint16 sounds[] = {0x1425, 0x1442};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x47:
		playSound(0x145d);
		break;
	case 0x48:
		playSound(0x1467);
		break;
	default:
		break;
	}
	return 0;
}

PSound3::PSound3(Audio::Mixer *mixer) : PhantomPSound(mixer, kPSound3Data) {
}

int PSound3::executeCommand(int commandId) {
	switch (commandId) {
	case 0x10: {
		static const uint16 music[] = {
			0x0798, 0x0873, 0x08c3, 0x0935, 0x09bf, 0x09e7};
		if (!isSoundActive(0x0798)) {
			command1();
			loadFixedChannels(music, ARRAYSIZE(music));
		}
		break;
	}
	case 0x18: {
		static const uint16 sounds[] = {0x05da, 0x05e6};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x19: {
		static const uint16 sounds[] = {0x05f4, 0x0600};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x1a:
		playSound(0x060e);
		break;
	case 0x1b:
		playSound(0x0616);
		break;
	case 0x20: {
		static const uint16 music[] = {
			0x2c37, 0x2c86, 0x2ca6, 0x2ce6,
			0x2d3c, 0x2d3c, 0x2d3e, 0x2d5c};
		loadFixedChannels(music, ARRAYSIZE(music));
		break;
	}
	case 0x21: {
		static const uint16 music[] = {
			0x122a, 0x144d, 0x16a8, 0x17ae, 0x198a, 0x1be8};
		if (!isSoundActive(0x122a)) {
			command1();
			loadFixedChannels(music, ARRAYSIZE(music));
		}
		break;
	}
	case 0x22: {
		static const uint16 music[] = {
			0x1d68, 0x205d, 0x216d, 0x2365,
			0x257d, 0x27c5, 0x290d, 0x2ac5};
		loadFixedChannels(music, ARRAYSIZE(music));
		break;
	}
	case 0x23: {
		static const uint16 music[] = {
			0x0a30, 0x0bd3, 0x0ceb, 0x0dcf, 0x0ecd, 0x1095};
		if (!isSoundActive(0x0a30)) {
			command1();
			loadFixedChannels(music, ARRAYSIZE(music));
		}
		break;
	}
	case 0x24: {
		static const uint16 music[] = {
			0x0178, 0x01f0, 0x02b1, 0x0506, 0x0573};
		if (!isSoundActive(0x0178)) {
			command1();
			playMusic(music, ARRAYSIZE(music));
		}
		break;
	}
	case 0x25:
		playSound(0x0653);
		break;
	case 0x40:
	case 0x4b: {
		static const uint16 sounds[] = {0x0622, 0x0627};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x41: {
		static const uint16 sounds[] = {0x067d, 0x06c0};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x42: {
		static const uint16 sounds[] = {0x0622, 0x0641};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x43:
		playSound(0x064b);
		break;
	case 0x44:
		playSound(0x0669);
		break;
	case 0x45:
		playSound(0x0671);
		break;
	case 0x46:
		playSound(0x0722);
		break;
	case 0x47:
		playSound(0x0701);
		break;
	case 0x48: {
		static const uint16 sounds[] = {0x0739, 0x0622, 0x0627};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x49:
		playSound(0x0745);
		break;
	case 0x4a:
		playSound(0x0757);
		break;
	default:
		break;
	}
	return 0;
}

PSound4::PSound4(Audio::Mixer *mixer) : PhantomPSound(mixer, kPSound4Data) {
}

bool PSound4::callFunction(uint16 targetOffset, Channel &channel) {
	(void)channel;
	if (targetOffset != 0x2a70)
		return false;

	const uint tableIndex = nextRandom() & 0x0f;
	writeDataUint16(0x019a, readDataUint16(0x0464 + tableIndex * 2));
	return true;
}

int PSound4::executeCommand(int commandId) {
	switch (commandId) {
	case 0x10: {
		static const uint16 music[] = {
			0x0178, 0x03a6, 0x03cc, 0x03f0, 0x0422, 0x0416};
		loadFixedIfInactive(0x0178, music, ARRAYSIZE(music));
		break;
	}
	case 0x18: {
		static const uint16 sounds[] = {0x0490, 0x049c};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x19: {
		static const uint16 sounds[] = {0x049e, 0x04aa};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x1a:
		playSound(0x04b8);
		break;
	case 0x1b:
		playSound(0x04c0);
		break;
	case 0x40:
		playSound(0x04cc);
		break;
	case 0x41:
		playSound(0x04d6);
		break;
	case 0x42:
		playSound(0x04de);
		break;
	case 0x43:
		playSound(0x04e6);
		break;
	case 0x44:
		playSound(0x04ee);
		break;
	case 0x45:
		playSound(0x04f6);
		break;
	case 0x46:
		playSound(0x04fe);
		break;
	default:
		break;
	}
	return 0;
}

PSound5::PSound5(Audio::Mixer *mixer) : PhantomPSound(mixer, kPSound5Data) {
}

int PSound5::executeCommand(int commandId) {
	switch (commandId) {
	case 0x10: {
		static const uint16 music[] = {
			0x3bc2, 0x3c42, 0x3ca7, 0x3d0a, 0x3d59, 0x3de4};
		if (!isSoundActive(0x3bc2)) {
			command1();
			loadFixedChannels(music, ARRAYSIZE(music));
		}
		break;
	}
	case 0x18: {
		static const uint16 sounds[] = {0x4380, 0x438c};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x19: {
		static const uint16 sounds[] = {0x439a, 0x43a6};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x1a:
		playSound(0x43b4);
		break;
	case 0x1b:
		playSound(0x43bc);
		break;
	case 0x20: {
		static const uint16 music[] = {
			0x3e68, 0x3fb9, 0x4013, 0x40f7,
			0x419d, 0x423d, 0x4289, 0x4311};
		command1();
		loadFixedChannels(music, ARRAYSIZE(music));
		break;
	}
	case 0x21: {
		static const uint16 music[] = {
			0x1e66, 0x20d3, 0x2330, 0x2444, 0x2652, 0x28c8};
		if (!isSoundActive(0x1e66)) {
			command1();
			loadFixedChannels(music, ARRAYSIZE(music));
		}
		break;
	}
	case 0x22: {
		static const uint16 music[] = {
			0x0cc8, 0x0e6a, 0x0fee, 0x108e, 0x1136,
			0x1166, 0x11a1, 0x11de, 0x1219};
		loadFixedChannels(music, ARRAYSIZE(music));
		break;
	}
	case 0x23: {
		static const uint16 music[] = {0x2a72, 0x3051, 0x35e1, 0x2a7d};
		command1();
		playMusic(music, ARRAYSIZE(music));
		break;
	}
	case 0x24: {
		static const uint16 music[] = {0x19c2, 0x1b1b, 0x1c4b, 0x19d4};
		command1();
		playMusic(music, ARRAYSIZE(music));
		break;
	}
	case 0x25: {
		static const uint16 music[] = {
			0x123f, 0x1597, 0x16f4, 0x192b, 0x1236};
		command1();
		playMusic(music, ARRAYSIZE(music));
		break;
	}
	case 0x26: {
		static const uint16 music[] = {
			0x0876, 0x09ae, 0x0a74, 0x0b2a, 0x0be0};
		loadFixedChannels(music, ARRAYSIZE(music));
		leaveMalformedChannelSilent("PSOUND.PH5", 5, 0x704c);
		break;
	}
	case 0x27: {
		static const uint16 music[] = {
			0x0178, 0x03a6, 0x04cf, 0x0600, 0x0701, 0x07df};
		loadFixedIfInactive(0x0178, music, ARRAYSIZE(music));
		break;
	}
	case 0x40:
		playSound(0x4430);
		break;
	case 0x41:
		playSound(0x43c8);
		break;
	case 0x42:
		playSound(0x43d0);
		break;
	case 0x43:
		loadChannel(8, 0x43dc);
		break;
	case 0x44:
		playSound(0x43e4);
		break;
	case 0x45:
		playSound(0x43f4);
		break;
	case 0x46:
		playSound(0x440c);
		break;
	case 0x47:
		playSound(0x4416);
		break;
	case 0x48:
		playSound(0x4428);
		break;
	case 0x49:
		playSound(0x4378);
		break;
	case 0x4a:
		playSound(0x4438);
		break;
	case 0x4b:
		playSound(0x444a);
		break;
	case 0x4c:
		playSound(0x4452);
		break;
	case 0x4d:
		playSound(0x445c);
		break;
	case 0x4e:
		playSound(0x4464);
		break;
	default:
		break;
	}
	return 0;
}

PSound9::PSound9(Audio::Mixer *mixer) : PhantomPSound(mixer, kPSound9Data) {
}

int PSound9::executeCommand(int commandId) {
	switch (commandId) {
	case 0x18: {
		static const uint16 sounds[] = {0x2558, 0x2564};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x19: {
		static const uint16 sounds[] = {0x2572, 0x257e};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x1a:
		playSound(0x258c);
		break;
	case 0x1b:
		playSound(0x2594);
		break;
	case 0x20: {
		static const uint16 music[] = {
			0x1016, 0x107e, 0x10e0, 0x12e5, 0x1303, 0x1323, 0x1357};
		loadFixedChannels(music, ARRAYSIZE(music));
		break;
	}
	case 0x22: {
		static const uint16 music[] = {
			0x237c, 0x23ba, 0x241b, 0x2462, 0x24c9, 0x2504};
		command1();
		playMusic(music, ARRAYSIZE(music));
		break;
	}
	case 0x23: {
		static const uint16 music[] = {
			0x05f6, 0x0633, 0x065d, 0x0683, 0x06dd, 0x073b, 0x077b};
		command0();
		loadFixedChannels(music, ARRAYSIZE(music));
		break;
	}
	case 0x24: {
		static const uint16 music[] = {
			0x223a, 0x226d, 0x22a1, 0x22d5, 0x230d, 0x2339};
		command1();
		playMusic(music, ARRAYSIZE(music));
		break;
	}
	case 0x25: {
		static const uint16 music[] = {
			0x07b0, 0x0827, 0x091d, 0x099d, 0x09f5, 0x0c55, 0x0f77};
		command0();
		loadFixedChannels(music, ARRAYSIZE(music));
		break;
	}
	case 0x26: {
		static const uint16 music[] = {
			0x138c, 0x166d, 0x177b, 0x1971,
			0x1b87, 0x1dcd, 0x1f13, 0x20c9};
		command1();
		loadFixedChannels(music, ARRAYSIZE(music));
		break;
	}
	case 0x27: {
		static const uint16 music[] = {
			0x0178, 0x02b2, 0x0386, 0x044a, 0x050e};
		command0();
		loadFixedChannels(music, ARRAYSIZE(music));
		break;
	}
	case 0x40:
		playSound(0x25e5);
		break;
	case 0x41:
		playSound(0x25f7);
		break;
	case 0x42: {
		static const uint16 sounds[] = {0x25f7, 0x25fc, 0x2605};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x43: {
		static const uint16 sounds[] = {0x25bc, 0x25db};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x44:
		playSound(0x25ef);
		break;
	case 0x45:
		playSound(0x25a0);
		break;
	case 0x46: {
		static const uint16 sounds[] = {0x25b0, 0x25b7};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	case 0x47: {
		static const uint16 sounds[] = {0x25bc, 0x25c1};
		playSounds(sounds, ARRAYSIZE(sounds));
		break;
	}
	default:
		break;
	}
	return 0;
}

PSoundDemo::PSoundDemo(Audio::Mixer *mixer) : PhantomPSound(mixer, kPSoundDemoData) {
}

int PSoundDemo::executeCommand(int commandId) {
	switch (commandId) {
	case 0x09: {
		static const uint16 music[] = {
			0x1d02, 0x1e53, 0x1ead, 0x1f91,
			0x2037, 0x20d7, 0x2123, 0x21ab};
		command1();
		loadFixedChannels(music, ARRAYSIZE(music));
		break;
	}
	case 0x10: {
		static const uint16 music[] = {
			0x0170, 0x0430, 0x0602, 0x0814, 0x097e, 0x0ade};
		command5();
		loadFixedChannels(music, ARRAYSIZE(music));
		break;
	}
	case 0x16: {
		static const uint16 music[] = {
			0x149c, 0x1513, 0x1609, 0x1689, 0x16e1, 0x1941, 0x1c63};
		command1();
		loadFixedChannels(music, ARRAYSIZE(music));
		break;
	}
	case 0x17: {
		static const uint16 music[] = {
			0x12e2, 0x131f, 0x1349, 0x136f, 0x13c9, 0x1427, 0x1467};
		command1();
		loadFixedChannels(music, ARRAYSIZE(music));
		break;
	}
	case 0x18: {
		static const uint16 music[] = {
			0x2206, 0x2237, 0x2265, 0x2297, 0x22cd, 0x22f7};
		command1();
		playMusic(music, ARRAYSIZE(music));
		break;
	}
	case 0x19: {
		static const uint16 music[] = {
			0x2338, 0x2376, 0x23d7, 0x241e, 0x2485, 0x24c0};
		command1();
		playMusic(music, ARRAYSIZE(music));
		break;
	}
	case 0x1a: {
		static const uint16 music[] = {
			0x0d95, 0x0dbc, 0x0e2f, 0x0ef0, 0x0daa};
		command1();
		playMusic(music, ARRAYSIZE(music));
		break;
	}
	case 0x1b: {
		static const uint16 music[] = {
			0x10ff, 0x1124, 0x1231, 0x12d6, 0x1114};
		command1();
		playMusic(music, ARRAYSIZE(music));
		break;
	}
	default:
		break;
	}
	return 0;
}

} // namespace Sound
} // namespace Phantom
} // namespace MADS
