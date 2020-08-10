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
#include "director/lingo/lingo.h"

namespace Director {

Common::String preprocessWhen(Common::String in, bool *changed);
Common::String preprocessPlay(Common::String in);
Common::String preprocessSound(Common::String in);

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

static Common::String prevtok(const char *s, const char *lineStart, const char **newP = nullptr) {
	Common::String res;

	// Scan first non-whitespace
	while (s >= lineStart && (*s == ' ' || *s == '\t' || *s == '\xC2')) // If we see a whitespace
		if (s > lineStart) {
			s--;
		} else {
			break;
		}

	if (*s == '"') { // If it is a string then scan till end quote
		res += *s--;

		while (s >= lineStart && *s != '"')
			res = *s-- + res;

		if (*s == '"')
			res = *s-- + res;
	} else if (Common::isAlnum(*s)) { 	// Now copy everything till whitespace
		// Now copy everything till whitespace
		while (s >= lineStart && (Common::isAlnum(*s) || *s == '.' || *s == '#' || *s == '_'))
			res = *s-- + res;
	} else {
		while (s >= lineStart && isspec(*s))
			res = *s-- + res;
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

Common::String Lingo::codePreprocessor(const char *s, LingoArchive *archive, ScriptType type, uint16 id, bool simple) {
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

	// Preprocess if statements
	// Here we add ' end if' at end of each statement, which lets us
	// make the grammar very straightforward
	Common::String line, tok, res1;
	const char *lineStart, *prevEnd;
	int iflevel = 0;
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
		debugC(2, kDebugParse | kDebugPreprocess, "line: %d                         '%s'", iflevel, line.c_str());

		if (!defFound && (type == kMovieScript || type == kCastScript) && (_vm->getVersion() < 400 || _vm->getCurrentMovie()->_allowOutdatedLingo)) {
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

		if (!changed) {
			res1 = preprocessPlay(res1);
			res1 = preprocessSound(res1);
		}

		res += res1;

		linenumber++;	// We do it here because of 'continue' statements

		if (line.size() < 4 || changed) { // If line is too small, then skip it
			if (*s)	// copy newline symbol
				res += *s++;

			debugC(2, kDebugParse | kDebugPreprocess, "too small");

			continue;
		}

		tok = nexttok(line.c_str(), &lineStart);
		if (tok.equals("if")) {
			tok = prevtok(&line.c_str()[line.size() - 1], lineStart, &prevEnd);
			debugC(2, kDebugParse | kDebugPreprocess, "start-if <%s>", tok.c_str());

			if (tok.equals("if")) {
				debugC(2, kDebugParse | kDebugPreprocess, "end-if");
				tok = prevtok(prevEnd, lineStart);

				if (tok.equals("end")) {
					// do nothing, we open and close same line
					debugC(2, kDebugParse | kDebugPreprocess, "end-end");
				} else {
					iflevel++;
				}
			} else if (tok.equals("then")) {
				debugC(2, kDebugParse | kDebugPreprocess, "last-then");
				iflevel++;
			} else if (tok.equals("else")) {
				debugC(2, kDebugParse | kDebugPreprocess, "last-else");
				iflevel++;
			} else { // other token
				// Now check if we have tNLELSE
				if (!*s) {
					iflevel++;	// end, we have to add 'end if'
					break;
				}
				const char *s1 = s + 1;

				while (*s1 && *s1 == '\n')
					s1++;
				tok = nexttok(s1);

				if (tok.equalsIgnoreCase("else")) { // ignore case because it is look-ahead
					debugC(2, kDebugParse | kDebugPreprocess, "tNLELSE");
					iflevel++;
				} else {
					debugC(2, kDebugParse | kDebugPreprocess, "++++ end if (no nlelse after single liner)");
					res += " end if";
				}
			}
		} else if (tok.equals("else")) {
			debugC(2, kDebugParse | kDebugPreprocess, "start-else");
			bool elseif = false;

			tok = nexttok(lineStart);
			if (tok.equals("if")) {
				debugC(2, kDebugParse | kDebugPreprocess, "second-if");
				elseif = true;
			} else if (tok.empty()) {
				debugC(2, kDebugParse | kDebugPreprocess, "lonely-else");

				if (*s)	// copy newline symbol
					res += *s++;

				continue;
			}

			tok = prevtok(&line.c_str()[line.size() - 1], lineStart, &prevEnd);
			debugC(2, kDebugParse | kDebugPreprocess, "last: '%s'", tok.c_str());

			if (tok.equals("if")) {
				debugC(2, kDebugParse | kDebugPreprocess, "end-if");
				tok = prevtok(prevEnd, lineStart);

				if (tok.equals("end")) {
					debugC(2, kDebugParse | kDebugPreprocess, "end-end");
					iflevel--;
				}
			} else if (tok.equals("then")) {
				debugC(2, kDebugParse | kDebugPreprocess, "last-then");

				if (elseif == false) {
					warning("Badly nested then");
				}
			} else if (tok.equals("else")) {
				debugC(2, kDebugParse | kDebugPreprocess, "last-else");
				if (elseif == false) {
					warning("Badly nested else");
				}
			} else { // check if we have tNLELSE or \nEND
				if (!*s) {
					break;
				}
				const char *s1 = s + 1;

				while (*s1 && *s1 == '\n')
					s1++;
				tok = nexttok(s1, &s1);

				if (tok.equalsIgnoreCase("else") && elseif) {
					// Nothing to do here, same level
					debugC(2, kDebugParse | kDebugPreprocess, "tNLELSE");
				} else if (tok.equalsIgnoreCase("end") && elseif) {
					tok = nexttok(s1);

					if (tok.equalsIgnoreCase("if")) {
						// Nothing to do here
						debugC(2, kDebugParse | kDebugPreprocess, "see-end-if");
					} else {
						debugC(2, kDebugParse | kDebugPreprocess, "++++ end if (no tNLELSE 2)");
						res += " end if";
						iflevel--;
					}
				} else {
					debugC(2, kDebugParse | kDebugPreprocess, "++++ end if (no tNLELSE)");
					res += " end if";
					iflevel--;
				}
			}
		} else if (tok.equals("end")) {
			debugC(2, kDebugParse | kDebugPreprocess, "start-end");

			tok = nexttok(lineStart);
			if (tok.equals("if")) {
				debugC(2, kDebugParse | kDebugPreprocess, "second-if");
				iflevel--;
			}
		} else if (tok.equals("when")) {
			debugC(2, kDebugParse | kDebugPreprocess, "start-when");

			if (strstr(lineStart, "if") && strstr(lineStart, "then")) {
				tok = prevtok(&line.c_str()[line.size() - 1], lineStart, &prevEnd);
				debugC(2, kDebugParse | kDebugPreprocess, "when-start-if <%s>", tok.c_str());

				if (tok.equals("if")) {
					debugC(2, kDebugParse | kDebugPreprocess, "when-end-if");
					tok = prevtok(prevEnd, lineStart);

					if (tok.equals("end")) {
						// do nothing, we open and close same line
						debugC(2, kDebugParse | kDebugPreprocess, "when-end-end");
					} else {
						res += " end if";
					}
				} else {
					res += " end if";
				}
			}
		} else {
			debugC(2, kDebugParse | kDebugPreprocess, "nothing");
		}

		if (*s)	// copy newline symbol
			res += *s++;
	}

	for (int i = 0; i < iflevel; i++) {
		debugC(2, kDebugParse | kDebugPreprocess, "++++ end if (unclosed)");
		res += "\nend if";
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

// play done -> play #done
Common::String preprocessPlay(Common::String in) {
	Common::String res, next;
	const char *ptr = in.c_str();
	const char *beg = ptr;
	const char *nextPtr;

	while ((ptr = scumm_strcasestr(beg, "play")) != NULL) {
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

		next = nexttok(ptr, &nextPtr);

		debugC(2, kDebugParse | kDebugPreprocess, "PLAY: nexttok: %s", next.c_str());

		if (next.equalsIgnoreCase("done")) {
			res += " #"; // Turn it into SYMBOL
		} else {
			res += ' ';
		}

		res += next;
		ptr = nextPtr;
		beg = ptr;
	}

	res += Common::String(beg);

	if (in.size() != res.size())
		debugC(2, kDebugParse | kDebugPreprocess, "PLAY: in: %s\nout: %s", in.c_str(), res.c_str());

	return res;
}

// sound fadeIn 5, 10 -> sound #fadeIn, 5, 10
Common::String preprocessSound(Common::String in) {
	Common::String res, next;
	const char *ptr = in.c_str();
	const char *beg = ptr;
	const char *nextPtr;

	while ((ptr = scumm_strcasestr(beg, "sound")) != NULL) {
		if (ptr != findtokstart(in.c_str(), ptr)) { // If we're in the middle of a word
			res += *beg++;
			continue;
		}

		ptr += 5; // end of 'sound'
		res += Common::String(beg, ptr);

		if (!*ptr)	// If it is end of the line
			break;

		if (Common::isAlnum(*ptr)) { // If it is in the middle of the word
			beg = ptr;
			continue;
		}

		next = nexttok(ptr, &nextPtr);

		debugC(2, kDebugParse | kDebugPreprocess, "SOUND: nexttok: %s", next.c_str());

		bool modified = false;

		if (next.equalsIgnoreCase("close") ||
				next.equalsIgnoreCase("fadeIn") ||
				next.equalsIgnoreCase("fadeOut") ||
				next.equalsIgnoreCase("playFile") ||
				next.equalsIgnoreCase("stop")) {
			res += '#'; // Turn it into SYMBOL
			modified = true;
		} else {
			res += ' ';
		}

		res += next;
		if (modified)
			res += ',';
		ptr = nextPtr;
		beg = ptr;
	}

	res += Common::String(beg);

	if (in.size() != res.size())
		debugC(2, kDebugParse | kDebugPreprocess, "SOUND: in: %s\nout: %s", in.c_str(), res.c_str());

	return res;
}

} // End of namespace Director
