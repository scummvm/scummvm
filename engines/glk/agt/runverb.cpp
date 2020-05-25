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

   This file contains several things:
   i) The code for each of the built-in verbs, all prefixed with 'v_'
      (so, for example, the code for DROP is in v_drop()).
   ii) The main routine for checking and running player commands.
   iii) The main routine for doing intelligent disambiguation.

   */


/* ------------------------------------------------------------------- */
/* VERBS: Functions that implement the predefined verbs.   */
/* ------------------------------------------------------------------- */

void v_look() {
	do_look = 1;
}


/* 1=N, etc. */
static void v_go(int dir) {
	int newloc, tmploc;
	int i;
	/*  rbool has_seen;*/

	dir--;

	tmploc = loc;
	newloc = room[loc].path[dir];
	if (newloc > exitmsg_base) { /* Customized error messages */
		msgout(newloc - exitmsg_base, 1);
		return;
	}
	if (newloc < 0) { /* Run autoverb */
		int v0;

		v0 = verb_code(-newloc);
		if (v0 == 0) {
			if (!PURE_ERROR)
				writeln("GAME ERROR: Invalid verb.");
			return;
		}
		clear_stack();
		(void)scan_metacommand(0, v0, 0, 0, 0, NULL);
		return;
	}
	if (newloc < first_room) {
		if (dir == 12) /* Special */
			sysmsg(182, "Nothing happens.");
		else if (dir == 10) /* ENTER */
			sysmsg(197, "$You$ can't enter anything here.");
		else if (dir == 11) /* EXIT */
			sysmsg(198, "$You're$ not inside anything that $you$ can exit.");
		else
			sysmsg(13, "$You$ can't go that way.");
		return;
	}
	if (newloc > maxroom) {
		if (!PURE_ERROR)
			writeln("GAME ERROR: Invalid room number.");
		return;
	}

	/* Then need to check for hostile creatures */
	/* (If we are going back to the room we came from and not PURE_HOSTILE
	   is set, then we don't need to check this) */

	if (dir != 12 && (PURE_HOSTILE || newloc != oldloc + first_room))
		creatloop(i)
		if (creature[i].location == loc + first_room &&
		        creature[i].hostile) {
			parse_rec tmpcreat;
			curr_creat_rec = &tmpcreat;
			make_parserec(i + first_creat, &tmpcreat);
			sysmsg(14, "$The_c$$c_name$ blocks $your$ way.");
			curr_creat_rec = NULL;
			return;
		}

	/*  has_seen=room[newloc-first_room].has_seen;*/
	goto_room(newloc - first_room);
	if (dir != 12 && newloc != tmploc + first_room) /* SPECIAL */
		oldloc = tmploc; /* Can backtrack as long as not from special */
	if (dir == 12 && special_ptr[loc].size > 0)
		/* need to print special of NEW room */
		runptr(loc, special_ptr, "INTERNAL ERROR: Invalid special ptr", 0, NULL, NULL);

	if (tmploc == loc && dir == 12) /* SPECIAL that sends us nowhere */
		do_look = 0;
}


/* PUSH, PULL, TURN, PLAY, CHANGE_LOCATIONS */
static void v_noun(int vc, parse_rec *nounrec) {
	int dobj_;

	dobj_ = p_obj(nounrec);

	if (vc == 0 && !it_pushable(dobj_)) {
		int msgnum;
		if (!tcreat(dobj_)) msgnum = 172;
		else if (creature[dobj_ - first_creat].gender == 0)
			if (creature[dobj_ - first_creat].hostile) msgnum = 167;
			else msgnum = 168;
		else if (creature[dobj_ - first_creat].hostile) msgnum = 169;
		else msgnum = 170;
		sysmsgd(msgnum, "$You$ can't $verb$ $the_n$$noun$.", nounrec); /* Push */
		return;
	}
	if (vc == 1 && !it_pullable(dobj_)) { /* Pull */
		sysmsgd(tcreat(dobj_) ? 173 : 175, "$You$ can't $verb$ $the_n$$noun$.",
		        nounrec);
		return;
	}
	if (vc == 2 && !it_turnable(dobj_)) { /* Turn */
		sysmsgd(tcreat(dobj_) ? 164 : 166, "$You$ can't $verb$ $the_n$$noun$.",
		        nounrec);
		return;
	}
	if (vc == 3 && !it_playable(dobj_)) { /* Play */
		sysmsgd(tcreat(dobj_) ? 176 : 178, "$You$ can't $verb$ $the_n$$noun$.",
		        nounrec);
		return;
	}
	if (matchclass(dobj_, room[loc].key)) { /* SPECIAL triggered */
		v_go(13);
		return;
	}
	if (vc == 4) {
		sysmsgd(tcreat(dobj_) ? 180 : 181, "Nothing happens.", nounrec);
		return;
	}
	/* otherwise, print out relevent description. */
	if (vc == 0) /* Push */
		runptr(dobj_ - first_noun, push_ptr,
		       "$You$ $verb$ $the_n$$noun$ for a while, but nothing happens.",
		       171, nounrec, NULL);
	if (vc == 1) /* Pull */
		runptr(dobj_ - first_noun, pull_ptr,
		       "$You$ $verb$ $the_n$$noun$ a bit, but nothing happens.", 174,
		       nounrec, NULL);
	if (vc == 2) /* Turn */
		runptr(dobj_ - first_noun, turn_ptr,
		       "$You$ $verb$ $the_n$$noun$, but nothing happens.", 165,
		       nounrec, NULL);
	if (vc == 3) /* Play */
		runptr(dobj_ - first_noun, play_ptr,
		       "$You$ $verb$ $the_n$$noun$ for a bit, but nothing happens.", 177,
		       nounrec, NULL);
}

/* vc==1 if ASK, 0 if TALK TO */
static void v_talk(int vc, parse_rec *nounrec, parse_rec *objrec) {
	int dobj_, iobj_;

	dobj_ = p_obj(nounrec);
	iobj_ = p_obj(objrec);

	if (nounrec->info == D_END || nounrec->info == D_AND) {
		alt_sysmsg(211, "Who $are$ $you$ addressing?", nounrec, objrec);
		return;
	}
	if (!genvisible(nounrec)) {
		alt_sysmsg(212, "Who $are$ $you$ addressing?", nounrec, objrec);
		return;
	}
	if (!tcreat(dobj_)) {
		alt_sysmsg(vc ? 161 : 156, "That isn't animate.", nounrec, objrec);
		return;
	}
	if (vc == 0)
		runptr(dobj_ - first_creat,
		       talk_ptr, "$Your$ conversational gambit is ignored.",
		       creature[dobj_ - first_creat].gender == 0 ? 157 : (iobj_ == 0 ? 159 : 158),
		       nounrec, objrec);
	if (vc == 1)
		runptr(dobj_ - first_creat, ask_ptr, "$You$ get no answer.",
		       iobj_ == 0 ? 162 : 163, nounrec, objrec);
}

static void v_examine(parse_rec *nounrec) {
	if (!islit()) {
		sysmsgd(room[loc].light == 1 ? 19 : 20, "It's too dark to see anything.",
		        nounrec);
	}
	it_describe(nounrec->obj);
}

