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

#ifndef COMMON_WINEXE_H
#define COMMON_WINEXE_H

#include "file.h"
#include "hash-str.h"
#include "common/str.h"

namespace Common {

/** The default Windows resources. */
enum WinResourceType {
	kWinCursor       = 0x01,
	kWinBitmap       = 0x02,
	kWinIcon         = 0x03,
	kWinMenu         = 0x04,
	kWinDialog       = 0x05,
	kWinString       = 0x06,
	kWinFontDir      = 0x07,
	kWinFont         = 0x08,
	kWinAccelerator  = 0x09,
	kWinRCData       = 0x0A,
	kWinMessageTable = 0x0B,
	kWinGroupCursor  = 0x0C,
	kWinGroupIcon    = 0x0E,
	kWinVersion      = 0x10,
	kWinDlgInclude   = 0x11,
	kWinPlugPlay     = 0x13,
	kWinVXD          = 0x14,
	kWinAniCursor    = 0x15,
	kWinAniIcon      = 0x16,
	kWinHTML         = 0x17,
	kWinManifest     = 0x18
};

class WinResourceID {
public:
	WinResourceID() { _idType = kIDTypeNull; }
	WinResourceID(String x) { _idType = kIDTypeString; _name = x; }
	WinResourceID(uint32 x) { _idType = kIDTypeNumerical; _id = x; }

	WinResourceID &operator=(const String &x);
	WinResourceID &operator=(uint32 x);

	bool operator==(const String &x) const;
	bool operator==(const uint32 &x) const;
	bool operator==(const WinResourceID &x) const;

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

struct WinResourceID_Hash {
	uint operator()(const WinResourceID &id) const { return hashit(id.toString()); }
};

struct WinResourceID_EqualTo {
	bool operator()(const WinResourceID &id1, const WinResourceID &id2) const { return id1 == id2; }
};

/**
 * A class able to load resources from a Windows Executable, such
 * as cursors, bitmaps, and sounds.
 */
class WinResources {
public:
	virtual ~WinResources() {}

	/** Clear all information. */
	virtual void clear() = 0;

	/** Load from an EXE file. */
	virtual bool loadFromEXE(const String &fileName);

	virtual bool loadFromEXE(File *stream) = 0;

	/** Return a list of IDs for a given type. */
	virtual const Array<WinResourceID> getIDList(const WinResourceID &type) const = 0;

	/** Return a list of languages for a given type and ID. */
	virtual const Array<WinResourceID> getLangList(const WinResourceID &type, const WinResourceID &id) const {
		Array<WinResourceID> array;
		return array;
	};

	/** Return a stream to the specified resource, taking the first language found (or 0 if non-existent). */
	virtual File *getResource(const WinResourceID &type, const WinResourceID &id) = 0;

	/** Return a stream to the specified resource (or 0 if non-existent). */
	virtual File *getResource(const WinResourceID &type, const WinResourceID &id, const WinResourceID &lang) {
		return getResource(type, id);
	}
};

} // End of namespace Common

#endif
