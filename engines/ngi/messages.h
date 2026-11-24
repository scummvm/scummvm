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

#ifndef NGI_MESSAGEQUEUE_H
#define NGI_MESSAGEQUEUE_H

#include "ngi/utils.h"
#include "ngi/inventory.h"
#include "ngi/gfx.h"
#include "ngi/sound.h"
#include "ngi/scene.h"

namespace NGI {

enum QueueFlags {
	kInGlobalQueue = 2,
	kBehaviorQueue = 8
};

class Message : public CObject {
 public:
	int _cmd;
	int16 _parentId;
	int _x;
	int _y;
	int _z;
	int _sceneClickX;
	int _sceneClickY;
	int _invId;
	int _wait;
	int _param;
	int _auxParam;
	int _oneShot;
	int _armed;

 public:
	Message();

	Message(int16 parentId, int messageKind, int x, int y, int a6, int a7, int sceneClickX, int sceneClickY, int a10);
};

class Command : public Message {
 public:
	enum { kCmdPause = 64 }; // Timed pause in XML queues.
	Common::String _messageName;
	bool _mirror = false;
	int _messageNum;
	int _field_3C;
	int _excFlags;
	int _parId;

	Command();
	Command(Command *src);
	Command(int16 parentId, int messageKind, int messageNum, int x, int y, int a7, int a8, int sceneClickX, int sceneClickY, int a11);
	~Command() override {}

	bool load(MfcArchive &file) override;

	virtual Command *createClone();

	bool handleMessage();
	void sendMessage();
	void postMessage();
	void handle();

	void firef34();
	void setf3c(int val);
};

class ExCommand2 : public Command {
 public:
	PointList _points;

	ExCommand2(int messageKind, int parentId, const PointList &points);
	ExCommand2(ExCommand2 *src);

	ExCommand2 *createClone() override;
};

class ObjstateCommand : public Command {
 public:
	Common::String _objCommandName;
	int _value;

 public:
	ObjstateCommand();
	ObjstateCommand(ObjstateCommand *src);

	bool load(MfcArchive &file) override;

	ObjstateCommand *createClone() override;
};

class MessageQueue : public CObject {
  public:
	int _id;
	int _flags;
	Common::String _title;
	int16 _queueId;
	CObject *_field_14;
	int _counter;
	int _field_38;
	int _isFinished;
	int _parId;
	int _flag1;

  private:
	Common::List<Command *> _commands;
	Common::ScopedPtr<Command> _pauseCommand;

 public:
	MessageQueue();
	MessageQueue(int dataId);
	MessageQueue(MessageQueue *src, int parId, int field_38);
	~MessageQueue() override;

	bool load(MfcArchive &file) override;

	int getFlags() { return _flags; }
	void setFlags(int flags) { _flags = flags; }

	uint getCount() { return _commands.size(); }

	void addCommandToFront(Command *ex);
	void addCommandToEnd(Command *ex);
	void addCommand(int pos, Command *ex);
	Command *getCommandByIndex(uint idx);
	void removeCommand(uint idx, bool doFree);

	void mergeQueue(MessageQueue *mq);

	void setObjectCopy(int key1, int key2);

	/** `ani` will own `this` if `run` returns true */
	bool run(AniObject *ani);
	void update();
	void next();
	bool updatePause(uint32 elapsed);
	void finish();

	void messageQueueCallback1(int par);

	bool checkGlobalExCommandList1();
	bool checkGlobalExCommandList2();

	int calcDuration(AniObject *obj);
	void setObjectCopyForId(int objId, int oldCopyId, int newCopyId);

	int activateExCommandsByKind(int kind);
};

class GlobalMessageQueueList : public Common::Array<MessageQueue *> {
public:
	MessageQueue *getMessageQueueById(int id);
	void deleteQueueById(int id);
	void removeQueueById(int id);
	void disableQueueById(int id);
	/** `msg` becomes owned by `this` */
	void addMessageQueue(MessageQueue *msg);
	void clear();

	int compact();
};

struct MessageHandler {
	int (*callback)(Command *cmd);
	int16 id;
	int16 field_6;
	int index;
	MessageHandler *nextItem;
};

bool removeMessageHandler(int16 id, int pos);
void updateMessageHandlerIndex(MessageHandler *msg, int offset);
void addMessageHandler(int (*callback)(Command *), int16 id);
MessageHandler *getMessageHandlerById(int16 id);
bool allocMessageHandler(MessageHandler *where, int16 id, int (*callback)(Command *), int index);
int getMessageHandlersCount();
bool addMessageHandlerByIndex(int (*callback)(Command *), int index, int16 id);
bool insertMessageHandler(int (*callback)(Command *), int index, int16 id);
void clearMessageHandlers();
void processMessages();
void updateGlobalMessageQueue(int id, int objid);
void clearMessages();
void clearGlobalMessageQueueList();
void clearGlobalMessageQueueList1();

bool runQueue(int queueId, int flags);
bool runQueueByIdWithAni(AniObject *obj, int queueId, int flags);
void postExCommand(int parentId, int keyCode, int x, int y, int f20, int f16);

} // End of namespace NGI

#endif /* NGI_MESSAGEQUEUE_H */