static void v_view(parse_rec *nounrec) { /* VIEW a picture */
	int i;
	int dobj_;
	dobj_ = p_obj(nounrec);

	if (tnoun(dobj_) && noun[dobj_ - first_noun].pict != 0)
		pictcmd(1, pictable[noun[dobj_ - first_noun].pict - 1]);
	else if (tcreat(dobj_) && creature[dobj_ - first_creat].pict != 0)
		pictcmd(1, pictable[creature[dobj_ - first_creat].pict - 1]);
	else if (dobj_ == -ext_code[wscene] && room[loc].pict != 0)
		/* View the room picture */
		pictcmd(1, pictable[room[loc].pict - 1]);
	else {  /* room.PIX_bits */
		if (dobj_ < 0)
			for (i = 0; i < maxpix; i++) /* Check them all */
				if (dobj_ == -pix_name[i] &&
				        (room[loc].PIX_bits & (1L << i))) {
					pictcmd(2, i);
					return;
				}
		sysmsgd(217, "That can't be VIEWed here.", nounrec);
	}
}


static void v_read(parse_rec *nounrec) {
	int dobj_;
	dobj_ = p_obj(nounrec);

	if (!tnoun(dobj_) || !noun[dobj_ - first_noun].readable) {
		sysmsg(134,
		       "$You$ can't read $the_n$$noun$, "
		       "so instead $you$ just examine $n_indir$.");
		it_describe(dobj_);
		return;
	}
	if (text_ptr[dobj_ - first_noun].size > 0)
		runptr(dobj_ - first_noun, text_ptr,
		       "INTERNAL ERROR: Invalid read pointer", 0, NULL, NULL);
	else
		runptr(dobj_ - first_noun, noun_ptr, "$You$ learn nothing new.",
		       193, nounrec, NULL);
}


static void v_eat(int vc, parse_rec *nounrec) {
	int dobj_;
	dobj_ = p_obj(nounrec);

	if (!tnoun(dobj_)) {
		sysmsgd(124, "That can't be consumed.", nounrec);
		return;
	}
	if (vc == 0 && !noun[dobj_ - first_noun].edible) {
		sysmsgd(124, "$You$ can't eat that.", nounrec);
		return;
	}
	if (vc == 1 && !noun[dobj_ - first_noun].drinkable) {
		sysmsgd(127, "$You$ can't drink that.", nounrec);
		return;
	}

	sysmsgd(128, "$You$ $verb$ $the_n$$adjective$ $noun$.", nounrec);

	if (noun[dobj_ - first_noun].movable) it_destroy(dobj_);
	if (noun[dobj_ - first_noun].poisonous) {
		sysmsgd(129, "Unfortunatly, $n_pro$ $n_was$ poisonous.", nounrec);
		deadflag = 1;
	}
}


/* assumes objrec is in the noun range */
static int can_wear(parse_rec *objrec) {
	static const char *errs[] = {
		"$The_n$$noun$ $n_is$ far too heavy to wear.",
		"$You're$ already loaded down with too much weight as it is.",
		"$The_n$$noun$ $n_is$ too big and bulky to wear.",
		"$You're$ wearing too much to also wear $the_n$$noun$."
	};
	int n;

	if (!it_canmove(objrec->obj)) {
		sysmsgd(202, "$You$ can't move $the_n$$noun$.", objrec);
	}
	n = check_fit(objrec->obj, 1000);
	if (n == FIT_OK /* || n>=FIT_SIZE */)  return 1;
	sysmsgd(37 + n, errs[n - 1], objrec);
	return 0;
}


/* assumes objrec is in the noun range */
static int can_carry(parse_rec *objrec) {
	static const char *errs[] = {
		"$The_n$$noun$ $n_is$ far too heavy to carry.",
		"$You're$ already carrying too much weight as it is.",
		"$The_n$$noun$ $n_is$ too big and bulky to pick up.",
		"$You're$ carrying too much to also carry $the_n$$noun$."
	};
	int n;

	n = check_fit(objrec->obj, 1);
	if (n == FIT_OK) return 1;
	sysmsgd(30 + n - 1, errs[n - 1], objrec);
	return 0;
}

static int v_get(parse_rec *objrec) {
	int cnt, i;
	int obj;

	obj = objrec->obj;

	/* If there is a hostile creature in the room and PURE_GETHOSTILE isn't
	   set, then don't let the player pick up anything */
	if (!PURE_GETHOSTILE)
		creatloop(i)
		if (creature[i].location == loc + first_room &&
		        creature[i].hostile) {
			parse_rec tmpcreat;
			make_parserec(i + first_creat, &tmpcreat);
			curr_creat_rec = &tmpcreat;
			sysmsgd(14, "$The_c$$c_name$ blocks $your$ way.", objrec);
			return 0;
		}

	if (objrec->info == D_ALL) {
		cnt = 0;
		nounloop(i)
		if (noun[i].location == loc + first_room && noun[i].movable) {
			/* Need to add weight/size check */
			parse_rec tmpnoun;
			make_parserec(i + first_noun, &tmpnoun);
			if (can_carry(&tmpnoun)) {
				get_obj(i + first_noun);
				sysmsgd(8, "$You$ pick up $the_n$$adjective$ $noun$.", &tmpnoun);
			}
			cnt++;
		}
		if (cnt == 0) {
			sysmsgd(24, "There doesn't seem to be anything here to take.", objrec);
			return 0;
		} else return 1;
	}
	if (it_door(obj, objrec->noun)) {
		if (room[loc].locked_door)
			sysmsgd(25, "You can't pick up the door.", objrec);
		else
			sysmsgd(26, "You can't pick up the doorway.", objrec);
		return 0;
	}
	if (!tnoun(obj) || !noun[obj - first_noun].movable) {
		sysmsgd(tcreat(obj) ? (creature[obj - first_creat].hostile ? 34 : 35) : 29,
		        "$You$ can't pick $the_n$$noun$ up.", objrec);
		return 0;
	}
	if (it_loc(obj) == 1) {
		sysmsgd(27, "$You$ already have $the_n$$noun$.", objrec);
		return 1;
	}
	if (!can_carry(objrec)) return 0;
	get_obj(obj);
	sysmsgd(8, "$You$ pick up $the_n$$adjective$ $noun$.", objrec);
	return 1;
}

static int v_remove(parse_rec *objrec) {
	int i, j;
	integer obj;

	obj = objrec->obj;
	if (objrec->info == D_ALL) {
		if (player_worn == 0) {
			sysmsgd(46, "$You're$ not wearing anything.", objrec);
			return 0;
		}
		safecontloop(i, j, 1000)
		if (it_canmove(i)) {
			parse_rec tmp;
			if (PURE_WEAR) drop_obj(i);
			else it_move(i, 1);  /* Really need to check to make sure
                   we haven't exceeded weight requirement
                 here */
			make_parserec(i, &tmp);
			sysmsgd(9, "$You$ take off $the_n$$noun$.", &tmp);
		}
		return 1;
	}
	if (it_loc(obj) != 1000) {
		sysmsgd(213, "$You're$ not wearing that.", objrec);
		return 0;
	}
	if (!it_canmove(obj)) {
		sysmsgd(201, "$You're$ not able to remove $the_n$$noun$.", objrec);
		return 0;
	}
	sysmsgd(9, "$You$ take off $the_n$$noun$.", objrec);
	if (PURE_WEAR) drop_obj(obj);  /* Required to be consistent w/ AGT */
	else v_get(objrec);  /* (trap can_carry problems) */
	return 1;
}

