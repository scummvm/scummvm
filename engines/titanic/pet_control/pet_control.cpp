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

#include "titanic/pet_control/pet_control.h"
#include "titanic/carry/carry.h"
#include "titanic/core/project_item.h"
#include "titanic/messages/messages.h"
#include "titanic/messages/pet_messages.h"
#include "titanic/game_manager.h"
#include "titanic/game_state.h"
#include "titanic/titanic.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPetControl, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MouseDragStartMsg)
	ON_MESSAGE(MouseDragMoveMsg)
	ON_MESSAGE(MouseDragEndMsg)
	ON_MESSAGE(MouseButtonUpMsg)
	ON_MESSAGE(MouseDoubleClickMsg)
	ON_MESSAGE(MouseWheelMsg)
	ON_MESSAGE(KeyCharMsg)
	ON_MESSAGE(VirtualKeyCharMsg)
	ON_MESSAGE(TimerMsg)
END_MESSAGE_MAP()

CPetControl::CPetControl() : CGameObject(),
		_currentArea(PET_CONVERSATION), _inputLockCount(0), _areaLockCount(0),
		_areaChangeType(-1), _activeNPC(nullptr), _remoteTarget(nullptr),
		_hiddenRoom(nullptr), _drawBounds(20, 350, 620, 480) {
	_sections[PET_INVENTORY] = &_inventory;
	_sections[PET_CONVERSATION] = &_conversations;
	_sections[PET_REMOTE] = &_remote;
	_sections[PET_ROOMS] = &_rooms;
	_sections[PET_REAL_LIFE] = &_realLife;
	_sections[PET_STARFIELD] = &_starfield;
	_sections[PET_TRANSLATION] = &_translation;
}

void CPetControl::save(SimpleFile *file, int indent) {
	// Ensure a remote target name is set if there is one
	if (_remoteTargetName.empty() && _remoteTarget)
		_remoteTargetName = _remoteTarget->getName();

	file->writeNumberLine(0, indent);
	file->writeNumberLine(_currentArea, indent);
	file->writeQuotedLine(_activeNPCName, indent);
	file->writeQuotedLine(_remoteTargetName, indent);

	saveAreas(file, indent);
	CGameObject::save(file, indent);
}

void CPetControl::load(SimpleFile *file) {
	int val = file->readNumber();
	isValid();

	if (!val) {
		_currentArea = (PetArea)file->readNumber();
		_activeNPCName = file->readString();
		_remoteTargetName = file->readString();

		loadAreas(file, 0);
	}

	CGameObject::load(file);
}

void CPetControl::reset() {
	_conversations.reset();
	_rooms.reset();
	_remote.reset();
	_inventory.reset();
	_starfield.reset();
	_realLife.reset();
	_translation.reset();
	_frame.reset();
}

bool CPetControl::isValid() {
	return _conversations.isValid(this) &&
		_rooms.isValid(this) &&
		_remote.isValid(this) &&
		_inventory.isValid(this) &&
		_starfield.isValid(this) &&
		_realLife.isValid(this) &&
		_translation.isValid(this) &&
		_frame.isValid(this);
}

void CPetControl::loadAreas(SimpleFile *file, int param) {
	_conversations.load(file, param);
	_rooms.load(file, param);
	_remote.load(file, param);
	_inventory.load(file, param);
	_starfield.load(file, param);
	_realLife.load(file, param);
	_translation.load(file, param);
	_frame.load(file, param);
}

void CPetControl::saveAreas(SimpleFile *file, int indent) {
	_conversations.save(file, indent);
	_rooms.save(file, indent);
	_remote.save(file, indent);
	_inventory.save(file, indent);
	_starfield.save(file, indent);
	_realLife.save(file, indent);
	_translation.save(file, indent);
	_frame.save(file, indent);
}

void CPetControl::draw(CScreenManager *screenManager) {
	CGameManager *gameManager = getGameManager();
	Rect bounds = _drawBounds;
	bounds.constrain(gameManager->_bounds);

	if (!bounds.isEmpty()) {
		if (_areaChangeType >= 0) {
			_inventory.changed(_areaChangeType);
			_areaChangeType = -1;
		}

		_frame.drawFrame(screenManager);

		// Draw the specific area that's currently active
		_sections[_currentArea]->draw(screenManager);
	}
}

