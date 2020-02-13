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

/*                                                                  */
/* This module contains most of the routines that implement the     */
/*   "physics" of the AGT world, including the scope routines.      */


/* ------------------------------------------------------------------- */
/*  Functions for manipulating parse_recs */
/* ------------------------------------------------------------------- */

/* Make artificial parse record for an object */
parse_rec *make_parserec(int obj, parse_rec *rec) {
	if (rec == NULL) rec = (parse_rec *)rmalloc(sizeof(parse_rec));
	rec->obj = obj;
	rec->info = D_NOUN;
	rec->noun = it_name(obj);
	rec->adj = it_adj(obj);
	rec->num = 0;
	return rec;
}

/* This is used by the parser to initialize a blank parse_rec */
void tmpobj(parse_rec *objrec) {
	objrec->info = D_NOUN;
	objrec->num = 0;
	objrec->noun = objrec->adj = 0;
	objrec->obj = 0;
}

parse_rec *copy_parserec(parse_rec *rec) {
	parse_rec *newrec;
	if (rec == NULL) return NULL;
	newrec = (parse_rec *)rmalloc(sizeof(parse_rec));
	memcpy(newrec, rec, sizeof(parse_rec));
	return newrec;
}

void free_all_parserec(void) {
	rfree(actor_rec);
	rfree(dobj_rec);
	rfree(iobj_rec);
}


/* ------------------------------------------------------------------- */
/* Functions for doing basic manipulation of items and extracting      */
/* nformation about these items                                         */
/* (often used to blackbox the difference between nouns and creatures) */



rbool matchclass(int obj, int oclass) {
	int i;
	if (oclass == 0) return 0;
	for (i = obj; i != oclass && i != 0; i = it_class(i));
	return i == oclass;
}


/* Functions for getting bascic information about items */

static const char *it_sdesc(int item) {
	if (tnoun(item)) return noun[item - first_noun].shortdesc;
	if (tcreat(item)) return creature[item - first_creat].shortdesc;
	if (item < 0) return dict[-item];
	return NULL;
}

rbool it_possess(int item) {
	int l;

	l = it_loc(item);
	return (l == 1 || l == 1000);
}

rbool it_proper(int item) {
	if (tcreat(item))
		return (!PURE_PROPER) || creature[item - first_creat].proper;
	if (tnoun(item))
		return noun[item - first_noun].proper;
	return 0;
}


static rbool invischeck(const char *s) {
	while (rspace(*s)) s++;
	return strncasecmp(s, "INVISIBLE", 9) == 0;
}


static rbool it_invisible(int item, rbool sdesc_flag) {
	if (sdesc_flag)
		return invischeck(it_sdesc(item));
	else {
		char *s;
		rbool tmp;

		if (it_name(item) == 0 && it_adj(item) == 0) return 1;
		s = objname(item); /* Must remember to rfree s before exiting */
		tmp = invischeck(s);
		rfree(s);
		return tmp;
	}
}


static rbool it_appears_empty(int item) {
	int i;
	int sdesc_flag;

	if (item < 0) return 1;
	sdesc_flag = !player_has(item);

	contloop(i, item)
	if (!it_invisible(i, sdesc_flag)) return 0;
	return 1;
}

/* We classify something as a weapon if it kills something. */
rbool it_isweapon(int objnum) {
	int i;

	creatloop(i)
	if (matchclass(objnum, creature[i].weapon)) return 1;
	return 0;
}

/* This used to be a macro like troom, tnoun, and tcreat, but it
   got too complicated and isn't used in time-critical areas,
   anyhow */

rbool it_door(int obj, word nword) {
	if (aver >= AGX00) return 0; /* No doors under Magx */
	if (tdoor(obj)) return 1; /* The basic door */
	if (it_loc(obj) == loc + first_room) return 0;
	return (nword == ext_code[wdoor]);
}


/* ------------------------------------------------------------------- */
/* Routines that manipulate the linked lists representing containment  */
/*  information   */