static void v_drop(parse_rec *objrec) {
	int i, j;
	int obj;
	obj = objrec->obj;

	if (obj == ALL_MARK) {
		if (player_contents == 0)
			sysmsgd(45, "$You$ don't have anything to drop.", objrec);
		else safecontloop(i, j, 1) {
			parse_rec tmp;
			make_parserec(i, &tmp);
			drop_obj(i);
			sysmsgd(9, "$You$ $verb$ $the_n$$noun$.", &tmp);
		}
		return;
	}
	if (!it_possess(obj)) {
		sysmsgd(47, "$You$ don't have that.", objrec);
		return;
	}
	if (!it_canmove(obj)) {
		sysmsgd(200, "$You're$ not able to $verb$ $the_n$$noun$.", objrec);
		return;
	}
	if (it_loc(obj) == 1000) {
		sysmsgd(216, "(Taking it off first)", objrec);
	}
	sysmsgd(9, "$You$ $verb$ $the_n$$noun$.", objrec);
	drop_obj(obj);
}

static void v_wear(parse_rec *objrec) {
	int i, cnt;
	int obj;

	obj = objrec->obj;
	if (objrec->info == D_ALL) {
		cnt = 0;
		nounloop(i)
		if (noun[i].location != 1000 && visible(i + first_noun) &&
		        noun[i].wearable) {
			parse_rec tmp;
			make_parserec(i + first_noun, &tmp);
			if (can_wear(&tmp)) {
				it_move(i + first_noun, 1000);
				sysmsgd(42, "$You$ put on $the_n$$adjective$ $noun$.", &tmp);
			}
			cnt++;
		}
		if (cnt == 0)
			sysmsgd(36, "There doesn't seem to be anything $you$ can wear here.",
			        objrec);
		return;
	}
	if (!tnoun(obj) || !noun[obj - first_noun].wearable) {
		sysmsgd(tcreat(obj) ? (creature[obj - first_creat].hostile ? 43 : 44) : 203,
		        "$You$ can't wear that.", objrec);
		return;
	}
	if (it_loc(obj) == 1000) {
		sysmsgd(37, "$You$ $are$ already wearing $the_n$$noun$.", objrec);
		return;
	}
	if (!can_wear(objrec)) return;
	sysmsgd(42, "$You$ put on $the_n$$noun$.", objrec);
	it_move(obj, 1000);
}

/* l_or_u: 0=lock, 1=unlock */
static int do_lock(uchar l_or_u, parse_rec *nounrec, parse_rec *objrec) {
	int dnoun;
	int dobj_, iobj_;
	word dobj_word;

	dobj_ = p_obj(nounrec);
	iobj_ = p_obj(objrec);
	dobj_word = nounrec->noun;

	if (it_door(dobj_, dobj_word) && l_or_u != room[loc].locked_door) {
		/* That is, trying to unlock an unlocked door, or lock a locked one. */
		if (l_or_u == 0)
			alt_sysmsg(114, "The door is already locked.", nounrec, objrec);
		else
			alt_sysmsg(105,
			           "There doesn't seem to be any door here that need unlocking.",
			           nounrec, objrec);
		return 0;
	}
	if (!it_lockable(dobj_, dobj_word)) {
		alt_sysmsg((l_or_u ? 108 : 118), "$The_n$$noun$ can't be $verb$ed.",
		           nounrec, objrec);
		return 0;
	}
	if (tnoun(dobj_) && noun[dobj_ - first_noun].closable && it_open(dobj_)) {
		if (l_or_u == 0) {
			alt_sysmsg(120, "$You$ will need to close $the_n$$noun$ first.",
			           nounrec, objrec);
			return 0;
		} else { /* l_or_u==1 */
			alt_sysmsg(110, "$The_n$$noun$ $n_is$ already open!",
			           nounrec, objrec);
			return 0;
		}
	}
	if (it_locked(dobj_, dobj_word) != l_or_u) {
		alt_sysmsg((l_or_u ? 109 : 119), "$The_n$$noun$ $n_is$ already $verb$ed",
		           nounrec, objrec);
		return 0;
	}
	if (it_door(dobj_, dobj_word) || dobj_ < 0) { /* i.e. a door */
		alt_sysmsg((l_or_u ? 104 : 115),
		           "$You$ try to $verb$ $the_n$$noun$, but fail.",
		           nounrec, objrec);
		return 0;
	}
	dnoun = dobj_ - first_noun;
	if (iobj_ == 0) {
		alt_sysmsg((l_or_u ? 106 : 208),
		           "$You$ will need to use something to do that.",
		           nounrec, objrec);
		return 0;
	}
	if (!player_has(iobj_)) {
		alt_sysmsg((l_or_u ? 107 : 117), "$You$ don't have $the_o$$object$.",
		           nounrec, objrec);
		return 0;
	}
	if (!matchclass(iobj_, noun[dnoun].key)) {
		alt_sysmsg(l_or_u ? (vb == 15 ? 80 : 111) : 121, /* vb 15 is OPEN */
		           "$The_o$$object$ doesn't fit.", nounrec, objrec);
		return 0;
	}
	noun[dnoun].locked = !l_or_u;
	return 1;
}

/* First argument indicates lock or unlock-- 0=lock, 1=unlock */
static void v_lock(uchar l_or_u, parse_rec *nounrec, parse_rec *objrec) {
	if (!do_lock(l_or_u, nounrec, objrec)) return;
	/* Need to fix these messages: */
	alt_sysmsg((l_or_u ? 112 : 122),
	           "$You$ $verb$ $the_n$$noun$ with $the_o$$object$.",
	           nounrec, objrec);
}

/* OPEN ... WITH ... */
static void v_open(parse_rec *nounrec, parse_rec *objrec) {
	int dnoun;
	int dobj_, iobj_;

	dobj_ = p_obj(nounrec);
	iobj_ = p_obj(objrec);

	dnoun = dobj_ - first_noun;
	if (it_door(dobj_, nounrec->noun)) {
		if (room[loc].locked_door)
			alt_sysmsg(71, "$The_n$$noun$ $n_is$ locked.",
			           nounrec, objrec);
		else
			alt_sysmsg(72, "$The_n$$noun$ $n_is$ already open.",
			           nounrec, objrec);
		return;
	}
	if (it_open(dobj_)) {
		alt_sysmsg(78, "$The_n$$noun$ $n_is$ already open.", nounrec, objrec);
		return;
	}
	if (!tnoun(dobj_) || !noun[dnoun].closable) {
		alt_sysmsg(77, "$You$ can't open $the_n$$noun$.", nounrec, objrec);
		return;
	}
	if (iobj_ != 0) { /* Need to do unlock action */
		if (!do_lock(1, nounrec, objrec)) return;
		/* If something goes wrong, return */
	}
	if (noun[dnoun].lockable && noun[dnoun].locked) {
		alt_sysmsg(79, "It is locked.", nounrec, objrec);
		return;
	}
	noun[dnoun].open = 1;
	if (iobj_ != 0) /* Obviously these messages need improvement */
		alt_sysmsg(81, "$You$ have opened $the_n$$noun$ with $the_o$$object$.",
		           nounrec, objrec);
	else alt_sysmsg(82, "$You$ have opened $the_n$$noun$.", nounrec, objrec);
	if (noun[dnoun].contents != 0)
		alt_sysmsg(187, "Inside, $you$ see the following:", nounrec, objrec);
	print_contents(dobj_, 1);
}

