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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef _SCI_EXE_H_
#define _SCI_EXE_H_

typedef struct _exe_file exe_file_t;

exe_file_t *
exe_open(const char *filename);
/* Opens an executable file
** Parameters: (const char *) filename: Filename of executable to open
** Returns   : (exe_file_t *) File handle, or NULL on error
** This function will try to find a decompressor that can handle this type
** of executable
*/

int
exe_read(exe_file_t *file, void *buf, int count);
/* Reads from an executable file
** Parameters: (exe_file_t *) file: File handle
**             (void *) buf: Buffer to store decompressed data
**             (int) count: Size of decompressed data requested, in bytes
** Returns   : (int) Number of bytes of decompressed data that was stored in
**                   buf. If this value is less than count an error has
**                   occured, or end-of-file was reached.
*/

void
exe_close(exe_file_t *handle);
/* Closes an executable file
** Parameters: (exe_file_t *) file: File handle
** Returns   : (void)
*/

#endif /* !_SCI_EXE_H_ */
