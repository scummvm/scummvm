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

#include "neverhood/diskplayerscene.h"
#include "neverhood/gamemodule.h"
#include "neverhood/modules/module1000_sprites.h"
#include "neverhood/modules/module1200_sprites.h"
#include "neverhood/modules/module1400.h"
#include "neverhood/modules/module1400_sprites.h"
#include "neverhood/modules/module2100_sprites.h"
#include "neverhood/modules/module2200_sprites.h"

namespace Neverhood {

Module1400::Module1400(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	_vm->_soundMan->addMusic(0x00AD0012, 0x06333232);
	_vm->_soundMan->addMusic(0x00AD0012, 0x624A220E);

	if (which < 0)
		createScene(_vm->gameState().sceneNum, -1);
	else
		createScene(0, 0);

}

Module1400::~Module1400() {
	_vm->_soundMan->deleteMusicGroup(0x00AD0012);
}

void Module1400::createScene(int sceneNum, int which) {
	debug(1, "Module1400::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		_vm->_soundMan->startMusic(0x06333232, 0, 2);
		_childObject = new Scene1401(_vm, this, which);
		break;
	case 1:
		_vm->gameState().sceneNum = 1;
		_vm->_soundMan->stopMusic(0x06333232, 0, 2);
		_vm->_soundMan->stopMusic(0x624A220E, 0, 2);
		_childObject = new Scene1402(_vm, this, which);
		break;
	case 2:
		_vm->gameState().sceneNum = 2;
		_vm->_soundMan->stopMusic(0x06333232, 0, 2);
		_vm->_soundMan->startMusic(0x624A220E, 0, 2);
		_childObject = new Scene1403(_vm, this, which);
		break;
	case 3:
		_vm->gameState().sceneNum = 3;
		_vm->_soundMan->startMusic(0x06333232, 0, 2);
		_childObject = new Scene1404(_vm, this, which);
		break;
	case 4:
		_vm->gameState().sceneNum = 4;
		_vm->_soundMan->startMusic(0x06333232, 0, 2);
		_childObject = new Scene1405(_vm, this);
		break;
	case 5:
		_vm->gameState().sceneNum = 5;
		_vm->_soundMan->stopMusic(0x06333232, 0, 2);
		_childObject = new DiskplayerScene(_vm, this, 2);
		break;
	case 6:
		_vm->gameState().sceneNum = 6;
		_vm->_soundMan->stopMusic(0x06333232, 0, 2);
		_childObject = new Scene1407(_vm, this);
		break;
	default:
		break;
	}
	SetUpdateHandler(&Module1400::updateScene);
	_childObject->handleUpdate();
}

void Module1400::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 0:
			if (_moduleResult == 1)
				createScene(1, 0);
			else if (_moduleResult == 2)
				createScene(3, 0);
			else
				leaveModule(0);
			break;
		case 1:
			if (_moduleResult == 1)
				createScene(2, 0);
			else if (_moduleResult == 2)
				createScene(6, -1);
			else
				createScene(0, 1);
			break;
		case 2:
			createScene(1, 1);
			break;
		case 3:
			if (_moduleResult == 1)
				createScene(4, 0);
			else if (_moduleResult == 2)
				createScene(5, -1);
			else
				createScene(0, 2);
			break;
		case 4:
			createScene(3, 1);
			break;
		case 5:
			createScene(3, 2);
			break;
		case 6:
			createScene(1, 2);
			break;
		default:
			break;
		}
	}
}

