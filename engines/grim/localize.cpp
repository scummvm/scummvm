/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "common/file.h"
#include "common/str.h"
#include "common/endian.h"

#include "engines/grim/localize.h"
#include "engines/grim/grim.h"
#include "engines/grim/resource.h"

namespace Grim {

Localizer *g_localizer = NULL;

Localizer::Localizer() {
	if (g_grim->getGameFlags() & ADGF_DEMO || g_grim->getGameType() == GType_MONKEY4)
		return;

	Common::SeekableReadStream *f = g_resourceloader->openNewStreamFile("grim.tab");
	if (!f) {
		error("Localizer::Localizer: Unable to find localization information (grim.tab)");
		return;
	}

	int32 filesize = f->size();

	// Read in the data
	char *data = new char[filesize + 1];
	f->read(data, filesize);
	data[filesize] = '\0';
	delete f;

	if (filesize < 4 || READ_BE_UINT32(data) != MKTAG('R','C','N','E'))
		error("Invalid magic reading grim.tab");

	// Decode the data
	for (int i = 4; i < filesize; i++)
		data[i] ^= '\xdd';

	char *nextline;
	for (char *line = data + 4; line != NULL && *line != '\0'; line = nextline + 1) {
		nextline = strchr(line, '\n');
		assert(nextline);

		char *tab = strchr(line, '\t');
		assert(tab);

		_entries[Common::String(line, tab - line)] = Common::String(tab + 1, (nextline - tab - 2));
	}
	delete[] data;
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
