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

#include "common/config-manager.h"
#include "common/translation.h"

#include "audio/mixer.h"

#include "gui/saveload.h"

#include "neverhood/menumodule.h"
#include "neverhood/gamemodule.h"

#include "engines/savestate.h"

namespace Neverhood {

enum {
	MAIN_MENU			= 0,
	CREDITS_SCENE		= 1,
	MAKING_OF			= 2,
	LOAD_GAME_MENU		= 3,
	SAVE_GAME_MENU		= 4,
	DELETE_GAME_MENU	= 5,
	QUERY_OVR_MENU		= 6
};

enum {
	kMainMenuRestartGame	= 0,
	kMainMenuLoadGame		= 1,
	kMainMenuSaveGame		= 2,
	kMainMenuResumeGame		= 3,
	kMainMenuQuitGame		= 4,
	kMainMenuCredits		= 5,
	kMainMenuMakingOf		= 6,
	kMainMenuToggleMusic	= 7,
	kMainMenuDeleteGame		= 8
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
	_vm->toggleSoundUpdate(false);

	createScene(MAIN_MENU, -1);
}

MenuModule::~MenuModule() {
	_vm->_mixer->pauseAll(false);
	_vm->toggleSoundUpdate(true);
	_vm->_screen->setPaletteData(_savedPaletteData);
}

void MenuModule::setLoadgameInfo(uint index) {
	_savegameSlot = (*_savegameList)[index].slotNum;
}

void MenuModule::setLoadgameSlot(int slot) {
	_savegameSlot = slot;
}

void MenuModule::setSavegameInfo(const Common::String &description, uint index, bool newSavegame) {
	_savegameDescription = description;
	_savegameSlot = newSavegame ? -1 : (*_savegameList)[index].slotNum;
}

void MenuModule::setDeletegameInfo(uint index) {
	_savegameSlot = (*_savegameList)[index].slotNum;
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
		createSmackerScene(kMakingOfSmackerFileHashList, ConfMan.getBool("scalemakingofvideos"), true, true);
		break;
	case LOAD_GAME_MENU:
		createLoadGameMenu();
		break;
	case SAVE_GAME_MENU:
		createSaveGameMenu();
		break;
	case DELETE_GAME_MENU:
		createDeleteGameMenu();
		break;
	case QUERY_OVR_MENU:
		_childObject = new QueryOverwriteMenu(_vm, this, _savegameDescription);
		break;
	default:
		break;
	}
	SetUpdateHandler(&MenuModule::updateScene);
	_childObject->handleUpdate();
}

void MenuModule::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case MAIN_MENU:
			switch (_moduleResult) {
			case kMainMenuRestartGame:
				_vm->_gameModule->requestRestartGame(false);
				leaveModule(0);
				break;
			case kMainMenuLoadGame:
				createScene(LOAD_GAME_MENU, -1);
				break;
			case kMainMenuSaveGame:
				createScene(SAVE_GAME_MENU, -1);
				break;
			case kMainMenuResumeGame:
				leaveModule(0);
				break;
			case kMainMenuQuitGame:
				_vm->quitGame();
				break;
			case kMainMenuCredits:
				createScene(CREDITS_SCENE, -1);
				break;
			case kMainMenuMakingOf:
				createScene(MAKING_OF, -1);
				break;
			case kMainMenuToggleMusic:
				_vm->toggleMusic(!_vm->musicIsEnabled());
				_vm->_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, !_vm->musicIsEnabled());
				createScene(MAIN_MENU, -1);
				break;
			case kMainMenuDeleteGame:
				createScene(DELETE_GAME_MENU, -1);
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
		case LOAD_GAME_MENU:
			handleLoadGameMenuAction(_moduleResult != 1);
			break;
		case SAVE_GAME_MENU:
			handleSaveGameMenuAction(_moduleResult != 1, true);
			break;
		case DELETE_GAME_MENU:
			handleDeleteGameMenuAction(_moduleResult != 1);
			break;
		case QUERY_OVR_MENU:
			handleSaveGameMenuAction(_moduleResult != 1, false);
			break;
		default:
			break;
		}
	}
}

