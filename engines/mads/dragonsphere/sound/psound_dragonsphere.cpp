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

#include "common/textconsole.h"
#include "common/util.h"
#include "mads/dragonsphere/sound/psound_dragonsphere.h"

namespace MADS {
namespace Dragonsphere {
namespace Sound {

namespace {

const PSoundDriverData kPSound1Data = {
	"PSOUND.DR1", 0x3470, 0x428c, 0x4680, 0x0c2a, 0x06de,
	0x101c, 56, { 0x0058, 0x00d8, 0x00f4, 0x00fe, 0x0108 }
};

const PSoundDriverData kPSound2Data = {
	"PSOUND.DR2", 0x2f20, 0x1b3d, 0x1f30, 0x0058, 0x0826,
	0x0c26, 37, { 0x0096, 0x0116, 0x0132, 0x013c, 0x0146 }
};

const PSoundDriverData kPSound3Data = {
	"PSOUND.DR3", 0x2ee0, 0x192f, 0x1d20, 0x0058, 0x016a,
	0x0328, 36, { 0x0096, 0x0116, 0x0132, 0x013c, 0x0146 }
};

const PSoundDriverData kPSound4Data = {
	"PSOUND.DR4", 0x3110, 0x2833, 0x2c30, 0x0058, 0x0d2a,
	0x016a, 47, { 0x0096, 0x0116, 0x0132, 0x013c, 0x0146 }
};

const PSoundDriverData kPSound5Data = {
	"PSOUND.DR5", 0x30c0, 0x27c9, 0x2bc0, 0x0058, 0x1fcc,
	0x016a, 44, { 0x0096, 0x0116, 0x0132, 0x013c, 0x0146 }
};

const PSoundDriverData kPSound6Data = {
	"PSOUND.DR6", 0x32f0, 0x2cc3, 0x30c0, 0x0058, 0x0cba,
	0x016a, 45, { 0x0096, 0x0116, 0x0132, 0x013c, 0x0146 }
};

const PSoundDriverData kPSound9Data = {
	"PSOUND.DR9", 0x33e0, 0x61ab, 0x65a0, 0x0058, 0x4b78,
	0x016a, 52, { 0x0096, 0x0116, 0x0132, 0x013c, 0x0146 }
};

const PSoundDriverData kPSoundDemo1Data = {
	"PSOUND.DR1", 0x3110, 0x4933, 0x4d30, 0x0970, 0x06da,
	0x0d62, 128, { 0x0058, 0x00d8, 0x00f4, 0x00fe, 0x0108 }
};

const PSoundDriverData kPSoundDemo9Data = {
	"PSOUND.DR9", 0x3170, 0x5f47, 0x6340, 0x0a44, 0x09d4,
	0x0b64, 128, { 0x0a90, 0x0b10, 0x0b2c, 0x0b36, 0x0b40 }
};

struct ValidationEntry {
	const PSoundDriverData *driverData;
	const char *first8192Md5;
};

const ValidationEntry kRetailValidation[] = {
	{ &kPSound1Data, "61922da6166387e3375431cebc9b7b1e" },
	{ &kPSound2Data, "ba888c3a1942510beb58fcb6fae2f8c2" },
	{ &kPSound3Data, "6835d68cac3a2ecf84cf0b38f80d433a" },
	{ &kPSound4Data, "32e02460b6bfd28b6a928fc41fa16a91" },
	{ &kPSound5Data, "ff00aed0b32ac3b1c3a3fec4cc4c1a4f" },
	{ &kPSound6Data, "01bce80ca94793ea14965264df2a59fb" },
	{ &kPSound9Data, "0f946874841f337fb2c267a70aff119a" }
};

const ValidationEntry kDemoValidation[] = {
	{ &kPSoundDemo1Data, "baa780e3793975905516ad95ffa07383" },
	{ &kPSoundDemo9Data, "cf13056c76a459a5739d5f2fa21dc4ff" }
};

const ValidationEntry *retailValidationForSection(int section) {
	switch (section) {
	case 1: return &kRetailValidation[0];
	case 2: return &kRetailValidation[1];
	case 3: return &kRetailValidation[2];
	case 4: return &kRetailValidation[3];
	case 5: return &kRetailValidation[4];
	case 6: return &kRetailValidation[5];
	case 9: return &kRetailValidation[6];
	default: return nullptr;
	}
}

const ValidationEntry *demoValidationForSection(int section) {
	switch (section) {
	case 1: return &kDemoValidation[0];
	case 9: return &kDemoValidation[1];
	default: return nullptr;
	}
}

const int kPSound5AlternateMusic = 0x100;

} // namespace

bool validateDragonspherePSoundFile(int section, bool isDemo,
		Common::String *reason) {
	const ValidationEntry *entry = isDemo ?
			demoValidationForSection(section) :
			retailValidationForSection(section);
	if (!entry) {
		if (reason)
			*reason = "unsupported section";
		return false;
	}

	return PSound::validateFile(*entry->driverData, entry->first8192Md5,
			reason);
}

DragonspherePSound::DragonspherePSound(Audio::Mixer *mixer,
		const PSoundDriverData &driverData, byte maxMusicCommand,
		bool resetClearsCallback) :
		PSound(mixer, driverData), _callbackCounter(0), _callbackPeriod(0),
		_pendingCommand(-1), _pendingLoadOnly(false), _musicIndex(-1),
		_maxMusicCommand(maxMusicCommand),
		_resetClearsCallback(resetClearsCallback) {
}

DragonspherePSoundDemo::DragonspherePSoundDemo(Audio::Mixer *mixer,
		const PSoundDriverData &driverData, bool resetClearsCallback) :
		DragonspherePSound(mixer, driverData, 0, resetClearsCallback) {
}

int DragonspherePSoundDemo::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	_commandParam = param;
	_frameCounter = 0;
	// The demo dispatchers call their group-32 handlers directly and never
	// maintain the retail driver's saved music-command word.
	return executeCommand(commandId, false);
}

int DragonspherePSound::dispatchBaseCommand(int commandId) {
	switch (commandId) {
	case 0: return resetSection();
	case 1: return command1();
	case 2: return command2();
	case 3: return command3();
	case 4: return command4();
	case 5: return command5();
	case 6: return command6();
	case 7: return command7();
	case 8: return command8();
	default: return 0;
	}
}

int DragonspherePSound::resetSection() {
	if (_resetClearsCallback) {
		_callbackCounter = 0;
		_callbackPeriod = 0;
		_pendingCommand = -1;
		_pendingLoadOnly = false;
	}
	// Every retail section preserves the exported dispatcher's saved music
	// command. Only the section callback hook varies between overlays.
	return PSound::command0();
}

void DragonspherePSound::playSounds(const uint16 *sequences, uint count,
		bool anyChannel) {
	for (uint i = 0; i < count; ++i) {
		if (anyChannel)
			playSoundAny(sequences[i]);
		else
			playSound(sequences[i]);
	}
}

void DragonspherePSound::loadChannels(const ChannelLoad *loads, uint count) {
	for (uint i = 0; i < count; ++i)
		loadChannel(loads[i].channel, loads[i].sequence);
}

bool DragonspherePSound::musicChannelsActive(uint count) const {
	for (uint i = 0; i < count; ++i) {
		if (_channels[i].activeCount)
			return true;
	}
	return false;
}

void DragonspherePSound::applyMusicLoad(const MusicLoad &load) {
	if (load.clearCallback) {
		_pendingCommand = -1;
		_pendingLoadOnly = false;
	}
	if (load.counter >= 0)
		_callbackCounter = load.counter;
	if (load.period >= 0)
		_callbackPeriod = load.period;
	if (load.musicIndex >= 0)
		_musicIndex = load.musicIndex;

	switch (load.reset) {
	case kClearMusicIdentity:
		command2();
		break;
	case kStopMusic:
		command3();
		break;
	case kStopAll:
		command1();
		break;
	case kKeepPlayback:
		break;
	}
	loadChannels(load.channels, load.channelCount);
}

bool DragonspherePSound::startOrDeferMusic(int commandId, uint16 guard,
		const MusicLoad &load, bool loadOnly, uint musicChannelCount) {
	if (!loadOnly) {
		if (guard && isSoundActive(guard))
			return false;
		if (musicChannelsActive(musicChannelCount)) {
			deferCommand(commandId, true);
			return false;
		}
	}
	applyMusicLoad(load);
	return true;
}

bool DragonspherePSound::startOrDeferMusicWhenActive(int commandId,
		uint16 guard, const MusicLoad &load, bool loadOnly,
		uint musicChannelCount) {
	// Several overlays test the sequence guard only after establishing that
	// music is already active. An idle driver therefore loads immediately,
	// even if the same sequence remains on a non-music channel.
	if (!loadOnly && musicChannelsActive(musicChannelCount)) {
		if (guard && isSoundActive(guard))
			return false;
		deferCommand(commandId, true);
		return false;
	}
	applyMusicLoad(load);
	return true;
}

void DragonspherePSound::deferCommand(int commandId, bool loadOnly) {
	_pendingCommand = commandId;
	_pendingLoadOnly = loadOnly;
}

void DragonspherePSound::serviceCallbacks() {
	// The native timer continues counting whenever a period is installed,
	// even while no callback is pending. A later deferred command therefore
	// joins the current phase instead of starting a fresh delay.
	if (!_callbackPeriod)
		return;
	if (--_callbackCounter)
		return;
	_callbackCounter = _callbackPeriod;
	if (_pendingCommand < 0)
		return;

	const int commandId = _pendingCommand;
	const bool loadOnly = _pendingLoadOnly;
	_pendingCommand = -1;
	_pendingLoadOnly = false;
	// Native code clears the stored near pointer before invoking it. The
	// callback may explicitly install another one while it runs.
	executeCommand(commandId, loadOnly);
}

int DragonspherePSound::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	_commandParam = param;
	_frameCounter = 0;
	// The exported native dispatcher records every accepted command in the
	// 0x20 music bucket before calling its section handler. Deferred loader
	// tails bypass the dispatcher and therefore leave this state untouched.
	if (commandId >= 0x20 && commandId <= _maxMusicCommand)
		_musicIndex = commandId;
	return executeCommand(commandId, false);
}

// The native overlays dispatch five sparse command buckets. Keeping the
// recovered handler offsets here makes unsupported slots and shared tails
// reviewable without pretending that two equal offsets are two functions.
const uint16 PSound1::_commandList[102] = {
	0x253c,0x29c3,0x288c,0x29ca,0x28b4,0x29f2,0x28ca,0x2936,0x2a9b,
	0,0,0,0,0,0,0, 0x21e4,0x311d,0x30fe,0,0,0,0,0,
	0x22da,0x22e8,0x22f6,0x22fd,0x230b,0x2304,0x23cc,0x23de,
	0x2234,0x2292,0x2410,0x2ffc,0x2d70,0x2dc4,0x2e12,0x2ec2,
	0x2f08,0x2e6e,0x3165,0x320a,0x30a2,0x2fb0,0x31ba,0x2f64,0x320f,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0x2312,0x2318,0x231e,0x2324,0x232a,0x2336,0x233c,0x2342,
	0x2348,0x234e,0x2354,0x235a,0x2360,0x2366,0x236c,0x2372,
	0x2378,0x237e,0x2384,0x238a,0x2390,0x2396,0x239c,0x23a2,
	0x23a8,0x23ba,0x23ae,0x23b4,0x2cd8,0x23c0,0x23c6,0x23d2,
	0x23e5,0x23ec,0x230b,0x23fa,0x2401,0x2408
};

PSound1::PSound1(Audio::Mixer *mixer) :
		DragonspherePSound(mixer, kPSound1Data, 48, true) {
}

bool PSound1::callFunction(uint16 targetOffset, Channel &channel) {
	(void)channel;
	switch (targetOffset) {
	case 0x29c3:
		command1();
		return true;
	case 0x2f98:
		// The stream installs command 16's entry point, not its loader tail.
		_callbackCounter = 0xc0;
		_callbackPeriod = 0x60;
		deferCommand(16, false);
		return true;
	case 0x315c:
		// The native callback re-enters command 41 and therefore repeats its
		// active-sequence guard before deciding whether to defer again.
		deferCommand(41, false);
		return true;
	case 0x2f08:
		executeCommand(40, false);
		return true;
	default:
		return false;
	}
}

int PSound1::executeCommand(int commandId, bool loadOnly) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList) ||
			!_commandList[commandId])
		return 0;
	if (commandId <= 8)
		return dispatchBaseCommand(commandId);

	static const uint16 effectCommands24[] = {
		0x06e0, 0x06ec, 0x06fa, 0x0706, 0x0714, 0x071c,
		0x09d0, 0x0728, 0x08e8, 0x090e
	};
	static const byte effectCommandStarts24[] = { 0, 2, 4, 5, 6, 7, 8, 9, 10 };
	static const uint16 effectCommands64[][2] = {
		{ 0x0750, 0 }, { 0x0760, 0 }, { 0x0768, 0 }, { 0x0770, 0 },
		{ 0x0780, 0 }, { 0x0778, 0 }, { 0x078c, 0 }, { 0x079e, 0 },
		{ 0x07aa, 0 }, { 0x0796, 0 }, { 0x07ba, 0 }, { 0x07c2, 0 },
		{ 0x07d2, 0 }, { 0x07da, 0 }, { 0x07ee, 0 }, { 0x07e2, 0 },
		{ 0x07f6, 0 }, { 0x07fe, 0 }, { 0x0816, 0 }, { 0x0828, 0 },
		{ 0x0836, 0 }, { 0x083e, 0 }, { 0x0846, 0 }, { 0x0778, 0 },
		{ 0x085c, 0 }, { 0x0864, 0 }, { 0x0870, 0 }, { 0x0898, 0 },
		{ 0, 0 }, { 0x08cc, 0 }, { 0x08d8, 0 }, { 0x08fa, 0x08f8 },
		{ 0x0969, 0 }, { 0x09b6, 0x099a }, { 0x09d0, 0 }, { 0x09d8, 0 },
		{ 0x09e4, 0 }, { 0x09f0, 0 }
	};

	if (commandId >= 24 && commandId <= 31) {
		const uint first = effectCommandStarts24[commandId - 24];
		const uint end = effectCommandStarts24[commandId - 23];
		playSounds(effectCommands24 + first, end - first);
		return 0;
	}
	if (commandId >= 64) {
		const uint index = commandId - 64;
		const uint16 first = effectCommands64[index][0];
		if (commandId == 68 && isSoundActive(first))
			return 0;
		if (first)
			playSound(first);
		if (effectCommands64[index][1])
			playSound(effectCommands64[index][1]);
		return 0;
	}

	switch (commandId) {
	case 16: {
		static const ChannelLoad channels[] = {
			{ 0, 0x012c }, { 1, 0x01d6 }, { 2, 0x027a },
			{ 3, 0x0355 }, { 4, 0x03ab }, { 5, 0x03b9 }
		};
		_musicIndex = 0x10;
		if (loadOnly || !isSoundActive(0x012c)) {
			const MusicLoad load = {
				kStopAll, -1, 0x90, 0x90, true,
				channels, ARRAYSIZE(channels)
			};
			applyMusicLoad(load);
		}
		break;
	}
	case 17: {
		static const ChannelLoad channels[] = {
			{ 0, 0x3f12 }, { 1, 0x3f46 },
			{ 2, 0x3f7b }, { 3, 0x3f9a }
		};
		if (loadOnly || !isSoundActive(0x3f12)) {
			const MusicLoad load = {
				kClearMusicIdentity, -1, 0xc0, 0x60, true,
				channels, ARRAYSIZE(channels)
			};
			applyMusicLoad(load);
		}
		break;
	}
	case 18:
		command2();
		if (_musicIndex >= 0 && _musicIndex != 18)
			return executeCommand(_musicIndex, false);
		break;
	case 32: {
		static const ChannelLoad channels[] = {
			{ 0, 0x03d5 }, { 6, 0x041b }, { 2, 0x0494 },
			{ 3, 0x048b }, { 4, 0x04bd }, { 5, 0x03c8 }
		};
		const MusicLoad load = {
			kStopAll, 0x20, 0xb0, 0xb0, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0x03d5, load, loadOnly);
		break;
	}
	case 33: {
		static const ChannelLoad channels[] = {
			{ 6, 0x04e8 }, { 1, 0x05cc }, { 2, 0x0631 },
			{ 3, 0x0665 }, { 4, 0x065e }, { 5, 0x05bf },
			{ 0, 0x056d }
		};
		const MusicLoad load = {
			kStopAll, -1, 0xb0, 0xb0, true,
			channels, ARRAYSIZE(channels)
		};
		applyMusicLoad(load);
		break;
	}
	case 34: {
		static const ChannelLoad channels[] = {
			{ 0, 0x0a06 }, { 1, 0x0a7b }, { 2, 0x0aee },
			{ 3, 0x0b81 }, { 4, 0x0bf6 }, { 5, 0x0a47 }
		};
		if (loadOnly || !isSoundActive(0x0a06)) {
			const MusicLoad load = {
				kStopAll, -1, -1, -1, false,
				channels, ARRAYSIZE(channels)
			};
			applyMusicLoad(load);
		}
		break;
	}
	case 35: {
		static const ChannelLoad channels[] = {
			{ 0, 0x3a1a }, { 1, 0x3ab1 }, { 2, 0x3b3a },
			{ 3, 0x3b90 }, { 4, 0x3b87 }
		};
		const MusicLoad load = {
			kStopAll, -1, 0x60, 0x60, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0x3a1a, load, loadOnly);
		break;
	}
	case 36: {
		static const ChannelLoad channels[] = {
			{ 0, 0x0c70 }, { 1, 0x0d43 }, { 2, 0x0e3a },
			{ 3, 0x0ecf }, { 4, 0x0d36 }
		};
		const MusicLoad load = {
			kStopAll, -1, 0x80, 0x80, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0x0c70, load, loadOnly);
		break;
	}
	case 37: {
		static const ChannelLoad channels[] = {
			{ 0, 0x0f46 }, { 1, 0x0fb5 },
			{ 2, 0x0fa8 }, { 3, 0x0f9d }
		};
		const MusicLoad load = {
			kStopAll, -1, 0xc0, 0xc0, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0x0f46, load, loadOnly);
		break;
	}
	case 38: {
		static const ChannelLoad channels[] = {
			{ 0, 0x1e1c }, { 1, 0x1efb }, { 2, 0x1ff3 },
			{ 3, 0x2100 }, { 4, 0x1e2a }, { 5, 0x1ffc }
		};
		const MusicLoad load = {
			kStopAll, -1, 0x60, 0x60, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0x1efb, load, loadOnly);
		break;
	}
	case 39: {
		static const ChannelLoad channels[] = {
			{ 0, 0x245e }, { 1, 0x24b2 }, { 2, 0x2513 }
		};
		const MusicLoad load = {
			kStopAll, -1, 0xb0, 0xb0, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0x245e, load, loadOnly);
		break;
	}
	case 40: {
		static const ChannelLoad channels[] = {
			{ 0, 0x25b0 }, { 1, 0x2790 }, { 2, 0x28e1 },
			{ 3, 0x2b7a }, { 4, 0x2d75 }, { 5, 0x28da }
		};
		const MusicLoad load = {
			kStopAll, -1, 0xa8, 0xa8, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0x25b0, load, loadOnly);
		break;
	}
	case 41: {
		static const ChannelLoad channels[] = {
			{ 0, 0x21f4 }, { 1, 0x2268 }, { 2, 0x22df },
			{ 3, 0x2336 }, { 4, 0x240f }
		};
		const MusicLoad load = {
			kStopAll, -1, 0x90, 0x90, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0x21f4, load, loadOnly);
		break;
	}
	case 42: {
		static const ChannelLoad channels[] = {
			{ 0, 0x3fd5 }, { 1, 0x4007 }, { 2, 0x403c },
			{ 3, 0x407d }, { 4, 0x40b8 }, { 5, 0x3fce }
		};
		const MusicLoad load = {
			kStopAll, 0x29, 0x90, 0x90, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0, load, loadOnly);
		break;
	}
	case 43:
	case 48: {
		static const ChannelLoad channels[] = {
			{ 0, 0x418e }, { 1, 0x41da }, { 2, 0x421f },
			{ 3, 0x4244 }, { 4, 0x4265 }
		};
		if (!loadOnly)
			writeDataByte(0x41da + 5, commandId == 43 ? 0x0b : 0x18);
		const MusicLoad load = {
			kStopAll, -1, 0x54, 0x54, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0, load, loadOnly);
		break;
	}
	case 44: {
		static const ChannelLoad channels[] = {
			{ 0, 0x3cb4 }, { 1, 0x3d06 }, { 2, 0x3d45 },
			{ 3, 0x3d6d }, { 4, 0x3dd3 }, { 5, 0x3e9d }
		};
		const MusicLoad load = {
			kStopAll, -1, 0x60, 0xe0, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0x3cb4, load, loadOnly);
		break;
	}
	case 45: {
		static const ChannelLoad channels[] = {
			{ 0, 0x34f4 }, { 1, 0x3624 }, { 2, 0x36d2 },
			{ 3, 0x37b3 }, { 4, 0x3873 }
		};
		const MusicLoad load = {
			kStopAll, -1, 0x60, 0x60, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0, load, loadOnly);
		break;
	}
	case 46: {
		static const ChannelLoad channels[] = {
			{ 0, 0x40d4 }, { 1, 0x414c }, { 2, 0x4155 },
			{ 3, 0x40e6 }, { 4, 0x40f3 }
		};
		const MusicLoad load = {
			kStopAll, -1, 0x90, 0x90, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0x40d4, load, loadOnly);
		break;
	}
	case 47: {
		static const ChannelLoad channels[] = {
			{ 0, 0x3010 }, { 1, 0x3084 }, { 2, 0x30fe },
			{ 3, 0x3132 }, { 4, 0x33f3 }, { 5, 0x345d },
			{ 6, 0x34bb }
		};
		const MusicLoad load = {
			kStopAll, -1, -1, -1, false,
			channels, ARRAYSIZE(channels)
		};
		applyMusicLoad(load);
		break;
	}
	default:
		break;
	}
	return 0;
}

