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

#ifndef NEVERHOOD_MODULES_MODULE1300_SPRITES_H
#define NEVERHOOD_MODULES_MODULE1300_SPRITES_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

class AsScene1302Bridge : public AnimatedSprite {
public:
	AsScene1302Bridge(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stLowerBridge();
	void stRaiseBridge();
	void cbLowerBridgeEvent();
};

class SsScene1302Fence : public StaticSprite {
public:
	SsScene1302Fence(NeverhoodEngine *vm);
protected:
	int16 _firstY;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void suMoveDown();
	void suMoveUp();
};

class AsScene1303Balloon : public AnimatedSprite {
public:
	AsScene1303Balloon(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmBalloonPopped(int messageNum, const MessageParam &param, Entity *sender);
	void stPopBalloon();
};

class AsScene1304Needle : public AnimatedSprite {
public:
	AsScene1304Needle(NeverhoodEngine *vm, Scene *parentScene, int surfacePriority, int16 x, int16 y);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1306Elevator : public AnimatedSprite {
public:
	AsScene1306Elevator(NeverhoodEngine *vm, Scene *parentScene, AnimatedSprite *asElevatorDoor);
protected:
	Scene *_parentScene;
	AnimatedSprite *_asElevatorDoor;
	bool _isUp;
	bool _isDown;
	int _countdown;
	void update();
	void upGoingDown();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stGoingUp();
	void cbGoingUpEvent();
	void stGoingDown();
	void cbGoingDownEvent();
};

class AsScene1307Key : public AnimatedSprite {
public:
	AsScene1307Key(NeverhoodEngine *vm, Scene *parentScene, uint keyIndex, NRect *clipRects);
protected:
	Scene *_parentScene;
	NPointArray *_pointList;
	uint _pointIndex;
	int _frameIndex;
	uint _keyIndex;
	NRect *_clipRects;
	bool _isClickable;
	int16 _prevX, _prevY;
	int16 _deltaX, _deltaY;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void suRemoveKey();
	void suInsertKey();
	void suMoveKey();
	void stRemoveKey();
	void stInsertKey();
	void stMoveKey();
	void stUnlock();
	void stInsert();
};

class AsScene1308JaggyDoor : public AnimatedSprite {
public:
	AsScene1308JaggyDoor(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stOpenDoor();
	void stOpenDoorDone();
	void stCloseDoor();
	void stCloseDoorDone();
};

class AsScene1308KeyboardDoor : public AnimatedSprite {
public:
	AsScene1308KeyboardDoor(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stFallingKeys();
	void stFallingKeysDone();
};

class AsScene1308LightWallSymbols : public AnimatedSprite {
public:
	AsScene1308LightWallSymbols(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stFadeIn();
	void stFadeOut();
	void stFadeOutDone();
};

class SsScene1308Number : public StaticSprite {
public:
	SsScene1308Number(NeverhoodEngine *vm, uint32 fileHash, int index);
};

class AsScene1308Mouse : public AnimatedSprite {
public:
	AsScene1308Mouse(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class KmScene1303 : public Klaymen {
public:
	KmScene1303(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene1304 : public Klaymen {
public:
	KmScene1304(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene1305 : public Klaymen {
public:
	KmScene1305(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	void stCrashDown();
	void stCrashDownFinished();

	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene1306 : public Klaymen {
public:
	KmScene1306(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene1308 : public Klaymen {
public:
	KmScene1308(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE1300_SPRITES_H */
