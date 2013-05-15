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

#ifndef NEVERHOOD_MODULES_MODULE1100_H
#define NEVERHOOD_MODULES_MODULE1100_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

// Module1100

class Module1100 : public Module {
public:
	Module1100(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module1100();
protected:
	int _sceneNum;
	int _countdown;
	void createScene(int sceneNum, int which);
	void updateScene();
};

class SsScene1105Button : public StaticSprite {
public:
	SsScene1105Button(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash, NRect &collisionBounds);
protected:
	Scene *_parentScene;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsScene1105Symbol : public StaticSprite {
public:
	SsScene1105Symbol(NeverhoodEngine *vm, uint32 fileHash, int16 x, int16 y);
	void hide();
};

class SsScene1105SymbolDie : public StaticSprite {
public:
	SsScene1105SymbolDie(NeverhoodEngine *vm, uint dieIndex, int16 x, int16 y);
	void hide();
protected:
	uint _dieIndex;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void loadSymbolSprite();
};

class AsScene1105TeddyBear : public AnimatedSprite {
public:
	AsScene1105TeddyBear(NeverhoodEngine *vm, Scene *parentScene);
	void show();
	void hide();
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsScene1105OpenButton : public StaticSprite {
public:
	SsScene1105OpenButton(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	int _countdown;
	bool _isClicked;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1105 : public Scene {
public:
	Scene1105(NeverhoodEngine *vm, Module *parentModule);
protected:
	int _countdown;
	int _backgroundIndex;
	bool _isPanelOpen;
	bool _isActionButtonClicked;
	bool _doMoveTeddy;
	bool _isClosePanelDone;
	int _leaveResult;
	AsScene1105TeddyBear *_asTeddyBear;
	SsScene1105Symbol *_ssSymbols[3];
	SsScene1105SymbolDie *_ssSymbolDice[3];
	Sprite *_ssSymbol1UpButton;
	Sprite *_ssSymbol1DownButton;
	Sprite *_ssSymbol2UpButton;
	Sprite *_ssSymbol2DownButton;
	Sprite *_ssSymbol3UpButton;
	Sprite *_ssSymbol3DownButton;
	Sprite *_ssActionButton;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void createObjects();
	void upOpenPanel();
	void upClosePanel();
	void update();
};

class Scene1109 : public Scene {
public:
	Scene1109(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_sprite1;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE1100_H */
