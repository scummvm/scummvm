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

#include "sci/resource.h"
#include "sci/engine/state.h"
#include "sci/engine/message.h"
#include "sci/engine/kernel.h"

namespace Sci {

#define CHECK_OVERFLOW1(pt, size, rv) \
	if (((pt) - (str_base)) + (size) > maxsize) { \
		error("String expansion exceeded heap boundaries"); \
		return rv;\
	}

/* Returns the string the script intended to address */
char *kernel_lookup_text(EngineState *s, reg_t address, int index) {
	char *seeker;
	Resource *textres;

	if (address.segment)
		return (char *)kernel_dereference_bulk_pointer(s, address, 0);
	else {
		int textlen;
		int _index = index;
		textres = s->resmgr->findResource(ResourceId(kResourceTypeText, address.offset), 0);

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
			error("Index %d out of bounds in text.%03d", _index, address.offset);
			return 0;
		}

	}
}


/*************************************************************/
/* Parser */
/**********/


reg_t kSaid(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t heap_said_block = argv[0];
	byte *said_block;
	int new_lastmatch;

	if (!heap_said_block.segment)
		return NULL_REG;

	said_block = (byte *) kernel_dereference_bulk_pointer(s, heap_said_block, 0);

	if (!said_block) {
		warning("Said on non-string, pointer %04x:%04x", PRINT_REG(heap_said_block));
		return NULL_REG;
	}

#ifdef DEBUG_PARSER
		debugC(2, kDebugLevelParser, "Said block:", 0);
		((SciEngine*)g_engine)->getVocabulary()->decipherSaidBlock(said_block);
#endif

	if (s->parser_event.isNull() || (GET_SEL32V(s->parser_event, claimed))) {
		return NULL_REG;
	}

	new_lastmatch = said(s, said_block, 
#ifdef DEBUG_PARSER
		1
#else
		0
#endif
		);
	if (new_lastmatch  != SAID_NO_MATCH) { /* Build and possibly display a parse tree */

#ifdef DEBUG_PARSER
		printf("kSaid: Match.\n");
#endif

		s->r_acc = make_reg(0, 1);

		if (new_lastmatch != SAID_PARTIAL_MATCH)
			PUT_SEL32V(s->parser_event, claimed, 1);

	} else {
		return NULL_REG;
	}
	return s->r_acc;
}


reg_t kSetSynonyms(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t object = argv[0];
	List *list;
	Node *node;
	int script;

	s->_synonyms.clear();

	list = lookup_list(s, GET_SEL32(object, elements));
	node = lookup_node(s, list->first);

	while (node) {
		reg_t objpos = node->value;
		int seg;
		int synonyms_nr = 0;

		script = GET_SEL32V(objpos, number);
		seg = s->seg_manager->segGet(script);

		if (seg >= 0)
			synonyms_nr = s->seg_manager->getScript(seg)->getSynonymsNr();

		if (synonyms_nr) {
			byte *synonyms;

			synonyms = s->seg_manager->getScript(seg)->getSynonyms();
			if (synonyms) {
				debugC(2, kDebugLevelParser, "Setting %d synonyms for script.%d\n",
				          synonyms_nr, script);

				if (synonyms_nr > 16384) {
					error("Segtable corruption: script.%03d has %d synonyms",
					         script, synonyms_nr);
					/* We used to reset the corrupted value here. I really don't think it's appropriate.
					 * Lars */
				} else
					for (int i = 0; i < synonyms_nr; i++) {
						synonym_t tmp;
						tmp.replaceant = (int16)READ_LE_UINT16(synonyms + i * 4);
						tmp.replacement = (int16)READ_LE_UINT16(synonyms + i * 4 + 2);
						s->_synonyms.push_back(tmp);
					}
			} else
				warning("Synonyms of script.%03d were requested, but script is not available", script);

		}

		node = lookup_node(s, node->succ);
	}

	debugC(2, kDebugLevelParser, "A total of %d synonyms are active now.\n", s->_synonyms.size());

	return s->r_acc;
}



