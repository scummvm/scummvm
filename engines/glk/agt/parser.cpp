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

namespace Glk {
namespace AGT {

/*

   This is probably the ugliest and least readable of all of the
   source files.  The parser isn't really that complex in principle,
   but it has to deal with a lot of of special cases and also be
   downward-compatible with the original parsers (which sometimes did
   strange things) which adds a lot of additional code.  The noun parsing
   code is particularly convoluted.

   Also, there are a fair number of global variables in this module
   that are really local variables at heart but ended up global
   because they were needed in too many different places and I didn't
   want to complicate things even more by having to explicitly pass
   them all up and down through the parsing routines. (One of those
   times it would be convenient to have the ability to nest procedures
   and functions like Pascal.)

*/

/* These store the previous turn's values for use in disambiguation and
   implementing OOPS */
/* parse_ip saves the ip after parsing, ip_back saves it before. */
word input_back[MAXINPUT];
words in_text_back[MAXINPUT];
int ip_back, parse_ip;

/* The following are global only for rfree() purposes and
   in order to maintain state for disambiguation */
static int vnum;  /* Verb number from synonym scan */

/* Pointers to negative-terminated arrays of possible nouns */
static parse_rec *lactor = NULL, *lobj = NULL, *lnoun = NULL;

static int ambig_flag = 0;
/* Was last input ambiguous? (so player could be entering
disambiguation info?)  1=ambig actor, 2=ambig noun,
3=ambig obj */


/* Empty ALL error messages, for those verbs that have their own */
int all_err_msg[] = {73, 83, 113, 103, /* open, close, lock, unlock: 15 - 18 */
                     239, 239, 239, 239, /* 19 - 22 */
                     123, 125
                    }; /* eat, drink: 23 - 24 */



/*-------------------------------------------------------------------*/
/* DEBUGGING OUTPUT FUNCTIONS & MISC UTILITIES                        */
/*-------------------------------------------------------------------*/

/* This routine frees the space used by the parsing data structures */
static void freeall(void) {
	rfree(lnoun);
	rfree(lobj);
	lnoun = lobj = NULL;
}


/* Print out parse error message and abort the parse */
static int parseerr(int msgid, const char *s, int n) {
	if (n >= 0)
		gen_sysmsg(msgid, s, MSG_PARSE, in_text[n]);
	else
		gen_sysmsg(msgid, s, MSG_PARSE, "");
	freeall();
	ep = n;
	ip = -1;
	return -1;
}


/* Print out noun list; used for debugging the parser */
static void print_nlist(parse_rec *n) {
	char *s;
	int c;
	char buff[100];

	if (n->info == D_END)
		writestr("----");
	if (n->info == D_ALL) {
		writestr("ALL ");
		n++;
	}
	for (c = 0; n->info != D_END && c < 20; n++, c++)
		if (n->info == D_AND) writestr(" AND ");
		else if (n->info == D_NUM) { /* Number entered */
			sprintf(buff, "#%ld(%d); ", n->num, n->obj);
			writestr(buff);
		} else if (n->obj < 0) {
			writestr(dict[-(n->obj)]);
			sprintf(buff, "(%d); ", n->obj);
			writestr(buff);
		} else {
			s = objname(n->obj);
			writestr(s);
			rfree(s);
			sprintf(buff, "(%d) ['%s %s']; ", n->obj, dict[n->adj], dict[n->noun]);
			writestr(buff);
		}
	if (n->info != D_END) writestr("///");
	writeln("");
}

/* Output the parser's analysis of the current input line; used
   for debugging */
static int parse_out(parse_rec *lactor_, int vb_, parse_rec *lnoun_, int prep_,
                     parse_rec *lobj_) {
	writeln("ANALYSIS:");
	writestr("Actor: ");
	print_nlist(lactor_);
	writestr("Verb:");
	writeln(dict[ syntbl[auxsyn[vb_]] ]);
	writestr("DObj: ");
	print_nlist(lnoun_);
	writestr("Prep: ");
	if (prep_ != 0) writeln(dict[prep_]);
	else writeln("---");
	writestr("IObj: ");
	print_nlist(lobj_);
	return 0;
}


static void save_input(void) {
	int i;

	for (i = 0; input[i] != -1 && i < MAXINPUT; i++) {
		input_back[i] = input[i];
		strncpy(in_text_back[i], in_text[i], 24);
	}
	input_back[i] = -1;
	ip_back = ip;
}

static void restore_input(void) {
	int i;

	for (i = 0; input_back[i] != -1 && i < MAXINPUT; i++) {
		input[i] = input_back[i];
		strncpy(in_text[i], in_text_back[i], 24);
	}
	input[i] = -1;
	ip = ip_back;
}



/*-------------------------------------------------------------------*/
/* Misc. Parsing Routines (includes parsing of verbs and preps)      */
/*-------------------------------------------------------------------*/

#define w_and(w) (w==ext_code[wand] || w==ext_code[wc])
#define w_but(w) (w==ext_code[wbut] || w==ext_code[wexcept])
#define w_isterm(w) (w==ext_code[wp] || w==ext_code[wthen] || \
                     w==ext_code[wsc] || w_and(w) || w==-1)



static word check_comb(int combptr) {
	int k;
	word w;

	if (combptr == 0) return 0;
	w = syntbl[combptr];
	for (combptr += 1, k = ip; syntbl[combptr] != 0; combptr++, k++)
		if (syntbl[combptr] != input[k]) break;
	if (syntbl[combptr] == 0) {
		ip = k - 1;
		return w;
	}
	return 0;
}


static int comb_verb(void)
/* This eliminates two-word verbs */
{
	int i;
	word w;

	for (i = 0; i < num_comb; i++) {
		w = check_comb(comblist[i]);
		if (w != 0) return w;
	}

	if (input[ip] == ext_code[wgo] && verb_authorsyn(ext_code[wgo]) == 0) {
		/* GO <dir> --> <dir> */
		w = input[ip + 1];
		if (w != 0) i = verb_builtin(w);
		else i = 0;
		if (i != 0) {
			ip++;
			return w;
		}
	}

	for (i = 0; i < num_auxcomb; i++) {
		w = check_comb(auxcomb[i]);
		if (w != 0) return w;
	}

	return input[ip];
}





/* This return true if the word in question is in the list
   of original AGT verbs, but is not the canonical verb.
   This is needed because verbs on this list
   are not overridden by dummy_verbs in pre-Magx games. */
static rbool orig_agt_verb(word w) {
	int i;
	if (aver <= AGT10 && w == ext_code[wg]) return 0; /* AGT 1.0 didn't have AGAIN */
	for (i = 0; old_agt_verb[i] != -1 && old_agt_verb[i] != w; i++);
	return (old_agt_verb[i] == w);
}

/* A few comments on id_verb:
   The sequence is as follows:
     i) Convert built-in synonyms to the base form (TAKE-->GET, for example)
        _unless_ there is an author-defined synonym. (The original AGT
    didn't have this 'unless'.)
     ii) Check room-synonyms
     iii) ID the verb based first on game-specific synonyms and then
        falling back to the built-in ones.
    (AGT gave the built-in ones higher priority than the game-specific
    ones, but this causes problems and is generally a bad idea.)
 */

static int id_verb(void)
/* Identify verb at ip=i ; return verb id if found, 0 otherwise */
{
	word w;
	int j, canon_word, tmp;

	w = comb_verb(); /* Combine 2-word verbs */
	if (w == 0) return 0;

	/* Pre-Canonization of w: see if w has any built-in synonyms */
	canon_word = verb_builtin(w);
	if (canon_word != 0) {
		if (aver < AGX00 && orig_agt_verb(w))
			/* In orig AGT, author-defined verbs don't override builtin syns */
			tmp = 0;
		else
			tmp = verb_authorsyn(w); /* Author-defined verbs override built-in ones */
		if (tmp == 0 || tmp == canon_word)
			w = syntbl[auxsyn[canon_word]];
	}

	/* Now check room-specific synonyms (being the most localized,
	      they have the highest priority) */
	for (j = room[loc].replacing_word; syntbl[j] != 0; j++)
		if (w == syntbl[j])
			w = room[loc].replace_word;

	/* Next check to see if we already have the canonical form of the verb. */
	/* and go through the built-in list of synonyms */
	canon_word = verb_code(w);
	if (!PURE_DUMMY && canon_word == 57) canon_word = 0; /* AFTER */
	return canon_word;
}


#define compr_prep(w1,w2,r) {if (w1==input[ip] && w2==input[ip+1]) \
		{ip+=2;return r;}}
