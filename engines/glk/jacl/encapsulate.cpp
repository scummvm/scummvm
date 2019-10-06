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

#include "glk/jacl/jacl.h"
#include "glk/jacl/types.h"
#include "glk/jacl/prototypes.h"

namespace Glk {
namespace JACL {

extern struct synonym_type *synonym_table;
extern struct filter_type *filter_table;

char            text_buffer[1024];

/* THIS IS A STRING CONSTANT TO POINT TO WHENEVER A COMMA IS
 * USED IN THE PLAYER'S INPUT */
const char     *comma = "comma\0";
const char     *then = "then\0";
const char     *word[MAX_WORDS];
int             quoted[MAX_WORDS];
int             percented[MAX_WORDS];
int             wp;

void
encapsulate() {
	int             index,
	                length;
	int             position = 0;
	int             new_word = TRUE;

	length = strlen(text_buffer);

	/* QUOTED IS USED TO STORE WHETHER EACH WORD WAS ENCLOSED IN QUOTES
	 * IN THE PLAYERS COMMAND - RESET EACH WORD TO NO */
	for (index = 0; index < MAX_WORDS; index++) {
		quoted[index] = 0;
		percented[index] = 0;
	}

	for (index = 0; index < length; index++) {

		switch (text_buffer[index]) {
		case ':':
		case '\t':
		case ' ':
		case ',':
			text_buffer[index] = 0;
			new_word = TRUE;
			break;
		case ';':
		case '#':
		case '\r':
		case '\n':
			/* TERMINATE THE WHOLE COMMAND ON HITTING A NEWLINE CHARACTER, A
			 * SEMICOLON OR A HASH */
			text_buffer[index] = 0;
			index = length;
			break;
		case '"':
			index++;
			/* NEED TO REMEMBER THAT THIS WORD WAS ENCLOSED IN QUOTES FOR
			 * THE COMMAND 'write'*/
			quoted[position] = 1;

			word[position] = &text_buffer[index];

			if (position < MAX_WORDS)
				position++;

			/* IF A WORD IS ENCLOSED IN QUOTES, KEEP GOING UNTIL THE END
			 * OF THE LINE OR A CLOSING QUOTE IS FOUND, NOT BREAKING AT
			 * WHITESPACE AS USUAL */
			for (; index < length; index++) {
				if (text_buffer[index] == '"') {
					text_buffer[index] = 0;
					new_word = TRUE;
					break;
				} else if (text_buffer[index] == '\r' || text_buffer[index] == '\n') {
					text_buffer[index] = 0;
					index = length;
					break;
				}
			}
			break;
		default:
			if (new_word) {
				if (text_buffer[index] == '%' && text_buffer[index + 1] != ' ' && text_buffer[index + 1] != '\t') {
					percented[position]++;
					break;
				}
				word[position] = &text_buffer[index];
				new_word = FALSE;
				if (position < MAX_WORDS)
					position++;
			}
			break;
		}

	}

	/* NULL OUT ALL THE WORD POINTERS BEYOND THE LAST WORD */
	for (index = position; index < MAX_WORDS; index++)
		word[index] = NULL;

	wp = 0;
}

// THIS VERSION OF ENCAPSULATE DOESN'T LOOK FOR CERTAIN SPECIAL CHARACTERS
void
command_encapsulate() {
	int             index,
	                length;
	int             position = 0;
	int             new_word = TRUE;

	length = strlen(text_buffer);

	// QUOTED IS USED TO STORE WHETHER EACH WORD WAS ENCLOSED IN QUOTES
	// IN THE PLAYERS COMMAND - RESET EACH WORD TO NO
	for (index = 0; index < MAX_WORDS; index++) {
		quoted[index] = 0;
	}

	for (index = 0; index < length; index++) {

		// REDUSE EVERYTHING TO LOWER CASE EXCEPT TEXT ENCLOSED IN QUOTES
		text_buffer[index] = tolower((int) text_buffer[index]);

		switch (text_buffer[index]) {
		case ':':
		case '\t':
		case ' ':
			text_buffer[index] = 0;
			new_word = TRUE;
			break;
		case ',':
			text_buffer[index] = 0;
			// SET THIS WORD TO POINT TO A STRING CONSTANT OF 'comma' AS THE
			// COMMA ITSELF WILL BE NULLED OUT TO TERMINATE THE PRECEEDING
			// WORD IN THE COMMAND.
			word[position] = comma;
			if (position < MAX_WORDS)
				position++;
			new_word = TRUE;
			break;
		case '.':
			text_buffer[index] = 0;
			// SET THIS WORD TO POINT TO A STRING CONSTANT OF 'comma' AS THE
			// COMMA ITSELF WILL BE NULLED OUT TO TERMINATE THE PRECEEDING
			// WORD IN THE COMMAND
			word[position] = then;
			if (position < MAX_WORDS)
				position++;
			new_word = TRUE;
			break;
		case ';':
		case '\r':
		case '\n':
			// TERMINATE THE WHOLE COMMAND ON HITTING A NEWLINE CHARACTER, A
			// SEMICOLON OR A HASH
			text_buffer[index] = 0;
			index = length;
			break;
		case '"':
			index++;
			// NEED TO REMEMBER THAT THIS WORD WAS ENCLOSED IN QUOTES FOR
			// THE COMMAND 'write'
			quoted[position] = 1;

			word[position] = &text_buffer[index];

			if (position < MAX_WORDS)
				position++;

			// IF A WORD IS ENCLOSED IN QUOTES, KEEP GOING UNTIL THE END
			// OF THE LINE OR A CLOSING QUOTE IS FOUND, NOT BREAKING AT
			// WHITESPACE AS USUAL
			for (; index < length; index++) {
				if (text_buffer[index] == '"') {
					text_buffer[index] = 0;
					new_word = TRUE;
					break;
				} else if (text_buffer[index] == '\r' || text_buffer[index] == '\n') {
					text_buffer[index] = 0;
					index = length;
					break;
				}
			}
			break;
		default:
			if (new_word) {
				word[position] = &text_buffer[index];
				new_word = FALSE;
				if (position < MAX_WORDS)
					position++;
			}
			break;
		}

	}

	// NULL OUT ALL THE WORD POINTERS BEYOND THE LAST WORD
	for (index = position; index < MAX_WORDS; index++) {
		word[index] = NULL;
	}

	wp = 0;
}

void
jacl_truncate() {
	int             index,
	                counter,
	                match;
	int             position = 0;

	struct synonym_type *synonym;
	struct filter_type *filter = filter_table;

	// REMOVE ALL THE DEFINED 'filter's FROM THE PLAYER'S COMMAND
	if (filter != NULL) {
		while (word[position] != NULL) {
			match = FALSE;
			do {
				if (!strcmp(word[position], filter->word)) {
					for (index = position; word[index + 1] != NULL;
					        index++)
						word[index] = word[index + 1];
					word[index] = NULL;
					match = TRUE;
				}
				filter = filter->next_filter;
			} while (filter != NULL && !match);
			filter = filter_table;
			if (!match)
				position++;
		};
	}

	// SUBTITUTE ALL THE DEFINED 'synonym's IN THE PLAYER'S COMMAND
	if (synonym_table != NULL) {
		for (counter = 0; word[counter] != NULL; counter++) {
			synonym = synonym_table;
			do {
				if (!strcmp(word[counter], synonym->original)) {
					word[counter] = synonym->standard;
					break;
				}
				if (synonym->next_synonym != NULL)
					synonym = synonym->next_synonym;
				else
					break;
			} while (TRUE);
		}
	}
}

} // End of namespace JACL
} // End of namespace Glk
