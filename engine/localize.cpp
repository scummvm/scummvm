/* Residual - Virtual machine to run LucasArts' 3D adventure games
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 *
 */

#include "common/file.h"

#include "engine/localize.h"
#include "engine/engine.h"

#include <cstring>

Localizer *g_localizer = NULL;

Localizer::Localizer() {
	Common::File f;
	const char *namesToTry[] = { "GRIM.TAB", "Grim.tab", "grim.tab" };

	if (g_flags & GF_DEMO)
		return;

	for (unsigned i = 0; i < sizeof(namesToTry) / sizeof(namesToTry[0]); i++) {
		f.open(namesToTry[i]);
		if (f.isOpen())
			break;
	}
	if (!f.isOpen()) {
		error("Localizer::Localizer: Unable to find localization information (grim.tab)!");
		return;
	}

	long filesize = f.size();

	// Read in the data
	char *data = new char[filesize + 1];
	f.read(data, filesize);
	data[filesize] = '\0';
	f.close();

	if (filesize < 4 || std::memcmp(data, "RCNE", 4) != 0)
		error("Invalid magic reading grim.tab");

	// Decode the data
	for (int i = 4; i < filesize; i++)
		data[i] ^= '\xdd';

	char *nextline;
	for (char *line = data + 4; line != NULL && *line != '\0'; line = nextline) {
		nextline = std::strchr(line, '\n');

		if (nextline) {
			if (nextline[-1] == '\r')
				nextline[-1] = '\0';
			nextline++;
		}
		char *tab = std::strchr(line, '\t');

		if (!tab)
			continue;

		std::string key(line, tab - line);
		std::string val = tab + 1;
		_entries[key] = val;
	}

	delete[] data;
}

std::string Localizer::localize(const char *str) const {
	assert(str);

	if (str[0] != '/' || str[0] == 0)
		return str;

	const char *slash2 = std::strchr(str + 1, '/');
	if (!slash2)
		return str;

	std::string key(str + 1, slash2 - str - 1);
	StringMap::const_iterator i = _entries.find(key);
	if (i == _entries.end())
		return slash2 + 1;

	return i->second;
}
