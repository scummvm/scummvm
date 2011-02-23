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

#ifndef COMMON_WINEXE_NE_H
#define COMMON_WINEXE_NE_H

#include "common/array.h"
#include "common/list.h"
#include "common/winexe.h"

namespace Common {

class MemoryReadStream;
class SeekableReadStream;
class String;

/** A New Executable cursor. */
class NECursor {
public:
	NECursor();
	~NECursor();

	/** Return the cursor's width. */
	uint16 getWidth() const;
	/** Return the cursor's height. */
	uint16 getHeight() const;
	/** Return the cursor's hotspot's x coordinate. */
	uint16 getHotspotX() const;
	/** Return the cursor's hotspot's y coordinate. */
	uint16 getHotspotY() const;

	const byte *getSurface() const { return _surface; }
	const byte *getPalette() const { return _palette; }

	/** Set the cursor's dimensions. */
	void setDimensions(uint16 width, uint16 height);
	/** Set the cursor's hotspot. */
	void setHotspot(uint16 x, uint16 y);

	/** Read the cursor's data out of a stream. */
	bool readCursor(SeekableReadStream &stream, uint32 count);

private:
	byte *_surface;
	byte _palette[256 * 3];

	uint16 _width;    ///< The cursor's width.
	uint16 _height;   ///< The cursor's height.
	uint16 _hotspotX; ///< The cursor's hotspot's x coordinate.
	uint16 _hotspotY; ///< The cursor's hotspot's y coordinate.

	/** Clear the cursor. */
	void clear();
};

/** A New Executable cursor group. */
struct NECursorGroup {
	WinResourceID id;
	Array<NECursor *> cursors; ///< The cursors.
};

/** The default Windows resources. */
enum NEResourceType {
	kNECursor = 0x8001,
	kNEBitmap = 0x8002,
	kNEIcon = 0x8003,
	kNEMenu = 0x8004,
	kNEDialog = 0x8005,
	kNEString = 0x8006,
	kNEFontDir = 0x8007,
	kNEFont = 0x8008,
	kNEAccelerator = 0x8009,
	kNERCData = 0x800A,
	kNEMessageTable = 0x800B,
	kNEGroupCursor = 0x800C,
	kNEGroupIcon = 0x800D,
	kNEVersion = 0x8010,
	kNEDlgInclude = 0x8011,
	kNEPlugPlay = 0x8013,
	kNEVXD = 0x8014,
	kNEAniCursor = 0x8015,
	kNEAniIcon = 0x8016,
	kNEHTML = 0x8017,
	kNEManifest = 0x8018
};

/**
 * A class able to load resources from a Windows New Executable, such
 * as cursors, bitmaps, and sounds.
 *
 * See http://en.wikipedia.org/wiki/New_Executable for more info.
 */
class NEResources {
public:
	NEResources();
	~NEResources();

	/** Clear all information. */
	void clear();

	/** Load from an EXE file. */
	bool loadFromEXE(const String &fileName);

	/** Load from a Windows compressed EXE file. */
	bool loadFromCompressedEXE(const String &fileName);

	/** Load from a stream. */
	bool loadFromEXE(SeekableReadStream *stream);

	/** Get all cursor's read from the New Executable. */
	const Array<NECursorGroup> &getCursors() const;

	/** Return a list of resources for a given type. */
	const Array<WinResourceID> getIDList(uint16 type) const;

	/** Return a stream to the specified resource (or 0 if non-existent). */
	SeekableReadStream *getResource(uint16 type, WinResourceID id);

private:
	/** A resource. */
	struct Resource {
		WinResourceID id;

		uint16 type; ///< Type of the resource.

		uint32 offset; ///< Offset within the EXE.
		uint32 size;   ///< Size of the data.

		uint16 flags;
		uint16 handle;
		uint16 usage;
	};

	SeekableReadStream *_exe;        ///< Current file.

	/** All resources. */
	List<Resource> _resources;

	/** All cursor resources. */
	Array<NECursorGroup> _cursors;

	/** Read the offset to the resource table. */
	uint32 getResourceTableOffset();
	/** Read the resource table. */
	bool readResourceTable(uint32 offset);

	// Cursor reading helpers
	bool readCursors();
	bool readCursorGroup(NECursorGroup &group, const Resource &resource);
	bool readCursor(NECursor &cursor, const Resource &resource, uint32 size);

	/** Find a specific resource. */
	const Resource *findResource(uint16 type, WinResourceID id) const;

	/** Read a resource string. */
	static String getResourceString(SeekableReadStream &exe, uint32 offset);
};

} // End of namespace Common

#endif
