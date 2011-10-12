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

#include "neverhood/module2700.h"
#include "neverhood/gamemodule.h"
#include "neverhood/module1000.h"

namespace Neverhood {

static const NRect kScene2710ClipRect = NRect(0, 0, 626, 480);

static const uint32 kScene2710StaticSprites[] = {
	0x0D2016C0,
	0
};

static const NRect kScene2711ClipRect = NRect(0, 0, 521, 480);

static const uint32 kScene2711FileHashes1[] = {
	0,
	0x100801A1,
	0x201081A0,
	0x006800A4,
	0x40390120,
	0x000001B1,
	0x001000A1,
	0
};

static const uint32 kScene2711FileHashes2[] = {
	0,
	0x40403308,
	0x71403168,
	0x80423928,
	0x224131A8,
	0x50401328,
	0x70423328,
	0
};

static const uint32 kScene2711FileHashes3[] = {
	0,
	0x1088A021,
	0x108120E5,
	0x18A02321,
	0x148221A9,
	0x10082061,
	0x188820E1,
	0
};

static const NRect kScene2724ClipRect = NRect(0, 141, 640, 480);

static const uint32 kScene2724StaticSprites[] = {
	0xC20D00A5,
	0
};

static const NRect kScene2725ClipRect = NRect(0, 0, 640, 413);

static const uint32 kScene2725StaticSprites[] = {
	0xC20E00A5,
	0
};

Module2700::Module2700(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule), _soundResource1(vm), _soundResource2(vm),
	_soundResource3(vm), _soundResource4(vm), _soundIndex(0), _flag1(false) {
	
	// TODO Music18hList_add(0x42212411, 0x04020210);
	// TODO Music18hList_play(0x04020210, 24, 2, 1);
	SetMessageHandler(&Module2700::handleMessage);


	if (which < 0) {
		which = _vm->gameState().which;
		if (_vm->gameState().sceneNum == 0 || _vm->gameState().sceneNum == 30 || _vm->gameState().sceneNum == 31)
			which = -1;
		createScene(_vm->gameState().sceneNum, which);
	} else {
		createScene(0, 0);
	}

	_soundResource1.load(0x00880CCC);
	_soundResource2.load(0x00880CC0);
	_soundResource3.load(0x00880CCC);
	_soundResource4.load(0x00880CC0);

}

Module2700::~Module2700() {
	// TODO Sound1ChList_sub_407A50(0x42212411);
}

void Module2700::createScene(int sceneNum, int which) {
	debug("Module2700::createScene(%d, %d)", sceneNum, which);
	if (sceneNum != 30 && sceneNum != 31)
		_vm->gameState().which = which;
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
//TODO		_childObject = new Scene2701(_vm, this, which);
		break;
	case 1:
//TODO		_childObject = new Scene2702(_vm, this, which);
		break;
	case 2:
		if (which == 6 || which == 7)
			createScene2703(which, 0x004B1710);
		else if (which == 4 || which == 5)
			createScene2703(which, 0x004B1738);
		else if (which == 2 || which == 3)
			createScene2703(which, 0x004B1760);
		else
			createScene2703(which, 0x004B1788);
		break;
	case 3:
		createScene2704(which, 0x004B17B0, 150);
		break;
	case 4:
		createScene2704(which, 0x004B17D8, 150);
		break;
	case 5:
		if (which >= 4) {
//TODO		_childObject = new Scene2706(_vm, this, which);
		} else if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1828, 150);
		} else {
			createScene2704(which, 0x004B1800, 150);
		}
		break;
	case 6:
		createScene2704(which, 0x004B1850, 150);
		break;
	case 7:
		if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1878, 150);
		} else {
			createScene2704(which, 0x004B18A0, 150);
		}
		break;
	case 8:
		if (which == 2 || which == 3) {
			createScene2704(which, 0x004B18C8, 150);
		} else {
			createScene2704(which, 0x004B18F0, 150);
		}
		break;
	case 9:
		createScene2704(which, 0x004B18F0, 150, kScene2710StaticSprites, &kScene2710ClipRect);
		break;
	case 10:
		// TODO _vm->gameModule()->initScene2808Vars2();
		_scene2711StaticSprites[0] = kScene2711FileHashes1[getSubVar(0x40005834, 2)];
		_scene2711StaticSprites[1] = kScene2711FileHashes2[getSubVar(0x40005834, 1)];
		_scene2711StaticSprites[2] = kScene2711FileHashes3[getSubVar(0x40005834, 0)];
		_scene2711StaticSprites[3] = 0x0261282E;
		_scene2711StaticSprites[4] = 0x9608E5A0;
		_scene2711StaticSprites[5] = 0;
		createScene2704(which, 0x004B1950, 150, _scene2711StaticSprites, &kScene2711ClipRect);
		break;
	case 11:
		createScene2704(which, 0x004B19E0, 150);
		break;
	case 12:
		createScene2704(which, 0x004B1A08, 150);
		break;
	case 13:
		createScene2704(which, 0x004B1A30, 150);
		break;
	case 14:
		if (which == 4 || which == 5) {
			createScene2704(which, 0x004B1A58, 150);
		} else if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1A80, 150);
		} else {
			createScene2704(which, 0x004B1AA8, 150);
		}
		break;
	case 15:
		if (which == 4 || which == 5) {
			createScene2704(which, 0x004B1AD0, 150);
		} else if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1AF8, 150);
		} else {
			createScene2704(which, 0x004B1B20, 150);
		}
		break;
	case 16:
		if (which == 4 || which == 5) {
			createScene2704(which, 0x004B1B48, 150);
		} else if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1B70, 150);
		} else {
			createScene2704(which, 0x004B1B98, 150);
		}
		break;
	case 17:
		if (which == 4 || which == 5) {
			createScene2704(which, 0x004B1BC0, 150);
		} else if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1BE8, 150);
		} else {
			createScene2704(which, 0x004B1C10, 150);
		}
		break;
	case 18:
		if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1C38, 150);
		} else {
			createScene2704(which, 0x004B1C60, 150);
		}
		break;
	case 19:
		if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1CB0, 150);
		} else {
			createScene2704(which, 0x004B1C88, 150);
		}
		break;
	case 20:
		if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1CD8, 150);
		} else {
			createScene2704(which, 0x004B1D00, 150);
		}
		break;
	case 21:
		createScene2704(which, 0x004B1D28, 150);
		break;
	case 22:
		createScene2704(which, 0x004B1D50, 150);
		break;
	case 23:
		createScene2704(which, 0x004B1D78, 150, kScene2724StaticSprites, &kScene2724ClipRect);
		break;
	case 24:
		createScene2704(which, 0x004B1DB0, 150, kScene2725StaticSprites, &kScene2725ClipRect);
		break;
	case 25:
		createScene2704(which, 0x004B1DE8, 150);
		break;
	case 26:
		createScene2704(which, 0x004B1E10, 150);
		break;
	case 27:
		createScene2704(which, 0x004B1E38, 150);
		break;
	case 28:
		createScene2704(which, 0x004B1E60, 150);
		break;
	case 30:
		_childObject = new Class152(_vm, this, 0x09507248, 0x0724C09D);
		break;
	case 31:
//TODO		_childObject = new Scene2732(_vm, this, which);
		break;
	}
	SetUpdateHandler(&Module2700::updateScene);
	_childObject->handleUpdate();
}

void Module2700::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			if (_moduleResult == 1) {
				createScene(1, 3);
			} else {
				leaveModule(0);
			}
			break;
		}
	}
}

void Module2700::update() {
}

uint32 Module2700::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Module::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x200D:
		switch (_soundIndex) {
		case 0:
			_soundResource1.play();
			break;
		case 1:
			_soundResource2.play();
			break;
		case 2:
			_soundResource3.play();
			break;
		case 3:
			_soundResource4.play();
			break;
		}
		_soundIndex++;
		if (_soundIndex >= 4)
			_soundIndex = 0;
		break;
	}
	return messageResult;
}
			
void Module2700::createScene2703(int which, uint32 sceneInfoId, const uint32 *staticSprites, const NRect *clipRect) {
	// TODO
}

void Module2700::createScene2704(int which, uint32 sceneInfoId, int16 value, const uint32 *staticSprites, const NRect *clipRect) {
	// TODO
}

} // End of namespace Neverhood
