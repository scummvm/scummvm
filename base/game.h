/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef BASE_GAME_H
#define BASE_GAME_H

#include "common/str.h"
#include "common/array.h"
#include "common/hash-str.h"

struct PlainGameDescriptor {
	const char *gameid;
	const char *description;	// TODO: Rename this to "title" or so
};

class GameDescriptor : public Common::StringMap {
public:
	GameDescriptor() {}

	GameDescriptor(const PlainGameDescriptor &pgd) {
		this->operator []("gameid") = pgd.gameid;
		this->operator []("description") = pgd.description;
	}

	GameDescriptor(Common::String g, Common::String d, Common::Language l  = Common::UNK_LANG,
	             Common::Platform p = Common::kPlatformUnknown) {
		this->operator []("gameid") = g;
		this->operator []("description") = d;
		if (l != Common::UNK_LANG)
			this->operator []("language") = Common::getLanguageCode(l);
		if (p != Common::kPlatformUnknown)
			this->operator []("platform") = Common::getPlatformCode(p);
	}

	/**
	 * Update the description string by appending (LANG/PLATFORM/EXTRA) to it.
	 */
	void updateDesc(const char *extra = 0);

	Common::String &gameid() { return this->operator []("gameid"); }
	Common::String &description() { return this->operator []("description"); }
	Common::Language language() { return Common::parseLanguage(this->operator []("language")); }
	Common::Platform platform() { return Common::parsePlatform(this->operator []("platform")); }
};

/** List of games. */
class GameList : public Common::Array<GameDescriptor> {
public:
	GameList() {}
	GameList(const GameList &list) : Common::Array<GameDescriptor>(list) {}
	GameList(const PlainGameDescriptor *g) {
		while (g->gameid) {
			push_back(GameDescriptor(g->gameid, g->description));
			g++;
		}
	}
};



class Plugin;

namespace Base {

// TODO: Find a better place for this function.
GameDescriptor findGame(const Common::String &gameName, const Plugin **plugin = NULL);

} // End of namespace Base


#endif
