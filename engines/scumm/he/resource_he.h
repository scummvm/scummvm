/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Parts of this code are based on:
 * icoutils - A set of programs dealing with MS Windows icons and cursors.
 * Copyright (C) 1998-2001 Oskar Liljeblad
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

#ifndef SCUMM_HE_RESOURCE_HE_H
#define SCUMM_HE_RESOURCE_HE_H

#include "common/macresman.h"
#include "common/pe_exe.h"

namespace Scumm {

class ScummEngine_v70he;

class ResExtractor {
public:
	ResExtractor(ScummEngine_v70he *scumm);
	virtual ~ResExtractor();

	void setCursor(int id);

	virtual int extractResource(int id, byte **buf) { return 0; }
	virtual int convertIcons(byte *data, int datasize, byte **cursor, int *w, int *h,
							 int *hotspot_x, int *hotspot_y, int *keycolor,
							 byte **palette, int *palSize) { return 0; }

	enum {
		MAX_CACHED_CURSORS = 10
	};

	struct CachedCursor {
		bool valid;
		int id;
		byte *bitmap;
		int w, h;
		int hotspot_x, hotspot_y;
		uint32 last_used;
		byte *palette;
		int palSize;
	};

	ScummEngine_v70he *_vm;

	ResExtractor::CachedCursor *findCachedCursor(int id);
	ResExtractor::CachedCursor *getCachedCursorSlot();

	bool _arg_raw;
	Common::String _fileName;
	CachedCursor _cursorCache[MAX_CACHED_CURSORS];
};

class Win32ResExtractor : public ResExtractor {
public:
	Win32ResExtractor(ScummEngine_v70he *scumm);
	~Win32ResExtractor() {}
	int extractResource(int id, byte **data);
	void setCursor(int id);
	int convertIcons(byte *data, int datasize, byte **cursor, int *w, int *h,
			 int *hotspot_x, int *hotspot_y, int *keycolor, byte **palette, int *palSize);

private:
	Common::PEResources _exe;

/*
 * Structures
 */

#include "common/pack-start.h"	// START STRUCT PACKING

	struct Win32BitmapInfoHeader {
		uint32 size;
		int32 width;
		int32 height;
		uint16 planes;
		uint16 bit_count;
		uint32 compression;
		uint32 size_image;
		int32 x_pels_per_meter;
		int32 y_pels_per_meter;
		uint32 clr_used;
		uint32 clr_important;
	} PACKED_STRUCT;

	struct Win32RGBQuad {
		byte blue;
		byte green;
		byte red;
		byte reserved;
	} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

	uint32 simple_vec(byte *data, uint32 ofs, byte size);
	void fix_win32_bitmap_info_header_endian(Win32BitmapInfoHeader *obj);
};

class MacResExtractor : public ResExtractor {

public:
	MacResExtractor(ScummEngine_v70he *scumm);
	~MacResExtractor() { }

private:
	Common::MacResManager *_resMgr;

	int extractResource(int id, byte **buf);
	int convertIcons(byte *data, int datasize, byte **cursor, int *w, int *h,
			 int *hotspot_x, int *hotspot_y, int *keycolor, byte **palette, int *palSize);
};

} // End of namespace Scumm

#endif
