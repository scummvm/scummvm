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

#include "sci/scicore/resource.h"
#include "sci/engine/state.h"
#include "common/util.h"

namespace Sci {

// #define SCRIPT_DEBUG

#define END Script_None

opcode_format formats[128][4] = {
	/*00*/
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	/*04*/
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	/*08*/
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	/*0C*/
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	/*10*/
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	/*14*/
	{Script_None}, {Script_None}, {Script_None}, {Script_SRelative, END},
	/*18*/
	{Script_SRelative, END}, {Script_SRelative, END}, {Script_SVariable, END}, {Script_None},
	/*1C*/
	{Script_SVariable, END}, {Script_None}, {Script_None}, {Script_Variable, END},
	/*20*/
	{Script_SRelative, Script_Byte, END}, {Script_Variable, Script_Byte, END}, {Script_Variable, Script_Byte, END}, {Script_Variable, Script_SVariable, Script_Byte, END},
	/*24 (24=ret)*/
	{Script_End}, {Script_Byte, END}, {Script_Invalid}, {Script_Invalid},
	/*28*/
	{Script_Variable, END}, {Script_Invalid}, {Script_Byte, END}, {Script_Variable, Script_Byte, END},
	/*2C*/
	{Script_SVariable, END}, {Script_SVariable, Script_Variable, END}, {Script_None}, {Script_Invalid},
	/*30*/
	{Script_None}, {Script_Property, END}, {Script_Property, END}, {Script_Property, END},
	/*34*/
	{Script_Property, END}, {Script_Property, END}, {Script_Property, END}, {Script_Property, END},
	/*38*/
	{Script_Property, END}, {Script_SRelative, END}, {Script_SRelative, END}, {Script_None},
	/*3C*/
	{Script_None}, {Script_None}, {Script_None}, {Script_Invalid},
	/*40-4F*/
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	/*50-5F*/
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	/*60-6F*/
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	/*70-7F*/
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END},
	{Script_Global, END}, {Script_Local, END}, {Script_Temp, END}, {Script_Param, END}
};
#undef END

void script_adjust_opcode_formats(int res_version) {
	switch (res_version) {
	case SCI_VERSION_0:
	case SCI_VERSION_01:
		break;
	case SCI_VERSION_01_VGA:
	case SCI_VERSION_01_VGA_ODD:
	case SCI_VERSION_1_EARLY:
	case SCI_VERSION_1_LATE:
	case SCI_VERSION_1_1:
		formats[op_lofsa][0] = Script_Offset;
		formats[op_lofss][0] = Script_Offset;
		break;
	default:
		sciprintf("script_adjust_opcode_formats(): Unknown script version %d\n", res_version);
	}
}

int script_find_selector(EngineState *s, const char *selectorname) {
	for (uint pos = 0; pos < s->_selectorNames.size(); ++pos) {
		if (s->_selectorNames[pos] == selectorname)
			return pos;
	}

	warning("Could not map '%s' to any selector!", selectorname);

	return -1;
}

#define FIND_SELECTOR(_slc_) map->_slc_ = script_find_selector(s, #_slc_)
#define FIND_SELECTOR2(_slc_, _slcstr_) map->_slc_ = script_find_selector(s, _slcstr_)

