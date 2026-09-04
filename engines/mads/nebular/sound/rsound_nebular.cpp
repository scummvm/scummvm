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

#include "mads/nebular/sound/rsound_nebular.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

RSoundDemo::RSoundDemo(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver, const Common::Path &filename,
		int dataOffset, int dataSize, int sysExOffset,
		int firstEffectChannel) :
		RSound(mixer, midiDriver, filename, dataOffset, dataSize, sysExOffset,
				kRSoundFadeCheckAlternating),
		_firstEffectChannel(firstEffectChannel) {
}

void RSoundDemo::startVoice(int channelIndex, int sequenceOffset) {
	assert(channelIndex >= 0 && channelIndex < RSOUND_CHANNEL_COUNT);
	playSoundStatic(sequenceOffset, channelIndex + 1);
}

int RSoundDemo::startVoiceInRange(int sequenceOffset, int firstChannel,
		int lastChannel) {
	assert(firstChannel >= 0 && firstChannel <= lastChannel && lastChannel < 8);

	for (int channel = firstChannel; channel <= lastChannel; ++channel) {
		if (!_channels[channel]._deltaCounter) {
			startVoice(channel, sequenceOffset);
			return channel;
		}
	}

	for (int channel = lastChannel; channel >= firstChannel; --channel) {
		if (_channels[channel]._fadeOutActive) {
			startVoice(channel, sequenceOffset);
			return channel;
		}
	}

	return -1;
}

int RSoundDemo::startAnyVoice(int sequenceOffset) {
	// Both demo overlays exclude rhythm channel 9 from their melodic pools.
	return startVoiceInRange(sequenceOffset, 0, 7);
}

int RSoundDemo::startEffectVoice(int sequenceOffset) {
	return startVoiceInRange(sequenceOffset, _firstEffectChannel, 7);
}

void RSoundDemo::requestStopRange(int firstChannel, int channelCount) {
	assert(firstChannel >= 0 && channelCount >= 0 &&
			firstChannel + channelCount <= RSOUND_CHANNEL_COUNT);
	for (int channel = firstChannel;
			channel < firstChannel + channelCount; ++channel)
		_channels[channel].setFadeOut(true);
}

void RSoundDemo::requestStopAll() {
	requestStopRange(0, RSOUND_CHANNEL_COUNT);
}

void RSoundDemo::stopAndResetRange(int firstChannel, int channelCount) {
	assert(firstChannel >= 0 && channelCount > 0 &&
			firstChannel + channelCount <= RSOUND_CHANNEL_COUNT);
	resetChannelRange(firstChannel + 1, firstChannel + channelCount);
	clearActiveNotesRange(firstChannel + 1, firstChannel + channelCount);
	sendMidiChannelReset(firstChannel + 1, firstChannel + channelCount);
}

void RSoundDemo::setVoiceVolume(int channelIndex, byte volume) {
	assert(channelIndex >= 0 && channelIndex < RSOUND_CHANNEL_COUNT);
	_channels[channelIndex]._volume = volume;
	sendVolume(channelIndex + 1, volume);
}

bool RSoundDemo::isSequenceActive(int sequenceOffset) {
	return isSoundPlaying(loadData(sequenceOffset));
}

const RSound1::CommandPtr RSound1::_commandList[42] = {
	&RSound1::command0, &RSound1::command1, &RSound1::command2, &RSound1::command3,
	&RSound1::command4, &RSound1::command5, &RSound1::command6, &RSound1::command7,
	&RSound1::command8, &RSound1::command9, &RSound1::command10, &RSound1::command11,
	&RSound1::command12, &RSound1::command13, &RSound1::command14, &RSound1::command15,
	&RSound1::command16, &RSound1::command17, &RSound1::command18, &RSound1::command19,
	&RSound1::command20, &RSound1::command21, &RSound1::command22, &RSound1::command23,
	&RSound1::command24, &RSound1::command25, &RSound1::command26, &RSound1::command27,
	&RSound1::command28, &RSound1::command29, &RSound1::command30, &RSound1::command31,
	&RSound1::command32, &RSound1::command33, &RSound1::command34, &RSound1::command35,
	&RSound1::command36, &RSound1::command37, &RSound1::command38, &RSound1::command39,
	&RSound1::command40, &RSound1::command41
};

RSound1::RSound1(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver) : 
	RSound(mixer, midiDriver, "rsound.001", 0x1350, 0x1A90, 0x67, kRSoundFadeCheckAlternating) {
}

int RSound1::command(int commandId, int param) {
	if (commandId > 41)
		return 0;

	_commandParam = param;
	_ticksSinceLastCommand = 0;
	return (this->*_commandList[commandId])();
}

int RSound1::clampParam() {
	return (_commandParam > 0x40) ? _commandParam - 0x40 : 0;
}

void RSound1::playCommand11_12_13SharedChannels() {
	byte *pData = loadData(0x1166);
	if (!isSoundPlaying(pData)) {
		command1();
		playSoundStatic(pData, 1);
		playSoundStatic(0x13BC, 2);
		playSoundStatic(0x155C, 3);
		playSoundStatic(0x15D8, 4);
	}
}

int RSound1::command9() {
	playSoundDynamic(0xAD4);
	return 0;
}

int RSound1::command10() {
	byte *pData = loadData(0xCE4);
	if (!isSoundPlaying(pData)) {
		playSoundStatic(pData, 1);
		playSoundStatic(0xD18, 2);
		playSoundStatic(0xE9C, 3);
		playSoundStatic(0xEE8, 4);
	}
	return 0;
}

int RSound1::command11() {
	playCommand11_12_13SharedChannels();
	setChannelVolume(1, 0);
	setChannelVolume(2, 0);
	return 0;
}

int RSound1::command12() {
	playCommand11_12_13SharedChannels();
	setChannelVolume(1, 80);
	setChannelVolume(2, 0);
	return 0;
}

int RSound1::command13() {
	playCommand11_12_13SharedChannels();
	setChannelVolume(1, 80);
	setChannelVolume(2, 80);
	return 0;
}

int RSound1::command14() {
	playSoundDynamic(0x16C2);
	return 0;
}

int RSound1::command15() {
	byte *pData = loadData(0xF3A);
	if (!isSoundPlaying(pData)) {
		command1();
		playSoundStatic(pData, 5);
		playSoundStatic(0x102A, 6);
		playSoundStatic(0x110E, 7);
	}
	return 0;
}

int RSound1::command16() {
	playSoundDynamic(0xADE);
	return 0;
}

int RSound1::command17() {
	playSoundDynamic(0xAE8);
	return 0;
}

int RSound1::command18() {
	playSoundDynamic(0xAF2);
	return 0;
}

int RSound1::command19() {
	command1();
	playSoundDynamic(0xB04);
	return 0;
}

int RSound1::command20() {
	playSoundDynamic(0xB5E);
	return 0;
}

int RSound1::command21() {
	playSoundDynamic(0xB4C);
	return 0;
}

int RSound1::command22() {
	playSoundDynamic(0xB6E);
	return 0;
}

int RSound1::command23() {
	byte *pData = loadData(0xB7A);
	pData[6] ^= 0x1F;
	playSoundDynamic(0xB7A);
	return 0;
}

int RSound1::command24() {
	playSoundDynamic(0xB84);
	return 0;
}

int RSound1::command25() {
	playSoundDynamic(0xB8E);
	return 0;
}

int RSound1::command26() {
	byte *pData = loadData(0xCCA);
	int v1 = (generateRandomNumber() & 24) + 45;
	pData[8] = v1;
	int v2 = clampParam() + 64;
	pData[5] = v2;
	playSoundStatic(pData, 8);
	return 0;
}

int RSound1::command27() {
	byte *pData = loadData(0xCBE);
	int v1 = (generateRandomNumber() & 24) + 45;
	pData[8] = v1;
	int v2 = clampParam() + 64;
	pData[5] = v2;
	playSoundStatic(pData, 8);
	return 0;
}

int RSound1::command28() {
	playSoundDynamic(0xB9E);
	return 0;
}

int RSound1::command29() {
	byte *pData = loadData(0xC6C);
	int v = (clampParam() >> 1) + 32;
	pData[0xB] = v;
	if (!isSoundPlaying(pData))
		playSoundDynamic(0xC6C);
	return 0;
}

int RSound1::command30() {
	byte *pData = loadData(0xC80);
	int v = clampParam() + 63;
	pData[0xB] = v;
	if (!isSoundPlaying(pData))
		playSoundAnyChannel(0xC80);
	return 0;
}

int RSound1::command31() {
	playSoundDynamic(0xBBE);
	return 0;
}

int RSound1::command32() {
	byte *pData = loadData(0xC96);
	int half = clampParam() >> 1;
	pData[0xB] = pData[0x17] = half + 68;
	pData[0x11] = pData[0x1D] = half + 20;
	if (!isSoundPlaying(pData))
		playSoundAnyChannel(0xC96);
	return 0;
}

int RSound1::command33() {
	playSoundDynamic(0xBD0);
	playSoundDynamic(0xBDA);
	return 0;
}

int RSound1::command34() {
	byte *pData = loadData(0xBE8);
	int v = (generateRandomNumber() & 12) + 45;
	pData[9] = v;
	pData[0x10] = v + 36;
	playSoundDynamic(0xBE8);
	return 0;
}

int RSound1::command35() {
	playSoundDynamic(0xBFC);
	playSoundDynamic(0xC0E);
	playSoundDynamic(0xC20);
	return 0;
}

int RSound1::command36() {
	playSoundDynamic(0xC3E);
	return 0;
}

int RSound1::command37() {
	byte *pData = loadData(0xC4C);
	int r = generateRandomNumber() & 15;
	pData[6] = r + 42;
	pData[3] = 62 - (r << 1);
	playSoundDynamic(0xC4C);
	return 0;
}

