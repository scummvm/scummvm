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

#ifndef DGDS_SOUND_RESOURCE_SCI_RESOURCE_H
#define DGDS_SOUND_RESOURCE_SCI_RESOURCE_H

#include "common/str.h"
#include "common/list.h"
#include "common/hashmap.h"

#include "dgds/dgds.h"
#include "dgds/sound/scispan.h"

namespace Common {
class File;
class FSList;
class FSNode;
class WriteStream;
class SeekableReadStream;
}

namespace Dgds {

/** Class for storing resources in memory */
class SciResource : public SciSpan<const byte> {
public:
	SciResource(const byte *data, int dataSz, int16 id);
	~SciResource();
	void unalloc();
};

class SoundResource {
public:
	struct Channel {
		byte number;
		byte flags;
		byte poly;
		uint16 prio;
		SciSpan<const byte> data;
		uint16 curPos;
		long time;
		byte prev;

		Channel() :
			number(0),
			flags(0),
			poly(0),
			prio(0),
			data(),
			curPos(0) {
			time = 0;
			prev = 0;
		}
	};

	struct Track {
		byte type;
		byte channelCount;
		SciSpan<const byte> header;
		Channel *channels;
		int16 digitalChannelNr;
		uint16 digitalSampleRate;
		uint16 digitalSampleSize;
		uint16 digitalSampleStart;
		uint16 digitalSampleEnd;
	};
public:
	SoundResource(uint32 resNumberu, const byte *data, int dataSz);
	~SoundResource();

	Track *getTrackByType(byte type);
	Track *getDigitalTrack();

	byte getSoundPriority() const { return _soundPriority; }
	bool exists() const { return _resource != nullptr; }

private:
	int _trackCount;
	Track *_tracks;
	SciResource *_resource;
	byte _soundPriority;
};

} // End of namespace Dgds

#endif // DGDS_SOUND_RESOURCE_SCI_RESOURCE_H
