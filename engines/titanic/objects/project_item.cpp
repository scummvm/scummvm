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

#include "common/savefile.h"
#include "titanic/titanic.h"
#include "titanic/compressed_file.h"
#include "titanic/objects/project_item.h"

namespace Titanic {

void CProjectItem::load(int id) {
	CompressedFile file;
	Common::InSaveFile *saveFile = nullptr;

	// Clear any existing project contents
	clear();

	// Open either an existing savegame slot or the new game template
	if (id > 0) {
		saveFile = g_system->getSavefileManager()->openForLoading(
			Common::String::format("slot%d.gam", id));
		file.open(saveFile);
	} else {
		file.open("newgame.st");
	}

	// Load the contents in
	loadData(file);

	file.close();
}

void CProjectItem::clear() {

}

void CProjectItem::loadData(SimpleFile &file) {

}

} // End of namespace Titanic
