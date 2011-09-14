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

namespace Neverhood {

// Module1300

class Module1300 : public Module {
public:
	Module1300(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module1300();
protected:
	uint32 _musicFileHash;
	void createScene1302(int which);			
	void createScene1303(int which);			
	void createScene1304(int which);			
	void createScene1305(int which);			
	void createScene1306(int which);			
	void createScene1307(int which);			
	void createScene1308(int which);			
	void createScene1309(int which);			
	void createScene1310(int which);			
	void createScene1311(int which);			
	void createScene1312(int which);			
	void createScene1313(int which);			
	void createScene1314(int which);			
	void createScene1315(int which);			
	void createScene1316(int which);			
	void createScene1317(int which);			
	void createScene1318(int which);			
	void updateScene1302();			
	void updateScene1303();			
	void updateScene1304();			
	void updateScene1305();			
	void updateScene1306();			
	void updateScene1307();			
	void updateScene1308();			
	void updateScene1309();			
	void updateScene1310();			
	void updateScene1311();			
	void updateScene1312();			
	void updateScene1313();			
	void updateScene1314();			
	void updateScene1315();			
	void updateScene1316();			
	void updateScene1317();			
	void updateScene1318();			
};

class AsScene1302Bridge : public AnimatedSprite {
public:
	AsScene1302Bridge(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stLowerBridge();
	void stRaiseBridge();
	void cbLowerBridgeEvent();
};

class SsScene1302Fence : public StaticSprite {
public:
	SsScene1302Fence(NeverhoodEngine *vm);
protected:
	SoundResource _soundResource1;
	SoundResource _soundResource2;
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
	SoundResource _soundResource;
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
	SoundResource _soundResource;
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
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	SoundResource _soundResource3;
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

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE1300_H */
