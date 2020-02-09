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

#ifndef NEVERHOOD_MODULES_MODULE1400_H
#define NEVERHOOD_MODULES_MODULE1400_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

class Module1400 : public Module {
public:
	Module1400(NeverhoodEngine *vm, Module *parentModule, int which);
	~Module1400() override;
protected:
	int _sceneNum;
	void createScene(int sceneNum, int which);
	void updateScene();
};

class AsCommonProjector;
class AsScene1201Tape;
class AsScene1405Tile;

class Scene1401 : public Scene {
public:
	Scene1401(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	bool _projectorBorderFlag;
	Sprite *_ssFloorButton;
	AsCommonProjector *_asProjector;
	Sprite *_asPipe;
	Sprite *_asMouse;
	Sprite *_asCheese;
	Sprite *_asBackDoor;
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	Sprite *_ssButton;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1402 : public Scene {
public:
	Scene1402(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_ssBridgePart1;
	Sprite *_ssBridgePart2;
	Sprite *_ssBridgePart3;
	Sprite *_asPuzzleBox;
	AsCommonProjector *_asProjector;
	bool _isShaking;
	void upShaking();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void startShaking();
	void stopShaking();
};

class Scene1407 : public Scene {
public:
	Scene1407(NeverhoodEngine *vm, Module *parentModule);
protected:
	Sprite *_asMouse;
	Sprite *_ssResetButton;
	int _puzzleSolvedCountdown;
	int _resetButtonCountdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1403 : public Scene {
public:
	Scene1403(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	AsScene1201Tape *_asTape1;
	AsScene1201Tape *_asTape2;
	AsCommonProjector *_asProjector;
	bool _isProjecting;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1404 : public Scene {
public:
	Scene1404(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene1404() override;
protected:
	Sprite *_sprite1;
	Sprite *_asTape;
	AsCommonProjector *_asProjector;
	Sprite *_asKey;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1405 : public Scene {
public:
	Scene1405(NeverhoodEngine *vm, Module *parentModule);
	int getCountdown() const { return _countdown; }
protected:
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

#endif /* NEVERHOOD_MODULES_MODULE1400_H */
