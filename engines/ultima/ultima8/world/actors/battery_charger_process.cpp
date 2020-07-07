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
#include "ultima/ultima8/world/actors/battery_charger_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/audio/audio_process.h"

#include "common/util.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(BatteryChargerProcess)

BatteryChargerProcess::BatteryChargerProcess() : Process() {
	// TODO: This should be the "current" avatar (if controlling a robot etc)
	MainActor *avatar = getMainActor();
	if (!avatar) {
		_itemNum = 0;
		_targetMaxEnergy = 0;
	} else {
		_itemNum = avatar->getObjId();
		_targetMaxEnergy = avatar->getMaxEnergy();
		AudioProcess *audio = AudioProcess::get_instance();
		if (audio) {
			audio->playSFX(0xa4, 0x80, _itemNum, 1, false);
		}
	}
	_type = 0x254; // CONSTANT!
}

void BatteryChargerProcess::run() {
	// TODO: This should be the "current" avatar (if controlling a robot etc)
	MainActor *avatar = getMainActor();
	AudioProcess *audio = AudioProcess::get_instance();

	if (!avatar || avatar->isDead() || avatar->getMana() >= _targetMaxEnergy) {
		// dead or finished healing
		terminate();
		if (audio)
			audio->stopSFX(0xa4, _itemNum);
		return;
	}

    if (!audio->isSFXPlayingForObject(0x10b, _itemNum))
		audio->playSFX(0x10b, 0x80, _itemNum, 1);

	uint16 newEnergy = avatar->getMana() + 25;
	if (newEnergy > _targetMaxEnergy)
		newEnergy = _targetMaxEnergy;

	avatar->setMana(newEnergy);
}

uint32 BatteryChargerProcess::I_create(const uint8 *args, unsigned int /*argsize*/) {
	return Kernel::get_instance()->addProcess(new BatteryChargerProcess());
}

void BatteryChargerProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);
	ws->writeUint16LE(_targetMaxEnergy);
}

bool BatteryChargerProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;
	_targetMaxEnergy = rs->readUint16LE();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
