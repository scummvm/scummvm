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

#include "glk/magnetic/magnetic.h"

namespace Glk {
namespace Magnetic {

byte Magnetic::init_snd(size_t size) {
	if (!(snd_buf = new byte[MAX_MUSIC_SIZE])) {
		return 1;
	}

	snd_hsize = _sndFile.readUint16LE();
	if (!(snd_hdr = new byte[snd_hsize])) {
		delete[] snd_buf;
		snd_buf = nullptr;
		return 1;
	}

	if (_sndFile.read(snd_hdr, snd_hsize) != snd_hsize) {
		delete[] snd_buf;
		delete[] snd_hdr;
		snd_buf = nullptr;
		snd_hdr = nullptr;
		return 1;
	}

	return 2;
}

int16 Magnetic::find_name_in_sndheader(const Common::String &name) {
	int16 header_pos = 0;

	while (header_pos < snd_hsize) {
		const char *hname = (const char *)(snd_hdr + header_pos);
		if (name == hname)
			return header_pos;
		header_pos += 18;
	}

	return -1;
}

byte *Magnetic::sound_extract(const Common::String &name, uint32 *length, uint16 *tempo) {
	uint32 offset = 0;
	int16 header_pos = -1;

	if (header_pos < 0)
		header_pos = find_name_in_sndheader(name);
	if (header_pos < 0)
		return 0;

	*tempo = READ_BE_UINT16(snd_hdr + header_pos + 8);
	offset = READ_BE_UINT32(snd_hdr + header_pos + 10);
	*length = READ_BE_UINT32(snd_hdr + header_pos + 14);

	if (offset != 0) {
		if (!snd_buf)
			return nullptr;
		if (!_sndFile.seek(offset))
			return nullptr;
		if (_sndFile.read(snd_buf, *length) != *length)
			return nullptr;

		return snd_buf;
	}

	return nullptr;
}

} // End of namespace Magnetic
} // End of namespace Glk
