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

#include "common/file.h"
#include "common/savefile.h"
#include "titanic/game_manager.h"
#include "titanic/titanic.h"
#include "titanic/core/dont_save_file_item.h"
#include "titanic/core/project_item.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

void CFileListItem::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(0, indent);
	file->writeQuotedLine(_name, indent);

	ListItem::save(file, indent);
}

void CFileListItem::load(SimpleFile *file) {
	file->readNumber();
	_name = file->readString();

	ListItem::load(file);
}

/*------------------------------------------------------------------------*/

CProjectItem::CProjectItem() : _nextRoomNumber(0), _nextMessageNumber(0),
		_nextObjectNumber(0), _gameManager(nullptr) {
}

void CProjectItem::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(6, indent);
	file->writeQuotedLine("Next Avail. Object Number", indent);
	file->writeNumberLine(_nextObjectNumber, indent);
	file->writeQuotedLine("Next Avail. Message Number", indent);
	file->writeNumberLine(_nextMessageNumber, indent);
	file->writeQuotedLine("Next Avail. Room Number", indent);
	file->writeNumberLine(_nextRoomNumber, indent);

	CTreeItem::save(file, indent);
}

void CProjectItem::buildFilesList() {
	_files.destroyContents();

	CTreeItem *treeItem = getFirstChild();
	while (treeItem) {
		if (treeItem->isFileItem()) {
			CString name = static_cast<CFileItem *>(treeItem)->getFilename();
			_files.add()->_name = name;
		}

		treeItem = getNextSibling();
	}
}

void CProjectItem::load(SimpleFile *file) {
	int val = file->readNumber();
	_files.destroyContents();
	int count;

	switch (val) {
	case 1:
		file->readBuffer();
		_nextRoomNumber = file->readNumber();
		// Deliberate fall-through

	case 0:
		// Load the list of files
		count = file->readNumber();
		for (int idx = 0; idx < count; ++idx) {
			CString name = file->readString();
			_files.add()->_name = name;
		}
		break;

	case 6:
		file->readBuffer();
		_nextObjectNumber = file->readNumber();
		// Deliberate fall-through

	case 5:
		file->readBuffer();
		_nextMessageNumber = file->readNumber();
		// Deliberate fall-through

	case 4:
		file->readBuffer();
		// Deliberate fall-through

	case 2:
	case 3:
		_files.load(file);
		file->readBuffer();
		_nextRoomNumber = file->readNumber();
		break;

	default:
		break;
	}

	CTreeItem::load(file);
}

CGameManager *CProjectItem::getGameManager() const {
	return _gameManager;
}

void CProjectItem::resetGameManager() {
	_gameManager = nullptr;
}

void CProjectItem::loadGame(int slotId) {
	CompressedFile file;

	// Clear any existing project contents
	clear();

	// Open either an existing savegame slot or the new game template
	if (slotId >= 0) {
		Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(
			Common::String::format("slot%d.gam", slotId));
		file.open(saveFile);
	} else {
		Common::File *newFile = new Common::File();
		if (!newFile->open("newgame.st"))
			error("Could not open newgame.st");
		file.open(newFile);
	}

	// Load the contents in
	CProjectItem *newProject = loadData(&file);
	file.IsClassStart();
	getGameManager()->load(&file);

	file.close();

	// Clear existing project
	clear();

	// Detach each item under the loaded project, and re-attach them
	// to the existing project instance (this)
	CTreeItem *item;
	while ((item = newProject->getFirstChild()) != nullptr) {
		item->detach();
		item->addUnder(this);
	}

	// Loaded project instance is no longer needed
	newProject->destroyAll();

	// Post-load processing
	gameLoaded();
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
	CTreeItem *item;
	while ((item = getFirstChild()) != nullptr)
		item->destroyAll();
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
				root = dynamic_cast<CProjectItem *>(item);
				assert(root);
				root->_filename = _filename;
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

void CProjectItem::gameLoaded() {
	CGameManager *gameManager = getGameManager();
	if (gameManager)
		gameManager->gameLoaded();

	CPetControl *petControl = getPetControl();
	if (petControl)
		petControl->gameLoaded();
}

CPetControl *CProjectItem::getPetControl() const {
	CDontSaveFileItem *fileItem = getDontSaveFileItem();
	CTreeItem *treeItem;

	if (!fileItem || (treeItem = fileItem->getLastChild()) == nullptr)
		return nullptr;

	while (treeItem) {
		CPetControl *petControl = dynamic_cast<CPetControl *>(treeItem);
		if (petControl)
			return petControl;

		treeItem = treeItem->getPriorSibling();
	}

	return nullptr;
}

CRoomItem *CProjectItem::findFirstRoom() const {
	return dynamic_cast<CRoomItem *>(findChildInstance(*CRoomItem::_type));
}

CTreeItem *CProjectItem::findChildInstance(ClassDef &classDef) const {
	CTreeItem *treeItem = getFirstChild();
	if (treeItem == nullptr)
		return nullptr;

	do {
		CTreeItem *childItem = treeItem->getFirstChild();
		if (childItem) {
			do {
				if (childItem->isInstanceOf(classDef))
					return childItem;
			} while ((childItem = childItem->getNextSibling()) != nullptr);
		}
	} while ((treeItem = treeItem->getNextSibling()) != nullptr);

	return nullptr;
}

CRoomItem *CProjectItem::findNextRoom(CRoomItem *priorRoom) const {
	return dynamic_cast<CRoomItem *>(findSiblingInstanceOf(*CRoomItem::_type, priorRoom));
}

CTreeItem *CProjectItem::findSiblingInstanceOf(ClassDef &classDef, CTreeItem *startItem) const {
	CTreeItem *treeItem = startItem->getParent()->getNextSibling();
	if (treeItem == nullptr)
		return nullptr;

	return findChildInstance(classDef);
}

CDontSaveFileItem *CProjectItem::getDontSaveFileItem() const {
	for (CTreeItem *treeItem = getFirstChild(); treeItem; treeItem = treeItem->getNextSibling()) {
		if (treeItem->isInstanceOf(*CDontSaveFileItem::_type))
			return dynamic_cast<CDontSaveFileItem *>(treeItem);
	}

	return nullptr;
}

CRoomItem *CProjectItem::findHiddenRoom() const {
	return dynamic_cast<CRoomItem *>(findByName("HiddenRoom"));
}

CNamedItem *CProjectItem::findByName(const CString &name, int maxChars) const {
	/*
	CString nameLower = name;
	nameLower.toLowercase();

	CTreeItem *treeItem = this;
	while (treeItem) {
		CString nodeName = treeItem->getName();

	}
	*/
	return nullptr;
}


} // End of namespace Titanic
