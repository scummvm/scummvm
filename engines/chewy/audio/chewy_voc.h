/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CHEWY_AUDIO_CHEWY_VOC_H
#define CHEWY_AUDIO_CHEWY_VOC_H

#include "audio/decoders/voc.h"

#include "common/endian.h"

namespace Chewy {

// This stream differs from the standard VOC stream on 2 points:
// - VOC data header is not present, so not processed.
// - Some VOC blocks contain non-standard headers. These are removed because
//   otherwise they will be interpreted as audio data and cause static.
class ChewyVocStream : public Audio::VocStream {
protected:
	static const uint32 FOURCC_SCRS = MKTAG('S', 'C', 'R', 'S');
	static const uint32 FOURCC_RIFF = MKTAG('R', 'I', 'F', 'F');

public:
	ChewyVocStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse);

protected:
	void removeHeaders();
};

} // End of namespace Audio

#endif
