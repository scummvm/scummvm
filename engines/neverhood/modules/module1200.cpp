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

#include "neverhood/modules/module1200.h"
#include "neverhood/modules/module1200_sprites.h"

namespace Neverhood {

Module1200::Module1200(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	SetMessageHandler(&Module1200::handleMessage);

	if (which < 0)
		createScene(_vm->gameState().sceneNum, -1);
	else if (which == 1)
		createScene(0, 2);
	else
		createScene(0, 0);

	_vm->_soundMan->addMusic(0x00478311, 0x62222CAE);
	_vm->_soundMan->startMusic(0x62222CAE, 0, 0);
}

Module1200::~Module1200() {
	_vm->_soundMan->deleteMusicGroup(0x00478311);
}

void Module1200::createScene(int sceneNum, int which) {
	debug(1, "Module1200::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		_childObject = new Scene1201(_vm, this, which);
		break;
	case 1:
		_vm->gameState().sceneNum = 1;
		_childObject = new Scene1202(_vm, this);
		break;
	case 2:
		_vm->gameState().sceneNum = 2;
		_vm->_soundMan->stopMusic(0x62222CAE, 0, 0);
		createSmackerScene(0x31890001, true, true, false);
		setGlobalVar(V_SEEN_CREATURE_EXPLODE_VID, 1);
		break;
	default:
		break;
	}
	SetUpdateHandler(&Module1200::updateScene);
	_childObject->handleUpdate();
}

void Module1200::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 0:
			if (_moduleResult == 1)
				createScene(1, 0);
			else if (_moduleResult == 2) {
				if (getGlobalVar(V_CREATURE_EXPLODED) && !getGlobalVar(V_SEEN_CREATURE_EXPLODE_VID))
					createScene(2, -1);
				else
					leaveModule(1);
			} else
				leaveModule(0);
			break;
		case 1:
			createScene(0, 1);
			break;
		case 2:
			_vm->_soundMan->startMusic(0x62222CAE, 0, 0);
			createScene(0, 3);
			break;
		default:
			break;
		}
	}
}

static const uint32 kScene1201InitArray[] = {
	1, 0, 2, 4, 5, 3, 6, 7, 8, 10, 9, 11, 13, 14, 12, 16, 17, 15
};

