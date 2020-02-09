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

#ifndef NEVERHOOD_MODULES_MODULE1900_H
#define NEVERHOOD_MODULES_MODULE1900_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

class Module1900 : public Module {
public:
	Module1900(NeverhoodEngine *vm, Module *parentModule, int which);
	~Module1900() override;
protected:
	int _sceneNum;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void createScene(int sceneNum, int which);
	void updateScene();
};

class Scene1901 : public Scene {
public:
	Scene1901(NeverhoodEngine *vm, Module *parentModule, int which);
};

class AsScene1907Symbol;
class SsScene1907UpDownButton;
class AsScene1907WaterHint;

class Scene1907 : public Scene {
public:
	Scene1907(NeverhoodEngine *vm, Module *parentModule);
	void plugInFailed();
	void setPositionFree(int index, bool value) { _positionFree[index] = value; }
	int getNextPosition() { return _pluggedInCount++; }
protected:
	AsScene1907Symbol *_asSymbols[9];
	SsScene1907UpDownButton *_ssUpDownButton;
	AsScene1907WaterHint *_asWaterHint;
	int _currMovingSymbolIndex;
	int _pluggedInCount;
	int _moveDownCountdown;
	int _moveUpCountdown;
	int _countdown3;
	bool _hasPlugInFailed;
	bool _positionFree[9];
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	int getRandomPositionIndex();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE1900_H */
