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

#include "titanic/core/dont_save_file_item.h"
#include "titanic/core/node_item.h"
#include "titanic/core/project_item.h"
#include "titanic/core/view_item.h"
#include "titanic/events.h"
#include "titanic/game_manager.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/titanic.h"
#include "common/file.h"
#include "common/savefile.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"

namespace Titanic {

#define CURRENT_SAVEGAME_VERSION 1
#define MINIMUM_SAVEGAME_VERSION 1

static const char *const SAVEGAME_STR = "TNIC";
#define SAVEGAME_STR_SIZE 4

EMPTY_MESSAGE_MAP(CProjectItem, CFileItem);

/*------------------------------------------------------------------------*/

void TitanicSavegameHeader::clear() {
	_version = 0;
	_saveName = "";
	_thumbnail = nullptr;
	_year = _month = _day = 0;
	_hour = _minute = 0;
	_totalFrames = 0;
}

/*------------------------------------------------------------------------*/

void CFileListItem::save(SimpleFile *file, int indent) {
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

void CProjectItem::save(SimpleFile *file, int indent) {
	file->writeNumberLine(6, indent);
	file->writeQuotedLine("Next Avail. Object Number", indent);
	file->writeNumberLine(_nextObjectNumber, indent);
	file->writeQuotedLine("Next Avail. Message Number", indent);
	file->writeNumberLine(_nextMessageNumber, indent);

	file->writeQuotedLine("", indent);
	_files.save(file, indent);

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
		// Intentional fall-through

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
		// Intentional fall-through

	case 5:
		file->readBuffer();
		_nextMessageNumber = file->readNumber();
		// Intentional fall-through

	case 4:
		file->readBuffer();
		// Intentional fall-through

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

void CProjectItem::setGameManager(CGameManager *gameManager) {
	if (!_gameManager)
		_gameManager = gameManager;
}

void CProjectItem::resetGameManager() {
	_gameManager = nullptr;
}

void CProjectItem::loadGame(int slotId) {
	CompressedFile file;

	// Clear any existing project contents and call preload code
	preLoad();
	clear();
	g_vm->_loadSaveSlot = -1;

	// Open either an existing savegame slot or the new game template
	if (slotId >= 0) {
		Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(
			g_vm->getSaveStateName(slotId));
		file.open(saveFile);
	} else {
		Common::File *newFile = new Common::File();
		if (!newFile->open("newgame.st"))
			error("Could not open newgame.st");
		file.open(newFile);
	}

	// Load the savegame header in
	TitanicSavegameHeader header;
	if (!readSavegameHeader(&file, header))
		error("Failed to read save game header");

	g_vm->_events->setTotalPlayTicks(header._totalFrames);

	// Load the contents in
	CProjectItem *newProject = loadData(&file);
	file.isClassStart();
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
	postLoad();
}

void CProjectItem::saveGame(int slotId, const CString &desc) {
	CompressedFile file;
	Common::OutSaveFile *saveFile = g_system->getSavefileManager()->openForSaving(
		g_vm->getSaveStateName(slotId), false);
	file.open(saveFile);

	// Signal the game is being saved
	preSave();

	// Write out the savegame header
	TitanicSavegameHeader header;
	header._saveName = desc;
	writeSavegameHeader(&file, header);

	// Save the contents out
	saveData(&file, this);

	// Save the game manager data
	_gameManager->save(&file);

	// Close the file and signal that the saving has finished
	file.close();
	postSave();
}

void CProjectItem::clear() {
	CTreeItem *item;
	while ((item = getFirstChild()) != nullptr)
		item->destroyAll();
}

CProjectItem *CProjectItem::loadData(SimpleFile *file) {
	if (!file->isClassStart())
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

		file->isClassStart();
	} while (file->isClassStart());

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

void CProjectItem::preLoad() {
	if (_gameManager)
		_gameManager->preLoad();

	CScreenManager *scrManager = CScreenManager::_currentScreenManagerPtr;
	if (scrManager)
		scrManager->preLoad();
}

void CProjectItem::postLoad() {
	CGameManager *gameManager = getGameManager();
	if (gameManager)
		gameManager->postLoad(this);

	CPetControl *petControl = getPetControl();
	if (petControl)
		petControl->postLoad();
}

void CProjectItem::preSave() {
	if (_gameManager)
		_gameManager->preSave(this);
}

void CProjectItem::postSave() {
	if (_gameManager)
		_gameManager->postSave();
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
	return dynamic_cast<CRoomItem *>(findChildInstance(CRoomItem::_type));
}

CTreeItem *CProjectItem::findChildInstance(ClassDef *classDef) const {
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
	return dynamic_cast<CRoomItem *>(findSiblingChildInstanceOf(CRoomItem::_type, priorRoom));
}

CTreeItem *CProjectItem::findSiblingChildInstanceOf(ClassDef *classDef, CTreeItem *startItem) const {
	for (CTreeItem *treeItem = startItem->getParent()->getNextSibling();
			treeItem; treeItem = treeItem->getNextSibling()) {
		for (CTreeItem *childItem = treeItem->getFirstChild();
				childItem; childItem = childItem->getNextSibling()) {
			if (childItem->isInstanceOf(classDef))
				return childItem;
		}
	}

	return nullptr;
}

CDontSaveFileItem *CProjectItem::getDontSaveFileItem() const {
	for (CTreeItem *treeItem = getFirstChild(); treeItem; treeItem = treeItem->getNextSibling()) {
		if (treeItem->isInstanceOf(CDontSaveFileItem::_type))
			return dynamic_cast<CDontSaveFileItem *>(treeItem);
	}

	return nullptr;
}

CRoomItem *CProjectItem::findHiddenRoom() {
	return dynamic_cast<CRoomItem *>(findByName("HiddenRoom"));
}

CViewItem *CProjectItem::findView(int roomNumber, int nodeNumber, int viewNumber) {
	CTreeItem *treeItem = getFirstChild();
	CRoomItem *roomItem = nullptr;

	// Scan for the specified room
	if (treeItem) {
		do {
			CTreeItem *childItem = treeItem->getFirstChild();
			CRoomItem *rItem = dynamic_cast<CRoomItem *>(childItem);
			if (rItem && rItem->_roomNumber == roomNumber) {
				roomItem = rItem;
				break;
			}
		} while ((treeItem = treeItem->getNextSibling()) != nullptr);
	}
	if (!roomItem)
		return nullptr;

	// Scan for the specified node within the room
	CNodeItem *nodeItem = nullptr;

	CNodeItem *nItem = dynamic_cast<CNodeItem *>(
		roomItem->findChildInstanceOf(CNodeItem::_type));
	for (; nItem && !nodeItem; nItem = dynamic_cast<CNodeItem *>(
			findNextInstanceOf(CNodeItem::_type, nItem))) {
		if (nItem->_nodeNumber == nodeNumber)
			nodeItem = nItem;
	}
	if (!nodeItem)
		return nullptr;

	// Scan for the specified view within the node
	CViewItem *viewItem = dynamic_cast<CViewItem *>(
		nodeItem->findChildInstanceOf(CViewItem::_type));
	for (; viewItem; viewItem = dynamic_cast<CViewItem *>(
			findNextInstanceOf(CViewItem::_type, viewItem))) {
		if (viewItem->_viewNumber == viewNumber)
			return viewItem;
	}

	return nullptr;
}

SaveStateList CProjectItem::getSavegameList(const Common::String &target) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0??", target.c_str());
	TitanicSavegameHeader header;

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort to get the files in numerical order

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot <= MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				SimpleFile f;
				f.open(in);
				if (readSavegameHeader(&f, header))
					saveList.push_back(SaveStateDescriptor(slot, header._saveName));

				delete in;
			}
		}
	}

	return saveList;
}

