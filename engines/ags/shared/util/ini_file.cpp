/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/util.h"
#include "ags/shared/util/ini_file.h"
#include "ags/shared/util/text_stream_reader.h"
#include "ags/shared/util/text_stream_writer.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

inline static void ReplaceSubString(String &line, IniFile::StrPos &sub_pos, const String &new_sub) {
	line.ReplaceMid(sub_pos.first, sub_pos.second - sub_pos.first, new_sub);
}

IniFile::ItemDef::ItemDef(const String &key, const String &value) {
	Line = String::FromFormat("%s=%s", key.GetCStr(), value.GetCStr());
	Key.first = 0;
	Key.second = Key.first + key.GetLength();
	SepAt = Key.second;
	Value.first = Key.second + 1;
	Value.second = Value.first + value.GetLength();
}

IniFile::ItemDef::ItemDef(const String &line, const StrPos &key, const StrPos &value, size_t sep_at) {
	Line = line;
	Key = key;
	Value = value;
	SepAt = sep_at;
}

void IniFile::ItemDef::SetKey(const String &key) {
	if (key.IsEmpty())
		return;

	if (IsKeyValue()) {
		size_t diff = key.GetLength() - (Key.second - Key.first);
		ReplaceSubString(Line, Key, key);
		Key.second += diff;
		Value.first += diff;
		Value.second += diff;
	} else {
		*this = ItemDef(key, "");
	}
}

void IniFile::ItemDef::SetValue(const String &value) {
	if (!IsKeyValue())
		return; // no key

	if (SepAt != String::NoIndex) {   // replacing existing value
		size_t diff = value.GetLength() - (Value.second - Value.first);
		ReplaceSubString(Line, Value, value);
		Value.second += diff;
	} else {   // inserting value behind the key
		StrPos valpos(Key.second, Key.second);
		ReplaceSubString(Line, valpos, String::FromFormat("=%s", value.GetCStr()));
	}
}

IniFile::SectionDef::SectionDef(const String &name) {
	if (name.IsEmpty()) {
		// global section
		Name = StrPos(0, 0);
	} else {
		// named section
		Header = String::FromFormat("[%s]", name.GetCStr());
		Name.first = 1;
		Name.second = 1 + Header.GetLength();
	}
}

IniFile::SectionDef::SectionDef(const String &line, const StrPos &name) {
	Header = line;
	Name = name;
}

void IniFile::SectionDef::SetName(const String &sec_name) {
	if (sec_name.IsEmpty())
		return;

	size_t diff = sec_name.GetLength() - (Name.second - Name.first);
	ReplaceSubString(Header, Name, sec_name);
	Name.second += diff;
}

void IniFile::SectionDef::Clear() {
	Items.clear();
}

IniFile::ItemIterator IniFile::SectionDef::InsertItem(ItemIterator item, const ItemDef &itemdef) {
	return Items.insert(item, itemdef);
}

void IniFile::SectionDef::EraseItem(ItemIterator item) {
	Items.erase(item);
}

IniFile::ItemIterator IniFile::InsertItem(SectionIterator sec, ItemIterator item, const String &key, const String &value) {
	ItemDef itemdef(key, value);
	return sec->InsertItem(item, itemdef);
}

IniFile::SectionIterator IniFile::InsertSection(SectionIterator sec, const String &name) {
	if (name.IsEmpty())
		return _sections.end(); // do not allow adding random global sections

	SectionDef secdef(name);
	return _sections.insert(sec, secdef);
}

void IniFile::RemoveItem(SectionIterator sec, ItemIterator item) {
	sec->EraseItem(item);
}

void IniFile::RemoveSection(SectionIterator sec) {
	if (sec == _sections.begin())
		// do not remove global section, clear items instead
		sec->Clear();
	else
		_sections.erase(sec);
}


// Moves string pointer forward to the first non-space character
const char *SkipSpace(const char *line, const char *endl) {
	for (; line != endl && Common::isSpace(*line); ++line);
	return line;
}

