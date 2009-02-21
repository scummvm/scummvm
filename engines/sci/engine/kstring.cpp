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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* String and parser handling */

#include "sci/include/sciresource.h"
#include "sci/include/engine.h"
#include "sci/engine/message.h"

namespace Sci {

#define CHECK_OVERFLOW1(pt, size, rv) \
	if (((pt) - (str_base)) + (size) > maxsize) { \
		error("String expansion exceeded heap boundaries\n"); \
		return rv;\
	}

/* Returns the string the script intended to address */
char *kernel_lookup_text(EngineState *s, reg_t address, int index) {
	char *seeker;
	resource_t *textres;

	if (address.segment)
		return (char *)kernel_dereference_bulk_pointer(s, address, 0);
	else {
		int textlen;
		int _index = index;
		textres = scir_find_resource(s->resmgr, sci_text, address.offset, 0);

		if (!textres) {
			error("text.%03d not found", address.offset);
			return NULL; /* Will probably segfault */
		}

		textlen = textres->size;
		seeker = (char *) textres->data;

		while (index--)
			while ((textlen--) && (*seeker++))
				;

		if (textlen)
			return seeker;
		else {
			error("Index %d out of bounds in text.%03d\n", _index, address.offset);
			return 0;
		}

	}
}


/*************************************************************/
/* Parser */
/**********/

#ifdef SCI_SIMPLE_SAID_CODE
int vocab_match_simple(EngineState *s, heap_ptr addr) {
	int nextitem;
	int listpos = 0;

	if (!s->parser_valid)
		return SAID_NO_MATCH;

	if (s->parser_valid == 2) { /* debug mode: sim_said */
		do {
			sciprintf("DEBUGMATCH: ");
			nextitem = s->heap[addr++];

			if (nextitem < 0xf0) {
				nextitem = nextitem << 8 | s->heap[addr++];
				if (s->parser_nodes[listpos].type
				        || nextitem != s->parser_nodes[listpos++].content.value)
					return SAID_NO_MATCH;
			} else {

				if (nextitem == 0xff)
					return (s->parser_nodes[listpos++].type == -1) ? SAID_FULL_MATCH : SAID_NO_MATCH; /* Finished? */

				if (s->parser_nodes[listpos].type != 1
				        || nextitem != s->parser_nodes[listpos++].content.value)
					return SAID_NO_MATCH;

			}
		} while (42);
	} else { /* normal simple match mode */
		return vocab_simple_said_test(s, addr);
	}
}
#endif /* SCI_SIMPLE_SAID_CODE */


reg_t kSaid(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t heap_said_block = argv[0];
	byte *said_block;
	int new_lastmatch;

	if (!heap_said_block.segment)
		return NULL_REG;

	said_block = (byte *) kernel_dereference_bulk_pointer(s, heap_said_block, 0);

	if (!said_block) {
		warning("Said on non-string, pointer "PREG"", PRINT_REG(heap_said_block));
		return NULL_REG;
	}

	if (s->debug_mode & (1 << SCIkPARSER_NR)) {
		SCIkdebug(SCIkPARSER, "Said block:", 0);
		vocab_decypher_said_block(s, said_block);
	}

	if (IS_NULL_REG(s->parser_event) || (GET_SEL32V(s->parser_event, claimed))) {
		return NULL_REG;
	}

#ifdef SCI_SIMPLE_SAID_CODE

	s->acc = 0;

	if (s->parser_lastmatch_word == SAID_FULL_MATCH)
		return; /* Matched before; we're not doing any more matching work today. */

	if ((new_lastmatch = vocab_match_simple(s, said_block)) != SAID_NO_MATCH) {

		if (s->debug_mode & (1 << SCIkPARSER_NR))
			sciprintf("Match (simple).\n");
		s->acc = 1;

		if (new_lastmatch == SAID_FULL_MATCH) /* Finished matching? */
			PUT_SELECTOR(s->parser_event, claimed, 1); /* claim event */
		/* otherwise, we have a partial match: Set new lastmatch word in all cases. */

		s->parser_lastmatch_word = new_lastmatch;
	}

#else /* !SCI_SIMPLE_SAID_CODE */
	if ((new_lastmatch = said(s, said_block, (s->debug_mode & (1 << SCIkPARSER_NR))))
	        != SAID_NO_MATCH) { /* Build and possibly display a parse tree */

		if (s->debug_mode & (1 << SCIkPARSER_NR))
			sciprintf("Match.\n");

		s->r_acc = make_reg(0, 1);

		if (new_lastmatch != SAID_PARTIAL_MATCH)
			PUT_SEL32V(s->parser_event, claimed, 1);

		s->parser_lastmatch_word = new_lastmatch;

	} else {
		s->parser_lastmatch_word = SAID_NO_MATCH;
		return NULL_REG;
	}
#endif /* !SCI_SIMPLE_SAID_CODE */
	return s->r_acc;
}


reg_t kSetSynonyms(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t object = argv[0];
	list_t *list;
	node_t *node;
	int script;
	int synpos = 0;

	if (s->synonyms_nr)
		free(s->synonyms);

	s->synonyms_nr = 0;

	list = LOOKUP_LIST(GET_SEL32(object, elements));
	node = LOOKUP_NODE(list->first);

	while (node) {
		reg_t objpos = node->value;
		int seg;
		int synonyms_nr = 0;

		script = GET_SEL32V(objpos, number);
		seg = sm_seg_get(&(s->seg_manager), script);

		if (seg >= 0) synonyms_nr = sm_get_synonyms_nr(&(s->seg_manager), seg, SEG_ID);

		if (synonyms_nr) {
			byte *synonyms;

			synonyms = sm_get_synonyms(&(s->seg_manager), seg, SEG_ID);
			if (synonyms) {
				int i;
				if (s->synonyms_nr)
					s->synonyms = (synonym_t*)sci_realloc(s->synonyms,
					                                      sizeof(synonym_t) * (s->synonyms_nr + synonyms_nr));
				else
					s->synonyms = (synonym_t*)sci_malloc(sizeof(synonym_t) * synonyms_nr);

				s->synonyms_nr +=  synonyms_nr;

				SCIkdebug(SCIkPARSER, "Setting %d synonyms for script.%d\n",
				          synonyms_nr, script);

				if (synonyms_nr > 16384) {
					error("Segtable corruption: script.%03d has %d synonyms",
					         script, synonyms_nr);
					/* We used to reset the corrupted value here. I really don't think it's appropriate.
					 * Lars */
				} else
					for (i = 0; i < synonyms_nr; i++) {
						s->synonyms[synpos].replaceant = getInt16(synonyms + i * 4);
						s->synonyms[synpos].replacement = getInt16(synonyms + i * 4 + 2);

						synpos++;
					}
			} else
				warning("Synonyms of script.%03d were requested, but script is not available", script);

		}

		node = LOOKUP_NODE(node->succ);
	}

	SCIkdebug(SCIkPARSER, "A total of %d synonyms are active now.\n", s->synonyms_nr);

	if (!s->synonyms_nr)
		s->synonyms = NULL;
	return s->r_acc;
}



reg_t kParse(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t stringpos = argv[0];
	char *string = kernel_dereference_char_pointer(s, stringpos, 0);
	int words_nr;
	char *error;
	result_word_t *words;
	reg_t event = argv[1];

	s->parser_event = event;

	s->parser_lastmatch_word = SAID_NO_MATCH;

	if (s->parser_valid == 2) {
		sciprintf("Parsing skipped: Parser in simparse mode\n");
		return s->r_acc;
	}

	words = vocab_tokenize_string(string, &words_nr,
	                              s->parser_words, s->parser_words_nr,
	                              s->parser_suffices, s->parser_suffices_nr,
	                              &error);
	s->parser_valid = 0; /* not valid */

	if (words) {

		int syntax_fail = 0;

		vocab_synonymize_tokens(words, words_nr, s->synonyms, s->synonyms_nr);

		s->r_acc = make_reg(0, 1);

		if (s->debug_mode & (1 << SCIkPARSER_NR)) {
			int i;

			SCIkdebug(SCIkPARSER, "Parsed to the following blocks:\n", 0);

			for (i = 0; i < words_nr; i++)
				SCIkdebug(SCIkPARSER, "   Type[%04x] Group[%04x]\n", words[i].w_class, words[i].group);
		}

		if (vocab_build_parse_tree(&(s->parser_nodes[0]), words, words_nr, s->parser_branches,
		                           s->parser_rules))
			syntax_fail = 1; /* Building a tree failed */

#ifdef SCI_SIMPLE_SAID_CODE
		vocab_build_simple_parse_tree(&(s->parser_nodes[0]), words, words_nr);
#endif /* SCI_SIMPLE_SAID_CODE */

		free(words);

		if (syntax_fail) {

			s->r_acc = make_reg(0, 1);
			PUT_SEL32V(event, claimed, 1);

			invoke_selector(INV_SEL(s->game_obj, syntaxFail, 0), 2, s->parser_base, stringpos);
			/* Issue warning */

			SCIkdebug(SCIkPARSER, "Tree building failed\n");

		} else {
			s->parser_valid = 1;
			PUT_SEL32V(event, claimed, 0);
#ifndef SCI_SIMPLE_SAID_CODE
			if (s->debug_mode & (1 << SCIkPARSER_NR))
				vocab_dump_parse_tree("Parse-tree", s->parser_nodes);
#endif /* !SCI_SIMPLE_SAID_CODE */
		}

	} else {

		s->r_acc = make_reg(0, 0);
		PUT_SEL32V(event, claimed, 1);
		if (error) {
			char *pbase_str = kernel_dereference_char_pointer(s, s->parser_base, 0);
			strcpy(pbase_str, error);
			SCIkdebug(SCIkPARSER, "Word unknown: %s\n", error);
			/* Issue warning: */

			invoke_selector(INV_SEL(s->game_obj, wordFail, 0), 2, s->parser_base, stringpos);
			free(error);
			return make_reg(0, 1); /* Tell them that it dind't work */
		}
	}

	return s->r_acc;
}


reg_t kStrEnd(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t address = argv[0];
	char *seeker = kernel_dereference_char_pointer(s, address, 0);

	while (*seeker++)
		++address.offset;

	return address;
}

reg_t kStrCat(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *s1 = kernel_dereference_char_pointer(s, argv[0], 0);
	char *s2 = kernel_dereference_char_pointer(s, argv[1], 0);

	strcat(s1, s2);
	return argv[0];
}

reg_t kStrCmp(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *s1 = kernel_dereference_char_pointer(s, argv[0], 0);
	char *s2 = kernel_dereference_char_pointer(s, argv[1], 0);

	if (argc > 2)
		return make_reg(0, strncmp(s1, s2, UKPV(2)));
	else
		return make_reg(0, strcmp(s1, s2));
}


reg_t kStrCpy(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *dest = (char *) kernel_dereference_bulk_pointer(s, argv[0], 0);
	char *src = (char *) kernel_dereference_bulk_pointer(s, argv[1], 0);

	if (!dest) {
		warning("Attempt to strcpy TO invalid pointer "PREG"",
		          PRINT_REG(argv[0]));
		return NULL_REG;
	}
	if (!src) {
		warning("Attempt to strcpy FROM invalid pointer "PREG"",
		          PRINT_REG(argv[1]));
		return NULL_REG;
	}

	if (argc > 2) {
		int length = SKPV(2);

		if (length >= 0)
			strncpy(dest, src, length);
		else {
			if (s->seg_manager.heap[argv[0].segment]->type == MEM_OBJ_DYNMEM) {
				reg_t *srcp = (reg_t *) src;

				int i;
				warning("Performing reg_t to raw conversion for AvoidPath");
				for (i = 0; i < -length / 2; i++) {
					dest[2 * i] = srcp->offset & 0xff;
					dest[2 * i + 1] = srcp->offset >> 8;
					srcp++;
				}
			} else
				memcpy(dest, src, -length);
		}
	} else
		strcpy(dest, src);

	return argv[0];
}


reg_t kStrAt(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	unsigned char *dest = (unsigned char *) kernel_dereference_bulk_pointer(s, argv[0], 0);
	reg_t *dest2;

	if (!dest) {
		warning("Attempt to StrAt at invalid pointer "PREG"",
		          PRINT_REG(argv[0]));
		return NULL_REG;
	}

	if ((argc == 2) &&
	        /* Our pathfinder already works around the issue we're trying to fix */
	        (strcmp(sm_get_description(&(s->seg_manager), argv[0]),
	                AVOIDPATH_DYNMEM_STRING) != 0)  &&
	        ((strlen((const char*)dest) < 2) || (!is_print_str((char*)dest))))
		/* SQ4 array handling detected */
	{
#ifndef WORDS_BIGENDIAN
		int odd = KP_UINT(argv[1]) & 1;
#else
		int odd = !(KP_UINT(argv[1]) & 1);
#endif
		dest2 = ((reg_t *) dest) + (KP_UINT(argv[1]) / 2);
		dest = ((unsigned char *)(&dest2->offset)) + odd;
	} else dest += KP_UINT(argv[1]);

	s->r_acc = make_reg(0, *dest);

	if (argc > 2)
		*dest = KP_SINT(argv[2]); /* Request to modify this char */

	return s->r_acc;
}


reg_t kReadNumber(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *source = kernel_dereference_char_pointer(s, argv[0], 0);

	while (isspace(*source))
		source++; /* Skip whitespace */

	if (*source == '$') /* SCI uses this for hex numbers */
		return make_reg(0, (gint16)strtol(source + 1, NULL, 16)); /* Hex */
	else
		return make_reg(0, (gint16)strtol(source, NULL, 10)); /* Force decimal */
}


#define ALIGN_NONE 0
#define ALIGN_RIGHT 1
#define ALIGN_LEFT -1
#define ALIGN_CENTRE 2

/*  Format(targ_address, textresnr, index_inside_res, ...)
** or
**  Format(targ_address, heap_text_addr, ...)
** Formats the text from text.textresnr (offset index_inside_res) or heap_text_addr according to
** the supplied parameters and writes it to the targ_address.
*/
reg_t kFormat(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int *arguments;
	reg_t dest = argv[0];
	char *target = (char *) kernel_dereference_bulk_pointer(s, dest, 0);
	reg_t position = argv[1]; /* source */
	int index = UKPV(2);
	char *source;
	char *str_base = target;
	int mode = 0;
	int paramindex = 0; /* Next parameter to evaluate */
	char xfer;
	int i;
	int startarg;
	int str_leng = 0; /* Used for stuff like "%13s" */
	int unsigned_var = 0;
	int maxsize = 4096; /* Arbitrary... */


	if (position.segment)
		startarg = 2;
	else
		startarg = 3; /* First parameter to use for formatting */

	source = kernel_lookup_text(s, position, index);

	SCIkdebug(SCIkSTRINGS, "Formatting \"%s\"\n", source);


	arguments = (int*)sci_malloc(sizeof(int) * argc);
#ifdef SATISFY_PURIFY
	memset(arguments, 0, sizeof(int) * argc);
#endif

	for (i = startarg; i < argc; i++)
		arguments[i-startarg] = UKPV(i); /* Parameters are copied to prevent overwriting */

	while ((xfer = *source++)) {
		if (xfer == '%') {
			if (mode == 1) {
				CHECK_OVERFLOW1(target, 2, NULL_REG);
				*target++ = '%'; /* Literal % by using "%%" */
				mode = 0;
			} else {
				mode = 1;
				str_leng = 0;
			}
		} else if (mode == 1) { /* xfer != '%' */
			char fillchar = ' ';
			int align = ALIGN_NONE;

			char *writestart = target; /* Start of the written string, used after the switch */

			/* int writelength; -- unused atm */

			if (xfer && (isdigit(xfer) || xfer == '-' || xfer == '=')) {
				char *destp;

				if (xfer == '0')
					fillchar = '0';
				else

					if (xfer == '=') {
						align = ALIGN_CENTRE;
						source++;
					} else

						if (isdigit(xfer))
							source--; /* Stepped over length argument */

				str_leng = strtol(source, &destp, 10);

				if (destp > source)
					source = destp;

				if (str_leng < 0) {
					align = ALIGN_LEFT;
					str_leng = -str_leng;
				} else if (align != ALIGN_CENTRE)
					align = ALIGN_RIGHT;

				xfer = *source++;
			} else
				str_leng = 0;

			CHECK_OVERFLOW1(target, str_leng + 1, NULL_REG);

			switch (xfer) {
			case 's': { /* Copy string */
				reg_t reg = argv[startarg + paramindex];
				char *tempsource = kernel_lookup_text(s, reg,
				                                      arguments[paramindex + 1]);
				int slen = strlen(tempsource);
				int extralen = str_leng - slen;
				CHECK_OVERFLOW1(target, extralen, NULL_REG);
				if (extralen < 0)
					extralen = 0;

				if (reg.segment) /* Heap address? */
					paramindex++;
				else
					paramindex += 2; /* No, text resource address */

				switch (align) {

				case ALIGN_NONE:
				case ALIGN_RIGHT:
					while (extralen-- > 0)
						*target++ = ' '; /* Format into the text */
					break;

				case ALIGN_CENTRE: {
					int half_extralen = extralen >> 1;
					while (half_extralen-- > 0)
						*target++ = ' '; /* Format into the text */
					break;
				}

				default:
					break;

				}

				strcpy(target, tempsource);
				target += slen;

				switch (align) {

				case ALIGN_CENTRE: {
					int half_extralen;
					align = 0;
					half_extralen = extralen - (extralen >> 1);
					while (half_extralen-- > 0)
						*target++ = ' '; /* Format into the text */
					break;
				}

				default:
					break;

				}

				mode = 0;
			}
			break;

			case 'c': { /* insert character */
				CHECK_OVERFLOW1(target, 2, NULL_REG);
				if (align >= 0)
					while (str_leng-- > 1)
						*target++ = ' '; /* Format into the text */

				*target++ = arguments[paramindex++];
				mode = 0;
			}
			break;

			case 'x':
			case 'u':
				unsigned_var = 1;
			case 'd': { /* Copy decimal */
				/* int templen; -- unused atm */
				const char *format_string = "%d";

				if (xfer == 'x')
					format_string = "%x";

				if (!unsigned_var)
					if (arguments[paramindex] & 0x8000)
						/* sign extend */
						arguments[paramindex] = (~0xffff) | arguments[paramindex];

				target += sprintf(target, format_string, arguments[paramindex++]);
				CHECK_OVERFLOW1(target, 0, NULL_REG);

				unsigned_var = 0;

				mode = 0;
			}
			break;
			default:
				*target = '%';
				target++;
				*target = xfer;
				target++;
				mode = 0;
			}

			if (align) {
				int written = target - writestart;
				int padding = str_leng - written;

				if (padding > 0) {
					if (align > 0) {
						memmove(writestart + padding,
						        writestart, written);
						memset(writestart, fillchar, padding);
					} else {
						memset(target, ' ', padding);
					}
					target += padding;
				}
			}
		} else { /* mode != 1 */
			*target = xfer;
			target++;
		}
	}

	free(arguments);

	*target = 0; /* Terminate string */
	return dest; /* Return target addr */
}


reg_t kStrLen(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *str = kernel_dereference_char_pointer(s, argv[0], 0);

	return make_reg(0, strlen(str));
}


reg_t kGetFarText(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	resource_t *textres = scir_find_resource(s->resmgr, sci_text, UKPV(0), 0);
	char *seeker;
	int counter = UKPV(1);


	if (!textres) {
		error("text.%d does not exist\n", UKPV(0));
		return NULL_REG;
	}

	seeker = (char *) textres->data;

	while (counter--)
		while (*seeker++);
	/* The second parameter (counter) determines the number of the string inside the text
	** resource.
	*/

	strcpy(kernel_dereference_char_pointer(s, argv[2], 0), seeker); /* Copy the string and get return value */
	return argv[2];
}

#define DUMMY_MESSAGE "No MESSAGE support in FreeSCI yet"

static message_state_t state;

reg_t kMessage(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	if (!state.initialized)
		message_state_initialize(s->resmgr, &state);

	switch (UKPV(0)) {
	case 0 : {
		char *buffer = argc == 7 ? kernel_dereference_char_pointer(s, argv[6], 0) : NULL;
		message_tuple_t tuple;
		int module = UKPV(1);

		tuple.noun = UKPV(2);
		tuple.verb = UKPV(3);
		tuple.cond = UKPV(4);
		tuple.seq = UKPV(5);

		if (message_state_load_res(&state, module) && message_get_specific(&state, &tuple)) {
			if (buffer)
				message_get_text(&state, buffer, 100);
			return make_reg(0, message_get_talker(&state)); /* Talker id */
		} else {
			if (buffer) strcpy(buffer, DUMMY_MESSAGE);
			return NULL_REG;
		}
	}
	case 1 : {
		char *buffer = argc == 7 ? kernel_dereference_char_pointer(s, argv[6], 0) : NULL;

		if (message_get_next(&state)) {
			if (buffer)
				message_get_text(&state, buffer, 100);
			return make_reg(0, message_get_talker(&state)); /* Talker id */
		} else {
			if (buffer) strcpy(buffer, DUMMY_MESSAGE);
			return NULL_REG;
		}
	}
	case 2 : {
		message_tuple_t tuple;
		int module = UKPV(1);
		tuple.noun = UKPV(2);
		tuple.verb = UKPV(3);
		tuple.cond = UKPV(4);
		tuple.seq = UKPV(5);

		if (message_state_load_res(&state, module) && message_get_specific(&state, &tuple))
			return make_reg(0, message_get_length(&state) + 1);
		else return NULL_REG;
	}
	}

	return NULL_REG;
}

} // End of namespace Sci
