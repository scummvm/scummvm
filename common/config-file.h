/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include "common/list.h"
#include "common/map.h"
#include "common/str.h"
#include "common/util.h"

class Config {
public:
	typedef Common::String String;
	typedef Common::StringList StringList;
	typedef Common::StringMap StringMap;
	typedef Common::Map<String, StringMap> DomainMap;

	Config (const String & = String("config.cfg"), const String & = String("default"));
	const char *get(const String &key, const String &dom = String()) const;
	const int getInt(const String &key, int def = 0, const String &dom = String()) const;
	const bool getBool(const String &key, bool def = false, const String &dom = String()) const;

	void set(const String &key, const String &value, const String &dom = String());
	void setInt(const String &key, int value, const String &dom = String());
	void setBool(const String &key, bool value, const String &dom = String());

	void set_domain(const String &d);
	void flush() const;
	void rename_domain(const String &oldD, const String &newD);
	void delete_domain(const String &d);
	bool has_domain(const String &d) const;
	void set_filename(const String &);
	void merge_config(const Config &);
	void set_writing(bool);

	const int count_domains();
	StringList get_domains();

protected:
	DomainMap domains;
	String filename;
	String defaultDomain;

	bool willwrite;
};

// The global config object
extern Config *g_config;

#endif