#define cprep(c1,c2,r)  compr_prep(ext_code[c1],ext_code[c2],ext_code[r])

/* Eventually should add support for the handful of two word preps */
/* (eg IN TO, OUT OF,...); otherwise, this is pretty trivial. */
static int parse_prep(void) {
	int i;
	int j, k;

	for (j = 0; j < num_prep; j++) { /* Scan user-defined prepositions */
		/* This table is formatted like the multi-verb table:
		     end-prep prep-word1 prepword2 etc. */
		for (k = 0; syntbl[userprep[j] + k + 1] != 0; k++)
			if (syntbl[userprep[j] + k + 1] != input[ip + k]) break;
		if (syntbl[userprep[j] + k + 1] == 0) {
			ip += k;
			return syntbl[userprep[j]];
		}
	}
	cprep(win, wto, winto);
	cprep(wout, wof, wfrom);
	for (i = win; i <= wabout; ++i)
		if (ext_code[i] == input[ip]) return input[ip++];
	return 0;
}



static int noun_syn(word w, int obj)
/* Is the word w  a synonym for the object obj? */
/* obj is encoded as ususal. */
/* 0=no match, 1=adjective match, 2=synonym match, 3=noun match */
/* 2 will only occur if PURE_SYN is false */
{
	int i;

	if (w <= 0) return 0;

	if (obj >= first_noun && obj <= maxnoun) {
		obj = obj - first_noun;
		if (w == noun[obj].name) return 3;
		if (noun[obj].has_syns)
			for (i = noun[obj].syns; syntbl[i] != 0; i++)
				if (w == syntbl[i]) return (PURE_SYN ? 3 : 2);
		if (w == noun[obj].adj) return 1;
		return 0;
	}
	if (obj >= first_creat && obj <= maxcreat) {
		obj = obj - first_creat;
		if (w == creature[obj].name) return 3;
		if (creature[obj].has_syns)
			for (i = creature[obj].syns; syntbl[i] != 0; i++)
				if (w == syntbl[i]) return (PURE_SYN ? 3 : 2);
		if (w == creature[obj].adj) return 1;
		return 0;
	}
	return 0; /* If the object doesn't exist, can't have synonyms */
}



/*-------------------------------------------------------------------*/
/*   Noun-list manipulation functions.                               */
/*-------------------------------------------------------------------*/

static parse_rec *new_list(void) {
	parse_rec *list;

	list = (parse_rec *)rmalloc(sizeof(parse_rec));
	list[0].obj = 0;
	list[0].num = 0;
	list[0].adj = list[0].noun = 0;
	list[0].info = D_END;
	return list;
}

static parse_rec *add_w_rec(parse_rec *pold, int obj0, long num0, int info0,
                            word adj0, word noun0) {
	parse_rec *pnew;
	int n;

	for (n = 0; pold[n].info != D_END; n++);
	pnew = (parse_rec *)rrealloc(pold, (n + 2) * sizeof(parse_rec));
	pnew[n].obj = obj0;
	pnew[n].num = num0;
	pnew[n].info = info0;
	pnew[n].adj = adj0;
	pnew[n].noun = noun0;
	pnew[n + 1].obj = 0;
	pnew[n + 1].info = D_END;
	return pnew;
}

static parse_rec *add_rec(parse_rec *old, int obj0, long num0, int info0) {
	word w;

	if (obj0 < 0) w = -obj0;  /* The NOUN field of literal words will just be
               that word */
	else w = 0;
	return add_w_rec(old, obj0, num0, info0, 0, w);
}


static parse_rec *kill_rec(parse_rec *old, int index)
/* Remove record old[index] */
{
	parse_rec *pnew;
	int i;

	for (i = index; old[i].info != D_END; i++) {
		old[i].obj = old[i + 1].obj;
		old[i].num = old[i + 1].num;
		old[i].noun = old[i + 1].noun;
		old[i].adj = old[i + 1].adj;
		old[i].info = old[i + 1].info;
		old[i].score = old[i + 1].score;
	}
	pnew = (parse_rec *)rrealloc(old, i * sizeof(parse_rec)); /* Shrink it by one */
	return pnew;
}

static parse_rec *concat_list(parse_rec *dest, parse_rec *src) {
	int i, j;

	for (i = 0; dest[i].info != D_END; i++); /* Put i at end of first list */
	for (j = 0; src[j].info != D_END; j++); /* j marks end of the second list */
	dest = (parse_rec *)rrealloc(dest, sizeof(parse_rec) * (i + j + 1));
	memcpy(dest + i, src, (j + 1)*sizeof(parse_rec));
	return dest;
}

static parse_rec *purge_list(parse_rec *list)
/* It should be possible to make this more efficiant */
{
	int i;

	for (i = 0; list[i].info != D_END;)
		if ((list[i].info & D_MARK) != 0)
			list = kill_rec(list, i);
		else i++;      /* (Note: we only increment i if we _don't_ kill) */
	return list;
}

static void clean_list(parse_rec *list) {
	for (; list->info != D_END; list++)
		list->info &= ~D_MARK; /* Clear mark */
}


static parse_rec *copy_list(parse_rec *list) {
	parse_rec *cpy;
	int i;

	cpy = new_list();
	for (i = 0; list[i].info != D_END; i++);
	cpy = (parse_rec *)rmalloc(sizeof(parse_rec) * (i + 1));
	memcpy(cpy, list, (i + 1)*sizeof(parse_rec));
	return cpy;
}



/* Among other things, this is used to add disambiguation information */
static int scan_rec(int item, int num, parse_rec *list) {
	int i;

	for (i = 0; list[i].info != D_END && list[i].info != D_AND; i++)
		if (list[i].obj == item && list[i].num == num)
			return i;
	return -1;
}

static rbool scan_andrec(int item, parse_rec *list) {
	for (; list->info != D_END; list++)
		if (list->obj == item && list->info != D_AND
		        && list->info != D_ALL) return 1;
	return 0;
}

