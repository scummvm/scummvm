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

#include "common/str.h"
#include "common/textconsole.h"
#include "m4/adv_db_r/db_catalog.h"
#include "m4/adv_r/db_env.h"

namespace M4 {

char *env_find(const Common::String &descName) {
	static char name[144];
	static char resultPath[144];
	int32 sceneCode;
	Common::strcpy_s(name, descName.c_str());

	if (descName.hasPrefixIgnoreCase(".raw") || descName.hasPrefixIgnoreCase(".hmp")) {
		return name;

	} else {
		db_rmlst_get_asset_room_path(name, resultPath, &sceneCode);
		if (strlen(resultPath) == 0)
			return nullptr;

		env_get_path(name, sceneCode, resultPath);
		return name;
	}
}

char *env_get_path(char *resultPath, int room_num, char *fileName) {
	error("env_get_path not implemented in ScummVM");
}

} // End of namespace M4