int RSound1::command38() {
	playSoundAnyChannel(0xC56);
	playSoundDynamic(0xC60);
	return 0;
}

int RSound1::command39() {
	byte *pData = loadData(0x1818);
	if (!isSoundPlaying(pData)) {
		playSoundStatic(pData, 5);
		playSoundStatic(0x1848, 6);
		playSoundStatic(0x1874, 7);
		playSoundStatic(0x18B4, 8);
		playSoundStatic(0x18CE, 9);
	}
	return 0;
}

int RSound1::command40() {
	playSoundDynamic(0xC34);
	return 0;
}

int RSound1::command41() {
	playSoundDynamic(0xCD6);
	return 0;
}

/*-----------------------------------------------------------------------*/

RSoundDemo1::RSoundDemo1(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver) :
		RSoundDemo(mixer, midiDriver, "rsound.001", 0x12E0, 0x1D28, 0x67, 0),
		_command23Toggle(false) {
}

byte RSoundDemo1::adjustedCommandParam() const {
	const byte value = (byte)_commandParam;
	return value > 0x40 ? value - 0x40 : 0;
}

void RSoundDemo1::playCommand11_12_13CommonChannels() {
	if (isSequenceActive(0x1586))
		return;

	requestStopAll();
	startVoice(0, 0x1586);
	startVoice(1, 0x17DC);
	startVoice(2, 0x197C);
	startVoice(3, 0x19F8);
}

int RSoundDemo1::executeDemoCommonCommand(int commandId) {
	switch (commandId) {
	case 0:
		return RSound::command0();
	case 1:
		requestStopAll();
		return 0;
	case 2:
		stopAndResetRange(0, 4);
		return 0;
	case 3:
		requestStopRange(0, 4);
		return 0;
	case 4:
		stopAndResetRange(4, 5);
		return 0;
	case 5:
		requestStopRange(4, 5);
		return 0;
	case 6:
		return RSound::command6();
	case 7:
		return RSound::command7();
	case 8:
		return RSound::command8();
	default:
		return 0;
	}
}

int RSoundDemo1::command(int commandId, int param) {
	if (commandId < 0 || commandId > 40)
		return 0;

	_commandParam = param;
	_ticksSinceLastCommand = 0;
	if (commandId <= 8)
		return executeDemoCommonCommand(commandId);

	switch (commandId) {
	case 9:
		startAnyVoice(0x0F34);
		break;
	case 10:
		if (!isSequenceActive(0x1104)) {
			requestStopAll();
			startVoice(4, 0x1104);
			startVoice(5, 0x1138);
			startVoice(6, 0x12BC);
			startVoice(7, 0x1308);
		}
		break;
	case 11:
		playCommand11_12_13CommonChannels();
		setVoiceVolume(0, 0x00);
		setVoiceVolume(1, 0x00);
		break;
	case 12:
		playCommand11_12_13CommonChannels();
		setVoiceVolume(0, 0x50);
		setVoiceVolume(1, 0x00);
		break;
	case 13:
		playCommand11_12_13CommonChannels();
		setVoiceVolume(0, 0x50);
		setVoiceVolume(1, 0x50);
		break;
	case 14:
		startAnyVoice(0x1AE2);
		break;
	case 15:
		if (!isSequenceActive(0x135A)) {
			requestStopAll();
			startVoice(4, 0x135A);
			startVoice(5, 0x144A);
			startVoice(6, 0x152E);
		}
		break;
	case 16:
		startAnyVoice(0x0F3E);
		break;
	case 17:
		startAnyVoice(0x0F48);
		break;
	case 18:
		startAnyVoice(0x0F52);
		break;
	case 19:
		requestStopAll();
		startAnyVoice(0x0F64);
		break;
	case 20:
		startAnyVoice(0x0FBE);
		break;
	case 21:
		startAnyVoice(0x0FAC);
		break;
	case 22: {
		byte *data = sequenceData(0x0FCE);
		data[6] = (generateRandomNumber() & 0x07) + 0x73;
		startAnyVoice(0x0FCE);
		break;
	}
	case 23:
		_command23Toggle = !_command23Toggle;
		startAnyVoice(_command23Toggle ? 0x0FD8 : 0x0FE0);
		break;
	case 24:
		startAnyVoice(0x0FE8);
		break;
	case 25:
		startAnyVoice(0x0FF2);
		break;
	case 26:
	case 27: {
		const int sequenceOffset = commandId == 26 ? 0x10F8 : 0x10EC;
		byte *data = sequenceData(sequenceOffset);
		data[8] = (generateRandomNumber() & 0x18) + 0x2D;
		data[5] = adjustedCommandParam() + 0x40;
		startVoice(7, sequenceOffset);
		break;
	}
	case 28:
		startAnyVoice(0x1002);
		break;
	case 29: {
		byte *data = sequenceData(0x109A);
		data[11] = (adjustedCommandParam() >> 1) + 0x20;
		if (!isSequenceActive(0x109A))
			startAnyVoice(0x109A);
		break;
	}
	case 30: {
		byte *data = sequenceData(0x10AE);
		data[11] = adjustedCommandParam() + 0x3F;
		if (!isSequenceActive(0x10AE))
			startAnyVoice(0x10AE);
		break;
	}
	case 31:
		startAnyVoice(0x1022);
		break;
	case 32: {
		const byte value = adjustedCommandParam() >> 1;
		byte *data = sequenceData(0x10C4);
		data[11] = data[23] = value + 0x44;
		data[17] = data[29] = value + 0x14;
		if (!isSequenceActive(0x10C4))
			startAnyVoice(0x10C4);
		break;
	}
	case 33:
		startAnyVoice(0x1034);
		startAnyVoice(0x103E);
		break;
	case 34: {
		byte *data = sequenceData(0x104C);
		data[9] = (generateRandomNumber() & 0x0C) + 0x2D;
		data[16] = data[9] + 0x24;
		startAnyVoice(0x104C);
		break;
	}
	case 35:
		startAnyVoice(0x1060);
		break;
	case 36:
		startAnyVoice(0x1078);
		break;
	case 37:
		startAnyVoice(0x1086);
		break;
	case 38:
		startAnyVoice(0x1090);
		break;
	case 39:
		if (!isSequenceActive(0x1C38)) {
			startVoice(4, 0x1C38);
			startVoice(5, 0x1C68);
			startVoice(6, 0x1C94);
			startVoice(7, 0x1CD4);
			startVoice(8, 0x1CEE);
		}
		break;
	case 40:
		startAnyVoice(0x106E);
		break;
	}

	return 0;
}

/*-----------------------------------------------------------------------*/

const uint16 RSound2::_command18RandomSfx[16] = {
	0x3234, 0x3250, 0x326A, 0x3284, 0x329E, 0x32D6, 0x3304, 0x333C,
	0x3352, 0x3378, 0x33B6, 0x33D0, 0x33EA, 0x3404, 0x341E, 0x343E
};

const RSound2::CommandPtr RSound2::_commandList[44] = {
	&RSound2::command0, &RSound2::command1, &RSound2::command2, &RSound2::command3,
	&RSound2::command4, &RSound2::command5, &RSound2::command6, &RSound2::command7,
	&RSound2::command8, &RSound2::command9, &RSound2::command10, &RSound2::command11,
	&RSound2::command12, &RSound2::command13, &RSound2::command14, &RSound2::command15,
	&RSound2::command16, &RSound2::command17, &RSound2::command18, &RSound2::command19,
	&RSound2::command20, &RSound2::command21, &RSound2::command22, &RSound2::command23,
	&RSound2::command24, &RSound2::command25, &RSound2::command26, &RSound2::command27,
	&RSound2::command28, &RSound2::command29, &RSound2::command30, &RSound2::command31,
	&RSound2::command32, &RSound2::command33, &RSound2::command34, &RSound2::command35,
	&RSound2::command36, &RSound2::command37, &RSound2::command38, &RSound2::command39,
	&RSound2::command40, &RSound2::command41, &RSound2::command42, &RSound2::command43
};

RSound2::RSound2(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver) : 
	RSound(mixer, midiDriver, "rsound.002", 0x1390, 0x42F0, 0x87, kRSoundFadeCheckAlternating) {
}

int RSound2::command(int commandId, int param) {
	if (commandId > 43)
		return 0;

	_commandParam = param;
	_ticksSinceLastCommand = 0;
	return (this->*_commandList[commandId])();
}

int RSound2::command5() {
	_volumeCycleCounter = 47;
	for (int i = 4; i <= 8; i++) {
		getChannel(i)->setFadeOut(true);
	}
	return 0;
}

int RSound2::command9() {
	byte *pData = loadData(0x103C);
	if (!isSoundPlaying(pData)) {
		command1();
		playSoundStatic(pData, 1);
		playSoundStatic(0x11B2, 2);
		playSoundStatic(0x127E, 9);
	}
	return 0;
}

int RSound2::command10() {
	byte *pData = loadData(0x132E);
	if (!isSoundPlaying(pData)) {
		command1();
		playSoundStatic(pData, 3);
		playSoundStatic(0x1384, 9);
	}
	return 0;
}

int RSound2::command11() {
	byte *pData = loadData(0x1548);
	if (!isSoundPlaying(pData)) {
		command1();
		playSoundStatic(pData, 3);
		playSoundStatic(0x1648, 9);
	}
	return 0;
}

int RSound2::command12() {
	byte *pData = loadData(0xE52);
	_volumeCycleCounter += 16;
	pData[3] = _volumeCycleCounter & 0x7F;
	playSoundDynamic(0xE52);
	return 0;
}

int RSound2::command13() {
	playSoundDynamic(0xE5C);
	playSoundDynamic(0xE66);
	return 0;
}

