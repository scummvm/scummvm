
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

namespace Bagel {

/**
 * Counts number of occurrences of a specified char in String
 * @param str       Pointer to string to parse
 * @param c         Character to count in str
 * @return          Number of times character c occurs in string str
 */
extern int strCharCount(const char *str, char c);

/**
 * Replaces all occurrences of cOld in string with cNew
 * @param pszbuf    String to search
 * @param cOld      Character to find in pszBuf
 * @param cNew      Character to replace cOld
 * @return          Pointer to pszBuf
 */
extern char *strreplaceChar(char *pszbuf, char cOld, char cNew);

/**
 * Replaces every occurrence of 1 character with another
 * @param pBuf      Buffer to search
 * @param chOld     Old character to be replaced
 * @param chNew     New character to replace with
 * @param lSize     Size of buffer
 */
extern void memreplaceChar(byte *pBuf, byte chOld, byte chNew, int32 lSize);

/**
 * Replaces all tokens in string with new token
 * @param pszBuf    Buffer to search
 * @param pszTok1   Token to search for in pszBuf
 * @param pszTok2   Replacement token
 * @return          Pointer to pszBuf.
 */
extern char *strreplaceStr(char *pszBuf, const char *pszTok1, const char *pszTok2);

} // namespace Bagel

#endif