void script_map_selectors(EngineState *s, selector_map_t *map) {
	FIND_SELECTOR(init);
	FIND_SELECTOR(play);
	FIND_SELECTOR(replay);
	FIND_SELECTOR(x);
	FIND_SELECTOR(y);
	FIND_SELECTOR(z);
	FIND_SELECTOR(priority);
	FIND_SELECTOR(view);
	FIND_SELECTOR(loop);
	FIND_SELECTOR(cel);
	FIND_SELECTOR(brLeft);
	FIND_SELECTOR(brRight);
	FIND_SELECTOR(brTop);
	FIND_SELECTOR(brBottom);
	FIND_SELECTOR(xStep);
	FIND_SELECTOR(yStep);
	FIND_SELECTOR(nsBottom);
	FIND_SELECTOR(nsTop);
	FIND_SELECTOR(nsLeft);
	FIND_SELECTOR(nsRight);
	FIND_SELECTOR(font);
	FIND_SELECTOR(text);
	FIND_SELECTOR(type);
	FIND_SELECTOR(state);
	FIND_SELECTOR(doit);
	FIND_SELECTOR2(delete_, "delete");
	FIND_SELECTOR(signal);
	FIND_SELECTOR(underBits);
	FIND_SELECTOR(canBeHere);
	FIND_SELECTOR(client);
	FIND_SELECTOR(dx);
	FIND_SELECTOR(dy);
	FIND_SELECTOR(xStep);
	FIND_SELECTOR(yStep);
	FIND_SELECTOR2(b_movCnt, "b-moveCnt");
	FIND_SELECTOR2(b_i1, "b-i1");
	FIND_SELECTOR2(b_i2, "b-i2");
	FIND_SELECTOR2(b_di, "b-di");
	FIND_SELECTOR2(b_xAxis, "b-xAxis");
	FIND_SELECTOR2(b_incr, "b-incr");
	FIND_SELECTOR(completed);
	FIND_SELECTOR(illegalBits);
	FIND_SELECTOR(dispose);
	FIND_SELECTOR(prevSignal);
	FIND_SELECTOR(message);
	FIND_SELECTOR(modifiers);
	FIND_SELECTOR(cue);
	FIND_SELECTOR(owner);
	FIND_SELECTOR(handle);
	FIND_SELECTOR(number);
	FIND_SELECTOR(max);
	FIND_SELECTOR(cursor);
	FIND_SELECTOR(claimed);
	FIND_SELECTOR(edgeHit);
	FIND_SELECTOR(wordFail);
	FIND_SELECTOR(syntaxFail);
	FIND_SELECTOR(semanticFail);
	FIND_SELECTOR(cycler);
	FIND_SELECTOR(elements);
	FIND_SELECTOR(lsTop);
	FIND_SELECTOR(lsBottom);
	FIND_SELECTOR(lsLeft);
	FIND_SELECTOR(lsRight);
	FIND_SELECTOR(baseSetter);
	FIND_SELECTOR(who);
	FIND_SELECTOR(distance);
	FIND_SELECTOR(mover);
	FIND_SELECTOR(looper);
	FIND_SELECTOR(isBlocked);
	FIND_SELECTOR(heading);
	FIND_SELECTOR(mode);
	FIND_SELECTOR(caller);
	FIND_SELECTOR(moveDone);
	FIND_SELECTOR(vol);
	FIND_SELECTOR(pri);
	FIND_SELECTOR(min);
	FIND_SELECTOR(sec);
	FIND_SELECTOR(frame);
	FIND_SELECTOR(dataInc);
	FIND_SELECTOR(size);
	FIND_SELECTOR(palette);
	FIND_SELECTOR(moveSpeed);
	FIND_SELECTOR(cantBeHere);
	FIND_SELECTOR(nodePtr);
	FIND_SELECTOR(flags);
	FIND_SELECTOR(points);
}

int sci_hexdump(byte *data, int length, int offsetplus) {
	char tempstr[40];
	int i;

	for (i = 0; i < length; i += 8) {
		int j;

		sprintf(tempstr, "%04x:                                 ", i + offsetplus);
		for (j = 0; j < MIN(8, length - i); j++)
			sprintf(tempstr + 6 + (j*3) + (j > 3), "%02x  ", data[i+j]);
		for (j = 0; j < MIN(8, length - i); j++) {
			int thechar;
			thechar = data[i+j];
			sprintf(tempstr + 31 + j, "%c", ((thechar < ' ') || (thechar > 127)) ? '.' : thechar);
		}

		for (j = 0; j < 38; j++)
			if (!tempstr[j])
				tempstr[j] = ' '; // get rid of sprintf's \0s

		sciprintf("%s\n", tempstr);
	}

	return 0;
}

