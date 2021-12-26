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

#ifndef COMMON_WINEXE_PE_H
#define COMMON_WINEXE_PE_H

#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "common/winexe.h"

namespace Common {

/**
 * @defgroup common_winexe_ne Windows Portable Executable resources
 * @ingroup common_winexe
 *
 * @brief API for managing Windows Portable Executable resources.
 *
 * @{
 */

template<class T> class Array;
class SeekableReadStream;

/**
 * A class able to load resources from a Windows Portable Executable, such
 * as cursors, bitmaps, and sounds.
 */
class PEResources : public WinResources {
public:
	PEResources();
	~PEResources();

	/** Clear all information. */
	void clear();

	/** Load from an EXE file. */
	using WinResources::loadFromEXE;

	/** Load from a stream. */
	bool loadFromEXE(SeekableReadStream *stream, DisposeAfterUse::Flag disposeFileHandle = DisposeAfterUse::YES);

	/** Return a list of resource types. */
	const Array<WinResourceID> getTypeList() const;

	/** Return a list of IDs for a given type. */
	const Array<WinResourceID> getIDList(const WinResourceID &type) const;

	/** Return a list of languages for a given type and ID. */
	const Array<WinResourceID> getLangList(const WinResourceID &type, const WinResourceID &id) const;

	/** Return a stream to the specified resource, taking the first language found (or 0 if non-existent). */
	SeekableReadStream *getResource(const WinResourceID &type, const WinResourceID &id);

	/** Return a stream to the specified resource (or 0 if non-existent). */
	SeekableReadStream *getResource(const WinResourceID &type, const WinResourceID &id, const WinResourceID &lang);

	/** Get a string from a string resource. */
	String loadString(uint32 stringID);

protected:
	VersionInfo *parseVersionInfo(SeekableReadStream *stream);

private:
	struct Section {
		uint32 virtualAddress;
		uint32 size;
		uint32 offset;
	};

	HashMap<String, Section, IgnoreCase_Hash, IgnoreCase_EqualTo> _sections;

	SeekableReadStream *_exe;
	DisposeAfterUse::Flag _disposeFileHandle;

	void parseResourceLevel(Section &section, uint32 offset, int level);
	WinResourceID _curType, _curID, _curLang;

	struct Resource {
		uint32 offset;
		uint32 size;
	};

	typedef HashMap<WinResourceID, Resource, WinResourceID_Hash, WinResourceID_EqualTo> LangMap;
	typedef HashMap<WinResourceID,  LangMap, WinResourceID_Hash, WinResourceID_EqualTo> IDMap;
	typedef HashMap<WinResourceID,    IDMap, WinResourceID_Hash, WinResourceID_EqualTo> TypeMap;

	TypeMap _resources;
};

/** @} */

} // End of namespace Common

#endif
