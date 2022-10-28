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
	// Addy Junior Base
	{"visage01.inf", kSaveModeSave, nullptr, "face"         }, // Child 01
	{"visage02.inf", kSaveModeSave, nullptr, "face"         }, // Child 02
	{"visage03.inf", kSaveModeSave, nullptr, "face"         }, // Child 03
	{"visage04.inf", kSaveModeSave, nullptr, "face"         }, // Child 04
	{"visage05.inf", kSaveModeSave, nullptr, "face"         }, // Child 05
	{"visage06.inf", kSaveModeSave, nullptr, "face"         }, // Child 06
	{"visage07.inf", kSaveModeSave, nullptr, "face"         }, // Child 07
	{"visage08.inf", kSaveModeSave, nullptr, "face"         }, // Child 08
	{"visage09.inf", kSaveModeSave, nullptr, "face"         }, // Child 09
	{"visage10.inf", kSaveModeSave, nullptr, "face"         }, // Child 10
	{"visage11.inf", kSaveModeSave, nullptr, "face"         }, // Child 11
	{"visage12.inf", kSaveModeSave, nullptr, "face"         }, // Child 12
	{"visage13.inf", kSaveModeSave, nullptr, "face"         }, // Child 13
	{"visage14.inf", kSaveModeSave, nullptr, "face"         }, // Child 14
	{"visage15.inf", kSaveModeSave, nullptr, "face"         }, // Child 15
	{"visage16.inf", kSaveModeSave, nullptr, "face"         }, // Child 16
	{  "enfant.inf", kSaveModeSave, nullptr, "children"     },
	{   "debil.tmp", kSaveModeSave, nullptr, nullptr        },
	{  "config.inf", kSaveModeSave, nullptr, "configuration"},
	// Adibou environment (flowers, vegetables...)
	{"enviro01.inf", kSaveModeSave, 0, "environment" }, // Child 01
	{"enviro02.inf", kSaveModeSave, 0, "environment" }, // Child 02
	{"enviro03.inf", kSaveModeSave, 0, "environment" }, // Child 03
	{"enviro04.inf", kSaveModeSave, 0, "environment" }, // Child 04
	{"enviro05.inf", kSaveModeSave, 0, "environment" }, // Child 05
	{"enviro06.inf", kSaveModeSave, 0, "environment" }, // Child 06
	{"enviro07.inf", kSaveModeSave, 0, "environment" }, // Child 07
	{"enviro08.inf", kSaveModeSave, 0, "environment" }, // Child 08
    {"enviro09.inf", kSaveModeSave, 0, "environment" }, // Child 09
	{"enviro10.inf", kSaveModeSave, 0, "environment" }, // Child 10
	{"enviro11.inf", kSaveModeSave, 0, "environment" }, // Child 11
	{"enviro12.inf", kSaveModeSave, 0, "environment" }, // Child 12
    {"enviro13.inf", kSaveModeSave, 0, "environment" }, // Child 13
	{"enviro14.inf", kSaveModeSave, 0, "environment" }, // Child 14
	{"enviro15.inf", kSaveModeSave, 0, "environment" }, // Child 15
	{"enviro16.inf", kSaveModeSave, 0, "environment" }, // Child 16

	// Adibou environment weather
	{"temp01.csa"  , kSaveModeSave, 0, "weather" },
	{"temp02.csa"  , kSaveModeSave, 0, "weather" },
	{"temp03.csa"  , kSaveModeSave, 0, "weather" },
	{"temp04.csa"  , kSaveModeSave, 0, "weather" },
	{"temp05.csa"  , kSaveModeSave, 0, "weather" },
	{"temp06.csa"  , kSaveModeSave, 0, "weather" },
	{"temp07.csa"  , kSaveModeSave, 0, "weather" },
	{"temp08.csa"  , kSaveModeSave, 0, "weather" },
	{"temp09.csa"  , kSaveModeSave, 0, "weather" },
	{"temp10.csa"  , kSaveModeSave, 0, "weather" },
	{"temp11.csa"  , kSaveModeSave, 0, "weather" },
	{"temp12.csa"  , kSaveModeSave, 0, "weather" },
	{"temp13.csa"  , kSaveModeSave, 0, "weather" },
	{"temp14.csa"  , kSaveModeSave, 0, "weather" },
	{"temp15.csa"  , kSaveModeSave, 0, "weather" },
	{"temp16.csa"  , kSaveModeSave, 0, "weather" },

	// Adibou brekout game progress
	{"brique01.inf"  , kSaveModeSave, 0, "breakout game progress" },
	{"brique02.inf"  , kSaveModeSave, 0, "breakout game progress" },
	{"brique03.inf"  , kSaveModeSave, 0, "breakout game progress" },
	{"brique04.inf"  , kSaveModeSave, 0, "breakout game progress" },
	{"brique05.inf"  , kSaveModeSave, 0, "breakout game progress" },
	{"brique06.inf"  , kSaveModeSave, 0, "breakout game progress" },
	{"brique07.inf"  , kSaveModeSave, 0, "breakout game progress" },
	{"brique08.inf"  , kSaveModeSave, 0, "breakout game progress" },
	{"brique09.inf"  , kSaveModeSave, 0, "breakout game progress" },
	{"brique10.inf"  , kSaveModeSave, 0, "breakout game progress" },
	{"brique11.inf"  , kSaveModeSave, 0, "breakout game progress" },
	{"brique12.inf"  , kSaveModeSave, 0, "breakout game progress" },
	{"brique13.inf"  , kSaveModeSave, 0, "breakout game progress" },
	{"brique14.inf"  , kSaveModeSave, 0, "breakout game progress" },
	{"brique15.inf"  , kSaveModeSave, 0, "breakout game progress" },
	{"brique16.inf"  , kSaveModeSave, 0, "breakout game progress" },

	// Adibou Playtoon-like minigame
	{"construc.inf"  , kSaveModeSave, 0, "construction game progress" },
	{"ptrecord.inf"  , kSaveModeSave, 0, "construction game progress" }, // PTRECORD = "Playtoons record" probably
	{"aide.inf"      , kSaveModeSave, 0, "construction game progress" },

	{"constr01.inf"  , kSaveModeSave, 0, "construction game progress" }, // Child 01
	{"ptreco01.inf"  , kSaveModeSave, 0, "construction game progress" },
	{"aide01.inf"    , kSaveModeSave, 0, "construction game progress" },
	{"constr02.inf"  , kSaveModeSave, 0, "construction game progress" }, // Child 02
	{"ptreco02.inf"  , kSaveModeSave, 0, "construction game progress" },
	{"aide02.inf"    , kSaveModeSave, 0, "construction game progress" },
	{"constr03.inf"  , kSaveModeSave, 0, "construction game progress" }, // Child 03
	{"ptreco03.inf"  , kSaveModeSave, 0, "construction game progress" },
	{"aide03.inf"    , kSaveModeSave, 0, "construction game progress" },
	{"constr04.inf"  , kSaveModeSave, 0, "construction game progress" }, // Child 04
	{"ptreco04.inf"  , kSaveModeSave, 0, "construction game progress" },
	{"aide04.inf"    , kSaveModeSave, 0, "construction game progress" },
	{"constr05.inf"  , kSaveModeSave, 0, "construction game progress" }, // Child 05
	{"ptreco05.inf"  , kSaveModeSave, 0, "construction game progress" },
	{"aide05.inf"    , kSaveModeSave, 0, "construction game progress" },
	{"constr06.inf"  , kSaveModeSave, 0, "construction game progress" }, // Child 06
	{"ptreco06.inf"  , kSaveModeSave, 0, "construction game progress" },
	{"aide06.inf"    , kSaveModeSave, 0, "construction game progress" },
	{"constr07.inf"  , kSaveModeSave, 0, "construction game progress" }, // Child 07
	{"ptreco07.inf"  , kSaveModeSave, 0, "construction game progress" },
	{"aide07.inf"    , kSaveModeSave, 0, "construction game progress" },
	{"constr08.inf"  , kSaveModeSave, 0, "construction game progress" }, // Child 08
	{"ptreco08.inf"  , kSaveModeSave, 0, "construction game progress" },
	{"aide08.inf"    , kSaveModeSave, 0, "construction game progress" },
	{"constr09.inf"  , kSaveModeSave, 0, "construction game progress" }, // Child 09
	{"ptreco09.inf"  , kSaveModeSave, 0, "construction game progress" },
	{"aide09.inf"    , kSaveModeSave, 0, "construction game progress" },
	{"constr10.inf"  , kSaveModeSave, 0, "construction game progress" }, // Child 10
	{"ptreco10.inf"  , kSaveModeSave, 0, "construction game progress" },
	{"aide10.inf"    , kSaveModeSave, 0, "construction game progress" },
	{"constr11.inf"  , kSaveModeSave, 0, "construction game progress" }, // Child 11
	{"ptreco11.inf"  , kSaveModeSave, 0, "construction game progress" },
	{"aide11.inf"    , kSaveModeSave, 0, "construction game progress" },
	{"constr12.inf"  , kSaveModeSave, 0, "construction game progress" }, // Child 12
	{"ptreco12.inf"  , kSaveModeSave, 0, "construction game progress" },
	{"aide12.inf"    , kSaveModeSave, 0, "construction game progress" },
	{"constr13.inf"  , kSaveModeSave, 0, "construction game progress" }, // Child 13
	{"ptreco13.inf"  , kSaveModeSave, 0, "construction game progress" },
	{"aide13.inf"    , kSaveModeSave, 0, "construction game progress" },
	{"constr14.inf"  , kSaveModeSave, 0, "construction game progress" }, // Child 14
	{"ptreco14.inf"  , kSaveModeSave, 0, "construction game progress" },
	{"aide14.inf"    , kSaveModeSave, 0, "construction game progress" },
	{"constr15.inf"  , kSaveModeSave, 0, "construction game progress" }, // Child 15
	{"ptreco15.inf"  , kSaveModeSave, 0, "construction game progress" },
	{"aide15.inf"    , kSaveModeSave, 0, "construction game progress" },
	{"constr16.inf"  , kSaveModeSave, 0, "construction game progress" }, // Child 16
	{"ptreco16.inf"  , kSaveModeSave, 0, "construction game progress" },
	{"aide16.inf"    , kSaveModeSave, 0, "construction game progress" },

	// Adibou Applications 1-5
    {"Gsa01_01.inf", kSaveModeSave, 0, "app progress" }, // Child 01
    {"Gsa02_01.inf", kSaveModeSave, 0, "app progress" },
    {"Gsa03_01.inf", kSaveModeSave, 0, "app progress" },
    {"Gsa04_01.inf", kSaveModeSave, 0, "app progress" },
    {"Gsa05_01.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_02.inf", kSaveModeSave, 0, "app progress" }, // Child 02
	{"Gsa02_02.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_02.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_02.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_02.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_03.inf", kSaveModeSave, 0, "app progress" }, // Child 03
	{"Gsa02_03.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_03.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_03.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_03.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_04.inf", kSaveModeSave, 0, "app progress" }, // Child 04
	{"Gsa02_04.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_04.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_04.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_04.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_05.inf", kSaveModeSave, 0, "app progress" }, // Child 05
	{"Gsa02_05.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_05.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_05.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_05.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_06.inf", kSaveModeSave, 0, "app progress" }, // Child 06
	{"Gsa02_06.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_06.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_06.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_06.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_07.inf", kSaveModeSave, 0, "app progress" }, // Child 07
	{"Gsa02_07.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_07.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_07.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_07.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_08.inf", kSaveModeSave, 0, "app progress" }, // Child 08
	{"Gsa02_08.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_08.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_08.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_08.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_09.inf", kSaveModeSave, 0, "app progress" }, // Child 09
	{"Gsa02_09.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_09.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_09.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_09.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_10.inf", kSaveModeSave, 0, "app progress" }, // Child 10
	{"Gsa02_10.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_10.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_10.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_10.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_11.inf", kSaveModeSave, 0, "app progress" }, // Child 11
	{"Gsa02_11.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_11.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_11.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_11.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_12.inf", kSaveModeSave, 0, "app progress" }, // Child 12
	{"Gsa02_12.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_12.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_12.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_12.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_13.inf", kSaveModeSave, 0, "app progress" }, // Child 13
	{"Gsa02_13.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_13.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_13.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_13.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_14.inf", kSaveModeSave, 0, "app progress" }, // Child 14
	{"Gsa02_14.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_14.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_14.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_14.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_15.inf", kSaveModeSave, 0, "app progress" }, // Child 15
	{"Gsa02_15.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_15.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_15.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_15.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_16.inf", kSaveModeSave, 0, "app progress" }, // Child 16
	{"Gsa02_16.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_16.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_16.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_16.inf", kSaveModeSave, 0, "app progress" },
	{"memo_01.inf", kSaveModeSave, 0, "memo" }, // Child 01
	{"memo_02.inf", kSaveModeSave, 0, "memo" }, // Child 02
	{"memo_03.inf", kSaveModeSave, 0, "memo" }, // Child 03
	{"memo_04.inf", kSaveModeSave, 0, "memo" }, // Child 04
	{"memo_05.inf", kSaveModeSave, 0, "memo" }, // Child 05
	{"memo_06.inf", kSaveModeSave, 0, "memo" }, // Child 06
	{"memo_07.inf", kSaveModeSave, 0, "memo" }, // Child 07
	{"memo_08.inf", kSaveModeSave, 0, "memo" }, // Child 08
	{"memo_09.inf", kSaveModeSave, 0, "memo" }, // Child 09
	{"memo_10.inf", kSaveModeSave, 0, "memo" }, // Child 10
	{"memo_11.inf", kSaveModeSave, 0, "memo" }, // Child 11
	{"memo_12.inf", kSaveModeSave, 0, "memo" }, // Child 12
	{"memo_13.inf", kSaveModeSave, 0, "memo" }, // Child 13
	{"memo_14.inf", kSaveModeSave, 0, "memo" }, // Child 14
	{"memo_15.inf", kSaveModeSave, 0, "memo" }, // Child 15
	{"memo_16.inf", kSaveModeSave, 0, "memo" }, // Child 16
	{"diplo_01.inf", kSaveModeSave, 0, "diplo" }, // Child 01
	{"diplo_02.inf", kSaveModeSave, 0, "diplo" }, // Child 02
	{"diplo_03.inf", kSaveModeSave, 0, "diplo" }, // Child 03
	{"diplo_04.inf", kSaveModeSave, 0, "diplo" }, // Child 04
	{"diplo_05.inf", kSaveModeSave, 0, "diplo" }, // Child 05
	{"diplo_06.inf", kSaveModeSave, 0, "diplo" }, // Child 06
	{"diplo_07.inf", kSaveModeSave, 0, "diplo" }, // Child 07
	{"diplo_08.inf", kSaveModeSave, 0, "diplo" }, // Child 08
	{"diplo_09.inf", kSaveModeSave, 0, "diplo" }, // Child 09
	{"diplo_10.inf", kSaveModeSave, 0, "diplo" }, // Child 10
	{"diplo_11.inf", kSaveModeSave, 0, "diplo" }, // Child 11
	{"diplo_12.inf", kSaveModeSave, 0, "diplo" }, // Child 12
	{"diplo_13.inf", kSaveModeSave, 0, "diplo" }, // Child 13
	{"diplo_14.inf", kSaveModeSave, 0, "diplo" }, // Child 14
	{"diplo_15.inf", kSaveModeSave, 0, "diplo" }, // Child 15
	{"diplo_16.inf", kSaveModeSave, 0, "diplo" }, // Child 16
	{"appli_01.inf", kSaveModeSave, 0, "app info" },
	{"appli_02.inf", kSaveModeSave, 0, "app info" },
	{"appli_03.inf", kSaveModeSave, 0, "app info" },
	{"appli_04.inf", kSaveModeSave, 0, "app info" },
	{"appli_05.inf", kSaveModeSave, 0, "app info" },
	{"crite_01.inf", kSaveModeSave, 0, "app info" },
	{"crite_02.inf", kSaveModeSave, 0, "app info" },
	{"crite_03.inf", kSaveModeSave, 0, "app info" },
	{"crite_04.inf", kSaveModeSave, 0, "app info" },
	{"crite_05.inf", kSaveModeSave, 0, "app info" },
	{"exo_01.inf", kSaveModeSave, 0, "app info" },
	{"exo_02.inf", kSaveModeSave, 0, "app info" },
	{"exo_03.inf", kSaveModeSave, 0, "app info" },
	{"exo_04.inf", kSaveModeSave, 0, "app info" },
	{"exo_05.inf", kSaveModeSave, 0, "app info" },
	{"ico_01.inf", kSaveModeSave, 0, "app info" },
	{"ico_02.inf", kSaveModeSave, 0, "app info" },
	{"ico_03.inf", kSaveModeSave, 0, "app info" },
	{"ico_04.inf", kSaveModeSave, 0, "app info" },
	{"ico_05.inf", kSaveModeSave, 0, "app info" },
	{"applis.inf", kSaveModeSave, 0, "app info" },
	{"lance.inf", kSaveModeSave, 0, "app info" },
	{"retour.inf", kSaveModeSave, 0, "app info" },
	// Adibou2 saved drawings (up to 12)
	// Child1
	{"cart0101.inf", kSaveModeSave, 0, "drawing" },
	{"cart0201.inf", kSaveModeSave, 0, "drawing" },
	{"cart0301.inf", kSaveModeSave, 0, "drawing" },
	{"cart0401.inf", kSaveModeSave, 0, "drawing" },
	{"cart0501.inf", kSaveModeSave, 0, "drawing" },
	{"cart0601.inf", kSaveModeSave, 0, "drawing" },
	{"cart0701.inf", kSaveModeSave, 0, "drawing" },
	{"cart0801.inf", kSaveModeSave, 0, "drawing" },
	{"cart0901.inf", kSaveModeSave, 0, "drawing" },
	{"cart1001.inf", kSaveModeSave, 0, "drawing" },
	{"cart1101.inf", kSaveModeSave, 0, "drawing" },
	{"cart1201.inf", kSaveModeSave, 0, "drawing" },
	{"pal00_01.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal01_01.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal02_01.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal03_01.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal04_01.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal05_01.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal06_01.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal07_01.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal08_01.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal09_01.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal10_01.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal11_01.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"palett01.inf", kSaveModeSave, 0, "drawing palette" },
	{"plettr01.inf", kSaveModeSave, 0, "drawing in mailbox" },
	{"pgatod01.inf", kSaveModeSave, 0, "photo of cake" },
	{"pfleur01.inf", kSaveModeSave, 0, "photo of flowers" },
	{"mfleur01.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mfleud01.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mtable01.inf", kSaveModeSave, 0, "poster" },
	// Child2
	{"cart0102.inf", kSaveModeSave, 0, "drawing" },
	{"cart0202.inf", kSaveModeSave, 0, "drawing" },
	{"cart0302.inf", kSaveModeSave, 0, "drawing" },
	{"cart0402.inf", kSaveModeSave, 0, "drawing" },
	{"cart0502.inf", kSaveModeSave, 0, "drawing" },
	{"cart0602.inf", kSaveModeSave, 0, "drawing" },
	{"cart0702.inf", kSaveModeSave, 0, "drawing" },
	{"cart0802.inf", kSaveModeSave, 0, "drawing" },
	{"cart0902.inf", kSaveModeSave, 0, "drawing" },
	{"cart1002.inf", kSaveModeSave, 0, "drawing" },
	{"cart1102.inf", kSaveModeSave, 0, "drawing" },
	{"cart1202.inf", kSaveModeSave, 0, "drawing" },
	{"pal00_02.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal01_02.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal02_02.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal03_02.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal04_02.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal05_02.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal06_02.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal07_02.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal08_02.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal09_02.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal10_02.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal11_02.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"palett02.inf", kSaveModeSave, 0, "drawing palette" },
	{"plettr02.inf", kSaveModeSave, 0, "drawing in mailbox" },
	{"pgatod02.inf", kSaveModeSave, 0, "photo of cake" },
	{"pfleur02.inf", kSaveModeSave, 0, "photo of flowers" },
	{"mfleur02.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mfleud02.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mtable02.inf", kSaveModeSave, 0, "poster" },
	// Child3
	{"cart0103.inf", kSaveModeSave, 0, "drawing" },
	{"cart0203.inf", kSaveModeSave, 0, "drawing" },
	{"cart0303.inf", kSaveModeSave, 0, "drawing" },
	{"cart0403.inf", kSaveModeSave, 0, "drawing" },
	{"cart0503.inf", kSaveModeSave, 0, "drawing" },
	{"cart0603.inf", kSaveModeSave, 0, "drawing" },
	{"cart0703.inf", kSaveModeSave, 0, "drawing" },
	{"cart0803.inf", kSaveModeSave, 0, "drawing" },
	{"cart0903.inf", kSaveModeSave, 0, "drawing" },
	{"cart1003.inf", kSaveModeSave, 0, "drawing" },
	{"cart1103.inf", kSaveModeSave, 0, "drawing" },
	{"cart1203.inf", kSaveModeSave, 0, "drawing" },
	{"pal00_03.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal01_03.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal02_03.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal03_03.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal04_03.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal05_03.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal06_03.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal07_03.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal08_03.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal09_03.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal10_03.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal11_03.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"palett03.inf", kSaveModeSave, 0, "drawing palette" },
	{"plettr03.inf", kSaveModeSave, 0, "drawing in mailbox" },
	{"pgatod03.inf", kSaveModeSave, 0, "photo of cake" },
	{"pfleur03.inf", kSaveModeSave, 0, "photo of flowers" },
	{"mfleur03.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mfleud03.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mtable03.inf", kSaveModeSave, 0, "poster" },
	// Child4
	{"cart0104.inf", kSaveModeSave, 0, "drawing" },
	{"cart0204.inf", kSaveModeSave, 0, "drawing" },
	{"cart0304.inf", kSaveModeSave, 0, "drawing" },
	{"cart0404.inf", kSaveModeSave, 0, "drawing" },
	{"cart0504.inf", kSaveModeSave, 0, "drawing" },
	{"cart0604.inf", kSaveModeSave, 0, "drawing" },
	{"cart0704.inf", kSaveModeSave, 0, "drawing" },
	{"cart0804.inf", kSaveModeSave, 0, "drawing" },
	{"cart0904.inf", kSaveModeSave, 0, "drawing" },
	{"cart1004.inf", kSaveModeSave, 0, "drawing" },
	{"cart1104.inf", kSaveModeSave, 0, "drawing" },
	{"cart1204.inf", kSaveModeSave, 0, "drawing" },
	{"pal00_04.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal01_04.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal02_04.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal03_04.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal04_04.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal05_04.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal06_04.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal07_04.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal08_04.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal09_04.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal10_04.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal11_04.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"palett04.inf", kSaveModeSave, 0, "drawing palette" },
	{"plettr04.inf", kSaveModeSave, 0, "drawing in mailbox" },
	{"pgatod04.inf", kSaveModeSave, 0, "photo of cake" },
	{"pfleur04.inf", kSaveModeSave, 0, "photo of flowers" },
	{"mfleur04.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mfleud04.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mtable04.inf", kSaveModeSave, 0, "poster" },
	// Child5
	{"cart0105.inf", kSaveModeSave, 0, "drawing" },
	{"cart0205.inf", kSaveModeSave, 0, "drawing" },
	{"cart0305.inf", kSaveModeSave, 0, "drawing" },
	{"cart0405.inf", kSaveModeSave, 0, "drawing" },
	{"cart0505.inf", kSaveModeSave, 0, "drawing" },
	{"cart0605.inf", kSaveModeSave, 0, "drawing" },
	{"cart0705.inf", kSaveModeSave, 0, "drawing" },
	{"cart0805.inf", kSaveModeSave, 0, "drawing" },
	{"cart0905.inf", kSaveModeSave, 0, "drawing" },
	{"cart1005.inf", kSaveModeSave, 0, "drawing" },
	{"cart1105.inf", kSaveModeSave, 0, "drawing" },
	{"cart1205.inf", kSaveModeSave, 0, "drawing" },
	{"pal00_05.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal01_05.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal02_05.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal03_05.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal04_05.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal05_05.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal06_05.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal07_05.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal08_05.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal09_05.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal10_05.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal11_05.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"palett05.inf", kSaveModeSave, 0, "drawing palette" },
	{"plettr05.inf", kSaveModeSave, 0, "drawing in mailbox" },
	{"pgatod05.inf", kSaveModeSave, 0, "photo of cake" },
	{"pfleur05.inf", kSaveModeSave, 0, "photo of flowers" },
	{"mfleur05.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mfleud05.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mtable05.inf", kSaveModeSave, 0, "poster" },
	// Child6
	{"cart0106.inf", kSaveModeSave, 0, "drawing" },
	{"cart0206.inf", kSaveModeSave, 0, "drawing" },
	{"cart0306.inf", kSaveModeSave, 0, "drawing" },
	{"cart0406.inf", kSaveModeSave, 0, "drawing" },
	{"cart0506.inf", kSaveModeSave, 0, "drawing" },
	{"cart0606.inf", kSaveModeSave, 0, "drawing" },
	{"cart0706.inf", kSaveModeSave, 0, "drawing" },
	{"cart0806.inf", kSaveModeSave, 0, "drawing" },
	{"cart0906.inf", kSaveModeSave, 0, "drawing" },
	{"cart1006.inf", kSaveModeSave, 0, "drawing" },
	{"cart1106.inf", kSaveModeSave, 0, "drawing" },
	{"cart1206.inf", kSaveModeSave, 0, "drawing" },
	{"pal00_06.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal01_06.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal02_06.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal03_06.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal04_06.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal05_06.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal06_06.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal07_06.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal08_06.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal09_06.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal10_06.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal11_06.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"palett06.inf", kSaveModeSave, 0, "drawing palette" },
	{"plettr06.inf", kSaveModeSave, 0, "drawing in mailbox" },
	{"pgatod06.inf", kSaveModeSave, 0, "photo of cake" },
	{"pfleur06.inf", kSaveModeSave, 0, "photo of flowers" },
	{"mfleur06.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mfleud06.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mtable06.inf", kSaveModeSave, 0, "poster" },
	// Child7
	{"cart0107.inf", kSaveModeSave, 0, "drawing" },
	{"cart0207.inf", kSaveModeSave, 0, "drawing" },
	{"cart0307.inf", kSaveModeSave, 0, "drawing" },
	{"cart0407.inf", kSaveModeSave, 0, "drawing" },
	{"cart0507.inf", kSaveModeSave, 0, "drawing" },
	{"cart0607.inf", kSaveModeSave, 0, "drawing" },
	{"cart0707.inf", kSaveModeSave, 0, "drawing" },
	{"cart0807.inf", kSaveModeSave, 0, "drawing" },
	{"cart0907.inf", kSaveModeSave, 0, "drawing" },
	{"cart1007.inf", kSaveModeSave, 0, "drawing" },
	{"cart1107.inf", kSaveModeSave, 0, "drawing" },
	{"cart1207.inf", kSaveModeSave, 0, "drawing" },
	{"pal00_07.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal01_07.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal02_07.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal03_07.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal04_07.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal05_07.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal06_07.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal07_07.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal08_07.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal09_07.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal10_07.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal11_07.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"palett07.inf", kSaveModeSave, 0, "drawing palette" },
	{"plettr07.inf", kSaveModeSave, 0, "drawing in mailbox" },
	{"pgatod07.inf", kSaveModeSave, 0, "photo of cake" },
	{"pfleur07.inf", kSaveModeSave, 0, "photo of flowers" },
	{"mfleur07.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mfleud07.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mtable07.inf", kSaveModeSave, 0, "poster" },
	// Child8
	{"cart0108.inf", kSaveModeSave, 0, "drawing" },
	{"cart0208.inf", kSaveModeSave, 0, "drawing" },
	{"cart0308.inf", kSaveModeSave, 0, "drawing" },
	{"cart0408.inf", kSaveModeSave, 0, "drawing" },
	{"cart0508.inf", kSaveModeSave, 0, "drawing" },
	{"cart0608.inf", kSaveModeSave, 0, "drawing" },
	{"cart0708.inf", kSaveModeSave, 0, "drawing" },
	{"cart0808.inf", kSaveModeSave, 0, "drawing" },
	{"cart0908.inf", kSaveModeSave, 0, "drawing" },
	{"cart1008.inf", kSaveModeSave, 0, "drawing" },
	{"cart1108.inf", kSaveModeSave, 0, "drawing" },
	{"cart1208.inf", kSaveModeSave, 0, "drawing" },
	{"pal00_08.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal01_08.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal02_08.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal03_08.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal04_08.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal05_08.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal06_08.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal07_08.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal08_08.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal09_08.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal10_08.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal11_08.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"palett08.inf", kSaveModeSave, 0, "drawing palette" },
	{"plettr08.inf", kSaveModeSave, 0, "drawing in mailbox" },
	{"pgatod08.inf", kSaveModeSave, 0, "photo of cake" },
	{"pfleur08.inf", kSaveModeSave, 0, "photo of flowers" },
	{"mfleur08.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mfleud08.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mtable08.inf", kSaveModeSave, 0, "poster" },
	// Child9
	{"cart0109.inf", kSaveModeSave, 0, "drawing" },
	{"cart0209.inf", kSaveModeSave, 0, "drawing" },
	{"cart0309.inf", kSaveModeSave, 0, "drawing" },
	{"cart0409.inf", kSaveModeSave, 0, "drawing" },
	{"cart0509.inf", kSaveModeSave, 0, "drawing" },
	{"cart0609.inf", kSaveModeSave, 0, "drawing" },
	{"cart0709.inf", kSaveModeSave, 0, "drawing" },
	{"cart0809.inf", kSaveModeSave, 0, "drawing" },
	{"cart0909.inf", kSaveModeSave, 0, "drawing" },
	{"cart1009.inf", kSaveModeSave, 0, "drawing" },
	{"cart1109.inf", kSaveModeSave, 0, "drawing" },
	{"cart1209.inf", kSaveModeSave, 0, "drawing" },
	{"pal00_09.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal01_09.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal02_09.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal03_09.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal04_09.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal05_09.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal06_09.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal07_09.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal08_09.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal09_09.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal10_09.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal11_09.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"palett09.inf", kSaveModeSave, 0, "drawing palette" },
	{"plettr09.inf", kSaveModeSave, 0, "drawing in mailbox" },
	{"pgatod09.inf", kSaveModeSave, 0, "photo of cake" },
	{"pfleur09.inf", kSaveModeSave, 0, "photo of flowers" },
	{"mfleur09.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mfleud09.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mtable09.inf", kSaveModeSave, 0, "poster" },
	// Child10
	{"cart0110.inf", kSaveModeSave, 0, "drawing" },
	{"cart0210.inf", kSaveModeSave, 0, "drawing" },
	{"cart0310.inf", kSaveModeSave, 0, "drawing" },
	{"cart0410.inf", kSaveModeSave, 0, "drawing" },
	{"cart0510.inf", kSaveModeSave, 0, "drawing" },
	{"cart0610.inf", kSaveModeSave, 0, "drawing" },
	{"cart0710.inf", kSaveModeSave, 0, "drawing" },
	{"cart0810.inf", kSaveModeSave, 0, "drawing" },
	{"cart0910.inf", kSaveModeSave, 0, "drawing" },
	{"cart1010.inf", kSaveModeSave, 0, "drawing" },
	{"cart1110.inf", kSaveModeSave, 0, "drawing" },
	{"cart1210.inf", kSaveModeSave, 0, "drawing" },
	{"pal00_10.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal01_10.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal02_10.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal03_10.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal04_10.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal05_10.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal06_10.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal07_10.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal08_10.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal09_10.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal10_10.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal11_10.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"palett10.inf", kSaveModeSave, 0, "drawing palette" },
	{"plettr10.inf", kSaveModeSave, 0, "drawing in mailbox" },
	{"pgatod10.inf", kSaveModeSave, 0, "photo of cake" },
	{"pfleur10.inf", kSaveModeSave, 0, "photo of flowers" },
	{"mfleur10.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mfleud10.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mtable10.inf", kSaveModeSave, 0, "poster" },
	// Child11
	{"cart0111.inf", kSaveModeSave, 0, "drawing" },
	{"cart0211.inf", kSaveModeSave, 0, "drawing" },
	{"cart0311.inf", kSaveModeSave, 0, "drawing" },
	{"cart0411.inf", kSaveModeSave, 0, "drawing" },
	{"cart0511.inf", kSaveModeSave, 0, "drawing" },
	{"cart0611.inf", kSaveModeSave, 0, "drawing" },
	{"cart0711.inf", kSaveModeSave, 0, "drawing" },
	{"cart0811.inf", kSaveModeSave, 0, "drawing" },
	{"cart0911.inf", kSaveModeSave, 0, "drawing" },
	{"cart1011.inf", kSaveModeSave, 0, "drawing" },
	{"cart1111.inf", kSaveModeSave, 0, "drawing" },
	{"cart1211.inf", kSaveModeSave, 0, "drawing" },
	{"pal00_11.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal01_11.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal02_11.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal03_11.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal04_11.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal05_11.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal06_11.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal07_11.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal08_11.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal09_11.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal10_11.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal11_11.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"palett11.inf", kSaveModeSave, 0, "drawing palette" },
	{"plettr11.inf", kSaveModeSave, 0, "drawing in mailbox" },
	{"pgatod11.inf", kSaveModeSave, 0, "photo of cake" },
	{"pfleur11.inf", kSaveModeSave, 0, "photo of flowers" },
	{"mfleur11.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mfleud11.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mtable11.inf", kSaveModeSave, 0, "poster" },
	// Child12
	{"cart0112.inf", kSaveModeSave, 0, "drawing" },
	{"cart0212.inf", kSaveModeSave, 0, "drawing" },
	{"cart0312.inf", kSaveModeSave, 0, "drawing" },
	{"cart0412.inf", kSaveModeSave, 0, "drawing" },
	{"cart0512.inf", kSaveModeSave, 0, "drawing" },
	{"cart0612.inf", kSaveModeSave, 0, "drawing" },
	{"cart0712.inf", kSaveModeSave, 0, "drawing" },
	{"cart0812.inf", kSaveModeSave, 0, "drawing" },
	{"cart0912.inf", kSaveModeSave, 0, "drawing" },
	{"cart1012.inf", kSaveModeSave, 0, "drawing" },
	{"cart1112.inf", kSaveModeSave, 0, "drawing" },
	{"cart1212.inf", kSaveModeSave, 0, "drawing" },
	{"pal00_12.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal01_12.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal02_12.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal03_12.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal04_12.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal05_12.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal06_12.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal07_12.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal08_12.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal09_12.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal10_12.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal11_12.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"palett12.inf", kSaveModeSave, 0, "drawing palette" },
	{"plettr12.inf", kSaveModeSave, 0, "drawing in mailbox" },
	{"pgatod12.inf", kSaveModeSave, 0, "photo of cake" },
	{"pfleur12.inf", kSaveModeSave, 0, "photo of flowers" },
	{"mfleur12.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mfleud12.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mtable12.inf", kSaveModeSave, 0, "poster" },
	// Child13
	{"cart0113.inf", kSaveModeSave, 0, "drawing" },
	{"cart0213.inf", kSaveModeSave, 0, "drawing" },
	{"cart0313.inf", kSaveModeSave, 0, "drawing" },
	{"cart0413.inf", kSaveModeSave, 0, "drawing" },
	{"cart0513.inf", kSaveModeSave, 0, "drawing" },
	{"cart0613.inf", kSaveModeSave, 0, "drawing" },
	{"cart0713.inf", kSaveModeSave, 0, "drawing" },
	{"cart0813.inf", kSaveModeSave, 0, "drawing" },
	{"cart0913.inf", kSaveModeSave, 0, "drawing" },
	{"cart1013.inf", kSaveModeSave, 0, "drawing" },
	{"cart1113.inf", kSaveModeSave, 0, "drawing" },
	{"cart1213.inf", kSaveModeSave, 0, "drawing" },
	{"pal00_13.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal01_13.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal02_13.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal03_13.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal04_13.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal05_13.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal06_13.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal07_13.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal08_13.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal09_13.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal10_13.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal11_13.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"palett13.inf", kSaveModeSave, 0, "drawing palette" },
	{"plettr13.inf", kSaveModeSave, 0, "drawing in mailbox" },
	{"pgatod13.inf", kSaveModeSave, 0, "photo of cake" },
	{"pfleur13.inf", kSaveModeSave, 0, "photo of flowers" },
	{"mfleur13.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mfleud13.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mtable13.inf", kSaveModeSave, 0, "poster" },
	// Child14
	{"cart0114.inf", kSaveModeSave, 0, "drawing" },
	{"cart0214.inf", kSaveModeSave, 0, "drawing" },
	{"cart0314.inf", kSaveModeSave, 0, "drawing" },
	{"cart0414.inf", kSaveModeSave, 0, "drawing" },
	{"cart0514.inf", kSaveModeSave, 0, "drawing" },
	{"cart0614.inf", kSaveModeSave, 0, "drawing" },
	{"cart0714.inf", kSaveModeSave, 0, "drawing" },
	{"cart0814.inf", kSaveModeSave, 0, "drawing" },
	{"cart0914.inf", kSaveModeSave, 0, "drawing" },
	{"cart1014.inf", kSaveModeSave, 0, "drawing" },
	{"cart1114.inf", kSaveModeSave, 0, "drawing" },
	{"cart1214.inf", kSaveModeSave, 0, "drawing" },
	{"pal00_14.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal01_14.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal02_14.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal03_14.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal04_14.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal05_14.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal06_14.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal07_14.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal08_14.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal09_14.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal10_14.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal11_14.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"palett14.inf", kSaveModeSave, 0, "drawing palette" },
	{"plettr14.inf", kSaveModeSave, 0, "drawing in mailbox" },
	{"pgatod14.inf", kSaveModeSave, 0, "photo of cake" },
	{"pfleur14.inf", kSaveModeSave, 0, "photo of flowers" },
	{"mfleur14.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mfleud14.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mtable14.inf", kSaveModeSave, 0, "poster" },
	// Child15
	{"cart0115.inf", kSaveModeSave, 0, "drawing" },
	{"cart0215.inf", kSaveModeSave, 0, "drawing" },
	{"cart0315.inf", kSaveModeSave, 0, "drawing" },
	{"cart0415.inf", kSaveModeSave, 0, "drawing" },
	{"cart0515.inf", kSaveModeSave, 0, "drawing" },
	{"cart0615.inf", kSaveModeSave, 0, "drawing" },
	{"cart0715.inf", kSaveModeSave, 0, "drawing" },
	{"cart0815.inf", kSaveModeSave, 0, "drawing" },
	{"cart0915.inf", kSaveModeSave, 0, "drawing" },
	{"cart1015.inf", kSaveModeSave, 0, "drawing" },
	{"cart1115.inf", kSaveModeSave, 0, "drawing" },
	{"cart1215.inf", kSaveModeSave, 0, "drawing" },
	{"pal00_15.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal01_15.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal02_15.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal03_15.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal04_15.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal05_15.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal06_15.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal07_15.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal08_15.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal09_15.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal10_15.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal11_15.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"palett15.inf", kSaveModeSave, 0, "drawing palette" },
	{"plettr15.inf", kSaveModeSave, 0, "drawing in mailbox" },
	{"pgatod15.inf", kSaveModeSave, 0, "photo of cake" },
	{"pfleur15.inf", kSaveModeSave, 0, "photo of flowers" },
	{"mfleur15.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mfleud15.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mtable15.inf", kSaveModeSave, 0, "poster" },
	// Child16
	{"cart0116.inf", kSaveModeSave, 0, "drawing" },
	{"cart0216.inf", kSaveModeSave, 0, "drawing" },
	{"cart0316.inf", kSaveModeSave, 0, "drawing" },
	{"cart0416.inf", kSaveModeSave, 0, "drawing" },
	{"cart0516.inf", kSaveModeSave, 0, "drawing" },
	{"cart0616.inf", kSaveModeSave, 0, "drawing" },
	{"cart0716.inf", kSaveModeSave, 0, "drawing" },
	{"cart0816.inf", kSaveModeSave, 0, "drawing" },
	{"cart0916.inf", kSaveModeSave, 0, "drawing" },
	{"cart1016.inf", kSaveModeSave, 0, "drawing" },
	{"cart1116.inf", kSaveModeSave, 0, "drawing" },
	{"cart1216.inf", kSaveModeSave, 0, "drawing" },
	{"pal00_16.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal01_16.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal02_16.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal03_16.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal04_16.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal05_16.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal06_16.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal07_16.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal08_16.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal09_16.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal10_16.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"pal11_16.inf", kSaveModeSave, 0, "drawing thumbnail" },
	{"palett16.inf", kSaveModeSave, 0, "drawing palette" },
	{"plettr16.inf", kSaveModeSave, 0, "drawing in mailbox" },
	{"pgatod16.inf", kSaveModeSave, 0, "photo of cake" },
	{"pfleur16.inf", kSaveModeSave, 0, "photo of flowers" },
	{"mfleur16.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mfleud16.inf", kSaveModeSave, 0, "photo of flowers (vase)" },
	{"mtable16.inf", kSaveModeSave, 0, "poster" },


    // Addy 4 Base
	{"config00.inf", kSaveModeSave, nullptr, nullptr        },
	{"statev00.inf", kSaveModeSave, nullptr, nullptr        },
	// Addy 4 Grundschule
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

bool SaveLoad_v7::SpriteHandler::load(int16 dataVar, int32 size, int32 offset)
{
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

bool SaveLoad_v7::SpriteHandler::save(int16 dataVar, int32 size, int32 offset)
{
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
	_saveFiles[index++].handler = _debilHandler    = new FakeFileHandler(_vm);
	_saveFiles[index++].handler = _configHandler   = new GameFileHandler(_vm, targetName, "cfg");

	for (uint32 i = 0; i < kChildrenCount; i++)
	{
		_saveFiles[index++].handler = _adibou2EnvHandler[i] = new GameFileHandler(_vm,
																				  targetName,
																				  Common::String::format("env%02d", i + 1));
	}

	for (uint32 i = 0; i < kChildrenCount; i++)
	{
		_saveFiles[index++].handler = _adibou2WeatherHandler[i] = new SpriteHandler(_vm,
																					targetName,
																					Common::String::format("weather_%02d", i + 1));
	}

	for (uint32 i = 0; i < kChildrenCount; i++)
	{
		_saveFiles[index++].handler = _adibou2BreakoutGameProgressHandler[i] = new GameFileHandler(_vm,
																								   targetName,
																								   Common::String::format("breakout_%02d", i + 1));
	}

	for (uint32 i = 0; i < kAdibou2NbrOfConstructionGameFiles; i++)
	{
		_saveFiles[index++].handler = _adibou2ConstructionGameTempFileHandler[i] = new FakeFileHandler(_vm);
	}

	for (uint32 i = 0; i < kChildrenCount; i++)
	{
		for (uint32 j = 0; j < kAdibou2NbrOfConstructionGameFiles; j++)
		{
			const char *fileName = (j == 0)?"construc":((j == 1)?"ptreco":"aide");
			_saveFiles[index++].handler = _adibou2ConstructionGameProgressHandler[i][j] = new GameFileHandler(_vm,
																											  targetName,
																											  Common::String::format("%s_%02d", fileName, i + 1));
		}
	}

	for (uint32 i = 0; i < kChildrenCount; i++)
	{
		for (uint32 j = 0; j < kAdibou2NbrOfApplications; j++)
		{
			Common::String ext = Common::String::format("gsa%02d_%02d", j + 1, i + 1);
			_saveFiles[index++].handler = _adibou2AppProgressHandler[i][j]  = new GameFileHandler(_vm,
																								 targetName,
																								 ext);
		}
	}

	for (uint32 i = 0; i < kChildrenCount; i++)
	{
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
	delete _debilHandler;
	delete _childrenHandler;

	for (uint32 i = 0; i < kChildrenCount; i++)
	{
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

	for (uint32 i = 0; i < kChildrenCount; i++)
	{
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
}

const SaveLoad_v7::SaveFile *SaveLoad_v7::getSaveFile(const char *fileName) const {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return nullptr;
}

SaveLoad_v7::SaveFile *SaveLoad_v7::getSaveFile(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
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

} // End of namespace Gob
