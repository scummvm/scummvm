/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/game.h"
#include "base/plugins.h"


const PlainGameDescriptor *findPlainGameDescriptor(const char *gameid, const PlainGameDescriptor *list) {
	const PlainGameDescriptor *g = list;
	while (g->gameid) {
		if (0 == strcasecmp(gameid, g->gameid))
			return g;
		g++;
	}
	return 0;
}

GameDescriptor::GameDescriptor() {
	setVal("gameid", "");
	setVal("description", "");
}

GameDescriptor::GameDescriptor(const PlainGameDescriptor &pgd) {
	setVal("gameid", pgd.gameid);
	setVal("description", pgd.description);
}

GameDescriptor::GameDescriptor(const PlainGameDescriptorGUIOpts &pgd) {
	setVal("gameid", pgd.gameid);
	setVal("description", pgd.description);

	if (pgd.guioptions != 0)
		setVal("guioptions", Common::getGameGUIOptionsDescription(pgd.guioptions));
}

GameDescriptor::GameDescriptor(const Common::String &g, const Common::String &d, Common::Language l, Common::Platform p, uint32 guioptions) {
	setVal("gameid", g);
	setVal("description", d);
	if (l != Common::UNK_LANG)
		setVal("language", Common::getLanguageCode(l));
	if (p != Common::kPlatformUnknown)
		setVal("platform", Common::getPlatformCode(p));
	if (guioptions != 0)
		setVal("guioptions", Common::getGameGUIOptionsDescription(guioptions));
}

void GameDescriptor::setGUIOptions(uint32 guioptions) {
	if (guioptions != 0)
		setVal("guioptions", Common::getGameGUIOptionsDescription(guioptions));
	else
		erase("guioptions");
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

bool SaveStateDescriptor::getBool(const Common::String &key) const {
	if (contains(key)) {
		Common::String value = getVal(key);
		if (value.equalsIgnoreCase("true") ||
			value.equalsIgnoreCase("yes") ||
			value.equals("1"))
			return true;
		if (value.equalsIgnoreCase("false") ||
			value.equalsIgnoreCase("no") ||
			value.equals("0"))
			return false;
		error("SaveStateDescriptor: %s '%s' has unknown value '%s' for boolean '%s'",
				save_slot().c_str(), description().c_str(), value.c_str(), key.c_str());
	}
	return false;
}

void SaveStateDescriptor::setDeletableFlag(bool state) {
	setVal("is_deletable", state ? "true" : "false");
}

void SaveStateDescriptor::setWriteProtectedFlag(bool state) {
	setVal("is_write_protected", state ? "true" : "false");
}

void SaveStateDescriptor::setSaveDate(int year, int month, int day) {
	char buffer[32];
	snprintf(buffer, 32, "%.2d.%.2d.%.4d", day, month, year);
	setVal("save_date", buffer);
}

void SaveStateDescriptor::setSaveTime(int hour, int min) {
	char buffer[32];
	snprintf(buffer, 32, "%.2d:%.2d", hour, min);
	setVal("save_time", buffer);
}

void SaveStateDescriptor::setPlayTime(int hours, int minutes) {
	char buffer[32];
	snprintf(buffer, 32, "%.2d:%.2d", hours, minutes);
	setVal("play_time", buffer);
}

