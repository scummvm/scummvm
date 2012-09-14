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

#ifndef NEVERHOOD_MODULE2500_H
#define NEVERHOOD_MODULE2500_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/module1000.h"
#include "neverhood/module1600.h"
#include "neverhood/module2700.h"

namespace Neverhood {

// Module2500

class Module2500 : public Module {
public:
	Module2500(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module2500();
protected:
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	SoundResource _soundResource3;
	SoundResource _soundResource4;
	int _soundIndex;
	void createScene(int sceneNum, int which);
	void updateScene();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void createScene2704(int which, uint32 sceneInfoId, int16 value, const uint32 *staticSprites = NULL, const NRect *clipRect = NULL);
};

class Class541 : public AnimatedSprite {
public:
	Class541(NeverhoodEngine *vm, int16 x, int16 y);
};

class Class542 : public AnimatedSprite {
public:
	Class542(NeverhoodEngine *vm, int16 x, int16 y);
};

class Scene2501 : public Scene {
public:
	Scene2501(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Scene2501();
protected:
	Class521 *_class521;
	Sprite *_class437;
	Sprite *_class517;
	Sprite *_class519;
	Sprite *_class520;
	Sprite *_class541;
	Sprite *_class542;
	Klayman *_kmScene2501;
	NPointArray *_trackPoints;
	SceneInfo2700 *_sceneInfos[3];
	int _currTrackIndex;
	NPoint _clickPoint;
	int _pointListsCount;
	int _newTrackIndex;
	int _carStatus;
	bool _klaymanInCar;
	void update();
	void upCarAtHome();
	void upGettingOutOfCar();
	void upRidingCar();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmRidingCar(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmCarAtHome(int messageNum, const MessageParam &param, Entity *sender);
	void findClosestTrack(NPoint &pt);
	void changeTrack();
	void updateKlaymanCliprect();
};

class Class450 : public StaticSprite {
public:
	Class450(NeverhoodEngine *vm);
protected:
	int _countdown;
	bool _flag1;
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	SoundResource _soundResource3;
	SoundResource _soundResource4;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2504 : public Scene {
public:
	Scene2504(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE2500_H */
