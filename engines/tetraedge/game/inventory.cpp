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

#include "common/textconsole.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/application.h"
#include "tetraedge/game/cellphone.h"
#include "tetraedge/game/character.h"
#include "tetraedge/game/game.h"
#include "tetraedge/game/inventory.h"
#include "tetraedge/game/inventory_object.h"
#include "tetraedge/game/inventory_objects_xml_parser.h"

#include "tetraedge/te/te_core.h"

namespace Tetraedge {

Inventory::Inventory() : _cellphone(nullptr), _selectedObject(nullptr), _currentPage(0) {
}

Inventory::~Inventory() {
	if (_cellphone) {
		_cellphone->unload();
		delete _cellphone;
	}
}

void Inventory::enter() {
	setVisible(true);
	
	if (g_engine->gameIsAmerzone()) {
		currentPage(_currentPage);
	} else {
		Game *game = g_engine->getGame();
		Character *character = game->scene()._character;
		character->stop();
		character->setAnimation(character->characterSettings()._idleAnimFileName, true);

		_gui.layoutChecked("textObject")->setVisible(false);

		if (!game->_firstInventory && !g_engine->gameIsAmerzone()) {
			_gui.buttonLayoutChecked("Aide")->setVisible(false);
		} else {
			game->_firstInventory = false;
		}
	}

	if (_selectedObject)
		selectedObject(_selectedObject);
}

void Inventory::leave() {
	setVisible(false);
	if (_selectedObject) {
		Game *game = g_engine->getGame();
		if (game->entered())
			game->luaScript().execute("OnSelectedObject", _selectedObject->name());
	}
}

void Inventory::load() {
	setName("_inventory");
	setSizeType(RELATIVE_TO_PARENT);
	setSize(TeVector3f32(1.0f, 1.0f, userSize().z()));
	_gui.load("Inventory/Inventory.lua");
	TeLayout *invlayout = _gui.layoutChecked("inventory");
	addChild(invlayout);

	if (g_engine->gameIsAmerzone()) {
		TeLayout *bglayout = _gui.layoutChecked("background");
		bglayout->setRatioMode(RATIO_MODE_NONE);

		TeButtonLayout *btn;
		btn = _gui.buttonLayoutChecked("previousPage");
		btn->onMouseClickValidated().add(this, &Inventory::onPreviousPage);
	
		btn = _gui.buttonLayoutChecked("nextPage");
		btn->onMouseClickValidated().add(this, &Inventory::onNextPage);
	} else {
		TeButtonLayout *btn;
		btn = _gui.buttonLayoutChecked("cellphone");
		btn->onMouseClickValidated().add(this, &Inventory::onVisibleCellphone);

		btn = _gui.buttonLayoutChecked("prendre");
		btn->setVisible(false);
		btn->onMouseClickValidated().add(this, &Inventory::onTakeObjectSelected);

		btn = _gui.buttonLayoutChecked("lire");
		btn->setEnable(false);
		btn->onMouseClickValidated().add(this, &Inventory::onZoomed);

		btn = _gui.buttonLayoutChecked("quitButton");
		btn->setVisible(true);
		btn->onMouseClickValidated().add(this, &Inventory::onQuitButton);

		btn = _gui.buttonLayoutChecked("quitBackground");
		btn->setVisible(true);
		btn->onMouseClickValidated().add(this, &Inventory::onQuitButton);

		btn = _gui.buttonLayoutChecked("mainMenuButton");
		btn->setVisible(true);
		btn->onMouseClickValidated().add(this, &Inventory::onMainMenuButton);

		loadCellphone();
	}
	_currentPage = 0;
	_selectedObject = nullptr;

	const Common::Path objectsPathPrefix("Inventory/Objects/Objects_");
	const Common::String &lang = g_engine->getCore()->language();
	Common::Path langXmlPath = objectsPathPrefix.append(lang).appendInPlace(".xml");
	if (!Common::File::exists(langXmlPath)) {
		langXmlPath = objectsPathPrefix.append(".xml");
		if (!Common::File::exists(langXmlPath)) {
			langXmlPath = objectsPathPrefix.append("en.xml");
			if (!Common::File::exists(langXmlPath)) {
				error("Inventory::load Couldn't find inventory objects xml.");
			}
		}
	}

	loadXMLFile(langXmlPath);

	TeLayout *layout = _gui.layoutChecked("selectionSprite");
	layout->setVisible(false);
	_invObjects.clear();

	setVisible(false);
}

void Inventory::loadXMLFile(const Common::Path &path) {
	Common::File xmlfile;
	xmlfile.open(path);
	int64 fileLen = xmlfile.size();
	char *buf = new char[fileLen + 1];
	buf[fileLen] = '\0';
	xmlfile.read(buf, fileLen);
	const Common::String xmlContents = Common::String::format("<?xml version=\"1.0\" encoding=\"UTF-8\"?><document>%s</document>", buf);
	delete [] buf;
	xmlfile.close();

	InventoryObjectsXmlParser parser;
	if (!parser.loadBuffer((const byte *)xmlContents.c_str(), xmlContents.size()))
		error("Couldn't load inventory xml.");
	if (!parser.parse())
		error("Couldn't parse inventory xml.");
	_objectData = parser._objects;
}

void Inventory::unload() {
	int pageNo = 0;
	while (true) {
		const Common::String pageStr = Common::String::format("page%d", pageNo);
		if (_gui.layout(pageStr)) {
			int slotNo = 0;
			while (true) {
				const Common::String slotStr = Common::String::format("page%dSlot%d", pageNo, slotNo);
				TeLayout *slotLayout = _gui.layout(slotStr);
				if (!slotLayout)
					break;

				// Take a copy of the list as we may be deleting some
				// and that removes them from the parent.
				Common::Array<Te3DObject2 *> children = slotLayout->childList();
				for (Te3DObject2 *child : children) {
					InventoryObject *invObj = dynamic_cast<InventoryObject *>(child);
					if (invObj)
						delete invObj;
				}
				slotNo++;
			}
			pageNo++;
		} else {
			break;
		}
	}
	_gui.unload();
}

void Inventory::loadCellphone() {
	_cellphone = new Cellphone();
	_cellphone->load();
}

//void loadFromBackup(TiXmlNode *node);
//void saveToBackup(TiXmlNode *node);

void Inventory::addObject(const Common::String &objId) {
	InventoryObject *newobj = new InventoryObject();
	newobj->load(objId);
	if (!addObject(newobj))
		delete newobj;
}

bool Inventory::addObject(InventoryObject *obj) {
	_invObjects.push_front(obj);
	obj->selectedSignal().add(this, &Inventory::onObjectSelected);
	if (_invObjects.size() > 1) {
		int pageNo = 0;
		while (true) {
			TeLayout *page = _gui.layout(Common::String::format("page%d", pageNo));
			int slotNo = 0;
			if (!page)
				break;
			while (true) {
				TeLayout *slot = _gui.layout(Common::String::format("page%dSlot%d", pageNo, slotNo));
				if (!slot)
					break;
				for (int c = 0; c < slot->childCount(); c++) {
					Te3DObject2 *child = slot->child(c);
					InventoryObject *iobj = dynamic_cast<InventoryObject *>(child);
					if (iobj) {
						slot->removeChild(child);
						c--;
					}
				}
				slotNo++;
			}
			pageNo++;
		}
	}

	int pageno = 0;
	uint totalSlots = 0;
	bool retval = false;
	const Common::String newObjName = obj->name();
	auto invObjIter = _invObjects.begin();
	bool finished = false;
	while (!finished) {
		TeLayout *page = _gui.layout(Common::String::format("page%d", pageno));
		retval = false;
		if (!page)
			break;
		int slotno = 0;
		while (true) {
			TeLayout *slot = _gui.layout(Common::String::format("page%dSlot%d", pageno, slotno));
			if (!slot)
				break;
			retval = true;

			if (totalSlots == _invObjects.size()) {
				finished = true;
				break;
			}

			TeTextLayout *newText = new TeTextLayout();
			newText->setSizeType(CoordinatesType::RELATIVE_TO_PARENT);
			newText->setPosition(TeVector3f32(1.0, 1.0, 0.0));
			newText->setSize(TeVector3f32(1.0, 1.0, 0.0));
			newText->setTextSizeType(1);
			newText->setTextSizeProportionalToWidth(200);
			newText->setText(_gui.value("textAttributs").toString() + objectName((*invObjIter)->name()));
			newText->setName((*invObjIter)->name());
			newText->setVisible(false);

			TeLayout *layout = _gui.layout("textObject");
			layout->addChild(newText);
			slot->addChild(*invObjIter);

			totalSlots++;
			slotno++;
			invObjIter++;
		}
		pageno++;
	}
	return retval;
}

bool Inventory::isDocument(const Common::String &objId) {
	if (!_objectData.contains(objId))
		return false;
	return _objectData.getVal(objId)._isDocument;
}

int Inventory::objectCount(const Common::String &objId) {
	for (const InventoryObject *obj : _invObjects) {
		if (obj->name() == objId)
			return 1;
	}
	return 0;
}

Common::String Inventory::objectDescription(const Common::String &objId) {
	// Note: The game XML files don't actually include descriptions.
	return "";
}

Common::String Inventory::objectName(const Common::String &objId) {
	if (!_objectData.contains(objId))
		return "";
	return _objectData.getVal(objId)._name;
}

void Inventory::currentPage(uint page) {
	TeLayout *pageLayout = _gui.layout(Common::String::format("page%d", page));
	if (pageLayout) {
		_currentPage = page;
		uint p = 0;
		while (true) {
			pageLayout = _gui.layout(Common::String::format("page%d", p));
			if (!pageLayout)
				break;
			pageLayout->setVisible(p == _currentPage);
			TeButtonLayout *diodeLayout = _gui.buttonLayoutChecked(Common::String::format("diode%d", p));
			diodeLayout->setEnable(p != _currentPage);
			p++;
		}
		if (_selectedObject)
			selectedObject(_selectedObject);
	}
}

bool Inventory::onPreviousPage() {
	currentPage(_currentPage - 1);
	return false;
}

bool Inventory::onNextPage() {
	currentPage(_currentPage + 1);
	return false;
}

bool Inventory::onMainMenuButton() {
	leave();
	Game *game = g_engine->getGame();
	game->leave(false);
	Application *app = g_engine->getApplication();
	app->mainMenu().enter();
	return true;
}

bool Inventory::onObjectSelected(InventoryObject &obj) {
	selectedObject(&obj);
	if (_selectedTimer.running()) {
		if (_selectedTimer.timeElapsed() < 300000)
			g_engine->getGame()->inventoryMenu().leave();
	} else {
		_selectedTimer.start();
	}
	return false;
}

bool Inventory::onQuitButton() {
	Game *game = g_engine->getGame();
	game->inventoryMenu().leave();
	return true;
}

bool Inventory::onTakeObjectSelected() {
	Game *game = g_engine->getGame();
	game->inventoryMenu().leave();
	return false;
}

bool Inventory::onVisibleCellphone() {
	_cellphone->enter();
	Game *game = g_engine->getGame();
	game->inventoryMenu().leave();
	leave();
	return false;
}

bool Inventory::onZoomed() {
	const Common::String &selected = selectedObject();
	if (!selected.empty()) {
		Game *game = g_engine->getGame();
		game->documentsBrowser().showDocument(selected, 0);
	}
	return false;
}

void Inventory::pauseAnims() {
	Game *game = g_engine->getGame();
	if (game->scene()._character) {

	}
	error("TODO: implement Inventory::pauseAnims");
}

void Inventory::unPauseAnims() {
	Game *game = g_engine->getGame();
	if (game->scene()._character) {

	}
	error("TODO: implement Inventory::unPauseAnims");
}

void Inventory::removeObject(const Common::String &name) {
	if (!name.size()) {
		warning("Reqeust to remove an object with no name?");
		return;
	}

	// Take a copy of the name to be sure as we will be deleting the object
	const Common::String objname = name;
	int pageNo = 0;
	while (true) {
		TeLayout *page = _gui.layout(Common::String::format("page%d", pageNo));
		if (!page)
			break;
		int slotNo = 0;
		while (true) {
			const Common::String slotStr = Common::String::format("page%dSlot%d", pageNo, slotNo);
			TeLayout *slotLayout = _gui.layout(slotStr);
			if (!slotLayout)
				break;

			for (Te3DObject2 *child : slotLayout->childList()) {
				InventoryObject *childObj = dynamic_cast<InventoryObject *>(child);
				if (childObj && childObj->name() == objname) {
					if (_selectedObject == childObj)
						selectedObject(nullptr);
					for (auto iter = _invObjects.begin(); iter != _invObjects.end(); iter++) {
						if ((*iter)->name() == objname) {
							_invObjects.erase(iter);
							break;
						}
					}
					slotLayout->removeChild(child);
					delete childObj;
					updateLayout();
					return;
				}
			}
			slotNo++;
		}
		pageNo++;
	}
}

void Inventory::removeSelectedObject() {
	if (_selectedObject) {
		removeObject(_selectedObject->name());
		selectedObject(nullptr);
	}
}

InventoryObject *Inventory::selectedInventoryObject() {
	return _selectedObject;
}

void Inventory::selectedObject(const Common::String &objname) {
	int pageNo = 0;
	while (true) {
		TeLayout *page = _gui.layout(Common::String::format("page%d", pageNo));
		if (!page)
			break;
		int slotNo = 0;
		while (true) {
			const Common::String slotStr = Common::String::format("page%dSlot%d", pageNo, slotNo);
			TeLayout *slotLayout = _gui.layout(slotStr);
			if (!slotLayout)
				break;

			for (Te3DObject2 *child : slotLayout->childList()) {
				InventoryObject *invObj = dynamic_cast<InventoryObject *>(child);
				if (invObj && invObj->name() == objname) {
					selectedObject(invObj);
					// NOTE: Original then iterates _invObjects here..
					// why double iterate like that?
					return;
				}
			}
			slotNo++;
		}
		pageNo++;
	}
}

void Inventory::selectedObject(InventoryObject *obj) {
	Game *game = g_engine->getGame();
	game->setCurrentObjectSprite("");
	_gui.layoutChecked("prendre")->setVisible(false);
	_gui.layoutChecked("textObject")->setVisible(false);
	_selectedObject = obj;
	if (!obj) {
		_gui.spriteLayoutChecked("selectionSprite")->setVisible(false);
		_gui.textLayout("text")->setText("");
		game->inGameGui().spriteLayoutChecked("selectedObject")->unload();
	} else {
		TeSpriteLayout *selection = _gui.spriteLayoutChecked("selectionSprite");
		selection->setVisible(obj->worldVisible());
		TeLayout *parentLayout = dynamic_cast<TeLayout *>(obj->parent());
		if (!parentLayout)
			error("Couldn't get parent of object");
		TeVector3f32 pos = parentLayout->position();
		pos.z() = selection->position().z();
		selection->setPosition(pos);
		const Common::String &objId = obj->name();
		static const char *textStyle = "<section style=\"center\" /><color r=\"200\" g=\"200\" b=\"200\"/><font file=\"Common/Fonts/Colaborate-Regular.otf\" size=\"24\" />";
		Common::String text = Common::String::format("%s%s<br/>%s", textStyle,
					objectName(objId).c_str(),
					objectDescription(objId).c_str());
		_gui.textLayout("text")->setText(text);
		_gui.buttonLayoutChecked("lire")->setEnable(isDocument(objId));
		const Common::String spritePathStr = obj->spritePath();
		game->setCurrentObjectSprite(spritePathStr);
		TeLayout *textObj = _gui.layout("textObject");
		for (int i = 0; i < textObj->childCount(); i++) {
			if (textObj->child(i)->name() == obj->name()) {
				textObj->setVisible(true);
				textObj->child(i)->setVisible(true);
			} else {
				textObj->child(i)->setVisible(false);
			}
		}
		game->inGameGui().spriteLayoutChecked("selectedObject")->load(spritePathStr);
	}
}

const Common::String &Inventory::selectedObject() {
	if (_selectedObject == nullptr)
		return _blankStr;
	else
		return _selectedObject->name();
}

bool Inventory::updateLayout() {
	int pageNo = 0;
	while (true) {
		TeLayout *page = _gui.layout(Common::String::format("page%d", pageNo));
		if (!page)
			break;
		int slotNo = 0;
		while (true) {
			const Common::String slotStr = Common::String::format("page%dSlot%d", pageNo, slotNo);
			TeLayout *slotLayout = _gui.layout(slotStr);
			if (!slotLayout)
				break;

			// Take a copy of the list as we are deleting some
			// and that removes them from the parent's list.
			Common::Array<Te3DObject2 *> children = slotLayout->childList();
			for (Te3DObject2 *child : children) {
				InventoryObject *invObj = dynamic_cast<InventoryObject *>(child);
				if (invObj)
					slotLayout->removeChild(child);
			}
			slotNo++;
		}
		pageNo++;
	}

	// If list is empty, we're done.
	if (_invObjects.size() == 0)
		return true;

	pageNo = 0;
	Common::List<InventoryObject *>::iterator invObjIter = _invObjects.begin();
	while (true) {
		TeLayout *page = _gui.layout(Common::String::format("page%d", pageNo));
		if (!page)
			break;
		int slotNo = 0;
		while (true) {
			const Common::String slotStr = Common::String::format("page%dSlot%d", pageNo, slotNo);
			TeLayout *slotLayout = _gui.layout(slotStr);
			if (!slotLayout)
				break;
			slotLayout->addChild(*invObjIter);
			invObjIter++;
			slotNo++;
			if (invObjIter == _invObjects.end())
				return true;
		}
		pageNo++;
	}
	return false;
}

//#define TETRAEDGE_DEBUG_SAVELOAD

Common::Error Inventory::syncState(Common::Serializer &s) {
	uint nitems = _invObjects.size();
	s.syncAsUint32LE(nitems);
	return syncStateWithCount(s, nitems);
}

Common::Error Inventory::syncStateWithCount(Common::Serializer &s, uint nitems) {
	if (nitems > 1000)
		error("Unexpected number of elems syncing inventory");

	if (s.isLoading()) {
		_invObjects.clear();
		_selectedObject = nullptr;
		// Clear the layout if needed
		if (_gui.loaded())
			updateLayout();
#ifdef TETRAEDGE_DEBUG_SAVELOAD
		debug("Inventory::syncState: --- Loading %d inventory items: ---", nitems);
#endif
		for (uint i = 0; i < nitems; i++) {
			Common::String objname;
			s.syncString(objname);
			addObject(objname);
#ifdef TETRAEDGE_DEBUG_SAVELOAD
			debug("Inventory::syncState: 	%s", objname.c_str());
#endif
		}
	} else if (nitems) {
#ifdef TETRAEDGE_DEBUG_SAVELOAD
		debug("Inventory::syncState: --- Saving %d inventory items: --- ", _invObjects.size());
#endif
		// Add items in reverse order as the "addObject" on load will
		// add to front of list.InventoryObject
		auto iter = _invObjects.end();
		while (iter != _invObjects.begin()) {
			iter--;
			Common::String objname = (*iter)->name();
			s.syncString(objname);
#ifdef TETRAEDGE_DEBUG_SAVELOAD
			debug("Inventory::syncState: 	%s", objname.c_str());
#endif
		}
	}

#ifdef TETRAEDGE_DEBUG_SAVELOAD
	debug("Inventory::syncState: -------- end --------");
#endif
	return Common::kNoError;
}


} // end namespace Tetraedge
