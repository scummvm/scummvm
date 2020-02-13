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

#include "glk/agt/agility.h"

namespace Glk {
namespace AGT {

void dbgprintf(const char *fmt, ...) {
	va_list vp;
	char buff[300];

	va_start(vp, fmt);
	vsprintf(buff, fmt, vp);
	va_end(vp);

	debugout(buff);
}


static void print_msg(descr_ptr dptr) {
	int j;
	descr_line *txt;

	txt = read_descr(dptr.start, dptr.size);
	if (txt != NULL) {
		for (j = 0; txt[j] != NULL; j++) {
			dbgprintf("\n");
			debugout(txt[j]);
		}
	}
	free_descr(txt);
}


static char *getname(int inum)
/* Name should be 20 chars or less */
{
	if (inum == 0) return rstrdup("* 0 *");
	return objname(inum);
}


extern integer dobj, iobj, actor;

void print_special_obj(int i)
/* This is called by the disassembler in agtdbg.c */
/* i=0 NOUN, 1 OBJECT, 2 NAME */
{
	int dval;
	char *s;
	switch (i) {
	case 0:
		dval = dobj;
		dbgprintf("NOUN");
		break;
	case 1:
		dval = iobj;
		dbgprintf("OBJECT");
		break;
	case 2:
		dval = actor;
		dbgprintf("NAME");
		break;
	default:
		dval = 0; /* Silence compiler warnings. */
		fatal("INTERNAL ERROR: Invalid *dval* in print_special_obj.");
	}
	if (dbgflagptr == NULL)
		/* This determines whether we are linked with agtout or agil */
		return;
	s = getname(dval);
	dbgprintf("(%d:%s)", dval, s);
	rfree(s);
}

#define printval(str,index,ptr) {dbgprintf("[%s%d",str,index);\
		if (ptr==NULL) dbgprintf("]");\
		else dbgprintf("=%ld]",(long)ptr[index]);}

int argout(int dtype, int dval, int optype) {
	char *s;

	if (dtype & AGT_VAR) dtype = AGT_VAR;

	if ((optype & 3) == 1) /* variable */
		dtype = AGT_VAR;
	if (optype & 2) { /* NOUN or OBJECT */
		if (dtype >= 64 && dtype != AGT_NUM)
			dbgprintf("ILL:");
		if (optype == 2)
			print_special_obj(0); /* NOUN */
		else
			print_special_obj(1);  /* OBJECT */
		return 0;
	}

	if (!interp_arg)
		dbgprintf("%d", dval);
	else {
		if (dtype < 64) {
			if (dval == -1)
				print_special_obj(2); /* NAME */
			else {
				s = getname(dval);
				if (dtype & (AGT_ITEM | AGT_CREAT | AGT_SELF | AGT_WORN))
					dbgprintf("<%d:%s>", dval, s);
				else
					dbgprintf("{%d:%s}", dval, s);
				rfree(s);
			}
		} else if ((dtype & AGT_VAR) != 0) {
			if (dval == -1)
				print_tos();
			else
				printval("Var", dval, dbgvarptr);
		} else switch (dtype) {
			case AGT_TIME:
				dbgprintf("%2d:%2d", dval / 100, dval % 100);
				break;
			case AGT_NUM:  /* Numeric */
				dbgprintf("%d", dval);
				break;
			case AGT_FLAG:  /* Flag */
				printval("Flg", dval, dbgflagptr);
				break;
			case AGT_ROOMFLAG: /* Roomflag */
				dbgprintf("RoomFlag%d", dval);
				break;
			case AGT_QUEST:  /* Question */
				if (dval <= MaxQuestion && dval >= 1 && question != NULL) {
					dbgprintf("\nQ%d:%s\n", dval, question[dval - 1]);
					dbgprintf("[A:%s]", answer[dval - 1]);
				} else if (quest_ptr != NULL) {
					dbgprintf("\nQ%d: ", dval);
					print_msg(quest_ptr[dval - 1]);
					dbgprintf("[A:");
					print_msg(ans_ptr[dval - 1]);
				}
				break;
			case AGT_MSG: /* Message */
				if (dval > last_message || dval < 1 || msg_ptr == NULL)
					dbgprintf("ILLEGAL MESSAGE");
				else {
					dbgprintf("(Msg%d)", dval);
					if (!dbg_nomsg)
						print_msg(msg_ptr[dval - 1]);
				}
				break;
			case AGT_ERR: /* Message */
				if (dval > NUM_ERR || dval < 1 || err_ptr == NULL)
					dbgprintf("ILLEGAL MESSAGE");
				else {
					dbgprintf("(Std%d)", dval);
					if (!dbg_nomsg)
						print_msg(err_ptr[dval - 1]);
				}
				break;
			case AGT_STR: /* String */
				if (dval - 1 >= MAX_USTR || userstr == NULL)
					dbgprintf("ILLEGAL STRING");
				else
					dbgprintf("\nStr%d:%s", dval, userstr[dval]);
				break;
			case AGT_CNT: /* Counter */
				printval("Cnt", dval, dbgcntptr);
				break;
			case AGT_DIR: /* Direction */
				if (dval >= 1 && dval <= 13)
					dbgprintf("%s", exitname[dval - 1]);
				else dbgprintf("ILL_DIR(%d)", dval);
				break;
			case AGT_SUB: /* Subroutine */
				dbgprintf("Subroutine %d", dval);
				break;
			case AGT_PIC: /* Picture */
			case AGT_PIX:
				dbgprintf("Picture #%d", dval);
				break;
			case AGT_FONT: /* Font */
				dbgprintf("Font #%d", dval);
				break;
			case AGT_SONG:  /* Song */
				dbgprintf("Song #%d", dval);
				break;
			case AGT_OBJFLAG:
				dbgprintf("ObjFlag%d", dval);
				break;
			case AGT_OBJPROP:
				dbgprintf("ObjProp%d", dval);
				break;
			case AGT_ATTR:
				if (dval < 0 || dval >= NUM_ATTR)
					dbgprintf("UnkownAttr%d", dval);
				else
					dbgprintf("%s", attrlist[dval].name);
				break;
			case AGT_PROP:
				if (dval < 0 || dval >= NUM_PROP)
					dbgprintf("UnknownProp%d", dval);
				else
					dbgprintf("%s", proplist[dval].name);
				break;
			case AGT_EXIT:
				if (dval >= exitmsg_base)
					argout(AGT_MSG, dval - exitmsg_base, 0);
				else
					argout(AGT_ROOM, dval, 0);
				break;
			default:
				dbgprintf("?+%d", dval);
			}
	}
	return 1;
}


void debug_newline(integer op, rbool first_nl) {
	rbool early_nl;

	if (!dbg_nomsg) return;
	early_nl = (op == 1008 || op == 1027 || op == 1083 || op == 1105
	            || (op >= 1126 && op <= 1131));
	if (early_nl == first_nl)
		debugout("\n");
}


void debug_cmd_out(int ip, integer op, int arg1, int arg2, int optype) {
	int j;
	const opdef *opdata;
	rbool save_dbg_nomsg;

	dbgprintf("  %2d:", ip);
	save_dbg_nomsg = 0; /* Just to silence compiler warnings. */

	opdata = get_opdef(op);
	if (opdata == &illegal_def)
		dbgprintf("ILLEGAL %d\n", op);
	else {
		if (op >= END_ACT) dbgprintf("!"); /* "Terminal" Actions */
		else if (op <= MAX_COND) dbgprintf("?"); /* Condition */
		if (op == 1063) { /* RandomMessage needs special handling */
			save_dbg_nomsg = dbg_nomsg;
			dbg_nomsg = 1;
		}
		dbgprintf("%s", opdata->opcode);
		for (j = 0; j < opdata->argnum; j++) {
			dbgprintf("\t");
			argout(j == 0 ? opdata->arg1 : opdata->arg2 , j == 0 ? arg1 : arg2,
			       optype >> 2);
			optype <<= 2;
		}
		if (op == 1063)
			dbg_nomsg = save_dbg_nomsg;
	}
	debug_newline(op, 1);
}


void debug_head(int i) {
	int v, w, a;

	v = verb_code(command[i].verbcmd);
	if (v >= BASE_VERB && v < BASE_VERB + DUMB_VERB && syntbl[synlist[v]] != 0)
		w = syntbl[synlist[v]];
	else w = command[i].verbcmd;
	if (command[i].actor > 0) {
		dbgprintf("CMD %d: ", i);
		a = command[i].actor;
	} else {
		dbgprintf("REDIR: ");
		a = -command[i].actor;
	}

	if (a == 2)
		dbgprintf("anybody, ");
	else if (a > 2) {
		char *name;
		name = objname(a);
		name[0] = toupper(name[0]);
		dbgprintf("%s, ", name);
		rfree(name);
	}

	dbgprintf("%s ", w == 0 ? "any" : dict[w]);
	if (command[i].noun_adj != 0)
		dbgprintf("%s ", gdict(command[i].noun_adj));
	dbgprintf("%s %s ", gdict(command[i].nouncmd),
	          (ver == 3) ? gdict(command[i].prep) : "->");
	if (command[i].obj_adj != 0)
		dbgprintf("%s ", gdict(command[i].obj_adj));
	dbgprintf("%s\n", gdict(command[i].objcmd));

}

} // End of namespace AGT
} // End of namespace Glk