WARN_UNUSED_RESULT bool CProjectItem::readSavegameHeader(SimpleFile *file, TitanicSavegameHeader &header, bool skipThumbnail) {
	char saveIdentBuffer[SAVEGAME_STR_SIZE + 1];
	header._thumbnail = nullptr;
	header._totalFrames = 0;

	// Validate the header Id
	file->unsafeRead(saveIdentBuffer, SAVEGAME_STR_SIZE + 1);
	if (strncmp(saveIdentBuffer, SAVEGAME_STR, SAVEGAME_STR_SIZE)) {
		file->seek(-SAVEGAME_STR_SIZE, SEEK_CUR);
		header._saveName = "Unnamed";
		return true;
	}

	header._version = file->readByte();
	if (header._version < MINIMUM_SAVEGAME_VERSION || header._version > CURRENT_SAVEGAME_VERSION)
		return false;

	// Read in the string
	header._saveName.clear();
	char ch;
	while ((ch = (char)file->readByte()) != '\0') header._saveName += ch;

	// Get the thumbnail
	if (!Graphics::loadThumbnail(*file, header._thumbnail, skipThumbnail))
		return false;

	// Read in save date/time
	header._year = file->readUint16LE();
	header._month = file->readUint16LE();
	header._day = file->readUint16LE();
	header._hour = file->readUint16LE();
	header._minute = file->readUint16LE();
	header._totalFrames = file->readUint32LE();

	return true;
}

