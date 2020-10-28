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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/conf/ini_file.h"

#include "ultima/ultima8/filesys/file_system.h"
#include "ultima/ultima8/filesys/idata_source.h"

namespace Ultima {
namespace Ultima8 {

using Std::string;

INIFile::INIFile()
	: _isFile(false), _readOnly(false) {

}

INIFile::INIFile(string fname, istring root)
	: _root(root), _isFile(false), _readOnly(false) {
	readConfigFile(fname);
}

INIFile::~INIFile() {
}

bool INIFile::Section::hasKey(istring key) {
	return (getKey(key) != nullptr);
}

INIFile::KeyValue *INIFile::Section::getKey(istring key) {
	Std::list<KeyValue>::iterator i;
	for (i = _keys.begin(); i != _keys.end(); ++i) {
		if (i->_key == key) {
			return &(*i);
		}
	}
	return nullptr;
}

void INIFile::Section::setKey(istring key, string value) {
	KeyValue *kv = getKey(key);
	if (kv) {
		kv->_value = value;
		return;
	}

	KeyValue newkey;
	newkey._key = key;
	newkey._value = value;
	newkey._comment = "";
	_keys.push_back(newkey);
}

void INIFile::Section::unsetKey(istring key) {
	Std::list<KeyValue>::iterator i;
	for (i = _keys.begin(); i != _keys.end(); ++i) {
		if (i->_key == key) {
			i = _keys.erase(i);
		}
	}
}

string INIFile::Section::dump() {
	string s = _comment;
	s += "[" + _name + "]\n";
	Std::list<KeyValue>::iterator i;
	for (i = _keys.begin(); i != _keys.end(); ++i) {
		s += i->_comment;
		s += i->_key + "=" + i->_value + "\n";
	}

	return s;
}

bool INIFile::readConfigFile(const string &fname) {
	IDataSource *f = FileSystem::get_instance()->ReadFile(fname, true);
	if (!f) return false;

	string sbuf, line;
	while (!f->eos()) {
		f->readline(line);
		string::size_type pos = line.findFirstOf("\n\r");
		if (pos != string::npos) {
			sbuf += line.substr(0, pos) + "\n";
		} else {
			sbuf += line + "\n";
		}
	}

	delete f;

	if (!readConfigString(sbuf))
		return false;

	_isFile = true; // readConfigString sets _isFile = false
	_filename = fname;
	return true;
}


static void rtrim(string &s) {
	string::size_type pos = s.findLastNotOf(" \t");
	if (pos != string::npos) {
		if (pos + 1 < s.size())
			s.erase(pos + 1);
	} else {
		s.clear();
	}
}

static void ltrim(string &s) {
	string::size_type pos = s.findFirstNotOf(" \t");
	if (pos != string::npos) {
		if (pos > 0)
			s.erase(0, pos - 1);
	} else {
		s.clear();
	}
}


// Large parts of the following function are borrowed from ScummVM's
// config-manager.cpp, copyright (C) 2001-2004 The ScummVM project
// http://www.scummvm.org/

bool INIFile::readConfigString(string config) {
	_isFile = false;

	string line;
	string comment;
	unsigned int lineno = 0;
	Section section;

	while (!config.empty()) {
		lineno++;

		string::size_type pos = config.find('\n');
		if (pos != string::npos) {
			line = config.substr(0, pos);
			config.erase(0, pos + 1);
		} else {
			line = config;
			config.clear();
		}

		if (line.length() > 0 && line[0] == '#') {
			// Accumulate comments here. Once we encounter either the start
			// of a new section, or a key-value-pair, we associate the value
			// of the 'comment' variable with that entity.
			comment += line + "\n";
		} else if (line.length() > 0 && line[0] == '[') {
			// It's a new section which begins here.
			unsigned int p = 1;

			if (line[p] == ']') {
				perr << "Config file buggy: empty section name in line "
				     << lineno << Std::endl;
				return false;
			}

			// Get the section name, and check whether it's valid (that
			// is, verify that it only consists of alphanumerics,
			// dashes, underscores and colons).
			while (p < line.size() && (Common::isAlnum(line[p]) || line[p] == '-' ||
			                           line[p] == '_' || line[p] == ':' || line[p] == ' '))
				p++;

			if (p >= line.size()) {
				perr << "Config file buggy: missing ] in line " << lineno
				     << ": '" << line << "'" << Std::endl;
				return false;
			}
			if (line[p] != ']') {
				perr << "Config file buggy: Invalid character '" << line[p]
				     << "' occured in section name in line " << lineno
				     << Std::endl;
				return false;
			}

			if (!section._name.empty()) {
				// save previous section
				_sections.push_back(section);
			}

			section._keys.clear();
			section._name = line.substr(1, p - 1);
			section._comment = comment;
			comment.clear();

		} else {
			// Skip leading & trailing whitespaces
			rtrim(line);
			ltrim(line);

			// Skip empty lines
			if (line.empty())
				continue;

			// If no section has been set, this config file is invalid!
			if (section._name.empty()) {
				perr << "Config file buggy: Key/value pair found outside "
				     << "a section in line " << lineno << Std::endl;
				return false;
			}

			// Split string at '=' into 'key' and 'value'.
			string::size_type p = line.find('=');
			if (p == string::npos || p == 0) {
				perr << "Config file buggy: Junk found in line " << lineno
				     << ": '" << line << "'" << Std::endl;
				return false;
			}

			KeyValue v;

			string t = line.substr(0, p);
			rtrim(t);
			v._key = t;

			if (p + 1 < line.size())
				t = line.substr(p + 1);
			else
				t = "";
			ltrim(t);
			v._value = t;

			v._comment = comment;
			comment.clear();

#if 0
			pout << "section: " << section._name << ", key: " << v._key
			     << ", value: " << v._value << Std::endl;
#endif

			section._keys.push_back(v);
		}
	}

	if (!section._name.empty()) {
		// save last section
		_sections.push_back(section);
	}

	return true;
}

void INIFile::clear(istring root) {
	_sections.clear();
	_root = root;
	_isFile = false;
	_readOnly = false;
	_filename = "";
}

string INIFile::dump() {
	string s;

	Std::list<Section>::iterator i;
	for (i = _sections.begin(); i != _sections.end(); ++i) {
		if (i != _sections.begin())
			s += "\n";

		s += i->dump();
	}

	return s;
}

void INIFile::write() {
	if (!_isFile || _readOnly)
		return;

	Common::WriteStream *f = FileSystem::get_instance()->WriteFile(_filename, true);
	if (!f) return;

	Std::string s = dump();
	const char *cstr = s.c_str();
	f->write(cstr, strlen(cstr));

	delete f;
}

bool INIFile::stripRoot(istring &key) {
	string::size_type pos = key.find('/');
	if (pos == istring::npos) return false;

	istring keyroot = key.substr(0, pos);
	if (keyroot != _root) return false;

	key.erase(0, pos + 1);

	return true;
}

INIFile::Section *INIFile::getSection(istring section) {
	Std::list<Section>::iterator i;
	for (i = _sections.begin(); i != _sections.end(); ++i) {
		if (i->_name == section) {
			return &(*i);
		}
	}
	return nullptr;
}

bool INIFile::splitKey(istring key, istring &section, istring &sectionkey) {
	// TODO: more sanity checks might be nice

	string::size_type pos = key.find('/');
	if (pos == istring::npos || pos + 1 >= key.size()) return false;

	section = key.substr(0, pos);
	sectionkey = key.substr(pos + 1);

	return true;
}

bool INIFile::hasSection(istring section) {
	if (!stripRoot(section)) return false;

	return (getSection(section) != nullptr);
}

bool INIFile::hasKey(istring key) {
	if (!stripRoot(key)) return false;
	istring s, k;
	splitKey(key, s, k);

	Section *section = getSection(s);
	if (!section) return false;

	return section->hasKey(k);
}

bool INIFile::checkRoot(istring key) {
	return (_root == key || stripRoot(key));
}

bool INIFile::value(istring key, string &ret) {
	if (!stripRoot(key)) return false;
	istring s, k;
	splitKey(key, s, k);

	Section *section = getSection(s);
	if (!section) return false;

	KeyValue *kv = section->getKey(k);
	if (!kv) return false;

	ret = kv->_value;
	return true;
}

bool INIFile::value(istring key, int &ret) {
	string stringval;
	bool found = value(key, stringval);

	if (!found) return false;

	ret = Std::strtol(stringval.c_str(), 0, 0);
	return true;
}

bool INIFile::value(istring key, bool &ret) {
	istring stringval;
	bool found = value(key, stringval);

	if (!found) return false;

	ret = (stringval == "yes" || stringval == "true");
	return true;
}

void INIFile::set(istring key, string strValue) {
	if (!stripRoot(key)) return;
	istring s, k;
	splitKey(key, s, k);

	Section *section = getSection(s);
	if (!section) {
		Section newsec;
		newsec._name = s;
		newsec._comment = "";
		_sections.push_back(newsec);
		section = getSection(s);
		assert(section);
	}

	section->setKey(k, strValue);
}

void INIFile::set(istring key, const char *strValue) {
	string v = strValue;
	set(key, v);
}

void INIFile::set(istring key, int intValue) {
	char buf[32];
	snprintf(buf, 32, "%d", intValue);
	set(key, buf);
}

void INIFile::set(istring key, bool boolValue) {
	if (boolValue)
		set(key, "true");
	else
		set(key, "false");
}

void INIFile::unset(istring key) {
	if (!stripRoot(key)) return;
	istring s, k;
	splitKey(key, s, k);

	Section *section = getSection(s);
	if (section) {
		section->unsetKey(k);
	}
}

void INIFile::listKeys(Std::set<istring> &_keys, istring sectionName,
                       bool longformat) {
	if (!stripRoot(sectionName)) return;

	const Section *section = getSection(sectionName);
	if (!section) return;

	Std::list<KeyValue>::const_iterator i;
	for (i = section->_keys.begin(); i != section->_keys.end(); ++i) {
		istring k;
		if (longformat)
			k = _root + "/" + section->_name + "/" + i->_key;
		else
			k = i->_key;

		_keys.insert(k);
	}
}

void INIFile::listSections(Std::set<istring> &sections, bool longformat) {
	Std::list<Section>::const_iterator i;
	for (i = _sections.begin(); i != _sections.end(); ++i) {
		istring s;
		if (longformat)
			s = _root + "/" + i->_name;
		else
			s = i->_name;

		sections.insert(s);
	}
}

void INIFile::listKeyValues(KeyMap &keyvalues, istring sectionName, bool longformat) {
	if (!stripRoot(sectionName)) return;

	Section *section = getSection(sectionName);
	if (!section) return;

	Std::list<KeyValue>::const_iterator i;
	for (i = section->_keys.begin(); i != section->_keys.end(); ++i) {
		istring k;
		if (longformat)
			k = _root + "/" + section->_name + "/" + i->_key;
		else
			k = i->_key;

		keyvalues[k] = i->_value;
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
