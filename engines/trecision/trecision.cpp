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

#include "common/scummsys.h"

#include "common/error.h"
#include "common/system.h"
#include "common/events.h"
#include "graphics/pixelformat.h"
#include "engines/util.h"

#include "trecision/trecision.h"

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/fs.h"

#include "trecision/nl/extern.h"
#include "trecision/nl/sysdef.h"
#include "trecision/graphics.h"
#include "trecision/video.h"

namespace Trecision {

TrecisionEngine *g_vm;

TrecisionEngine::TrecisionEngine(OSystem *syst) : Engine(syst) {
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "AUTORUN");
	SearchMan.addSubDirectoryMatching(gameDataDir, "DATA");
	SearchMan.addSubDirectoryMatching(gameDataDir, "FMV");

	syncSoundSettings();

	g_vm = this;

	_curRoom = 0;
	_oldRoom = 0;
	_curInventory = 0;
	_curSortTableNum = 0;

	for (int i = 0; i < 10; ++i)
		CurScriptFrame[i] = 0;

	// Inventory
	for (int i = 0; i < MAXICON; ++i) {
		_inventory[i] = 0;
		_cyberInventory[i] = 0;
	}

	_inventorySize = 0;
	_cyberInventorySize = 0;
	_iconBase = 0;
	_regenInvStartIcon = 0;
	_lastCurInventory = 0;

	_fastWalk = false;
	_fastWalkLocked = false;
	_mouseONOFF = true;

	// Use With
	_useWith[0] = _useWith[1] = 0;
	_useWithInv[0] = _useWithInv[1] = false;

	// Messages
	for (int i = 0; i < MAXOBJNAME; ++i)
		_objName[i] = nullptr;

	for (int i = 0; i < MAXSENTENCE; ++i)
		_sentence[i] = nullptr;

	for (int i = 0; i < MAXSYSTEXT; ++i)
		_sysText[i] = nullptr;

	_curMessage = nullptr;

	// Original values
	_idleMsg = {MC_IDLE, 0, MP_DEFAULT, 0, 0, 0, 0, 0};
	_curObj = 1;
	_inventoryStatus = INV_OFF;
	_lightIcon = 0xFF;
	_regenInvStartLine = INVENTORY_HIDE;
	_lastLightIcon = 0xFF;
	_inventoryCounter = INVENTORY_HIDE;

	for (int i = 0; i < 260; ++i) {
		_newData[i] = 0;
		_newData2[i] = 0;
	}

	_video2 = nullptr;
	_graphicsMgr = nullptr;
	_animMgr = nullptr;

	for (int i = 0; i < 50; ++i) {
		for (int j = 0; j < 4; ++j) {
			_limits[i][j] = 0;
		}
	}

	_limitsNum = 0;
	_actorLimit = 0;
	NextRefresh = 0;
	UStr[0] = '\0';

	CurKey = CurAscii = 0;
	wmx = wmy = 0;
	wmleft = wmright = false;
	omx = omy = 0;
	KeybInput = false;

	_gamePaused = false;

}

TrecisionEngine::~TrecisionEngine() {
	delete _animMgr;
	delete _graphicsMgr;
}

Common::Error TrecisionEngine::run() {
	_graphicsMgr = new GraphicsManager(this);
	if (!_graphicsMgr->initScreen())
		return Common::kUnsupportedColorMode;
	_animMgr = new AnimManager(this);

	initMain();

	while (!g_engine->shouldQuit()) {
		EventLoop();
		NextMessage();
	}

	return Common::kNoError;
}

void TrecisionEngine::EventLoop() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			wmx = event.mouse.x;
			wmy = event.mouse.y;
			break;

		case Common::EVENT_LBUTTONDOWN:
			wmleft = true;
			break;

		case Common::EVENT_LBUTTONUP:
			wmleft = false;
			break;

		case Common::EVENT_RBUTTONDOWN:
			wmright = true;
			break;

		case Common::EVENT_RBUTTONUP:
			wmright = false;
			break;

		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_CAPSLOCK) {
				if (!_fastWalkLocked)
					_fastWalk ^= true;
				_fastWalkLocked = true;
			}
			break;

		case Common::EVENT_KEYUP:
			CurKey = event.kbd.keycode;
			CurAscii = event.kbd.ascii;
			switch (event.kbd.keycode) {
			case Common::KEYCODE_p:
				if (!_gamePaused && !KeybInput) {
					CurKey = 0;
					_gamePaused = true;
					waitKey();
				}
				_gamePaused = false;
				break;

			case Common::KEYCODE_CAPSLOCK:
				_fastWalkLocked = false;
				break;
			}
			break;

		default:
			break;
		}
	}
	//g_system->delayMillis(10);
	g_system->updateScreen();
}

} // End of namespace Trecision
