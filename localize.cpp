// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "stdafx.h"
#include "debug.h"
#include "localize.h"
#include "registry.h"

#include <cstdio>
#include <cstring>

Localizer *g_localizer = NULL;

Localizer::Localizer() {
	std::FILE *f;
	const char *namesToTry[] = { "/GRIM.TAB", "/Grim.tab", "/grim.tab" };

	for (unsigned i = 0; i < sizeof(namesToTry) / sizeof(namesToTry[0]); i++) {
		const char *datadir = g_registry->get("DataDir", NULL);
		std::string fname = (datadir != NULL ? datadir : ".");
		fname += namesToTry[i];
		f = std::fopen(fname.c_str(), "rb");
		if (f != NULL)
			break;
	}
	if (f == NULL) {
		error("Localizer::Localizer: Unable to find localization information (grim.tab)!");
		return;
	}

	// Get the file size
	std::fseek(f, 0, SEEK_END);
	long filesize = std::ftell(f);
	std::fseek(f, 0, SEEK_SET);

	// Read in the data
	char *data = new char[filesize + 1];
	std::fread(data, 1, filesize, f);
	data[filesize] = '\0';
	std::fclose(f);

	if (filesize < 4 || std::memcmp(data, "RCNE", 4) != 0)
		error("Invalid magic reading grim.tab\n");

	// Decode the data
	for (int i = 4; i < filesize; i++)
		data[i] ^= '\xdd';

	char *nextline;
	for (char *line = data + 4; line != NULL && *line != '\0'; line = nextline) {
		nextline = std::strchr(line, '\n');

		if (nextline != NULL) {
			if (nextline[-1] == '\r')
				nextline[-1] = '\0';
			nextline++;
		}
		char *tab = std::strchr(line, '\t');

		if (tab == NULL)
			continue;

		std::string key(line, tab - line);
		std::string val = tab + 1;
		_entries[key] = val;
	}

	delete[] data;
}

std::string Localizer::localize(const char *str) const {
	assert(str);

	if ((str[0] != '/') || (str[0] == 0))
		return str;

	const char *slash2 = std::strchr(str + 1, '/');
	if (slash2 == NULL)
		return str;

	std::string key(str + 1, slash2 - str - 1);
	StringMap::const_iterator i = _entries.find(key);
	if (i == _entries.end())
		return slash2 + 1;

	return i->second;
}
