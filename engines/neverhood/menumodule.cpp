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
	MAKING_OF		= 2,
	SAVE_GAME_MENU	= 3
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
	: Module(vm, parentModule), _savegameList(NULL) {
	
	SetMessageHandler(&MenuModule::handleMessage);
	
	_savedPaletteData = _vm->_screen->getPaletteData();
	_vm->_mixer->pauseAll(true);

	createScene(MAIN_MENU, -1);
}

MenuModule::~MenuModule() {
	_vm->_mixer->pauseAll(false);
	_vm->_screen->setPaletteData(_savedPaletteData);
}

void MenuModule::setSavegameInfo(const Common::String &description, uint slot, bool newSavegame) {
	_savegameDescription = description;
	_savegameSlot = slot;
	_newSavegame = newSavegame;
	debug("SAVEGAME: description = [%s]; slot = %d; new = %d", description.c_str(), slot, newSavegame);
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
	case SAVE_GAME_MENU:
		createSaveGameMenu();
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
				createScene(SAVE_GAME_MENU, -1);
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
		case SAVE_GAME_MENU:
			handleSaveGameMenuAction(_moduleResult);
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

void MenuModule::createSaveGameMenu() {
	// TODO Load actual savegames list :)
	_savegameList = new StringArray();
	_savegameList->push_back(Common::String("Annoying scene"));
	_savegameList->push_back(Common::String("Stuff happens"));
	for (uint i = 0; i < 33; ++i)
		_savegameList->push_back(Common::String::format("Game %d", i));
	_childObject = new SaveGameMenu(_vm, this, _savegameList);
}

void MenuModule::handleSaveGameMenuAction(int action) {
	if (action != 0) {
		createScene(MAIN_MENU, -1);
	} else {
		// TODO Actual saving later 0048A62E
		createScene(MAIN_MENU, -1);
	}
	delete _savegameList;
	_savegameList = NULL;
}

MenuButton::MenuButton(NeverhoodEngine *vm, Scene *parentScene, uint buttonIndex, uint32 fileHash, const NRect &collisionBounds)
	: StaticSprite(vm, 900), _parentScene(parentScene), _buttonIndex(buttonIndex), _countdown(0) {

	loadSprite(fileHash, kSLFDefDrawOffset | kSLFDefPosition, 100);
	_collisionBounds = collisionBounds;
	setVisible(false);
	SetUpdateHandler(&MenuButton::update);
	SetMessageHandler(&MenuButton::handleMessage);
}

void MenuButton::update() {
	updatePosition();
	if (_countdown != 0 && (--_countdown) == 0) {
		setVisible(false);
		sendMessage(_parentScene, 0x2000, _buttonIndex);
	}
}

uint32 MenuButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
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
	
	static const uint32 kMenuButtonFileHashes[] = {
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
	
	static const NRect kMenuButtonCollisionBounds[] = {
		NRect(52, 121, 110, 156),
		NRect(52, 192, 109, 222),
		NRect(60, 257, 119, 286),
		NRect(67, 326, 120, 354),
		NRect(70, 389, 128, 416),
		NRect(523, 113, 580, 144),
		NRect(525, 176, 577, 206),
		NRect(527, 384, 580, 412),
		NRect(522, 255, 580, 289)
	};
	
	setBackground(0x08C0020C);
	setPalette(0x08C0020C);
	insertMouse433(0x00208084);
	
	insertStaticSprite(0x41137051, 100);
	insertStaticSprite(0xC10B2015, 100);
	
	// TODO Only if music is enabled
	_musicOnButton = insertStaticSprite(0x0C24C0EE, 100);

	for (uint buttonIndex = 0; buttonIndex < 9; ++buttonIndex) {
		Sprite *menuButton = insertSprite<MenuButton>(this, buttonIndex,
			kMenuButtonFileHashes[buttonIndex], kMenuButtonCollisionBounds[buttonIndex]);
		_vm->_collisionMan->addSprite(menuButton);
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
		if (param.asInteger() == Common::KEYCODE_ESCAPE && _canAbort)
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

NPoint WidgetScene::getMousePos() {
	NPoint pt;
	pt.x = _mouseCursor->getX();
	pt.y = _mouseCursor->getY();
	return pt;
}

void WidgetScene::setCurrWidget(Widget *newWidget) {
	if (newWidget && newWidget != _currWidget) {
		if (_currWidget)
			_currWidget->exitWidget();
		newWidget->enterWidget();
		_currWidget = newWidget;
	}
}

void WidgetScene::handleEvent(int16 itemID, int eventType) {
}

Widget::Widget(NeverhoodEngine *vm, int16 x, int16 y, int16 itemID, WidgetScene *parentScene,
	int baseObjectPriority, int baseSurfacePriority)
	: StaticSprite(vm, baseObjectPriority), _itemID(itemID), _parentScene(parentScene),
	_baseObjectPriority(baseObjectPriority), _baseSurfacePriority(baseSurfacePriority) {

	SetUpdateHandler(&Widget::update);
	SetMessageHandler(&Widget::handleMessage);
	
	setPosition(x, y);
}

void Widget::onClick() {
	_parentScene->setCurrWidget(this);
	// TODO _parentScene->onClick(_itemID, 0);
}

void Widget::setPosition(int16 x, int16 y) {
	_x = x;
	_y = y;
	updateBounds();
}

void Widget::refreshPosition() {
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

TextLabelWidget::TextLabelWidget(NeverhoodEngine *vm, int16 x, int16 y, int16 itemID, WidgetScene *parentScene,
	int baseObjectPriority, int baseSurfacePriority,
	const byte *string, int stringLen, BaseSurface *drawSurface, int16 tx, int16 ty, TextSurface *textSurface)
	: Widget(vm, x, y, itemID, parentScene,	baseObjectPriority, baseSurfacePriority),
	_string(string), _stringLen(stringLen), _drawSurface(drawSurface), _tx(tx), _ty(ty), _textSurface(textSurface) {
	
}

void TextLabelWidget::addSprite() {
	_parentScene->addSprite(this);
	_vm->_collisionMan->addSprite(this);
}

int16 TextLabelWidget::getWidth() {
	return _textSurface->getStringWidth(_string, _stringLen);
}

int16 TextLabelWidget::getHeight() {
	return _textSurface->getCharHeight();
}

void TextLabelWidget::drawString(int maxStringLength) {
	_textSurface->drawString(_drawSurface, _x, _y, _string, MIN(_stringLen, maxStringLength));
	_collisionBoundsOffset.set(_tx, _ty, getWidth(), getHeight());
	updateBounds();
}

void TextLabelWidget::clear() {
	_collisionBoundsOffset.set(0, 0, 0, 0);
	updateBounds();
}

void TextLabelWidget::onClick() {
	Widget::onClick();
	// TODO Click handler?
}

void TextLabelWidget::setString(const byte *string, int stringLen) {
	_string = string;
	_stringLen = stringLen;
}

TextEditWidget::TextEditWidget(NeverhoodEngine *vm, int16 x, int16 y, int16 itemID, WidgetScene *parentScene,
	int baseObjectPriority, int baseSurfacePriority, int maxStringLength, TextSurface *textSurface,
	uint32 fileHash, const NRect &rect)
	: Widget(vm, x, y, itemID, parentScene,	baseObjectPriority, baseSurfacePriority),
	_maxStringLength(maxStringLength), _textSurface(textSurface), _fileHash(fileHash), _rect(rect),
	_cursorSurface(NULL), _cursorTicks(0), _cursorPos(0), _cursorFileHash(0), _cursorWidth(0), _cursorHeight(0),
	_modified(false) {

	_maxVisibleChars = (_rect.x2 - _rect.x1) / _textSurface->getCharWidth();
	_cursorPos = 0;
	
	SetUpdateHandler(&TextEditWidget::update);
	SetMessageHandler(&TextEditWidget::handleMessage);
}

TextEditWidget::~TextEditWidget() {
	delete _cursorSurface;
}

void TextEditWidget::onClick() {
	NPoint mousePos = _parentScene->getMousePos();
	mousePos.x -= _x + _rect.x1;
	mousePos.y -= _y + _rect.y1;
	if (mousePos.x >= 0 && mousePos.x <= _rect.x2 - _rect.x1 &&
		mousePos.y >= 0 && mousePos.y <= _rect.y2 - _rect.y1) {
		if (_entryString.size() == 1)
			_cursorPos = 0;
		else {
			int newCursorPos = mousePos.x / _textSurface->getCharWidth();
			if (mousePos.x % _textSurface->getCharWidth() > _textSurface->getCharWidth() / 2 && newCursorPos <= (int)_entryString.size())//###
				++newCursorPos;
			_cursorPos = MIN((int)_entryString.size(), newCursorPos);
		}
		_cursorSurface->setVisible(true);
		refresh();
	}
	Widget::onClick();
}

void TextEditWidget::addSprite() {
	SpriteResource cursorSpriteResource(_vm);

	_spriteResource.load2(_fileHash);
	createSurface(_baseSurfacePriority, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	refreshPosition();
	_parentScene->addSprite(this);
	_vm->_collisionMan->addSprite(this);
	_surface->setVisible(true);
	_textLabelWidget = new TextLabelWidget(_vm, _rect.x1, _rect.y1 + (_rect.y2 - _rect.y1 + 1 - _textSurface->getCharHeight()) / 2,
		0, _parentScene, _baseObjectPriority + 1, _baseSurfacePriority + 1,
		(const byte*)_entryString.c_str(), _entryString.size(), _surface, _x, _y, _textSurface);
	_textLabelWidget->addSprite();
	cursorSpriteResource.load2(_cursorFileHash);
	_cursorSurface = new BaseSurface(_vm, 0, cursorSpriteResource.getDimensions().width, cursorSpriteResource.getDimensions().height);
	_cursorSurface->drawSpriteResourceEx(cursorSpriteResource, false, false, cursorSpriteResource.getDimensions().width, cursorSpriteResource.getDimensions().height);
	_cursorSurface->setVisible(true);
	refresh();
}

void TextEditWidget::enterWidget() {
	_cursorSurface->setVisible(true);
	refresh();
}

void TextEditWidget::exitWidget() {
	_cursorSurface->setVisible(false);
	refresh();
}

void TextEditWidget::setCursor(uint32 cursorFileHash, int16 cursorWidth, int16 cursorHeight) {
	_cursorFileHash = cursorFileHash;
	_cursorWidth = cursorWidth;
	_cursorHeight = cursorHeight;
}

void TextEditWidget::drawCursor() {
	if (_cursorSurface->getVisible() && _cursorPos >= 0 && _cursorPos <= _maxVisibleChars) {
		NDrawRect sourceRect(0, 0, _cursorWidth, _cursorHeight);
		_surface->copyFrom(_cursorSurface->getSurface(), _rect.x1 + _cursorPos * _textSurface->getCharWidth(),
			_rect.y1 + (_rect.y2 - _cursorHeight - _rect.y1 + 1) / 2, sourceRect, true);
	} else
		_cursorSurface->setVisible(false);
}

void TextEditWidget::updateString() {
	_textLabelWidget->setString((const byte *)_entryString.c_str(), _entryString.size());
	_textLabelWidget->drawString(_maxVisibleChars);
}

Common::String& TextEditWidget::getString() {
	return _entryString;
}

void TextEditWidget::setString(const Common::String &string) {
	_entryString = string;
	_cursorPos = _entryString.size();
	_modified = false;
	refresh();
}

void TextEditWidget::handleAsciiKey(char ch) {
	if ((int)_entryString.size() < _maxStringLength &&
		((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == ' ')) {
		_entryString.insertChar(ch, _cursorPos);
		++_cursorPos;
		_modified = true;
		refresh();
	}
}

void TextEditWidget::handleKeyDown(Common::KeyCode keyCode) {
	bool doRefresh = true;
	switch (keyCode) {
	case Common::KEYCODE_HOME:
		_cursorPos = 0;
		break;
	case Common::KEYCODE_END:
		_cursorPos = _entryString.size();
		break;
	case Common::KEYCODE_LEFT:
		if (_entryString.size() > 0 && _cursorPos > 0)
			--_cursorPos;
		break;
	case Common::KEYCODE_RIGHT:
		if (_cursorPos < (int)_entryString.size())
			++_cursorPos;
		break;
	case Common::KEYCODE_DELETE:
		if (_entryString.size() > 0 && _cursorPos < (int)_entryString.size()) {
			_entryString.deleteChar(_cursorPos);
			_modified = true;
		}
		break;
	case Common::KEYCODE_BACKSPACE:
		if (_entryString.size() > 0 && _cursorPos > 0) {
			_entryString.deleteChar(--_cursorPos);
			_modified = true;
		}
		break;
	default:
		break;
	}
	if (doRefresh) {
		_cursorSurface->setVisible(true);
		_cursorTicks = 0;
		refresh();
	}
}

void TextEditWidget::refresh() {
	refreshPosition();
	updateString();
	drawCursor();
}

void TextEditWidget::update() {
	Widget::update();
	if (_parentScene->getCurrWidget() == this && _cursorTicks++ == 10) {
		_cursorSurface->setVisible(!_cursorSurface->getVisible());
		refresh();
		_cursorTicks = 0;
	}
}

uint32 TextEditWidget::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Widget::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x000A:
		handleAsciiKey(param.asInteger());
		break;
	case 0x000B:
		handleKeyDown((Common::KeyCode)param.asInteger());
		break;
	}
	return messageResult;
}

SavegameListBox::SavegameListBox(NeverhoodEngine *vm, int16 x, int16 y, int16 itemID, WidgetScene *parentScene,
	int baseObjectPriority, int baseSurfacePriority,
	StringArray *savegameList, TextSurface *textSurface, uint32 bgFileHash, const NRect &rect)
	: Widget(vm, x, y, itemID, parentScene,	baseObjectPriority, baseSurfacePriority),
	_savegameList(savegameList), _textSurface(textSurface), _bgFileHash(bgFileHash), _rect(rect),
	_maxStringLength(0), _firstVisibleItem(0), _lastVisibleItem(0), _currIndex(0) {

	_maxVisibleItemsCount = (_rect.y2 - _rect.y1) / _textSurface->getCharHeight();
	_maxStringLength = (_rect.x2 - _rect.x1) / _textSurface->getCharWidth();
}

void SavegameListBox::onClick() {
	NPoint mousePos = _parentScene->getMousePos();
	mousePos.x -= _x + _rect.x1;
	mousePos.y -= _y + _rect.y1;
	if (mousePos.x >= 0 && mousePos.x <= _rect.x2 - _rect.x1 &&
		mousePos.y >= 0 && mousePos.y <= _rect.y2 - _rect.y1) {
		int newIndex = _firstVisibleItem + mousePos.y / _textSurface->getCharHeight();
		if (newIndex <= _lastVisibleItem) {
			_currIndex = newIndex;
			refresh();
			_parentScene->setCurrWidget(this);
			debug("_currIndex = %d", _currIndex);
			_parentScene->handleEvent(_itemID, 5);
		}
	}
}

void SavegameListBox::addSprite() {
	_spriteResource.load2(_bgFileHash);
	createSurface(_baseSurfacePriority, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	refreshPosition();
	_parentScene->addSprite(this);
	_vm->_collisionMan->addSprite(this);
	_surface->setVisible(true);
	buildItems();
	_firstVisibleItem = 0;
	_lastVisibleItem = MIN(_maxVisibleItemsCount, (int)_textLabelItems.size());
	refresh();
}

void SavegameListBox::buildItems() {
	StringArray &savegameList = *_savegameList;
	int16 itemX = _rect.x1, itemY = 0;
	for (uint i = 0; i < savegameList.size(); ++i) {
		const byte *string = (const byte*)savegameList[i].c_str();
		int stringLen = (int)savegameList[i].size();
		TextLabelWidget *label = new TextLabelWidget(_vm, itemX, itemY, i, _parentScene, _baseObjectPriority + 1,
			_baseSurfacePriority + 1, string, MIN(stringLen, _maxStringLength), _surface, _x, _y, _textSurface);
		label->addSprite();
		_textLabelItems.push_back(label);
	}
}

void SavegameListBox::drawItems() {
	for (int i = 0; i < (int)_textLabelItems.size(); ++i) {
		TextLabelWidget *label = _textLabelItems[i];		
		if (i >= _firstVisibleItem && i < _lastVisibleItem) {
			label->setY(_rect.y1 + (i - _firstVisibleItem) * _textSurface->getCharHeight());
			label->updateBounds();
			label->drawString(_maxStringLength);
		} else
			label->clear();
	}
}

void SavegameListBox::refresh() {
	refreshPosition();
	drawItems();
}

void SavegameListBox::scrollUp() {
	if (_firstVisibleItem > 0) {
		--_firstVisibleItem;
		--_lastVisibleItem;
		refresh();
	}
}

void SavegameListBox::scrollDown() {
	if (_lastVisibleItem < (int)_textLabelItems.size()) {
		++_firstVisibleItem;
		++_lastVisibleItem;
		refresh();
	}
}

void SavegameListBox::pageUp() {
	int amount = MIN(_firstVisibleItem, _maxVisibleItemsCount);
	if (amount > 0) {
		_firstVisibleItem -= amount;
		_lastVisibleItem -= amount;
		refresh();
	}
}

void SavegameListBox::pageDown() {
	int amount = MIN((int)_textLabelItems.size() - _lastVisibleItem, _maxVisibleItemsCount);
	if (amount > 0) {
		_firstVisibleItem += amount;
		_lastVisibleItem += amount;
		refresh();
	}
}

SaveGameMenu::SaveGameMenu(NeverhoodEngine *vm, Module *parentModule, StringArray *savegameList)
	: WidgetScene(vm, parentModule), _savegameList(savegameList) {

	static const uint32 kSaveGameMenuButtonFileHashes[] = {
		0x8359A824,
		0x0690E260,
		0x0352B050,
		0x1392A223,
		0x13802260,
		0x0B32B200
	};
	
	static const NRect kSaveGameMenuButtonCollisionBounds[] = {
		NRect(518, 106, 602, 160),
		NRect(516, 378, 596, 434),
		NRect(394, 108, 458, 206),
		NRect(400, 204, 458, 276),
		NRect(398, 292, 456, 352),
		NRect(396, 352, 460, 444)
	};

	static const NRect kListBoxRect(0, 0, 320, 272);
	static const NRect kTextEditRect(0, 0, 377, 17);
	static const NRect kMouseRect(50, 47, 427, 64);

	_textSurface = new TextSurface(_vm, 0x2328121A, 7, 32, 32, 11, 17);
	
	setBackground(0x30084E25);
	setPalette(0x30084E25);
	insertMouse433(0x84E21308, &kMouseRect);
	insertStaticSprite(0x1340A5C2, 200);
	insertStaticSprite(0x1301A7EA, 200);

	_listBox = new SavegameListBox(_vm, 60, 142, 69/*ItemID*/, this, 1000, 1000,
		_savegameList, _textSurface, 0x1115A223, kListBoxRect);
	_listBox->addSprite();

	_textEditWidget = new TextEditWidget(_vm, 50, 47, 70/*ItemID*/, this, 1000, 1000, 29,
		_textSurface, 0x3510A868, kTextEditRect);
	_textEditWidget->setCursor(0x8290AC20, 2, 13);
	_textEditWidget->addSprite();
	setCurrWidget(_textEditWidget);
	
	for (uint buttonIndex = 0; buttonIndex < 6; ++buttonIndex) {
		Sprite *menuButton = insertSprite<MenuButton>(this, buttonIndex,
			kSaveGameMenuButtonFileHashes[buttonIndex], kSaveGameMenuButtonCollisionBounds[buttonIndex]);
		_vm->_collisionMan->addSprite(menuButton);
	}


	SetUpdateHandler(&Scene::update);
	SetMessageHandler(&SaveGameMenu::handleMessage);
}

SaveGameMenu::~SaveGameMenu() {
	delete _textSurface;
}

void SaveGameMenu::handleEvent(int16 itemID, int eventType) {
	if (itemID == 69 && eventType == 5) {
		uint currIndex = _listBox->getCurrIndex();
		_textEditWidget->setString((*_savegameList)[currIndex]);
		setCurrWidget(_textEditWidget);
	}
}

uint32 SaveGameMenu::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x000A:
		sendMessage(_textEditWidget, 0x000A, param.asInteger());
		setCurrWidget(_textEditWidget);
		break;
	case 0x000B:
		if (param.asInteger() == Common::KEYCODE_RETURN) {
			((MenuModule*)_parentModule)->setSavegameInfo(_textEditWidget->getString(),
				_listBox->getCurrIndex(), _textEditWidget->isModified());
			leaveScene(0);
		} else if (param.asInteger() == Common::KEYCODE_ESCAPE) {
			leaveScene(1);
		} else {
			sendMessage(_textEditWidget, 0x000B, param.asInteger());
			setCurrWidget(_textEditWidget);
		}
		break;
	case 0x2000:
		// Handle menu button click
		switch (param.asInteger()) {
		case 0:
			// TODO Same handling as Return, merge
			((MenuModule*)_parentModule)->setSavegameInfo(_textEditWidget->getString(),
				_listBox->getCurrIndex(), _textEditWidget->isModified());
			leaveScene(0);
			break;
		case 1:
			leaveScene(1);
			break;
		case 2:
			_listBox->pageUp();
			break;
		case 3:
			_listBox->scrollUp();
			break;
		case 4:
			_listBox->scrollDown();
			break;
		case 5:
			_listBox->pageDown();
			break;
		}
		break;
	}
	return 0;
}

} // End of namespace Neverhood
