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

#ifndef NEVERHOOD_MODULES_MODULE3000_H
#define NEVERHOOD_MODULES_MODULE3000_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

class Module3000 : public Module {
public:
	Module3000(NeverhoodEngine *vm, Module *parentModule, int which);
	~Module3000() override;
protected:
	int _waterfallSoundVolume;
	bool _isWaterfallRunning;
	void createScene(int sceneNum, int which);
	void updateScene();
};

class SsScene3009SymbolEdges;
class SsScene3009TargetLine;
class AsScene3009VerticalIndicator;
class AsScene3009HorizontalIndicator;
class AsScene3009Symbol;

class Scene3009 : public Scene {
public:
	Scene3009(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene3009() override;
	bool isTurning();
protected:
	int _lockSymbolsPart1Countdown;
	int _lockSymbolsPart2Countdown;
	SmackerPlayer *_cannonSmackerPlayer;
	Sprite *_ssFireCannonButton;
	SsScene3009SymbolEdges *_ssSymbolEdges[2];
	SsScene3009TargetLine *_ssTargetLines[2];
	AsScene3009VerticalIndicator *_asVerticalIndicator;
	AsScene3009HorizontalIndicator *_asHorizontalIndicator;
	AsScene3009Symbol *_asSymbols[6];
	uint32 _cannonTargetStatus;
	uint32 _correctSymbols[6];
	bool _keepVideo;
	bool _moveCannonLeftFirst;
	bool _isTurning;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void playActionVideo();
	bool isSymbolsPart1Solved();
	bool isSymbolsPart2Solved();
	void openSmacker(uint32 fileHash, bool keepLastFrame);
};

class SsScene3010DeadBoltButton;
class AsScene3010DeadBolt;

class Scene3010 : public Scene {
public:
	Scene3010(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	int _countdown;
	bool _doorUnlocked;
	bool _checkUnlocked;
	SsScene3010DeadBoltButton *_ssDeadBoltButtons[3];
	AsScene3010DeadBolt *_asDeadBolts[3];
	bool _boltUnlocked[3];
	bool _boltUnlocking[3];
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene3011Symbol;

class Scene3011 : public Scene {
public:
	Scene3011(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_ssButton;
	AsScene3011Symbol *_asSymbols[12];
	int _updateStatus;
	bool _buttonClicked;
	int _countdown;
	int _noisySymbolIndex;
	int _currentSymbolIndex;
	int _noisyRandomSymbolIndex;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void fadeIn();
	void fadeOut();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE3000_H */
