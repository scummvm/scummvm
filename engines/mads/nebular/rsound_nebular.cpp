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

#include "mads/nebular/rsound_nebular.h"

namespace MADS {
namespace RexNebular {

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

RSound9::RSound9(Audio::Mixer *mixer) : RSound(mixer, "rsound.009", 0x1520, 0x8920) {
	_callbackCounter = 0;
	_callbackPeriod = 0;
	_callbackFnPtr = nullptr;
}

int RSound9::command(int commandId, int param) {
	if (commandId > 51)
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

void RSound9::tickCallback() {
	if (!_callbackPeriod)
		return;
	if (--_callbackCounter)
		return;

	_callbackCounter = _callbackPeriod;
	if (_callbackFnPtr)
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
	_channels[0].load(loadData(0x16E4));
	_channels[1].load(loadData(0x1E9E));
	_channels[2].load(loadData(0x2F9C));
	_channels[3].load(loadData(0x2644));
	_channels[4].load(loadData(0x1FF4));
	_channels[5].load(loadData(0x2382));
	_channels[6].load(loadData(0x1AAE));
	return 0;
}

int RSound9::command10() {
	_channels[0].load(loadData(0x31E2));
	_channels[1].load(loadData(0x31FA));
	_channels[2].load(loadData(0x3212));
	_channels[3].load(loadData(0x322C));
	return 0;
}

int RSound9::command11() {
	_channels[7].load(loadData(0x33E2));
	return 0;
}

int RSound9::command12() {
	_channels[7].load(loadData(0x342E));
	return 0;
}

int RSound9::command13() {
	_channels[7].load(loadData(0x343A));
	return 0;
}

int RSound9::command14() {
	_channels[7].load(loadData(0x3442));
	return 0;
}

int RSound9::command15() {
	_channels[7].load(loadData(0x3462));
	return 0;
}

int RSound9::command16() {
	_channels[7].load(loadData(0x347A));
	return 0;
}

int RSound9::command17() {
	_channels[7].load(loadData(0x3470));
	return 0;
}

int RSound9::command18() {
	playSound(0x3248);
	return 0;
}

int RSound9::command19() {
	playSound(0x3262);
	return 0;
}

int RSound9::command20() {
	int v = (getRandomNumber() & 24) + 77;
	byte *pData = loadData(0x3284);
	pData[6] = v & 0x7F;
	playSound(0x3284);
	return 0;
}

int RSound9::command21() {
	byte *pData = loadData(0x3298);
	pData[9] = 70;
	if (!isSoundActive(pData))
		playSound(0x3298);
	return 0;
}

int RSound9::command22() {
	byte *pData = loadData(0x3298);
	pData[9] = 45;
	if (!isSoundActive(pData))
		playSound(0x3298);
	return 0;
}

int RSound9::command23() {
	Channel *chan = playSound(0x32B0);
	if (chan)
		chan->_innerLoopPtr = loadData(0x32CE);

	chan = playSound(0x32B6);
	if (chan)
		chan->_innerLoopPtr = loadData(0x32CE);

	chan = playSound(0x32C8);
	if (chan)
		chan->_innerLoopPtr = loadData(0x32CE);
	return 0;
}

int RSound9::command24() {
	playSound(0x32E0);
	return 0;
}

int RSound9::command25() {
	playSound(0x32F6);
	return 0;
}

int RSound9::command26() {
	playSound(0x331A);
	return 0;
}

int RSound9::command27() {
	playSound(0x3332);
	return 0;
}

int RSound9::command28() {
	int v = (getRandomNumber() & 28) + 15;
	byte *pData = loadData(0x334A);
	pData[6] = v & 0x7F;
	_channels[7].load(pData);
	return 0;
}

int RSound9::command29() {
	int v = (getRandomNumber() & 12) + 33;
	byte *pData = loadData(0x335E);
	pData[6] = v & 0x7F;
	playSound(0x335E);
	return 0;
}

int RSound9::command30() {
	playSound(0x3386);
	return 0;
}

int RSound9::command31() {
	playSound(0x3396);
	playSound(0x33A4);
	playSound(0x33B2);
	return 0;
}

int RSound9::command32() {
	playSound(0x33C0);
	return 0;
}

int RSound9::command33() {
	playSound(0x33CA);
	return 0;
}

int RSound9::command34() {
	command1();
	_callbackCounter = 96;
	_callbackPeriod = 96;

	*loadData(0x6841) = 2;
	*loadData(0x7DCD) = 2;
	*loadData(0x8791) = 2;

	_channels[0].load(loadData(0x4D2A));
	_channels[1].load(loadData(0x51AA));
	_channels[2].load(loadData(0x5634));
	_channels[3].load(loadData(0x6844));
	_channels[4].load(loadData(0x7DD0));
	return 0;
}

int RSound9::command35() {
	playSound(0x344C);
	return 0;
}

int RSound9::command36() {
	playSound(0x334A);

	Channel *chan = playSound(0x32C2);
	if (chan)
		chan->_innerLoopPtr = loadData(0x3378);

	chan = playSound(0x32BC);
	if (chan)
		chan->_innerLoopPtr = loadData(0x3368);
	return 0;
}

int RSound9::command37() {
	int v = (getRandomNumber() & 2) + 72;
	byte *pData = loadData(0x349C);
	pData[6] = v & 0x7F;
	playSound(0x349C);
	return 0;
}

int RSound9::command38() {
	_callbackFnPtr = &RSound9::loadCommand38;
	return 0;
}

void RSound9::loadCommand38() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x1878));
	_channels[1].load(loadData(0x1F64));
	_channels[2].load(loadData(0x308E));
	_channels[3].load(loadData(0x2A10));
	_channels[4].load(loadData(0x21C8));
	_channels[5].load(loadData(0x2558));
	_channels[6].load(loadData(0x1E9A));
}

