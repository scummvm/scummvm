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
 * $URL:https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/branches/gsoc2007-fsnode/sound/iff.h $
 * $Id:iff.h 26949 2007-05-26 20:23:24Z david_corrales $
 *
 */

#ifndef SOUND_IFF_H
#define SOUND_IFF_H

#include "common/iff_container.h"

namespace Audio {

struct Voice8Header {
	uint32	oneShotHiSamples;
	uint32	repeatHiSamples;
	uint32	samplesPerHiCycle;
	uint16	samplesPerSec;
	byte	octaves;
	byte	compression;
	uint32	volume;

	Voice8Header() {
		memset(this, 0, sizeof(Voice8Header));
	}
};


/*
	A8SVX decoder reads 8SVX subtype of IFF files.

	TODO: make a factory function for this kind of stream?
 */
class A8SVXDecoder : public Common::IFFParser {

protected:
	Voice8Header  	&_header;
	byte* 			&_data;
	uint32			&_dataSize;

protected:
	void readVHDR(Common::IFFChunk &chunk);
	void readBODY(Common::IFFChunk &chunk);

public:
	A8SVXDecoder(Common::ReadStream &input, Voice8Header &header, byte *&data, uint32 &dataSize);
	void decode();
};


/*
	TODO: Implement a parser for AIFF subtype.
 */

}

#endif
