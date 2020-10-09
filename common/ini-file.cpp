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

#include "common/ini-file.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace Common {

bool INIFile::isValidName(const String &name) const {
	if (_allowNonEnglishCharacters)
		return true;
	const char *p = name.c_str();
	while (*p && (isAlnum(*p) || *p == '-' || *p == '_' || *p == '.' || *p == ' '))
		p++;
	return *p == 0;
}

INIFile::INIFile() {
	_allowNonEnglishCharacters = false;
}

void INIFile::clear() {
	_sections.clear();
}

bool INIFile::loadFromFile(const String &filename) {
	File file;
	if (file.open(filename))
		return loadFromStream(file);
	else
		return false;
}

bool INIFile::loadFromSaveFile(const String &filename) {
	assert(g_system);
	SaveFileManager *saveFileMan = g_system->getSavefileManager();
	SeekableReadStream *loadFile;

	assert(saveFileMan);
	if (!(loadFile = saveFileMan->openForLoading(filename)))
		return false;

	bool status = loadFromStream(*loadFile);
	delete loadFile;
	return status;
}

bool INIFile::loadFromStream(SeekableReadStream &stream) {
	Section section;
	KeyValue kv;
	String comment;
	int lineno = 0;
	section.name = _defaultSectionName;

	// TODO: Detect if a section occurs multiple times (or likewise, if
	// a key occurs multiple times inside one section).

	while (!stream.eos() && !stream.err()) {
		lineno++;

		// Read a line
		String line = stream.readLine();

		if (line.size() == 0) {
			// Do nothing
		} else if (line[0] == '#' || line[0] == ';' || line.hasPrefix("//")) {
			// Accumulate comments here. Once we encounter either the start
			// of a new section, or a key-value-pair, we associate the value
			// of the 'comment' variable with that entity. The semicolon and
			// C++-style comments are used for Living Books games in Mohawk.
			comment += line;
			comment += "\n";
		} else if (line[0] == '(') {
			// HACK: The following is a hack added by Kirben to support the
			// "map.ini" used in the HE SCUMM game "SPY Fox in Hold the Mustard".
			//
			// It would be nice if this hack could be restricted to that game,
			// but the current design of this class doesn't allow to do that
			// in a nice fashion (a "isMustard" parameter is *not* a nice
			// solution).
			comment += line;
			comment += "\n";
		} else if (line[0] == '[') {
			// It's a new section which begins here.
			const char *p = line.c_str() + 1;
			// Get the section name, and check whether it's valid (that
			// is, verify that it only consists of alphanumerics,
			// periods, dashes and underscores). Mohawk Living Books games
			// can have periods in their section names.
			while (*p && ((_allowNonEnglishCharacters && *p != ']') || isAlnum(*p) || *p == '-' || *p == '_' || *p == '.' || *p == ' '))
				p++;

			if (*p == '\0')
				error("INIFile::loadFromStream: missing ] in line %d", lineno);
			else if (*p != ']')
				error("INIFile::loadFromStream: Invalid character '%c' occurred in section name in line %d", *p, lineno);

			// Previous section is finished now, store it.
			if (!section.name.empty())
				_sections.push_back(section);

			section.name = String(line.c_str() + 1, p);
			section.keys.clear();
			section.comment = comment;
			comment.clear();

			if (!isValidName(section.name)) {
				warning("Invalid section name: %s", section.name.c_str());
				return false;
			}
		} else {
			// This line should be a line with a 'key=value' pair, or an empty one.

			// Skip leading whitespaces
			const char *t = line.c_str();
			while (isSpace(*t))
				t++;

			// Skip empty lines / lines with only whitespace
			if (*t == 0)
				continue;

			// If no section has been set, this config file is invalid!
			if (section.name.empty()) {
				error("INIFile::loadFromStream: Key/value pair found outside a section in line %d", lineno);
			}

			// Split string at '=' into 'key' and 'value'. First, find the "=" delimeter.
			const char *p = strchr(t, '=');
			if (!p) {
				warning("Config file buggy: Junk found in line line %d: '%s'", lineno, t);
				kv.key = String(t);
				kv.value.clear();
			}  else {
				// Extract the key/value pair
				kv.key = String(t, p);
				kv.value = String(p + 1);
			}

			// Trim of spaces
			kv.key.trim();
			kv.value.trim();

			// Store comment
			kv.comment = comment;
			comment.clear();

			if (!isValidName(kv.key)) {
				warning("Invalid key name: %s", kv.key.c_str());
				return false;
			}

			section.keys.push_back(kv);
		}
	}

	// Save last section
	if (!section.name.empty())
		_sections.push_back(section);

	return (!stream.err() || stream.eos());
}

bool INIFile::saveToFile(const String &filename) {
	DumpFile file;
	if (file.open(filename))
		return saveToStream(file);
	else
		return false;
}

bool INIFile::saveToSaveFile(const String &filename) {
	assert(g_system);
	SaveFileManager *saveFileMan = g_system->getSavefileManager();
	WriteStream *saveFile;

	assert(saveFileMan);
	if (!(saveFile = saveFileMan->openForSaving(filename)))
		return false;

	bool status = saveToStream(*saveFile);
	delete saveFile;
	return status;
}