Scene1201::Scene1201(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _creatureExploded(false), _asMatch(NULL), _asTntMan(NULL),
	_asCreature(NULL), _asTntManRope(NULL), _asLeftDoor(NULL), _asRightDoor(NULL), _asTape(NULL) {

	int16 topY1, topY2, topY3, topY4;
	int16 x1, x2;
	Sprite *tempSprite;

	SetUpdateHandler(&Scene1201::update);
	SetMessageHandler(&Scene1201::handleMessage);

	setHitRects(0x004AEBD0);

	if (!getSubVar(VA_IS_PUZZLE_INIT, 0xE8058B52)) {
		setSubVar(VA_IS_PUZZLE_INIT, 0xE8058B52, 1);
		for (uint32 index = 0; index < 18; index++)
			setSubVar(VA_TNT_POSITIONS, index, kScene1201InitArray[index]);
	}

	insertScreenMouse(0x9A2C0409);

	_asTape = insertSprite<AsScene1201Tape>(this, 3, 1100, 243, 340, 0x9148A011);
	addCollisionSprite(_asTape);

	tempSprite = insertStaticSprite(0x03C82530, 100);
	topY1 = tempSprite->getY() + tempSprite->getDrawRect().height;

	tempSprite = insertStaticSprite(0x88182069, 200);
	topY2 = tempSprite->getY() + tempSprite->getDrawRect().height;

	tempSprite = insertStaticSprite(0x476014E0, 300);
	topY3 = tempSprite->getY() + tempSprite->getDrawRect().height;

	tempSprite = insertStaticSprite(0x04063110, 500);
	topY4 = tempSprite->getY() + 1;

	_asTntManRope = insertSprite<AsScene1201TntManRope>(getGlobalVar(V_TNT_DUMMY_BUILT) && which != 1);
	_asTntManRope->setClipRect(0, topY4, 640, 480);

	insertStaticSprite(0x400B04B0, 1200);

	tempSprite = insertStaticSprite(0x40295462, 1200);
	x1 = tempSprite->getX();

	tempSprite = insertStaticSprite(0xA29223FA, 1200);
	x2 = tempSprite->getX() + tempSprite->getDrawRect().width;

	_asKlaymenHead = insertSprite<AsScene1201KlaymenHead>();

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene1201>(364, 333);
		setMessageList(0x004AEC08);
	} else if (which == 3) {
		// Klaymen standing after the weasel exploded
		insertKlaymen<KmScene1201>(400, 329);
		setMessageList(0x004AEC08);
	} else if (which == 2) {
		// Klaymen entering from the right
		if (getGlobalVar(V_CREATURE_ANGRY) && !getGlobalVar(V_CREATURE_EXPLODED)) {
			insertKlaymen<KmScene1201>(374, 333);
			setMessageList(0x004AEC08);
		} else {
			insertKlaymen<KmScene1201>(640, 329);
			setMessageList(0x004AEC20);
		}
	} else if (which == 1) {
		// Klaymen returning from the TNT console
		if (getGlobalVar(V_KLAYMEN_IS_DELTA_X)) {
			insertKlaymen<KmScene1201>(364, 333);
			_klaymen->setDoDeltaX(1);
		} else {
			insertKlaymen<KmScene1201>(246, 333);
		}
		setMessageList(0x004AEC30);
	} else {
		// Klaymen entering from the left
		insertKlaymen<KmScene1201>(0, 336);
		setMessageList(0x004AEC10);
	}

	_klaymen->setClipRect(x1, 0, x2, 480);
	_klaymen->setRepl(64, 0);

	if (getGlobalVar(V_CREATURE_ANGRY) && !getGlobalVar(V_CREATURE_EXPLODED)) {
		setBackground(0x4019A2C4);
		setPalette(0x4019A2C4);
		_asRightDoor = NULL;
	} else {
		setBackground(0x40206EC5);
		setPalette(0x40206EC5);
		_asRightDoor = insertSprite<AsScene1201RightDoor>(_klaymen, which == 2);
	}

	if (getGlobalVar(V_TNT_DUMMY_BUILT)) {
		insertStaticSprite(0x10002ED8, 500);
		if (!getGlobalVar(V_CREATURE_EXPLODED)) {
			_asTntMan = insertSprite<AsScene1201TntMan>(this, _asTntManRope, which == 1);
			_asTntMan->setClipRect(x1, 0, x2, 480);
			_asTntMan->setRepl(64, 0);
			addCollisionSprite(_asTntMan);
			tempSprite = insertSprite<AsScene1201TntManFlame>(_asTntMan);
			tempSprite->setClipRect(x1, 0, x2, 480);
		}

		uint32 tntIndex = 1;
		while (tntIndex < 18) {
			uint32 elemIndex = getSubVar(VA_TNT_POSITIONS, tntIndex);
			int16 clipY2;
			if (kScene1201PointArray[elemIndex].y < 175)
				clipY2 = topY1;
			else if (kScene1201PointArray[elemIndex].y < 230)
				clipY2 = topY2;
			else
				clipY2 = topY3;
			insertSprite<SsScene1201Tnt>(tntIndex, getSubVar(VA_TNT_POSITIONS, tntIndex), clipY2);
			elemIndex = getSubVar(VA_TNT_POSITIONS, tntIndex + 1);
			if (kScene1201PointArray[elemIndex].y < 175)
				clipY2 = topY1;
			else if (kScene1201PointArray[elemIndex].y < 230)
				clipY2 = topY2;
			else
				clipY2 = topY3;
			insertSprite<SsScene1201Tnt>(tntIndex + 1, getSubVar(VA_TNT_POSITIONS, tntIndex + 1), clipY2);
			tntIndex += 3;
		}

		if (getGlobalVar(V_CREATURE_ANGRY) && !getGlobalVar(V_CREATURE_EXPLODED)) {
			setRectList(0x004AEE58);
		} else {
			setRectList(0x004AEDC8);
		}

	} else {

		insertStaticSprite(0x8E8A1981, 900);

		uint32 tntIndex = 0;
		while (tntIndex < 18) {
			uint32 elemIndex = getSubVar(VA_TNT_POSITIONS, tntIndex);
			int16 clipY2;
			if (kScene1201PointArray[elemIndex].x < 300) {
				clipY2 = 480;
			} else {
				if (kScene1201PointArray[elemIndex].y < 175)
					clipY2 = topY1;
				else if (kScene1201PointArray[elemIndex].y < 230)
					clipY2 = topY2;
				else
					clipY2 = topY3;
			}
			insertSprite<SsScene1201Tnt>(tntIndex, getSubVar(VA_TNT_POSITIONS, tntIndex), clipY2);
			tntIndex++;
		}

		if (getGlobalVar(V_CREATURE_ANGRY) && !getGlobalVar(V_CREATURE_EXPLODED))
			setRectList(0x004AEE18);
		else
			setRectList(0x004AED88);

	}

	tempSprite = insertStaticSprite(0x63D400BC, 900);

	_asLeftDoor = insertSprite<AsScene1201LeftDoor>(_klaymen);
	_asLeftDoor->setClipRect(x1, tempSprite->getDrawRect().y, tempSprite->getDrawRect().x2(), 480);

	if (getGlobalVar(V_CREATURE_ANGRY) && getGlobalVar(V_MATCH_STATUS) == 0)
		setGlobalVar(V_MATCH_STATUS, 1);

	_asMatch = NULL;

	if (getGlobalVar(V_MATCH_STATUS) < 3) {
		_asMatch = insertSprite<AsScene1201Match>(this);
		addCollisionSprite(_asMatch);
	}

	if (getGlobalVar(V_CREATURE_ANGRY) && getGlobalVar(V_CREATURE_EXPLODED) == 0) {
		_asCreature = insertSprite<AsScene1201Creature>(this, _klaymen);
		_asCreature->setClipRect(x1, 0, x2, 480);
	}

}

