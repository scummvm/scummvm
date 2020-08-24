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

#include "neverhood/modules/module1000.h"
#include "neverhood/modules/module1000_sprites.h"

namespace Neverhood {

Module1000::Module1000(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	_musicFileHash = getGlobalVar(V_ENTRANCE_OPEN) ? 0x81106480 : 0x00103144;

	_vm->_soundMan->addMusic(0x03294419, 0x061880C6);
	_vm->_soundMan->addMusic(0x03294419, _musicFileHash);

	if (which < 0)
		createScene(_vm->gameState().sceneNum, -1);
	else if (which == 0)
		createScene(0, 0);
	else if (which == 1)
		createScene(1, 1);

}

Module1000::~Module1000() {
	_vm->_soundMan->deleteMusicGroup(0x03294419);
}

void Module1000::createScene(int sceneNum, int which) {
	debug(1, "Module1000::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		_vm->_soundMan->startMusic(0x061880C6, 0, 0);
		_childObject = new Scene1001(_vm, this, which);
		break;
	case 1:
		_vm->gameState().sceneNum = 1;
		_vm->_soundMan->startMusic(0x061880C6, 0, 0);
		_childObject = new Scene1002(_vm, this, which);
		break;
	case 2:
		_vm->gameState().sceneNum = 2;
		_vm->_soundMan->startMusic(0x061880C6, 0, 0);
		createStaticScene(0xC084110C, 0x41108C00);
		break;
	case 3:
		_vm->gameState().sceneNum = 3;
		_vm->_soundMan->stopMusic(0x061880C6, 0, 2);
		_childObject = new Scene1004(_vm, this, which);
		break;
	case 4:
		_vm->gameState().sceneNum = 4;
		_vm->_soundMan->stopMusic(0x061880C6, 0, 0);
		_vm->_soundMan->startMusic(_musicFileHash, 0, 0);
		_childObject = new Scene1005(_vm, this, which);
		break;
	default:
		break;
	}
	SetUpdateHandler(&Module1000::updateScene);
	_childObject->handleUpdate();
}

void Module1000::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 0:
			if (_moduleResult == 2)
				createScene(2, 0);
			else
				createScene(1, 0);
			break;
		case 1:
			if (_moduleResult == 1)
				leaveModule(0);
			else if (_moduleResult == 2) {
				if (_vm->isDemo() && !_vm->isBigDemo())
					// Demo version returns to the same scene
					createScene(1, 2);
				else
					createScene(3, 0);
			} else
				createScene(0, 1);
			break;
		case 2:
			createScene(0, 2);
			break;
		case 3:
			if (_moduleResult == 1)
				createScene(4, 0);
			else
				createScene(1, 2);
			break;
		case 4:
			_vm->_soundMan->stopMusic(_musicFileHash, 0, 1);
			createScene(3, 1);
			break;
		default:
			break;
		}
	}
}

