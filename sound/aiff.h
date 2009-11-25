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

/**
 * @file
 * Sound decoder used in engines:
 *  - saga
 *  - sword1
 */

#ifndef SOUND_AIFF_H
#define SOUND_AIFF_H

#include "common/scummsys.h"

namespace Common { class SeekableReadStream; }

namespace Audio {

class AudioStream;

/**
 * Try to load an AIFF from the given seekable stream. Returns true if
 * successful. In that case, the stream's seek position will be set to the
 * start of the audio data, and size, rate and flags contain information
 * necessary for playback. Currently this function only supports uncompressed
 * raw PCM data as well as IMA ADPCM.
 */
extern bool loadAIFFFromStream(Common::SeekableReadStream &stream, int &size, int &rate, byte &flags);

/**
 * Try to load an AIFF from the given seekable stream and create an AudioStream
 * from that data.
 *
 * This function uses loadAIFFFromStream() internally.
 */
AudioStream *makeAIFFStream(Common::SeekableReadStream &stream);

} // End of namespace Audio

#endif
