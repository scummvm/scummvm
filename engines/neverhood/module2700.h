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

#ifndef NEVERHOOD_MODULE2700_H
#define NEVERHOOD_MODULE2700_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/module1600.h"

namespace Neverhood {

// Module2700

class Module2700 : public Module {
public:
	Module2700(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module2700();
protected:
	int _soundIndex;
	bool _flag1;
	uint32 _scene2711StaticSprites[6];
	uint32 _musicFileHash;
	void createScene(int sceneNum, int which);
	void updateScene();
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void createScene2703(int which, uint32 sceneInfoId, const uint32 *staticSprites = NULL, const NRect *clipRect = NULL);
	void createScene2704(int which, uint32 sceneInfoId, int16 value, const uint32 *staticSprites = NULL, const NRect *clipRect = NULL);
};

class SsCommonTrackShadowBackground : public StaticSprite {
public:
	SsCommonTrackShadowBackground(NeverhoodEngine *vm, uint32 fileHash);
};

class AsCommonCarShadow : public AnimatedSprite {
public:
	AsCommonCarShadow(NeverhoodEngine *vm, AnimatedSprite *asCar, BaseSurface *shadowSurface, uint index);
protected:
	uint _index;
	AnimatedSprite *_asCar;
	uint32 _animFileHash;
	void update();
	void updateShadow();
};

class AsCommonCarConnectorShadow : public AnimatedSprite {
public:
	AsCommonCarConnectorShadow(NeverhoodEngine *vm, Sprite *asCar, BaseSurface *shadowSurface, uint index);
protected:
	uint _index;
	Sprite *_asCar;
	void update();
};

class AsCommonCarTrackShadow : public AnimatedSprite {
public:
	AsCommonCarTrackShadow(NeverhoodEngine *vm, Sprite *asCar, BaseSurface *shadowSurface, int16 frameIndex);
protected:
	Sprite *_asCar;
	void update();
};

class Scene2701 : public Scene {
public:
	Scene2701(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	AsCommonCar *_asCar;
	Sprite *_ssTrackShadowBackground;
	Sprite *_asCarShadow;
	Sprite *_asCarTrackShadow;
	Sprite *_asCarConnectorShadow;
	Sprite *_asCarConnector;
	int _which1, _which2;
	NPointArray *_trackPoints;
	uint32 hmRidingCar(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmCarAtHome(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2702 : public Scene {
public:
	Scene2702(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	AsCommonCar *_asCar;
	Sprite *_ssTrackShadowBackground;
	Sprite *_asCarShadow;
	Sprite *_asCarTrackShadow;
	Sprite *_asCarConnectorShadow;
	int16 _newTrackDestX;
	int _currTrackIndex, _newTrackIndex;
	int _count;
	bool _flag1;
	SceneInfo2700 *_sceneInfos[2][3];
	SceneInfo2700 **_currSceneInfos;
	NPointArray *_trackPoints;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void findClosestTrack(NPoint pt);
	void changeTrack();
};

class Scene2704 : public Scene {
public:
	Scene2704(NeverhoodEngine *vm, Module *parentModule, int which, uint32 sceneInfoId, int16 value,
		const uint32 *staticSprites = NULL, const NRect *clipRect = NULL);
protected:
	AsCommonCar *_asCar;
	Sprite *_ssTrackShadowBackground;
	Sprite *_asCarShadow;
	Sprite *_asCarConnector;
	Sprite *_asCarTrackShadow;
	Sprite *_asCarConnectorShadow;
	int _which1, _which2;
	NPointArray *_trackPoints;
	NRectArray *_rectList;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2706 : public Scene {
public:
	Scene2706(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	AsCommonCar *_asCar;
	Sprite *_ssTrackShadowBackground;
	Sprite *_asCarShadow;
	Sprite *_asCarConnector;
	Sprite *_asCarTrackShadow;
	Sprite *_asCarConnectorShadow;
	int16 _newTrackDestX;
	int _currTrackIndex, _newTrackIndex;
	NPointArray *_trackPoints;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void findClosestTrack(NPoint pt);
	void changeTrack();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE2700_H */
