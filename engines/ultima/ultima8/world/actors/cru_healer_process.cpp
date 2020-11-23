/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/world/actors/cru_healer_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/audio/audio_process.h"

#include "common/util.h"

namespace Ultima {
namespace Ultima8 {


// These SFX IDs are the same in both No Regret and No Remorse.
static const uint16 HEAL_START_SFX = 0xdb;
static const uint16 HEAL_GOING_SFX = 0xba;

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(CruHealerProcess)

CruHealerProcess::CruHealerProcess() : Process() {
	MainActor *avatar = dynamic_cast<MainActor *>(getActor(World::get_instance()->getControlledNPCNum()));
	if (!avatar) {
		_itemNum = 0;
		_targetMaxHP = 0;
	} else {
		_itemNum = avatar->getObjId();
		_targetMaxHP = avatar->getMaxHP();
		AudioProcess *audio = AudioProcess::get_instance();
		if (audio) {
			// Sound num is the same in both No Remorse and No Regret
			audio->playSFX(HEAL_START_SFX, 0x80, _itemNum, 1, false);
		}
	}
	Ultima8Engine::get_instance()->setAvatarInStasis(true);
	_type = 0x254; // CONSTANT!
}

void CruHealerProcess::run() {
	MainActor *avatar = dynamic_cast<MainActor *>(getActor(World::get_instance()->getControlledNPCNum()));
	AudioProcess *audio = AudioProcess::get_instance();

	if (!avatar || avatar->isDead() || avatar->getHP() >= _targetMaxHP) {
		if (avatar && avatar->getHP() >= _targetMaxHP) {
			Ultima8Engine::get_instance()->setAvatarInStasis(false);
		}
		// dead or finished healing
		if (audio)
			audio->stopSFX(HEAL_START_SFX, _itemNum);
		terminate();
		return;
	}

    if (audio && !audio->isSFXPlayingForObject(HEAL_GOING_SFX, _itemNum))
		audio->playSFX(HEAL_GOING_SFX, 0x80, _itemNum, 1);

	uint16 newHP = avatar->getHP() + 1;
	if (newHP > _targetMaxHP)
		newHP = _targetMaxHP;

	avatar->setHP(newHP);
}

uint32 CruHealerProcess::I_create(const uint8 *args, unsigned int /*argsize*/) {
	return Kernel::get_instance()->addProcess(new CruHealerProcess());
}

void CruHealerProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);
	ws->writeUint16LE(_targetMaxHP);
}

bool CruHealerProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;
	_targetMaxHP = rs->readUint16LE();
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
