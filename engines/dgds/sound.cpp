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

#include "common/debug.h"
#include "common/endian.h"

#include "dgds/sound.h"

namespace Dgds {

static inline
void readHeader(byte* &pos, uint32 &sci_header) {
	sci_header = 0;
	if (READ_LE_UINT16(pos) == 0x0084) sci_header = 2;

	pos += sci_header;
	if (pos[0] == 0xF0) {
		debug("SysEx transfer = %d bytes", pos[1]);
		pos += 2;
		pos += 6;
	}
}

static inline
void readPartHeader(byte* &pos, uint16 &off, uint16 &siz) {
	pos += 2;
	off = READ_LE_UINT16(pos);
	pos += 2;
	siz = READ_LE_UINT16(pos);
	pos += 2;
}

static inline
void skipPartHeader(byte* &pos) {
	pos += 6;
}

uint32 availableSndTracks(byte *data, uint32 size) {
	byte *pos = data;

        uint32 sci_header;
	readHeader(pos, sci_header);

	uint32 tracks = 0;
	while (pos[0] != 0xFF) {
		byte drv = *pos++;

		debug("(%d)", drv);

		while (pos[0] != 0xFF) {
			uint16 off, siz;
			readPartHeader(pos, off, siz);
			off += sci_header;

			debug("%06d:%d ", off, siz);

			debug("Header bytes");
			debug("[%06X]  ", data[off]);
			debug("[%02X]  ", data[off+0]);
			debug("[%02X]  ", data[off+1]);

			bool digital_pcm = false;
			if (READ_LE_UINT16(&data[off]) == 0x00FE) {
				digital_pcm = true;
			}

			switch (drv) {
			case 0:	if (digital_pcm) {
					debug("- Soundblaster");
					tracks |= DIGITAL_PCM;
				} else {
					debug("- Adlib");
					tracks |= TRACK_ADLIB;
				}					break;
			case 7:		debug("- General MIDI");
					tracks |= TRACK_GM;		break;
			case 9:		debug("- CMS");			break;
			case 12:	debug("- MT-32");
					tracks |= TRACK_MT32;		break;
			case 18:	debug("- PC Speaker");		break;
			case 19:	debug("- Tandy 1000");		break;
			default:	debug("- Unknown %d", drv);	break;
			}
		}

		pos++;
	}
	pos++;
	return tracks;
}

byte loadSndTrack(uint32 track, byte** trackPtr, uint16* trackSiz, byte *data, uint32 size) {
	byte matchDrv;
	switch (track) {
	case DIGITAL_PCM:
	case TRACK_ADLIB: matchDrv = 0;    break;
	case TRACK_GM:	  matchDrv = 7;    break;
	case TRACK_MT32:  matchDrv = 12;   break;
	default:			   return 0;
	}

	byte *pos = data;

	uint32 sci_header;
	readHeader(pos, sci_header);

	while (pos[0] != 0xFF) {
		byte drv = *pos++;

		byte part;
		byte *ptr;

		part = 0;
		for (ptr = pos; *ptr != 0xFF; skipPartHeader(ptr))
			part++;

		if (matchDrv == drv) {
			part = 0;
			while (pos[0] != 0xFF) {
				uint16 off, siz;
				readPartHeader(pos, off, siz);
				off += sci_header;

				trackPtr[part] = data + off;
				trackSiz[part] = siz;
				part++;
			}
			debug("- (%d) Play parts = %d", drv, part);
			return part;
		} else {
			pos = ptr;
		}
		pos++;
	}
	pos++;
	return 0;
}

} // End of namespace Dgds

