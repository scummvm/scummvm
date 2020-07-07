/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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

#ifndef ULTIMA8_CONF_INIFILE_H
#define ULTIMA8_CONF_INIFILE_H

#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima8 {

typedef Std::map<istring, Std::string, Common::IgnoreCase_Hash> KeyMap;

class INIFile {
public:
	INIFile();
	INIFile(Std::string fname, istring root);
	~INIFile();

	bool readConfigFile(const Std::string &fname);

	//! read configuration from a string s. Lines must be separated by \n
	bool readConfigString(Std::string s);

	void clear(istring root);

	Std::string dump();
	void write();

	void setReadonly() {
		_readOnly = true;
	}
	bool isReadonly() const {
		return _readOnly;
	}

	bool hasSection(istring section);
	bool hasKey(istring key);
	bool checkRoot(istring key);

	// get value
	bool value(istring key, Std::string &ret);
	bool value(istring key, int &ret);
	bool value(istring key, bool &ret);

	// set value
	void set(istring key, Std::string strValue);
	void set(istring key, const char *strValue);
	void set(istring key, int intValue);
	void set(istring key, bool boolValue);

	// remove key
	void unset(istring key);

	void listKeys(Std::set<istring> &keys,
	              istring section,
	              bool longformat = false);

	void listSections(Std::set<istring> &sections,
	                  bool longformat = false);

	void listKeyValues(KeyMap &keyvalues,
	                   istring section,
	                   bool longformat = false);

private:
	Std::string _filename;
	istring _root;
	bool _isFile;
	bool _readOnly;

	struct KeyValue {
		istring _key;
		Std::string _value;
		Std::string _comment;
	};

	struct Section {
		istring _name;
		Std::list<KeyValue> _keys;
		Std::string _comment;

		bool hasKey(istring key);
		KeyValue *getKey(istring key);
		void setKey(istring key, Std::string value);
		void unsetKey(istring key);

		Std::string dump();
	};

	Std::list<Section> _sections;


	bool stripRoot(istring &key);
	Section *getSection(istring section);
	bool splitKey(istring key, istring &section,
	              istring &sectionkey);

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
