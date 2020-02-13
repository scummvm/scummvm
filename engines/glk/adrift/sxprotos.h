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

#ifndef SCAREEXT_PROTOTYPES_H
#define SCAREEXT_PROTOTYPES_H

#include "glk/adrift/scare.h"
#include "common/stream.h"

namespace Glk {
namespace Adrift {

/* True and false, unless already defined. */
#ifndef FALSE
# define FALSE 0
#endif
#ifndef TRUE
# define TRUE (!FALSE)
#endif

/* Alias typedef for a test script. */
typedef Common::SeekableReadStream *sx_script;

/* Typedef representing a test descriptor. */
typedef struct sx_test_descriptor_s {
	const sc_char *name;
	sc_game game;
	sx_script script;
} sx_test_descriptor_t;

/*
 * Small utility and wrapper functions.  For printf wrappers, try to apply
 * gcc printf argument checking; this code is cautious about applying the
 * checks.
 */
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
extern void sx_trace(const sc_char *format, ...)
__attribute__((__format__(__printf__, 1, 2)));
extern void sx_error(const sc_char *format, ...)
__attribute__((__format__(__printf__, 1, 2)));
extern void sx_fatal(const sc_char *format, ...)
__attribute__((__format__(__printf__, 1, 2)));
#else
extern void sx_trace(const sc_char *format, ...);
extern void sx_error(const sc_char *format, ...);
extern void sx_fatal(const sc_char *format, ...);
#endif
extern void *sx_malloc(size_t size);
extern void *sx_realloc(void *pointer, size_t size);
extern void sx_free(void *pointer);
extern Common::SeekableReadStream *sx_fopen(const sc_char *name,
        const sc_char *extension, const sc_char *mode);
extern sc_char *sx_trim_string(sc_char *string);
extern sc_char *sx_normalize_string(sc_char *string);

/* Test controller function. */
extern sc_int test_run_game_tests(const sx_test_descriptor_t tests[],
                                  sc_int count, sc_bool is_verbose);

/* Globbing function. */
extern sc_bool glob_match(const sc_char *pattern, const sc_char *string);

/* Script running and checking functions. */
//extern void scr_test_failed(const sc_char *format, const sc_char *string);
//extern void scr_set_verbose(sc_bool flag);

/* Serialization helper for script running and checking. */
extern void *file_open_file_callback(sc_bool is_save);
extern sc_int file_read_file_callback(void *opaque,
                                      sc_byte *buffer, sc_int length);
extern void file_write_file_callback(void *opaque,
                                     const sc_byte *buffer, sc_int length);
extern void file_close_file_callback(void *opaque);
extern void file_cleanup(void);

} // End of namespace Adrift
} // End of namespace Glk

#endif