Scene1401::Scene1401(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _projectorBorderFlag(false), _ssFloorButton(NULL), _asProjector(NULL),
	_asPipe(NULL), _asMouse(NULL), _asCheese(NULL), _asBackDoor(NULL),
	_sprite1(NULL), _sprite2(NULL), _sprite3(NULL), _ssButton(NULL) {

	SetMessageHandler(&Scene1401::handleMessage);
	SetUpdateHandler(&Scene1401::update);

	setRectList(0x004B6758);
	setBackground(0x08221FA5);
	setPalette(0x08221FA5);
	insertScreenMouse(0x21FA108A);

	_ssFloorButton = insertSprite<SsCommonFloorButton>(this, 0x980F3124, 0x12192892, 100, 0);
	_asPipe = insertSprite<AsScene1401Pipe>();

	if (!getGlobalVar(V_MOUSE_SUCKED_IN)) {
		_asMouse = insertSprite<AsScene1401Mouse>();
		_asCheese = insertSprite<AsScene1401Cheese>();
	}

	_sprite3 = insertStaticSprite(0xA82BA811, 1100);
	insertStaticSprite(0x0A116C60, 1100);
	_ssButton = insertSprite<SsCommonButtonSprite>(this, 0xB84B1100, 100, 0);
	_sprite1 = insertStaticSprite(0x38EA100C, 1005);
	_sprite2 = insertStaticSprite(0x98D0223C, 1200);
	_sprite2->setVisible(false);

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene1401>(380, 447);
		setMessageList(0x004B65C8);
		_sprite1->setVisible(false);
	} else if (which == 1) {
		// Klaymen entering from the left
		insertKlaymen<KmScene1401>(0, 447);
		setMessageList(0x004B65D0);
		_sprite1->setVisible(false);
	} else if (which == 2) {
		// Klaymen entering from the right
		insertKlaymen<KmScene1401>(660, 447);
		setMessageList(0x004B65D8);
		_sprite1->setVisible(false);
	} else {
		// Klaymen entering from the back
		insertKlaymen<KmScene1401>(290, 413);
		setMessageList(0x004B65E8);
		_sprite1->setVisible(false);
	}

	if (getGlobalVar(V_PROJECTOR_LOCATION) == 2) {
		_asProjector = insertSprite<AsCommonProjector>(this, _klaymen, _asPipe);
		addCollisionSprite(_asProjector);
		if (getGlobalVar(V_PROJECTOR_SLOT) == 6) {
			sendEntityMessage(_klaymen, 0x1014, _asProjector);
			_klaymen->setX(_asProjector->getX() + 100);
			_klaymen->updateBounds();
			setMessageList(0x004B6670);
		} else if (getGlobalVar(V_PROJECTOR_SLOT) == 0) {
			sendEntityMessage(_klaymen, 0x1014, _asProjector);
			_klaymen->setX(_asProjector->getX() - 100);
			_klaymen->updateBounds();
			setMessageList(0x004B6670);
		}
		_asProjector->setClipRect(_sprite3->getDrawRect().x, _sprite2->getDrawRect().y, 640, 480);
	}

	_klaymen->setClipRect(_sprite3->getDrawRect().x, 0, 640, 480);

	if (which == 0 && _asProjector)
		sendMessage(_asProjector, NM_MOVE_TO_FRONT, 0);

	_asBackDoor = insertSprite<AsScene1401BackDoor>(_klaymen, which == 0);

}

void Scene1401::update() {
	Scene::update();
	if (_asProjector && !_projectorBorderFlag && _asProjector->getY() < 360) {
		_sprite2->setVisible(true);
		_projectorBorderFlag = true;
	} else
		_sprite2->setVisible(false);
}

