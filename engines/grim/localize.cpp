/* Residual - A 3D game interpreter
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
#include "common/str.h"
#include "common/endian.h"

#include "engines/grim/localize.h"
#include "engines/grim/grim.h"

namespace Grim {

Localizer *g_localizer = NULL;

static int sortCallback(const void *entry1, const void *entry2) {
	return strcasecmp(((Localizer::LocaleEntry *)entry1)->text, ((Localizer::LocaleEntry *)entry2)->text);
}

Localizer::Localizer() {
	Common::File f;
	const char *namesToTry[] = { "GRIM.TAB", "Grim.tab", "grim.tab" };

	if (g_grim->getGameFlags() & GF_DEMO)
		return;

	for (unsigned i = 0; i < sizeof(namesToTry) / sizeof(namesToTry[0]); i++) {
		f.open(namesToTry[i]);
		if (f.isOpen())
			break;
	}
	if (!f.isOpen()) {
		error("Localizer::Localizer: Unable to find localization information (grim.tab)");
		return;
	}

	long filesize = f.size();

	// Read in the data
	char *data = new char[filesize + 1];
	f.read(data, filesize);
	data[filesize] = '\0';
	f.close();

	if (filesize < 4 || READ_BE_UINT32(data) != MKID_BE('RCNE'))
		error("Invalid magic reading grim.tab");

	// Decode the data
	for (int i = 4; i < filesize; i++)
		data[i] ^= '\xdd';

	char *nextline;
	for (char *line = data + 4; line != NULL && *line != '\0'; line = nextline) {
		nextline = strchr(line, '\n');

		if (nextline) {
			if (nextline[-1] == '\r')
				nextline[-1] = '\0';
			nextline++;
		}
		char *tab = strchr(line, '\t');

		if (!tab)
			continue;

		LocaleEntry entry;
		entry.text = new char[(tab - line) + 1];
		strncpy(entry.text, line, tab - line);
		entry.text[tab - line] = '\0';
		entry.translation = new char[strlen(tab + 1) + 1];
		strcpy(entry.translation, tab + 1);
		_entries.push_back(entry);
	}

	qsort(_entries.begin(), _entries.size(), sizeof(LocaleEntry), sortCallback);

	delete[] data;
}

Common::String Localizer::localize(const char *str) const {
	assert(str);

	if (str[0] != '/' || str[0] == 0)
		return str;

	const char *slash2 = strchr(str + 1, '/');
	if (!slash2)
		return str;

	LocaleEntry key, *result;
	Common::String s(str + 1, slash2 - str - 1);
	key.text = (char *)s.c_str();
	result = (Localizer::LocaleEntry *)bsearch(&key, _entries.begin(), _entries.size(), sizeof(LocaleEntry), sortCallback);

	if (!result)
		return slash2 + 1;

	return result->translation;
}

Localizer::~Localizer() {
	for (unsigned int i = 0; i < _entries.size(); i++) {
		delete[] _entries[i].text;
		delete[] _entries[i].translation;
	}
}

} // end of namespace Grim