uint32 MenuModule::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	switch(messageNum) {
	case NM_KEYPRESS_ESC:
		leaveModule(0);
		break;
	default:
		break;
	}

	return Module::handleMessage(messageNum, param, sender);
}

void MenuModule::createLoadGameMenu() {
	refreshSaveGameList();
	_childObject = new LoadGameMenu(_vm, this, _savegameList);
}

void MenuModule::createSaveGameMenu() {
	refreshSaveGameList();
	_childObject = new SaveGameMenu(_vm, this, _savegameList);
}

void MenuModule::createDeleteGameMenu() {
	refreshSaveGameList();
	_childObject = new DeleteGameMenu(_vm, this, _savegameList);
}

void MenuModule::refreshSaveGameList() {
	_savegameSlot = -1;
	delete _savegameList;
	_savegameList = NULL;
	_savegameList = new SavegameList();
	loadSavegameList();
}

void MenuModule::handleLoadGameMenuAction(bool doLoad) {
	createScene(MAIN_MENU, -1);
	if (doLoad && _savegameSlot >= 0) {
		_vm->loadGameState(_savegameSlot);
		leaveModule(0);
	}
	delete _savegameList;
	_savegameList = NULL;
}

void MenuModule::handleSaveGameMenuAction(bool doSave, bool doQuery) {
	if (doSave && doQuery && _savegameSlot >= 0) {
		createScene(QUERY_OVR_MENU, -1);
	} else if (doSave) {
		// Get a new slot number if it's a new savegame
		if (_savegameSlot < 0)
			_savegameSlot = _savegameList->size() > 0 ? _savegameList->back().slotNum + 1 : 0;
		// Restore the scene palette and background so that the correct thumbnail is saved
		byte *menuPaletteData = _vm->_screen->getPaletteData();
		_vm->_screen->setPaletteData(_savedPaletteData);
		_vm->_gameModule->redrawPrevChildObject();
		_vm->saveGameState(_savegameSlot, _savegameDescription);
		_vm->_screen->setPaletteData(menuPaletteData);
		createScene(MAIN_MENU, -1);
	} else {
		createScene(MAIN_MENU, -1);
	}
	delete _savegameList;
	_savegameList = NULL;
}

void MenuModule::handleDeleteGameMenuAction(bool doDelete) {
	createScene(MAIN_MENU, -1);
	if (doDelete && _savegameSlot >= 0) {
		_vm->removeGameState(_savegameSlot);
	}
	delete _savegameList;
	_savegameList = NULL;
}

void MenuModule::loadSavegameList() {

	Common::SaveFileManager *saveFileMan = _vm->_system->getSavefileManager();
	Neverhood::NeverhoodEngine::SaveHeader header;
	Common::String pattern = _vm->getTargetName();
	pattern += ".???";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end());

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); file++) {
		int slotNum = atoi(file->c_str() + file->size() - 3);
		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Neverhood::NeverhoodEngine::readSaveHeader(in, header) == Neverhood::NeverhoodEngine::kRSHENoError) {
					SavegameItem savegameItem;
					savegameItem.slotNum = slotNum;
					savegameItem.description = header.description;
					_savegameList->push_back(savegameItem);
				}
				delete in;
			}
		}
	}

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
	default:
		break;
	}
	return messageResult;
}