uint32 Scene1401::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x02144CB1)
			sendEntityMessage(_klaymen, 0x1014, _ssFloorButton);
		else if (param.asInteger() == 0x402064D8)
			sendEntityMessage(_klaymen, 0x1014, _ssButton);
		else if (param.asInteger() == 0x01C66840) {
			if (sendMessage(_asBackDoor, 0x2001, 0) != 0)
				setMessageList(0x004B6690);
			else
				setMessageList(0x004B66B0);
		}
		break;
	case NM_SCENE_LEAVE:
		if (param.asInteger() != 0)
			leaveScene(2);
		else
			leaveScene(1);
		break;
	case 0x480B:
		if (sender == _ssFloorButton) {
			sendMessage(_asPipe, 0x2000, 0);
			if (!getGlobalVar(V_MOUSE_SUCKED_IN)) {
				sendMessage(_asMouse, 0x4839, 0);
				sendMessage(_asCheese, 0x4839, 0);
				setGlobalVar(V_MOUSE_SUCKED_IN, 1);
			}
			if (_asProjector && _asProjector->getX() > 404 && _asProjector->getX() < 504)
				sendMessage(_asProjector , 0x4839, 0);
		} else if (sender == _ssButton)
			sendMessage(_asBackDoor, NM_KLAYMEN_OPEN_DOOR, 0);
		break;
	case 0x4826:
		if (sender == _asProjector) {
			if (sendMessage(_asProjector, 0x480C, _klaymen->getX() > _asProjector->getX() ? 1 : 0) != 0) {
				sendEntityMessage(_klaymen, 0x1014, _asProjector);
				setMessageList2(0x004B6658);
			} else
				setMessageList2(0x004B65F0);
		}
		break;
	case NM_MOVE_TO_BACK:
		_sprite1->setVisible(true);
		if (_asProjector)
			sendMessage(_asProjector, NM_MOVE_TO_FRONT, 0);
		break;
	case NM_MOVE_TO_FRONT:
		_sprite1->setVisible(false);
		if (_asProjector)
			sendMessage(_asProjector, NM_MOVE_TO_BACK, 0);
		break;
	default:
		break;
	}
	return 0;
}

Scene1402::Scene1402(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _isShaking(false), _asPuzzleBox(NULL), _asProjector(NULL) {

	SetMessageHandler(&Scene1402::handleMessage);

	_vm->_screen->setYOffset(0);

	setBackground(0x231482F0);
	setBackgroundY(-10);
	setPalette(0x231482F0);
	_palette->addPalette(0x91D3A391, 0, 64, 0);
	insertScreenMouse(0x482F4239);

	_ssBridgePart1 = insertSprite<SsScene1402BridgePart>(0x15402D64, 1100);
	_ssBridgePart2 = insertSprite<SsScene1402BridgePart>(0x10A02120, 1100);
	_ssBridgePart3 = insertSprite<SsScene1402BridgePart>(0x60882BE0, 1100);

	if (getGlobalVar(V_MOUSE_PUZZLE_SOLVED))
		setRectList(0x004B0C48);
	else
		setRectList(0x004B0C98);

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene1402>(377, 391);
		setMessageList(0x004B0B48);
		if (!getGlobalVar(V_MOUSE_PUZZLE_SOLVED))
			_asPuzzleBox = insertSprite<AsScene1402PuzzleBox>(this, 0);
	} else if (which == 1) {
		// Klaymen entering from the left
		insertKlaymen<KmScene1402>(42, 391);
		setMessageList(0x004B0B50);
	} else if (which == 2) {
		// Klaymen returning from the puzzle box
		insertKlaymen<KmScene1402>(377, 391);
		setMessageList(0x004B0B60);
		_klaymen->setDoDeltaX(1);
		if (getGlobalVar(V_MOUSE_PUZZLE_SOLVED)) {
			_asPuzzleBox = insertSprite<AsScene1402PuzzleBox>(this, 1);
			clearRectList();
			showMouse(false);
			startShaking();
		} else
			_asPuzzleBox = insertSprite<AsScene1402PuzzleBox>(this, 0);
	} else {
		// Klaymen entering from the right
		insertKlaymen<KmScene1402>(513, 391);
		setMessageList(0x004B0B58);
		if (!getGlobalVar(V_MOUSE_PUZZLE_SOLVED)) {
			_asPuzzleBox = insertSprite<AsScene1402PuzzleBox>(this, 2);
			startShaking();
		}
	}

	if (_asPuzzleBox)
		_asPuzzleBox->setClipRect(0, 0, 640, _ssBridgePart3->getDrawRect().y2());

	if (getGlobalVar(V_PROJECTOR_LOCATION) == 1) {
		_asProjector = insertSprite<AsCommonProjector>(this, _klaymen, (Sprite*)NULL);
		addCollisionSprite(_asProjector);
		if (getGlobalVar(V_PROJECTOR_SLOT) == 4) {
			sendEntityMessage(_klaymen, 0x1014, _asProjector);
			_klaymen->setX(_asProjector->getX() + 100);
			_klaymen->updateBounds();
			setMessageList(0x004B0BD0);
		} else if (getGlobalVar(V_PROJECTOR_SLOT) == 0) {
			sendEntityMessage(_klaymen, 0x1014, _asProjector);
			_klaymen->setX(_asProjector->getX() - 100);
			_klaymen->updateBounds();
			setMessageList(0x004B0BD0);
		}
		_asProjector->setClipRect(_ssBridgePart1->getDrawRect().x, 0, _ssBridgePart2->getDrawRect().x, _ssBridgePart3->getDrawRect().y2());
	}

	_klaymen->setClipRect(_ssBridgePart1->getDrawRect().x, 0, _ssBridgePart2->getDrawRect().x2(), _ssBridgePart3->getDrawRect().y2());

}

