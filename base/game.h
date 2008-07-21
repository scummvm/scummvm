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

namespace Graphics {
	struct Surface;
}

/**
 * A simple structure used to map gameids (like "monkey", "sword1", ...) to
 * nice human readable and descriptive game titles (like "The Secret of Monkey Island").
 * This is a plain struct to make it possible to declare NULL-terminated C arrays
 * consisting of PlainGameDescriptors.
 */
struct PlainGameDescriptor {
	const char *gameid;
	const char *description;
};

/**
 * Given a list of PlainGameDescriptors, returns the first PlainGameDescriptor
 * matching the given gameid. If not match is found return 0.
 * The end of the list must marked by a PlainGameDescriptor with gameid equal to 0.
 */
const PlainGameDescriptor *findPlainGameDescriptor(const char *gameid, const PlainGameDescriptor *list);

/**
 * A hashmap describing details about a given game. In a sense this is a refined
 * version of PlainGameDescriptor, as it also contains a gameid and a description string.
 * But in addition, platform and language settings, as well as arbitrary other settings,
 * can be contained in a GameDescriptor.
 * This is an essential part of the glue between the game engines and the launcher code.
 */
class GameDescriptor : public Common::StringMap {
public:
	GameDescriptor() {
		setVal("gameid", "");
		setVal("description", "");
	}

	GameDescriptor(const PlainGameDescriptor &pgd) {
		setVal("gameid", pgd.gameid);
		setVal("description", pgd.description);
	}

	GameDescriptor(const Common::String &g, const Common::String &d, Common::Language l  = Common::UNK_LANG,
	             Common::Platform p = Common::kPlatformUnknown) {
		setVal("gameid", g);
		setVal("description", d);
		if (l != Common::UNK_LANG)
			setVal("language", Common::getLanguageCode(l));
		if (p != Common::kPlatformUnknown)
			setVal("platform", Common::getPlatformCode(p));
	}

	/**
	 * Update the description string by appending (LANG/PLATFORM/EXTRA) to it.
	 */
	void updateDesc(const char *extra = 0);

	Common::String &gameid() { return getVal("gameid"); }
	Common::String &description() { return getVal("description"); }
	const Common::String &gameid() const { return getVal("gameid"); }
	const Common::String &description() const { return getVal("description"); }
	Common::Language language() const { return contains("language") ? Common::parseLanguage(getVal("language")) : Common::UNK_LANG; }
	Common::Platform platform() const { return contains("platform") ? Common::parsePlatform(getVal("platform")) : Common::kPlatformUnknown; }
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

/**
 * A hashmap describing details about a given save state.
 * TODO
 * Guaranteed to contain save_slot, filename and description values.
 * Additional ideas: Playtime, creation date, thumbnail, ...
 */
class SaveStateDescriptor : public Common::StringMap {
protected:
	Graphics::Surface	*_thumbnail;	// can be NULL
public:
	SaveStateDescriptor() : _thumbnail(0) {
		setVal("save_slot", "-1");	// FIXME: default to 0 (first slot) or to -1 (invalid slot) ?
		setVal("description", "");
		setVal("filename", "");
	}

	SaveStateDescriptor(int s, const Common::String &d, const Common::String &f) : _thumbnail(0) {
		char buf[16];
		sprintf(buf, "%d", s);
		setVal("save_slot", buf);
		setVal("description", d);
		setVal("filename", f);
	}

	SaveStateDescriptor(const Common::String &s, const Common::String &d, const Common::String &f) : _thumbnail(0) {
		setVal("save_slot", s);
		setVal("description", d);
		setVal("filename", f);
	}

	~SaveStateDescriptor() {
		setThumbnail(0);
	}

	/** The saveslot id, as it would be passed to the "-x" command line switch. */
	Common::String &save_slot() { return getVal("save_slot"); }

	/** The saveslot id, as it would be passed to the "-x" command line switch (read-only variant). */
	const Common::String &save_slot() const { return getVal("save_slot"); }

	/** A human readable description of the save state. */
	Common::String &description() { return getVal("description"); }

	/** A human readable description of the save state (read-only variant). */
	const Common::String &description() const { return getVal("description"); }

	/** The filename of the savestate, for use with the SaveFileManager API. */
	Common::String &filename() { return getVal("filename"); }

	/** The filename of the savestate, for use with the SaveFileManager API (read-only variant). */
	const Common::String &filename() const { return getVal("filename"); }

	/**
	 * Return a thumbnail graphics surface representing the savestate visually
	 * This is usually a scaled down version of the game graphics. The size
	 * should be either 160x100 or 160x120 pixels, depending on the aspect
	 * ratio of the game. If another ratio is required, contact the core team.
	 *
	 * TODO: it is probably a bad idea to read this for *all* games at once,
	 * at least on low-end devices. So this info should probably normally only
	 * be included optionally. I.e. only upon a query for a specific savegame...
	 * To this end, add a getFullSaveStateInfo(target, slot) to the plugin API.
	 */
	const Graphics::Surface *getThumbnail() const { return _thumbnail; }

	
	void setThumbnail(Graphics::Surface *t);
};

/** List of savestates. */
typedef Common::Array<SaveStateDescriptor> SaveStateList;

#endif
