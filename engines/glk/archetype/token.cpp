/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers || c == whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License || c == or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not || c == g_vm->write to the Free Software
 * Foundation || c == Inc. || c == 51 Franklin Street || c == Fifth Floor || c == Boston || c == MA 02110-1301 || c == USA.
 *
 */

#include "glk/archetype/token.h"
#include "glk/archetype/archetype.h"
#include "glk/archetype/id_table.h"
#include "glk/archetype/misc.h"
#include "glk/archetype/keywords.h"

namespace Glk {
namespace Archetype {

enum StateType { START, STOP, DECIDE, WHITE, COMMENT, QUOTE, LITERAL, IDENTIFIER, NUMBER, OPERATOR };

bool isWhitespace(char c) {
	return c == ' ' || c == '\t' || c == NEWLINE_CH;
}

bool isLiteralType(char c) {
	return c == '"' || c == '\'';
}

bool isLetter(char c) {
	return Common::isAlpha(c);
}

bool isDigit(char c) {
	return Common::isDigit(c);
}

bool isStartChar(char c) {
	return Common::isAlpha(c) || c == '_';
}

bool isIdChar(char c) {
	return isStartChar(c) || isDigit(c);
}

bool isLongOper(char c) {
	return c == '<' || c == '>' || c == ':' || c == '+' || c == '-' || c == '*'
		|| c == '/' || c == '&' || c == '~';
}

bool isOperChar(char c) {
	return isLongOper(c) || c == '=' || c == '.' || c == '^' || c == '?';
}

/**
 * Performs a binary search on the given ordered array, passing back the
 * index of the given string if it's in the array.
 * Used for quickly finding an operator or reserved word.
 * @param the_array		ordered array of short strings
 * @param elements		number of elements in the array
 * @param match_str		string to match
 * @param a_index		Outputs the array index
 * @returns				true if match_str was an element in the_array; false otherwise
 */
static bool binary_search(const LookupType &the_array, int elements,
		const ShortStringType &match_str, int &a_index) {
	int left = 0, right = elements - 1, mid;

	do {
		mid = (left + right) / 2;
		if (match_str < the_array[mid])
			right = mid - 1;
		else
			left = mid + 1;
	} while (match_str != the_array[mid] && left <= right);

	if (match_str != the_array[mid]) {
		return false;
	} else {
		a_index = mid;
		return true;
	}
}

/**
 * Searches the given unordered xarray for a string matching the given
 * string; if found, returns the index in the list of the string.  If
 * not found, adds it to the list.
 * @param the_xarray		xarray to be searched
 * @param the_str			string to be compared
 * @returns					The index of the_str in the_xarray.
 */
static int add_unique_str(XArrayType &the_xarray, const String &the_str) {
	StringPtr new_str;
	int i;
	void *p;

	// Duplicate the given string
	new_str = NewConstStr(the_str);

	if (the_xarray.empty()) {
		append_to_xarray(the_xarray, (void *)new_str);
		return the_xarray.size();
	} else {
		i = 1;
		while (index_xarray(the_xarray, i, p) && *((StringPtr)p) != the_str)
			++i;

		if (*((StringPtr)p) == the_str) {
			FreeConstStr(new_str);
			return i;
		} else {
			append_to_xarray(the_xarray, (void *)new_str);
			return the_xarray.size();
		}
	}
}

/**
 * Similar to the above, except that it is to be used when the strings are
 * not expected to repeat much.
 */
static int add_non_unique_str(XArrayType &the_xarray, const String &the_str) {
	append_to_xarray(the_xarray, (void *)NewConstStr(the_str));
	return the_xarray.size();
}

bool get_token(progfile &f) {
	StateType state;
	bool more_chars;
	char bracket, next_ch = '\0';
	String s;

	// Check for old token.  f.newlines may have changed while an old token was unconsumed,
	// so if the unconsumed token was a NEWLINE and f.newlines is false, we must continue
	// and get another token; otherwise we jump out with what we have
	if (!f.consumed) {
		f.consumed = true;

		if (!((f.ttype == NEWLINE) && !f.newlines))
			return true;
	}

	more_chars = true;
	state      = START;

	while (state != STOP) {
		switch (state) {
		case START:
			if (f.readChar(next_ch)) {
				state = DECIDE;
			} else {
				more_chars = false;
				state = STOP;
			}
			break;

		case DECIDE:
			if (!more_chars)
				state = STOP;
			else if (isWhitespace(next_ch))
				state = WHITE;
			else if (isLiteralType(next_ch))
				state = LITERAL;
			else if (isStartChar(next_ch))
				state = IDENTIFIER;
			else if (isDigit(next_ch))
				state = NUMBER;
			else if (isOperChar(next_ch))
				state = OPERATOR;
			else {
				// a single-character token
				switch (next_ch) {
				case '#':
					state = COMMENT;
				// fallthrough
				// FIXME: is this fallthrough intentional?
				case ';':
					if (!f.newlines) {
						state = START;
					} else {
						f.ttype = NEWLINE;
						f.tnum = (int)NEWLINE_CH;
						state = STOP;
					}
					// fallthrough
					// FIXME: is this fallthrough intentional?
				default:
					f.ttype = PUNCTUATION;
					f.tnum = (int)next_ch;
					state = STOP;
					break;
				}
			}
			break;

		case WHITE:
			while (state == WHITE && isWhitespace(next_ch)) {
				if (next_ch == NEWLINE_CH && f.newlines) {
					f.ttype = NEWLINE;
					state = STOP;
				} else {
					more_chars = f.readChar(next_ch);
				}
			}
			if (state == WHITE) {
				if (more_chars)
					// decide on new non-white character
					state = DECIDE;
				else
					state = STOP;
			}
			break;

		case COMMENT:
		case QUOTE:
			s = "";
			more_chars = f.readChar(next_ch);
			while (more_chars && next_ch != NEWLINE_CH) {
				s = s + next_ch;
				more_chars = f.readChar(next_ch);
			}
			if (state == COMMENT) {
				if (more_chars)
					state = START;
				else
					state = STOP;
			} else {
				// quoted literal
				f.unreadChar(next_ch);           // leave \n for the next guy
				f.ttype    = QUOTE_LIT;
				f.tnum     = add_non_unique_str(g_vm->Literals, s);
				state = STOP;
			}
			break;

		case LITERAL:
			bracket = next_ch;
			s = "";
			more_chars = f.readChar(next_ch);     // start the loop
			while (more_chars && next_ch != NEWLINE_CH && next_ch != bracket) {
				if (next_ch == '\\') {
					more_chars = f.readChar(next_ch);
					switch (next_ch) {
					case 't':
						next_ch = '\t';
						break;
					case 'b':
						next_ch = '\x8';
						break;
					case 'e':
						next_ch = (char)27;
						break;
					case'n':
						s = s + '\r';
						next_ch = '\n';
						break;
					default:
						break;
					}
				}
				s = s + next_ch;

				more_chars = f.readChar(next_ch);
			}

			if (next_ch != bracket) {
				f.sourcePos();
				error("Unterminated literal");
			} else {
				switch (bracket) {
				case '"':
					f.ttype = TEXT_LIT;
					f.tnum = add_non_unique_str(g_vm->Literals, s);
					break;
				case '\'':
					f.ttype = MESSAGE;
					f.tnum = add_unique_str(g_vm->Vocabulary, s);
					break;
				default:
					error("Programmer error: unknown literal type");
					break;
				}

				state = STOP;
			}
			break;

		case IDENTIFIER:
			s = "";
			while (isIdChar(next_ch)) {
				s = s + next_ch;
				more_chars = f.readChar(next_ch);
			}
			if (!isIdChar(next_ch))
				f.unreadChar(next_ch);

			// Check for reserved words or operators
			if (binary_search(Reserved_Wds, NUM_RWORDS, s, f.tnum))
				f.ttype = RESERVED;
			else if (binary_search(Operators, NUM_OPERS, s, f.tnum))
				f.ttype = OPER;
			else {
				f.ttype = IDENT;
				f.tnum = add_ident(s);
			}

			state = STOP;
			break;

		case NUMBER:
			s = "";
			while (more_chars && isDigit(next_ch)) {
				s = s + next_ch;
				more_chars = f.readChar(next_ch);
			}

			if (!isDigit(next_ch))
				f.unreadChar(next_ch);
			f.ttype = NUMERIC;

			f.tnum = atoi(s.c_str());
			state = STOP;
			break;

		case OPERATOR:
			s = "";

			while (more_chars && isLongOper(next_ch) && s != ">>") {
				// have to stop short with >>
				s = s + next_ch;
				more_chars = f.readChar(next_ch);
			}

			if (s == ">>") {
				f.unreadChar(next_ch);
				state = QUOTE;
			} else {
				if (!isOperChar(next_ch))
					f.unreadChar(next_ch);
				else
					s = s + next_ch;

				state = STOP;

				if (s == ":") {
					f.ttype = PUNCTUATION;
					f.tnum = (int)':';
				} else if (!binary_search(Operators, NUM_OPERS, s, f.tnum)) {
					f.sourcePos();
					error("Unknown operator %s", s.c_str());
				} else {
					f.ttype = OPER;
				}
			}
			break;

		default:
			break;
		}
	}

	return more_chars;
}

void write_token(AclType the_type, int the_number) {
	StringPtr str_ptr;
	IdRecPtr the_id_ptr;
	void *p;

	switch (the_type) {
	case IDENT:
		if (the_number < 0) {
			g_vm->write("an identifier");
		} else {
			g_vm->write("<identifier %d >: ", the_number);
			if (index_ident(the_number, the_id_ptr))
				g_vm->write("\"%s\"", the_id_ptr->id_name);
		}
		break;

	case RESERVED:
		if (the_number < 0)
			g_vm->write("a reserved word");
		else
			g_vm->write("reserved word \"%s\"", Reserved_Wds[the_number]);
		break;

	case OPER:
		if (the_number < 0)
			g_vm->write("an operator");
		else
			g_vm->write("operator \"%s\"", Operators[the_number]);
		break;

	case PUNCTUATION:
		g_vm->write("%c", (char)the_number);
		break;

	case TEXT_LIT:
		if (the_number < 0)
			g_vm->write("a text literal");
		else if (index_xarray(g_vm->Literals, the_number, p)) {
			str_ptr = (StringPtr)p;
			g_vm->write("\"%s\"", str_ptr->c_str());
		} else {
			g_vm->write("<text literal %d >: ", the_number);
		}
		break;

	case MESSAGE:
		if (the_number < 0)
			g_vm->write("a message");
		else if (index_xarray(g_vm->Vocabulary, the_number, p)) {
			str_ptr = (StringPtr)p;
			g_vm->write("\'%s\'", str_ptr->c_str());
		} else {
			g_vm->write("<message %d>: ", the_number);
		}
		break;

	case NUMERIC:
		g_vm->write("the number %d", the_number);
		break;

	default:
		g_vm->write("<unknown token>");
	}
}

} // End of namespace Archetype
} // End of namespace Glk
