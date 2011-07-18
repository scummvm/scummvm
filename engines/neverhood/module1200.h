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

class Class468 : public AnimatedSprite {
public:
	Class468(NeverhoodEngine *vm, Sprite *klayman, bool flag);
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

class Class463 : public AnimatedSprite {
public:
	Class463(NeverhoodEngine *vm, Scene *parentScene, Sprite *class466, bool flag);
	virtual ~Class463();
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
	Sprite *_class461;
	Sprite *_class463;
	Sprite *_class462;
	Sprite *_class466;
	Sprite *_class467;
	Sprite *_class468;
	Sprite *_asTape;
	bool _flag;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE1200_H */
