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

#ifdef USE_TIMIDITY_MIDI

#ifndef ULTIMA8_AUDIO_MIDI_TIMIDITY_TIMIDITY_COMMON_H
#define ULTIMA8_AUDIO_MIDI_TIMIDITY_TIMIDITY_COMMON_H

#include "timidity.h"

namespace Ultima {
namespace Ultima8 {

#ifdef NS_TIMIDITY
namespace NS_TIMIDITY {
#endif

extern char *program_name, current_filename[];

extern FILE *msgfp;

struct PathList {
	const char *path;
	PathList *next;
};

/* Noise modes for open_file */
#define OF_SILENT   0
#define OF_NORMAL   1
#define OF_VERBOSE  2

extern FILE *open_file(const char *name, int decompress, int noise_mode);
extern void add_to_pathlist(const char *s);
extern void close_file(FILE *fp);
extern void skip(FILE *fp, size_t len);
extern void *safe_malloc(size_t count);

template<class T> T *safe_Malloc(size_t count = 1) {
	return static_cast<T *>(safe_malloc(count * sizeof(T)));
}

#ifdef NS_TIMIDITY
};
#endif

} // End of namespace Ultima8
} // End of namespace Ultima

#endif

#endif
