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
 */

#include "engines/game.h"


const PlainGameDescriptor *findPlainGameDescriptor(const char *gameid, const PlainGameDescriptor *list) {
	const PlainGameDescriptor *g = list;
	while (g->gameid) {
		if (0 == scumm_stricmp(gameid, g->gameid))
			return g;
		g++;
	}
	return 0;
}

GameDescriptor::GameDescriptor() {
	setVal("gameid", "");
	setVal("description", "");
}

GameDescriptor::GameDescriptor(const PlainGameDescriptor &pgd, uint32 guioptions) {
	setVal("gameid", pgd.gameid);
	setVal("description", pgd.description);

	if (guioptions != 0)
		setVal("guioptions", Common::getGameGUIOptionsDescription(guioptions));
}

GameDescriptor::GameDescriptor(const Common::String &g, const Common::String &d, Common::Language l, Common::Platform p, uint32 guioptions, WIPLevel wipLevel) {
	setVal("gameid", g);
	setVal("description", d);
	if (l != Common::UNK_LANG)
		setVal("language", Common::getLanguageCode(l));
	if (p != Common::kPlatformUnknown)
		setVal("platform", Common::getPlatformCode(p));
	if (guioptions != 0)
		setVal("guioptions", Common::getGameGUIOptionsDescription(guioptions));

	setWIPLevel(wipLevel);
}

void GameDescriptor::setGUIOptions(uint32 guioptions) {
	if (guioptions != 0)
		setVal("guioptions", Common::getGameGUIOptionsDescription(guioptions));
	else
		erase("guioptions");
}

void GameDescriptor::appendGUIOptions(const Common::String &str) {
	setVal("guioptions", getVal("guioptions", "") + " " + str);
}

void GameDescriptor::updateDesc(const char *extra) {
	// TODO: The format used here (LANG/PLATFORM/EXTRA) is not set in stone.
	// We may want to change the order (PLATFORM/EXTRA/LANG, anybody?), or
	// the seperator (instead of '/' use ', ' or ' ').
	const bool hasCustomLanguage = (language() != Common::UNK_LANG);
	const bool hasCustomPlatform = (platform() != Common::kPlatformUnknown);
	const bool hasExtraDesc = (extra && extra[0]);

	// Adapt the description string if custom platform/language is set.
	if (hasCustomLanguage || hasCustomPlatform || hasExtraDesc) {
		Common::String descr = description();

		descr += " (";
		if (hasExtraDesc)
			descr += extra;
		if (hasCustomPlatform) {
			if (hasExtraDesc)
				descr += "/";
			descr += Common::getPlatformDescription(platform());
		}
		if (hasCustomLanguage) {
			if (hasExtraDesc || hasCustomPlatform)
				descr += "/";
			descr += Common::getLanguageDescription(language());
		}
		descr += ")";
		setVal("description", descr);
	}
}

WIPLevel GameDescriptor::getWIPLevel() {
	WIPLevel wipLevel = WIP_STABLE;
	if (contains("wiplevel")) {
		Common::String wipLevelString = getVal("wiplevel");
		if (wipLevelString.equals("unstable"))
			wipLevel = WIP_UNSTABLE;
		else if (wipLevelString.equals("testing"))
			wipLevel = WIP_TESTING;
	}
	return wipLevel;
}

void GameDescriptor::setWIPLevel(WIPLevel wipLevel) {
	switch (wipLevel) {
	case WIP_UNSTABLE:
		setVal("wiplevel", "unstable");
		break;
	case WIP_TESTING:
		setVal("wiplevel", "testing");
		break;
	case WIP_STABLE:
		// Fall Through intended
	default:
		erase("wiplevel");
	}
}
