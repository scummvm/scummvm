
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

#ifndef BAGEL_BOFLIB_STRING_FUNCTIONS_H
#define BAGEL_BOFLIB_STRING_FUNCTIONS_H

#include "bagel/boflib/stdinc.h"

namespace Bagel {

/**
 * Makes 1st letter of every word upper-case
 * @param pszText       String to convert
 */
extern void StrWordCaps(char *pszText);

/**
 * Compares 2 strings, and determines how different they are.
 * Sort of like a CRC on the 2 strings.  This allows slight
 * spelling errors while still finding a match.
 * @param mask      1st string
 * @param text      2nd string
 * @return divergence between the 2 stings. 0 or 1 indicates a very close match.
 */
extern int StrFreqMatch(const char *mask, const char *text);

/**
 * Compares two strings
 * @param pszStr1   1st string
 * @param pszStr2   2nd string
 * @return          True if strings match
 */
extern bool StrCompare(const char *pszStr1, const char *pszStr2);

/**
 * Counts number of occurences of a specified char in String
 * @param str       Pointer to string to parse
 * @param c         Character to count in str
 * @return          Number of times character c occurs in string str
 */
extern int StrCharCount(const char *str, char c);

/**
 * Same as strstr() except ignores case
 * @param s1        String to parse
 * @param           Substring to find in s1
 * @return          Pointer to substring s2 in s1 or nullptr if not found
 */
extern char *StriStr(const char *, const char *);

/**
 * Converts substring s2 in s1 to upper-case
 * @param s1        Source string to search
 * @param s2        Token in source to be upper-cased
 */
extern void StrUprStr(char *, const char *);

/**
 * Converts substring s2 in s1 to lower-case
 * @param s1        Source string to search
 * @param s2        Token in source to be upper-cased
 */
extern void StrLwrStr(char *, const char *);

/**
 * Same as strcpy() except specified character will be stripped
 * from the destination string.
 * @param dest      Destinaton string
 * @param source    Source string
 * @param c         Character to strip from dest
 */
extern void StrCpyStripChar(char *dest, const char *source, char c);

extern char *StrStripChar(char *, char);

/**
 * Replaces all occurances of cOld in string with cNew
 * @param pszbuf    String to search
 * @param cOld      Character to find in pszBuf
 * @param cNew      Character to replace cOld
 * @return          Pointer to pszBuf
 */
extern char *StrReplaceChar(char *, char, char);

/**
 * Replaces every occurrence of 1 character with another
 * @param pBuf      Buffer to search
 * @param chOld     Old character to be replaced
 * @param chNew     New character to replace with
 * @param lSize     Size of buffer
 */
extern void MemReplaceChar(byte *pBuf, byte chOld, byte chNew, int32 lSize);

/**
 * Replaces all tokens in string with new token
 * @param pszBuf    Buffer to search
 * @param pszTok1   Token to search for in pszBuf
 * @param pszTok2   Replacement token
 * @return          Pointer to pszBuf.
 */
extern char *StrReplaceStr(char *pszBuf, const char *, const char *);

/**
 * Inverts the case of each character in specified string
 * @param pszBuf    String to parse
 */
void StrInvertCase(char *pszBuf);

/**
 * Converts nullptr-terminated string to a Pascal string
 * @param pStr      Pointer to string to convert
 * @return          Pointer to converted string
 */
char *StrCToPascal(char *pszBuffer);

/**
 * Converts Pascal style string to a nullptr-terminated C string
 * @param pStr      Pointer to Pascal string to convert
 * @return          Pointer to converted string
 */
char *StrPascalToC(char *pszBuffer);

} // namespace Bagel

#endif