void Scene1402::upShaking() {
	if (_isShaking) {
		setBackgroundY(_vm->_rnd->getRandomNumber(10 - 1) - 10);
		_vm->_screen->setYOffset(-10 - getBackgroundY());
	} else {
		setBackgroundY(-10);
		_vm->_screen->setYOffset(0);
		SetUpdateHandler(&Scene::update);
	}
	Scene::update();
	if (_asPuzzleBox)
		_asPuzzleBox->setClipRect(0, 0, 640, _ssBridgePart3->getDrawRect().y2());
	_klaymen->setClipRect(_ssBridgePart1->getDrawRect().x, 0, _ssBridgePart2->getDrawRect().x2(), _ssBridgePart3->getDrawRect().y2());
}

uint32 Scene1402::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x00F43389) {
			if (getGlobalVar(V_MOUSE_PUZZLE_SOLVED))
				leaveScene(0);
			else {
				clearRectList();
				_klaymen->setVisible(false);
				showMouse(false);
				sendMessage(_asPuzzleBox, NM_POSITION_CHANGE, 0);
				startShaking();
			}
		}
		break;
	case NM_SCENE_LEAVE:
		if (param.asInteger())
			leaveScene(0);
		else
			leaveScene(1);
		break;
	case NM_ANIMATION_UPDATE:
		stopShaking();
		showMouse(true);
		setRectList(0x004B0C48);
		break;
	case 0x2001:
		stopShaking();
		leaveScene(0);
		break;
	case 0x2003:
		stopShaking();
		break;
	case 0x4826:
		if (sender == _asProjector) {
			if (sendMessage(_asProjector, 0x480C, _klaymen->getX() > _asProjector->getX() ? 1 : 0) != 0) {
				sendEntityMessage(_klaymen, 0x1014, _asProjector);
				setMessageList2(0x004B0BB8);
			} else
				setMessageList2(0x004B0B68);
		}
		break;
	default:
		break;
	}
	return 0;
}

void Scene1402::startShaking() {
	_isShaking = true;
	SetUpdateHandler(&Scene1402::upShaking);
}

void Scene1402::stopShaking() {
	_isShaking = false;
}

Scene1407::Scene1407(NeverhoodEngine *vm, Module *parentModule)
	: Scene(vm, parentModule), _puzzleSolvedCountdown(0),	_resetButtonCountdown(0) {

	SetMessageHandler(&Scene1407::handleMessage);
	SetUpdateHandler(&Scene1407::update);

	setBackground(0x00442225);
	setPalette(0x00442225);
	insertPuzzleMouse(0x4222100C, 20, 620);

	_asMouse = insertSprite<AsScene1407Mouse>(this);
	_ssResetButton = insertStaticSprite(0x12006600, 100);
	_ssResetButton->setVisible(false);

}