static void set_contents(int p, int newval) {
	if (troom(p)) room[p - first_room].contents = newval;
	else if (p == 1) player_contents = newval;
	else if (p == 1000) player_worn = newval;
	else if (tnoun(p)) noun[p - first_noun].contents = newval;
	else if (tcreat(p)) creature[p - first_creat].contents = newval;
	else {
		writeln("INT ERR: Invalid object heading chain.");
		return;
	}
}

static void set_next(int p, int newval) {
	if (tnoun(p)) noun[p - first_noun].next = newval;
	else if (tcreat(p)) creature[p - first_creat].next = newval;
	else {
		writeln("INT ERR: Invalid object in chain.");
		return;
	}
}

void add_object(int loc_, int item) {
	int p, q;

	set_next(item, 0);

	if (loc_ == 0) return;
	p = it_contents(loc_);

	if (p == 0 || p > item) {
		set_contents(loc_, item);
		set_next(item, p);
	} else {  /* Figure out where to put the item */
		do {
			q = p;
			p = it_next(p);
		} while (p != 0 && p < item);

		set_next(q, item);
		set_next(item, p);
	}
}


static void set_location(int item, int newloc)
/* This routine assumes item is either a noun or a creature */
{
	int p, q;

	p = it_loc(item);
	if (p != 0) {  /* Fix .next values */
		q = it_contents(p);
		if (q == item) set_contents(p, it_next(item));
		else {
			while (q != item && q != 0) {
				p = q;
				q = it_next(p);
			}
			assert(q != 0); /* This would mean the list structure was corrupted */
			set_next(p, it_next(item));
		}
	}
	/* We've unlinked it from the list at this point. */

	if (tnoun(item))
		noun[item - first_noun].location = newloc;
	else if (tcreat(item))
		creature[item - first_creat].location = newloc;

	add_object(newloc, item);
}


void it_reposition(int item, int newloc, rbool save_pos) {
	integer i;

	if (tnoun(item)) {
		if (player_has(item)) totwt -= noun[item - first_noun].weight;
		if (it_loc(item) == 1) totsize -= noun[item - first_noun].size;

		/* Set position to NULL */
		if (!save_pos) {
			noun[item - first_noun].pos_prep = 0;
			noun[item - first_noun].pos_name = 0;
			noun[item - first_noun].nearby_noun = 0;
			noun[item - first_noun].position = NULL;
#if 0  /* I think this was wrong, so I'm commenting it out. */
			noun[item - first_noun].initdesc = 0;
#endif
		}

		set_location(item, newloc);

		if (player_has(item)) {
			totwt += noun[item - first_noun].weight;
			if (noun[item - first_noun].win)
				winflag = 1;
		}
		if (it_loc(item) == 1) /* only things you are carrying directly count vs.
              size limit. */
			totsize += noun[item - first_noun].size;
	} else if (tcreat(item)) {
		if (newloc == 0) {
			creature[item - first_creat].timecounter = 0; /* Reset attack counter */
			creature[item - first_creat].counter = 0;
		}
		set_location(item, newloc);
	}

	nounloop(i)
	if (noun[i].nearby_noun == item) {
		noun[i].nearby_noun = 0;
		noun[i].pos_prep = 0;
		noun[i].pos_name = 0;
		noun[i].position = NULL;
	}
}



/* ------------------------------------------------------------------- */
/*  Routines to deal with size and weight      */

static long contsize(integer obj) {
	int i;
	long net;

	net = 0;
	contloop(i, obj) {
		if (tnoun(i))
			net += noun[i - first_noun].size;
		if (aver < AGX00) /* Under Magx, size isn't recursive */
			net += contsize(i);
	}
	return net;
}

static long contweight(integer obj) {
	int i;
	long net;

	net = 0;
	contloop(i, obj) {
		if (tnoun(i))
			net += noun[i - first_noun].weight;
		net += contweight(i);
	}
	return net;
}

