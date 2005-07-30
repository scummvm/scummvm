/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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

#include "common/stdafx.h"

#include "common/config-file.h"
#include "common/file.h"
#include "common/util.h"

#define MAXLINELEN 256

namespace Common {

static char *ltrim(char *t) {
	while (isspace(*t))
		t++;
	return t;
}

static char *rtrim(char *t) {
	int l = strlen(t) - 1;
	while (l >= 0 && isspace(t[l]))
		t[l--] = 0;
	return t;
}

/**
 * Check whether the given string is a valid section or key name.
 * For that, it must only consist of letters, numbers, dashes and
 * underscores. In particular, white space and "#", "=", "[", "]"
 * are not valid!
 */
bool ConfigFile::isValidName(const Common::String &name) {
	const char *p = name.c_str();
	while (*p && (isalnum(*p) || *p == '-' || *p == '_'))
		p++;
	return *p == 0;
}

ConfigFile::ConfigFile() {
}

ConfigFile::~ConfigFile() {
}

void ConfigFile::clear() {
	_sections.clear();
}

bool ConfigFile::loadFromFile(const String &filename) {
	File file;
	if (file.open(filename.c_str(), File::kFileReadMode))
		return loadFromStream(file);
	else
		return false;
}

bool ConfigFile::loadFromStream(SeekableReadStream &stream) {
	char buf[MAXLINELEN];
	Section section;
	KeyValue kv;
	String comment;
	int lineno = 0;

	// TODO: Detect if a section occurs multiple times (or likewise, if
	// a key occurs multiple times inside one section).

	while (!stream.eos()) {
		lineno++;
		if (!stream.readLine(buf, MAXLINELEN))
			break;

		if (buf[0] == '#') {
			// Accumulate comments here. Once we encounter either the start
			// of a new section, or a key-value-pair, we associate the value
			// of the 'comment' variable with that entity.
			comment += buf;
			comment += "\n";
		} else if (buf[0] == '(') {
			// Special case for map.ini included in mustard
			// Includes comment within () on the first line
			comment += buf;
			comment += "\n";
		} else if (buf[0] == '[') {
			// It's a new section which begins here.
			char *p = buf + 1;
			// Get the section name, and check whether it's valid (that
			// is, verify that it only consists of alphanumerics,
			// dashes and underscores).
			while (*p && (isalnum(*p) || *p == '-' || *p == '_'))
				p++;

			if (*p == '\0')
				error("Config file buggy: missing ] in line %d", lineno);
			else if (*p != ']')
				error("Config file buggy: Invalid character '%c' occured in section name in line %d", *p, lineno);

			*p = 0;

			// Previous section is finished now, store it.
			if (!section.name.isEmpty())
				_sections.push_back(section);

			section.name = buf + 1;
			section.keys.clear();
			section.comment = comment;
			comment.clear();

			assert(isValidName(section.name));
		} else {
			// Skip leading & trailing whitespaces
			char *t = rtrim(ltrim(buf));

			// Skip empty lines
			if (*t == 0)
				continue;

			// If no section has been set, this config file is invalid!
			if (section.name.isEmpty()) {
				error("Config file buggy: Key/value pair found outside a section in line %d", lineno);
			}

			// Split string at '=' into 'key' and 'value'.
			char *p = strchr(t, '=');
			if (!p)
				error("Config file buggy: Junk found in line line %d: '%s'", lineno, t);
			*p = 0;

			kv.key = rtrim(t);
			kv.value = ltrim(p + 1);
			kv.comment = comment;
			comment.clear();

			assert(isValidName(kv.key));

			section.keys.push_back(kv);
		}
	}

	// Save last section
	if (!section.name.isEmpty())
		_sections.push_back(section);

	return !stream.ioFailed();
}

bool ConfigFile::saveToFile(const String &filename) {
	File file;
	if (file.open(filename.c_str(), File::kFileWriteMode))
		return saveToStream(file);
	else
		return false;
}

bool ConfigFile::saveToStream(WriteStream &stream) {
	for (List<Section>::iterator i = _sections.begin(); i != _sections.end(); ++i) {
		// Write out the section comment, if any
		if (! i->comment.isEmpty()) {
			stream.writeString(i->comment);
		}

		// Write out the section name
		stream.writeByte('[');
		stream.writeString(i->name);
		stream.writeByte(']');
		stream.writeByte('\n');

		// Write out the key/value pairs
		for (List<KeyValue>::iterator kv = i->keys.begin(); kv != i->keys.end(); ++kv) {
			// Write out the comment, if any
			if (! kv->comment.isEmpty()) {
				stream.writeString(kv->comment);
			}
			// Write out the key/value pair
			stream.writeString(kv->key);
			stream.writeByte('=');
			stream.writeString(kv->value);
			stream.writeByte('\n');
		}
	}

	return !stream.ioFailed();
}


void ConfigFile::removeSection(const String &section) {
	assert(isValidName(section));
	for (List<Section>::iterator i = _sections.begin(); i != _sections.end(); ++i) {
		if (!scumm_stricmp(section.c_str(), i->name.c_str())) {
			_sections.erase(i);
			return;
		}
	}
}

bool ConfigFile::hasSection(const String &section) const {
	assert(isValidName(section));
	const Section *s = getSection(section);
	return s != 0;
}

void ConfigFile::renameSection(const String &oldName, const String &newName) {
	assert(isValidName(oldName));
	assert(isValidName(newName));

	//Section *os = getSection(oldName);
	Section *ns = getSection(newName);
	if (ns) {
		ns->name = newName;
	}
	// TODO: Check here whether there already is a section with the
	// new name. Not sure how to cope with that case, we could:
	// - simply remove the existing "newName" section
	// - error out
	// - merge the two sections "oldName" and "newName"
}


bool ConfigFile::hasKey(const String &key, const String &section) const {
	assert(isValidName(key));
	assert(isValidName(section));

	const Section *s = getSection(section);
	if (!s)
		return false;
	return s->hasKey(key);
}

void ConfigFile::removeKey(const String &key, const String &section) {
	assert(isValidName(key));
	assert(isValidName(section));

	Section *s = getSection(section);
	if (s)
		 s->removeKey(key);
}

bool ConfigFile::getKey(const String &key, const String &section, String &value) const {
	assert(isValidName(key));
	assert(isValidName(section));

	const Section *s = getSection(section);
	if (!s)
		return false;
	const KeyValue *kv = s->getKey(key);
	if (!kv)
		return false;
	value = kv->value;
	return true;
}

void ConfigFile::setKey(const String &key, const String &section, const String &value) {
	assert(isValidName(key));
	assert(isValidName(section));
	// TODO: Verify that value is valid, too. In particular, it shouldn't
	// contain CR or LF...

	Section *s = getSection(section);
	if (!s) {
		KeyValue newKV;
		newKV.key = key;
		newKV.value = value;

		Section newSection;
		newSection.name = section;
		newSection.keys.push_back(newKV);

		_sections.push_back(newSection);
	} else {
		s->setKey(key, value);
	}
}

ConfigFile::Section *ConfigFile::getSection(const String &section) {
	for (List<Section>::iterator i = _sections.begin(); i != _sections.end(); ++i) {
		if (!scumm_stricmp(section.c_str(), i->name.c_str())) {
			return &(*i);
		}
	}
	return 0;
}

const ConfigFile::Section *ConfigFile::getSection(const String &section) const {
	for (List<Section>::const_iterator i = _sections.begin(); i != _sections.end(); ++i) {
		if (!scumm_stricmp(section.c_str(), i->name.c_str())) {
			return &(*i);
		}
	}
	return 0;
}

bool ConfigFile::Section::hasKey(const String &key) const {
	return getKey(key) != 0;
}

const ConfigFile::KeyValue* ConfigFile::Section::getKey(const String &key) const {
	for (List<KeyValue>::const_iterator i = keys.begin(); i != keys.end(); ++i) {
		if (!scumm_stricmp(key.c_str(), i->key.c_str())) {
			return &(*i);
		}
	}
	return 0;
}

void ConfigFile::Section::setKey(const String &key, const String &value) {
	for (List<KeyValue>::iterator i = keys.begin(); i != keys.end(); ++i) {
		if (!scumm_stricmp(key.c_str(), i->key.c_str())) {
			i->value = value;
			return;
		}
	}

	KeyValue newKV;
	newKV.key = key;
	newKV.value = value;
	keys.push_back(newKV);
}

void ConfigFile::Section::removeKey(const String &key) {
	for (List<KeyValue>::iterator i = keys.begin(); i != keys.end(); ++i) {
		if (!scumm_stricmp(key.c_str(), i->key.c_str())) {
			keys.erase(i);
			return;
		}
	}
}

}	// End of namespace Common