static rbool multinoun(parse_rec *list)
/* Determines if LIST refers to a multiple object */
{
	if (list->info == D_ALL) return 1;
	for (; list->info != D_END; list++)
		if (list->info == D_AND) return 1;
	return 0;
}

/* This updates the adj and noun fields of a record and also
   updates the disambiguation priority */
/* We are already either D_ADJ or D_SYN; this just handles
   possible promotion */
static void add_words_to_rec(parse_rec *nrec, word w, int tmp) {
	word w2;

	if (tmp == 2 || tmp == 3) {
		w2 = nrec->noun;
		nrec->noun = w;
		w = w2;
	}
	if (nrec->adj == 0)
		nrec->adj = w;
	if (tmp == 2) nrec->info = D_SYN;
	else if (tmp == 3) nrec->info = D_NOUN;
}

/* This updates nrec with the information in newobj; this routine
   is called while adding disambiguation information; newobj is from
   what the player just typed in to clarify the ambiguity */
void update_rec_words(parse_rec *nrec, parse_rec *newobj) {
	int tmp;

	if (nrec->adj == 0) nrec->adj = newobj->adj;
	if (newobj->info == D_ADJ) tmp = 1;
	else if (newobj->info == D_SYN) tmp = 2;
	else if (newobj->info == D_NOUN) tmp = 3;
	else return;
	add_words_to_rec(nrec, newobj->noun, tmp);
}


static rbool ident_objrec(parse_rec *p1, parse_rec *p2) {
	word noun1, adj1, noun2, adj2;

	if (p1->obj == p2->obj) return 1;
	if (p1->obj <= 0 || p2->obj <= 0) return 0;
	if (tnoun(p1->obj)) {
		noun1 = noun[p1->obj - first_noun].name;
		adj1 = noun[p1->obj - first_noun].adj;
	} else if (tcreat(p1->obj)) {
		noun1 = creature[p1->obj - first_creat].name;
		adj1 = creature[p1->obj - first_creat].adj;
	} else return 0;
	if (tnoun(p2->obj)) {
		noun2 = noun[p2->obj - first_noun].name;
		adj2 = noun[p2->obj - first_noun].adj;
	} else if (tcreat(p2->obj)) {
		noun2 = creature[p2->obj - first_creat].name;
		adj2 = creature[p2->obj - first_creat].adj;
	} else return 0;
	return (noun1 == noun2 && adj1 == adj2);
}

/*-------------------------------------------------------------------*/
/*  Disambiguation and ALL expansion routines                        */
/*-------------------------------------------------------------------*/

/* Eliminate non-creatures and non-present creatures from list. */
/* Very similar to disambig below, but simpler. */
static parse_rec *fix_actor(parse_rec *alist) {
	int i, cnt;

	assert(alist != NULL);
	if (alist[0].info == D_ALL) { /* ALL?! */
		rfree(alist);
		return new_list();
	}

	/* Start by eliminating non-creatures */
	cnt = 0;
	for (i = 0; alist[i].info != D_END; i++)
		if ((alist[i].obj < first_creat || alist[i].obj > maxcreat)
		        && alist[i].obj != -ext_code[weverybody]) {
			if (alist[i].info != D_AND)
				alist[i].info |= D_MARK;
		} else cnt++;
	alist = purge_list(alist);
	if (cnt <= 1) return alist;

	/* Now eliminate those not present */
	cnt = 0;
	for (i = 0; alist[i].info != D_END; i++)
		if (!genvisible(&alist[i])) {
			if (alist[i].info != D_AND)
				alist[i].info |= D_MARK;
		} else cnt++;

	if (cnt == 0) alist[0].info &= ~D_MARK;

	return purge_list(alist);
}


/* Convert disambig list to all list, moving things up. */
static parse_rec *convert_to_all(parse_rec *list, int *ofsref) {
	int i;
	int cnt;

	for (i = *ofsref; list[i].info != D_AND && list[i].info != D_END; i++);
	cnt = i - *ofsref; /* Number of objects. We will add cnt-1 ANDs */

	while (list[i].info != D_END) i++;
	list = (parse_rec *)rrealloc(list, (i + cnt) * sizeof(parse_rec));
	memmove(list + *ofsref + 2 * cnt - 1, list + *ofsref + cnt,
	        (i + 1 - cnt - *ofsref)*sizeof(parse_rec));
	for (i = cnt - 1; i >= 0; i--) {
		int k;
		list[*ofsref + 2 * i] = list[*ofsref + i];
		if (i == 0) break;
		k = *ofsref + 2 * i - 1;
		list[k].obj = 0;
		list[k].num = 0;
		list[k].adj = list[k].noun = 0;
		list[k].info = D_AND;
	}
	/* *ofsref+=2*cnt-1; */
	return list;
}



static parse_rec *add_disambig_info(parse_rec *ilist, int ofs,
                                    parse_rec *truenoun)
/* Basically, try to find interesection of tmp and truenoun,
or failing that, between ilist and truenoun */
/*  truenoun is what the player just typed in to resolve
the ambiguity */
{
	int i, n;

	for (i = ofs; ilist[i].info != D_AND && ilist[i].info != D_END; i++) {
		if (truenoun[0].info == D_EITHER) {
			/* Mark all but the first for deletion */
			if (i > ofs) ilist[i].info |= D_MARK;
		} else {
			n = scan_rec(ilist[i].obj, ilist[i].num, truenoun);
			if (n == -1)
				ilist[i].info |= D_MARK;
			else  /* Add any new information to the words */
				update_rec_words(&ilist[i], &truenoun[n]);
		}
	}
	ilist = purge_list(ilist);
	truenoun[0].obj = 0;
	truenoun[0].num = 0;
	truenoun[0].info = D_END; /* Truenoun no longer useful */
	return ilist;
}

static int max_disambig_score;

static int score_disambig(parse_rec *rec, int ambig_type)
/* This is just a wrapper for check_obj (defined in runverb.c) */
/* ambig_type=1 for actor, 2 for noun, 3 for object */
/* We can assume that the earlier bits have already been disambiguated */
/*  Parse of current command in lactor, vnum, lnoun, prep, and lobj */
{
	if (ambig_type == 1) /* ACTOR */
		return DISAMBIG_SUCC;
	else if (ambig_type == 2) /* NOUN */
		return check_obj(lactor, vnum, rec, prep, NULL);
	else if (ambig_type == 3) /* IOBJ */
		return check_obj(lactor, vnum, lnoun, prep, rec);
	else fatal("Invalid ambig_type!");
	return 0;
}


/* This routine does all expansion: it returns a list of ALL objects in the
 current context (lactor, vnum, <ALL>, prep, lobj) */
/* lnoun is assumed to be a list of exceptions, which needs to be freed
   at the end. (i.e. we want to expand to ALL EXCEPT <lnoun>) */