void Scene1407::update() {
	Scene::update();
	if (_puzzleSolvedCountdown != 0 && (--_puzzleSolvedCountdown == 0))
		leaveScene(1);
	else if (_resetButtonCountdown != 0 && (--_resetButtonCountdown == 0))
		_ssResetButton->setVisible(false);
}

uint32 Scene1407::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_MOUSE_CLICK:
		if (_puzzleSolvedCountdown == 0) {
			if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
				// Exit scene
				leaveScene(0);
			} else if (param.asPoint().x >= 75 && param.asPoint().x <= 104 &&
				param.asPoint().y >= 62 && param.asPoint().y <= 90) {
				// The reset button was clicked
				sendMessage(_asMouse, 0x2001, 0);
				_ssResetButton->setVisible(true);
				playSound(0, 0x44045000);
				_resetButtonCountdown = 12;
			} else {
				// Handle the mouse
				sendMessage(_asMouse, messageNum, param);
			}
		}
		break;
	case NM_ANIMATION_UPDATE:
		// The mouse got the cheese (nomnom)
		setGlobalVar(V_MOUSE_PUZZLE_SOLVED, 1);
		playSound(0, 0x68E25540);
		showMouse(false);
		_puzzleSolvedCountdown = 72;
		break;
	default:
		break;
	}
	return 0;
}

Scene1403::Scene1403(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _asProjector(NULL), _isProjecting(false) {

	SetMessageHandler(&Scene1403::handleMessage);

	setRectList(0x004B1FF8);
	setBackground(0x2110A234);
	setPalette(0x2110A234);
	insertScreenMouse(0x0A230219);

	_sprite1 = insertStaticSprite(0x01102A33, 100);
	_sprite1->setVisible(false);
	_sprite2 = insertStaticSprite(0x04442520, 995);
	_sprite3 = insertStaticSprite(0x08742271, 995);
	_asTape1 = insertSprite<AsScene1201Tape>(this, 12, 1100, 201, 468, 0x9148A011);
	addCollisionSprite(_asTape1);
	_asTape1->setRepl(64, 0);
	_asTape2 = insertSprite<AsScene1201Tape>(this, 16, 1100, 498, 468, 0x9048A093);
	addCollisionSprite(_asTape2);
	_asTape2->setRepl(64, 0);

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene1403>(380, 463);
		setMessageList(0x004B1F18);
	} else {
		// Klaymen entering from the right
		insertKlaymen<KmScene1403>(640, 463);
		setMessageList(0x004B1F20);
	}
	_klaymen->setRepl(64, 0);

	if (getGlobalVar(V_PROJECTOR_LOCATION) == 0) {
		_asProjector = insertSprite<AsCommonProjector>(this, _klaymen, (Sprite*)NULL);
		addCollisionSprite(_asProjector);
		if (getGlobalVar(V_PROJECTOR_SLOT) == 4) {
			sendEntityMessage(_klaymen, 0x1014, _asProjector);
			_klaymen->setX(_asProjector->getX() + 100);
			_klaymen->updateBounds();
			setMessageList(0x004B1F70);
		}
		_asProjector->setClipRect(0, 0, 640, _sprite2->getDrawRect().y2());
		_asProjector->setRepl(64, 0);
	}

}

