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

/***************************************************************************/
/*                                                                         */
/*  ftsystem.c                                                             */
/*    ANSI-specific FreeType low-level system interface (body).            */
/*                                                                         */
/***************************************************************************/

/*************************************************************************/
/*                                                                       */
/* This file contains the default interface used by FreeType to access   */
/* low-level, i.e. memory management, i/o access as well as thread       */
/* synchronisation.  It can be replaced by user-specific routines if     */
/* necessary.                                                            */
/*                                                                       */
/*************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/config/ftconfig.h"
#include "engines/ags/lib/freetype-2.1.3/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/ftsystem.h"
#include "engines/ags/lib/freetype-2.1.3/fterrors.h"
#include "engines/ags/lib/freetype-2.1.3/fttypes.h"

#include <stdio.h>
#include <stdlib.h>

namespace AGS3 {
namespace FreeType213 {

/**** MEMORY MANAGEMENT INTERFACE ***/

FT_CALLBACK_DEF(void *)
ft_alloc(FT_Memory memory, long size) {
	FT_UNUSED(memory);

	return malloc(size);
}

FT_CALLBACK_DEF(void *)
ft_realloc(FT_Memory memory, long cur_size, long new_size, void *block) {
	FT_UNUSED(memory);
	FT_UNUSED(cur_size);

	return realloc(block, new_size);
}

FT_CALLBACK_DEF(void)
ft_free(FT_Memory memory, void *block) {
	FT_UNUSED(memory);

	free(block);
}

/**** RESOURCE MANAGEMENT INTERFACE ****/

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_io

/* We use the macro STREAM_FILE for convenience to extract the       */
/* system-specific stream handle from a given FreeType stream object */
#define STREAM_FILE(stream) ((FILE *)stream->descriptor.pointer)

FT_CALLBACK_DEF(void)
ft_ansi_stream_close(FT_Stream stream) {
	fclose(STREAM_FILE(stream));

	stream->descriptor.pointer = NULL;
	stream->size = 0;
	stream->base = 0;
}

FT_CALLBACK_DEF(unsigned long)
ft_ansi_stream_io(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count) {
	FILE *file;

	file = STREAM_FILE(stream);

	fseek(file, offset, SEEK_SET);

	return (unsigned long)fread(buffer, 1, count, file);
}

FT_EXPORT_DEF(FT_Error)
FT_Stream_Open(FT_Stream stream, const char *filepathname) {
	FILE *file;

	if (!stream)
		return FT_Err_Invalid_Stream_Handle;

	file = fopen(filepathname, "rb");
	if (!file) {
		FT_ERROR(("FT_Stream_Open:"));
		FT_ERROR((" could not open `%s'\n", filepathname));

		return FT_Err_Cannot_Open_Resource;
	}

	fseek(file, 0, SEEK_END);
	stream->size = ftell(file);
	fseek(file, 0, SEEK_SET);

	stream->descriptor.pointer = file;
	stream->pathname.pointer = const_cast<char *>(filepathname);
	stream->pos = 0;

	stream->read = ft_ansi_stream_io;
	stream->close = ft_ansi_stream_close;

	FT_TRACE1(("FT_Stream_Open:"));
	FT_TRACE1((" opened `%s' (%d bytes) successfully\n", filepathname, stream->size));

	return FT_Err_Ok;
}


FT_EXPORT_DEF(FT_Memory)
FT_New_Memory(void) {
	FT_Memory memory;

	memory = (FT_Memory)malloc(sizeof(*memory));
	if (memory) {
		memory->user = 0;
		memory->alloc = ft_alloc;
		memory->realloc = ft_realloc;
		memory->free = ft_free;
	}

	return memory;
}

FT_EXPORT_DEF(void)
FT_Done_Memory(FT_Memory memory) {
	memory->free(memory, memory);
}

} // End of namespace FreeType213
} // End of namespace AGS3
