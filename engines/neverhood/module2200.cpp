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

#include "neverhood/module2200.h"
#include "neverhood/module1000.h"
#include "neverhood/module1200.h"
#include "neverhood/gamemodule.h"
#include "neverhood/diskplayerscene.h"

namespace Neverhood {

Module2200::Module2200(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	debug("Create Module2200(%d)", which);

	// TODO: Music18hList_add(0x11391412, 0x601C908C); 

	if (which < 0) {
		switch (_vm->gameState().sceneNum) {
		default:
			createScene2201(-1);
			break;
		case 1:
			createScene2202(-1);
			break;
		case 2:
			createScene2203(-1);
			break;
		case 3:
			createScene2204(-1);
			break;
		case 4:
			createScene2205(-1);
			break;
		case 5:
			createScene2206(-1);
			break;
		case 6:
			createScene2207(-1);
			break;
		case 7:
			createScene2208(-1);
			break;
		case 8:
			createScene2209(-1);
			break;
		case 9:
			createScene2210(-1);
			break;
		case 10:
			createScene2211(-1);
			break;
		case 11:
			createScene2212(-1);
			break;
		case 12:
			createScene2213(-1);
			break;
		case 13:
			createScene2214(-1);
			break;
		case 14:
			createScene2215(-1);
			break;
		case 15:
			createScene2216(-1);
			break;
		case 16:
			createScene2217(-1);
			break;
		case 17:
			createScene2218(-1);
			break;
		case 18:
			createScene2219(-1);
			break;
		case 19:
			createScene2220(-1);
			break;
		case 20:
			createScene2221(-1);
			break;
		case 21:
			createScene2222(-1);
			break;
		case 22:
			createScene2223(-1);
			break;
		case 23:
			createScene2224(-1);
			break;
		case 24:
			createScene2225(-1);
			break;
		case 25:
			createScene2226(-1);
			break;
		case 26:
			createScene2227(-1);
			break;
		case 27:
			createScene2228(-1);
			break;
		case 28:
			createScene2229(-1);
			break;
		case 29:
			createScene2230(-1);
			break;
		case 30:
			createScene2231(-1);
			break;
		case 31:
			createScene2232(-1);
			break;
		case 32:
			createScene2233(-1);
			break;
		case 33:
			createScene2234(-1);
			break;
		case 34:
			createScene2235(-1);
			break;
		case 35:
			createScene2236(-1);
			break;
		case 36:
			createScene2237(-1);
			break;
		case 37:
			createScene2238(-1);
			break;
		case 38:
			createScene2239(-1);
			break;
		case 39:
			createScene2240(-1);
			break;
		case 40:
			createScene2241(-1);
			break;
		case 41:
			createScene2242(-1);
			break;
		case 42:
			createScene2243(-1);
			break;
		case 43:
			createScene2244(-1);
			break;
		case 44:
			createScene2245(-1);
			break;
		case 45:
			createScene2246(-1);
			break;
		case 46:
			createScene2247(-1);
			break;
		case 47:
			createScene2248(-1);
			break;
		}
	} else {
		createScene2201(0);
	}

}

Module2200::~Module2200() {
	// TODO Sound1ChList_sub_407A50(0x11391412);
}

void Module2200::createScene2201(int which) {
	_vm->gameState().sceneNum = 0;
	_childObject = new Scene2201(_vm, this, which);
	SetUpdateHandler(&Module2200::updateScene2201);
}

void Module2200::createScene2202(int which) {
	// TODO Music18hList_play(0x601C908C, 0, 2, 1);
	_vm->gameState().sceneNum = 1;
	_childObject = new Scene2202(_vm, this, which);
	SetUpdateHandler(&Module2200::updateScene2202);
}

void Module2200::createScene2203(int which) {
	// TODO Music18hList_play(0x601C908C, 0, 2, 1);
	_vm->gameState().sceneNum = 2;
	_childObject = new Scene2203(_vm, this, which);
	SetUpdateHandler(&Module2200::updateScene2203);
}

void Module2200::createScene2204(int which) {
	_vm->gameState().sceneNum = 3;
	// TODO Music18hList_stop(0x601C908C, 0, 2);
	_childObject = new DiskplayerScene(_vm, this, 3);
	SetUpdateHandler(&Module2200::updateScene2204);
}
			
void Module2200::createScene2205(int which) {
	_vm->gameState().sceneNum = 4;
	// TODO Music18hList_stop(0x601C908C, 0, 2);
	_childObject = new Scene2205(_vm, this, which);
	SetUpdateHandler(&Module2200::updateScene2205);
}
			
void Module2200::createScene2206(int which) {
	_vm->gameState().sceneNum = 5;
	// TODO Music18hList_stop(0x601C908C, 0, 2);
	_childObject = new Scene2206(_vm, this, which);
	SetUpdateHandler(&Module2200::updateScene2206);
}
			
void Module2200::createScene2207(int which) {
	_vm->gameState().sceneNum = 6;
	_childObject = new Scene2207(_vm, this, which);
	SetUpdateHandler(&Module2200::updateScene2207);
}
			
void Module2200::createScene2208(int which) {
	if (which >= 0)
		_vm->gameState().which = _vm->gameState().sceneNum; 
	_vm->gameState().sceneNum = 7;
	_childObject = new Scene2208(_vm, this, which);
	SetUpdateHandler(&Module2200::updateScene2208);
}
			
void Module2200::createScene2209(int which) {
	_vm->gameState().sceneNum = 8;
	_childObject = new Scene2208(_vm, this, which);
	SetUpdateHandler(&Module2200::updateScene2209);
}
			
void Module2200::createScene2210(int which) {
	_vm->gameState().sceneNum = 9;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7180);
	SetUpdateHandler(&Module2200::updateScene2210);
}
			
void Module2200::createScene2211(int which) {
	_vm->gameState().sceneNum = 10;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7198);
	SetUpdateHandler(&Module2200::updateScene2211);
}
			
void Module2200::createScene2212(int which) {
	_vm->gameState().sceneNum = 11;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B71B0);
	SetUpdateHandler(&Module2200::updateScene2212);
}
			
void Module2200::createScene2213(int which) {
	_vm->gameState().sceneNum = 12;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B71C8);
	SetUpdateHandler(&Module2200::updateScene2213);
}

void Module2200::createScene2214(int which) {
	_vm->gameState().sceneNum = 13;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B71E0);
	SetUpdateHandler(&Module2200::updateScene2214);
}
			
void Module2200::createScene2215(int which) {
	_vm->gameState().sceneNum = 14;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B71F8);
	SetUpdateHandler(&Module2200::updateScene2215);
}
			
void Module2200::createScene2216(int which) {
	_vm->gameState().sceneNum = 15;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7210);
	SetUpdateHandler(&Module2200::updateScene2216);
}
			
void Module2200::createScene2217(int which) {
	_vm->gameState().sceneNum = 16;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7228);
	SetUpdateHandler(&Module2200::updateScene2217);
}
			
void Module2200::createScene2218(int which) {
	_vm->gameState().sceneNum = 17;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7240);
	SetUpdateHandler(&Module2200::updateScene2218);
}
			
void Module2200::createScene2219(int which) {
	_vm->gameState().sceneNum = 18;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7258);
	SetUpdateHandler(&Module2200::updateScene2219);
}
			
void Module2200::createScene2220(int which) {
	_vm->gameState().sceneNum = 19;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7270);
	SetUpdateHandler(&Module2200::updateScene2220);
}
			
void Module2200::createScene2221(int which) {
	_vm->gameState().sceneNum = 20;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7288);
	SetUpdateHandler(&Module2200::updateScene2221);
}
			
void Module2200::createScene2222(int which) {
	_vm->gameState().sceneNum = 21;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B72A0);
	SetUpdateHandler(&Module2200::updateScene2222);
}
			
void Module2200::createScene2223(int which) {
	_vm->gameState().sceneNum = 22;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B72B8);
	SetUpdateHandler(&Module2200::updateScene2223);
}
			
void Module2200::createScene2224(int which) {
	_vm->gameState().sceneNum = 23;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B72D0);
	SetUpdateHandler(&Module2200::updateScene2224);
}
			
void Module2200::createScene2225(int which) {
	_vm->gameState().sceneNum = 24;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B72E8);
	SetUpdateHandler(&Module2200::updateScene2225);
}
			
void Module2200::createScene2226(int which) {
	_vm->gameState().sceneNum = 25;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7300);
	SetUpdateHandler(&Module2200::updateScene2226);
}
			
void Module2200::createScene2227(int which) {
	_vm->gameState().sceneNum = 26;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7318);
	SetUpdateHandler(&Module2200::updateScene2227);
}
			
void Module2200::createScene2228(int which) {
	_vm->gameState().sceneNum = 27;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7330);
	SetUpdateHandler(&Module2200::updateScene2228);
}
			
void Module2200::createScene2229(int which) {
	_vm->gameState().sceneNum = 28;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7348);
	SetUpdateHandler(&Module2200::updateScene2229);
}
			
void Module2200::createScene2230(int which) {
	_vm->gameState().sceneNum = 29;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7360);
	SetUpdateHandler(&Module2200::updateScene2230);
}
			
void Module2200::createScene2231(int which) {
	_vm->gameState().sceneNum = 30;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7378);
	SetUpdateHandler(&Module2200::updateScene2231);
}
			
void Module2200::createScene2232(int which) {
	_vm->gameState().sceneNum = 31;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7390);
	SetUpdateHandler(&Module2200::updateScene2232);
}
			
void Module2200::createScene2233(int which) {
	_vm->gameState().sceneNum = 32;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B73A8);
	SetUpdateHandler(&Module2200::updateScene2233);
}
			
void Module2200::createScene2234(int which) {
	_vm->gameState().sceneNum = 33;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B73C0);
	SetUpdateHandler(&Module2200::updateScene2234);
}
			
void Module2200::createScene2235(int which) {
	_vm->gameState().sceneNum = 34;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B73D8);
	SetUpdateHandler(&Module2200::updateScene2235);
}
			
void Module2200::createScene2236(int which) {
	_vm->gameState().sceneNum = 35;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B73F0);
	SetUpdateHandler(&Module2200::updateScene2236);
}
			
void Module2200::createScene2237(int which) {
	_vm->gameState().sceneNum = 36;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7408);
	SetUpdateHandler(&Module2200::updateScene2237);
}
			
void Module2200::createScene2238(int which) {
	_vm->gameState().sceneNum = 37;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7420);
	SetUpdateHandler(&Module2200::updateScene2238);
}
			
void Module2200::createScene2239(int which) {
	_vm->gameState().sceneNum = 38;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7438);
	SetUpdateHandler(&Module2200::updateScene2239);
}
			
void Module2200::createScene2240(int which) {
	_vm->gameState().sceneNum = 39;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7450);
	SetUpdateHandler(&Module2200::updateScene2240);
}
			
void Module2200::createScene2241(int which) {
	_vm->gameState().sceneNum = 40;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7468);
	SetUpdateHandler(&Module2200::updateScene2241);
}
			
void Module2200::createScene2242(int which) {
	_vm->gameState().sceneNum = 41;
	_childObject = new Scene2242(_vm, this, which);
	SetUpdateHandler(&Module2200::updateScene2242);
}
			
void Module2200::createScene2243(int which) {
	_vm->gameState().sceneNum = 42;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7480);
	SetUpdateHandler(&Module2200::updateScene2243);
}
			
void Module2200::createScene2244(int which) {
	_vm->gameState().sceneNum = 43;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B7498);
	SetUpdateHandler(&Module2200::updateScene2244);
}
			
void Module2200::createScene2245(int which) {
	_vm->gameState().sceneNum = 44;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B74B0);
	SetUpdateHandler(&Module2200::updateScene2245);
}
			
void Module2200::createScene2246(int which) {
	_vm->gameState().sceneNum = 45;
	_childObject = new HallOfRecordsScene(_vm, this, which, 0x004B74C8);
	SetUpdateHandler(&Module2200::updateScene2246);
}
			
void Module2200::createScene2247(int which) {
	_vm->gameState().sceneNum = 46;
	_childObject = new Scene2247(_vm, this, which);
	SetUpdateHandler(&Module2200::updateScene2247);
}
			
