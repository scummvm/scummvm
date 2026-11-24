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

#include "ngi/ngi.h"

#include "ngi/objects.h"
#include "ngi/messages.h"
#include "ngi/modal.h"
#include "ngi/statics.h"
#include "ngi/gameloader.h"

namespace NGI {

Command::Command() {
	_field_3C = 1;
	_messageNum = 0;
	_excFlags = 0;
	_parId = 0;
}

Command::Command(Command *src) : Message(*src) {
	_messageName = src->_messageName;
	_mirror = src->_mirror;
	_field_3C = 1;
	_messageNum = src->_messageNum;
	_excFlags = src->_excFlags;
	_parId = src->_parId;
}

Command *Command::createClone() {
	return new Command(this);
}

Command::Command(int16 parentId, int messageKind, int messageNum, int x, int y, int a7, int a8, int sceneClickX, int sceneClickY, int a11) :
	Message(parentId, messageKind, x, y, a7, a8, sceneClickX, sceneClickY, a11) {
	_field_3C = 1;
	_messageNum = messageNum;
	_excFlags = 0;
	_parId = 0;
}

struct exDesc {
	byte num;
	const char *name;
} static const exTypes[] = {
	{ 1,  "START_MOVEMENT" },
	{ 5,  "SHOW" },
	{ 17, "MESSAGE" },
	{ 63, "USER" },
	{ 0,  "" }
};

static const char *exCommandType2str(int type) {
	static char buf[10];

	for (int i = 0; exTypes[i].num; i++)
		if (exTypes[i].num == type)
			return exTypes[i].name;

	snprintf(buf, 10, "%d", type);

	return buf;
}

bool Command::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "Command::load()");

	_parentId = file.readUint16LE();
	_cmd = file.readUint32LE();
	_x = file.readSint32LE();
	_y = file.readSint32LE();
	_z = file.readUint32LE();
	_sceneClickX = file.readUint32LE();
	_sceneClickY = file.readUint32LE();
	_invId = file.readUint32LE();
	_wait = file.readUint32LE();
	_param = file.readUint32LE();
	_auxParam = file.readUint32LE();
	_oneShot = file.readUint32LE();
	_armed = file.readUint32LE();

	_messageNum = file.readUint32LE();

	_field_3C = 0;

	if (g_nmi->_gameProjectVersion >= 12) {
		_excFlags = file.readUint32LE();
		_parId = file.readUint32LE();
	}

	_objtype = kObjTypeExCommand;

	debugC(6, kDebugXML, "%% <COMMAND parent=%d cmd=%s x=%d y=%d f14=%d sceneX=%d sceneY=%d f20=%d f24=%d param=%d f2c=%d f30=%d f34=%d num=%d flags=%d parId=%d />",
			_parentId, exCommandType2str(_cmd), _x, _y, _z, _sceneClickX, _sceneClickY, _invId, _wait, _param, _auxParam,
			_oneShot, _armed, _messageNum, _excFlags, _parId);

	return true;
}

bool Command::handleMessage() {
	int cnt = 0;
	for (MessageHandler *m = g_nmi->_messageHandlers; m; m = m->nextItem)
		cnt += m->callback(this);

	if (_cmd == 17 || (_excFlags & 1)) {
		if (_parId) {
			MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(_parId);
			if (mq)
				mq->update();
		}
	}

	if (_excFlags & 2)
		delete this;

	return (cnt > 0);
}

void Command::sendMessage() {
	g_nmi->_exCommandList.push_back(this);

	processMessages();
}

void Command::postMessage() {
	g_nmi->_exCommandList.push_back(this);
}

void Command::handle() {
	if (g_nmi->_modalObject) {
		g_nmi->_modalObject->handleMessage(this);

		delete this;
	} else {
		postMessage();
	}
}

void Command::setf3c(int val) {
	if (val != -1)
		_field_3C = val;

	_armed = 1;
}

void Command::firef34() {
	if (_armed) {
		if (_field_3C >= _param) {
			_armed = 0;

			sendMessage();

			if (!_oneShot )
				setf3c(_auxParam);
		}
	}
}

ExCommand2::ExCommand2(int messageKind, int parentId, const PointList &points) : Command(parentId, messageKind, 0, 0, 0, 0, 1, 0, 0, 0) {
	_objtype = kObjTypeExCommand2;
	_points = points;
}

