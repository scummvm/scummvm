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

#include "ultima/ultima8/graphics/inverter_process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/ultima8.h"


namespace Ultima {
namespace Ultima8 {

static unsigned int states[] = { 0, 8, 63, 211, 493, 945, 1594, 2459, 3552,
                                 4870, 6406, 8139, 10042, 12078, 14207, 16384,
                                 18561, 20690, 22726, 24629, 26362, 27898,
                                 29216, 30308, 31174, 31823, 32274, 32556,
                                 32704, 32760, 32768,
                                 32775, 32831, 32979, 33261, 33713, 34362,
                                 35227, 36320, 37638, 39174, 40907, 42810,
                                 44846, 46975, 49152, 51328, 53457, 55494,
                                 57396, 59129, 60665, 61984, 63076, 63942,
                                 64591, 65042, 65324, 65472, 65528, 65536
                               };

InverterProcess *InverterProcess::_inverter = nullptr;

// p_dynamic_class stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(InverterProcess)

InverterProcess::InverterProcess()
	: Process(), _targetState(0) {

}

InverterProcess::InverterProcess(unsigned int target)
	: Process(), _targetState(target) {
}

InverterProcess::~InverterProcess(void) {
	if (_inverter == this)
		_inverter = nullptr;
}

void InverterProcess::run() {
	Ultima8Engine *app = Ultima8Engine::get_instance();

	unsigned int state = app->getInversion();
	if (state == _targetState) {
		terminate();
	} else {
		unsigned int i = 0;
		while (states[i] <= state) i++;
		app->setInversion(states[i]);
	}
}

void InverterProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint16LE(static_cast<uint16>(_targetState));
}

bool InverterProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_targetState = rs->readUint16LE();

	_inverter = this; //static
	return true;
}

// static
ProcId InverterProcess::invertScreen() {
	if (_inverter) {
		if (_inverter->_targetState == 0)
			_inverter->setTarget(0x8000);
		else
			_inverter->setTarget(0);
		return _inverter->getPid();
	} else {
		unsigned int target = 0x8000;
		if (Ultima8Engine::get_instance()->isInverted()) target = 0;
		_inverter = new InverterProcess(target);
		return Kernel::get_instance()->addProcess(_inverter);
	}
}


uint32 InverterProcess::I_invertScreen(const uint8 *args,
                                       unsigned int /*argsize*/) {
	return invertScreen();
}

} // End of namespace Ultima8
} // End of namespace Ultima
