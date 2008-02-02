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
 * $URL$
 * $Id$
 */

#ifndef ENGINES_METAENGINE_H
#define ENGINES_METAENGINE_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/error.h"

#include "base/game.h"

class Engine;
class OSystem;

/**
 * A meta engine is essentially a factory for Engine instances with the
 * added ability of listing and detecting supported games.
 * Every engine "plugin" provides a hook to get an instance of a MetaEngine
 * subclass for that "engine plugin". E.g. SCUMM povides ScummMetaEngine.
 * This is then in turn used by the frontend code to detect games,
 * and instantiate actual Engine objects.
 */
class MetaEngine {
public:
	virtual ~MetaEngine() {}

	virtual const char *getName() const = 0;
	virtual const char *getCopyright() const = 0;
//	virtual int getVersion() const = 0;	// TODO!

	virtual GameList getSupportedGames() const = 0;
	virtual GameDescriptor findGame(const char *gameid) const = 0;
	virtual GameList detectGames(const FSList &fslist) const = 0;

	virtual PluginError createInstance(OSystem *syst, Engine **engine) const = 0;
};


/**
 * The META_COMPATIBLITY_WRAPPER macro is there to ease the transition from the
 * old plugin API to the new MetaEngine class. Ultimately, this macro will go
 * and REGISTER_PLUGIN will be changedd to simply take an ID and a METACLASS.
 * Until then, use META_COMPATIBLITY_WRAPPER + REGISTER_PLUGIN.
 */
#define META_COMPATIBLITY_WRAPPER(ID,METACLASS) \
	static MetaEngine &getMetaEngine() { \
		static MetaEngine *meta = 0; \
		if (!meta) meta = new METACLASS(); \
		return *meta; \
	} \
	GameList Engine_##ID##_gameIDList() { return getMetaEngine().getSupportedGames(); } \
	GameDescriptor Engine_##ID##_findGameID(const char *gameid) { return getMetaEngine().findGame(gameid); } \
	PluginError Engine_##ID##_create(OSystem *syst, Engine **engine) { return getMetaEngine().createInstance(syst, engine); } \
	GameList Engine_##ID##_detectGames(const FSList &fslist) { return getMetaEngine().detectGames(fslist); } \
	void dummyFuncToAllowTrailingSemicolon()



#endif