static void script_dump_object(char *data, int seeker, int objsize, const Common::StringList &selectorNames) {
	int selectors, overloads, selectorsize;
	int species = getInt16((unsigned char *) data + 8 + seeker);
	int superclass = getInt16((unsigned char *) data + 10 + seeker);
	int namepos = getInt16((unsigned char *) data + 14 + seeker);
	int i = 0;

	sciprintf("Object\n");

	sci_hexdump((unsigned char *) data + seeker, objsize - 4, seeker);
	//-4 because the size includes the two-word header

	sciprintf("Name: %s\n", namepos ? ((char *)(data + namepos)) : "<unknown>");
	sciprintf("Superclass: %x\n", superclass);
	sciprintf("Species: %x\n", species);
	sciprintf("-info-:%x\n", getInt16((unsigned char *) data + 12 + seeker) & 0xffff);

	sciprintf("Function area offset: %x\n", getInt16((unsigned char *) data + seeker + 4));
	sciprintf("Selectors [%x]:\n", selectors = (selectorsize = getInt16((unsigned char *) data + seeker + 6)));

	seeker += 8;

	while (selectors--) {
		sciprintf("  [#%03x] = 0x%x\n", i++, getInt16((unsigned char *)data + seeker) & 0xffff);

		seeker += 2;
	}

	sciprintf("Overridden functions: %x\n", selectors = overloads = getInt16((unsigned char *)data + seeker));

	seeker += 2;

	if (overloads < 100)
		while (overloads--) {
			int selector = getInt16((unsigned char *) data + (seeker));

			sciprintf("  [%03x] %s: @", selector & 0xffff, (selector >= 0 && selector < (int)selectorNames.size()) ? selectorNames[selector].c_str() : "<?>");
			sciprintf("%04x\n", getInt16((unsigned char *)data + seeker + selectors*2 + 2) & 0xffff);

			seeker += 2;
		}
}

static void script_dump_class(char *data, int seeker, int objsize, const Common::StringList &selectorNames) {
	int selectors, overloads, selectorsize;
	int species = getInt16((unsigned char *) data + 8 + seeker);
	int superclass = getInt16((unsigned char *) data + 10 + seeker);
	int namepos = getInt16((unsigned char *) data + 14 + seeker);

	sciprintf("Class\n");

	sci_hexdump((unsigned char *) data + seeker, objsize - 4, seeker);

	sciprintf("Name: %s\n", namepos ? ((char *)data + namepos) : "<unknown>");
	sciprintf("Superclass: %x\n", superclass);
	sciprintf("Species: %x\n", species);
	sciprintf("-info-:%x\n", getInt16((unsigned char *)data + 12 + seeker) & 0xffff);

	sciprintf("Function area offset: %x\n", getInt16((unsigned char *)data + seeker + 4));
	sciprintf("Selectors [%x]:\n", selectors = (selectorsize = getInt16((unsigned char *)data + seeker + 6)));

	seeker += 8;
	selectorsize <<= 1;

	while (selectors--) {
		int selector = getInt16((unsigned char *) data + (seeker) + selectorsize);

		sciprintf("  [%03x] %s = 0x%x\n", 0xffff & selector, (selector >= 0 && selector < (int)selectorNames.size()) ? selectorNames[selector].c_str() : "<?>",
		          getInt16((unsigned char *)data + seeker) & 0xffff);

		seeker += 2;
	}

	seeker += selectorsize;

	sciprintf("Overloaded functions: %x\n", selectors = overloads = getInt16((unsigned char *)data + seeker));

	seeker += 2;

	while (overloads--) {
		int selector = getInt16((unsigned char *)data + (seeker));
		fprintf(stderr, "selector=%d; selectorNames.size() =%d\n", selector, selectorNames.size());
		sciprintf("  [%03x] %s: @", selector & 0xffff, (selector >= 0 && selector < (int)selectorNames.size()) ?
		          selectorNames[selector].c_str() : "<?>");
		sciprintf("%04x\n", getInt16((unsigned char *)data + seeker + selectors * 2 + 2) & 0xffff);

		seeker += 2;
	}
}

