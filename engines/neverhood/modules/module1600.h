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

#ifndef NEVERHOOD_MODULES_MODULE1600_H
#define NEVERHOOD_MODULES_MODULE1600_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

class Module1600 : public Module {
public:
	Module1600(NeverhoodEngine *vm, Module *parentModule, int which);
	~Module1600() override;
protected:
	int _sceneNum;
	void createScene(int sceneNum, int which);
	void updateScene();
};

class AsCommonCar;

class Scene1608 : public Scene {
public:
	Scene1608(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene1608() override;
protected:
	AsCommonCar *_asCar;
	Sprite *_asKey;
	Sprite *_asIdleCarLower;
	Sprite *_asIdleCarFull;
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	Sprite *_asTape;
	Klaymen *_kmScene1608;
	NRect _clipRect1;
	NRect _clipRect2;
	NRect _clipRect3;
	int _carStatus;
	bool _carClipFlag;
	bool _klaymenInCar;
	int _countdown1;
	NPointArray *_roomPathPoints;
	void upLowerFloor();
	void upUpperFloor();
	void upCarAtHome();
	void upGettingOutOfCar();
	void upRidingCar();
	uint32 hmLowerFloor(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmUpperFloor(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmRidingCar(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmCarAtHome(int messageNum, const MessageParam &param, Entity *sender);
	void updateKlaymenCliprect();
};

class AsScene3011Symbol;

class Scene1609 : public Scene {
	friend class Console;
public:
	Scene1609(NeverhoodEngine *vm, Module *parentModule);
protected:
	Sprite *_ssButton;
	AsScene3011Symbol *_asSymbols[12];
	int _currentSymbolIndex;
	int _noisySymbolIndex;
	int _symbolPosition;
	int _countdown1;
	bool _changeCurrentSymbol;
	bool _isSolved;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	bool testVars();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE1600_H */