Scene1001::Scene1001(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _asDoor(NULL), _asWindow(NULL) {

	Sprite *tempSprite;

	SetMessageHandler(&Scene1001::handleMessage);

	setHitRects(0x004B4860);
	setBackground(0x4086520E);
	setPalette(0x4086520E);
	insertScreenMouse(0x6520A400);

	if (which < 0) {
		// Restoring game
		setRectList(0x004B49F0);
		insertKlaymen<KmScene1001>(200, 433);
		setMessageList(0x004B4888);
	} else if (which == 1) {
		// Klaymen entering from the right
		setRectList(0x004B49F0);
		insertKlaymen<KmScene1001>(640, 433);
		setMessageList(0x004B4898);
	} else if (which == 2) {
		// Klaymen returning from looking through the window
		setRectList(0x004B49F0);
		if (getGlobalVar(V_KLAYMEN_IS_DELTA_X)) {
			insertKlaymen<KmScene1001>(390, 433);
			_klaymen->setDoDeltaX(1);
		} else {
			insertKlaymen<KmScene1001>(300, 433);
		}
		setMessageList(0x004B4970);
	} else {
		// Klaymen sleeping
		setRectList(0x004B4A00);
		insertKlaymen<KmScene1001>(200, 433);
		setMessageList(0x004B4890);
	}

	tempSprite = insertStaticSprite(0x2080A3A8, 1300);

	_klaymen->setClipRect(0, 0, tempSprite->getDrawRect().x2(), 480);

	if (!getGlobalVar(V_DOOR_BUSTED)) {
		_asDoor = insertSprite<AsScene1001Door>();
		_asDoor->setClipRect(0, 0, tempSprite->getDrawRect().x2(), 480);
	}

	_asLever = insertSprite<AsScene1001Lever>(this, 150, 433, 1);

	insertStaticSprite(0x809861A6, 950);
	insertStaticSprite(0x89C03848, 1100);

	_ssButton = insertSprite<SsCommonButtonSprite>(this, 0x15288120, 100, 0);

	if (!getGlobalVar(V_WINDOW_OPEN)) {
		tempSprite = insertStaticSprite(0x8C066150, 200);
		_asWindow = insertSprite<AsScene1001Window>();
		_asWindow->setClipRect(tempSprite->getDrawRect());
	}

	_asHammer = insertSprite<AsScene1001Hammer>(_asDoor);

}

Scene1001::~Scene1001() {
	setGlobalVar(V_KLAYMEN_IS_DELTA_X, _klaymen->isDoDeltaX());
}

uint32 Scene1001::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x00342624) {
			sendEntityMessage(_klaymen, 0x1014, _asLever);
			setMessageList2(0x004B4910);
			messageResult = 1;
		} else if (param.asInteger() == 0x21E64A00) {
			if (getGlobalVar(V_DOOR_BUSTED)) {
				setMessageList(0x004B48A8);
			} else {
				setMessageList(0x004B48C8);
			}
			messageResult = 1;
		} else if (param.asInteger() == 0x040424D0) {
			sendEntityMessage(_klaymen, 0x1014, _ssButton);
		} else if (param.asInteger() == 0x80006358) {
			if (getGlobalVar(V_WINDOW_OPEN)) {
				setMessageList(0x004B4938);
			} else {
				setMessageList(0x004B4960);
			}
		}
		break;
	case NM_POSITION_CHANGE:
		setRectList(0x004B49F0);
		break;
	case 0x480B:
		sendMessage(_asWindow, 0x2001, 0);
		break;
	case NM_KLAYMEN_LOWER_LEVER:
		sendMessage(_asHammer, 0x2000, 0);
		break;
	default:
		break;
	}
	return messageResult;
}