void script_dissect(ResourceManager *resmgr, int res_no, const Common::StringList &selectorNames) {
	int objectctr[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned int _seeker = 0;
	resource_t *script = resmgr->findResource(sci_script, res_no, 0);
	word_t **words;
	int word_count;

	if (!script) {
		sciprintf("Script not found!\n");
		return;
	}

	words = vocab_get_words(resmgr, &word_count);

	while (_seeker < script->size) {
		int objtype = getInt16(script->data + _seeker);
		int objsize;
		unsigned int seeker = _seeker + 4;

		if (!objtype) {
			sciprintf("End of script object (#0) encountered.\n");
			sciprintf("Classes: %i, Objects: %i, Export: %i,\n Var: %i (all base 10)",
			          objectctr[6], objectctr[1], objectctr[7], objectctr[10]);
			vocab_free_words(words, word_count);
			return;
		}

		sciprintf("\n");

		objsize = getInt16(script->data + _seeker + 2);

		sciprintf("Obj type #%x, size 0x%x: ", objtype, objsize);

		_seeker += objsize;

		objectctr[objtype]++;

		switch (objtype) {
		case sci_obj_object:
			script_dump_object((char *)script->data, seeker, objsize, selectorNames);
			break;

		case sci_obj_code: {
			sciprintf("Code\n");
			sci_hexdump(script->data + seeker, objsize - 4, seeker);
		};
		break;

		case 3: {
			sciprintf("<unknown>\n");
			sci_hexdump(script->data + seeker, objsize - 4, seeker);
		};
		break;

		case sci_obj_said: {
			sciprintf("Said\n");
			sci_hexdump(script->data + seeker, objsize - 4, seeker);

			sciprintf("%04x: ", seeker);
			while (seeker < _seeker) {
				unsigned char nextitem = script->data [seeker++];
				if (nextitem == 0xFF)
					sciprintf("\n%04x: ", seeker);
				else if (nextitem >= 0xF0) {
					switch (nextitem) {
					case 0xf0:
						sciprintf(", ");
						break;
					case 0xf1:
						sciprintf("& ");
						break;
					case 0xf2:
						sciprintf("/ ");
						break;
					case 0xf3:
						sciprintf("( ");
						break;
					case 0xf4:
						sciprintf(") ");
						break;
					case 0xf5:
						sciprintf("[ ");
						break;
					case 0xf6:
						sciprintf("] ");
						break;
					case 0xf7:
						sciprintf("# ");
						break;
					case 0xf8:
						sciprintf("< ");
						break;
					case 0xf9:
						sciprintf("> ");
						break;
					}
				} else {
					nextitem = nextitem << 8 | script->data [seeker++];
					sciprintf("%s[%03x] ", vocab_get_any_group_word(nextitem, words, word_count), nextitem);
				}
			}
			sciprintf("\n");
		}
		break;

		case sci_obj_strings: {
			sciprintf("Strings\n");
			while (script->data [seeker]) {
				sciprintf("%04x: %s\n", seeker, script->data + seeker);
				seeker += strlen((char *)script->data + seeker) + 1;
			}
			seeker++; // the ending zero byte
		};
		break;

		case sci_obj_class:
			script_dump_class((char *)script->data, seeker, objsize, selectorNames);
			break;

		case sci_obj_exports: {
			sciprintf("Exports\n");
			sci_hexdump((unsigned char *)script->data + seeker, objsize - 4, seeker);
		};
		break;

		case sci_obj_pointers: {
			sciprintf("Pointers\n");
			sci_hexdump(script->data + seeker, objsize - 4, seeker);
		};
		break;

		case 9: {
			sciprintf("<unknown>\n");
			sci_hexdump(script->data + seeker, objsize - 4, seeker);
		};
		break;

		case sci_obj_localvars: {
			sciprintf("Local vars\n");
			sci_hexdump(script->data + seeker, objsize - 4, seeker);
		};
		break;

		default:
			sciprintf("Unsupported!\n");
			return;
		}

	}

	sciprintf("Script ends without terminator\n");
}

} // End of namespace Sci
