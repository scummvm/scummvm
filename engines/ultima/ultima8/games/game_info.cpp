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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/games/game_info.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/misc/util.h"

namespace Ultima {
namespace Ultima8 {

struct GameTypeDesc {
	const char *shortname;
	const char *longname;
};
struct GameLangDesc {
	char letter;
	char usecodeletter;
	const char *name;
};

// Keep order the same as the GameType enum!
static const GameTypeDesc gametypes[] = {
	{ "", "" },
	{ "ultima8", "Ultima VIII: Pagan" },
	{ "remorse", "Crusader: No Remorse" },
	{ "regret", "Crusader: No Regret" },
	{ "pentmenu", "Pentagram Menu" },
	{ 0, 0 }
};

// Keep order the same as the GameLanguage enum!
static const GameLangDesc gamelangs[] = {
	{ 0, 0, "unknown" },
	{ 'e', 'e', "English" },
	{ 'f', 'f', "French" },
	{ 'g', 'g', "German" },
	{ 'e', 'e', "Spanish" },
	{ 'e', 'j', "Japanese" },
	{ '\0', '\0', 0 }
};


GameInfo::GameInfo() : _type(GAME_UNKNOWN), version(0), _language(GAMELANG_UNKNOWN) {
	for (int i = 0; i < 16; ++i)
		_md5[i] = 0;
}

char GameInfo::getLanguageFileLetter() const {
	switch (_type) {
	case GAME_U8: {
		unsigned int l = static_cast<unsigned int>(_language);
		assert(l < (sizeof(gamelangs) / sizeof(gamelangs[0])) - 1);

		return gamelangs[l].letter;
	}
	case GAME_REMORSE:
	case GAME_REGRET:
		return 'e';
	default:
		return 0;
	}
}

char GameInfo::getLanguageUsecodeLetter() const {
	switch (_type) {
	case GAME_U8: {
		unsigned int l = static_cast<unsigned int>(_language);
		assert(l < (sizeof(gamelangs) / sizeof(gamelangs[0])) - 1);

		return gamelangs[l].usecodeletter;
	}
	case GAME_REMORSE:
	case GAME_REGRET:
		return 'e';
	default:
		return 0;
	}
}

Std::string GameInfo::getLanguage() const {
	unsigned int l = static_cast<unsigned int>(_language);
	assert(l < (sizeof(gamelangs) / sizeof(gamelangs[0])) - 1);

	return gamelangs[l].name;
}

Std::string GameInfo::getGameTitle() const {
	unsigned int t = static_cast<unsigned int>(_type);
	assert(t < (sizeof(gametypes) / sizeof(gametypes[0])) - 1);

	return gametypes[t].longname;
}

Std::string GameInfo::getPrintableVersion() const {
	char buf[32];
	sprintf(buf, "%d.%02d", version / 100, version % 100);
	return buf;
}

Std::string GameInfo::getPrintDetails() const {
	Std::string ret;

	Std::string title = getGameTitle();
	if (title == "") title = "Unknown";

	ret = title + ", ";

	Std::string lang = getLanguage();
	if (lang == "") lang = "Unknown";
	ret += lang;

	if (_type != GAME_PENTAGRAM_MENU) {
		// version, md5 don't make sense for the pentagram menu

		ret += ", version ";
		ret += getPrintableVersion();

		ret += ", md5 ";
		ret += getPrintableMD5();
	}

	return ret;
}


Std::string GameInfo::getPrintableMD5() const {
	Std::string ret;

	char buf[33];
	for (int i = 0; i < 16; ++i) {
		sprintf(buf + 2 * i, "%02x", _md5[i]);
	}

	ret = buf;

	return ret;
}

bool GameInfo::match(GameInfo &other, bool ignoreMD5) const {
	if (_type != other._type) return false;
	if (_language != other._language) return false;
	if (version != other.version) return false;

	if (ignoreMD5) return true;

	return (Std::memcmp(_md5, other._md5, 16) == 0);
}

void GameInfo::save(Common::WriteStream *ws) {
	unsigned int l = static_cast<unsigned int>(_language);
	assert(l < (sizeof(gamelangs) / sizeof(gamelangs[0])) - 1);
	unsigned int t = static_cast<unsigned int>(_type);
	assert(t < (sizeof(gametypes) / sizeof(gametypes[0])) - 1);

	Std::string game = gametypes[t].shortname;
	Std::string lang = gamelangs[l].name;

	char buf[16];
	sprintf(buf, "%d", version);
	Std::string ver = buf;
	Std::string md5Str = getPrintableMD5();

	Std::string d = game + "," + lang + "," + ver + "," + md5Str + "\n";
	ws->write(d.c_str(), d.size());
}

bool GameInfo::load(IDataSource *ids, uint32 ver) {
	Std::string s;
	Std::vector<Std::string> parts;

	ids->readline(s);
	SplitString(s, ',', parts);
	if (parts.size() != 4) return false;

	int i = 0;
	while (gametypes[i].shortname) {
		if (parts[0] == gametypes[i].shortname) {
			_type = static_cast<GameType>(i);
			break;
		}
		i++;
	}
	if (!gametypes[i].shortname) return false;

	i = 0;
	while (gamelangs[i].name) {
		if (parts[1] == gamelangs[i].name) {
			_language = static_cast<GameLanguage>(i);
			break;
		}
		i++;
	}
	if (!gamelangs[i].name) return false;

	this->version = Std::strtol(parts[2].c_str(), 0, 0);

	for (i = 0; i < 16; ++i) {
		char buf[3];
		buf[0] = parts[3][2 * i];
		buf[1] = parts[3][2 * i + 1];
		buf[2] = 0;
		long x = Std::strtol(buf, 0, 16);
		_md5[i] = static_cast<uint8>(x);
	}

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
