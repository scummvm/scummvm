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

#include "glk/archetype/parser.h"
#include "glk/archetype/archetype.h"
#include "common/algorithm.h"

namespace Glk {
namespace Archetype {

//const int WORD_LEN = 32; // FIXME: Unused. Remove?

struct ParseType {
	StringPtr word;
	int object;

	ParseType() : word(nullptr), object(0) {}
};
typedef ParseType *ParsePtr;

/**
 * Is a word character
 */
static bool isWordChar(char c) {
	return Common::isAlnum(c) || c == '-' || c == '\"';
}

/**
 * Puts into lowercase the given character.
 */
static char locase(char ch) {
	return tolower(ch);
}

void normalize_string(const String &first, String &second) {
	int i, j, lfirst;
	bool in_word, done;

	i = j = 0;
	in_word = false;
	done = false;
	lfirst = first.size() - 1;
	second = " ";

	do {
		if (i > lfirst || !isWordChar(first[i])) {
			if (in_word) {
				j = 0;
				in_word = false;
				second = second + " ";
			} else {
				++i;
			}

			if (i > lfirst)
				done = true;
		
		} else if (in_word) {
			if (j < g_vm->Abbreviate) {
				second = second + locase(first[i]);
				++j;
			}

			++i;
		} else {
			in_word = true;
		}
	} while (!done);
}

void add_parse_word(TargetListType which_list, String &the_word, int the_object) {
	ListType the_list;
	String tempstr;
	NodePtr np;
	ParsePtr pp;
	int bar;

	if (which_list == PARSER_VERBLIST)
		the_list = g_vm->verb_names;
	else
		the_list = g_vm->object_names;

	the_word += '|';

	for (;;) {
		bar = the_word.indexOf('|');
		if (bar == -1)
			break;

		pp = new ParseType();
		tempstr = the_word.left(bar).left(g_vm->Abbreviate);

		pp->word = NewConstStr(tempstr);
		pp->word->toLowercase();
		the_word = String(the_word.c_str() + bar + 1);
		pp->object = the_object;

		np = new NodeType();
		np->key = pp->word->size();
		np->data = pp;

		insert_item(the_list, np);
	}
}

static void parse_sentence_substitute(int start, ParsePtr pp, int &next_starting) {
	int sublen = pp->word->size();

	if (sublen > g_vm->Abbreviate)
		sublen = g_vm->Abbreviate;

	// WORKAROUND: Original encoded object number as two bytes. ScummVM strings don't like
	// 0 bytes in the middle of the string, so we encode it as plain text
	g_vm->Command = g_vm->Command.left(start)
		+ String::format(" %%%d^", pp->object)
		+ g_vm->Command.mid(start + sublen + 1);

	next_starting = next_starting - sublen + 4;
}

static bool parse_sentence_next_chunk(int &start_at, String &the_chunk, int &next_starting) {
	int i;

	if (next_starting == -1) {
		return false;
	} else {
		do {
			start_at = next_starting;
			the_chunk = g_vm->Command.mid(start_at);

			i = the_chunk.indexOf('%');
			if (i == -1) {
				next_starting = -1;
			} else {
				next_starting = the_chunk.indexOf("^", i) + 1;
				assert(next_starting != 0);

				the_chunk = the_chunk.left(i);
			}

			the_chunk.trim();
		} while (!(next_starting == -1 || !the_chunk.empty()));

		return !the_chunk.empty();
	}
}

void parse_sentence() {
	const int nfillers = 3;
	const char *const FILTERS[nfillers] = { " a ", " an ", " the " };
	int next_starting = 0;
	String s;
	NodePtr np, near_match, far_match;
	ParsePtr pp;
	int i, lchunk;

	// Rip out those fillers
	s = g_vm->Command;
	for (i = 0; i < nfillers; ++i) {
		int filterIndex;
		while ((filterIndex = g_vm->Command.indexOf(FILTERS[i])) != -1)
			g_vm->Command.del(filterIndex, strlen(FILTERS[i]) - 1);
	}

	// Restore the original string if filler removal destroyed it completely
	if (g_vm->Command == " ")
		g_vm->Command = s;

	// Two passes: one matching all verbs and prepositions from the verb list, longest strings first

	np = nullptr;
	while (iterate_list(g_vm->verb_names, np)) {
		pp = (ParsePtr)np->data;
		s = String::format(" %s ", pp->word->left(g_vm->Abbreviate).c_str());

		i = g_vm->Command.indexOf(s);
		if (i != -1)
			parse_sentence_substitute(i, pp, next_starting);
	}

	// Second pass:  carefully search for the remaining string chunks; search only the part
	// of the noun list of the same length; give preference to those in the Proximate list
	next_starting = 0;

	while (parse_sentence_next_chunk(i, s, next_starting)) {
		lchunk = s.size();

		np = find_item(g_vm->object_names, lchunk);
		if (np != nullptr) {
			near_match = nullptr;
			far_match = nullptr;

			do {
				pp = (ParsePtr)np->data;
				if (pp->word->left(g_vm->Abbreviate) == s) {
					if (find_item(g_vm->Proximate, pp->object) != nullptr)
						near_match = np;
					else
						far_match = np;
				}
			} while (iterate_list(g_vm->object_names, np) && (lchunk = (int)((ParsePtr)np->data)->word->size()) != 0);

			if (near_match != nullptr)
				parse_sentence_substitute(i, (ParsePtr)near_match->data, next_starting);
			else if (far_match != nullptr)
				parse_sentence_substitute(i, (ParsePtr)far_match->data, next_starting);
		}
	}

	g_vm->Command.trim();
}

bool pop_object(int &intback, String &strback) {
	int i;

	if (g_vm->Command.empty()) {
		return false;
	} else {
		if (g_vm->Command.firstChar() == '%') {
			// parsed object
			int nextPos = -1;
			intback = String(g_vm->Command.c_str() + 1).val(&nextPos);
			g_vm->Command = String(g_vm->Command.c_str() + nextPos + 1);
		} else {
			intback = -1;
			i = g_vm->Command.indexOf('%');
			if (i < 0)
				i = g_vm->Command.size();

			strback = g_vm->Command.left(i);
			g_vm->Command = g_vm->Command.mid(i);
			strback.trim();
		}

		g_vm->Command.trim();
		return true;
	}
}

int find_object(const String &s) {
	NodePtr np;

	np = nullptr;
	while (iterate_list(g_vm->object_names, np)) {
		if (*((ParsePtr)np->data)->word == s)
			return ((ParsePtr)np->data)->object;
	}

	np = nullptr;
	while (iterate_list(g_vm->verb_names, np)) {
		if (*((ParsePtr)np->data)->word == s)
			return ((ParsePtr)np->data)->object;
	}

	return 0;
}

void clear_parse_list(ListType &the_list) {
	ParsePtr pp;
	NodePtr np = nullptr;

	while (iterate_list(the_list, np)) {
		pp = (ParsePtr)np->data;
		FreeConstStr(pp->word);
		delete pp;
	}

	dispose_list(the_list);
	new_list(the_list);
}

void new_parse_list() {
	clear_parse_list(g_vm->verb_names);
	clear_parse_list(g_vm->object_names);
}

} // End of namespace Archetype
} // End of namespace Glk
