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


#include "ultima/ultima8/world/actors/auto_firer_process.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(AutoFirerProcess)

AutoFirerProcess::AutoFirerProcess() : Process() {
	Actor *a = getControlledActor();
	if (a)
		_itemNum = a->getObjId();
	_type = 0x260; // CONSTANT !
	_startTicks = Kernel::get_instance()->getTickNum();
}

void AutoFirerProcess::run() {
	if (Kernel::get_instance()->getTickNum() > _startTicks + 10) {
		Actor *a = getControlledActor();

		if (!a) {
			terminate();
			return;
		}

		uint16 weaponno = a->getActiveWeapon();
		const Item *wpn = getItem(weaponno);
		if (wpn && wpn->getShape() == 0x38d && wpn->getShapeInfo()->_weaponInfo) {
			const WeaponInfo *info = wpn->getShapeInfo()->_weaponInfo;
			int shotsleft;
			if (info->_ammoShape) {
				shotsleft = wpn->getQuality();
			} else if (info->_energyUse) {
				shotsleft = a->getMana() / info->_energyUse;
			} else {
				shotsleft = 1;
			}
			if (shotsleft > 0) {
				int32 x = 0;
				int32 y = 0;
				int32 z = 0;
				a->addFireAnimOffsets(x, y, z);
				a->fireWeapon(x, y, z, a->getDir(), info->_damageType, true);

				AudioProcess *audioproc = AudioProcess::get_instance();
				if (audioproc && info->_sound)
					audioproc->playSFX(info->_sound, 0x80, a->getObjId(), 0, false);
			}
		}
		terminate();
	}
}

void AutoFirerProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);
	ws->writeUint32LE(_startTicks);
}

bool AutoFirerProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_startTicks = rs->readUint32LE();
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