Scene1002::Scene1002(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _isKlaymenFloor(false), _isClimbingLadder(false), _asKlaymenPeekHand(nullptr) {

	NRect tempClipRect;
	Sprite *tempSprite;

	SetUpdateHandler(&Scene1002::update);
	SetMessageHandler(&Scene1002::handleMessage);

	setHitRects(0x004B4138);
	setBackground(0x12C23307);
	setPalette(0x12C23307);

	insertStaticSprite(0x06149428, 1100);
	insertStaticSprite(0x312C8774, 1100);

	_ssLadderArch = insertStaticSprite(0x152C1313, 1015);
	_ssLadderArchPart1 = insertStaticSprite(0x060000A0, 1200);
	_ssLadderArchPart2 = insertStaticSprite(0xB2A423B0, 1100);
	_ssLadderArchPart3 = insertStaticSprite(0x316E0772, 1100);

	_ssCeiling = insertStaticSprite(0x316C4BB4, 1015);

	if (which < 0) {
		// Restoring game
		if (_vm->_gameState.which == 0) {
			// Klaymen on top
			insertKlaymen<KmScene1002>(90, 226);
			_asKlaymenLadderHands = insertSprite<AsScene1002KlaymenLadderHands>(_klaymen);
			setMessageList(0x004B4270);
			_klaymen->setClipRect(31, 0, _ssLadderArchPart2->getDrawRect().x2(), _ssLadderArchPart3->getDrawRect().y2());
			_asKlaymenLadderHands->getSurface()->getClipRect() = _klaymen->getSurface()->getClipRect();
			_klaymen->setRepl(64, 0);
		} else {
			// Klaymen on the floor
			insertKlaymen<KmScene1002>(379, 435);
			_asKlaymenLadderHands = insertSprite<AsScene1002KlaymenLadderHands>(_klaymen);
			setMessageList(0x004B4270);
			_klaymen->setClipRect(_ssLadderArch->getDrawRect().x, 0, _ssLadderArchPart2->getDrawRect().x2(), _ssLadderArchPart1->getDrawRect().y2());
			_asKlaymenLadderHands->setClipRect(_klaymen->getClipRect());
		}
	} else if (which == 1) {
		// Klaymen entering from the right
		insertKlaymen<KmScene1002>(650, 435);
		_asKlaymenLadderHands = insertSprite<AsScene1002KlaymenLadderHands>(_klaymen);
		setMessageList(0x004B4478);
		_klaymen->setClipRect(_ssLadderArch->getDrawRect().x, 0, _ssLadderArchPart2->getDrawRect().x2(), _ssLadderArchPart1->getDrawRect().y2());
		_asKlaymenLadderHands->setClipRect(_klaymen->getClipRect());
		_vm->_gameState.which = 1;
	} else if (which == 2) {
		// Klaymen coming up the ladder
		insertKlaymen<KmScene1002>(68, 645);
		_asKlaymenLadderHands = insertSprite<AsScene1002KlaymenLadderHands>(_klaymen);
		setMessageList(0x004B4298);
		_klaymen->setClipRect(_ssLadderArch->getDrawRect().x, 0, _ssLadderArchPart2->getDrawRect().x2(), _ssLadderArchPart1->getDrawRect().y2());
		_asKlaymenLadderHands->setClipRect(_klaymen->getClipRect());
		_vm->_gameState.which = 1;
		sendMessage(_klaymen, 0x4820, 0);
	} else {
		// Klaymen entering from the left, peeking
		insertKlaymen<KmScene1002>(90, 226);
		_asKlaymenLadderHands = insertSprite<AsScene1002KlaymenLadderHands>(_klaymen);
		setMessageList(0x004B4470);
		_klaymen->setClipRect(31, 0, _ssLadderArchPart2->getDrawRect().x2(), _ssLadderArchPart3->getDrawRect().y2());
		_asKlaymenLadderHands->setClipRect(_klaymen->getClipRect());
		_asKlaymenPeekHand = insertSprite<AsScene1002KlaymenPeekHand>(this, _klaymen);
		_asKlaymenPeekHand->setClipRect(_klaymen->getClipRect());
		_klaymen->setRepl(64, 0);
		_vm->_gameState.which = 0;
	}

	insertScreenMouse(0x23303124);

	tempSprite = insertStaticSprite(0xB3242310, 825);
	tempClipRect.set(tempSprite->getDrawRect().x, tempSprite->getDrawRect().y,
		_ssLadderArchPart2->getDrawRect().x2(), _ssLadderArchPart2->getDrawRect().y2());

	_asRing1 = insertSprite<AsScene1002Ring>(this, false, 258, 191, _ssCeiling->getDrawRect().y, false);
	_asRing2 = insertSprite<AsScene1002Ring>(this, false, 297, 189, _ssCeiling->getDrawRect().y, false);
	_asRing3 = insertSprite<AsScene1002Ring>(this, true, 370, 201, _ssCeiling->getDrawRect().y, getGlobalVar(V_FLYTRAP_RING_DOOR));
	_asRing4 = insertSprite<AsScene1002Ring>(this, false, 334, 191, _ssCeiling->getDrawRect().y, false);
	_asRing5 = insertSprite<AsScene1002Ring>(this, false, 425, 184, _ssCeiling->getDrawRect().y, false);

	_asDoor = insertSprite<AsScene1002Door>(tempClipRect);
	tempSprite = insertSprite<AsScene1002BoxingGloveHitEffect>();
	_asDoorSpy = insertSprite<AsScene1002DoorSpy>(tempClipRect, this, _asDoor, tempSprite);
	_ssPressButton = insertSprite<SsCommonPressButton>(this, 0x00412692, 0x140B60BE, 800, 0);
	_asVenusFlyTrap = insertSprite<AsScene1002VenusFlyTrap>(this, _klaymen, false);
	addCollisionSprite(_asVenusFlyTrap);

	sendEntityMessage(_klaymen, NM_CAR_MOVE_TO_PREV_POINT, _asVenusFlyTrap);

	_asOutsideDoorBackground = insertSprite<AsScene1002OutsideDoorBackground>();

	setRectList(0x004B43A0);

	loadSound(1, 0x60755842);
	loadSound(2, 0x616D5821);

}

Scene1002::~Scene1002() {
}

void Scene1002::update() {
	Scene::update();
	if (!_isKlaymenFloor && _klaymen->getY() > 230) {
		_klaymen->setClipRect(_ssLadderArch->getDrawRect().x, 0, _ssLadderArchPart2->getDrawRect().x2(), _ssLadderArchPart1->getDrawRect().y2());
		_asKlaymenLadderHands->setClipRect(_klaymen->getClipRect());
		deleteSprite(&_ssLadderArchPart3);
		_klaymen->clearRepl();
		_isKlaymenFloor = true;
		_vm->_gameState.which = 1;
	}
}

uint32 Scene1002::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0xE6EE60E1) {
			if (getGlobalVar(V_FLYTRAP_RING_DOOR))
				setMessageList(0x004B4428);
			else
				setMessageList(0x004B4448);
			messageResult = 1;
		} else if (param.asInteger() == 0x4A845A00)
			sendEntityMessage(_klaymen, 0x1014, _asRing1);
		else if (param.asInteger() == 0x43807801)
			sendEntityMessage(_klaymen, 0x1014, _asRing2);
		else if (param.asInteger() == 0x46C26A01) {
			if (getGlobalVar(V_FLYTRAP_RING_DOOR)) {
				setMessageList(0x004B44B8);
			} else {
				sendEntityMessage(_klaymen, 0x1014, _asRing3);
				if (_asVenusFlyTrap->getX() - 10 < 366 && _asVenusFlyTrap->getX() + 10 > 366) {
					setGlobalVar(V_FLYTRAP_RING_EATEN, 1);
					setMessageList(0x004B44A8);
				} else {
					setMessageList(0x004B44A0);
				}
			}
			messageResult = 1;
		} else if (param.asInteger() == 0x468C7B11)
			sendEntityMessage(_klaymen, 0x1014, _asRing4);
		else if (param.asInteger() == 0x42845B19)
			sendEntityMessage(_klaymen, 0x1014, _asRing5);
		else if (param.asInteger() == 0xC0A07458)
			sendEntityMessage(_klaymen, 0x1014, _ssPressButton);
		break;
	case 0x1024:
		sendMessage(_parentModule, 0x1024, param.asInteger());
		break;
	case NM_ANIMATION_UPDATE:
		if (_isClimbingLadder) {
			setMessageList2(0x004B43D0);
		} else {
			if (_klaymen->getY() > 420) {
				sendEntityMessage(_klaymen, 0x1014, _asVenusFlyTrap);
				setMessageList2(0x004B4480);
			} else if (_klaymen->getY() > 227) {
				setMessageList2(0x004B41E0);
			} else {
				setMessageList2(0x004B4148);
			}
		}
		break;
	case NM_POSITION_CHANGE:
		_messageList = NULL;
		break;
	case NM_KLAYMEN_CLIMB_LADDER:
		_isClimbingLadder = true;
		setRectList(0x004B4418);
		break;
	case NM_KLAYMEN_STOP_CLIMBING:
		_isClimbingLadder = false;
		setRectList(0x004B43A0);
		break;
	case NM_KLAYMEN_USE_OBJECT:
		if (sender == _asRing1) {
			setGlobalVar(V_RADIO_ENABLED, 0);
			playSound(0, 0x665198C0);
		} else if (sender == _asRing2) {
			setGlobalVar(V_RADIO_ENABLED, 0);
			playSound(0, 0xE2D389C0);
		} else if (sender == _asRing3) {
			setGlobalVar(V_RADIO_ENABLED, 0);
			playSound(1);
			sendMessage(_asDoor, NM_KLAYMEN_OPEN_DOOR, 0);
			sendMessage(_asOutsideDoorBackground, NM_KLAYMEN_OPEN_DOOR, 0);
		} else if (sender == _asRing4) {
			setGlobalVar(V_RADIO_ENABLED, 0);
			playSound(0, 0xE0558848);
		} else if (sender == _asRing5) {
			setGlobalVar(V_RADIO_ENABLED, 1);
			playSound(0, 0x44014282);
		}
		break;
	case NM_KLAYMEN_RAISE_LEVER:
		if (sender == _asRing3) {
			playSound(2);
			sendMessage(_asDoor, NM_KLAYMEN_CLOSE_DOOR, 0);
			sendMessage(_asOutsideDoorBackground, NM_KLAYMEN_CLOSE_DOOR, 0);
		} else if (sender == _asVenusFlyTrap) {
			if (getGlobalVar(V_FLYTRAP_RING_DOOR)) {
				sendMessage(_asRing3, NM_KLAYMEN_RAISE_LEVER, 0);
			}
		}
		break;
	case 0x480B:
		sendEntityMessage(_klaymen, 0x1014, _asDoorSpy);
		break;
	case NM_KLAYMEN_LOWER_LEVER:
		setGlobalVar(V_RADIO_ENABLED, 0);
		playSound(1);
		sendMessage(_asDoor, NM_KLAYMEN_OPEN_DOOR, 0);
		sendMessage(_asOutsideDoorBackground, NM_KLAYMEN_OPEN_DOOR, 0);
		break;
	case 0x8000:
		setSpriteSurfacePriority(_ssCeiling, 995);
		setSpriteSurfacePriority(_ssLadderArch, 995);
		break;
	case 0x8001:
		setSpriteSurfacePriority(_ssCeiling, 1015);
		setSpriteSurfacePriority(_ssLadderArch, 1015);
		break;
	default:
		break;
	}
	return messageResult;
}

