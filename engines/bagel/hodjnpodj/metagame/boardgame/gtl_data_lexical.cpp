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

#include "bagel/hodjnpodj/metagame/boardgame/gtl_data.h"
#include "bagel/hodjnpodj/metagame/bgen/bgen.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {


static char IDENT_CHARS[] =
"abcdefghijklmnopqrstuvwxyz"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"0123456789_.";

static const CKeyTab KEYWORDS[] = {
	{ KT_BMP, "BITMAP" },
	{ KT_NODE, "NODE" },
	{ KT_LINK, "LINK" },
	{ KT_ABOVE, "ABOVE" },
	{ KT_BELOW, "BELOW" },
	{ KT_LEFT, "LEFT" },
	{ KT_RIGHT, "RIGHT" },
	{ KT_PALETTE, "PALETTE" },
	{ KT_OVERLAY, "OVERLAY" },
	{ KT_MASKED, "MASKED" },
	{ KT_METAGAME, "METAGAME" },
	{ KT_SPRITE, "SPRITE" },
	{ KT_PASSTHRU, "PASSTHRU" },
	{ KT_WEIGHT, "WEIGHT" },
	{ KT_SENSITIVITY, "SENSITIVITY" },
	{ KT_SECTOR, "SECTOR" },
	{ KT_BORDER, "BORDER" },
	{ KT_PREVIOUS, "PREVIOUS" },
	{ KT_LINKPREV, "LINKPREV" },
	{ KT_POSITION, "POSITION" },
	{ KT_DIRECTORY, "DIRECTORY" },
	{ KT_FOREGROUND, "FOREGROUND" },
	{ KT_BACKGROUND, "BACKGROUND" },
	{ KT_ANIMATED, "ANIMATED" },
	{ KT_SIZE, "SIZE" },
	{ KT_MIDDLE, "MIDDLE" },
	{ KT_FREQUENCY, "FREQ" },
	{ KT_EOF, "END" },
	{ 0, "????" }
};

const char *CGtlData::FindKeyword(int iType) {
	const CKeyTab *xpKytb;	// ptr to keyword table entry

	for (xpKytb = &KEYWORDS[0]; xpKytb->m_iKeyValue &&
		xpKytb->m_iKeyValue != iType; ++xpKytb) {
	}

	// Return string point
	return xpKytb->m_xpszKeyString;
}

