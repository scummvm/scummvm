/***************************************************************************
 script.c  Copyright (C) 2003 Magnus Reftel, Christoph Reichenbach


 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence as
 published by the Free Software Foundaton; either version 2 of the
 Licence, or (at your option) any later version.

 It is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 merchantibility or fitness for a particular purpose. See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with this program; see the file COPYING. If not, write to
 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.


 Please contact the maintainer for any program-related bug reports or
 inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

#include <sciresource.h>
#include <engine.h>

/* #define SCRIPT_DEBUG */

#define END Script_None

opcode_format formats[128][4]={
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

void script_adjust_opcode_formats(int res_version)
{
	switch (res_version)
	{
		case SCI_VERSION_0:
		case SCI_VERSION_01:
			break;
		case SCI_VERSION_01_VGA:
		case SCI_VERSION_01_VGA_ODD:
		case SCI_VERSION_1_EARLY:
		case SCI_VERSION_1_LATE:
			formats[op_lofsa][0]=Script_Offset;
			formats[op_lofss][0]=Script_Offset;
			break;
		default:
			sciprintf("script_adjust_opcode_formats(): Unknown script version %d\n", res_version);
	}
}
	
int
script_find_selector(state_t *s, const char *selectorname)
{
  int i;
  for (i = 0; i < s->selector_names_nr; i++)
    if (strcmp(selectorname, s->selector_names[i]) == 0)
        return i;

  sciprintf("Warning: Could not map '%s' to any selector!\n", selectorname);
  return -1;
}

#define FIND_SELECTOR(_slc_, _slcstr_) map->_slc_ = script_find_selector(s, _slcstr_);

void
script_map_selectors(state_t *s, selector_map_t *map)
{
  map->init = script_find_selector(s, "init");
  map->play = script_find_selector(s, "play");
  FIND_SELECTOR(replay, "replay");
  map->x = script_find_selector(s, "x");
  map->y = script_find_selector(s, "y");
  map->z = script_find_selector(s, "z");
  map->priority = script_find_selector(s, "priority");
  map->view = script_find_selector(s, "view");
  map->loop = script_find_selector(s, "loop");
  map->cel = script_find_selector(s, "cel");
  FIND_SELECTOR(brLeft, "brLeft");
  FIND_SELECTOR(brRight, "brRight");
  FIND_SELECTOR(brTop, "brTop");
  FIND_SELECTOR(brBottom, "brBottom");
  FIND_SELECTOR(xStep, "xStep");
  FIND_SELECTOR(yStep, "yStep");
  FIND_SELECTOR(nsBottom, "nsBottom");
  FIND_SELECTOR(nsTop, "nsTop");
  FIND_SELECTOR(nsLeft, "nsLeft");
  FIND_SELECTOR(nsRight, "nsRight");
  FIND_SELECTOR(font, "font");
  FIND_SELECTOR(text, "text");
  FIND_SELECTOR(type, "type");
  FIND_SELECTOR(state, "state");
  FIND_SELECTOR(doit, "doit");
  FIND_SELECTOR(delete, "delete");
  FIND_SELECTOR(signal, "signal");
  FIND_SELECTOR(underBits, "underBits");
  FIND_SELECTOR(canBeHere, "canBeHere");
  FIND_SELECTOR(client, "client");
  FIND_SELECTOR(dx, "dx");
  FIND_SELECTOR(dy, "dy");
  FIND_SELECTOR(xStep, "xStep");
  FIND_SELECTOR(yStep, "yStep");
  FIND_SELECTOR(b_movCnt, "b-moveCnt");
  FIND_SELECTOR(b_i1, "b-i1");
  FIND_SELECTOR(b_i2, "b-i2");
  FIND_SELECTOR(b_di, "b-di");
  FIND_SELECTOR(b_xAxis, "b-xAxis");
  FIND_SELECTOR(b_incr, "b-incr");
  FIND_SELECTOR(completed, "completed");
  FIND_SELECTOR(illegalBits, "illegalBits");
  FIND_SELECTOR(dispose, "dispose");
  FIND_SELECTOR(prevSignal, "prevSignal");
  FIND_SELECTOR(message, "message");
  FIND_SELECTOR(modifiers, "modifiers");
  FIND_SELECTOR(cue, "cue");
  FIND_SELECTOR(owner, "owner");
  FIND_SELECTOR(handle, "handle");
  FIND_SELECTOR(number, "number");
  FIND_SELECTOR(max, "max");
  FIND_SELECTOR(cursor, "cursor");
  FIND_SELECTOR(claimed, "claimed");
  FIND_SELECTOR(edgeHit, "edgeHit");
  FIND_SELECTOR(wordFail, "wordFail");
  FIND_SELECTOR(syntaxFail, "syntaxFail");
  FIND_SELECTOR(semanticFail, "semanticFail");
  FIND_SELECTOR(cycler, "cycler");
  FIND_SELECTOR(elements, "elements");
  FIND_SELECTOR(lsTop, "lsTop");
  FIND_SELECTOR(lsBottom, "lsBottom");
  FIND_SELECTOR(lsLeft, "lsLeft");
  FIND_SELECTOR(lsRight, "lsRight");
  FIND_SELECTOR(baseSetter, "baseSetter");
  FIND_SELECTOR(who, "who");
  FIND_SELECTOR(distance, "distance");
  FIND_SELECTOR(mover, "mover");
  FIND_SELECTOR(looper, "looper");
  FIND_SELECTOR(isBlocked, "isBlocked");
  FIND_SELECTOR(heading, "heading");
  FIND_SELECTOR(mode, "mode");
  FIND_SELECTOR(caller, "caller");
  FIND_SELECTOR(moveDone, "moveDone");
  FIND_SELECTOR(vol, "vol");
  FIND_SELECTOR(pri, "pri");
  FIND_SELECTOR(min, "min");
  FIND_SELECTOR(sec, "sec");
  FIND_SELECTOR(frame, "frame");
  FIND_SELECTOR(dataInc, "dataInc");
  FIND_SELECTOR(size, "size");
  FIND_SELECTOR(palette, "palette");
  FIND_SELECTOR(moveSpeed, "moveSpeed");
  FIND_SELECTOR(cantBeHere, "cantBeHere");
  FIND_SELECTOR(nodePtr, "nodePtr");
  FIND_SELECTOR(flags, "flags");
  FIND_SELECTOR(points, "points");
}

int
sci_hexdump(byte *data, int length, int offsetplus)
{
  char tempstr[40];
  int i;
  for (i = 0; i < length; i+= 8) {
    int j;

    sprintf(tempstr, "%04x:                                 ", i + offsetplus);
    for (j = 0; j < MIN(8, length - i); j++)
      sprintf(tempstr + 6 + (j*3) + (j > 3), "%02x  ", data[i+j]);
    for (j = 0; j < MIN(8, length - i); j++) {
      int thechar;
      thechar = data[i+j];
      sprintf(tempstr + 31 + j, "%c",
	      ((thechar < ' ') || (thechar > 127))? '.' : thechar );
    }

    for (j = 0; j < 38; j++)
      if (!tempstr[j])
	tempstr[j] = ' '; /* get rid of sprintf's \0s */

    sciprintf("%s\n", tempstr);
  }
  return 0;
}

static void
script_dump_object(char *data, int seeker, int objsize, char **snames, int snames_nr)
{
  int selectors, overloads, selectorsize;
  int species = getInt16((unsigned char *) data + 8 + seeker);
  int superclass = getInt16((unsigned char *) data + 10 + seeker);
  int namepos = getInt16((unsigned char *) data + 14 + seeker);
  int i = 0;

  sciprintf("Object\n");

  sci_hexdump((unsigned char *) data + seeker, objsize -4, seeker);
  /*-4 because the size includes the two-word header */

  sciprintf("Name: %s\n", namepos? ((char *)(data + namepos)) : "<unknown>");
  sciprintf("Superclass: %x\n", superclass);
  sciprintf("Species: %x\n", species);
  sciprintf("-info-:%x\n", getInt16((unsigned char *) data + 12 + seeker) & 0xffff);

  sciprintf("Function area offset: %x\n", getInt16((unsigned char *) data + seeker + 4));
  sciprintf("Selectors [%x]:\n",
	    selectors = (selectorsize = getInt16((unsigned char *) data + seeker + 6)));

  seeker += 8;

  while (selectors--) {
    sciprintf("  [#%03x] = 0x%x\n", i++, getInt16((unsigned char *) data + seeker) & 0xffff);

    seeker += 2;
  }


  sciprintf("Overridden functions: %x\n", selectors =
	    overloads = getInt16((unsigned char *) data + seeker));

  seeker += 2;

  if (overloads < 100)
  while (overloads--) {
    int selector = getInt16((unsigned char *) data + (seeker));

    sciprintf("  [%03x] %s: @", selector & 0xffff,
	      (snames && selector >= 0 && selector < snames_nr)? snames[selector] : "<?>");
    sciprintf("%04x\n", getInt16((unsigned char *) data + seeker + selectors*2 + 2) & 0xffff);

    seeker += 2;
  }
}

static void
script_dump_class(char *data, int seeker, int objsize, char **snames, int snames_nr)
{
  int selectors, overloads, selectorsize;
  int species = getInt16((unsigned char *) data + 8 + seeker);
  int superclass = getInt16((unsigned char *) data + 10 + seeker);
  int namepos = getInt16((unsigned char *) data + 14 + seeker);

  sciprintf("Class\n");

  sci_hexdump((unsigned char *) data + seeker, objsize -4, seeker);

  sciprintf("Name: %s\n", namepos? ((char *)data + namepos) : "<unknown>");
  sciprintf("Superclass: %x\n", superclass);
  sciprintf("Species: %x\n", species);
  sciprintf("-info-:%x\n", getInt16((unsigned char *) data + 12 + seeker) & 0xffff);

  sciprintf("Function area offset: %x\n", getInt16((unsigned char *) data + seeker + 4));
  sciprintf("Selectors [%x]:\n",
	    selectors = (selectorsize = getInt16((unsigned char *) data + seeker + 6)));

  seeker += 8;
  selectorsize <<= 1;

  while (selectors--) {
    int selector = getInt16((unsigned char *) data + (seeker) + selectorsize);

    sciprintf("  [%03x] %s = 0x%x\n", 0xffff & selector,
	      (snames && selector >= 0 && selector < snames_nr)? snames[selector] : "<?>",
	      getInt16((unsigned char *) data + seeker) & 0xffff);

    seeker += 2;
  }

  seeker += selectorsize;

  sciprintf("Overloaded functions: %x\n", selectors =
	    overloads = getInt16((unsigned char *) data + seeker));

  seeker += 2;

  while (overloads--) {
    int selector = getInt16((unsigned char *) data + (seeker));
    fprintf(stderr,"selector=%d; snames_nr =%d\n", selector, snames_nr);
    sciprintf("  [%03x] %s: @", selector & 0xffff,
	      (snames && selector >= 0 && selector < snames_nr)?
	      snames[selector] : "<?>");
    sciprintf("%04x\n", getInt16((unsigned char *) data + seeker + selectors*2 + 2) & 0xffff);

    seeker += 2;
  }
}


void
script_dissect(resource_mgr_t *resmgr, int res_no, char **snames, int snames_nr)
{
  int objectctr[11] = {0,0,0,0,0,0,0,0,0,0,0};
  unsigned int _seeker = 0;
  resource_t *script = scir_find_resource(resmgr, sci_script, res_no, 0);
  word_t **words;
  int word_count;

  if (!script) {
    sciprintf("Script not found!\n");
    return;
  }

  words=vocab_get_words (resmgr, &word_count);

  while (_seeker < script->size) {
    int objtype = getInt16(script->data + _seeker);
    int objsize;
    int seeker = _seeker + 4;

    if (!objtype) {
      sciprintf("End of script object (#0) encountered.\n");
      sciprintf("Classes: %i, Objects: %i, Export: %i,\n Var: %i (all base 10)",
		objectctr[6], objectctr[1], objectctr[7], objectctr[10]);
      /*vocabulary_free_snames(snames);*/
      vocab_free_words (words, word_count);
      return;
    }

    sciprintf("\n");

    objsize = getInt16(script->data + _seeker + 2);

    sciprintf("Obj type #%x, size 0x%x: ", objtype, objsize);

    _seeker += objsize;

    objectctr[objtype]++;

    switch (objtype) {
    case sci_obj_object:
      script_dump_object ((char *) script->data, seeker, objsize, snames, snames_nr);
      break;

    case sci_obj_code: {
      sciprintf("Code\n");
      sci_hexdump(script->data + seeker, objsize -4, seeker);
    };
    break;

    case 3: {
      sciprintf("<unknown>\n");
      sci_hexdump(script->data + seeker, objsize -4, seeker);
    };
    break;

    case sci_obj_said: {
      sciprintf("Said\n");
      sci_hexdump(script->data + seeker, objsize -4, seeker);

      sciprintf ("%04x: ", seeker);
      while (seeker < _seeker)
      {
        unsigned char nextitem=script->data [seeker++];
        if (nextitem == 0xFF)
          sciprintf ("\n%04x: ", seeker);
        else if (nextitem >= 0xF0)
        {
          switch (nextitem)
          {
          case 0xf0: sciprintf(", "); break;
          case 0xf1: sciprintf("& "); break;
          case 0xf2: sciprintf("/ "); break;
          case 0xf3: sciprintf("( "); break;
          case 0xf4: sciprintf(") "); break;
          case 0xf5: sciprintf("[ "); break;
          case 0xf6: sciprintf("] "); break;
          case 0xf7: sciprintf("# "); break;
          case 0xf8: sciprintf("< "); break;
          case 0xf9: sciprintf("> "); break;
          }
        }
        else {
          nextitem = nextitem << 8 | script->data [seeker++];
          sciprintf ("%s[%03x] ", vocab_get_any_group_word (nextitem, words, word_count), nextitem);
        }
      }
      sciprintf ("\n");
    }
    break;

    case sci_obj_strings: {
      sciprintf("Strings\n");
      while (script->data [seeker])
      {
        sciprintf ("%04x: %s\n", seeker, script->data+seeker);
        seeker += strlen ((char *) script->data+seeker)+1;
      }
      seeker++; /* the ending zero byte */
    };
    break;

    case sci_obj_class:
      script_dump_class ((char *) script->data, seeker, objsize, snames, snames_nr);
      break;

    case sci_obj_exports: {
      sciprintf("Exports\n");
      sci_hexdump((unsigned char *) script->data + seeker, objsize -4, seeker);
    };
    break;

    case sci_obj_pointers: {
      sciprintf("Pointers\n");
      sci_hexdump(script->data + seeker, objsize -4, seeker);
    };
    break;

    case 9: {
      sciprintf("<unknown>\n");
      sci_hexdump(script->data + seeker, objsize -4, seeker);
    };
    break;

    case sci_obj_localvars: {
      sciprintf("Local vars\n");
      sci_hexdump(script->data + seeker, objsize -4, seeker);
    };
    break;

    default:
      sciprintf("Unsupported!\n");
      return;
    }

  }

  sciprintf("Script ends without terminator\n");

  /*vocabulary_free_snames(snames);*/
}
