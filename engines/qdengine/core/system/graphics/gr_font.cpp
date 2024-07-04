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

/* ---------------------------- INCLUDE SECTION ----------------------------- */
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#define _NO_ZIP_
#include "common/file.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/system/graphics/gr_dispatcher.h"
#include "qdengine/core/system/graphics/gr_font.h"
#include "qdengine/core/qdcore/qd_file_manager.h"


namespace QDEngine {
/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

grFont::grFont() : alpha_buffer_(NULL) {
	size_x_ = size_y_ = 0;
	alpha_buffer_sx_ = alpha_buffer_sy_ = 0;

	chars_.reserve(256);
}

grFont::~grFont() {
	delete alpha_buffer_;
}

bool grFont::load(const char *fname) {
	Common::String str(fname);
	str += ".tga";

	Common::File file;
	file.open(Common::Path(str, '\\'));

	if (load_alpha(&file)) {
		Common::String str(fname);
		str += ".idx";

		file.open(Common::Path(str));
		if (load_index(&file))
			return true;
	}
	return false;
}

bool grFont::load_index(XStream &fh) {
	warning("STUB: grFont::load_index(XStream &fh)");
	return true;
}

bool grFont::load_index(XZipStream &fh) {
	warning("STUB: grFont::load_index(XZipStream &fh)");
	return true;
}

bool grFont::load_alpha(XStream &fh) {
	warning("STUB: grFont::load_alpha(XStream &fh)");
	return true;
}

bool grFont::load_alpha(XZipStream &fh) {
	warning("STUB: grFont::load_alpha(XZipStream &fh");
	return true;
}

bool grFont::load_index(Common::SeekableReadStream *fh) {
	int64 bufSize = fh->size();
	byte buf[bufSize];
	fh->read(buf, bufSize);

	XBuffer XBuf(buf, bufSize);

	int num_ch, sx, sy;
	XBuf >= sx >= sy >= num_ch;

	grFontChar chr;
	for (int i = 0; i < num_ch; i ++) {
		int x, y, sx, sy;
		XBuf >= chr.code_ >= x >= y >= sx >= sy;

		chr.region_ = grScreenRegion(x, y, sx, sy);
		chars_.push_back(chr);

		if (sx > size_x_) size_x_ = sx;
		if (sy > size_y_) size_y_ = sy;
	};

	return true;
}

bool grFont::load_alpha(Common::SeekableReadStream *fh) {
	unsigned char header[18];
	fh->read(header, 18);

	if (header[0])
		return false;

	if (header[1])
		return false;

	if (header[2] != 2 && header[2] != 3)
		return false;

	int sx = alpha_buffer_sx_ = header[12] + (header[13] << 8);
	int sy = alpha_buffer_sy_ = header[14] + (header[15] << 8);

	int colors = header[16];
	int flags = header[17];

	int ssx = sx * colors / 8;

	alpha_buffer_ = new unsigned char[ssx * sy];

	if (!(flags & 0x20)) {
		int idx = (sy - 1) * ssx;
		for (int i = 0; i < sy; i ++) {
			fh->read(alpha_buffer_ + idx, ssx);
			idx -= ssx;
		}
	} else
		fh->read(alpha_buffer_, ssx * sy);

	return true;
}


} // namespace QDEngine
