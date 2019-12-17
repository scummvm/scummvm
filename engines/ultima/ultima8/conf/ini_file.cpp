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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/conf/ini_file.h"

#include "ultima/ultima8/filesys/file_system.h"
#include "ultima/ultima8/filesys/idata_source.h"

namespace Ultima {
namespace Ultima8 {

using Pentagram::istring;
using std::string;

INIFile::INIFile()
	: is_file(false), readonly(false) {

}

INIFile::INIFile(string fname, istring root_)
	: root(root_), is_file(false), readonly(false) {
	readConfigFile(fname);
}

INIFile::~INIFile() {

}

bool INIFile::Section::hasKey(istring key) {
	return (getKey(key) != 0);
}

INIFile::KeyValue *INIFile::Section::getKey(istring key) {
	std::list<KeyValue>::iterator i;
	for (i = keys.begin(); i != keys.end(); ++i) {
		if (i->key == key) {
			return &(*i);
		}
	}
	return 0;
}

void INIFile::Section::setKey(istring key, string value) {
	KeyValue *kv = getKey(key);
	if (kv) {
		kv->value = value;
		return;
	}

	KeyValue newkey;
	newkey.key = key;
	newkey.value = value;
	newkey.comment = "";
	keys.push_back(newkey);
}

void INIFile::Section::unsetKey(istring key) {
	std::list<KeyValue>::iterator i;
	for (i = keys.begin(); i != keys.end(); ++i) {
		if (i->key == key) {
			i = keys.erase(i);
		}
	}
}

string INIFile::Section::dump() {
	string s = comment;
	s += "[" + name + "]\n";
	std::list<KeyValue>::iterator i;
	for (i = keys.begin(); i != keys.end(); ++i) {
		s += i->comment;
		s += i->key + "=" + i->value + "\n";
	}

	return s;
}

bool INIFile::readConfigFile(string fname) {
	IDataSource *f = FileSystem::get_instance()->ReadFile(fname, true);
	if (!f) return false;

	string sbuf, line;
	while (!f->eof()) {
		f->readline(line);
		string::size_type pos = line.find_first_of("\n\r");
		if (pos != string::npos) {
			sbuf += line.substr(0, pos) + "\n";
		} else {
			sbuf += line + "\n";
		}
	}

	delete f;

	if (!readConfigString(sbuf))
		return false;

	is_file = true; // readConfigString sets is_file = false
	filename = fname;
	return true;
}


static void rtrim(string &s) {
	string::size_type pos = s.find_last_not_of(" \t");
	if (pos != string::npos) {
		if (pos + 1 < s.size())
			s.erase(pos + 1);
	} else {
		s.clear();
	}
}

static void ltrim(string &s) {
	string::size_type pos = s.find_first_not_of(" \t");
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
	is_file = false;

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
				     << lineno << std::endl;
				return false;
			}

			// Get the section name, and check whether it's valid (that
			// is, verify that it only consists of alphanumerics,
			// dashes, underscores and colons).
			while (p < line.size() && (Common::isAlnum(line[p]) || line[p] == '-' ||
			                           line[p] == '_' || line[p] == ':'))
				p++;

			if (p >= line.size()) {
				perr << "Config file buggy: missing ] in line " << lineno
				     << ": '" << line << "'" << std::endl;
				return false;
			}
			if (line[p] != ']') {
				perr << "Config file buggy: Invalid character '" << line[p]
				     << "' occured in section name in line " << lineno
				     << std::endl;
				return false;
			}

			if (!section.name.empty()) {
				// save previous section
				sections.push_back(section);
			}
			section.name.clear();
			section.comment.clear();
			section.keys.clear();

			section.name = line.substr(1, p - 1);
			section.comment = comment;
			comment.clear();

		} else {
			// Skip leading & trailing whitespaces
			rtrim(line);
			ltrim(line);

			// Skip empty lines
			if (line.empty())
				continue;

			// If no section has been set, this config file is invalid!
			if (section.name.empty()) {
				perr << "Config file buggy: Key/value pair found outside "
				     << "a section in line " << lineno << std::endl;
				return false;
			}

			// Split string at '=' into 'key' and 'value'.
			string::size_type p = line.find('=');
			if (p == string::npos || p == 0) {
				perr << "Config file buggy: Junk found in line " << lineno
				     << ": '" << line << "'" << std::endl;
				return false;
			}

			KeyValue v;

			string t = line.substr(0, p);
			rtrim(t);
			v.key = t;

			if (p + 1 < line.size())
				t = line.substr(p + 1);
			else
				t = "";
			ltrim(t);
			v.value = t;

			v.comment = comment;
			comment.clear();

#if 0
			pout << "section: " << section.name << ", key: " << v.key
			     << ", value: " << v.value << std::endl;
#endif

			section.keys.push_back(v);
		}
	}

	if (!section.name.empty()) {
		// save last section
		sections.push_back(section);
	}

	return true;
}

