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

#ifndef ULTIMA8_MISC_ERRORS_H
#define ULTIMA8_MISC_ERRORS_H

namespace Ultima {
namespace Ultima8 {

////////////////////////////////
//                            //
// Pentagram Base Error Codes //
//                            //
////////////////////////////////

// Base Error Code type
struct ECode {
	int32 _error;

	ECode() : _error(0) { }
	ECode(int32 _e) : _error(_e) { }

	bool failed() const {
		return _error != 0;
	}
	bool succeded() const {
		return _error == 0;
	}

	ECode &operator = (int32 &_e) {
		_error = _e;
		return (*this);
	}

	ECode &operator = (ECode &_e) {
		_error = _e._error;
		return (*this);
	}

	bool operator != (int32 &_e) {
		return _error != _e;
	}

	friend bool operator != (int32 &_e, ECode &_e2) {
		return _e2._error != _e;
	}

	bool operator == (int32 &_e) {
		return _error == _e;
	}

	friend bool operator == (int32 &_e, ECode &_e2) {
		return _e2._error == _e;
	}
};


//
// Helper Macros
//

// Checks to see if a return code is an _error
#define P_FAILED(e) ((e)!=0)

// Checks to see if an _error code indicates success
#define P_SUCCEDED(e) ((e)==0)


//
// Error Code Bases
//

// Kernel Error Code Base
#define KERNEL_ERROR_BASE                   (-0x10000)

// Usecode Error Code Base
#define UESCODE_ERROR_BASE                  (-0x20000)

// FileSystem Error Code Base
#define FILESYS_ERROR_BASE                  (-0x30000)

// Graphics Error Code Base
#define GRAPHICS_ERROR_BASE                 (-0x40000)

// Gump Error Code Base
#define GUMPS_ERROR_BASE                    (-0x50000)

// Convert Error Code Base
#define CONVERT_ERROR_BASE                  (-0x60000)

// World Error Code Base
#define WORLD_ERROR_BASE                    (-0x70000)

// Audio Error Code Base
#define AUDIO_ERROR_BASE                    (-0x80000)

// Misc Error Code Base
#define MISC_ERROR_BASE                     (-0x90000)


//
// Basic Error Codes
//

// No Error
#define P_NO_ERROR                          (0)

// Generic Error
#define P_GENERIC_ERROR                     (-1)

// Undefined Error
#define P_UNDEFINED_ERROR                   (-2)

// Memory Allocation Error
#define P_MEMORY_ALLOC_ERRO                 (-3)

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
