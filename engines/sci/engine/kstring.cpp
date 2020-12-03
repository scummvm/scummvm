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

/* String and parser handling */

#include "sci/resource/resource.h"
#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/message.h"
#include "sci/engine/selector.h"
#include "sci/engine/kernel.h"

namespace Sci {

reg_t kStrEnd(EngineState *s, int argc, reg_t *argv) {
	reg_t address = argv[0];
	address.incOffset(s->_segMan->strlen(address));

	return address;
}

reg_t kStrCat(EngineState *s, int argc, reg_t *argv) {
	Common::String s1 = s->_segMan->getString(argv[0]);
	Common::String s2 = s->_segMan->getString(argv[1]);

	// Japanese PC-9801 interpreter splits strings here
	//  see bug #5834
	//  Verified for Police Quest 2 + Quest For Glory 1
	//  However Space Quest 4 PC-9801 doesn't
	if ((g_sci->getLanguage() == Common::JA_JPN)
		&& (getSciVersion() <= SCI_VERSION_01)) {
		s1 = g_sci->strSplit(s1.c_str(), NULL);
		s2 = g_sci->strSplit(s2.c_str(), NULL);
	}

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
	} else {
		s->_segMan->strcpy(argv[0], argv[1]);
	}

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
	uint16 offset = argv[1].toUint16();
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

		bool oddOffset = offset & 1;
		if (g_sci->isBE())
			oddOffset = !oddOffset;

		if (!oddOffset) {
			value = tmp.getOffset() & 0x00ff;
			if (argc > 2) { /* Request to modify this char */
				uint16 tmpOffset = tmp.toUint16();
				tmpOffset &= 0xff00;
				tmpOffset |= newvalue;
				tmp.setOffset(tmpOffset);
				tmp.setSegment(0);
			}
		} else {
			value = tmp.getOffset() >> 8;
			if (argc > 2)  { /* Request to modify this char */
				uint16 tmpOffset = tmp.toUint16();
				tmpOffset &= 0x00ff;
				tmpOffset |= newvalue << 8;
				tmp.setOffset(tmpOffset);
				tmp.setSegment(0);
			}
		}
	}

	return make_reg(0, value);
}


reg_t kReadNumber(EngineState *s, int argc, reg_t *argv) {
	Common::String source_str = s->_segMan->getString(argv[0]);
	const char *source = source_str.c_str();

	while (Common::isSpace(*source))
		source++; /* Skip whitespace */

	int16 result = 0;
	int16 sign = 1;

	if (*source == '-') {
		sign = -1;
		source++;
	}
	if (*source == '$') {
		// Hexadecimal input
		source++;
		char c;
		while ((c = *source++) != 0) {
			int16 x = 0;
			if ((c >= '0') && (c <= '9'))
				x = c - '0';
			else if ((c >= 'a') && (c <= 'f'))
				x = c - 'a' + 10;
			else if ((c >= 'A') && (c <= 'F'))
				x = c - 'A' + 10;
			else
				// Stop if we encounter anything other than a digit (like atoi)
				break;
			result *= 16;
			result += x;
		}
	} else {
		// Decimal input. We can not use strtol/atoi in here, because while
		// Sierra used atoi, it was a non standard compliant atoi, that didn't
		// do clipping. In SQ4 we get the door code in here and that's even
		// larger than uint32!
		char c;
		while ((c = *source++) != 0) {
			if ((c < '0') || (c > '9'))
				// Stop if we encounter anything other than a digit (like atoi)
				break;
			result *= 10;
			result += c - '0';
		}
	}

	result *= sign;

	return make_reg(0, result);
}


