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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/errors.h"
#include "saga2/iff.h"

namespace Saga2 {

/* ======================================================================= *
   General IFF routines (not completely general, but will do for now...)
 * ======================================================================= */

iffHandle::iffHandle(void) {
	fileHandle = NULL;
	flags = iffStreamIO;

	form.id = 0;
	form.size = 0;
	form.pos = 0;
	form.patch = 0;

	chunk.id = 0;
	chunk.size = 0;
	chunk.pos = 0;
	chunk.patch = 0;

	lastErr = errOK;
}

void iffHandle::swapLong(int32 &l) {
#if defined(MSDOS) || defined(__MSDOS__)
	l = ((l >> 24) & 0x000000ffL) |
	    ((l >>  8) & 0x0000ff00L) |
	    ((l <<  8) & 0x00ff0000L) |
	    ((l << 24) & 0xff000000L);
#endif
}

bool iffReadHandle::readChunk(void) {
	int32       hdr[ 2 ];

	if (fread(&hdr, 4, 2, fileHandle) != 2) {
		lastErr = dosErrCode();
		return FALSE;
	}

	swapLong(hdr[ 1 ]);

	chunk.id = hdr[ 0 ];
	chunk.size = hdr[ 1 ];
	chunk.pos = 0;
	chunk.patch = 0;

	form.pos += 8;

	if (form.size > 0 &&
	        (chunk.size < 0 || chunk.size > form.size - form.pos)) {
		lastErr = errMangledForm;
		return FALSE;
	}

	return TRUE;
}

bool iffReadHandle::skipChunk(void) {
	if (lastErr != errOK) return FALSE;

	//  skip over old chunk

	if (chunk.size & 1) chunk.size++;       // pad to even size
	if (chunk.pos < chunk.size) {
		if (fseek(fileHandle, chunk.size - chunk.pos, SEEK_CUR) != 0) {
			lastErr = dosErrCode();
			return FALSE;
		}
		form.pos += chunk.size - chunk.pos;
	}

	return TRUE;
}

iffReadHandle::iffReadHandle(char *filename, ChunkID &formName) {
	name = filename;

	fileHandle = fopen(filename, "rb");
	if (fileHandle == NULL) {
		lastErr = dosErrCode();
	} else if (readChunk()) {
		form = chunk;
		chunk.pos = chunk.size = 0;

		if (form.id != ID_FORM) {
			lastErr = errNotIFF;
		} else if (fread(&form.id, 4, 1, fileHandle) != 1) {
			lastErr = dosErrCode();
		} else {
			formName = form.id;
			form.pos += 4;
		}
	}
}

iffReadHandle::~iffReadHandle() {
	if (fileHandle) fclose(fileHandle);
}

bool iffReadHandle::nextChunk(ChunkID &id) {
	if (skipChunk() == FALSE) return FALSE;

	if (form.pos >= form.size) {
		id = 0;                             // ID of zero means EOF
		return TRUE;
	}

	//  Read chunk header

	if (readChunk() == FALSE) return FALSE;
	id = chunk.id;                          // return ID of read chunk
	return TRUE;
}

#if 0
bool iffReadHandle::seekChunk(ChunkID id) {
	ChunkID         cid;

	//  keep reading chunks until we find one we want

	do {
		nextChunk(cid);
		if (lastErr) break;

	} while (cid != id && cid != 0);

	return (cid != id);
}
#endif

int32 iffReadHandle::read(void *buffer, int32 length) {
	if (lastErr != errOK) return -1;

	if (length <= 0) return 0;
	else if (length > chunk.size - chunk.pos)
		length = chunk.size - chunk.pos;

	//  REM: If using borland, break this up using jburks method

	if (fread(buffer, length, 1, fileHandle) != 1) {
		lastErr = dosErrCode();
		return -1;
	}
	chunk.pos += length;
	form.pos += length;
	return length;
}

#if 0
void iffReadHandle::stopChunks(ChunkID *stopList) {
}
#endif

iffWriteHandle::iffWriteHandle(char *filename, ChunkID formName) {
	name = filename;

	fileHandle = fopen(filename, "wb");
	if (fileHandle == NULL) {
		lastErr = dosErrCode();
	} else {
		uint32  hdr[ 3 ];

		//  Set up the form handle

		form.id = formName;
		form.size = -1;
		form.pos = 4;
		form.patch = 4;

		//  write the form handle

		hdr[ 0 ] = ID_FORM;
		hdr[ 1 ] = 0;
		hdr[ 2 ] = formName;

		if (fwrite(hdr, sizeof hdr, 1, fileHandle) != 1) {
			lastErr = dosErrCode();
		}
	}
}

iffWriteHandle::~iffWriteHandle() {
	if (fileHandle) {
		if (lastErr == errOK) {
			swapLong(form.pos);

			if (fseek(fileHandle, form.patch, SEEK_SET) != 0
			        || fwrite(&form.pos, 4, 1, fileHandle)   != 1
			        || fseek(fileHandle, form.pos, SEEK_SET) != 0) {
				lastErr = dosErrCode();
			}

			swapLong(form.pos);
		}
		fclose(fileHandle);
	}
}

bool iffWriteHandle::pushChunk(ChunkID id, int32 length) {
	int32       hdr[ 2 ];

	if (lastErr != errOK) return FALSE;

	chunk.id = id;
	chunk.pos = 0;
	if (length >= 0) {
		chunk.size = length;
		chunk.patch = 0;
	} else {
		chunk.size = -1;
		chunk.patch = form.pos + 8 + 4;
	}

	hdr[ 0 ] = id;
	hdr[ 1 ] = chunk.size;
	swapLong(hdr[ 1 ]);

	if (fwrite(hdr, sizeof hdr, 1, fileHandle) != 1) {
		lastErr = dosErrCode();
	}

	form.pos += 8;

	return TRUE;
}

bool iffWriteHandle::popChunk(void) {
	int32           length;

	if (lastErr != errOK) return FALSE;

	//  Figure out length of chunk...

	if (chunk.size == -1)
		length = chunk.pos;
	else length = chunk.size;

	//  Add NULL pad bytes to fill to end of chunk

	while (chunk.pos < length) {
		if (fputc(0, fileHandle) == EOF) {
			lastErr = dosErrCode();
			return FALSE;
		}
		chunk.pos++;
		form.pos++;
	}

	//  extend chunk length to include pad byte

	if (length & 1) {
		if (fputc(0, fileHandle) == EOF) {
			lastErr = dosErrCode();
			return FALSE;
		}
		chunk.pos++;
		form.pos++;
	}

	//  Now, patch the chunk length if needed

	if (chunk.patch > 0) {
		swapLong(length);

		if (fseek(fileHandle, chunk.patch, SEEK_SET) != 0) {
			lastErr = dosErrCode();
			return FALSE;
		}

		if (fwrite(&length, 4, 1, fileHandle) != 1) {
			lastErr = dosErrCode();
			return FALSE;
		}

		if (fseek(fileHandle, form.pos + 8, SEEK_SET) != 0) {
			lastErr = dosErrCode();
			return FALSE;
		}
	}
	return TRUE;
}

int32 iffWriteHandle::write(void *buffer, int32 length) {
	if (lastErr != errOK) return FALSE;

	if (length <= 0) return 0;
	else if (chunk.size > 0 && length > chunk.size - chunk.pos)
		length = chunk.size - chunk.pos;

	//  REM: If using borland, break this up using jburks method

	if (fwrite(buffer, length, 1, fileHandle) != 1) {
		lastErr = dosErrCode();
		return FALSE;
	}

	chunk.pos += length;
	form.pos += length;

	return length;
}

int32 iffWriteHandle::writeChunk(ChunkID id, void *buffer, int32 length) {
	return (pushChunk(id, length)
	        && write(buffer, length)
	        && popChunk());
}

void swapLong(int32 &l) {
#if defined(MSDOS) || defined(__MSDOS__)
	l = ((l >> 24) & 0x000000ffL) |
	    ((l >>  8) & 0x0000ff00L) |
	    ((l <<  8) & 0x00ff0000L) |
	    ((l << 24) & 0xff000000L);
#endif
}

void swapWord(int16 &w) {
#if defined(MSDOS) || defined(__MSDOS__)
	w = ((w >>  8) & 0x00ff) | ((w <<  8) & 0xff00);
#endif
}

} // end of namespace Saga2