Scene1004::Scene1004(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _paletteAreaStatus(-1) {

	Sprite *tempSprite;

	SetUpdateHandler(&Scene1004::update);
	SetMessageHandler(&Scene1004::handleMessage);

	setBackground(0x50C03005);

	if (getGlobalVar(V_ENTRANCE_OPEN)) {
		setPalette(0xA30BA329);
		_palette->addBasePalette(0xA30BA329, 0, 256, 0);
	} else {
		setPalette(0x50C03005);
		_palette->addBasePalette(0x50C03005, 0, 256, 0);
	}
	addEntity(_palette);

	insertScreenMouse(0x03001504);

	if (which < 0) {
		// Restoring game
		setRectList(0x004B7C70);
		insertKlaymen<KmScene1004>(330, 327);
		setMessageList(0x004B7C18);
	} else if (which == 1) {
		// Klaymen returning from reading a note
		setRectList(0x004B7C70);
		insertKlaymen<KmScene1004>(330, 327);
		setMessageList(0x004B7C08);
	} else {
		// Klaymen coming down the ladder
		loadDataResource(0x01900A04);
		insertKlaymen<KmScene1004>(_dataResource.getPoint(0x80052A29).x, 27);
		setMessageList(0x004B7BF0);
	}

	updatePaletteArea();

	_asKlaymenLadderHands = insertSprite<AsScene1002KlaymenLadderHands>(_klaymen);

	insertStaticSprite(0x800034A0, 1100);
	insertStaticSprite(0x64402020, 1100);
	insertStaticSprite(0x3060222E, 1300);
	tempSprite = insertStaticSprite(0x0E002004, 1300);

	_klaymen->setClipRect(0, tempSprite->getDrawRect().y, 640, 480);
	_asKlaymenLadderHands->setClipRect(_klaymen->getClipRect());

	_asTrashCan = insertSprite<AsScene1004TrashCan>();

}

void Scene1004::update() {
	Scene::update();
	updatePaletteArea();
}

uint32 Scene1004::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x926500A1) {
			setMessageList(0x004B7C20);
			messageResult = 1;
		}
		break;
	case NM_ANIMATION_UPDATE:
		loadDataResource(0x01900A04);
		break;
	case 0x2001:
		setRectList(0x004B7C70);
		break;
	case NM_POSITION_CHANGE:
		sendMessage(_asTrashCan, NM_POSITION_CHANGE, 0);
		break;
	default:
		break;
	}
	return messageResult;
}