bool INIFile::saveToStream(WriteStream &stream) {
	for (List<Section>::iterator i = _sections.begin(); i != _sections.end(); ++i) {
		// Write out the section comment, if any
		if (! i->comment.empty()) {
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
			if (! kv->comment.empty()) {
				stream.writeString(kv->comment);
			}
			// Write out the key/value pair
			stream.writeString(kv->key);
			stream.writeByte('=');
			stream.writeString(kv->value);
			stream.writeByte('\n');
		}
	}

	stream.flush();
	return !stream.err();
}

void INIFile::addSection(const String &section) {
	Section *s = getSection(section);
	if (s)
		return;

	Section newSection;
	newSection.name = section;
	_sections.push_back(newSection);
}

void INIFile::removeSection(const String &section) {
	if (!isValidName(section)) {
		warning("Invalid section name: %s", section.c_str());
		return;
	}

	for (List<Section>::iterator i = _sections.begin(); i != _sections.end(); ++i) {
		if (section.equalsIgnoreCase(i->name)) {
			_sections.erase(i);
			return;
		}
	}
}

bool INIFile::hasSection(const String &section) const {
	if (!isValidName(section)) {
		warning("Invalid section name: %s", section.c_str());
		return false;
	}

	const Section *s = getSection(section);
	return s != nullptr;
}

void INIFile::renameSection(const String &oldName, const String &newName) {
	if (!isValidName(oldName)) {
		warning("Invalid section name: %s", oldName.c_str());
		return;
	}

	if (!isValidName(newName)) {
		warning("Invalid section name: %s", newName.c_str());
		return;
	}

	Section *os = getSection(oldName);
	const Section *ns = getSection(newName);
	if (os) {
		// HACK: For now we just print a warning, for more info see the TODO
		// below.
		if (ns)
			warning("INIFile::renameSection: Section name \"%s\" already used", newName.c_str());
		else
			os->name = newName;
	}
	// TODO: Check here whether there already is a section with the
	// new name. Not sure how to cope with that case, we could:
	// - simply remove the existing "newName" section
	// - error out
	// - merge the two sections "oldName" and "newName"
}

void INIFile::setDefaultSectionName(const String &name) {
	_defaultSectionName = name;
}

bool INIFile::hasKey(const String &key, const String &section) const {
	if (!isValidName(key)) {
		warning("Invalid key name: %s", key.c_str());
		return false;
	}

	if (!isValidName(section)) {
		warning("Invalid section name: %s", section.c_str());
		return false;
	}

	const Section *s = getSection(section);
	if (!s)
		return false;
	return s->hasKey(key);
}

void INIFile::removeKey(const String &key, const String &section) {
	if (!isValidName(key)) {
		warning("Invalid key name: %s", key.c_str());
		return;
	}

	if (!isValidName(section)) {
		warning("Invalid section name: %s", section.c_str());
		return;
	}

	Section *s = getSection(section);
	if (s)
		 s->removeKey(key);
}

bool INIFile::getKey(const String &key, const String &section, String &value) const {
	if (!isValidName(key)) {
		warning("Invalid key name: %s", key.c_str());
		return false;
	}

	if (!isValidName(section)) {
		warning("Invalid section name: %s", section.c_str());
		return false;
	}
	const Section *s = getSection(section);
	if (!s)
		return false;
	const KeyValue *kv = s->getKey(key);
	if (!kv)
		return false;
	value = kv->value;
	return true;
}

void INIFile::setKey(const String &key, const String &section, const String &value) {
	if (!isValidName(key)) {
		warning("Invalid key name: %s", key.c_str());
		return;
	}

	if (!isValidName(section)) {
		warning("Invalid section name: %s", section.c_str());
		return;
	}

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

const INIFile::SectionKeyList INIFile::getKeys(const String &section) const {
	const Section *s = getSection(section);

	return s->getKeys();
}

INIFile::Section *INIFile::getSection(const String &section) {
	for (List<Section>::iterator i = _sections.begin(); i != _sections.end(); ++i) {
		if (section.equalsIgnoreCase(i->name)) {
			return &(*i);
		}
	}
	return nullptr;
}

const INIFile::Section *INIFile::getSection(const String &section) const {
	for (List<Section>::const_iterator i = _sections.begin(); i != _sections.end(); ++i) {
		if (section.equalsIgnoreCase(i->name)) {
			return &(*i);
		}
	}
	return nullptr;
}

bool INIFile::Section::hasKey(const String &key) const {
	return getKey(key) != nullptr;
}

const INIFile::KeyValue* INIFile::Section::getKey(const String &key) const {
	for (List<KeyValue>::const_iterator i = keys.begin(); i != keys.end(); ++i) {
		if (key.equalsIgnoreCase(i->key)) {
			return &(*i);
		}
	}
	return nullptr;
}

void INIFile::Section::setKey(const String &key, const String &value) {
	for (List<KeyValue>::iterator i = keys.begin(); i != keys.end(); ++i) {
		if (key.equalsIgnoreCase(i->key)) {
			i->value = value;
			return;
		}
	}

	KeyValue newKV;
	newKV.key = key;
	newKV.value = value;
	keys.push_back(newKV);
}

void INIFile::Section::removeKey(const String &key) {
	for (List<KeyValue>::iterator i = keys.begin(); i != keys.end(); ++i) {
		if (key.equalsIgnoreCase(i->key)) {
			keys.erase(i);
			return;
		}
	}
}

void INIFile::allowNonEnglishCharacters() {
	_allowNonEnglishCharacters = true;
}

} // End of namespace Common
