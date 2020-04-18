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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#ifndef CGE2_BITMAP_H
#define CGE2_BITMAP_H

#include "cge2/general.h"
#include "common/file.h"

namespace CGE2 {

class CGE2Engine;
class EncryptedStream;
class V2D;

#define kMaxPath      128

enum {
	kBmpEOI = 0x0000,
	kBmpSKP = 0x4000,
	kBmpREP = 0x8000,
	kBmpCPY = 0xC000
};

#include "common/pack-start.h"

struct HideDesc {
	uint16 _skip;
	uint16 _hide;
};

#include "common/pack-end.h"

class Bitmap {
	CGE2Engine *_vm;

	Common::String setExtension(const Common::String &str, const Common::String &ext);
	bool loadVBM(EncryptedStream *f);
public:
	uint16 _w;
	uint16 _h;
	uint8 *_v;
	int32 _map;
	HideDesc *_b;

	Bitmap();
	Bitmap(CGE2Engine *vm, const char *fname);
	Bitmap(CGE2Engine *vm, uint16 w, uint16 h, uint8 *map);
	Bitmap(CGE2Engine *vm, uint16 w, uint16 h, uint8 fill);
	Bitmap(CGE2Engine *vm, const Bitmap &bmp);
	~Bitmap();

	void setVM(CGE2Engine *vm);
	Bitmap *code(uint8 *map);
	Bitmap &operator=(const Bitmap &bmp);
	void release();
	void hide(V2D pos);
	void show(V2D pos);
	bool solidAt(V2D pos);
	void xLatPos(V2D &p);

	static uint8 *makeSpeechBubbleTail(int des, uint8 colorSet[][4]);
};


typedef Bitmap *BitmapPtr;

} // End of namespace CGE2

#endif // CGE2_BITMAP_H
