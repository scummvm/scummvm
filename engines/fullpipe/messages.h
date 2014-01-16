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

#ifndef FULLPIPE_MESSAGEQUEUE_H
#define FULLPIPE_MESSAGEQUEUE_H

#include "fullpipe/utils.h"
#include "fullpipe/inventory.h"
#include "fullpipe/gfx.h"
#include "fullpipe/sound.h"
#include "fullpipe/scene.h"

namespace Fullpipe {

class Message : public CObject {
 public:
	int _messageKind;
	int16 _parentId;
	int _x;
	int _y;
	int _field_14;
	int _sceneClickX;
	int _sceneClickY;
	int _field_20;
	int _field_24;
	int _keyCode;
	int _field_2C;
	int _field_30;
	int _field_34;

 public:
	Message();
	Message(Message *src);
	virtual ~Message() {}

	Message(int16 parentId, int messageKind, int x, int y, int a6, int a7, int sceneClickX, int sceneClickY, int a10);
};

class ExCommand : public Message {
 public:
	int _messageNum;
	int _field_3C;
	int _excFlags;
	int _parId;

	ExCommand();
	ExCommand(ExCommand *src);
	ExCommand(int16 parentId, int messageKind, int messageNum, int x, int y, int a7, int a8, int sceneClickX, int sceneClickY, int a11);
	virtual ~ExCommand() {}

	virtual bool load(MfcArchive &file);

	virtual ExCommand *createClone();

	bool handleMessage();
	void sendMessage();
	void postMessage();
	void handle();

	void firef34();
	void setf3c(int val);
};

class ExCommand2 : public ExCommand {
 public:
	Common::Point **_points;
	int _pointsSize;

	ExCommand2(int messageKind, int parentId, Common::Point **points, int pointsSize);
	ExCommand2(ExCommand2 *src);
	virtual ~ExCommand2();

	virtual ExCommand2 *createClone();
};

class ObjstateCommand : public ExCommand {
 public:
	char *_objCommandName;
	int _value;

 public:
	ObjstateCommand();
	ObjstateCommand(ObjstateCommand *src);
	virtual ~ObjstateCommand();

	virtual bool load(MfcArchive &file);

	virtual ObjstateCommand *createClone();
};

class MessageQueue : public CObject {
  public:
	int _id;
	int _flags;
	char *_queueName;
	int16 _dataId;
	CObject *_field_14;
	int _counter;
	int _field_38;
	int _isFinished;
	int _parId;
	int _flag1;

  private:
	Common::List<ExCommand *> _exCommands;

 public:
	MessageQueue();
	MessageQueue(int dataId);
	MessageQueue(MessageQueue *src, int parId, int field_38);
	virtual ~MessageQueue();

	virtual bool load(MfcArchive &file);

	int getFlags() { return _flags; }
	void setFlags(int flags) { _flags = flags; }

	uint getCount() { return _exCommands.size(); }

	void addExCommand(ExCommand *ex);
	void addExCommandToEnd(ExCommand *ex);
	void insertExCommandAt(int pos, ExCommand *ex);
	ExCommand *getExCommandByIndex(uint idx);
	void deleteExCommandByIndex(uint idx, bool doFree);

	void transferExCommands(MessageQueue *mq);

	void replaceKeyCode(int key1, int key2);

	bool chain(StaticANIObject *ani);
	void update();
	void sendNextCommand();
	void finish();

	void messageQueueCallback1(int par);

	bool checkGlobalExCommandList1();
	bool checkGlobalExCommandList2();

	int calcDuration(StaticANIObject *obj);
	void changeParam28ForObjectId(int objId, int oldParam28, int newParam28);
};

class GlobalMessageQueueList : public Common::Array<MessageQueue *> {
  public:
	MessageQueue *getMessageQueueById(int id);
	void deleteQueueById(int id);
	void removeQueueById(int id);
	void disableQueueById(int id);
	void addMessageQueue(MessageQueue *msg);

	int compact();
};

struct MessageHandler {
	int (*callback)(ExCommand *cmd);
	int16 id;
	int16 field_6;
	int index;
	MessageHandler *nextItem;
};

bool removeMessageHandler(int16 id, int pos);
void updateMessageHandlerIndex(MessageHandler *msg, int offset);
void addMessageHandler(int (*callback)(ExCommand *), int16 id);
MessageHandler *getMessageHandlerById(int16 id);
bool allocMessageHandler(MessageHandler *where, int16 id, int (*callback)(ExCommand *), int index);
int getMessageHandlersCount();
bool addMessageHandlerByIndex(int (*callback)(ExCommand *), int index, int16 id);
bool insertMessageHandler(int (*callback)(ExCommand *), int index, int16 id);
void clearMessageHandlers();
void processMessages();
void updateGlobalMessageQueue(int id, int objid);
void clearMessages();
void clearGlobalMessageQueueList1();

bool chainQueue(int queueId, int flags);
bool chainObjQueue(StaticANIObject *obj, int queueId, int flags);
void postExCommand(int parentId, int keyCode, int x, int y, int f20, int f16);

} // End of namespace Fullpipe

#endif /* FULLPIPE_MESSAGEQUEUE_H */