static parse_rec *expand_all(parse_rec *lnoun_) {
	parse_rec *list;
	int i, j;
	rbool prev_obj; /* Is there a previous object on the list? */
	rbool kill_obj; /* Don't put current object on ALL list after all */
	parse_rec temp_obj; /* Used to pass object info to disambiguation routine */

	if (debug_parse) {
		writestr("ALL BUT:");
		print_nlist(lnoun_);
	}
	tmpobj(&temp_obj);
	nounloop(i)
	noun[i].scratch = 0;
	creatloop(i)
	creature[i].scratch = 0;
	objloop(i)
	if (((verbflag[vnum]&VERB_GLOBAL) != 0 || visible(i))
	        && (lnoun_ == NULL || !scan_andrec(i, lnoun_))) {
		temp_obj.obj = i;
		if (score_disambig(&temp_obj, 2) >= 500) {
			if (tnoun(i)) noun[i - first_noun].scratch = 1;
			else if (tcreat(i)) creature[i - first_creat].scratch = 1;
			else writeln("INTERNAL ERROR: Invalid object type in expand_all().");
		}
	}

	/* The following ensures that if an object and it's container
	   are both selected, only the container will actually make it
	   onto the list.*/
	list = new_list();
	prev_obj = 0;
	objloop(i)
	if (it_scratch(i)) {
		kill_obj = 0;
		for (j = it_loc(i); tnoun(j) || tcreat(j); j = it_loc(j))
			if (it_scratch(j)) {
				kill_obj = 1;
				break;
			}
		if (kill_obj) continue;
		/* Now actually add object to list. */
		if (prev_obj) list = add_rec(list, 0, 0, D_AND);
		list = add_rec(list, i, 0, D_SYN);
		prev_obj = 1;
	}

	if (debug_parse) {
		writestr("ALL==>");
		print_nlist(list);
	}
	rfree(lnoun_);
	return list;
}



/* disambig check checks for the various things that can eliminate a noun
     from the list. The higher dlev is, the more things that are bad */
/* Returns 1 if we should keep it, 0 if we should kill it */
/* The elimination order is based on AGT:
  0-Eliminate adjective matches if PURE_NOUN is set and
      out-of-scope adjectives that are not at the head of the list.
  1-Eliminate adj matches if the adjective is out-of-scope.
  2-eliminate SCENE and DOOR (D_INTERN)
  3-eliminate out-of-scope nouns that are not at the head of the list
  4-eliminate out-of-scope nouns
  5-eliminate numbers that don't have associated dictionary words
  6-eliminate adj only matches (i.e. noun-free) [only if PURE_ADJ]
  7-eliminate pronouns (D_PRO)
   -eliminate ALL (D_ALL)
   -eliminate numbers
 (Never eliminated: FLAG,GLOBAL,PIX,SYN,NOUN)
*/

#define MAX_DSCHEME 3
#define MAX_DLEV 2
/* ambig_type=1 for actor, 2 for noun, 3 for object */
/* We can assume that the earlier bits have already been disambiguated */
/*  Parse of current command in lactor, vnum, lnoun, prep, and lobj */
/* pick_one is used to select the first noun in the case that all of
   them get eliminated during visibility testing (if none of the nouns
   are visible, we don't want to ask disambiguation questions) */

static rbool disambig_check(parse_rec *rec, int dsch, int dlev,
                            int ambig_type, rbool pick_one) {
	switch (dsch) {
	case 0:
		switch (dlev) { /* Syntactic checks: pre-scope */
		case 0:
			return (!PURE_ADJ || rec->info != D_ADJ);
		case 1:
			return (rec->info != D_INTERN); /* Elim SCENE and DOOR */
		case 2:
			return (rec->info != D_NUM || rec->obj != 0); /* Eliminate numbers w/o
                           corrosponding word matches */
		default:
			return 0;
		}
	case 1:
		switch (dlev) { /* Scope checks */
		case 0:
			/* Just compute the scores, but don't eliminate anything yet */
			/* if PURE_DISAMBIG, no intel dismbig */
			if (PURE_DISAMBIG || rec->info == D_NUM)
				rec->score = DISAMBIG_SUCC;
			else rec->score = score_disambig(rec, ambig_type);
			if (rec->score >= max_disambig_score)
				max_disambig_score = rec->score;
			return 1;
		case 1:
			return (rec->score == max_disambig_score);
		case 2:
			return (rec->info == D_NUM
			        || ((verbflag[vnum] & VERB_GLOBAL) != 0 && rec->score >= 500)
			        || (tnoun(rec->obj) && noun[rec->obj - first_noun].scope)
			        || (tcreat(rec->obj) && creature[rec->obj - first_creat].scope));
		default:
			return 1;
		}
	case 2:
		switch (dlev) { /* Syntax, take 2 */
		case 0: /* Reserved for alternative adjective checking */
			return 1;
		/* Kill internal matches */
		case 1:
			return (rec->info != D_PRO && rec->info != D_ALL &&
			        rec->info != D_INTERN && rec->info != D_NUM);
		default:
			return 0;
		}
	case 3:
		return pick_one;
	default:
		return 0;
	}
}


/* disambig_a_noun does disambiguation for a single AND-terminated block */
/* *list* contains the list of possible objects,                      */
/*    *ofs* is our starting offset within the list (since with ANDs we */
/*    may not be starting at list[0]) */

static parse_rec *disambig_a_noun(parse_rec *list, int ofs, int ambig_type)
/* ambig_type=1 for actor, 2 for noun, 3 for object */
/* We can assume that the earlier bits have already been disambiguated */
/*  Parse of current command in lactor, vnum, lnoun, prep, and lobj */
{
	int i, cnt; /* cnt keeps track of how many nouns we've let through */
	int dsch; /* Dismabiguation scheme; we run through these in turn,
           pushing dlev as high as possible for each scheme */
	int dlev; /* Disambiguation level: how picky do we want to be?
           We keep raising this until we get a unique choice
           or until we reach MAX_DLEV and have to give up */
	rbool one_in_scope; /* True if at least one noun is visible; if no nouns
            are visible then we never ask for disambiguation
            from the player (to avoid giving things away) but
            just take the first one. */

	cnt = 2; /* Arbitrary number > 1 */
	one_in_scope = 0;
	max_disambig_score = -1000;  /* Nothing built in returns anything lower than 0,
                but some game author might come up with a
                clever application of negative scores */
	for (dsch = 0; dsch <= MAX_DSCHEME; dsch++)
		for (dlev = 0; dlev <= MAX_DLEV; dlev++) {
			if (DEBUG_DISAMBIG)
				rprintf("\nDISAMBIG%c%d:%d: ", (dsch == 1 ? '*' : ' '), dsch, dlev);
			cnt = 0;
			for (i = ofs; list[i].info != D_END && list[i].info != D_AND; i++)
				if (disambig_check(&list[i], dsch, dlev, ambig_type,
				                   one_in_scope || (i == ofs))
				   ) {
					cnt++;
					if (DEBUG_DISAMBIG)
						rprintf("+%d ", list[i].obj);
				} else {
					if (DEBUG_DISAMBIG)
						rprintf("-%d ", list[i].obj);
					list[i].info |= D_MARK; /* Mark it for deletion */
				}
			if (cnt != 0) {
				list = purge_list(list); /* Delete marked items */
				if (cnt == 1) return list;
				if (dsch == 1 && dlev == MAX_DLEV)
					one_in_scope = 1;
			} else {
				clean_list(list); /* Remove marks; we're not purging */
				break;
			}
		}
	/* Check to make sure we don't have a list of multiple identical items */
	for (i = ofs; list[i].info != D_END && list[i].info != D_AND; i++) {
		if (!ident_objrec(&list[i], &list[ofs])) break;
		list[i].info |= D_MARK;
	}
	if (list[i].info == D_END || list[i].info == D_AND) {
		/* If all of the items are identical, just pick the first */
		if (one_in_scope) writeln("(Picking one at random)");
		list[0].info &= ~D_MARK;
		list = purge_list(list);
	} else clean_list(list);
	if (DEBUG_DISAMBIG) rprintf("\n");
	return list;
}



