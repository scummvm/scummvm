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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef COMMON_PE_EXE_H
#define COMMON_PE_EXE_H

#include "common/array.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace Common {

class SeekableReadStream;
class String;

class PEResourceID {
public:
	PEResourceID() { _idType = kIDTypeNull; }
	PEResourceID(String x) { _idType = kIDTypeString; _name = x; }
	PEResourceID(uint32 x) { _idType = kIDTypeNumerical; _id = x; }

	PEResourceID &operator=(String string);
	PEResourceID &operator=(uint32 x);

	bool operator==(const String &x) const;
	bool operator==(const uint32 &x) const;
	bool operator==(const PEResourceID &x) const;

	String getString() const;
	uint32 getID() const;
	String toString() const;

private:
	/** An ID Type. */
	enum IDType {
		kIDTypeNull,      ///< No type set
		kIDTypeNumerical, ///< A numerical ID.
		kIDTypeString     ///< A string ID.
	} _idType;

	String _name;         ///< The resource's string ID.
	uint32 _id;           ///< The resource's numerical ID.
};

struct PEResourceID_Hash {
	uint operator()(const PEResourceID &id) const { return hashit(id.toString()); }
};

struct PEResourceID_EqualTo {
	bool operator()(const PEResourceID &id1, const PEResourceID &id2) const { return id1 == id2; }
};

/**
 * A class able to load resources from a Windows Portable Executable, such
 * as cursors, bitmaps, and sounds.
 */
class PEResources {
public:
	PEResources();
	~PEResources();

	/** Clear all information. */
	void clear();

	/** Load from an EXE file. */
	bool loadFromEXE(const String &fileName);

	/** Load from a stream. */
	bool loadFromEXE(SeekableReadStream *stream);

	/** Return a list of resource types. */
	const Array<PEResourceID> getTypeList() const;

	/** Return a list of names for a given type. */
	const Array<PEResourceID> getNameList(const PEResourceID &type) const;

	/** Return a list of languages for a given type and name. */
	const Array<PEResourceID> getLangList(const PEResourceID &type, const PEResourceID &name) const;

	/** Return a stream to the specified resource (or 0 if non-existent). */
	SeekableReadStream *getResource(const PEResourceID &type, const PEResourceID &name, const PEResourceID &lang);

private:
	struct Section {
		uint32 virtualAddress;
		uint32 size;
		uint32 offset;
	};

	HashMap<String, Section, IgnoreCase_Hash, IgnoreCase_EqualTo> _sections;

	SeekableReadStream *_exe;

	void parseResourceLevel(Section &section, uint32 offset, int level);
	PEResourceID _curType, _curName, _curLang;

	struct Resource {
		uint32 offset;
		uint32 size;
	};
	
	typedef HashMap<PEResourceID, Resource, PEResourceID_Hash, PEResourceID_EqualTo> LangMap;
	typedef HashMap<PEResourceID,  LangMap, PEResourceID_Hash, PEResourceID_EqualTo> NameMap;
	typedef HashMap<PEResourceID,  NameMap, PEResourceID_Hash, PEResourceID_EqualTo> TypeMap;

	TypeMap _resources;
};

} // End of namespace Common

#endif