void CProjectItem::writeSavegameHeader(SimpleFile *file, TitanicSavegameHeader &header) {
	// Write out a savegame header
	file->write(SAVEGAME_STR, SAVEGAME_STR_SIZE + 1);

	file->writeByte(CURRENT_SAVEGAME_VERSION);

	// Write savegame name
	file->write(header._saveName.c_str(), header._saveName.size());
	file->writeByte('\0');

	// Create a thumbnail of the screen and save it out
	Graphics::Surface *thumb = createThumbnail();
	Graphics::saveThumbnail(*file, *thumb);
	thumb->free();
	delete thumb;

	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	file->writeUint16LE(td.tm_year + 1900);
	file->writeUint16LE(td.tm_mon + 1);
	file->writeUint16LE(td.tm_mday);
	file->writeUint16LE(td.tm_hour);
	file->writeUint16LE(td.tm_min);
	file->writeUint32LE(g_vm->_events->getTotalPlayTicks());
}

Graphics::Surface *CProjectItem::createThumbnail() {
	Graphics::Surface *thumb = new Graphics::Surface();

	::createThumbnailFromScreen(thumb);
	return thumb;
}

CViewItem *CProjectItem::parseView(const CString &viewString) {
	int firstIndex = viewString.indexOf('.');
	int lastIndex = viewString.lastIndexOf('.');
	CString roomName, nodeName, viewName;

	if (firstIndex == -1) {
		roomName = viewString;
	}
	else {
		roomName = viewString.left(firstIndex);

		if (lastIndex > firstIndex) {
			nodeName = viewString.mid(firstIndex + 1, lastIndex - firstIndex - 1);
			viewName = viewString.mid(lastIndex + 1);
		}
		else {
			nodeName = viewString.mid(firstIndex + 1);
		}
	}

	CGameManager *gameManager = getGameManager();
	if (!gameManager)
		return nullptr;

	CRoomItem *room = gameManager->getRoom();
	CProjectItem *project = room->getRoot();

	// Ensure we have the specified room
	if (project) {
		if (room->getName().compareToIgnoreCase(roomName)) {
			// Scan for the correct room
			for (room = project->findFirstRoom();
			room && room->getName().compareToIgnoreCase(roomName);
				room = project->findNextRoom(room));
		}
	}
	if (!room)
		return nullptr;

	// Find the designated node within the room
	CNodeItem *node = dynamic_cast<CNodeItem *>(room->findChildInstanceOf(CNodeItem::_type));
	while (node && node->getName().compareToIgnoreCase(nodeName))
		node = dynamic_cast<CNodeItem *>(room->findNextInstanceOf(CNodeItem::_type, node));
	if (!node)
		return nullptr;

	CViewItem *view = dynamic_cast<CViewItem *>(node->findChildInstanceOf(CViewItem::_type));
	while (view && view->getName().compareToIgnoreCase(viewName))
		view = dynamic_cast<CViewItem *>(node->findNextInstanceOf(CViewItem::_type, view));
	if (!view)
		return nullptr;

	// Find the view, so return it
	return view;
}

bool CProjectItem::changeView(const CString &viewName) {
	return changeView(viewName, "");
}

bool CProjectItem::changeView(const CString &viewName, const CString &clipName) {
	CViewItem *newView = parseView(viewName);
	CGameManager *gameManager = getGameManager();
	CViewItem *oldView = gameManager->getView();

	if (!oldView || !newView)
		return false;

	CMovieClip *clip = nullptr;
	if (!clipName.empty()) {
		clip = oldView->findNode()->findRoom()->findClip(clipName);
	} else {
		CLinkItem *link = oldView->findLink(newView);
		if (link)
			clip = link->getClip();
	}

	gameManager->_gameState.changeView(newView, clip);
	return true;
}


} // End of namespace Titanic