/* Note that this routine must be _restartable_, when new input comes in. */
/* Truenoun is 0 or else the "correct" noun for where disambig first
   got stuck. */
/* Returns the offset at which it got stuck, or -1 if everything
 went ok. Return -2 if we eliminate everything */
/* *tn_ofs* contains the offset where truenoun is supposed to be used */

#define list (*ilist)    /* in disambig_phrase only */


static int disambig_phrase(parse_rec **ilist, parse_rec *truenoun, int tn_ofs,
                           int ambig_type)
/* Note that ilist is double dereferenced: this is so we can realloc it */
/* ambig_type=1 for actor, 2 for noun, 3 for object */
/* We can assume that the earlier bits have already been disambiguated */
{
	int ofs, i;
	char *s;

	ofs = 0;
	if (list[0].info == D_END) return -1; /* No nouns, so no ambiguity */
	if (list[0].info == D_ALL) ofs = 1; /* might have ALL EXCEPT construction */
#ifdef OMEGA
	return -1; /* No ambiguity over ALL, either */
	/* (at least if it appears as the first element of the list) */
#endif

	while (list[ofs].info != D_END) { /* Go through each AND block */
		if (ofs == tn_ofs) {
			if (truenoun[0].info == D_ALL) /* Convert to ALL list */
				list = convert_to_all(list, &ofs);
			else {
				list = add_disambig_info(list, ofs, truenoun);
				if (list[ofs].info == D_END) { /* We have eliminated all matches */
					gen_sysmsg(240, "In that case, I don't know what you mean.",
					           MSG_PARSE, "");
					return -2;
				}
			}
		}
		list = disambig_a_noun(list, ofs, ambig_type);
		assert(list[ofs].info != D_END && list[ofs].info != D_AND);
		if (list[ofs + 1].info != D_END &&  list[ofs + 1].info != D_AND)
			/* Disambiguation failed */
		{
			writestr("Do you mean");
			for (i = ofs; list[i].info != D_END && list[i].info != D_AND; i++) {
				if (list[i + 1].info == D_END || list[i + 1].info == D_AND)
					if (i > ofs + 1) writestr(", or");
					else writestr(" or");
				else if (i > ofs) writestr(",");
				writestr(" the ");
				if (list[i].info != D_NUM || list[i].obj != 0)
					s = (char *)objname(list[i].obj);
				else {
					s = (char *)rmalloc(30 * sizeof(char));
					sprintf(s, "%ld", list[i].num);
				}
				writestr(s);
				rfree(s);
			}
			writeln("?");
			return ofs;
		}
		/* Skip forward to next AND  */
		while ((*ilist)[ofs].info != D_END && (*ilist)[ofs].info != D_AND)
			ofs++;
		if ((*ilist)[ofs].info == D_AND) ofs++;
	}
	return -1;
}

#undef list

static int disambig_ofs; /* Offset where disambig failed */

/* ambig_flag stores what we were disambiguating the last time this
   routine was called: it tells us where we failed so that if the
   player enters new disambiguation information, we can figure out where
   it should go */

static parse_rec *disambig(int ambig_set, parse_rec *list, parse_rec *truenoun)
/* ambig_set = 1 for actor, 2 for noun, 3 for object */
{
	if (ambig_flag == ambig_set || ambig_flag == 0) { /* restart where we left off...*/
		if (truenoun == NULL || truenoun[0].info == D_END) disambig_ofs = -1;
		disambig_ofs = disambig_phrase(&list, truenoun, disambig_ofs, ambig_set);
		if (disambig_ofs == -1) ambig_flag = 0; /* Success */
		else if (disambig_ofs == -2) ambig_flag = -1; /* Error: elim all choices */
		else ambig_flag = ambig_set;
	}
	return list;
}




/*-------------------------------------------------------------------*/
/*   Noun parsing routines                                           */
/*-------------------------------------------------------------------*/


/* PARSE_A_NOUN(), parses a single noun, leaves ip pointing after it. */
/*   Just be greedy: grab as many of the input words as possible */
/* Leave ip pointing _after_ last word we get. */
/*   Return list of all possible objects */
/*   Go to some difficullty to make sure "all the kings men" will
     not be accidentally parsed as "all" + "the kings men" */
/*   (Yes, yes, I know -- you can't have an AGT object with a name as
     complex as 'all the king's men'-- but you could try to simulate it using
     synonyms) */
/*   May also want to use more intellegence along the adj--noun distinction */
/*   all_ok indicates whether ALL is acceptable */

