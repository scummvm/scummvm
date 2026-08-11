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

#include "director/director.h"
#include "director/lingo/lingo.h"

namespace Director {

static const struct CharNormProto {
	Common::u32char_type_t from;
	const char *to;
} charNormProtos[] = {
	{ 0x00C0, "a" },		// À - \xCB in Mac Roman, \xC0 in Windows-1252
	{ 0x00C1, "a" },		// Á - \xE7 in Mac Roman, \xC1 in Windows-1252
	{ 0x00C2, "a" },		// Â - \xE5 in Mac Roman, \xC2 in Windows-1252
	{ 0x00C3, "a" },		// Ã - \xCC in Mac Roman, \xC3 in Windows-1252
	{ 0x00C4, "a" },		// Ä - \x80 in Mac Roman, \xC4 in Windows-1252
	{ 0x00C5, "a" },		// Å - \x81 in Mac Roman, \xC5 in Windows-1252
	{ 0x00C6, "ae" },		// Æ - \xAE in Mac Roman, \xC6 in Windows-1252
	{ 0x00C7, "c" },		// Ç - \x82 in Mac Roman, \xC7 in Windows-1252
	{ 0x00C8, "e" },		// È - \xE9 in Mac Roman, \xC8 in Windows-1252
	{ 0x00C9, "e" },		// É - \x83 in Mac Roman, \xC9 in Windows-1252
	{ 0x00CA, "e" },		// Ê - \xE6 in Mac Roman, \xCA in Windows-1252
	{ 0x00CB, "e" },		// Ë - \xE8 in Mac Roman, \xCB in Windows-1252
	{ 0x00CC, "i" },		// Ì - \xED in Mac Roman, \xCC in Windows-1252
	{ 0x00CD, "i" },		// Í - \xEA in Mac Roman, \xCD in Windows-1252
	{ 0x00CE, "i" },		// Î - \xEB in Mac Roman, \xCE in Windows-1252
	{ 0x00CF, "i" },		// Ï - \xEC in Mac Roman, \xCF in Windows-1252
	{ 0x00D0, "d" },		// Ð - \xD0 in Windows-1252
	{ 0x00D1, "n" },		// Ñ - \x84 in Mac Roman, \xD1 in Windows-1252
	{ 0x00D2, "o" },		// Ò - \xF1 in Mac Roman, \xD2 in Windows-1252
	{ 0x00D3, "o" },		// Ó - \xEE in Mac Roman, \xD3 in Windows-1252
	{ 0x00D4, "o" },		// Ô - \xEF in Mac Roman, \xD4 in Windows-1252
	{ 0x00D5, "o" },		// Õ - \xCD in Mac Roman, \xD5 in Windows-1252
	{ 0x00D6, "o" },		// Ö - \x85 in Mac Roman, \xD6 in Windows-1252
	{ 0x00D8, "o" },		// Ø - \xAF in Mac Roman, \xD8 in Windows-1252
	{ 0x00D9, "u" },		// Ù - \xF4 in Mac Roman, \xD9 in Windows-1252
	{ 0x00DA, "u" },		// Ú - \xF2 in Mac Roman, \xDA in Windows-1252
	{ 0x00DB, "u" },		// Û - \xF3 in Mac Roman, \xDB in Windows-1252
	{ 0x00DC, "u" },		// Ü - \x86 in Mac Roman, \xDC in Windows-1252
	{ 0x00DD, "y" },		// Ý - \xDD in Windows-1252
	{ 0x00DE, "\xC3\xBE" },	// Þ - \xDE in Windows-1252
	{ 0x00DF, "s" },		// ß - \xDF in Windows-1252
	{ 0x00E0, "a" },		// à - \x88 in Mac Roman, \xE0 in Windows-1252
	{ 0x00E1, "a" },		// á - \x87 in Mac Roman, \xE1 in Windows-1252
	{ 0x00E2, "a" },		// â - \x89 in Mac Roman, \xE2 in Windows-1252
	{ 0x00E3, "a" },		// ã - \x8B in Mac Roman, \xE3 in Windows-1252
	{ 0x00E4, "a" },		// ä - \x8A in Mac Roman, \xE4 in Windows-1252
	{ 0x00E5, "a" },		// å - \x8C in Mac Roman, \xE5 in Windows-1252
	{ 0x00E6, "ae" },		// æ - \xBE in Mac Roman, \xE6 in Windows-1252
	{ 0x00E7, "c" },		// ç - \x8D in Mac Roman, \xE7 in Windows-1252
	{ 0x00E8, "e" },		// è - \x8F in Mac Roman, \xE8 in Windows-1252
	{ 0x00E9, "e" },		// é - \x8E in Mac Roman, \xE9 in Windows-1252
	{ 0x00EA, "e" },		// ê - \x90 in Mac Roman, \xEA in Windows-1252
	{ 0x00EB, "e" },		// ë - \x91 in Mac Roman, \xEB in Windows-1252
	{ 0x00EC, "i" },		// ì - \x93 in Mac Roman, \xEC in Windows-1252
	{ 0x00ED, "i" },		// í - \x92 in Mac Roman, \xED in Windows-1252
	{ 0x00EE, "i" },		// î - \x94 in Mac Roman, \xEE in Windows-1252
	{ 0x00EF, "i" },		// ï - \x95 in Mac Roman, \xEF in Windows-1252
	{ 0x00F0, "d" },		// ð - \xF0 in Windows-1252
	{ 0x00F1, "n" },		// ñ - \x96 in Mac Roman, \xF1 in Windows-1252
	{ 0x00F2, "o" },		// ò - \x98 in Mac Roman, \xF2 in Windows-1252
	{ 0x00F3, "o" },		// ó - \x97 in Mac Roman, \xF3 in Windows-1252
	{ 0x00F4, "o" },		// ô - \x99 in Mac Roman, \xF4 in Windows-1252
	{ 0x00F5, "o" },		// õ - \x9B in Mac Roman, \xF5 in Windows-1252
	{ 0x00F6, "o" },		// ö - \x9A in Mac Roman, \xF6 in Windows-1252
	{ 0x00F8, "o" },		// ø - \xBF in Mac Roman, \xF8 in Windows-1252
	{ 0x00F9, "u" },		// ù - \x9D in Mac Roman, \xF9 in Windows-1252
	{ 0x00FA, "u" },		// ú - \x9C in Mac Roman, \xFA in Windows-1252
	{ 0x00FB, "u" },		// û - \x9E in Mac Roman, \xFB in Windows-1252
	{ 0x00FC, "u" },		// ü - \x9F in Mac Roman, \xFC in Windows-1252
	{ 0x00FD, "y" },		// ý - \xFD in Windows-1252
	{ 0x00FF, "y" },		// ÿ - \xD8 in Mac Roman
	{ 0x0131, "i" },		// ı - \xF5 in Mac Roman
	{ 0x0152, "oe" },		// Œ - \xCE in Mac Roman, \x8C in Windows-1252
	{ 0x0153, "oe" },		// œ - \xCF in Mac Roman, \x9C in Windows-1252
	{ 0x0160, "s" },		// Š - \x8A in Windows-1252
	{ 0x0161, "s" },		// š - \x9A in Windows-1252
	{ 0x0178, "y" },		// Ÿ - \xD9 in Mac Roman, \x9F in Windows-1252
	{ 0x017D, "z" },		// Ž - \x8E in Windows-1252
	{ 0x017E, "z" },		// ž - \x9E in Windows-1252
	{ 0xFB01, "fi" },		// ﬁ - \xDE in Mac Roman
	{ 0xFB02, "fl" },		// ﬂ - \xDF in Mac Roman
	{ 0, NULL }
};

void Lingo::initCharNormalizations() {
	for (char ch = 'A'; ch <= 'Z'; ch++) {
		_charNormalizations[ch] = Common::U32String(Common::String(tolower(ch)), Common::kUtf8);
	}
	for (const CharNormProto *norm = charNormProtos; norm->to; norm++) {
		_charNormalizations[norm->from] = Common::U32String(norm->to, Common::kUtf8);
	}
}

Common::String Lingo::normalizeString(const Common::String &str) {
	Common::U32String u32Str = str.decode(Common::kUtf8);
	Common::U32String res;
	for (const Common::u32char_type_t *ch = u32Str.c_str(); *ch; ch++) {
		if (_charNormalizations.contains(*ch))
			res += _charNormalizations[*ch];
		else
			res += *ch;
	}
	return res.encode(Common::kUtf8);
}

} // End of namespace Director
