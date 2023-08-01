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

#include "gob/save/saveload.h"
#include "gob/save/saveconverter.h"
#include "gob/inter.h"
#include "gob/variables.h"

namespace Gob {

SaveLoad_v7::SaveFile SaveLoad_v7::_saveFiles[] = {
	// Adibou 2 / Addy Junior Base
	{"DATA/visage01.inf", kSaveModeSave, nullptr, "face"         }, // Child 01
	{"DATA/visage02.inf", kSaveModeSave, nullptr, "face"         }, // Child 02
	{"DATA/visage03.inf", kSaveModeSave, nullptr, "face"         }, // Child 03
	{"DATA/visage04.inf", kSaveModeSave, nullptr, "face"         }, // Child 04
	{"DATA/visage05.inf", kSaveModeSave, nullptr, "face"         }, // Child 05
	{"DATA/visage06.inf", kSaveModeSave, nullptr, "face"         }, // Child 06
	{"DATA/visage07.inf", kSaveModeSave, nullptr, "face"         }, // Child 07
	{"DATA/visage08.inf", kSaveModeSave, nullptr, "face"         }, // Child 08
	{"DATA/visage09.inf", kSaveModeSave, nullptr, "face"         }, // Child 09
	{"DATA/visage10.inf", kSaveModeSave, nullptr, "face"         }, // Child 10
	{"DATA/visage11.inf", kSaveModeSave, nullptr, "face"         }, // Child 11
	{"DATA/visage12.inf", kSaveModeSave, nullptr, "face"         }, // Child 12
	{"DATA/visage13.inf", kSaveModeSave, nullptr, "face"         }, // Child 13
	{"DATA/visage14.inf", kSaveModeSave, nullptr, "face"         }, // Child 14
	{"DATA/visage15.inf", kSaveModeSave, nullptr, "face"         }, // Child 15
	{"DATA/visage16.inf", kSaveModeSave, nullptr, "face"         }, // Child 16
	{"DATA/enfant.inf",   kSaveModeSave, nullptr, "children"     },

	{"TEMP/debil.tmp",    kSaveModeSave, nullptr, nullptr        },
	{"DATA/debil.tmp",    kSaveModeSave, nullptr, nullptr        },
	{"APPLIS/debil.tmp",    kSaveModeSave, nullptr, nullptr      },
	{"ENVIR/debil.tmp",    kSaveModeSave, nullptr, nullptr       },


	{  "DATA/config.inf", kSaveModeSave, nullptr, "configuration"},
	// Adibou environment (flowers, vegetables...)
	{"DATA/enviro01.inf", kSaveModeSave, nullptr, "environment" }, // Child 01
	{"DATA/enviro02.inf", kSaveModeSave, nullptr, "environment" }, // Child 02
	{"DATA/enviro03.inf", kSaveModeSave, nullptr, "environment" }, // Child 03
	{"DATA/enviro04.inf", kSaveModeSave, nullptr, "environment" }, // Child 04
	{"DATA/enviro05.inf", kSaveModeSave, nullptr, "environment" }, // Child 05
	{"DATA/enviro06.inf", kSaveModeSave, nullptr, "environment" }, // Child 06
	{"DATA/enviro07.inf", kSaveModeSave, nullptr, "environment" }, // Child 07
	{"DATA/enviro08.inf", kSaveModeSave, nullptr, "environment" }, // Child 08
	{"DATA/enviro09.inf", kSaveModeSave, nullptr, "environment" }, // Child 09
	{"DATA/enviro10.inf", kSaveModeSave, nullptr, "environment" }, // Child 10
	{"DATA/enviro11.inf", kSaveModeSave, nullptr, "environment" }, // Child 11
	{"DATA/enviro12.inf", kSaveModeSave, nullptr, "environment" }, // Child 12
	{"DATA/enviro13.inf", kSaveModeSave, nullptr, "environment" }, // Child 13
	{"DATA/enviro14.inf", kSaveModeSave, nullptr, "environment" }, // Child 14
	{"DATA/enviro15.inf", kSaveModeSave, nullptr, "environment" }, // Child 15
	{"DATA/enviro16.inf", kSaveModeSave, nullptr, "environment" }, // Child 16

	// Adibou environment weather
	{"DATA/temp01.csa"  , kSaveModeSave, nullptr, "weather" },
	{"DATA/temp02.csa"  , kSaveModeSave, nullptr, "weather" },
	{"DATA/temp03.csa"  , kSaveModeSave, nullptr, "weather" },
	{"DATA/temp04.csa"  , kSaveModeSave, nullptr, "weather" },
	{"DATA/temp05.csa"  , kSaveModeSave, nullptr, "weather" },
	{"DATA/temp06.csa"  , kSaveModeSave, nullptr, "weather" },
	{"DATA/temp07.csa"  , kSaveModeSave, nullptr, "weather" },
	{"DATA/temp08.csa"  , kSaveModeSave, nullptr, "weather" },
	{"DATA/temp09.csa"  , kSaveModeSave, nullptr, "weather" },
	{"DATA/temp10.csa"  , kSaveModeSave, nullptr, "weather" },
	{"DATA/temp11.csa"  , kSaveModeSave, nullptr, "weather" },
	{"DATA/temp12.csa"  , kSaveModeSave, nullptr, "weather" },
	{"DATA/temp13.csa"  , kSaveModeSave, nullptr, "weather" },
	{"DATA/temp14.csa"  , kSaveModeSave, nullptr, "weather" },
	{"DATA/temp15.csa"  , kSaveModeSave, nullptr, "weather" },
	{"DATA/temp16.csa"  , kSaveModeSave, nullptr, "weather" },

	// Adibou breakout game progress
	{"DATA/brique01.inf"  , kSaveModeSave, nullptr, "breakout game progress" },
	{"DATA/brique02.inf"  , kSaveModeSave, nullptr, "breakout game progress" },
	{"DATA/brique03.inf"  , kSaveModeSave, nullptr, "breakout game progress" },
	{"DATA/brique04.inf"  , kSaveModeSave, nullptr, "breakout game progress" },
	{"DATA/brique05.inf"  , kSaveModeSave, nullptr, "breakout game progress" },
	{"DATA/brique06.inf"  , kSaveModeSave, nullptr, "breakout game progress" },
	{"DATA/brique07.inf"  , kSaveModeSave, nullptr, "breakout game progress" },
	{"DATA/brique08.inf"  , kSaveModeSave, nullptr, "breakout game progress" },
	{"DATA/brique09.inf"  , kSaveModeSave, nullptr, "breakout game progress" },
	{"DATA/brique10.inf"  , kSaveModeSave, nullptr, "breakout game progress" },
	{"DATA/brique11.inf"  , kSaveModeSave, nullptr, "breakout game progress" },
	{"DATA/brique12.inf"  , kSaveModeSave, nullptr, "breakout game progress" },
	{"DATA/brique13.inf"  , kSaveModeSave, nullptr, "breakout game progress" },
	{"DATA/brique14.inf"  , kSaveModeSave, nullptr, "breakout game progress" },
	{"DATA/brique15.inf"  , kSaveModeSave, nullptr, "breakout game progress" },
	{"DATA/brique16.inf"  , kSaveModeSave, nullptr, "breakout game progress" },

	// Adibou Playtoon-like minigame
	{"construc.inf"  , kSaveModeSave, nullptr, "construction game progress" },
	{"ptrecord.inf"  , kSaveModeSave, nullptr, "construction game progress" }, // PTRECORD = "Playtoons record" probably
	{"aide.inf"      , kSaveModeSave, nullptr, "construction game progress" },

	{"DATA/constr01.inf"  , kSaveModeSave, nullptr, "construction game progress" }, // Child 01
	{"DATA/ptreco01.inf"  , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/aide01.inf"    , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/constr02.inf"  , kSaveModeSave, nullptr, "construction game progress" }, // Child 02
	{"DATA/ptreco02.inf"  , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/aide02.inf"    , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/constr03.inf"  , kSaveModeSave, nullptr, "construction game progress" }, // Child 03
	{"DATA/ptreco03.inf"  , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/aide03.inf"    , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/constr04.inf"  , kSaveModeSave, nullptr, "construction game progress" }, // Child 04
	{"DATA/ptreco04.inf"  , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/aide04.inf"    , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/constr05.inf"  , kSaveModeSave, nullptr, "construction game progress" }, // Child 05
	{"DATA/ptreco05.inf"  , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/aide05.inf"    , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/constr06.inf"  , kSaveModeSave, nullptr, "construction game progress" }, // Child 06
	{"DATA/ptreco06.inf"  , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/aide06.inf"    , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/constr07.inf"  , kSaveModeSave, nullptr, "construction game progress" }, // Child 07
	{"DATA/ptreco07.inf"  , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/aide07.inf"    , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/constr08.inf"  , kSaveModeSave, nullptr, "construction game progress" }, // Child 08
	{"DATA/ptreco08.inf"  , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/aide08.inf"    , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/constr09.inf"  , kSaveModeSave, nullptr, "construction game progress" }, // Child 09
	{"DATA/ptreco09.inf"  , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/aide09.inf"    , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/constr10.inf"  , kSaveModeSave, nullptr, "construction game progress" }, // Child 10
	{"DATA/ptreco10.inf"  , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/aide10.inf"    , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/constr11.inf"  , kSaveModeSave, nullptr, "construction game progress" }, // Child 11
	{"DATA/ptreco11.inf"  , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/aide11.inf"    , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/constr12.inf"  , kSaveModeSave, nullptr, "construction game progress" }, // Child 12
	{"DATA/ptreco12.inf"  , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/aide12.inf"    , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/constr13.inf"  , kSaveModeSave, nullptr, "construction game progress" }, // Child 13
	{"DATA/ptreco13.inf"  , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/aide13.inf"    , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/constr14.inf"  , kSaveModeSave, nullptr, "construction game progress" }, // Child 14
	{"DATA/ptreco14.inf"  , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/aide14.inf"    , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/constr15.inf"  , kSaveModeSave, nullptr, "construction game progress" }, // Child 15
	{"DATA/ptreco15.inf"  , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/aide15.inf"    , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/constr16.inf"  , kSaveModeSave, nullptr, "construction game progress" }, // Child 16
	{"DATA/ptreco16.inf"  , kSaveModeSave, nullptr, "construction game progress" },
	{"DATA/aide16.inf"    , kSaveModeSave, nullptr, "construction game progress" },

	// Adibou Applications 1-5
	{"DATA/Gsa01_01.inf", kSaveModeSave, nullptr, "app progress" }, // Child 01
	{"DATA/Gsa02_01.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa03_01.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa04_01.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa05_01.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa01_02.inf", kSaveModeSave, nullptr, "app progress" }, // Child 02
	{"DATA/Gsa02_02.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa03_02.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa04_02.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa05_02.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa01_03.inf", kSaveModeSave, nullptr, "app progress" }, // Child 03
	{"DATA/Gsa02_03.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa03_03.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa04_03.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa05_03.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa01_04.inf", kSaveModeSave, nullptr, "app progress" }, // Child 04
	{"DATA/Gsa02_04.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa03_04.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa04_04.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa05_04.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa01_05.inf", kSaveModeSave, nullptr, "app progress" }, // Child 05
	{"DATA/Gsa02_05.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa03_05.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa04_05.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa05_05.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa01_06.inf", kSaveModeSave, nullptr, "app progress" }, // Child 06
	{"DATA/Gsa02_06.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa03_06.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa04_06.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa05_06.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa01_07.inf", kSaveModeSave, nullptr, "app progress" }, // Child 07
	{"DATA/Gsa02_07.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa03_07.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa04_07.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa05_07.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa01_08.inf", kSaveModeSave, nullptr, "app progress" }, // Child 08
	{"DATA/Gsa02_08.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa03_08.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa04_08.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa05_08.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa01_09.inf", kSaveModeSave, nullptr, "app progress" }, // Child 09
	{"DATA/Gsa02_09.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa03_09.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa04_09.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa05_09.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa01_10.inf", kSaveModeSave, nullptr, "app progress" }, // Child 10
	{"DATA/Gsa02_10.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa03_10.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa04_10.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa05_10.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa01_11.inf", kSaveModeSave, nullptr, "app progress" }, // Child 11
	{"DATA/Gsa02_11.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa03_11.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa04_11.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa05_11.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa01_12.inf", kSaveModeSave, nullptr, "app progress" }, // Child 12
	{"DATA/Gsa02_12.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa03_12.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa04_12.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa05_12.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa01_13.inf", kSaveModeSave, nullptr, "app progress" }, // Child 13
	{"DATA/Gsa02_13.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa03_13.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa04_13.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa05_13.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa01_14.inf", kSaveModeSave, nullptr, "app progress" }, // Child 14
	{"DATA/Gsa02_14.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa03_14.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa04_14.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa05_14.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa01_15.inf", kSaveModeSave, nullptr, "app progress" }, // Child 15
	{"DATA/Gsa02_15.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa03_15.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa04_15.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa05_15.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa01_16.inf", kSaveModeSave, nullptr, "app progress" }, // Child 16
	{"DATA/Gsa02_16.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa03_16.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa04_16.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/Gsa05_16.inf", kSaveModeSave, nullptr, "app progress" },
	{"DATA/memo_01.inf", kSaveModeSave, nullptr, "memo" }, // Child 01
	{"DATA/memo_02.inf", kSaveModeSave, nullptr, "memo" }, // Child 02
	{"DATA/memo_03.inf", kSaveModeSave, nullptr, "memo" }, // Child 03
	{"DATA/memo_04.inf", kSaveModeSave, nullptr, "memo" }, // Child 04
	{"DATA/memo_05.inf", kSaveModeSave, nullptr, "memo" }, // Child 05
	{"DATA/memo_06.inf", kSaveModeSave, nullptr, "memo" }, // Child 06
	{"DATA/memo_07.inf", kSaveModeSave, nullptr, "memo" }, // Child 07
	{"DATA/memo_08.inf", kSaveModeSave, nullptr, "memo" }, // Child 08
	{"DATA/memo_09.inf", kSaveModeSave, nullptr, "memo" }, // Child 09
	{"DATA/memo_10.inf", kSaveModeSave, nullptr, "memo" }, // Child 10
	{"DATA/memo_11.inf", kSaveModeSave, nullptr, "memo" }, // Child 11
	{"DATA/memo_12.inf", kSaveModeSave, nullptr, "memo" }, // Child 12
	{"DATA/memo_13.inf", kSaveModeSave, nullptr, "memo" }, // Child 13
	{"DATA/memo_14.inf", kSaveModeSave, nullptr, "memo" }, // Child 14
	{"DATA/memo_15.inf", kSaveModeSave, nullptr, "memo" }, // Child 15
	{"DATA/memo_16.inf", kSaveModeSave, nullptr, "memo" }, // Child 16
	{"DATA/diplo_01.inf", kSaveModeSave, nullptr, "diplo" }, // Child 01
	{"DATA/diplo_02.inf", kSaveModeSave, nullptr, "diplo" }, // Child 02
	{"DATA/diplo_03.inf", kSaveModeSave, nullptr, "diplo" }, // Child 03
	{"DATA/diplo_04.inf", kSaveModeSave, nullptr, "diplo" }, // Child 04
	{"DATA/diplo_05.inf", kSaveModeSave, nullptr, "diplo" }, // Child 05
	{"DATA/diplo_06.inf", kSaveModeSave, nullptr, "diplo" }, // Child 06
	{"DATA/diplo_07.inf", kSaveModeSave, nullptr, "diplo" }, // Child 07
	{"DATA/diplo_08.inf", kSaveModeSave, nullptr, "diplo" }, // Child 08
	{"DATA/diplo_09.inf", kSaveModeSave, nullptr, "diplo" }, // Child 09
	{"DATA/diplo_10.inf", kSaveModeSave, nullptr, "diplo" }, // Child 10
	{"DATA/diplo_11.inf", kSaveModeSave, nullptr, "diplo" }, // Child 11
	{"DATA/diplo_12.inf", kSaveModeSave, nullptr, "diplo" }, // Child 12
	{"DATA/diplo_13.inf", kSaveModeSave, nullptr, "diplo" }, // Child 13
	{"DATA/diplo_14.inf", kSaveModeSave, nullptr, "diplo" }, // Child 14
	{"DATA/diplo_15.inf", kSaveModeSave, nullptr, "diplo" }, // Child 15
	{"DATA/diplo_16.inf", kSaveModeSave, nullptr, "diplo" }, // Child 16
	{"DATA/appli_01.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/appli_02.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/appli_03.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/appli_04.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/appli_05.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/crite_01.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/crite_02.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/crite_03.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/crite_04.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/crite_05.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/exo_01.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/exo_02.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/exo_03.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/exo_04.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/exo_05.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/ico_01.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/ico_02.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/ico_03.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/ico_04.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/ico_05.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/applis.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/lance.inf", kSaveModeSave, nullptr, "app info" },
	{"DATA/retour.inf", kSaveModeSave, nullptr, "app info" },
	// Adibou2 saved drawings (up to 12)
	// Child1
	{"DATA/cart0101.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0201.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0301.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0401.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0501.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0601.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0701.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0801.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0901.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1001.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1101.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1201.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/pal00_01.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal01_01.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal02_01.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal03_01.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal04_01.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal05_01.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal06_01.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal07_01.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal08_01.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal09_01.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal10_01.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal11_01.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/palett01.inf", kSaveModeSave, nullptr, "drawing palette" },
	{"DATA/plettr01.inf", kSaveModeSave, nullptr, "drawing in mailbox" },
	{"DATA/pgatod01.inf", kSaveModeSave, nullptr, "photo of cake" },
	{"DATA/pfleur01.inf", kSaveModeSave, nullptr, "photo of flowers" },
	{"DATA/mfleur01.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mfleud01.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mtable01.inf", kSaveModeSave, nullptr, "poster" },
	// Child2
	{"DATA/cart0102.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0202.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0302.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0402.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0502.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0602.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0702.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0802.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0902.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1002.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1102.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1202.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/pal00_02.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal01_02.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal02_02.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal03_02.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal04_02.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal05_02.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal06_02.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal07_02.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal08_02.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal09_02.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal10_02.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal11_02.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/palett02.inf", kSaveModeSave, nullptr, "drawing palette" },
	{"DATA/plettr02.inf", kSaveModeSave, nullptr, "drawing in mailbox" },
	{"DATA/pgatod02.inf", kSaveModeSave, nullptr, "photo of cake" },
	{"DATA/pfleur02.inf", kSaveModeSave, nullptr, "photo of flowers" },
	{"DATA/mfleur02.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mfleud02.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mtable02.inf", kSaveModeSave, nullptr, "poster" },
	// Child3
	{"DATA/cart0103.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0203.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0303.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0403.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0503.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0603.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0703.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0803.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0903.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1003.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1103.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1203.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/pal00_03.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal01_03.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal02_03.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal03_03.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal04_03.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal05_03.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal06_03.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal07_03.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal08_03.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal09_03.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal10_03.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal11_03.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/palett03.inf", kSaveModeSave, nullptr, "drawing palette" },
	{"DATA/plettr03.inf", kSaveModeSave, nullptr, "drawing in mailbox" },
	{"DATA/pgatod03.inf", kSaveModeSave, nullptr, "photo of cake" },
	{"DATA/pfleur03.inf", kSaveModeSave, nullptr, "photo of flowers" },
	{"DATA/mfleur03.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mfleud03.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mtable03.inf", kSaveModeSave, nullptr, "poster" },
	// Child4
	{"DATA/cart0104.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0204.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0304.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0404.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0504.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0604.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0704.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0804.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0904.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1004.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1104.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1204.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/pal00_04.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal01_04.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal02_04.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal03_04.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal04_04.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal05_04.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal06_04.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal07_04.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal08_04.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal09_04.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal10_04.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal11_04.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/palett04.inf", kSaveModeSave, nullptr, "drawing palette" },
	{"DATA/plettr04.inf", kSaveModeSave, nullptr, "drawing in mailbox" },
	{"DATA/pgatod04.inf", kSaveModeSave, nullptr, "photo of cake" },
	{"DATA/pfleur04.inf", kSaveModeSave, nullptr, "photo of flowers" },
	{"DATA/mfleur04.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mfleud04.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mtable04.inf", kSaveModeSave, nullptr, "poster" },
	// Child5
	{"DATA/cart0105.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0205.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0305.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0405.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0505.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0605.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0705.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0805.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0905.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1005.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1105.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1205.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/pal00_05.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal01_05.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal02_05.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal03_05.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal04_05.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal05_05.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal06_05.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal07_05.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal08_05.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal09_05.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal10_05.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal11_05.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/palett05.inf", kSaveModeSave, nullptr, "drawing palette" },
	{"DATA/plettr05.inf", kSaveModeSave, nullptr, "drawing in mailbox" },
	{"DATA/pgatod05.inf", kSaveModeSave, nullptr, "photo of cake" },
	{"DATA/pfleur05.inf", kSaveModeSave, nullptr, "photo of flowers" },
	{"DATA/mfleur05.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mfleud05.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mtable05.inf", kSaveModeSave, nullptr, "poster" },
	// Child6
	{"DATA/cart0106.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0206.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0306.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0406.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0506.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0606.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0706.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0806.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0906.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1006.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1106.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1206.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/pal00_06.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal01_06.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal02_06.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal03_06.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal04_06.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal05_06.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal06_06.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal07_06.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal08_06.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal09_06.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal10_06.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal11_06.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/palett06.inf", kSaveModeSave, nullptr, "drawing palette" },
	{"DATA/plettr06.inf", kSaveModeSave, nullptr, "drawing in mailbox" },
	{"DATA/pgatod06.inf", kSaveModeSave, nullptr, "photo of cake" },
	{"DATA/pfleur06.inf", kSaveModeSave, nullptr, "photo of flowers" },
	{"DATA/mfleur06.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mfleud06.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mtable06.inf", kSaveModeSave, nullptr, "poster" },
	// Child7
	{"DATA/cart0107.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0207.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0307.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0407.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0507.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0607.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0707.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0807.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0907.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1007.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1107.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1207.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/pal00_07.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal01_07.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal02_07.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal03_07.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal04_07.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal05_07.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal06_07.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal07_07.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal08_07.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal09_07.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal10_07.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal11_07.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/palett07.inf", kSaveModeSave, nullptr, "drawing palette" },
	{"DATA/plettr07.inf", kSaveModeSave, nullptr, "drawing in mailbox" },
	{"DATA/pgatod07.inf", kSaveModeSave, nullptr, "photo of cake" },
	{"DATA/pfleur07.inf", kSaveModeSave, nullptr, "photo of flowers" },
	{"DATA/mfleur07.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mfleud07.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mtable07.inf", kSaveModeSave, nullptr, "poster" },
	// Child8
	{"DATA/cart0108.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0208.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0308.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0408.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0508.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0608.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0708.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0808.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0908.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1008.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1108.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1208.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/pal00_08.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal01_08.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal02_08.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal03_08.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal04_08.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal05_08.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal06_08.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal07_08.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal08_08.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal09_08.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal10_08.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal11_08.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/palett08.inf", kSaveModeSave, nullptr, "drawing palette" },
	{"DATA/plettr08.inf", kSaveModeSave, nullptr, "drawing in mailbox" },
	{"DATA/pgatod08.inf", kSaveModeSave, nullptr, "photo of cake" },
	{"DATA/pfleur08.inf", kSaveModeSave, nullptr, "photo of flowers" },
	{"DATA/mfleur08.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mfleud08.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mtable08.inf", kSaveModeSave, nullptr, "poster" },
	// Child9
	{"DATA/cart0109.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0209.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0309.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0409.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0509.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0609.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0709.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0809.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0909.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1009.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1109.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1209.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/pal00_09.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal01_09.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal02_09.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal03_09.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal04_09.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal05_09.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal06_09.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal07_09.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal08_09.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal09_09.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal10_09.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal11_09.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/palett09.inf", kSaveModeSave, nullptr, "drawing palette" },
	{"DATA/plettr09.inf", kSaveModeSave, nullptr, "drawing in mailbox" },
	{"DATA/pgatod09.inf", kSaveModeSave, nullptr, "photo of cake" },
	{"DATA/pfleur09.inf", kSaveModeSave, nullptr, "photo of flowers" },
	{"DATA/mfleur09.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mfleud09.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mtable09.inf", kSaveModeSave, nullptr, "poster" },
	// Child10
	{"DATA/cart0110.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0210.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0310.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0410.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0510.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0610.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0710.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0810.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0910.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1010.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1110.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1210.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/pal00_10.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal01_10.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal02_10.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal03_10.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal04_10.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal05_10.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal06_10.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal07_10.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal08_10.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal09_10.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal10_10.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal11_10.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/palett10.inf", kSaveModeSave, nullptr, "drawing palette" },
	{"DATA/plettr10.inf", kSaveModeSave, nullptr, "drawing in mailbox" },
	{"DATA/pgatod10.inf", kSaveModeSave, nullptr, "photo of cake" },
	{"DATA/pfleur10.inf", kSaveModeSave, nullptr, "photo of flowers" },
	{"DATA/mfleur10.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mfleud10.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mtable10.inf", kSaveModeSave, nullptr, "poster" },
	// Child11
	{"DATA/cart0111.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0211.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0311.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0411.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0511.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0611.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0711.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0811.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0911.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1011.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1111.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1211.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/pal00_11.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal01_11.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal02_11.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal03_11.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal04_11.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal05_11.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal06_11.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal07_11.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal08_11.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal09_11.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal10_11.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal11_11.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/palett11.inf", kSaveModeSave, nullptr, "drawing palette" },
	{"DATA/plettr11.inf", kSaveModeSave, nullptr, "drawing in mailbox" },
	{"DATA/pgatod11.inf", kSaveModeSave, nullptr, "photo of cake" },
	{"DATA/pfleur11.inf", kSaveModeSave, nullptr, "photo of flowers" },
	{"DATA/mfleur11.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mfleud11.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mtable11.inf", kSaveModeSave, nullptr, "poster" },
	// Child12
	{"DATA/cart0112.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0212.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0312.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0412.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0512.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0612.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0712.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0812.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0912.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1012.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1112.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1212.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/pal00_12.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal01_12.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal02_12.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal03_12.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal04_12.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal05_12.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal06_12.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal07_12.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal08_12.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal09_12.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal10_12.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal11_12.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/palett12.inf", kSaveModeSave, nullptr, "drawing palette" },
	{"DATA/plettr12.inf", kSaveModeSave, nullptr, "drawing in mailbox" },
	{"DATA/pgatod12.inf", kSaveModeSave, nullptr, "photo of cake" },
	{"DATA/pfleur12.inf", kSaveModeSave, nullptr, "photo of flowers" },
	{"DATA/mfleur12.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mfleud12.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mtable12.inf", kSaveModeSave, nullptr, "poster" },
	// Child13
	{"DATA/cart0113.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0213.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0313.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0413.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0513.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0613.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0713.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0813.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0913.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1013.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1113.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1213.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/pal00_13.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal01_13.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal02_13.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal03_13.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal04_13.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal05_13.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal06_13.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal07_13.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal08_13.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal09_13.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal10_13.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal11_13.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/palett13.inf", kSaveModeSave, nullptr, "drawing palette" },
	{"DATA/plettr13.inf", kSaveModeSave, nullptr, "drawing in mailbox" },
	{"DATA/pgatod13.inf", kSaveModeSave, nullptr, "photo of cake" },
	{"DATA/pfleur13.inf", kSaveModeSave, nullptr, "photo of flowers" },
	{"DATA/mfleur13.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mfleud13.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mtable13.inf", kSaveModeSave, nullptr, "poster" },
	// Child14
	{"DATA/cart0114.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0214.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0314.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0414.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0514.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0614.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0714.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0814.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0914.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1014.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1114.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1214.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/pal00_14.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal01_14.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal02_14.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal03_14.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal04_14.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal05_14.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal06_14.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal07_14.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal08_14.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal09_14.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal10_14.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal11_14.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/palett14.inf", kSaveModeSave, nullptr, "drawing palette" },
	{"DATA/plettr14.inf", kSaveModeSave, nullptr, "drawing in mailbox" },
	{"DATA/pgatod14.inf", kSaveModeSave, nullptr, "photo of cake" },
	{"DATA/pfleur14.inf", kSaveModeSave, nullptr, "photo of flowers" },
	{"DATA/mfleur14.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mfleud14.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mtable14.inf", kSaveModeSave, nullptr, "poster" },
	// Child15
	{"DATA/cart0115.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0215.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0315.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0415.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0515.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0615.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0715.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0815.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0915.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1015.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1115.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1215.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/pal00_15.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal01_15.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal02_15.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal03_15.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal04_15.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal05_15.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal06_15.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal07_15.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal08_15.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal09_15.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal10_15.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal11_15.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/palett15.inf", kSaveModeSave, nullptr, "drawing palette" },
	{"DATA/plettr15.inf", kSaveModeSave, nullptr, "drawing in mailbox" },
	{"DATA/pgatod15.inf", kSaveModeSave, nullptr, "photo of cake" },
	{"DATA/pfleur15.inf", kSaveModeSave, nullptr, "photo of flowers" },
	{"DATA/mfleur15.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mfleud15.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mtable15.inf", kSaveModeSave, nullptr, "poster" },
	// Child16
	{"DATA/cart0116.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0216.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0316.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0416.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0516.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0616.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0716.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0816.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart0916.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1016.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1116.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/cart1216.inf", kSaveModeSave, nullptr, "drawing" },
	{"DATA/pal00_16.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal01_16.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal02_16.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal03_16.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal04_16.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal05_16.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal06_16.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal07_16.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal08_16.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal09_16.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal10_16.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/pal11_16.inf", kSaveModeSave, nullptr, "drawing thumbnail" },
	{"DATA/palett16.inf", kSaveModeSave, nullptr, "drawing palette" },
	{"DATA/plettr16.inf", kSaveModeSave, nullptr, "drawing in mailbox" },
	{"DATA/pgatod16.inf", kSaveModeSave, nullptr, "photo of cake" },
	{"DATA/pfleur16.inf", kSaveModeSave, nullptr, "photo of flowers" },
	{"DATA/mfleur16.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mfleud16.inf", kSaveModeSave, nullptr, "photo of flowers (vase)" },
	{"DATA/mtable16.inf", kSaveModeSave, nullptr, "poster" },

	{"adibou.pal",   kSaveModeSave, nullptr, "drawing on floppy disk" },
	{"adibour.pal",  kSaveModeSave, nullptr, "drawing on floppy disk (thumbnail)" },
	{"test.dob",     kSaveModeSave, nullptr, "test floppy disk file" },

	{"TEMP/liste.$$$", kSaveModeSave, nullptr, "exercise list" },

    // Adi 4 / Addy 4 Base
	{"config00.inf", kSaveModeSave, nullptr, nullptr        },
	{"statev00.inf", kSaveModeSave, nullptr, nullptr        },
	// Adi 4 / Addy 4 Grundschule
	{ "premier.dep", kSaveModeSave, nullptr, nullptr        },
	{ "quitter.dep", kSaveModeSave, nullptr, nullptr        },
	{   "appel.dep", kSaveModeSave, nullptr, nullptr        },
	{  "parole.dep", kSaveModeSave, nullptr, nullptr        },
	{    "ado4.inf", kSaveModeSave, nullptr, nullptr        },
	{"mcurrent.inf", kSaveModeSave, nullptr, nullptr        },
	{   "perso.dep", kSaveModeSave, nullptr, nullptr        },
	{ "nouveau.dep", kSaveModeSave, nullptr, nullptr        },
	{     "adi.tmp", kSaveModeSave, nullptr, nullptr        },
	{     "adi.inf", kSaveModeSave, nullptr, nullptr        },
	{    "adi4.tmp", kSaveModeSave, nullptr, nullptr        }
};

SaveLoad_v7::SpriteHandler::File::File(GobEngine *vm, const Common::String &base, const Common::String &ext) :
SlotFileStatic(vm, base, ext) {
}

SaveLoad_v7::SpriteHandler::File::~File() {
}


SaveLoad_v7::SpriteHandler::SpriteHandler(GobEngine *vm, const Common::String &target, const Common::String &ext)
	: TempSpriteHandler(vm), _file(vm, target, ext) {
}

SaveLoad_v7::SpriteHandler::~SpriteHandler() {
}

int32 SaveLoad_v7::SpriteHandler::getSize() {
	Common::String fileName = _file.build();

	if (fileName.empty())
		return -1;;

	SaveReader reader(1, 0, fileName);
	SaveHeader header;

	if (!reader.load())
		return -1;

	if (!reader.readPartHeader(0, &header))
		return -1;

	// Return the part's size
	return header.getSize();
}

bool SaveLoad_v7::SpriteHandler::load(int16 dataVar, int32 size, int32 offset) {
	if (!TempSpriteHandler::createFromSprite(dataVar, size, offset))
		return false;

	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	SaveReader reader(1, 0, fileName);
	if (!reader.load())
		return false;

	if (!reader.readPart(0, _sprite))
		return false;

	return TempSpriteHandler::load(dataVar, size, offset);
}

bool SaveLoad_v7::SpriteHandler::save(int16 dataVar, int32 size, int32 offset) {
	if (!TempSpriteHandler::save(dataVar, size, offset))
		return false;

	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	SaveWriter writer(1, 0, fileName);
	return writer.writePart(0, _sprite);
}

bool SaveLoad_v7::SpriteHandler::loadToRaw(byte *ptr, int32 size, int32 offset) {
	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	SaveReader reader(1, 0, fileName);
	if (!reader.load())
		return false;

	if (!reader.readPart(0, _sprite))
		return false;

	return TempSpriteHandler::loadToRaw(ptr, size, offset);
}

bool SaveLoad_v7::SpriteHandler::saveFromRaw(const byte *ptr, int32 size, int32 offset) {
	if (!TempSpriteHandler::saveFromRaw(ptr, size, offset))
		return false;

	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	SaveWriter writer(1, 0, fileName);
	return writer.writePart(0, _sprite);
}

bool SaveLoad_v7::SpriteHandler::deleteFile() {
	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	SaveWriter writer(1, 0, fileName);
	return writer.deleteFile();
}

SaveLoad_v7::DrawingOnFloppyDiskHandler::File::File(GobEngine *vm, const Common::String &base, const Common::String &ext) :
	SlotFileStatic(vm, base, ext) {
}

SaveLoad_v7::DrawingOnFloppyDiskHandler::File::~File() {
}


SaveLoad_v7::DrawingOnFloppyDiskHandler::DrawingOnFloppyDiskHandler(GobEngine *vm,
																	SaveReader *reader,
																	SaveWriter *writer,
																	bool isThumbnail,
																	uint32 chunkSize)
	: TempSpriteHandler(vm), _reader(reader), _writer(writer), _isThumbnail(isThumbnail), _chunkSize(chunkSize) {
}

SaveLoad_v7::DrawingOnFloppyDiskHandler::~DrawingOnFloppyDiskHandler() {
	// Assume reader and writer are owned by the handler of the full picture
	if (!_isThumbnail) {
		delete _reader;
		delete _writer;
	}
}

int32 SaveLoad_v7::DrawingOnFloppyDiskHandler::getSize() {
	if (_reader == nullptr || !_reader->load())
		return -1;

	if (_isThumbnail) {
		// First part is the thumbnail
		SaveHeader header;
		if (!_reader->readPartHeader(0, &header))
			return -1;

		return header.getSize();
	} else {
		// Following parts are the full picture chunks
		int32 size = -1;
		SaveHeader header;
		for (int i = 1; _reader->readPartHeader(i, &header); i++) {
			if (size == -1)
				size = 0;
			size += header.getSize();
		}

		return size;
	}
}

bool SaveLoad_v7::DrawingOnFloppyDiskHandler::load(int16 dataVar, int32 size, int32 offset) {
	if (!TempSpriteHandler::createFromSprite(dataVar, size, offset))
		return false;

	if (!_reader->load())
		return false;

	int32 part = 0;
	if (_isThumbnail)
		part = 0;
	else
		part = 1 + offset / _chunkSize;
	if (!_reader->readPart(part, _sprite))
		return false;

	return TempSpriteHandler::load(dataVar, size, offset);
}

bool SaveLoad_v7::DrawingOnFloppyDiskHandler::save(int16 dataVar, int32 size, int32 offset) {
	if (!TempSpriteHandler::save(dataVar, size, offset))
		return false;

	int32 part = 0;
	if (_isThumbnail)
		part = 0;
	else
		part = 1 + offset / _chunkSize;

	return _writer->writePart(part, _sprite);
}

bool SaveLoad_v7::DrawingOnFloppyDiskHandler::deleteFile() {
	return _writer->deleteFile();
}

SaveLoad_v7::GameFileHandler::File::File(GobEngine *vm, const Common::String &base, const Common::String &ext) :
SlotFileStatic(vm, base, ext) {
}

SaveLoad_v7::GameFileHandler::File::~File() {
}


SaveLoad_v7::GameFileHandler::GameFileHandler(GobEngine *vm, const Common::String &target, const Common::String &ext) :
SaveHandler(vm), _file(vm, target, ext) {
}

SaveLoad_v7::GameFileHandler::~GameFileHandler() {
}

int32 SaveLoad_v7::GameFileHandler::getSize() {
	Common::String fileName = _file.build();
	if (fileName.empty())
		return -1;

	SaveReader reader(1, 0, fileName);
	SaveHeader header;

	if (!reader.load())
		return -1;

	if (!reader.readPartHeader(0, &header))
		return -1;

	// Return the part's size
	return header.getSize();
}

bool SaveLoad_v7::GameFileHandler::load(int16 dataVar, int32 size, int32 offset) {
	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	if (size == 0) {
		uint32 varSize = SaveHandler::getVarSize(_vm);
		// Indicator to load all variables
		dataVar = 0;
		size = (int32) varSize;
	}

	int32 fileSize = getSize();
	if (fileSize < 0)
		return false;

	SaveReader reader(1, 0, fileName);
	SavePartVars vars(_vm, fileSize);

	if (!reader.load()) {
		return false;
	}

	if (!reader.readPart(0, &vars)) {
		return false;
	}

	if (!vars.writeInto((uint16) dataVar, offset, size)) {
		return false;
	}

	return true;
}

bool SaveLoad_v7::GameFileHandler::save(const byte *ptrRaw, int16 dataVar, int32 size, int32 offset) {
	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	if (size == 0) {
		// Indicator to save all variables
		dataVar = 0;
		uint32 varSize = SaveHandler::getVarSize(_vm);
		size = (int32) varSize;
	}

	int32 fileSize = getSize();
	int32 newFileSize = size;
	if (fileSize > 0) {
		newFileSize = MAX<int32>(fileSize, size + offset);
	}

	SavePartVars vars(_vm, newFileSize);
	if (fileSize > 0
		&&
		(offset > 0 || size < fileSize)) {
		// Load data from file, as some of it will not be overwritten
		SaveReader reader(1, 0, fileName);
		if (!reader.load()) {
			return false;
		}

		if (fileSize == newFileSize) {
			// We can use the same SavePartVars object
			if (!reader.readPart(0, &vars)) {
				return false;
			}
		} else {
			// We need to use a temporary SavePartVars object to load data
			SavePartVars vars_from_file(_vm, fileSize);
			if (!reader.readPart(0, &vars_from_file)) {;
				return false;
			}

			// Copy data from temporary SavePartVars object to the real one
			vars.readFromRaw(vars_from_file.data(), 0, fileSize);
		}
	}

	SaveWriter writer(1, 0, fileName);
	if (ptrRaw) {
		// Write data from raw pointer
		vars.readFromRaw(ptrRaw, offset, size);
	} else {
		// Write data from variables
		if (!vars.readFrom((uint16) dataVar, offset, size))
			return false;
	}

	return writer.writePart(0, &vars);
}

bool SaveLoad_v7::GameFileHandler::save(int16 dataVar, int32 size, int32 offset) {
	return save(nullptr, dataVar, size, offset);
}

bool SaveLoad_v7::GameFileHandler::loadToRaw(byte *ptr, int32 size, int32 offset) {
	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	if (size == 0) {
		uint32 varSize = SaveHandler::getVarSize(_vm);
		// Indicator to load all variables
		size = (int32) varSize;
	}

	int32 fileSize = getSize();
	if (fileSize < 0)
		return false;

	SaveReader reader(1, 0, fileName);
	SavePartVars vars(_vm, fileSize);

	if (!reader.load()) {
		return false;
	}

	if (!reader.readPart(0, &vars)) {
		return false;
	}

	if (!vars.writeIntoRaw(ptr, offset, size)) {
		return false;
	}

	return true;
}


bool SaveLoad_v7::GameFileHandler::saveFromRaw(const byte *ptr, int32 size, int32 offset) {
	return save(ptr, 0, size, offset);
}

bool SaveLoad_v7::GameFileHandler::deleteFile() {
	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	SaveWriter writer(1, 0, fileName);
	return writer.deleteFile();
}

SaveLoad_v7::SaveLoad_v7(GobEngine *vm, const char *targetName) :
		SaveLoad(vm) {

	uint32 index = 0;
	for (uint32 i = 0; i < kChildrenCount; i++) {
		_saveFiles[index++].handler = _faceHandler[i] = new SpriteHandler(_vm,
																		targetName,
																		Common::String::format("vsg%02d", i + 1));
	}

	_saveFiles[index++].handler = _childrenHandler = new FakeFileHandler(_vm);
	for (int i = 0; i < 4; ++i) {
		_saveFiles[index++].handler = _debilHandler[i] = new FakeFileHandler(_vm);
	}

	_saveFiles[index++].handler = _configHandler   = new GameFileHandler(_vm, targetName, "cfg");

	for (uint32 i = 0; i < kChildrenCount; i++) {
		_saveFiles[index++].handler = _adibou2EnvHandler[i] = new GameFileHandler(_vm,
																				  targetName,
																				  Common::String::format("env%02d", i + 1));
	}

	for (uint32 i = 0; i < kChildrenCount; i++) {
		_saveFiles[index++].handler = _adibou2WeatherHandler[i] = new SpriteHandler(_vm,
																					targetName,
																					Common::String::format("weather_%02d", i + 1));
	}

	for (uint32 i = 0; i < kChildrenCount; i++) {
		_saveFiles[index++].handler = _adibou2BreakoutGameProgressHandler[i] = new GameFileHandler(_vm,
																								   targetName,
																								   Common::String::format("breakout_%02d", i + 1));
	}

	for (uint32 i = 0; i < kAdibou2NbrOfConstructionGameFiles; i++) {
		_saveFiles[index++].handler = _adibou2ConstructionGameTempFileHandler[i] = new FakeFileHandler(_vm);
	}

	for (uint32 i = 0; i < kChildrenCount; i++) {
		for (uint32 j = 0; j < kAdibou2NbrOfConstructionGameFiles; j++) {
			const char *fileName = (j == 0)?"construc":((j == 1)?"ptreco":"aide");
			_saveFiles[index++].handler = _adibou2ConstructionGameProgressHandler[i][j] = new GameFileHandler(_vm,
																											  targetName,
																											  Common::String::format("%s_%02d", fileName, i + 1));
		}
	}

	for (uint32 i = 0; i < kChildrenCount; i++) {
		for (uint32 j = 0; j < kAdibou2NbrOfApplications; j++) {
			Common::String ext = Common::String::format("gsa%02d_%02d", j + 1, i + 1);
			_saveFiles[index++].handler = _adibou2AppProgressHandler[i][j]  = new GameFileHandler(_vm,
																								 targetName,
																								 ext);
		}
	}

	for (uint32 i = 0; i < kChildrenCount; i++) {
		_saveFiles[index++].handler = _adibou2MemoHandler[i] = new GameFileHandler(_vm, targetName, Common::String::format("memo%02d", i + 1));
		_saveFiles[index++].handler = _adibou2DiploHandler[i] = new GameFileHandler(_vm, targetName, Common::String::format("diplo%02d", i + 1));
	}


	for (uint32 i = 0; i < kAdibou2NbrOfApplications; i++) {
		_saveFiles[index++].handler = _adibou2AppliHandler[i] = new GameFileHandler(_vm, targetName, Common::String::format("appli%02d", i + 1));
		_saveFiles[index++].handler = _adibou2CriteHandler[i] = new GameFileHandler(_vm, targetName, Common::String::format("crite%02d", i + 1));
		_saveFiles[index++].handler = _adibou2ExoHandler[i] = new GameFileHandler(_vm, targetName, Common::String::format("exo%02d", i + 1));
	}

	for (uint32 i = 0; i < kAdibou2NbrOfApplications; i++) {
		_saveFiles[index++].handler = _adibou2AppIcoHandler[i] = new SpriteHandler(_vm, targetName, Common::String::format("app_ico%02d", i + 1));
	}

	const Common::Array<int> applisOffsets = {0, 4, 8, 12, 16, 20};
	_saveFiles[index++].handler = _adibou2ApplicationsInfoHandler = new GameFileHandler(_vm, targetName, "applis");
	_saveFiles[index++].handler = _adibou2RetourHandler = new FakeFileHandler(_vm);
	_saveFiles[index++].handler = _adibou2LanceHandler = new FakeFileHandler(_vm);

	for (uint32 i = 0; i < kChildrenCount; i++) {
		for (uint32 j = 0; j < kAdibou2NbrOfSavedDrawings; j++) {
			_saveFiles[index++].handler = _adibou2DrawingHandler[i][j] = new SpriteHandler(_vm,
																						   targetName,
																						   Common::String::format("draw%02d_%02d", i + 1, j + 1));
			_saveFiles[index++].handler = _adibou2DrawingThumbnailHandler[i][j] = new SpriteHandler(_vm,
																									targetName,
																									Common::String::format("draw_thumbnail_%02d_%02d", i + 1, j + 1));
		}

		_saveFiles[index++].handler = _adibou2DrawingPaletteHandler[i] = new GameFileHandler(_vm,
																							 targetName,
																							 Common::String::format("draw_palette_%02d",
																													i + 1));
		_saveFiles[index++].handler = _adibou2DrawingMailboxHandler[i] = new SpriteHandler(_vm,
																						   targetName,
																						   Common::String::format("draw_mailbox_%02d", i + 1));

		_saveFiles[index++].handler = _adibou2CakePhotoHandler[i] = new SpriteHandler(_vm,
																					  targetName,
																					  Common::String::format("photo_cake_%02d", i + 1));

		_saveFiles[index++].handler = _adibou2FlowerPhotoHandler[i] = new SpriteHandler(_vm,
																						targetName,
																						Common::String::format("photo_flower_%02d", i + 1));

		_saveFiles[index++].handler = _adibou2FlowerInVaseHandler[i] = new SpriteHandler(_vm,
																							targetName,
																							Common::String::format("photo_flower_vase_%02d", i + 1));

		_saveFiles[index++].handler = _adibou2FadedFlowerInVaseHandler[i] = new SpriteHandler(_vm,
																							 targetName,
																							 Common::String::format("photo_flower_vase_2_%02d", i + 1));

		_saveFiles[index++].handler = _adibou2PosterHandler[i] = new SpriteHandler(_vm,
																				   targetName,
																				   Common::String::format("poster_%02d", i + 1));
	}

	uint32 drawingOnFloppyDiskFullSize = 163200;
	uint32 drawingOnFloppyDiskChunkSize = 4800;
	uint32 nbrOfParts = 1 + drawingOnFloppyDiskFullSize / drawingOnFloppyDiskChunkSize; // 1 for the thumbnail, 34 for the drawing
	Common::String drawingOnFloppyDiskName = "adibou2_my_drawing_on_floppy_disk"; // Fix filename to facilitate sharing

	SaveReader *reader = new SaveReader(nbrOfParts, 0, drawingOnFloppyDiskName);
	SaveWriter *writer = new SaveWriter(nbrOfParts, 0, drawingOnFloppyDiskName);

	_saveFiles[index++].handler = _adibou2DrawingOnFloppyDiskHandler = new DrawingOnFloppyDiskHandler(_vm,
																									  reader,
																									  writer,
																									  false,
																									  drawingOnFloppyDiskChunkSize);
	_saveFiles[index++].handler = _adibou2DrawingThumbnailOnFloppyDiskHandler = new DrawingOnFloppyDiskHandler(_vm,
																											   reader,
																											   writer,
																											   true);
	_saveFiles[index++].handler = _adibou2TestDobHandler = new FakeFileHandler(_vm);
	_saveFiles[index++].handler = _adibou2ExerciseListHandler = new FakeFileHandler(_vm);

	for (int i = 0; i < 2; i++)
		_saveFiles[index++].handler = _addy4BaseHandler[i] = new FakeFileHandler(_vm);

	for (int i = 0; i < 11; i++)
		_saveFiles[index++].handler = _addy4GrundschuleHandler[i] = new FakeFileHandler(_vm);
}

SaveLoad_v7::~SaveLoad_v7() {
	for (int i = 0; i < 11; i++)
		delete _addy4GrundschuleHandler[i];

	for (int i = 0; i < 2; i++)
		delete _addy4BaseHandler[i];

	delete _configHandler;
	for (int i = 0; i < 4; i++)
		delete _debilHandler[i];
	delete _childrenHandler;

	for (uint32 i = 0; i < kChildrenCount; i++) {
		delete _adibou2EnvHandler[i];
		delete _adibou2WeatherHandler[i];
		delete _adibou2BreakoutGameProgressHandler[i];

		for (uint32 j = 0; j < kAdibou2NbrOfConstructionGameFiles; j++)
			delete _adibou2ConstructionGameProgressHandler[i][j];

		for (uint32 j = 0; j < kAdibou2NbrOfApplications; j++)
			delete _adibou2AppProgressHandler[i][j];
	}

	for (uint32 i = 0; i < kAdibou2NbrOfConstructionGameFiles; i++)
		delete _adibou2ConstructionGameTempFileHandler[i];

	for (uint32 i = 0; i < kChildrenCount; i++) {
		delete _faceHandler[i];
		delete _adibou2MemoHandler[i];
		delete _adibou2DiploHandler[i];
	}

	for (uint32 i = 0; i < kAdibou2NbrOfApplications; i++) {
		delete _adibou2AppliHandler[i];
		delete _adibou2CriteHandler[i];
		delete _adibou2ExoHandler[i];
	}

	for (uint32 i = 0; i < kAdibou2NbrOfApplications; i++) {
		delete _adibou2AppIcoHandler[i];
	}

	delete _adibou2ApplicationsInfoHandler;
	delete _adibou2RetourHandler;
	delete _adibou2LanceHandler;

	for (uint32 i = 0; i < kChildrenCount; i++) {
		for (uint32 j = 0; j < kAdibou2NbrOfSavedDrawings; j++) {
			delete _adibou2DrawingHandler[i][j];
			delete _adibou2DrawingThumbnailHandler[i][j];
		}

		delete _adibou2DrawingPaletteHandler[i];
		delete _adibou2DrawingMailboxHandler[i];
		delete _adibou2CakePhotoHandler[i];
		delete _adibou2FlowerPhotoHandler[i];
		delete _adibou2FlowerInVaseHandler[i];
		delete _adibou2FadedFlowerInVaseHandler[i];
		delete _adibou2PosterHandler[i];
	}

	delete _adibou2DrawingOnFloppyDiskHandler;
	delete _adibou2DrawingThumbnailOnFloppyDiskHandler;
	delete _adibou2TestDobHandler;
	delete _adibou2ExerciseListHandler;
}

const SaveLoad_v7::SaveFile *SaveLoad_v7::getSaveFile(const char *fileName) const {
	Common::String pathNormalized = replacePathSeparators(fileName, '/');

	// First, search for full path match
	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(pathNormalized.c_str(), _saveFiles[i].sourceName))
			return &_saveFiles[i];

	// If no match has been found, search for a filename-only match, ignoring paths
	fileName = stripPath(pathNormalized.c_str(), '/');
	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, stripPath(_saveFiles[i].sourceName, '/')))
			return &_saveFiles[i];

	return nullptr;
}

SaveHandler *SaveLoad_v7::getHandler(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->handler;

	return nullptr;
}

const char *SaveLoad_v7::getDescription(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->description;

	return nullptr;
}

SaveLoad::SaveMode SaveLoad_v7::getSaveMode(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->mode;

	return kSaveModeNone;
}

Common::List<Common::String> SaveLoad_v7::getFilesMatchingPattern(const char *pattern) const {
	Common::List<Common::String> files;
	Common::String patternNormalized = replacePathSeparators(pattern, '/');

	for (const SaveFile &saveFile : _saveFiles) {
		// Full path match only
		if (Common::matchString(saveFile.sourceName, patternNormalized.c_str(), true))
			files.push_back(saveFile.sourceName);
	}

	return files;
}

} // End of namespace Gob
