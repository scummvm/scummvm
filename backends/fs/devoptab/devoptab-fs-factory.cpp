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

#if defined(__DS__)

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/fs/devoptab/devoptab-fs-factory.h"

#include <sys/iosupport.h>

DevoptabFilesystemFactory::DevoptabFilesystemFactory() {
	// skip in, out and err
	for (uint8 i = 3; i < STD_MAX; ++i) {
		const devoptab_t *dt = devoptab_list[i];

		if (!dt || !dt->name || !dt->open_r || !dt->diropen_r)
			continue;

		addDrive(Common::String(dt->name) + ":/");
	}

}

#endif