ExCommand2::ExCommand2(ExCommand2 *src) : Command(src), _points(src->_points) {}

ExCommand2 *ExCommand2::createClone() {
	return new ExCommand2(this);
}

Message::Message() {
	_cmd = 0;
	_parentId = 0;

	_x = 0;
	_y = 0;
	_z = 0;
	_sceneClickX = 0;
	_sceneClickY = 0;
	_invId = 0;
	_wait = 0;
	_param = 0;
	_auxParam = 0;
	_oneShot = 0;
	_armed = 0;
}

Message::Message(int16 parentId, int messageKind, int x, int y, int z, int a7, int sceneClickX, int sceneClickY, int a10) {
	_cmd = messageKind;
	_parentId = parentId;
	_x = x;
	_y = y;
	_z = z;
	_sceneClickX = sceneClickX;
	_sceneClickY = sceneClickY;
	_wait = a7;
	_invId = a10;
	_param = 0;
	_auxParam = 0;
	_oneShot = 0;
	_armed = 0;
}

ObjstateCommand::ObjstateCommand() {
	_value = 0;
	_objtype = kObjTypeObjstateCommand;
}

ObjstateCommand::ObjstateCommand(ObjstateCommand *src) : Command(src) {
	_value = src->_value;
	_objtype = kObjTypeObjstateCommand;

	_objCommandName = src->_objCommandName;
}

bool ObjstateCommand::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "ObjStateCommand::load()");

	_objtype = kObjTypeObjstateCommand;

	Command::load(file);

	_value = file.readUint32LE();

	_objCommandName = file.readPascalString();

	debugC(6, kDebugXML, "%% <COMMAND cmd=\"USER\" type=\"SET_LOGIC\" title=\"%s\" state=\"%d\" />", transCyrillic(_objCommandName.c_str()), _value);

	return true;
}

ObjstateCommand *ObjstateCommand::createClone() {
	return new ObjstateCommand(this);
}

MessageQueue::MessageQueue() {
	_field_14 = nullptr;
	_parId = 0;
	_queueId = 0;
	_id = 0;
	_isFinished = 0;
	_flags = 0;
	_counter = 0;
	_field_38 = 0;
	_flag1 = 0;
}

MessageQueue::MessageQueue(int dataId) {
	_field_14 = nullptr;
	_parId = 0;
	_queueId = dataId;
	_id = g_nmi->_globalMessageQueueList->compact();
	_isFinished = 0;
	_flags = 0;
	_counter = 0;
	_field_38 = 0;
	_flag1 = 0;
}

MessageQueue::MessageQueue(MessageQueue *src, int parId, int field_38) {
	_counter = 0;
	_field_38 = (field_38 == 0);

	for (Common::List<Command *>::iterator it = src->_commands.begin(); it != src->_commands.end(); ++it) {
		Command *ex = (*it)->createClone();
		ex->_excFlags |= 2;

		_commands.push_back(ex);
	}
	_field_14 = src->_field_14;

	if (parId)
		_parId = parId;
	else
		_parId = src->_parId;

	_id = g_nmi->_globalMessageQueueList->compact();
	_queueId = src->_queueId;
	_flags = src->_flags & ~kInGlobalQueue;
	_title = "";

	g_nmi->_globalMessageQueueList->addMessageQueue(this);

	_isFinished = 0;
	_flag1 = 0;
}

MessageQueue::~MessageQueue() {
	for (Common::List<Command *>::iterator it = _commands.begin(); it != _commands.end(); ++it) {
		Command *ex = *it;

		if (ex && ex->_excFlags & 2)
			delete ex;
	}

	if (_field_14)
		delete _field_14;

	if (_flags & kInGlobalQueue) {
		g_nmi->_globalMessageQueueList->removeQueueById(_id);
	}

	finish();
}

bool MessageQueue::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "MessageQueue::load()");

	_queueId = file.readUint16LE();

	int count = file.readUint16LE();

	assert(g_nmi->_gameProjectVersion >= 12);

	_title = file.readPascalString();

	for (int i = 0; i < count; i++) {
		Command *tmp = file.readClass<Command>();
		tmp->_excFlags |= 2;
		_commands.push_back(tmp);
	}

	_id = -1;
	_field_14 = nullptr;
	_parId = 0;
	_isFinished = 0;

	return true;
}

