// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
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

#ifndef REGISTRY_HH
#define REGISTRY_HH

#include <string>
#include <map>

class Registry {
public:
	const char *get(const char *key, const char *defval) const;
	void set(const char *key, const char *val);
	void save();

	Registry();
	~Registry() { }

private:

	static Registry *_instance;

	typedef std::map<std::string, std::string> Group;
	Group _settings;
	bool _dirty;
};

extern Registry *g_registry;

#endif
