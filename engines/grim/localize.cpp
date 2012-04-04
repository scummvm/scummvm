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

static int sortCallback(const void *entry1, const void *entry2) {
	return scumm_stricmp(((const Localizer::LocaleEntry *)entry1)->text, ((const Localizer::LocaleEntry *)entry2)->text);
}

Localizer::Localizer() {
	_data = 0;

	if (g_grim->getGameFlags() & ADGF_DEMO || g_grim->getGameType() == GType_MONKEY4)
		return;

	Common::SeekableReadStream *f = g_resourceloader->openNewStreamFile("grim.tab");
	if (!f) {
		error("Localizer::Localizer: Unable to find localization information (grim.tab)");
		return;
	}

	long filesize = f->size();

	// Read in the data
	_data = new char[filesize + 1];
	f->read(_data, filesize);
	_data[filesize] = '\0';
	delete f;

	if (filesize < 4 || READ_BE_UINT32(_data) != MKTAG('R','C','N','E'))
		error("Invalid magic reading grim.tab");

	// Decode the data
	for (int i = 4; i < filesize; i++)
		_data[i] ^= '\xdd';

	char *nextline;
	for (char *line = _data + 4; line != NULL && *line != '\0'; line = nextline) {
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
		entry.text = line;
		entry.text[tab - line] = '\0';
		entry.translation = tab + 1;
		_entries.push_back(entry);
	}

	qsort(_entries.begin(), _entries.size(), sizeof(LocaleEntry), sortCallback);

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
	key.text = const_cast<char *>(s.c_str());
	result = (Localizer::LocaleEntry *)bsearch(&key, _entries.begin(), _entries.size(), sizeof(LocaleEntry), sortCallback);

	if (!result)
		return slash2 + 1;

	return result->translation;
}

Localizer::~Localizer() {
	delete[] _data;
}

} // end of namespace Grim
