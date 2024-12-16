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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

/* Detection tables for Le pays des Pierres Magiques / The Land of the Magic Stones / Das Zauberland der Zaubersteine */
/* This Game uses the DEV7 Engine, more Information can be found here: https://wiki.scummvm.org/index.php?title=DEV7_Information */

#ifndef GOB_DETECTION_TABLES_MAGICSTONES_H
#define GOB_DETECTION_TABLES_MAGICSTONES_H

// -- French: Le pays des Pierres Magiques --

{
	{ // Added by Strangerke in 2009

		"magicstones",
		MetaEngineDetection::GAME_NOT_IMPLEMENTED, // Le pays des Pierres Magiques (Engine: DEV7 version 1.2.0.0)
		AD_ENTRY2s("ed4.stk", "98721a7cfdc5a358d7ac56b7c6d3ba3d", 541882,
			   		"ed4cd.itk", "0627a91d9a6f4772c33747ce752024c2", 606993908),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	"ed4.stk","main.obc",0
},

// -- German: Das Zauberland der Zaubersteine --

{
	{

		"magicstones",
		MetaEngineDetection::GAME_NOT_IMPLEMENTED, // Die Suche nach dem Feuerdrachen (Engine: DEV7 version 1.2.0.0)
		AD_ENTRY2s("ed4.stk", "805ceadf60b9446e06078ba9cb7f75ee", 542754,
			   		"ed4_cd.itk", "02de8ac4f12ed0e4cf5577683f443dcb", 599645278),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	"ed4.stk","main.obc",0
},

#endif // GOB_DETECTION_TABLES_MAGICSTONES_H
