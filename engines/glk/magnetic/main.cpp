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

#include "glk/magnetic/magnetic.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"

namespace Glk {
namespace Magnetic {

#define WIDTH 78

type8 Magnetic::ms_load_file(const char *name, type8 *ptr, type16 size) {
	assert(name);
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(name);
	if (!file)
		return 1;

	if (file->read(ptr, size) != size) {
		delete file;
		return 1;
	}

	delete file;
	return 0;
}

type8 Magnetic::ms_save_file(const char *name, type8 *ptr, type16 size) {
	assert(name);
	Common::OutSaveFile *file = g_system->getSavefileManager()->openForSaving(name);
	if (!file)
		return 1;

	if (file->write(ptr, size) != size) {
		delete file;
		return 1;
	}

	file->finalize();
	delete file;
	return 0;
}

void Magnetic::script_write(type8 c) {
	if (log_on == 2)
		log1->writeByte(c);
}

void Magnetic::transcript_write(type8 c) {
	if (log2)
		log2->writeByte(c);
}

} // End of namespace Magnetic
} // End of namespace Glk