// Parse given line and extract a meaningful string;
// Parses line from 'line' to 'endl', skips padding (spaces)
// at the beginning and the end. Assignes the starting and ending
// pointers of the string. Returns pointer to where parsing stopped.
// The 'endl' must point beyond the last character of the string
// (e.g. terminator).
const char *ParsePaddedString(const char *line, const char *endl,
                              const char *&str_at, const char *&str_end) {
	// skip left padding
	for (; line != endl && Common::isBlank(*line); ++line);
	str_at = line;
	// skip right padding
	const char *p_value = line;
	for (line = endl; line != p_value && Common::isBlank(*(line - 1)); --line);
	str_end = line;
	return line;
}

IniFile::IniFile() {
	// precreate global section
	_sections.push_back(SectionDef(""));
}

void IniFile::Read(Stream *in) {
	TextStreamReader reader(in);

	_sections.clear();
	// Create a global section;
	// all the items we meet before explicit section declaration
	// will be treated as "global" items.
	_sections.push_back(SectionDef(""));
	SectionDef *cur_section = &_sections.back();

	do {
		String line = reader.ReadLine();
		if (line.IsEmpty() && reader.EOS())
			break;

		const char *cstr = line.GetCStr();
		const char *pstr = cstr;
		const char *endl = cstr + line.GetLength();

		// Find first non-space character
		pstr = SkipSpace(pstr, endl);
		if (pstr == endl)
			continue; // empty line

		// Detect the kind of string we found
		if ((endl - pstr >= 2 && *pstr == '/' && *(pstr + 1) == '/') ||
		        (endl - pstr >= 1 && (*pstr == '#' || *pstr == ';'))) {
			StrPos nullpos(0, 0);
			cur_section->InsertItem(cur_section->End(), ItemDef(line, nullpos, nullpos, String::NoIndex));
			continue;
		}

		if (*pstr == '[') {
			// Parse this as section
			const char *pstr_end = strrchr(pstr, ']');
			if (pstr_end < pstr)
				continue; // no closing bracket
			// Parse the section name
			const char *str_at, *str_end;
			ParsePaddedString(++pstr, pstr_end, str_at, str_end);
			if (str_end == str_at)
				continue; // inappropriate data or empty string
			StrPos namepos(str_at - cstr, str_end - cstr);
			_sections.push_back(SectionDef(line, namepos));
			cur_section = &_sections.back();
		} else {
			// Parse this as a key-value pair
			const char *pstr_end = strchr(pstr, '=');
			if (pstr_end == pstr)
				continue; // no key part, skip the line
			if (!pstr_end)
				pstr_end = endl; // no value part
			// Parse key
			const char *str_at, *str_end;
			ParsePaddedString(pstr, pstr_end, str_at, str_end);
			pstr = pstr_end;
			if (str_end == str_at)
				continue; // inappropriate data or empty string
			// Create an item and parse value, if any
			StrPos keypos(str_at - cstr, str_end - cstr);
			StrPos valpos(0, 0);
			size_t sep_at = String::NoIndex;
			if (pstr != endl) {
				sep_at = pstr - cstr;
				ParsePaddedString(++pstr, endl, str_at, str_end);
				valpos.first = str_at - cstr;
				valpos.second = str_end - cstr;
			}
			cur_section->InsertItem(cur_section->End(), ItemDef(line, keypos, valpos, sep_at));
		}
	} while (!reader.EOS());

	reader.ReleaseStream();
}

void IniFile::Write(Stream *out) const {
	TextStreamWriter writer(out);
	for (ConstSectionIterator sec = _sections.begin(); sec != _sections.end(); ++sec) {
		if (sec != _sections.begin()) // do not write global section's name
			writer.WriteLine(sec->GetLine());
		for (ConstItemIterator item = sec->CBegin(); item != sec->CEnd(); ++item)
			writer.WriteLine(item->GetLine());
	}
	writer.ReleaseStream();
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