void Module2200::createScene2248(int which) {
	_vm->gameState().sceneNum = 47;
	if (!getGlobalVar(0x98109F12)) {
		if (getGlobalVar(0x4D080E54))
			_childObject = new Class152(_vm, this, 0x83110287, 0x10283839);
		else
			_childObject = new Class152(_vm, this, 0x83412B9D, 0x12B9983C);
	} else {
		if (getGlobalVar(0x4D080E54))
			_childObject = new Class152(_vm, this, 0x48632087, 0x3208348E);
		else
			_childObject = new Class152(_vm, this, 0x08C74886, 0x74882084);
	}
	SetUpdateHandler(&Module2200::updateScene2248);
}
			
void Module2200::updateScene2201() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2203(0);
		} else if (_field20 == 2) {
			createScene2202(0);
		} else {
			sendMessage(_parentModule, 0x1009, 0);
		}
	}
}
			
void Module2200::updateScene2202() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		createScene2201(2);
	}
}

void Module2200::updateScene2203() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2205(0);
		} else if (_field20 == 2) {
			createScene2204(0);
		} else {
			createScene2201(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2204() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		createScene2203(2);
	}
}
			
void Module2200::updateScene2205() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2206(0);
		} else if (_field20 == 2) {
			createScene2205(2);
		} else {
			createScene2203(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2206() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2247(0);
		} else if (_field20 == 2) {
			createScene2207(0);
		} else if (_field20 == 3) {
			createScene2209(0);
		} else {
			createScene2205(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2207() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		createScene2206(2);
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2208() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		// This is madness!
		switch (_vm->gameState().which) {
		case 4:
			createScene2205(2);
			break;
		case 9:
			createScene2210(2);
			break;
		case 10:
			createScene2211(2);
			break;
		case 11:
			createScene2212(2);
			break;
		case 12:
			createScene2213(2);
			break;
		case 13:
			createScene2214(2);
			break;
		case 14:
			createScene2215(2);
			break;
		case 15:
			createScene2216(2);
			break;
		case 16:
			createScene2217(2);
			break;
		case 17:
			createScene2218(2);
			break;
		case 18:
			createScene2219(2);
			break;
		case 19:
			createScene2220(2);
			break;
		case 20:
			createScene2221(2);
			break;
		case 21:
			createScene2222(2);
			break;
		case 22:
			createScene2223(2);
			break;
		case 23:
			createScene2224(2);
			break;
		case 24:
			createScene2225(2);
			break;
		case 25:
			createScene2226(2);
			break;
		case 26:
			createScene2227(2);
			break;
		case 27:
			createScene2228(2);
			break;
		case 28:
			createScene2229(2);
			break;
		case 29:
			createScene2230(2);
			break;
		case 30:
			createScene2231(2);
			break;
		case 31:
			createScene2232(2);
			break;
		case 32:
			createScene2233(2);
			break;
		case 33:
			createScene2234(2);
			break;
		case 34:
			createScene2235(2);
			break;
		case 35:
			createScene2236(2);
			break;
		case 36:
			createScene2237(2);
			break;
		case 37:
			createScene2238(2);
			break;
		case 38:
			createScene2239(2);
			break;
		case 39:
			createScene2240(2);
			break;
		case 40:
			createScene2241(2);
			break;
		case 41:
			createScene2242(2);
			break;
		case 42:
			createScene2243(2);
			break;
		case 43:
			createScene2244(2);
			break;
		case 44:
			createScene2245(2);
			break;
		case 46:
			createScene2247(2);
			break;
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2209() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		createScene2206(3);
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2210() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2211(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2247(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2211() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2212(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2210(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2212() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2213(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2211(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2213() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2214(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2212(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2214() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2215(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2213(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2215() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2216(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2214(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2216() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2217(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2215(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2217() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2218(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2216(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2218() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2219(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2217(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2219() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2220(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2218(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2220() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2221(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2219(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2221() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2222(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2220(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2222() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2223(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2221(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2223() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2224(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2222(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2224() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2225(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2223(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2225() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2226(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2224(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2226() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2227(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2225(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2227() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2228(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2226(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2228() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2229(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2227(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2229() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2230(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2228(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2230() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2231(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2229(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2231() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2232(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2230(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2232() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2233(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2231(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2233() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2234(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2232(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2234() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2235(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2233(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2235() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2243(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2234(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2236() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2237(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2246(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2237() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2238(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2236(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2238() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2239(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2237(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2239() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2240(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2238(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2240() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2241(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2239(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2241() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2242(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2240(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2242() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2248(0);
			_childObject->handleUpdate();
		} else if (_field20 == 2) {
			createScene2208(0);
			_childObject->handleUpdate();
		} else {
			createScene2241(1);
			_childObject->handleUpdate();
		}
	}
}
			
void Module2200::updateScene2243() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2244(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2235(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2244() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2245(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2243(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2245() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2246(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2244(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2246() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2236(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2245(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2247() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2210(0);
		} else if (_field20 == 2) {
			createScene2208(0);
		} else {
			createScene2206(1);
		}
		_childObject->handleUpdate();
	}
}
			
void Module2200::updateScene2248() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		createScene2242(1);
		_childObject->handleUpdate();
	}
}

// Scene2201

AsScene2201CeilingFan::AsScene2201CeilingFan(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100) {

	_x = 403;
	_y = 259;
	createSurface(100, 233, 96);
	setFileHash(0x8600866, 0, -1);
	SetUpdateHandler(&AnimatedSprite::update);
}

AsScene2201Door::AsScene2201Door(NeverhoodEngine *vm, Klayman *klayman, Sprite *doorLightSprite, bool flag1)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _klayman(klayman), _doorLightSprite(doorLightSprite), 
	_countdown(0), _doorOpen(flag1) {

	_x = 408;
	_y = 290;	
	createSurface(900, 63, 266);
	SetUpdateHandler(&AsScene2201Door::update);
	SetMessageHandler(&AsScene2201Door::handleMessage);
	if (_doorOpen) {
		setFileHash(0xE2CB0412, -1, -1);
		_countdown = 48;
		_newHashListIndex = -2;
	} else {
		setFileHash(0xE2CB0412, 0, -1);
		_newHashListIndex = 0;
		_doorLightSprite->getSurface()->setVisible(false);
	}
}

void AsScene2201Door::update() {
	if (_countdown != 0 && _doorOpen && (--_countdown == 0)) {
		stCloseDoor();
	}
	AnimatedSprite::update();
}

uint32 AsScene2201Door::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x11001090) {
			if (_doorOpen)
				_doorLightSprite->getSurface()->setVisible(true);
		} else if (param.asInteger() == 0x11283090) {
			if (!_doorOpen)
				_doorLightSprite->getSurface()->setVisible(false);
		}
		break;
	case 0x2000:
		if (_doorOpen)
			_countdown = 144;
		messageResult = _doorOpen ? 1 : 0;
		break;
	case 0x3002:
		removeCallbacks();
		break;
	case 0x4808:
		_countdown = 144;
		if (!_doorOpen)
			stOpenDoor();
		break;
	}
	return messageResult;
}

void AsScene2201Door::stOpenDoor() {
	_doorOpen = true;
	setFileHash(0xE2CB0412, 0, -1);
	_newHashListIndex = -2;
	_soundResource.play(calcHash("fxDoorOpen33"));
}

void AsScene2201Door::stCloseDoor() {
	_doorOpen = false;
	setFileHash(0xE2CB0412, -1, -1);
	_playBackwards = true;
	_newHashListIndex = 0;
	_soundResource.play(calcHash("fxDoorClose33"));
}

Class444::Class444(NeverhoodEngine *vm, int pointIndex, int spriteIndex)
	: StaticSprite(vm, 900) {
	
	_spriteResource.load2(kClass444FileHashes[spriteIndex]);
	createSurface(100, 16, 16);
	_drawRect.x = -(_spriteResource.getDimensions().width / 2);
	_drawRect.y = -(_spriteResource.getDimensions().height / 2);
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = kClass444Points[pointIndex].x;
	_y = kClass444Points[pointIndex].y;
	_needRefresh = true;
}

Scene2201::Scene2201(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundFlag(false) {

	Sprite *tempSprite;

	if (!getSubVar(0x40050052, 0x60400854)) {
		// TODO _vm->gameModule()->initScene2201Vars();
	}

	_surfaceFlag = true;
	SetMessageHandler(&Scene2201::handleMessage);
	SetUpdateHandler(&Scene2201::update);
	
	loadDataResource(0x04104242);
	loadHitRectList();

	_background = addBackground(new DirtyBackground(_vm, 0x40008208, 0, 0));
	_palette = new Palette(_vm, 0x40008208);
	_palette->usePalette();
	_mouseCursor = addSprite(new Mouse433(_vm, 0x0820C408, NULL));

	_asTape = addSprite(new AsScene1201Tape(_vm, this, 7, 1100, 459, 432, 0x9148A011));
	_vm->_collisionMan->addSprite(_asTape); 

	_ssDoorButton = addSprite(new Class426(_vm, this, 0xE4A43E29, 0xE4A43E29, 100, 0));
	
	for (uint32 i = 0; i < 9; i++) {
		if ((int16)getSubVar(0x484498D0, i) >= 0) {
			addSprite(new Class444(_vm, i, (int16)getSubVar(0x484498D0, i)));
		}
	}

	_rect1.y1 = 0;
	_rect1.x2 = 640;
	_rect2.x2 = 640;
	_rect2.y2 = 480;
	
	if (!getGlobalVar(0x404290D5)) {
		insertStaticSprite(0x00026027, 900);
	}
	
	tempSprite = insertStaticSprite(0x030326A0, 1100);
	_rect1.x1 = tempSprite->getSurface()->getDrawRect().x;
	
	insertStaticSprite(0x811DA061, 1100);

	tempSprite = insertStaticSprite(0x11180022, 1100);
	_rect2.x1 = tempSprite->getSurface()->getDrawRect().x;

	tempSprite = insertStaticSprite(0x0D411130, 1100);
	_rect1.y2 = tempSprite->getSurface()->getDrawRect().y + tempSprite->getSurface()->getDrawRect().height;
	_rect2.y1 = tempSprite->getSurface()->getDrawRect().y + tempSprite->getSurface()->getDrawRect().height;
	
	_doorLightSprite = insertStaticSprite(0xA4062212, 900);

	if (which < 0) {
		InsertKlaymanInitArgs(KmScene2201, 300, 427, (&_rect1, 2));
		setMessageList(0x004B8118);
		_asDoor = addSprite(new AsScene2201Door(_vm, _klayman, _doorLightSprite, false));
	} else if (which == 1) {
		InsertKlaymanInitArgs(KmScene2201, 412, 393, (&_rect1, 2));
		setMessageList(0x004B8130);
		_asDoor = addSprite(new AsScene2201Door(_vm, _klayman, _doorLightSprite, false));
	} else if (which == 2) {
		if (getGlobalVar(0xC0418A02)) {
			InsertKlaymanInitArgs(KmScene2201, 379, 427, (&_rect1, 2));
			_klayman->setDoDeltaX(1);
		} else {
			InsertKlaymanInitArgs(KmScene2201, 261, 427, (&_rect1, 2));
		}
		setMessageList(0x004B8178);
		_asDoor = addSprite(new AsScene2201Door(_vm, _klayman, _doorLightSprite, false));
	} else {
		NPoint pt = _dataResource.getPoint(0x0304D8DC);
		InsertKlaymanInitArgs(KmScene2201, pt.x, pt.y, (&_rect1, 2));
		setMessageList(0x004B8120);
		_asDoor = addSprite(new AsScene2201Door(_vm, _klayman, _doorLightSprite, true));
	}
	
	addSprite(new AsScene2201CeilingFan(_vm));

	// TODO Sound1ChList_addSoundResource(0x04106220, 0x81212040, true);

}

Scene2201::~Scene2201() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
	// TODO Sound1ChList_sub_407AF0(0x04106220);
}

void Scene2201::update() {
	Scene::update();
	if (!_soundFlag) {
		// TODO Sound1ChList_playLooping(0x81212040);
		_soundFlag = true;
	}
}

uint32 Scene2201::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x402064D8) {
			sendEntityMessage(_klayman, 0x1014, _ssDoorButton);
		} else if (param.asInteger() == 0x35803198) {
			if (_asDoor->hasMessageHandler() && sendMessage(_asDoor, 0x2000, 0)) {
				setMessageList(0x004B81A0);
			} else {
				setMessageList(0x004B81B8);
			} 
		} else if (param.asInteger() == 0x51445010) {
			if (getGlobalVar(0x404290D5)) {
				setMessageList(0x004B8108);
			} else {
				setMessageList(0x004B8150);
			}
		} else if (param.asInteger() == 0x1D203082) {
			setMessageList(0x004B8180);
		} else if (param.asInteger() == 0x00049091) {
			if (getGlobalVar(0x404290D5)) {
				setMessageList(0x004B8138);
			} else {
				setMessageList(0x004B8108);
			}
		}
		break;
	case 0x480B:
		if (sender == _ssDoorButton) {
			sendMessage(_asDoor, 0x4808, 0);
		}
		break;
	case 0x4826:
		if (sender == _asTape) {
			sendEntityMessage(_klayman, 0x1014, _asTape);
			setMessageList(0x004B81C8);
		}
		break;
	}
	return 0;
}

static const NPoint kSsScene2202PuzzleTilePoints[] = {
	{196, 105},
	{323, 102},
	{445, 106},
	{192, 216},
	{319, 220},
	{446, 216},
	{188, 320},
	{319, 319},
	{443, 322}
};

static const uint32 kSsScene2202PuzzleTileFileHashes1[] = {
	0xA500800C,
	0x2182910C,
	0x2323980C,
	0x23049084,
	0x21008080,
	0x2303900C,
	0x6120980C,
	0x2504D808
};

static const uint32 kSsScene2202PuzzleTileFileHashes2[] = {
	0x0AAD8080,
	0x0A290291,
	0x0A2BA398,
	0x822B8490,
	0x86298080,
	0x0A2B8390,
	0x0A69A098,
	0x0E2D84D8
};

SsScene2202PuzzleTile::SsScene2202PuzzleTile(NeverhoodEngine *vm, Scene *parentScene, int16 tileIndex, int16 value)
	: StaticSprite(vm, 900), _soundResource1(vm), _soundResource2(vm), _parentScene(parentScene),
	_value(value), _tileIndex(tileIndex), _isMoving(false) {
	
	SetUpdateHandler(&SsScene2202PuzzleTile::update);
	SetMessageHandler(&SsScene2202PuzzleTile::handleMessage);
	_spriteResource.load2(kSsScene2202PuzzleTileFileHashes2[_value]);
	if (_tileIndex >= 0 && _tileIndex <= 2) {
		createSurface(100, 128, 128);
	} else	if (_tileIndex >= 3 && _tileIndex <= 5) {
		createSurface(300, 128, 128);
	} else {
		createSurface(500, 128, 128);
	}
	_drawRect.x = -(_spriteResource.getDimensions().width / 2);
	_drawRect.y = -(_spriteResource.getDimensions().height / 2);
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_deltaRect = _drawRect;
	_x = kSsScene2202PuzzleTilePoints[_tileIndex].x;
	_y = kSsScene2202PuzzleTilePoints[_tileIndex].y;
	processDelta();
	_needRefresh = true;
	StaticSprite::update();
	_soundResource1.load(0x40958621);
	_soundResource2.load(0x51108241);
}

void SsScene2202PuzzleTile::update() {
	handleSpriteUpdate();
	StaticSprite::update();
}

uint32 SsScene2202PuzzleTile::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_isMoving && !getGlobalVar(0x404290D5)) {
			sendMessage(_parentScene, 0x2000, _tileIndex);
		}
		messageResult = 1;
		break;
	case 0x2001:
		_isMoving = true;
		moveTile(param.asInteger());
		break;
	}
	return messageResult;
}
				
void SsScene2202PuzzleTile::suMoveTileX() {

	bool done = false;

	if (_counterDirection) {
		if (_counter > 2)
			_counter -= 2;
	} else {
		if (_counter < 20)
			_counter += 2;
	}

	for (int16 i = 0; i < _counter; i++) {
		_x += _xIncr;
		_errValue += _yDelta;
		if (_errValue >= _xDelta) {
			_errValue -= _xDelta;
			_y += _yIncr;
		}
		if (_x == _newX && _y == _newY) {
			done = true;
			break;
		}
		if (_x == _xFlagPos)
			_counterDirection = true;
	}
	
	if (done) {
		stopMoving();			
	}

	processDelta();

}

void SsScene2202PuzzleTile::suMoveTileY() {

	bool done = false;

	if (_counterDirection) {
		if (_counter > 2)
			_counter -= 2;
	} else {
		if (_counter < 20)
			_counter += 2;
	}

	for (int16 i = 0; i < _counter; i++) {
		_y += _yIncr;
		_errValue += _xDelta;
		if (_errValue >= _yDelta) {
			_errValue -= _yDelta;
			_x += _xIncr;
		}
		if (_x == _newX && _y == _newY) {
			done = true;
			break;
		}
		if (_x == _xFlagPos)
			_counterDirection = true;
	}
	
	if (done) {
		stopMoving();			
	}

	processDelta();

}

void SsScene2202PuzzleTile::moveTile(int16 newTileIndex) {

	_spriteResource.load2(kSsScene2202PuzzleTileFileHashes1[_value]);
	_drawRect.x = -(_spriteResource.getDimensions().width / 2);
	_drawRect.y = -(_spriteResource.getDimensions().height / 2);
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_needRefresh = true;

	setSubVar(0x484498D0, _tileIndex, (uint32)-1);
	setSubVar(0x484498D0, newTileIndex, (uint32)_value);
	
	_tileIndex = newTileIndex;
	
	_errValue = 0;
	_counterDirection = false;
	_counter = 0;

	_newX = kSsScene2202PuzzleTilePoints[newTileIndex].x;
	_newY = kSsScene2202PuzzleTilePoints[newTileIndex].y;

	if (_x == _newX && _y == _newY)
		return;

	if (_x <= _newX) {
		if (_y <= _newY) {
			_xDelta = _newX - _x;
			_yDelta = _newY - _y;
			_xIncr = 1;
			_yIncr = 1;
		} else {
			_xDelta = _newX - _x;
			_yDelta = _y - _newY;
			_xIncr = 1;
			_yIncr = -1;
		}
	} else {
		if (_y <= _newY) {
			_xDelta = _x - _newX;
			_yDelta = _newY - _y;
			_xIncr = -1;
			_yIncr = 1;
		} else {
			_xDelta = _x - _newX;
			_yDelta = _y - _newY;
			_xIncr = -1;
			_yIncr = -1;
		}
	}

	if (_xDelta > _yDelta) {
		SetSpriteCallback(&SsScene2202PuzzleTile::suMoveTileX);
		if (_xIncr > 0) {
			if (_newX - _x >= 180)
				_xFlagPos = _newX - 90;
			else
				_xFlagPos = _x + _newX / 2;				
		} else {
			if (_x - _newX >= 180)
				_xFlagPos = _x + 90;
			else
				_xFlagPos = _x / 2 + _newX;
		}
		_soundResource1.play();
	} else {
		SetSpriteCallback(&SsScene2202PuzzleTile::suMoveTileY);
		if (_yIncr > 0) {
			if (_newY - _y >= 180)
				_xFlagPos = _newY - 90;
			else
				_xFlagPos = _y + _newY / 2;				
		} else {
			if (_y - _newY >= 180)
				_xFlagPos = _y + 90;
			else
				_xFlagPos = _y / 2 + _newY;
		}
		_soundResource2.play();
	}
	
}

void SsScene2202PuzzleTile::stopMoving() {
	_spriteResource.load2(kSsScene2202PuzzleTileFileHashes2[_value]);
	_drawRect.x = -(_spriteResource.getDimensions().width / 2);
	_drawRect.y = -(_spriteResource.getDimensions().height / 2);
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_needRefresh = true;
	SetSpriteCallback(NULL);
	_isMoving = false;
	sendMessage(_parentScene, 0x2002, _tileIndex);
}

Scene2202::Scene2202(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource1(vm), _soundResource2(vm),
	_isSolved(false), _leaveScene(false), _isTileMoving(false), _movingTileSprite(NULL), _doneMovingTileSprite(NULL) {

	Palette2 *palette2;

	// TODO initScene2201Vars();
	SetMessageHandler(&Scene2202::handleMessage);
	SetUpdateHandler(&Scene2202::update);

	_surfaceFlag = true;

	_background = addBackground(new DirtyBackground(_vm, 0x08100A0C, 0, 0));
	palette2 = new Palette2(_vm, 0x08100A0C);
	_palette = palette2;
	_palette->usePalette();
	addEntity(palette2);
	_mouseCursor = addSprite(new Mouse435(_vm, 0x00A08089, 20, 620));

	//DEBUG!
	for (uint32 index = 0; index < 9; index++)
		setSubVar(0x484498D0, index, 7 - index);

	for (uint32 index = 0; index < 9; index++) {
		int16 value = (int16)getSubVar(0x484498D0, index);
		if (value >= 0) {
			Sprite *puzzleTileSprite = addSprite(new SsScene2202PuzzleTile(_vm, this, index, value));
			_vm->_collisionMan->addSprite(puzzleTileSprite);
		}
	}

	insertStaticSprite(0x55C043B8, 200);
	insertStaticSprite(0x85500158, 400);
	insertStaticSprite(0x25547028, 600);

	_soundResource1.load(0x68E25540);
	_soundResource2.load(0x40400457);

	// TODO Sound1ChList_addSoundResource(0x60400854, 0x8101A241, true);
	// TODO Sound1ChList_playLooping(0x8101A241);

}

Scene2202::~Scene2202() {
	// TODO Sound1ChList_sub_407AF0(0x60400854);
}

void Scene2202::update() {
	Scene::update();

	if (_leaveScene && !_soundResource2.isPlaying()) {
		sendMessage(_parentModule, 0x1009, 0);
	}

	if (_isSolved && !_soundResource1.isPlaying()) {
		_soundResource2.play();
		_isSolved = false;
		_leaveScene = true;
	}

	if (_movingTileSprite && !_isTileMoving) {
		int16 value = getFreeTileIndex(_movingTileIndex);
		if (value != -1) {
			setSurfacePriority(_movingTileSprite->getSurface(), 700);
			sendMessage(_movingTileSprite, 0x2001, value);
			_movingTileSprite = NULL;
			_isTileMoving = true;
		}
	}

	if (_doneMovingTileSprite) {
		setSurfacePriority(_doneMovingTileSprite->getSurface(), _surfacePriority);
		_doneMovingTileSprite = NULL;
		if (testIsSolved()) {
			_soundResource1.play();
			setGlobalVar(0x404290D5, 1);
			_isSolved = true;
		}
	}
	
}

uint32 Scene2202::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// TODO Debug stuff
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			sendMessage(_parentModule, 0x1009, 0);
		}
		break;
	case 0x000D:
		// TODO Debug stuff
		break;
	case 0x2000:
		_movingTileIndex = (int16)param.asInteger();
		_movingTileSprite = (Sprite*)sender;
		break;
	case 0x2002:
		_isTileMoving = false;
		_doneMovingTileSprite = (Sprite*)sender;
		if (param.asInteger() <= 2) {
			_surfacePriority = 100;
		} else if (param.asInteger() >= 3 && param.asInteger() <= 5) {
			_surfacePriority = 300;
		} else {
			_surfacePriority = 500;
		}
		break;
	}
	return 0;
}

int16 Scene2202::getFreeTileIndex(int16 index) {
	if (index >= 3 && (int16)getSubVar(0x484498D0, index - 3) == -1) {
		return index - 3;
	} else if (index <= 5 && (int16)getSubVar(0x484498D0, index + 3) == -1) {
		return index + 3;
	} else if (index != 0 && index != 3 && index != 6 && (int16)getSubVar(0x484498D0, index - 1) == -1) {
		return index - 1;
	} else if (index != 2 && index != 5 && index != 8 && (int16)getSubVar(0x484498D0, index + 1) == -1) {
		return index + 1;
	} else
		return -1;
}

bool Scene2202::testIsSolved() {
	return 
		getSubVar(0x484498D0, 0) == 0 &&
		getSubVar(0x484498D0, 2) == 2 &&
		getSubVar(0x484498D0, 3) == 3 &&
		getSubVar(0x484498D0, 4) == 4 &&
		getSubVar(0x484498D0, 5) == 5 &&
		getSubVar(0x484498D0, 6) == 6 &&
		getSubVar(0x484498D0, 8) == 7;
}

static const uint32 kClass545FileHashes[] = {
	0x2450D850,
	0x0C9CE8D0,
	0x2C58A152
};

Class545::Class545(NeverhoodEngine *vm, Scene *parentScene, int index, int surfacePriority, int16 x, int16 y)
	: AnimatedSprite(vm, kClass545FileHashes[index], surfacePriority, x, y), _parentScene(parentScene), _index(index) {

	if (!getSubVar(0x0090EA95, _index) && !getSubVar(0x08D0AB11, _index)) {
		SetMessageHandler(&Class545::handleMessage);
	} else {
		_surface->setVisible(false);
		SetMessageHandler(NULL);
	}
}

uint32 Class545::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case 0x4806:
		setSubVar(0x0090EA95, _index, 1);
		_surface->setVisible(false);
		SetMessageHandler(NULL);
	}
	return messageResult;
}

static const uint32 kAsScene2203DoorFileHashes[] = {
	0x7868AE10,
	0x1A488110
};

AsScene2203Door::AsScene2203Door(NeverhoodEngine *vm, Scene *parentScene, uint index)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _parentScene(parentScene),
	_index(index) {

	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2203Door::handleMessage);
	_x = 320;
	_y = 240;
	createSurface1(kAsScene2203DoorFileHashes[_index], 900);
	if (getGlobalVar(0x9A500914) == _index) {
		setFileHash(kAsScene2203DoorFileHashes[_index], -1, -1);
		_newHashListIndex = -2;
	} else {
		setFileHash(kAsScene2203DoorFileHashes[_index], 0, -1);
		_newHashListIndex = 0;
	}
}

uint32 AsScene2203Door::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_index == getGlobalVar(0x9A500914))
			sendMessage(_parentScene, 0x2002, 0);
		else
			sendMessage(_parentScene, 0x2001, 0);
		messageResult = 1;
		break;
	case 0x2000:
		_otherDoor = (Sprite*)param.asEntity();
		break;
	case 0x3002:
		if (_index == getGlobalVar(0x9A500914))
			sendMessage(_parentScene, 0x4808, 0);
		setFileHash1();
		break;
	case 0x4808:
		setGlobalVar(0x9A500914, _index);
		sendMessage(_otherDoor, 0x4809, 0);
		openDoor();
		break;
	case 0x4809:
		closeDoor();
		sendMessage(_parentScene, 0x2003, 0);
		break;
	}
	return messageResult;
}

void AsScene2203Door::openDoor() {
	_soundResource.play(0x341014C4);
	setFileHash(kAsScene2203DoorFileHashes[_index], 1, -1);
}

void AsScene2203Door::closeDoor() {
	setFileHash(kAsScene2203DoorFileHashes[_index], -1, -1);
	_playBackwards = true;
	_newHashListIndex = 0;
}

Scene2203::Scene2203(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {

	if (getGlobalVar(0xC0780812) && !getGlobalVar(0x13382860))
		setGlobalVar(0x13382860, 1);

	SetMessageHandler(&Scene2203::handleMessage);
	_surfaceFlag = true;

	_background = addBackground(new DirtyBackground(_vm, 0x82C80334, 0, 0));
	_palette = new Palette(_vm, 0x82C80334);
	_palette->usePalette();
	_mouseCursor = addSprite(new Mouse433(_vm, 0x80330824, NULL));

	_vm->_collisionMan->setHitRects(0x004B8320);

	if (getGlobalVar(0x13382860) == 1) {
		_class545 = addSprite(new Class545(_vm, this, 2, 1100, 282, 432));
		_vm->_collisionMan->addSprite(_class545);
	}

	_asTape = addSprite(new AsScene1201Tape(_vm, this, 1, 1100, 435, 432, 0x9148A011));
	_vm->_collisionMan->addSprite(_asTape);

	_asLeftDoor = addSprite(new AsScene2203Door(_vm, this, 0));
	_asRightDoor = addSprite(new AsScene2203Door(_vm, this, 1));
	
	_ssSmallLeftDoor = insertStaticSprite(0x542CC072, 1100);
	_ssSmallRightDoor = insertStaticSprite(0x0A2C0432, 1100);
	
	_leftDoorClipRect.x1 = _ssSmallLeftDoor->getSurface()->getDrawRect().x;
	_leftDoorClipRect.y1 = 0;
	_leftDoorClipRect.x2 = 640;
	_leftDoorClipRect.y2 = 480;
	
	_rightDoorClipRect.x1 = 0;
	_rightDoorClipRect.y1 = 0;
	_rightDoorClipRect.x2 = _ssSmallRightDoor->getSurface()->getDrawRect().x + _ssSmallRightDoor->getSurface()->getDrawRect().width;
	_rightDoorClipRect.y2 = 480;

	sendEntityMessage(_asLeftDoor, 0x2000, _asRightDoor);
	sendEntityMessage(_asRightDoor, 0x2000, _asLeftDoor);
	
	_vm->_collisionMan->addSprite(_asLeftDoor);
	_vm->_collisionMan->addSprite(_asRightDoor);

	if (which < 0) {
		InsertKlayman(KmScene2203, 200, 427);
		setMessageList(0x004B8340);
	} else if (which == 1) {
		InsertKlayman(KmScene2203, 640, 427);
		setMessageList(0x004B8350);
	} else if (which == 2) {
		if (getGlobalVar(0xC0418A02)) {
			InsertKlayman(KmScene2203, 362, 427);
			_klayman->setDoDeltaX(1);
		} else {
			InsertKlayman(KmScene2203, 202, 427);
		}
		setMessageList(0x004B8358);
	} else {
		InsertKlayman(KmScene2203, 0, 427);
		setMessageList(0x004B8348);
	}

	if (getGlobalVar(0x9A500914)) {
		_ssSmallLeftDoor->getSurface()->setVisible(false);
		_klayman->getSurface()->getClipRect() = _rightDoorClipRect;
	} else {
		_ssSmallRightDoor->getSurface()->setVisible(false);
		_klayman->getSurface()->getClipRect() = _leftDoorClipRect;
	}
	
	setRectList(0x004B8420);

}

Scene2203::~Scene2203() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
}

uint32 Scene2203::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2001:
		sendEntityMessage(_klayman, 0x1014, sender);
		if (sender == _asLeftDoor) {
			setMessageList2(0x004B83B0);
		} else {
			setMessageList2(0x004B83C8);
		}
		break;
	case 0x2002:
		if (sender == _asLeftDoor) {
			setMessageList2(0x004B8370);
		} else {
			setMessageList2(0x004B8360);
		}
		break;
	case 0x2003:
		if (sender == _asLeftDoor) {
			_ssSmallLeftDoor->getSurface()->setVisible(false);
		} else {
			_ssSmallRightDoor->getSurface()->setVisible(false);
		}
		break;
	case 0x4808:
		if (sender == _asLeftDoor) {
			_ssSmallLeftDoor->getSurface()->setVisible(true);
			_klayman->getSurface()->getClipRect() = _leftDoorClipRect;
		} else {
			_ssSmallRightDoor->getSurface()->setVisible(true);
			_klayman->getSurface()->getClipRect() = _rightDoorClipRect;
		}
		break;
	case 0x4826:
		if (sender == _asTape) {
			sendEntityMessage(_klayman, 0x1014, _asTape);
			setMessageList(0x004B83E0);
		} else if (sender == _class545) {
			sendEntityMessage(_klayman, 0x1014, _class545);
			setMessageList(0x004B83F0);
		}
		break;
	}
	return messageResult;
}

SsScene2205DoorFrame::SsScene2205DoorFrame(NeverhoodEngine *vm)
	: StaticSprite(vm, 900) {

	SetMessageHandler(&SsScene2205DoorFrame::handleMessage);
	_spriteResource.load2(getGlobalVar(0x4D080E54) ? 0x24306227 : 0xD90032A0);
	createSurface(1100, 45, 206);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_needRefresh = true;
	StaticSprite::update();
}

uint32 SsScene2205DoorFrame::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		_spriteResource.load2(getGlobalVar(0x4D080E54) ? 0x24306227 : 0xD90032A0);
		_drawRect.x = 0;
		_drawRect.y = 0;
		_drawRect.width = _spriteResource.getDimensions().width;
		_drawRect.height = _spriteResource.getDimensions().height;
		_x = _spriteResource.getPosition().x;
		_y = _spriteResource.getPosition().y;
		_needRefresh = true;
		StaticSprite::update();
	}
	return messageResult;
}

Scene2205::Scene2205(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {

	Palette2 *palette2;
	
	SetMessageHandler(&Scene2205::handleMessage);
	SetUpdateHandler(&Scene2205::update);
	
	_vm->_collisionMan->setHitRects(0x004B0620);
	_surfaceFlag = true;

	if (getGlobalVar(0x4D080E54)) {
		_isLightOn = true;
		_background = addBackground(new DirtyBackground(_vm, 0x0008028D, 0, 0));
		palette2 = new Palette2(_vm, 0x0008028D);
		_palette = palette2;
		_palette->usePalette();
		addEntity(palette2);
		_mouseCursor = addSprite(new Mouse433(_vm, 0x80289008, NULL));
		_ssLightSwitch = new Class426(_vm, this, 0x2D339030, 0x2D309030, 100, 0);
		addSprite(_ssLightSwitch);
	} else {
		_isLightOn = false;
		_background = addBackground(new DirtyBackground(_vm, 0xD00A028D, 0, 0));
		palette2 = new Palette2(_vm, 0xD00A028D);
		_palette = palette2;
		_palette->usePalette();
		addEntity(palette2);
		_mouseCursor = addSprite(new Mouse433(_vm, 0xA0289D08, NULL));
		_ssLightSwitch = new Class426(_vm, this, 0x2D339030, 0xDAC86E84, 100, 0);
		addSprite(_ssLightSwitch);
	}

	palette2->addPalette(0xD00A028D, 0, 256, 0);
	
	_ssDoorFrame = addSprite(new SsScene2205DoorFrame(_vm));

	if (which < 0) {
		InsertKlayman(KmScene2205, 320, 417);
		setMessageList(0x004B0658);
		if (!getGlobalVar(0x4D080E54)) {
			_palette->addPalette(0x68033B1C, 0, 65, 0);
		}
		_isKlaymanInLight = false;
	} else if (which == 1) {
		InsertKlayman(KmScene2205, 640, 417);
		setMessageList(0x004B0648);
		if (!getGlobalVar(0x4D080E54)) {
			_palette->addPalette(0x68033B1C, 0, 65, 0);
		}
		_isKlaymanInLight = false;
	} else {
		InsertKlayman(KmScene2205, 0, 417);
		setMessageList(0x004B0640);
		_isKlaymanInLight = true;
	}

	_klayman->getSurface()->getClipRect().x1 = _ssDoorFrame->getSurface()->getDrawRect().x;
	_klayman->getSurface()->getClipRect().y1 = 0;
	_klayman->getSurface()->getClipRect().x2 = 640;
	_klayman->getSurface()->getClipRect().y2 = 480;

	loadDataResource(0x00144822);
	_klayman->setSoundFlag(true);

}
	
void Scene2205::update() {
	Scene::update();

	if (!_isLightOn && getGlobalVar(0x4D080E54)) {
		_palette->addPalette(0x0008028D, 0, 256, 0);
		_background->load(0x0008028D);
		_ssLightSwitch->setFileHashes(0x2D339030, 0x2D309030);
		sendMessage(_ssDoorFrame, 0x2000, 0);
		((Mouse433*)_mouseCursor)->load(0x80289008);
		((Mouse433*)_mouseCursor)->updateCursor();
		_isLightOn = true;
	} else if (_isLightOn && !getGlobalVar(0x4D080E54)) {
		_palette->addPalette(0xD00A028D, 0, 256, 0);
		_background->load(0xD00A028D);
		_ssLightSwitch->setFileHashes(0x2D339030, 0xDAC86E84);
		sendMessage(_ssDoorFrame, 0x2000, 0);
		((Mouse433*)_mouseCursor)->load(0xA0289D08);
		((Mouse433*)_mouseCursor)->updateCursor();
		_isKlaymanInLight = true;
		if (_klayman->getX() > 85) {
			_palette->addPalette(0x68033B1C, 0, 65, 0);
			_isKlaymanInLight = false;
		}
		_isLightOn = false;
	}

	if (!getGlobalVar(0x4D080E54)) {
		if (_isKlaymanInLight && _klayman->getX() > 85) {
			((Palette2*)_palette)->addPalette(0x68033B1C, 0, 65, 0);
			((Palette2*)_palette)->startFadeToPalette(12);
			_isKlaymanInLight = false;
		} else if (!_isKlaymanInLight && _klayman->getX() <= 85) {
			((Palette2*)_palette)->addPalette(0xD00A028D, 0, 65, 0);
			((Palette2*)_palette)->startFadeToPalette(12);
			_isKlaymanInLight = true;
		}
	}
	
}

uint32 Scene2205::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x6449569A) {
			setMessageList(0x004B0690);
		} else if (param.asInteger() == 0x2841369C) {
			setMessageList(0x004B0630);
		} else if (param.asInteger() == 0x402064D8) {
			sendEntityMessage(_klayman, 0x1014, _ssLightSwitch);
		}
		break;
	case 0x480B:
		setGlobalVar(0x4D080E54, getGlobalVar(0x4D080E54) ? 0 : 1);
		break;
	}
	return 0;
}

static const int16 kScene2206XPositions[] = {
	384, 
	480, 
	572
};

static const uint32 kScene2206MessageIds1[] = {
	0x004B8998,
	0x004B89B8,
	0x004B89D8
};

static const uint32 kScene2206MessageIds2[] = {
	0x004B89F8,
	0x004B8A20,
	0x004B8A48
};

static const int16 kClass603XDeltas1[] = {
	-24, -28, -18, 6, 9, -8
};

static const int16 kClass603XDeltas2[] = {
	-8, 7, 11, 26, 13, 14
};

Class603::Class603(NeverhoodEngine *vm, uint32 fileHash)
	: StaticSprite(vm, fileHash, 200), _soundResource(vm) {
	
	if (getGlobalVar(0x18890C91))
		_x -= 63;
	SetUpdateHandler(&Class603::update);
	SetMessageHandler(&Class603::handleMessage);
	SetSpriteCallback(NULL);
}

void Class603::update() {
	handleSpriteUpdate();
	StaticSprite::update();
}

uint32 Class603::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4808:
		_index = 0;
		SetMessageHandler(NULL);
		SetSpriteCallback(&Class603::spriteUpdate481E60);
		_soundResource.play(0x032746E0);
		break;
	case 0x4809:
		_index = 0;
		SetMessageHandler(NULL);
		SetSpriteCallback(&Class603::spriteUpdate481E90);
		_soundResource.play(0x002642C0);
		break;
	}
	return messageResult;
}

void Class603::spriteUpdate481E60() {
	if (_index < 6) {
		_x += kClass603XDeltas1[_index];
		_index++;
	} else {
		SetMessageHandler(&Class603::handleMessage);
		SetSpriteCallback(NULL);
	}
}

void Class603::spriteUpdate481E90() {
	if (_index < 6) {
		_x += kClass603XDeltas2[_index];
		_index++;
	} else {
		SetMessageHandler(&Class603::handleMessage);
		SetSpriteCallback(NULL);
	}
}

Class604::Class604(NeverhoodEngine *vm, uint32 fileHash)
	: StaticSprite(vm, fileHash, 50) {

	SetUpdateHandler(&Class604::update);
	SetMessageHandler(&Class604::handleMessage);
	SetSpriteCallback(NULL);
}

void Class604::update() {
	handleSpriteUpdate();
	StaticSprite::update();
}

uint32 Class604::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4803:
		SetMessageHandler(NULL);
		SetSpriteCallback(&Class604::spriteUpdate482020);
		_yDelta = 0;
		break;
	}
	return messageResult;
}

void Class604::spriteUpdate482020() {
	_yDelta++;
	_y += _yDelta;
}

Class607::Class607(NeverhoodEngine *vm, Scene *parentScene, int surfacePriority, uint32 fileHash)
	: StaticSprite(vm, fileHash, surfacePriority), _parentScene(parentScene) {

	if (getGlobalVar(0x45080C38)) {
		_surface->setVisible(false);
		SetMessageHandler(NULL);
	} else {
		SetMessageHandler(&Class607::handleMessage);
	}
	_deltaRect = _drawRect;
	processDelta();
}

uint32 Class607::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case 0x4806:
		setGlobalVar(0x45080C38, 1);
		_surface->setVisible(false);
		SetMessageHandler(NULL);
		break;
	}
	return messageResult;
}

Scene2206::Scene2206(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource(vm) {

	uint32 fileHash;
	Palette2 *palette2;
	
	SetUpdateHandler(&Scene::update);
	SetMessageHandler(&Scene2206::handleMessage);
	_surfaceFlag = true;
	
	if (getGlobalVar(0x4D080E54)) {
		fileHash = 0x41983216;
		_sprite1 = insertStaticSprite(0x2201266A, 100);
		_sprite2 = insertStaticSprite(0x3406A333, 300);
		_sprite3 = insertStaticSprite(0x24A223A2, 100);
		_sprite4 = addSprite(new Class603(_vm, 0x26133023));
		_sprite4->getSurface()->getClipRect().x1 = _sprite2->getSurface()->getDrawRect().x;
		_sprite4->getSurface()->getClipRect().y1 = 0;
		_sprite4->getSurface()->getClipRect().x2 = 640;
		_sprite4->getSurface()->getClipRect().y2 = 480;
		setRectList(0x004B8AF8);
		_sprite5 = addSprite(new SsCommonButtonSprite(_vm, this, 0x0E038022, 100, 0));
		_mouseCursor = addSprite(new Mouse433(_vm, 0x83212411, NULL));
		_class607 = addSprite(new Class607(_vm, this, 1100, /*464, 433, */0x5E00E262));
		_class604 = addSprite(new Class604(_vm, 0x085E25E0));
	} else {
		fileHash = 0xE0102A45;
		_sprite1 = insertStaticSprite(0x1C1106B8, 100);
		_sprite2 = insertStaticSprite(0x020462E0, 300);
		_sprite3 = insertStaticSprite(0x900626A2, 100);
		_sprite4 = addSprite(new Class603(_vm, 0x544822A8));
		_sprite4->getSurface()->getClipRect().x1 = _sprite2->getSurface()->getDrawRect().x;
		_sprite4->getSurface()->getClipRect().y1 = 0;
		_sprite4->getSurface()->getClipRect().x2 = 640;
		_sprite4->getSurface()->getClipRect().y2 = 480;
		setRectList(0x004B8B58);
		_sprite5 = addSprite(new SsCommonButtonSprite(_vm, this, 0x16882608, 100, 0));
		_mouseCursor = addSprite(new Mouse433(_vm, 0x02A41E09, NULL));
		_class607 = addSprite(new Class607(_vm, this, 1100, /*464, 433, */0x52032563));
		_class604 = addSprite(new Class604(_vm, 0x317831A0));
	}

	_class604->getSurface()->getClipRect().x1 = _sprite2->getSurface()->getDrawRect().x;
	_class604->getSurface()->getClipRect().y1 = 0;
	_class604->getSurface()->getClipRect().x2 = _sprite3->getSurface()->getDrawRect().x + _sprite3->getSurface()->getDrawRect().width;
	_class604->getSurface()->getClipRect().y2 = _sprite1->getSurface()->getDrawRect().y + _sprite1->getSurface()->getDrawRect().height;

	_background = addBackground(new DirtyBackground(_vm, fileHash, 0, 0));

	palette2 = new Palette2(_vm, fileHash);
	_palette = palette2;
	_palette->usePalette();
	addEntity(palette2);

	palette2->addPalette(fileHash, 0, 256, 0);

	if (!getGlobalVar(0x4D080E54)) {
		_palette->addPalette(0x0263D144, 0, 65, 0);
	}
	
	_vm->_collisionMan->addSprite(_class607);
	
	if (which < 0) {
		InsertKlayman(KmScene2206, 200, 430);
		setMessageList(0x004B88A8);
	} else if (which == 1) {
		InsertKlayman(KmScene2206, 640, 430);
		setMessageList(0x004B88B8);
	} else if (which == 2) {
		InsertKlayman(KmScene2206, 205, 396);
		setMessageList(0x004B88C8);
		_palette->addPalette(getGlobalVar(0x4D080E54) ? 0xB103B604 : 0x0263D144, 0, 65, 0);
		sub4819D0();
		_soundResource.play(0x53B8284A);
	} else if (which == 3) {
		InsertKlayman(KmScene2206, kScene2206XPositions[getGlobalVar(0x48A68852)], 430);
		if (getGlobalVar(0xC0418A02))
			_klayman->setDoDeltaX(1);
		setMessageList(0x004B8A70);
	} else {
		InsertKlayman(KmScene2206, 0, 430);
		setMessageList(0x004B88B0);
	}

	_klayman->setSoundFlag(true);
	_klayman->setKlaymanTable2();

}

Scene2206::~Scene2206() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
}

uint32 Scene2206::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x800C6694) {
			sub481B00();
		} else if (param.asInteger() == 0x402064D8) {
			sendEntityMessage(_klayman, 0x1014, _sprite5);
		} else if (param.asInteger() == 0x11C40840) {
			if (getGlobalVar(0x18890C91))
				setMessageList(0x004B8948);
			else
				setMessageList(0x004B8970);
		}
		break;
	case 0x4803:
		sendMessage(_class604, 0x4803, 0);
		break;
	case 0x480B:
		if (sender == _sprite5) {
			setGlobalVar(0x18890C91, getGlobalVar(0x18890C91) ? 0 : 1);
			if (getGlobalVar(0x18890C91))
				sendMessage(_sprite4, 0x4808, 0);
			else
				sendMessage(_sprite4, 0x4809, 0);
		}
		break;
	case 0x4826:
		sendEntityMessage(_klayman, 0x1014, _class607);
		setMessageList(0x004B8988);
		break;
	case 0x482A:
		sub4819D0();
		break;
	case 0x482B:
		sub481950();
		break;
	}
	return messageResult;
}

void Scene2206::sub481950() {
	if (getGlobalVar(0x4D080E54)) {
		((Palette2*)_palette)->addPalette(0x41983216, 0, 65, 0);
		((Palette2*)_palette)->startFadeToPalette(12);
	}
	setSurfacePriority(_sprite1->getSurface(), 100);
	setSurfacePriority(_sprite2->getSurface(), 300);
	setSurfacePriority(_sprite3->getSurface(), 100);
	setSurfacePriority(_sprite4->getSurface(), 200);
	_klayman->getSurface()->getClipRect().x1 = 0;
	_klayman->getSurface()->getClipRect().y1 = 0;
	_klayman->getSurface()->getClipRect().x2 = 640;
	_klayman->getSurface()->getClipRect().y2 = 480;
}

void Scene2206::sub4819D0() {
	if (!getGlobalVar(0x4D080E54)) {
		((Palette2*)_palette)->addPalette(0xB103B604, 0, 65, 0);
		((Palette2*)_palette)->startFadeToPalette(12);
	}
	setSurfacePriority(_sprite1->getSurface(), 1100);
	setSurfacePriority(_sprite2->getSurface(), 1300);
	setSurfacePriority(_sprite3->getSurface(), 1100);
	setSurfacePriority(_sprite4->getSurface(), 1200);
	_klayman->getSurface()->getClipRect().x1 = _sprite2->getSurface()->getDrawRect().x;
	_klayman->getSurface()->getClipRect().y1 = 0;
	_klayman->getSurface()->getClipRect().x2 = _sprite3->getSurface()->getDrawRect().x + _sprite3->getSurface()->getDrawRect().width;
	_klayman->getSurface()->getClipRect().y2 = _sprite1->getSurface()->getDrawRect().y + _sprite1->getSurface()->getDrawRect().height;
}

void Scene2206::sub481B00() {
	setGlobalVar(0x48A68852, (_mouseClickPos.x - 354) / 96);
	if (getGlobalVar(0x48A68852) > 2)
		setGlobalVar(0x48A68852, 2);
	setGlobalVar(0x49C40058, (_mouseClickPos.y - 183) / 7);
	setGlobalVar(0xC8C28808, calcHash("stLineagex"));
	setGlobalVar(0x4CE79018, 0);
	if (ABS(kScene2206XPositions[getGlobalVar(0x48A68852)] - _klayman->getX()) >= 144) {
		setMessageList2(kScene2206MessageIds1[getGlobalVar(0x48A68852)]);
	} else {
		setMessageList2(kScene2206MessageIds2[getGlobalVar(0x48A68852)]);
	}
}

static const uint32 kScene2207FileHashes[] = {
	0x33B1E12E,
	0x33D1E12E,
	0x3311E12E,
	0x3291E12E,
	0x3191E12E,
	0x3791E12E,
	0x3B91E12E,
	0x2391E12E,
	0x1391E12E,
	0x3BB1E12E,
	0x23B1E12E,
	0x13B1E12E
};

AsScene2207Elevator::AsScene2207Elevator(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 900), _parentScene(parentScene), _soundResource(vm),
	_pointIndex(0), _destPointIndex(0), _destPointIndexDelta(0) {

	NPoint pt;

	_dataResource.load(0x00524846);
	_pointArray = _dataResource.getPointArray(0x005B02B7);
	pt = _dataResource.getPoint(0x403A82B1);
	_x = pt.x;
	_y = pt.y;
	createSurface(1100, 129, 103);
	setFileHash(getGlobalVar(0x4D080E54) ? 0xC858CC19 : 0x294B3377, 0, 0);
	SetUpdateHandler(&AsScene2207Elevator::update);
	SetSpriteCallback(&AsScene2207Elevator::suSetPosition);
	SetMessageHandler(&AsScene2207Elevator::handleMessage);
	_newHashListIndex = 0;
}

AsScene2207Elevator::~AsScene2207Elevator() {
	// TODO Sound1ChList_sub_407AF0(0x02700413);
}

void AsScene2207Elevator::update() {

	if (_destPointIndex + _destPointIndexDelta > _pointIndex) {
		_pointIndex++;
		setFileHash(getGlobalVar(0x4D080E54) ? 0xC858CC19 : 0x294B3377, _pointIndex, _pointIndex);
		_newHashListIndex = _pointIndex;		
		if (_destPointIndex + _destPointIndexDelta == _pointIndex) {
			if (_destPointIndexDelta != 0) {
				_destPointIndexDelta = 0;
			} else {
				// TODO Sound1ChList_deleteSoundByHash(0xD3B02847);
				_soundResource.play(0x53B8284A);
			}
		}
	}

	if (_destPointIndex + _destPointIndexDelta < _pointIndex) {
		_pointIndex--;
		if (_pointIndex == 0)
			sendMessage(_parentScene, 0x2003, 0);
		setFileHash(getGlobalVar(0x4D080E54) ? 0xC858CC19 : 0x294B3377, _pointIndex, _pointIndex);
		_newHashListIndex = _pointIndex;		
		if (_destPointIndex + _destPointIndexDelta == _pointIndex) {
			if (_destPointIndexDelta != 0) {
				_destPointIndexDelta = 0;
			} else {
				// TODO Sound1ChList_deleteSoundByHash(0xD3B02847);
				_soundResource.play(0x53B8284A);
			}
		}
	}

	if (_pointIndex > 20 && _surface->getPriority() != 900) {
		sendMessage(_parentScene, 0x2002, 900);
	} else if (_pointIndex < 20 && _surface->getPriority() != 1100) {
		sendMessage(_parentScene, 0x2002, 1100);
	}
	
	AnimatedSprite::update();
	
	if (_destPointIndex + _destPointIndexDelta == _pointIndex && _isMoving) {
		sendMessage(_parentScene, 0x2004, 0);
		_isMoving = false;
	}
	
}

void AsScene2207Elevator::suSetPosition() {
	_x = (*_pointArray)[_pointIndex].x;
	_y = (*_pointArray)[_pointIndex].y - 60;
	processDelta();
}

uint32 AsScene2207Elevator::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		moveToY(param.asInteger());
		break;
	}
	return messageResult;
}

void AsScene2207Elevator::moveToY(int16 y) {
	int16 minDistance = 480;

	if (!_pointArray || _pointArray->size() == 0)
		return;
	
	for (uint i = 0; i < _pointArray->size(); i++) {
		int16 distance = ABS(y - (*_pointArray)[i].y);
		if (distance < minDistance) {
			minDistance = distance;
			_destPointIndex = i;
		}
	}	

	if (_destPointIndex != _pointIndex) {
		if (_destPointIndex == 0 || _destPointIndex == (int)_pointArray->size() - 1) {
			_destPointIndexDelta = 0;
		} else if (_destPointIndex < _pointIndex) {
			_destPointIndexDelta = -2;
		} else {
			_destPointIndexDelta = 2;
		}
		// TODO Sound1ChList_addSoundResource(0x02700413, 0xD3B02847, true);
		// TODO Sound1ChList_playLooping(0xD3B02847);
	}

	_isMoving = true;

}

AsScene2207Lever::AsScene2207Lever(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, int doDeltaX)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _parentScene(parentScene) {
	
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2207Lever::handleMessage);
	createSurface(1010, 71, 73);
	setDoDeltaX(doDeltaX);
	setFileHash(0x80880090, 0, -1);
	_newHashListIndex = 0;
	_x = x;
	_y = y;
}

uint32 AsScene2207Lever::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case 0x3002:
		removeCallbacks();
		setFileHash1();
		break;
	case 0x4807:
		stLeverUp();
		break;
	case 0x480F:
		stLeverDown();
		break;
	case 0x482A:
		sendMessage(_parentScene, 0x1022, 990);
		break;
	case 0x482B:
		sendMessage(_parentScene, 0x1022, 1010);
		break;
	}
	return messageResult;
}

void AsScene2207Lever::stLeverDown() {
	setFileHash(0x80880090, 1, -1);
	setCallback1(AnimationCallback(&AsScene2207Lever::stLeverDownEvent));
	_soundResource.play(0x40581882);
}

void AsScene2207Lever::stLeverDownEvent() {
	sendMessage(_parentScene, 0x480F, 0);
}

void AsScene2207Lever::stLeverUp() {
	setFileHash(0x80880090, 6, -1);
	setCallback1(AnimationCallback(&AsScene2207Lever::stLeverUpEvent));
	_playBackwards = true;
	_soundResource.play(0x40581882);
}

void AsScene2207Lever::stLeverUpEvent() {
	sendMessage(_parentScene, 0x4807, 0);
}

AsScene2207WallRobotAnimation::AsScene2207WallRobotAnimation(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 1200), _soundResource1(vm), _soundResource2(vm),
	_soundResource3(vm), _soundResource4(vm), _idle(true) {
	
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2207WallRobotAnimation::handleMessage);
	createSurface1(0xCCFD6090, 100);
	_x = 309;
	_y = 320;
	setFileHash(0xCCFD6090, 0, -1);
	_newHashListIndex = 0;
	_soundResource2.load(0x40330872);
	_soundResource3.load(0x72A2914A);
	_soundResource4.load(0xD4226080);
}

AsScene2207WallRobotAnimation::~AsScene2207WallRobotAnimation() {
	// TODO Sound1ChList_sub_407AF0(0x80D00820);
}

uint32 AsScene2207WallRobotAnimation::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (!_idle) {
			if (param.asInteger() == 0x3423093) {
				// TODO Sound1ChList_addSoundResource(0x80D00820, 0x12121943, true);
				// TODO Sound1ChList_playLooping(0x12121943);
			} else if (param.asInteger() == 0x834AB011) {
				_soundResource1.stop();
				_soundResource2.stop();
				_soundResource3.stop();
				_soundResource4.stop();
				// TODO Sound1ChList_deleteSoundByHash(0x12121943);
			} else if (param.asInteger() == 0x3A980501) {
				_soundResource2.play();
			} else if (param.asInteger() == 0x2A2AD498) {
				_soundResource3.play();
			} else if (param.asInteger() == 0xC4980008) {
				_soundResource4.play();
			} else if (param.asInteger() == 0x06B84228) {
				_soundResource1.play(0xE0702146);
			}
		}
		break;
	case 0x2006:
		stStartAnimation();
		break;
	case 0x2007:
		stStopAnimation();
		break;
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

void AsScene2207WallRobotAnimation::stStartAnimation() {
	if (!_idle) {
		SetAnimationCallback3(NULL);
	} else {
		setFileHash(0xCCFD6090, 0, -1);
		_idle = false;
		_surface->setVisible(true);
	}
}

void AsScene2207WallRobotAnimation::stStopAnimation() {
	SetAnimationCallback3(&AsScene2207WallRobotAnimation::cbStopAnimation);
}

void AsScene2207WallRobotAnimation::cbStopAnimation() {
	setFileHash1();
	_soundResource1.stop();
	_soundResource2.stop();
	_soundResource3.stop();
	_soundResource4.stop();
	// TODO Sound1ChList_deleteSoundByHash(0x12121943);
	_idle = true;
	_surface->setVisible(false);
}

AsScene2207WallCannonAnimation::AsScene2207WallCannonAnimation(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200), _idle(true) {
	
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2207WallCannonAnimation::handleMessage);
	createSurface1(0x8CAA0099, 100);
	_x = 309;
	_y = 320;
	setFileHash(0x8CAA0099, 0, -1);
	_newHashListIndex = 0;
}

uint32 AsScene2207WallCannonAnimation::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2006:
		stStartAnimation();
		break;
	case 0x2007:
		stStopAnimation();
		break;
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

void AsScene2207WallCannonAnimation::stStartAnimation() {
	if (!_idle) {
		SetAnimationCallback3(NULL);
	} else {
		_surface->setVisible(true);
		setFileHash(0x8CAA0099, 0, -1);
		_idle = false;
	}
}

void AsScene2207WallCannonAnimation::stStopAnimation() {
	SetAnimationCallback3(&AsScene2207WallCannonAnimation::cbStopAnimation);
}

void AsScene2207WallCannonAnimation::cbStopAnimation() {
	setFileHash1();
	_surface->setVisible(false);
	_idle = true;
}

SsScene2207Symbol::SsScene2207Symbol(NeverhoodEngine *vm, uint32 fileHash, int index)
	: StaticSprite(vm, fileHash, 100) {

	_x = 330;
	_y = 246 + index * 50;
	StaticSprite::update();	
}

Scene2207::Scene2207(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource1(vm), _soundResource2(vm),
	_klaymanAtElevator(true), _elevatorSurfacePriority(0) {

	//DEBUG
	setGlobalVar(0x4D080E54, 1);

	_vm->gameModule()->initScene3009Vars();

	if (!getSubVar(0x40050052, 0x88460852))
		setSubVar(0x40050052, 0x88460852, 1);

	SetMessageHandler(&Scene2207::handleMessage);
	SetUpdateHandler(&Scene2207::update);
	_surfaceFlag = true;

	InsertKlayman(KmScene2207, 0, 0);
	_klayman->setRepl(64, 0);
	
	setMessageList(0x004B38E8);
	
	_asElevator = addSprite(new AsScene2207Elevator(_vm, this));
	
	if (getGlobalVar(0x4D080E54)) {

		_background = addBackground(new DirtyBackground(_vm, 0x88C00241, 0, 0));
		_palette = new Palette(_vm, 0x88C00241);
		_palette->usePalette();
		_mouseCursor = addSprite(new Mouse433(_vm, 0x00245884, NULL));
	
		_ssMaskPart1 = insertStaticSprite(0xE20A28A0, 1200);
		_ssMaskPart2 = insertStaticSprite(0x688F62A5, 1100);
		_ssMaskPart3 = insertStaticSprite(0x0043B038, 1100);
	
		_asTape = addSprite(new AsScene1201Tape(_vm, this, 4, 1100, 277, 428, 0x9148A011));
		_vm->_collisionMan->addSprite(_asTape); 
	
		_asLever = addSprite(new AsScene2207Lever(_vm, this, 527, 333, 0));
		_vm->_collisionMan->addSprite(_asLever);
		
		_asWallRobotAnimation = addSprite(new AsScene2207WallRobotAnimation(_vm, this));
		_asWallCannonAnimation = addSprite(new AsScene2207WallCannonAnimation(_vm));
		
		_asWallRobotAnimation->getSurface()->setVisible(false);
		_asWallCannonAnimation->getSurface()->setVisible(false);

		_ssButton = addSprite(new SsCommonButtonSprite(_vm, this, 0x2C4061C4, 100, 0));
	
		_asLever->getSurface()->getClipRect().x1 = 0;
		_asLever->getSurface()->getClipRect().y1 = 0;
		_asLever->getSurface()->getClipRect().x2 = _ssMaskPart3->getSurface()->getDrawRect().x + _ssMaskPart3->getSurface()->getDrawRect().width;
		_asLever->getSurface()->getClipRect().y2 = 480;
	
		_klayman->getSurface()->getClipRect().x1 = 0;
		_klayman->getSurface()->getClipRect().y1 = _ssMaskPart1->getSurface()->getDrawRect().y;
		_klayman->getSurface()->getClipRect().x2 = 640;
		_klayman->getSurface()->getClipRect().y2 = _ssMaskPart2->getSurface()->getDrawRect().y + _ssMaskPart2->getSurface()->getDrawRect().height;
	
		_asElevator->getSurface()->getClipRect().x1 = 0;
		_asElevator->getSurface()->getClipRect().y1 = _ssMaskPart1->getSurface()->getDrawRect().y;
		_asElevator->getSurface()->getClipRect().x2 = 640;
		_asElevator->getSurface()->getClipRect().y2 = _ssMaskPart2->getSurface()->getDrawRect().y + _ssMaskPart2->getSurface()->getDrawRect().height;
	
	} else {

		setGlobalVar(0x81890D14, 1);

		_background = addBackground(new DirtyBackground(_vm, 0x05C02A55, 0, 0));
		_palette = new Palette(_vm, 0x05C02A55);
		_palette->usePalette();
		_mouseCursor = addSprite(new Mouse433(_vm, 0x02A51054, NULL));

		_ssMaskPart1 = insertStaticSprite(0x980E46A4, 1200);

		addSprite(new SsScene2207Symbol(_vm, kScene2207FileHashes[getSubVar(0x00504B86, 0)], 0));
		addSprite(new SsScene2207Symbol(_vm, kScene2207FileHashes[getSubVar(0x00504B86, 1)], 1));
		addSprite(new SsScene2207Symbol(_vm, kScene2207FileHashes[getSubVar(0x00504B86, 2)], 2));

		_asTape = NULL;
		_asLever = NULL;
		_asWallRobotAnimation = NULL;
		_asWallCannonAnimation = NULL;
		_ssButton = NULL;

		_klayman->getSurface()->getClipRect().x1 = 0;
		_klayman->getSurface()->getClipRect().y1 = _ssMaskPart1->getSurface()->getDrawRect().y;
		_klayman->getSurface()->getClipRect().x2 = 640;
		_klayman->getSurface()->getClipRect().y2 = 480;
		
		_asElevator->getSurface()->getClipRect().x1 = 0;
		_asElevator->getSurface()->getClipRect().y1 = _ssMaskPart1->getSurface()->getDrawRect().y;
		_asElevator->getSurface()->getClipRect().x2 = 640;
		_asElevator->getSurface()->getClipRect().y2 = 480;

	}

	_dataResource.load(0x00524846);

	setRectList(0x004B38B8);

	sendEntityMessage(_klayman, 0x1014, _asElevator);
	sendMessage(_klayman, 0x2001, 0);
	sendMessage(_asElevator, 0x2000, 480);

	_soundResource2.load(calcHash("fxFogHornSoft"));

}

void Scene2207::update() {
	Scene::update();
	if (_elevatorSurfacePriority != 0) {
		setSurfacePriority(_asElevator->getSurface(), _elevatorSurfacePriority);
		_elevatorSurfacePriority = 0;
	}
	if (_klayman->getY() == 423) {
		_klaymanAtElevator = _klayman->getX() > 459 && _klayman->getX() < 525;
	}
}

uint32 Scene2207::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x0014F275) {
			if (_klaymanAtElevator) {
				sendMessage(_asElevator, 0x2000, _mouseClickPos.y);
				sendEntityMessage(_klayman, 0x1014, _asElevator);
				sendMessage(_klayman, 0x2001, 0);
			} else {
				messageList402220();
			}
		} else if (param.asInteger() == 0x34569073) {
			if (_klaymanAtElevator) {
				_messageListFlag1 = true;
				sendMessage(_asElevator, 0x2000, 0);
				sendEntityMessage(_klayman, 0x1014, _asElevator);
				sendMessage(_klayman, 0x2001, 0);
			} else {
				messageList402220();
			}
		} else if (param.asInteger() == 0x4054C877) {
			if (_klaymanAtElevator) {
				sendMessage(_asElevator, 0x2000, 480);
				sendEntityMessage(_klayman, 0x1014, _asElevator);
				sendMessage(_klayman, 0x2001, 0);
			} else {
				messageList402220();
			}
		} else if (param.asInteger() == 0x0CBC6211) {
			sendEntityMessage(_klayman, 0x1014, _asElevator);
			sendMessage(_klayman, 0x2001, 0);
			setRectList(0x004B38B8);
		} else if (param.asInteger() == 0x402064D8) {
			sendEntityMessage(_klayman, 0x1014, _ssButton);
		} else if (param.asInteger() == 0x231DA241) {
			if (_ssButton) {
				setMessageList(0x004B38F0);
			} else {
				setMessageList(0x004B37D8);
			}
		}
		break;
	case 0x2002:
		_elevatorSurfacePriority = param.asInteger();
		break;
	case 0x2003:
		_messageListFlag1 = false;
		break;
	case 0x4807:
		sendMessage(_asWallRobotAnimation, 0x2007, 0);
		sendMessage(_asWallCannonAnimation, 0x2007, 0);
		break;
	case 0x480B:
		if (sender == _ssButton) {
			if (getSubVar(0x14800353, 0x40119852)) {
				setSubVar(0x14800353, 0x40119852, 0);
				_soundResource1.play(calcHash("fx3LocksDisable"));
			} else {
				setSubVar(0x14800353, 0x40119852, 1);
				_soundResource2.play();
			}
		}
		break;
	case 0x480F:
		sendMessage(_asWallRobotAnimation, 0x2006, 0);
		sendMessage(_asWallCannonAnimation, 0x2006, 0);
		_asWallRobotAnimation->getSurface()->setVisible(true);
		_asWallCannonAnimation->getSurface()->setVisible(true);
		break;
	case 0x4826:
		if (sender == _asTape) {
			if (_klayman->getY() == 423) {
				sendEntityMessage(_klayman, 0x1014, _asTape);
				setMessageList(0x004B3958);
			}
		} else if (_klaymanAtElevator) {
			SetMessageHandler(&Scene2207::handleMessage2);
			sendMessage(_asElevator, 0x2000, 347);
			sendEntityMessage(_klayman, 0x1014, _asElevator);
			sendMessage(_klayman, 0x2001, 0);
		}
		break;
	}
	return messageResult;
}

uint32 Scene2207::handleMessage2(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2002:
		_elevatorSurfacePriority = param.asInteger();
		break;
	case 0x2004:
		SetMessageHandler(&Scene2207::handleMessage);
		sendMessage(_klayman, 0x2005, 0);
		sendEntityMessage(_klayman, 0x1014, _asLever);
		setMessageList(0x004B3920);
		setRectList(0x004B3948);
		break;
	}
	return messageResult;
}

static const uint32 kScene2208FileHashes1[] = {
	0x041023CB,
	0x041020CB,
	0x041026CB,
	0x04102ACB,
	0x041032CB,
	0x041002CB
};
	
static const uint32 kScene2208FileHashes2[] = {
	0x091206C9,
	0x091406C9,
	0x091806C9,
	0x090006C9,
	0x093006C9,
	0x095006C9
};

Scene2208::Scene2208(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _textResource(vm) {

	SpriteResource spriteResource(_vm);
	const char *textStart, *textEnd;

	if (!getGlobalVar(0xC8C28808))
		setGlobalVar(0xC8C28808, calcHash("stLineagex"));

	_textResource.load(getGlobalVar(0xC8C28808));
	
	textStart = _textResource.getString(getGlobalVar(0x48A68852), textEnd);
	while (textStart < textEnd) {
		_strings.push_back(textStart);
		textStart += strlen(textStart) + 1;
	}
	
	_maxRowIndex = 8 + 10 * (3 - (getGlobalVar(0xC8C28808) == calcHash("stLineagex") ? 1 : 0));

	_background = new Background(_vm, 0);
	_background->createSurface(0, 640, 528);
	_background->getSpriteResource().getPosition().y = 480;
	addBackground(_background);
	
	_palette = new Palette(_vm, 0x08100289);
	_palette->usePalette();
	addEntity(_palette); // Why?

	_mouseCursor = addSprite(new Mouse435(_vm, 0x0028D089, 40, 600));
	
	createFontSurface();
	
	_backgroundSurface = new BaseSurface(_vm, 0, 640, 480);
	spriteResource.load2(0x08100289);
	_backgroundSurface->drawSpriteResourceEx(spriteResource, false, false, 0, 0);

	_topBackgroundSurface = new BaseSurface(_vm, 0, 640, 192);
	spriteResource.load2(!getGlobalVar(0x4CE79018) ? kScene2208FileHashes1[getGlobalVar(0x48A68852) % 6] : getGlobalVar(0x4CE79018));
	_topBackgroundSurface->drawSpriteResourceEx(spriteResource, false, false, 0, 0);

	_bottomBackgroundSurface = new BaseSurface(_vm, 0, 640, 192);
	spriteResource.load2(kScene2208FileHashes2[getGlobalVar(0x48A68852) % 6]);
	_bottomBackgroundSurface->drawSpriteResourceEx(spriteResource, false, false, 0, 0);
	
	SetUpdateHandler(&Scene2208::update);
	SetMessageHandler(&Scene2208::handleMessage);

	_visibleRowsCount = 10;

	_newRowIndex = (int16)getGlobalVar(0x49C40058);
	if (_newRowIndex + _visibleRowsCount > _maxRowIndex)
		_newRowIndex = _maxRowIndex - _visibleRowsCount;
	if (_newRowIndex < 6)
		_newRowIndex = 0;

	_rowScrollY = 0;

	_backgroundScrollY = 48 * _newRowIndex;		

	_currRowIndex = _newRowIndex;

	for (int16 rowIndex = 0; rowIndex < _visibleRowsCount; rowIndex++)
		drawRow(_newRowIndex + rowIndex);

	_background->getSurface()->getSysRect().y = _backgroundScrollY;

	// TODO Screen.yOffset = _backgroundScrollY;
	// TODO Scene2208_sub409080 (creates background Sprites via the text, doesn't seem to be used?)

}

Scene2208::~Scene2208() {
	delete _fontSurface;
	delete _backgroundSurface;
	delete _topBackgroundSurface;
	delete _bottomBackgroundSurface;
}

void Scene2208::update() {

	int16 mouseY = _vm->getMouseY();
	
	if (mouseY < 48) {
		if (_currRowIndex > 0)
			_newRowIndex = _currRowIndex - 1;
	} else if (mouseY > 432) {
		if (_currRowIndex < _maxRowIndex - _visibleRowsCount)
			_newRowIndex = _currRowIndex + 1;
	} else {
		if (_currRowIndex > _newRowIndex)
			_newRowIndex = _currRowIndex;
	}

	if (_currRowIndex < _newRowIndex) {
		if (_rowScrollY == 0) {
			drawRow(_currRowIndex + _visibleRowsCount);
		}
		_backgroundScrollY += 4;
		_rowScrollY += 4;
		if (_rowScrollY == 48) {
			_rowScrollY = 0;
			_currRowIndex++;
		}
		_background->getSurface()->getSysRect().y = _backgroundScrollY;
	} else if (_currRowIndex > _newRowIndex || _rowScrollY > 0) {
		if (_rowScrollY == 0) {
			drawRow(_currRowIndex - 1);
			_currRowIndex--;
		}
		_backgroundScrollY -= 4;
		if (_rowScrollY == 0)
			_rowScrollY = 48;
		_rowScrollY -= 4;
		_background->getSurface()->getSysRect().y = _backgroundScrollY;
	}

	// TODO Screen.yOffset = _backgroundScrollY;
	Scene::update();

}

uint32 Scene2208::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x <= 40 || param.asPoint().x >= 600) {
			sendMessage(_parentModule, 0x1009, 0);
		}
		break;
	}
	return messageResult;
}

void Scene2208::createFontSurface() {
	DataResource fontData(_vm);
	SpriteResource spriteResource(_vm);
	fontData.load(calcHash("asRecFont"));
	uint16 numRows = fontData.getPoint(calcHash("meNumRows")).x;
	uint16 firstChar = fontData.getPoint(calcHash("meFirstChar")).x;
	uint16 charWidth = fontData.getPoint(calcHash("meCharWidth")).x;
	uint16 charHeight = fontData.getPoint(calcHash("meCharHeight")).x;
	NPointArray *tracking = fontData.getPointArray(calcHash("meTracking"));
	spriteResource.load2(0x0800090C);
	_fontSurface = new FontSurface(_vm, tracking, numRows, firstChar, charWidth, charHeight);
	_fontSurface->drawSpriteResourceEx(spriteResource, false, false, 0, 0);
}

void Scene2208::drawRow(int16 rowIndex) {
	NDrawRect sourceRect;	
	int16 y = (rowIndex * 48) % 528;
	if (rowIndex < 4) {
		sourceRect.x = 0;
		sourceRect.y = y;
		sourceRect.width = 640;
		sourceRect.height = 48;
		_background->getSurface()->copyFrom(_topBackgroundSurface->getSurface(), 0, y, sourceRect, true);
	} else if (rowIndex > _maxRowIndex - 5) {
		sourceRect.x = 0;
		sourceRect.y = (rowIndex - _maxRowIndex + 4) * 48;
		sourceRect.width = 640;
		sourceRect.height = 48;
		_background->getSurface()->copyFrom(_bottomBackgroundSurface->getSurface(), 0, y, sourceRect, true);
	} else {
		rowIndex -= 4;
		sourceRect.x = 0;
		sourceRect.y = (rowIndex * 48) % 480;
		sourceRect.width = 640;
		sourceRect.height = 48;
		_background->getSurface()->copyFrom(_backgroundSurface->getSurface(), 0, y, sourceRect, true);
		if (rowIndex < (int)_strings.size()) {
			const char *text = _strings[rowIndex];
			// TODO/CHECKME: Use temporary string up to '{' character (see above)
			_fontSurface->drawString(_background->getSurface(), 95, y, (const byte*)text);
		}
	}
}

static const int16 kScene2242XPositions[] = {
	68, 
	158
};

static const uint32 kScene2242MessageListIds2[] = {
	0x004B3CB8,
	0x004B3CD8
};

static const uint32 kScene2242MessageListIds1[] = {
	0x004B3CF8,
	0x004B3D20
};

Scene2242::Scene2242(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _isKlaymanInLight(false) {

	Palette2 *palette2;
	
	_surfaceFlag = true;
	SetMessageHandler(&Scene2242::handleMessage);
	SetUpdateHandler(&Scene2242::update);
	
	if (getGlobalVar(0x4D080E54)) {
		_background = addBackground(new DirtyBackground(_vm, 0x11840E24, 0, 0));
		_palette = new Palette(_vm, 0x11840E24);
		_palette->usePalette();
		_mouseCursor = addSprite(new Mouse433(_vm, 0x40E20110, NULL));
		setRectList(0x004B3DC8);
	} else {
		_background = addBackground(new DirtyBackground(_vm, 0x25848E24, 0, 0));
		palette2 = new Palette2(_vm, 0x25848E24);
		_palette = palette2;
		_palette->usePalette();
		addEntity(palette2);
		((Palette2*)_palette)->copyBasePalette(0, 256, 0);
		_palette->addPalette(0x68033B1C, 0, 65, 0);
		_mouseCursor = addSprite(new Mouse433(_vm, 0x48E20250, NULL));
		setRectList(0x004B3E18);
	}

	_asTape = addSprite(new AsScene1201Tape(_vm, this, 10, 1100, 464, 435, 0x9148A011));
	_vm->_collisionMan->addSprite(_asTape); 

	if (which < 0) {
		InsertKlayman(KmScene2242, 200, 430);
		setMessageList(0x004B3C18);
	} else if (which == 1) {
		InsertKlayman(KmScene2242, 530, 430);
		setMessageList(0x004B3D60);
	} else if (which == 2) {
		InsertKlayman(KmScene2242, kScene2242XPositions[!getGlobalVar(0x48A68852) ? 0 : 1], 430);
		setMessageList(0x004B3D48);
		if (getGlobalVar(0xC0418A02))
			_klayman->setDoDeltaX(1);
	} else {
		InsertKlayman(KmScene2242, 0, 430);
		setMessageList(0x004B3C20);
	}

	_klayman->setSoundFlag(true);

}

Scene2242::~Scene2242() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
}

void Scene2242::update() {
	if (!getGlobalVar(0x4D080E54)) {
		if (_isKlaymanInLight && _klayman->getX() < 440) {
			((Palette2*)_palette)->addPalette(0x68033B1C, 0, 65, 0);
			((Palette2*)_palette)->startFadeToPalette(12);
			_isKlaymanInLight = false;
		} else if (!_isKlaymanInLight && _klayman->getX() >= 440) {
			((Palette2*)_palette)->addPalette(0x25848E24, 0, 65, 0);
			((Palette2*)_palette)->startFadeToPalette(12);
			_isKlaymanInLight = true;
		}
	}
	Scene::update();
}

uint32 Scene2242::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x800C6694) {
			readClickedColumn();
		}
		break;
	case 0x4826:
		if (sender == _asTape) {
			sendEntityMessage(_klayman, 0x1014, _asTape);
			setMessageList(0x004B3D50);
		}
		break;
	}
	return messageResult;
}

void Scene2242::readClickedColumn() {
	int index;
	if (_mouseClickPos.x < 108) {
		setGlobalVar(0xC8C28808, 0x04290188);
		setGlobalVar(0x48A68852, 42);
		setGlobalVar(0x4CE79018, calcHash("bgRecPanelStart1"));
		index = 0;
	} else {
		setGlobalVar(0xC8C28808, 0x04290188);
		setGlobalVar(0x48A68852, 43);
		setGlobalVar(0x4CE79018, calcHash("bgRecPanelStart2"));
		index = 1;
	}
	setGlobalVar(0x49C40058, (_mouseClickPos.y - 100) / 7);
	if (ABS(_klayman->getX() - kScene2242XPositions[index]) < 133) {
		setMessageList2(kScene2242MessageListIds1[index]);
	} else {
		setMessageList2(kScene2242MessageListIds2[index]);
	}
}

static const int16 kHallOfRecordsKlaymanXPos[] = {
	68, 
	157, 
	246, 
	335, 
	424, 
	513, 
	602
};

static const uint32 kHallOfRecordsSceneMessageListIds2[] = {
	0x004B2978,
	0x004B2998,
	0x004B29B8,
	0x004B29D8,
	0x004B29F8,
	0x004B2A18,
	0x004B2A38
};

static const uint32 kHallOfRecordsSceneMessageListIds1[] = {
	0x004B2A58,
	0x004B2A80,
	0x004B2AA8,
	0x004B2AD0,
	0x004B2AF8,
	0x004B2B20,
	0x004B2B48
};

HallOfRecordsScene::HallOfRecordsScene(NeverhoodEngine *vm, Module *parentModule, int which, uint32 sceneInfo140Id)
	: Scene(vm, parentModule, true) {
	
	_sceneInfo140 = _vm->_staticData->getSceneInfo140Item(sceneInfo140Id);

	_surfaceFlag = true;
	SetMessageHandler(&HallOfRecordsScene::handleMessage);
	SetUpdateHandler(&Scene::update);

	if (!getGlobalVar(0x4D080E54) && _sceneInfo140->bgFilename2) {
		setRectList(0x004B2BF8);
		_background = addBackground(new DirtyBackground(_vm, _sceneInfo140->bgFilename2, 0, 0));
		_palette = new Palette(_vm, _sceneInfo140->bgFilename2);
		_palette->usePalette();
		_mouseCursor = addSprite(new Mouse433(_vm, 0x14320138, NULL));
	} else {
		setRectList(0x004B2BB8);
		_background = addBackground(new DirtyBackground(_vm, _sceneInfo140->bgFilename1, 0, 0));
		_palette = new Palette(_vm, _sceneInfo140->bgFilename1);
		_palette->usePalette();
		_mouseCursor = addSprite(new Mouse433(_vm, 0x63A40028, NULL));
	}

	if (which < 0) {
		InsertKlayman(KmHallOfRecords, 200, 430);
		setMessageList(0x004B2900);
	} else if (which == 1) {
		InsertKlayman(KmHallOfRecords, 640, 430);
		setMessageList(0x004B2910);
	} else if (which == 2) {
		InsertKlayman(KmHallOfRecords, kHallOfRecordsKlaymanXPos[getGlobalVar(0x48A68852) - _sceneInfo140->xPosIndex], 430);
		setMessageList(0x004B2B70);
		if (getGlobalVar(0xC0418A02))
			_klayman->setDoDeltaX(1);
	} else {
		InsertKlayman(KmHallOfRecords, 0, 430);
		setMessageList(0x004B2908);
	}

	_klayman->setSoundFlag(true);
	_klayman->setKlaymanTable2();

}

HallOfRecordsScene::~HallOfRecordsScene() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
}

uint32 HallOfRecordsScene::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x800C6694) {
			readClickedColumn();
		}
		break;
	}
	return messageResult;
}

void HallOfRecordsScene::readClickedColumn() {
	int16 index = (_mouseClickPos.x - 23) / 89;
	if (index >= _sceneInfo140->count) {
		setMessageList2(0x004B2920);
	} else {
		setGlobalVar(0x48A68852, _sceneInfo140->xPosIndex + index);
		setGlobalVar(0x49C40058, (_mouseClickPos.y - 100) / 7);
		setGlobalVar(0xC8C28808, _sceneInfo140->txFilename);
		if (index == 0 && _sceneInfo140->bgFilename3) {
			setGlobalVar(0x4CE79018, _sceneInfo140->bgFilename3);
		} else {
			setGlobalVar(0x4CE79018, 0);
		}
		if (ABS(_klayman->getX() - kHallOfRecordsKlaymanXPos[index]) < 133) {
			setMessageList2(kHallOfRecordsSceneMessageListIds1[index]);
		} else {
			setMessageList2(kHallOfRecordsSceneMessageListIds2[index]);
		}
	}
}

static const int16 kScene2247XPositions[] = {
	513,
	602
};

static const uint32 kScene2247MessageListIds2[] = {
	0x004B54A0,
	0x004B54C0
};

static const uint32 kScene2247MessageListIds1[] = {
	0x004B54E0,
	0x004B5508
};

Scene2247::Scene2247(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {
	
	//DEBUG
	setGlobalVar(0x4D080E54, 1);

	_surfaceFlag = true;
	SetMessageHandler(&Scene2247::handleMessage);
	SetUpdateHandler(&Scene::update);

	if (getGlobalVar(0x4D080E54)) {
		setRectList(0x004B5588);
		_background = addBackground(new DirtyBackground(_vm, 0x40339414, 0, 0));
		_palette = new Palette(_vm, 0x40339414);
		_palette->usePalette();
		_mouseCursor = addSprite(new Mouse433(_vm, 0x3941040B, NULL));
	} else {
		setRectList(0x004B55C8);
		_background = addBackground(new DirtyBackground(_vm, 0x071963E5, 0, 0));
		_palette = new Palette(_vm, 0x071963E5);
		_palette->usePalette();
		_mouseCursor = addSprite(new Mouse433(_vm, 0x14320138, NULL));
	}

	if (which < 0) {
		InsertKlayman(KmScene2247, 200, 430);
		setMessageList(0x004B5428);
	} else if (which == 1) {
		InsertKlayman(KmScene2247, 640, 430);
		setMessageList(0x004B5438);
	} else if (which == 2) {
		InsertKlayman(KmScene2247, kScene2247XPositions[getGlobalVar(0xC8C28808) == 0x0008E486 ? 0 : 1], 430);
		if (getGlobalVar(0xC0418A02))
			_klayman->setDoDeltaX(1);
		setMessageList(0x004B5530);
	} else {
		InsertKlayman(KmScene2247, 0, 430);
		setMessageList(0x004B5430);
	}

	_klayman->setSoundFlag(true);
	
}

Scene2247::~Scene2247() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
}

uint32 Scene2247::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	// TODO: Debug stuff
	case 0x100D:
		if (param.asInteger() == 0x800C6694) {
			readClickedColumn();
		}
		break;
	}
	return messageResult;
}

void Scene2247::readClickedColumn() {
	int index;
	if (_mouseClickPos.x < 553) {
		setGlobalVar(0xC8C28808, 0x0008E486);
		setGlobalVar(0x4CE79018, calcHash("bgFatherHeader"));
		index = 0;
	} else {
		setGlobalVar(0xC8C28808, 0x03086004);
		setGlobalVar(0x4CE79018, calcHash("bgQuaterHeader"));
		index = 1;
	}
	setGlobalVar(0x48A68852, 0);
	setGlobalVar(0x49C40058, (_mouseClickPos.y - 100) / 7);
	if (ABS(_klayman->getX() - kScene2247XPositions[index]) < 133) {
		setMessageList2(kScene2247MessageListIds1[index]);
	} else {
		setMessageList2(kScene2247MessageListIds2[index]);
	}
}

} // End of namespace Neverhood