static parse_rec *parse_a_noun(void)
/* Returns a list of parse_rec's containing the various possible
   nouns.  */
{
	parse_rec *nlist;
	char *errptr;
	int i, tmp, numval, num, oip;

	nlist = new_list();
	oip = ip; /* Save starting input pointer value */

	if (input[ip] == -1) /* End of input */
		return nlist;
	if (input[ip] == 0) { /* i.e. tokeniser threw up its hands */
		numval = strtol(in_text[ip], &errptr, 10); /* Is it a number? */
		if (errptr == in_text[ip]) /* Nope. */
			return nlist;
		if (*errptr != 0) return nlist; /* May want to be less picky */
		nlist = add_rec(nlist, 0, numval, D_NUM);
		ip++;
		return nlist;
	}

	/* Basic strategy: try to match nouns. If all matches are of length<=1,
	   then go looking for flag nouns, global nouns, ALL, DOOR, etc. */
	num = 0;
	objloop(i)
	if ((tmp = noun_syn(input[ip], i)) != 0) {
		numval = strtol(in_text[ip], &errptr, 10); /* Is it a number, too? */
		if (*errptr != 0) numval = 0; /* Only accept perfectly formed numbers */
		nlist = add_w_rec(nlist, i, numval,
		                  (tmp == 1) ? D_ADJ : (tmp == 2 ? D_SYN : D_NOUN),
		                  (tmp == 1) ? input[ip] : 0, /* Adjective */
		                  (tmp == 1) ? 0 : input[ip]); /* Noun */
		num++;
	}

	/* Now we need to try to match more words and reduce our list. */
	/*   Basically, we keep advancing ip until we get no matches at all. */
	/*   Note that nouns may only come at the end, so if we find one we know */
	/*   we are done.  Synonyms and adjectives can come anywhere */
	/* (If PURE_SYN is set, then we won't see any synonyms-- they */
	/*   get converted into nouns by noun_syn() */
	/* *num* is used to keep track of how many matches we have so we know */
	/*    when to stop. */

	/* compare against the next word in the queue */
	while (num > 0 && input[ip] != -1 && ip < MAXINPUT) {
		ip++;
		if (input[ip] == -1) break;
		num = 0;
		for (i = 0; nlist[i].info != D_END; i++) /* Which nouns match the new word? */
			if (nlist[i].info == D_NOUN)      /* Nothing can come after a noun */
				nlist[i].info |= D_MARK;
			else if ((tmp = noun_syn(input[ip], nlist[i].obj)) == 0)
				nlist[i].info |= D_MARK; /* Mark this noun to be eliminated */
			else {  /* Noun _does_ match */
				num++;  /* Count them up */
				add_words_to_rec(&nlist[i], input[ip], tmp);
			}
		/* If we had any matches, kill the nouns that didn't match */
		if (num != 0) {
			nlist = purge_list(nlist);
			for (i = 0; nlist[i].info != D_END; i++)
				nlist[i].num = 0; /* Multi-word nouns can't be numbers */
		} else /* num==0; we need to clear all of the marks */
			clean_list(nlist);
	}

	/* So at this point num is zero-- that is we have reached the limit
	   of our matches. */
	/* If ip==oip is 0 (meaning no matches so far) or ip==oip+1
	   (meaning we have a one-word match) then we need to check for
	   flag nouns, global nouns, ALL, DOOR, pronouns, etc.
	   and add them to the list if neccessary */

	if (ip == oip || ip == oip + 1) {

		/* First match the built in things... */
		if ((input[oip] == ext_code[wdoor] && aver <= AGX00)
		        || input[oip] == ext_code[wscene])
			nlist = add_rec(nlist, -input[oip], 0, D_INTERN);
		else if (input[oip] == ext_code[wall] ||
		         input[oip] == ext_code[weverything])
			nlist = add_rec(nlist, ALL_MARK, 0, D_ALL);
		else if (input[oip] == ext_code[whe] || input[oip] == ext_code[whim])
			nlist = add_rec(nlist, last_he, 0, D_PRO);
		else if (input[oip] == ext_code[wshe] || input[oip] == ext_code[wher])
			nlist = add_rec(nlist, last_she, 0, D_PRO);
		else if (input[oip] == ext_code[wit])
			nlist = add_rec(nlist, last_it, 0, D_PRO);
		else if (input[oip] == ext_code[wthey] || input[oip] == ext_code[wthem])
			nlist = add_rec(nlist, last_they, 0, D_PRO);
		else for (i = 0; i < 10; i++) /* Match direction names */
				if (input[oip] == syntbl[auxsyn[i]])
					nlist = add_rec(nlist, -syntbl[auxsyn[i]], 0, D_INTERN);
				else if (input[oip] == ext_code[weveryone] ||
				         input[oip] == ext_code[weverybody])
					nlist = add_rec(nlist, -ext_code[weverybody], 0, D_INTERN);

		/* Next look for number word matches */
		numval = strtol(in_text[oip], &errptr, 10); /* Is it a number? */
		if (*errptr == 0) /* Yes */
			nlist = add_rec(nlist, -input[oip], numval, D_NUM);

		/* Next handle the flag nouns and global nouns */
		if (globalnoun != NULL)
			for (i = 0; i < numglobal; i++)
				if (input[oip] == globalnoun[i])
					nlist = add_rec(nlist, -input[oip], 0, D_GLOBAL);
		for (i = 0; i < MAX_FLAG_NOUN; i++)
			if (flag_noun[i] != 0 && input[oip] == flag_noun[i]
#if 0
			        && (room[loc].flag_noun_bits & (1L << i)) != 0
#endif
			   )
				nlist = add_rec(nlist, -input[oip], 0, D_FLAG);

		/* Finally the PIX names */
		for (i = 0; i < MAX_PIX; i++)
			if (pix_name[i] != 0 && input[oip] == pix_name[i] &&
			        (room[loc].PIX_bits & (1L << i)) != 0)
				nlist = add_rec(nlist, -input[oip], 0, D_PIX);

		if (nlist[0].info != D_END) ip = oip + 1;
	}
	return nlist;
}

static parse_rec *parse_noun(int and_ok, int is_actor)
/* handles multiple nouns */
/* and_ok indicates wheter multiple objects are acceptable */
/* Either_ok indicates whether EITHER is okay (only true when
resolving disambiguation) */
{
	parse_rec *next, *lnoun_;
	int saveinfo;
	rbool all_except;

	all_except = 0;
	next = lnoun_ = parse_a_noun();
	saveinfo = next[0].info;

#if 0  /* Let the main parser sort this out */
	if (!and_ok) return lnoun_; /* If no ANDs allowed, stop here. */
#endif
	/* We need to explicitly handle the actor case here because
	   the comma after an actor can be confused with the comma
	   used to separate multiple objects */
	if (is_actor) return lnoun_; /* If no ANDs allowed, stop here. */

	if (lnoun_[0].info == D_ALL && w_but(input[ip])) /* ALL EXCEPT ... */
		all_except = 1; /* This will cause us to skip over EXCEPT and
             start creating an AND list */

	/* Now, itereate over <noun> AND <noun> AND ... */
	while ((all_except || w_and(input[ip])) &&
	        saveinfo != D_END) { /* && saveinfo!=D_ALL */
		ip++; /* Skip over AND or EXCEPT */
		next = parse_a_noun();
		saveinfo = next[0].info;
		if (next[0].info != D_END) {          /* We found a word */
			if (!all_except) lnoun_ = add_rec(lnoun_, AND_MARK, 0, D_AND);
			lnoun_ = concat_list(lnoun_, next);
		} else ip--; /* We hit trouble: back up to the AND  */
		all_except = 0; /* Only skip EXCEPT once */
		rfree(next);
	}
	return lnoun_;
}


parse_rec *parse_disambig_answer(void) {
	parse_rec *temp;

	if (input[ip + 1] == -1) {
		if (input[ip] == ext_code[wall] || input[ip] == ext_code[weverything]
		        || input[ip] == ext_code[wboth]) {
			temp = new_list();
			ip++;
			return add_rec(temp, ALL_MARK, 0, D_ALL);
		}
		if (input[ip] == ext_code[weither] || input[ip] == ext_code[w_any]) {
			temp = new_list();
			ip++;
			return add_rec(temp, 0, 0, D_EITHER);
		}
	}
	return parse_noun(0, 0);
}



/*-------------------------------------------------------------------*/
/*      Main parsing routines                                        */
/*-------------------------------------------------------------------*/


