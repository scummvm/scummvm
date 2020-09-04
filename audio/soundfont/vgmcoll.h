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
#ifndef AUDIO_SOUNDFONT_VGMCOLL_H
#define AUDIO_SOUNDFONT_VGMCOLL_H

#include "common.h"
#include "common/array.h"

class VGMInstrSet;
class VGMSampColl;
class VGMSamp;
class SF2File;
class SynthFile;

class VGMColl {
public:
	SF2File *CreateSF2File(VGMInstrSet *theInstrSet);

private:
	SynthFile *CreateSynthFile(VGMInstrSet *theInstrSet);
	void UnpackSampColl(SynthFile &synthfile, VGMSampColl *sampColl,
						Common::Array<VGMSamp *> &finalSamps);
};

#endif // AUDIO_SOUNDFONT_VGMCOLL_H
