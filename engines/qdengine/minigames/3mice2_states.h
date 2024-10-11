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

#define MASK1 "\xec\xe0\xf1\xea\xe0\x31"  // "маска1"
#define MASK2 "\xec\xe0\xf1\xea\xe0\x32"  // "маска2"
#define MASK3 "\xec\xe0\xf1\xea\xe0\x33"  // "маска3"
#define MASK4 "\xec\xe0\xf1\xea\xe0\x34"  // "маска4"
#define MAIN "\xee\xf1\xed\xee\xe2\xed\xee\xe9"  // "основной"
#define SET1 "\xed\xe0\xe1\xee\xf0\x31"		// "набор1"
#define SET2 "\xed\xe0\xe1\xee\xf0\x32"		// "набор2"
#define SET3 "\xed\xe0\xe1\xee\xf0\x33"		// "набор3
#define SET4 "\xed\xe0\xe1\xee\xf0\x34"		// "набор4"
#define SET5 "\xed\xe0\xe1\xee\xf0\x35"		// "набор5"
#define SET6 "\xed\xe0\xe1\xee\xf0\x36"		// "набор6"
#define SET7 "\xed\xe0\xe1\xee\xf0\x37"		// "набор7"
#define SET8 "\xed\xe0\xe1\xee\xf0\x38"		// "набор8"
#define BG_MASK "\xd4\xee\xed\x20\x2d\x20\xec\xe0\xf1\xea\xe0"		// "Фон - маска"
#define BG_MASK1 "\xd4\xee\xed\x20\x2d\x20\xec\xe0\xf1\xea\xe0\x31"	// "Фон - маска1"
#define BG_MASK2 "\xd4\xee\xed\x20\x2d\x20\xec\xe0\xf1\xea\xe0\x32"	// "Фон - маска2"
#define BG_FRONT_LEFT "\xf4\xee\xed front \xeb\xe5\xe2\xee"			// "фон front лево"
#define BG_FRONT_RIGHT "\xf4\xee\xed front \xef\xf0\xe0\xe2\xee"	// "фон front право"


	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "3mice2States::init()");

		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return false;

		_objScene = _scene->object_interface("$\xF1\xF6\xE5\xED\xE0"); // "$сцена"
		_objSelectedSet = _scene->object_interface("$select_nabor");

		_objects[1]   = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xEF\xF0\xE0\xE2\xFB\xE9 \xEE\xF2\xEA\xF0\xFB\xF2"); // "_МАСКА front ящик правый открыт"
		_objects[2]   = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xEF\xF0\xE0\xE2\xFB\xE9 \xE7\xE0\xEA\xF0\xFB\xF2"); // "_МАСКА front ящик правый закрыт"
		_objects[3]   = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xFF\xF9\xE8\xEA \xEF\xF0\xE0\xE2\xFB\xE9 \xEE\xF2\xEA\xF0\xFB\xF2"); // "накладка front ящик правый открыт"
		_objects[4]   = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xFF\xF9\xE8\xEA \xEB\xE5\xE2\xFB\xE9 \xEE\xF2\xEA\xF0\xFB\xF2"); // "накладка front ящик левый открыт"
		_objects[5]   = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xEB\xE5\xE2\xFB\xE9 \xE7\xE0\xEA\xF0\xFB\xF2"); // "_МАСКА front ящик левый закрыт"
		_objects[6]   = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xEB\xE5\xE2\xFB\xE9 \xEE\xF2\xEA\xF0\xFB\xF2"); // "_МАСКА front ящик левый открыт"
		_objects[7]   = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xEB\xE5\xE2\xEE \xF1\xEA\xE0\xEB\xEA\xE0"); // "накладка front лево скалка"
		_objects[8]   = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xEB\xE5\xE2\xEE \xEF\xE0\xEB\xEE\xF7\xEA\xE8"); // "накладка front лево палочки"
		_objects[9]   = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xE3\xE8\xF0\xE8 \xE2 \xF6\xE5\xED\xF2\xF0\xE0\xEB\xFC\xED\xEE\xEC \xFF\xF9\xE8\xEA\xE5"); // "_МАСКА front гири в центральном ящике"
		_objects[10]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xF1\xEE\xEB\xFC"); // "накладка front соль"
		_objects[11]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xE3\xE8\xF0\xE8"); // "накладка front гири"
		_objects[12]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xF6\xE5\xED\xF2\xF0\xE0\xEB\xFC\xED\xFB\xE9"); // "_МАСКА front ящик центральный"
		_objects[13]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2"); // "накладка front ящик открыт"
		_objects[14]  = _scene->object_interface(BG_FRONT_RIGHT);
		_objects[15]  = _scene->object_interface(BG_FRONT_LEFT);
		_objects[16]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEF\xF0\xE0\xE2\xEE \xEA front"); // "_МАСКА front право к front"
		_objects[17]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEB\xE5\xE2\xEE \xEA front"); // "_МАСКА front лево к front"
		_objects[18]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEA front \xEF\xF0\xE0\xE2\xEE"); // "_МАСКА front к front право"
		_objects[19]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEA front \xEB\xE5\xE2\xEE"); // "_МАСКА front к front лево"
		_objects[20]  = _scene->object_interface("\xF4\xEE\xED front"); // "фон front"
		_objects[21]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xF6\xE5\xED\xF2\xF0"); // "_МАСКА front центр"
		_objects[22]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xE3\xE8\xF0\xE8"); // "накладка front гири"
		_objects[23]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xF1\xEE\xEB\xFC"); // "накладка front соль"
		_objects[24]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xE3\xE8\xF0\xE8 \xE2 \xF6\xE5\xED\xF2\xF0\xE0\xEB\xFC\xED\xEE\xEC \xFF\xF9\xE8\xEA\xE5"); // "_МАСКА front гири в центральном ящике"
		_objects[25]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xEB\xE5\xE2\xFB\xE9 \xE7\xE0\xEA\xF0\xFB\xF2"); // "_МАСКА front ящик левый закрыт"
		_objects[26]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xF6\xE5\xED\xF2\xF0\xE0\xEB\xFC\xED\xFB\xE9"); // "_МАСКА front ящик центральный"
		_objects[27]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2"); // "накладка front ящик открыт"
		_objects[28]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEA front \xEF\xF0\xE0\xE2\xEE"); // "_МАСКА front к front право"
		_objects[29]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEA front \xEB\xE5\xE2\xEE"); // "_МАСКА front к front лево"
		_objects[30]  = _scene->object_interface("\xF4\xEE\xED front"); // "фон front"
		_objects[31]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEB\xE5\xE2\xEE \xEA front"); // "_МАСКА front лево к front"
		_objects[32]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xF6\xE5\xED\xF2\xF0"); // "_МАСКА front центр"
		_objects[33]  = _scene->object_interface(BG_FRONT_LEFT);
		_objects[34]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xE3\xE8\xF0\xE8"); // "накладка front гири"
		_objects[35]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xF1\xEE\xEB\xFC"); // "накладка front соль"
		_objects[36]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xE3\xE8\xF0\xE8 \xE2 \xF6\xE5\xED\xF2\xF0\xE0\xEB\xFC\xED\xEE\xEC \xFF\xF9\xE8\xEA\xE5"); // "_МАСКА front гири в центральном ящике"
		_objects[37]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xEF\xF0\xE0\xE2\xFB\xE9 \xE7\xE0\xEA\xF0\xFB\xF2"); // "_МАСКА front ящик правый закрыт"
		_objects[38]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xF6\xE5\xED\xF2\xF0\xE0\xEB\xFC\xED\xFB\xE9"); // "_МАСКА front ящик центральный"
		_objects[39]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2"); // "накладка front ящик открыт"
		_objects[40]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEA front \xEF\xF0\xE0\xE2\xEE"); // "_МАСКА front к front право"
		_objects[41]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEA front \xEB\xE5\xE2\xEE"); // "_МАСКА front к front лево"
		_objects[42]  = _scene->object_interface("\xF4\xEE\xED front"); // "фон front"
		_objects[43]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEF\xF0\xE0\xE2\xEE \xEA front"); // "_МАСКА front право к front"
		_objects[44]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xF6\xE5\xED\xF2\xF0"); // "_МАСКА front центр"
		_objects[45]  = _scene->object_interface(BG_FRONT_RIGHT);
		_objects[46]  = _scene->object_interface("\xEA\xEE\xEC\xEE\xE4 \xEB\xE5\xE2\xFB\xE9 \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2"); // "комод левый ящик открыт"
		_objects[47]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEB\xE5\xE2\xFB\xE9 \xFF\xF9\xE8\xEA \xE7\xE0\xEA\xF0\xFB\xF2"); // "_МАСКА center левый ящик закрыт"
		_objects[48]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEB\xE5\xE2\xFB\xE9 \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2 \xEB\xE5\xE2\xEE"); // "_МАСКА center левый ящик открыт лево"
		_objects[49]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEB\xE5\xE2\xFB\xE9 \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2 \xEF\xF0\xE0\xE2\xEE"); // "_МАСКА center левый ящик открыт право"
		_objects[50]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 center \xFF\xE9\xF6\xE0"); // "накладка center яйца"
		_objects[51]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 center \xE7\xE5\xEB\xE5\xED\xFB\xE9 \xEF\xE0\xEA\xE5\xF2"); // "накладка center зеленый пакет"
		_objects[52]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 center \xEC\xE0\xF1\xEB\xEE"); // "накладка center масло"
		_objects[53]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 center \xEC\xEE\xEB\xEE\xEA\xEE"); // "накладка center молоко"
		_objects[54]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 center \xE2\xE5\xF1\xFB"); // "накладка center весы"
		_objects[55]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 center \xEC\xF3\xEA\xE0"); // "накладка center мука"
		_objects[56]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 center \xEA\xED\xE8\xE3\xE0 \xF1 \xF0\xE5\xF6\xE5\xEF\xF2\xE0\xEC\xE8"); // "накладка center книга с рецептами"
		_objects[57]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xE2\xE5\xF1\xFB"); // "_МАСКА center весы"
		_objects[58]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEA\xED\xE8\xE3\xE0 \xF1 \xF0\xE5\xF6\xE5\xEF\xF2\xE0\xEC\xE8"); // "_МАСКА center книга с рецептами"
		_objects[59]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEF\xF0\xE0\xE2\xFB\xE9 \xFF\xF9\xE8\xEA \xE7\xE0\xEA\xF0\xFB\xF2"); // "_МАСКА center правый ящик закрыт"
		_objects[60]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEF\xF0\xE0\xE2\xFB\xE9 \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2 \xEB\xE5\xE2\xEE"); // "_МАСКА center правый ящик открыт лево"
		_objects[61]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEF\xF0\xE0\xE2\xFB\xE9 \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2 \xEF\xF0\xE0\xE2\xEE"); // "_МАСКА center правый ящик открыт право"
		_objects[62]  = _scene->object_interface("\xEA\xEE\xEC\xEE\xE4 \xEF\xF0\xE0\xE2\xFB\xE9 \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2"); // "комод правый ящик открыт"
		_objects[63]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEB\xE5\xE2\xEE"); // "_МАСКА center лево"
		_objects[64]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEF\xF0\xE0\xE2\xEE"); // "_МАСКА center право"
		_objects[65]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xF4\xF0\xEE\xED\xF2"); // "_МАСКА center фронт"
		_objects[66]  = _scene->object_interface("\xF4\xEE\xED center"); // "фон center"
		_objects[67]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xEB\xE5\xE2\xFB\xE9 \xE7\xE0\xEA\xF0\xFB\xF2"); // "_МАСКА front ящик левый закрыт"
		_objects[68]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xEF\xF0\xE0\xE2\xFB\xE9 \xE7\xE0\xEA\xF0\xFB\xF2"); // "_МАСКА front ящик правый закрыт"
		_objects[69]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xEF\xF0\xE0\xE2\xFB\xE9 \xEE\xF2\xEA\xF0\xFB\xF2"); // "_МАСКА front ящик правый открыт"
		_objects[70]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xEB\xE5\xE2\xFB\xE9 \xEE\xF2\xEA\xF0\xFB\xF2"); // "_МАСКА front ящик левый открыт"
		_objects[71]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xEB\xE5\xE2\xEE \xEF\xE0\xEB\xEE\xF7\xEA\xE8"); // "накладка front лево палочки"
		_objects[72]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xEB\xE5\xE2\xEE \xF1\xEA\xE0\xEB\xEA\xE0"); // "накладка front лево скалка"
		_objects[73]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xFF\xF9\xE8\xEA \xEF\xF0\xE0\xE2\xFB\xE9 \xEE\xF2\xEA\xF0\xFB\xF2"); // "накладка front ящик правый открыт"
		_objects[74]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xFF\xF9\xE8\xEA \xEB\xE5\xE2\xFB\xE9 \xEE\xF2\xEA\xF0\xFB\xF2"); // "накладка front ящик левый открыт"
		_objects[75]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xF1\xEE\xEB\xFC"); // "_МАСКА front соль"
		_objects[76]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xF1\xEE\xEB\xFC"); // "накладка front соль"
		_objects[77]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xF6\xE5\xED\xF2\xF0\xE0\xEB\xFC\xED\xFB\xE9"); // "_МАСКА front ящик центральный"
		_objects[78]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2"); // "накладка front ящик открыт"
		_objects[79]  = _scene->object_interface(BG_FRONT_RIGHT);
		_objects[80]  = _scene->object_interface(BG_FRONT_LEFT);
		_objects[81]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEF\xF0\xE0\xE2\xEE \xEA front"); // "_МАСКА front право к front"
		_objects[82]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEB\xE5\xE2\xEE \xEA front"); // "_МАСКА front лево к front"
		_objects[83]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEA front \xEF\xF0\xE0\xE2\xEE"); // "_МАСКА front к front право"
		_objects[84]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEA front \xEB\xE5\xE2\xEE"); // "_МАСКА front к front лево"
		_objects[85]  = _scene->object_interface("\xF4\xEE\xED front"); // "фон front"
		_objects[86]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xF6\xE5\xED\xF2\xF0"); // "_МАСКА front центр"
		_objects[87]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xEB\xE5\xE2\xFB\xE9 \xE7\xE0\xEA\xF0\xFB\xF2"); // "_МАСКА front ящик левый закрыт"
		_objects[88]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xF1\xEE\xEB\xFC"); // "накладка front соль"
		_objects[89]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xF1\xEE\xEB\xFC"); // "_МАСКА front соль"
		_objects[90]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xF6\xE5\xED\xF2\xF0\xE0\xEB\xFC\xED\xFB\xE9"); // "_МАСКА front ящик центральный"
		_objects[91]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2"); // "накладка front ящик открыт"
		_objects[92]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEA front \xEF\xF0\xE0\xE2\xEE"); // "_МАСКА front к front право"
		_objects[93]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEA front \xEB\xE5\xE2\xEE"); // "_МАСКА front к front лево"
		_objects[94]  = _scene->object_interface("\xF4\xEE\xED front"); // "фон front"
		_objects[95]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEB\xE5\xE2\xEE \xEA front"); // "_МАСКА front лево к front"
		_objects[96]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xF6\xE5\xED\xF2\xF0"); // "_МАСКА front центр"
		_objects[97]  = _scene->object_interface(BG_FRONT_LEFT);
		_objects[98]  = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xEF\xF0\xE0\xE2\xFB\xE9 \xE7\xE0\xEA\xF0\xFB\xF2"); // "_МАСКА front ящик правый закрыт"
		_objects[99]  = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xF1\xEE\xEB\xFC"); // "накладка front соль"
		_objects[100] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xF1\xEE\xEB\xFC"); // "_МАСКА front соль"
		_objects[101] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xFF\xF9\xE8\xEA \xF6\xE5\xED\xF2\xF0\xE0\xEB\xFC\xED\xFB\xE9"); // "_МАСКА front ящик центральный"
		_objects[102] = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 front \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2"); // "накладка front ящик открыт"
		_objects[103] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEA front \xEF\xF0\xE0\xE2\xEE"); // "_МАСКА front к front право"
		_objects[104] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEA front \xEB\xE5\xE2\xEE"); // "_МАСКА front к front лево"
		_objects[105] = _scene->object_interface("\xF4\xEE\xED front"); // "фон front"
		_objects[106] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xEF\xF0\xE0\xE2\xEE \xEA front"); // "_МАСКА front право к front"
		_objects[107] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 front \xF6\xE5\xED\xF2\xF0"); // "_МАСКА front центр"
		_objects[108] = _scene->object_interface(BG_FRONT_RIGHT);
		_objects[109] = _scene->object_interface("\xEA\xEE\xEC\xEE\xE4 \xEB\xE5\xE2\xFB\xE9 \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2"); // "комод левый ящик открыт"
		_objects[110] = _scene->object_interface("\xEA\xEE\xEC\xEE\xE4 \xEF\xF0\xE0\xE2\xFB\xE9 \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2"); // "комод правый ящик открыт"
		_objects[111] = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 center \xE7\xE5\xEB\xE5\xED\xFB\xE9 \xEF\xE0\xEA\xE5\xF2"); // "накладка center зеленый пакет"
		_objects[112] = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 center \xEC\xE0\xF1\xEB\xEE"); // "накладка center масло"
		_objects[113] = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 center \xEC\xEE\xEB\xEE\xEA\xEE"); // "накладка center молоко"
		_objects[114] = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 center \xEC\xF3\xEA\xE0"); // "накладка center мука"
		_objects[115] = _scene->object_interface("\xED\xE0\xEA\xEB\xE0\xE4\xEA\xE0 center \xFF\xE9\xF6\xE0"); // "накладка center яйца"
		_objects[116] = _scene->object_interface("\xF4\xEE\xED center"); // "фон center"
		_objects[117] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xF4\xF0\xEE\xED\xF2"); // "_МАСКА center фронт"
		_objects[118] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEF\xF0\xE0\xE2\xEE"); // "_МАСКА center право"
		_objects[119] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEF\xF0\xE0\xE2\xFB\xE9 \xFF\xF9\xE8\xEA \xE7\xE0\xEA\xF0\xFB\xF2"); // "_МАСКА center правый ящик закрыт"
		_objects[120] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEB\xE5\xE2\xFB\xE9 \xFF\xF9\xE8\xEA \xE7\xE0\xEA\xF0\xFB\xF2"); // "_МАСКА center левый ящик закрыт"
		_objects[121] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEB\xE5\xE2\xEE"); // "_МАСКА center лево"
		_objects[122] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEB\xE5\xE2\xFB\xE9 \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2 \xEB\xE5\xE2\xEE"); // "_МАСКА center левый ящик открыт лево"
		_objects[123] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEB\xE5\xE2\xFB\xE9 \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2 \xEF\xF0\xE0\xE2\xEE"); // "_МАСКА center левый ящик открыт право"
		_objects[124] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEF\xF0\xE0\xE2\xFB\xE9 \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2 \xEB\xE5\xE2\xEE"); // "_МАСКА center правый ящик открыт лево"
		_objects[125] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEF\xF0\xE0\xE2\xFB\xE9 \xFF\xF9\xE8\xEA \xEE\xF2\xEA\xF0\xFB\xF2 \xEF\xF0\xE0\xE2\xEE"); // "_МАСКА center правый ящик открыт право"
		_objects[126] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xE7\xE5\xEB\xE5\xED\xFB\xE9 \xEF\xE0\xEA\xE5\xF2"); // "_МАСКА center зеленый пакет"
		_objects[127] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEC\xE0\xF1\xEB\xEE"); // "_МАСКА center масло"
		_objects[128] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEC\xEE\xEB\xEE\xEA\xEE"); // "_МАСКА center молоко"
		_objects[129] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xEC\xF3\xEA\xE0"); // "_МАСКА center мука"
		_objects[130] = _scene->object_interface("_\xCC\xC0\xD1\xCA\xC0 center \xFF\xE9\xF6\xE0"); // "_МАСКА center яйца"

  		return true;
	}

	bool quant(float dt) {
		debugC(3, kDebugMinigames, "3mice2States::quant(%f)", dt);

		_engine->set_interface_text(MAIN, "text", "START");

		if (!_objSelectedSet->is_state_active("none")) {
			if (_objSelectedSet->is_state_active(SET1)) {
				_engine->set_interface_text(MAIN, "text", SET1);
				_objects[1]->set_state(MASK2);
				_objects[2]->set_state(MASK1);
				_objects[3]->set_state(MASK3);
				_objects[4]->set_state(MASK2);
				_objects[5]->set_state(MASK1);
				_objects[6]->set_state(MASK2);
				_objects[7]->set_state(MASK2);
				_objects[8]->set_state(MASK2);
				_objects[9]->set_state(MASK2);
				_objects[10]->set_state(MASK2);
				_objects[11]->set_state(MASK2);
				_objects[12]->set_state(MASK1);
				_objects[13]->set_state(MASK1);
				_objects[14]->set_state(MASK2);
				_objects[15]->set_state(MASK2);
				_objects[16]->set_state(MASK2);
				_objects[17]->set_state(MASK2);
				_objects[18]->set_state(MASK1);
				_objects[19]->set_state(MASK1);
				_objects[20]->set_state(MASK1);
				_objects[21]->set_state(MASK1);
			} else if (_objSelectedSet->is_state_active(SET2)) {
				_engine->set_interface_text(MAIN, "text", SET2);
				_objects[22]->set_state(MASK3);
				_objects[23]->set_state(MASK3);
				_objects[24]->set_state(MASK3);
				_objects[25]->set_state(BG_MASK);
				_objects[26]->set_state(MASK2);
				_objects[27]->set_state(MASK3);
				_objects[28]->set_state(MASK2);
				_objects[29]->set_state(MASK2);
				_objects[30]->set_state(MASK2);
				_objects[31]->set_state(BG_MASK);
				_objects[32]->set_state(BG_MASK1);
				_objects[33]->set_state(BG_FRONT_LEFT);
			} else if (_objSelectedSet->is_state_active(SET3)) {
				_engine->set_interface_text(MAIN, "text", SET3);
				_objects[34]->set_state(MASK4);
				_objects[35]->set_state(MASK4);
				_objects[36]->set_state(MASK4);
				_objects[37]->set_state(BG_MASK);
				_objects[38]->set_state(MASK3);
				_objects[39]->set_state(MASK4);
				_objects[40]->set_state(MASK3);
				_objects[41]->set_state(MASK3);
				_objects[42]->set_state(MASK3);
				_objects[43]->set_state(BG_MASK);
				_objects[44]->set_state(BG_MASK2);
				_objects[45]->set_state(BG_FRONT_RIGHT);
			} else if (_objSelectedSet->is_state_active(SET4)) {
				_engine->set_interface_text(MAIN, "text", SET4);
				_objects[46]->set_state(MASK2);
				_objects[47]->set_state(MASK1);
				_objects[48]->set_state(MASK2);
				_objects[49]->set_state(MASK2);
				_objects[50]->set_state(MASK2);
				_objects[51]->set_state(MASK2);
				_objects[52]->set_state(MASK2);
				_objects[53]->set_state(MASK2);
				_objects[54]->set_state(MASK2);
				_objects[55]->set_state(MASK2);
				_objects[56]->set_state(MASK1);
				_objects[57]->set_state(MASK2);
				_objects[58]->set_state(MASK2);
				_objects[59]->set_state(MASK1);
				_objects[60]->set_state(MASK2);
				_objects[61]->set_state(MASK2);
				_objects[62]->set_state(MASK2);
				_objects[63]->set_state(MASK1);
				_objects[64]->set_state(MASK1);
				_objects[65]->set_state(MASK1);
				_objects[66]->set_state(MASK1);
			} else if (_objSelectedSet->is_state_active(SET5)) {
				_engine->set_interface_text(MAIN, "text", SET5);
				_objects[67]->set_state(MASK3);
				_objects[68]->set_state(MASK3);
				_objects[69]->set_state(MASK3);
				_objects[70]->set_state(MASK3);
				_objects[71]->set_state(MASK3);
				_objects[72]->set_state(MASK3);
				_objects[73]->set_state(MASK3);
				_objects[74]->set_state(MASK3);
				_objects[75]->set_state(MASK2);
				_objects[76]->set_state(MASK2);
				_objects[77]->set_state(MASK2);
				_objects[78]->set_state(MASK1);
				_objects[79]->set_state(MASK2);
				_objects[80]->set_state(MASK2);
				_objects[81]->set_state(MASK2);
				_objects[82]->set_state(MASK2);
				_objects[83]->set_state(MASK1);
				_objects[84]->set_state(MASK1);
				_objects[85]->set_state(MASK1);
				_objects[86]->set_state(MASK1);
			} else if (_objSelectedSet->is_state_active(SET6)) {
				_engine->set_interface_text(MAIN, "text", SET6);
				_objects[87]->set_state(BG_MASK);
				_objects[88]->set_state(MASK3);
				_objects[89]->set_state(MASK3);
				_objects[90]->set_state(MASK1);
				_objects[91]->set_state(MASK3);
				_objects[92]->set_state(MASK2);
				_objects[93]->set_state(MASK2);
				_objects[94]->set_state(MASK2);
				_objects[95]->set_state(BG_MASK);
				_objects[96]->set_state(BG_MASK1);
				_objects[97]->set_state(BG_FRONT_LEFT);
			} else if (_objSelectedSet->is_state_active(SET7)) {
				_engine->set_interface_text(MAIN, "text", SET7);
				_objects[98]->set_state(BG_MASK);
				_objects[99]->set_state(MASK4);
				_objects[100]->set_state(MASK4);
				_objects[101]->set_state(MASK3);
				_objects[102]->set_state(MASK4);
				_objects[103]->set_state(MASK3);
				_objects[104]->set_state(MASK3);
				_objects[105]->set_state(MASK3);
				_objects[106]->set_state(BG_MASK);
				_objects[107]->set_state(BG_MASK2);
				_objects[108]->set_state(BG_FRONT_RIGHT);
			} else if (_objSelectedSet->is_state_active(SET8)) {
				_objects[109]->set_state(MASK2);
				_objects[110]->set_state(MASK1);
				_objects[111]->set_state(MASK1);
				_objects[112]->set_state(MASK1);
				_objects[113]->set_state(MASK1);
				_objects[114]->set_state(MASK1);
				_objects[115]->set_state(MASK1);
				_objects[116]->set_state(MASK1);
				_objects[117]->set_state(MASK1);
				_objects[118]->set_state(MASK1);
				_objects[119]->set_state(MASK2);
				_objects[120]->set_state(MASK2);
				_objects[121]->set_state(MASK1);
				_objects[122]->set_state(MASK1);
				_objects[123]->set_state(MASK1);
				_objects[124]->set_state(MASK1);
				_objects[125]->set_state(MASK1);
				_objects[126]->set_state(MASK1);
				_objects[127]->set_state(MASK1);
				_objects[128]->set_state(MASK1);
				_objects[129]->set_state(MASK1);
				_objects[130]->set_state(MASK1);
			}

			_objSelectedSet->set_state("none");
		}

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
