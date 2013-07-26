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
	_param28 = file.readUint32LE();
	_field_2C = file.readUint32LE();
	_field_30 = file.readUint32LE();
	_field_34 = file.readUint32LE();

	_messageNum = file.readUint32LE();

	_field_3C = 0;

	if (g_fullpipe->_gameProjectVersion >= 12) {
		_excFlags = file.readUint32LE();
		_parId = file.readUint32LE();
	}

	return true;
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
	_param28 = 0;
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
	_param28 = src->_param28;
	_field_2C = src->_field_2C;
	_field_30 = src->_field_30;
	_field_34 = src->_field_34;
}

CObjstateCommand::CObjstateCommand() {
	_value = 0;
}

bool CObjstateCommand::load(MfcArchive &file) {
	debug(5, "CObjStateCommand::load()");

	_cmd.load(file);

	_value = file.readUint32LE();

	_objCommandName = file.readPascalString();

	return true;
}

MessageQueue::MessageQueue() {
	_field_14 = 0;
	_parId = 0;
	_dataId = 0;
	_id = 0;
	_isFinished = 0;
	_flags = 0;
}

bool MessageQueue::load(MfcArchive &file) {
	debug(5, "MessageQueue::load()");

	_dataId = file.readUint16LE();

	int count = file.readUint16LE();

	assert(g_fullpipe->_gameProjectVersion >= 12);

	_queueName = file.readPascalString();

	for (int i = 0; i < count; i++) {
		CObject *tmp = file.readClass();

		_exCommands.push_back(tmp);
	}

	_id = -1;
	_field_14 = 0;
	_parId = 0;
	_isFinished = 0;

	return true;
}

MessageQueue *GlobalMessageQueueList::getMessageQueueById(int id) {
	for (CPtrList::iterator s = begin(); s != end(); ++s) {
		if (((MessageQueue *)s)->_id == id)
			return (MessageQueue *)s;
	}

	return 0;
}

void GlobalMessageQueueList::deleteQueueById(int id) {
	for (uint i = 0; i < size(); i++)
		if (((MessageQueue *)((*this).operator[](i)))->_id == id) {
			delete (MessageQueue *)remove_at(i);

			disableQueueById(id);
			return;
		}
}

void GlobalMessageQueueList::disableQueueById(int id) {
	for (CPtrList::iterator s = begin(); s != end(); ++s) {
		if (((MessageQueue *)s)->_parId == id)
			((MessageQueue *)s)->_parId = 0;
	}
}

bool removeMessageHandler(int16 id, int pos) {
	if (g_fullpipe->_messageHandlers) {
		MessageHandler *curItem = g_fullpipe->_messageHandlers;
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

	MessageHandler *curItem = g_fullpipe->_messageHandlers;

	if (!curItem)
		return;

	int index = 0;
	for (MessageHandler *i = g_fullpipe->_messageHandlers->nextItem; i; i = i->nextItem) {
		curItem = i;
		index++;
	}

	allocMessageHandler(curItem, id, callback, index);

	if (curItem)
      updateMessageHandlerIndex(curItem->nextItem->nextItem, 1);

}

MessageHandler *getMessageHandlerById(int16 id) {
	MessageHandler *curItem = g_fullpipe->_messageHandlers;

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

		g_fullpipe->_messageHandlers = msg;
	}

	return true;
}

int getMessageHandlersCount() {
	int result;
	MessageHandler *curItem = g_fullpipe->_messageHandlers;

	for (result = 0; curItem; result++)
		curItem = curItem->nextItem;

	return result;
}

bool addMessageHandlerByIndex(int (*callback)(ExCommand *), int index, int16 id) {
	if (getMessageHandlerById(id))
		return false;

	if (index) {
		MessageHandler *curItem = g_fullpipe->_messageHandlers;

		for (int i = index - 1; i > 0; i--)
			if (curItem)
				curItem = curItem->nextItem;

		bool res = allocMessageHandler(curItem, id, callback, index);

		if (res)
			updateMessageHandlerIndex(curItem->nextItem->nextItem, 1);

		return res;
	} else {
		MessageHandler *newItem = new MessageHandler;

		newItem->nextItem = g_fullpipe->_messageHandlers;
		newItem->id = id;
		newItem->callback = callback;
		newItem->index = 0;

		updateMessageHandlerIndex(g_fullpipe->_messageHandlers, 1);
		g_fullpipe->_messageHandlers = newItem;

		return true;
	}
}

bool insertMessageHandler(int (*callback)(ExCommand *), int index, int16 id) {
	if (getMessageHandlerById(id))
		return false;

	MessageHandler *curItem = g_fullpipe->_messageHandlers;

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

	curItem = g_fullpipe->_messageHandlers;
	if (curItem) {
		do {
			nextItem = curItem->nextItem;

			delete curItem;

			curItem = nextItem;
		} while (nextItem);

		g_fullpipe->_messageHandlers = 0;
	}
}

} // End of namespace Fullpipe