static int parse_cmd(void)
/* Parse entered text and execute it, one statement at a time */
/* Needs to leave ip pointing at beginning of next statement */
{
	rbool new_actor; /* This is used for some error checking; it is set
             if we just found an actor on this command
             (as opposed to inheriting one from a previous
             command in a multiple statement) */
	parse_rec *tmp;
	int tp;

	/* First go looking for an actor. */
	ap = ip;
	new_actor = 0;
	if (lactor == NULL) {
		new_actor = 1;
		lactor = parse_noun(0, 1);
		/* Check that actor is a creature. */
		if (lactor[0].info != D_END) {
			lactor = fix_actor(lactor); /* eliminate non-creatures */
			if (lactor[0].info == D_END) { /* Not a creature. */
				/* print intelligent error message */
				if (input[ip] == ext_code[wc]) /* ie there is a comma */
					return parseerr(229, "Who is this '$word$' you are addressing?", ap);
				else ip = ap; /* Otherwise, assume we shouldn't have parsed it as
               an actor-- it may be a verb. */
			}
		}
		if (lactor[0].info != D_END && input[ip] == ext_code[wc])
			ip++;  /* this skips over a comma after an actor. */
	}
	/* Now onwards... */
	vp = ip;
	vnum = id_verb(); /* May increment ip (ip will point at last word in verb) */
	if (vnum == 0 && new_actor && lactor[0].info != D_END) {
		/* maybe actor is messing us up. */
		ip = ap; /* restart from beginning */
		vnum = id_verb();
		if (vnum == 0) /* if it's still bad, probably we really have an actor */
			ip = vp;
		else {  /* no actor; really a verb */
			lactor[0].obj = 0;
			lactor[0].info = D_END;
			vp = ap;
		}
	}

TELLHack:  /* This is used to restart the noun/prep/object scan
          if we find a TELL <actor> TO <verb> ... command */

	if (vnum == 0)
		return parseerr(230, "I don't understand '$word$' as a verb.", ip);

	/* Now we need to find noun, obj, and prep (if they all exist) */
	/* standard grammer:  verb noun prep obj */
	prep = 0;
	np = ++ip; /* ip now points just _after_ verb */
	lnoun = parse_noun((verbflag[vnum] & VERB_MULTI) != 0, 0);
	/* leaves ip pointing just after it.;
	   lnoun[0].info==D_END means no noun. */
	if (prep == 0) { /* prep==0 unless we've met the special TURN ON|OFF case */
		pp = ip;
		prep = parse_prep(); /* Should be trivial */
		op = ip;
		lobj = parse_noun(prep == 0, 0); /* and_ok if no prep */
	}

	/* Check for TELL <actor> TO <verb> ... construction */
	/* If found, convert to <actor>, <verb> ... construction */
	if (lactor[0].info == D_END && lnoun[0].info != D_END &&
	        vnum == 31 && prep == ext_code[wto] && !multinoun(lnoun)) {
		ip = op; /* Back up */
		rfree(lactor);
		rfree(lobj);
		lactor = lnoun;
		lnoun = NULL;
		vp = ip; /* Replace TELL with new verb */
		vnum = id_verb(); /* May increment ip (ip points att last word in verb) */
		goto TELLHack;  /* Go back up and reparse the sentence from
             the new start point. */
	}

	/* Convert TURN <noun> ON to TURN ON <noun> */
	if (vnum == 35 && (prep == ext_code[won] || prep == ext_code[woff])
	        && lobj[0].info == D_END) {
		tmp = lobj;
		lobj = lnoun;
		lnoun = tmp;
		tp = op;
		np = op;
		op = tp;
	}


	/* For pre-Magx versions of AGT,
	      convert <verb> <prep> <noun>  ==>  <verb> <noun> <prep> <noun> */
	if (aver < AGX00 && lnoun[0].info == D_END && lobj[0].info != D_END) {
		rfree(lnoun);
		lnoun = copy_list(lobj);
		np = op;
	}

	/* Next we check to convert SHOOT <noun> AT <object> to
	   SHOOT <object> WITH <noun> */
	if (vnum == 49 && prep == ext_code[wat] && !multinoun(lnoun)) {
		tmp = lobj;
		lobj = lnoun;
		lnoun = tmp;
		tp = np;
		np = op;
		op = tp;
		prep = ext_code[wwith];
	}

	/* Now to convert SHOW <*nothing*> to SHOW EXITS */
	if (vnum == 40 && prep == 0 && lnoun[0].info == D_END && lobj[0].info == D_END)
		vnum = 42; /* LISTEXITS */

	/* Convert LOOK <something> into EXAMINE <something> */
	if (smart_look && vnum == 19 && lnoun[0].info != D_END) vnum = 20;

	/* need better error msgs */
	if ((verbflag[vnum]&VERB_MULTI) == 0 && multinoun(lnoun)) {
		/* Multiple objects when they are not allowed */
		int msgnum;
		if (vnum == 31) msgnum = 155; /* TALK */
		else if (vnum == 34) msgnum = 160; /* ASK */
		else msgnum = 231;
		return parseerr(msgnum,
		                "The verb '$word$' doesn't take multiple objects.", vp);
	} else if (multinoun(lobj))
		return parseerr(232, "You can't use multiple indirect objects.", op);
	else if (lnoun[0].info == D_END && !w_isterm(input[np]) && np != pp)
		/* i.e. invalid noun */
		return parseerr(233, "I don't understand the word '$word$' as a noun.", np);
	else if (lnoun[0].obj == 0 && lnoun[0].info == D_PRO)
		return parseerr(234, "I don't know to what '$word$' refers.", np);
	else if (lobj[0].info == D_END && !w_isterm(input[op]))
		/* i.e. invalid object */
		return parseerr(235, "I don't understand the word '$word$' as a noun.", op);
	else if (lobj[0].obj == 0 && lobj[0].info == D_PRO)
		return parseerr(236, "I don't know to what '$word$' refers.", op);
	else if (!w_isterm(input[ip]))
		return parseerr(238, "Extra input found: \"$word$...\"", ip);

	return 0;
}


static void v_undo(void) {
	if (undo_state == NULL) {
		writeln("There is insufficiant memory to support UNDO");
		ip = -1;
		return;
	}
	if (can_undo == 0) {
		if (newlife_flag)
			writeln("You can't UNDO on the first turn.");
		else writeln("You can only UNDO one turn.");
		ip = -1;
		return;
	}
	writeln("");
	writeln("UNDOing a turn...");
	can_undo = 0;
	putstate(undo_state);
	ip = 1;
	set_statline();
	return;
}

