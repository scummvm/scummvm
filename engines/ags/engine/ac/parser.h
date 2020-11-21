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
//
//
//
//=============================================================================
#ifndef __AGS_EE_AC__PARSER_H
#define __AGS_EE_AC__PARSER_H

int Parser_FindWordID(const char *wordToFind);
const char* Parser_SaidUnknownWord();
void ParseText (const char*text);
int Said (const char*checkwords);

//=============================================================================

int find_word_in_dictionary (const char *lookfor);
int is_valid_word_char(char theChar);
int FindMatchingMultiWordWord(char *thisword, const char **text);
int parse_sentence (const char *src_text, int *numwords, short*wordarray, short*compareto, int comparetonum);

#endif // __AGS_EE_AC__PARSER_H
