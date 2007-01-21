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

#include "base/game.h"
#include "base/plugins.h"

void GameDescriptor::updateDesc(const char *extra) {
	// TODO: The format used here (LANG/PLATFORM/EXTRA) is not set in stone.
	// We may want to change the order (PLATFORM/EXTRA/LANG, anybody?), or
	// the seperator (instead of '/' use ', ' or ' ').
	const bool hasCustomLanguage = (this->contains("language") && (this->language() != Common::UNK_LANG));
	const bool hasCustomPlatform = (this->contains("platform") && (this->platform() != Common::kPlatformUnknown));
	const bool hasExtraDesc = (extra && extra[0]);

	// Adapt the description string if custom platform/language is set.
	if (hasCustomLanguage || hasCustomPlatform || hasExtraDesc) {
		Common::String descr = this->description();

		descr += " (";
		if (hasCustomLanguage)
			descr += Common::getLanguageDescription(this->language());
		if (hasCustomPlatform) {
			if (hasCustomLanguage)
				descr += "/";
			descr += Common::getPlatformDescription(this->platform());
		}
		if (hasExtraDesc) {
			if (hasCustomPlatform || hasCustomLanguage)
				descr += "/";
			descr += extra;
		}
		descr += ")";
		this->operator []("description") = descr;
	}
}

namespace Base {

// TODO: Find a better place for this function.
GameDescriptor findGame(const Common::String &gameName, const Plugin **plugin) {
	// Find the GameDescriptor for this target
	const PluginList &plugins = PluginManager::instance().getPlugins();
	GameDescriptor result;

	if (plugin)
		*plugin = 0;

	PluginList::const_iterator iter = plugins.begin();
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		result = (*iter)->findGame(gameName.c_str());
		if (!result.gameid().empty()) {
			if (plugin)
				*plugin = *iter;
			break;
		}
	}
	return result;
}

} // End of namespace Base
