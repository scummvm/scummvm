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

#include "director/director.h"
#include "director/movie.h"
#include "director/lingo/lingo-codegen.h"

namespace Director {

Common::String preprocessWhen(Common::String in, bool *changed);

bool isspec(char c) {
	return strchr("-+*/%%^:,()><&[]=", c) != NULL;
}

static Common::String nexttok(const char *s, const char **newP = nullptr) {
	Common::String res;

	// Scan first non-whitespace
	while (*s && (*s == ' ' || *s == '\t' || *s == '\xC2')) // If we see a whitespace
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

static const char *findtokstart(const char *start, const char *token) {
	// First, determine, if we sit inside of a string
	//
	// Since we do not have escaping characters, simple count is enough
	int numquotes = 0;
	const char *ptr = start;

	while (*ptr && ptr <= token) {
		if (*ptr == '"')
			numquotes++;
		ptr++;
	}

	// We're inside of quote. Scan backwards
	if (numquotes % 2) {
		while (*ptr != '"')
			ptr--;

		return ptr;
	}

	// If we're in the middle of a word
	while (ptr > start && Common::isAlnum(*(ptr - 1)))
		ptr--;

	return ptr;
}

Common::String LingoCompiler::codePreprocessor(const char *s, LingoArchive *archive, ScriptType type, CastMemberID id, bool simple) {
	Common::String res;

	// We start from processing the continuation synbols
	// \xC2\n  ->  \xC2
	// This will greatly simplify newline processing, still leaving
	// the line number tracking intact
	while (*s) {
		if (*s == '\xC2') {
			res += *s++;
			if (!*s)	// Who knows, maybe it is the last symbol in the script
				break;
			s++;
			continue;
		}
		res += *s++;
	}

	Common::String tmp(res);
	res.clear();
	s = tmp.c_str();

	// Strip comments
	bool inString = false;
	while (*s) {
		if (*s == '"')
			inString = !inString;

		if (!inString && *s == '-' && *(s + 1) == '-') { // At the end of the line we will have \0
			while (*s && *s != '\n')
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
		if (*s == ' ' || *s == '\t' || *s == '\xC2') { // If we see a whitespace
			const char *ps = s; // Remember where we saw it

			while (*ps == ' ' || *ps == '\t' || *ps == '\xC2') // Scan until end of whitespaces
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

	if (simple)
		return res;

	tmp = res;
	s = tmp.c_str();
	res.clear();

	Common::String line, tok, res1;
	int linenumber = 1;
	bool defFound = false;

	while (*s) {
		line.clear();
		res1.clear();

		// Get next line
		while (*s && *s != '\n') { // If we see a whitespace
			res1 += *s;
			line += tolower(*s++);

			if (*s == '\xc2')
				linenumber++;
		}
		debugC(2, kDebugParse | kDebugPreprocess, "line: '%s'", line.c_str());

		if (!defFound && (type == kMovieScript || type == kCastScript) && (g_director->getVersion() < 400 || g_director->getCurrentMovie()->_allowOutdatedLingo)) {
			tok = nexttok(line.c_str());
			if (tok.equals("macro") || tok.equals("factory") || tok.equals("on")) {
				defFound = true;
			} else {
				debugC(2, kDebugParse | kDebugPreprocess, "skipping line before first definition");
				linenumber++;
				if (*s)	// copy newline symbol
					res += *s++;
				continue;
			}
		}

		res1 = patchLingoCode(res1, archive, type, id, linenumber);

		bool changed = false;
		res1 = preprocessWhen(res1, &changed);

		res += res1;

		linenumber++;	// We do it here because of 'continue' statements

		if (*s)	// copy newline symbol
			res += *s++;
	}

	// Make the parser happier when there is no newline at the end
	res += '\n';

	debugC(2, kDebugParse | kDebugPreprocess, "#############\n%s\n#############", res.c_str());

	return res;
}

// when ID then statement -> when ID then "statement"
Common::String preprocessWhen(Common::String in, bool *changed) {
	Common::String res, next;
	const char *ptr = in.c_str();
	const char *beg = ptr;
	const char *nextPtr;

	while ((ptr = scumm_strcasestr(beg, "when")) != NULL) {
		if (ptr != findtokstart(in.c_str(), ptr)) { // If we're in the middle of a word
			res += *beg++;
			continue;
		}

		ptr += 4; // end of 'play'
		res += Common::String(beg, ptr);

		if (!*ptr)	// If it is end of the line
			break;

		if (Common::isAlnum(*ptr)) { // If it is in the middle of the word
			beg = ptr;
			continue;
		}

		*changed = true;

		res += ' ';
		next = nexttok(ptr, &nextPtr);	// ID
		res += next;

		res += ' ';
		next = nexttok(nextPtr, &nextPtr);	// then
		res += next;

		res += ' ';
		res += '"';

		// now we need to preprocess quotes
		bool skipQuote = false;
		while (*nextPtr) {
			if (*nextPtr == '"') {
				res += "\" & QUOTE ";

				if (*(nextPtr + 1))
					res += "& \"";
				else
					skipQuote = true;	// we do not want the last quote
			} else {
				res += *nextPtr;
			}

			nextPtr++;
		}

		if (!skipQuote)
			res += '"';

		beg = nextPtr;

		break;
	}

	res += Common::String(beg);

	if (in.size() != res.size())
		debugC(2, kDebugParse | kDebugPreprocess, "WHEN: in: %s\nout: %s", in.c_str(), res.c_str());

	return res;
}

} // End of namespace Director
