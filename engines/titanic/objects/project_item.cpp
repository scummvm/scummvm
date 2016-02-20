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

void CProjectItem::save(SimpleFile *file, int indent) const {
	error("TODO");
}

void CProjectItem::load(SimpleFile *file) {
	error("TODO");
}

void CProjectItem::loadGame(int slotId) {
	CompressedFile file;
	Common::InSaveFile *saveFile = nullptr;

	// Clear any existing project contents
	clear();

	// Open either an existing savegame slot or the new game template
	if (slotId > 0) {
		saveFile = g_system->getSavefileManager()->openForLoading(
			Common::String::format("slot%d.gam", slotId));
		file.open(saveFile);
	} else {
		file.open("newgame.st");
	}

	// Load the contents in
	loadData(&file);

	file.close();
}

void CProjectItem::saveGame(int slotId) {
	CompressedFile file;
	Common::OutSaveFile *saveFile = g_system->getSavefileManager()->openForSaving(
		Common::String::format("slot%d.gam", slotId));
	file.open(saveFile);

	// Save the contents out
	saveData(&file, this);

	file.close();
}

void CProjectItem::clear() {

}

CProjectItem *CProjectItem::loadData(SimpleFile *file) {
	if (!file->IsClassStart())
		return nullptr;

	CProjectItem *root = nullptr;
	CTreeItem *parent = nullptr;
	CTreeItem *item = nullptr;

	do {
		CString entryString = file->readString();

		if (entryString == "ALONG") {
			// Move along, nothing needed
		} else if (entryString == "UP") {
			// Move up
			if (parent == nullptr ||
				(parent = parent->getParent()) == nullptr)
				break;
		} else if (entryString == "DOWN") {
			// Move down
			if (parent == nullptr)
				parent = item;
			else
				parent = parent->getLastChild();
		} else {
			// Create new class instance
			item = dynamic_cast<CTreeItem *>(CSaveableObject::createInstance(entryString));
			assert(item);

			if (root) {
				// Already created root project
				item->addUnder(parent);
			} else {
				// TODO: Validate this is correct
				root = dynamic_cast<CProjectItem *>(item);
				assert(root);

				_filename = root->_filename;
			}

			// Load the data for the item
			item->load(file);
		}

		file->IsClassStart();
	} while (file->IsClassStart());

	return root;
}

void CProjectItem::saveData(SimpleFile *file, CTreeItem *item) const {
	while (item) {
		item->saveHeader(file, 0);
		item->save(file, 1);
		item->saveFooter(file, 0);

	
		CTreeItem *child = item->getFirstChild();
		if (child) {
			file->write("\n{\n", 3);
			file->writeQuotedString("DOWN");
			file->write("\n}\n", 3);
			saveData(file, child);
			file->write("\n{\n", 3);
			file->writeQuotedString("UP");
		} else {
			file->write("\n{\n", 3);
			file->writeQuotedString("ALONG");
		}
		
		file->write("\n}\n", 3);
		item = item->getNextSibling();
	}
}

} // End of namespace Titanic
