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

#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/file.h"
#include "chewy/global.h"
#include "chewy/resource.h"

namespace Chewy {

Common::File *File::open(const char *name) {
	Common::File *f = new Common::File();
	if (f->open(name)) {
		return f;
	} else {
		delete f;
		return nullptr;
	}
}

bool File::readArray(Common::SeekableReadStream *src, uint16 *arr, size_t size) {
	Common::SeekableReadStream *rs = src->readStream(size * 2);

	bool result = (uint32)rs->size() == (size * 2);
	if (result) {
		for (; size > 0; --size, ++arr)
			*arr = rs->readUint16LE();
	}

	delete rs;
	return result;
}

int16 call_fileio(int16 palette, int16 mode) {
	short ret;
	ret = 0;
	ioptr.popx = 30;
	ioptr.popy = 50;
	switch (palette) {

	case PAL1:
		ioptr.m_col = io_pal1;
		break;
	}
	switch (mode) {

	case LOADIO:
		ioptr.f1 = 0;
		ioptr.f2 = 1;
		ioptr.f3 = 1;
		ioptr.f4 = 1;
		ioptr.key_nr = 2;
		break;

	case SAVEIO:
		ioptr.f1 = 1;
		ioptr.f2 = 0;
		ioptr.f3 = 1;
		ioptr.f4 = 1;
		ioptr.key_nr = 1;
		break;
	case ALLIO:
		ioptr.f1 = 1;
		ioptr.f2 = 1;
		ioptr.f3 = 1;
		ioptr.f4 = 1;
		ioptr.key_nr = 0;
		break;

	}
	curblk.no_back = false;
	cursor_wahl(CUR_SAVE);
	_G(out)->sprite_save(cur_back, minfo.x + curblk.page_off_x,
	                  minfo.y + curblk.page_off_y, 16,
	                  16, scr_width);
	_G(cur)->hide_cur();
	_G(cur)->show_cur();
	_G(cur)->hide_cur();
	ret = _G(iog)->io_menu(&ioptr);
	_G(cur)->show_cur();
	curblk.no_back = true;
	return ret;
}

} // namespace Chewy
