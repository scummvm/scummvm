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

#ifndef QDENGINE_MINIGAMES_3MICE2_RASKR_ALL_H
#define QDENGINE_MINIGAMES_3MICE2_RASKR_ALL_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

const int zoneCountInit1[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
	22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 64, 89, 90, 91,
	92, 93, 94, 96, 97, 98, 99, 129, 130, 132, 134, 135, 136, 138, 139, 140, 141,
	142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 153, 154, 155, 156, 157, 158,
	159, 160, 161, 162, 163, 164, 166, 167, 170, 172, 173, 174, 178, 179, 180, 182,
	184, 185, 186, 187, 190, 192, 196, 197, 198, 199, 204, 205, 211, 212, 213, 214,
	216, 217, 218, 219, 220, 221, 222, 223, 225, 226, 227, 229, 230, 231, 232, 234,
	242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 254, 255, 260, 261, 263,
	265, 267, 268, 271, 272, 273, 274, 280, 283, 284, 287, 288, 289, 290, 292, 294,
	296, 308, 309, 0
};

const int colorRegions1[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
		27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 89, 308, 309, 311, 0,
	64, 65, 0,
	90, 91, 92, 190, 207, 0,
	93, 94, 95, 0,
	96, 97, 98, 99, 103, 0,
	129, 132, 133, 0,
	130, 131, 0,
	134, 135, 136, 137, 0,
	138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 0,
	153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 0,
	166, 167, 194, 0,
	170, 171, 0,
	172, 173, 174, 175, 0,
	178, 179, 180, 182, 184, 185, 186, 187, 229, 230, 231, 232, 233, 0,
	192, 282, 0,
	196, 208, 0,
	197, 198, 199, 200, 0,
	204, 205, 206, 0,
	211, 212, 213, 214, 215, 0,
	216, 217, 218, 219, 220, 221, 222, 223, 224, 0,
	225, 226, 227, 228, 0,
	234, 235, 0,
	242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 0,
	254, 255, 256, 0,
	260, 261, 262, 0,
	263, 264, 0,
	265, 266, 0,
	267, 268, 269, 0,
	271, 272, 273, 274, 275, 0,
	280, 281, 0,
	283, 306, 0,
	284, 287, 288, 289, 290, 307, 0,
	292, 293, 0,
	294, 295, 0,
	296, 297, 0,
	0,
};

const int zoneCountInit2[] = {
	1, 2, 3, 4, 5, 6, 8, 16, 17, 18, 37, 46, 48, 49, 50, 51, 52, 53, 54, 55,
	56, 57, 58, 59, 60, 61, 66, 69, 70, 71, 72, 73, 74, 75, 76, 78, 80, 81,
	82, 83, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100,
	101, 102, 106, 107, 108, 109, 111, 113, 114, 115, 116, 117, 118, 119, 121,
	124, 126, 128, 130, 131, 132, 143, 146, 154, 156, 157, 191, 192, 193, 194,
	195, 205, 0,
};

int colorRegions2[] = {
	1, 2, 3, 4, 5, 55, 56, 57, 58, 59, 60, 93, 94, 95, 96, 97, 98, 99, 100, 101, 134, 0,
	6, 7, 0,
	8, 9, 0,
	16, 17, 18, 19, 0,
	37, 38, 0,
	46, 47, 0,
	48, 49, 50, 51, 52, 53, 54, 102, 103, 0,
	61, 68, 0,
	66, 67, 0,
	69, 70, 71, 72, 73, 74, 75, 76, 81, 152, 0,
	78, 79, 0,
	80, 85, 86, 87, 88, 89, 90, 91, 92, 141, 0,
	82, 83, 84, 0,
	106, 107, 108, 109, 110, 0,
	111, 112, 0,
	113, 114, 115, 116, 117, 118, 119, 120, 0,
	121, 122, 0,
	124, 125, 0,
	126, 127, 0,
	128, 129, 0,
	130, 131, 132, 133, 0,
	143, 144, 0,
	146, 147, 0,
	154, 155, 0,
	156, 159, 0,
	157, 158, 0,
	191, 192, 193, 194, 195, 196, 0,
	205, 206, 0,
	0,
};

const int zoneCountInit3[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
	25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 189, 190, -1
};

const int colorRegions3[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
		26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 0,
	190, 191, 192, 0,
	0,
};

const int zoneCountInit4[] = {
	0
};

const int colorRegions4[] = {
	0,
	0,
};