MainMenu::MainMenu(NeverhoodEngine *vm, Module *parentModule)
	: Scene(vm, parentModule) {

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
		{  52, 121, 110, 156 },
		{  52, 192, 109, 222 },
		{  60, 257, 119, 286 },
		{  67, 326, 120, 354 },
		{  70, 389, 128, 416 },
		{ 523, 113, 580, 144 },
		{ 525, 176, 577, 206 },
		{ 527, 384, 580, 412 },
		{ 522, 255, 580, 289 }
	};

	setBackground(0x08C0020C);
	setPalette(0x08C0020C);
	insertScreenMouse(0x00208084);

	insertStaticSprite(0x41137051, 100);	// "Options" header text
	insertStaticSprite(0xC10B2015, 100);	// Button texts

	if (!_vm->musicIsEnabled())
		insertStaticSprite(0x0C24C0EE, 100);	// "Music is off" button

	for (uint buttonIndex = 0; buttonIndex < 9; ++buttonIndex) {
		Sprite *menuButton = insertSprite<MenuButton>(this, buttonIndex,
			kMenuButtonFileHashes[buttonIndex], kMenuButtonCollisionBounds[buttonIndex]);
		addCollisionSprite(menuButton);
	}

	SetUpdateHandler(&Scene::update);
	SetMessageHandler(&MainMenu::handleMessage);

}

uint32 MainMenu::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_UPDATE:
		leaveScene(param.asInteger());
		break;
	default:
		break;
	}
	return 0;
}

static const uint32 kCreditsSceneFileHashes[] = {
	0x6081128C, 0x608112BC, 0x608112DC,
	0x6081121C, 0x6081139C, 0x6081109C,
	0x6081169C, 0x60811A9C, 0x6081029C,
	0x0081128C, 0x008112BC, 0x008012BC,
	0x008112DC, 0x0081121C, 0x0081139C,
	0x0081109C, 0x0081169C, 0x00811A9C,
	0x0081029C, 0x0081329C, 0xC08112BC,
	0xC08112DC, 0xC081121C, 0xC081139C,
	0
};

CreditsScene::CreditsScene(NeverhoodEngine *vm, Module *parentModule, bool canAbort)
	: Scene(vm, parentModule), _canAbort(canAbort), _screenIndex(0), _ticksDuration(0),
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
	case NM_KEYPRESS_SPACE:
		leaveScene(0);
		break;
	case 0x000B:
		if (param.asInteger() == Common::KEYCODE_ESCAPE && _canAbort)
			leaveScene(0);
		break;
	case NM_MOUSE_HIDE:
		_ticksDuration = _ticksTime - _vm->_system->getMillis();
		break;
	case NM_MOUSE_SHOW:
		_ticksTime = _ticksDuration + _vm->_system->getMillis();
		break;
	default:
		break;
	}
	return 0;
}

Widget::Widget(NeverhoodEngine *vm, int16 x, int16 y, GameStateMenu *parentScene,
	int baseObjectPriority, int baseSurfacePriority)
	: StaticSprite(vm, baseObjectPriority), _parentScene(parentScene),
	_baseObjectPriority(baseObjectPriority), _baseSurfacePriority(baseSurfacePriority) {

	SetUpdateHandler(&Widget::update);
	SetMessageHandler(&Widget::handleMessage);

	setPosition(x, y);
}

void Widget::onClick() {
	_parentScene->setCurrWidget(this);
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

void Widget::initialize() {
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
	default:
		break;
	}
	return messageResult;
}

TextLabelWidget::TextLabelWidget(NeverhoodEngine *vm, int16 x, int16 y, GameStateMenu *parentScene,
	int baseObjectPriority, int baseSurfacePriority,
	const byte *string, int stringLen, BaseSurface *drawSurface, int16 tx, int16 ty, FontSurface *fontSurface)
	: Widget(vm, x, y, parentScene,	baseObjectPriority, baseSurfacePriority),
	_string(string), _stringLen(stringLen), _drawSurface(drawSurface), _tx(tx), _ty(ty), _fontSurface(fontSurface) {

}

void TextLabelWidget::initialize() {
	_parentScene->addSprite(this);
	_parentScene->addCollisionSprite(this);
}

int16 TextLabelWidget::getWidth() {
	return _fontSurface->getStringWidth(_string, _stringLen);
}

int16 TextLabelWidget::getHeight() {
	return _fontSurface->getCharHeight();
}

void TextLabelWidget::drawString(int maxStringLength) {
	_fontSurface->drawString(_drawSurface, _x, _y, _string, MIN(_stringLen, maxStringLength));
	_collisionBoundsOffset.set(_tx, _ty, getWidth(), getHeight());
	updateBounds();
}