static void v_close(parse_rec *nounrec) {
	int dobj_;
	dobj_ = nounrec->obj;

	if (it_door(dobj_, nounrec->noun)) {
		if (room[loc].locked_door)
			sysmsgd(84, "The door is already closed.", nounrec);
		else
			sysmsgd(85, "That apparently can't be closed.", nounrec);
		return;
	}
	if (!it_open(dobj_)) {
		sysmsgd(88, "$The_n$$noun$ $n_is$ already closed.", nounrec);
		return;
	}
	if (!tnoun(dobj_) || !noun[dobj_ - first_noun].closable) {
		sysmsgd(87, "$You$ can't close $the_n$$noun$.", nounrec);
		return;
	}
	noun[dobj_ - first_noun].open = 0;
	sysmsgd(89, "$You$ have closed $the_n$$noun$.", nounrec);
}


static void v_light(int newstate, parse_rec *nounrec) {
	int dobj_;
	dobj_ = p_obj(nounrec);

	if (!tnoun(dobj_) || !noun[dobj_ - first_noun].light) {
		sysmsgd(newstate ? 135 : 140, "$You$ can't $verb$ $the_n$$noun$.", nounrec);
		return;
	}
	dobj_ -= first_noun;
	if (noun[dobj_].on == newstate) {
		if (newstate)
			sysmsgd(136, "$The_n$$noun$ $n_is$ already lit.", nounrec);
		else sysmsgd(141,
			             "$The_n$$noun$ $n_is$n't lit, so $you$ can't extinguish $n_indir$",
			             nounrec);
		return;
	}
	noun[dobj_].on = newstate;
	if (newstate) sysmsgd(138, "$The_n$$noun$ $n_is$ now lit.", nounrec);
	else sysmsgd(143, "$The_n$$noun$ $n_is$ no longer lit.", nounrec);
}

static void v_turn(word prep_, parse_rec *nounrec) {
	int newstate;  /* 1=on, 0=off */
	int dobj_;
	dobj_ = p_obj(nounrec);

	newstate = (prep_ == ext_code[won]); /* ON or OFF ? */
	if (!it_turnable(dobj_) && !nounattr(dobj_, light)) {
		sysmsgd(newstate ? 209 : 210,
		        "$You$ can't turn $the_n$$noun$ $prep_$.", nounrec);
		return;
	}
	if (matchclass(dobj_, room[loc].key)) { /* SPECIAL triggered */
		v_go(13);
		return;
	}
	if (!tnoun(dobj_)) { /* This should be redundant */
		writeln("INTERNAL ERROR: Non-noun turn on/off not supported");
		return;
	}
	dobj_ -= first_noun;
	if (noun[dobj_].on == newstate) {
		sysmsgd(newstate ? 137 : 142, "$The_n$$noun$ $n_is$ already $prep_$.",
		        nounrec);
		return;
	}
	noun[dobj_].on = newstate;
	sysmsgd(newstate ? 139 : 144, "$The_n$$noun$ $n_is$ now $prep_$.", nounrec);
}



/* Missile=1 if actually firing a weapon. */
static void v_attack(uchar missile, parse_rec *targrec, parse_rec *weprec) {
	int targ, wep;
	targ = targrec->obj;
	wep = weprec->obj;

	/* The following fix really belongs in the parser, but it might
	   break some games to do this translation before running metacommands */
	if (missile && targ == 0) /* SHOOT <target> */
		if (!tnoun(wep) || !noun[wep - first_noun].shootable) {
			targ = wep;
			targrec = weprec;
			wep = 0;
		}

	curr_creat_rec = targrec; /* So error messages will print properly */
	if (wep > 0 && !player_has(wep)) {
		alt_sysmsg(98, "(Getting $the_o$$object$ first)", targrec, weprec);
		if (!v_get(weprec)) return;
	}
	if ((targ > 0 && !tcreat(targ)) || targ < 0) {
		alt_sysmsg(missile ? 90 : 93,
		           "It only makes sense to attack living things.",
		           targrec, weprec);
		return;
	}
	if (missile) {
		if (wep == 0) {
			sysmsgd(94, "It's not clear what $you$ want to $verb$ with.", targrec);
			return;
		} else if (!tnoun(wep) || !noun[wep - first_noun].shootable) {
			alt_sysmsg(it_isweapon(wep) ? 96 : 95,
			           "$The_o$$object$ doesn't seem to be able to fire.",
			           targrec, weprec);
			return;
		} else if (noun[wep - first_noun].num_shots <= 0) {
			alt_sysmsg(97, "$The_o$$object$ $o_is$ out of ammunition.",
			           targrec, weprec);
			return;
		} else noun[wep - first_noun].num_shots--;
	}

	if (targ == 0) {
		if (!missile) {
			alt_sysmsg(206, "Attack what???", NULL, weprec);
			return;
		} else {
			alt_sysmsg(188, "$You$ fire a shot into the air.", NULL, weprec);
			return;
		}
	}

	if (wep == 0) { /* and !missile, but that's taken care of above */
		sysmsgd(creature[targ - first_creat].hostile ? 91 : 92,
		        "$You$ attack $the_n$$noun$ with $your$ bare hands, but $n_pro$ "
		        "evades $your$ attack.", targrec);
		return;
	}

	if (matchclass(wep, creature[targ - first_creat].weapon)) {
		if (missile)
			alt_sysmsg(creature[targ - first_creat].hostile ? 99 : 101,
			           "$You$ shoot $the_n$$noun$; "
			           "$n_pro$ vanishes in a cloud of red smoke."
			           , targrec, weprec);
		else
			alt_sysmsg(creature[targ - first_creat].hostile ? 49 : 53,
			           "$You$ kill $the_o$$object$; "
			           "$o_pro$ vanishes in a cloud of red smoke.",
			           weprec, targrec);
		it_destroy(targ);
		if (!missile) drop_obj(wep);
		return;
	} else {
		if (!missile) {
			int msgnum;
			if (creature[targ - first_creat].hostile) {
				alt_sysmsg(50, NULL, weprec, targrec); /* Preliminary message */
				msgnum = 51;
			} else msgnum = 54;
			if (noun[wep - first_noun].drinkable) { /* i.e. a liquid */
				alt_sysmsg(msgnum + 1, "$You$ splash $the_o$$object$ with "
				           "$the_n$$noun$, but the liquid quickly evaporates "
				           "without noticable effect.", weprec, targrec);
				it_destroy(wep);
			} else {
				alt_sysmsg(msgnum,
				           "$You$ strike at $the_o$$object$ with $the_n$$noun$, "
				           "but $your$ weapon bounces off of $o_indir$ harmlessly",
				           weprec, targrec);
				drop_obj(wep);
			}
		} else
			alt_sysmsg(creature[targ - first_creat].hostile ? 100 : 102 ,
			           "$You$ fire at $the_n$$noun$ with $the_o$$object$, but $your$ "
			           "shots don't seem to have any effect.", targrec, weprec);

		if (creature[targ - first_creat].hostile &&
		        ++creature[targ - first_creat].counter >=
		        creature[targ - first_creat].threshold) {
			alt_sysmsg(204, "$The_n$$noun$ counterattacks! $N_pro$ fights "
			           "viciously and $you$ $are$ unable to defend $your$self "
			           "against $n_indir$.", targrec, weprec);
			deadflag = 1;
		}
	}
}

/* child_proc is true if v_put is being called by v_put, and so
   shouldn't print success messages */