bool MessageQueue::run(AniObject *ani) {
	if (checkGlobalExCommandList1() && checkGlobalExCommandList2()) {
		if (!(getFlags() & kInGlobalQueue)) {
			g_nmi->_globalMessageQueueList->addMessageQueue(this);
		}
		if (ani) {
			ani->setQueue(this);
		} else {
			next();
		}
		return true;
	}
	return false;
}

void MessageQueue::update() {
	if (_counter > 0)
		_counter--;

	if (getCount()) {
		next();
	} else if (_counter == 0) {
		_isFinished = 1;
		finish();
	}
}

void MessageQueue::messageQueueCallback1(int par) {
	if (g_nmi->_isSaveAllowed && par == 16) {
		if (g_nmi->_globalMessageQueueList->size() && (*g_nmi->_globalMessageQueueList)[0] != 0) {
			for (uint i = 0; i < g_nmi->_globalMessageQueueList->size(); i++) {
				if ((*g_nmi->_globalMessageQueueList)[i]->_flags & 1)
					if ((*g_nmi->_globalMessageQueueList)[i] != this && !(*g_nmi->_globalMessageQueueList)[i]->_isFinished)
						return;
			}
		}

		if (g_nmi->_currentScene)
			g_nmi->_gameLoader->writeSavegame(g_nmi->_currentScene, "savetmp.sav", "");
	}
}

void MessageQueue::addCommandToFront(Command *ex) {
	_commands.push_front(ex);
}

void MessageQueue::addCommandToEnd(Command *ex) {
	_commands.push_back(ex);
}

void MessageQueue::addCommand(int pos, Command *ex) {
	Common::List<Command *>::iterator it = _commands.begin();

	for (int i = pos; i > 0; i--)
		++it;

	_commands.insert(it, ex);
}

Command *MessageQueue::getCommandByIndex(uint idx) {
	if (idx >= getCount())
		return nullptr;

	Common::List<Command *>::iterator it = _commands.begin();

	while (idx) {
		++it;
		idx--;
	}

	return *it;
}

void MessageQueue::removeCommand(uint idx, bool doFree) {
	if (idx >= getCount())
		return;

	Common::List<Command *>::iterator it = _commands.begin();

	while (idx) {
		++it;
		idx--;
	}

	if (doFree)
		delete *it;

	_commands.erase(it);
}

void MessageQueue::mergeQueue(MessageQueue *mq) { // Original belongs to AniHandler
	while (mq->_commands.size()) {
		_commands.push_back(mq->_commands.front());
		mq->_commands.pop_front();
	}
}

void MessageQueue::next() {
	if (getCount()) {
		if (!(_flags & 4) && (_flags & 1)) {
			messageQueueCallback1(16);
		}
		Command *ex = _commands.front();

		_commands.pop_front();

		_counter++;
		ex->_parId = _id;
		ex->_excFlags |= (ex->_wait == 0 ? 1 : 0) | (ex->_field_3C != 0 ? 2 : 0);

		_flags |= 4;
		if (ex->_cmd == Command::kCmdPause && ex->_wait && g_nmi->getPlatform() == Common::kPlatformAndroid)
			_pauseCommand.reset(ex);
		else
			ex->sendMessage();
	} else if (_counter <= 0) {
		_isFinished = 1;
		finish();
	}
}

bool MessageQueue::updatePause(uint32 elapsed) {
	if (!_pauseCommand)
		return false;

	if (elapsed < (uint32)_pauseCommand->_messageNum) {
		_pauseCommand->_messageNum -= elapsed;
		return false;
	}
	_pauseCommand->_excFlags |= 1;
	_pauseCommand.release()->postMessage();
	return true;
}

