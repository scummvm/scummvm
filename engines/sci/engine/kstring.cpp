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
#include "sci/engine/selector.h"
#include "sci/engine/kernel.h"

namespace Sci {

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
	if (argv[0] == SIGNAL_REG) {
		warning("Attempt to perform kStrAt() on a signal reg");
		return NULL_REG;
	}

	SegmentRef dest_r = s->_segMan->dereference(argv[0]);
	if (!dest_r.isValid()) {
		warning("Attempt to StrAt at invalid pointer %04x:%04x", PRINT_REG(argv[0]));
		return NULL_REG;
	}

	byte value;
	byte newvalue = 0;
	unsigned int offset = argv[1].toUint16();
	if (argc > 2)
		newvalue = argv[2].toSint16();

	// in kq5 this here gets called with offset 0xFFFF
	//  (in the desert wheng getting the staff)
	if ((int)offset >= dest_r.maxSize) {
		warning("kStrAt offset %X exceeds maxSize", offset);
		return s->r_acc;
	}

	// FIXME: Move this to segman
	if (dest_r.isRaw) {
		value = dest_r.raw[offset];
		if (argc > 2) /* Request to modify this char */
			dest_r.raw[offset] = newvalue;
	} else {
		if (dest_r.skipByte)
			offset++;

		reg_t &tmp = dest_r.reg[offset / 2];
		if (!(offset & 1)) {
			value = tmp.offset & 0x00ff;
			if (argc > 2) { /* Request to modify this char */
				tmp.offset &= 0xff00;
				tmp.offset |= newvalue;
				tmp.segment = 0;
			}
		} else {
			value = tmp.offset >> 8;
			if (argc > 2)  { /* Request to modify this char */
				tmp.offset &= 0x00ff;
				tmp.offset |= newvalue << 8;
				tmp.segment = 0;
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

	int16 result = 0;

	if (*source == '$') {
		// Hexadecimal input
		result = (int16)strtol(source + 1, NULL, 16);
	} else {
		// Decimal input. We can not use strtol/atoi in here, because while
		// Sierra used atoi, it was a non standard compliant atoi, that didn't
		// do clipping. In SQ4 we get the door code in here and that's even
		// larger than uint32!
		if (*source == '-') {
			result = -1;
			source++;
		}
		while (*source) {
			if ((*source < '0') || (*source > '9')) {
				// Sierra's atoi stopped processing at anything which is not
				// a digit. Sometimes the input has a trailing space, that's
				// fine (example: lsl3)
				if (*source != ' ') {
					// TODO: this happens in lsl5 right in the intro -> we get '1' '3' 0xCD 0xCD 0xCD 0xCD 0xCD
					//       find out why this happens and fix it
					warning("Invalid character in kReadNumber input");
				}
				break;
			}
			result *= 10;
			result += *source - 0x30;
			source++;
		}
	}

	return make_reg(0, result);
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

	Common::String source_str = g_sci->getKernel()->lookupText(position, index);
	const char* source = source_str.c_str();

	debugC(2, kDebugLevelStrings, "Formatting \"%s\"", source);


	arguments = (uint16 *)malloc(sizeof(uint16) * argc);
	memset(arguments, 0, sizeof(uint16) * argc);

	for (i = startarg; i < argc; i++)
		arguments[i-startarg] = argv[i].toUint16(); /* Parameters are copied to prevent overwriting */

	while ((xfer = *source++)) {
		if (xfer == '%') {
			if (mode == 1) {
				assert((target - targetbuf) + 2 <= maxsize);
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
				else if (xfer == '=')
					align = ALIGN_CENTRE;
				else if (isdigit(xfer) || (xfer == '-'))
					source--; // Go to start of length argument

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

			assert((target - targetbuf) + str_leng + 1 <= maxsize);

			switch (xfer) {
			case 's': { /* Copy string */
				reg_t reg = argv[startarg + paramindex];

#ifdef ENABLE_SCI32
				// If the string is a string object, get to the actual string in the data selector
				if (s->_segMan->isObject(reg))
					reg = readSelector(s->_segMan, reg, SELECTOR(data));
#endif

				Common::String tempsource = (reg == NULL_REG) ? "" : g_sci->getKernel()->lookupText(reg,
				                                  arguments[paramindex + 1]);
				int slen = strlen(tempsource.c_str());
				int extralen = str_leng - slen;
				assert((target - targetbuf) + extralen <= maxsize);
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
				assert((target - targetbuf) + 2 <= maxsize);
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
				assert((target - targetbuf) <= maxsize);

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

#ifdef ENABLE_SCI32
	// Resize SCI32 strings if necessary
	if (getSciVersion() >= SCI_VERSION_2) {
		SciString *string = s->_segMan->lookupString(dest);
		string->setSize(strlen(targetbuf) + 1);
	}
#endif

	s->_segMan->strcpy(dest, targetbuf);

	return dest; /* Return target addr */
}

reg_t kStrLen(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, s->_segMan->strlen(argv[0]));
}


reg_t kGetFarText(EngineState *s, int argc, reg_t *argv) {
	Resource *textres = g_sci->getResMan()->findResource(ResourceId(kResourceTypeText, argv[0].toUint16()), 0);
	char *seeker;
	int counter = argv[1].toUint16();

	if (!textres) {
		error("text.%d does not exist", argv[0].toUint16());
		return NULL_REG;
	}

	seeker = (char *)textres->data;
	
	// The second parameter (counter) determines the number of the string
	// inside the text resource.
	while (counter--) {
		while (*seeker++)
			;
	}

	// If the third argument is NULL, allocate memory for the destination. This
	// occurs in SCI1 Mac games. The memory will later be freed by the game's
	// scripts.
	if (argv[2] == NULL_REG)
		s->_segMan->allocDynmem(strlen(seeker) + 1, "Mac FarText", &argv[2]);

	s->_segMan->strcpy(argv[2], seeker); // Copy the string and get return value
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

#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		// In complete weirdness, SCI32 bumps up subops 3-8 to 4-9 and stubs off subop 3.
		// In addition, SCI32 reorders the REF* subops.
		if (func == 3)
			error("SCI32 kMessage(3)");
		else if (func > 3) {
			func--;
			if (func == K_MESSAGE_REFCOND)
				func = K_MESSAGE_REFNOUN;
			else if (func == K_MESSAGE_REFNOUN || func == K_MESSAGE_REFVERB)
				func--;
		}
	}
#endif

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

		bool ok = false;

		if (s->_segMan->dereference(argv[1]).isRaw) {
			byte *buffer = s->_segMan->derefBulkPtr(argv[1], 10);

			if (buffer) {
				ok = true;
				WRITE_LE_UINT16(buffer, module);
				WRITE_LE_UINT16(buffer + 2, msg.noun);
				WRITE_LE_UINT16(buffer + 4, msg.verb);
				WRITE_LE_UINT16(buffer + 6, msg.cond);
				WRITE_LE_UINT16(buffer + 8, msg.seq);
			}
		} else {
			reg_t *buffer = s->_segMan->derefRegPtr(argv[1], 5);

			if (buffer) {
				ok = true;
				buffer[0] = make_reg(0, module);
				buffer[1] = make_reg(0, msg.noun);
				buffer[2] = make_reg(0, msg.verb);
				buffer[3] = make_reg(0, msg.cond);
				buffer[4] = make_reg(0, msg.seq);
			}
		}

		if (!ok)
			warning("Message: buffer %04x:%04x invalid or too small to hold the tuple", PRINT_REG(argv[1]));

		return NULL_REG;
	}
	default:
		warning("Message: subfunction %i invoked (not implemented)", func);
	}

	return NULL_REG;
}

reg_t kSetQuitStr(EngineState *s, int argc, reg_t *argv) {
	Common::String quitStr = s->_segMan->getString(argv[0]);
	//debug("Setting quit string to '%s'", quitStr.c_str());
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
	Common::String str = g_sci->strSplit(format.c_str(), sep);

	// Make sure target buffer is large enough
	SegmentRef buf_r = s->_segMan->dereference(argv[0]);
	if (!buf_r.isValid() || buf_r.maxSize < (int)str.size() + 1) {
		warning("StrSplit: buffer %04x:%04x invalid or too small to hold the following text of %i bytes: '%s'",
						PRINT_REG(argv[0]), str.size() + 1, str.c_str());
		return NULL_REG;
	}
	s->_segMan->strcpy(argv[0], str.c_str());
	return argv[0];
}

} // End of namespace Sci
