/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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

#ifndef FCNTL_H
#define FCNTL_H

#define O_TEXT			0x0			/* 960827: Added this for Visual C++ compatibility. */
#define O_RDWR			0x1			/* open the file in read/write mode */	/*- mm 980420 -*/
#define O_RDONLY		0x2			/* open the file in read only mode */	/*- mm 980420 -*/
#define O_WRONLY		0x4			/* open the file in write only mode */	/*- mm 980420 -*/
#define O_APPEND		0x0100		/* open the file in append mode */
#define O_CREAT			0x0200		/* create the file if it doesn't exist */
#define O_EXCL			0x0400		/* if the file already exists don't create it again */
#define O_TRUNC			0x0800		/* truncate the file after opening it */
#define O_NRESOLVE		0x1000		/* Don't resolve any aliases */
#define O_ALIAS			0x2000		/* Open alias file (if the file is an alias) */
#define O_RSRC 			0x4000		/* Open the resource fork */
#define O_BINARY		0x8000		/* open the file in binary mode (default is text mode) */
#define F_DUPFD			0x0			/* return a duplicate file descriptor */

#endif