Scene1201::~Scene1201() {
	if (_creatureExploded)
		setGlobalVar(V_CREATURE_EXPLODED, 1);
	setGlobalVar(V_KLAYMEN_IS_DELTA_X, _klaymen->isDoDeltaX() ? 1 : 0);
}

void Scene1201::update() {
	Scene::update();
	if (_asMatch && getGlobalVar(V_MATCH_STATUS) == 3)
		deleteSprite(&_asMatch);
}

uint32 Scene1201::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x07053000) {
			_creatureExploded = true;
			sendMessage(_asCreature, 0x2004, 0);
		} else if (param.asInteger() == 0x140E5744)
			sendMessage(_asCreature, NM_KLAYMEN_CLIMB_LADDER, 0);
		else if (param.asInteger() == 0x40253C40) {
			_canAcceptInput = false;
			sendMessage(_asCreature, NM_KLAYMEN_STOP_CLIMBING, 0);
		} else if (param.asInteger() == 0x090EB048) {
			if (_klaymen->getX() < 572)
				setMessageList2(0x004AEC90);
			else
				setMessageList2(0x004AEC20);
		}
		break;
	case 0x2001:
		if (getGlobalVar(V_MATCH_STATUS) == 0)
			setMessageList2(0x004AECB0);
		else {
			sendEntityMessage(_klaymen, 0x1014, _asMatch);
			setMessageList2(0x004AECC0);
		}
		break;
	case NM_POSITION_CHANGE:
		if (getGlobalVar(V_TNT_DUMMY_FUSE_LIT)) {
			// Move the TNT dummy if the fuse is burning
			sendEntityMessage(_klaymen, 0x1014, _asTntMan);
			setMessageList2(0x004AECF0, false);
		} else if (getGlobalVar(V_MATCH_STATUS) == 3) {
			// Light the TNT dummy if we have the match
			sendEntityMessage(_klaymen, 0x1014, _asTntMan);
			if (_klaymen->getX() > _asTntMan->getX())
				setMessageList(0x004AECD0);
			else
				setMessageList(0x004AECE0);
		}
		break;
	case 0x4814:
		cancelMessageList();
		break;
	case 0x4826:
		if (sender == _asTape) {
			sendEntityMessage(_klaymen, 0x1014, _asTape);
			setMessageList(0x004AED38);
		}
		break;
	case 0x4829:
		sendMessage(_asRightDoor, 0x4829, 0);
		break;
	case 0x8000:
		sendMessage(_asKlaymenHead, NM_KLAYMEN_STOP_CLIMBING, 0);
		break;
	default:
		break;
	}
	return messageResult;
}

