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

#ifndef MADS_CORE_FILEIO_H
#define MADS_CORE_FILEIO_H

#include "common/stream.h"

namespace MADS {
namespace MADSV2 {

#define fread_bufsize  1024

extern bool fileio_suppress_unbuffering;


extern void fileio_purge_trailing_spaces(char *myline);
extern char *fileio_ffgets(char *mystring, int num, Common::SeekableReadStream *stream);
extern int fileio_ffputs(const char *mystring, Common::WriteStream *stream);
extern char *fileio_fix_lf_input(char *mystring);
extern void fileio_fix_lf_output(char *mystring);
extern int fileio_copy(const char *source, const char *dest);
extern long fileio_setpos(Common::Stream *handle, long pos);
extern long fileio_fread_f(void *buffer, long size, long count, Common::SeekableReadStream *stream);
extern long fileio_fwrite_f(const void *buffer, long size, long count, Common::WriteStream *stream);
extern long fileio_file_to_file(Common::SeekableReadStream *from, Common::WriteStream *to, long count);
extern long fileio_get_file_size(const char *filename);
extern bool fileio_exist(const char *inp);

/**
 * Returns the identifying portion of the file header in OUTP string
 */
extern char *fileio_get_filetype(char *outp, char *inp);

/**
 * Returns the DOS time/date stamp for a given file.
 * @return	Returns 0 if file is not found. Returned 32 bit number contains
 * the date stamp in it's upper 16 bits, and the time stamp in it's lower.
 */
extern long fileio_get_file_time(char *filename);

extern char *fileio_read_header(char *target, Common::SeekableReadStream *handle);
extern void fileio_write_header(char *text, Common::WriteStream *handle);
extern char *fileio_get_line(char *target, Common::SeekableReadStream *handle);
extern int fileio_put_line(char *source, Common::WriteStream *handle);
extern long fileio_get_disk_free(char drive);
extern void fileio_add_ext(char *name, const char *ext);
extern void fileio_new_ext(char *target, const char *name, const char *ext);
extern int fileio_logpath(const char *path);
extern char *fileio_parse_filename(char *target, char *filepath);
extern char *fileio_parse_path(char *target, const char *filepath);
extern char *fileio_swap_path(char *target, const char *base, const char *file);
extern char *fileio_join_path(char *target, const char *path, const char *file);
extern void fileio_get_volume_label(char *volume_label, char drive_letter);
extern int fileio_set_file_time(char *filename, long new_time);
extern int fileio_get_file_attributes(const char *filename, word *attributes);
extern int fileio_set_file_attributes(const char *filename, word attributes);
extern int fileio_read_till_null(char *target, Common::Stream *handle);
extern char *fileio_prepend(char *target, const char *source, const char *prepend);
extern char *fileio_chop_ext(char *target, const char *source);
extern void fileio_purge_all_spaces(char *text);

} // namespace MADSV2
} // namespace MADS

#endif