reg_t kParse(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t stringpos = argv[0];
	char *string = kernel_dereference_char_pointer(s, stringpos, 0);
	char *error;
	ResultWordList words;
	reg_t event = argv[1];
	Vocabulary *voc = ((SciEngine*)g_engine)->getVocabulary();

	s->parser_event = event;

	bool res = voc->tokenizeString(words, string, &error);
	s->parser_valid = 0; /* not valid */

	if (res && !words.empty()) {

		int syntax_fail = 0;

		vocab_synonymize_tokens(words, s->_synonyms);

		s->r_acc = make_reg(0, 1);

#ifdef DEBUG_PARSER
			debugC(2, kDebugLevelParser, "Parsed to the following blocks:\n", 0);

			for (ResultWordList::const_iterator i = words.begin(); i != words.end(); ++i)
				debugC(2, kDebugLevelParser, "   Type[%04x] Group[%04x]\n", i->_class, i->_group);
#endif

		if (voc->parseGNF(s->parser_nodes, words))
			syntax_fail = 1; /* Building a tree failed */

		if (syntax_fail) {

			s->r_acc = make_reg(0, 1);
			PUT_SEL32V(event, claimed, 1);

			invoke_selector(INV_SEL(s->game_obj, syntaxFail, kStopOnInvalidSelector), 2, s->parser_base, stringpos);
			/* Issue warning */

			debugC(2, kDebugLevelParser, "Tree building failed\n");

		} else {
			s->parser_valid = 1;
			PUT_SEL32V(event, claimed, 0);

#ifdef DEBUG_PARSER
			vocab_dump_parse_tree("Parse-tree", s->parser_nodes);
#endif
		}

	} else {

		s->r_acc = make_reg(0, 0);
		PUT_SEL32V(event, claimed, 1);
		if (error) {
			char *pbase_str = kernel_dereference_char_pointer(s, s->parser_base, 0);
			strcpy(pbase_str, error);
			debugC(2, kDebugLevelParser, "Word unknown: %s\n", error);
			/* Issue warning: */

			invoke_selector(INV_SEL(s->game_obj, wordFail, kStopOnInvalidSelector), 2, s->parser_base, stringpos);
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
		return make_reg(0, strncmp(s1, s2, argv[2].toUint16()));
	else
		return make_reg(0, strcmp(s1, s2));
}


reg_t kStrCpy(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *dest = (char *) kernel_dereference_bulk_pointer(s, argv[0], 0);
	char *src = (char *) kernel_dereference_bulk_pointer(s, argv[1], 0);

	if (!dest) {
		warning("Attempt to strcpy TO invalid pointer %04x:%04x",
		          PRINT_REG(argv[0]));
		return NULL_REG;
	}
	if (!src) {
		warning("Attempt to strcpy FROM invalid pointer %04x:%04x",
		          PRINT_REG(argv[1]));
		*dest = 0;
		return argv[1];
	}

	if (argc > 2) {
		int length = argv[2].toSint16();

		if (length >= 0)
			strncpy(dest, src, length);
		else {
			if (s->seg_manager->_heap[argv[0].segment]->getType() == MEM_OBJ_DYNMEM) {
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

/* Simple heuristic to work around array handling peculiarity in SQ4:
It uses StrAt() to read the individual elements, so we must determine
whether a string is really a string or an array. */
static int is_print_str(const char *str) {
	int printable = 0;
	int len = strlen(str);

	if (len == 0) return 1;

	while (*str) {
		// The parameter passed to isprint() needs to be in the range
		// 0 to 0xFF or EOF, according to MSDN, therefore we cast it
		// to an unsigned char. Values outside this range (in this
		// case, negative values) yield unpredictable results. Refer to:
		// http://msdn.microsoft.com/en-us/library/ewx8s4kw.aspx
		if (isprint((byte)*str))
			printable++;
		str++;
	}

	return ((float)printable / (float)len >= 0.5);
}


reg_t kStrAt(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	byte *dest = (byte *)kernel_dereference_bulk_pointer(s, argv[0], 0);
	reg_t *dest2;

	if (!dest) {
		warning("Attempt to StrAt at invalid pointer %04x:%04x", PRINT_REG(argv[0]));
		return NULL_REG;
	}

	bool lsl5PasswordWorkaround = false;
	// LSL5 stores the password at the beginning in memory.drv, using XOR encryption,
	// which means that is_print_str() will fail. Therefore, do not use the heuristic to determine
	// if we're handling a string or an array for LSL5's password screen (room 155)
	if (s->_gameName.equalsIgnoreCase("lsl5") && s->currentRoomNumber() == 155)
		lsl5PasswordWorkaround = true;

	const char* dst = (const char *)dest; // used just for code beautification purposes

	if ((argc == 2) &&
	        /* Our pathfinder already works around the issue we're trying to fix */
	        (strcmp(s->seg_manager->getDescription(argv[0]), AVOIDPATH_DYNMEM_STRING) != 0) &&
	        ((strlen(dst) < 2) || (!lsl5PasswordWorkaround && !is_print_str(dst)))) {
		// SQ4 array handling detected
#ifndef SCUMM_BIG_ENDIAN
		int odd = argv[1].toUint16() & 1;
#else
		int odd = !(argv[1].toUint16() & 1);
#endif
		dest2 = ((reg_t *) dest) + (argv[1].toUint16() / 2);
		dest = ((byte *)(&dest2->offset)) + odd;
	} else
		dest += argv[1].toUint16();

	s->r_acc = make_reg(0, *dest);

	if (argc > 2)
		*dest = argv[2].toSint16(); /* Request to modify this char */

	return s->r_acc;
}


reg_t kReadNumber(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *source = kernel_dereference_char_pointer(s, argv[0], 0);

	while (isspace(*source))
		source++; /* Skip whitespace */

	if (*source == '$') /* SCI uses this for hex numbers */
		return make_reg(0, (int16)strtol(source + 1, NULL, 16)); /* Hex */
	else
		return make_reg(0, (int16)strtol(source, NULL, 10)); /* Force decimal */
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
	int index = argv[2].toUint16();
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

	debugC(2, kDebugLevelStrings, "Formatting \"%s\"\n", source);


	arguments = (int*)malloc(sizeof(int) * argc);
#ifdef SATISFY_PURIFY
	memset(arguments, 0, sizeof(int) * argc);
#endif

	for (i = startarg; i < argc; i++)
		arguments[i-startarg] = argv[i].toUint16(); /* Parameters are copied to prevent overwriting */

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

	if (!str) {
		warning("StrLen: invalid pointer %04x:%04x", PRINT_REG(argv[0]));
		return NULL_REG;
	}

	return make_reg(0, strlen(str));
}


reg_t kGetFarText(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	Resource *textres = s->resmgr->findResource(ResourceId(kResourceTypeText, argv[0].toUint16()), 0);
	char *seeker;
	int counter = argv[1].toUint16();


	if (!textres) {
		error("text.%d does not exist", argv[0].toUint16());
		return NULL_REG;
	}

	seeker = (char *) textres->data;

	while (counter--) {
		while (*seeker++)
			;
	}
	/* The second parameter (counter) determines the number of the string inside the text
	** resource.
	*/

	strcpy(kernel_dereference_char_pointer(s, argv[2], 0), seeker); /* Copy the string and get return value */
	return argv[2];
}

#define DUMMY_MESSAGE "Message not found!"

enum kMessageFunc {
	K_MESSAGE_GET,
	K_MESSAGE_NEXT,
	K_MESSAGE_SIZE,
	K_MESSAGE_REFCOND,
	K_MESSAGE_REFVERB,
	K_MESSAGE_REFNOUN,
	K_MESSAGE_PUSH,
	K_MESSAGE_POP,
	K_MESSAGE_LASTMESSAGE
};

reg_t kMessage(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	MessageTuple tuple;
	int func;
	// For earlier version of of this function (GetMessage)
	bool isGetMessage = argc == 4;

	if (isGetMessage) {
		func = K_MESSAGE_GET;

		tuple.noun = argv[0].toUint16();
		tuple.verb = argv[2].toUint16();
		tuple.cond = 0;
		tuple.seq = 1;
	} else {
		func = argv[0].toUint16();

		if (argc >= 6) {
			tuple.noun = argv[2].toUint16();
			tuple.verb = argv[3].toUint16();
			tuple.cond = argv[4].toUint16();
			tuple.seq = argv[5].toUint16();
		}
	}

	switch (func) {
	case K_MESSAGE_GET:
	case K_MESSAGE_NEXT: {
		reg_t bufferReg;
		char *buffer = NULL;
		Common::String str;
		reg_t retval;

		if (func == K_MESSAGE_GET) {
			s->_msgState.loadRes(s->resmgr, argv[1].toUint16(), true);
			s->_msgState.findTuple(tuple);

			if (isGetMessage)
				bufferReg = (argc == 4 ? argv[3] : NULL_REG);
			else
				bufferReg = (argc == 7 ? argv[6] : NULL_REG);
		} else {
			bufferReg = (argc == 2 ? argv[1] : NULL_REG);
		}

		if (s->_msgState.getMessage()) {
			str = s->_msgState.getText();
			if (isGetMessage)
				retval = bufferReg;
			else
				retval = make_reg(0, s->_msgState.getTalker());
		} else {
			str = Common::String(DUMMY_MESSAGE);
			retval = NULL_REG;
		}

		if (!bufferReg.isNull()) {
			int len = str.size() + 1;
			buffer = kernel_dereference_char_pointer(s, bufferReg, len);

			if (buffer) {
				strcpy(buffer, str.c_str());
			} else {
				warning("Message: buffer %04x:%04x invalid or too small to hold the following text of %i bytes: '%s'", PRINT_REG(bufferReg), len, str.c_str());

				// Set buffer to empty string if possible
				buffer = kernel_dereference_char_pointer(s, bufferReg, 1);
				if (buffer)
					*buffer = 0;
			}

			s->_msgState.gotoNext();
		}

		return retval;
	}
	case K_MESSAGE_SIZE: {
		MessageState tempState;

		if (tempState.loadRes(s->resmgr, argv[1].toUint16(), false) && tempState.findTuple(tuple) && tempState.getMessage())
			return make_reg(0, tempState.getText().size() + 1);
		else
			return NULL_REG;
	}
	case K_MESSAGE_REFCOND:
	case K_MESSAGE_REFVERB:
	case K_MESSAGE_REFNOUN: {
		MessageState tempState;

		if (tempState.loadRes(s->resmgr, argv[1].toUint16(), false) && tempState.findTuple(tuple)) {
			MessageTuple t = tempState.getRefTuple();
			switch (func) {
			case K_MESSAGE_REFCOND:
				return make_reg(0, t.cond);
			case K_MESSAGE_REFVERB:
				return make_reg(0, t.verb);
			case K_MESSAGE_REFNOUN:
				return make_reg(0, t.noun);
			}
		}

		return NULL_REG;
	}
	case K_MESSAGE_LASTMESSAGE: {
		MessageTuple msg = s->_msgState.getLastTuple();
		int module = s->_msgState.getLastModule();
		byte *buffer = kernel_dereference_bulk_pointer(s, argv[1], 10);

		if (buffer) {
			WRITE_LE_UINT16(buffer, module);
			WRITE_LE_UINT16(buffer + 2, msg.noun);
			WRITE_LE_UINT16(buffer + 4, msg.verb);
			WRITE_LE_UINT16(buffer + 6, msg.cond);
			WRITE_LE_UINT16(buffer + 8, msg.seq);
		} else {
			warning("Message: buffer %04x:%04x invalid or too small to hold the tuple", PRINT_REG(argv[1]));
		}

		return NULL_REG;
	}
	default:
		warning("Message: subfunction %i invoked (not implemented)", func);
	}

	return NULL_REG;
}

reg_t kSetQuitStr(EngineState *s, int funct_nr, int argc, reg_t *argv) {
        char *quitStr = kernel_dereference_char_pointer(s, argv[0], 0);
        debug("Setting quit string to '%s'", quitStr);
        return s->r_acc;
}

reg_t kStrSplit(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	const char *format = kernel_dereference_char_pointer(s, argv[1], 0);
	const char *sep = !argv[2].isNull() ? kernel_dereference_char_pointer(s, argv[2], 0) : NULL;
	Common::String str = s->strSplit(format, sep);

	// Make sure target buffer is large enough
	char *buf = kernel_dereference_char_pointer(s, argv[0], str.size() + 1);

	if (buf) {
		strcpy(buf, str.c_str());
		return argv[0];
	} else {
		warning("StrSplit: buffer %04x:%04x invalid or too small to hold the following text of %i bytes: '%s'", PRINT_REG(argv[0]), str.size() + 1, str.c_str());
		return NULL_REG;
	}
}

} // End of namespace Sci
