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

#include "ags/lib/std/algorithm.h"
#include "ags/shared/ac/words_dictionary.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string_compat.h"
#include "ags/globals.h"

namespace AGS3 {

using AGS::Shared::Stream;

WordsDictionary::WordsDictionary()
	: num_words(0)
	, word(nullptr)
	, wordnum(nullptr) {
}

WordsDictionary::~WordsDictionary() {
	free_memory();
}

void WordsDictionary::allocate_memory(int wordCount) {
	num_words = wordCount;
	if (num_words > 0) {
		word = new char *[wordCount];
		word[0] = new char[wordCount * MAX_PARSER_WORD_LENGTH];
		wordnum = new short[wordCount];
		for (int i = 1; i < wordCount; i++) {
			word[i] = word[0] + MAX_PARSER_WORD_LENGTH * i;
		}
	}
}

void WordsDictionary::free_memory() {
	if (num_words > 0) {
		delete[] word[0];
		delete[] word;
		delete[] wordnum;
		word = nullptr;
		wordnum = nullptr;
		num_words = 0;
	}
}

void WordsDictionary::sort() {
	int aa, bb;
	for (aa = 0; aa < num_words; aa++) {
		for (bb = aa + 1; bb < num_words; bb++) {
			if (((wordnum[aa] == wordnum[bb]) && (ags_stricmp(word[aa], word[bb]) > 0))
			        || (wordnum[aa] > wordnum[bb])) {
				short temp = wordnum[aa];
				char tempst[30];

				wordnum[aa] = wordnum[bb];
				wordnum[bb] = temp;
				strcpy(tempst, word[aa]);
				strcpy(word[aa], word[bb]);
				strcpy(word[bb], tempst);
				bb = aa;
			}
		}
	}
}

int WordsDictionary::find_index(const char *wrem) {
	int aa;
	for (aa = 0; aa < num_words; aa++) {
		if (ags_stricmp(wrem, word[aa]) == 0)
			return aa;
	}
	return -1;
}

void decrypt_text(char *toenc, size_t buf_sz) {
	int adx = 0;
	const char *p_end = toenc + buf_sz;

	while (toenc < p_end) {
		toenc[0] -= _G(passwencstring)[adx];
		if (toenc[0] == 0)
			break;

		adx++;
		toenc++;

		if (adx > 10)
			adx = 0;
	}
}

void read_string_decrypt(Stream *in, char *buf, size_t buf_sz) {
	size_t len = in->ReadInt32();
	size_t slen = std::min(buf_sz - 1, len);
	in->Read(buf, slen);
	if (len > slen)
		in->Seek(len - slen);
	decrypt_text(buf, slen);
	buf[slen] = 0;
}

void read_dictionary(WordsDictionary *dict, Stream *out) {
	int ii;

	dict->allocate_memory(out->ReadInt32());
	for (ii = 0; ii < dict->num_words; ii++) {
		read_string_decrypt(out, dict->word[ii], MAX_PARSER_WORD_LENGTH);
		dict->wordnum[ii] = out->ReadInt16();
	}
}

#if defined (OBSOLETE)
// TODO: not a part of wordsdictionary, move to obsoletes
void freadmissout(short *pptr, Stream *in) {
	in->ReadArrayOfInt16(&pptr[0], 5);
	in->ReadArrayOfInt16(&pptr[7], NUM_CONDIT - 7);
	pptr[5] = pptr[6] = 0;
}
#endif

void encrypt_text(char *toenc) {
	int adx = 0, tobreak = 0;

	while (tobreak == 0) {
		if (toenc[0] == 0)
			tobreak = 1;

		toenc[0] += _G(passwencstring)[adx];
		adx++;
		toenc++;

		if (adx > 10)
			adx = 0;
	}
}

void write_string_encrypt(Stream *out, const char *s) {
	int stlent = (int)strlen(s) + 1;

	out->WriteInt32(stlent);
	char *enc = ags_strdup(s);
	encrypt_text(enc);
	out->WriteArray(enc, stlent, 1);
	free(enc);
}

void write_dictionary(WordsDictionary *dict, Stream *out) {
	int ii;

	out->WriteInt32(dict->num_words);
	for (ii = 0; ii < dict->num_words; ii++) {
		write_string_encrypt(out, dict->word[ii]);
		out->WriteInt16(dict->wordnum[ii]);
	}
}

} // namespace AGS3