int RSound9::command39() {
	_callbackFnPtr = &RSound9::loadCommand39;
	return 0;
}

void RSound9::loadCommand39() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x1A24));
	_channels[1].load(loadData(0x1FD0));
	_channels[2].load(loadData(0x318A));
	_channels[3].load(loadData(0x2E4A));
	_channels[4].load(loadData(0x2380));
	_channels[5].load(loadData(0x2642));
	_channels[6].load(loadData(0x1E9C));
}

int RSound9::command40() {
	_callbackFnPtr = &RSound9::loadCommand40;
	return 0;
}

void RSound9::loadCommand40() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x4F00));
	_channels[1].load(loadData(0x534A));
	_channels[2].load(loadData(0x5CDE));
	_channels[3].load(loadData(0x6F8E));
	_channels[4].load(loadData(0x8110));
}

int RSound9::command41() {
	_callbackFnPtr = &RSound9::loadCommand41;
	return 0;
}

void RSound9::loadCommand41() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x4F26));
	_channels[1].load(loadData(0x53BC));
	_channels[2].load(loadData(0x5DFE));
	_channels[3].load(loadData(0x747E));
	_channels[4].load(loadData(0x8340));
}

int RSound9::command42() {
	_callbackFnPtr = &RSound9::loadCommand42;
	return 0;
}

void RSound9::loadCommand42() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x4F30));
	_channels[1].load(loadData(0x555C));
	_channels[2].load(loadData(0x6582));
	_channels[3].load(loadData(0x7A2E));
	_channels[4].load(loadData(0x8480));
}

int RSound9::command43() {
	_callbackCounter = 80;
	_callbackPeriod = 80;
	_channels[0].load(loadData(0x34BE));
	_channels[1].load(loadData(0x3A46));
	_channels[2].load(loadData(0x3F52));
	_channels[3].load(loadData(0x439A));
	return 0;
}

int RSound9::command44_46() {
	_callbackFnPtr = &RSound9::loadCommand44_46;
	return 0;
}

void RSound9::loadCommand44_46() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x3518));
	_channels[1].load(loadData(0x3AA2));
	_channels[2].load(loadData(0x403A));
	_channels[3].load(loadData(0x4486));
}

int RSound9::command45() {
	_callbackFnPtr = &RSound9::loadCommand45;
	return 0;
}

void RSound9::loadCommand45() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x37AC));
	_channels[1].load(loadData(0x3CF8));
	_channels[2].load(loadData(0x41E6));
	_channels[3].load(loadData(0x4630));
}

int RSound9::command47() {
	_callbackFnPtr = &RSound9::loadCommand47;
	return 0;
}

void RSound9::loadCommand47() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x47D6));
	_channels[1].load(loadData(0x48FA));
	_channels[2].load(loadData(0x4A20));
	_channels[3].load(loadData(0x4BAE));
}

int RSound9::command48() {
	byte *pData = loadData(0x34B0);
	pData[6] ^= 0x1F;
	pData[0xA] ^= 0x1F;
	playSound(0x34B0);
	return 0;
}

int RSound9::command49() {
	_channels[0].load(loadData(0x12CA));
	_channels[1].load(loadData(0x132A));
	_channels[2].load(loadData(0x1384));
	_channels[3].load(loadData(0x1666));
	_channels[4].load(loadData(0x1682));
	_channels[5].load(loadData(0x16A0));
	_channels[6].load(loadData(0x16BE));
	return 0;
}

int RSound9::command50() {
	_callbackFnPtr = &RSound9::loadCommand50;
	return 0;
}

void RSound9::loadCommand50() {
	_callbackFnPtr = nullptr;

	*loadData(0x6841) = 0;
	*loadData(0x7DCD) = 0;
	*loadData(0x8791) = 0;

	_channels[0].load(loadData(0x50B8));
	_channels[1].load(loadData(0x7DCE));
	_channels[2].load(loadData(0x676A));
	_channels[3].load(loadData(0x7D08));
	_channels[4].load(loadData(0x85C4));
}

int RSound9::command51() {
	command1();
	_callbackCounter = 96;
	_callbackPeriod = 96;

	*loadData(0x6841) = 2;
	*loadData(0x7DCD) = 2;
	*loadData(0x8791) = 2;

	_channels[0].load(loadData(0x4D3C));
	_channels[1].load(loadData(0x51EE));
	_channels[2].load(loadData(0x5A62));
	_channels[3].load(loadData(0x6986));
	_channels[4].load(loadData(0x7E5C));
	return 0;
}

} // namespace RexNebular
} // namespace MADS