const uint16 PSound2::_commandList[73] = {
	// 0..8: common driver commands
	0x013c,0x05c3,0x048c,0x05ca,0x04b4,0x05f2,0x04ca,0x0536,0x069b,
	// 9..15: unsupported
	0,0,0,0,0,0,0,
	// 16..18: section music controls
	0x2b62,0x2cdb,0x2cbc,
	// 19..23: unsupported
	0,0,0,0,0,
	// 24..35: effects and long-form music
	0x2ae0,0x2aee,0x2afc,0x2b03,0x2b5a,0x2b0a,0x2b1b,0x2b3e,
	0x2a44,0x2a92,0x2b94,0x2c64,
	// 36..63: unsupported
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,
	// 64..72: effects
	0x2b11,0x2c4e,0x2b22,0x2b29,0x2b30,0x2b37,0x2b37,0x2b45,0x2b53
};

PSound2::PSound2(Audio::Mixer *mixer) :
		DragonspherePSound(mixer, kPSound2Data, 35, false) {
}

bool PSound2::callFunction(uint16 targetOffset, Channel &channel) {
	(void)targetOffset;
	(void)channel;
	return false;
}

int PSound2::executeCommand(int commandId, bool loadOnly) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList) ||
			!_commandList[commandId])
		return 0;
	if (commandId <= 8)
		return dispatchBaseCommand(commandId);

	static const uint16 effectCommands24[] = {
		0x0828, 0x0834, 0x0842, 0x084e, 0x0864,
		0x086c, 0x085c, 0x0878, 0x08b2, 0x0904
	};
	static const byte effectCommandStarts24[] = {
		0, 2, 4, 5, 6, 7, 8, 9, 10
	};

	if (commandId >= 24 && commandId <= 31) {
		const uint first = effectCommandStarts24[commandId - 24];
		const uint end = effectCommandStarts24[commandId - 23];
		playSounds(effectCommands24 + first, end - first);
		return 0;
	}
	if (commandId >= 64) {
		switch (commandId) {
		case 64:
			// Native code calls the ordinary allocator twice without changing CX.
			playSound(0x08a0);
			playSound(0x08a0);
			break;
		case 65: {
			static const uint16 sequences[] = { 0x1566, 0x157b, 0x1589 };
			playSounds(sequences, ARRAYSIZE(sequences), true);
			break;
		}
		case 66: playSound(0x08c2); break;
		case 67: playSound(0x08ca); break;
		case 68: playSound(0x08d2); break;
		case 69:
		case 70: playSound(0x08dc); break;
		case 71: {
			static const uint16 sequences[] = { 0x095f, 0x099e };
			playSounds(sequences, ARRAYSIZE(sequences));
			break;
		}
		case 72: playSound(0x09b7); break;
		default: break;
		}
		return 0;
	}

	switch (commandId) {
	case 16: {
		static const ChannelLoad channels[] = {
			{ 0, 0x0a66 }, { 1, 0x0ab9 }
		};
		const MusicLoad load = {
			kStopAll, -1, 0x60, 0x60, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0, load, loadOnly);
		break;
	}
	case 17: {
		static const ChannelLoad channels[] = {
			{ 0, 0x1a82 }, { 1, 0x1ab6 },
			{ 2, 0x1aeb }, { 3, 0x1b0a }
		};
		if (loadOnly || !isSoundActive(0x1a82)) {
			const MusicLoad load = {
				kClearMusicIdentity, -1, 0xc0, 0x60, true,
				channels, ARRAYSIZE(channels)
			};
			applyMusicLoad(load);
		}
		break;
	}
	case 18:
		command2();
		if (_musicIndex >= 0 && _musicIndex != 18)
			return executeCommand(_musicIndex, false);
		break;
	case 32: {
		static const ChannelLoad channels[] = {
			{ 0, 0x016a }, { 1, 0x01cf }, { 2, 0x0271 },
			{ 3, 0x032f }, { 4, 0x03bb }, { 5, 0x040b }
		};
		const MusicLoad load = {
			kStopAll, -1, 0x60, 0x60, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0, load, loadOnly);
		break;
	}
	case 33: {
		static const ChannelLoad channels[] = {
			{ 0, 0x043e }, { 1, 0x04e5 }, { 2, 0x0585 },
			{ 3, 0x0627 }, { 4, 0x06f7 }, { 5, 0x077d }
		};
		const MusicLoad load = {
			kStopAll, -1, 0x60, 0x60, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0, load, loadOnly);
		break;
	}
	case 34: {
		static const ChannelLoad channels[] = {
			{ 0, 0x09ca }, { 1, 0x0a68 }, { 2, 0x0ab9 },
			{ 3, 0x0b10 }, { 4, 0x0b65 }, { 5, 0x0bbe }
		};
		const MusicLoad load = {
			kStopAll, -1, 0x60, 0x60, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0, load, loadOnly);
		break;
	}
	case 35: {
		static const ChannelLoad channels[] = {
			{ 0, 0x15aa }, { 1, 0x16df }, { 2, 0x1775 },
			{ 3, 0x184b }, { 4, 0x191f }, { 5, 0x19cb }
		};
		if (!loadOnly && musicChannelsActive()) {
			if (!isSoundActive(0x15aa))
				deferCommand(commandId, true);
			break;
		}
		const MusicLoad load = {
			kStopAll, -1, 0xc0, 0x50, true,
			channels, ARRAYSIZE(channels)
		};
		applyMusicLoad(load);
		break;
	}
	default:
		break;
	}
	return 0;
}

