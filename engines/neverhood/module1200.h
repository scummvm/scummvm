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

#ifndef NEVERHOOD_MODULE1200_H
#define NEVERHOOD_MODULE1200_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

// Module1200

class Module1200 : public Module {
public:
	Module1200(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module1200();
protected:
	// TODO ResourceTable _resourceTable;
	bool _moduleDone;
	uint32 _moduleDoneStatus; 
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void createScene1201(int which);			
	void createScene1202(int which);			
	void createScene1203(int which);			
	void updateScene1201();			
	void updateScene1202();			
	void updateScene1203();			
};

// Scene1201

class AsScene1201Tape : public AnimatedSprite {
public:
	AsScene1201Tape(NeverhoodEngine *vm, Scene *parentScene, uint32 nameHash, int surfacePriority, int16 x, int16 y, uint32 fileHash);
protected:
	Scene *_parentScene;
	uint32 _nameHash;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Class466 : public AnimatedSprite {
public:
	Class466(NeverhoodEngine *vm, bool flag);
protected:
	SoundResource _soundResource;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void sub40D340();
	void sub40D360();
	void sub40D380();
};

class AsScene1201RightDoor : public AnimatedSprite {
public:
	AsScene1201RightDoor(NeverhoodEngine *vm, Sprite *klayman, bool flag);
protected:
	SoundResource _soundResource;
	Sprite *_klayman;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void sub40D7E0();
	void sub40D830();
	void sub40D880();
};

class Class464 : public AnimatedSprite {
public:
	Class464(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene1201TntMan : public AnimatedSprite {
public:
	AsScene1201TntMan(NeverhoodEngine *vm, Scene *parentScene, Sprite *class466, bool flag);
	virtual ~AsScene1201TntMan();
protected:
	Scene *_parentScene;
	Sprite *_class466;
	Sprite *_sprite;
	SoundResource _soundResource;
	bool _flag;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage40CCD0(int messageNum, const MessageParam &param, Entity *sender);
	void spriteUpdate40CD10();
	void sub40CD30();
	void sub40CD60();
	void sub40CD90();
};

class Class465 : public AnimatedSprite {
public:
	Class465(NeverhoodEngine *vm, Sprite *asTntMan);
	~Class465();
protected:
	Sprite *_asTntMan;
	void update();
	void spriteUpdate40D150();
};

class AsScene1201Match : public AnimatedSprite {
public:
	AsScene1201Match(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	SoundResource _soundResource;
	int _countdown;
	int _status;
	void update();
	uint32 handleMessage40C2D0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage40C320(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage40C360(int messageNum, const MessageParam &param, Entity *sender);
	void sub40C3E0();
	void sub40C420();
	void sub40C470();
	void sub40C4C0();
	void sub40C4F0();
};

class AsScene1201Creature : public AnimatedSprite {
public:
	AsScene1201Creature(NeverhoodEngine *vm, Scene *parentScene, Sprite *klayman);
protected:
	Scene *_parentScene;
	Sprite *_klayman;
	SoundResource _soundResource;
	int _countdown1;
	int _countdown2;
	int _countdown3;
	bool _flag;
	void update();
	uint32 handleMessage40C710(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage40C7B0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage40C830(int messageNum, const MessageParam &param, Entity *sender);
	void sub40C8E0();
	void sub40C930();
	void sub40C960();
	void sub40C990();
	void sub40C9B0();
	void sub40C9E0();
};

class AsScene1201LeftDoor : public AnimatedSprite {
public:
	AsScene1201LeftDoor(NeverhoodEngine *vm, Sprite *klayman);
protected:
	Sprite *_klayman;
	SoundResource _soundResource;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void sub40D590();
};

class SsScene1201Tnt : public StaticSprite {
public:
	SsScene1201Tnt(NeverhoodEngine *vm, uint32 elemIndex, uint32 pointIndex, int16 clipY2);
protected:
	uint32 _elemIndex;
	int16 _field7A;	
};

class Scene1201 : public Scene {
public:
	Scene1201(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Scene1201();
protected:
	// TODO ResourceTable _resourceTable1;
	// TODO ResourceTable _resourceTable2;
	Sprite *_asMatch;
	Sprite *_asTntMan;
	Sprite *_asCreature;
	Sprite *_class466;
	Sprite *_asLeftDoor;
	Sprite *_asRightDoor;
	Sprite *_asTape;
	bool _flag;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

// Scene1202

class AsScene1202TntItem : public AnimatedSprite {
public:
	AsScene1202TntItem(NeverhoodEngine *vm, Scene *parentScene, int index);
protected:
	Scene *_parentScene;
	int _index, _index2;
	uint32 handleMessage453FE0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage454060(int messageNum, const MessageParam &param, Entity *sender);
	void sub4540A0();
	void sub4540D0();
	void sub454100();
	void sub454160();
};

class Scene1202 : public Scene {
public:
	Scene1202(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Scene1202();
protected:
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	SoundResource _soundResource3;
	SoundResource _soundResource4;
	PaletteResource _paletteResource;
	Sprite *_asTntItems[18];
	int _counter;
	int _index;
	byte _paletteData[1024];
	bool _soundFlag;
	bool _flag;
	void update();
	uint32 handleMessage453C10(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage453D90(int messageNum, const MessageParam &param, Entity *sender);
	bool isSolved();
	void doPaletteEffect();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE1200_H */