uint32 Scene1403::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x88C11390) {
			setRectList(0x004B2008);
			_isProjecting = true;
		} else if (param.asInteger() == 0x08821382) {
			sendEntityMessage(_klaymen, 0x1014, _asProjector);
			setRectList(0x004B1FF8);
			_isProjecting = false;
		}
		break;
	case NM_SCENE_LEAVE:
		leaveScene(0);
		break;
	case NM_PRIORITY_CHANGE:
		if (sender == _asProjector) {
			if (param.asInteger() >= 1000)
				setSurfacePriority(_sprite3->getSurface(), 1100);
			else
				setSurfacePriority(_sprite3->getSurface(), 995);
		}
		break;
	case NM_KLAYMEN_RAISE_LEVER:
		_sprite1->setVisible(false);
		break;
	case NM_KLAYMEN_LOWER_LEVER:
		_sprite1->setVisible(true);
		break;
	case 0x4826:
		if (sender == _asProjector) {
			if (_isProjecting)
				setMessageList2(0x004B1FA8);
			else if (param.asInteger() == 1) {
				sendEntityMessage(_klaymen, 0x1014, _asProjector);
				setMessageList2(0x004B1F88);
			} else if (sendMessage(_asProjector, 0x480C, _klaymen->getX() > _asProjector->getX() ? 1 : 0) != 0) {
				sendEntityMessage(_klaymen, 0x1014, _asProjector);
				setMessageList2(0x004B1F58);
			} else
				setMessageList2(0x004B1F28);
		} else if (sender == _asTape1 || sender == _asTape2) {
			if (_isProjecting)
				setMessageList2(0x004B1FA8);
			else if (_messageListStatus != 2) {
				sendEntityMessage(_klaymen, 0x1014, sender);
				setMessageList2(0x004B1FB8);
			}
		}
		break;
	default:
		break;
	}
	return 0;
}

Scene1404::Scene1404(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _asProjector(NULL), _asKey(NULL) {

	if (getGlobalVar(V_HAS_FINAL_KEY) && getGlobalVar(V_KEY3_LOCATION) == 0)
		setGlobalVar(V_KEY3_LOCATION, 5);

	SetMessageHandler(&Scene1404::handleMessage);

	setRectList(0x004B8D80);
	setBackground(0xAC0B006F);
	setPalette(0xAC0B006F);
	_palette->addPalette(0x00801510, 0, 65, 0);
	insertScreenMouse(0xB006BAC8);

	if (getGlobalVar(V_KEY3_LOCATION) == 5) {
		_asKey = insertSprite<AsCommonKey>(this, 2, 1100, 267, 411);
		addCollisionSprite(_asKey);
	}

	_sprite1 = insertStaticSprite(0x1900A1F8, 1100);
	_asTape = insertSprite<AsScene1201Tape>(this, 14, 1100, 281, 411, 0x9148A011);
	addCollisionSprite(_asTape);

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene1404>(376, 406);
		setMessageList(0x004B8C28);
	} else if (which == 1) {
		// Klaymen returning from the tiles puzzle
		insertKlaymen<KmScene1404>(376, 406);
		setMessageList(0x004B8C30);
	} else if (which == 2) {
		// Klaymen returning from the diskplayer
		if (getGlobalVar(V_KLAYMEN_IS_DELTA_X)) {
			insertKlaymen<KmScene1404>(347, 406);
			_klaymen->setDoDeltaX(1);
		} else {
			insertKlaymen<KmScene1404>(187, 406);
		}
		setMessageList(0x004B8D28);
	} else {
		// Klaymen entering from the left
		insertKlaymen<KmScene1404>(30, 406);
		setMessageList(0x004B8C38);
	}

	if (getGlobalVar(V_PROJECTOR_LOCATION) == 3) {
		_asProjector = insertSprite<AsCommonProjector>(this, _klaymen, (Sprite*)NULL);
		addCollisionSprite(_asProjector);
		if (getGlobalVar(V_PROJECTOR_SLOT) == 0) {
			sendEntityMessage(_klaymen, 0x1014, _asProjector);
			_klaymen->setX(_asProjector->getX() - 100);
			_klaymen->updateBounds();
			setMessageList(0x004B8CB8);
		}
		_asProjector->setClipRect(_sprite1->getDrawRect().x, 0, 640, 480);
	}

	_klaymen->setClipRect(_sprite1->getDrawRect().x, 0, 640, 480);

}