rbool is_within(integer obj1, integer obj2, rbool stop_if_closed)
/* True if obj1 is contained in obj2 */
{
	int i;
	long cnt;

	for (i = obj1, cnt = 0;
	        i != obj2 && i >= maxroom && i != 1000 && cnt < 40000L;
	        cnt++) {
		i = it_loc(i);
		if (stop_if_closed && !it_open(i)) break;
	}
	if (cnt >= 40000L) {
		/* writeln("GAME ERROR: Loop in object tree.");*/
		return 0;
	}
	if (i == obj2) return 1;
	return 0;
}



int check_fit(int obj1, int obj2)
/* Does obj1 fit inside obj2?  Return one of the FIT_... values
defined in exec.h */
{
	int size, weight;
	long net;

	assert(tnoun(obj1)); /* This should have been checked earlier */
	if (obj2 == 1000) obj2 = 1;

	if (obj2 == 1) size = weight = 100;
	else {
		assert(tnoun(obj2)); /* check_fit shouldn't be called otherwise */
		size = noun[obj2 - first_noun].size;
		weight = noun[obj2 - first_noun].weight;
	}

	/* Weight */
	if (obj2 == 1 || (aver > AGTME15 && aver < AGX00)) {
		/* Pre-1.56 interpreters forgot to check this;
		   Magx deliberatly *doesn't* check this */

		net = noun[obj1 - first_noun].weight;
		if (aver >= AGX00) net += contweight(obj1);
		if (net > weight) return FIT_WEIGHT;

		if (obj2 == 1) {
			if (is_within(obj1, 1, 0) || is_within(obj1, 1000, 0)) net = 0;
			net += contweight(1);
			if (aver >= AGX00)
				net += contweight(1000);
			if (!PURE_SIZE) net = 0;
		} else {
			if (is_within(obj1, obj2, 0)) net = 0; /* Avoid double-counting */
			net += contweight(obj2); /* Net size of contents of obj2 */
		}
		if (net > weight) return FIT_NETWEIGHT;
	}

	net = noun[obj1 - first_noun].size;
	if (net > size) return FIT_SIZE;

	if (obj2 == 1 && !PURE_SIZE) return FIT_OK;

	if (obj2 == 1 || aver > AGTME15) {
		/* Pre-ME/1.56 interpreters didn't check this except for the player's
		   inventory */
		if (it_loc(obj1) == obj2
		        || (aver < AGX00 && is_within(obj1, obj2, 0)))
			net = 0; /* Avoid double-counting */
		net += contsize(obj2); /* Net size of contents of obj2 */
		if (net > size) return FIT_NETSIZE;
	}

	return FIT_OK;
}



/* ------------------------------------------------------------------- */
/*  Scope and visibility routines                                      */

integer it_room(int item) {
	int tmploc;
	long cnt;

	cnt = 0;
	while (!troom(item)) {
		tmploc = item;
		if (item == 0) return 0;
		if (item == 1 || item == 1000) item = loc;
		else item = it_loc(item);
		if (item == tmploc || ++cnt >= 40000L) {
			/* writeln("GAME ERROR: Loop in object tree."); */
			return 0;
		}
	}
	return item;
}

rbool player_has(int item) {
	return is_within(item, 1, 0) || is_within(item, 1000, 0);
}


rbool in_scope(int item)
/* strictly speaking, visible actually checks scope; this routine
   determines if an object would be in scope if there were no light
   problems. */
{
	int curloc;
	int tmp;
	long cnt;

	if (it_isglobal(item)) return 1;  /* Global objects always in scope. */

	/* Flag objects in scope if their associated flag is set. */
	tmp = it_flagnum(item);
	if (tmp &&
	        (room[loc].flag_noun_bits & (1L << (tmp - 1)))) return 1;

	curloc = it_loc(item); /* Should work for nouns or creatures */
	cnt = 0;
	while (curloc > maxroom && curloc != 1000 && it_open(curloc)) {
		int tmploc;
		tmploc = it_loc(curloc);
		if (tmploc == curloc || ++cnt >= 40000L) {
			/* writeln("GAME ERROR: Loop in the object tree."); */
			return 0;
		} else curloc = tmploc;
	}
	if (curloc == 1 || curloc == 1000 || curloc == loc + first_room) return 1;
	else return 0;
}