Rect CPetControl::getBounds() const {
	return _sections[_currentArea]->getBounds();
}

void CPetControl::postLoad() {
	CProjectItem *root = getRoot();

	if (!_activeNPCName.empty() && root)
		_activeNPC = root->findByName(_activeNPCName);
	if (!_remoteTargetName.empty() && root)
		_remoteTarget = dynamic_cast<CGameObject *>(root->findByName(_remoteTargetName));

	setArea(_currentArea, true);
	loaded();
}

void CPetControl::loaded() {
	_conversations.postLoad();
	_rooms.postLoad();
	_remote.postLoad();
	_inventory.postLoad();
	_starfield.postLoad();
	_realLife.postLoad();
	_translation.postLoad();
	_frame.postLoad();
}

void CPetControl::enterNode(CNodeItem *node) {
	getGameManager()->_gameState.enterNode();
}

void CPetControl::enterRoom(CRoomItem *room) {
	_rooms.enterRoom(room);
	_remote.enterRoom(room);
	_inventory.enterRoom(room);
}

void CPetControl::resetRemoteTarget() {
	_remoteTarget = nullptr;
	_remoteTargetName.clear();
}

void CPetControl::setActiveNPC(CTrueTalkNPC *npc) {
	if (_activeNPC != npc) {
		_activeNPC = npc;

		if (_activeNPC) {
			_activeNPCName = npc->getName();
			_conversations.displayNPCName(npc);
		} else {
			_activeNPCName = "";
		}
	}
}

void CPetControl::setActiveNPC(const CString &name) {
	_conversations.setActiveNPC(name);
}

void CPetControl::refreshNPC() {
	_conversations.setNPC(_activeNPCName);
}

void CPetControl::resetActiveNPC() {
	_activeNPC = nullptr;
	_activeNPCName = "";
}

PetArea CPetControl::setArea(PetArea newArea, bool forceChange) {
	if ((!forceChange && newArea == _currentArea) || !isAreaUnlocked())
		return _currentArea;

	// Signal the currently active area that it's being left
	_sections[_currentArea]->leave();

	// Change the current area
	PetArea oldArea = _currentArea;
	_frame.setArea(newArea);
	_currentArea = newArea;

	// Signal to the new view that it's been activated
	_sections[_currentArea]->enter(oldArea);

	makeDirty();
	return newArea;
}

void CPetControl::hideCursor() {
	_sections[_currentArea]->hideCursor();
}

void CPetControl::showCursor() {
	_sections[_currentArea]->showCursor();
}

void CPetControl::highlightGlyph(int id) {
	_sections[_currentArea]->highlight(id);
}

void CPetControl::setRemoteTarget(CGameObject *item) {
	_remoteTarget = item;
	if (item)
		_remoteTargetName = item->getName();
	else
		_remoteTargetName.clear();
}

CRoomItem *CPetControl::getHiddenRoom() {
	if (!_hiddenRoom)
		_hiddenRoom = CGameObject::getHiddenRoom();

	return _hiddenRoom;
}

CGameObject *CPetControl::getHiddenObject(const CString &name) {
	CRoomItem *room = getHiddenRoom();
	return room ? dynamic_cast<CGameObject *>(findUnder(room, name)) : nullptr;
}

bool CPetControl::containsPt(const Common::Point &pt) const {
	return _drawBounds.contains(pt);
}

bool CPetControl::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (!containsPt(msg->_mousePos) || isInputLocked())
		return false;

	bool result = false;
	if (isAreaUnlocked())
		result = _frame.MouseButtonDownMsg(msg);

	if (!result) {
		result = _sections[_currentArea]->MouseButtonDownMsg(msg);
	}

	makeDirty();
	return result;
}

bool CPetControl::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (!containsPt(msg->_mousePos) || isInputLocked())
		return false;

	return _sections[_currentArea]->MouseDragStartMsg(msg);
}

bool CPetControl::MouseDragMoveMsg(CMouseDragMoveMsg *msg) {
	return _sections[_currentArea]->MouseDragMoveMsg(msg);
}

bool CPetControl::MouseDragEndMsg(CMouseDragEndMsg *msg) {
	return _sections[_currentArea]->MouseDragEndMsg(msg);
}

