/*
Copyright (C) 2004 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef ULTIMA8_CONF_INIFILE_H
#define ULTIMA8_CONF_INIFILE_H

#include "ultima/ultima8/std/containers.h"

namespace Ultima8 {

typedef std::map<Pentagram::istring, std::string, Common::IgnoreCase_Hash> KeyMap;

class INIFile {
public:
	INIFile();
	INIFile(std::string fname, Pentagram::istring root);
	~INIFile();

	bool readConfigFile(std::string fname);

	//! read configuration from a string s. Lines must be separated by \n
	bool readConfigString(std::string s);

	void clear(Pentagram::istring root);

	std::string dump();
	void write();

	void setReadonly() {
		readonly = true;
	}
	bool isReadonly() const {
		return readonly;
	}

	bool hasSection(Pentagram::istring section);
	bool hasKey(Pentagram::istring key);
	bool checkRoot(Pentagram::istring key);

	// get value
	bool value(Pentagram::istring key, std::string &ret);
	bool value(Pentagram::istring key, int &ret);
	bool value(Pentagram::istring key, bool &ret);

	// set value
	void set(Pentagram::istring key, std::string value);
	void set(Pentagram::istring key, const char *value);
	void set(Pentagram::istring key, int value);
	void set(Pentagram::istring key, bool value);

	// remove key
	void unset(Pentagram::istring key);

	void listKeys(std::set<Pentagram::istring> &keys,
	              Pentagram::istring section,
	              bool longformat = false);

	void listSections(std::set<Pentagram::istring> &sections,
	                  bool longformat = false);

	void listKeyValues(KeyMap &keyvalues,
	                   Pentagram::istring section,
	                   bool longformat = false);

private:
	std::string filename;
	Pentagram::istring root;
	bool is_file;
	bool readonly;

	struct KeyValue {
		Pentagram::istring key;
		std::string value;
		std::string comment;
	};

	struct Section {
		Pentagram::istring name;
		std::list<KeyValue> keys;
		std::string comment;

		bool hasKey(Pentagram::istring key);
		KeyValue *getKey(Pentagram::istring key);
		void setKey(Pentagram::istring key, std::string value);
		void unsetKey(Pentagram::istring key);

		std::string dump();
	};

	std::list<Section> sections;


	bool stripRoot(Pentagram::istring &key);
	Section *getSection(Pentagram::istring section);
	bool splitKey(Pentagram::istring key, Pentagram::istring &section,
	              Pentagram::istring &sectionkey);

};

} // End of namespace Ultima8

#endif
