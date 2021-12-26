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

#ifndef AGS_LIB_ALLEGRO_ERROR_H
#define AGS_LIB_ALLEGRO_ERROR_H

#include "common/scummsys.h"

namespace AGS3 {

// Error codes
enum AllegroError {
	AL_NOERROR = 0,
	AL_EPERM = 1,
	AL_ENOENT = 2,
	AL_ESRCH = 3,
	AL_EINTR = 4,
	AL_EIO = 5,
	AL_ENXIO = 6,
	AL_E2BIG = 7,
	AL_ENOEXEC = 8,
	AL_EBADF = 9,
	AL_ECHILD = 10,
	AL_EAGAIN = 11,
	AL_ENOMEM = 12,
	AL_EACCES = 13,
	AL_EFAULT = 14,
	AL_EBUSY = 16,
	AL_EEXIST = 17,
	AL_EXDEV = 18,
	AL_ENODEV = 19,
	AL_ENOTDIR = 20,
	AL_EISDIR = 21,
	AL_EINVAL = 22,
	AL_ENFILE = 23,
	AL_EMFILE = 24,
	AL_ENOTTY = 25,
	AL_EFBIG = 27,
	AL_ENOSPC = 28,
	AL_ESPIPE = 29,
	AL_EROFS = 30,
	AL_EMLINK = 31,
	AL_EPIPE = 32,
	AL_EDOM = 33,
	AL_ERANGE = 34,
	AL_EDEADLK = 36,
	AL_ENAMETOOLONG = 38,
	AL_ENOLCK = 39,
	AL_ENOSYS = 40,
	AL_ENOTEMPTY = 41,
	AL_EILSEQ = 42
};

} // namespace AGS3

#endif
