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

// Resource library

#include "common/archive.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "common/memstream.h"

#include "dgds/sound/resource/sci_resource.h"
#include "dgds/sound/scispan.h"

namespace Dgds {


SoundResource::SoundResource(uint32 resourceNr, const byte *rawData, int dataSz) :
	_trackCount(0), _tracks(nullptr), _soundPriority(0xFF) {
	_resource = new SciResource(rawData, dataSz, resourceNr);

	Channel *channel;

	SciSpan<const byte> data = *_resource;
	// Count # of tracks
	_trackCount = 0;
	while ((*data++) != 0xFF) {
		_trackCount++;
		while (*data != 0xFF)
			data += 6;
		++data;
	}
	_tracks = new Track[_trackCount];
	data = *_resource;

	for (int trackNr = 0; trackNr < _trackCount; trackNr++) {
		// Track info starts with track type:BYTE
		// Then the channel information gets appended Unknown:WORD, ChannelOffset:WORD, ChannelSize:WORD
		// 0xFF:BYTE as terminator to end that track and begin with another track type
		// Track type 0xFF is the marker signifying the end of the tracks

		_tracks[trackNr].type = *data++;
		// Counting # of channels used
		SciSpan<const byte> data2 = data;
		byte channelCount = 0;
		while (*data2 != 0xFF) {
			data2 += 6;
			channelCount++;
		}
		_tracks[trackNr].channels = new Channel[channelCount];
		_tracks[trackNr].channelCount = 0;
		_tracks[trackNr].digitalChannelNr = -1; // No digital sound associated
		_tracks[trackNr].digitalSampleRate = 0;
		_tracks[trackNr].digitalSampleSize = 0;
		_tracks[trackNr].digitalSampleStart = 0;
		_tracks[trackNr].digitalSampleEnd = 0;
		if (_tracks[trackNr].type != 0xF0) { // Digital track marker - not supported currently
			int channelNr = 0;
			while (channelCount--) {
				channel = &_tracks[trackNr].channels[channelNr];
				const uint16 dataOffset = data.getUint16LEAt(2);

				if (dataOffset >= _resource->size()) {
					warning("Invalid offset inside sound resource %d: track %d, channel %d", resourceNr, trackNr, channelNr);
					data += 6;
					continue;
				}

				uint16 size = data.getUint16LEAt(4);

				if ((uint32)dataOffset + size > _resource->size()) {
					warning("Invalid size inside sound resource %d: track %d, channel %d", resourceNr, trackNr, channelNr);
					size = _resource->size() - dataOffset;
				}

				if (size == 0) {
					warning("Empty channel in sound resource %d: track %d, channel %d", resourceNr, trackNr, channelNr);
					data += 6;
					continue;
				}

				channel->data = _resource->subspan(dataOffset, size);

				channel->curPos = 0;
				channel->number = channel->data[0];

				channel->poly = channel->data[1] & 0x0F;
				channel->prio = channel->data[1] >> 4;
				channel->time = channel->prev = 0;
				channel->data += 2; // skip over header
				if (channel->number == 0xFE) { // Digital channel
					_tracks[trackNr].digitalChannelNr = channelNr;
					_tracks[trackNr].digitalSampleRate = channel->data.getUint16LEAt(0);
					_tracks[trackNr].digitalSampleSize = channel->data.getUint16LEAt(2);
					_tracks[trackNr].digitalSampleStart = channel->data.getUint16LEAt(4);
					_tracks[trackNr].digitalSampleEnd = channel->data.getUint16LEAt(6);
					channel->data += 8; // Skip over header
					channel->flags = 0;
				} else {
					channel->flags = channel->number >> 4;
					channel->number = channel->number & 0x0F;
					// Flag 1:	Channel start offset is 0 instead of 10 (currently: everything 0)
					//			Also: Don't map the channel to the device at all, but still play it.
					//			It doesn't stop other sounds playing sounds on that channel, it even
					//			allows other sounds to map to this channel (in that case the dontmap
					//			channel has limited access, it can't send control change, program
					//			change and pitch wheel messages.
					//			This is basically a marker for the channel as a "real" channel
					//			(used mostly for rhythm channels on devices that have one). These
					//			channels will also consequently start the parsing at offset 0 instead
					//			of 10: Normal channels would read the parameters of the first couple of
					//			events into the channel structs, but the "real" channels have to
					//			send these to the device right away, since they don't use the stored
					//			data.
					//			Very early games like KQ5 (but including the DOS CD version) and SQ2
					//			have support for this flag, only. It isn't even a flag there, since
					//			all these games do is check for a channel number below 0x10.
					//
					// Flag 2:	Don't remap the channel. It is placed in the map, but only in the
					//			exact matching slot of the channel number. All the other games except
					//			the very early ones use this flag to mark the rhythm channels. I
					//			haven't seen any usage of flag 1 in any of these games. They all use
					//			flag 2 instead, but still have full support of flag 1 in the code.
					//			Using this flag is really preferable, since there can't be conflicts
					//			with different sounds playing on the channel.
					//
					// Flag 4:	Start up muted. The channel won't be mapped (and thus, not have any
					//			output), until the mute gets removed.
				}
				_tracks[trackNr].channelCount++;
				channelNr++;
				data += 6;
			}
		} else {
			// The first byte of the 0xF0 track's channel list is priority
			_soundPriority = *data;

			// Skip over digital track
			data += 6;
		}
		++data; // Skipping 0xFF that closes channels list
	}
}

SoundResource::~SoundResource() {
	if (_tracks != nullptr) {
		for (int trackNr = 0; trackNr < _trackCount; trackNr++)
			delete[] _tracks[trackNr].channels;
		delete[] _tracks;
	}
}


SoundResource::Track *SoundResource::getTrackByType(byte type) {
	for (int trackNr = 0; trackNr < _trackCount; trackNr++) {
		if (_tracks[trackNr].type == type)
			return &_tracks[trackNr];
	}
	return nullptr;
}

SoundResource::Track *SoundResource::getDigitalTrack() {
	for (int trackNr = 0; trackNr < _trackCount; trackNr++) {
		if (_tracks[trackNr].digitalChannelNr != -1)
			return &_tracks[trackNr];
	}
	return nullptr;
}

} // End of namespace Dgds
