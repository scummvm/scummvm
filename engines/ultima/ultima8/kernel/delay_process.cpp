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

#include "ultima/ultima8/kernel/delay_process.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(DelayProcess)



DelayProcess::DelayProcess(int count) : Process(), _count(count) {
}

DelayProcess::~DelayProcess() {
}

void DelayProcess::run() {
	if (--_count == 0)
		terminate();
}

Common::String DelayProcess::dumpInfo() const {
	return Process::dumpInfo() +
		Common::String::format(", frames left: %d", _count);
}


bool DelayProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_count = static_cast<int>(rs->readUint32LE());

	return true;
}

void DelayProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);
	ws->writeUint32LE(static_cast<uint32>(_count));
}

} // End of namespace Ultima8
} // End of namespace Ultima
