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
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included VGMTrans_LICENSE.txt file
 */
#ifndef AUDIO_SOUNDFONT_VAB_H
#define AUDIO_SOUNDFONT_VAB_H

#include "audio/soundfont/common.h"
#include "common/str.h"
#include "audio/soundfont/vgminstrset.h"
#include "audio/soundfont/vgmsamp.h"

class Vab : public VGMInstrSet {
public:
	Vab(RawFile *file, uint32 offset);
	virtual ~Vab(void);

	virtual bool GetHeaderInfo();
	virtual bool GetInstrPointers();
};

// ********
// VabInstr
// ********

class VabInstr : public VGMInstr {
public:
	VabInstr(VGMInstrSet *instrSet, uint32 offset, uint32 length, uint32 theBank,
			 uint32 theInstrNum, const Common::String &name = "Instrument");
	virtual ~VabInstr();

	virtual bool LoadInstr();

public:
	uint8 _tones;
	uint8 _masterVol;
};

// ******
// VabRgn
// ******

class VabRgn : public VGMRgn {
public:
	VabRgn(VabInstr *instr, uint32 offset);

	virtual bool LoadRgn();

public:
	uint16 _ADSR1;  // raw ps2 ADSR1 value (articulation data)
	uint16 _ADSR2;  // raw ps2 ADSR2 value (articulation data)
};

#endif // AUDIO_SOUNDFONT_VAB_H
