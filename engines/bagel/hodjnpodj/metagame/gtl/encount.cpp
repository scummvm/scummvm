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
		MG_SECTOR_ANY, ".\\SOUND\\BB41.WAV",
		{MG_ACT_CROWNS, -14, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB42.WAV",
		{MG_ACT_CROWNS, -6, 0}, ""
	},

	{
		MG_SECTOR_BRIDGE, ".\\SOUND\\BB39.WAV",
		{MG_ACT_OBJECT, -1, 0}, ""
	},

	{
		MG_SECTOR_BRIDGE, ".\\SOUND\\BB40.WAV",
		{MG_ACT_GO, MG_SECTOR_CHUCKLE, 0}, ""
	},

	{
		MG_SECTOR_CHUCKLE, ".\\SOUND\\BB19.WAV",
		{MG_ACT_GO, MG_SECTOR_RIVER, 0}, ""
	},

	{
		MG_SECTOR_CHUCKLE, ".\\SOUND\\BB20.WAV",
		{MG_ACT_OBJECT, +1, 0}, ""
	},

	{
		MG_SECTOR_DESERT, ".\\SOUND\\BB01.WAV",
		{MG_ACT_GO2, MG_SECTOR_CHUCKLE, MG_SECTOR_PATSHWERQUE, 0}, ""
	},

	{
		MG_SECTOR_DESERT, ".\\SOUND\\BB02.WAV",
		{MG_ACT_OBJECT, -1, 0}, ""
	},

	{
		MG_SECTOR_ESTATE, ".\\SOUND\\BB05.WAV",
		{MG_ACT_CROWNS, +50, 0}, ""
	},

	{
		MG_SECTOR_ESTATE, ".\\SOUND\\BB06.WAV",
		{MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_FARM, ".\\SOUND\\BB17.WAV",
		{MG_ACT_CROWNS, +25, MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_FARM, ".\\SOUND\\BB18.WAV",
		{MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_GLACIER, ".\\SOUND\\BB07.WAV",
		{MG_ACT_GO, MG_SECTOR_PASTEESH, 0}, ""
	},

	{
		MG_SECTOR_GLACIER, ".\\SOUND\\BB08.WAV",
		{MG_ACT_OBJECT, +1, 0}, ""
	},

	{
		MG_SECTOR_HILLSIDE, ".\\SOUND\\BB13.WAV",
		{MG_ACT_CROWNS, -15, 0}, ""
	},

	{
		MG_SECTOR_HILLSIDE, ".\\SOUND\\BB14.WAV",
		{MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_LAKE, ".\\SOUND\\BB31.WAV",
		{MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_LAKE, ".\\SOUND\\BB32.WAV",
		{MG_ACT_OBJECT, +1, 0}, ""
	},

	{
		MG_SECTOR_MEADOW, ".\\SOUND\\BB35.WAV",
		{MG_ACT_CROWNS, +18, 0}, ""
	},

	{
		MG_SECTOR_MEADOW, ".\\SOUND\\BB36.WAV",
		{MG_ACT_TURN, +1, 0}, ""
	},

	{
		MG_SECTOR_MLPARK, ".\\SOUND\\BB03.WAV",
		{MG_ACT_CROWNS, -6, MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_MLPARK, ".\\SOUND\\BB04.WAV",
		{MG_ACT_CROWNS, -20, 0}, ""
	},

	{
		MG_SECTOR_MOUNTAINS, ".\\SOUND\\BB29.WAV",
		{MG_ACT_CROWNS, +20, 0}, ""
	},

	{
		MG_SECTOR_MOUNTAINS, ".\\SOUND\\BB30.WAV",
		{MG_ACT_GO, MG_SECTOR_MEADOW, 0}, ""
	},

	{
		MG_SECTOR_MOUNTAINVIEW, ".\\SOUND\\BB33.WAV",
		{MG_ACT_TURN, +1, 0}, ""
	},

	{
		MG_SECTOR_MOUNTAINVIEW, ".\\SOUND\\BB34.WAV",
		{MG_ACT_GO, MG_SECTOR_POPORREE, 0}, ""
	},

	{
		MG_SECTOR_PASTEESH, ".\\SOUND\\BB09.WAV",
		{MG_ACT_CROWNS, -30, 0}, ""
	},

	{
		MG_SECTOR_PASTEESH, ".\\SOUND\\BB10.WAV",
		{MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_PATSHWERQUE, ".\\SOUND\\BB21.WAV",
		{MG_ACT_GO, MG_SECTOR_FARM, 0}, ""
	},

	{
		MG_SECTOR_PATSHWERQUE, ".\\SOUND\\BB22.WAV",
		{MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_PLAINS, ".\\SOUND\\BB11.WAV",
		{MG_ACT_CROWNS, -15, 0}, ""
	},

	{
		MG_SECTOR_PLAINS, ".\\SOUND\\BB12.WAV",
		{MG_ACT_OBJECT, +1, 0}, ""
	},

	{
		MG_SECTOR_POPORREE, ".\\SOUND\\BB25.WAV",
		{MG_ACT_CROWNS, +16, 0}, ""
	},

	{
		MG_SECTOR_POPORREE, ".\\SOUND\\BB26.WAV",
		{MG_ACT_TURN, +1, 0}, ""
	},

	{
		MG_SECTOR_RIVER, ".\\SOUND\\BB37.WAV",
		{MG_ACT_TURN, +1, 0}, ""
	},

	{
		MG_SECTOR_RIVER, ".\\SOUND\\BB38.WAV",
		{MG_ACT_OBJECT, +1, 0}, ""
	},

	{
		MG_SECTOR_SWAMP, ".\\SOUND\\BB23.WAV",
		{MG_ACT_CROWNS, -8, 0}, ""
	},

	{
		MG_SECTOR_SWAMP, ".\\SOUND\\BB24.WAV",
		{MG_ACT_OBJECT, -1, 0}, ""
	},

	{
		MG_SECTOR_VOLCANO, ".\\SOUND\\BB15.WAV",
		{MG_ACT_CROWNS, -15, 0}, ""
	},

	{
		MG_SECTOR_VOLCANO, ".\\SOUND\\BB16.WAV",
		{MG_ACT_CROWNS, +35, 0}, ""
	},

	{
		MG_SECTOR_WITTY, ".\\SOUND\\BB27.WAV",
		{MG_ACT_TURN, +1, 0}, ""
	},

	{
		MG_SECTOR_WITTY, ".\\SOUND\\BB28.WAV",
		{MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB43.WAV",
		{MG_ACT_CROWNS, +16, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB44.WAV",
		{MG_ACT_TURN, -1, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB45.WAV",
		{MG_ACT_CROWNS, +10, MG_ACT_OBJECT, -1, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB46.WAV",
		{MG_ACT_CROWNS, +22, MG_ACT_OBJECT, +1, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB47.WAV",
		{MG_ACT_CROWNS, -11, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB48.WAV",
		{MG_ACT_GOCOND, MG_SECTOR_LAKE, MG_SECTOR_HILLSIDE, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB49.WAV",
		{MG_ACT_GOCOND, MG_SECTOR_GLACIER, MG_SECTOR_MOUNTAINS, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB50.WAV",
		{MG_ACT_OBJECT, -1, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB51.WAV",
		{MG_ACT_CROWNS, -6, MG_ACT_TURN, +1, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB52.WAV",
		{MG_ACT_CROWNS, +21, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB53.WAV",
		{MG_ACT_CROWNS, -18, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB54.WAV",
		{MG_ACT_CROWNS, -17, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB55.WAV",
		{MG_ACT_TURN, +1, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB56.WAV",
		{MG_ACT_OBJECT, -1, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB57.WAV",
		{MG_ACT_CROWNS, +26, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB58.WAV",
		{MG_ACT_CROWNS, -9, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB59.WAV",
		{MG_ACT_CROWNS, -15, 0}, ""
	},

	{
		MG_SECTOR_ANY, ".\\SOUND\\BB60.WAV",
		{MG_ACT_CROWNS, -12, 0}, ""
	},

	//
	// Start of Color Narrations
	//

	{MG_SECTOR_ANY, ".\\SOUND\\CL161.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL168.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL162.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL169.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL163.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL170.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL164.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL171.WAV", { MG_ACT_PODJ, 0 }, ""},

	{MG_SECTOR_BRIDGE, ".\\SOUND\\CL153.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_BRIDGE, ".\\SOUND\\CL154.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_BRIDGE, ".\\SOUND\\CL155.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_BRIDGE, ".\\SOUND\\CL156.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_BRIDGE, ".\\SOUND\\CL157.WAV", { 0 }, ""},
	{MG_SECTOR_BRIDGE, ".\\SOUND\\CL158.WAV", { 0 }, ""},
	{MG_SECTOR_BRIDGE, ".\\SOUND\\CL159.WAV", { 0 }, ""},
	{MG_SECTOR_BRIDGE, ".\\SOUND\\CL160.WAV", { 0 }, ""},

	{MG_SECTOR_CHUCKLE, ".\\SOUND\\CL073.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_CHUCKLE, ".\\SOUND\\CL074.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_CHUCKLE, ".\\SOUND\\CL075.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_CHUCKLE, ".\\SOUND\\CL076.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_CHUCKLE, ".\\SOUND\\CL077.WAV", { 0 }, ""},
	{MG_SECTOR_CHUCKLE, ".\\SOUND\\CL078.WAV", { 0 }, ""},
	{MG_SECTOR_CHUCKLE, ".\\SOUND\\CL079.WAV", { 0 }, ""},
	{MG_SECTOR_CHUCKLE, ".\\SOUND\\CL080.WAV", { 0 }, ""},

	{MG_SECTOR_DESERT, ".\\SOUND\\CL001.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_DESERT, ".\\SOUND\\CL002.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_DESERT, ".\\SOUND\\CL003.WAV", { 0 }, ""},
	{MG_SECTOR_DESERT, ".\\SOUND\\CL004.WAV", { 0 }, ""},
	{MG_SECTOR_DESERT, ".\\SOUND\\CL005.WAV", { 0 }, ""},
	{MG_SECTOR_DESERT, ".\\SOUND\\CL006.WAV", { 0 }, ""},
	{MG_SECTOR_DESERT, ".\\SOUND\\CL007.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_DESERT, ".\\SOUND\\CL008.WAV", { MG_ACT_PODJ, 0 }, ""},

	{MG_SECTOR_ESTATE, ".\\SOUND\\CL017.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ESTATE, ".\\SOUND\\CL018.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_ESTATE, ".\\SOUND\\CL019.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ESTATE, ".\\SOUND\\CL020.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_ESTATE, ".\\SOUND\\CL021.WAV", { 0 }, ""},
	{MG_SECTOR_ESTATE, ".\\SOUND\\CL022.WAV", { 0 }, ""},
	{MG_SECTOR_ESTATE, ".\\SOUND\\CL023.WAV", { 0 }, ""},
	{MG_SECTOR_ESTATE, ".\\SOUND\\CL024.WAV", { 0 }, ""},

	{MG_SECTOR_FARM, ".\\SOUND\\CL065.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_FARM, ".\\SOUND\\CL066.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_FARM, ".\\SOUND\\CL067.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_FARM, ".\\SOUND\\CL068.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_FARM, ".\\SOUND\\CL069.WAV", { 0 }, ""},
	{MG_SECTOR_FARM, ".\\SOUND\\CL070.WAV", { 0 }, ""},
	{MG_SECTOR_FARM, ".\\SOUND\\CL071.WAV", { 0 }, ""},
	{MG_SECTOR_FARM, ".\\SOUND\\CL072.WAV", { 0 }, ""},

	{MG_SECTOR_GLACIER, ".\\SOUND\\CL025.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_GLACIER, ".\\SOUND\\CL026.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_GLACIER, ".\\SOUND\\CL027.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_GLACIER, ".\\SOUND\\CL028.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_GLACIER, ".\\SOUND\\CL029.WAV", { 0 }, ""},
	{MG_SECTOR_GLACIER, ".\\SOUND\\CL030.WAV", { 0 }, ""},
	{MG_SECTOR_GLACIER, ".\\SOUND\\CL031.WAV", { 0 }, ""},
	{MG_SECTOR_GLACIER, ".\\SOUND\\CL032.WAV", { 0 }, ""},

	{MG_SECTOR_HILLSIDE, ".\\SOUND\\CL049.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_HILLSIDE, ".\\SOUND\\CL050.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_HILLSIDE, ".\\SOUND\\CL051.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_HILLSIDE, ".\\SOUND\\CL052.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_HILLSIDE, ".\\SOUND\\CL053.WAV", { 0 }, ""},
	{MG_SECTOR_HILLSIDE, ".\\SOUND\\CL054.WAV", { 0 }, ""},
	{MG_SECTOR_HILLSIDE, ".\\SOUND\\CL055.WAV", { 0 }, ""},
	{MG_SECTOR_HILLSIDE, ".\\SOUND\\CL056.WAV", { 0 }, ""},

	{MG_SECTOR_LAKE, ".\\SOUND\\CL121.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_LAKE, ".\\SOUND\\CL122.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_LAKE, ".\\SOUND\\CL123.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_LAKE, ".\\SOUND\\CL124.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_LAKE, ".\\SOUND\\CL125.WAV", { 0 }, ""},
	{MG_SECTOR_LAKE, ".\\SOUND\\CL126.WAV", { 0 }, ""},
	{MG_SECTOR_LAKE, ".\\SOUND\\CL127.WAV", { 0 }, ""},
	{MG_SECTOR_LAKE, ".\\SOUND\\CL128.WAV", { 0 }, ""},

	{MG_SECTOR_MEADOW, ".\\SOUND\\CL137.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_MEADOW, ".\\SOUND\\CL138.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_MEADOW, ".\\SOUND\\CL139.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_MEADOW, ".\\SOUND\\CL140.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_MEADOW, ".\\SOUND\\CL141.WAV", { 0 }, ""},
	{MG_SECTOR_MEADOW, ".\\SOUND\\CL142.WAV", { 0 }, ""},
	{MG_SECTOR_MEADOW, ".\\SOUND\\CL143.WAV", { 0 }, ""},
	{MG_SECTOR_MEADOW, ".\\SOUND\\CL144.WAV", { 0 }, ""},

	{MG_SECTOR_MLPARK, ".\\SOUND\\CL009.WAV", { 0 }, ""},
	{MG_SECTOR_MLPARK, ".\\SOUND\\CL010.WAV", { 0 }, ""},
	{MG_SECTOR_MLPARK, ".\\SOUND\\CL011.WAV", { 0 }, ""},
	{MG_SECTOR_MLPARK, ".\\SOUND\\CL012.WAV", { 0 }, ""},
	{MG_SECTOR_MLPARK, ".\\SOUND\\CL013.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_MLPARK, ".\\SOUND\\CL014.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_MLPARK, ".\\SOUND\\CL015.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_MLPARK, ".\\SOUND\\CL016.WAV", { MG_ACT_PODJ, 0 }, ""},

	{MG_SECTOR_MOUNTAINS, ".\\SOUND\\CL113.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_MOUNTAINS, ".\\SOUND\\CL114.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_MOUNTAINS, ".\\SOUND\\CL115.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_MOUNTAINS, ".\\SOUND\\CL116.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_MOUNTAINS, ".\\SOUND\\CL117.WAV", { 0 }, ""},
	{MG_SECTOR_MOUNTAINS, ".\\SOUND\\CL118.WAV", { 0 }, ""},
	{MG_SECTOR_MOUNTAINS, ".\\SOUND\\CL119.WAV", { 0 }, ""},
	{MG_SECTOR_MOUNTAINS, ".\\SOUND\\CL120.WAV", { 0 }, ""},

	{MG_SECTOR_MOUNTAINVIEW, ".\\SOUND\\CL129.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_MOUNTAINVIEW, ".\\SOUND\\CL130.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_MOUNTAINVIEW, ".\\SOUND\\CL131.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_MOUNTAINVIEW, ".\\SOUND\\CL132.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_MOUNTAINVIEW, ".\\SOUND\\CL133.WAV", { 0 }, ""},
	{MG_SECTOR_MOUNTAINVIEW, ".\\SOUND\\CL134.WAV", { 0 }, ""},
	{MG_SECTOR_MOUNTAINVIEW, ".\\SOUND\\CL135.WAV", { 0 }, ""},
	{MG_SECTOR_MOUNTAINVIEW, ".\\SOUND\\CL136.WAV", { 0 }, ""},

	{MG_SECTOR_PASTEESH, ".\\SOUND\\CL033.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_PASTEESH, ".\\SOUND\\CL034.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_PASTEESH, ".\\SOUND\\CL035.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_PASTEESH, ".\\SOUND\\CL036.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_PASTEESH, ".\\SOUND\\CL037.WAV", { 0 }, ""},
	{MG_SECTOR_PASTEESH, ".\\SOUND\\CL038.WAV", { 0 }, ""},
	{MG_SECTOR_PASTEESH, ".\\SOUND\\CL039.WAV", { 0 }, ""},
	{MG_SECTOR_PASTEESH, ".\\SOUND\\CL040.WAV", { 0 }, ""},

	{MG_SECTOR_PATSHWERQUE, ".\\SOUND\\CL081.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_PATSHWERQUE, ".\\SOUND\\CL082.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_PATSHWERQUE, ".\\SOUND\\CL083.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_PATSHWERQUE, ".\\SOUND\\CL084.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_PATSHWERQUE, ".\\SOUND\\CL085.WAV", { 0 }, ""},
	{MG_SECTOR_PATSHWERQUE, ".\\SOUND\\CL086.WAV", { 0 }, ""},
	{MG_SECTOR_PATSHWERQUE, ".\\SOUND\\CL087.WAV", { 0 }, ""},
	{MG_SECTOR_PATSHWERQUE, ".\\SOUND\\CL088.WAV", { 0 }, ""},

	{MG_SECTOR_PLAINS, ".\\SOUND\\CL041.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_PLAINS, ".\\SOUND\\CL042.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_PLAINS, ".\\SOUND\\CL043.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_PLAINS, ".\\SOUND\\CL044.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_PLAINS, ".\\SOUND\\CL045.WAV", { 0 }, ""},
	{MG_SECTOR_PLAINS, ".\\SOUND\\CL046.WAV", { 0 }, ""},
	{MG_SECTOR_PLAINS, ".\\SOUND\\CL047.WAV", { 0 }, ""},
	{MG_SECTOR_PLAINS, ".\\SOUND\\CL048.WAV", { 0 }, ""},

	{MG_SECTOR_POPORREE, ".\\SOUND\\CL097.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_POPORREE, ".\\SOUND\\CL098.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_POPORREE, ".\\SOUND\\CL099.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_POPORREE, ".\\SOUND\\CL100.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_POPORREE, ".\\SOUND\\CL101.WAV", { 0 }, ""},
	{MG_SECTOR_POPORREE, ".\\SOUND\\CL102.WAV", { 0 }, ""},
	{MG_SECTOR_POPORREE, ".\\SOUND\\CL103.WAV", { 0 }, ""},
	{MG_SECTOR_POPORREE, ".\\SOUND\\CL104.WAV", { 0 }, ""},

	{MG_SECTOR_RIVER, ".\\SOUND\\CL145.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_RIVER, ".\\SOUND\\CL146.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_RIVER, ".\\SOUND\\CL147.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_RIVER, ".\\SOUND\\CL148.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_RIVER, ".\\SOUND\\CL149.WAV", { 0 }, ""},
	{MG_SECTOR_RIVER, ".\\SOUND\\CL150.WAV", { 0 }, ""},
	{MG_SECTOR_RIVER, ".\\SOUND\\CL151.WAV", { 0 }, ""},
	{MG_SECTOR_RIVER, ".\\SOUND\\CL152.WAV", { 0 }, ""},

	{MG_SECTOR_SWAMP, ".\\SOUND\\CL089.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_SWAMP, ".\\SOUND\\CL090.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_SWAMP, ".\\SOUND\\CL091.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_SWAMP, ".\\SOUND\\CL092.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_SWAMP, ".\\SOUND\\CL093.WAV", { 0 }, ""},
	{MG_SECTOR_SWAMP, ".\\SOUND\\CL094.WAV", { 0 }, ""},
	{MG_SECTOR_SWAMP, ".\\SOUND\\CL095.WAV", { 0 }, ""},
	{MG_SECTOR_SWAMP, ".\\SOUND\\CL096.WAV", { 0 }, ""},

	{MG_SECTOR_VOLCANO, ".\\SOUND\\CL057.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_VOLCANO, ".\\SOUND\\CL058.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_VOLCANO, ".\\SOUND\\CL059.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_VOLCANO, ".\\SOUND\\CL060.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_VOLCANO, ".\\SOUND\\CL061.WAV", { 0 }, ""},
	{MG_SECTOR_VOLCANO, ".\\SOUND\\CL062.WAV", { 0 }, ""},
	{MG_SECTOR_VOLCANO, ".\\SOUND\\CL063.WAV", { 0 }, ""},
	{MG_SECTOR_VOLCANO, ".\\SOUND\\CL064.WAV", { 0 }, ""},

	{MG_SECTOR_WITTY, ".\\SOUND\\CL105.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_WITTY, ".\\SOUND\\CL106.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_WITTY, ".\\SOUND\\CL107.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_WITTY, ".\\SOUND\\CL108.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_WITTY, ".\\SOUND\\CL109.WAV", { 0 }, ""},
	{MG_SECTOR_WITTY, ".\\SOUND\\CL110.WAV", { 0 }, ""},
	{MG_SECTOR_WITTY, ".\\SOUND\\CL111.WAV", { 0 }, ""},
	{MG_SECTOR_WITTY, ".\\SOUND\\CL112.WAV", { 0 }, ""},

	{MG_SECTOR_ANY, ".\\SOUND\\CL165.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL166.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL167.WAV", { MG_ACT_HODJ, 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL172.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL173.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL174.WAV", { MG_ACT_PODJ, 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL175.WAV", { 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL176.WAV", { 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL177.WAV", { 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL178.WAV", { 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL179.WAV", { 0 }, ""},
	{MG_SECTOR_ANY, ".\\SOUND\\CL180.WAV", { 0 }, ""},
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

								nChangeAmount = brand() % nRandFactor + 1;           //...subtract from 1 to nRandFactor
								lCrowns -= nChangeAmount;
							} else {
								nChangeAmount = brand() % nRandFactor + 1;           // Add from 1 to nRandFactor
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
