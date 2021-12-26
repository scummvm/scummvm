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

#ifndef AGS_SHARED_AC_WORDS_DICTIONARY_H
#define AGS_SHARED_AC_WORDS_DICTIONARY_H

#include "ags/shared/core/types.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

#define MAX_PARSER_WORD_LENGTH 30
#define ANYWORD     29999
#define RESTOFLINE  30000

struct WordsDictionary {
	int   num_words;
	char **word;
	short *wordnum;

	WordsDictionary();
	~WordsDictionary();
	void allocate_memory(int wordCount);
	void free_memory();
	void  sort();
	int   find_index(const char *);
};

extern void decrypt_text(char *toenc, size_t buf_sz);
extern void read_string_decrypt(Shared::Stream *in, char *buf, size_t buf_sz);
extern void read_dictionary(WordsDictionary *dict, Shared::Stream *in);

#if defined (OBSOLETE)
// TODO: not a part of wordsdictionary, move to obsoletes
extern void freadmissout(short *pptr, Shared::Stream *in);
#endif

extern void encrypt_text(char *toenc);
extern void write_string_encrypt(Shared::Stream *out, const char *s);
extern void write_dictionary(WordsDictionary *dict, Shared::Stream *out);

} // namespace AGS3

#endif
