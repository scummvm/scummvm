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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/sound/decoders/adpcm.h $
 * $Id: adpcm.h 55381 2011-01-21 14:37:46Z mthreepwood $
 *
 */

/**
 * @file
 * Sound decoder used in engines:
 *  - agos
 *  - lastexpress
 *  - mohawk
 *  - saga
 *  - scumm
 *  - tinsel
 */

#ifndef SOUND_ADPCM_H
#define SOUND_ADPCM_H

//#include "common/scummsys.h"
#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/sound/mixer/types.h"
#include "decoder/wave/stream.h"


namespace Audio {

class AudioStream;
class RewindableAudioStream;

// There are several types of ADPCM encoding, only some are supported here
// For all the different encodings, refer to:
// http://wiki.multimedia.cx/index.php?title=Category:ADPCM_Audio_Codecs
// Usually, if the audio stream we're trying to play has the FourCC header
// string intact, it's easy to discern which encoding is used
enum typesADPCM {
	kADPCMOki,                 // Dialogic/Oki ADPCM (aka VOX)
	kADPCMMSIma,               // Microsoft IMA ADPCM
	kADPCMMSImaLastExpress,    // Microsoft IMA ADPCM (with inverted samples)
	kADPCMMS,                  // Microsoft ADPCM
	kADPCMTinsel4,             // 4-bit ADPCM used by the Tinsel engine
	kADPCMTinsel6,             // 6-bit ADPCM used by the Tinsel engine
	kADPCMTinsel8,             // 8-bit ADPCM used by the Tinsel engine
	kADPCMIma,                 // Standard IMA ADPCM
	kADPCMApple,               // Apple QuickTime IMA ADPCM
	kADPCMDK3                  // Duck DK3 IMA ADPCM
};

/**
 * Takes an input stream containing ADPCM compressed sound data and creates
 * an RewindableAudioStream from that.
 *
 * @param stream            the SeekableReadStream from which to read the ADPCM data
 * @param disposeAfterUse   whether to delete the stream after use
 * @param size              how many bytes to read from the stream (0 = all)
 * @param type              the compression type used
 * @param rate              the sampling rate
 * @param channels          the number of channels
 * @param blockAlign        block alignment ???
 * @return   a new RewindableAudioStream, or NULL, if an error occurred
 */
RewindableAudioStream *makeADPCMStream(
    Common::SeekableReadStream *stream,
    DisposeAfterUse::Flag disposeAfterUse,
    uint32 size, typesADPCM type,
    int rate = 22050,
    int channels = 2,
    uint32 blockAlign = 0);

} // End of namespace Audio

#endif