bool CPetControl::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	if (!containsPt(msg->_mousePos) || isInputLocked())
		return false;

	bool result = false;
	if (isAreaUnlocked())
		result = _frame.MouseButtonUpMsg(msg);

	if (!result)
		result = _sections[_currentArea]->MouseButtonUpMsg(msg);

	makeDirty();
	return result;
}

bool CPetControl::MouseDoubleClickMsg(CMouseDoubleClickMsg *msg) {
	if (!containsPt(msg->_mousePos) || isInputLocked())
		return false;

	return _sections[_currentArea]->MouseDoubleClickMsg(msg);
}

bool CPetControl::MouseWheelMsg(CMouseWheelMsg *msg) {
	if (!containsPt(msg->_mousePos) || isInputLocked())
		return false;

	return _sections[_currentArea]->MouseWheelMsg(msg);
}

bool CPetControl::KeyCharMsg(CKeyCharMsg *msg) {
	if (isInputLocked())
		return false;

	makeDirty();
	bool result = _sections[_currentArea]->KeyCharMsg(msg);

	if (!result) {
		switch (msg->_key) {
		case Common::KEYCODE_TAB:
			if (isAreaUnlocked()) {
				setArea(PET_INVENTORY);
				result = true;
			}
			break;
		default:
			break;
		}
	}

	return result;
}

bool CPetControl::VirtualKeyCharMsg(CVirtualKeyCharMsg *msg) {
	if (isInputLocked())
		return false;

	bool result = _sections[_currentArea]->VirtualKeyCharMsg(msg);

	if (!result) {
		switch (msg->_keyState.keycode) {
		case Common::KEYCODE_F1:
			result = true;
			setArea(PET_CONVERSATION);
			break;
		case Common::KEYCODE_F2:
			setArea(PET_INVENTORY);
			result = true;
			break;
		case Common::KEYCODE_F3:
			result = true;
			setArea(PET_REMOTE);
			break;
		case Common::KEYCODE_F4:
			result = true;
			setArea(PET_ROOMS);
			break;
		case Common::KEYCODE_F6:
			result = true;
			setArea(PET_REAL_LIFE);
			break;
		case Common::KEYCODE_F8:
			if (g_vm->isGerman()) {
				result = true;
				setArea(PET_TRANSLATION);
			}
			break;
		default:
			break;
		}
	}

	return result;
}

bool CPetControl::TimerMsg(CTimerMsg *msg) {
	if (_timers[msg->_actionVal]._target)
		_timers[msg->_actionVal]._target->timerExpired(msg->_actionVal);
	return true;
}

void CPetControl::drawSquares(CScreenManager *screenManager, int count) {
	_frame.drawSquares(screenManager, count);
}

CGameObject *CPetControl::dragEnd(const Point &pt) const {
	return _currentArea == PET_INVENTORY ? _inventory.dragEnd(pt) : nullptr;
}

bool CPetControl::checkDragEnd(CGameObject *item) const {
	return _sections[_currentArea]->checkDragEnd(item);
}

void CPetControl::displayMessage(StringId stringId, int param) const {
	CString msg = CString::format(g_vm->_strings[stringId].c_str(), param);
	_sections[_currentArea]->displayMessage(msg);
}

void CPetControl::displayMessage(const CString &str, int param) const {
	CString msg = CString::format(str.c_str(), param);
	_sections[_currentArea]->displayMessage(msg);
}

void CPetControl::addTranslation(StringId id1, StringId id2) {
	setArea(PET_TRANSLATION);
	_translation.addTranslation(g_vm->_strings[id1], g_vm->_strings[id2]);
}

void CPetControl::clearTranslation() {
	_translation.clearTranslation();
}

CGameObject *CPetControl::getFirstObject() const {
	return dynamic_cast<CGameObject *>(getFirstChild());
}

CGameObject *CPetControl::getNextObject(CGameObject *prior) const {
	if (!prior || prior->getParent() != this)
		return nullptr;

	return dynamic_cast<CGameObject *>(prior->getNextSibling());
}

void CPetControl::addToInventory(CGameObject *item) {
	item->detach();

	if (item->getName() == "CarryParcel") {
		CCarry *child = dynamic_cast<CCarry *>(getLastChild());
		if (child)
			child->detach();

		item->petMoveToHiddenRoom();
		if (!child)
			return;

		item = child;
	}

	item->addUnder(this);
	_inventory.itemsChanged();

	setArea(PET_INVENTORY);
	if (_currentArea == PET_INVENTORY)
		_inventory.highlightItem(item);

	makeDirty();
	CPETGainedObjectMsg msg;
	msg.execute(item);
}

