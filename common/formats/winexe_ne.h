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

#ifndef COMMON_WINEXE_NE_H
#define COMMON_WINEXE_NE_H

#include "common/formats/winexe.h"
#include "common/list.h"
#include "common/str.h"

namespace Common {

/**
 * @defgroup common_winexe_ne Windows New Executable resources
 * @ingroup common_winexe
 *
 * @brief API for managing Windows New Executable resources.
 *
 * @{
 */

template<class T>
class Array;
class SeekableReadStream;

// NE executable segment
struct NESegment {
	static constexpr uint16 kFlagSingleData = 1;
	static constexpr uint16 kFlagMultipleData = 2;
	static constexpr uint16 kFlagLinkError = 0x2000u;
	static constexpr uint16 kFlagLibModule = 0x8000u;

	uint32 offset;
	uint32 size;
	uint16 flags;
};

/**
 * A class able to load resources from a Windows New Executable, such
 * as cursors, bitmaps, and sounds.
 *
 * See https://en.wikipedia.org/wiki/New_Executable for more info.
 */
class NEResources : public WinResources {
public:
	NEResources();
	~NEResources();

	static constexpr uint32 kInvalidAddress = 0xFFFFFFFF;

	/** Clear all information. */
	void clear() override;

	/** Load from an EXE file. */
	using WinResources::loadFromEXE;

	/** Load from a stream. */
	bool loadFromEXE(SeekableReadStream *stream, DisposeAfterUse::Flag disposeFileHandle = DisposeAfterUse::YES) override;

	/** Return a list of resources for a given type. */
	const Array<WinResourceID> getIDList(const WinResourceID &type) const override;

	/** Return a stream to the specified resource (or 0 if non-existent). */
	SeekableReadStream *getResource(const WinResourceID &type, const WinResourceID &id) override;

	/** Get a string from a string resource. */
	String loadString(uint32 stringID) override;

	/** Get all NE segments */
	const Array<NESegment> &getSegments() const { return _segments; }

	/* Index of NE automatic segment */
	uint32 getAutomaticSegmentIndex() const { return _autoSegmentIndex; }

protected:
	VersionInfo *parseVersionInfo(SeekableReadStream *stream) override;

private:
	/** A resource. */
	struct Resource {
		WinResourceID id;

		WinResourceID type; ///< Type of the resource.

		uint32 offset; ///< Offset within the EXE.
		uint32 size;   ///< Size of the data.

		uint16 flags;
		uint16 handle;
		uint16 usage;
	};

	SeekableReadStream *_exe; ///< Current file.
	DisposeAfterUse::Flag _disposeFileHandle;

	uint32 _autoSegmentIndex = 0;
	Array<NESegment> _segments;

	/** All resources. */
	List<Resource> _resources;

	typedef HashMap<WinResourceID, Common::String, WinResourceID_Hash, WinResourceID_EqualTo> IDMap;
	typedef HashMap<WinResourceID, IDMap, WinResourceID_Hash, WinResourceID_EqualTo> TypeMap;
	TypeMap _nameTable;

	/** Read the offset to the resource table. */
	uint32 getResourceTableOffset();
	/** Read the resource table. */
	bool readResourceTable(uint32 offset);

	/** Read the name table. */
	bool readNameTable(uint32 offset, uint32 size);

	/** Find a specific resource. */
	const Resource *findResource(const WinResourceID &type, const WinResourceID &id) const;

	/** Read a resource string. */
	static String getResourceString(SeekableReadStream &exe, uint32 offset);
};

/** @} */

} // End of namespace Common

#endif
