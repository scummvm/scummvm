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

#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"
#include "bagel/hodjnpodj/metagame/gtl/gtlfrm.h"
#include "bagel/hodjnpodj/metagame/gtl/gtldoc.h"
#include "bagel/hodjnpodj/metagame/gtl/gtlview.h"
#include "bagel/hodjnpodj/metagame/gtl/encount.h"
#include "bagel/hodjnpodj/metagame/gtl/citemdlg.h"
#include "bagel/hodjnpodj/metagame/gtl/cturndlg.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

extern CBfcMgr      *lpMetaGameStruct;

// local prototypes
//
bool PlayEncounter(CWnd *, int);
int  FindEncounter(bool, int, long, int, int, int, int, bool *);
bool EligibleEncounter(int, bool, int, long);


static const CEncounterTable Encounters[MG_ENC_COUNT] = {
	{
		MG_SECTOR_ANY, "sound\\BB41.WAV",
		{MG_ACT_CROWNS, -14, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB42.WAV",
		{MG_ACT_CROWNS, -6, 0}, ""
	},

	{
		MG_SECTOR_BRIDGE, "sound\\BB39.WAV",
		{MG_ACT_OBJECT, -1, 0}, ""
	},

	{
		MG_SECTOR_BRIDGE, "sound\\BB40.WAV",
		{MG_ACT_GO, MG_SECTOR_CHUCKLE, 0}, ""
	},

	{
		MG_SECTOR_CHUCKLE, "sound\\BB19.WAV",
		{MG_ACT_GO, MG_SECTOR_RIVER, 0}, ""
	},

	{
		MG_SECTOR_CHUCKLE, "sound\\BB20.WAV",
		{MG_ACT_OBJECT, +1, 0}, ""
	},

	{
		MG_SECTOR_DESERT, "sound\\BB01.WAV",
		{MG_ACT_GO2, MG_SECTOR_CHUCKLE, MG_SECTOR_PATSHWERQUE, 0}, ""
	},

	{
		MG_SECTOR_DESERT, "sound\\BB02.WAV",
		{MG_ACT_OBJECT, -1, 0}, ""
	},

	{
		MG_SECTOR_ESTATE, "sound\\BB05.WAV",
		{MG_ACT_CROWNS, +50, 0}, ""
	},

	{
		MG_SECTOR_ESTATE, "sound\\BB06.WAV",
		{MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_FARM, "sound\\BB17.WAV",
		{MG_ACT_CROWNS, +25, MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_FARM, "sound\\BB18.WAV",
		{MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_GLACIER, "sound\\BB07.WAV",
		{MG_ACT_GO, MG_SECTOR_PASTEESH, 0}, ""
	},

	{
		MG_SECTOR_GLACIER, "sound\\BB08.WAV",
		{MG_ACT_OBJECT, +1, 0}, ""
	},

	{
		MG_SECTOR_HILLSIDE, "sound\\BB13.WAV",
		{MG_ACT_CROWNS, -15, 0}, ""
	},

	{
		MG_SECTOR_HILLSIDE, "sound\\BB14.WAV",
		{MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_LAKE, "sound\\BB31.WAV",
		{MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_LAKE, "sound\\BB32.WAV",
		{MG_ACT_OBJECT, +1, 0}, ""
	},

	{
		MG_SECTOR_MEADOW, "sound\\BB35.WAV",
		{MG_ACT_CROWNS, +18, 0}, ""
	},

	{
		MG_SECTOR_MEADOW, "sound\\BB36.WAV",
		{MG_ACT_TURN, +1, 0}, ""
	},

	{
		MG_SECTOR_MLPARK, "sound\\BB03.WAV",
		{MG_ACT_CROWNS, -6, MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_MLPARK, "sound\\BB04.WAV",
		{MG_ACT_CROWNS, -20, 0}, ""
	},

	{
		MG_SECTOR_MOUNTAINS, "sound\\BB29.WAV",
		{MG_ACT_CROWNS, +20, 0}, ""
	},

	{
		MG_SECTOR_MOUNTAINS, "sound\\BB30.WAV",
		{MG_ACT_GO, MG_SECTOR_MEADOW, 0}, ""
	},

	{
		MG_SECTOR_MOUNTAINVIEW, "sound\\BB33.WAV",
		{MG_ACT_TURN, +1, 0}, ""
	},

	{
		MG_SECTOR_MOUNTAINVIEW, "sound\\BB34.WAV",
		{MG_ACT_GO, MG_SECTOR_POPORREE, 0}, ""
	},

	{
		MG_SECTOR_PASTEESH, "sound\\BB09.WAV",
		{MG_ACT_CROWNS, -30, 0}, ""
	},

	{
		MG_SECTOR_PASTEESH, "sound\\BB10.WAV",
		{MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_PATSHWERQUE, "sound\\BB21.WAV",
		{MG_ACT_GO, MG_SECTOR_FARM, 0}, ""
	},

	{
		MG_SECTOR_PATSHWERQUE, "sound\\BB22.WAV",
		{MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_PLAINS, "sound\\BB11.WAV",
		{MG_ACT_CROWNS, -15, 0}, ""
	},

	{
		MG_SECTOR_PLAINS, "sound\\BB12.WAV",
		{MG_ACT_OBJECT, +1, 0}, ""
	},

	{
		MG_SECTOR_POPORREE, "sound\\BB25.WAV",
		{MG_ACT_CROWNS, +16, 0}, ""
	},

	{
		MG_SECTOR_POPORREE, "sound\\BB26.WAV",
		{MG_ACT_TURN, +1, 0}, ""
	},

	{
		MG_SECTOR_RIVER, "sound\\BB37.WAV",
		{MG_ACT_TURN, +1, 0}, ""
	},

	{
		MG_SECTOR_RIVER, "sound\\BB38.WAV",
		{MG_ACT_OBJECT, +1, 0}, ""
	},

	{
		MG_SECTOR_SWAMP, "sound\\BB23.WAV",
		{MG_ACT_CROWNS, -8, 0}, ""
	},

	{
		MG_SECTOR_SWAMP, "sound\\BB24.WAV",
		{MG_ACT_OBJECT, -1, 0}, ""
	},

	{
		MG_SECTOR_VOLCANO, "sound\\BB15.WAV",
		{MG_ACT_CROWNS, -15, 0}, ""
	},

	{
		MG_SECTOR_VOLCANO, "sound\\BB16.WAV",
		{MG_ACT_CROWNS, +35, 0}, ""
	},

	{
		MG_SECTOR_WITTY, "sound\\BB27.WAV",
		{MG_ACT_TURN, +1, 0}, ""
	},

	{
		MG_SECTOR_WITTY, "sound\\BB28.WAV",
		{MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB43.WAV",
		{MG_ACT_CROWNS, +16, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB44.WAV",
		{MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB45.WAV",
		{MG_ACT_CROWNS, +10, MG_ACT_OBJECT, -1, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB46.WAV",
		{MG_ACT_CROWNS, +22, MG_ACT_OBJECT, +1, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB47.WAV",
		{MG_ACT_CROWNS, -11, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB48.WAV",
		{MG_ACT_GOCOND, MG_SECTOR_LAKE, MG_SECTOR_HILLSIDE, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB49.WAV",
		{MG_ACT_GOCOND, MG_SECTOR_GLACIER, MG_SECTOR_MOUNTAINS, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB50.WAV",
		{MG_ACT_OBJECT, -1, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB51.WAV",
		{MG_ACT_CROWNS, -6, MG_ACT_TURN, +1, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB52.WAV",
		{MG_ACT_CROWNS, +21, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB53.WAV",
		{MG_ACT_CROWNS, -18, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB54.WAV",
		{MG_ACT_CROWNS, -17, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB55.WAV",
		{MG_ACT_TURN, +1, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB56.WAV",
		{MG_ACT_OBJECT, -1, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB57.WAV",
		{MG_ACT_CROWNS, +26, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB58.WAV",
		{MG_ACT_CROWNS, -9, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB59.WAV",
		{MG_ACT_CROWNS, -15, 0}, ""
	},

	{
		MG_SECTOR_ANY, "sound\\BB60.WAV",
		{MG_ACT_CROWNS, -12, 0}, ""
	},

	//
	// Start of Color Narrations
	//

	{MG_SECTOR_ANY, "sound\\CL161.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL168.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL162.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL169.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL163.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL170.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL164.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL171.WAV", { MG_ACT_PODJ, 0 }, ""},

	{MG_SECTOR_BRIDGE, "sound\\CL153.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_BRIDGE, "sound\\CL154.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_BRIDGE, "sound\\CL155.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_BRIDGE, "sound\\CL156.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_BRIDGE, "sound\\CL157.WAV", { 0 }, ""},
	{MG_SECTOR_BRIDGE, "sound\\CL158.WAV", { 0 }, ""},
	{MG_SECTOR_BRIDGE, "sound\\CL159.WAV", { 0 }, ""},
	{MG_SECTOR_BRIDGE, "sound\\CL160.WAV", { 0 }, ""},

	{MG_SECTOR_CHUCKLE, "sound\\CL073.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_CHUCKLE, "sound\\CL074.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_CHUCKLE, "sound\\CL075.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_CHUCKLE, "sound\\CL076.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_CHUCKLE, "sound\\CL077.WAV", { 0 }, ""},
	{MG_SECTOR_CHUCKLE, "sound\\CL078.WAV", { 0 }, ""},
	{MG_SECTOR_CHUCKLE, "sound\\CL079.WAV", { 0 }, ""},
	{MG_SECTOR_CHUCKLE, "sound\\CL080.WAV", { 0 }, ""},

	{MG_SECTOR_DESERT, "sound\\CL001.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_DESERT, "sound\\CL002.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_DESERT, "sound\\CL003.WAV", { 0 }, ""},
	{MG_SECTOR_DESERT, "sound\\CL004.WAV", { 0 }, ""},
	{MG_SECTOR_DESERT, "sound\\CL005.WAV", { 0 }, ""},
	{MG_SECTOR_DESERT, "sound\\CL006.WAV", { 0 }, ""},
	{MG_SECTOR_DESERT, "sound\\CL007.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_DESERT, "sound\\CL008.WAV", { MG_ACT_PODJ, 0 }, ""},

	{MG_SECTOR_ESTATE, "sound\\CL017.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ESTATE, "sound\\CL018.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_ESTATE, "sound\\CL019.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ESTATE, "sound\\CL020.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_ESTATE, "sound\\CL021.WAV", { 0 }, ""},
	{MG_SECTOR_ESTATE, "sound\\CL022.WAV", { 0 }, ""},
	{MG_SECTOR_ESTATE, "sound\\CL023.WAV", { 0 }, ""},
	{MG_SECTOR_ESTATE, "sound\\CL024.WAV", { 0 }, ""},

	{MG_SECTOR_FARM, "sound\\CL065.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_FARM, "sound\\CL066.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_FARM, "sound\\CL067.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_FARM, "sound\\CL068.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_FARM, "sound\\CL069.WAV", { 0 }, ""},
	{MG_SECTOR_FARM, "sound\\CL070.WAV", { 0 }, ""},
	{MG_SECTOR_FARM, "sound\\CL071.WAV", { 0 }, ""},
	{MG_SECTOR_FARM, "sound\\CL072.WAV", { 0 }, ""},

	{MG_SECTOR_GLACIER, "sound\\CL025.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_GLACIER, "sound\\CL026.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_GLACIER, "sound\\CL027.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_GLACIER, "sound\\CL028.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_GLACIER, "sound\\CL029.WAV", { 0 }, ""},
	{MG_SECTOR_GLACIER, "sound\\CL030.WAV", { 0 }, ""},
	{MG_SECTOR_GLACIER, "sound\\CL031.WAV", { 0 }, ""},
	{MG_SECTOR_GLACIER, "sound\\CL032.WAV", { 0 }, ""},

	{MG_SECTOR_HILLSIDE, "sound\\CL049.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_HILLSIDE, "sound\\CL050.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_HILLSIDE, "sound\\CL051.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_HILLSIDE, "sound\\CL052.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_HILLSIDE, "sound\\CL053.WAV", { 0 }, ""},
	{MG_SECTOR_HILLSIDE, "sound\\CL054.WAV", { 0 }, ""},
	{MG_SECTOR_HILLSIDE, "sound\\CL055.WAV", { 0 }, ""},
	{MG_SECTOR_HILLSIDE, "sound\\CL056.WAV", { 0 }, ""},

	{MG_SECTOR_LAKE, "sound\\CL121.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_LAKE, "sound\\CL122.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_LAKE, "sound\\CL123.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_LAKE, "sound\\CL124.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_LAKE, "sound\\CL125.WAV", { 0 }, ""},
	{MG_SECTOR_LAKE, "sound\\CL126.WAV", { 0 }, ""},
	{MG_SECTOR_LAKE, "sound\\CL127.WAV", { 0 }, ""},
	{MG_SECTOR_LAKE, "sound\\CL128.WAV", { 0 }, ""},

	{MG_SECTOR_MEADOW, "sound\\CL137.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_MEADOW, "sound\\CL138.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_MEADOW, "sound\\CL139.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_MEADOW, "sound\\CL140.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_MEADOW, "sound\\CL141.WAV", { 0 }, ""},
	{MG_SECTOR_MEADOW, "sound\\CL142.WAV", { 0 }, ""},
	{MG_SECTOR_MEADOW, "sound\\CL143.WAV", { 0 }, ""},
	{MG_SECTOR_MEADOW, "sound\\CL144.WAV", { 0 }, ""},

	{MG_SECTOR_MLPARK, "sound\\CL009.WAV", { 0 }, ""},
	{MG_SECTOR_MLPARK, "sound\\CL010.WAV", { 0 }, ""},
	{MG_SECTOR_MLPARK, "sound\\CL011.WAV", { 0 }, ""},
	{MG_SECTOR_MLPARK, "sound\\CL012.WAV", { 0 }, ""},
	{MG_SECTOR_MLPARK, "sound\\CL013.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_MLPARK, "sound\\CL014.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_MLPARK, "sound\\CL015.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_MLPARK, "sound\\CL016.WAV", { MG_ACT_PODJ, 0 }, ""},

	{MG_SECTOR_MOUNTAINS, "sound\\CL113.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_MOUNTAINS, "sound\\CL114.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_MOUNTAINS, "sound\\CL115.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_MOUNTAINS, "sound\\CL116.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_MOUNTAINS, "sound\\CL117.WAV", { 0 }, ""},
	{MG_SECTOR_MOUNTAINS, "sound\\CL118.WAV", { 0 }, ""},
	{MG_SECTOR_MOUNTAINS, "sound\\CL119.WAV", { 0 }, ""},
	{MG_SECTOR_MOUNTAINS, "sound\\CL120.WAV", { 0 }, ""},

	{MG_SECTOR_MOUNTAINVIEW, "sound\\CL129.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_MOUNTAINVIEW, "sound\\CL130.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_MOUNTAINVIEW, "sound\\CL131.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_MOUNTAINVIEW, "sound\\CL132.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_MOUNTAINVIEW, "sound\\CL133.WAV", { 0 }, ""},
	{MG_SECTOR_MOUNTAINVIEW, "sound\\CL134.WAV", { 0 }, ""},
	{MG_SECTOR_MOUNTAINVIEW, "sound\\CL135.WAV", { 0 }, ""},
	{MG_SECTOR_MOUNTAINVIEW, "sound\\CL136.WAV", { 0 }, ""},

	{MG_SECTOR_PASTEESH, "sound\\CL033.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_PASTEESH, "sound\\CL034.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_PASTEESH, "sound\\CL035.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_PASTEESH, "sound\\CL036.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_PASTEESH, "sound\\CL037.WAV", { 0 }, ""},
	{MG_SECTOR_PASTEESH, "sound\\CL038.WAV", { 0 }, ""},
	{MG_SECTOR_PASTEESH, "sound\\CL039.WAV", { 0 }, ""},
	{MG_SECTOR_PASTEESH, "sound\\CL040.WAV", { 0 }, ""},

	{MG_SECTOR_PATSHWERQUE, "sound\\CL081.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_PATSHWERQUE, "sound\\CL082.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_PATSHWERQUE, "sound\\CL083.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_PATSHWERQUE, "sound\\CL084.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_PATSHWERQUE, "sound\\CL085.WAV", { 0 }, ""},
	{MG_SECTOR_PATSHWERQUE, "sound\\CL086.WAV", { 0 }, ""},
	{MG_SECTOR_PATSHWERQUE, "sound\\CL087.WAV", { 0 }, ""},
	{MG_SECTOR_PATSHWERQUE, "sound\\CL088.WAV", { 0 }, ""},

	{MG_SECTOR_PLAINS, "sound\\CL041.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_PLAINS, "sound\\CL042.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_PLAINS, "sound\\CL043.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_PLAINS, "sound\\CL044.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_PLAINS, "sound\\CL045.WAV", { 0 }, ""},
	{MG_SECTOR_PLAINS, "sound\\CL046.WAV", { 0 }, ""},
	{MG_SECTOR_PLAINS, "sound\\CL047.WAV", { 0 }, ""},
	{MG_SECTOR_PLAINS, "sound\\CL048.WAV", { 0 }, ""},

	{MG_SECTOR_POPORREE, "sound\\CL097.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_POPORREE, "sound\\CL098.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_POPORREE, "sound\\CL099.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_POPORREE, "sound\\CL100.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_POPORREE, "sound\\CL101.WAV", { 0 }, ""},
	{MG_SECTOR_POPORREE, "sound\\CL102.WAV", { 0 }, ""},
	{MG_SECTOR_POPORREE, "sound\\CL103.WAV", { 0 }, ""},
	{MG_SECTOR_POPORREE, "sound\\CL104.WAV", { 0 }, ""},

	{MG_SECTOR_RIVER, "sound\\CL145.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_RIVER, "sound\\CL146.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_RIVER, "sound\\CL147.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_RIVER, "sound\\CL148.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_RIVER, "sound\\CL149.WAV", { 0 }, ""},
	{MG_SECTOR_RIVER, "sound\\CL150.WAV", { 0 }, ""},
	{MG_SECTOR_RIVER, "sound\\CL151.WAV", { 0 }, ""},
	{MG_SECTOR_RIVER, "sound\\CL152.WAV", { 0 }, ""},

	{MG_SECTOR_SWAMP, "sound\\CL089.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_SWAMP, "sound\\CL090.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_SWAMP, "sound\\CL091.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_SWAMP, "sound\\CL092.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_SWAMP, "sound\\CL093.WAV", { 0 }, ""},
	{MG_SECTOR_SWAMP, "sound\\CL094.WAV", { 0 }, ""},
	{MG_SECTOR_SWAMP, "sound\\CL095.WAV", { 0 }, ""},
	{MG_SECTOR_SWAMP, "sound\\CL096.WAV", { 0 }, ""},

	{MG_SECTOR_VOLCANO, "sound\\CL057.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_VOLCANO, "sound\\CL058.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_VOLCANO, "sound\\CL059.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_VOLCANO, "sound\\CL060.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_VOLCANO, "sound\\CL061.WAV", { 0 }, ""},
	{MG_SECTOR_VOLCANO, "sound\\CL062.WAV", { 0 }, ""},
	{MG_SECTOR_VOLCANO, "sound\\CL063.WAV", { 0 }, ""},
	{MG_SECTOR_VOLCANO, "sound\\CL064.WAV", { 0 }, ""},

	{MG_SECTOR_WITTY, "sound\\CL105.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_WITTY, "sound\\CL106.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_WITTY, "sound\\CL107.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_WITTY, "sound\\CL108.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_WITTY, "sound\\CL109.WAV", { 0 }, ""},
	{MG_SECTOR_WITTY, "sound\\CL110.WAV", { 0 }, ""},
	{MG_SECTOR_WITTY, "sound\\CL111.WAV", { 0 }, ""},
	{MG_SECTOR_WITTY, "sound\\CL112.WAV", { 0 }, ""},

	{MG_SECTOR_ANY, "sound\\CL165.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL166.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL167.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL172.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL173.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL174.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL175.WAV", { 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL176.WAV", { 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL177.WAV", { 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL178.WAV", { 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL179.WAV", { 0 }, ""},
	{MG_SECTOR_ANY, "sound\\CL180.WAV", { 0 }, ""},
};


/*****************************************************************
 *
 * DoEncounter
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Called from the meta-game
 *      Use the status of the players to choose either
 *      a Booby Trap or a Colour Narration, and then
 *      execute the Encounter.
 *
 * FORMAL PARAMETERS:
 *
 *      bool        bHodj           Flag to tell which player this is
 *      CInventory  *pInventory     pointer to the player's inventory object
 *      int         nNumSteps       Number of steps in the current move
 *      unsigned int        nPSector        Sector of player
 *      unsigned int        nOSector        Sector of opponent
 *      bool        bArray          Array of whether an encounter has been used already
 *
 * RETURN VALUE:
 *
 *      int     what action, if any, is to be taken as a result
 *              of a trap or narration
 *
 ****************************************************************/

int DoEncounter(CWnd *pWnd, CPalette *pPalette, bool bHodj, CInventory *pInventory,
                CInventory *pPawn, CInventory *pGeneral,
                int nEncType, int nNumSteps, int nPSector, int nOSector, bool *pArray) {
	CInventory  *pTryOne, *pTryTwo;             // Temp pointers for Pawn & General to randomly pick one
	CItem   *pItem;                             // Item pointer for player Inventory changes
	long    lCrowns = 0;
	int     nItems = 0;
	int     nID;
	int     i;
	int     nPick,                              // Holder for random number to choose Pawn or General
	        nRandFactor,                        // No. within given range of crowns to add or to remove
	        nChangeAmount;                      // Amount of change to inventory item, turns, etc.
	int     nNarrsLeft = MG_NARR_COUNT,
	        nTestProb;
	int     RetVal = ENC_DO_NOTHING;
	bool    bDone;                              // flag to check if an appropriate object was found
	bool    bFoundTrap = false;
	int     nRandom = 0;

	if (nEncType == ENC_BOOBYTRAP) {
		for (i = 0; i < MG_TRAP_COUNT; i++) {                       // Traps are at the front of the array
			if (!pArray[i]) {                                       // Check to see if there are any
				bFoundTrap = true;                                  // traps left
				break;
			}
		} // end for

		if (bFoundTrap) {                                           // If there are TRAPS LEFT
			nTestProb = TRAP_PROB;                                  // Get probability, scaled by ten
			nRandom = brand() % RAND_FACTOR;
			if (nTestProb >= nRandom) {                                        // If we are in the probability percentage...
				// check ItemCount(), set Item flag
				pItem = (*pInventory).FindItem(MG_OBJ_MISH);                    // See if they have mish or mosh
				if (pItem != nullptr)                                              // don't count crowns and notebook
					nItems = (*pInventory).ItemCount() - INVENT_MIN_ITEM_COUNT - ENC_MISHMOSH; // and Mish & Mosh
				else
					nItems = (*pInventory).ItemCount() - INVENT_MIN_ITEM_COUNT;     // don't count crowns and notebook
				pItem = (*pInventory).FindItem(MG_OBJ_CROWN);               // get money item obj, set Money int to am't
				if (pItem == nullptr)
					lCrowns = 0;
				else
					lCrowns = (int)(*pItem).GetQuantity();

				// players crowns in his inventory is bogus if this fails
				assert(lCrowns >= 0);

				nID = FindEncounter(bHodj, nPSector, lCrowns, nItems, 0, MG_TRAP_COUNT, TRAP_PER_SECTOR, pArray);
				if (nID >= 0) {
					pArray[nID] = PlayEncounter(pWnd, nID);                 // play encounter and set it to USED
					i = 0;
					while (Encounters[nID].m_Actions[i] != 0) {             // Until there are no more Actions
						if (Encounters[nID].m_Actions[i] == MG_ACT_GO) {    // If it's a Go to
							RetVal = Encounters[nID].m_Actions[i + 1];      // Return the new location
							i++;                                            // skip the one we just read in
						} else if (Encounters[nID].m_Actions[i] == MG_ACT_GO2) {    // Go to one of 2 locations
							RetVal = Encounters[nID].m_Actions[i + (brand() % 2) + 1];   // Randomly pick one
							i += 2;                                                   // skip the two we just read in
						} else if (Encounters[nID].m_Actions[i] == MG_ACT_GOCOND) { // Go where the opponent isn't
							RetVal = Encounters[nID].m_Actions[i + 1];              // Get the first
							if (nOSector == RetVal)                                 // Check if Opponent is there
								RetVal = Encounters[nID].m_Actions[i + 2];          // If so, return the second loc.
							i += 2;                                                   // skip the two we just read in
						} else if (Encounters[nID].m_Actions[i] == MG_ACT_CROWNS) { // Effects the player's crowns
							nRandFactor = Encounters[nID].m_Actions[i + 1];         // Get limit of crown change
							if (nRandFactor < 0) {                                  // If we are to remove crowns

								// we should not have been eligible for this action
								// if we have no crowns
								assert(lCrowns != 0);

								nRandFactor = -(int)min((long)abs(nRandFactor), lCrowns);// can't remove more gold than the player has

								nChangeAmount = brand() % (nRandFactor + 1);           //...subtract from 1 to nRandFactor
								lCrowns -= nChangeAmount;
							} else {
								nChangeAmount = brand() % (nRandFactor + 1);           // Add from 1 to nRandFactor
								lCrowns += nChangeAmount;
							}
							// our logic was wrong if we end up negative
							assert(lCrowns >= 0);

							if (lCrowns < 0)                                        // If we end up with negative
								lCrowns = 0;                                        //...crowns, make it zero

							pItem = (*pInventory).FindItem(MG_OBJ_CROWN);           // get money item obj
							if (pItem != nullptr)                                      //...if successful
								(*pItem).SetQuantity(lCrowns);                      //...set Money int to am't
							CItemDialog ItemDlg(pWnd, pPalette, pItem, bHodj, (nRandFactor > 0) ? true : false, nChangeAmount);
							i++;                                                    // skip the one we just read in
						} else if (Encounters[nID].m_Actions[i] == MG_ACT_OBJECT) { // Trap effects the inventory
							if (Encounters[nID].m_Actions[i + 1] > 0) {             // If we are to add an item
								nPick = brand() % 2;                                 // Randomly select one
								if (nPick == 0) {                                   //...of the two Sources
									pTryOne = pGeneral;
									pTryTwo = pPawn;
								} else {
									pTryOne = pPawn;
									pTryTwo = pGeneral;
								}
								if ((*pTryOne).ItemCount() > 0) {                   // If the first one has items
									nItems = (*pTryOne).ItemCount();                // How many items
									pItem = (*pTryOne).FetchItem(brand() % nItems);  // Get one and
									(*pTryOne).RemoveItem(pItem);                   //...remove it to pItem
								} else {                                            // Otherwise, get the first
									nItems = (*pTryTwo).ItemCount();                // How many items
									pItem = (*pTryTwo).FetchItem(brand() % nItems);  // Get one and
									(*pTryTwo).RemoveItem(pItem);                   //...and remove _it_ to pItem
								}
								(*pInventory).AddItem(pItem);                       // And put pItem in the player's
								CItemDialog ItemDlg(pWnd, pPalette, pItem, bHodj, true, 1);
							}                                                       //...backpack Inventory
							else {                                                  // Otherwise, he loses an item
								nItems = (*pInventory).ItemCount() - INVENT_MIN_ITEM_COUNT;     // don't count crowns and notebook
								bDone = false;
								while (!bDone) {
									assert(nItems != 0);
									pItem = (*pInventory).FetchItem(INVENT_MIN_ITEM_COUNT + brand() % nItems);   // Get one
									if ((pItem != nullptr) &&                          //...make sure it's valid
									        ((pItem->GetID() != MG_OBJ_MISH) &&
									         (pItem->GetID() != MG_OBJ_MOSH))) {

										(*pInventory).RemoveItem(pItem);            // Remove it from the backpack
										nPick = brand() % 2;                         //...get a random destination
										if (nPick == 0)
											(*pGeneral).AddItem(pItem);             //...and put it there
										else
											(*pPawn).AddItem(pItem);
										CItemDialog ItemDlg(pWnd, pPalette, pItem, bHodj, false, 1);
										bDone = true;
									} // end if
								} // end while
							} // end elses
							i++;                                                    // skip the one we just read in
						} else if (Encounters[nID].m_Actions[i] == MG_ACT_TURN) {   // If he will lose or gain a turn
							if (Encounters[nID].m_Actions[i + 1] > 0) {                 // If it's gain a turn
								RetVal = ENC_GAIN_TURN;                             //...return that flag
								CTurnDialog TurnDlg(pWnd, pPalette, bHodj, true, true);
							} else {
								RetVal = ENC_LOSE_TURN;                             //...or return the lose turn flag
								CTurnDialog TurnDlg(pWnd, pPalette, bHodj, false, true);
							}
							i++;                                                    // skip the one we just read in
						}
						i++;                                                        //  Go to the next Action flag
					} // end while
					return (RetVal);                                                // successful trap trigger
				} // end if nID
			} // end if TestProb
		} // end if Traps left
	} // end if ENC_BOOBYTRAP

	else if (nEncType == ENC_NARRATION) {
		for (i = MG_TRAP_COUNT; i < MG_ENC_COUNT; i++) {                    // Narrations are after the traps in the array
			if (pArray[i])                                                  // for every narration that's been used
				nNarrsLeft--;                                               //...take one off of the total
		} // end for

		if (nNarrsLeft != 0) {                                              // NARR'S LEFT
			nTestProb = nNumSteps * NARR_PROB;                              // Get probability, scaled by ten
			nRandom = (brand() % RAND_FACTOR);
			if (nTestProb >= nRandom) {                                    // If we are in the probability percentage...
				nID = FindEncounter(bHodj, nPSector, 0, 0, MG_TRAP_COUNT, MG_NARR_COUNT, NARR_PER_SECTOR, pArray);
				if (nID >= MG_TRAP_COUNT) {                                 // if it's a valid NARRATION ID
					pArray[nID] = PlayEncounter(pWnd, nID);                 // play encounter and set it to USED
					return (ENC_DO_NOTHING);                                // successful narration trigger
				}
			} // end if TestProb
		} // end if NarrsLeft
	} // end if ENC_NARRATION

	return (ENC_DO_NOTHING);                                            //...return to meta game

} // end DoEncounter


/*****************************************************************
 *
 * FindEncounter
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Find and encounter in the given encounter table which matches the criteria given
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *     int          the ID of the Table member which fits the criteria
 *
 ****************************************************************/

int FindEncounter(bool bHodj, int nPlayerSector, long lCrowns, int nItems,
                  int nStartEncounter, int nNumEncounters, int nEncPerSector,
                  bool *pArray) {
	int nStart = 0;                     // random point to start looking for encounter
	int nGeneral = 0;                   // number of Generally-applicable encounters
	int nID = 0;                        // ID of table member we're looking at
	int i = 0;
	int nFirstEnc = nStartEncounter + ((nPlayerSector - MG_SECTOR_BASE) * nEncPerSector);  // First Enc for nPlayerSector

	if (nFirstEnc >= 0) {

		nID = nFirstEnc;                                                                // Go to where that sector is in the table
		assert(nEncPerSector != 0);
		nStart = brand() % nEncPerSector;                                                // Start at a random point
		nID += nStart;                                                                  //...in that sector's range
		for (i = 0; i < nEncPerSector; i++) {                                           // Step thru sector-spec encs

			// if not already used
			if (pArray[nID] == false) {

				// if eligible for this action
				if (EligibleEncounter(nID, bHodj, nItems, lCrowns)) {
					return (nID);
				}
			}

			// if nID out of range
			//
			if (++nID >= (nFirstEnc + nEncPerSector)) {

				//...go back to start
				nID = nFirstEnc;
			}
		} // end for i
	} // end if

	// ANY encounters are all the encounters left after the specific ones
	//...the total count, less the specific enc's, less MG_SECTOR_ANY, times the specific ones per sector

	nGeneral = nNumEncounters - (MG_SECTOR_COUNT * nEncPerSector);                  // no. of general encounters
	assert(nGeneral != 0);
	nID = nStartEncounter + (MG_SECTOR_COUNT * nEncPerSector) + (brand() % nGeneral);// Get random start spot
	for (i = 0; i < nGeneral; i++) {                                                // Go thru all the general ones

		// if not already used
		//
		if (pArray[nID] == false) {                                                 // If it's unused

			// if eligible for this action
			if (EligibleEncounter(nID, bHodj, nItems, lCrowns)) {
				return (nID);
			}
		}

		if (++nID >= (nStartEncounter + (MG_SECTOR_COUNT * nEncPerSector) + nGeneral)) {   // If out of range
			nID = nStartEncounter + (MG_SECTOR_COUNT * nEncPerSector);                            //...back to first
		}
	}

	// return unusable value
	return -1;
}

/*****************************************************************
 *
 * PlayEncounter
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Initialize, and begin playing, an encounter narration.
 *      If it is a BoobyTrap, make the corresponding adjustments
 *      to the player's inventory and location, as required
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      bool            success / failure condition
 *
 ****************************************************************/

bool PlayEncounter(CWnd *pWnd, int nID) {
	CGtlFrame   *pFrame = (CGtlFrame *) pWnd;
	CSound      *pNarration = nullptr;
	CGtlDoc     * xpGtlDoc = nullptr ;
	CGtlView    * xpGtlFocusView, *xpGtlMouseView ;

	if (Encounters[nID].m_lpszWavEFile == nullptr)                         // punt if no spec
		return false;

	pFrame->GetCurrentDocAndView(xpGtlDoc, xpGtlFocusView, xpGtlMouseView) ;

	pNarration = new CSound(xpGtlDoc->m_xpGtlData->m_xpGtlView, Encounters[nID].m_lpszWavEFile,   // get a new sound object in
	                        SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH | SOUND_AUTODELETE);       //...wave format, which deletes

	if (pNarration == nullptr)
		return false;

	(*pNarration).setDrivePath(lpMetaGameStruct->m_chCDPath);
	(*pNarration).play();                                           //...play the narration
	if (nID < MG_TRAP_COUNT)                                            // For Booby Traps:
		CSound::waitWaveSounds();

	return true;
}


bool EligibleEncounter(int nID, bool bHodj, int nItems, long lCrowns) {
	int nAction = 0;                    // Holder for action to be taken
	int i;
	bool bEligible;

	// assume eligible, must prove otherwise
	bEligible = true;

	// Until there are no more Actions
	//
	i = 0;
	while ((nAction = Encounters[nID].m_Actions[i]) != 0) {

		switch (nAction) {

		// No action
		//
		case 0:
			break;


		// GoTo action
		//
		case MG_ACT_GO:

			// must be a booby trap
			assert(nID < MG_TRAP_COUNT);

			i += 1;
			break;


		// GoTo random location
		//
		case MG_ACT_GO2:
		case MG_ACT_GOCOND:

			// must be a booby trap
			assert(nID < MG_TRAP_COUNT);

			i += 2;
			break;


		// Gain/Lose Turn
		//
		case MG_ACT_TURN:

			// must be a booby trap
			assert(nID < MG_TRAP_COUNT);

			i += 1;
			break;


		// Gain/Lose money
		//
		case MG_ACT_CROWNS:

			// must be a booby trap
			assert(nID < MG_TRAP_COUNT);

			if ((lCrowns <= 0) && (Encounters[nID].m_Actions[i + 1] < 0)) {
				bEligible = false;
			}
			i += 1;
			break;


		// Gain/Lose Object
		//
		case MG_ACT_OBJECT:

			// must be a booby trap
			assert(nID < MG_TRAP_COUNT);

			if ((nItems >= MG_OBJ_COUNT) && (Encounters[nID].m_Actions[i + 1] > 0)) {
				bEligible = false;

			} else if ((nItems <= 0) && (Encounters[nID].m_Actions[i + 1] < 0)) {
				bEligible = false;
			}
			i += 1;
			break;


		// Hodj Specific narration
		//
		case MG_ACT_HODJ:
			if (!bHodj) {
				bEligible = false;
			}
			break;


		// Podj specific narration
		//
		case MG_ACT_PODJ:
			if (bHodj) {
				bEligible = false;
			}
			break;


		// Error, this case not handled
		//
		default:
			assert(0);
			break;
		}
		i++;
	} // end while

	return (bEligible);
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