void Scene1004::updatePaletteArea() {
	if (_klaymen->getY() < 150) {
		if (_paletteAreaStatus != 0) {
			_paletteAreaStatus = 0;
			_palette->addBasePalette(0x406B0D10, 0, 64, 0);
			_palette->startFadeToPalette(12);
		}
	} else {
		if (_paletteAreaStatus != 1) {
			_paletteAreaStatus = 1;
			_palette->addBasePalette(0x24332243, 0, 64, 0);
			_palette->startFadeToPalette(12);
		}
	}
}

Scene1005::Scene1005(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	SetMessageHandler(&Scene1005::handleMessage);

	if (getGlobalVar(V_ENTRANCE_OPEN)) {
		setBackground(0x2800E011);
		setPalette(0x2800E011);
		insertStaticSprite(0x492D5AD7, 100);
		insertPuzzleMouse(0x0E015288, 20, 620);
	} else {
		setBackground(0x8870A546);
		setPalette(0x8870A546);
		insertStaticSprite(0x40D1E0A9, 100);
		insertStaticSprite(0x149C00A6, 100);
		insertPuzzleMouse(0x0A54288F, 20, 620);
	}

	drawTextToBackground();

}

uint32 Scene1005::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_MOUSE_CLICK:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620)
			leaveScene(0);
		break;
	default:
		break;
	}
	return 0;
}