Scene1404::~Scene1404() {
	setGlobalVar(V_KLAYMEN_IS_DELTA_X, _klaymen->isDoDeltaX() ? 1 : 0);
}

uint32 Scene1404::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x410650C2) {
			if (_asProjector && _asProjector->getX() == 220)
				setMessageList(0x004B8C40);
			else
				setMessageList(0x004B8CE8);
		}
		break;
	case NM_SCENE_LEAVE:
		leaveScene(0);
		break;
	case 0x4826:
		if (sender == _asProjector) {
			if (sendMessage(_asProjector, 0x480C, _klaymen->getX() > _asProjector->getX() ? 1 : 0) != 0) {
				sendEntityMessage(_klaymen, 0x1014, _asProjector);
				setMessageList2(0x004B8CA0);
			} else
				setMessageList2(0x004B8C40);
		} else if (sender == _asTape && _messageListStatus != 2) {
			sendEntityMessage(_klaymen, 0x1014, _asTape);
			setMessageList(0x004B8CD0);
		} else if (sender == _asKey && _messageListStatus != 2) {
			sendEntityMessage(_klaymen, 0x1014, _asKey);
			setMessageList(0x004B8D18);
		}
		break;
	default:
		break;
	}
	return 0;
}

Scene1405::Scene1405(NeverhoodEngine *vm, Module *parentModule)
	: Scene(vm, parentModule), _selectFirstTile(true), _tilesLeft(48), _countdown(0) {

	_vm->gameModule()->initMemoryPuzzle();

	SetUpdateHandler(&Scene1405::update);
	SetMessageHandler(&Scene1405::handleMessage);

	setBackground(0x0C0C007D);
	setPalette(0x0C0C007D);
	insertPuzzleMouse(0xC00790C8, 20, 620);

	for (uint32 tileIndex = 0; tileIndex < 48; tileIndex++) {
		_tiles[tileIndex] = insertSprite<AsScene1405Tile>(this, tileIndex);
		addCollisionSprite(_tiles[tileIndex]);
		if (getSubVar(VA_IS_TILE_MATCH, tileIndex))
			_tilesLeft--;
	}

	loadSound(0, 0x68E25540);
}

void Scene1405::update() {
	Scene::update();
	// Check if the player chose a wrong tile, in which case the whole grid gets reset
	if (_countdown != 0 && (--_countdown == 0)) {
		_tilesLeft = 48;
		_tiles[_firstTileIndex]->hide(true);
		_tiles[_secondTileIndex]->hide(false);
		for (uint32 i = 0; i < 48; i++) {
			if (getSubVar(VA_IS_TILE_MATCH, i)) {
				_tiles[i]->hide(false);
				setSubVar(VA_IS_TILE_MATCH, i, 0);
			}
		}
	}
}

uint32 Scene1405::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_MOUSE_CLICK:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620)
			leaveScene(0);
		break;
	case NM_ANIMATION_UPDATE:
		if (_selectFirstTile) {
			_firstTileIndex = param.asInteger();
			_selectFirstTile = false;
		} else {
			_secondTileIndex = param.asInteger();
			if (_firstTileIndex != _secondTileIndex) {
				_selectFirstTile = true;
				if (getSubVar(VA_TILE_SYMBOLS, _secondTileIndex) == getSubVar(VA_TILE_SYMBOLS, _firstTileIndex)) {
					setSubVar(VA_IS_TILE_MATCH, _firstTileIndex, 1);
					setSubVar(VA_IS_TILE_MATCH, _secondTileIndex, 1);
					_tilesLeft -= 2;
					if (_tilesLeft == 0)
						playSound(0);
				} else
					_countdown = 10;
			}
		}
		break;
	default:
		break;
	}
	return 0;
}

} // End of namespace Neverhood
