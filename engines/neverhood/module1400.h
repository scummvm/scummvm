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

#ifndef NEVERHOOD_MODULE1400_H
#define NEVERHOOD_MODULE1400_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/module1200.h"

namespace Neverhood {

class Module1400 : public Module {
public:
	Module1400(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module1400();
protected:
	void createScene(int sceneNum, int which);
	void updateScene();
};

// Scene1401

class Class525 : public AnimatedSprite {
public:
	Class525(NeverhoodEngine *vm);
	virtual ~Class525();
protected:
	int _countdown1;
	int _countdown2;
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	void update4662A0();
	void update466300();
	uint32 handleMessage466320(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage4663C0(int messageNum, const MessageParam &param, Entity *sender);
	void sub466420();
	void sub466460();
	void sub4664B0();
};

class Class526 : public AnimatedSprite {
public:
	Class526(NeverhoodEngine *vm, Sprite *class525);
protected:
	Sprite *_class525;
	SoundResource _soundResource;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void spriteUpdate466720();
	void sub466770();
};

class Class527 : public AnimatedSprite {
public:
	Class527(NeverhoodEngine *vm, Sprite *class526);
protected:
	Sprite *_class526;
	SoundResource _soundResource;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void spriteUpdate466920();
	void sub466970();
};

class Class528 : public AnimatedSprite {
public:
	Class528(NeverhoodEngine *vm, Sprite *klayman, bool flag);
protected:
	Sprite *_klayman;
	SoundResource _soundResource;
	int _countdown;
	bool _flag;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void sub466BF0();
	void sub466C50();
	void sub466CB0();
};

struct Class489Item {
	NPoint point;
	int8 varIndex1;
	int8 varIndex2;
	int8 index1;
	int8 flag2;
	int8 flag4;
	int8 flag;
};

class Class489 : public AnimatedSprite {
public:
	Class489(NeverhoodEngine *vm, Scene *parentScene, Sprite *klayman, Sprite *class525);
	virtual ~Class489();
protected:
	Scene *_parentScene;
	Sprite *_klayman;
	Sprite *_class525;
	const Class489Item *_class489Item;
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	SoundResource _soundResource3;
	int16 _remX;
	bool _flag;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage4348E0(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage434B20(int messageNum, const MessageParam &param, Entity *sender);
	void spriteUpdate434B60();
	void sub434C80();
	void sub434D80();
	void sub434DD0();
	void sub434DF0();
	void sub434E60();
	void sub434E90();
	void sub434EC0();
	void sub434F40();
	void sub434F80();
	void sub434FF0();
	void sub435040();
};

class Scene1401 : public Scene {
public:
	Scene1401(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	bool _flag;
	Sprite *_class427;
	Class489 *_class489;
	Sprite *_class525;
	Sprite *_class526;
	Sprite *_class527;
	Sprite *_class528;
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	Sprite *_ssButton;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

// Scene1402

class Class454 : public StaticSprite {
public:
	Class454(NeverhoodEngine *vm, uint32 fileHash, int surfacePriority);
};

class Class482 : public AnimatedSprite {
public:
	Class482(NeverhoodEngine *vm, Scene *parentScene, int which);
protected:
	Scene *_parentScene;
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void sub428500();
	void sub428530();
	void sub428560();
};

class Scene1402 : public Scene {
public:
	Scene1402(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_class454_1;
	Sprite *_class454_2;
	Sprite *_class454_3;
	Sprite *_class482;
	Class489 *_class489;
	bool _flag;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void sub428220();
	void sub428230();
};

// Scene1407

class AsScene1407Mouse : public AnimatedSprite {
public:
	AsScene1407Mouse(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	int16 _walkDestX;
	int16 _currSectionIndex;
	int16 _nextHoleIndex;
	int _countdown;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void suWalkTo();
	void upGoThroughHole();
	void stIdleLookAtGoodHole();
	void stWalkToDest();
	void stWalkToHole();
	void stGoThroughHole();
	void stArriveAtHole();
};

class Scene1407 : public Scene {
public:
	Scene1407(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	SoundResource _soundResource;
	Sprite *_asMouse;
	Sprite *_ssResetButton;
	int _puzzleSolvedCountdown;
	int _resetButtonCountdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

// Scene1403

class Scene1403 : public Scene {
public:
	Scene1403(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_class401_1;
	Sprite *_class401_2;
	Sprite *_class401_3;
	AsScene1201Tape *_asTape1;
	AsScene1201Tape *_asTape2;
	Class489 *_class489;
	bool _flag;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

// Scene1404

class Scene1404 : public Scene {
public:
	Scene1404(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Scene1404();
protected:
	Sprite *_sprite1;
	Sprite *_asTape;
	Class489 *_class489;
	Sprite *_class545;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

// Scene1405

class Scene1405;

class AsScene1405Tile : public AnimatedSprite {
public:
	AsScene1405Tile(NeverhoodEngine *vm, Scene1405 *parentScene, uint32 index);
	void show();
	void hide();
protected:
	Scene1405 *_parentScene;
	SoundResource _soundResource;
	bool _flag;
	uint32 _index;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1405 : public Scene {
public:
	Scene1405(NeverhoodEngine *vm, Module *parentModule, int which);
	int getCountdown() const { return _countdown; }
protected:
	SoundResource _soundResource;
	bool _selectFirstTile;
	int _firstTileIndex;
	int _secondTileIndex;
	int _tilesLeft;
	int _countdown;
	AsScene1405Tile *_tiles[48];
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE1400_H */
