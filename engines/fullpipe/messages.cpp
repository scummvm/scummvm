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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "fullpipe/fullpipe.h"

#include "fullpipe/objects.h"
#include "fullpipe/messages.h"
#include "fullpipe/modal.h"
#include "fullpipe/statics.h"

namespace Fullpipe {

ExCommand::ExCommand() {
	_field_3C = 1;
	_messageNum = 0;
	_excFlags = 0;
	_parId = 0;
}

ExCommand::ExCommand(ExCommand *src) : Message(src) {
	_field_3C = 1;
	_messageNum = src->_messageNum;
	_excFlags = src->_excFlags;
	_parId = src->_parId;
}

ExCommand *ExCommand::createClone() {
	return new ExCommand(this);
}

ExCommand::ExCommand(int16 parentId, int messageKind, int messageNum, int x, int y, int a7, int a8, int sceneClickX, int sceneClickY, int a11) : 
	Message(parentId, messageKind, x, y, a7, a8, sceneClickX, sceneClickY, a11) {
	_field_3C = 1;
	_messageNum = messageNum;
	_excFlags = 0;
	_parId = 0;
}

bool ExCommand::load(MfcArchive &file) {
	debug(5, "ExCommand::load()");

	_parentId = file.readUint16LE();
	_messageKind = file.readUint32LE();
	_x = file.readUint32LE();
	_y = file.readUint32LE();
	_field_14 = file.readUint32LE();
	_sceneClickX = file.readUint32LE();
	_sceneClickY = file.readUint32LE();
	_field_20 = file.readUint32LE();
	_field_24 = file.readUint32LE();
	_keyCode = file.readUint32LE();
	_field_2C = file.readUint32LE();
	_field_30 = file.readUint32LE();
	_field_34 = file.readUint32LE();

	_messageNum = file.readUint32LE();

	_field_3C = 0;

	if (g_fp->_gameProjectVersion >= 12) {
		_excFlags = file.readUint32LE();
		_parId = file.readUint32LE();
	}

	_objtype = kObjTypeExCommand;

	return true;
}

bool ExCommand::handleMessage() {
	int cnt = 0;
	for (MessageHandler *m = g_fp->_messageHandlers; m; m = m->nextItem)
		cnt += m->callback(this);

	if (_messageKind == 17 || (_excFlags & 1)) {
		if (_parId) {
			MessageQueue *mq = g_fp->_globalMessageQueueList->getMessageQueueById(_parId);
			if (mq)
				mq->update();
		}
	}

	if (_excFlags & 2)
		delete this;

	return (cnt > 0);
}

void ExCommand::sendMessage() {
	g_fp->_exCommandList.push_back(this);

	processMessages();
}

void ExCommand::postMessage() {
	g_fp->_exCommandList.push_back(this);
}

void ExCommand::handle() {
	if (g_fp->_modalObject) {
		g_fp->_modalObject->handleMessage(this);

		delete this;
	} else {
		postMessage();
	}
}

void ExCommand::setf3c(int val) {
	if (val != -1)
		_field_3C = val;

	_field_34 = 1;
}

void ExCommand::firef34() {
	if (_field_34) {
		if (_field_3C >= _keyCode) {
			_field_34 = 0;

			sendMessage();

			if (!_field_30 )
				setf3c(_field_2C);
		}
	}
}

ExCommand2::ExCommand2(int messageKind, int parentId, Common::Point **points, int pointsSize) : ExCommand(parentId, messageKind, 0, 0, 0, 0, 1, 0, 0, 0) {
	_objtype = kObjTypeExCommand2;

	_pointsSize = pointsSize;
	_points = (Common::Point **)malloc(sizeof(Common::Point *) * pointsSize);

	for (int i = 0; i < pointsSize; i++) {
		_points[i] = new Common::Point;

		*_points[i] = *points[i];
	}
}

ExCommand2::ExCommand2(ExCommand2 *src) : ExCommand(src) {
	_pointsSize = src->_pointsSize;
	_points = (Common::Point **)malloc(sizeof(Common::Point *) * _pointsSize);

	for (int i = 0; i < _pointsSize; i++) {
		_points[i] = new Common::Point;

		*_points[i] = *src->_points[i];
	}
}

ExCommand2::~ExCommand2() {
	for (int i = 0; i < _pointsSize; i++)
		delete _points[i];

	free(_points);
}

ExCommand2 *ExCommand2::createClone() {
	return new ExCommand2(this);
}

Message::Message() {
	_messageKind = 0;
	_parentId = 0;		

	_x = 0;
	_y = 0;
	_field_14 = 0;
	_sceneClickX = 0;
	_sceneClickY = 0;
	_field_20 = 0;
	_field_24 = 0;
	_keyCode = 0;
	_field_2C = 0;
	_field_30 = 0;
	_field_34 = 0;
}

Message::Message(Message *src) {
	_parentId = src->_parentId;
	_messageKind = src->_messageKind;
	_x = src->_x;
	_y = src->_y;
	_field_14 = src->_field_14;
	_sceneClickX = src->_sceneClickX;
	_sceneClickY = src->_sceneClickY;
	_field_20 = src->_field_20;
	_field_24 = src->_field_24;
	_keyCode = src->_keyCode;
	_field_2C = src->_field_2C;
	_field_30 = src->_field_30;
	_field_34 = src->_field_34;
}

Message::Message(int16 parentId, int messageKind, int x, int y, int a6, int a7, int sceneClickX, int sceneClickY, int a10) {
	_messageKind = messageKind;
	_parentId = parentId;
	_x = x;
	_y = y;
	_field_14 = a6;
	_sceneClickX = sceneClickX;
	_sceneClickY = sceneClickY;
	_field_24 = a7;
	_field_20 = a10;
	_keyCode = 0;
	_field_2C = 0;
	_field_30 = 0;
	_field_34 = 0;
}

ObjstateCommand::ObjstateCommand() {
	_value = 0;
	_objCommandName = 0;
}

ObjstateCommand::ObjstateCommand(ObjstateCommand *src) : ExCommand(src) {
	_value = src->_value;
	_objCommandName = (char *)calloc(strlen(src->_objCommandName) + 1, 1);

	strncpy(_objCommandName, src->_objCommandName, strlen(src->_objCommandName));
}

ObjstateCommand::~ObjstateCommand() {
	free(_objCommandName);
}

bool ObjstateCommand::load(MfcArchive &file) {
	debug(5, "ObjStateCommand::load()");

	_objtype = kObjTypeObjstateCommand;

	ExCommand::load(file);

	_value = file.readUint32LE();

	_objCommandName = file.readPascalString();

	return true;
}

ObjstateCommand *ObjstateCommand::createClone() {
	return new ObjstateCommand(this);
}

MessageQueue::MessageQueue() {
	_field_14 = 0;
	_parId = 0;
	_dataId = 0;
	_id = 0;
	_isFinished = 0;
	_flags = 0;
	_queueName = 0;
	_counter = 0;
	_field_38 = 0;
	_flag1 = 0;
}

MessageQueue::MessageQueue(int dataId) {
	_field_14 = 0;
	_parId = 0;
	_dataId = dataId;
	_id = g_fp->_globalMessageQueueList->compact();
	_isFinished = 0;
	_flags = 0;
	_queueName = 0;
	_counter = 0;
	_field_38 = 0;
	_flag1 = 0;
}

MessageQueue::MessageQueue(MessageQueue *src, int parId, int field_38) {
	_counter = 0;
	_field_38 = (field_38 == 0);

	for (Common::List<ExCommand *>::iterator it = src->_exCommands.begin(); it != src->_exCommands.end(); ++it) {
		ExCommand *ex = (*it)->createClone();
		ex->_excFlags |= 2;

		_exCommands.push_back(ex);
	}
	_field_14 = src->_field_14;

	if (parId)
		_parId = parId;
	else
		_parId = src->_parId;

	_id = g_fp->_globalMessageQueueList->compact();
	_dataId = src->_dataId;
	_flags = src->_flags;
	_queueName = 0;

	g_fp->_globalMessageQueueList->addMessageQueue(this);

	_isFinished = 0;
	_flag1 = 0;
}

MessageQueue::~MessageQueue() {
	for (Common::List<ExCommand *>::iterator it = _exCommands.begin(); it != _exCommands.end(); ++it) {
		ExCommand *ex = (ExCommand *)*it;

		if (ex && ex->_excFlags & 2)
			delete ex;
	}

	_exCommands.clear();

	if (_field_14)
		delete _field_14;

	if (_flags & 2) {
		g_fp->_globalMessageQueueList->removeQueueById(_id);
	}

	finish();

	free(_queueName);
}

bool MessageQueue::load(MfcArchive &file) {
	debug(5, "MessageQueue::load()");

	_dataId = file.readUint16LE();

	int count = file.readUint16LE();

	assert(g_fp->_gameProjectVersion >= 12);

	_queueName = file.readPascalString();

	for (int i = 0; i < count; i++) {
		ExCommand *tmp = (ExCommand *)file.readClass();

		_exCommands.push_back(tmp);
	}

	_id = -1;
	_field_14 = 0;
	_parId = 0;
	_isFinished = 0;

	return true;
}

bool MessageQueue::chain(StaticANIObject *ani) {
	if (checkGlobalExCommandList1() && checkGlobalExCommandList2()) {
		if (!(getFlags() & 2)) {
			g_fp->_globalMessageQueueList->addMessageQueue(this);
			_flags |= 2;
		}
		if (ani) {
			ani->queueMessageQueue(this);
			return true;
		} else {
			sendNextCommand();
			return true;
		}
	}
	return false;
}

void MessageQueue::update() {
	if (_counter > 0)
		_counter--;

	if (getCount()) {
		sendNextCommand();
	} else if (_counter == 0) {
		_isFinished = 1;
		finish();
	}
}

void MessageQueue::messageQueueCallback1(int par) {
	// Autosave
	debug(3, "STUB: MessageQueue::messageQueueCallback1()");
}

void MessageQueue::addExCommand(ExCommand *ex) {
	_exCommands.push_front(ex);
}

void MessageQueue::addExCommandToEnd(ExCommand *ex) {
	_exCommands.push_back(ex);
}

void MessageQueue::insertExCommandAt(int pos, ExCommand *ex) {
	Common::List<ExCommand *>::iterator it = _exCommands.begin();

	for (int i = pos; i > 0; i--)
		++it;

	_exCommands.insert(it, ex);
}

ExCommand *MessageQueue::getExCommandByIndex(uint idx) {
	if (idx >= getCount())
		return 0;

	Common::List<ExCommand *>::iterator it = _exCommands.begin();

	while (idx) {
		++it;
		idx--;
	}

	return *it;
}

void MessageQueue::deleteExCommandByIndex(uint idx, bool doFree) {
	if (idx >= getCount())
		return;

	Common::List<ExCommand *>::iterator it = _exCommands.begin();

	while (idx) {
		++it;
		idx--;
	}

	if (doFree)
		delete *it;

	_exCommands.erase(it);
}

void MessageQueue::transferExCommands(MessageQueue *mq) {
	while (mq->_exCommands.size()) {
		_exCommands.push_back(mq->_exCommands.front());
		mq->_exCommands.pop_front();
	}
}

void MessageQueue::sendNextCommand() {
	if (getCount()) {
		if (!(_flags & 4) && (_flags & 1)) {
			messageQueueCallback1(16);
		}
		ExCommand *ex = _exCommands.front();

		_exCommands.pop_front();

		_counter++;
		ex->_parId = _id;
		ex->_excFlags |= (ex->_field_24 == 0 ? 1 : 0) | (ex->_field_3C != 0 ? 2 : 0);

		_flags |= 4;
		ex->sendMessage();
	} else if (_counter <= 0) {
		_isFinished = 1;
		finish();
	}
}

bool MessageQueue::checkGlobalExCommandList1() {
	ExCommand *ex, *ex1;

	for (uint i = 0; i < getCount(); i++) {
		ex = getExCommandByIndex(i);

		if (ex->_messageKind != 1 && ex->_messageKind != 20 && ex->_messageKind != 5 && ex->_messageKind != 27)
			continue;

		for (Common::List<ExCommand *>::iterator it = g_fp->_exCommandList.begin(); it != g_fp->_exCommandList.end(); it++) {
			ex1 = *it;

			if (ex1->_messageKind != 1 && ex1->_messageKind != 20 && ex1->_messageKind != 5 && ex1->_messageKind != 27)
				continue;

			if (ex1->_keyCode != ex->_keyCode && ex1->_keyCode != -1 && ex->_keyCode != -1)
				continue;

			MessageQueue *mq = g_fp->_globalMessageQueueList->getMessageQueueById(ex1->_parId);

			if (mq) {
				if (mq->getFlags() & 1)
					return false;
			}
		}
	}
	return true;
}

bool MessageQueue::checkGlobalExCommandList2() {
	ExCommand *ex, *ex1;

	for (uint i = 0; i < getCount(); i++) {
		ex = getExCommandByIndex(i);

		if (ex->_messageKind != 1 && ex->_messageKind != 20 && ex->_messageKind != 5 && ex->_messageKind != 27)
			continue;

		for (Common::List<ExCommand *>::iterator it = g_fp->_exCommandList.begin(); it != g_fp->_exCommandList.end();) {
			ex1 = *it;

			if (ex1->_messageKind != 1 && ex1->_messageKind != 20 && ex1->_messageKind != 5 && ex1->_messageKind != 27) {
				it++;
				continue;
			}

			if (ex1->_keyCode != ex->_keyCode && ex1->_keyCode != -1 && ex->_keyCode != -1) {
				it++;
				continue;
			}

			MessageQueue *mq = g_fp->_globalMessageQueueList->getMessageQueueById(ex1->_parId);

			if (mq) {
				if (mq->getFlags() & 1)
					return false;

				delete mq;
			}

			it = g_fp->_exCommandList.erase(it);

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

	MessageQueue *mq = g_fp->_globalMessageQueueList->getMessageQueueById(_parId);

	_parId = 0;

	if (!mq)
		return;

	if (!_flag1) {
		mq->update();
		return;
	}

	mq->_counter--;

	if (!mq->_counter && !mq->_exCommands.size())
		mq->update();
}

void MessageQueue::replaceKeyCode(int key1, int key2) {
	for (uint i = 0; i < getCount(); i++) {
		ExCommand *ex = getExCommandByIndex(i);
		int k = ex->_messageKind;
		if ((k == 1 || k == 20 || k == 5 || k == 6 || k == 2 || k == 18 || k == 19 || k == 22 || k == 55)
					&& ex->_keyCode == key1)
			ex->_keyCode = key2;
    }
}

int MessageQueue::calcDuration(StaticANIObject *obj) {
	int res = 0;
	ExCommand *ex;
	Movement *mov;

	for (uint i = 0; i < getCount(); i++) {
		ex = getExCommandByIndex(i);
		if (ex->_parentId == obj->_id) {
			if (ex->_messageKind == 1 || ex->_messageKind == 20) {
				if ((mov = obj->getMovementById(ex->_messageNum)) != 0) {
					if (ex->_field_14 >= 1)
						res += ex->_field_14;
					else
						res += mov->calcDuration();
				}
			}
		}
	}

	return res;
}

void MessageQueue::changeParam28ForObjectId(int objId, int oldParam28, int newParam28) {
	for (uint i = 0; i < getCount(); i++) {
		ExCommand *ex = getExCommandByIndex(i);
		int k = ex->_messageKind;

		if ((k == 1 || k == 20 || k == 5 || k == 6 || k == 2 || k == 18 || k == 19 || k == 22 || k == 55)
			 && ex->_keyCode == oldParam28
			 && ex->_parentId == objId)
			ex->_keyCode = newParam28;
    }
}

MessageQueue *GlobalMessageQueueList::getMessageQueueById(int id) {
	for (Common::Array<MessageQueue *>::iterator s = begin(); s != end(); ++s) {
		if ((*s)->_id == id)
			return *s;
	}

	return 0;
}

void GlobalMessageQueueList::deleteQueueById(int id) {
	for (uint i = 0; i < size(); i++)
		if (_storage[i]->_id == id) {
			remove_at(i);

			disableQueueById(id);
			return;
		}
}

void GlobalMessageQueueList::removeQueueById(int id) {
	for (uint i = 0; i < size(); i++)
		if (_storage[i]->_id == id) {
			_storage[i]->_flags &= 0xFD; // It is quite pointless
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
	int *useList = new int[size() + 2];

	for (uint i = 0; i < size() + 2; i++)
		useList[i] = 0;

	for (uint i = 0; i < size();) {
		if (((MessageQueue *)_storage[i])->_isFinished) {
			disableQueueById(_storage[i]->_id);
			remove_at(i);
		} else {
			if ((uint)_storage[i]->_id < size() + 2)
				useList[_storage[i]->_id] = 1;
			i++;
		}
	}

	uint i;

	for (i = 1; i < size() + 2; i++) {
		if (!useList[i])
			break;
	}

	delete [] useList;

	return i;
}

void GlobalMessageQueueList::addMessageQueue(MessageQueue *msg) {
	msg->setFlags(msg->getFlags() | 2);

	push_back(msg);
}

void clearGlobalMessageQueueList1() {
	clearMessages();

	g_fp->_globalMessageQueueList->clear();
}

void clearMessages() {
	while (g_fp->_exCommandList.size()) {
		ExCommand *ex = g_fp->_exCommandList.front();

		g_fp->_exCommandList.pop_front();

		if (ex->_excFlags & 2)
			delete ex;
	}
}

bool removeMessageHandler(int16 id, int pos) {
	if (g_fp->_messageHandlers) {
		MessageHandler *curItem = g_fp->_messageHandlers;
		MessageHandler *prevItem = 0;
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

void addMessageHandler(int (*callback)(ExCommand *), int16 id) {
	if (getMessageHandlerById(id))
		return;

	MessageHandler *curItem = g_fp->_messageHandlers;

	if (!curItem)
		return;

	int index = 0;
	for (MessageHandler *i = g_fp->_messageHandlers->nextItem; i; i = i->nextItem) {
		curItem = i;
		index++;
	}

	allocMessageHandler(curItem, id, callback, index);

	if (curItem)
		updateMessageHandlerIndex(curItem->nextItem->nextItem, 1);
}

MessageHandler *getMessageHandlerById(int16 id) {
	MessageHandler *curItem = g_fp->_messageHandlers;

	if (!curItem)
		return 0;

	while (id != curItem->id) {
		curItem = curItem->nextItem;

		if (!curItem)
			return 0;
	}

	return curItem;
}

bool allocMessageHandler(MessageHandler *where, int16 id, int (*callback)(ExCommand *), int index) {
	MessageHandler *msg = new MessageHandler;

	if (where) {
		msg->nextItem = where->nextItem;
		where->nextItem = msg;
		msg->id = id;
		msg->callback = callback;
		msg->index = index;
	} else {
		msg->nextItem = 0;
		msg->id = id;
		msg->callback = callback;
		msg->index = 0;

		g_fp->_messageHandlers = msg;
	}

	return true;
}

int getMessageHandlersCount() {
	int result;
	MessageHandler *curItem = g_fp->_messageHandlers;

	for (result = 0; curItem; result++)
		curItem = curItem->nextItem;

	return result;
}

bool addMessageHandlerByIndex(int (*callback)(ExCommand *), int index, int16 id) {
	if (getMessageHandlerById(id))
		return false;

	if (index) {
		MessageHandler *curItem = g_fp->_messageHandlers;

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

		newItem->nextItem = g_fp->_messageHandlers;
		newItem->id = id;
		newItem->callback = callback;
		newItem->index = 0;

		updateMessageHandlerIndex(g_fp->_messageHandlers, 1);
		g_fp->_messageHandlers = newItem;

		return true;
	}
}

bool insertMessageHandler(int (*callback)(ExCommand *), int index, int16 id) {
	if (getMessageHandlerById(id))
		return false;

	MessageHandler *curItem = g_fp->_messageHandlers;

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

	curItem = g_fp->_messageHandlers;
	if (curItem) {
		do {
			nextItem = curItem->nextItem;

			delete curItem;

			curItem = nextItem;
		} while (nextItem);

		g_fp->_messageHandlers = 0;
	}
}

void processMessages() {
	if (!g_fp->_isProcessingMessages) {
		g_fp->_isProcessingMessages = true;

		while (g_fp->_exCommandList.size()) {
			ExCommand *ex = g_fp->_exCommandList.front();
			g_fp->_exCommandList.pop_front();
			ex->handleMessage();
		}
		g_fp->_isProcessingMessages = false;
	}
}

void updateGlobalMessageQueue(int id, int objid) {
	MessageQueue *m = g_fp->_globalMessageQueueList->getMessageQueueById(id);  
	if (m) {
		m->update();
	}
}

bool chainQueue(int queueId, int flags) {
	MessageQueue *mq = g_fp->_currentScene->getMessageQueueById(queueId);

	if (!mq)
		return false;

	MessageQueue *nmq = new MessageQueue(mq, 0, 0);

	nmq->_flags |= flags;

	if (!nmq->chain(0)) {
		delete nmq;

		return false;
	}

	return true;
}

bool chainObjQueue(StaticANIObject *obj, int queueId, int flags) {
	MessageQueue *mq = g_fp->_currentScene->getMessageQueueById(queueId);

	if (!mq)
		return false;

	MessageQueue *nmq = new MessageQueue(mq, 0, 0);

	nmq->_flags |= flags;

	if (!nmq->chain(obj)) {
		delete nmq;

		return false;
	}

	return true;
}

void postExCommand(int parentId, int keyCode, int x, int y, int f20, int f14) {
	ExCommand *ex = new ExCommand(parentId, 17, 64, 0, 0, 0, 1, 0, 0, 0);

	ex->_keyCode = keyCode;
	ex->_excFlags |= 3;
	ex->_x = x;
	ex->_y = y;
	ex->_field_20 = f20;
	ex->_field_14 = f14;

	ex->postMessage();
}

} // End of namespace Fullpipe