static rbool v_put(parse_rec *nounrec, word prep_,
                   parse_rec *objrec, rbool child_proc) {
	rbool in_prep;
	int dobj_, iobj_;

	dobj_ = p_obj(nounrec);
	iobj_ = p_obj(objrec);

	in_prep = (prep_ == ext_code[win] || prep_ == ext_code[winto]
	           || prep_ == ext_code[winside]);

	if (prep_ == 0 || iobj_ == 0) {
		v_drop(nounrec);
		return 1;
	}
	if (!tnoun(dobj_)) {
		alt_sysmsg(tcreat(dobj_) ? 11 : 10,
		           "$You$ can't do that with $the_n$$noun$.",
		           nounrec, objrec);
		return 0;
	}
	if (!noun[dobj_ - first_noun].movable) {
		alt_sysmsg(61, "$You$ can't move $the_n$$adjective$ $noun$.",
		           nounrec, objrec);
		return 0;
	}
	if (tcreat(iobj_)) {
		alt_sysmsg(189, "$The_o$$object$ doesn't want $n_indir$.",
		           nounrec, objrec);
		return 0;
	}
	if (!tnoun(iobj_)) {
		alt_sysmsg(tcreat(iobj_) ? 12 : 64,
		           "$You$ can't put something $prep_$ $the_o$$object$.",
		           nounrec, objrec);
		return 0;
	}
	if (dobj_ == iobj_) {
		alt_sysmsg(62, "$You$ can't put something $prep_$ $n_indir$self.",
		           nounrec, objrec);
		return 0;
	}
	if (!it_open(iobj_) && in_prep) {
		alt_sysmsg(65, "$The_o$$object$ $o_is$n't open.", nounrec, objrec);
		return 0;
	}
	if (player_has(iobj_) && !in_prep) {
		alt_sysmsg(is_within(iobj_, 1, 0) ? 68 : 69,
		           "$You$ can't put $the_n$$noun$ $prep_$ something that $you$ "
		           "$are$ carrying.", nounrec, objrec);
		return 0;
	}

	if (in_prep) { /* PUT IN */
		if (check_fit(dobj_, iobj_) != FIT_OK) {
			alt_sysmsg(66, "$You$ can't fit $the_n$$noun$ into $the_o$$object$.",
			           nounrec, objrec);
			return 0;
		}
		if (it_loc(dobj_) == 1000)
			alt_sysmsg(216, "(Taking $n_indir$ off first)", nounrec, objrec);
		it_move(dobj_, iobj_);
	} else { /* PUT <prep_> with a preposition other than IN */
		int parent;

		parent = it_loc(iobj_);
		if (!troom(parent)) {
			parse_rec parent_rec;
			make_parserec(parent, &parent_rec);
			if (!v_put(nounrec, ext_code[win], &parent_rec, 1)) return 0;
		} else {
			if (it_loc(dobj_) == 1000)
				alt_sysmsg(216, "(Taking $n_indir$ off first)", nounrec, objrec);
			drop_obj(dobj_);
		}
		dobj_ -= first_noun;
		assert(noun[dobj_].pos_prep == 0); /* v_put should have ensured this */
		noun[dobj_].pos_prep = prep_;
		noun[dobj_].pos_name = it_name(iobj_);
		if (iobj_ > 0) noun[dobj_].nearby_noun = iobj_;
	}
	if (!child_proc)
		alt_sysmsg(67, "$You$ place $the_n$$noun$ $prep_$ $the_o$$object$.",
		           nounrec, objrec);
	return 1;
}


/* at, to, in, into, across, inside */
static void v_throw(parse_rec *nounrec, word prep_, parse_rec *objrec) {
	int dobj_, iobj_;
	dobj_ = p_obj(nounrec);
	iobj_ = p_obj(objrec);

	/* Need to check to see what the preposition is-- if it is AT
	   then we should send it to attack routine. */
	if (!player_has(nounrec->obj)) {
		alt_sysmsg(47, "$You$ don't have $the_n$$noun$.", nounrec, objrec);
		return;
	}
	if (prep_ == 0) {
		v_drop(nounrec);
		return;
	}
	if (prep_ != ext_code[wat])
		v_put(nounrec, prep_, objrec, 0);
	else /* prep_ is AT */
		if (!noun[dobj_ - first_noun].movable) {
			alt_sysmsg(215, "$You$ can't move $the_n$$adjective$ $noun$.",
			           nounrec, objrec);
			return;
		}
	if (tcreat(iobj_))  /* If a creature, treat as an attack */
		v_attack(0, objrec, nounrec);
	else {  /* THROW AT somethin inanimate */
		if (dobj_ == iobj_) {
			alt_sysmsg(56, "$You$ can't $verb$ $the_n$$noun$ $prep_$ $n_indir$self.",
			           nounrec, objrec);
			return;
		}
		if (it_loc(dobj_) == 1000)
			alt_sysmsg(216, "(Taking it off first)", nounrec, objrec);

		if (tnoun(dobj_) && noun[dobj_ - first_noun].drinkable) {
			/* A liquid */
			if (tnoun(iobj_) && noun[iobj_ - first_noun].open)
				alt_sysmsg(58, "$You$ throw $the_n$$noun$ into $the_o$$object$, "
				           "but $n_pro$ quickly evaporates.",
				           nounrec, objrec);
			else
				alt_sysmsg(57, "$The_n$$noun$ splashes on $the_o$$object$ but "
				           "quickly evaporates.", nounrec, objrec);
			it_destroy(dobj_);
		} else { /* _Not_ a liquid: */
			if (tnoun(iobj_) && noun[iobj_ - first_noun].open)
				if (check_fit(dobj_, iobj_)) {
					alt_sysmsg(60, "$The_n$$noun$ lands inside $the_o$$object$.",
					           nounrec, objrec);
					it_move(dobj_, iobj_);
					return;
				} else {
					alt_sysmsg(205, "You $verb$ $the_n$$noun$ into $the_o$$object$, "
					           "but there isn't enough room and $n_pro$ falls out.",
					           nounrec, objrec);
				}
			else
				alt_sysmsg(59, "$The_n$$noun$ bounces off $the_o$$object$.",
				           nounrec, objrec);
			/* At this point, either the object is closed or doesn't have enough
			room */
			it_move(dobj_, first_room + loc);
		}
	}
}



void v_inventory(void) {
	if (player_contents != 0) {
		sysmsg(130, "$You're$ carrying:");
		print_contents(1, 1);  /* obj=1=self, ind_lev=1 */
	} else sysmsg(131, "$You$ $are$ empty-handed.");
	if (player_worn != 0) {
		sysmsg(132, "$You're$ wearing:");
		print_contents(1000, 1);
	}
}





static void v_quit(void) {
	sysmsg(145, "Are you sure you want to quit?");
	if (yesno("")) {
		sysmsg(146, NULL);
		quitflag = 1;
	}
}

const char dirname[12][10] = {"north", "south", "east", "west",
                              "northeast", "northwest", "southeast", "southwest",
                              "up", "down", "in", "out"
                             };

void v_listexit(void) {
	int i, j, k;

	if (!islit()) {
		sysmsg(23, "It is too dark to see anything.");
		return;
	}
	j = k = 0;
	for (i = 0; i < 12; i++)
		if (room[loc].path[i] != 0) k++;
	if (k == 0)
		sysmsg(224, "There are no immediately visible exits.");
	else {
		sysmsg(225, "There are exits to");
		for (i = 0; i < 12; i++)
			if (room[loc].path[i] != 0) {
				j++;
				if (j > 1) writestr(", ");
				if (j > 1 && j == k) writestr("or ");
				if (i < 8) writestr("the ");
				writestr(dirname[i]);
			}
		writeln(".");
	}
}


