/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "common/file.h"
#include "common/str.h"
#include "common/endian.h"
#include "common/tokenizer.h"

#include "engines/grim/localize.h"
#include "engines/grim/grim.h"
#include "engines/grim/resource.h"

namespace Grim {

Localizer *g_localizer = nullptr;

Localizer::Localizer() {
	// To avoid too wide lines further below, we just name these here.
	bool isAnyDemo = g_grim->getGameFlags() & ADGF_DEMO;
	bool isGrimDemo = g_grim->getGameType() == GType_GRIM && isAnyDemo;
	bool isGerman = g_grim->getGameLanguage() == Common::DE_DEU;
	bool isFrench = g_grim->getGameLanguage() == Common::FR_FRA;
	bool isItalian = g_grim->getGameLanguage() == Common::IT_ITA;
	bool isSpanish = g_grim->getGameLanguage() == Common::ES_ESP;
	bool isTranslatedGrimDemo = (isGerman || isFrench || isItalian || isSpanish) && isGrimDemo;
	bool isPS2 = g_grim->getGamePlatform() == Common::kPlatformPS2;
	bool isRemastered = g_grim->getGameFlags() & ADGF_REMASTERED; // TODO: Add handling of this from g_grim.

	if (isGrimDemo && !isTranslatedGrimDemo)
		return;

	Common::String filename;
	if (g_grim->getGameType() == GType_MONKEY4) {
		filename = "script.tab";
	} else {
		if (isRemastered) {
			filename = Common::String("grim.") + g_grim->getLanguagePrefix() + Common::String(".tab"); // TODO: Detect based on language.
		} else if (isTranslatedGrimDemo) {
			filename = "language.tab";
		} else {
			filename = "grim.tab";
		}
	}

	Common::SeekableReadStream *f = g_resourceloader->openNewStreamFile(filename);
	if (!f) {
		error("Localizer::Localizer: Unable to find localization information (%s)", filename.c_str());
		return;
	}

	int32 filesize = f->size();

	// Read in the data
	char *data = new char[filesize + 1];
	f->read(data, filesize);
	data[filesize] = '\0';
	delete f;

	if (isRemastered) {
		parseRemasteredData(Common::String(data));
		return;
	}

	// Explicitly white-list german demo, as it has a .tab-file
	if ((isTranslatedGrimDemo) || (!isAnyDemo && !isPS2)) {
		if (filesize < 4)
			error("%s to short: %i", filename.c_str(), filesize);
		switch (READ_BE_UINT32(data)) {
		case MKTAG('R','C','N','E'):
			// Decode the data
			if (g_grim->getGameType() == GType_MONKEY4) {
				uint32 next = 0x16;
				for (int i = 4; i < filesize; i++) {
					next = next * 0x343FD + 0x269EC3;
					data[i] ^= (int)(((((next >> 16) & 0x7FFF) / 32767.f) * 254) + 1);
				}
			} else {
				for (int i = 4; i < filesize; i++) {
					data[i] ^= '\xdd';
				}
			}
		case MKTAG('D', 'O', 'E', 'L'):
		case MKTAG('a', 'r', 't', 'p'):
			break;
		default:
			error("Invalid magic reading %s: %08x (%s)", filename.c_str(), READ_BE_UINT32(data), tag2str(READ_BE_UINT32(data)));
		}
	}

	char *nextline = data;
	Common::String last_entry;
	//Read file till end
	for (char *line = data + 4; line - data <= filesize; line = nextline + 1) {
		if (line == nullptr || nextline == nullptr) {
			break;
		}

		nextline = strchr(line, '\n');
		//if there is no next line we arrived the last one
		if (nextline == nullptr) {
			nextline = strchr(line, '\0');
		}

		//in grim we have to exit on first empty line else skip line
		if (*line == '\r') {
			if (g_grim->getGameType() == GType_GRIM) {
				break;
			}

			nextline = strchr(line + 2, '\n');
			continue;
		}

		//EMI has an garbage line which should be ignored
		if (g_grim->getGameType() == GType_MONKEY4 && *line == '\x1A')
			continue;

		char *tab = strchr(line, '\t');
		//skip line if no tab found
		if (tab == nullptr) {
			continue;
		}

		if (tab > nextline) {
			Common::String cont = Common::String(line, nextline - line - 1);
			assert(last_entry != "");
			warning("Continuation line: \"%s\" = \"%s\" + \"%s\"", last_entry.c_str(), _entries[last_entry].c_str(), cont.c_str());
			_entries[last_entry] += cont;
		} else {
			_entries[last_entry = Common::String(line, tab - line)] = Common::String(tab + 1, (nextline - tab - 2));
		}
	}
	delete[] data;
}

void Localizer::parseRemasteredData(const Common::String &data) {
	// This is probably cleaner implemented using a read line-by-line, but for now this works.
	Common::StringTokenizer tokens(data, "\t\n");
	while (!tokens.empty()) {
		Common::String key = tokens.nextToken();
		// Not sure if this is right, but it is necessary to get by the second line
		key.trim();
		// Handle comments
		if (!(key.size() > 0 && !(key[0] == '#'))) {
			continue;
		}
		Common::String string = tokens.nextToken();
		_entries[key] = string;
	}
}

Common::String Localizer::localize(const char *str) const {
	assert(str);

	const char *slash2;

	if (str[0] != '/' || str[0] == 0 || !(slash2 = strchr(str + 1, '/')))
		return str;

	Common::String key(str + 1, slash2 - str - 1);
	Common::StringMap::iterator it = _entries.find(key);
	if (it != _entries.end()) {
		return it->_value;
	} else {
		return slash2 + 1;
	}
}

} // end of namespace Grim
