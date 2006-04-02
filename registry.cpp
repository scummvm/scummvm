/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
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

#include "stdafx.h"
#include "registry.h"
#include "debug.h"

#include <cstdlib>

Registry *g_registry = NULL;

Registry::Registry() : _dirty(false) {
#ifndef __DC__
#ifdef WIN32
	std::string filename = "residual.ini";
#else
	std::string filename = std::string(std::getenv("HOME")) + "/.residualrc";
#endif

	std::FILE *f = fopen(filename.c_str(), "r");
	if (f != NULL) {
		char line[1024];
		while (!feof(f) && fgets(line, sizeof(line), f) != NULL) {
			char *equals = std::strchr(line, '=');
			char *newline = std::strchr(line, '\n');
			if (newline != NULL)
				*newline = '\0';
			if (equals != NULL) {
				std::string key = std::string(line, equals - line);
				std::string val = std::string(equals + 1);
				_settings[key] = val;
			}
		}
		std::fclose(f);
	}
#endif
}

const char *Registry::get(const char *key, const char *defval) const {
	// GrimDataDir is an alias for DataDir for our purposes
	if (!strcmp(key, "GrimDataDir"))
		key = "DataDir";
	
	Group::const_iterator i = _settings.find(key);
	if (i == _settings.end())
		return defval;
	else
		return i->second.c_str();
}

void Registry::set(const char *key, const char *val) {
	// Hack: Don't save these, so we can run in good_times mode
	// without new games being bogus.
	if (strstr(key, "GrimLastSet") || strstr(key, "GrimMannyState"))
		return;

	_settings[key] = val;
	_dirty = true;
}

void Registry::save() {
	if (!_dirty)
		return;

#ifndef __DC__
#ifdef WIN32
	std::string filename = "residual.ini";
#else
	std::string filename = std::string(std::getenv("HOME")) + "/.residualrc";
#endif

	std::FILE *f = std::fopen(filename.c_str(), "w");
	if (f == NULL) {
		warning("Could not open registry file %s for writing\n",
			filename.c_str());
		return;
	}

	for (Group::iterator i = _settings.begin(); i != _settings.end(); i++)
		std::fprintf(f, "%s=%s\n", i->first.c_str(), i->second.c_str());

	std::fclose(f);
#endif
	_dirty = false;
}
