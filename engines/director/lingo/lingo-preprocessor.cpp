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
#include "director/movie.h"
#include "director/lingo/lingo-codegen.h"

namespace Director {

bool isspec(Common::u32char_type_t c) {
	if (c > 127)
		return false;

	return strchr("-+*/%^:,()><&[]=", (char)c) != nullptr;
}

static Common::U32String nexttok(const Common::u32char_type_t *s, const Common::u32char_type_t **newP = nullptr) {
	Common::U32String res;

	// Scan first non-whitespace
	while (*s && (*s == ' ' || *s == '\t' || *s == CONTINUATION)) // If we see a whitespace
		s++;

	if (*s == '"') { // If it is a string then scan till end quote
		res += *s++;

		while (*s && *s != '"')
			res += *s++;

		if (*s == '"')
			res += *s++;
	} else if (Common::isAlnum(*s) || *s == '#' || *s == '.') {
		// Now copy everything till whitespace
		while (*s && (Common::isAlnum(*s) || *s == '.' || *s == '#' || *s == '_'))
			res += *s++;
	} else {
		while (*s && isspec(*s))
			res += *s++;
	}

	if (newP)
		*newP = s;

	return res;
}

Common::U32String LingoCompiler::codePreprocessor(const Common::U32String &code, LingoArchive *archive, ScriptType type, CastMemberID id, uint32 flags) {
	const Common::u32char_type_t *s = code.c_str();
	Common::U32String res;

	// We start from processing the continuation symbols
	// (The continuation symbol is \xC2 in Mac Roman, \xAC in Unicode.)
	// \xAC\n  ->  \xAC
	// This will greatly simplify newline processing, still leaving
	// the line number tracking intact
	while (*s) {
		if (*s == CONTINUATION) {
			res += *s++;
			if (!*s)	// Who knows, maybe it is the last symbol in the script
				break;
			s++;
			continue;
		} else if (*s == 0xFF82) {	// Misparsed Japanese continuation
			if (!*(s+1)) { // EOS - write as is and finish up
				res += *s++;
				break;
			}
			// Next character isn't a newline; write as is and keep
			// going.
			if (*(s+1) != 13) {
				res += *s++;
				continue;
			}

			s++;
			// This is a bit of a hack; in MacJapanese the codepoint at
			// C2 is the half-width katakana "tsu", so ScummVM is
			// getting confused about what's here in the script after
			// translating from MacJapanese to Unicode.
			// Just swap the character out for the right Unicode character here.
			// This can be removed if Lingo parsing is reworked to act
			// on the original raw bytes instead of a Unicode translation.
			res += CONTINUATION;
			if (!*s)
				break;
			s++;
			continue;
		}
		res += *s++;
	}

	Common::U32String tmp(res);
	res.clear();
	s = tmp.c_str();

	// Strip comments
	bool inString = false;
	while (*s) {
		if (*s == '"')
			inString = !inString;

		if (!inString && *s == '-' && *(s + 1) == '-') { // At the end of the line we will have \0
			while (*s && *s != '\r' && *s != '\n')
				s++;
		}

		if (*s == '\r')
			res += '\n';
		else if (*s)
			res += *s;

		if (*s)
			s++;
	}

	tmp = res;
	res.clear();

	// Strip trailing whitespaces
	s = tmp.c_str();
	while (*s) {
		if (*s == ' ' || *s == '\t' || *s == CONTINUATION) { // If we see a whitespace
			const Common::u32char_type_t *ps = s; // Remember where we saw it

			while (*ps == ' ' || *ps == '\t' || *ps == CONTINUATION) // Scan until end of whitespaces
				ps++;

			if (*ps) {	// Not end of the string
				if (*ps == '\n') {	// If it is newline, then we continue from it
					s = ps;
				} else {	// It is not a newline
					while (s != ps) {	// Add all whitespaces
						res += *s;
						s++;
					}
				}
			}
		}

		if (*s)
			res += *s;

		s++;
	}

	if (flags & kLPPSimple)
		return res;

	tmp = res;
	s = tmp.c_str();
	res.clear();

	Common::U32String line, tok, res1;
	int linenumber = 1;
	bool defFound = false;

	const Common::U32String macro("macro"), factory("factory"), on("on"), global("global"), property("property"),
		mci("mci");

	while (*s) {
		line.clear();
		res1.clear();

		// Get next line
		int continuationCount = 0;
		while (*s && *s != '\n') { // If we see a whitespace
			res1 += *s;
			line += tolower(*s++);

			if (*s == CONTINUATION) {
				linenumber++;
				continuationCount++;
			}
		}
		debugC(2, kDebugParse | kDebugPreprocess, "line: '%s'", line.encode().c_str());

		if (!defFound && (type == kMovieScript || type == kCastScript) && (g_director->getVersion() < 400 || g_director->getCurrentMovie()->_allowOutdatedLingo)) {
			tok = nexttok(line.c_str());
			if (tok.equals(macro) || tok.equals(factory)) {
				defFound = true;
			} else if (!(flags & kLPPForceD2)) {
				if (tok.equals(on) || tok.equals(global) || tok.equals(property)) {
					defFound = true;
				}
			}

			if (!defFound) {
				debugC(2, kDebugParse | kDebugPreprocess, "skipping line before first definition");
				for (int i = 0; i < continuationCount; i++) {
					res += CONTINUATION;
				}
				linenumber++;
				if (*s)	// copy newline symbol
					res += *s++;
				continue;
			}
		}

		// In MultiMedia Movie format, .MMM files used by Microsoft
		// 'mci' keyword is followed by the unquoted commands, e.g.
		//     mci close all
		//     mci play wave to 15228 hold
		//
		// Since Director requires them in a single thing, we add
		// quotes around
		const Common::u32char_type_t *contLine;
		tok = nexttok(line.c_str(), &contLine);

		if (tok.equals(mci) && *contLine != 0 && !Common::U32String(contLine).contains('\"')) {
			// Scan first non-whitespace
			while (*contLine && (*contLine == ' ' || *contLine == '\t' || *contLine == CONTINUATION)) // If we see a whitespace
				contLine++;

			res1 = Common::U32String::format("%S \"%S\"", tok.c_str(), contLine);

			debugC(2, kDebugParse | kDebugPreprocess, "wrapped mci command into quotes");
		}

		res1 = patchLingoCode(res1, archive, type, id, linenumber);

		res += res1;

		linenumber++;	// We do it here because of 'continue' statements

		if (*s)	// copy newline symbol
			res += *s++;
	}

	// Make the parser happier when there is no newline at the end
	res += '\n';

	debugC(2, kDebugParse | kDebugPreprocess, "#############\n%s\n#############", res.encode().c_str());

	return res;
}

MethodHash LingoCompiler::prescanMethods(const Common::U32String &code) {
	const Common::u32char_type_t *s = code.c_str();
	Common::U32String line, tok;
	MethodHash res;

	const Common::U32String macro("macro"), on("on"), method("method");

	while (*s) {
		line.clear();

		// Get next line
		while (*s && *s != '\n')
			line += tolower(*s++);

		const Common::u32char_type_t *contLine;
		tok = nexttok(line.c_str(), &contLine);

		if ((tok.equals(macro) || tok.equals(on) || tok.equals(method)) && *contLine != 0) {
			Common::U32String methodname = nexttok(contLine);

			res[methodname] = true;
		}

		if (*s)
			s++;	// Newline symbol
	}

	return res;
}

} // End of namespace Director