static int good_light(int obj, int roomlight, rbool active)
/* obj is a noun number */
/* If active is false, we don't care if the light is actually turned
on is the valid light */
{
	if (roomlight == 1 && !noun[obj].light)
		return 0; /* obj is not a light source */
	if (roomlight > 1) {
		if (!matchclass(first_noun + obj, roomlight))
			return 0; /* Not the correct light */
		else return 1; /* The correct light _always_ illuminates the room */
	}
	if (!active) return 1;
	/* Now need to determine if obj is actually providing light */
	if (!noun[obj].on)
		return 0;  /* Light source is off or extinguished */
	return 1;
}

int lightcheck(int parent, int roomlight, rbool active)
/* This checks to see if anything contained in parent is a valid
room light */
/* active=1 means that we only want active lights;
active=0 indicates that extinguished light sources are okay. */
{
	int i;

	contloop(i, parent) {
		if (tnoun(i) && good_light(i - first_noun, roomlight, active)) return 1;
		if (it_open(i) && lightcheck(i, roomlight, active))
			return 1; /* Check children */
	}
	return 0;
	/*
	    nounloop(i)
	      if (good_light(i,room[loc].light) && in_scope(i+first_noun))
	    return 1;
	  return 0;*/
}


rbool islit(void) {
	if (room[loc].light == 0) return 1;
	if (lightcheck(loc + first_room, room[loc].light, 1)) return 1;
	if (lightcheck(1, room[loc].light, 1)) return 1; /* Player carried light */
	if (lightcheck(1000, room[loc].light, 1)) return 1; /* Worn light */
	return 0;
}

/* Is item visible to player? */
/* visible only works for "normal" items; if the object could
   be a virtual object (i.e. with negative item number), then use
   gen_visible() below */
rbool visible(int item) {
	assert(item >= 0);
	if (islit())
		return in_scope(item);
	else
		return player_has(item);
}

rbool genvisible(parse_rec *dobj_) {
	int i;

	if (dobj_->obj > 0) return visible(dobj_->obj);

	if (dobj_->info == D_INTERN) {
		if (dobj_->obj != -ext_code[wdoor]) return 1;
		return islit(); /* If item is a is a door */
	}
	if (dobj_->info == D_GLOBAL || dobj_->info == D_NUM) return 1;
	if (dobj_->info == D_FLAG) {
		for (i = 0; i < MAX_FLAG_NOUN; i++) /* Flag nouns */
			if (flag_noun[i] != 0 && dobj_->obj == -flag_noun[i]
			        && (room[loc].flag_noun_bits & (1L << i)) != 0)
				return 1;
		return 0;
	}
	if (dobj_->info == D_PIX) {
		for (i = 0; i < MAX_PIX; i++) /* PIX names */
			if (pix_name[i] != 0 && dobj_->obj == -pix_name[i] &&
			        (room[loc].PIX_bits & (1L << i)) != 0)
				return 1;
		return 0;
	}
	fatal("INTERNAL ERROR: Invalid gen_visible type.");
	return 0;
}




/* Need to find a noun related to w */
/* If there is an object with name w in scope, it returns 0
   (since the object will have already been added to the menu).
   if there are none, it returns the first object with name w. */
static integer find_related(word w) {
	int i;
	int item;

	if (w == 0) return 0;
	item = 0;
	nounloop(i)
	if (noun[i].name == w) {
		if (visible(i + first_noun)) return i + first_noun;
		else if (item == 0) item = i + first_noun;
	}
	creatloop(i)
	if (creature[i].name == w) {
		if (visible(i + first_creat)) return i + first_creat;
		else if (item == 0) item = i + first_creat;
	}
	return item;
}


