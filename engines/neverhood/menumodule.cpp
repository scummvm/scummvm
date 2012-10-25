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

#include "neverhood/menumodule.h"

namespace Neverhood {

enum {
	MAIN_MENU		= 0,
	CREDITS_SCENE	= 1,
	MAKING_OF		= 2
};

static const uint32 kMakingOfSmackerFileHashList[] = {
	0x21082409,
	0x21082809,
	0x21083009,
	0x21080009,
	0x21086009,
	0x2108A009,
	0x21092009,
	0x210A2009,
	0x210C2009,
	0x21082411,
	0x21082811,
	0x21083011,
	0x21080011,
	0
};

MenuModule::MenuModule(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	SetMessageHandler(&MenuModule::handleMessage);
	
	// TODO Check if the background actually needs to be saved
	_savedBackground = new Background(_vm, 0);
	_savedBackground->createSurface(0, 640, 480);
	// TODO Save current palette
	// TODO Stop all sounds and music

	_savedPaletteData = _vm->_screen->getPaletteData();

	createScene(MAIN_MENU, -1);
}

MenuModule::~MenuModule() {
	_vm->_screen->setPaletteData(_savedPaletteData);
}

void MenuModule::createScene(int sceneNum, int which) {
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case MAIN_MENU:
		_childObject = new MainMenu(_vm, this);
		break;
	case CREDITS_SCENE:
		_childObject = new CreditsScene(_vm, this, true);
		break;
	case MAKING_OF:
		createSmackerScene(kMakingOfSmackerFileHashList, false, true, true);
		break;
	}
	SetUpdateHandler(&MenuModule::updateScene);
	_childObject->handleUpdate();
}

void MenuModule::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case MAIN_MENU:
			// TODO
			switch (_moduleResult) {
			case 0:
				// 0048A2A5
				debug("RESTART GAME");
				break;
			case 1:
				debug("LOAD GAME");
				// TODO createLoadGameMenu();
				break;
			case 2:
				debug("SAVE GAME");
				// TODO createSaveGameMenu();
				break;
			case 3:
				debug("RESUME GAME");
				leaveModule(0);
				break;
			case 4:
				debug("QUIT GAME");
				leaveModule(0);
				_vm->quitGame();
				break;
			case 5:
				debug("CREDITS");
				createScene(CREDITS_SCENE, -1);
				break;
			case 6:
				debug("MAKING OF");
				createScene(MAKING_OF, -1);
				break;
			case 7:
				debug("TOGGLE MUSIC");
				// TODO Toggle music 0048A367
				createScene(MAIN_MENU, -1);
				break;
			case 8:
				debug("DELETE GAME");
				// TODO createDeleteGameMenu();
				break;
			default:
				createScene(MAIN_MENU, -1);
				break;
			}
			break;
		case CREDITS_SCENE:
		case MAKING_OF:
			createScene(MAIN_MENU, -1);
			break;
		default:
			break;
		}
	}
}

uint32 MenuModule::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	// TODO CHECKME Handles 0x101F, possibly just a debug/cache message which can be ignored?
	return Module::handleMessage(messageNum, param, sender);;
}

static const uint32 kMainMenuButtonFileHashes[] = {
	0x36C62120,
	0x56C62120,
	0x96C62120,
	0x16C62121,
	0x16C62122,
	0x16C62124,
	0x16C62128,
	0x16C62130,
	0x16C62100
};

MainMenuButton::MainMenuButton(NeverhoodEngine *vm, Scene *parentScene, uint buttonIndex)
	: StaticSprite(vm, 900), _parentScene(parentScene), _buttonIndex(buttonIndex), _countdown(0) {

	loadSprite(kMainMenuButtonFileHashes[_buttonIndex], kSLFDefDrawOffset | kSLFDefPosition, 100);

	// TODO Move to const array
	switch (_buttonIndex) {
	case 0:
		_collisionBounds.set(52, 121, 110, 156);
		break;
	case 1:
		_collisionBounds.set(52, 192, 109, 222);
		break;
	case 2:
		_collisionBounds.set(60, 257, 119, 286);
		break;
	case 3:
		_collisionBounds.set(67, 326, 120, 354);
		break;
	case 4:
		_collisionBounds.set(70, 389, 128, 416);
		break;
	case 5:
		_collisionBounds.set(523, 113, 580, 144);
		break;
	case 6:
		_collisionBounds.set(525, 176, 577, 206);
		break;
	case 7:
		_collisionBounds.set(527, 384, 580, 412);
		break;
	case 8:
		_collisionBounds.set(522, 255, 580, 289);
		break;
	}
	
	setVisible(false);
	SetUpdateHandler(&MainMenuButton::update);
	SetMessageHandler(&MainMenuButton::handleMessage);
}

void MainMenuButton::update() {
	updatePosition();
	if (_countdown != 0 && (--_countdown) == 0) {
		setVisible(false);
		sendMessage(_parentScene, 0x2000, _buttonIndex);
	}
}

uint32 MainMenuButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0) {
			setVisible(true);
			_countdown = 4;
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

MainMenu::MainMenu(NeverhoodEngine *vm, Module *parentModule)
	: Scene(vm, parentModule, true) {
	
	setBackground(0x08C0020C);
	setPalette(0x08C0020C);
	insertMouse433(0x00208084);
	
	insertStaticSprite(0x41137051, 100);
	insertStaticSprite(0xC10B2015, 100);
	
	// TODO Only is music is disabled
	_musicOnButton = insertStaticSprite(0x0C24C0EE, 100);

	for (uint buttonIndex = 0; buttonIndex < 9; ++buttonIndex) {
		Sprite *mainMenuButton = insertSprite<MainMenuButton>(this, buttonIndex);
		_vm->_collisionMan->addSprite(mainMenuButton);
	}
	
	SetUpdateHandler(&Scene::update);	
	SetMessageHandler(&MainMenu::handleMessage);	

}

uint32 MainMenu::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		leaveScene(param.asInteger());
		break;
	}
	return 0;
}

