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

#include "common/list.h"
#include "common/str.h"
#include "common/winexe.h"

namespace Common {

/**
 * @defgroup common_winexe_ne Windows New Executable resources
 * @ingroup common_winexe
 *
 * @brief API for managing Windows New Executable resources.
 *
 * @{
 */

template<class T> class Array;
class SeekableReadStream;

/**
 * A class able to load resources from a Windows New Executable, such
 * as cursors, bitmaps, and sounds.
 *
 * See http://en.wikipedia.org/wiki/New_Executable for more info.
 */
class NEResources : public WinResources {
public:
	NEResources();
	~NEResources();

	/** Clear all information. */
	void clear();

	/** Load from an EXE file. */
	using WinResources::loadFromEXE;

	/** Load from a stream. */
	bool loadFromEXE(SeekableReadStream *stream, DisposeAfterUse::Flag disposeFileHandle = DisposeAfterUse::YES);

	/** Return a list of resources for a given type. */
	const Array<WinResourceID> getIDList(const WinResourceID &type) const;

	/** Return a stream to the specified resource (or 0 if non-existent). */
	SeekableReadStream *getResource(const WinResourceID &type, const WinResourceID &id);

	/** Get a string from a string resource. */
	String loadString(uint32 stringID);

protected:
	VersionInfo *parseVersionInfo(SeekableReadStream *stream);

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

	SeekableReadStream *_exe;        ///< Current file.
	DisposeAfterUse::Flag _disposeFileHandle;

	/** All resources. */
	List<Resource> _resources;

	/** Read the offset to the resource table. */
	uint32 getResourceTableOffset();
	/** Read the resource table. */
	bool readResourceTable(uint32 offset);

	/** Find a specific resource. */
	const Resource *findResource(const WinResourceID &type, const WinResourceID &id) const;

	/** Read a resource string. */
	static String getResourceString(SeekableReadStream &exe, uint32 offset);
};

/** @} */

} // End of namespace Common

#endif