int RSound2::command14() {
	playSoundDynamic(0xE70);
	playSoundDynamic(0xE8A);
	return 0;
}

int RSound2::command15() {
	byte *pData = loadData(0x1DFC);
	if (!isSoundPlaying(pData)) {
		command1();
		playSoundAnyChannel(0x1DFC);
		playSoundAnyChannel(0x222E);
		playSoundAnyChannel(0x2648);
		playSoundStatic(0x26A2, 9);
	}
	return 0;
}

int RSound2::command16() {
	byte *pData = loadData(0x3456);
	if (!isSoundPlaying(pData)) {
		command1();
		playSoundAnyChannel(0x3456);
		playSoundAnyChannel(0x3572);
		playSoundAnyChannel(0x367E);
		playSoundAnyChannel(0x37C6);
		playSoundAnyChannel(0x39AE);
		playSoundAnyChannel(0x3A3A);
	}
	return 0;
}

int RSound2::command17() {
	byte *pData = loadData(0x3AC0);
	if (!isSoundPlaying(pData)) {
		playSoundDynamic(0x3AC0);
		playSoundDynamic(0x3C70);
		playSoundDynamic(0x3E16);
		playSoundDynamic(0x3FBE);
	}
	return 0;
}

int RSound2::command18() {
	if (getChannel(8)->_deltaCounter > 0)
		return 0;

	int idx = (generateRandomNumber() & 0x1E) >> 1;
	playSoundStatic(_command18RandomSfx[idx], 8);
	return 0;
}

int RSound2::command19() {
	byte *pData = loadData(0x2A64);
	if (!isSoundPlaying(pData)) {
		command1();
		playSoundAnyChannel(0x2A64);
		playSoundAnyChannel(0x2BE2);
		playSoundAnyChannel(0x2DAC);
		playSoundAnyChannel(0x2ECE);
		playSoundAnyChannel(0x3026);
		playSoundAnyChannel(0x30C2);
	}
	return 0;
}

int RSound2::command20() {
	playSoundDynamic(0xF1E);
	playSoundDynamic(0xF1E);
	playSoundDynamic(0xF1E);
	playSoundDynamic(0xF1E);
	return 0;
}

int RSound2::command21() {
	playSoundDynamic(0xF58);
	return 0;
}

int RSound2::command22() {
	playSoundDynamic(0xF44);
	return 0;
}

int RSound2::command23() {
	playSoundDynamic(0xEBA);
	return 0;
}

int RSound2::command24() {
	playSoundDynamic(0xEB0);
	return 0;
}

int RSound2::command25() {
	playSoundDynamic(0xEA6);
	return 0;
}

int RSound2::command26() {
	playSoundDynamic(0xF4E);
	return 0;
}

int RSound2::command27() {
	Channel *ch = playSoundDynamic(0xFAC);
	if (ch != nullptr)
		ch->_innerLoopStart = loadData(0xFB8);

	ch = playSoundDynamic(0xFB2);
	if (ch != nullptr)
		ch->_innerLoopStart = loadData(0xFB8);

	playSoundDynamic(0xFB8);
	return 0;
}

int RSound2::command28() {
	byte *pData = loadData(0xEDA);
	int r = generateRandomNumber();
	int v1 = r & 0x7F;
	pData[7] = v1;
	int v2 = (v1 & 0x0F) + 0x43;
	pData[8] = v2;
	int v3 = v2 + 0x0C;
	pData[0xA] = v3;
	playSoundDynamic(0xEDA);
	return 0;
}

int RSound2::command29() {
	playSoundAnyChannel(0xF80);
	return 0;
}

int RSound2::command30() {
	playSoundDynamic(0xF14);
	byte *pData = loadData(0xF0A);
	pData[3] = 40;
	playSoundDynamic(0xF0A);
	return 0;
}

int RSound2::command31() {
	byte *pData = loadData(0xF0A);
	pData[3] = 0x18;
	playSoundDynamic(0xF0A);
	return 0;
}

int RSound2::command32() {
	playSoundDynamic(0xEC4);
	return 0;
}

int RSound2::command33() {
	playSoundDynamic(0xED0);
	return 0;
}

int RSound2::command34() {
	playSoundDynamic(0xEE8);
	return 0;
}

int RSound2::command35() {
	playSoundDynamic(0xEF8);
	return 0;
}

int RSound2::command36() {
	playSoundDynamic(0xFF4);
	playSoundDynamic(0x1008);
	return 0;
}

int RSound2::command37() {
	playSoundDynamic(0xFCC);
	return 0;
}

int RSound2::command38() {
	byte *pData = loadData(0x2B0E);
	if (!isSoundPlaying(pData)) {
		command1();
		playSoundAnyChannel(0x2B0E);
		playSoundAnyChannel(0x2CD0);
		playSoundAnyChannel(0x2E46);
		playSoundAnyChannel(0x2F7C);
		playSoundAnyChannel(0x3074);
		playSoundAnyChannel(0x317E);
	}
	return 0;
}

int RSound2::command39() {
	playSoundDynamic(0xFDA);
	return 0;
}

int RSound2::command40() {
	playSoundDynamic(0xFE6);
	return 0;
}

int RSound2::command41() {
	playSoundAnyChannel(0xF62);
	return 0;
}

int RSound2::command42() {
	playSoundDynamic(0xF92);
	return 0;
}

