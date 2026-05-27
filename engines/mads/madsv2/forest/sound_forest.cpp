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

#include "common/endian.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/textconsole.h"
#include "mads/madsv2/forest/sound_forest.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {

void ForestSoundManager::validate() {
	Common::File f;
	static const char *const MD5[] = {
		"cac84f53ccf18ca56f4c03352037790f",
		"2dcdbe18ca5225384cdb97ceb7f5642a",
		"c6001b0dfe32cb9399ab60742b631c2e",
		"1596b657c6171e13714eaf114bf94641",
		"ecbb8bdf1e2e36fcacedce79761e625b",
		nullptr,
		nullptr,
		nullptr,
		"379fcc9af2142f15a3e7166eee6dd49d"
	};

	for (int i = 1; i <= 9; ++i) {
		if (i == 7 || i == 8)
			continue;
		Common::Path filename(Common::String::format("asound.dr%d", i));
		if (!f.open(filename))
			error("Could not process - %s", filename.toString().c_str());
		Common::String md5str = Common::computeStreamMD5AsString(f, 8192);
		f.close();

		if (md5str != MD5[i - 1])
			error("Invalid sound file - %s", filename.toString().c_str());
	}
}

void ForestSoundManager::loadDriver(int sectionNumber) {
	// TODO
}

} // namespace Forest
} // namespace MADSV2
} // namespace MADS