static void v_yell(void) {
	sysmsg(150, "YAAAAEEEEEEEEOOOOOOUUUUUAAAAHHHHHH!!!!!");
}



/* ------------------------------------------------------------------- */
/*  VERB EXECUTION AND GRAMMER CHECKING */


static int checkgram(int vb_, int dobj_, word prep_, int iobj_, rbool redir_flag) {
	int i;
	int msgnum;

	/* We turn off certain sorts of grammar checking if either PURE_GRAMMAR
	   is set or there has been signicant redirection. */
	if (redir_flag < 2) redir_flag = 0;
	if (PURE_GRAMMAR) redir_flag = 1;

	/* First of all, no constraints on dummy_verb grammer */
	if (vb_ >= BASE_VERB && vb_ < TOTAL_VERB) return 0;

	if (!(verbflag[vb_]&VERB_TAKEOBJ)
	        && (dobj_ != 0 || iobj_ != 0 || prep_ > 0)
	        && vb_ != OLD_VERB + 11) {
		if (redir_flag) return 0; /* Original AGT doesn't check this. */
		sysmsg(190, "$Verb$ doesn't take an object.");
		return -1;
	}

	/* Now verify prepositons. If PURE_GRAMMAR is set, we don't
	   check prepositions unless the verb actually accepts at least one.
	   (this reflects the behavior of the original AGT interpreters). */
	if (prep_ > 0 && !(redir_flag && syntbl[preplist[vb_]] == 0)) {
		for (i = preplist[vb_]; syntbl[i] != 0 && syntbl[i] != prep_; i++);
		if (syntbl[i] != prep_) {
			msgnum = 191;
			if (vb_ == 15) msgnum = 74; /* Open */
			if (vb_ == 17) msgnum = 116; /* Lock */
			if (vb_ == 14) msgnum = 48; /* Throw */
			sysmsg(msgnum, "$Verb$ doesn't take $prep_$ as a preposition.");
			return -1;
		}
	}
	if (iobj_ == ALL_MARK) {
		sysmsg(199, "You can't use ALL as an indirect object");
		return -1;
	}
	if (dobj_ == ALL_MARK && vb_ != 33 && vb_ != 41 && vb_ != 51 && vb_ != 52) {
		/* i.e. verb is not GET,DROP,WEAR,REMOVE */
		msgnum = 5;
		if (vb_ == 31) msgnum = 155; /* Talk */
		if (vb_ == 34) msgnum = 160; /* Ask */
		sysmsg(5, "You can't use ALL with '$verb$'.");
		return -1;
	}
	return 0;
}


/* This checks to make sure that all of the objects are present */
static rbool verify_scope(int vb_, parse_rec *nounrec,
                          word prep_, parse_rec *objrec) {
	int msgnum;
	int dobj_, iobj_;
	dobj_ = nounrec->obj;
	iobj_ = objrec->obj;

	if (!(verbflag[vb_]&VERB_TAKEOBJ)) return 1;
	/* No objects (and we've already checked the grammar in
	a previous routine) */

	if (vb_ == 31 || vb_ == 34) /* TELL, ASK */
		return 1;  /* These verbs handle this themselves */

	if (dobj_ == 0) {
		sysmsg(184, "What do $you$ want to $verb$?");
		return 0;
	}
	if (dobj_ != ALL_MARK && !genvisible(nounrec)
	        && !(it_door(dobj_, nounrec->noun) && /* DOOR object handling */
	             (vb_ == 33 || vb_ == 15 || vb_ == 16 || vb_ == 17 || vb_ == 18
	              || vb_ == 29 || vb_ == 24 || vb_ == 22 || vb_ == 21))) {
		msgnum = 3;
		if (vb_ == 33) msgnum = 28; /* Get */
		if (vb_ == 29) msgnum = 63; /* Put */
		if (vb_ == 15) msgnum = 75; /* Open */
		if (vb_ == 16) msgnum = 86; /* Close */
		if (vb_ == 24) msgnum = 126; /* Drink */
		if (vb_ == 22) msgnum = 133; /* Read */
		if (vb_ == 21) msgnum = 179; /* Change_Locations */
		sysmsg(msgnum, "$You$ don't see any $noun$ here.");
		return 0;
	}

	if (prep_ != 0 && vb_ != 35) { /* verb 35 is TURN e.g. ON|OFF */
		if (iobj_ == 0) {
			msgnum = 214;
			if (vb_ == 29) msgnum = 70; /* Put */
			sysmsg(msgnum, "What do $you$ want to $verb$ $the_n$$noun$ $prep_$?");
			return 0;
		}
		if (iobj_ == -ext_code[wdoor]) {
			sysmsg(183, "You can't $verb$ $prep_$ $the_o$$object$.");
			return 0;
		}
		if (iobj_ != ALL_MARK && !genvisible(objrec)) {
			msgnum = 4;
			if (vb_ == 15) msgnum = 76; /* Open */
			if (vb_ == 18) msgnum = 207; /* Unlock */
			sysmsg(msgnum, "$You$ don't see any $object$ here.");
			return 0;
		}
	}
	return 1;
}


static void exec_verb_info(void) {
	char *a, *b, *c;
	char buff[200];

	a = objname(dobj);
	b = objname(iobj);
	c = objname(actor);
	sprintf(buff, "\t\t]]%s, %s %s(%ld) %s %s(%ld)", c, dict[ syntbl[auxsyn[vb]] ],
	        a, dobj_rec->num, prep == 0 ? "->" : dict[prep], b, iobj_rec->num);
	writeln(buff);
	rfree(a);
	rfree(b);
	rfree(c);
}


/* Returns true if the turn is done. */
rbool metacommand_cycle(int save_vb, int *p_redir_flag) {
	if (!have_meta) return 0;


	/* Now check metacommands */
	if (DEBUG_AGT_CMD)
		debugout("*** Scanning: ANY metacommands ****\n");
	/* ANY metacommands: */
	supress_debug = !debug_any;
	clear_stack();
	if ((PURE_METAVERB || !was_metaverb)
	        && 2 == scan_metacommand(0, 0, 0, 0, 0, NULL))
		return 1;

	supress_debug = 0;

	vb = save_vb;
	actor_in_scope |= visible(actor); /* Set up for ActorWasPresent */

	clear_stack();
	if (actor != 0 && aver < AGX00) {
		if (DEBUG_AGT_CMD)
			debugout("*** Scanning: ANYBODY metacommands ****\n");
		if (2 == scan_metacommand(2, vb, dobj, prep, iobj, NULL))
			return 1;
	}

	clear_stack();
	if (DEBUG_AGT_CMD)
		debugout("*** Scanning: VERB metacommands ****\n");
	/* Normal treatment */
	if (2 == scan_metacommand(actor, vb, dobj, prep, iobj, p_redir_flag))
		return 1;
	/* Note that scan_metacommand will change the -global- copy of vb if a
	RedirectTo occurs. */

	return 0;
}