int RSound2::command43() {
	playSoundDynamic(0x1018);
	playSoundDynamic(0x102A);
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound3::CommandPtr RSound3::_commandList[61] = {
	&RSound3::command0, &RSound3::command1, &RSound3::command2, &RSound3::command3,
	&RSound3::command4, &RSound3::command5, &RSound3::command6, &RSound3::command7,
	&RSound3::command8, &RSound3::command9, &RSound3::command10, &RSound3::command11,
	&RSound3::nullCommand, &RSound3::command13, &RSound3::command14, &RSound3::command15,
	&RSound3::command16, &RSound3::command17, &RSound3::command18, &RSound3::command19,
	&RSound3::command20, &RSound3::command21, &RSound3::command22, &RSound3::command23,
	&RSound3::command24, &RSound3::command25, &RSound3::command26, &RSound3::command27_42,
	&RSound3::command28, &RSound3::command29, &RSound3::command30, &RSound3::command31,
	&RSound3::command32, &RSound3::command33, &RSound3::command34, &RSound3::command35,
	&RSound3::command36, &RSound3::command37, &RSound3::command38, &RSound3::command39,
	&RSound3::command40, &RSound3::command41, &RSound3::command27_42, &RSound3::command43,
	&RSound3::command44, &RSound3::command45, &RSound3::command46, &RSound3::command47_49,
	&RSound3::command48, &RSound3::command47_49, &RSound3::command50, &RSound3::command51,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::command57, &RSound3::nullCommand, &RSound3::command59,
	&RSound3::command60
};

RSound3::RSound3(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver) : 
	RSound(mixer, midiDriver, "rsound.003", 0x14E0, 0x4C60, 0x67, kRSoundFadeCheckProgrammable) {
}

int RSound3::command(int commandId, int param) {
	if (commandId > 60)
		return 0;

	_commandParam = param;
	_ticksSinceLastCommand = 0;
	return (this->*_commandList[commandId])();
}

Channel *RSound3::patchAndPlaySound(int offset, byte value) {
	byte *pData = loadData(offset);
	pData[5] = value;
	return playSoundDynamic(offset);
}

int RSound3::command9() {
	command1();
	setFadeOutSpeed((byte)_commandParam);
	return 0;
}

int RSound3::command10() {
	byte *pData = loadData(0x14FE);
	if (!isSoundPlaying(pData)) {
		command1();
		playSoundStatic(pData, 1);
		playSoundStatic(0x1630, 2);
		playSoundStatic(0x186E, 3);
		playSoundStatic(0x1A68, 4);
		playSoundStatic(0x1AA6, 9);
	}
	return 0;
}

void RSound3::setVariantByte(byte value) {
	loadData(0x204A)[1] = value;
	loadData(0x229C)[1] = value;
	loadData(0x2748)[1] = value;
	loadData(0x2C56)[1] = value;
}

int RSound3::command11() {
	if (!isSoundPlaying(loadData(0x1AE6))) {
		setVariantByte(0x64);
		playSoundStatic(0x1AE6, 1);
		playSoundStatic(0x1E00, 2);
		playSoundStatic(0x1E66, 3);
		playSoundStatic(0x204A, 4);
		playSoundStatic(0x229C, 5);
		playSoundStatic(0x2748, 6);
		playSoundStatic(0x2C56, 7);
	}
	return 0;
}

int RSound3::command13() {
	command1();
	playSoundAnyChannel(0x1364);
	playSoundAnyChannel(0x1364);
	playSoundAnyChannel(0x1364);
	playSoundAnyChannel(0x1364);
	playSoundAnyChannel(0x1364);
	return 0;
}

int RSound3::command14() {
	playSoundStatic(0x32DC, 1);
	playSoundStatic(0x32FC, 2);
	playSoundStatic(0x331C, 3);
	playSoundStatic(0x333E, 4);
	playSoundStatic(0x335C, 5);
	playSoundStatic(0x339E, 6);
	playSoundStatic(0x33DE, 7);
	playSoundStatic(0x341E, 8);
	return 0;
}

void RSound3::sendDualVolume(byte volume) {
	setChannelVolume(1, volume);
	// The original code sets volume on the channel data for channel 1 (again),
	// but sends it to the MT-32 on channel 2. Most likely a bug.
	setChannelVolume(2, volume);
}

int RSound3::command15() {
	setVariantByte(0x60);
	sendDualVolume(0x60);

	if (isSoundPlaying(4, loadData(0x204A))) {
		for (int i = 3; i <= 8; i++) {
			getChannel(i)->_fadeOutActive = true;
		}
		setFadeOutSpeed(1);
		return 0;
	}

	command1();
	playSoundStatic(0x1AE6, 1);
	playSoundStatic(0x1E00, 2);
	return 0;
}

int RSound3::command16() {
	_command16AltFlag = !_command16AltFlag;

	if (_command16AltFlag) {
		byte *pData = loadData(0x345E);
		if (!isSoundPlaying(pData)) {
			playSoundStatic(pData, 1);
			playSoundStatic(0x364C, 2);
			playSoundStatic(0x3806, 3);
			playSoundStatic(0x399E, 4);
		}
	} else {
		byte *pData = loadData(0x3B26);
		if (!isSoundPlaying(pData)) {
			command1();
			playSoundStatic(pData, 1);
			playSoundStatic(0x3BD8, 2);
			playSoundStatic(0x3CF8, 3);
			playSoundStatic(0x3E46, 4);
		}
	}
	return 0;
}

int RSound3::command17() {
	byte *pData = loadData(0x3F5C);
	if (!isSoundPlaying(pData)) {
		command1();
		playSoundStatic(pData, 1);
		playSoundStatic(0x4022, 2);
		playSoundStatic(0x41F0, 3);
		playSoundStatic(0x42F8, 4);
	}
	return 0;
}

int RSound3::command18() {
	command1();
	playSoundStatic(0x4492, 1);
	playSoundStatic(0x45A4, 2);
	playSoundStatic(0x46A2, 3);
	playSoundStatic(0x47DC, 4);
	playSoundStatic(0x49C0, 5);
	playSoundStatic(0x4A4E, 6);
	return 0;
}

int RSound3::command19() {
	if (!isSoundPlaying(loadData(0x1AE6)))
		playSoundDynamic(0x12B4);
	return 0;
}

int RSound3::command20() {
	if (!isSoundPlaying(loadData(0x1AE6)))
		playSoundDynamic(0x1246);
	return 0;
}

int RSound3::command21() {
	if (!isSoundPlaying(loadData(0x1AE6))) {
		playSoundDynamic(0x1232);
		playSoundDynamic(0x123C);
	}
	return 0;
}

int RSound3::command22() {
	playSoundDynamic(0x126E);
	return 0;
}

int RSound3::command23() {
	if (!isSoundPlaying(loadData(0x1AE6))) {
		playSoundDynamic(0x12D2);
		playSoundDynamic(0x12E0);
	}
	return 0;
}

int RSound3::command24() {
	if (!isSoundPlaying(loadData(0x1AE6))) {
		playSoundDynamic(0x11E2);
		playSoundDynamic(0x120A);
	}
	return 0;
}

int RSound3::command25() {
	// The dispatcher's preceding xor leaves ZF set, so the native jz
	// selects 0x25 for both calls.
	patchAndPlaySound(0x11A6, 0x25);
	patchAndPlaySound(0x11C4, 0x25);
	return 0;
}

int RSound3::command26() {
	playSoundDynamic(0x1252);
	return 0;
}

int RSound3::command27_42() {
	playSoundDynamic(0x14BE);
	return 0;
}

int RSound3::command28() {
	byte *pData = loadData(0x12EE);
	pData[3] = 0x4D; // Medium volume
	playSoundDynamic(0x12EE);
	return 0;
}

int RSound3::command29() {
	byte *pData = loadData(0x12EE);
	pData[3] = 0x7F; // High volume
	playSoundDynamic(0x12EE);
	return 0;
}

int RSound3::command30() {
	playSoundDynamic(0x14B4);
	playSoundDynamic(0x14AA);
	return 0;
}

int RSound3::command31() {
	playSoundDynamic(0x12F8);
	playSoundDynamic(0x130E);
	return 0;
}

int RSound3::command32() {
	playSoundDynamic(0x1472);
	return 0;
}

int RSound3::command33() {
	playSoundDynamic(0x147E);
	return 0;
}

int RSound3::command34() {
	playSoundDynamic(0x1488);
	return 0;
}

int RSound3::command35() {
	playSoundDynamic(0x1498);
	return 0;
}

int RSound3::command36() {
	playSoundDynamic(0x14DA);
	playSoundDynamic(0x14EE);
	return 0;
}

int RSound3::command37() {
	playSoundDynamic(0x14CC);
	return 0;
}

int RSound3::command38() {
	playSoundDynamic(0x133A);
	return 0;
}

int RSound3::command39() {
	byte *pData = loadData(0x1346);
	pData[3] = 77;
	_command39_40NoteToggle ^= 4;
	pData[6] = _command39_40NoteToggle + 0x28;
	playSoundDynamic(0x1346);
	return 0;
}

int RSound3::command40() {
	byte *pData = loadData(0x1346);
	pData[3] = 47;
	_command39_40NoteToggle ^= 4;
	pData[6] = _command39_40NoteToggle + 0x28;
	playSoundDynamic(0x1346);
	return 0;
}

int RSound3::command41() {
	playSoundDynamic(0x1184);
	return 0;
}

int RSound3::command43() {
	playSoundDynamic(0x1350);
	playSoundDynamic(0x135A);
	return 0;
}

int RSound3::command44() {
	playSoundDynamic(0x12A0);
	return 0;
}

int RSound3::command45() {
	playSoundDynamic(0x12AA);
	return 0;
}

int RSound3::command46() {
	playSoundDynamic(0x13AA);
	playSoundDynamic(0x13C6);
	return 0;
}

int RSound3::command47_49() {
	playSoundDynamic(0x13E6);
	playSoundDynamic(0x13FE);
	return 0;
}

int RSound3::command48() {
	playSoundDynamic(0x141E);
	return 0;
}

int RSound3::command50() {
	playSoundDynamic(0x1436);
	playSoundDynamic(0x144C);
	return 0;
}

int RSound3::command51() {
	playSoundDynamic(0x125C);
	return 0;
}

int RSound3::command57() {
	playSoundDynamic(0x1466);
	return 0;
}

int RSound3::command59() {
	playSoundDynamic(0x1324);
	return 0;
}

int RSound3::command60() {
	playSoundDynamic(0x132E);
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound4::CommandPtr RSound4::_commandList[60] = {
	&RSound4::command0, &RSound4::command1, &RSound4::command2, &RSound4::command3,
	&RSound4::command4, &RSound4::command5, &RSound4::command6, &RSound4::command7,
	&RSound4::command8, &RSound4::command9, &RSound4::command10, &RSound4::nullCommand,
	&RSound4::command12, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::command19,
	&RSound4::command20, &RSound4::command21, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::command27,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::command30, &RSound4::nullCommand,
	&RSound4::command32, &RSound4::command33, &RSound4::command34, &RSound4::command35,
	&RSound4::command36, &RSound4::command37, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::command52, &RSound4::command53, &RSound4::command54, &RSound4::command55,
	&RSound4::command56, &RSound4::command57, &RSound4::command58, &RSound4::command59
};

RSound4::RSound4(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver) : 
	RSound(mixer, midiDriver, "rsound.004", 0x1340, 0x2E20, 0x67, kRSoundFadeCheckProgrammable) {
}

int RSound4::command(int commandId, int param) {
	if (commandId > 59)
		return 0;

	_commandParam = param;
	_ticksSinceLastCommand = 0;
	return (this->*_commandList[commandId])();
}

void RSound4::tickCallback() {
	if (_callbackPeriod == 0 || --_callbackCounter > 0)
		return;

	_callbackCounter = _callbackPeriod;
	if (_callbackFnPtr != nullptr)
		(this->*_callbackFnPtr)();
}

void RSound4::stopChannel(byte channel) {
	getChannel(channel)->_pSrc = loadData(0x1188);
}

int RSound4::command9() {
	command1();
	setFadeOutSpeed((byte)_commandParam);
	return 0;
}

void RSound4::playCommand10_58CommonChannels() {
	playSoundStatic(0x1274, 1);
	playSoundStatic(0x13A6, 2);
	playSoundStatic(0x15E4, 3);
}

int RSound4::command10() {
	command1();
	playSoundStatic(0x17DE, 4);
	playSoundStatic(0x181C, 9);
	playCommand10_58CommonChannels();
	return 0;
}

void RSound4::setCommand12Variant() {
	byte value = (byte)((_commandParam >> 1) + 36);
	loadData(0x1966)[1] = value;
	loadData(0x1DC8)[1] = value;
	loadData(0x1FBA)[1] = value;
	loadData(0x211E)[1] = value;
	loadData(0x24A8)[1] = value;
}

int RSound4::command12() {
	if (isSoundPlaying(5, loadData(0x1966))) {
		setCommand12Variant();
		return 0;
	}

	setCommand12Variant();
	command1();
	playSoundStatic(0x1966, 5);
	playSoundStatic(0x1DC8, 6);
	playSoundStatic(0x1FBA, 7);
	playSoundStatic(0x211E, 8);
	playSoundStatic(0x24A8, 9);
	return 0;
}

int RSound4::command19() {
	playSoundDynamic(0x1196);
	return 0;
}

int RSound4::command20() {
	playSoundDynamic(0x118A);
	return 0;
}

int RSound4::command21() {
	playSoundDynamic(0x1260);
	playSoundDynamic(0x126A);
	return 0;
}

int RSound4::command27() {
	playSoundDynamic(0x1220);
	return 0;
}

int RSound4::command30() {
	playSoundDynamic(0x1216);
	playSoundDynamic(0x120C);
	return 0;
}

int RSound4::command32() {
	playSoundDynamic(0x11D4);
	return 0;
}

int RSound4::command33() {
	playSoundDynamic(0x11E0);
	return 0;
}

int RSound4::command34() {
	playSoundDynamic(0x11EA);
	return 0;
}

int RSound4::command35() {
	playSoundDynamic(0x11FA);
	return 0;
}

int RSound4::command36() {
	playSoundDynamic(0x123C);
	playSoundDynamic(0x1250);
	return 0;
}

int RSound4::command37() {
	playSoundDynamic(0x122E);
	return 0;
}

int RSound4::command52() {
	stopChannel(1);
	stopChannel(2);
	stopChannel(3);
	playSoundStatic(0x2A0C, 5);
	return 0;
}

int RSound4::command53() {
	command1();
	_callbackCounter = 56;
	_callbackPeriod = 56;
	playSoundAnyChannel(0x1888);
	playSoundAnyChannel(0x18DE);
	return 0;
}

void RSound4::command54Callback() {
	_callbackFnPtr = nullptr;
	playSoundAnyChannel(0x18B2);
	playSoundAnyChannel(0x1904);
}

int RSound4::command54() {
	// Add an instrument to command 53's music at the start of the next bar.
	_callbackFnPtr = &RSound4::command54Callback;
	return 0;
}

void RSound4::command55Callback() {
	_callbackFnPtr = nullptr;
	playSoundAnyChannel(0x191E);
}

int RSound4::command55() {
	// Add an instrument to command 53's music at the start of the next bar.
	_callbackFnPtr = &RSound4::command55Callback;
	return 0;
}

void RSound4::command56Callback() {
	_callbackFnPtr = nullptr;
	playSoundAnyChannel(0x185C);
}

int RSound4::command56() {
	// Add an instrument to command 53's music at the start of the next bar.
	_callbackFnPtr = &RSound4::command56Callback;
	return 0;
}

int RSound4::command57() {
	playSoundDynamic(0x11C8);
	return 0;
}

int RSound4::command58() {
	stopChannel(5);
	playCommand10_58CommonChannels();
	return 0;
}

int RSound4::command59() {
	playSoundDynamic(0x11BE);
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound5::CommandPtr RSound5::_commandList[42] = {
	&RSound5::command0, &RSound5::command1, &RSound5::command2, &RSound5::command3,
	&RSound5::command4, &RSound5::command5, &RSound5::command6, &RSound5::command7,
	&RSound5::command8, &RSound5::command9, &RSound5::command10, &RSound5::command11_24,
	&RSound5::command12_25, &RSound5::command13, &RSound5::command14, &RSound5::command15,
	&RSound5::command16, &RSound5::command17, &RSound5::command18, &RSound5::command19,
	&RSound5::command20, &RSound5::command21, &RSound5::command22, &RSound5::command23,
	&RSound5::command11_24, &RSound5::command12_25, &RSound5::command26, &RSound5::command27,
	&RSound5::command28, &RSound5::command29, &RSound5::command30, &RSound5::command31,
	&RSound5::command32, &RSound5::command33, &RSound5::command34, &RSound5::command35,
	&RSound5::command36, &RSound5::command37, &RSound5::command38, &RSound5::command39,
	&RSound5::command40, &RSound5::command41
};

RSound5::RSound5(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver) : 
	RSound(mixer, midiDriver, "rsound.005", 0x12A0, 0x1FD0, 0x67, kRSoundFadeCheckProgrammable) {
}

int RSound5::command(int commandId, int param) {
	if (commandId > 41)
		return 0;

	_commandParam = param;
	_ticksSinceLastCommand = 0;
	return (this->*_commandList[commandId])();
}

void RSound5::stopChannel(byte channel) {
	getChannel(channel)->_pSrc = loadData(0x1182);
}

int RSound5::command9() {
	playSoundDynamic(0x11C4);
	return 0;
}

int RSound5::command10() {
	playSoundDynamic(0x1238);
	return 0;
}

int RSound5::command11_24() {
	playSoundDynamic(0x1196);
	return 0;
}

int RSound5::command12_25() {
	playSoundDynamic(0x1242);
	return 0;
}

int RSound5::command13() {
	playSoundDynamic(0x125E);
	playSoundDynamic(0x1268);
	return 0;
}

int RSound5::command14() {
	playSoundStatic(0x1272, 8);
	return 0;
}

int RSound5::command15() {
	Channel *chan = getChannel(8);
	if (chan->_soundData == loadData(0x1272)) {
		byte *pData = loadData(0x1288);
		chan->_innerLoopStart = pData;
		chan->_outerLoopStart = pData;
		chan->_deltaCounter = 1;
	}
	return 0;
}

int RSound5::command16() {
	playSoundDynamic(0x129A);
	playSoundDynamic(0x129A);
	playSoundDynamic(0x129A);
	playSoundDynamic(0x129A);
	return 0;
}

int RSound5::command17() {
	playSoundDynamic(0x11B4);
	return 0;
}

int RSound5::command18() {
	playSoundDynamic(0x12E4);
	playSoundDynamic(0x12F6);
	playSoundDynamic(0x1308);
	playSoundDynamic(0x131A);
	return 0;
}

int RSound5::command19() {
	playSoundDynamic(0x132C);
	return 0;
}

int RSound5::command20() {
	playSoundDynamic(0x1368);
	return 0;
}

int RSound5::command21() {
	playSoundDynamic(0x1388);
	return 0;
}

int RSound5::command22() {
	playSoundDynamic(0x139A);
	return 0;
}

int RSound5::command23() {
	playSoundDynamic(0x13AA);
	playSoundDynamic(0x13AA);
	playSoundDynamic(0x13AA);
	playSoundDynamic(0x13AA);
	return 0;
}

int RSound5::command26() {
	playSoundDynamic(0x13D6);
	return 0;
}

int RSound5::command27() {
	playSoundDynamic(0x13F0);
	return 0;
}

int RSound5::command28() {
	playSoundDynamic(0x121C);
	return 0;
}

void RSound5::playCommand29_38CommonChannels() {
	playSoundStatic(0x1688, 4);
	playSoundStatic(0x1882, 9);
}

int RSound5::command29() {
	byte *pData = loadData(0x1488);
	if (!isSoundPlaying(pData)) {
		command1();
		playSoundStatic(pData, 1);
		playSoundStatic(0x1534, 2);
		playSoundStatic(0x15EA, 3);
		playCommand29_38CommonChannels();
	}
	return 0;
}

int RSound5::command30() {
	playSoundDynamic(0x1212);
	playSoundDynamic(0x1208);
	return 0;
}

int RSound5::command31() {
	playSoundDynamic(0x140A);
	return 0;
}

int RSound5::command32() {
	playSoundDynamic(0x11D0);
	return 0;
}

int RSound5::command33() {
	playSoundDynamic(0x11DC);
	return 0;
}

int RSound5::command34() {
	playSoundDynamic(0x11E6);
	return 0;
}

int RSound5::command35() {
	playSoundDynamic(0x11F6);
	return 0;
}

int RSound5::command36() {
	playSoundDynamic(0x1464);
	playSoundDynamic(0x1478);
	return 0;
}

int RSound5::command37() {
	playSoundDynamic(0x122A);
	return 0;
}

int RSound5::command38() {
	stopChannel(5);
	playCommand29_38CommonChannels();
	return 0;
}

int RSound5::command39() {
	playSoundDynamic(0x141E);
	playSoundDynamic(0x1428);
	return 0;
}

int RSound5::command40() {
	playSoundDynamic(0x1432);
	return 0;
}

int RSound5::command41() {
	stopChannel(9);
	playSoundStatic(0x1BB6, 4);
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound6::CommandPtr RSound6::_commandList[30] = {
	&RSound6::command0, &RSound6::command1, &RSound6::command2, &RSound6::command3,
	&RSound6::command4, &RSound6::command5, &RSound6::command6, &RSound6::command7,
	&RSound6::command8, &RSound6::command9, &RSound6::command10, &RSound6::command11,
	&RSound6::command12, &RSound6::command13_14, &RSound6::command13_14, &RSound6::command15,
	&RSound6::command16, &RSound6::command17, &RSound6::command18, &RSound6::command19,
	&RSound6::command20, &RSound6::command21, &RSound6::command22, &RSound6::command23,
	&RSound6::command24, &RSound6::command25, &RSound6::nullCommand, &RSound6::nullCommand,
	&RSound6::nullCommand, &RSound6::command29
};

RSound6::RSound6(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver) : 
	RSound(mixer, midiDriver, "rsound.006", 0x12D0, 0x1EF0, 0x67, kRSoundFadeCheckProgrammable) {
}

int RSound6::command(int commandId, int param) {
	if (commandId > 29)
		return 0;

	_commandParam = param;
	_ticksSinceLastCommand = 0;
	return (this->*_commandList[commandId])();
}

void RSound6::tickCallback() {
	if (_callbackPeriod == 0 || --_callbackCounter > 0)
		return;

	_callbackCounter = _callbackPeriod;
	if (_callbackFnPtr != nullptr)
		(this->*_callbackFnPtr)();
}

int RSound6::command9() {
	playSoundDynamic(0x111C);
	return 0;
}

int RSound6::command10() {
	playSoundDynamic(0x1190);
	return 0;
}

int RSound6::command11() {
	playSoundDynamic(0x11A4);
	playSoundDynamic(0x11A4);
	playSoundDynamic(0x11A4);
	playSoundDynamic(0x11A4);
	return 0;
}

int RSound6::command12() {
	playSoundDynamic(0x11C8);
	return 0;
}

int RSound6::command13_14() {
	playSoundDynamic(0x11E8);
	return 0;
}

int RSound6::command15() {
	playSoundDynamic(0x11F2);
	playSoundDynamic(0x11F2);
	playSoundDynamic(0x11F2);
	playSoundDynamic(0x11F2);
	return 0;
}

int RSound6::command16() {
	playSoundDynamic(0x121E);
	return 0;
}

int RSound6::command17() {
	playSoundDynamic(0x1228);
	playSoundDynamic(0x1228);
	playSoundDynamic(0x1228);
	playSoundDynamic(0x1228);
	return 0;
}

int RSound6::command18() {
	playSoundDynamic(0x1254);
	return 0;
}

int RSound6::command19() {
	playSoundDynamic(0x1266);
	return 0;
}

int RSound6::command20() {
	playSoundDynamic(0x1278);
	return 0;
}

int RSound6::command21() {
	playSoundStatic(0x1282, 5);
	playSoundDynamic(0x1282);
	playSoundDynamic(0x1282);
	playSoundDynamic(0x12AA);
	return 0;
}

int RSound6::command22() {
	playSoundStatic(0x12CE, 5);
	playSoundStatic(0x12CE, 6);
	playSoundStatic(0x12CE, 7);
	playSoundStatic(0x12CE, 8);
	return 0;
}

int RSound6::command23() {
	playSoundDynamic(0x1174);
	return 0;
}

void RSound6::command24Callback() {
	_callbackFnPtr = nullptr;
	command1();
	_callbackCounter = 84;
	_callbackPeriod = 84;
	playSoundStatic(0x1A38, 1);
	playSoundStatic(0x1BBE, 2);
	playSoundStatic(0x1B90, 9);
}

int RSound6::command24() {
	if (isSoundPlaying(1, loadData(0x130A))) {
		// The music of command 29 is playing. Cut into command 24's music
		// at the next bar, which is when the callback is invoked.
		_callbackFnPtr = &RSound6::command24Callback;
		return 0;
	}

	command24Callback();
	return 0;
}

int RSound6::command25() {
	playSoundStatic(0x12FE, 5);
	return 0;
}

void RSound6::command29Callback() {
	_callbackFnPtr = nullptr;
	command1();
	_callbackCounter = 84;
	_callbackPeriod = 84;
	playSoundStatic(0x130A, 1);
	playSoundStatic(0x13B6, 2);
	playSoundStatic(0x146C, 3);
	playSoundStatic(0x150A, 4);
	playSoundStatic(0x1704, 9);
}

int RSound6::command29() {
	if (isSoundPlaying(loadData(0x130A)))
		return 0;

	if (isSoundPlaying(1, loadData(0x1A38))) {
		// The music of command 24 is playing. Cut into command 29's music
		// at the next bar, which is when the callback is invoked.
		_callbackFnPtr = &RSound6::command29Callback;
		return 0;
	}

	command29Callback();
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound7::CommandPtr RSound7::_commandList[38] = {
	&RSound7::command0, &RSound7::command1, &RSound7::command2, &RSound7::command3,
	&RSound7::command4, &RSound7::command5, &RSound7::command6, &RSound7::command7,
	&RSound7::command8, &RSound7::command9, &RSound7::nullCommand, &RSound7::nullCommand,
	&RSound7::nullCommand, &RSound7::nullCommand, &RSound7::nullCommand, &RSound7::command15,
	&RSound7::command16, &RSound7::command17, &RSound7::command18, &RSound7::command19,
	&RSound7::command20, &RSound7::command21, &RSound7::command22, &RSound7::command23,
	&RSound7::command24, &RSound7::command25, &RSound7::nullCommand, &RSound7::command27,
	&RSound7::nullCommand, &RSound7::nullCommand, &RSound7::command30, &RSound7::nullCommand,
	&RSound7::command32, &RSound7::command33, &RSound7::command34, &RSound7::command35,
	&RSound7::command36, &RSound7::command37
};

RSound7::RSound7(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver) : 
	RSound(mixer, midiDriver, "rsound.007", 0x1240, 0x1EF0, 0x67, kRSoundFadeCheckProgrammable) {
}

int RSound7::command(int commandId, int param) {
	if (commandId > 37)
		return 0;

	_commandParam = param;
	_ticksSinceLastCommand = 0;
	return (this->*_commandList[commandId])();
}

int RSound7::command9() {
	command1();
	playSoundStatic(0x1C0E, 1);
	playSoundStatic(0x1C88, 2);
	playSoundStatic(0x1CD4, 3);
	playSoundStatic(0x1D4E, 4);
	return 0;
}

int RSound7::command15() {
	playSoundDynamic(0x125C);
	return 0;
}

int RSound7::command16() {
	playSoundDynamic(0x12DE);
	return 0;
}

int RSound7::command17() {
	playSoundDynamic(0x12C2);
	return 0;
}

int RSound7::command18() {
	playSoundStatic(0x12FC, 8);
	return 0;
}

int RSound7::command19() {
	Channel *chan = getChannel(8);
	if (chan->_soundData == loadData(0x12FC)) {
		byte *pData = loadData(0x1312);
		chan->_innerLoopStart = pData;
		chan->_outerLoopStart = pData;
		chan->_deltaCounter = 1;
	}
	return 0;
}

int RSound7::command20() {
	playSoundDynamic(0x1324);
	playSoundDynamic(0x1324);
	return 0;
}

int RSound7::command21() {
	playSoundDynamic(0x1336);
	return 0;
}

int RSound7::command22() {
	playSoundDynamic(0x1340);
	return 0;
}

int RSound7::command23() {
	playSoundDynamic(0x12B4);
	return 0;
}

int RSound7::command24() {
	playSoundStatic(0x137C, 1);
	playSoundStatic(0x1406, 2);
	playSoundStatic(0x1492, 3);
	playSoundStatic(0x1516, 4);
	playSoundStatic(0x1588, 5);
	return 0;
}

int RSound7::command25() {
	command1();
	playSoundStatic(0x1612, 1);
	playSoundStatic(0x16C8, 2);
	playSoundStatic(0x177E, 3);
	playSoundStatic(0x1838, 4);
	return 0;
}

int RSound7::command27() {
	playSoundStatic(0x1932, 1);
	playSoundStatic(0x1986, 2);
	playSoundStatic(0x19EC, 3);
	playSoundStatic(0x1A66, 4);
	playSoundStatic(0x1B3C, 5);
	return 0;
}

int RSound7::command30() {
	playSoundDynamic(0x12AA);
	playSoundDynamic(0x12A0);
	return 0;
}

int RSound7::command32() {
	playSoundDynamic(0x1268);
	return 0;
}

int RSound7::command33() {
	playSoundDynamic(0x1274);
	return 0;
}

int RSound7::command34() {
	playSoundDynamic(0x127E);
	return 0;
}

int RSound7::command35() {
	playSoundDynamic(0x128E);
	return 0;
}

int RSound7::command36() {
	playSoundDynamic(0x1358);
	playSoundDynamic(0x136C);
	return 0;
}

int RSound7::command37() {
	playSoundDynamic(0x134A);
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound8::CommandPtr RSound8::_commandList[38] = {
	&RSound8::command0, &RSound8::command1, &RSound8::command2, &RSound8::command3,
	&RSound8::command4, &RSound8::command5, &RSound8::command6, &RSound8::command7,
	&RSound8::command8, &RSound8::command9, &RSound8::command10, &RSound8::command11,
	&RSound8::command12, &RSound8::command13, &RSound8::command14, &RSound8::command15,
	&RSound8::command16, &RSound8::command17, &RSound8::command18, &RSound8::command19,
	&RSound8::command20, &RSound8::command21, &RSound8::command22, &RSound8::command23,
	&RSound8::command24, &RSound8::command25, &RSound8::command26, &RSound8::command27,
	&RSound8::command28, &RSound8::command29, &RSound8::command30, &RSound8::command31,
	&RSound8::command32, &RSound8::command33, &RSound8::command34, &RSound8::command35,
	&RSound8::command36, &RSound8::command37
};

RSound8::RSound8(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver) : 
	RSound(mixer, midiDriver, "rsound.008", 0x1290, 0x19A0, 0x67, kRSoundFadeCheckProgrammable) {
}

int RSound8::command(int commandId, int param) {
	if (commandId > 37)
		return 0;

	_commandParam = param;
	_ticksSinceLastCommand = 0;
	return (this->*_commandList[commandId])();
}

int RSound8::command9() {
	playSoundDynamic(0x10BA);
	return 0;
}

int RSound8::command10() {
	playSoundStatic(0x115A, 1);
	playSoundStatic(0x115A, 2);
	playSoundStatic(0x115A, 3);
	playSoundStatic(0x1150, 4);
	return 0;
}

int RSound8::command11() {
	playSoundDynamic(0x1194);
	return 0;
}

int RSound8::command12() {
	playSoundDynamic(0x11B2);
	return 0;
}

int RSound8::command13() {
	playSoundDynamic(0x11D0);
	playSoundDynamic(0x11D0);
	playSoundDynamic(0x11D0);
	playSoundDynamic(0x11D0);
	return 0;
}

void RSound8::playCommand14_15Variant(byte v1, byte v2) {
	byte *pData = loadData(0x1204);
	pData[3] = v1;
	pData[6] = v2;
	pData[9] = v2;
	playSoundDynamic(0x1204);
	playSoundDynamic(0x1204);
	playSoundDynamic(0x1204);
	playSoundDynamic(0x1204);
}

int RSound8::command14() {
	playCommand14_15Variant(40, 1);
	return 0;
}

int RSound8::command15() {
	playCommand14_15Variant(100, 255);
	return 0;
}

int RSound8::command16() {
	playSoundDynamic(0x112E);
	playSoundDynamic(0x112E);
	return 0;
}

int RSound8::command17() {
	playSoundDynamic(0x1234);
	return 0;
}

int RSound8::command18() {
	playSoundDynamic(0x1244);
	return 0;
}

int RSound8::command19() {
	playSoundDynamic(0x1254);
	return 0;
}

int RSound8::command20() {
	playSoundDynamic(0x125E);
	return 0;
}

int RSound8::command21() {
	playSoundDynamic(0x126E);
	return 0;
}

int RSound8::command22() {
	playSoundDynamic(0x1278);
	return 0;
}

int RSound8::command23() {
	playSoundStatic(0x128E, 1);
	playSoundStatic(0x128E, 2);
	playSoundStatic(0x128E, 3);
	playSoundStatic(0x128E, 4);
	return 0;
}

int RSound8::command24() {
	playSoundDynamic(0x12B4);
	return 0;
}

int RSound8::command25() {
	playSoundDynamic(0x12CA);
	return 0;
}

int RSound8::command26() {
	playSoundDynamic(0x12DC);
	return 0;
}

int RSound8::command27() {
	playSoundDynamic(0x1112);
	return 0;
}

int RSound8::command28() {
	byte *pData = loadData(0x130A);
	if (!isSoundPlaying(pData)) {
		command1();
		playSoundStatic(pData, 1);
		playSoundStatic(0x1480, 2);
		playSoundStatic(0x154C, 9);
	}
	return 0;
}

int RSound8::command29() {
	byte *pData = loadData(0x15FC);
	if (!isSoundPlaying(pData)) {
		command1();
		playSoundStatic(pData, 3);
		playSoundStatic(0x1652, 9);
	}
	return 0;
}

int RSound8::command30() {
	playSoundDynamic(0x1108);
	playSoundDynamic(0x10FE);
	return 0;
}

int RSound8::command31() {
	playSoundDynamic(0x1140);
	return 0;
}

int RSound8::command32() {
	playSoundDynamic(0x10C6);
	return 0;
}

int RSound8::command33() {
	playSoundDynamic(0x10D2);
	return 0;
}

int RSound8::command34() {
	playSoundDynamic(0x10DC);
	return 0;
}

int RSound8::command35() {
	playSoundDynamic(0x10EC);
	return 0;
}

int RSound8::command36() {
	playSoundDynamic(0x12E6);
	playSoundDynamic(0x12FA);
	return 0;
}

int RSound8::command37() {
	playSoundDynamic(0x1120);
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound9::CommandPtr RSound9::_commandList[52] = {
	&RSound9::command0, &RSound9::command1, &RSound9::command2, &RSound9::command3,
	&RSound9::command4, &RSound9::command5, &RSound9::command6, &RSound9::command7,
	&RSound9::command8, &RSound9::command9, &RSound9::command10, &RSound9::command11,
	&RSound9::command12, &RSound9::command13, &RSound9::command14, &RSound9::command15,
	&RSound9::command16, &RSound9::command17, &RSound9::command18, &RSound9::command19,
	&RSound9::command20, &RSound9::command21, &RSound9::command22, &RSound9::command23,
	&RSound9::command24, &RSound9::command25, &RSound9::command26, &RSound9::command27,
	&RSound9::command28, &RSound9::command29, &RSound9::command30, &RSound9::command31,
	&RSound9::command32, &RSound9::command33, &RSound9::command34, &RSound9::command35,
	&RSound9::command36, &RSound9::command37, &RSound9::command38, &RSound9::command39,
	&RSound9::command40, &RSound9::command41, &RSound9::command42, &RSound9::command43,
	&RSound9::command44_46, &RSound9::command45, &RSound9::command44_46, &RSound9::command47,
	&RSound9::command48, &RSound9::command49, &RSound9::command50, &RSound9::command51
};

RSound9::RSound9(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver) : 
	RSound(mixer, midiDriver, "rsound.009", 0x1520, 0x8920, 0x6F, kRSoundFadeCheckAlternating) {
	_callbackCounter = 0;
	_callbackPeriod = 0;
	_callbackFnPtr = nullptr;
}

int RSound9::command(int commandId, int param) {
	if (commandId > 51)
		return 0;

	_commandParam = param;
	_ticksSinceLastCommand = 0;
	return (this->*_commandList[commandId])();
}

void RSound9::tickCallback() {
	if (_callbackPeriod == 0 || --_callbackCounter > 0)
		return;

	_callbackCounter = _callbackPeriod;
	if (_callbackFnPtr != nullptr)
		(this->*_callbackFnPtr)();
}

int RSound9::command0() {
	_callbackCounter = 0;
	_callbackPeriod = 0;
	_callbackFnPtr = nullptr;
	return RSound::command0();
}

int RSound9::command9() {
	_callbackCounter = 1848;
	_callbackPeriod = 84;
	playSoundStatic(0x16E4, 1);
	playSoundStatic(0x1E9E, 2);
	playSoundStatic(0x2F9C, 3);
	playSoundStatic(0x2644, 4);
	playSoundStatic(0x1FF4, 5);
	playSoundStatic(0x2382, 6);
	playSoundStatic(0x1AAE, 7);
	return 0;
}

int RSound9::command10() {
	playSoundStatic(0x31E2, 1);
	playSoundStatic(0x31FA, 2);
	playSoundStatic(0x3212, 3);
	playSoundStatic(0x322C, 4);
	return 0;
}

int RSound9::command11() {
	playSoundStatic(0x33E2, 8);
	return 0;
}

int RSound9::command12() {
	playSoundStatic(0x342E, 8);
	return 0;
}

int RSound9::command13() {
	playSoundStatic(0x343A, 8);
	return 0;
}

int RSound9::command14() {
	playSoundStatic(0x3442, 8);
	return 0;
}

int RSound9::command15() {
	playSoundStatic(0x3462, 8);
	return 0;
}

int RSound9::command16() {
	playSoundStatic(0x347A, 8);
	return 0;
}

int RSound9::command17() {
	playSoundStatic(0x3470, 8);
	return 0;
}

int RSound9::command18() {
	playSoundDynamic(0x3248);
	return 0;
}

int RSound9::command19() {
	playSoundDynamic(0x3262);
	return 0;
}

int RSound9::command20() {
	int v = (generateRandomNumber() & 24) + 77;
	byte *pData = loadData(0x3284);
	pData[6] = v & 0x7F;
	playSoundDynamic(0x3284);
	return 0;
}

int RSound9::command21() {
	byte *pData = loadData(0x3298);
	pData[9] = 70;
	if (!isSoundPlaying(pData))
		playSoundDynamic(0x3298);
	return 0;
}

int RSound9::command22() {
	byte *pData = loadData(0x3298);
	pData[9] = 45;
	if (!isSoundPlaying(pData))
		playSoundDynamic(0x3298);
	return 0;
}

int RSound9::command23() {
	Channel *chan = playSoundDynamic(0x32B0);
	if (chan != nullptr)
		chan->_innerLoopStart = loadData(0x32CE);

	chan = playSoundDynamic(0x32B6);
	if (chan != nullptr)
		chan->_innerLoopStart = loadData(0x32CE);

	chan = playSoundDynamic(0x32C8);
	if (chan != nullptr)
		chan->_innerLoopStart = loadData(0x32CE);
	return 0;
}

int RSound9::command24() {
	playSoundDynamic(0x32E0);
	return 0;
}

int RSound9::command25() {
	playSoundDynamic(0x32F6);
	return 0;
}

int RSound9::command26() {
	playSoundDynamic(0x331A);
	return 0;
}

int RSound9::command27() {
	playSoundDynamic(0x3332);
	return 0;
}

int RSound9::command28() {
	int v = (generateRandomNumber() & 28) + 15;
	byte *pData = loadData(0x334A);
	pData[6] = v & 0x7F;
	playSoundStatic(pData, 8);
	return 0;
}

int RSound9::command29() {
	int v = (generateRandomNumber() & 12) + 33;
	byte *pData = loadData(0x335E);
	pData[6] = v & 0x7F;
	playSoundDynamic(0x335E);
	return 0;
}

int RSound9::command30() {
	playSoundDynamic(0x3386);
	return 0;
}

int RSound9::command31() {
	playSoundDynamic(0x3396);
	playSoundDynamic(0x33A4);
	playSoundDynamic(0x33B2);
	return 0;
}

int RSound9::command32() {
	playSoundDynamic(0x33C0);
	return 0;
}

int RSound9::command33() {
	playSoundDynamic(0x33CA);
	return 0;
}

int RSound9::command34() {
	command1();
	_callbackCounter = 96;
	_callbackPeriod = 96;

	*loadData(0x6841) = 2;
	*loadData(0x7DCD) = 2;
	*loadData(0x8791) = 2;

	playSoundStatic(0x4D2A, 1);
	playSoundStatic(0x51AA, 2);
	playSoundStatic(0x5634, 3);
	playSoundStatic(0x6844, 4);
	playSoundStatic(0x7DD0, 5);
	return 0;
}

int RSound9::command35() {
	playSoundDynamic(0x344C);
	return 0;
}

int RSound9::command36() {
	playSoundDynamic(0x334A);

	Channel *chan = playSoundDynamic(0x32C2);
	if (chan != nullptr)
		chan->_innerLoopStart = loadData(0x3378);

	chan = playSoundDynamic(0x32BC);
	if (chan != nullptr)
		chan->_innerLoopStart = loadData(0x3368);
	return 0;
}

int RSound9::command37() {
	int v = (generateRandomNumber() & 2) + 72;
	byte *pData = loadData(0x349C);
	pData[6] = v & 0x7F;
	playSoundDynamic(0x349C);
	return 0;
}

int RSound9::command38() {
	_callbackFnPtr = &RSound9::command38Callback;
	return 0;
}

void RSound9::command38Callback() {
	_callbackFnPtr = nullptr;
	playSoundStatic(0x1878, 1);
	playSoundStatic(0x1F64, 2);
	playSoundStatic(0x308E, 3);
	playSoundStatic(0x2A10, 4);
	playSoundStatic(0x21C8, 5);
	playSoundStatic(0x2558, 6);
	playSoundStatic(0x1E9A, 7);
}

int RSound9::command39() {
	_callbackFnPtr = &RSound9::command39Callback;
	return 0;
}

void RSound9::command39Callback() {
	_callbackFnPtr = nullptr;
	playSoundStatic(0x1A24, 1);
	playSoundStatic(0x1FD0, 2);
	playSoundStatic(0x318A, 3);
	playSoundStatic(0x2E4A, 4);
	playSoundStatic(0x2380, 5);
	playSoundStatic(0x2642, 6);
	playSoundStatic(0x1E9C, 7);
}

int RSound9::command40() {
	_callbackFnPtr = &RSound9::command40Callback;
	return 0;
}

void RSound9::command40Callback() {
	_callbackFnPtr = nullptr;
	playSoundStatic(0x4F00, 1);
	playSoundStatic(0x534A, 2);
	playSoundStatic(0x5CDE, 3);
	playSoundStatic(0x6F8E, 4);
	playSoundStatic(0x8110, 5);
}

int RSound9::command41() {
	_callbackFnPtr = &RSound9::command41Callback;
	return 0;
}

void RSound9::command41Callback() {
	_callbackFnPtr = nullptr;
	playSoundStatic(0x4F26, 1);
	playSoundStatic(0x53BC, 2);
	playSoundStatic(0x5DFE, 3);
	playSoundStatic(0x747E, 4);
	playSoundStatic(0x8340, 5);
}

int RSound9::command42() {
	_callbackFnPtr = &RSound9::command42Callback;
	return 0;
}

void RSound9::command42Callback() {
	_callbackFnPtr = nullptr;
	playSoundStatic(0x4F30, 1);
	playSoundStatic(0x555C, 2);
	playSoundStatic(0x6582, 3);
	playSoundStatic(0x7A2E, 4);
	playSoundStatic(0x8480, 5);
}

int RSound9::command43() {
	_callbackCounter = 80;
	_callbackPeriod = 80;
	playSoundStatic(0x34BE, 1);
	playSoundStatic(0x3A46, 2);
	playSoundStatic(0x3F52, 3);
	playSoundStatic(0x439A, 4);
	return 0;
}

int RSound9::command44_46() {
	_callbackFnPtr = &RSound9::command44_46Callback;
	return 0;
}

void RSound9::command44_46Callback() {
	_callbackFnPtr = nullptr;
	playSoundStatic(0x3518, 1);
	playSoundStatic(0x3AA2, 2);
	playSoundStatic(0x403A, 3);
	playSoundStatic(0x4486, 4);
}

int RSound9::command45() {
	_callbackFnPtr = &RSound9::command45Callback;
	return 0;
}

void RSound9::command45Callback() {
	_callbackFnPtr = nullptr;
	playSoundStatic(0x37AC, 1);
	playSoundStatic(0x3CF8, 2);
	playSoundStatic(0x41E6, 3);
	playSoundStatic(0x4630, 4);
}

int RSound9::command47() {
	_callbackFnPtr = &RSound9::command47Callback;
	return 0;
}

void RSound9::command47Callback() {
	_callbackFnPtr = nullptr;
	playSoundStatic(0x47D6, 1);
	playSoundStatic(0x48FA, 2);
	playSoundStatic(0x4A20, 3);
	playSoundStatic(0x4BAE, 4);
}

int RSound9::command48() {
	byte *pData = loadData(0x34B0);
	pData[6] ^= 0x1F;
	pData[0xA] ^= 0x1F;
	playSoundDynamic(0x34B0);
	return 0;
}

int RSound9::command49() {
	playSoundStatic(0x12CA, 1);
	playSoundStatic(0x132A, 2);
	playSoundStatic(0x1384, 3);
	playSoundStatic(0x1666, 4);
	playSoundStatic(0x1682, 5);
	playSoundStatic(0x16A0, 6);
	playSoundStatic(0x16BE, 7);
	return 0;
}

int RSound9::command50() {
	_callbackFnPtr = &RSound9::command50Callback;
	return 0;
}

void RSound9::command50Callback() {
	_callbackFnPtr = nullptr;

	*loadData(0x6841) = 0;
	*loadData(0x7DCD) = 0;
	*loadData(0x8791) = 0;

	playSoundStatic(0x50B8, 1);
	playSoundStatic(0x7DCE, 2);
	playSoundStatic(0x676A, 3);
	playSoundStatic(0x7D08, 4);
	playSoundStatic(0x85C4, 5);
}

int RSound9::command51() {
	command1();
	_callbackCounter = 96;
	_callbackPeriod = 96;

	*loadData(0x6841) = 2;
	*loadData(0x7DCD) = 2;
	*loadData(0x8791) = 2;

	playSoundStatic(0x4D3C, 1);
	playSoundStatic(0x51EE, 2);
	playSoundStatic(0x5A62, 3);
	playSoundStatic(0x6986, 4);
	playSoundStatic(0x7E5C, 5);
	return 0;
}

/*-----------------------------------------------------------------------*/

RSoundDemo9::RSoundDemo9(Audio::Mixer *mixer, MidiDriver_MT32GM *midiDriver) :
		RSoundDemo(mixer, midiDriver, "rsound.009", 0x11D0, 0x3664, 0x69, 5) {
}

int RSoundDemo9::executeDemoCommonCommand(int commandId) {
	switch (commandId) {
	case 0:
		return RSound::command0();
	case 1:
		requestStopRange(0, 9);
		return 0;
	case 2:
		stopAndResetRange(0, 5);
		// The opening overlay repeats its first embedded DT1 record here.
		sendSysEx(0x69);
		return 0;
	case 3:
		requestStopRange(0, 5);
		return 0;
	case 4:
		stopAndResetRange(5, 4);
		return 0;
	case 5:
		requestStopRange(5, 4);
		return 0;
	case 6:
		return RSound::command6();
	case 7:
		return RSound::command7();
	case 8:
		return RSound::command8();
	default:
		return 0;
	}
}

int RSoundDemo9::command(int commandId, int param) {
	if (commandId < 0 || commandId > 39)
		return 0;

	_commandParam = param;
	_ticksSinceLastCommand = 0;
	if (commandId <= 8)
		return executeDemoCommonCommand(commandId);

	switch (commandId) {
	case 9:
	case 10:
		break;
	case 11:
		startVoice(7, 0x1454);
		break;
	case 12:
		startVoice(7, 0x14A0);
		break;
	case 13:
		startVoice(7, 0x14AC);
		break;
	case 14:
		startVoice(7, 0x14B4);
		break;
	case 15:
		startVoice(7, 0x14D4);
		break;
	case 16:
		startVoice(7, 0x14EC);
		break;
	case 17:
		startVoice(7, 0x14E2);
		break;
	case 18:
		startEffectVoice(0x12BA);
		break;
	case 19:
		startEffectVoice(0x12D4);
		break;
	case 20: {
		byte *data = sequenceData(0x12F6);
		data[6] = (byte)(((generateRandomNumber() & 0x38) + 0x4D) & 0x7F);
		startEffectVoice(0x12F6);
		break;
	}
	case 21:
	case 22: {
		byte *data = sequenceData(0x130A);
		data[9] = commandId == 21 ? 0x46 : 0x2D;
		if (!isSequenceActive(0x130A))
			startEffectVoice(0x130A);
		break;
	}
	case 23: {
		static const int sequences[] = { 0x1322, 0x1328, 0x133A };
		for (uint index = 0; index < 3; ++index) {
			const int channel = startEffectVoice(sequences[index]);
			if (channel >= 0)
				voice(channel)._innerLoopStart = loadData(0x1340);
		}
		break;
	}
	case 24:
		startEffectVoice(0x1352);
		break;
	case 25:
		startEffectVoice(0x1368);
		break;
	case 26:
		startEffectVoice(0x138C);
		break;
	case 27:
		startEffectVoice(0x13A4);
		break;
	case 28: {
		byte *data = sequenceData(0x13BC);
		data[6] = (byte)(((generateRandomNumber() & 0x1C) + 0x0F) & 0x7F);
		startEffectVoice(0x13BC);
		break;
	}
	case 29: {
		byte *data = sequenceData(0x13D0);
		data[6] = (byte)(((generateRandomNumber() & 0x0C) + 0x21) & 0x7F);
		startEffectVoice(0x13D0);
		break;
	}
	case 30:
		startEffectVoice(0x13F8);
		break;
	case 31:
		startEffectVoice(0x1408);
		startEffectVoice(0x1416);
		startEffectVoice(0x1424);
		break;
	case 32:
		startEffectVoice(0x1432);
		break;
	case 33:
		startEffectVoice(0x143C);
		break;
	case 34:
	case 39:
		startVoice(0, 0x1522);
		startVoice(1, 0x1700);
		startVoice(2, 0x1892);
		startVoice(3, 0x21F2);
		startVoice(4, 0x2E4A);
		break;
	case 35:
		startEffectVoice(0x14BE);
		break;
	case 36: {
		startEffectVoice(0x13BC);

		int channel = startEffectVoice(0x1334);
		if (channel >= 0)
			voice(channel)._innerLoopStart = loadData(0x13EA);

		channel = startEffectVoice(0x132E);
		if (channel >= 0)
			voice(channel)._innerLoopStart = loadData(0x13DA);
		break;
	}
	case 37: {
		byte *data = sequenceData(0x150E);
		data[6] = (byte)(((generateRandomNumber() & 0x02) + 0x48) & 0x7F);
		startEffectVoice(0x150E);
		break;
	}
	case 38:
		startVoice(0, 0x35C4);
		startVoice(1, 0x35CE);
		startVoice(2, 0x3612);
		startVoice(3, 0x3656);
		break;
	}

	return 0;
}

} // namespace Sound
} // namespace RexNebular
} // namespace MADS