void CPetControl::removeFromInventory(CGameObject *item, CTreeItem *newParent,
		bool refreshUI, bool sendMsg) {
	if (item && newParent) {
		item->detach();
		item->addUnder(newParent);

		if (refreshUI)
			_inventory.itemRemoved(item);
		if (sendMsg) {
			CPETLostObjectMsg lostMsg;
			lostMsg.execute(item);
		}
	}
}

void CPetControl::removeFromInventory(CGameObject *item, bool refreshUI, bool sendMsg) {
	CViewItem *view = getGameManager()->getView();
	removeFromInventory(item, view, refreshUI, sendMsg);
}

void CPetControl::invChange(CGameObject *item) {
	_inventory.change(item);
}

void CPetControl::moveToHiddenRoom(CTreeItem *item) {
	CRoomItem *room = getHiddenRoom();
	if (room) {
		item->detach();
		item->addUnder(room);
	}
}

bool CPetControl::checkNode(const CString &name) {
	CGameManager *gameManager = getGameManager();
	if (!gameManager)
		return true;
	if (name == "NULL")
		return false;

	CViewItem *view = gameManager->getView();
	if (!view)
		return true;

	CNodeItem *node = view->findNode();
	if (!node)
		return true;

	CString viewName = view->getName();
	CString nodeName = node->getName();
	CRoomItem *room = getGameManager()->getRoom();

	if (room) {
		CString roomName = room->getName();
		CString newNode;

		if (roomName == "1stClassRestaurant") {
		} else if (nodeName == "Lobby Node") {
			nodeName = "Node 1";
		} else if (nodeName == "Entrance Node") {
			nodeName = "Node 2";
		} else if (nodeName == "MaitreD Node") {
			nodeName = "Node 3";
		} else if (nodeName == "Scraliontis Table Standing Node") {
			nodeName = "Node 4";
		} else if (nodeName == "Pellerator Node") {
			nodeName = "Node 5";
		} else if (nodeName == "SUB Node") {
			nodeName = "Node 6";
		} else if (nodeName == "Phonograph Node") {
			nodeName = "Node 7";
		} else if (nodeName == "Scraliontis Table Seated Node") {
			nodeName = "Node 8";
		}

		if (roomName == "MusicRoom") {
			if (nodeName == "Musical Instruments")
				nodeName = "Node 1";
			if (nodeName == "Phonograph Node")
				nodeName = "Node 2";
		}
	}

	CString str = CString::format("%s.%s", nodeName.c_str(), viewName.c_str());
	str = str.right(5);
	str.toLowercase();

	CString nameLower = name;
	nameLower.toLowercase();

	return nameLower.contains(str);
}

void CPetControl::syncSoundSettings() {
	_realLife.syncSoundSettings();
}

void CPetControl::playSound(int soundNum) {
	CTreeItem *player = getHiddenObject("PETSoundPlayer");
	if (player) {
		CPETPlaySoundMsg playMsg(soundNum);
		playMsg.execute(player);
	}
}

int CPetControl::canSummonBot(const CString &name) {
	// If player is the very same view as the NPC, then it's already present
	if (isBotInView(name))
		return SUMMON_CAN;

	// Get the room
	CGameManager *gameManager = getGameManager();
	if (!gameManager)
		return SUMMON_CANT;
	CRoomItem *room = gameManager->getRoom();
	if (!room)
		return SUMMON_CANT;

	// WORKAROUND: Handle special disallowed locations
	if (isBotDisallowedLocation())
		return SUMMON_CANT;

	// Query current room to see whether the bot can be summoned to it
	CSummonBotQueryMsg queryMsg(name);
	return queryMsg.execute(room) ? SUMMON_CAN : SUMMON_CANT;
}

bool CPetControl::isBotInView(const CString &name) const {
	CGameManager *gameManager = getGameManager();
	if (!gameManager)
		return false;
	CViewItem *view = gameManager->getView();
	if (!view)
		return false;

	// Iterate to find NPC
	for (CTreeItem *child = view->getFirstChild(); child; child = child->scan(view)) {
		CGameObject *gameObject = dynamic_cast<CGameObject *>(child);
		if (gameObject) {
			if (!gameObject->getName().compareToIgnoreCase(name))
				return true;
		}
	}

	return false;
}

