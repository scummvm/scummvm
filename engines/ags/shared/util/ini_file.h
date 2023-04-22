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

//=============================================================================
//
// IniFile class defines contents of the configuration file.
// It serves as a INI parser and plain enumerator of all the sections and items
// found in file, or, oppositely, as INI file constructor.
// But is not much suitable for regular key/value lookup. It is suggested to
// create a proper map to store items, from IniFile contents.
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_INIFILE_H
#define AGS_SHARED_UTIL_INIFILE_H

#include "ags/lib/std/list.h"
#include "ags/lib/std/utility.h"
#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class IniFile {
public:
	// Position of a string in the line of text:
	// is defined by a pair of first and next-after-last character indices
	typedef std::pair<size_t, size_t> StrPos;
	// Location of section in the array of text lines:
	// is defined by a pair of first and next-after-last line indices
	typedef std::pair<size_t, size_t> SectionPos;

	inline static bool IsValidStrPos(const StrPos &pos) {
		return pos.first < pos.second;
	}

	// Item definition
	// Valid key indicates a key-value line; no key means unparsed
	// line of text, e.g. comment or incorrectly formatted item.
	class ItemDef {
	public:
		ItemDef(const String &key, const String &value);
		ItemDef(const String &line, const StrPos &key, const StrPos &value, size_t sep_at);
		String GetLine()  const {
			return Line;
		}
		String GetKey()   const {
			return SubString(Line, Key);
		}
		String GetValue() const {
			return SubString(Line, Value);
		}
		// Tells if this is a valid key/value item, which means that it has a valid key
		bool IsKeyValue() const {
			return IsValidStrPos(Key);
		}
		void SetKey(const String &key);
		void SetValue(const String &value);

	private:
		String  Line;  // actual text
		StrPos  Key;   // position of item key
		size_t  SepAt; // position of the separator (assignment) symbol
		StrPos  Value; // position of item value
	};
	// Linked list of items
	typedef std::list<ItemDef> LItems;
	typedef LItems::iterator          ItemIterator;
	typedef LItems::const_iterator    ConstItemIterator;

	// Section definition
	class SectionDef {
	public:
		SectionDef(const String &name);
		SectionDef(const String &line, const StrPos &name);
		String GetLine() const {
			return Header;
		}
		String GetName() const {
			return SubString(Header, Name);
		}
		size_t GetItemCount() const {
			return Items.size();
		}
		// Tells if this is a "global" section, which means that it has no name
		bool IsGlobal() const {
			return !IsValidStrPos(Name);
		}
		ItemIterator Begin() {
			return Items.begin();
		}
		ItemIterator End() {
			return Items.end();
		}
		ConstItemIterator CBegin() const {
			return Items.begin();
		}
		ConstItemIterator CEnd()   const {
			return Items.end();
		}
		void SetName(const String &sec_name);
		void Clear();
		ItemIterator InsertItem(ItemIterator item, const ItemDef &itemdef);
		void EraseItem(ItemIterator item);

	private:
		String      Header;// section's heading line
		StrPos      Name;  // location of section name in the header line
		LItems      Items; // linked list of items belonging to the section
	};

	// Linked list of sections
	typedef std::list<SectionDef>     LSections;
	typedef LSections::iterator       SectionIterator;
	typedef LSections::const_iterator ConstSectionIterator;

private:
	inline static String SubString(const String &line, const StrPos &pos) {
		return line.Mid(pos.first, pos.second - pos.first);
	}

public:
	IniFile();

	SectionIterator Begin() {
		return _sections.begin();
	}
	SectionIterator End() {
		return _sections.end();
	}
	ConstSectionIterator CBegin() const {
		return _sections.begin();
	}
	ConstSectionIterator CEnd()   const {
		return _sections.end();
	}

	void Read(Stream *in);
	void Write(Stream *out) const;

	// Return number of sections
	size_t GetSectionCount() const {
		return _sections.size();
	}
	// Insert new item *before* existing item
	ItemIterator InsertItem(SectionIterator sec, ItemIterator item, const String &key, const String &value);
	// Insert new section *before* existing section
	SectionIterator InsertSection(SectionIterator sec, const String &name);
	// Remove a single item
	void RemoveItem(SectionIterator sec, ItemIterator item);
	// Completely remove whole section; this removes all lines between section
	// header and the last item found in that section (inclusive).
	void RemoveSection(SectionIterator sec);

private:
	LSections _sections;
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
