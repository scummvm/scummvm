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

#ifndef ULTIMA8_GAMES_GAMEINFO_H
#define ULTIMA8_GAMES_GAMEINFO_H

#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Ultima8 {

class IDataSource;

//! GameInfo contains detailed information about the game
struct GameInfo {
	GameInfo();

	Std::string _name;

	enum GameType {
		GAME_UNKNOWN = 0,
		GAME_U8,
		GAME_REMORSE,
		GAME_REGRET
	} _type;

	// Usecode coff variant
	enum GameUsecodeOffsetVariant {
		GAME_UC_DEFAULT, // Most versions of most games
		GAME_UC_ORIG,	 // Original (pre-patch) CD versions of Crusader games
		GAME_UC_DEMO,    // Crusader: No Remorse or No Regret Demos
		GAME_UC_REM_ES,  // Crusader: No Remorse Spanish
		GAME_UC_REM_FR,	 // Crusader: No Remorse French
		GAME_UC_REM_JA,  // Crusader: No Remorse Japanese
		GAME_UC_REG_DE   // Crusader: No Regret German
	} _ucOffVariant;

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
	} _language;

	uint8 _md5[16];

	char getLanguageFileLetter() const;
	char getLanguageUsecodeLetter() const;
	Std::string getLanguage() const;
	Std::string getGameTitle() const;
	Std::string getPrintableVersion() const;

	Std::string getPrintDetails() const;
	Std::string getPrintableMD5() const;

	bool match(GameInfo &other, bool ignoreMD5 = false) const;

	void save(Common::WriteStream *ws);
	bool load(Common::SeekableReadStream *rs, uint32 /* version */);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
