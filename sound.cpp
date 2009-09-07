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

#include "engines/stark/sound.h"
#include "engines/stark/adpcm.h"

#include "engines/stark/archive.h"

#include "sound/audiostream.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/util.h"

namespace Stark {

ISS::ISS(Common::SeekableReadStream *str){
	Common::String codec = Common::String::emptyString;
	uint16 blockSize, channels, freq;
	uint32 size;
	
	codec = XARCArchive::readString(str);

		if (codec.equals("IMA_ADPCM_Sound")) {

			codec = XARCArchive::readString(str);
			blockSize = (uint16)strtol(codec.c_str(), 0, 10);
			
			XARCArchive::readString(str);
			// name ?

			XARCArchive::readString(str);
			// ?

			codec = XARCArchive::readString(str);
			channels = (uint16)strtol(codec.c_str(), 0, 10) + 1;

			XARCArchive::readString(str);
			// ?

			codec = XARCArchive::readString(str);
			freq = 44100 / (uint16)strtol(codec.c_str(), 0, 10);
			
			XARCArchive::readString(str);

			XARCArchive::readString(str);

			codec = XARCArchive::readString(str);
			size = (uint32)strtol(codec.c_str(), 0, 10);

			_stream = Stark::makeADPCMStream(str, true, size, Stark::kADPCMISS, freq, channels, blockSize);
		}

}

} // End of namespace Stark