static void add_to_scope(integer item) {
	integer i;

	if (tnoun(item)) {
		noun[item - first_noun].scope = 1;
		i = find_related(noun[item - first_noun].related_name);
		if (i != 0) {
			if (tnoun(i)) noun[i - first_noun].scope = 1;
			else if (tcreat(i)) creature[i - first_creat].scope = 1;
		}
	} else if (tcreat(item)) creature[item - first_creat].scope = 1;
	if (item == 1 || item == 1000 || troom(item) || it_open(item))
		contloop(i, item)
		add_to_scope(i);
}


void compute_scope(void) {
	int i;
	uint32 rflag;

	nounloop(i) noun[i].scope = 0;
	creatloop(i) creature[i].scope = 0;
	add_to_scope(1);
	add_to_scope(1000);
	add_to_scope(loc + first_room);
	rflag = room[loc].flag_noun_bits;
	nounloop(i)
	if (noun[i].isglobal ||
	        (noun[i].flagnum && (rflag & (1L << (noun[i].flagnum - 1)))))
		add_to_scope(i + first_noun);
	creatloop(i)
	if (creature[i].isglobal ||
	        (creature[i].flagnum && (rflag & (1L << (creature[i].flagnum - 1)))))
		add_to_scope(i + first_creat);
}

void compute_seen(void) {
	int i;

	compute_scope();
	nounloop(i)
	noun[i].seen = noun[i].seen || noun[i].scope;
	creatloop(i)
	creature[i].seen = creature[i].seen || creature[i].scope;
}


/*---------------------------------------------------------------------*/
/*  Routines to compute the score */

void recompute_score(void) {
	int obj;

	tscore -= objscore;
	objscore = 0;
	nounloop(obj)
	if (noun[obj].points && !noun[obj].unused &&
	        (visible(obj + first_noun)
	         || is_within(obj + first_noun, treas_room, 0)))
		objscore += noun[obj].points;
	creatloop(obj)
	if (!creature[obj].unused && creature[obj].points
	        && visible(obj + first_creat))
		objscore += creature[obj].points;
	tscore += objscore;
}



/*---------------------------------------------------------------------*/
/*  Menu Noun section: routines to get a list of 'relevant' nouns for  */
/*    the menuing system. They're here because they belong next to the */
/*    scope routines, above   */

static int *nlist, nleng; /* These are really local variables */

static void add_mnoun(int n) {
	nlist = (int *)rrealloc(nlist, (nleng + 2) * sizeof(int));
	nlist[nleng] = n;
	nlist[++nleng] = 0;
}


/* This adds mitem and everything it contains */
static void add_mitem(int item) {
	integer i;

	if (tnoun(item) || tcreat(item)) add_mnoun(item);
	if (item == 1 || item == 1000 || troom(item) || it_open(item))
		contloop(i, item)
		add_mitem(i);
	/* Need to check related nouns */
	if (tnoun(item)) {
		i = find_related(noun[item - first_noun].related_name);
		if (i != 0) add_mnoun(i);
	}
}


static word getword(int item, int n)
/* Gets nth word associated with item */
{
	if (n == 1) {
		if (item < 0) return -item;
		else if (tnoun(item)) return noun[item - first_noun].adj;
		else if (tcreat(item)) return creature[item - first_creat].adj;
	}
	if (n == 2)
		if (tnoun(item) || tcreat(item)) return it_name(item);
	return 0;
}

static int cmp_nouns(const void *a, const void *b)
/*   *a, *b are object numbers; need alphabetic sort.*/
{
	word wa, wb;
	int cmp;

	wa = getword(*((const int *)a), 1);
	wb = getword(*((const int *)b), 1);
	cmp = strcmp(dict[wa], dict[wb]);
	if (cmp != 0) return cmp;
	wa = getword(*(const int *)a, 2);
	wb = getword(*(const int *)b, 2);
	return strcmp(dict[wa], dict[wb]);
}

