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

#ifndef NEVERHOOD_MODULES_MODULE2200_H
#define NEVERHOOD_MODULES_MODULE2200_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/modules/module1000.h"
#include "neverhood/graphics.h"

namespace Neverhood {

class Module2200 : public Module {
public:
	Module2200(NeverhoodEngine *vm, Module *parentModule, int which);
	~Module2200() override;
protected:
	int _sceneNum;
	void createScene(int sceneNum, int which);
	void updateScene();
	void createHallOfRecordsScene(int which, uint32 hallOfRecordsInfoId);
};

class Scene2201 : public Scene {
public:
	Scene2201(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2201() override;
protected:
	NRect _clipRects[2];
	Sprite *_ssDoorLight;
	Sprite *_asDoor;
	Sprite *_ssDoorButton;
	Sprite *_asTape;
	bool _isSoundPlaying;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2202 : public Scene {
public:
	Scene2202(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2202() override;
protected:
	Sprite *_ssMovingCube;
	Sprite *_ssDoneMovingCube;
	bool _isCubeMoving;
	int16 _movingCubePosition;
	int _surfacePriority;
	bool _leaveScene;
	bool _isSolved;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	int16 getFreeCubePosition(int16 index);
	bool testIsSolved();
};

class Scene2203 : public Scene {
public:
	Scene2203(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2203() override;
protected:
	Sprite *_asLeftDoor;
	Sprite *_asRightDoor;
	Sprite *_ssSmallLeftDoor;
	Sprite *_ssSmallRightDoor;
	Sprite *_asTape;
	Sprite *_asKey;
	NRect _leftDoorClipRect;
	NRect _rightDoorClipRect;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsCommonPressButton;

class Scene2205 : public Scene {
public:
	Scene2205(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	SsCommonPressButton *_ssLightSwitch;
	Sprite *_ssDoorFrame;
	bool _isKlaymenInLight;
	bool _isLightOn;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2206 : public Scene {
public:
	Scene2206(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2206() override;
protected:
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	Sprite *_asDoorSpikes;
	Sprite *_ssButton;
	Sprite *_asPlatform;
	Sprite *_ssTestTube;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void klaymenInFrontSpikes();
	void klaymenBehindSpikes();
	void readClickedColumn();
};

class Scene2207 : public Scene {
public:
	Scene2207(NeverhoodEngine *vm, Module *parentModule);
protected:
	Sprite *_asElevator;
	Sprite *_ssMaskPart1;
	Sprite *_ssMaskPart2;
	Sprite *_ssMaskPart3;
	Sprite *_asTape;
	Sprite *_asLever;
	Sprite *_asWallRobotAnimation;
	Sprite *_asWallCannonAnimation;
	Sprite *_ssButton;
	int _elevatorSurfacePriority;
	bool _klaymenAtElevator;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage2(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2208 : public Scene {
public:
	Scene2208(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2208() override;
protected:
	FontSurface *_fontSurface;
	BaseSurface *_backgroundSurface;
	BaseSurface *_topBackgroundSurface;
	BaseSurface *_bottomBackgroundSurface;
	TextResource _textResource;
	int16 _backgroundScrollY;
	int16 _newRowIndex;
	int16 _currRowIndex;
	int16 _rowScrollY;
	int16 _maxRowIndex;
	int16 _visibleRowsCount;
	Common::Array<const char*> _strings;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void drawRow(int16 rowIndex);
};

class Scene2242 : public Scene {
public:
	Scene2242(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2242() override;
protected:
	Sprite *_asTape;
	bool _isKlaymenInLight;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void readClickedColumn();
};

class HallOfRecordsScene : public Scene {
public:
	HallOfRecordsScene(NeverhoodEngine *vm, Module *parentModule, int which, uint32 hallOfRecordsInfoId);
	~HallOfRecordsScene() override;
protected:
	HallOfRecordsInfo *_hallOfRecordsInfo;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void readClickedColumn();
};

class Scene2247 : public Scene {
public:
	Scene2247(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2247() override;
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void readClickedColumn();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE2200_H */