const uint16 PSound3::_commandList[74] = {
	0x013c,0x05c3,0x048c,0x05ca,0x04b4,0x05f2,0x04ca,0x0536,0x069b,
	0,0,0,0,0,0,0,
	0x2c0e,0x2bcf,0x2bb0,
	0,0,0,0,0,
	0x2a62,0x2a70,0x2a7e,0x2a85,0x2b30,0x2a8c,0x2a93,0x2ac2,
	0x2c6c,0x2ca2,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0x2a9a,0x2aa1,0x2aa8,0x2ac9,0x2ad0,0x2ad7,0x2ade,0x2ae5,
	0x2b13,0x2b29
};

PSound3::PSound3(Audio::Mixer *mixer) :
		DragonspherePSound(mixer, kPSound3Data, 33, true) {
}

bool PSound3::callFunction(uint16 targetOffset, Channel &channel) {
	(void)channel;
	if (targetOffset != 0x2aec)
		return false;

	Channel *source = &_channels[0];
	if (_channels[0].innerLoopCount) {
		source = &_channels[2];
		if (_channels[2].innerLoopCount)
			source = &_channels[3];
	}
	byte note = source->note;
	while (note < 0x45)
		note += 12;
	writeDataByte(0x02b0, note);
	return true;
}

int PSound3::executeCommand(int commandId, bool loadOnly) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList) ||
			!_commandList[commandId])
		return 0;
	if (commandId <= 8)
		return dispatchBaseCommand(commandId);

	static const uint16 effectCommands24[] = {
		0x016c, 0x0178, 0x0186, 0x0192, 0x01a8,
		0x01b0, 0x01a0, 0x01bc, 0x01e4, 0x021c
	};
	static const byte effectCommandStarts24[] = {
		0, 2, 4, 5, 6, 7, 8, 9, 10
	};

	if (commandId >= 24 && commandId <= 31) {
		const uint first = effectCommandStarts24[commandId - 24];
		const uint end = effectCommandStarts24[commandId - 23];
		playSounds(effectCommands24 + first, end - first);
		return 0;
	}
	if (commandId >= 64) {
		switch (commandId) {
		case 64: playSound(0x01f4); break;
		case 65: playSound(0x0208); break;
		case 66: {
			Channel *active = findActiveSound(0x01f4);
			if (active) {
				active->innerLoopCount = 1;
				active->outerLoopCount = 1;
			}
			playSound(0x0212);
			break;
		}
		case 67: playSound(0x0277); break;
		case 68: playSound(0x027f); break;
		case 69: playSound(0x028b); break;
		case 70: playSound(0x0293); break;
		case 71: playSound(0x02a9); break;
		case 72: {
			static const uint16 sequences[] = { 0x02b4, 0x02d3, 0x02f2 };
			playSounds(sequences, ARRAYSIZE(sequences));
			break;
		}
		case 73: playSound(0x030f); break;
		default: break;
		}
		return 0;
	}

	switch (commandId) {
	case 16: {
		static const ChannelLoad channels[] = {
			{ 0, 0x0ce4 }, { 1, 0x0db2 }, { 2, 0x0e1f },
			{ 3, 0x0fa0 }, { 4, 0x1015 }, { 5, 0x10ef }
		};
		const MusicLoad load = {
			kStopAll, 0x10, 0x90, 0x90, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusic(commandId, 0x0ce4, load, loadOnly);
		break;
	}
	case 17: {
		static const ChannelLoad channels[] = {
			{ 0, 0x0c28 }, { 1, 0x0c5c },
			{ 2, 0x0c91 }, { 3, 0x0cb0 }
		};
		if (loadOnly || !isSoundActive(0x0c28)) {
			const MusicLoad load = {
				kClearMusicIdentity, -1, 0xc0, 0x60, true,
				channels, ARRAYSIZE(channels)
			};
			applyMusicLoad(load);
		}
		break;
	}
	case 18:
		command2();
		if (_musicIndex >= 0 && _musicIndex != 18)
			return executeCommand(_musicIndex, false);
		break;
	case 32: {
		static const ChannelLoad channels[] = {
			{ 0, 0x121c }, { 1, 0x12d8 }, { 2, 0x1405 },
			{ 3, 0x14b8 }, { 4, 0x14df }, { 5, 0x15c8 }
		};
		const MusicLoad load = {
			kStopAll, -1, -1, -1, true,
			channels, ARRAYSIZE(channels)
		};
		applyMusicLoad(load);
		break;
	}
	case 33: {
		static const ChannelLoad channels[] = {
			{ 0, 0x1644 }, { 1, 0x1724 }, { 2, 0x1790 },
			{ 3, 0x17b1 }, { 4, 0x1894 }, { 5, 0x18c4 }
		};
		const MusicLoad load = {
			kStopAll, -1, -1, -1, true,
			channels, ARRAYSIZE(channels)
		};
		applyMusicLoad(load);
		break;
	}
	default:
		break;
	}
	return 0;
}