static const uint32 kCreditsSceneFileHashes[] = {
	0x6081128C,
	0x608112BC,
	0x608112DC,
	0x6081121C,
	0x6081139C,
	0x6081109C,
	0x6081169C,
	0x60811A9C,
	0x6081029C,
	0x0081128C,
	0x008112BC,
	0x008012BC,
	0x008112DC,
	0x0081121C,
	0x0081139C,
	0x0081109C,
	0x0081169C,
	0x00811A9C,
	0x0081029C,
	0x0081329C,
	0xC08112BC,
	0xC08112DC,
	0xC081121C,
	0xC081139C,
	0
};

CreditsScene::CreditsScene(NeverhoodEngine *vm, Module *parentModule, bool canAbort)
	: Scene(vm, parentModule, true), _canAbort(canAbort), _screenIndex(0), _ticksDuration(0),
	_countdown(216) {

	SetUpdateHandler(&CreditsScene::update);	
	SetMessageHandler(&CreditsScene::handleMessage);
	
	setBackground(0x6081128C);
	setPalette(0x6081128C);

	_ticksTime = _vm->_system->getMillis() + 202100;
		
	_musicResource = new MusicResource(_vm);
	_musicResource->load(0x30812225);
	_musicResource->play(0);
	
}

CreditsScene::~CreditsScene() {
	_musicResource->unload();
	delete _musicResource;
}

void CreditsScene::update() {
	Scene::update();
	if (_countdown != 0) {
		if (_screenIndex == 23 && _vm->_system->getMillis() > _ticksTime)
			leaveScene(0);
		else if ((--_countdown) == 0) {
			++_screenIndex;
			if (kCreditsSceneFileHashes[_screenIndex] == 0)
				leaveScene(0);
			else {
				_background->load(kCreditsSceneFileHashes[_screenIndex]);
				_palette->addPalette(kCreditsSceneFileHashes[_screenIndex], 0, 256, 0);
				if (_screenIndex < 5)
					_countdown = 192;
				else if (_screenIndex < 15)
					_countdown = 144;
				else if (_screenIndex < 16)
					_countdown = 216;
				else if (_screenIndex < 23)
					_countdown = 144;
				else
					_countdown = 1224;
			}
		}
	}
}

uint32 CreditsScene::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0009:
		leaveScene(0);
		break;
	case 0x000B://TODO Implement this message
		if (param.asInteger() == 27 && _canAbort)
			leaveScene(0);
		break;
	case 0x101D:
		_ticksDuration = _ticksTime - _vm->_system->getMillis();
		break;
	case 0x101E:
		_ticksTime = _ticksDuration + _vm->_system->getMillis();
		break;
	}
	return 0;
}

WidgetScene::WidgetScene(NeverhoodEngine *vm, Module *parentModule)
	: Scene(vm, parentModule, true), _currWidget(NULL) {
}

void WidgetScene::getMousPos(NPoint &pt) {
	pt.x = _mouseCursor->getX();
	pt.y = _mouseCursor->getY();
}

void WidgetScene::setCurrWidget(Widget *newWidget) {
	if (newWidget && newWidget != _currWidget) {
		if (_currWidget)
			_currWidget->exitWidget();
		newWidget->enterWidget();
		_currWidget = newWidget;
	}
}

Widget::Widget(NeverhoodEngine *vm, int16 x, int16 y, int16 itemID, WidgetScene *parentScene,
	int baseObjectPriority, int baseSurfacePriority, bool visible)
	: StaticSprite(vm, baseObjectPriority), _itemID(itemID), _parentScene(parentScene),
	_baseObjectPriority(baseObjectPriority), _baseSurfacePriority(baseSurfacePriority), _visible(visible) {

	SetUpdateHandler(&Widget::update);
	SetMessageHandler(&Widget::handleMessage);
	
	setPosition(x, y);
}

void Widget::show() {
	if (_surface)
		_surface->setVisible(true);
	_visible = true;
}

void Widget::hide() {
	if (_surface)
		_surface->setVisible(false);
	_visible = false;
}

void Widget::onClick() {
	_parentScene->setCurrWidget(this);
	// TODO Somehow _parentScene->onClick(_itemID, 0);
}

void Widget::setPosition(int16 x, int16 y) {
	_x = x;
	_y = y;
	updateBounds();
}

void Widget::refresh() {
	_needRefresh = true;
	StaticSprite::updatePosition();
	_collisionBoundsOffset.set(0, 0,
		_spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	updateBounds();
}

void Widget::addSprite() {
	// Empty
}

int16 Widget::getWidth() {
	return _spriteResource.getDimensions().width;
}

int16 Widget::getHeight() {
	return _spriteResource.getDimensions().height;
}

void Widget::enterWidget() {
	// Empty
}

void Widget::exitWidget() {
	// Empty
}

void Widget::update() {
	handleSpriteUpdate();
	StaticSprite::updatePosition();
}

uint32 Widget::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		onClick();
		messageResult = 1;
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
