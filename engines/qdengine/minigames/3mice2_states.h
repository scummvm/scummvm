/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef QDENGINE_MINIGAMES_3MICE2_STATES_H
#define QDENGINE_MINIGAMES_3MICE2_STATES_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

class qd3mice2StatesMiniGame : public qdMiniGameInterface {
public:
	qd3mice2StatesMiniGame() {}
	~qd3mice2StatesMiniGame() {}

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "3mice2States::init()");

		_objScene = _scene->object_interface("$сцена");
		_objSelectedSet = _scene->object_interface("$select_nabor");

		_objects[1]   = _scene->object_interface("_МАСКА front ящик правый открыт");
		_objects[1]   = _scene->object_interface("_МАСКА front ящик правый открыт");
		_objects[2]   = _scene->object_interface("_МАСКА front ящик правый закрыт");
		_objects[2]   = _scene->object_interface("_МАСКА front ящик правый закрыт");
		_objects[3]   = _scene->object_interface("накладка front ящик правый открыт");
		_objects[3]   = _scene->object_interface("накладка front ящик правый открыт");
		_objects[4]   = _scene->object_interface("накладка front ящик левый открыт");
		_objects[4]   = _scene->object_interface("накладка front ящик левый открыт");
		_objects[5]   = _scene->object_interface("_МАСКА front ящик левый закрыт");
		_objects[5]   = _scene->object_interface("_МАСКА front ящик левый закрыт");
		_objects[6]   = _scene->object_interface("_МАСКА front ящик левый открыт");
		_objects[6]   = _scene->object_interface("_МАСКА front ящик левый открыт");
		_objects[7]   = _scene->object_interface("накладка front лево скалка");
		_objects[7]   = _scene->object_interface("накладка front лево скалка");
		_objects[8]   = _scene->object_interface("накладка front лево палочки");
		_objects[8]   = _scene->object_interface("накладка front лево палочки");
		_objects[9]   = _scene->object_interface("_МАСКА front гири в центральном ящике");
		_objects[9]   = _scene->object_interface("_МАСКА front гири в центральном ящике");
		_objects[10]  = _scene->object_interface("накладка front соль");
		_objects[11]  = _scene->object_interface("накладка front гири");
		_objects[12]  = _scene->object_interface("_МАСКА front ящик центральный");
		_objects[13]  = _scene->object_interface("накладка front ящик открыт");
		_objects[14]  = _scene->object_interface("фон front право");
		_objects[15]  = _scene->object_interface("фон front лево");
		_objects[16]  = _scene->object_interface("_МАСКА front право к front");
		_objects[17]  = _scene->object_interface("_МАСКА front лево к front");
		_objects[18]  = _scene->object_interface("_МАСКА front к front право");
		_objects[19]  = _scene->object_interface("_МАСКА front к front лево");
		_objects[20]  = _scene->object_interface("фон front");
		_objects[21]  = _scene->object_interface("_МАСКА front центр");
		_objects[22]  = _scene->object_interface("накладка front гири");
		_objects[23]  = _scene->object_interface("накладка front соль");
		_objects[24]  = _scene->object_interface("_МАСКА front гири в центральном ящике");
		_objects[25]  = _scene->object_interface("_МАСКА front ящик левый закрыт");
		_objects[26]  = _scene->object_interface("_МАСКА front ящик центральный");
		_objects[27]  = _scene->object_interface("накладка front ящик открыт");
		_objects[28]  = _scene->object_interface("_МАСКА front к front право");
		_objects[29]  = _scene->object_interface("_МАСКА front к front лево");
		_objects[30]  = _scene->object_interface("фон front");
		_objects[31]  = _scene->object_interface("_МАСКА front лево к front");
		_objects[32]  = _scene->object_interface("_МАСКА front центр");
		_objects[33]  = _scene->object_interface("фон front лево");
		_objects[34]  = _scene->object_interface("накладка front гири");
		_objects[35]  = _scene->object_interface("накладка front соль");
		_objects[36]  = _scene->object_interface("_МАСКА front гири в центральном ящике");
		_objects[37]  = _scene->object_interface("_МАСКА front ящик правый закрыт");
		_objects[38]  = _scene->object_interface("_МАСКА front ящик центральный");
		_objects[39]  = _scene->object_interface("накладка front ящик открыт");
		_objects[40]  = _scene->object_interface("_МАСКА front к front право");
		_objects[41]  = _scene->object_interface("_МАСКА front к front лево");
		_objects[42]  = _scene->object_interface("фон front");
		_objects[43]  = _scene->object_interface("_МАСКА front право к front");
		_objects[44]  = _scene->object_interface("_МАСКА front центр");
		_objects[45]  = _scene->object_interface("фон front право");
		_objects[46]  = _scene->object_interface("комод левый ящик открыт");
		_objects[47]  = _scene->object_interface("_МАСКА center левый ящик закрыт");
		_objects[48]  = _scene->object_interface("_МАСКА center левый ящик открыт лево");
		_objects[49]  = _scene->object_interface("_МАСКА center левый ящик открыт право");
		_objects[50]  = _scene->object_interface("накладка center яйца");
		_objects[51]  = _scene->object_interface("накладка center зеленый пакет");
		_objects[52]  = _scene->object_interface("накладка center масло");
		_objects[53]  = _scene->object_interface("накладка center молоко");
		_objects[54]  = _scene->object_interface("накладка center весы");
		_objects[55]  = _scene->object_interface("накладка center мука");
		_objects[56]  = _scene->object_interface("накладка center книга с рецептами");
		_objects[57]  = _scene->object_interface("_МАСКА center весы");
		_objects[58]  = _scene->object_interface("_МАСКА center книга с рецептами");
		_objects[59]  = _scene->object_interface("_МАСКА center правый ящик закрыт");
		_objects[60]  = _scene->object_interface("_МАСКА center правый ящик открыт лево");
		_objects[61]  = _scene->object_interface("_МАСКА center правый ящик открыт право");
		_objects[62]  = _scene->object_interface("комод правый ящик открыт");
		_objects[63]  = _scene->object_interface("_МАСКА center лево");
		_objects[64]  = _scene->object_interface("_МАСКА center право");
		_objects[65]  = _scene->object_interface("_МАСКА center фронт");
		_objects[66]  = _scene->object_interface("фон center");
		_objects[67]  = _scene->object_interface("_МАСКА front ящик левый закрыт");
		_objects[68]  = _scene->object_interface("_МАСКА front ящик правый закрыт");
		_objects[69]  = _scene->object_interface("_МАСКА front ящик правый открыт");
		_objects[70]  = _scene->object_interface("_МАСКА front ящик левый открыт");
		_objects[71]  = _scene->object_interface("накладка front лево палочки");
		_objects[72]  = _scene->object_interface("накладка front лево скалка");
		_objects[73]  = _scene->object_interface("накладка front ящик правый открыт");
		_objects[74]  = _scene->object_interface("накладка front ящик левый открыт");
		_objects[75]  = _scene->object_interface("_МАСКА front соль");
		_objects[76]  = _scene->object_interface("накладка front соль");
		_objects[77]  = _scene->object_interface("_МАСКА front ящик центральный");
		_objects[78]  = _scene->object_interface("накладка front ящик открыт");
		_objects[79]  = _scene->object_interface("фон front право");
		_objects[80]  = _scene->object_interface("фон front лево");
		_objects[81]  = _scene->object_interface("_МАСКА front право к front");
		_objects[82]  = _scene->object_interface("_МАСКА front лево к front");
		_objects[83]  = _scene->object_interface("_МАСКА front к front право");
		_objects[84]  = _scene->object_interface("_МАСКА front к front лево");
		_objects[85]  = _scene->object_interface("фон front");
		_objects[86]  = _scene->object_interface("_МАСКА front центр");
		_objects[87]  = _scene->object_interface("_МАСКА front ящик левый закрыт");
		_objects[88]  = _scene->object_interface("накладка front соль");
		_objects[89]  = _scene->object_interface("_МАСКА front соль");
		_objects[90]  = _scene->object_interface("_МАСКА front ящик центральный");
		_objects[91]  = _scene->object_interface("накладка front ящик открыт");
		_objects[92]  = _scene->object_interface("_МАСКА front к front право");
		_objects[93]  = _scene->object_interface("_МАСКА front к front лево");
		_objects[94]  = _scene->object_interface("фон front");
		_objects[95]  = _scene->object_interface("_МАСКА front лево к front");
		_objects[96]  = _scene->object_interface("_МАСКА front центр");
		_objects[97]  = _scene->object_interface("фон front лево");
		_objects[98]  = _scene->object_interface("_МАСКА front ящик правый закрыт");
		_objects[99]  = _scene->object_interface("накладка front соль");
		_objects[100] = _scene->object_interface("_МАСКА front соль");
		_objects[101] = _scene->object_interface("_МАСКА front ящик центральный");
		_objects[102] = _scene->object_interface("накладка front ящик открыт");
		_objects[103] = _scene->object_interface("_МАСКА front к front право");
		_objects[104] = _scene->object_interface("_МАСКА front к front лево");
		_objects[105] = _scene->object_interface("фон front");
		_objects[106] = _scene->object_interface("_МАСКА front право к front");
		_objects[107] = _scene->object_interface("_МАСКА front центр");
		_objects[108] = _scene->object_interface("фон front право");
		_objects[109] = _scene->object_interface("комод левый ящик открыт");
		_objects[110] = _scene->object_interface("комод правый ящик открыт");
		_objects[111] = _scene->object_interface("накладка center зеленый пакет");
		_objects[112] = _scene->object_interface("накладка center масло");
		_objects[113] = _scene->object_interface("накладка center молоко");
		_objects[114] = _scene->object_interface("накладка center мука");
		_objects[115] = _scene->object_interface("накладка center яйца");
		_objects[116] = _scene->object_interface("фон center");
		_objects[117] = _scene->object_interface("_МАСКА center фронт");
		_objects[118] = _scene->object_interface("_МАСКА center право");
		_objects[119] = _scene->object_interface("_МАСКА center правый ящик закрыт");
		_objects[120] = _scene->object_interface("_МАСКА center левый ящик закрыт");
		_objects[121] = _scene->object_interface("_МАСКА center лево");
		_objects[122] = _scene->object_interface("_МАСКА center левый ящик открыт лево");
		_objects[123] = _scene->object_interface("_МАСКА center левый ящик открыт право");
		_objects[124] = _scene->object_interface("_МАСКА center правый ящик открыт лево");
		_objects[125] = _scene->object_interface("_МАСКА center правый ящик открыт право");
		_objects[126] = _scene->object_interface("_МАСКА center зеленый пакет");
		_objects[127] = _scene->object_interface("_МАСКА center масло");
		_objects[128] = _scene->object_interface("_МАСКА center молоко");
		_objects[129] = _scene->object_interface("_МАСКА center мука");
		_objects[130] = _scene->object_interface("_МАСКА center яйца");

  		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "3mice2States::quant(%f)", dt);

		return true;
	}

	bool finit() {
		debugC(1, kDebugMinigames, "3mice2States::finit()");

		if (_scene)  {
			_engine->release_scene_interface(_scene);
			_scene = 0;
		}

 		return true;
	}

	bool new_game(const qdEngineInterface *engine_interface) {
		return true;
	}

	int save_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, char *buffer, int buffer_size) {
		return 0;
	}

	int load_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, const char *buffer, int buffer_size) {
		return 0;
	}

	enum { INTERFACE_VERSION = 112 };
	int version() const {
		return INTERFACE_VERSION;
	}

private:
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	qdMinigameObjectInterface *_objScene = nullptr;
	qdMinigameObjectInterface *_objSelectedSet = nullptr;

	qdMinigameObjectInterface *_objects[131];
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_3MICE2_STATES_H
