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

#include "m4/graphics/gr_color.h"
#include "m4/core/errors.h"
#include "m4/fileio/sys_file.h"
#include "m4/mem/reloc.h"

namespace M4 {

InvPal::InvPal(const char *filename) {
	handle = nullptr;
	if (!filename)
		return;

	SysFile ipl5(filename, BINARY);

	if (!ipl5.exists())
		return;

	handle = NewHandle(32768, "5 bit ict");

	if (!handle) {
		MakeMem(32768, "5 bit ict");
		handle = NewHandle(32768, "5 bit ict");
		if (!handle) {
			ipl5.close();
			error_show(FL, 'OOM!', "5 bit ict");
			return;
		}
	}

	ipl5.read(handle, 32768);
}

InvPal::~InvPal() {
	if (handle)
		DisposeHandle(handle);
}

uint8 *InvPal::get_ptr() {
	if (!handle)
		return nullptr;

	HLock(handle);
	return (uint8 *)*handle;
}

void InvPal::release() {
	if (handle)
		HUnLock(handle);
}

} // namespace M4