void CPetControl::summonBot(const CString &name, int val) {
	CGameManager *gameManager = getGameManager();
	if (gameManager) {
		CRoomItem *room = gameManager->getRoom();

		if (room) {
			CSummonBotMsg summonMsg(name, val);
			summonMsg.execute(room);
		}
	}
}

void CPetControl::onSummonBot(const CString &name, int val) {
	CGameObject *bot = findBot(name, getHiddenRoom());
	if (!bot) {
		bot = findBot(name, getRoot());
	}

	if (bot) {
		removeFromInventory(bot, false, false);

		COnSummonBotMsg summonMsg(val);
		summonMsg.execute(bot);
		makeDirty();
	}
}

bool CPetControl::dismissBot(const CString &name) {
	CGameManager *gameManager = getGameManager();
	if (!gameManager)
		return false;
	CViewItem *view = gameManager->getView();
	if (!view)
		return false;

	bool result = false;
	CDismissBotMsg dismissMsg;
	for (CTreeItem *treeItem = view->getFirstChild(); treeItem;
			treeItem = treeItem->scan(view)) {
		CGameObject *obj = dynamic_cast<CGameObject *>(treeItem);
		if (obj) {
			if (!obj->getName().compareToIgnoreCase(name))
				result = true;
			else
				dismissMsg.execute(treeItem);
		}
	}

	return result;
}

bool CPetControl::isDoorOrBellbotPresent() const {
	CGameManager *gameManager = getGameManager();
	if (!gameManager)
		return false;
	CViewItem *view = gameManager->getView();
	if (!view)
		return false;

	for (CTreeItem *treeItem = view->getFirstChild(); treeItem;
			treeItem = treeItem->scan(view)) {
		CString name = treeItem->getName();
		if (dynamic_cast<CGameObject *>(treeItem) &&
				(name.containsIgnoreCase("Doorbot") || name.containsIgnoreCase("BellBot")))
			return true;
	}

	return false;
}

void CPetControl::startPetTimer(uint timerIndex, uint firstDuration, uint duration, CPetSection *target) {
	stopPetTimer(timerIndex);
	_timers[timerIndex]._id = addTimer(timerIndex, firstDuration, duration);
	_timers[timerIndex]._target = target;
	setTimerPersisent(_timers[timerIndex]._id, false);
}

void CPetControl::stopPetTimer(uint timerIndex) {
	if (_timers[timerIndex]._target) {
		stopTimer(_timers[timerIndex]._id);
		_timers[timerIndex]._target = nullptr;
	}
}

void CPetControl::setTimerPersisent(int id, bool flag) {
	getGameManager()->setTimerPersisent(id, flag);
}

CGameObject *CPetControl::findBot(const CString &name, CTreeItem *root) {
	for (CTreeItem *item = root; item; item = item->scan(root)) {
		if (!item->getName().compareToIgnoreCase(name)) {
			CGameObject *obj = dynamic_cast<CGameObject *>(item);
			if (obj)
				return obj;
		}
	}

	return nullptr;
}

bool CPetControl::isSuccUBusActive() const {
	if (!_activeNPC)
		return false;

	CString name = getName();
	return name.containsIgnoreCase("Succubus") || name.containsIgnoreCase("Sub");
}

void CPetControl::convResetDials(int flag) {
	if (flag == 1)
		_conversations.resetDials(_activeNPCName);
}

void CPetControl::resetDials0() {
	_conversations.resetDials0();
}

PassengerClass CPetControl::getMailDestClass(const CRoomFlags &roomFlags) const {
	if (!roomFlags.isSuccUBusRoomFlags())
		return roomFlags.getPassengerClassNum();

	return roomFlags.getSuccUBusClass(roomFlags.getSuccUBusRoomName());
}

void CPetControl::starsSetButtons(int matchIndex, bool isMarkerClose) {
	_starfield.setButtons(matchIndex, isMarkerClose);
	if (_currentArea == PET_STARFIELD)
		_starfield.makePetDirty();
}

void CPetControl::starsSetReference() {
	_starfield.setHasReference(true);
}

} // End of namespace Titanic