const uint16 PSound4::_commandList[79] = {
	0x013c,0x05c3,0x048c,0x05ca,0x04b4,0x2ba8,0x04ca,0x0536,0x069b,
	0,0,0,0,0,0,0,
	0x2da2,0x2ec7,0x2ea8,
	0,0,0,0,0,
	0x2a62,0x2a70,0x2a7e,0x2a85,0x2b1e,0x2a8c,0x2ad1,0x2b02,
	0x2baa,0x2c02,0x2ba8,0x2c5a,0x2df8,0x2caa,0x2cfa,0x2e50,0x2d52,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,
	0x2a99,0x2aa0,0x2aa7,0x2aae,0x2ab5,0x2abc,0x2ac3,0x2aca,
	0x2ad8,0x2ae6,0x2aed,0x2af4,0x2afb,0x2b09,0x2b10
};

PSound4::PSound4(Audio::Mixer *mixer) :
		DragonspherePSound(mixer, kPSound4Data, 40, false) {
}

bool PSound4::callFunction(uint16 targetOffset, Channel &channel) {
	(void)targetOffset;
	(void)channel;
	return false;
}

int PSound4::executeCommand(int commandId, bool loadOnly) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList) ||
			!_commandList[commandId])
		return 0;
	if (commandId <= 8) {
		// This section replaces the normal command 5 handler with a RET.
		return commandId == 5 ? 0 : dispatchBaseCommand(commandId);
	}

	static const uint16 effectCommands24[] = {
		0x0d2c, 0x0d38, 0x0d46, 0x0d52, 0x0d68,
		0x0d70, 0x0d60, 0x0d7c, 0x0da4, 0x0eb2
	};
	static const byte effectCommandStarts24[] = {
		0, 2, 4, 5, 6, 7, 8, 9, 10
	};

	if (commandId >= 24 && commandId <= 31) {
		const uint first = effectCommandStarts24[commandId - 24];
		const uint end = effectCommandStarts24[commandId - 23];
		if (commandId != 29 || !isSoundActive(effectCommands24[first]))
			playSounds(effectCommands24 + first, end - first);
		return 0;
	}
	if (commandId >= 64) {
		static const uint16 effects[][2] = {
			{ 0x0dd1, 0 }, { 0x0ded, 0 }, { 0x0df7, 0 }, { 0x0e03, 0 },
			{ 0x0db4, 0 }, { 0x0e0b, 0 }, { 0x0e2b, 0 }, { 0x0e58, 0 },
			{ 0x0e60, 0x0e72 }, { 0x0e72, 0 }, { 0x0e7a, 0 },
			{ 0x0e82, 0 }, { 0x0e8a, 0 }, { 0x0f0d, 0 },
			{ 0x0f15, 0x0f23 }
		};
		const uint index = commandId - 64;
		playSound(effects[index][0]);
		if (effects[index][1])
			playSound(effects[index][1]);
		return 0;
	}

	switch (commandId) {
	case 16: {
		static const ChannelLoad channels[] = {
			{ 0, 0x1c7f }, { 1, 0x1cd1 }, { 2, 0x1d0c },
			{ 3, 0x1d57 }, { 4, 0x1c78 }
		};
		const MusicLoad load = {
			kStopMusic, 0x10, 0xc0, 0xc0, true,
			channels, ARRAYSIZE(channels)
		};
		startOrDeferMusicWhenActive(commandId, 0x1c7f, load, loadOnly);
		break;
	}
	case 17: {
		static const ChannelLoad channels[] = {
			{ 0, 0x2778 }, { 1, 0x27ac }, { 2, 0x27e1 }, { 3, 0x2800 }
		};
		if (loadOnly || !isSoundActive(0x2778)) {
			const MusicLoad load = {
				kClearMusicIdentity, -1, 0xc0, 0x60, true,
				channels, ARRAYSIZE(channels)
			};
			applyMusicLoad(load);
		}
		break;
	}
	case 18:
		command2();
		return executeCommand(_musicIndex <= 18 ? 16 : _musicIndex, false);
	case 32: {
		static const ChannelLoad channels[] = {
			{ 0, 0x0f42 }, { 1, 0x1010 }, { 2, 0x1208 },
			{ 3, 0x12b4 }, { 4, 0x1302 }, { 5, 0x137e }
		};
		const MusicLoad load = { kStopMusic, -1, 0x60, 0x60, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0x0f42, load, loadOnly);
		break;
	}
	case 33: {
		static const ChannelLoad channels[] = {
			{ 0, 0x140b }, { 1, 0x1435 }, { 2, 0x1464 },
			{ 3, 0x14a5 }, { 4, 0x158c }, { 5, 0x1404 }
		};
		const MusicLoad load = { kStopMusic, -1, 0xc0, 0xc0, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0x140b, load, loadOnly);
		break;
	}
	case 34:
		break;
	case 35: {
		static const ChannelLoad channels[] = {
			{ 0, 0x1646 }, { 1, 0x162a }, { 2, 0x1639 },
			{ 3, 0x16cc }, { 4, 0x16de }
		};
		const MusicLoad load = { kStopAll, -1, 0x54, 0x54, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0x1646, load, loadOnly);
		break;
	}
	case 36: {
		static const ChannelLoad channels[] = {
			{ 0, 0x1e39 }, { 1, 0x20a3 }, { 2, 0x20dd },
			{ 3, 0x20fe }, { 4, 0x1e2c }, { 5, 0x20d6 }
		};
		const MusicLoad load = { kStopAll, -1, 0x54, 0x54, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0x1e39, load, loadOnly);
		break;
	}
	case 37: {
		static const ChannelLoad channels[] = {
			{ 0, 0x1786 }, { 1, 0x17c8 }, { 2, 0x17f7 },
			{ 3, 0x1830 }, { 4, 0x1871 }
		};
		const MusicLoad load = { kStopAll, -1, 0x40, 0x40, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0x1786, load, loadOnly);
		break;
	}
	case 38: {
		static const ChannelLoad channels[] = {
			{ 0, 0x18a6 }, { 1, 0x18ec }, { 2, 0x1923 },
			{ 3, 0x1958 }, { 4, 0x199b }, { 5, 0x1ace }
		};
		const MusicLoad load = { kStopAll, -1, 0x40, 0x40, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0x18a6, load, loadOnly);
		break;
	}
	case 39: {
		static const ChannelLoad channels[] = {
			{ 0, 0x22d2 }, { 1, 0x234c }, { 2, 0x23bf },
			{ 3, 0x2484 }, { 4, 0x2543 }, { 5, 0x2604 }
		};
		const MusicLoad load = { kStopAll, -1, 0x48, 0x48, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0x22d2, load, loadOnly);
		break;
	}
	case 40: {
		static const ChannelLoad channels[] = {
			{ 0, 0x1c6f }, { 1, 0x1ccc }, { 2, 0x1d07 },
			{ 3, 0x1d57 }, { 4, 0x1c66 }
		};
		const MusicLoad load = { kStopMusic, -1, 0xc0, 0xc0, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0x1c66, load, loadOnly);
		break;
	}
	default:
		break;
	}
	return 0;
}

const uint16 PSound5::_commandList[79] = {
	0x013c,0x05c3,0x048c,0x05ca,0x04b4,0x05f2,0x04ca,0x0536,0x069b,
	0,0,0,0,0,0,0,
	0x2aee,0x2df9,0x2dda,
	0,0,0,0,0,
	0x2ca0,0x2cae,0x2cbc,0x2cc3,0x2d7a,0x2cca,0x2cd1,0x2d73,
	0x2d82,0x2b98,0x2bf0,0x2c48,0x2c99,0x2e9a,0x2e38,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0x2cd8,0x2cdf,0x2ce6,0x2ced,0x2cf4,0x2cfb,0x2d02,0x2d09,
	0x2d10,0x2d1e,0x2d25,0x2d33,0x2d3a,0x2d41,0x2d6c
};

PSound5::PSound5(Audio::Mixer *mixer) :
		DragonspherePSound(mixer, kPSound5Data, 38, true) {
}

void PSound5::loadAlternateMusic() {
	static const ChannelLoad channels[] = {
		{ 0, 0x0c87 }, { 1, 0x0dfe }, { 2, 0x0e98 },
		{ 3, 0x1038 }, { 4, 0x1226 }, { 5, 0x1438 }
	};
	const MusicLoad load = {
		kStopAll, 0x10, 0xc0, 0xc0, true,
		channels, ARRAYSIZE(channels)
	};
	applyMusicLoad(load);
}

