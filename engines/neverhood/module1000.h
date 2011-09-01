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

#ifndef NEVERHOOD_MODULE1000_H
#define NEVERHOOD_MODULE1000_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

// Module1000

class Module1000 : public Module {
public:
	Module1000(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module1000();
protected:
	uint32 _musicFileHash;
	// TODO ResourceTable _resourceTable1;
	// TODO ResourceTable _resourceTable2;
	// TODO ResourceTable _resourceTable3;
	// TODO ResourceTable _resourceTable4;
	void createScene1001(int which);			
	void createScene1002(int which);			
	void createScene1003(int which);			
	void createScene1004(int which);			
	void createScene1005(int which);			
	void updateScene1001();			
	void updateScene1002();			
	void updateScene1003();			
	void updateScene1004();			
	void updateScene1005();			
};

// Scene1001

class AsScene1001Door : public AnimatedSprite {
public:
	AsScene1001Door(NeverhoodEngine *vm);
protected:
	SoundResource _soundResource1;	
	SoundResource _soundResource2;	
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void handleMessage2000h();
	void callback1();
	void callback2();
	void callback3();
};

class AsScene1001Hammer : public AnimatedSprite {
public:
	AsScene1001Hammer(NeverhoodEngine *vm, Sprite *asDoor);
protected:
	Sprite *_asDoor;
	SoundResource _soundResource;	
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1001Window : public AnimatedSprite {
public:
	AsScene1001Window(NeverhoodEngine *vm);
protected:
	SoundResource _soundResource;	
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1001Lever : public AnimatedSprite {
public:
	AsScene1001Lever(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, int deltaXType);
protected:
	Scene *_parentScene;
	SoundResource _soundResource;	
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsCommonButtonSprite : public StaticSprite {
public:
	SsCommonButtonSprite(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash, int surfacePriority, uint32 soundFileHash);
protected:
	Scene *_parentScene;
	SoundResource _soundResource;
	uint32 _soundFileHash;
	int16 _countdown;
	void update();	
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1001 : public Scene {
public:
	Scene1001(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Scene1001();
protected:
	Sprite *_asHammer;
	Sprite *_asDoor;
	Sprite *_asWindow;
	Sprite *_asLever;
	Sprite *_ssButton;
	int16 _fieldE4;
	int16 _fieldE6;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

// TODO: Move this to some common file since it's used several times

class Class152 : public Scene {
public:
	Class152(NeverhoodEngine *vm, Module *parentModule, uint32 backgroundFileHash, uint32 cursorFileHash);
protected:
	// TODO: Are these used?
	int16 _fieldD0;	
	int16 _fieldD2;	
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

// Scene1002

class SsScene1002LadderArch : public StaticSprite {
public:
	SsScene1002LadderArch(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Class599 : public StaticSprite {
public:
	Class599(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1002Ring : public AnimatedSprite {
public:
	AsScene1002Ring(NeverhoodEngine *vm, Scene *parentScene, bool flag1, int16 x, int16 y, int16 clipY1, bool flag2);
protected:
	Scene *_parentScene;
	bool _flag1;
	SoundResource _soundResource;
	void update();
	uint32 handleMessage4475E0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage447760(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage447890(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage447930(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage447A00(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1002Door : public StaticSprite {
public:
	AsScene1002Door(NeverhoodEngine *vm, NRect &clipRect);
protected:
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void suOpenDoor();
	void suCloseDoor();
};

class Class505 : public AnimatedSprite {
public:
	Class505(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1002DoorSpy : public AnimatedSprite {
public:
	AsScene1002DoorSpy(NeverhoodEngine *vm, NRect &clipRect, Scene *parentScene, Sprite *asDoor, Sprite *class505);
protected:
	Scene *_parentScene;
	Sprite *_asDoor;
	Sprite *_class505;
	SoundResource _soundResource;
	NRect _rect;
	uint32 handleMessage4489D0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage448A60(int messageNum, const MessageParam &param, Entity *sender);
	void spriteUpdate448AA0();
	void sub448AC0();
	void sub448B10();
};

class Class426 : public StaticSprite {
public:
	Class426(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash1, uint32 fileHash2, int surfacePriority, uint32 soundFileHash);
	void setFileHashes(uint32 fileHash1, uint32 fileHash2);
protected:
	Scene *_parentScene;
	int _countdown;
	uint32 _fileHashes[2];
	int _status;
	SoundResource _soundResource;
	uint32 _soundFileHash;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1002VenusFlyTrap : public AnimatedSprite {
public:
	AsScene1002VenusFlyTrap(NeverhoodEngine *vm, Scene *parentScene, Sprite *klayman, bool flag);
protected:
	Scene *_parentScene;
	Sprite *_klayman;
	int _countdown;
	SoundResource _soundResource;
	bool _flag;
	void update();
	void update447FB0();
	uint32 handleMessage448000(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage4482E0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage448320(int messageNum, const MessageParam &param, Entity *sender);
	void sub4484F0();
	void sub448530();
	void sub448560();
	void sub4485B0();
	void sub4485F0();
	void sub448620();
	void sub448660();
	void sub448720();
	void sub448750();
	void sub448780();
};

class Class506 : public AnimatedSprite {
public:
	Class506(NeverhoodEngine *vm);
protected:
	int _countdown;
	bool _flag;
	void update();
	uint32 handleMessage4491B0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage449210(int messageNum, const MessageParam &param, Entity *sender);
	void sub449250();
	void sub449280();
	void sub4492C0();
};

class Class478 : public AnimatedSprite {
public:
	Class478(NeverhoodEngine *vm, Klayman *klayman);
protected:
	Klayman *_klayman;
	void update();
};

class Scene1002 : public Scene {
public:
	Scene1002(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Scene1002();
protected:
	// TODO ResourceTable _resourceTable1;
	// TODO ResourceTable _resourceTable2;
	// TODO ResourceTable _resourceTable3;
	// TODO ResourceTable _resourceTable4;
	// TODO ResourceTable _resourceTable5;
	// TODO ResourceTable _resourceTable6;
	// TODO ResourceTable _resourceTable7;
	// TODO ResourceTable _resourceTable8;
	Sprite *_asRing1;
	Sprite *_asRing2;
	Sprite *_asRing3;
	Sprite *_asRing4;
	Sprite *_asRing5;
	Sprite *_asDoor;
	Sprite *_asDoorSpy;
	Sprite *_asVenusFlyTrap;
	Sprite *_ssLadderArch;
	Sprite *_ssLadderArchPart1;
	Sprite *_ssLadderArchPart2;
	Sprite *_ssLadderArchPart3;
	Sprite *_class599;
	Sprite *_class478;
	Sprite *_class479;
	Sprite *_class506;
	Sprite *_class426;
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	SoundResource _soundResource3;
	bool _flag1B4;
	bool _flag1BE;
	bool _flag;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

// Scene1004

class AsScene1004TrashCan : public AnimatedSprite {
public:
	AsScene1004TrashCan(NeverhoodEngine *vm);
protected:
	SoundResource _soundResource;	
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1004 : public Scene {
public:
	Scene1004(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_class478;
	Sprite *_asTrashCan;
	int _paletteAreaStatus;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void updatePaletteArea();
};

// Scene1005

class Scene1005 : public Scene {
public:
	Scene1005(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void drawTextToBackground();
	FontSurface *createFontSurface();
	uint32 getTextIndex();
	uint32 getTextIndex1();
	uint32 getTextIndex2();
	uint32 getTextIndex3();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE1000_H */
