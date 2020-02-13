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

#ifndef NEVERHOOD_MODULES_MODULE2500_H
#define NEVERHOOD_MODULES_MODULE2500_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/modules/module1600_sprites.h"	// for Tracks

namespace Neverhood {

class Module2500 : public Module {
public:
	Module2500(NeverhoodEngine *vm, Module *parentModule, int which);
	~Module2500() override;
protected:
	int _sceneNum;
	int _soundIndex;
	void createScene(int sceneNum, int which);
	void updateScene();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void createScene2704(int which, uint32 sceneInfoId, int16 value, const uint32 *staticSprites = NULL, const NRect *clipRect = NULL);
};

class AsCommonCar;

class Scene2501 : public Scene {
public:
	Scene2501(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2501() override;
protected:
	AsCommonCar *_asCar;
	Sprite *_ssTrackShadowBackground;
	Sprite *_asCarShadow;
	Sprite *_asCarConnectorShadow;
	Sprite *_asCarTrackShadow;
	Sprite *_asIdleCarLower;
	Sprite *_asIdleCarFull;
	Klaymen *_kmScene2501;
	Tracks _tracks;
	NPointArray *_trackPoints;
	int _currTrackIndex;
	NPoint _clickPoint;
	int _newTrackIndex;
	int _carStatus;
	bool _klaymenInCar;
	void update();
	void upCarAtHome();
	void upGettingOutOfCar();
	void upRidingCar();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmRidingCar(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmCarAtHome(int messageNum, const MessageParam &param, Entity *sender);
	void moveCarToPoint(NPoint &pt);
	void changeTrack();
	void updateKlaymenClipRect();
};

class Scene2504 : public Scene {
public:
	Scene2504(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE2500_H */