/* Execute both meta-commands and more normal commands */
/* May need tweaking for AGAIN and UNDO */
void exec_verb(void) {
	int objswap;  /* 1=if iobj has been moved to dobj */
	/* (Done for metacommands when there is an iobj but no dobj) */
	rbool turndone;
	int save_vb;
	int redir_flag;

	if (DEBUG_EXEC_VERB) exec_verb_info();

	do_disambig = 0; /* We're doing this for real */

	save_vb = vb;
	cmd_saveable = 1;
	redir_flag = 0;

	was_metaverb = (verbflag[vb] & VERB_META)
	               && actor == 0 && dobj == 0 && prep == 0 && iobj == 0;

	/* The following is purely for metacommands */
	if (dobj == 0 && dobj_rec->info != D_NUM && iobj != 0) {
		dobj = iobj;
		rfree(dobj_rec);
		dobj_rec = copy_parserec(iobj_rec);
		objswap = 1;
	} else objswap = 0;

	beforecmd = 1; /* This is for 1.8x support */

	turndone = metacommand_cycle(save_vb, &redir_flag) || deadflag;

	if (!turndone && DEBUG_AGT_CMD)
		debugout("*** Executing Built-in Verbs ****\n");

	if (actor > 0 && !turndone) {
		if (!actor_in_scope)
			sysmsg(196, "I don't see whom $you$ $are$ trying to address here.");
		else
			sysmsg(192, "$The_name$$name$ doesn't want to.");
	} else if (vb == 19 && dobj == 0 && prep == 0 && iobj == 0)
		/* LOOK: Doesn't matter if turn is done. */
		v_look();
	else if (!turndone) {
		/* Execute normal verbs: check grammer and then call */
		if (!objswap) {
			if (checkgram(vb, dobj, prep, iobj, redir_flag) == -1) return;
		} else if (checkgram(vb, 0, prep, iobj, redir_flag) == -1) return;

		if (!verify_scope(vb, dobj_rec, prep, iobj_rec)) return;

		if (vb < 13 && vb > 0) v_go(vb);
		else switch (vb) {

			case 14:
				v_throw(dobj_rec, prep, iobj_rec);
				break;
			case 29:
				v_put(dobj_rec, prep, iobj_rec, 0);
				break;

			/* _with_ verbs */
			case 15:
				v_open(dobj_rec, iobj_rec);
				break;
			case 16:
				v_close(dobj_rec);
				break;
			case 17:
				v_lock(0, dobj_rec, iobj_rec);
				break; /* LOCK */
			case 18:
				v_lock(1, dobj_rec, iobj_rec);
				break; /* UNLOCK */
			case 36:
				v_noun(0, dobj_rec);
				break; /* PUSH (WITH);Ignore indir object*/

			case 26:
				v_attack(0, dobj_rec, iobj_rec);
				break;
			case 49:
				if (prep == ext_code[wwith])
					v_attack(1, dobj_rec, iobj_rec); /* SHOOT WITH */
				else
					v_attack(1, iobj_rec, dobj_rec); /* SHOOT AT */
				break;

			/* _about_ verbs */
			case 31:
				v_talk(0, dobj_rec, iobj_rec);
				break; /* TELL */
			case 34:
				v_talk(1, dobj_rec, iobj_rec);
				break; /* ASK */

			case 28:
				v_yell();
				break;
			case 27:
				sysmsg(149, "Time passes...");
				break;            /* wait */
			case 55:
				v_go(13);
				break;  /* magic_word */

			/* case 19: v_look();break;  -- this is moved up above */

			case 50:
				runptr(loc, help_ptr, "Sorry, you're on your own here.",
				       2, NULL, NULL);
				break;   /* HELP */
			case 32:
				v_inventory();
				break;
			case 56:
				v_view(dobj_rec);
				break;  /* VIEW */
			case 35:
				if (prep > 0)
					v_turn(prep, dobj_rec); /* TURN ON|OFF */
				else
					v_noun(2, dobj_rec); /* TURN */
				break;
			case 20:
				v_examine(dobj_rec);
				break;
			case 22:
				v_read(dobj_rec);
				break;
			case 23:
				v_eat(0, dobj_rec);
				break;  /* EAT */
			case 24:
				v_eat(1, dobj_rec);
				break;  /* DRINK */
			case 37:
				v_noun(1, dobj_rec);
				break;   /* PULL  */
			case 38:
				v_noun(3, dobj_rec);
				break;  /* PLAY */
			case 47:
				v_light(1, dobj_rec);
				break;   /* LIGHT */
			case 48:
				v_light(0, dobj_rec);
				break;   /* EXTINGUISH */
			case 21:
				v_noun(4, dobj_rec);
				break; /* Change Location */

			case 51:
				v_wear(dobj_rec);
				break;
			case 33:
				v_get(dobj_rec);
				break;    /* ? */
			case 52:
				v_remove(dobj_rec);
				break;
			case 41:
				v_drop(dobj_rec);
				break;

			case 19:
				v_look();
				break;
			case 25:
				print_score();
				break;
			case 30:
				cmd_saveable = 0;
				v_quit();
				break;
			/* case 40:  SHOW --> default message */
			case 39:
			case 42:
				v_listexit();
				break;
			case 43:
				cmd_saveable = 0;
				verboseflag = 0; /* BRIEF */
				writeln(
				    "[Now in BRIEF mode (room descriptions will only be printed"
				    " when they are entered the first time)]");
				break;
			case 44:
				cmd_saveable = 0;
				verboseflag = 1;
				v_look();  /* VERBOSE */
				writeln("[Now in VERBOSE mode (room descriptions will be"
				        " printed every time you enter a room)]");
				break;
			case 45:
				cmd_saveable = 0;
				g_vm->saveGame();
				break;
			case 46:
				cmd_saveable = 0;
				doing_restore = 1;
				return;
				break;
			case 53:
				cmd_saveable = 0;
				script(1);
				break;
			case 54:
				cmd_saveable = 0;
				script(0);
				break;
			case 58:         /* INSTRUCTIONS */
				agt_clrscr();
				print_instructions(hold_fc);
				close_ins_file();
				break;
			case (OLD_VERB+1):
				cmd_saveable = 0; /* RESTART */
				if (restart_state == NULL)
					writeln("Sorry, too little memory to support RESTART.");
				else {
					doing_restore = 2;
					return;
				}
				break;
			case (OLD_VERB+4):
				cmd_saveable = 0;  /* NOTIFY */
				notify_flag = !notify_flag;
				if (notify_flag) writeln("Score notification is now on.");
				else writeln("Score notification is now off.");
				break;
			case (OLD_VERB+5):
				listexit_flag = 1;
				writeln("[LISTEXIT mode on: room exits will be listed.]");
				break; /* LISTEXIT ON */
			case (OLD_VERB+6):
				listexit_flag = 0;
				writeln("[LISTEXIT mode off: room exits will not be listed.]");
				break;
			case (OLD_VERB+7):  /* AGILDEBUG */
				if (debug_mode) get_debugcmd();
				else writeln("Nice try.");
				break;
			case (OLD_VERB+8): /* LOG, LOG ON */
				logon();
				break;
			case (OLD_VERB+9): /* LOG OFF */
				if (logflag & 2) break; /* We're replaying; ignore. */
				if (logflag & 1) close_pfile(log_out, 5);
				logflag = 0;
				break;
			case (OLD_VERB+10): /* REPLAY n */
				fast_replay = 0;
				replay(dobj_rec->num);
				break;
			case (OLD_VERB+11): /* REPLAY STEP */
				fast_replay = 0;
				replay(-1);
				break;
			case (OLD_VERB+13): /* REPLAY FAST */
				fast_replay = 1;
				replay(0);
				break;
			case (OLD_VERB+12): /* MENU */
				if (verbmenu == NULL) {
					writeln("Sorry, but menus are not supported by this game.");
					menu_mode = 0;
					break;
				}
				if (freeze_mode) {
					writeln("Sorry, but that is not allowed.");
					break;
				}
				menu_mode = !menu_mode;
				break;
			case 57: /* AFTER ?!? */
				writeln("INTERNAL ERROR: Invalid execution of AFTER");
				break;
			case (OLD_VERB+14): /* SOUND ON */
				musiccmd(8, 0);
				break;
			case (OLD_VERB+15): /* SOUND OFF */
				musiccmd(9, 0);
				break;
			case (OLD_VERB+16):  /* INTRO */
				agt_clrscr();
				print_descr(intro_ptr, 1);
				break;
			default:
				sysmsg(185, "Don't know how to $verb$ here...");
				return;
			}
	}

	compute_seen();

	if (!PURE_AFTER && !doing_restore && end_of_turn)
		increment_turn();

	beforecmd = 0;

	/* In AGT 1.8x, run aftercommand verb metacommands. */
	/* (This is the most serious flaw in 1.82/1.83; it drastically changes the
	   semantics of metacommand execution from the earlier formats) */
	if (TWO_CYCLE && !quitflag && !turndone && !deadflag) {
		if (DEBUG_AGT_CMD)
			debugout("*** Scanning (after) metacommands ****\n");
		/* Normal treatment */
		turndone = turndone || metacommand_cycle(save_vb, &redir_flag);
	}

	if (aver >= AGT15 && !quitflag && !endflag && !deadflag) {
		if (DEBUG_AGT_CMD)
			debugout("*** Scanning: AFTER metacommands ****\n");
		/* AFTER metacommands: */
		supress_debug = !debug_any;
		clear_stack();
		if ((PURE_METAVERB || !was_metaverb) &&
		        2 == scan_metacommand(0, 57, 0, 0, 0, NULL))
			turndone = 1;
		supress_debug = 0;
	}

	/* If the player really typed 'q' and we generated an "EndGame"
	   metacommand, then really quit. (usually it just gives the
	   "restart, restore, undo, quit..." message */
	if (save_vb == 30 && endflag) quitflag = 1;
}







