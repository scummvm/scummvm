/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SOUND_ADPCM_H
#define SOUND_ADPCM_H

#include "common/sys.h"
#include "common/stream.h"

#include "sound/audiostream.h"

namespace Stark {

class Audio::AudioStream;

// There are several types of ADPCM encoding, only some are supported here
// For all the different encodings, refer to:
// http://wiki.multimedia.cx/index.php?title=Category:ADPCM_Audio_Codecs
// Usually, if the audio stream we're trying to play has the FourCC header
// string intact, it's easy to discern which encoding is used
enum typesADPCM {
	kADPCMOki,		// Dialogic/Oki ADPCM (aka VOX)
	kADPCMMSIma,	// Microsoft IMA ADPCM
	kADPCMMS,		// Microsoft ADPCM
	kADPCMTinsel4,	// 4-bit ADPCM used by the Tinsel engine
	kADPCMTinsel6,	// 6-bit ADPCM used by the Tinsel engine
	kADPCMTinsel8,	// 8-bit ADPCM used by the Tinsel engine
	kADPCMIma,		// Standard IMA ADPCM
	kADPCMISS		// ISS IMA ADPCM
};

/**
 * Takes an input stream containing ADPCM compressed sound data and creates
 * an AudioStream from that.
 *
 * @param stream			the SeekableReadStream from which to read the ADPCM data
 * @param disposeAfterUse	whether to delete the stream after use
 * @param size				how many bytes to read from the stream (0 = all)
 * @param type				the compression type used
 * @param rate				the sampling rate (default = 22050)
 * @param channels			the number of channels (default = 2)
 * @param blockAlign		block alignment ??? (default = 0)
 * @param numLoop		how many types the sounds should loop, 0 for infinite loop (default = 1)
 * @return	a new AudioStream, or NULL, if an error occured
 */
Audio::AudioStream *makeADPCMStream(
	Common::SeekableReadStream *stream,
	bool disposeAfterUse,
	uint32 size,
	typesADPCM type,
	int rate = 22050,
	int channels = 2,
	uint32 blockAlign = 0,
	uint numLoops = 1);

} // End of namespace Audio

#endif
