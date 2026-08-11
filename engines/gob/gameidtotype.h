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

#ifndef GOB_GAMEIDTOTYPE_H
#define GOB_GAMEIDTOTYPE_H

#include "gob/detection/detection.h"

namespace Gob {

struct GameIdToType {
	const char *gameId;
	GameType gameType;
};

static const GameIdToType gameIdToType[] = {
	{                    "gob1", kGameTypeGob1 },
	{                    "gob2", kGameTypeGob2 },
	{                    "gob3", kGameTypeGob3 },
	{                    "ween", kGameTypeWeen },
	{                  "bargon", kGameTypeBargon },
	{                "babayaga", kGameTypeBabaYaga },
	{             "abracadabra", kGameTypeAbracadabra },
	{            "englishfever", kGameTypeNone },
	{               "littlered", kGameTypeLittleRed },
	{                "onceupon", kGameTypeOnceUponATime },
	{                 "crousti", kGameTypeCrousti },
	{                     "lit", kGameTypeLostInTime },
	{                    "lit1", kGameTypeLostInTime },
	{                    "lit2", kGameTypeLostInTime },
	{          "nathanvacances", kGameTypeNone },
	{                   "inca2", kGameTypeInca2 },
	{                "woodruff", kGameTypeWoodruff },
	{                 "dynasty", kGameTypeDynasty },
	{             "dynastywood", kGameTypeDynastyWood },
	{                   "urban", kGameTypeUrban },
	{              "playtoons1", kGameTypePlaytoons },
	{              "playtoons2", kGameTypePlaytoons },
	{              "playtoons3", kGameTypePlaytoons },
	{              "playtoons4", kGameTypePlaytoons },
	{              "playtoons5", kGameTypePlaytoons },
	{               "playtnck1", kGameTypePlaytoons },
	{               "playtnck2", kGameTypePlaytoons },
	{               "playtnck3", kGameTypePlaytoons },
	{           "playtoonsdemo", kGameTypePlaytoons },
	{         "pierresmagiques", kGameTypeNone },
	{                  "bambou", kGameTypeBambou },
	{             "fascination", kGameTypeFascination },
	{                  "geisha", kGameTypeGeisha },
	{                    "adi1", kGameTypeAdi1 },
	{                    "adi2", kGameTypeAdi2 },
	{                    "adi4", kGameTypeAdi4 },
	{                    "adi5", kGameTypeNone },
	{                 "adibou1", kGameTypeAdibou1 },
	{                 "adibou2", kGameTypeAdibou2 },
	{                 "adibou3", kGameTypeNone },
	{           "adiboucuisine", kGameTypeNone },
	{            "adiboudessin", kGameTypeNone },
	{             "adiboumagie", kGameTypeNone },
	{          "adiboudchoumer", kGameTypeNone },
	{     "adiboudchoubanquise", kGameTypeNone },
	{     "adiboudchoucampagne", kGameTypeNone },
	{ "adiboudchoujunglesavane", kGameTypeNone },
	{                   nullptr, kGameTypeNone }
};

} // End of namespace Gob

#endif // GOB_GAMEIDTOTYPE_H