/* We need to be able to handle both NOUN and OBJECT searches */
/* If obj==0, then we are doing a noun search, otherwise we are doing
  an object search */
/* Return the disambiguation score;
     0 if the object doesn't trigger anything
     1000 if it runs an action token or built in verb.
     Other values may be returned if an ErrMessage token is encountered.
     500 is the cutoff for ALL expansion.
  */


int objcheck_cycle(rbool *success, parse_rec *act, int verbid,
                   parse_rec *dorec, word prep_, parse_rec *iorec) {
	int result;

	actor = act->obj;
	actor_rec = copy_parserec(act);
	/* The xobj_rec don't really matter */
	dobj = dorec->obj;
	dobj_rec = copy_parserec(dorec);
	if (iorec == NULL) {
		iobj_rec = make_parserec(0, NULL);
		iobj = 0;
	} else {
		iobj = iorec->obj;
		iobj_rec = copy_parserec(iorec);
	}

	clear_stack();
	*success = 1;
	supress_debug = !debug_disambig;
	if (actor != 0 && aver < AGX00) {
		result = scan_metacommand(2, verbid, dobj, prep_, iobj, NULL);
		if (result == 2) {
			free_all_parserec();
			return disambig_score;
		}
		if (result == -2) {
			free_all_parserec();
			return DISAMBIG_SUCC;
		}
	}
	clear_stack();
	result = scan_metacommand(actor, verbid, dobj, prep_, iobj, NULL);
	supress_debug = 0;
	switch (result) {
	case -2:
		free_all_parserec();
		return DISAMBIG_SUCC; /* We matched with something */
	case 0:
	case 1:
		break;  /* Nothing matched, but we still need to check
              built-in verbs */
	case 2:
		free_all_parserec();
		return disambig_score; /* End of turn, no match */
	default:
		writeln("INTERNAL ERROR: Invalid scan_metacommand return value.");
	}
	*success = 0;
	free_all_parserec();
	return 0;
}



int check_obj(parse_rec *act, int verbid,
              parse_rec *dorec, word prep_, parse_rec *iorec) {
	int result;
	rbool success;

	if (iorec == NULL)
		do_disambig = 1; /* Disambiguating dobj */
	else
		do_disambig = 2; /* Disambiguating iobj */

	disambig_score = 0;
	if (have_meta) {
		beforecmd = 1;
		result = objcheck_cycle(&success, act, verbid, dorec, prep_, iorec);
		if (success) return result;
	}

	/* Check built-in verbs here */
	if (verbid < BASE_VERB)
		switch (verbid) {
		case 14: /* THROW dobj prep_ iobj */
		case 29: /* PUT dobj prep_ iobj */
			if (do_disambig == 2 && genvisible(iorec)) return DISAMBIG_SUCC;
		// fallthrough
		case 41: /* DROP */
			if (do_disambig == 1 && it_possess(dobj)) return DISAMBIG_SUCC;
			break;

		case 49: /* SHOOT ... AT or WITH ... */
			if (prep_ == ext_code[wwith]) {
				if (do_disambig == 1 && tcreat(dobj)) return DISAMBIG_SUCC;
				else if (do_disambig == 2 && it_possess(iobj) && tnoun(iobj)
				         && noun[iobj - first_noun].shootable)
					return DISAMBIG_SUCC;
			} else {         /* prep_!=wwith */
				if (do_disambig == 2 && tcreat(iobj)) return DISAMBIG_SUCC;
				else if (do_disambig == 1 && it_possess(dobj) && tnoun(dobj)
				         && noun[dobj - first_noun].shootable)
					return DISAMBIG_SUCC;
			}
			break;

		case 26: /* ATTACK ... WITH ... */
			if (do_disambig == 2 && it_possess(iobj)) return DISAMBIG_SUCC;
			if (do_disambig == 1 && tcreat(dobj) && visible(dobj))
				return DISAMBIG_SUCC;
			break;

		case 51:  /* WEAR */
			if (do_disambig == 1)
				if (tnoun(dobj) && visible(dobj) && noun[dobj - first_noun].wearable
				        && it_loc(dobj) != 1000)
					return DISAMBIG_SUCC;
			break;
		case 33: /* GET */
			if (do_disambig == 1 && tnoun(dobj)
			        && visible(dobj)
			        && noun[dobj - first_noun].location != 1
			        && noun[dobj - first_noun].movable)
				return (player_has(dobj)) ? 499 : DISAMBIG_SUCC;
			break;
		case 52:  /* REMOVE */
			if (do_disambig == 1 && it_loc(dobj) == 1000) return DISAMBIG_SUCC;
			break;

		/* The following could be better, but I don't want to give
		away puzzles by accident */
		case 15: /* OPEN */
		case 17: /* LOCK */
		case 18: /* UNLOCK */
			if (do_disambig == 2 && it_possess(iobj)) return DISAMBIG_SUCC;
		/* ... fall through ... */
		default: /* All other verbs just use visibility check */
			if (do_disambig == 1 && genvisible(dorec)) return DISAMBIG_SUCC;
			if (do_disambig == 2 && genvisible(iorec)) return DISAMBIG_SUCC;
		}

	if (have_meta && TWO_CYCLE) {
		beforecmd = 0;
		result = objcheck_cycle(&success, act, verbid, dorec, prep_, iorec);
		if (success) return result;
	}

	return disambig_score; /* Failed to find a match */
}

} // End of namespace AGT
} // End of namespace Glk
