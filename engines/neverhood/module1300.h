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

#ifndef NEVERHOOD_MODULE1300_H
#define NEVERHOOD_MODULE1300_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/smackerplayer.h"

namespace Neverhood {

// Module1300

class Module1300 : public Module {
public:
	Module1300(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module1300();
protected:
	uint32 _musicFileHash;
	void createScene(int sceneNum, int which);
	void updateScene();
};

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

class Class595 : public StaticSprite {
public:
	Class595(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1302 : public Scene {
public:
	Scene1302(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_asVenusFlyTrap;
	Sprite *_asBridge;
	Sprite *_ssFence;
	Sprite *_asRing1;
	Sprite *_asRing2;
	Sprite *_asRing3;
	Sprite *_asRing4;
	Sprite *_asRing5;
	Sprite *_class595;
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
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

class Scene1303 : public Scene {
public:
	Scene1303(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_sprite1;
	Sprite *_asBalloon;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Class544 : public AnimatedSprite {
public:
	Class544(NeverhoodEngine *vm, Scene *parentScene, int surfacePriority, int16 x, int16 y);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1304 : public Scene {
public:
	Scene1304(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_sprite1;
	Sprite *_class545;
	Sprite *_class544;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1305 : public Scene {
public:
	Scene1305(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
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

class Scene1306 : public Scene {
public:
	Scene1306(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene1306();
protected:
	Sprite *_ssButton;
	Sprite *_asTape;
	AnimatedSprite *_asElevatorDoor;
	Sprite *_asElevator;
	Sprite *_sprite1;
	Sprite *_class545;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage416EB0(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1307Key : public AnimatedSprite {
public:
	AsScene1307Key(NeverhoodEngine *vm, Scene *parentScene, uint index, NRect *clipRects);
protected:
	Scene *_parentScene;
	NPointArray *_pointList;
	uint _pointIndex;
	int _frameIndex;
	uint _index;
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

class Scene1307 : public Scene {
public:
	Scene1307(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	NPointArray *_keyHolePoints;
	NRect _keyHoleRects[16];
	NRect _clipRects[4];
	Sprite *_asKeys[3];
	int _countdown;
	Sprite *_asCurrKey;
	bool _isInsertingKey;
	bool _doLeaveScene;
	bool _isPuzzleSolved;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Class549 : public AnimatedSprite {
public:
	Class549(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void sub455470();
	void hide();
	void sub4554F0();
	void sub455550();
};

class Class592 : public AnimatedSprite {
public:
	Class592(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void sub455710();
	void sub455740();
};

class Class593 : public AnimatedSprite {
public:
	Class593(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void sub4558F0();
	void sub455920();
	void sub455950();
};

class Class601 : public StaticSprite {
public:
	Class601(NeverhoodEngine *vm, uint32 fileHash, int index);
};

class Class513 : public AnimatedSprite {
public:
	Class513(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1308 : public Scene {
public:
	Scene1308(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_asTape;
	Sprite *_class549;
	Sprite *_class593;
	Sprite *_class601_1;
	Sprite *_class601_2;
	Sprite *_class601_3;
	AnimatedSprite *_class489;
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	Sprite *_sprite4;
	Sprite *_sprite5;
	bool _flag1;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1317 : public Scene {
public:
	Scene1317(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	SmackerPlayer *_smackerPlayer;
	bool _klaymanBlinks;
	int _klaymanBlinkCountdown;
	int _decisionCountdown;
	uint32 _smackerFileHash;
	bool _keepLastSmackerFrame;
	void update();
	void upChooseKing();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmChooseKing(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmNoDecisionYet(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmHoborgAsKing(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmKlaymanAsKing(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmEndMovie(int messageNum, const MessageParam &param, Entity *sender);
	void stChooseKing();
	void stNoDecisionYet();
	void stHoborgAsKing();
	void stKlaymanAsKing();
	void stEndMovie();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE1300_H */
