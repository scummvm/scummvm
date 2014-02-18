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

#ifndef NEVERHOOD_MODULES_MODULE2900_H
#define NEVERHOOD_MODULES_MODULE2900_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

// Module2900

class Module2900 : public Module {
public:
	Module2900(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	int _sceneNum;
	int _teleporterModuleResult;
	void createScene(int sceneNum, int which);
	void updateScene();
	void updateMusic(bool halfVolume);
};

class SsScene2901LocationButtonLight;

class Scene2901 : public Scene {
public:
	Scene2901(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_ssLocationButtons[6];
	SsScene2901LocationButtonLight *_ssLocationButtonLights[6];
	Sprite *_ssBigButton;
	int _currWhirlButtonNum;
	int _prevWhirlButtonNum;
	int _countdown1;
	int _currLocationButtonNum;
	int _selectedButtonNum;
	int _skipCountdown;
	int _blinkOn;
	bool _isButton2Broken;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE2900_H */