void INIFile::clear(istring root_) {
	sections.clear();
	root = root_;
	is_file = false;
	readonly = false;
	filename = "";
}

string INIFile::dump() {
	string s;

	std::list<Section>::iterator i;
	for (i = sections.begin(); i != sections.end(); ++i) {
		if (i != sections.begin())
			s += "\n";

		s += i->dump();
	}

	return s;
}

void INIFile::write() {
	if (!is_file || readonly)
		return;

	ODataSource *f = FileSystem::get_instance()->WriteFile(filename, true);
	if (!f) return;

	std::string s = dump();
	const char *cstr = s.c_str();
	f->write(cstr, strlen(cstr));

	delete f;
}

bool INIFile::stripRoot(istring &key) {
	string::size_type pos = key.find('/');
	if (pos == istring::npos) return false;

	istring keyroot = key.substr(0, pos);
	if (keyroot != root) return false;

	key.erase(0, pos + 1);

	return true;
}

INIFile::Section *INIFile::getSection(istring section) {
	std::list<Section>::iterator i;
	for (i = sections.begin(); i != sections.end(); ++i) {
		if (i->name == section) {
			return &(*i);
		}
	}
	return 0;
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

	return (getSection(section) != 0);
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
	return (root == key || stripRoot(key));
}

bool INIFile::value(istring key, string &ret) {
	if (!stripRoot(key)) return false;
	istring s, k;
	splitKey(key, s, k);

	Section *section = getSection(s);
	if (!section) return false;

	KeyValue *kv = section->getKey(k);
	if (!kv) return false;

	ret = kv->value;
	return true;
}

bool INIFile::value(istring key, int &ret) {
	string stringval;
	bool found = value(key, stringval);

	if (!found) return false;

	ret = std::strtol(stringval.c_str(), 0, 0);
	return true;
}

bool INIFile::value(istring key, bool &ret) {
	istring stringval;
	bool found = value(key, stringval);

	if (!found) return false;

	ret = (stringval == "yes" || stringval == "true");
	return true;
}

void INIFile::set(istring key, string value) {
	if (!stripRoot(key)) return;
	istring s, k;
	splitKey(key, s, k);

	Section *section = getSection(s);
	if (!section) {
		Section newsec;
		newsec.name = s;
		newsec.comment = "";
		sections.push_back(newsec);
		section = getSection(s);
		assert(section);
	}

	section->setKey(k, value);
}

void INIFile::set(istring key, const char *value) {
	string v = value;
	set(key, v);
}

void INIFile::set(istring key, int value) {
	char buf[32];
	snprintf(buf, 32, "%d", value);
	set(key, buf);
}

void INIFile::set(istring key, bool value) {
	if (value)
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

void INIFile::listKeys(std::set<istring> &keys, istring section_,
                       bool longformat) {
	if (!stripRoot(section_)) return;

	Section *section = getSection(section_);
	if (!section) return;

	std::list<KeyValue>::iterator i;
	for (i = section->keys.begin(); i != section->keys.end(); ++i) {
		istring k;
		if (longformat)
			k = root + "/" + section->name + "/" + i->key;
		else
			k = i->key;

		keys.insert(k);
	}
}

void INIFile::listSections(std::set<istring> &sections_, bool longformat) {
	std::list<Section>::iterator i;
	for (i = sections.begin(); i != sections.end(); ++i) {
		istring s;
		if (longformat)
			s = root + "/" + i->name;
		else
			s = i->name;

		sections_.insert(s);
	}
}

void INIFile::listKeyValues(KeyMap &keyvalues, istring section_, bool longformat) {
	if (!stripRoot(section_)) return;

	Section *section = getSection(section_);
	if (!section) return;

	std::list<KeyValue>::iterator i;
	for (i = section->keys.begin(); i != section->keys.end(); ++i) {
		istring k;
		if (longformat)
			k = root + "/" + section->name + "/" + i->key;
		else
			k = i->key;

		keyvalues[k] = i->value;
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
