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
#include "glk/agt/interp.h"
#include "glk/agt/exec.h"

namespace Glk {
namespace AGT {

/*

   This contains the code for actually executing each of the
   individual metacommand tokens as well as the routines for
   doing range checking to prevent a bad metacommand from crashing
   the interpreter.

*/

static void it_newdesc(integer item, descr_ptr *newdesc) {
	descr_ptr *desc;

	if (tnoun(item)) desc = noun_ptr + (item - first_noun);
	else if (tcreat(item)) desc = creat_ptr + (item - first_creat);
	else if (item >= first_room && item <= maxroom)
		desc = room_ptr + (item - first_room);
	else {
		writeln("INTERNAL ERROR: it_newdesc called with invalid object");
		return;
	}
	desc->start = newdesc->start;
	desc->size = newdesc->size;
}

static void changepict(int old_pict, int new_pict)
/* Replace old_pict by new_pict */
{
	pictable[old_pict - 1] = new_pict - 1;
}


static const int antidir[12] = {1, 0, 3, 2, 7, 6, 5, 4, 9, 8, 11, 10};

static void change_passage(int start, int dir, int newend)
/* dir is 1..12 */
/* START is a room number: i.e. starting at 0. */
/* NEWEND is an object number, so starts at first_room */
{
	int oldend, i;

	dir--;  /* Convert direction to 0..11 scale */
	oldend = room[start].path[dir];
	room[start].path[dir] = newend;
	if (newend == 0) {
		if (room[oldend - first_room].path[antidir[dir]] == start + first_room)
			room[oldend - first_room].path[antidir[dir]] = 0;
		else for (i = 0; i < 12; i++)
				if (room[oldend - first_room].path[i] == start + first_room) {
					room[oldend - first_room].path[i] = 0;
					break;
				}
	} else
		room[newend - first_room].path[antidir[dir]] = start + first_room;
}


static long ask_for_number(int n1, int n2) {
	char s[50];
	int n;

	if (n1 != n2)
		sprintf(s, "Enter a number from %d to %d: ", n1, n2);
	else
		sprintf(s, "Enter a number: ");
	for (;;) {
		writestr(s);
		n = read_number();
		if (n1 == n2 || (n >= n1 && n <= n2)) return n;
		writeln("");
	}
}

/* Check if n1*n2 will fit in 32 bits and print an error message if not */
/* This errs on the side of caution */
static rbool mult_rangecheck(long n1, long n2) {
	int cnt;

	if (n1 == 0 || n2 == 0) return 1;
	if (n1 < 0) n1 = -n1;
	if (n2 < 0) n2 = -n2;

	for (cnt = 0; n1 != 0; n1 >>= 1, cnt++);
	for (; n2 != 0; n2 >>= 1, cnt++);
	cnt--;

	if (cnt <= 31) return 1; /* We're okay */

	if (!PURE_ERROR)
		writeln("GAME ERROR: Multiplication out of range.");
	return 0;
}

static rbool is_numeric(parse_rec *objrec) {
	char *s;

	if (objrec->num != 0 || objrec->info == D_NUM) return 1;
	if (objrec->adj != 0) return 0;
	if (objrec->noun <= 0) return 0;
	(void)strtol(dict[objrec->noun], &s, 10);
	return (*s == 0); /* *s==0 means no error-- it parsed as a number. */
}

static void setcase(char *s, rbool up) {
	for (; *s != 0; s++)
		if (up) *s = toupper(*s);
		else *s = tolower(*s);
}


void move_in_dir(int obj, int dir) {
	int r;

	r = it_room(obj);
	if (!troom(r)) {
		writeln("GAME ERROR: Object not in a room.");
		return;
	}
	r = room[r - first_room].path[dir - 1];
	if (!troom(r)) return; /* Can't go that way; Fail silently */
	if (obj == 1)
		goto_room(r);
	else
		it_move(obj, r);
}


/* ------------------------------------------------------------------- */
/*  Stack routines:   Manipulating the expression stack                */
/* ------------------------------------------------------------------- */

static long *stack = NULL;
static int sp = 0; /* Stack pointer */
static int stacksize = 0; /* Actual space allocated to the stack */

void init_stack(void) {
	rfree(stack);
	sp = 0;
	stacksize = 0;
}

/* This resets the stack to an empty state. */
void clear_stack(void) {
	sp = 0;
}

static void push_stack(long val) {
	sp++;
	if (sp > stacksize) {
		stacksize += 10;
		stack = (long *)rrealloc(stack, stacksize * sizeof(long));
	}
	stack[sp - 1] = val;
}

static long pop_stack(void) {
	long n;
	if (sp == 0) {
		writeln("GAME ERROR: Stack underflow.");
		return 0;
	}
	n = stack[--sp];
	if (sp + 100 < stacksize) {
		stacksize -= 50;
		stack = (long *)rrealloc(stack, stacksize * sizeof(long));
	}
	return n;
}

long pop_expr_stack(void) {
	return pop_stack();
}


/* opnum: 0=+, 1=-, 2=*, 3=/ 4=% */
static void op_stack(int opnum) {
	long n1, n2;
	n1 = pop_stack();
	n2 = pop_stack();
	switch (opnum) {
	case 0:
		n1 = n1 + n2;
		break;
	case 1:
		n1 = n1 - n2;
		break;
	case 2:
		if (mult_rangecheck(n1, n2)) n1 = n1 * n2;
		break;
	case 3:
		if (n2 != 0) n1 = n1 / n2;
		else writeln("GAME ERROR: Division by zero.");
		break;
	case 4:
		if (n2 != 0) n1 = n1 % n2;
		else writeln("GAME ERROR: Division by zero.");
		break;
	default:
		writeln("INTERNAL ERROR: Invalid stack operation.");
	}
	push_stack(n1);
}

/* This is called from the disassembler */
void print_tos(void) {
	if (sp > 0)
		dbgprintf("TOS(%ld)", stack[sp - 1]);
	else
		debugout("TOS(xxx)");
}



/* ------------------------------------------------------------------- */
/* METACOMMAND ROUTINES  */
/*  Functions for scanning and decoding of metacommands */
/* ------------------------------------------------------------------- */

/* #define cret(b) return ((b) ? -1 : 0)*/
#define cret(b) return (b)
#define cretn(i,f) cret(tnoun(i) && noun[i-first_noun].f)
#define cretc(i,f) cret(tcreat(i) && creature[i-first_creat].f)
#define icretc(f) cret(do_disambig==1 || \
                       (tcreat(iobj) && creature[iobj-first_creat].f))

static int obj_cond(int op_, int obj, int arg) {
	switch (op_) {
	case 0:
		cret(in_scope(obj));  /* Present--
                      Do we want to use visible here?? */
	case 1:
		cret(is_within(obj, 1000, 1)); /* IsWearing */
	case 2:
		cret(is_within(obj, 1, 1));
	/* if (PURE_WEAR)  return (it_loc(obj)==1); else */
	case 3:
		cret(it_loc(obj) == 0); /* Nowhere */
	case 4:
		cret(it_loc(obj) != 0);
	case 5:
		cret(!player_has(obj) && in_scope(obj));
	case 6:
		cret(it_loc(obj) == arg);
	case 7:
		cret(it_on(obj));
	case 8:
		cret(!it_on(obj));
	case 9:
		cret(it_open(obj));
	case 10:
		cret(!it_open(obj));
	case 11:
		cretn(obj, locked);
	case 12:
		cret(!tnoun(obj) || !noun[obj - first_noun].locked);
	case 13:
		cretn(obj, edible);
	case 14:
		cretn(obj, drinkable);
	case 15:
		cretn(obj, poisonous);
	case 16:
		cretn(obj, movable);
	default:
		writeln("INTERNAL ERROR: Bad obj_cond value.");
		return 2;
	}
}


static int exec_cond(int op_, int arg1, int arg2) {
	int i;

	switch (op_) {
	/* First the conditions */
	case 0:
		cret(loc + first_room == arg1); /* AtLoc(Room) */
	case 1:
		cret(loc + first_room > arg1);
	case 2:
		cret(loc + first_room < arg1);
	case 3:
		return musiccmd(-1, -1); /* SongPlaying */
	case 4:
		return musiccmd(-2, -1); /* SoundIsOn */
	case 5:
		cret(vb <= 13 && vb > 0 &&
		     room[loc].path[vb - 1] >= first_room); /*DirOK*/
	case 6:
		cret(vb == arg1); /* DirectionIs */
	case 7:
		cret(loc + first_room >= arg1 &&
		     loc + first_room <= arg2); /* BetweenRooms  ?? */
	case 8:
		cret(room[arg1 - first_room].seen);
	case 9:  /* Entered Object? i.e. is iobj valid */
		cret(do_disambig == 1 || iobj > 0);
	case 10:
		cret(curr_time > arg1); /* TimeGT */
	case 11:
		cret(curr_time < arg1); /* TimeLT */
	case 12:
		cret(first_visit_flag);
	case 13:
		cret(newlife_flag);
	case 14:
		cret(player_contents != 0); /* CarrySome */
	case 15:
		cret(player_contents == 0); /* CarryNo */
	case 16:
		cret(player_worn != 0); /* WearSome */
	case 18:
		cret(player_worn == 0); /* WearNo */
	case 17:          /* CarryTreas */
		contloop(i, 1)
		if (tnoun(i) && noun[i - first_noun].points >= arg1) return 1;
		contloop(i, 1000)
		if (tnoun(i) && noun[i - first_noun].points >= arg1) return 1;
		return 0;
	case 19:
		cret(totwt == arg1);
	case 20:
		cret(totwt > arg1);
	case 21:
		cret(totwt < arg1);
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
		return obj_cond(op_ - 22, arg1, arg2);
	case 29:
		cret(it_loc(arg1) == it_loc(arg2));
	case 30:
	case 31:
		return obj_cond(op_ - 23, arg1, arg2);
	case 32:
		cret(it_group(arg1));
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
	case 38:
	case 39:
	case 40:
		return obj_cond(op_ - 24, arg1, arg2);
	case 41:
	case 42:
	case 43:
	case 44:
	case 45:
	case 46:
	case 47:
	case 48:
	case 49:
	case 50:
	case 51:
	case 52:
	case 53:
	case 54:
	case 55:
	case 56:
	case 57:
		return obj_cond(op_ - 41, dobj, arg1);
	case 58:
		cretn(dobj, points == arg1);
	case 59:
		cretn(dobj, points > arg1);
	case 60:
		cretn(dobj, points < arg1);
	case 61:
		cretn(dobj, weight == arg1);
	case 62:
		cretn(dobj, weight > arg1);
	case 63:
		cretn(dobj, weight < arg1);
	case 64:
		cret(islit());
	case 65:
		cret(room[loc].light != 0);
	case 66:
		cret(flag[arg1]);
	case 67:
		cret(!flag[arg1]);
	case 68:
		cret(room[loc].flag_noun_bits & (1 << (arg1 - 1)));
	case 70:
		cret(!(room[loc].flag_noun_bits & (1 << (arg1 - 1))));
	case 69:
		cret(room[loc].PIX_bits & (1 << (arg1 - 1))); /* Room Pix here? */
	case 71:
		cret(tscore == arg1);
	case 72:
		cret(tscore > arg1);
	case 73:
		cret(tscore < arg1);
	case 74:
		cret(agt_number == arg1);
	case 75:
		cret(agt_number > arg1);
	case 76:
		cret(agt_number < arg1);
	case 77:
		cret(agt_answer);
	case 78:
		cret(!agt_answer);
	case 79:
		cret(turncnt == arg1);
	case 80:
		cret(turncnt > arg1);
	case 81:
		cret(turncnt < arg1);
	case 82:
		cret(cnt_val(agt_counter[arg1]) == arg2);
	case 83:
		cret(cnt_val(agt_counter[arg1]) > arg2);
	case 84:
		cret(cnt_val(agt_counter[arg1]) < arg2);

	case 85:
		cret(agt_var[arg1] == arg2);
	case 86:
		cret(agt_var[arg1] > arg2);
	case 87:
		cret(agt_var[arg1] < arg2);
	case 88:
		cret(agt_var[arg1] < agt_var[arg2]);
	case 89:
		cret(agt_var[arg1] < agt_rand(1, arg2));
	case 90:
		cret((actor != 0) && (it_loc(actor) == loc + first_room));
	case 91:
		cret(actor == arg1);
	case 92:
		cret(dobj == arg1);
	case 93:
		cret(do_disambig == 1 || iobj == arg1);
	case 94:
		cret(it_contents(arg1) != 0);
	case 95:
		cret(agt_rand(1, 100) <= arg1);
	case 96:
		cret(yesno("Yes or no? "));
	case 97:
		cret(!yesno("Yes or no? "));
	case 98:
		cret(vb > 0 && vb <= 13);
	case 99:
		cret(tcreat(dobj));
	case 100:
		cretc(dobj, gender == 2); /* man */
	case 101:
		cretc(dobj, gender == 1); /* woman */
	case 102:
		cretc(dobj, gender == 0); /* thing */
	case 103:
		cretc(iobj, gender == 2);
	case 104:
		cretc(iobj, gender == 1); /* woman */
	case 105:
		cretc(iobj, gender == 0); /* thing */
	case 106:
		cret(do_disambig == 1 || tcreat(iobj));
	case 107:
		return (do_disambig == 1 || obj_cond(0, iobj, 0));
	/* OR and NOT are handled higher up. */
	/* The following are all v1.8x metacommands */
	case 110:
		cret(beforecmd);
	case 111:
		cret(!beforecmd);
	case 112:
		cret(curr_time / 100 == arg1); /* HoursEqual */
	case 113:
		cret(curr_time / 100 > arg1);
	case 114:
		cret(curr_time / 100 < arg1);
	case 115:
		cret(curr_time % 100 == arg1); /* MinutesEqual */
	case 116:
		cret(curr_time % 100 > arg1);
	case 117:
		cret(curr_time % 100 < arg1);
	case 118:
		cret(curr_time < 1200);  /* IsAM */

	case 119:
		cret(do_disambig);     /* OnDisambig */
	case 120:
		cretc(arg1, hostile);   /* IsHostile */
	case 121:         /* HostilePresent */
		creatloop(i)
		if (creature[i].location == loc + first_room &&
		        creature[i].hostile) return 1;
		return 0;
	/* Otherwise, we're in trouble. */
	case 122:
		cret(actor_in_scope); /* NameWasPresent */
	case 123: /* OncePerTurn */
		if (beforecmd)
			cret(start_of_turn);
		else
			cret(end_of_turn);
	case 124:  /* IsClass */
		cret(arg2 == 0 || matchclass(arg1, arg2));
	case 125:
		cret(getattr(arg1, arg2)); /* IsSet */
	case 126:
		cret(is_numeric(dobj_rec));
	case 127:
		cret(is_numeric(iobj_rec));
	case 128:
		cret(arg1 == arg2);
	case 129:
		cret(arg1 > arg2);
	case 130:
		cret(arg1 < arg2);
	case 131:
		cret(arg1 >= arg2);
	case 132:
		cret(arg1 <= arg2);
	case 133:
		cret(strcmp(userstr[arg1 - 1], userstr[arg2 - 1]) == 0);
	case 134:
		cret(strcmp(userstr[arg1 - 1], userstr[arg2 - 1]) < 0);
	case 135:
		cret(strcmp(userstr[arg1 - 1], userstr[arg2 - 1]) > 0);
	case 136:
		cret(strcasecmp(userstr[arg1 - 1], userstr[arg2 - 1]) == 0);
	case 137:
		cret(strcasecmp(userstr[arg1 - 1], userstr[arg2 - 1]) < 0);
	case 138:
		cret(strcasecmp(userstr[arg1 - 1], userstr[arg2 - 1]) > 0);
	case 139:
		cret(match_answer(rstrdup(userstr[arg1 - 1]), arg2 - 1));
	/* Note that match_answer rfrees it's first argument */
	case 140:
		cret(it_seen(arg1));
	case 141:
		cret(op_objflag(2, arg1, arg2));
	case 142:
		cret(!op_objflag(2, arg1, arg2));
	case 143:
		i = it_room(arg1);
		cret(troom(i) && troom(room[i - first_room].path[arg2 - 1]));
	default:
		writeln("INTERNAL ERROR: Condition token not supported.");
		rprintf("Condition #%d", op_);
		writeln("");
		return 0;
	}
}

#undef cret
#undef cretn
#undef cretc


static void obj_act(int op_, int obj) {
	switch (op_) {
	case 0:
	case 1:  /* open and close */
		if (tnoun(obj))
			noun[obj - first_noun].open = (op_ == 0);
		break;
	case 2:
	case 3: /* lock and unlock */
		if (tnoun(obj))
			noun[obj - first_noun].locked = (op_ == 2);
		break;
	default:
		break;
	}
}


static void exec_action(int op_, int arg1, int arg2) {
	int i, j;
	char *tmpstr;

	switch (op_) {
	case 1000:
		goto_room(arg1 - first_room);
		break;
	case 1001:
		goto_room(agt_rand(arg1, arg2) - first_room);
		break;
	case 1002:
		agt_var[arg1] = loc + first_room;
		break;
	case 1003:
		agt_var[arg1] = dobj;
		break;
	case 1004:
		agt_var[arg1] = iobj;
		break;
	case 1005:
		goto_room(agt_var[arg1] - first_room);
		break;
	case 1006:
		it_move(arg1, agt_var[arg2]);
		break;
	case 1007:
		get_obj(agt_var[arg1]);
		break;
	case 1008:
		msgout(agt_var[arg1], 1);
		break;
	case 1009:
		get_obj(arg1);
		break;
	case 1010:
		get_obj(arg1);
		it_move(arg1, 1000);
		break;
	case 1011:
		drop_obj(arg1);
		break;
	case 1012:
		if (it_loc(arg1) == 1000) {
			if (PURE_WEAR) drop_obj(arg1);
			else it_move(arg1, 1);
		}
		break;
	case 1013:
		fontcmd(0, arg1 - 1);
		break;  /* Load font */
	case 1014:
		pictcmd(1, pictable[arg1 - 1]);
		break;  /* Show picture */
	case 1015:
		changepict(arg1, arg2);
		break; /* ChangePicture */
	case 1016:
		if (PICT_SUPPORT &&
		        yesno("Would you like to see the picture?"))
			pictcmd(1, pictable[arg1 - 1]);
		break;
	case 1017:
		pictcmd(2, arg1);
		break;  /* Show room pix */
	case 1018:
		if (PICT_SUPPORT &&
		        yesno("Would you like to see the picture?"))
			pictcmd(2, arg1 - 1);
		break;
	case 1019:
		musiccmd(1, arg1 - 1);
		break;
	case 1020:
		musiccmd(1, agt_rand(arg1, arg2) - 1);
		break;
	case 1021:
		musiccmd(2, arg1 - 1);
		break;
	case 1022:
		musiccmd(3, -1);
		break;  /* Stop Repeat */
	case 1023:
		musiccmd(4, -1);
		break;  /* Stop song */
	case 1024:
		musiccmd(5, -1);
		break;  /* Suspend song */
	case 1025:
		musiccmd(6, -1);
		break;   /* Resume song */
	case 1026:
		if (tnoun(dobj))
			noun[dobj - first_noun].movable = !noun[dobj - first_noun].movable;
		break;
	case 1027:
		it_newdesc(arg1, &msg_ptr[arg2 - 1]);
		break;
	case 1028:
		if (tnoun(arg1)) noun[arg1 - first_noun].points = arg2;
		else if (tcreat(arg1)) creature[arg1 - first_creat].points = arg2;
		else if (troom(arg1)) room[arg1 - first_room].points = arg2;
		break;
	case 1029:
		it_destroy(iobj);
		break;
	case 1030:
		tmpstr = agt_readline(3);
		i = strlen(tmpstr) - 1;
		if (i > 0 && tmpstr[i] == '\n') tmpstr[i] = 0;
		strncpy(userstr[arg1 - 1], tmpstr, 80);
		rfree(tmpstr);
		break;
	case 1031:
		agt_var[arg1] = read_number();
		break;
	case 1032:
		agt_var[arg1] = curr_time;
		break;
	case 1033:
		curr_time = normalize_time(agt_var[arg1]);
		break;
	case 1034:
		curr_time = normalize_time(arg1);
		break;
	case 1035:
		add_time(arg1);
		break;
	case 1036:
		delta_time = arg1;
		break;
	/* 1037 and 1038 are subroutine commands */
	case 1039:
		get_obj(dobj);
		break;
	case 1040:
		it_move(dobj, 1000);
		break;
	case 1041:
		drop_obj(dobj);
		break;
	case 1042:
		if (it_loc(dobj) == 1000) {
			if (PURE_WEAR) it_move(dobj, 1);
			else drop_obj(dobj);
		}
		break;
	case 1043: /* drop all */
		safecontloop(i, j, 1) drop_obj(i);
		break;
	case 1044: /* remove all */
		safecontloop(i, j, 1000) drop_obj(i);
		break;
	case 1045:
		deadflag = 1;
		break;
	case 1046:
		it_move(arg1, loc + first_room);
		break;
	case 1047:
		it_move(arg1, arg2);
		break;
	case 1048:
		it_reposition(arg1, arg2, 1);
		break; /* RePosition */
	case 1049:
		it_move(dobj, loc + first_room);
		break;
	case 1050:
		it_move(dobj, arg1);
		break;
	case 1051:
		safecontloop(i, j, 1) it_move(i, arg1);
		safecontloop(i, j, 1000) it_move(i, arg1);
		break;
	case 1052:
		nounloop(i)
		if (player_has(i + first_noun) && noun[i].points > arg2)
			it_move(i + first_noun, arg1);
		break;
	case 1053:
		safecontloop(i, j, arg1)
		if (tnoun(i)) it_move(i, arg2);
		break;
	case 1054:
		it_destroy(arg1);
		break;
	case 1055:
		it_destroy(dobj);
		break;
	case 1056:
		i = it_loc(arg1);
		it_move(arg1, it_loc(arg2));
		it_move(arg2, i);
		break;
	case 1057:
		it_move(arg1, it_loc(arg2));
		break;
	case 1058:
		it_move(dobj, it_loc(arg2));
		break;
	case 1059:
	case 1060: /* Add to/remove from group */
		if (tcreat(arg1))
			creature[arg1 - first_creat].groupmemb = (op_ == 1059);
		break;
	case 1061:   /* Move group */
		safecontloop(i, j, loc + first_room)
		if (it_group(i)) it_move(i, arg1);
		break;
	/* 1062 is RedirectTo */
	case 1063:
		msgout(agt_rand(arg1, arg2), 1);
		break;
	case 1064:
		print_contents(arg1, 1);
		break;
	case 1065:
	case 1066:
	case 1067:
	case 1068:
		obj_act(op_ - 1065, arg1);
		break;
	case 1069:
	case 1070:
	case 1071:
	case 1072:
		obj_act(op_ - 1069, dobj);
		break;
	case 1073:
		print_score();
		break;
	case 1074:
		tscore += arg1;
		break;
	case 1075:
		tscore -= arg1;
		break;
	case 1076:
		v_inventory();
		break;
	case 1077:
		wait_return();
		break;
	case 1078:
		writeln("Time passes...");
		break;
	case 1079:
		agt_delay(arg1);
		break;
	case 1080:
		agt_clrscr();
		break;
	case 1081:
		it_describe(arg1);
		break;
	case 1082:
		look_room();
		break;   /* LOOK */
	case 1083:
		msgout(arg1, 1);
		break;
	case 1084:
		writeln("");
		break;
	case 1085:
		if (PURE_TONE && sound_on)
			agt_tone(arg1, arg2);
		break; /* Tone */
	case 1086:
		agt_number = ask_for_number(arg1, arg2);
		break;
	case 1087:
		agt_answer = ask_question(arg1);
		break;
	case 1088:
		change_passage(loc, arg1, arg2);
		break;
	case 1089:
		flag[arg1] = 1;
		break;
	case 1090:
		flag[arg1] = 0;
		break;
	case 1091:
		flag[arg1] = !flag[arg1];
		break;
	case 1092:
		room[loc].flag_noun_bits |= (1 << (arg1 - 1));
		break; /* Roomflag on */
	case 1093:
		room[loc].flag_noun_bits &= ~(1 << (arg1 - 1));
		break; /* Off */
	case 1094:
		room[loc].flag_noun_bits ^= (1 << (arg1 - 1));
		break; /* Toggle */
	case 1095: /* if (agt_counter[arg1]==-1)*/
		agt_counter[arg1] = 1;
		break;
	case 1096:
		agt_counter[arg1] = -1;
		break;
	case 1097:
		agt_var[arg1] = arg2;
		break;
	case 1098:
		agt_var[arg1] += arg2;
		break;
	case 1099:
		agt_var[arg1] -= arg2;
		break;
	case 1100:
		agt_var[arg1] += agt_var[arg2];
		break;
	case 1101:
		agt_var[arg1] -= agt_var[arg2];
		break;
	case 1102:
		agt_var[arg1] = agt_rand(0, arg2);
		break;
	case 1103:
		agt_var[arg1] = dobj_rec->num;
		break;
	case 1104:
		agt_var[arg1] = iobj_rec->num;
		break;

	/* The following are v1.8x specific */
	case 1105:
		quote(arg1);
		break;
	case 1106:
		add_time(arg1);
		break;
	case 1107:
		add_time(-arg1);
		break;
	case 1108:
		curr_time = (curr_time % 100) + 100 * arg1;
		break;
	case 1109:
		curr_time = (curr_time / 100) * 100 + arg1;
		break;
	case 1110:
		add_time(agt_var[arg1]);
		break;
	case 1111:
		add_time(-agt_var[arg1]);
		break;
	case 1112:
		curr_time = (curr_time % 100) + 100 * agt_var[arg1];
		break;
	case 1113:
		curr_time = (curr_time / 100) * 100 + agt_var[arg1];
		break;

	/* Now for the AGX additions */
	case 1114:
		add_time(-arg1);
		break; /* ME-style SubtractFromTime */
	case 1115:
		disambig_score = arg1;
		break; /* SetDisambigPriority */
	case 1116:
		agt_var[arg1] = delta_time;
		break;
	case 1117: /* ChangeStatus */
		statusmode = arg1;
		break;
	case 1118:
		if (!mult_rangecheck(agt_var[arg1], arg2)) break;
		agt_var[arg1] *= arg2;
		break;
	case 1119:
		if (arg2 == 0) {
			if (!PURE_ERROR)
				writeln("GAME ERROR: Division by zero.");
		} else agt_var[arg1] /= arg2;
		break;
	case 1120:
		if (arg2 == 0) {
			if (!PURE_ERROR)
				writeln("GAME ERROR: Attempt to divide by zero.");
		} else agt_var[arg1] %= arg2;
		break;
	case 1121:
		agt_waitkey();
		break;
	case 1122:
		last_he = arg1;
		break;   /* SetHE */
	case 1123:
		last_she = arg1;
		break;
	case 1124:
		last_it = arg1;
		break;
	case 1125:
		last_they = arg1;
		break;
	case 1126:
		msgout(arg1, 0);
		break;
	case 1127:
		if (!PURE_ERROR)
			sysmsg(arg1, "GAME ERROR: Standard message not defined.");
		break;
	case 1128:
		msgout(arg1, 1);
		break; /* FailMessage */
	case 1129:  /* StdMessage */
		sysmsg(arg1, "GAME ERROR: Standard message not defined.");
		break;
	case 1130:
		msgout(arg2, 1);
		break; /* ErrMessage */
	case 1131: /* StdErrMessage */
		sysmsg(arg1, "GAME ERROR: Standard message not defined.");
		break;
	case 1132: /* AND */
		break;  /* These don't do anything under normal circumstances */
	case 1133:  /* SetClass */
		if (troom(arg1)) room[arg1 - first_room].oclass = arg2;
		else if (tnoun(arg1)) noun[arg1 - first_noun].oclass = arg2;
		else if (tcreat(arg1)) noun[arg1 - first_creat].oclass = arg2;
		break;
	case 1134:
		agt_var[arg1] = it_class(arg2);
		break; /* SetVariableToClass */

	/* Stack commands */
	case 1135:
		push_stack(arg1);
		break;
	case 1136:
		agt_var[arg1] = pop_stack();
		break;
	case 1137:
	case 1138:
	case 1139:
	case 1140:
	case 1141:
		op_stack(op_ - 1137); /* +,-,*, /,% * */
		break;
	case 1142: { /* DupStack */
		long n;
		n = pop_stack();
		push_stack(n);
		push_stack(n);
		break;
	}
	case 1143:
		pop_stack();
		break; /* Discard TOS */
	case 1144:
		agt_var[arg1] = agt_number;
		break; /* SetVariableToInput */
	case 1145:
		setattr(arg1, arg2, 1);
		break; /* Set */
	case 1146:
		setattr(arg1, arg2, 0);
		break; /* Clear */
	case 1147:
		push_stack(getprop(arg1, arg2));
		break; /* PushProp */
	case 1148:
		setprop(arg1, arg2, pop_stack());
		break; /* PopProp */
	/* 1149, 1150 handled by run_metacommand */
	/* 1151 is EndDisambig */
	/* 1152 is XRedirect */
	case 1153:
		rstrncpy(userstr[arg1 - 1], userstr[arg2 - 1], 81);
		break;
	case 1154:
		setcase(userstr[arg1 - 1], 1);
		break;
	case 1155:
		setcase(userstr[arg1 - 1], 0);
		break;
	case 1156:
		op_objflag(1, arg1, arg2);
		break;
	case 1157:
		op_objflag(0, arg1, arg2);
		break;
	case 1158:
		op_objflag(3, arg1, arg2);
		break;
	case 1159:
		push_stack(op_objprop(2, arg1, arg2, 0));
		break;
	case 1160:
		op_objprop(1, arg1, arg2, pop_stack());
		break;
	case 1161:
		move_in_dir(arg1, arg2);
		break;
	default:
		writeln("INTERNAL ERROR: Action token not supported.");
		rprintf("Action #%d", op_);
		writeln("");
	}
}

int exec_instr(op_rec *oprec)
/* This routine is responsible for executing all conditions and action
   tokens. Remember action tokens are numbered from 1000. */
/* Return codes:
   0 to continue running commands
   1 for a failed conditional token
   100 Next metacommand
   101 Stop running metacommands
   102 End turn
   103 Redirection */
{
	rbool r;

	if (oprec->op < 1000) {
		r = exec_cond(oprec->op, oprec->arg1, oprec->arg2);
		return (oprec->negate ? r : !r); /* Is it prefixed by NOT? */
	}
	switch (oprec->op) {
	case 1151: /* EndDisambig <num> */
		if (do_disambig) {
			disambig_score = oprec->arg1;
			return 102;  /* "End turn" if disambiguating */
		}
		return 0;  /* ... otherwise do nothing */
	case 1062:
	case 1152:
		return 103; /* Redirect */
	case WIN_ACT:
		winflag = 1;
		return 0;  /* win game */
	case (WIN_ACT+1):
		endflag = 1;
		return 102; /* end game */
	case (WIN_ACT+2):
		return 100; /* end this command */
	case (WIN_ACT+3):
		return 101; /* end all commands */
	case (WIN_ACT+4):
		return 102; /* end turn */
	default:
		exec_action(oprec->op, oprec->arg1, oprec->arg2);
		if (oprec->failmsg) return 102;
		else return 0;
	}
}

} // End of namespace AGT
} // End of namespace Glk
