/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA8_GAMES_GAMEINFO_H
#define ULTIMA8_GAMES_GAMEINFO_H

#include "ultima/ultima8/misc/istring.h"

namespace Ultima8 {

class IDataSource;
class ODataSource;

//! GameInfo contains detailed information about the game
struct GameInfo {
	GameInfo();

	Pentagram::istring name;

	enum GameType {
		GAME_UNKNOWN = 0,
		GAME_U8,
		GAME_REMORSE,
		GAME_REGRET,
		GAME_PENTAGRAM_MENU
	} type;

	//! version number, encoded as 100*major + minor
	//! so, 2.12 becomes 212
	//! 0 = unknown
	int version;

	enum GameLanguage {
		GAMELANG_UNKNOWN = 0,
		GAMELANG_ENGLISH,
		GAMELANG_FRENCH,
		GAMELANG_GERMAN,
		GAMELANG_SPANISH,
		GAMELANG_JAPANESE
	} language;

	uint8 md5[16];

	char getLanguageFileLetter() const;
	char getLanguageUsecodeLetter() const;
	std::string getLanguage() const;
	std::string getGameTitle() const;
	std::string getPrintableVersion() const;

	std::string getPrintDetails() const;
	std::string getPrintableMD5() const;

	bool match(GameInfo &other, bool ignoreMD5 = false) const;

	void save(ODataSource *ods);
	bool load(IDataSource *ids, uint32 /* version */);
};

} // End of namespace Ultima8

#endif
