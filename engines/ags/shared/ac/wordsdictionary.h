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

#ifndef AGS_SHARED_AC_WORDSDICTIONARY_H
#define AGS_SHARED_AC_WORDSDICTIONARY_H

#include "core/types.h"

namespace AGS { namespace Common { class Stream; } }
using namespace AGS; // FIXME later

#define MAX_PARSER_WORD_LENGTH 30
#define ANYWORD     29999
#define RESTOFLINE  30000

struct WordsDictionary {
    int   num_words;
    char**word;
    short*wordnum;

    WordsDictionary();
    ~WordsDictionary();
    void allocate_memory(int wordCount);
    void free_memory();
    void  sort();
    int   find_index (const char *);
};

extern const char *passwencstring;

extern void decrypt_text(char*toenc);
extern void read_string_decrypt(Common::Stream *in, char *buf, size_t buf_sz);
extern void read_dictionary (WordsDictionary *dict, Common::Stream *in);

#if defined (OBSOLETE)
// TODO: not a part of wordsdictionary, move to obsoletes
extern void freadmissout(short *pptr, Common::Stream *in);
#endif

extern void encrypt_text(char *toenc);
extern void write_string_encrypt(Common::Stream *out, const char *s);
extern void write_dictionary (WordsDictionary *dict, Common::Stream *out);

#endif
