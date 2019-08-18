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

/* TADS OS interface file type definitions
 *
 * Defines certain datatypes used in the TADS operating system interface
 */

#ifndef GLK_TADS_OS_DATATYPE
#define GLK_TADS_OS_DATATYPE

namespace Glk {
namespace TADS {

/**
 * File types.  These type codes are used when opening or creating a
 *   file, so that the OS routine can set appropriate file system metadata
 *   to describe or find the file type.
 *   
 *   The type os_filetype_t is defined for documentary purposes; it's
 *   always just an int.  
 */
enum os_filetype_t {
	OSFTGAME  = 0,                               /* a game data file (.gam) */
	OSFTSAVE  = 1,                                   /* a saved game (.sav) */
	OSFTLOG   = 2,                               /* a transcript (log) file */
	OSFTSWAP  = 3,                                             /* swap file */
	OSFTDATA  = 4,      /* user data file (used with the TADS fopen() call) */
	OSFTCMD   = 5,                                   /* QA command/log file */
	OSFTERRS  = 6,                                    /* error message file */
	OSFTTEXT  = 7,                     /* text file - used for source files */
	OSFTBIN   = 8,          /* binary file of unknown type - resources, etc */
	OSFTCMAP  = 9,                                /* character mapping file */
	OSFTPREF  = 10,                                     /* preferences file */
	OSFTUNK   = 11,         /* unknown - as a filter, matches any file type */
	OSFTT3IMG = 12,                  /* T3 image file (.t3 - formerly .t3x) */
	OSFTT3OBJ = 13,                                /* T3 object file (.t3o) */
	OSFTT3SYM = 14,                         /* T3 symbol export file (.t3s) */
	OSFTT3SAV = 15                            /* T3 saved state file (.t3v) */
};

} // End of namespace TADS
} // End of namespace Glk

#endif
