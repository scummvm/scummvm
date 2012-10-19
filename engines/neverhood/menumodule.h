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

// TODO: I couldn't come up with a better name than 'Module' so far

#ifndef NEVERHOOD_MENUMODULE_H
#define NEVERHOOD_MENUMODULE_H

#include "common/str.h"
#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

class MenuModule : public Module {
public:
	MenuModule(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~MenuModule();
protected:
	int _sceneNum;
	Common::String _savegameName;
	Background *_savedBackground;
	byte *_savedPaletteData;
	// TODO _savegameList (list of strings?)
	void createScene(int sceneNum, int which);
	void updateScene();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class MainMenuButton : public StaticSprite {
public:
	MainMenuButton(NeverhoodEngine *vm, Scene *parentScene, uint buttonIndex);
protected:
	Scene *_parentScene;
	int _countdown;
	uint _buttonIndex;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class MainMenu : public Scene {
public:
	MainMenu(NeverhoodEngine *vm, Module *parentModule);
protected:
	Sprite *_musicOnButton;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MENUMODULE_H */
