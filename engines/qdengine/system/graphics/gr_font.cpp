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

#include "common/file.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/system/graphics/gr_font.h"
#include "qdengine/qdcore/qd_file_manager.h"


namespace QDEngine {

grFont::grFont() : _alpha_buffer(NULL) {
	_size_x = _size_y = 0;
	_alpha_buffer_sx = _alpha_buffer_sy = 0;

	_chars.reserve(256);
}

grFont::~grFont() {
	delete[] _alpha_buffer;
}

bool grFont::load(const Common::Path fname) {
	Common::String str(fname.toString());
	str += ".tga";

	Common::File file;
	file.open(Common::Path(str));

	if (load_alpha(&file)) {
		str = fname.toString();
		str += ".idx";

		file.open(Common::Path(str));
		if (load_index(&file))
			return true;
	}
	return false;
}

bool grFont::load_index(Common::SeekableReadStream *fh) {
	Common::String buf = fh->readString();
	char *pos = buf.begin();

	int num_ch, sx, sy;

	sx = strtol(pos, &pos, 0);
	sy = strtol(pos, &pos, 0);
	num_ch = strtol(pos, &pos, 0);

	grFontChar chr;
	for (int i = 0; i < num_ch; i++) {
		int x, y;

		chr._code = strtol(pos, &pos, 0);
		x = strtol(pos, &pos, 0);
		y = strtol(pos, &pos, 0);
		sx = strtol(pos, &pos, 0);
		sy = strtol(pos, &pos, 0);

		chr._region = grScreenRegion(x, y, sx, sy);
		_chars.push_back(chr);

		if (sx > _size_x) _size_x = sx;
		if (sy > _size_y) _size_y = sy;
	};

	return true;
}

bool grFont::load_alpha(Common::SeekableReadStream *fh) {
	byte header[18];
	fh->read(header, 18);

	if (header[0])
		return false;

	if (header[1])
		return false;

	if (header[2] != 2 && header[2] != 3)
		return false;

	int sx = _alpha_buffer_sx = header[12] + (header[13] << 8);
	int sy = _alpha_buffer_sy = header[14] + (header[15] << 8);

	int colors = header[16];
	int flags = header[17];

	int ssx = sx * colors / 8;

	_alpha_buffer = new byte[ssx * sy];

	if (!(flags & 0x20)) {
		int idx = (sy - 1) * ssx;
		for (int i = 0; i < sy; i++) {
			fh->read(_alpha_buffer + idx, ssx);
			idx -= ssx;
		}
	} else
		fh->read(_alpha_buffer, ssx * sy);

	return true;
}

} // namespace QDEngine