bool MessageQueue::checkGlobalExCommandList1() {
	Command *ex, *ex1;

	for (uint i = 0; i < getCount(); i++) {
		ex = getCommandByIndex(i);

		if (ex->_cmd != 1 && ex->_cmd != 20 && ex->_cmd != 5 && ex->_cmd != 27)
			continue;

		for (Common::List<Command *>::iterator it = g_nmi->_exCommandList.begin(); it != g_nmi->_exCommandList.end(); it++) {
			ex1 = *it;

			if (ex1->_cmd != 1 && ex1->_cmd != 20 && ex1->_cmd != 5 && ex1->_cmd != 27)
				continue;

			if (ex1->_parentId != ex->_parentId)
				continue;

			if (ex1->_param != ex->_param && ex1->_param != -1 && ex->_param != -1)
				continue;

			MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(ex1->_parId);

			if (mq) {
				if (mq->getFlags() & 1)
					return false;
			}
		}
	}
	return true;
}

bool MessageQueue::checkGlobalExCommandList2() {
	Command *ex, *ex1;

	for (uint i = 0; i < getCount(); i++) {
		ex = getCommandByIndex(i);

		if (ex->_cmd != 1 && ex->_cmd != 20 && ex->_cmd != 5 && ex->_cmd != 27)
			continue;

		for (Common::List<Command *>::iterator it = g_nmi->_exCommandList.begin(); it != g_nmi->_exCommandList.end();) {
			ex1 = *it;

			if (ex1->_cmd != 1 && ex1->_cmd != 20 && ex1->_cmd != 5 && ex1->_cmd != 27) {
				it++;
				continue;
			}

			if (ex1->_parentId != ex->_parentId) {
				it++;
				continue;
			}

			if (ex1->_param != ex->_param && ex1->_param != -1 && ex->_param != -1) {
				it++;
				continue;
			}

			MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(ex1->_parId);

			if (mq) {
				if (mq->getFlags() & 1)
					return false;

				delete mq;
			}

			it = g_nmi->_exCommandList.erase(it);

			if (ex1->_excFlags & 2) {
				delete ex1;
			}
		}
	}
	return true;
}

void MessageQueue::finish() {
	if (!_parId)
		return;

	MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(_parId);

	_parId = 0;

	if (!mq)
		return;

	if (!_flag1) {
		mq->update();
		return;
	}

	mq->_counter--;

	if (!mq->_counter && !mq->_commands.size())
		mq->update();
}

void MessageQueue::setObjectCopy(int key1, int key2) {
	for (uint i = 0; i < getCount(); i++) {
		Command *ex = getCommandByIndex(i);
		int k = ex->_cmd;
		if ((k == 1 || k == 20 || k == 5 || k == 6 || k == 2 || k == 18 || k == 19 || k == 22 || k == 55)
					&& ex->_param == key1)
			ex->_param = key2;
	}
}

int MessageQueue::calcDuration(AniObject *obj) {
	int res = 0;
	Command *ex;
	Movement *mov;

	for (uint i = 0; i < getCount(); i++) {
		ex = getCommandByIndex(i);
		if (ex->_parentId == obj->_id) {
			if (ex->_cmd == 1 || ex->_cmd == 20) {
				if ((mov = obj->getMovement(ex->_messageNum)) != nullptr) {
					if (ex->_z >= 1)
						res += ex->_z;
					else
						res += mov->calcDuration();
				}
			}
		}
	}

	return res;
}

void MessageQueue::setObjectCopyForId(int objId, int oldCopyId, int newCopyId) {
	for (uint i = 0; i < getCount(); i++) {
		Command *ex = getCommandByIndex(i);
		int k = ex->_cmd;

		if ((k == 1 || k == 20 || k == 5 || k == 6 || k == 2 || k == 18 || k == 19 || k == 22 || k == 55)
			 && ex->_param == oldCopyId
			 && ex->_parentId == objId)
			ex->_param = newCopyId;
	}
}

int MessageQueue::activateExCommandsByKind(int kind) {
	int res = 0;

	for (uint i = 0; i < getCount(); i++) {
		Command *ex = getCommandByIndex(i);

		if (ex->_cmd == kind) {
			ex->_cmd = 0;
			ex->_excFlags |= 1;

			res++;
		}
	}

	return res;
}

MessageQueue *GlobalMessageQueueList::getMessageQueueById(int id) {
	for (Common::Array<MessageQueue *>::iterator s = begin(); s != end(); ++s) {
		if ((*s)->_id == id)
			return *s;
	}

	return nullptr;
}

void GlobalMessageQueueList::deleteQueueById(int id) {
	for (uint i = 0; i < size(); i++)
		if (_storage[i]->_id == id) {
			delete remove_at(i);
			disableQueueById(id);
			return;
		}
}