int *get_nouns(void)
/* This returns the list of all objects that should show up on the menu */
/* The list should be 0 terminated and needs to be sorted */
{
	int i;
	uint32 rflag;

	nlist = (int *)rmalloc(sizeof(int));
	nlist[0] = 0;
	nleng = 0;

	for (i = 0; i < numglobal; i++)
		add_mnoun(-globalnoun[i]);
	for (i = 0; i < MAX_FLAG_NOUN; i++)
		if (room[loc].flag_noun_bits & (1L << i))
			add_mnoun(-flag_noun[i]);
	add_mitem(1);
	add_mitem(1000);
	add_mitem(loc + first_room);
	rflag = room[loc].flag_noun_bits;
	nounloop(i)
	if (noun[i].isglobal ||
	        (noun[i].flagnum && (rflag & (1L << (noun[i].flagnum - 1)))))
		add_mitem(i + first_noun);
	creatloop(i)
	if (creature[i].isglobal ||
	        (creature[i].flagnum && (rflag & (1L << (creature[i].flagnum - 1)))))
		add_mitem(i + first_creat);
	qsort(nlist, nleng, sizeof(int), cmp_nouns);
	return nlist;
}



/*---------------------------------------------------------------------*/
/* goto_room, the basic primitive used to move the player around       */

void goto_room(int newroom) {
	int i, j;

	/* Move group members in old room to new room */
	safecontloop(i, j, loc + first_room)
	if (it_group(i))
		it_move(i, newroom + first_room);

#if 0  /* -- this has been moved to v_go*/
	if (loc != newroom)
		oldloc = loc; /* Save old location for NO_BLOCK_HOSTILE purposes */
#endif
	loc = newroom;
	if (loc != newroom) oldloc = loc; /* No backtracking unless v_go allows it */
	if (!room[loc].seen) {
		room[loc].seen = 1;
		tscore += room[loc].points;
		first_visit_flag = 1;
		room_firstdesc = 1;
		v_look();
	} else {
		first_visit_flag = 0;
		if (verboseflag)
			v_look(); /* But see v_go() for a special case involving SPECIAL */
		room_firstdesc = 0;
	}
	if (room[loc].end) endflag = 1;
	if (room[loc].win) winflag = 1;
	if (room[loc].killplayer) deadflag = 1;
	do_autoverb = 1;
	set_statline();
}


static void rundesc(int i, descr_ptr dp_[], const char *shortdesc, int msgid) {
	if (dp_[i].size > 0)
		print_descr(dp_[i], 1);
	else if (!invischeck(shortdesc))
		raw_lineout(shortdesc, 1, MSG_DESC, NULL);
	else sysmsg(msgid, "$You$ see nothing unexpected.");
}

void it_describe(int dobj_) {
	if (troom(dobj_))
		print_descr(room_ptr[dobj_ - first_room], 1);
	else if (tnoun(dobj_))
		rundesc(dobj_ - first_noun, noun_ptr, noun[dobj_ - first_noun].shortdesc, 194);
	else if (tcreat(dobj_))
		rundesc(dobj_ - first_creat, creat_ptr,
		        creature[dobj_ - first_creat].shortdesc, 195);
	else if (dobj_ == -ext_code[wdoor]) { /* i.e. DOOR */
		if (room[loc].locked_door)
			sysmsg(21, "$You$ see a locked door.");
		else sysmsg(22, "$You$ see a perfectly normal doorway.");
	} else sysmsg(194, "$You$ see nothing unexpected.");
	if (tnoun(dobj_) &&
	        (noun[dobj_ - first_noun].open || !noun[dobj_ - first_noun].closable) &&
	        !it_appears_empty(dobj_)) {
		sysmsg(228, "Which contains:");
		print_contents(dobj_, 1);
	}
}



static char *build_position(word prep_, word name)
/* Return the malloc'd string '$prep_$ the $name$' */
{
	int leng;
	char *s;

	leng = strlen(dict[prep_]) + strlen(dict[name]) + 6; /* includes final '\0' */
	s = (char *)rmalloc(leng * sizeof(char));

	strcpy(s, dict[prep_]);
	strcat(s, " the ");
	strcat(s, dict[name]);
	assert((int)strlen(s) + 1 == leng);
	return s;
}