bool PSound5::callFunction(uint16 targetOffset, Channel &channel) {
	(void)channel;
	if (targetOffset != 0x2b40)
		return false;
	if (musicChannelsActive())
		deferCommand(kPSound5AlternateMusic, true);
	else
		loadAlternateMusic();
	return true;
}

int PSound5::executeCommand(int commandId, bool loadOnly) {
	if (commandId == kPSound5AlternateMusic) {
		loadAlternateMusic();
		return 0;
	}
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList) ||
			!_commandList[commandId])
		return 0;
	if (commandId <= 8)
		return dispatchBaseCommand(commandId);

	static const uint16 effectCommands24[] = {
		0x1fce, 0x1fda, 0x1fe8, 0x1ff4, 0x200a,
		0x2012, 0x2002, 0x201e, 0x2046, 0x212d
	};
	static const byte effectCommandStarts24[] = {
		0, 2, 4, 5, 6, 7, 8, 9, 10
	};
	if (commandId >= 24 && commandId <= 31) {
		const uint first = effectCommandStarts24[commandId - 24];
		const uint end = effectCommandStarts24[commandId - 23];
		playSounds(effectCommands24 + first, end - first);
		return 0;
	}
	if (commandId >= 64) {
		static const uint16 effects[][2] = {
			{ 0x2069, 0 }, { 0x2073, 0 }, { 0x207b, 0 }, { 0x2089, 0 },
			{ 0x20d7, 0 }, { 0x20af, 0 }, { 0x20cf, 0 }, { 0x20d7, 0 },
			{ 0x20e1, 0x20ef }, { 0x20fd, 0 }, { 0x2105, 0x210f },
			{ 0x2119, 0 }, { 0x2121, 0 }, { 0, 0 }, { 0x212d, 0 }
		};
		if (commandId == 77) {
			Channel &channel = _channels[8];
			if (!_commandParam) {
				if (channel.loopStart == 0x2056)
					channel.innerLoopStart = _nullSequenceOffset;
			} else {
				writeDataByte(0x2061, (byte(_commandParam) >> 1) + 0x40);
				if (!isSoundActive(0x2056))
					loadChannel(8, 0x2056);
			}
			return 0;
		}
		const uint index = commandId - 64;
		playSound(effects[index][0]);
		if (effects[index][1])
			playSound(effects[index][1]);
		return 0;
	}

	switch (commandId) {
	case 16: {
		static const ChannelLoad channels[] = {
			{ 0, 0x0c7a }, { 1, 0x0dfa }, { 2, 0x0e84 },
			{ 3, 0x1024 }, { 4, 0x1218 }, { 5, 0x1434 }
		};
		const MusicLoad load = { kStopAll, -1, 0xc0, 0xc0, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 17: {
		static const ChannelLoad channels[] = {
			{ 0, 0x23b4 }, { 1, 0x23e8 }, { 2, 0x241d }, { 3, 0x243c }
		};
		if (loadOnly || !isSoundActive(0x23b4)) {
			const MusicLoad load = { kClearMusicIdentity, -1, 0xc0, 0x60, true,
				channels, ARRAYSIZE(channels) };
			applyMusicLoad(load);
		}
		break;
	}
	case 18:
		command2();
		return executeCommand(_musicIndex <= 18 ? 16 : _musicIndex, false);
	case 32: {
		static const ChannelLoad channels[] = {
			{ 0, 0x2188 }, { 1, 0x21d2 }, { 2, 0x2201 },
			{ 3, 0x222b }, { 4, 0x22b4 }, { 5, 0x2347 }
		};
		const MusicLoad load = { kStopAll, 0x10, 0x48, 0x48, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 33: {
		static const ChannelLoad channels[] = {
			{ 0, 0x14ea }, { 1, 0x1592 }, { 2, 0x1628 },
			{ 3, 0x17be }, { 4, 0x1862 }, { 5, 0x1900 }
		};
		const MusicLoad load = { kStopAll, 0x10, 0x60, 0x60, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 34: {
		static const ChannelLoad channels[] = {
			{ 0, 0x19a4 }, { 1, 0x1a24 }, { 2, 0x1ad2 },
			{ 3, 0x1baa }, { 4, 0x1c46 }, { 5, 0x1db8 }
		};
		const MusicLoad load = { kStopAll, 0x10, 0xc0, 0xc0, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 35: {
		static const ChannelLoad channels[] = {
			{ 0, 0x1e02 }, { 1, 0x1e21 }, { 2, 0x1e5d },
			{ 3, 0x1f37 }, { 4, 0x1f65 }
		};
		const MusicLoad load = { kStopAll, 0x10, 0xc0, 0xc0, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 36:
		loadChannel(3, 0x1f9d);
		break;
	case 37: {
		static const ChannelLoad channels[] = {
			{ 0, 0x2700 }, { 1, 0x2745 }, { 2, 0x277b }, { 3, 0x27b9 }
		};
		const MusicLoad load = { kStopAll, -1, -1, -1, false,
			channels, ARRAYSIZE(channels) };
		applyMusicLoad(load);
		break;
	}
	case 38: {
		static const ChannelLoad channels[] = {
			{ 0, 0x2470 }, { 1, 0x24be }, { 2, 0x24fc },
			{ 3, 0x25ba }, { 4, 0x25d2 }, { 5, 0x26e6 }
		};
		const MusicLoad load = { kStopAll, 0x10, 0xc0, 0xc0, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusic(commandId, 0x2470, load, loadOnly);
		break;
	}
	default:
		break;
	}
	return 0;
}

const uint16 PSound6::_commandList[99] = {
	0x013c,0x05c3,0x048c,0x05ca,0x04b4,0x05f2,0x04ca,0x0536,0x069b,
	0,0,0,0,0,0,0,
	0x2cb2,0x300f,0x2ff0,
	0,0,0,0,0,
	0x2b26,0x2b34,0x2b42,0x2b49,0x2caa,0x2b50,0x2c88,0x2c9c,
	0x2d22,0x2d6c,0x2ddf,0x2da4,0x2e30,0x2e8a,0x2ee2,0x2f3a,
	0x2f98,0x2b25,0x2b25,0x2b25,0x304e,0x30b2,0x2b25,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0x2b58,0x2c8e,0x2b83,0x2c95,0x2b8a,0x2b98,0x2b9f,0x2ba6,
	0x2bad,0x2bbb,0x2bc2,0x2bc9,0x2bd0,0x2bd7,0x2bde,0x2be5,
	0x2bec,0x2bfb,0x2c02,0x2c09,0x2c17,0x2c1e,0x2c25,0x2c2c,
	0x2c33,0x2c3a,0x2c41,0x2c48,0x2c4f,0x2c56,0x2c5d,0x2c64,
	0x2c6b,0x2c81,0x2ca3
};

PSound6::PSound6(Audio::Mixer *mixer) :
		DragonspherePSound(mixer, kPSound6Data, 46, true) {
}

bool PSound6::callFunction(uint16 targetOffset, Channel &channel) {
	(void)channel;
	if (targetOffset != 0x2e8a)
		return false;
	// The C4 target is command 37's entry point, including its guard and
	// deferral logic, rather than a loader-only callback.
	executeCommand(37, false);
	return true;
}

int PSound6::executeCommand(int commandId, bool loadOnly) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList) ||
			!_commandList[commandId])
		return 0;
	if (commandId <= 8)
		return dispatchBaseCommand(commandId);

	static const uint16 effectCommands24[] = {
		0x0cbc, 0x0cc8, 0x0cd6, 0x0ce2, 0x0cf8,
		0x0d00, 0x0cf0, 0x0d0c, 0x0d75, 0x0f5b
	};
	static const byte effectCommandStarts24[] = {
		0, 2, 4, 5, 6, 7, 8, 9, 10
	};
	if (commandId >= 24 && commandId <= 31) {
		const uint first = effectCommandStarts24[commandId - 24];
		const uint end = effectCommandStarts24[commandId - 23];
		playSounds(effectCommands24 + first, end - first);
		return 0;
	}
	if (commandId >= 64) {
		if (commandId == 64) {
			Channel &channel = _channels[8];
			if (!_commandParam) {
				if (channel.loopStart == 0x0d34)
					channel.innerLoopStart = _nullSequenceOffset;
			} else {
				writeDataByte(0x0d3f, (byte(_commandParam) >> 1) + 0x40);
				if (!isSoundActive(0x0d34))
					loadChannel(8, 0x0d34);
			}
			return 0;
		}
		if (commandId == 80) {
			Channel *active = findActiveSound(0x0e6a);
			if (active)
				active->outerLoopCount = 1;
			return 0;
		}
		static const uint16 effects[][3] = {
			{ 0, 0, 0 }, { 0x0f3b, 0, 0 }, { 0x0d47, 0, 0 },
			{ 0x0f47, 0, 0 }, { 0x0d51, 0x0d63, 0 }, { 0x0d85, 0, 0 },
			{ 0x0d97, 0, 0 }, { 0x0db3, 0, 0 }, { 0x0dff, 0x0ddc, 0 },
			{ 0x0e2a, 0, 0 }, { 0x0e38, 0, 0 }, { 0x0d6b, 0, 0 },
			{ 0x0e42, 0, 0 }, { 0x0e56, 0, 0 }, { 0x0e60, 0, 0 },
			{ 0x0e6a, 0, 0 }, { 0, 0, 0 }, { 0x0e7c, 0, 0 },
			{ 0x0e84, 0, 0 }, { 0x0dbb, 0x0ddc, 0 }, { 0x0e8c, 0, 0 },
			{ 0x0e94, 0, 0 }, { 0x0ea0, 0, 0 }, { 0x0ea8, 0, 0 },
			{ 0x0eb8, 0, 0 }, { 0x0ec0, 0, 0 }, { 0x0ec8, 0, 0 },
			{ 0x0ed4, 0, 0 }, { 0x0edc, 0, 0 }, { 0x0ee6, 0, 0 },
			{ 0x0eee, 0, 0 }, { 0x0ef6, 0, 0 },
			{ 0x0f18, 0x0f18, 0x0f18 }, { 0x0f2f, 0, 0 },
			{ 0x0fb6, 0, 0 }
		};
		const uint index = commandId - 64;
		for (uint i = 0; i < 3 && effects[index][i]; ++i)
			playSound(effects[index][i]);
		return 0;
	}

	switch (commandId) {
	case 16: {
		if (!loadOnly && (isSoundActive(0x0fdc) ||
				isSoundActive(0x1675) || isSoundActive(0x1850)))
			break;
		static const ChannelLoad channels[] = {
			{ 0, 0x0fdc }, { 1, 0x107e }, { 2, 0x10c4 },
			{ 3, 0x0fce }, { 4, 0x1072 }, { 5, 0x0fc2 }
		};
		const MusicLoad load = { kStopAll, 0x10, 0xc8, 0xc8, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 17: {
		static const ChannelLoad channels[] = {
			{ 0, 0x2a3e }, { 1, 0x2a72 }, { 2, 0x2aa7 }, { 3, 0x2ac6 }
		};
		if (loadOnly || !isSoundActive(0x2a3e)) {
			const MusicLoad load = { kClearMusicIdentity, -1, 0xc0, 0x60, true,
				channels, ARRAYSIZE(channels) };
			applyMusicLoad(load);
		}
		break;
	}
	case 18:
		command2();
		return executeCommand(_musicIndex <= 18 ? 16 : _musicIndex, false);
	case 32: {
		static const ChannelLoad channels[] = {
			{ 0, 0x1120 }, { 1, 0x1168 }, { 2, 0x11a9 }, { 3, 0x11f2 }
		};
		const MusicLoad load = { kStopAll, -1, 0x3c, 0x3c, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusic(commandId, 0x1120, load, loadOnly);
		break;
	}
	case 33: {
		static const ChannelLoad channels[] = {
			{ 0, 0x1278 }, { 1, 0x1307 }, { 2, 0x1357 },
			{ 3, 0x137b }, { 4, 0x13c9 }, { 5, 0x1382 }
		};
		if (loadOnly || !isSoundActive(0x1278)) {
			const MusicLoad load = { kStopAll, -1, -1, -1, false,
				channels, ARRAYSIZE(channels) };
			applyMusicLoad(load);
		}
		break;
	}
	case 34: {
		static const ChannelLoad channels[] = {
			{ 0, 0x149f }, { 1, 0x150e }, { 2, 0x154d },
			{ 3, 0x15da }, { 4, 0x14e9 }
		};
		const MusicLoad load = { kStopAll, -1, 0x1e, 0x1e, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusic(commandId, 0x149f, load, loadOnly);
		break;
	}
	case 35: {
		static const ChannelLoad channels[] = {
			{ 0, 0x149c }, { 1, 0x14e9 }
		};
		const MusicLoad load = { kStopAll, -1, 0x1e, 0x1e, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusic(commandId, 0x149c, load, loadOnly);
		break;
	}
	case 36: {
		if (!loadOnly && (isSoundActive(0x161a) || isSoundActive(0x1850)))
			break;
		static const ChannelLoad channels[] = {
			{ 0, 0x161a }, { 1, 0x1675 }, { 2, 0x1715 },
			{ 3, 0x178e }, { 4, 0x17b5 }
		};
		const MusicLoad load = { kStopAll, -1, 0xc8, 0xc8, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 37: {
		static const ChannelLoad channels[] = {
			{ 0, 0x1850 }, { 1, 0x18a4 }, { 2, 0x18ff },
			{ 3, 0x1a06 }, { 4, 0x1b09 }, { 5, 0x1906 }
		};
		const MusicLoad load = { kStopAll, -1, 0xc8, 0xc8, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusic(commandId, 0x1850, load, loadOnly);
		break;
	}
	case 38: {
		static const ChannelLoad channels[] = {
			{ 0, 0x1b50 }, { 1, 0x1ba4 }, { 2, 0x1bf6 },
			{ 3, 0x1c80 }, { 4, 0x1d37 }, { 5, 0x1f20 }
		};
		const MusicLoad load = { kStopAll, -1, 0xc8, 0xc8, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusic(commandId, 0x1b50, load, loadOnly);
		break;
	}
	case 39: {
		static const ChannelLoad channels[] = {
			{ 0, 0x1f70 }, { 1, 0x20d1 }, { 2, 0x22dd },
			{ 3, 0x2369 }, { 4, 0x23ab }, { 5, 0x2370 }, { 8, 0x24d3 }
		};
		const MusicLoad load = { kStopAll, -1, 0x64, 0x64, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusic(commandId, 0x1f70, load, loadOnly);
		break;
	}
	case 40: {
		static const ChannelLoad channels[] = {
			{ 0, 0x2500 }, { 1, 0x25dc }, { 2, 0x26c2 },
			{ 3, 0x27b4 }, { 4, 0x28da }, { 5, 0x29da }
		};
		const MusicLoad load = { kStopAll, -1, 0x64, 0x64, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusic(commandId, 0x2500, load, loadOnly);
		break;
	}
	case 44: {
		static const ChannelLoad channels[] = {
			{ 0, 0x2afa }, { 1, 0x2b5a }, { 2, 0x2b82 },
			{ 3, 0x2bb2 }, { 4, 0x2be2 }, { 5, 0x2c44 }, { 8, 0x2b08 }
		};
		const MusicLoad load = { kStopAll, 0x10, 0x30, 0x30, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusic(commandId, 0x2b0f, load, loadOnly);
		break;
	}
	case 45: {
		static const ChannelLoad channels[] = {
			{ 0, 0x2c7e }, { 1, 0x2c8e }
		};
		const MusicLoad load = { kStopAll, -1, 0x1e, 0x1e, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusic(commandId, 0x2c99, load, loadOnly);
		break;
	}
	default:
		break;
	}
	return 0;
}

const uint16 PSound9::_commandList[64] = {
	0x013c,0x05c3,0x048c,0x05ca,0x04b4,0x05f2,0x04ca,0x0536,0x06b9,
	0,0,0,0,0,0,0,
	0x2af1,0x2fa0,0x2fae,
	0,0,0,0,0,
	0x2fa0,0x2fae,0x2fbc,0x2fc3,0x2ff4,0x2fdf,0x2fe6,0x2af1,
	0x2b82,0x2da4,0x2dfc,0x2e50,0x2ea4,0x2bd0,0x2c1e,0x2ef8,
	0x2f4c,0x2c6c,0x2d06,0x3030,0x2af1,0x2ff6,0x3013,0x2da4,
	0x2fca,0x2fd1,0x2fd8,0x3186,0x3146,0x2a3e,0x2dfc,0x2af2,
	0x2af1,0x3094,0x2b4a,0x3074,0x2af1,0x2fed,0x30f2,0x313f
};

PSound9::PSound9(Audio::Mixer *mixer) :
		DragonspherePSound(mixer, kPSound9Data, 63, false) {
}

bool PSound9::callFunction(uint16 targetOffset, Channel &channel) {
	(void)targetOffset;
	(void)channel;
	return false;
}

int PSound9::executeCommand(int commandId, bool loadOnly) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList) ||
			!_commandList[commandId])
		return 0;
	if (commandId <= 8)
		return dispatchBaseCommand(commandId);

	if (commandId == 17 || commandId == 24) {
		static const uint16 sounds[] = { 0x4b7a, 0x4b86 };
		playSounds(sounds, ARRAYSIZE(sounds));
		return 0;
	}
	if (commandId == 18 || commandId == 25) {
		static const uint16 sounds[] = { 0x4b94, 0x4ba0 };
		playSounds(sounds, ARRAYSIZE(sounds));
		return 0;
	}
	switch (commandId) {
	case 26: playSound(0x4bae); return 0;
	case 27: playSound(0x4bb6); return 0;
	case 28: return 0;
	case 29: playSound(0x4bc2); return 0;
	case 30: playSound(0x4bea); return 0;
	case 31: return 0;
	case 45: {
		static const uint16 sounds[] = { 0x4c2b, 0x4c9c, 0x4c1e, 0x4c8f };
		playSounds(sounds, ARRAYSIZE(sounds), true);
		return 0;
	}
	case 46: {
		static const uint16 sounds[] = { 0x4cf8, 0x4d5e, 0x4ceb, 0x4d51 };
		playSounds(sounds, ARRAYSIZE(sounds), true);
		return 0;
	}
	case 48: playSound(0x4bfa); return 0;
	case 49: playSound(0x4c04); return 0;
	case 50: playSound(0x4c0c); return 0;
	case 61: playSound(0x4c16); return 0;
	default: break;
	}

	switch (commandId) {
	case 16:
	case 44:
	case 56:
	case 60:
		break;
	case 32: {
		static const ChannelLoad channels[] = {
			{ 0, 0x13e4 }, { 1, 0x1410 }, { 2, 0x14ca },
			{ 3, 0x1502 }, { 4, 0x1638 }, { 5, 0x172c }
		};
		const MusicLoad load = { kStopAll, -1, 0x62, 0x54, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 33:
	case 47: {
		if (!loadOnly) {
			writeDataByte(0x2ace, 0x17);
			writeDataByte(0x2ae2, 0x17);
			writeDataByte(0x2be6, 0x17);
			writeDataByte(0x2bec, 0x17);
			writeDataByte(0x2a02, 0x2f);
			writeDataByte(0x2a18, 0x2f);
			writeDataByte(0x2a6a, 0x28);
			writeDataByte(0x2a80, 0x28);
		}
		static const ChannelLoad channels[] = {
			{ 0, 0x29f6 }, { 1, 0x2a5e }, { 2, 0x2ac6 },
			{ 3, 0x2b82 }, { 4, 0x2be0 }, { 5, 0x2c3a }, { 6, 0x2d9c }
		};
		const MusicLoad load = { kStopAll, -1, 0x62, 0x54, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 34:
	case 54: {
		static const ChannelLoad channels[] = {
			{ 0, 0x2e78 }, { 1, 0x3072 }, { 2, 0x3279 },
			{ 3, 0x347c }, { 4, 0x35e5 }, { 5, 0x36ea }, { 6, 0x3741 }
		};
		const MusicLoad load = { kStopAll, -1, 0x38, 0x38, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 35: {
		static const ChannelLoad channels[] = {
			{ 0, 0x37ca }, { 1, 0x382b }, { 2, 0x3883 },
			{ 3, 0x3949 }, { 4, 0x39b9 }, { 5, 0x3a15 }, { 6, 0x3ae1 }
		};
		const MusicLoad load = { kStopAll, -1, 0x50, 0x50, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 36: {
		static const ChannelLoad channels[] = {
			{ 0, 0x3c48 }, { 1, 0x3cbc }, { 2, 0x3d36 },
			{ 3, 0x3d6a }, { 4, 0x402b }, { 5, 0x4095 }, { 6, 0x40f3 }
		};
		const MusicLoad load = { kStopAll, -1, 0x28, 0x28, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 37: {
		static const ChannelLoad channels[] = {
			{ 0, 0x17b6 }, { 1, 0x1838 }, { 2, 0x18be },
			{ 3, 0x1902 }, { 4, 0x19d4 }, { 5, 0x1a39 }
		};
		const MusicLoad load = { kStopAll, -1, 0x50, 0x50, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 38: {
		static const ChannelLoad channels[] = {
			{ 0, 0x1b72 }, { 1, 0x1bec }, { 2, 0x1c6c },
			{ 3, 0x1c9e }, { 4, 0x1ca4 }, { 5, 0x1d06 }
		};
		const MusicLoad load = { kStopAll, -1, 0x28, 0x28, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 39: {
		static const ChannelLoad channels[] = {
			{ 0, 0x412c }, { 1, 0x41b4 }, { 2, 0x4244 },
			{ 3, 0x4336 }, { 4, 0x47b8 }, { 5, 0x482e }, { 6, 0x4860 }
		};
		const MusicLoad load = { kStopAll, -1, 0x28, 0x28, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 40: {
		static const ChannelLoad channels[] = {
			{ 0, 0x2e78 }, { 1, 0x3072 }, { 2, 0x3279 },
			{ 3, 0x495a }, { 4, 0x4a4c }, { 5, 0x4ae8 }, { 6, 0x4b3c }
		};
		const MusicLoad load = { kStopAll, -1, 0x38, 0x38, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 41: {
		static const ChannelLoad channels[] = {
			{ 0, 0x1d68 }, { 1, 0x2295 }, { 2, 0x2410 },
			{ 3, 0x24a5 }, { 4, 0x256a }, { 5, 0x27f3 }
		};
		const MusicLoad load = { kStopAll, -1, 0x54, 0x54, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 42: {
		static const ChannelLoad channels[] = {
			{ 0, 0x1dac }, { 6, 0x1df2 }, { 2, 0x230c },
			{ 3, 0x244e }, { 4, 0x24dc }, { 5, 0x264d }, { 1, 0x2890 }
		};
		const MusicLoad load = { kStopAll, -1, 0xa8, 0x50, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 43: {
		static const ChannelLoad channels[] = {
			{ 0, 0x4db8 }, { 1, 0x4e31 }, { 2, 0x4e99 },
			{ 3, 0x50a6 }, { 4, 0x50cc }, { 5, 0x50f2 }, { 6, 0x512a }
		};
		const MusicLoad load = { kKeepPlayback, -1, 0x60, 0x60, true,
			channels, ARRAYSIZE(channels) };
		applyMusicLoad(load);
		break;
	}
	case 51: {
		static const ChannelLoad channels[] = {
			{ 0, 0x600a }, { 1, 0x60a1 }, { 2, 0x612a },
			{ 3, 0x6180 }, { 4, 0x6177 }
		};
		const MusicLoad load = { kStopAll, -1, 0x60, 0x60, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 52: {
		static const ChannelLoad channels[] = {
			{ 0, 0x5b71 }, { 1, 0x5ddb }, { 2, 0x5e15 },
			{ 3, 0x5e36 }, { 4, 0x5b64 }, { 5, 0x5e0e }
		};
		const MusicLoad load = { kStopAll, -1, 0x54, 0x54, true,
			channels, ARRAYSIZE(channels) };
		applyMusicLoad(load);
		break;
	}
	case 53:
		_callbackCounter = 0x4b0;
		_callbackPeriod = 0x4b0;
		deferCommand(1, false);
		break;
	case 55: {
		static const ChannelLoad channels[] = {
			{ 0, 0x0e7a }, { 1, 0x0f38 }, { 2, 0x112b },
			{ 3, 0x11cb }, { 4, 0x11e5 }, { 5, 0x1259 }
		};
		const MusicLoad load = { kStopAll, -1, 0x60, 0x60, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0x0e7a, load, loadOnly);
		break;
	}
	case 57: {
		static const ChannelLoad channels[] = {
			{ 0, 0x51f4 }, { 3, 0x526d }, { 2, 0x52b0 },
			{ 1, 0x5324 }, { 4, 0x536c }, { 5, 0x53f2 }, { 8, 0x5206 }
		};
		const MusicLoad load = { kStopAll, -1, 0x30, 0x30, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusic(commandId, 0x526d, load, loadOnly);
		break;
	}
	case 58: {
		if (!loadOnly && isSoundActive(0x12d8))
			break;
		static const ChannelLoad channels[] = {
			{ 0, 0x12d8 }, { 1, 0x1309 }, { 2, 0x1356 },
			{ 3, 0x13a9 }, { 4, 0x13c8 }, { 5, 0x13d6 }
		};
		resetSection();
		loadChannels(channels, ARRAYSIZE(channels));
		break;
	}
	case 59: {
		static const ChannelLoad channels[] = {
			{ 1, 0x5160 }, { 2, 0x518a }, { 3, 0x51a9 }, { 4, 0x51d5 }
		};
		resetSection();
		loadChannels(channels, ARRAYSIZE(channels));
		break;
	}
	case 62: {
		static const ChannelLoad channels[] = {
			{ 0, 0x5459 }, { 1, 0x5653 }, { 2, 0x5858 },
			{ 3, 0x544e }, { 4, 0x5a59 }, { 5, 0x5ab0 }
		};
		const MusicLoad load = { kStopAll, -1, 0x38, 0x38, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 63:
		loadChannel(6, 0x5b39);
		break;
	default:
		break;
	}
	return 0;
}

const uint16 PSoundDemo1::_commandList[89] = {
	0x23f6,0x287d,0x2746,0x2884,0x276e,0x28ac,0x2784,0x27f0,0x2955,
	0,0,0,0,0,0,0,
	0x21ea,
	0,0,0,0,0,0,0,
	0x22a0,0x22ae,0x22bc,0x22c3,0x22ca,0x22cb,
	0,0,
	0x2218,0x226a,0x22d4,0x2e20,0x2bfe,0x2c4a,0x2c8e,0x2d2c,
	0x2d6a,0x2ce0,0x2b8e,0x2e6c,0x2ebe,0x2dd4,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0x22d3,0x22d3,0x22d3,0x22d3,0x22d3,0x22d3,0x22d3,0x22d3,
	0x22d3,0x22d3,0x22d3,0x22d3,0x22d3,0x22d3,0x22d3,0x22d3,
	0x22d3,0x22d3,0x22d3,0x22d3,0x22d3,0x22d3,0x22d3,0x22d3,
	0x22d3
};

PSoundDemo1::PSoundDemo1(Audio::Mixer *mixer) :
		DragonspherePSoundDemo(mixer, kPSoundDemo1Data, true) {
}

bool PSoundDemo1::callFunction(uint16 targetOffset, Channel &channel) {
	(void)channel;
	if (targetOffset != 0x2dbc)
		return false;
	_callbackCounter = 0xc0;
	_callbackPeriod = 0x60;
	deferCommand(16, false);
	return true;
}

int PSoundDemo1::executeCommand(int commandId, bool loadOnly) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList) ||
			!_commandList[commandId])
		return 0;
	if (commandId <= 8)
		return dispatchBaseCommand(commandId);
	if (commandId >= 64)
		return 0;
	if (commandId >= 24 && commandId <= 29) {
		static const uint16 effects[][2] = {
			{ 0x06dc, 0x06e8 }, { 0x06f6, 0x0702 }, { 0x0710, 0 },
			{ 0x0718, 0 }, { 0, 0 }, { 0x0724, 0 }
		};
		const uint index = commandId - 24;
		if (effects[index][0])
			playSound(effects[index][0]);
		if (effects[index][1])
			playSound(effects[index][1]);
		return 0;
	}

	switch (commandId) {
	case 16: {
		static const ChannelLoad channels[] = {
			{ 0, 0x012c }, { 1, 0x01d5 }, { 2, 0x0278 },
			{ 3, 0x0352 }, { 4, 0x03a7 }, { 5, 0x03b4 }
		};
		const MusicLoad load = { kStopAll, -1, -1, -1, false,
			channels, ARRAYSIZE(channels) };
		applyMusicLoad(load);
		break;
	}
	case 32: {
		static const ChannelLoad channels[] = {
			{ 0, 0x03d1 }, { 6, 0x0417 }, { 2, 0x0490 },
			{ 3, 0x0487 }, { 4, 0x04b9 }, { 5, 0x03c4 }
		};
		const MusicLoad load = { kStopAll, -1, 0xb0, 0xb0, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 33: {
		static const ChannelLoad channels[] = {
			{ 6, 0x04e4 }, { 1, 0x05c8 }, { 2, 0x062d },
			{ 3, 0x0661 }, { 4, 0x065a }, { 5, 0x05bb }, { 0, 0x0569 }
		};
		const MusicLoad load = { kStopAll, -1, -1, -1, false,
			channels, ARRAYSIZE(channels) };
		applyMusicLoad(load);
		break;
	}
	case 34: {
		static const ChannelLoad channels[] = {
			{ 0, 0x074c }, { 1, 0x07c1 }, { 2, 0x0834 },
			{ 3, 0x08c7 }, { 4, 0x093c }, { 5, 0x078d }
		};
		const MusicLoad load = { kStopAll, -1, -1, -1, false,
			channels, ARRAYSIZE(channels) };
		applyMusicLoad(load);
		break;
	}
	case 35: {
		static const ChannelLoad channels[] = {
			{ 0, 0x443c }, { 1, 0x44d3 }, { 2, 0x455c },
			{ 3, 0x45b2 }, { 4, 0x45a9 }
		};
		const MusicLoad load = { kStopAll, -1, 0x60, 0x60, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 36: {
		static const ChannelLoad channels[] = {
			{ 0, 0x09b4 }, { 1, 0x0a87 }, { 2, 0x0b7e },
			{ 3, 0x0c13 }, { 4, 0x0a7a }
		};
		const MusicLoad load = { kStopAll, -1, 0x80, 0x80, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 37: {
		static const ChannelLoad channels[] = {
			{ 0, 0x0c8a }, { 1, 0x0cf9 }, { 2, 0x0cec }, { 3, 0x0ce1 }
		};
		const MusicLoad load = { kStopAll, -1, 0xc0, 0xc0, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 38: {
		static const ChannelLoad channels[] = {
			{ 0, 0x2d62 }, { 1, 0x2e24 }, { 2, 0x2f1a },
			{ 3, 0x3013 }, { 4, 0x2d70 }, { 5, 0x2f23 }
		};
		const MusicLoad load = { kStopAll, -1, 0x60, 0x60, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 39: {
		static const ChannelLoad channels[] = {
			{ 0, 0x336e }, { 1, 0x33c2 }, { 2, 0x3423 }
		};
		const MusicLoad load = { kStopAll, -1, 0xb0, 0xb0, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 40: {
		static const ChannelLoad channels[] = {
			{ 0, 0x34c0 }, { 1, 0x36a0 }, { 2, 0x37f1 },
			{ 3, 0x3a88 }, { 4, 0x3c81 }, { 5, 0x37ea }
		};
		const MusicLoad load = { kStopAll, -1, 0xa8, 0xa8, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 41: {
		static const ChannelLoad channels[] = {
			{ 0, 0x3104 }, { 1, 0x3178 }, { 2, 0x31ef },
			{ 3, 0x3246 }, { 4, 0x331f }
		};
		const MusicLoad load = { kStopAll, -1, 0x90, 0x90, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 42:
		break;
	case 43: {
		static const ChannelLoad channels[] = {
			{ 0, 0x45de }, { 1, 0x4665 }, { 2, 0x467e },
			{ 3, 0x4699 }, { 4, 0x46b6 }, { 5, 0x4624 }
		};
		const MusicLoad load = { kStopAll, -1, 0x50, 0x50, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 44: {
		static const ChannelLoad channels[] = {
			{ 0, 0x46d6 }, { 1, 0x4728 }, { 2, 0x4767 },
			{ 3, 0x478f }, { 4, 0x47f5 }, { 5, 0x48bf }
		};
		const MusicLoad load = { kStopAll, -1, 0x60, 0xe0, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 45: {
		static const ChannelLoad channels[] = {
			{ 0, 0x3f1a }, { 1, 0x4048 }, { 2, 0x40f6 },
			{ 3, 0x41d7 }, { 4, 0x4297 }
		};
		const MusicLoad load = { kStopAll, -1, 0x60, 0x60, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	default:
		break;
	}
	return 0;
}

const uint16 PSoundDemo9::_commandList[51] = {
	0x0266,0x06ed,0x05b6,0x06f4,0x05de,0x071c,0x05f4,0x0660,0x07e3,
	0,0,0,0,0,0,0,
	0x0a46,
	0,0,0,0,0,0,0,
	0x0132,0x0140,0x014e,0x0155,0x0171,0x0171,
	0,0,
	0x0048,0x2cce,0x2d22,0x2d76,0x2dca,0x0096,0x00e4,0x2e1e,
	0x2e72,0x2be0,0x2c7a,0x2f2c,0x2f00,0x2ec6,0x2ee3,0x2cce,
	0x015c,0x0163,0x016a
};

PSoundDemo9::PSoundDemo9(Audio::Mixer *mixer) :
		DragonspherePSoundDemo(mixer, kPSoundDemo9Data, false) {
}

bool PSoundDemo9::callFunction(uint16 targetOffset, Channel &channel) {
	(void)targetOffset;
	(void)channel;
	return false;
}

int PSoundDemo9::executeCommand(int commandId, bool loadOnly) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList) ||
			!_commandList[commandId])
		return 0;
	if (commandId <= 8)
		return dispatchBaseCommand(commandId);
	if (commandId >= 24 && commandId <= 29) {
		static const uint16 effects[][2] = {
			{ 0x09d6, 0x09e2 }, { 0x09f0, 0x09fc }, { 0x0a0a, 0 },
			{ 0x0a12, 0 }, { 0, 0 }, { 0, 0 }
		};
		const uint index = commandId - 24;
		if (effects[index][0])
			playSound(effects[index][0]);
		if (effects[index][1])
			playSound(effects[index][1]);
		return 0;
	}
	switch (commandId) {
	case 45: {
		static const uint16 sounds[] = { 0x5981, 0x59f2, 0x5974, 0x59e5 };
		playSounds(sounds, ARRAYSIZE(sounds), true);
		return 0;
	}
	case 46: {
		static const uint16 sounds[] = { 0x5a4e, 0x5ab4, 0x5a41, 0x5aa7 };
		playSounds(sounds, ARRAYSIZE(sounds), true);
		return 0;
	}
	case 48: playSound(0x0a20); return 0;
	case 49: playSound(0x0a2a); return 0;
	case 50: playSound(0x0a32); return 0;
	default: break;
	}

	switch (commandId) {
	case 16:
		break;
	case 32: {
		static const ChannelLoad channels[] = {
			{ 0, 0x0050 }, { 1, 0x007c }, { 2, 0x0136 },
			{ 3, 0x016e }, { 4, 0x02a4 }, { 5, 0x0398 }
		};
		const MusicLoad load = { kStopAll, -1, 0x62, 0x54, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 33:
	case 47: {
		static const ChannelLoad channels[] = {
			{ 0, 0x37f2 }, { 1, 0x385a }, { 2, 0x38c2 },
			{ 3, 0x397e }, { 4, 0x39dc }, { 5, 0x3a36 }, { 6, 0x3b98 }
		};
		const MusicLoad load = { kStopAll, -1, 0x62, 0x54, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 34: {
		static const ChannelLoad channels[] = {
			{ 0, 0x3c74 }, { 1, 0x3e6e }, { 2, 0x4075 },
			{ 3, 0x4278 }, { 4, 0x43e1 }, { 5, 0x44e6 }, { 6, 0x453d }
		};
		const MusicLoad load = { kStopAll, -1, 0x38, 0x38, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 35: {
		static const ChannelLoad channels[] = {
			{ 0, 0x45c6 }, { 1, 0x4627 }, { 2, 0x467f },
			{ 3, 0x4745 }, { 4, 0x47b5 }, { 5, 0x4811 }, { 6, 0x48dd }
		};
		const MusicLoad load = { kStopAll, -1, 0x50, 0x50, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 36: {
		static const ChannelLoad channels[] = {
			{ 0, 0x4a44 }, { 1, 0x4ab8 }, { 2, 0x4b32 },
			{ 3, 0x4b66 }, { 4, 0x4e27 }, { 5, 0x4e91 }, { 6, 0x4eef }
		};
		const MusicLoad load = { kStopAll, -1, 0x28, 0x28, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 37: {
		static const ChannelLoad channels[] = {
			{ 0, 0x0422 }, { 1, 0x04a4 }, { 2, 0x052a },
			{ 3, 0x056e }, { 4, 0x0640 }, { 5, 0x06a5 }
		};
		const MusicLoad load = { kStopAll, -1, 0x50, 0x50, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 38: {
		static const ChannelLoad channels[] = {
			{ 0, 0x07de }, { 1, 0x0858 }, { 2, 0x08d8 },
			{ 3, 0x090a }, { 4, 0x0910 }, { 5, 0x0972 }
		};
		const MusicLoad load = { kStopAll, -1, 0x28, 0x28, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 39: {
		static const ChannelLoad channels[] = {
			{ 0, 0x4f28 }, { 1, 0x4fb0 }, { 2, 0x5040 },
			{ 3, 0x5132 }, { 4, 0x55b4 }, { 5, 0x562a }, { 6, 0x565c }
		};
		const MusicLoad load = { kStopAll, -1, 0x28, 0x28, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 40: {
		static const ChannelLoad channels[] = {
			{ 0, 0x3c74 }, { 1, 0x3e6e }, { 2, 0x4075 },
			{ 3, 0x5756 }, { 4, 0x5848 }, { 5, 0x58e4 }, { 6, 0x5938 }
		};
		const MusicLoad load = { kStopAll, -1, 0x38, 0x38, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 41: {
		static const ChannelLoad channels[] = {
			{ 0, 0x2b64 }, { 1, 0x3091 }, { 2, 0x320c },
			{ 3, 0x32a1 }, { 4, 0x3366 }, { 5, 0x35ef }
		};
		const MusicLoad load = { kStopAll, -1, 0x54, 0x54, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 42: {
		static const ChannelLoad channels[] = {
			{ 0, 0x2ba8 }, { 6, 0x2bee }, { 2, 0x3108 },
			{ 3, 0x324a }, { 4, 0x32d8 }, { 5, 0x3449 }, { 1, 0x368c }
		};
		const MusicLoad load = { kStopAll, -1, 0xa8, 0x50, true,
			channels, ARRAYSIZE(channels) };
		startOrDeferMusicWhenActive(commandId, 0, load, loadOnly);
		break;
	}
	case 43: {
		static const ChannelLoad channels[] = {
			{ 0, 0x5bce }, { 1, 0x5c36 }, { 2, 0x5c98 },
			{ 3, 0x5e9d }, { 4, 0x5ebb }, { 5, 0x5edb }, { 6, 0x5f13 }
		};
		const MusicLoad load = { kKeepPlayback, -1, 0x60, 0x60, true,
			channels, ARRAYSIZE(channels) };
		applyMusicLoad(load);
		break;
	}
	case 44: {
		static const ChannelLoad channels[] = {
			{ 0, 0x5b0e }, { 1, 0x5b5a }, { 2, 0x5b88 }
		};
		const MusicLoad load = { kStopAll, -1, 0xc0, 0xc0, true,
			channels, ARRAYSIZE(channels) };
		applyMusicLoad(load);
		break;
	}
	default:
		break;
	}
	return 0;
}

} // namespace Sound
} // namespace Dragonsphere
} // namespace MADS