void TextLabelWidget::clear() {
	_collisionBoundsOffset.set(0, 0, 0, 0);
	updateBounds();
}

void TextLabelWidget::setString(const byte *string, int stringLen) {
	_string = string;
	_stringLen = stringLen;
}

TextEditWidget::TextEditWidget(NeverhoodEngine *vm, int16 x, int16 y, GameStateMenu *parentScene,
	int maxStringLength, FontSurface *fontSurface, uint32 fileHash, const NRect &rect)
	: Widget(vm, x, y, parentScene,	1000, 1000),
	_maxStringLength(maxStringLength), _fontSurface(fontSurface), _fileHash(fileHash), _rect(rect),
	_cursorSurface(NULL), _cursorTicks(0), _cursorPos(0), _cursorFileHash(0), _cursorWidth(0), _cursorHeight(0),
	_modified(false), _readOnly(false) {

	_maxVisibleChars = (_rect.x2 - _rect.x1) / _fontSurface->getCharWidth();
	_cursorPos = 0;
	_textLabelWidget = NULL;

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
			int newCursorPos = mousePos.x / _fontSurface->getCharWidth();
			if (mousePos.x % _fontSurface->getCharWidth() > _fontSurface->getCharWidth() / 2 && newCursorPos <= (int)_entryString.size())
				++newCursorPos;
			_cursorPos = MIN((int)_entryString.size(), newCursorPos);
		}
		if (!_readOnly)
			_cursorSurface->setVisible(true);
		refresh();
	}
	Widget::onClick();
}

void TextEditWidget::initialize() {
	SpriteResource cursorSpriteResource(_vm);

	_spriteResource.load(_fileHash, true);
	createSurface(_baseSurfacePriority, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	refreshPosition();
	_parentScene->addSprite(this);
	_parentScene->addCollisionSprite(this);
	_surface->setVisible(true);
	_textLabelWidget = new TextLabelWidget(_vm, _rect.x1, _rect.y1 + (_rect.y2 - _rect.y1 + 1 - _fontSurface->getCharHeight()) / 2,
		_parentScene, _baseObjectPriority + 1, _baseSurfacePriority + 1,
		(const byte*)_entryString.c_str(), _entryString.size(), _surface, _x, _y, _fontSurface);
	_textLabelWidget->initialize();
	if (_cursorFileHash != 0) {
		cursorSpriteResource.load(_cursorFileHash, true);
		_cursorSurface = new BaseSurface(_vm, 0, cursorSpriteResource.getDimensions().width, cursorSpriteResource.getDimensions().height, "cursor");
		_cursorSurface->drawSpriteResourceEx(cursorSpriteResource, false, false, cursorSpriteResource.getDimensions().width, cursorSpriteResource.getDimensions().height);
		_cursorSurface->setVisible(!_readOnly);
	}
	refresh();
}

void TextEditWidget::enterWidget() {
	if (!_readOnly) {
		_cursorSurface->setVisible(true);
		_vm->_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	}
	refresh();
}

void TextEditWidget::exitWidget() {
	if (!_readOnly) {
		_cursorSurface->setVisible(false);
		_vm->_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
	}
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
		_surface->copyFrom(_cursorSurface->getSurface(), _rect.x1 + _cursorPos * _fontSurface->getCharWidth(),
			_rect.y1 + (_rect.y2 - _cursorHeight - _rect.y1 + 1) / 2, sourceRect);
	} else if (!_readOnly)
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
		doRefresh = false;
		break;
	}
	if (doRefresh) {
		_cursorSurface->setVisible(!_readOnly);
		_cursorTicks = 0;
		refresh();
	}
}

void TextEditWidget::refresh() {
	refreshPosition();
	updateString();
	if (_cursorFileHash != 0)
		drawCursor();
}

void TextEditWidget::update() {
	Widget::update();
	if (!_readOnly && _parentScene->getCurrWidget() == this && _cursorTicks++ == 10) {
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
	default:
		break;
	}
	return messageResult;
}