bool CGtlData::ReadLine() {
	int iError = 0;			// Error code
	Common::String line;
	char szLine[255];		// Input line
	const char *xpStart;	// Ptr to first char of element
	int iElementLength;		// Length of current lexical element
	int iStringListPos = 0;	// Current position in string list
	int iNumLxels = 0;		// Number of lexical elements so far
	char *xpStr;			// String pointer
	XPSTR xpszIdent;		// Identifier string pointer
	const CKeyTab *xpKytb;	// Pointer to keyword table
	CLexElement *xpLxel, *xpLastLxel = nullptr;	// last lexical element on line
	bool bDone;				// Loop variable

	// Zero out the lexical blocks
	for (int i = 0; i < MAX_LEXELTS; ++i)
		m_cLexElts[i].clear();

	bDone = false;
	while (!bDone) {
		// Read line and test for EOF
		if (m_xpGtlFile.eos()) {
			bDone = true;		// Last loop iteration
			iError = 100;
			m_bEof = true;		// End of file
			szLine[0] = '\0';	// Process empty line
		} else {
			line = m_xpGtlFile.readLine();
			Common::strcpy_s(szLine, line.c_str());
		}

		++m_iLineNumber;		// Increment line number

		// If there's a listing file
		if (m_xpListFile) {
			m_xpListFile->writeString(szLine);
			m_xpListFile->writeByte('\n');
		}

		xpStart = szLine;
		for (xpStr = szLine; *xpStr;) {
			xpLxel = &m_cLexElts[iNumLxels];   // point to current lexical block
			xpLxel->m_iLineNumber = m_iLineNumber;
			xpLxel->m_iColumn = xpStr - xpStart;

			// Test for blank, tab, line feed (newline)
			//  or carriage return
			if ((*xpStr == CCHBL) || (*xpStr == CCHTAB)
				|| (*xpStr == CCHLF) || (*xpStr == CCHCR))
				++xpStr;       // Just skip it

			else if ((*xpStr >= '0') && (*xpStr <= '9')) {
				xpLxel->m_iType = LXT_INTEGER;

				// Loop thru digits, adding to value
				while (*xpStr >= '0' && *xpStr <= '9')
					xpLxel->m_iVal = 10 * xpLxel->m_iVal + *xpStr++ - '0';

			// It's an identifier
			} else if ((iElementLength = strspn(xpStr, IDENT_CHARS)) > 0) {
				xpLxel->m_iType = LXT_IDENT;
				xpLxel->m_iStringListPos = iStringListPos;

				// Store position in string list
				if (iStringListPos + iElementLength + 2 > MAX_STRINGLIST) {
					iError = 100;  // String list overflow
					CGtlData::ErrorMsg(xpLxel, "ReadLine - String List overflow");
					goto cleanup;
				}

				// Copy string into string list
				strncpy((xpszIdent = &m_szStringList[iStringListPos]), xpStr, iElementLength);

				iStringListPos += iElementLength;  // new position

				xpStr += iElementLength;           // skip input string

				// Null terminator on string
				m_szStringList[iStringListPos++] = 0;

				for (xpKytb = &KEYWORDS[0]; xpKytb->m_iKeyValue &&
					scumm_stricmp(xpKytb->m_xpszKeyString, xpszIdent)
					!= 0;  ++xpKytb) {
				}

				// Copy keyword value (zero if none)
				xpLxel->m_iVal = xpKytb->m_iKeyValue;

			// check for string beginning with quote char
			} else if (*xpStr == CCHQUO) {
				// Store position in string list
				xpLxel->m_iType = LXT_STRING;
				xpLxel->m_iStringListPos = iStringListPos;

				// Find closing quote
				while (*++xpStr && *xpStr != CCHQUO) {
					if (*xpStr == '\\')
						++xpStr;
					m_szStringList[iStringListPos++] = *xpStr;
					if (iStringListPos + 5 > MAX_STRINGLIST) {
						iError = 101;  // string list overflow
						CGtlData::ErrorMsg(xpLxel, "ReadLine - String List overflow");
						goto cleanup;
					}
				}
				// If no closing quote
				if (*xpStr++ != CCHQUO) {
					iError = 102;	// Unterminated quote
					CGtlData::ErrorMsg(xpLxel, "ReadLine - Unterminated quoted string");
					goto cleanup;
				}
				m_szStringList[iStringListPos++] = 0;

			// Comment
			} else if (*xpStr == '/' && *(xpStr + 1) == '/') {
				*xpStr = 0;    // Insert null, loop will terminate

			} else if (strchr(",()=+-;:", *xpStr)) {
				xpLxel->m_iType = (*xpStr == ',') ? LXT_COMMA :
					(*xpStr == '(') ? LXT_LPR :
					(*xpStr == ')') ? LXT_RPR :
					(*xpStr == '=') ? LXT_EQU :
					(*xpStr == '+') ? LXT_PLUS :
					(*xpStr == '-') ? LXT_MINUS :
					(*xpStr == ';') ? LXT_SEMIC :
					(*xpStr == ':') ? LXT_COLON : 0;
				xpLxel->m_iVal = *xpStr++; // save char as val

			// Char not recognized
			} else {
				iError = 103;
				CGtlData::ErrorMsg(xpLxel, "Invalid char in input");
				goto cleanup;
			}

			if (xpLxel->m_iType)	// If we used this lex element
				xpLastLxel = xpLxel, ++iNumLxels; // skip to next one
		}

		// We're done if there was at least one lexical element,
		//  and the last one is not a comma
		if (xpLastLxel && xpLastLxel->m_iType != LXT_COMMA)
			bDone = true;
	}

	// End of statement
	m_cLexElts[iNumLxels].m_iType = LXT_EOX;

cleanup:
	return iError != 0;
}

bool CGtlData::ErrorMsg(const CLexElement *xpLxel, const char *szMessage) {
	char szLine[200];	// Output line
	char szTitle[40];	// Message box title

	Common::sprintf_s(szLine, "Error: %s.", szMessage);

	if (xpLxel) {
		Common::sprintf_s(szTitle, "Error in line %i, column %i.",
			xpLxel->m_iLineNumber, xpLxel->m_iColumn);
	} else {
		Common::strcpy_s(szTitle, "Error message");
	}

	error("%s\n%s", szTitle, szLine);
	return false;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