void GlobalMessageQueueList::removeQueueById(int id) {
	for (uint i = 0; i < size(); i++)
		if (_storage[i]->_id == id) {
			_storage[i]->_flags &= ~kInGlobalQueue;
			remove_at(i);

			disableQueueById(id);
			return;
		}
}

void GlobalMessageQueueList::disableQueueById(int id) {
	for (Common::Array<MessageQueue *>::iterator s = begin(); s != end(); ++s) {
		if ((*s)->_parId == id)
			(*s)->_parId = 0;
	}
}

int GlobalMessageQueueList::compact() {
	Common::Array<bool> useList(size() + 2);

	for (uint i = 0; i < size();) {
		if (_storage[i]->_isFinished) {
			disableQueueById(_storage[i]->_id);
			delete remove_at(i);
		} else {
			if ((uint)_storage[i]->_id < size() + 2)
				useList[_storage[i]->_id] = true;
			i++;
		}
	}

	uint i;

	for (i = 1; i < size() + 2; i++) {
		if (!useList[i])
			break;
	}

	return i;
}

void GlobalMessageQueueList::addMessageQueue(MessageQueue *msg) {
	if ((msg->getFlags() & kInGlobalQueue) == 0) {
		msg->setFlags(msg->getFlags() | kInGlobalQueue);
		push_back(msg);
	} else {
		warning("Trying to add a MessageQueue already in the queue");
	}
}

void GlobalMessageQueueList::clear() {
	for (iterator it = begin(); it != end(); ++it) {
		// The MessageQueue destructor will try to remove itself from the global
		// queue if it thinks it is in the global queue, which will break the
		// iteration over the list
		(*it)->_flags &= ~kInGlobalQueue;
		(*it)->_parId = 0;
	}
	Common::for_each(begin(), end(), Common::DefaultDeleter<MessageQueue>());
	Common::Array<MessageQueue *>::clear();
}

void clearGlobalMessageQueueList() {
	g_nmi->_globalMessageQueueList->clear();
}

void clearGlobalMessageQueueList1() {
	clearMessages();

	g_nmi->_globalMessageQueueList->clear();
}

void clearMessages() {
	while (g_nmi->_exCommandList.size()) {
		Command *ex = g_nmi->_exCommandList.front();

		g_nmi->_exCommandList.pop_front();

		if (ex->_excFlags & 2)
			delete ex;
	}
}

bool removeMessageHandler(int16 id, int pos) {
	if (g_nmi->_messageHandlers) {
		MessageHandler *curItem = g_nmi->_messageHandlers;
		MessageHandler *prevItem = nullptr;
		int curPos = 0;

		while (id != curItem->id) {
			prevItem = curItem;
			curItem = curItem->nextItem;
			curPos++;

			if (!curItem)
				return false;
		}

		if (pos == -1 || curPos == pos) {
			prevItem->nextItem = curItem->nextItem;
			delete curItem;
			updateMessageHandlerIndex(prevItem->nextItem, -1);

			return true;
		}
	}

	return false;
}

void updateMessageHandlerIndex(MessageHandler *msg, int offset) {
	for (; msg; msg = msg->nextItem)
		msg->index += offset;
}

void addMessageHandler(int (*callback)(Command *), int16 id) {
	if (getMessageHandlerById(id))
		return;

	MessageHandler *curItem = g_nmi->_messageHandlers;

	if (!curItem)
		return;

	int index = 0;
	for (MessageHandler *i = g_nmi->_messageHandlers->nextItem; i; i = i->nextItem) {
		curItem = i;
		index++;
	}

	allocMessageHandler(curItem, id, callback, index);

	if (curItem)
		updateMessageHandlerIndex(curItem->nextItem->nextItem, 1);
}

MessageHandler *getMessageHandlerById(int16 id) {
	MessageHandler *curItem = g_nmi->_messageHandlers;

	if (!curItem)
		return nullptr;

	while (id != curItem->id) {
		curItem = curItem->nextItem;

		if (!curItem)
			return nullptr;
	}

	return curItem;
}