/*  Format(targ_address, textresnr, index_inside_res, ...)
** or
**  Format(targ_address, heap_text_addr, ...)
** Formats the text from text.textresnr (offset index_inside_res) or heap_text_addr according to
** the supplied parameters and writes it to the targ_address.
*/
reg_t kFormat(EngineState *s, int argc, reg_t *argv) {
	enum {
		ALIGN_NONE   = 0,
		ALIGN_RIGHT  = 1,
		ALIGN_LEFT   = -1,
		ALIGN_CENTER = 2
	};

	uint16 *arguments;
	reg_t dest = argv[0];
	int maxsize = 4096; /* Arbitrary... */
	char targetbuf[4096];
	char *target = targetbuf;
	reg_t position = argv[1]; /* source */
	int mode = 0;
	int paramindex = 0; /* Next parameter to evaluate */
	char xfer;
	int i;
	int startarg;
	int strLength = 0; /* Used for stuff like "%13s" */
	bool unsignedVar = false;

	if (position.getSegment())
		startarg = 2;
	else {
		// WORKAROUND: QFG1 VGA Mac calls this without the first parameter (dest). It then
		// treats the source as the dest and overwrites the source string with an empty string.
		if (argc < 3)
			return NULL_REG;

		startarg = 3; /* First parameter to use for formatting */
	}

	int index = (startarg == 3) ? argv[2].toUint16() : 0;
	Common::String source_str = g_sci->getKernel()->lookupText(position, index);
	const char* source = source_str.c_str();

	debugC(kDebugLevelStrings, "Formatting \"%s\"", source);


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
				strLength = 0;
			}
		} else if (mode == 1) { /* xfer != '%' */
			char fillchar = ' ';
			int align = ALIGN_NONE;

			char *writestart = target; /* Start of the written string, used after the switch */

			/* int writelength; -- unused atm */

			if (xfer && (Common::isDigit(xfer) || xfer == '-' || xfer == '=')) {
				char *destp;

				if (xfer == '0')
					fillchar = '0';
				else if (xfer == '=')
					align = ALIGN_CENTER;
				else if (Common::isDigit(xfer) || (xfer == '-'))
					source--; // Go to start of length argument

				strLength = strtol(source, &destp, 10);

				if (destp > source)
					source = destp;

				if (strLength < 0) {
					align = ALIGN_LEFT;
					strLength = -strLength;
				} else if (align != ALIGN_CENTER)
					align = ALIGN_RIGHT;

				xfer = *source++;
			} else
				strLength = 0;

			assert((target - targetbuf) + strLength + 1 <= maxsize);

			switch (xfer) {
			case 's': { /* Copy string */
				reg_t reg = argv[startarg + paramindex];

				Common::String tempsource = g_sci->getKernel()->lookupText(reg,
				                                  arguments[paramindex + 1]);
				int slen = tempsource.size();
				int extralen = strLength - slen;
				assert((target - targetbuf) + extralen <= maxsize);
				if (extralen < 0)
					extralen = 0;

				if (reg.getSegment()) /* Heap address? */
					paramindex++;
				else
					paramindex += 2; /* No, text resource address */

				switch (align) {

				case ALIGN_NONE:
				case ALIGN_RIGHT:
					while (extralen-- > 0)
						*target++ = ' '; /* Format into the text */
					break;

				case ALIGN_CENTER: {
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

				case ALIGN_CENTER: {
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
					while (strLength-- > 1)
						*target++ = ' '; /* Format into the text */
				char argchar = arguments[paramindex++];
				if (argchar)
					*target++ = argchar;
				mode = 0;
			}
			break;

			case 'x':
			case 'u':
				unsignedVar = true;
				/* fall through */
			case 'd': { /* Copy decimal */
				/* int templen; -- unused atm */
				const char *format_string = "%d";

				if (xfer == 'x')
					format_string = "%x";

				int val = arguments[paramindex];
				if (!unsignedVar)
					val = (int16)arguments[paramindex];

				target += sprintf(target, format_string, val);
				paramindex++;
				assert((target - targetbuf) <= maxsize);

				unsignedVar = false;

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
				int padding = strLength - written;

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
	const Common::String text = g_sci->getKernel()->lookupText(make_reg(0, argv[0].toUint16()), argv[1].toUint16());

	// If the third argument is NULL, allocate memory for the destination. This
	// occurs in SCI1 Mac games. The memory will later be freed by the game's
	// scripts.
	if (argv[2] == NULL_REG)
		s->_segMan->allocDynmem(text.size() + 1, "Mac FarText", &argv[2]);

	s->_segMan->strcpy(argv[2], text.c_str()); // Copy the string and get return value
	return argv[2];
}

#define DUMMY_MESSAGE "Message not found!"

enum kMessageFunc {
	K_MESSAGE_GET,
	K_MESSAGE_NEXT,
	K_MESSAGE_SIZE,
	K_MESSAGE_REFNOUN,
	K_MESSAGE_REFVERB,
	K_MESSAGE_REFCOND,
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
	uint16 module = (argc >= 2) ? argv[1].toUint16() : 0;

#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		// In complete weirdness, SCI32 bumps up subops 3-8 to 4-9 and stubs off subop 3.
		if (func == 3)
			error("SCI32 kMessage(3)");
		else if (func > 3)
			func--;
	}
#endif

//	TODO: Perhaps fix this check, currently doesn't work with PUSH and POP subfunctions
//	Pepper uses them to to handle the glossary
//	if ((func != K_MESSAGE_NEXT) && (argc < 2)) {
//		warning("Message: not enough arguments passed to subfunction %d", func);
//		return NULL_REG;
//	}

	MessageTuple tuple;

	if (argc >= 6)
		tuple = MessageTuple(argv[2].toUint16(), argv[3].toUint16(), argv[4].toUint16(), argv[5].toUint16());

	// WORKAROUND for a script bug in Pepper. When using objects together,
	// there is code inside script 894 that shows appropriate messages.
	// In the case of the jar of cabbage (noun 26), the relevant message
	// shown when using any object with it is missing. This leads to the
	// script code being triggered, which modifies the jar's noun and
	// message selectors, and renders it useless. Thus, when using any
	// object with the jar of cabbage, it's effectively corrupted, and
	// can't be used on the goat to empty it, therefore the game reaches
	// an unsolvable state. It's almost impossible to patch the offending
	// script, as it is used in many cases. But we can prevent the
	// corruption of the jar here: if the message is found, the offending
	// code is never reached and the jar is never corrupted. To do this,
	// we substitute all verbs on the cabbage jar with the default verb,
	// which shows the "Cannot use this object with the jar" message, and
	// never triggers the offending script code that corrupts the object.
	// This only affects the jar of cabbage - any other object, including
	// the empty jar has a different noun, thus it's unaffected.
	// Fixes bug #3601090.
	// NOTE: To fix a corrupted jar object, type "send Glass_Jar message 52"
	// in the debugger.
	if (g_sci->getGameId() == GID_PEPPER && func == 0 && argc >= 6 && module == 894 &&
		tuple.noun == 26 && tuple.cond == 0 && tuple.seq == 1 &&
		!s->_msgState->getMessage(module, tuple, NULL_REG))
		tuple.verb = 0;

	switch (func) {
	case K_MESSAGE_GET:
		return make_reg(0, s->_msgState->getMessage(module, tuple, (argc == 7 ? argv[6] : NULL_REG)));
	case K_MESSAGE_NEXT:
		return make_reg(0, s->_msgState->nextMessage((argc == 2 ? argv[1] : NULL_REG)));
	case K_MESSAGE_SIZE:
		return make_reg(0, s->_msgState->messageSize(module, tuple));
	case K_MESSAGE_REFCOND:
	case K_MESSAGE_REFVERB:
	case K_MESSAGE_REFNOUN: {
		MessageTuple t;

		if (s->_msgState->messageRef(module, tuple, t)) {
			switch (func) {
			case K_MESSAGE_REFCOND:
				return make_reg(0, t.cond);
			case K_MESSAGE_REFVERB:
				return make_reg(0, t.verb);
			case K_MESSAGE_REFNOUN:
				return make_reg(0, t.noun);
			default:
				break;
			}
		}

		return SIGNAL_REG;
	}
	case K_MESSAGE_LASTMESSAGE: {
		MessageTuple msg;
		int lastModule;

		s->_msgState->lastQuery(lastModule, msg);

		bool ok = false;

		if (s->_segMan->dereference(argv[1]).isRaw) {
			byte *buffer = s->_segMan->derefBulkPtr(argv[1], 10);

			if (buffer) {
				ok = true;
				WRITE_LE_UINT16(buffer, lastModule);
				WRITE_LE_UINT16(buffer + 2, msg.noun);
				WRITE_LE_UINT16(buffer + 4, msg.verb);
				WRITE_LE_UINT16(buffer + 6, msg.cond);
				WRITE_LE_UINT16(buffer + 8, msg.seq);
			}
		} else {
			reg_t *buffer = s->_segMan->derefRegPtr(argv[1], 5);

			if (buffer) {
				ok = true;
				buffer[0] = make_reg(0, lastModule);
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
	case K_MESSAGE_PUSH:
		s->_msgState->pushCursorStack();
		break;
	case K_MESSAGE_POP:
		s->_msgState->popCursorStack();
		break;
	default:
		warning("Message: subfunction %i invoked (not implemented)", func);
	}

	return NULL_REG;
}

reg_t kSetQuitStr(EngineState *s, int argc, reg_t *argv) {
	//Common::String quitStr = s->_segMan->getString(argv[0]);
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

#ifdef ENABLE_SCI32

reg_t kString(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kStringNew(EngineState *s, int argc, reg_t *argv) {
	reg_t stringHandle;
	const uint16 size = argv[0].toUint16();
	s->_segMan->allocateArray(kArrayTypeString, size, &stringHandle);
	return stringHandle;
}

reg_t kStringGetChar(EngineState *s, int argc, reg_t *argv) {
	const uint16 index = argv[1].toUint16();

	// Game scripts may contain static raw string data
	if (!s->_segMan->isArray(argv[0])) {
		const Common::String string = s->_segMan->getString(argv[0]);
		if (index >= string.size()) {
			return make_reg(0, 0);
		}

		return make_reg(0, (byte)string[index]);
	}

	SciArray &array = *s->_segMan->lookupArray(argv[0]);

	if (index >= array.size()) {
		return make_reg(0, 0);
	}

	return array.getAsID(index);
}

reg_t kStringFree(EngineState *s, int argc, reg_t *argv) {
	s->_segMan->freeArray(argv[0]);
	return s->r_acc;
}

reg_t kStringCompare(EngineState *s, int argc, reg_t *argv) {
	const Common::String string1 = s->_segMan->getString(argv[0]);
	const Common::String string2 = s->_segMan->getString(argv[1]);

	int result;
	if (argc == 3) {
		result = strncmp(string1.c_str(), string2.c_str(), argv[2].toUint16());
	} else {
		result = strcmp(string1.c_str(), string2.c_str());
	}

	return make_reg(0, (result > 0) - (result < 0));
}

reg_t kStringLength(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, s->_segMan->getString(argv[0]).size());
}

namespace {
	bool isFlag(const char c) {
		return strchr("-+ 0#", c);
	}

	bool isPrecision(const char c) {
		return strchr(".0123456789*", c);
	}

	bool isWidth(const char c) {
		return strchr("0123456789*", c);
	}

	bool isLength(const char c) {
		return strchr("hjlLtz", c);
	}

	bool isType(const char c) {
		return strchr("dsuxXaAceEfFgGinop", c);
	}

	bool isSignedType(const char type) {
		// For whatever reason, %d ends up being treated as unsigned in SSCI
		return type == 'i';
	}

	bool isUnsignedType(const char type) {
		return strchr("duxXoc", type);
	}

	bool isStringType(const char type) {
		return type == 's';
	}

	Common::String readPlaceholder(const char *&in, reg_t arg) {
		const char *const start = in;

		assert(*in == '%');
		++in;

		while (isFlag(*in)) {
			++in;
		}
		while (isWidth(*in)) {
			++in;
		}
		while (isPrecision(*in)) {
			++in;
		}
		while (isLength(*in)) {
			++in;
		}

		char format[64];
		format[0] = '\0';
		const char type = *in++;
		Common::strlcpy(format, start, MIN<size_t>(64, in - start + 1));

		if (isType(type)) {
			if (isSignedType(type)) {
				const int value = arg.toSint16();
				return Common::String::format(format, value);
			} else if (isUnsignedType(type)) {
				const uint value = arg.toUint16();
				return Common::String::format(format, value);
			} else if (isStringType(type)) {
				Common::String value;
				SegManager *segMan = g_sci->getEngineState()->_segMan;
				if (segMan->isObject(arg)) {
					value = segMan->getString(readSelector(segMan, arg, SELECTOR(data)));
				} else {
					value = segMan->getString(arg);
				}
				return Common::String::format(format, value.c_str());
			} else {
				error("Unsupported format type %c", type);
			}
		} else {
			return Common::String::format("%s", format);
		}
	}
}

Common::String format(const Common::String &source, int argc, const reg_t *argv) {
	Common::String out;
	const char *in = source.c_str();
	int argIndex = 0;
	while (*in != '\0') {
		if (*in == '%') {
			if (in[1] == '%') {
				in += 2;
				out += "%";
				continue;
			}

			if (argIndex < argc) {
				out += readPlaceholder(in, argv[argIndex++]);
			} else {
				out += readPlaceholder(in, NULL_REG);
			}
		} else {
			out += *in++;
		}
	}

	return out;
}

reg_t kStringFormat(EngineState *s, int argc, reg_t *argv) {
	Common::Array<reg_t> args;
	args.resize(argc + 1);
	args[0] = NULL_REG;
	Common::copy(argv, argv + argc, &args[1]);
	return kStringFormatAt(s, args.size(), &args[0]);
}

reg_t kStringFormatAt(EngineState *s, int argc, reg_t *argv) {
	reg_t stringHandle;
	SciArray *target;
	if (argv[0].isNull()) {
		target = s->_segMan->allocateArray(kArrayTypeString, 0, &stringHandle);
	} else {
		target = s->_segMan->lookupArray(argv[0]);
		stringHandle = argv[0];
	}

	reg_t source = argv[1];
	// Str objects may be passed in place of direct references to string data
	if (s->_segMan->isObject(argv[1])) {
		source = readSelector(s->_segMan, argv[1], SELECTOR(data));
	}
	target->fromString(format(s->_segMan->getString(source), argc - 2, argv + 2));
	return stringHandle;
}

reg_t kStringToInteger(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, (uint16)s->_segMan->getString(argv[0]).asUint64());
}

reg_t kStringTrim(EngineState *s, int argc, reg_t *argv) {
	SciArray &array = *s->_segMan->lookupArray(argv[0]);
	const int8 flags = argv[1].toSint16();
	const char showChar = argc > 2 ? argv[2].toSint16() : '\0';
	array.trim(flags, showChar);
	return s->r_acc;
}

reg_t kStringToUpperCase(EngineState *s, int argc, reg_t *argv) {
	Common::String string = s->_segMan->getString(argv[0]);
	string.toUppercase();
	s->_segMan->strcpy(argv[0], string.c_str());
	return argv[0];
}

reg_t kStringToLowerCase(EngineState *s, int argc, reg_t *argv) {
	Common::String string = s->_segMan->getString(argv[0]);
	string.toLowercase();
	s->_segMan->strcpy(argv[0], string.c_str());
	return argv[0];
}

reg_t kStringReplaceSubstring(EngineState *s, int argc, reg_t *argv) {
	error("TODO: kStringReplaceSubstring not implemented");
	return argv[3];
}

reg_t kStringReplaceSubstringEx(EngineState *s, int argc, reg_t *argv) {
	error("TODO: kStringReplaceSubstringEx not implemented");
	return argv[3];
}
#endif

} // End of namespace Sci