rbool parse(void)
/* Wrapper around parse_cmd to handle disambiguation, etc */
/* It returns 1 if everything is okay; 0 if there is ambiguity */
{
	parse_rec *currnoun;
	int fixword;
	int start_ip;

	currnoun = NULL;
	start_ip = ip;
	/* First, we need to see if someone has issued an OOPS command.
	   OOPS commands are always assumed to stand alone. (i.e. no
	   THEN nonsense). OOPS commands are always of the form
	   OOPS <word> */
	if (ip == 0 && input[0] == ext_code[woops] && input[1] > 0 &&
	        input[2] == -1 && ep > -1) {
		fixword = input[ip + 1];
		restore_input();
		input[ep] = fixword;
		ambig_flag = 0;
	}
	ep = -1;


	/* Next, we need to determine if someone is trying to do
	   disambiguation. This is only the case if
	   i)ambig_flag is set
	   ii)ip=0 (no multiple command nonsense)
	   iii)there is only one noun on the line. */
	if (ip != 0) ambig_flag = 0;
	if (ambig_flag) {
		currnoun = parse_disambig_answer();
		if (input[ip] == -1 && currnoun[0].info != D_END) {
			restore_input();    /* Yep, we're disambiguaing. */
			ip = parse_ip;
		} else { /* nope; it's a new command */
			ip = 0;
			ambig_flag = 0;
			rfree(currnoun);
			freeall();
			currnoun = NULL;
		}
	}

	/* Next we go looking for UNDO; again, this must be the first
	 thing on an empty line. */
	if (ip == 0 && input[0] == ext_code[wundo] && input[1] == -1) {
		v_undo();
		return 1;
	}

	save_input();

	/* If starting a new line, clear out old the old actor */
	if (ip == 0) {
		actor_in_scope = 0; /* Clear this */
		rfree(lactor); /* This resets lactor to NULL  */
	}

	if (!ambig_flag)
		if (parse_cmd() == -1)
			return 1;  /* error condition */

	parse_ip = ip;

	if (debug_parse)
		parse_out(lactor, vnum, lnoun, prep, lobj);

	/*Disambiguation routines; do it here instead of earlier to get
	 error messages in the right place (it's silly and annoying to ask the
	 player for disambiguation and then die on a parse error after they've
	 provided it) */
	compute_scope(); /* The disambig routines use this information */
	lactor = disambig(1, lactor, currnoun);
	lnoun = disambig(2, lnoun, currnoun);
	lobj = disambig(3, lobj, currnoun);
	if (ambig_flag > 0) return 0; /* We need to get disambig info */
	if (ambig_flag == -1) {
		ambig_flag = 0;
		return 1;
	}
	/* We got rid of too much */
	rfree(currnoun);

	/* Next, expand ALL if neccessary */
	if (!PURE_ALL && lnoun[0].info == D_ALL) {
		lnoun = expand_all(lnoun);
		if (lnoun[0].info == D_END) { /* ALL expands to nothing */
			int msgnum;
			if (vnum >= 15 && vnum <= 24)
				msgnum = all_err_msg[vnum - 15];
			else
				msgnum = 239;
			parseerr(msgnum, "I don't know what you are referring to.", np);
			return 1;
		}
	}

	/* Now that we know that we have an executable command,
	   we save the undo state before executing if this is the first command
	   in a sequence. (That is, UNDO undoes whole lines of commands,
	   not just individual commands) */
	if (start_ip == 0 && undo_state != NULL) {
		undo_state = getstate(undo_state);
		can_undo = 1;
	}

	/* Now to actually execute the command that has been parsed. */
	/* Remember: disambiguation has been done by this time. */

	exec(&lactor[0], vnum, lnoun, prep, &lobj[0]);
	rfree(lobj);

	/* exec is responsible for freeing or whatever lnoun (this is for AGAIN) */

	/* Now we clear lnoun and lobj; lactor is handled elsewhere since
	   we might have FRED, GET ROCK THEN GO NORTH */
	lnoun = lobj = NULL;

	/* Finally check for THENs */

	if (ip != -1 && w_and(input[ip]) && input[ip + 1] == ext_code[wthen])
		ip++;  /* AND THEN construction */
	if (ip != -1 && input[ip] != -1) ip++;
	return 1;
}




void menu_cmd(void) {
	int i, j;
	int choice;
	char *curr_cmd, *tmp1, *tmp2;  /* String of current command */
	int objcnt; /* Number of objects taken by the current verb */
	int verbword; /* Verb word */
	parse_rec actrec;

	parse_rec mobj;
	int vnum_;  /* Verb number */
	word prep_;

	menuentry *nounmenu;
	int *nounval;  /* Object id's for the menu entries */
	int nm_size, nm_width; /* Size and width of noun menu */


	nounval = NULL;
	nounmenu = NULL;
	/* Get verb+prep */
	choice = agt_menu("", vm_size, vm_width, verbmenu);
	if (choice == -1 || doing_restore) return;

	verbword = verbinfo[choice].verb;
	prep_ = verbinfo[choice].prep;
	objcnt = verbinfo[choice].objnum;

	/* Now identify the verb */
	ip = 0;
	input[0] = verbword;
	input[1] = input[2] = -1;
	if (objcnt <= 1 && prep_ != 0) input[1] = prep_;
	vnum_ = id_verb();

	lnoun = (parse_rec *)rmalloc(sizeof(parse_rec) * 2);
	lnoun[0].obj = 0;
	lnoun[0].num = 0;
	lnoun[0].info = D_END;

	nm_size = nm_width = 0;

	if (objcnt >= 1) {
		/* Construct noun list */
		nounval = get_nouns();
		for (nm_size = 0; nounval[nm_size] != 0; nm_size++);
		nounmenu = (menuentry *)rmalloc(nm_size * sizeof(menuentry));
		nm_width = 0;
		for (i = 0; i < nm_size; i++) {
			tmp1 = objname(nounval[i]);
			strncpy(nounmenu[i], tmp1, MENU_WIDTH);
			j = strlen(tmp1);
			if (j > nm_width) nm_width = j;
		}
		if (nm_width > MENU_WIDTH) nm_width = MENU_WIDTH;

		if (objcnt >= 2 || prep_ == 0)
			curr_cmd = rstrdup(dict[verbword]);
		else
			curr_cmd = concdup(dict[verbword], dict[prep_]);

		choice = agt_menu(curr_cmd, nm_size, nm_width, nounmenu);
		rfree(curr_cmd);
		if (choice == -1 || doing_restore) {
			rfree(nounmenu);
			rfree(nounval);
			rfree(lnoun);
			return;
		}

		if (objcnt == 1 && prep_ != 0) { /* VERB PREP OBJ construction */
			mobj.obj = nounval[choice];
			mobj.num = 0;
			mobj.info = D_NOUN;
		} else {  /* Normal VERB OBJ construction */
			lnoun[0].obj = nounval[choice];
			lnoun[0].num = 0;
			lnoun[0].info = D_NOUN;
			lnoun[1].obj = 0;
			lnoun[1].num = 0;
			lnoun[1].info = D_END;
		}
	}

	if (objcnt >= 2) {
		tmp1 = objname(lnoun[0].obj);      /* Build up current command line */
		tmp2 = concdup(dict[verbword], tmp1); /* VERB NOUN */
		rfree(tmp1);
		curr_cmd = concdup(tmp2, dict[prep_]); /* VERB NOUN PREP */
		rfree(tmp2);

		choice = agt_menu(curr_cmd, nm_size, nm_width, nounmenu);
		rfree(curr_cmd);
		if (choice == -1 || doing_restore) {
			rfree(lnoun);
			rfree(nounmenu);
			rfree(nounval);
			return;
		}

		mobj.obj = nounval[choice];
		mobj.num = 0;
		mobj.info = D_NOUN;
	}

	rfree(nounmenu);
	rfree(nounval);

	if (vnum_ == OLD_VERB + 3) { /* UNDO */
		v_undo();
		return;
	}

	if (undo_state != NULL) {
		undo_state = getstate(undo_state);
		can_undo = 1;
	}

	/* Now to actually execute the command that has been selected. */
	tmpobj(&actrec);
	actrec.obj = 0;
	exec(&actrec, vnum_, lnoun, prep_, &mobj);
	lnoun = NULL; /* exec() is responsible for freeing lnoun */
}


/* Grammer structures:
 sverb, dverb                     (n,s,e,w,...,q,l,....)
 overb noun              (close,examine,read,eat,drink,pull,light,ext)
 averb noun|ALL          (drop,get,wear,remove)
 TURN noun ON|OFF
 TURN ON|OFF noun
 a-verb noun ABOUT obj   (tell, ask)
 pverb noun [prep object]  (put, throw)
 w-verb noun [WITH object]  (attack, open, lock, unlock, push, shoot)
 SHOOT noun [AT object]
 dummy noun [prep obj]
( pverb obj noun  ==> pverb noun TO obj  e.g. give dog the bone)
*/

} // End of namespace AGT
} // End of namespace Glk