static int print_obj(int obj, int ind_lev)
/* Prints out s on a line of its own if obj isn't INVISIBLE */
/* parent_descr is true if the parent has been described, false
   otherwise (say if the parent is invisible). */
/* ind_lev=indentation level */
/* Return 1 if we actually printed something, 0 if obj is invisible */
{
	int sdesc_flag; /* True if should print out as sdesc rather than
        as adjective-noun */
	int i, retval, parent;
	const char *s;
	char *t, *s0,  *posstr;

	if (tcreat(obj) && creature[obj - first_creat].initdesc != 0)
		return 0; /* Don't print normal description if printing initdesc */

	s0 = NULL;
	sdesc_flag = !player_has(obj); /* This should be tested. */
	sdesc_flag = sdesc_flag || (ind_lev > 1);  /* It seems that AGT uses the
                        sdesc for describing items
                        contained in other items */
	/* Some code below relies on this, as well */

	if (sdesc_flag)
		s = it_sdesc(obj);
	else if (it_name(obj) == 0 && it_adj(obj) == 0) /* Invisible */
		return 0;
	else {
		s0 = objname(obj); /* Must remember to rfree s before exiting */
		if (aver >= AGTME10) {
			for (t = s0; isspace(*t); t++); /* Skip over initial whitespace... */
			*t = toupper(*t); /* ...and upcase the first non-space character */
		}
		s = s0;
	}

	retval = 0;
	if (sdesc_flag && tnoun(obj) && noun[obj - first_noun].initdesc != 0) {
		retval = 1;
		msgout(noun[obj - first_noun].initdesc, 1);
		noun[obj - first_noun].initdesc = 0; /* Only show it once */
	} else if (!invischeck(s)) {
		retval = 1; /* We're actually going to print something */
		for (i = 0; i < ind_lev; i++) writestr("   ");
		raw_lineout(s, sdesc_flag, MSG_DESC, NULL);
		/* Do $word$ formatting if sdesc */
		/* Need to output container */
		parent = it_loc(obj);
		if (tnoun(obj) && noun[obj - first_noun].pos_prep != 0) {
			writestr(" (");
			if (noun[obj - first_noun].pos_prep == -1)
				writestr(noun[obj - first_noun].position);
			else  {
				posstr = build_position(noun[obj - first_noun].pos_prep,
				                        noun[obj - first_noun].pos_name);
				writestr(posstr);
				rfree(posstr);
			}
			writestr(")");
		} else if (parent >= first_noun && it_invisible(parent, sdesc_flag)
		           && (it_name(parent) != 0 || it_adj(parent) != 0)) {
			/* If the parent object *isn't* invisible, we will already have
			printed it out */
			/* This also relies on sdesc_flag being the same for parent
			and child objects */

			if (parent >= first_creat && parent <= maxcreat)
				sysmsg(221, "(Carried by");
			else
				sysmsg(222, " (Inside");
			t = objname(parent);
			writestr(t);
			rfree(t);
			sysmsg(223, ")");
		}
		if (tnoun(obj) && noun[obj - first_noun].light && noun[obj - first_noun].on
		        && PURE_OBJ_DESC)
			sysmsg(220, " (Providing light)");
		writeln("");
	}
	if (!sdesc_flag)
		rfree(s0);
	return retval;
}


int print_contents(int obj, int ind_lev)
/* obj=object to list contents of; ind_lev=indentation level */
/* Returns number of objects contained in obj that were listed */
{
	int i, cnt;

	cnt = 0;

	contloop(i, obj) {
		if (print_obj(i, ind_lev)) cnt++;
		if (it_open(i)) print_contents(i, ind_lev + 1);
	}
	return cnt;
}


/* ------------------------------------------------------------------- */
/*  Routines for directly getting and setting object properties and   */
/*   attributes. */


#define NUM_WPROP 6
#define NUM_WATTR 6


