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

#include "gnap/gnap.h"
#include "gnap/gamesys.h"
#include "gnap/resource.h"
#include "gnap/scenes/scenecore.h"

#include "gnap/scenes/scene01.h"

namespace Gnap {

int GnapEngine::initSceneLogic() {

	int backgroundId = -1;

	switch (_currentSceneNum) {
	case 0:
		backgroundId = scene00_init();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		break;
	case 1:
		_scene = new Scene01(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 2:
		backgroundId = scene02_init();
		scene02_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 3:
		backgroundId = scene03_init();
		scene03_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 4:
		backgroundId = scene04_init();
		scene04_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 5:
		backgroundId = scene05_init();
		scene05_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 6:
		backgroundId = scene06_init();
		scene06_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 7:
		backgroundId = scene07_init();
		scene07_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 8:
		backgroundId = scene08_init();
		scene08_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 9:
		backgroundId = scene09_init();
		scene09_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 10:
		backgroundId = scene10_init();
		scene10_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 11:
		backgroundId = scene11_init();
		scene11_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 12:
		backgroundId = scene12_init();
		scene12_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 13:
		backgroundId = scene13_init();
		scene13_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 14:
		backgroundId = scene14_init();
		scene14_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		break;
	case 15:
		backgroundId = scene15_init();
		scene15_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		break;
	case 16:
	case 47:
	case 48:
	case 54:
		backgroundId = cutscene_init();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		break;
	case 17:
		backgroundId = scene17_init();
		scene17_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 18:
		backgroundId = scene18_init();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		scene18_updateHotspots();
		break;
	case 19:
		backgroundId = scene19_init();
		scene19_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 20:
		backgroundId = scene20_init();
		scene20_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 21:
		backgroundId = scene21_init();
		scene21_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 22:
		backgroundId = scene22_init();
		scene22_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 23:
		backgroundId = scene23_init();
		scene23_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 24:
		backgroundId = scene24_init();
		scene24_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 136, 11, 10);
		break;
	case 25:
		backgroundId = scene25_init();
		scene25_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 26:
		backgroundId = scene26_init();
		scene26_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 27:
		backgroundId = scene27_init();
		scene27_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 28:
		backgroundId = scene28_init();
		scene28_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 29:
		backgroundId = scene29_init();
		scene29_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 30:
		backgroundId = scene30_init();
		scene30_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 31:
		backgroundId = scene31_init();
		scene31_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 32:
		backgroundId = scene32_init();
		scene32_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 33:
		backgroundId = scene33_init();
		scene33_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 34:
		backgroundId = scene03_init();
		scene03_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 35:
		backgroundId = scene05_init();
		scene05_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 36:
		backgroundId = scene06_init();
		scene06_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 37:
		backgroundId = scene04_init();
		scene04_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 38:
		backgroundId = scene38_init();
		scene38_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 39:
		backgroundId = scene39_init();
		scene39_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 40:
		backgroundId = scene40_init();
		scene40_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 41:
		backgroundId = scene41_init();
		scene41_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 42:
		backgroundId = scene42_init();
		scene42_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 43:
		backgroundId = scene43_init();
		scene43_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 44:
		backgroundId = scene44_init();
		scene44_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 45:
		backgroundId = scene45_init();
		scene45_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 46:
		backgroundId = scene46_init();
		scene46_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 49:
		backgroundId = scene49_init();
		scene49_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 50:
		backgroundId = scene50_init();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 51:
		backgroundId = scene51_init();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 52:
		backgroundId = scene52_init();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 53:
		backgroundId = scene53_init();
		scene53_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	}

	return backgroundId;
}

void GnapEngine::runSceneLogic() {
	switch (_currentSceneNum) {
	case 0:
		scene00_run();
		if (_newSceneNum == 55)
			_newSceneNum = 8;
		break;
	case 1:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 4;
		break;
	case 2:
		scene02_run();
		if (_newSceneNum == 55)
			_newSceneNum = 4;
		break;
	case 3:
		scene03_run();
		if (_newSceneNum == 55)
			_newSceneNum = 4;
		break;
	case 4:
		scene04_run();
		if (_newSceneNum == 55)
			_newSceneNum = 4;
		break;
	case 5:
		scene05_run();
		if (_newSceneNum == 55)
			_newSceneNum = 4;
		break;
	case 6:
		scene06_run();
		if (_newSceneNum == 55)
			_newSceneNum = 4;
		break;
	case 7:
		scene07_run();
		if (_newSceneNum == 55)
			_newSceneNum = 8;
		break;
	case 8:
		scene08_run();
		if (_newSceneNum == 55)
			_newSceneNum = 9;
		break;
	case 9:
		scene09_run();
		if (_newSceneNum == 55)
			_newSceneNum = 10;
		break;
	case 10:
		scene10_run();
		if (_newSceneNum == 55)
			_newSceneNum = 11;
		break;
	case 11:
		scene11_run();
		if (_newSceneNum == 55)
			_newSceneNum = 12;
		break;
	case 12:
		scene12_run();
		if (_newSceneNum == 55)
			_newSceneNum = 11;
		break;
	case 13:
		scene13_run();
		if (_newSceneNum == 55)
			_newSceneNum = 11;
		break;
	case 14:
		scene14_run();
		if (_newSceneNum == 55)
			_newSceneNum = 13;
		break;
	case 15:
		scene15_run();
		if (_newSceneNum == 55)
			_newSceneNum = 12;
		break;
	case 16:
		scene16_initCutscene();
		_newSceneNum = 17;
		_newCursorValue = 3;
		cutscene_run();
		break;
	case 17:
		scene17_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 18:
		scene18_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 19:
		scene19_run();
		if (_newSceneNum == 55)
			_newSceneNum = 19;
		break;
	case 20:
		scene20_run();
		if (_newSceneNum == 55)
			_newSceneNum = 22;
		break;
	case 21:
		scene21_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 22:
		scene22_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 23:
		scene23_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 24:
		scene24_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 25:
		scene25_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 26:
		scene26_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 27:
		scene27_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 28:
		scene28_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 29:
		scene29_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 30:
		scene30_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 31:
		scene31_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 32:
		scene32_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 33:
		scene33_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 34:
		scene03_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 35:
		scene05_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 36:
		scene06_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 37:
		scene04_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 38:
		scene38_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 39:
		scene39_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 40:
		scene40_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 41:
		scene41_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 42:
		scene42_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 43:
		scene43_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 44:
		scene44_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 45:
		scene45_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 46:
		scene46_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 47:
		if (_prevSceneNum == 49) {
			scene47_initCutscene1();
			_newSceneNum = 7;
			_newCursorValue = 2;
		} else if (_prevSceneNum == 13) {
			scene47_initCutscene2();
			_newSceneNum = 11;
		} else if (!isFlag(kGFPlatyPussDisguised) && _prevSceneNum == 2) {//CHECKME
			if (isFlag(kGFUnk25)) {
				scene47_initCutscene3();
				_newSceneNum = 2;
			} else {
				scene47_initCutscene4();
				_newSceneNum = 49;
			}
		} else if (_prevSceneNum == 21) {
			scene47_initCutscene5();
			_newSceneNum = 21;
			setFlag(kGFTwigTaken);
			setFlag(kGFKeysTaken);
		} else if (_prevSceneNum == 30) {
			scene47_initCutscene6();
			_newSceneNum = 26;
		} else if (isFlag(kGFPlatyPussDisguised) && _cursorValue == 1) {
			scene47_initCutscene7();
			_newSceneNum = 4;
		}
		cutscene_run();
		break;
	case 48:
		scene48_initCutscene();
		_newSceneNum = 33;
		_newCursorValue = 4;
		cutscene_run();
		break;
	case 49:
		scene49_run();
		if (_newSceneNum == 55)
			_newSceneNum = 47;
		break;
	case 50:
		scene50_run();
		_newSceneNum = _prevSceneNum;
		break;
	case 51:
		scene51_run();
		break;
	case 52:
		scene52_run();
		_newSceneNum = _prevSceneNum;
		break;
	case 53:
		scene53_run();
		if (_newSceneNum == 55)
			_newSceneNum = 53;
		break;
	case 54:
		if (_prevSceneNum == 45) {
			scene54_initCutscene1();
			_newSceneNum = 43;
			cutscene_run();
		} else {
			scene54_initCutscene2();
			cutscene_run();
			_gameDone = true;
		}
		break;
	}
}

} // End of namespace Gnap