void Scene1005::drawTextToBackground() {
	TextResource textResource(_vm);
	const char *textStart, *textEnd;
	int16 y = 36;
	uint32 textIndex = getTextIndex();
	FontSurface *fontSurface = FontSurface::createFontSurface(_vm, getGlobalVar(V_ENTRANCE_OPEN) ? 0x283CE401 : 0xC6604282);
	textResource.load(0x80283101);
	textStart = textResource.getString(textIndex, textEnd);
	while (textStart < textEnd) {
		fontSurface->drawString(_background->getSurface(), 188, y, (const byte*)textStart);
		y += 36;
		textStart += strlen(textStart) + 1;
	}
	delete fontSurface;
}

uint32 Scene1005::getTextIndex() {
	uint32 textIndex;
	textIndex = getTextIndex1();
	if (getGlobalVar(V_ENTRANCE_OPEN)) {
		textIndex = getKloggsTextIndex();
	}
	if (getGlobalVar(V_TEXT_FLAG1) && getGlobalVar(V_TEXT_INDEX) == textIndex) {
		textIndex = getTextIndex3();
	} else {
		setGlobalVar(V_TEXT_FLAG1, 1);
		setGlobalVar(V_TEXT_INDEX, textIndex);
	}
	return textIndex;
}

uint32 Scene1005::getTextIndex1() {
	uint32 textIndex;
	if (getGlobalVar(V_WORLDS_JOINED)) {
		if (!getGlobalVar(V_DOOR_PASSED))
			textIndex = 18;
		else if (!getGlobalVar(V_ROBOT_TARGET))
			textIndex = 19;
		else if (getGlobalVar(V_ROBOT_HIT)) {
			if (!getGlobalVar(V_ENTRANCE_OPEN))
				textIndex = 23;
			else if (!getSubVar(VA_HAS_KEY, 0) && !getSubVar(VA_IS_KEY_INSERTED, 0))
				textIndex = 24;
			else if (!getGlobalVar(V_HAS_FINAL_KEY))
				textIndex = 26;
			else if (!getSubVar(VA_HAS_KEY, 1) && !getSubVar(VA_IS_KEY_INSERTED, 1))
				textIndex = 27;
			else if (!getGlobalVar(V_HAS_FINAL_KEY))
				textIndex = 28;
			else
				textIndex = 29;
		} else if (!getGlobalVar(V_FELL_DOWN_HOLE))
			textIndex = 20;
		else if (!getGlobalVar(V_SEEN_SYMBOLS_NO_LIGHT))
			textIndex = 21;
		else
			textIndex = 22;
	} else if (getGlobalVar(V_BOLT_DOOR_UNLOCKED)) {
		if (!getGlobalVar(V_WALL_BROKEN))
			textIndex = 12;
		else if (!getGlobalVar(V_STAIRS_DOWN_ONCE))
			textIndex = 13;
		else if (!getGlobalVar(V_RADIO_ENABLED))
			textIndex = 50;
		else if (!getGlobalVar(V_UNUSED))
			textIndex = 14;
		else if (!getGlobalVar(V_BEEN_SHRINKING_ROOM))
			textIndex = 15;
		else if (!getGlobalVar(V_BEEN_STATUE_ROOM))
			textIndex = 16;
		else
			textIndex = 17;
	} else if (!getGlobalVar(V_FLYTRAP_RING_EATEN)) {
		textIndex = 0;
	} else if (getGlobalVar(V_CREATURE_EXPLODED)) {
		if (!getGlobalVar(V_TILE_PUZZLE_SOLVED))
			textIndex = 4;
		else if (!getGlobalVar(V_HAS_TEST_TUBE))
			textIndex = 5;
		else if (!getSubVar(VA_LOCKS_DISABLED, 0x40119852))
			textIndex = 6;
		else if (!getGlobalVar(V_WATER_RUNNING))
			textIndex = 7;
		else if (!getGlobalVar(V_NOTES_PUZZLE_SOLVED))
			textIndex = 8;
		else if (!getSubVar(VA_LOCKS_DISABLED, 0x304008D2))
			textIndex = 9;
		else if (!getSubVar(VA_LOCKS_DISABLED, 0x01180951))
			textIndex = 10;
		else
			textIndex = 11;
	} else if (!getGlobalVar(V_CREATURE_ANGRY)) {
		textIndex = 1;
	} else if (getGlobalVar(V_TNT_DUMMY_BUILT)) {
		textIndex = 3;
	} else {
		textIndex = 2;
	}
	return textIndex;
}

uint32 Scene1005::getKloggsTextIndex() {
	uint32 textIndex = getGlobalVar(V_TEXT_COUNTING_INDEX1);
	if (textIndex + 1 > 10) {
		textIndex = 0;
	}
	setGlobalVar(V_TEXT_COUNTING_INDEX1, textIndex + 1);
	return textIndex + 40;
}

uint32 Scene1005::getTextIndex3() {
	uint32 textIndex = getGlobalVar(V_TEXT_COUNTING_INDEX2);
	if (textIndex + 1 > 10) {
		textIndex = 0;
	}
	setGlobalVar(V_TEXT_COUNTING_INDEX2, textIndex + 1);
	return textIndex + 30;
}

} // End of namespace Neverhood