static void *compute_addr(int obj, int prop, const prop_struct *ptable) {
	int ofs;
	void *base;

	if (DIAG)
		rprintf("(Accessing %s->%s)\n", dict[it_name(obj)], ptable[prop].name);
	if (troom(obj)) {
		base = (void *)(&room[obj - first_room]);
		ofs = ptable[prop].room;
	} else if (tnoun(obj)) {
		base = (void *)(&noun[obj - first_noun]);
		ofs = ptable[prop].noun;
	} else if (tcreat(obj)) {
		base = (void *)(&creature[obj - first_creat]);
		ofs = ptable[prop].creature;
	} else return NULL;

	if (ofs == -1) /* Field doesn't exist in this type of object */
		return NULL;

	return (void *)(((char *)base) + ofs);
}


long getprop(int obj, int prop) {
	integer *paddr;

	if (prop >= NUM_PROP) return 0;
	paddr = (integer *)compute_addr(obj, prop, proplist);
	if (paddr == NULL) return 0;
	return *paddr;
}

void setprop(int obj, int prop, long val) {
	integer *paddr;

	if (prop >= NUM_WPROP) {
		writeln("GAME ERROR: Read-only or non-existant property.");
		return;
	}

	paddr = (integer *)compute_addr(obj, prop, proplist);
	if (paddr == NULL) {
		writeln("GAME ERROR: Property-object mismatch.");
		return;
	}
	*paddr = val;
}

rbool getattr(int obj, int prop) {
	rbool *paddr;

	if (prop >= NUM_ATTR) return 0;
	paddr = (rbool *)compute_addr(obj, prop, attrlist);
	if (paddr == NULL) return 0;
	return *paddr;
}

void setattr(int obj, int prop, rbool val) {
	rbool *paddr;

	if (prop >= NUM_WATTR && prop != 24) {
		writeln("GAME ERROR: Read-only or non-existant attribute.");
		return;
	}

	paddr = (rbool *)compute_addr(obj, prop, attrlist);
	if (paddr == NULL) {
		writeln("GAME ERROR: Property-object mismatch.");
		return;
	}
	*paddr = val;
}



/* ------------------------------------------------------------------- */
/*  This sets up the creat_fix[] array, which is used to determine the */
/*  scan ranges for addressed creatures in cases where there is more */
/*  than one creature of the same name */

void init_creat_fix(void) {
	int i, j;

	creat_fix = (integer *)rmalloc(rangefix(maxcreat - first_creat + 1) * sizeof(integer));
	for (i = 0; i < maxcreat - first_creat + 1; i++)
		creat_fix[i] = i + first_creat;
	for (i = 0; i < maxcreat - first_creat + 1; i++)
		if (creat_fix[i] == i + first_creat) /* That is, it hasn't changed. */
			for (j = i + 1; j < maxcreat - first_creat + 1; j++)
				if (creature[i].name == creature[j].name &&
				        creature[i].adj == creature[j].adj)
					creat_fix[j] = i + first_creat; /* That is, j --> i */
}

void free_creat_fix(void) {
	rfree(creat_fix);
}

/* ------------------------------------------------------------------- */

#ifndef IT_MACRO
int it_contents(integer obj) {
	if (tnoun(obj)) return noun[obj - first_noun].contents;
	else if (troom(obj)) return room[obj - first_room].contents;
	else if (tcreat(obj)) return creature[obj - first_creat].contents;
	else if (obj == 1) return player_contents;
	else if (obj == 1000) return player_worn;
	else return 0;
}

rbool it_lockable(integer obj, word nword) {
	if (tnoun(obj)) return noun[obj - first_noun].lockable;
	else if (it_door(obj, nword)) return 1;
	else return 0;
}

rbool it_locked(integer obj, word nword) {
	if (tnoun(obj)) return noun[obj - first_noun].locked;
	else if (it_door(obj, nword) && room[loc].locked_door) return 1;
	else return 0;
}

#endif

} // End of namespace AGT
} // End of namespace Glk