SavegameListBox::SavegameListBox(NeverhoodEngine *vm, int16 x, int16 y, GameStateMenu *parentScene,
	SavegameList *savegameList, FontSurface *fontSurface, uint32 bgFileHash, const NRect &rect)
	: Widget(vm, x, y, parentScene,	1000, 1000),
	_savegameList(savegameList), _fontSurface(fontSurface), _bgFileHash(bgFileHash), _rect(rect),
	_maxStringLength(0), _firstVisibleItem(0), _lastVisibleItem(0), _currIndex(0) {

	_maxVisibleItemsCount = (_rect.y2 - _rect.y1) / _fontSurface->getCharHeight();
	_maxStringLength = (_rect.x2 - _rect.x1) / _fontSurface->getCharWidth();
}

void SavegameListBox::onClick() {
	NPoint mousePos = _parentScene->getMousePos();
	mousePos.x -= _x + _rect.x1;
	mousePos.y -= _y + _rect.y1;
	if (mousePos.x >= 0 && mousePos.x <= _rect.x2 - _rect.x1 &&
		mousePos.y >= 0 && mousePos.y <= _rect.y2 - _rect.y1) {
		int newIndex = _firstVisibleItem + mousePos.y / _fontSurface->getCharHeight();
		if (newIndex <= _lastVisibleItem) {
			_currIndex = newIndex;
			refresh();
			_parentScene->setCurrWidget(this);
			_parentScene->refreshDescriptionEdit();
		}
	}
}

