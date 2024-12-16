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

#ifndef GOB_DETECTION_TABLES_H
#define GOB_DETECTION_TABLES_H

// Struct "GOBGameDescription"
#include "gob/detection/detection.h"

using namespace Common;

// Game IDs and proper names
static const PlainGameDescriptor gobGames[] = {
	{"gob1", "Gobliiins"},
	{"gob2", "Gobliins 2"},
	{"gob3", "Goblins Quest 3"},
	{"ween", "Ween: The Prophecy"},
	{"bargon", "Bargon Attack"},
	{"babayaga", "Once Upon A Time: Baba Yaga"},
	{"abracadabra", "Once Upon A Time: Abracadabra"},
	{"englishfever", "English Fever"},
	{"littlered", "Once Upon A Time: Little Red Riding Hood"},
	{"onceupon", "Once Upon A Time"},
	{"crousti", "Croustibat"},
	{"lit", "Lost in Time"},
	{"lit1", "Lost in Time Part 1"},
	{"lit2", "Lost in Time Part 2"},
	{"nathanvacances", "Nathan Vacances"},
	{"inca2", "Inca II: Wiracocha"},
	{"woodruff", "The Bizarre Adventures of Woodruff and the Schnibble"},
	{"dynasty", "The Last Dynasty"},
	{"dynastywood", "Woodruff and The Last Dynasty"},
	{"urban", "Urban Runner"},
	{"playtoons1", "Playtoons 1 - Uncle Archibald"},
	{"playtoons2", "Playtoons 2 - The Case of the Counterfeit Collaborator"},
	{"playtoons3", "Playtoons 3 - The Secret of the Castle"},
	{"playtoons4", "Playtoons 4 - The Mandarine Prince"},
	{"playtoons5", "Playtoons 5 - The Stone of Wakan"},
	{"playtnck1", "Playtoons Construction Kit 1 - Monsters"},
	{"playtnck2", "Playtoons Construction Kit 2 - Knights"},
	{"playtnck3", "Playtoons Construction Kit 3 - Far West"},
	{"playtoonsdemo", "Playtoons Demo"},
	{"magicstones", "The Land of the Magic Stones"},
	{"bambou", "Playtoons Limited Edition - Bambou le sauveur de la jungle"},
	{"fascination", "Fascination"},
	{"geisha", "Geisha"},
	{"adi1", "ADI 1"},
	{"adi2", "ADI 2"},
	{"adi4", "ADI 4"},
	{"adi5", "ADI 5"},
	{"adibou1", "Adibou 1"},
	{"adibou2", "Adibou 2"},
	{"adibou3", "Adibou 3"},
	{"adiboucuisine", "Adibou présente Cuisine"},
	{"adiboudessin", "Adibou présente Dessin"},
	{"adiboudchoumer", "Adiboud'chou a la mer"},
	{"adiboudchoubanquise", "Adiboud'chou sur la banquise"},
	{"adiboudchoucampagne", "Adiboud'chou a la campagne"},
	{"adiboudchoujunglesavane", "Adiboud'chou dans la jungle et la savane"},
	{0, 0}
};

namespace Gob {

// Detection tables
static const GOBGameDescription gameDescriptions[] = {
	#include "gob/detection/tables_gob1.h"      // Gobliiins
	#include "gob/detection/tables_gob2.h"      // Gobliins 2: The Prince Buffoon
	#include "gob/detection/tables_gob3.h"      // Goblins 3 / Goblins Quest 3
	#include "gob/detection/tables_ween.h"      // Ween: The Prophecy
	#include "gob/detection/tables_bargon.h"    // Bargon Attack
	#include "gob/detection/tables_littlered.h" // Once Upon A Time: Little Red Riding Hood
	#include "gob/detection/tables_onceupon.h"  // Once Upon A Time: Baba Yaga and Abracadabra
	#include "gob/detection/tables_lit.h"       // Lost in Time
	#include "gob/detection/tables_nathanvacances.h" // Nathan Vacances series
	#include "gob/detection/tables_fascin.h"    // Fascination
	#include "gob/detection/tables_geisha.h"    // Geisha
	#include "gob/detection/tables_inca2.h"     // Inca II: Wiracocha
	#include "gob/detection/tables_woodruff.h"  // (The Bizarre Adventures of) Woodruff and the Schnibble (of Azimuth)
	#include "gob/detection/tables_dynasty.h"   // The Last Dynasty
	#include "gob/detection/tables_urban.h"     // Urban Runner
	#include "gob/detection/tables_playtoons.h" // The Playtoons series
	#include "gob/detection/tables_magicstones.h" // Le pays des Pierres Magiques / The Land of the Magic Stones
	#include "gob/detection/tables_englishfever.h" // English Fever
	#include "gob/detection/tables_adi1.h"      // The ADI 1 series
	#include "gob/detection/tables_adi2.h"      // The ADI 2 series
	#include "gob/detection/tables_adi4.h"      // The ADI / Addy 4 series
	#include "gob/detection/tables_adi5.h"      // The ADI / Addy 5 series
	#include "gob/detection/tables_adibou1.h"   // Adibou 1 / A.J.'s World of Discovery / ADI Jr.
	#include "gob/detection/tables_adibou2.h"   // The Adibou 2 / Addy Junior series
	#include "gob/detection/tables_adibou3.h"	// Adibou 3 / Adiboo 3 series
	#include "gob/detection/tables_adiboupresente.h"	// Adibou présente series
	#include "gob/detection/tables_adiboudchou.h"		// Adiboud'chou / Addy Buschu series
	#include "gob/detection/tables_crousti.h"   // Croustibat

	{ AD_TABLE_END_MARKER, kFeaturesNone, 0, 0, 0}
};

// File-based fallback tables
#include "gob/detection/tables_fallback.h"
}

#endif // GOB_DETECTION_TABLES_H
