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
Common::String kernel_lookup_text(EngineState *s, reg_t address, int index) {
	char *seeker;
	Resource *textres;

	if (address.segment)
		return s->_segMan->getString(address);
	else {
		int textlen;
		int _index = index;
		textres = s->resMan->findResource(ResourceId(kResourceTypeText, address.offset), 0);

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


reg_t kSaid(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	reg_t heap_said_block = argv[0];
	byte *said_block;
	int new_lastmatch;
#ifdef DEBUG_PARSER
	const int debug_parser = 1;
#else
	const int debug_parser = 0;
#endif

	if (!heap_said_block.segment)
		return NULL_REG;

	said_block = (byte *)s->_segMan->derefBulkPtr(heap_said_block, 0);

	if (!said_block) {
		warning("Said on non-string, pointer %04x:%04x", PRINT_REG(heap_said_block));
		return NULL_REG;
	}

#ifdef DEBUG_PARSER
		debugC(2, kDebugLevelParser, "Said block:", 0);
		s->_voc->decipherSaidBlock(said_block);
#endif

	if (s->parser_event.isNull() || (GET_SEL32V(s->parser_event, claimed))) {
		return NULL_REG;
	}

	new_lastmatch = said(s, said_block, debug_parser);
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


reg_t kSetSynonyms(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	reg_t object = argv[0];
	List *list;
	Node *node;
	int script;
	int numSynonyms = 0;

	s->_voc->clearSynonyms();

	list = s->_segMan->lookupList(GET_SEL32(object, elements));
	node = s->_segMan->lookupNode(list->first);

	while (node) {
		reg_t objpos = node->value;
		int seg;

		script = GET_SEL32V(objpos, number);
		seg = s->_segMan->getScriptSegment(script);

		if (seg > 0)
			numSynonyms = s->_segMan->getScript(seg)->getSynonymsNr();

		if (numSynonyms) {
			byte *synonyms = s->_segMan->getScript(seg)->getSynonyms();

			if (synonyms) {
				debugC(2, kDebugLevelParser, "Setting %d synonyms for script.%d\n",
				          numSynonyms, script);

				if (numSynonyms > 16384) {
					error("Segtable corruption: script.%03d has %d synonyms",
					         script, numSynonyms);
					/* We used to reset the corrupted value here. I really don't think it's appropriate.
					 * Lars */
				} else
					for (int i = 0; i < numSynonyms; i++) {
						synonym_t tmp;
						tmp.replaceant = (int16)READ_LE_UINT16(synonyms + i * 4);
						tmp.replacement = (int16)READ_LE_UINT16(synonyms + i * 4 + 2);
						s->_voc->addSynonym(tmp);
					}
			} else
				warning("Synonyms of script.%03d were requested, but script is not available", script);

		}

		node = s->_segMan->lookupNode(node->succ);
	}

	debugC(2, kDebugLevelParser, "A total of %d synonyms are active now.\n", numSynonyms);

	return s->r_acc;
}



reg_t kParse(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	reg_t stringpos = argv[0];
	Common::String string = s->_segMan->getString(stringpos);
	char *error;
	ResultWordList words;
	reg_t event = argv[1];
	Vocabulary *voc = s->_voc;

	s->parser_event = event;

	bool res = voc->tokenizeString(words, string.c_str(), &error);
	s->parserIsValid = false; /* not valid */

	if (res && !words.empty()) {
		s->_voc->synonymizeTokens(words);

		s->r_acc = make_reg(0, 1);

#ifdef DEBUG_PARSER
			debugC(2, kDebugLevelParser, "Parsed to the following blocks:\n", 0);

			for (ResultWordList::const_iterator i = words.begin(); i != words.end(); ++i)
				debugC(2, kDebugLevelParser, "   Type[%04x] Group[%04x]\n", i->_class, i->_group);
#endif

		int syntax_fail = voc->parseGNF(words);

		if (syntax_fail) {
			s->r_acc = make_reg(0, 1);
			PUT_SEL32V(event, claimed, 1);

			invoke_selector(INV_SEL(s->game_obj, syntaxFail, kStopOnInvalidSelector), 2, s->parser_base, stringpos);
			/* Issue warning */

			debugC(2, kDebugLevelParser, "Tree building failed\n");

		} else {
			s->parserIsValid = true;
			PUT_SEL32V(event, claimed, 0);

#ifdef DEBUG_PARSER
			s->_voc->dumpParseTree();
#endif
		}

	} else {

		s->r_acc = make_reg(0, 0);
		PUT_SEL32V(event, claimed, 1);
		if (error) {
			s->_segMan->strcpy(s->parser_base, error);
			debugC(2, kDebugLevelParser, "Word unknown: %s\n", error);
			/* Issue warning: */

			invoke_selector(INV_SEL(s->game_obj, wordFail, kStopOnInvalidSelector), 2, s->parser_base, stringpos);
			free(error);
			return make_reg(0, 1); /* Tell them that it dind't work */
		}
	}

	return s->r_acc;
}


reg_t kStrEnd(EngineState *s, int argc, reg_t *argv) {
	reg_t address = argv[0];
	address.offset += s->_segMan->strlen(address);

	return address;
}

reg_t kStrCat(EngineState *s, int argc, reg_t *argv) {
	Common::String s1 = s->_segMan->getString(argv[0]);
	Common::String s2 = s->_segMan->getString(argv[1]);

	s1 += s2;
	s->_segMan->strcpy(argv[0], s1.c_str());
	return argv[0];
}

reg_t kStrCmp(EngineState *s, int argc, reg_t *argv) {
	Common::String s1 = s->_segMan->getString(argv[0]);
	Common::String s2 = s->_segMan->getString(argv[1]);

	if (argc > 2)
		return make_reg(0, strncmp(s1.c_str(), s2.c_str(), argv[2].toUint16()));
	else
		return make_reg(0, strcmp(s1.c_str(), s2.c_str()));
}


reg_t kStrCpy(EngineState *s, int argc, reg_t *argv) {
	if (argc > 2) {
		int length = argv[2].toSint16();

		if (length >= 0)
			s->_segMan->strncpy(argv[0], argv[1], length);
		else
			s->_segMan->memcpy(argv[0], argv[1], -length);
	} else
		s->_segMan->strcpy(argv[0], argv[1]);

	return argv[0];
}


reg_t kStrAt(EngineState *s, int argc, reg_t *argv) {
	SegmentRef dest_r = s->_segMan->dereference(argv[0]);
	if (!dest_r.raw) {
		warning("Attempt to StrAt at invalid pointer %04x:%04x", PRINT_REG(argv[0]));
		return NULL_REG;
	}

	byte value;
	byte newvalue = 0;
	unsigned int offset = argv[1].toUint16();
	if (argc > 2)
		newvalue = argv[2].toSint16();

	if (dest_r.isRaw) {
		value = dest_r.raw[offset];
		if (argc > 2) /* Request to modify this char */
			dest_r.raw[offset] = newvalue;
	} else {
		reg_t &tmp = dest_r.reg[offset / 2];
		if (!(offset & 1)) {
			value = tmp.offset & 0x00ff;
			if (argc > 2) { /* Request to modify this char */
				tmp.offset &= 0xff00;
				tmp.offset |= newvalue;
			}
		} else {
			value = tmp.offset >> 8;
			if (argc > 2)  { /* Request to modify this char */
				tmp.offset &= 0x00ff;
				tmp.offset |= newvalue << 8;
			}
		}
	}

	s->r_acc = make_reg(0, value);

	return s->r_acc;
}


reg_t kReadNumber(EngineState *s, int argc, reg_t *argv) {
	Common::String source_str = s->_segMan->getString(argv[0]);
	const char *source = source_str.c_str();

	while (isspace((unsigned char)*source))
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
reg_t kFormat(EngineState *s, int argc, reg_t *argv) {
	uint16 *arguments;
	reg_t dest = argv[0];
	int maxsize = 4096; /* Arbitrary... */
	char targetbuf[4096];
	char *target = targetbuf;
	reg_t position = argv[1]; /* source */
	int index = argv[2].toUint16();
	char *str_base = target;
	int mode = 0;
	int paramindex = 0; /* Next parameter to evaluate */
	char xfer;
	int i;
	int startarg;
	int str_leng = 0; /* Used for stuff like "%13s" */
	int unsigned_var = 0;

	if (position.segment)
		startarg = 2;
	else
		startarg = 3; /* First parameter to use for formatting */

	Common::String source_str = kernel_lookup_text(s, position, index);
	const char* source = source_str.c_str();

	debugC(2, kDebugLevelStrings, "Formatting \"%s\"\n", source);


	arguments = (uint16 *)malloc(sizeof(uint16) * argc);
#ifdef SATISFY_PURIFY
	memset(arguments, 0, sizeof(uint16) * argc);
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
				Common::String tempsource = (reg == NULL_REG) ? "" : kernel_lookup_text(s, reg,
				                                  arguments[paramindex + 1]);
				int slen = strlen(tempsource.c_str());
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

				strcpy(target, tempsource.c_str());
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

				int val = arguments[paramindex];
				if (!unsigned_var)
					val = (int16)arguments[paramindex];

				target += sprintf(target, format_string, val);
				paramindex++;
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

	s->_segMan->strcpy(dest, targetbuf);	

	return dest; /* Return target addr */
}


reg_t kStrLen(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, s->_segMan->strlen(argv[0]));
}


reg_t kGetFarText(EngineState *s, int argc, reg_t *argv) {
	Resource *textres = s->resMan->findResource(ResourceId(kResourceTypeText, argv[0].toUint16()), 0);
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

	s->_segMan->strcpy(argv[2], seeker); /* Copy the string and get return value */
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

reg_t kGetMessage(EngineState *s, int argc, reg_t *argv) {
	MessageTuple tuple = MessageTuple(argv[0].toUint16(), argv[2].toUint16());

	s->_msgState->getMessage(argv[1].toUint16(), tuple, argv[3]);

	return argv[3];
}

reg_t kMessage(EngineState *s, int argc, reg_t *argv) {
	uint func = argv[0].toUint16();

	if ((func != K_MESSAGE_NEXT) && (argc < 2)) {
		warning("Message: not enough arguments passed to subfunction %d", func);
		return NULL_REG;
	}

	MessageTuple tuple;

	if (argc >= 6)
		tuple = MessageTuple(argv[2].toUint16(), argv[3].toUint16(), argv[4].toUint16(), argv[5].toUint16());

	switch (func) {
	case K_MESSAGE_GET:
		return make_reg(0, s->_msgState->getMessage(argv[1].toUint16(), tuple, (argc == 7 ? argv[6] : NULL_REG)));
	case K_MESSAGE_NEXT:
		return make_reg(0, s->_msgState->nextMessage((argc == 2 ? argv[1] : NULL_REG)));
	case K_MESSAGE_SIZE:
		return make_reg(0, s->_msgState->messageSize(argv[1].toUint16(), tuple));
	case K_MESSAGE_REFCOND:
	case K_MESSAGE_REFVERB:
	case K_MESSAGE_REFNOUN: {
		MessageTuple t;

		if (s->_msgState->messageRef(argv[1].toUint16(), tuple, t)) {
			switch (func) {
			case K_MESSAGE_REFCOND:
				return make_reg(0, t.cond);
			case K_MESSAGE_REFVERB:
				return make_reg(0, t.verb);
			case K_MESSAGE_REFNOUN:
				return make_reg(0, t.noun);
			}
		}

		return SIGNAL_REG;
	}
	case K_MESSAGE_LASTMESSAGE: {
		MessageTuple msg;
		int module;

		s->_msgState->lastQuery(module, msg);

		byte *buffer = s->_segMan->derefBulkPtr(argv[1], 10);

		if (buffer) {
			assert(s->_segMan->dereference(argv[1]).isRaw);

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

reg_t kSetQuitStr(EngineState *s, int argc, reg_t *argv) {
	Common::String quitStr = s->_segMan->getString(argv[0]);
	debug("Setting quit string to '%s'", quitStr.c_str());
	return s->r_acc;
}

reg_t kStrSplit(EngineState *s, int argc, reg_t *argv) {
	Common::String format = s->_segMan->getString(argv[1]);
	Common::String sep_str;
	const char *sep = NULL;
	if (!argv[2].isNull()) {
		sep_str = s->_segMan->getString(argv[2]);
		sep = sep_str.c_str();
	}
	Common::String str = s->strSplit(format.c_str(), sep);

	// Make sure target buffer is large enough
	SegmentRef buf_r = s->_segMan->dereference(argv[0]);
	if (!buf_r.isValid() || buf_r.maxSize < (int)str.size() + 1) {
		warning("StrSplit: buffer %04x:%04x invalid or too small to hold the following text of %i bytes: '%s'", PRINT_REG(argv[0]), str.size() + 1, str.c_str());
		return NULL_REG;
	}
	s->_segMan->strcpy(argv[0], str.c_str());
	return argv[0];
}

} // End of namespace Sci