class qd3mice2RaskrAllMiniGame : public qdMiniGameInterface {
public:
	qd3mice2RaskrAllMiniGame(int gameNum) : _gameNum(gameNum) {}
	~qd3mice2RaskrAllMiniGame() {}

	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "3mice2RaskrAll::init()");

		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return false;

		const int *zoneCountInit = nullptr;
		int zoneCountDelta = 0;

		if (_gameNum == 1) {
			_numZones = 311;
			_colorRegions = colorRegions1;
			_colorRegionsSize = ARRAYSIZE(colorRegions1);
			_neededColors = 50;

			zoneCountInit = zoneCountInit1;
			zoneCountDelta = 0;
		} else if (_gameNum == 2) {
			_numZones = 206;
			_colorRegions = colorRegions2;
			_colorRegionsSize = ARRAYSIZE(colorRegions2);
			_neededColors = 50;

			zoneCountInit = zoneCountInit2;
			zoneCountDelta = 0;
		} else if (_gameNum == 3) {
			_numZones = 265;
			_colorRegions = colorRegions3;
			_colorRegionsSize = ARRAYSIZE(colorRegions3);
			_neededColors = 70;

			zoneCountInit = zoneCountInit3;
			zoneCountDelta = 1;
		} else if (_gameNum == 4) {
			_numZones = 133;
			_colorRegions = colorRegions4;
			_colorRegionsSize = ARRAYSIZE(colorRegions4);
			_neededColors = 50;

			zoneCountInit = zoneCountInit4;
			zoneCountDelta = 0;
		} else {
			error("qd3mice2RaskrAllMiniGame::init(): Unsupported gameNumber: %d", _gameNum);
		}

		for (int i = 1; i <= _numZones; i++) {
			_zones[i] = _scene->object_interface(Common::String::format("zone@%i#", i).c_str());
			_zoneCount[i] = true;
		}

		for (int i = 0; zoneCountInit[i] + zoneCountDelta != 0; i++)
			_zoneCount[zoneCountInit[i] + zoneCountDelta] = false;

		_objColor = _scene->object_interface("$color");
		_objColorSel = _scene->object_interface("$color_sel");
		_objLoaded = _scene->object_interface("$loaded");
		_objDone = _scene->object_interface("$done");

		if (_objLoaded->is_state_active("no")) {
			for (int i = 1; i <= _numZones; i++)
				_zones[i]->set_shadow(0xFEFEFF, 0);

			_objColorSel->set_state("!\xEC\xE0\xF1\xEA\xE0"); // "!маска"
			_objColor->set_shadow(0xFEFEFE, 0);
			_objLoaded->set_state("yes");
		}

		_timePassed = 0;

  		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "3mice2RaskrAll::quant(%f)", dt);

		_timePassed += dt;

		if (_timePassed > 0.5) {
			if (checkSolution())
				_objDone->set_state("\xe4\xe0");	// "да"

			_timePassed = 0.0;
		}

		qdMinigameObjectInterface *mouseObj = _scene->mouse_click_object_interface();

		if (!mouseObj)
			return true;

		const char *name = mouseObj->name();

		if (strstr(name, "@color@")) {
			_objColor->set_shadow(mouseObj->shadow_color(), mouseObj->shadow_alpha());

			Common::String color;
			for (int i = 8; name[i] != '#'; i++)
				color += name[i];

			_objColorSel->set_state(color.c_str());
		} else {
			if (strstr(name, "zone@")) {
				int num = getObjNum(name);

				int start = 0;
				for (int i = 0; i < _colorRegionsSize; i++) {
					if (_colorRegions[i] == 0) {
						start = i + 1;
						continue;
					}

					if (_colorRegions[i] == num)
						break;
				}

				debugC(2, kDebugMinigames, "zone: %d", num);

				if (start < _colorRegionsSize) {
					for (int i = start; _colorRegions[i] != 0; i++)
						_zones[_colorRegions[i]]->set_shadow(_objColor->shadow_color(), _objColor->shadow_alpha());
				} else {
					_zones[num]->set_shadow(_objColor->shadow_color(), _objColor->shadow_alpha());
				}
			}
		}
		_scene->release_object_interface(mouseObj);

		return true;
	}

	bool finit() {
		debugC(1, kDebugMinigames, "3mice2RaskrAll::finit()");

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
	bool checkSolution() {
		int count = 0;

		for (int i = 1; i <= _numZones; i++) {
			if (_zones[i]->shadow_color() != 0)
				if (_zoneCount[i])
					count++;
		}

		debugC(2, kDebugMinigames, "Solution count: %d  zone234: %06x", count, _zones[234]->shadow_color());

		return count > _neededColors;
	}

	int getObjNum(const char *name) {
		const char *from = strstr(name, "@");
		const char *to = strstr(name, "#");
		char tmp[20];

		Common::strlcpy(tmp, from + 1, to - from);

		return atol(tmp);
	}

private:
	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	qdMinigameObjectInterface *_zones[312] = { nullptr };
	qdMinigameObjectInterface *_objColor = nullptr;
	qdMinigameObjectInterface *_objColorSel = nullptr;
	qdMinigameObjectInterface *_objLoaded = nullptr;
	qdMinigameObjectInterface *_objDone = nullptr;

	bool _zoneCount[312] = { false };
	int _numZones = 0;

	float _timePassed = 0;

	int _gameNum = 0;

	const int *_colorRegions = nullptr;
	int _colorRegionsSize = 0;
	int _neededColors = 0;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_3MICE2_RASKR_ALL_H
