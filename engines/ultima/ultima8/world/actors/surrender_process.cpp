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

#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/world/actors/surrender_process.h"
#include "ultima/ultima8/world/actors/attack_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(SurrenderProcess)

static const uint16 SUIT_SUR_SNDS[] = {0xe9, 0xe0, 0xeb, 0xe1, 0xea};
static const uint16 CHEMSUIT_SUR_SNDS[] = {0xb4, 0xc5, 0xc6, 0xe8};
static const uint16 SCIENTIST_SUR_SNDS[] = {0xe3, 0xe4, 0xec, 0xf6};
static const uint16 HARDHAT_SUR_SNDS[] = {0xde, 0xdf, 0x8a, 0x8b};
static const uint16 FEMALE_SUR_SNDS[] = {0xd6, 0xff, 0xd7};

#define RANDOM_ELEM(array) (array[rs.getRandomNumber(ARRAYSIZE(array) - 1)])

SurrenderProcess::SurrenderProcess() :
	_playedSound(false), _soundDelayTicks(480), _soundTimestamp(0)
{
}

SurrenderProcess::SurrenderProcess(Actor *actor) :
	_playedSound(false), _soundDelayTicks(480), _soundTimestamp(0)
{
	assert(actor);
	_itemNum = actor->getObjId();

	if (!actor->hasActorFlags(Actor::ACT_SURRENDERED))
		actor->doAnim(Animation::surrender, actor->getDir());

	if (GAME_IS_REGRET) {
		Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
		_soundDelayTicks = rs.getRandomNumberRng(10, 24) * 60;
		if (rs.getRandomNumber(2) == 0)
			_soundTimestamp = Kernel::get_instance()->getTickNum();
	}

	_type = 0x25f; // CONSTANT!
}

void SurrenderProcess::run() {
	Actor *a = getActor(_itemNum);
	const MainActor *main = getMainActor();
	if (!a || a->isDead() || !main) {
		// dead
		terminate();
		return;
	}

	// do nothing while we are not in the fast area or busy
	if (!a->hasFlags(Item::FLG_FASTAREA) || a->isBusy())
		return;

	a->setActorFlag(Actor::ACT_SURRENDERED);

	Direction curdir = a->getDir();
	Direction direction = a->getDirToItemCentre(*main);

	if (curdir != direction) {
		uint16 animpid = a->turnTowardDir(direction);
		if (animpid) {
			waitFor(animpid);
		}
		return;
	}

	int16 soundno;
	if (GAME_IS_REMORSE)
		soundno = checkRandomSoundRemorse();
	else
		soundno = checkRandomSoundRegret();

	AudioProcess *audio = AudioProcess::get_instance();
	if (soundno != -1 && audio) {
		audio->playSFX(soundno, 0x80, _itemNum, 1);
	}
}

int16 SurrenderProcess::checkRandomSoundRemorse() {
	const Actor *a = getActor(_itemNum);
	const MainActor *main = getMainActor();
	if (_playedSound || a->getRangeIfVisible(*main) == 0)
		// Nothing to do.
		return - 1;

	_playedSound = true;

	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
	int16 soundno = -1;

	switch (a->getShape()) {
	case 0x2f7: // suit
		soundno = RANDOM_ELEM(SUIT_SUR_SNDS);
		break;
	case 0x2f5: // hardhat
		soundno = RANDOM_ELEM(HARDHAT_SUR_SNDS);
		break;
	case 0x2f6: // chemsuit
		soundno = RANDOM_ELEM(CHEMSUIT_SUR_SNDS);
		break;
	case 0x344: // chemsuit
		soundno = RANDOM_ELEM(SCIENTIST_SUR_SNDS);
		break;
	case 0x597: // female office worker
		soundno = RANDOM_ELEM(FEMALE_SUR_SNDS);
		break;
	}

	return soundno;
}

int16 SurrenderProcess::checkRandomSoundRegret() {
	AudioProcess *audio = AudioProcess::get_instance();

	const Actor *a = getActor(_itemNum);

	if (!readyForNextSoundRegret())
		return -1;

	if (audio->isSFXPlayingForObject(-1, a->getObjId()))
		return -1;

	return AttackProcess::getRandomAttackSoundRegret(a);
}

//
// This and the initializer in the constructor are duplicated logic from
// AttackProcess.  In the original No Regret code they inherit from the same
// type, but that makes the No Remorse code more messy for us since we support
// both, so just live with a bit of mess in the code.
//
bool SurrenderProcess::readyForNextSoundRegret() {
	uint32 now = Kernel::get_instance()->getTickNum();
	if (_soundTimestamp == 0 || now - _soundTimestamp >= _soundDelayTicks) {
		_soundTimestamp = now;
		return true;
	}
	return false;
}

void SurrenderProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);
	if (GAME_IS_REMORSE) {
		ws->writeByte(_playedSound ? 1 : 0);
	} else {
		ws->writeUint32LE(_soundDelayTicks);
		ws->writeUint32LE(_soundTimestamp);
	}
}

bool SurrenderProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;
	if (GAME_IS_REMORSE) {
		_playedSound = rs->readByte() != 0;
	} else {
		_soundDelayTicks = rs->readUint32LE();
		_soundTimestamp = rs->readUint32LE();
	}
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