static const uint32 kScene1202Table[] = {
	1, 2, 0, 4, 5, 3, 7, 8, 6, 10, 11, 9, 13, 14, 12, 16, 17, 15
};

Scene1202::Scene1202(NeverhoodEngine *vm, Module *parentModule)
	: Scene(vm, parentModule), _paletteResource(vm),
	_soundToggle(true), _isPuzzleSolved(false), _counter(0), _clickedIndex(-1) {

	SetMessageHandler(&Scene1202::handleMessage);
	SetUpdateHandler(&Scene1202::update);

	setBackground(0x60210ED5);
	setPalette(0x60210ED5);
	addEntity(_palette);

	_paletteResource.load(0x60250EB5);
	_paletteResource.copyPalette(_paletteData);

	insertPuzzleMouse(0x10ED160A, 20, 620);

	for (int tntIndex = 0; tntIndex < 18; tntIndex++) {
		_asTntItems[tntIndex] = insertSprite<AsScene1202TntItem>(this, tntIndex);
		addCollisionSprite(_asTntItems[tntIndex]);
	}

	insertStaticSprite(0x8E8419C1, 1100);

	if (getGlobalVar(V_TNT_DUMMY_BUILT))
		SetMessageHandler(&Scene1202::hmSolved);

	playSound(0, 0x40106542);
	loadSound(1, 0x40005446);
	loadSound(2, 0x40005446); // Same sound as slot 1
	loadSound(3, 0x68E25540);

}

Scene1202::~Scene1202() {
	if (isSolved())
		setGlobalVar(V_TNT_DUMMY_BUILT, 1);
}

void Scene1202::update() {
	Scene::update();
	if (_isPuzzleSolved) {
		if (!isSoundPlaying(3))
			leaveScene(0);
	} else if (_counter == 0 && isSolved()) {
		_clickedIndex = 0;
		SetMessageHandler(&Scene1202::hmSolved);
		setGlobalVar(V_TNT_DUMMY_BUILT, 1);
		_palette->copyToBasePalette(_paletteData);
		_palette->startFadeToPalette(24);
		playSound(3);
		_isPuzzleSolved = true;
	} else if (_clickedIndex >= 0 && _counter == 0) {
		// Swap TNT positions
		int destIndex = kScene1202Table[_clickedIndex];
		sendMessage(_asTntItems[_clickedIndex], 0x2001, getSubVar(VA_TNT_POSITIONS, destIndex));
		sendMessage(_asTntItems[destIndex], 0x2001, getSubVar(VA_TNT_POSITIONS, _clickedIndex));
		int temp = getSubVar(VA_TNT_POSITIONS, destIndex);
		setSubVar(VA_TNT_POSITIONS, destIndex, getSubVar(VA_TNT_POSITIONS, _clickedIndex));
		setSubVar(VA_TNT_POSITIONS, _clickedIndex, temp);
		_counter = 2;
		_clickedIndex = -1;
		playSound(_soundToggle ? 1 : 2);
		_soundToggle = !_soundToggle;
	}
}

uint32 Scene1202::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_MOUSE_CLICK:
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) && !_isPuzzleSolved)
			leaveScene(0);
		break;
	case NM_ANIMATION_UPDATE:
		_clickedIndex = (int)param.asInteger();
		break;
	case NM_POSITION_CHANGE:
		_counter--;
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 Scene1202::hmSolved(int messageNum, const MessageParam &param, Entity *sender) {
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

bool Scene1202::isSolved() {
	return
		getSubVar(VA_TNT_POSITIONS,  0) ==  0 && getSubVar(VA_TNT_POSITIONS,  3) ==  3 &&
		getSubVar(VA_TNT_POSITIONS,  6) ==  6 && getSubVar(VA_TNT_POSITIONS,  9) ==  9 &&
		getSubVar(VA_TNT_POSITIONS, 12) == 12 && getSubVar(VA_TNT_POSITIONS, 15) == 15;
}

} // End of namespace Neverhood
