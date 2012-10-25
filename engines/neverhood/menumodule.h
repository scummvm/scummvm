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

class CreditsScene : public Scene {
public:
	CreditsScene(NeverhoodEngine *vm, Module *parentModule, bool canAbort);
	virtual ~CreditsScene();
protected:
	int _screenIndex;
	int _countdown;
	MusicResource *_musicResource;
	uint32 _ticksTime;
	uint32 _ticksDuration;
	bool _canAbort;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Widget;

class WidgetScene : public Scene {
public:
	WidgetScene(NeverhoodEngine *vm, Module *parentModule);
	void getMousPos(NPoint &pt);
	virtual void setCurrWidget(Widget *newWidget);
protected:
	Widget *_currWidget;
};

class Widget : public StaticSprite {
public:
	Widget(NeverhoodEngine *vm, int16 x, int16 y, int16 itemID, WidgetScene *parentScene,
		int baseObjectPriority, int baseSurfacePriority, bool visible);
	virtual void show();
	virtual void hide();
	virtual void onClick();
	virtual void setPosition(int16 x, int16 y);
	virtual void refresh();
	virtual void addSprite();
	virtual int16 getWidth();
	virtual int16 getHeight();
	virtual void enterWidget();
	virtual void exitWidget();
protected:
	int16 _itemID;
	WidgetScene *_parentScene;
	int _baseObjectPriority;
	int _baseSurfacePriority;
	bool _visible;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MENUMODULE_H */