void SavegameListBox::initialize() {
	_spriteResource.load(_bgFileHash, true);
	createSurface(_baseSurfacePriority, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	refreshPosition();
	_parentScene->addSprite(this);
	_parentScene->addCollisionSprite(this);
	_surface->setVisible(true);
	buildItems();
	_firstVisibleItem = 0;
	_lastVisibleItem = MIN(_maxVisibleItemsCount, (int)_textLabelItems.size()) - 1;
	refresh();
}

void SavegameListBox::buildItems() {
	SavegameList &savegameList = *_savegameList;
	int16 itemX = _rect.x1, itemY = 0;
	for (uint i = 0; i < savegameList.size(); ++i) {
		const byte *string = (const byte*)savegameList[i].description.c_str();
		int stringLen = (int)savegameList[i].description.size();
		TextLabelWidget *label = new TextLabelWidget(_vm, itemX, itemY, _parentScene, _baseObjectPriority + 1,
			_baseSurfacePriority + 1, string, MIN(stringLen, _maxStringLength), _surface, _x, _y, _fontSurface);
		label->initialize();
		_textLabelItems.push_back(label);
	}
}

void SavegameListBox::drawItems() {
	for (int i = 0; i < (int)_textLabelItems.size(); ++i) {
		TextLabelWidget *label = _textLabelItems[i];
		if (i >= _firstVisibleItem && i <= _lastVisibleItem) {
			label->setY(_rect.y1 + (i - _firstVisibleItem) * _fontSurface->getCharHeight());
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
	if (_lastVisibleItem < (int)_textLabelItems.size() - 1) {
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
	int amount = MIN((int)_textLabelItems.size() - _lastVisibleItem - 1, _maxVisibleItemsCount);
	if (amount > 0) {
		_firstVisibleItem += amount;
		_lastVisibleItem += amount;
		refresh();
	}
}

int GameStateMenu::scummVMSaveLoadDialog(bool isSave, Common::String &saveDesc) {
	GUI::SaveLoadChooser *dialog;
	Common::String desc;
	int slot;

	if (isSave) {
		dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);

		slot = dialog->runModalWithCurrentTarget();
		desc = dialog->getResultString();

		if (desc.empty())
			desc = dialog->createDefaultSaveDescription(slot);

		if (desc.size() > 29)
			desc = Common::String(desc.c_str(), 29);

		saveDesc = desc;
	} else {
		dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
		slot = dialog->runModalWithCurrentTarget();
	}

	delete dialog;

	return slot;
}

GameStateMenu::GameStateMenu(NeverhoodEngine *vm, Module *parentModule, SavegameList *savegameList,
	const uint32 *buttonFileHashes, const NRect *buttonCollisionBounds,
	uint32 backgroundFileHash, uint32 fontFileHash,
	uint32 mouseFileHash, const NRect *mouseRect,
	uint32 listBoxBackgroundFileHash, int16 listBoxX, int16 listBoxY, const NRect &listBoxRect,
	uint32 textEditBackgroundFileHash, uint32 textEditCursorFileHash, int16 textEditX, int16 textEditY, const NRect &textEditRect,
	uint32 textFileHash1, uint32 textFileHash2)
	: Scene(vm, parentModule), _currWidget(NULL), _savegameList(savegameList) {

	bool isSave = (textEditCursorFileHash != 0);

	_fontSurface = new FontSurface(_vm, fontFileHash, 32, 7, 32, 11, 17);

	if (!ConfMan.getBool("originalsaveload")) {
		Common::String saveDesc;
		int saveCount = savegameList->size();
		int slot = scummVMSaveLoadDialog(isSave, saveDesc);

		if (slot >= 0) {
			if (!isSave) {
				((MenuModule*)_parentModule)->setLoadgameSlot(slot);
			} else {
				((MenuModule*)_parentModule)->setSavegameInfo(saveDesc,
					slot, slot >= saveCount);
			}
			leaveScene(0);
		} else {
			leaveScene(1);
		}
		return;
	}

	setBackground(backgroundFileHash);
	setPalette(backgroundFileHash);
	insertScreenMouse(mouseFileHash, mouseRect);
	insertStaticSprite(textFileHash1, 200);
	insertStaticSprite(textFileHash2, 200);

	_listBox = new SavegameListBox(_vm, listBoxX, listBoxY, this,
		_savegameList, _fontSurface, listBoxBackgroundFileHash, listBoxRect);
	_listBox->initialize();

	_textEditWidget = new TextEditWidget(_vm, textEditX, textEditY, this, 29,
		_fontSurface, textEditBackgroundFileHash, textEditRect);
	if (isSave)
		_textEditWidget->setCursor(textEditCursorFileHash, 2, 13);
	else
		_textEditWidget->setReadOnly(true);
	_textEditWidget->initialize();
	setCurrWidget(_textEditWidget);

	for (uint buttonIndex = 0; buttonIndex < 6; ++buttonIndex) {
		Sprite *menuButton = insertSprite<MenuButton>(this, buttonIndex,
			buttonFileHashes[buttonIndex], buttonCollisionBounds[buttonIndex]);
		addCollisionSprite(menuButton);
	}

	SetUpdateHandler(&Scene::update);
	SetMessageHandler(&GameStateMenu::handleMessage);
}

GameStateMenu::~GameStateMenu() {
	delete _fontSurface;
}

NPoint GameStateMenu::getMousePos() {
	NPoint pt;
	pt.x = _mouseCursor->getX();
	pt.y = _mouseCursor->getY();
	return pt;
}

void GameStateMenu::setCurrWidget(Widget *newWidget) {
	if (newWidget && newWidget != _currWidget) {
		if (_currWidget)
			_currWidget->exitWidget();
		newWidget->enterWidget();
		_currWidget = newWidget;
	}
}

void GameStateMenu::refreshDescriptionEdit() {
	uint currIndex = _listBox->getCurrIndex();
	_textEditWidget->setString((*_savegameList)[currIndex].description);
	setCurrWidget(_textEditWidget);
}

void GameStateMenu::performAction() {
	// Empty
}

uint32 GameStateMenu::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x000A:
		if (!_textEditWidget->isReadOnly()) {
			sendMessage(_textEditWidget, 0x000A, param.asInteger());
			setCurrWidget(_textEditWidget);
		}
		break;
	case 0x000B:
		if (param.asInteger() == Common::KEYCODE_RETURN)
			performAction();
		else if (param.asInteger() == Common::KEYCODE_ESCAPE)
			leaveScene(1);
		else if (!_textEditWidget->isReadOnly()) {
			sendMessage(_textEditWidget, 0x000B, param.asInteger());
			setCurrWidget(_textEditWidget);
		}
		break;
	case NM_ANIMATION_UPDATE:
		// Handle menu button click
		switch (param.asInteger()) {
		case 0:
			performAction();
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
		default:
			break;
		}
		break;
	case NM_MOUSE_WHEELUP:
		_listBox->scrollUp();
		break;
	case NM_MOUSE_WHEELDOWN:
		_listBox->scrollDown();
		break;
	default:
		break;
	}
	return 0;
}

static const uint32 kSaveGameMenuButtonFileHashes[] = {
	0x8359A824, 0x0690E260, 0x0352B050,
	0x1392A223, 0x13802260, 0x0B32B200
};

static const NRect kSaveGameMenuButtonCollisionBounds[] = {
	{ 518, 106, 602, 160 },
	{ 516, 378, 596, 434 },
	{ 394, 108, 458, 206 },
	{ 400, 204, 458, 276 },
	{ 398, 292, 456, 352 },
	{ 396, 352, 460, 444 }
};

static const NRect kSaveGameMenuListBoxRect = { 0, 0, 320, 272 };
static const NRect kSaveGameMenuTextEditRect = { 0, 0, 377, 17 };
static const NRect kSaveGameMenuMouseRect = { 50, 47, 427, 64 };

SaveGameMenu::SaveGameMenu(NeverhoodEngine *vm, Module *parentModule, SavegameList *savegameList)
	: GameStateMenu(vm, parentModule, savegameList, kSaveGameMenuButtonFileHashes, kSaveGameMenuButtonCollisionBounds,
		0x30084E25, 0x2328121A,
		0x84E21308, &kSaveGameMenuMouseRect,
		0x1115A223, 60, 142, kSaveGameMenuListBoxRect,
		0x3510A868, 0x8290AC20, 50, 47, kSaveGameMenuTextEditRect,
		0x1340A5C2, 0x1301A7EA) {

}

void SaveGameMenu::performAction() {
	if (!_textEditWidget->getString().empty()) {
		((MenuModule*)_parentModule)->setSavegameInfo(_textEditWidget->getString(),
			_listBox->getCurrIndex(), _textEditWidget->isModified());
		leaveScene(0);
	}
}

static const uint32 kLoadGameMenuButtonFileHashes[] = {
	0x100B2091, 0x84822B03, 0x20E22087,
	0x04040107, 0x04820122, 0x24423047
};

static const NRect kLoadGameMenuButtonCollisionBounds[] = {
	{  44, 115, 108, 147 },
	{  52, 396, 112, 426 },
	{ 188, 116, 245, 196 },
	{ 189, 209, 239, 269 },
	{ 187, 301, 233, 349 },
	{ 182, 358, 241, 433 }
};

static const NRect kLoadGameMenuListBoxRect = { 0, 0, 320, 272 };
static const NRect kLoadGameMenuTextEditRect = { 0, 0, 320, 17 };

#if 0
// Unlike the original game, the text widget in our load dialog is read-only so
// don't change the mouse cursor to indicate that you can type the name of the
// game to load.
//
// Since we allow multiple saved games to have the same name, it's probably
// better this way.
static const NRect kLoadGameMenuMouseRect = { 263, 48, 583, 65 };
#endif

LoadGameMenu::LoadGameMenu(NeverhoodEngine *vm, Module *parentModule, SavegameList *savegameList)
	: GameStateMenu(vm, parentModule, savegameList, kLoadGameMenuButtonFileHashes, kLoadGameMenuButtonCollisionBounds,
		0x98620234, 0x201C2474,
		0x2023098E, NULL /* &kLoadGameMenuMouseRect */,
		0x04040409, 263, 142, kLoadGameMenuListBoxRect,
		0x10924C03, 0, 263, 48, kLoadGameMenuTextEditRect,
		0x0BC600A3, 0x0F960021) {

}

void LoadGameMenu::performAction() {
	// TODO: The original would display a message here if nothing was selected.
	if (!_textEditWidget->getString().empty()) {
		((MenuModule*)_parentModule)->setLoadgameInfo(_listBox->getCurrIndex());
		leaveScene(0);
	}
}

static const uint32 kDeleteGameMenuButtonFileHashes[] = {
	0x8198E268, 0xDD0C4620, 0x81296520,
	0x8D284211, 0x8C004621, 0x07294020
};

static const NRect kDeleteGameMenuButtonCollisionBounds[] = {
	{ 518,  46, 595,  91 },
	{ 524, 322, 599, 369 },
	{ 395,  40, 462, 127 },
	{ 405, 126, 460, 185 },
	{ 397, 205, 456, 273 },
	{ 395, 278, 452, 372 }
};

static const NRect kDeleteGameMenuListBoxRect = { 0, 0, 320, 272 };
static const NRect kDeleteGameMenuTextEditRect = { 0, 0, 320, 17 };

DeleteGameMenu::DeleteGameMenu(NeverhoodEngine *vm, Module *parentModule, SavegameList *savegameList)
	: GameStateMenu(vm, parentModule, savegameList, kDeleteGameMenuButtonFileHashes, kDeleteGameMenuButtonCollisionBounds,
		0x4080E01C, 0x728523ED,
		0x0E018400, NULL,
		0xA5584211, 61, 64, kDeleteGameMenuListBoxRect,
		0x250A3060, 0, 49, 414, kDeleteGameMenuTextEditRect,
		0x80083C01, 0x84181E81) {

}

void DeleteGameMenu::performAction() {
	// TODO: The original would display a message here if no game was selected.
	if (!_textEditWidget->getString().empty()) {
		((MenuModule*)_parentModule)->setDeletegameInfo(_listBox->getCurrIndex());
		leaveScene(0);
	}
}

QueryOverwriteMenu::QueryOverwriteMenu(NeverhoodEngine *vm, Module *parentModule, const Common::String &description)
	: Scene(vm, parentModule) {

	static const uint32 kQueryOverwriteMenuButtonFileHashes[] = {
		0x90312400,
		0x94C22A22
	};

	static const NRect kQueryOverwriteMenuCollisionBounds[] = {
		{ 145, 334, 260, 385 },
		{ 365, 340, 477, 388 }
	};

	setBackground(0x043692C4);
	setPalette(0x043692C4);
	insertScreenMouse(0x692C004B);
	insertStaticSprite(0x08C0AC24, 200);

	for (uint buttonIndex = 0; buttonIndex < 2; ++buttonIndex) {
		Sprite *menuButton = insertSprite<MenuButton>(this, buttonIndex,
			kQueryOverwriteMenuButtonFileHashes[buttonIndex], kQueryOverwriteMenuCollisionBounds[buttonIndex]);
		addCollisionSprite(menuButton);
	}

	// Draw the query text to the background, each text line is centered
	// NOTE The original had this text in its own class
	FontSurface *fontSurface = new FontSurface(_vm, 0x94188D4D, 32, 7, 32, 11, 17);
	Common::StringArray textLines;
	textLines.push_back(description);
	textLines.push_back("Game exists.");
	textLines.push_back("Overwrite it?");
	for (uint i = 0; i < textLines.size(); ++i)
		fontSurface->drawString(_background->getSurface(), 106 + (423 - textLines[i].size() * 11) / 2,
			127 + 31 + i * 17, (const byte*)textLines[i].c_str());
	delete fontSurface;

	SetUpdateHandler(&Scene::update);
	SetMessageHandler(&QueryOverwriteMenu::handleMessage);
}

uint32 QueryOverwriteMenu::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_UPDATE:
		// Handle menu button click
		leaveScene(param.asInteger());
		break;
	default:
		break;
	}
	return 0;
}

} // End of namespace Neverhood
