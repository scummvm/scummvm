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

#ifndef _SCI_EXE_DEC_H_
#define _SCI_EXE_DEC_H_

typedef struct _exe_handle exe_handle_t;

typedef struct _exe_decompressor {
	const char *name; /* Decompressor name. Unique identifier, should consist
	            ** of lower-case (where applicable) alphanumerics
	            */

	exe_handle_t * (*open)(const char *filename);
	/* Opens an executable file
	** Parameters: (const char *) filename: Filename of executable to open.
	** Returns   : (exe_handle_t *) Decompressor file handle, or NULL on
	**                              error.
	** This function will verify that the file can be handled by the
	** decompressor. If this is not the case the function will fail.
	*/

	int (*read)(exe_handle_t *handle, void *buf, int count);
	/* Reads from executable file
	** Parameters: (exe_handle_t *) handle: Decompressor file handle.
	**             (void *) buf: Buffer to store decompressed data.
	**             (int) count: Size of decompressed data requested, in
	**                          bytes.
	** Returns   : (int) Number of bytes of decompressed data that was
	**                   stored in buf. If this value is less than count
	**                   an error has occured, or end-of-file was
	**                   reached.
	*/

	void (*close)(exe_handle_t *handle);
	/* Closes a decompressor file handle.
	** Parameters: (exe_handle_t *) handle: Decompressor file handle.
	** Returns   : (void)
	*/
} exe_decompressor_t;

#endif /* !_SCI_EXE_DEC_H_ */
