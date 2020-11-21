//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#ifndef __AC_WORDSDICTIONARY_H
#define __AC_WORDSDICTIONARY_H

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

#endif // __AC_WORDSDICTIONARY_H