bool allocMessageHandler(MessageHandler *where, int16 id, int (*callback)(Command *), int index) {
	MessageHandler *msg = new MessageHandler;

	if (where) {
		msg->nextItem = where->nextItem;
		where->nextItem = msg;
		msg->id = id;
		msg->callback = callback;
		msg->index = index;
	} else {
		msg->nextItem = nullptr;
		msg->id = id;
		msg->callback = callback;
		msg->index = 0;

		g_nmi->_messageHandlers = msg;
	}

	return true;
}

int getMessageHandlersCount() {
	int result;
	MessageHandler *curItem = g_nmi->_messageHandlers;

	for (result = 0; curItem; result++)
		curItem = curItem->nextItem;

	return result;
}

bool addMessageHandlerByIndex(int (*callback)(Command *), int index, int16 id) {
	if (getMessageHandlerById(id))
		return false;

	if (index) {
		MessageHandler *curItem = g_nmi->_messageHandlers;

		for (int i = index - 1; i > 0; i--)
			if (curItem)
				curItem = curItem->nextItem;

		if (!curItem)
			return false;

		bool res = allocMessageHandler(curItem, id, callback, index);

		if (res)
			updateMessageHandlerIndex(curItem->nextItem->nextItem, 1);

		return res;
	} else {
		MessageHandler *newItem = new MessageHandler;

		newItem->nextItem = g_nmi->_messageHandlers;
		newItem->id = id;
		newItem->callback = callback;
		newItem->index = 0;

		updateMessageHandlerIndex(g_nmi->_messageHandlers, 1);
		g_nmi->_messageHandlers = newItem;

		return true;
	}
}

bool insertMessageHandler(int (*callback)(Command *), int index, int16 id) {
	if (getMessageHandlerById(id))
		return false;

	MessageHandler *curItem = g_nmi->_messageHandlers;

	for (int i = index; i > 0; i--)
		if (curItem)
			curItem = curItem->nextItem;

	bool res = allocMessageHandler(curItem, id, callback, index + 1);
	if (curItem)
		updateMessageHandlerIndex(curItem->nextItem->nextItem, 1);

	return res;
}

void clearMessageHandlers() {
	MessageHandler *curItem;
	MessageHandler *nextItem;

	curItem = g_nmi->_messageHandlers;
	if (curItem) {
		do {
			nextItem = curItem->nextItem;

			delete curItem;

			curItem = nextItem;
		} while (nextItem);

		g_nmi->_messageHandlers = nullptr;
	}
}

void processMessages() {
	if (!g_nmi->_isProcessingMessages) {
		g_nmi->_isProcessingMessages = true;

		while (g_nmi->_exCommandList.size()) {
			Command *ex = g_nmi->_exCommandList.front();
			g_nmi->_exCommandList.pop_front();
			ex->handleMessage();
		}
		g_nmi->_isProcessingMessages = false;
	}
}

void updateGlobalMessageQueue(int id, int objid) {
	MessageQueue *m = g_nmi->_globalMessageQueueList->getMessageQueueById(id);
	if (m) {
		m->update();
	}
}

bool runQueue(int queueId, int flags) {
	MessageQueue *mq = g_nmi->_currentScene->findMessageQueue(queueId);

	if (!mq)
		return false;

	MessageQueue *nmq = new MessageQueue(mq, 0, 0);

	nmq->_flags |= flags;

	if (!nmq->run(nullptr)) {
		g_nmi->_globalMessageQueueList->deleteQueueById(nmq->_id);
		return false;
	}

	return true;
}

bool runQueueByIdWithAni(AniObject *obj, int queueId, int flags) {
	MessageQueue *mq = g_nmi->_currentScene->findMessageQueue(queueId);

	if (!mq)
		return false;

	MessageQueue *nmq = new MessageQueue(mq, 0, 0);

	nmq->_flags |= flags;

	if (!nmq->run(obj)) {
		g_nmi->_globalMessageQueueList->deleteQueueById(nmq->_id);
		return false;
	}

	return true;
}

void postExCommand(int parentId, int keyCode, int x, int y, int f20, int f14) {
	Command *ex = new Command(parentId, 17, 64, 0, 0, 0, 1, 0, 0, 0);

	ex->_param = keyCode;
	ex->_excFlags |= 3;
	ex->_x = x;
	ex->_y = y;
	ex->_invId = f20;
	ex->_z = f14;

	ex->postMessage();
}

} // End of namespace NGI
