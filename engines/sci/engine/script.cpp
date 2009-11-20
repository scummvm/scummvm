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

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/engine/state.h"
#include "common/util.h"

namespace Sci {

#define END Script_None

opcode_format g_opcode_formats[128][4] = {
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

void script_adjust_opcode_formats(EngineState *s) {
	// TODO: Check that this is correct
	if (s->detectLofsType() != SCI_VERSION_0_EARLY) {
		g_opcode_formats[op_lofsa][0] = Script_Offset;
		g_opcode_formats[op_lofss][0] = Script_Offset;
	}

#ifdef ENABLE_SCI32
	// In SCI32, some arguments are now words instead of bytes
	if (getSciVersion() >= SCI_VERSION_2) {
		g_opcode_formats[op_calle][2] = Script_Word;
		g_opcode_formats[op_callk][1] = Script_Word;
		g_opcode_formats[op_super][1] = Script_Word;
		g_opcode_formats[op_send][0] = Script_Word;
		g_opcode_formats[op_self][0] = Script_Word;
		g_opcode_formats[op_call][1] = Script_Word;
		g_opcode_formats[op_callb][1] = Script_Word;
	}
#endif
}

#if 1

#define FIND_SELECTOR(_slc_) _selectorCache._slc_ = findSelector(#_slc_)
#define FIND_SELECTOR2(_slc_, _slcstr_) _selectorCache._slc_ = findSelector(_slcstr_)

#else

// The defines below can be used to construct static selector tables for games which don't have
// a vocab.997 resource, by dumping the selector table from other similar versions or games
#define FIND_SELECTOR(_slc_) _selectorCache._slc_ = findSelector(#_slc_); \
	printf("\t{ \"%s\", %d },\n", #_slc_, _selectorCache._slc_)
 
#define FIND_SELECTOR2(_slc_, _slcstr_) _selectorCache._slc_ = findSelector(_slcstr_); \
	printf("\t{ \"%s\", %d },\n", _slcstr_, _selectorCache._slc_)

#endif

void Kernel::mapSelectors() {
	// species
	// superClass
	// -info-
	FIND_SELECTOR(y);
	FIND_SELECTOR(x);
	FIND_SELECTOR(view);
	FIND_SELECTOR(loop);
	FIND_SELECTOR(cel);
	FIND_SELECTOR(underBits);
	FIND_SELECTOR(nsTop);
	FIND_SELECTOR(nsLeft);
	FIND_SELECTOR(nsBottom);
	FIND_SELECTOR(lsTop);
	FIND_SELECTOR(lsLeft);
	FIND_SELECTOR(lsBottom);
	FIND_SELECTOR(lsRight);
	FIND_SELECTOR(nsRight);
	FIND_SELECTOR(signal);
	FIND_SELECTOR(illegalBits);
	FIND_SELECTOR(brTop);
	FIND_SELECTOR(brLeft);
	FIND_SELECTOR(brBottom);
	FIND_SELECTOR(brRight);
	// name
	// key
	// time
	FIND_SELECTOR(text);
	FIND_SELECTOR(elements);
	// color
	// back
	FIND_SELECTOR(mode);
	// style
	FIND_SELECTOR(state);
	FIND_SELECTOR(font);
	FIND_SELECTOR(type);
	// window
	FIND_SELECTOR(cursor);
	FIND_SELECTOR(max);
	// mark
	// who
	FIND_SELECTOR(message);
	// edit
	FIND_SELECTOR(play);
	FIND_SELECTOR(number);
	FIND_SELECTOR(handle);	// nodePtr
	FIND_SELECTOR(client);
	FIND_SELECTOR(dx);
	FIND_SELECTOR(dy);
	FIND_SELECTOR2(b_movCnt, "b-moveCnt");
	FIND_SELECTOR2(b_i1, "b-i1");
	FIND_SELECTOR2(b_i2, "b-i2");
	FIND_SELECTOR2(b_di, "b-di");
	FIND_SELECTOR2(b_xAxis, "b-xAxis");
	FIND_SELECTOR2(b_incr, "b-incr");
	FIND_SELECTOR(xStep);
	FIND_SELECTOR(yStep);
	FIND_SELECTOR(moveSpeed);
	FIND_SELECTOR(canBeHere);	// cantBeHere
	FIND_SELECTOR(heading);
	FIND_SELECTOR(mover);
	FIND_SELECTOR(doit);
	FIND_SELECTOR(isBlocked);
	FIND_SELECTOR(looper);
	FIND_SELECTOR(priority);
	FIND_SELECTOR(modifiers);
	FIND_SELECTOR(replay);
	// setPri
	// at
	// next
	// done
	// width
	FIND_SELECTOR(wordFail);
	FIND_SELECTOR(syntaxFail);
	// semanticFail
	// pragmaFail
	// said
	FIND_SELECTOR(claimed);
	// value
	// save
	// restore
	// title
	// button
	// icon
	// draw
	FIND_SELECTOR2(delete_, "delete");
	FIND_SELECTOR(z);
	// -----------------------------
	FIND_SELECTOR(size);
	FIND_SELECTOR(moveDone);
	FIND_SELECTOR(vol);
	FIND_SELECTOR(pri);
	FIND_SELECTOR(min);
	FIND_SELECTOR(sec);
	FIND_SELECTOR(frame);
	FIND_SELECTOR(dataInc);
	FIND_SELECTOR(palette);
	FIND_SELECTOR(cantBeHere);
	FIND_SELECTOR(nodePtr);
	FIND_SELECTOR(flags);
	FIND_SELECTOR(points);
	FIND_SELECTOR(syncCue);
	FIND_SELECTOR(syncTime);
	FIND_SELECTOR(printLang);
	FIND_SELECTOR(subtitleLang);
	FIND_SELECTOR(parseLang);
	FIND_SELECTOR(overlay);
	FIND_SELECTOR(newRoom);
	FIND_SELECTOR(setCursor);
	FIND_SELECTOR(topString);
	FIND_SELECTOR(scaleX);
	FIND_SELECTOR(scaleY);
}

void Kernel::dumpScriptObject(char *data, int seeker, int objsize) {
	int selectors, overloads, selectorsize;
	int species = (int16)READ_LE_UINT16((unsigned char *) data + 8 + seeker);
	int superclass = (int16)READ_LE_UINT16((unsigned char *) data + 10 + seeker);
	int namepos = (int16)READ_LE_UINT16((unsigned char *) data + 14 + seeker);
	int i = 0;

	printf("Object\n");

	Common::hexdump((unsigned char *) data + seeker, objsize - 4, 16, seeker);
	//-4 because the size includes the two-word header

	printf("Name: %s\n", namepos ? ((char *)(data + namepos)) : "<unknown>");
	printf("Superclass: %x\n", superclass);
	printf("Species: %x\n", species);
	printf("-info-:%x\n", (int16)READ_LE_UINT16((unsigned char *) data + 12 + seeker) & 0xffff);

	printf("Function area offset: %x\n", (int16)READ_LE_UINT16((unsigned char *) data + seeker + 4));
	printf("Selectors [%x]:\n", selectors = (selectorsize = (int16)READ_LE_UINT16((unsigned char *) data + seeker + 6)));

	seeker += 8;

	while (selectors--) {
		printf("  [#%03x] = 0x%x\n", i++, (int16)READ_LE_UINT16((unsigned char *)data + seeker) & 0xffff);
		seeker += 2;
	}

	printf("Overridden functions: %x\n", selectors = overloads = (int16)READ_LE_UINT16((unsigned char *)data + seeker));

	seeker += 2;

	if (overloads < 100)
		while (overloads--) {
			int selector = (int16)READ_LE_UINT16((unsigned char *) data + (seeker));

			printf("  [%03x] %s: @", selector & 0xffff, (selector >= 0 && selector < (int)_selectorNames.size()) ? _selectorNames[selector].c_str() : "<?>");
			printf("%04x\n", (int16)READ_LE_UINT16((unsigned char *)data + seeker + selectors*2 + 2) & 0xffff);

			seeker += 2;
		}
}

void Kernel::dumpScriptClass(char *data, int seeker, int objsize) {
	int selectors, overloads, selectorsize;
	int species = (int16)READ_LE_UINT16((unsigned char *) data + 8 + seeker);
	int superclass = (int16)READ_LE_UINT16((unsigned char *) data + 10 + seeker);
	int namepos = (int16)READ_LE_UINT16((unsigned char *) data + 14 + seeker);

	printf("Class\n");

	Common::hexdump((unsigned char *) data + seeker, objsize - 4, 16, seeker);

	printf("Name: %s\n", namepos ? ((char *)data + namepos) : "<unknown>");
	printf("Superclass: %x\n", superclass);
	printf("Species: %x\n", species);
	printf("-info-:%x\n", (int16)READ_LE_UINT16((unsigned char *)data + 12 + seeker) & 0xffff);

	printf("Function area offset: %x\n", (int16)READ_LE_UINT16((unsigned char *)data + seeker + 4));
	printf("Selectors [%x]:\n", selectors = (selectorsize = (int16)READ_LE_UINT16((unsigned char *)data + seeker + 6)));

	seeker += 8;
	selectorsize <<= 1;

	while (selectors--) {
		int selector = (int16)READ_LE_UINT16((unsigned char *) data + (seeker) + selectorsize);

		printf("  [%03x] %s = 0x%x\n", 0xffff & selector, (selector >= 0 && selector < (int)_selectorNames.size()) ? _selectorNames[selector].c_str() : "<?>",
		          (int16)READ_LE_UINT16((unsigned char *)data + seeker) & 0xffff);

		seeker += 2;
	}

	seeker += selectorsize;

	printf("Overloaded functions: %x\n", selectors = overloads = (int16)READ_LE_UINT16((unsigned char *)data + seeker));

	seeker += 2;

	while (overloads--) {
		int selector = (int16)READ_LE_UINT16((unsigned char *)data + (seeker));
		fprintf(stderr, "selector=%d; selectorNames.size() =%d\n", selector, _selectorNames.size());
		printf("  [%03x] %s: @", selector & 0xffff, (selector >= 0 && selector < (int)_selectorNames.size()) ?
		          _selectorNames[selector].c_str() : "<?>");
		printf("%04x\n", (int16)READ_LE_UINT16((unsigned char *)data + seeker + selectors * 2 + 2) & 0xffff);

		seeker += 2;
	}
}

void Kernel::dissectScript(int scriptNumber, Vocabulary *vocab) {
	int objectctr[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned int _seeker = 0;
	Resource *script = _resMan->findResource(ResourceId(kResourceTypeScript, scriptNumber), 0);

	if (!script) {
		warning("dissectScript(): Script not found!\n");
		return;
	}

	while (_seeker < script->size) {
		int objtype = (int16)READ_LE_UINT16(script->data + _seeker);
		int objsize;
		unsigned int seeker = _seeker + 4;

		if (!objtype) {
			printf("End of script object (#0) encountered.\n");
			printf("Classes: %i, Objects: %i, Export: %i,\n Var: %i (all base 10)",
			          objectctr[6], objectctr[1], objectctr[7], objectctr[10]);
			return;
		}

		printf("\n");

		objsize = (int16)READ_LE_UINT16(script->data + _seeker + 2);

		printf("Obj type #%x, size 0x%x: ", objtype, objsize);

		_seeker += objsize;

		objectctr[objtype]++;

		switch (objtype) {
		case SCI_OBJ_OBJECT:
			dumpScriptObject((char *)script->data, seeker, objsize);
			break;

		case SCI_OBJ_CODE: {
			printf("Code\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
		};
		break;

		case 3: {
			printf("<unknown>\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
		};
		break;

		case SCI_OBJ_SAID: {
			printf("Said\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);

			printf("%04x: ", seeker);
			while (seeker < _seeker) {
				unsigned char nextitem = script->data [seeker++];
				if (nextitem == 0xFF)
					printf("\n%04x: ", seeker);
				else if (nextitem >= 0xF0) {
					switch (nextitem) {
					case 0xf0:
						printf(", ");
						break;
					case 0xf1:
						printf("& ");
						break;
					case 0xf2:
						printf("/ ");
						break;
					case 0xf3:
						printf("( ");
						break;
					case 0xf4:
						printf(") ");
						break;
					case 0xf5:
						printf("[ ");
						break;
					case 0xf6:
						printf("] ");
						break;
					case 0xf7:
						printf("# ");
						break;
					case 0xf8:
						printf("< ");
						break;
					case 0xf9:
						printf("> ");
						break;
					}
				} else {
					nextitem = nextitem << 8 | script->data [seeker++];
					printf("%s[%03x] ", vocab->getAnyWordFromGroup(nextitem), nextitem);
				}
			}
			printf("\n");
		}
		break;

		case SCI_OBJ_STRINGS: {
			printf("Strings\n");
			while (script->data [seeker]) {
				printf("%04x: %s\n", seeker, script->data + seeker);
				seeker += strlen((char *)script->data + seeker) + 1;
			}
			seeker++; // the ending zero byte
		};
		break;

		case SCI_OBJ_CLASS:
			dumpScriptClass((char *)script->data, seeker, objsize);
			break;

		case SCI_OBJ_EXPORTS: {
			printf("Exports\n");
			Common::hexdump((unsigned char *)script->data + seeker, objsize - 4, 16, seeker);
		};
		break;

		case SCI_OBJ_POINTERS: {
			printf("Pointers\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
		};
		break;

		case 9: {
			printf("<unknown>\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
		};
		break;

		case SCI_OBJ_LOCALVARS: {
			printf("Local vars\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
		};
		break;

		default:
			printf("Unsupported!\n");
			return;
		}

	}

	printf("Script ends without terminator\n");
}

} // End of namespace Sci
