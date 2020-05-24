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

/* This file contains the wrapper for running player commands,
   routines run at the end of the turn, and various other functions
   needed by runverb.c and token.c. */

#define global

static rbool pronoun_mode;

word realverb = 0; /* Name of current verb. (normally ~= input[vp])*/


/* ------------------------------------------------------------------- */
/* High level output functions, used for printing messages, error      */
/* messages, and everything else. They call the direct output functions */
/* in interface.c The reason they're in runverb.c is that they need to */
/* access item info in order to fill in the blanks */


/* This updates the contents of compass_rose, which can be used by the
   OS layer to print out some sort of representation of which way the
   player can go. */

static void set_compass_rose(void) {
	int i, bit;

	compass_rose = 0;
	if (!islit()) return;  /* No compass in darkness */
	for (i = 0, bit = 1; i < 12; i++, bit <<= 1)
		if (troom(room[loc].path[i])) compass_rose |= bit;
}

static void time_out(char *s) {
	int hr, min;

	hr = curr_time / 100;
	min = curr_time % 100;

	if (milltime_mode)
		sprintf(s, "%02d:%02d", hr, min);
	else {
		if (hr > 12) hr = hr - 12;
		if (hr == 0) hr = 12;
		sprintf(s, "%2d:%02d %s", hr, min, (curr_time >= 1200) ? "pm" : "am");
	}
}





void set_statline() {
	char timestr[20];

	recompute_score();
	set_compass_rose();

	rstrncpy(l_stat, room[loc].name, 81);

	time_out(timestr);

	switch (statusmode) {
	case 0:
		sprintf(r_stat, "Score: %ld  Moves: %d", tscore, turncnt);
		break;
	case 1:
		sprintf(r_stat, "Score: %ld   %s", tscore, timestr);
		break;
	case 2:
		sprintf(r_stat, "Moves: %d", turncnt);
		break;
	case 3:
		sprintf(r_stat, "%s", timestr);
		break;
	case 4:
		r_stat[0] = '\0';
		break;  /* 'Trinity style' status line */
	case 5:
		sprintf(r_stat, "Score: %ld", tscore);
		break;
	default:
		break;
	}
}


/* -------------------------------------------------------------------- */
/*  Message printing / $ substitution Routines                          */
/* -------------------------------------------------------------------- */

#define FILL_SIZE 100


/* Tries to convert *pstr to a number, which it returns.
  If it fails, or if the number is not in the range 0..maxval,
  it returns -1.
  It advances *pstr to point after the number and after the
  terminating character, if relevant.
  <term_char> is the terminating character; if this is 0, then
     the calling routine will worry about the terminating character.
  <maxval> of 0 indicates no upper bound
*/

static int extract_number(const char **pstr, int maxval,
                          char term_char) {
	const char *s;
	long n;  /* n holds the value to be returned; i holds
          the number of characters parsed. */
	n = 0;
	s = *pstr;
	while (*s == ' ' || *s == '\t') s++;
	for (; *s != 0; s++) {
		if (*s < '0' || *s > '9') break;
		n = 10 * n + (*s - '0');
		if (maxval && n > maxval) return -1;
	}
	if (term_char) {
		if (*s == term_char) s++;
		else return -1;
	}
	*pstr = s;
	return n;
}

#define BAD_PROP (-1000)

/* This is used by #PROP[obj].[prop]# and $ATTR[obj].[attr]$, etc. */
/* isprop: Are we looking for a property (as opposed to an attribute)? */
static void extract_prop_val(const char **pstr,
                             int *id, int *val,
                             rbool isprop, const char term_char) {
	const char *s;
	int v; /* object number / final value */
	int i; /* Attribute id */
	rbool builtin; /* Expect builtin property or attribute? */

	*id = i = BAD_PROP;
	*val = 0; /* Failure case by default */
	builtin = 0;
	s = *pstr;
	if (match_str(&s, "NOUN")) v = dobj;
	else if (match_str(&s, "OBJECT")) v = iobj;
	else v = extract_number(&s, maxcreat, 0); /* Must be object number */
	while (*s == '.') {
		s++;
		if (*s == '-') {
			builtin = 1;
			s++;
		} else
			builtin = 0;
		i = extract_number(&s, 0, 0);
		if (!troom(v) && !tnoun(v) && !tcreat(v)) {
			i = -1;
			continue;
		}
		if (isprop || *s == '.') /* Treat as property */
			v = builtin ? getprop(v, i) : op_objprop(2, v, i, 0);
		else  /* Treat as attribute */
			v = builtin ? getattr(v, i) : op_objflag(2, v, i);
	}
	if (*s != term_char) return;
	*pstr = s + 1;
	if (i < 0) return;
	*id = builtin ? -1 : i;
	*val = v;
}




static word it_pronoun(int item, rbool ind_form)
/* Return the correct pronoun to go with item;
   ind_form is 1 if we want the object form, 0 if we want the
   subject form */
{
	if (it_plur(item))
		return (ind_form ? ext_code[wthem] : ext_code[wthey]);
	if (tcreat(item))
		switch (creature[item - first_creat].gender) {
		case 0:
		default:
			return ext_code[wit];
		case 1:
			return (ind_form ? ext_code[wher] : ext_code[wshe]);
		case 2:
			return (ind_form ? ext_code[whim] : ext_code[whe]);
		}
	return ext_code[wit];
}

/* This sets the value of "The" for the given noun. */
/* (In particular, proper nouns shouldn't have a "the") */
static void theset(char *buff, int item) {
	if (it_proper(item))
		strcpy(buff, "");
	else
		strcpy(buff, "the ");
}


static void num_name_func(parse_rec *obj_rec, char *fill_buff, word prev_adj)
/* This is a subroutine to wordcode_match. */
/* It gives either a noun name or a number, depending. */
/* prev_adj is a word if this was preceded by its associated $adjective$;
   the goal is to avoid having $adjective$ $noun$ expand to (e.g.)
   'silver silver' when the player types in "get silver" to pick up
   a magic charm with synonym 'silver'. */
{
	word w;

	if (obj_rec == NULL) {
		strcpy(fill_buff, "");
		return;
	}

	w = 0;
	if (obj_rec->noun != 0) w = obj_rec->noun;
	if ((w == 0 || w == prev_adj) && obj_rec->obj != 0)
		w = it_name(obj_rec->obj);

	if (w == 0) {
		if (obj_rec->info == D_NUM) sprintf(fill_buff, "%ld", (long)obj_rec->num);
		else strcpy(fill_buff, "");
#if 0
		strcpy(fill_buff, "that"); /* We can try and hope */
#endif
		return;
	}

	if (w == prev_adj) /* ... and prev_adj!=0 but we don't need to explicity
              test that since w!=0 */
		fill_buff[0] = 0; /* i.e. an empty string */
	else {
		rstrncpy(fill_buff, dict[w], FILL_SIZE);
		if (it_proper(obj_rec->obj)) fill_buff[0] = toupper(fill_buff[0]);
	}
}

static word get_adj(parse_rec *obj_rec, char *buff) {
	word w;

	if (obj_rec->adj != 0) w = obj_rec->adj;
	else w = it_adj(obj_rec->obj);

	if (w == 0) strcpy(buff, "");
	else {
		rstrncpy(buff, dict[w], FILL_SIZE);
		if (it_proper(obj_rec->obj)) buff[0] = toupper(buff[0]);
	}

	return w;
}



#define d2buff(i) {rstrncpy(fill_buff,dict[i],FILL_SIZE);return 1;}
#define num_name(obj_rec,jsa)  {num_name_func(obj_rec,fill_buff,jsa);return 1;}
/* jsa= Just seen adj */
#define youme(mestr,youstr) {strcpy(fill_buff,irun_mode?mestr:youstr);\
		return 1;}

word just_seen_adj;  /* This determines if we just saw $adjective$; if so,
              this is set to it, otherwise it is zero. See
              num_name_func above. */

static int wordcode_match(const char **pvarname, char *fill_buff,
                          int context, const char *pword)
/* Check <*p*pvarname> for a match; put subs text in fill_buf
   <context> indicates who called us; this determines
      what substitutions are valid. See interp.h for possible
      values.  Move *p*pvarname after whatever is matched.
   <pword> contains the parse word when context is MSG_PARSE. */
/* $ forms:
   $verb$, $noun$, $adjective$, $prep$, $object$, $name$,
   $n_pro$, $o_pro$, $n_indir$, $o_indir$,
      $name_pro$, $name_indir$
   $n_is$, $o_is$, $name_is$
   $c_name$
   $n_was$, $o_was$, $name_was$
   $the_n$, $the_o$, $the_name$
   */
/* Also $STRn$, $FLAGn$, $ONn$, $OPENn$, $LOCKEDn$ */
/*  Support for FLAG, ON, OPEN, and LOCKED added by Mitch Mlinar */
/* Return 0 if no match, 1 if there is  */
{
	int hold_val, hold_id;

	fill_buff[0] = 0; /* By default, return "\0" string */
	if (match_str(pvarname, "STR")) { /* String variable */
		hold_id = extract_number(pvarname, MAX_USTR, '$');
		if (hold_id < 1) return 0;
		rstrncpy(fill_buff, userstr[hold_id - 1], FILL_SIZE);
		return 1;
	} else if (match_str(pvarname, "VAR")) {
		hold_id = extract_number(pvarname, VAR_NUM, '$');
		if (hold_id < 0) return 0;
		hold_val = agt_var[hold_id];
		rstrncpy(fill_buff,
		         get_objattr_str(AGT_VAR, hold_id, hold_val), FILL_SIZE);
		return 1;
	} else if (match_str(pvarname, "FLAG")) {
		hold_id = extract_number(pvarname, FLAG_NUM, '$');
		if (hold_id < 0) return 0;
		rstrncpy(fill_buff,
		         get_objattr_str(AGT_FLAG, hold_id, flag[hold_id]), FILL_SIZE);
		return 1;
	} else if (match_str(pvarname, "ATTR")) {
		extract_prop_val(pvarname, &hold_id, &hold_val, 0, '$');
		if (hold_id == BAD_PROP) return 1;
		rstrncpy(fill_buff,
		         get_objattr_str(AGT_OBJFLAG, hold_id, hold_val), FILL_SIZE);
		return 1;
	} else if (match_str(pvarname, "PROP")) {
		extract_prop_val(pvarname, &hold_id, &hold_val, 1, '$');
		if (hold_id == BAD_PROP) return 1;
		rstrncpy(fill_buff,
		         get_objattr_str(AGT_OBJPROP, hold_id, hold_val), FILL_SIZE);
		return 1;
	} else if (match_str(pvarname, "OPEN")) {
		hold_val = extract_number(pvarname, maxnoun, '$');
		strcpy(fill_buff, it_open(hold_val) ? "open" : "closed");
		return 1;
	} else if (match_str(pvarname, "ON")) {
		hold_val = extract_number(pvarname, maxnoun, '$');
		strcpy(fill_buff, it_on(hold_val) ? "on" : "off");
		return 1;
	} else if (match_str(pvarname, "LOCKED")) {
		hold_val = extract_number(pvarname, maxnoun, '$');
		strcpy(fill_buff, it_locked(hold_val, 0) ? "locked" : "unlocked");
		return 1;
	}

	if (context == MSG_MAIN) return 0;

	if (context == MSG_PARSE) {
		/* The only special subsitution allowed is $word$. */
		if (match_str(pvarname, "WORD$")) {
			if (pword == NULL) fill_buff[0] = 0;
			else rstrncpy(fill_buff, pword, FILL_SIZE);
			return 1;
		} else return 0;
	}

	/* d2buff is a macro that returns 1 */
	if (match_str(pvarname, "NOUN$"))
		num_name(dobj_rec, just_seen_adj);
	just_seen_adj = 0; /* It doesn't matter. */
	if (match_str(pvarname, "VERB$"))
		d2buff(realverb);  /* auxsyn[vb][0] */
	if (match_str(pvarname, "OBJECT$"))
		num_name(iobj_rec, 0);
	if (match_str(pvarname, "NAME$"))
		num_name(actor_rec, 0);
	if (match_str(pvarname, "ADJECTIVE$")) {
		just_seen_adj = get_adj(dobj_rec, fill_buff);
		return 1;
	}
	if (match_str(pvarname, "PREP$"))
		d2buff(prep);
	if (match_str(pvarname, "N_PRO$"))
		d2buff(it_pronoun(dobj, 0));
	if (match_str(pvarname, "O_PRO$"))
		d2buff(it_pronoun(iobj, 0));
	if (match_str(pvarname, "NAME_PRO$"))
		d2buff(it_pronoun(actor, 0));
	if (match_str(pvarname, "N_INDIR$"))
		d2buff(it_pronoun(dobj, 1));
	if (match_str(pvarname, "O_INDIR$"))
		d2buff(it_pronoun(iobj, 1));
	if (match_str(pvarname, "NAME_INDIR$"))
		d2buff(it_pronoun(actor, 1));
	if (match_str(pvarname, "N_IS$")) {
		if (!it_plur(dobj)) d2buff(ext_code[wis])
			else d2buff(ext_code[ware]);
	}
	if (match_str(pvarname, "O_IS$")) {
		if (!it_plur(iobj)) d2buff(ext_code[wis])
			else d2buff(ext_code[ware]);
	}
	if (match_str(pvarname, "NAME_IS$")) {
		if (!it_plur(actor)) d2buff(ext_code[wis])
			else d2buff(ext_code[ware]);
	}

	if (match_str(pvarname, "N_WAS$")) {
		if (!it_plur(dobj)) d2buff(ext_code[wwas])
			else d2buff(ext_code[wwere]);
	}
	if (match_str(pvarname, "O_WAS$")) {
		if (!it_plur(iobj)) d2buff(ext_code[wwas])
			else d2buff(ext_code[wwere]);
	}
	if (match_str(pvarname, "NAME_WAS$")) {
		if (!it_plur(actor)) d2buff(ext_code[wwas])
			else d2buff(ext_code[wwere]);
	}
	if (match_str(pvarname, "THE_N$")) {
		theset(fill_buff, dobj);
		return 1;
	}
	if (match_str(pvarname, "THE_O$")) {
		theset(fill_buff, iobj);
		return 1;
	}
	if (match_str(pvarname, "THE_NAME$")) {
		theset(fill_buff, actor);
		return 1;
	}
	if (match_str(pvarname, "THE_C$")) {
		theset(fill_buff, curr_creat_rec->obj);
		return 1;
	}
	if (match_str(pvarname, "C_NAME$"))
		num_name(curr_creat_rec, 0);
	if (match_str(pvarname, "TIME$")) {
		time_out(fill_buff);
		return 1;
	}

	if (pronoun_mode && match_str(pvarname, "YOU$"))
		youme("I", "you");
	if (pronoun_mode && match_str(pvarname, "ARE$"))
		youme("am", "are");
	if (pronoun_mode && match_str(pvarname, "YOU_OBJ$"))
		youme("me", "you");
	if (pronoun_mode && match_str(pvarname, "YOUR$"))
		youme("my", "your");
	if (pronoun_mode && match_str(pvarname, "YOU'RE$"))
		youme("i'm", "you're");
	return 0;  /* Don't recognize $word$ */
}



static int capstate(const char *varname) {
	if (islower(varname[0])) return 0; /* $word$ */
	if (islower(varname[1])) return 2; /* $Word$ */
	if (!isalpha(varname[1]) && varname[1] != 0
	        && islower(varname[2])) return 2;
	else return 1; /* $WORD$ */
}

static char fill_buff[FILL_SIZE]; /* Buffer to hold returned string */

static char *wordvar_match(const char **pvarname, char match_type,
                           int context, const char *pword)
/* Match_type=='#' for variables, '$' for parsed words */
/* Possible # forms: #VARn#, #CNTn# */
/* See above for $ forms */
/* Moves *pvarname to point after matched object */
{
	int i, hold_val, hold_prop;
	const char *start;

	start = *pvarname;
	if (match_type == '$') {
		i = wordcode_match(pvarname, fill_buff, context, pword);
		if (i == 0) return NULL;
		/* Now need to fix capitalization */
		switch (capstate(start)) {
		case 0:
		default:
			break; /* $word$ */
		case 1:  /* $WORD$ */
			for (i = 0; fill_buff[i] != '\0'; i++)
				fill_buff[i] = toupper(fill_buff[i]);
			break;
		case 2: /* $Word$ */
			fill_buff[0] = toupper(fill_buff[0]);
			break;
		}
	} else {  /* So match type is '#' */
		if (match_str(pvarname, "VAR")) {
			hold_val = extract_number(pvarname, VAR_NUM, '#');
			if (hold_val < 0) return NULL;
			hold_val = agt_var[hold_val];
		} else if (match_str(pvarname, "CNT") ||
		           match_str(pvarname, "CTR")) {
			hold_val = extract_number(pvarname, CNT_NUM, '#');
			if (hold_val < 0) return NULL;
			hold_val = cnt_val(agt_counter[hold_val]);
		} else if (match_str(pvarname, "PROP")) {
			extract_prop_val(pvarname, &hold_prop, &hold_val, 1, '#');
			if (hold_prop == BAD_PROP) hold_val = 0;
		} else
			return NULL;

		/* Now to convert hold_val into a string */
		sprintf(fill_buff, "%d", hold_val);

	}
	return fill_buff;
}

static char  *format_line(const char *s, int context, const char *pword)
/* Do $word$ substituations; return the result */
{
	char *t; /* The new string after all the substitutions. */
	int t_size; /* How much space has been allocated for it. */
	const char *p, *oldp;  /* Pointer to the original string */
	int i;
	char *fill_word, *q; /* Word used to fill in the blanks, and a pointer
            used to iterate through it*/
	char fill_type; /* '#'=#variable#, '$'=$word$ */

	/* Need to do subsitutions and also correct for tabs */
	t_size = 200;
	t = (char *)rmalloc(t_size + FILL_SIZE + 10);
	just_seen_adj = 0;

	/* Note that I leave some margin here: t is 310 characters, but i will never
	   be allowed above around 200. This is to avoid having to put special
	   checking code throughout the following to make sure t isn't overrun */
	for (p = s, i = 0; *p != '\0'; p++) {
		if (i >= t_size) {
			t_size = i + 100;
			t = (char *)rrealloc(t, t_size + FILL_SIZE + 10);
		}
		if (!rspace(*p) && *p != '$')
			just_seen_adj = 0;
		if (*p == '$' || *p == '#')  {
			/* Read in $word$ or #var# and do substitution */
			fill_type = *p;
			oldp = p++;  /* Save old value in case we are wrong and then
          increment p */
			fill_word = wordvar_match(&p, fill_type, context, pword);
			if (fill_word == NULL) {
				/*i.e. no match-- so just copy it verbatim */
				t[i++] = fill_type;
				just_seen_adj = 0;
				p = oldp; /* Go back and try again... */
			} else { /* Fill in word */
				p--;
				if (fill_word[0] == '\0') { /* Empty string */
					/* We need to eliminate a 'double space' in this case */
					if ((oldp == s || rspace(*(oldp - 1))) && rspace(*(p + 1)))
						p++;
				} else  /* Normal case */
					for (q = fill_word; *q != '\0';)
						t[i++] = *q++;
			}
		} /* End $/# matcher */
		else
			t[i++] = *p;
	} /* End scanning loop */

	if (aver < AGX00 && i > 0 && t[i - 1] == ' ') {
		/* For pre-Magx, delete trailing spaces */
		do
			i--;
		while (i > 0 && t[i] == ' ');
		i++;
	}
	t[i] = 0;
	t = (char *)rrealloc(t, i + 1);
	return t;
}

void raw_lineout(const char *s, rbool do_repl, int context, const char *pword) {
	char *outstr;

	if (do_repl) {
		outstr = format_line(s, context, pword);
		writestr(outstr);
		rfree(outstr);
	} else
		writestr(s);
}


static void lineout(const char *s, rbool nl, int context, const char *pword) {
	raw_lineout(s, 1, context, pword);
	if (nl) writeln("");
	else writestr(" ");
}

static void gen_print_descr(descr_ptr dp_, rbool nl,
                            int context, const char *pword) {
	int j;
	descr_line *txt;

	agt_textcolor(7);
	textbold = 0;
	agt_par(1);
	txt = read_descr(dp_.start, dp_.size);
	if (txt != NULL)
		for (j = 0; txt[j] != NULL; j++)
			lineout(txt[j], nl || (txt[j + 1] != NULL), context, pword);
	free_descr(txt);
	agt_par(0);
	agt_textcolor(7);
	textbold = 0;
}

void print_descr(descr_ptr dp_, rbool nl) {
	gen_print_descr(dp_, nl, MSG_DESC, NULL);
}

void quote(int msgnum) {
	char **qptr;
	descr_line *txt;
	int i;
	int len;

	txt = read_descr(msg_ptr[msgnum - 1].start, msg_ptr[msgnum - 1].size);
	if (txt != NULL) {
		for (len = 0; txt[len] != NULL; len++);
		qptr = (char **)rmalloc(len * sizeof(char *));
		for (i = 0; i < len; i++)
			qptr[i] = format_line(txt[i], MSG_DESC, NULL);
		free_descr(txt);
		textbox(qptr, len, TB_BORDER | TB_CENTER);
		rfree(qptr);
	}
}


void msgout(int msgnum, rbool add_nl) {
	print_descr(msg_ptr[msgnum - 1], add_nl);
}


#define MAX_NUM_ERR 240      /* Highest numbered STANDARD message */
#define OLD_MAX_STD_MSG 185

/* Fallback messages should always have msgid less than the original */
int stdmsg_fallback[MAX_NUM_ERR - OLD_MAX_STD_MSG] = {
	0, 0, 0, 12, 0, /* 186 - 190 */
	0, 0, 0, 0, 0, /* 191 - 195 */
	0, 13, 13, 5, 10,    /* 196 - 200 */
	10, 61, 10, 16, 59,  /* 201 - 205 */
	90, 107, 116, 135, 140, /* 206 - 210 */
	184, 3, 47, 185, 61,  /* 211 - 215 */
	0, 0, 0, 0, 0,         /* 216 - 220 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 221 - 230 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0 /* 231 - 240 */
};

void gen_sysmsg(int msgid, const char *s, int context, const char *pword)
/* Prints either STANDARD message number <msgid> or default msg <s>;
   A <msgid> of 0 means there is no standard message counterpart.
   <context> determines what $$ substitutions are meaningful
   <parseword> gives the $pword$ substitution for MSG_PARSE messages
   msgid 3 should probably *not* be redirected to avoid giving hints to
   the player as to what nouns exist in the game.
*/
{
	/* Use gamefile's redefined version of message? */
	rbool use_game_msg;
	rbool nl; /* Should it be followed by a newline? */

	nl = 1; /* By default, follow message with newline */

	/* The following msgids shouldn't be followed by newlines: */
	if (msgid == 1 || msgid == 145 || (msgid >= 218 && msgid <= 223)
	        || msgid == 225)
		nl = 0;

	if (DEBUG_SMSG) rprintf("\nSTD %d", msgid);

	use_game_msg = ((PURE_SYSMSG || s == NULL)
	                && msgid != 0 && msgid <= NUM_ERR
	                && err_ptr != NULL);

	if (use_game_msg) {
		/* Check for fall-back messages */
		if (err_ptr[msgid - 1].size <= 0
		        && msgid > OLD_MAX_STD_MSG && msgid <= MAX_NUM_ERR) {
			msgid = stdmsg_fallback[msgid - OLD_MAX_STD_MSG - 1];
			if (DEBUG_SMSG) rprintf("==> %3d", msgid);
		}
		if (msgid != 0 && err_ptr[msgid - 1].size > 0) {
			if (DEBUG_SMSG) rprintf(" : From gamefile\n");
			gen_print_descr(err_ptr[msgid - 1], nl, context, pword);
		} else use_game_msg = 0;
	}

	if (DEBUG_SMSG && !use_game_msg) rprintf(" : Default\n");

	if (!use_game_msg) {
		/* Either the game doesn't redefine the message, or we're ignoring
		   redefinitions */
		if (s == NULL) return;
		pronoun_mode = 1;
		lineout(s, nl, context, pword);
		pronoun_mode = !PURE_PROSUB;
	}
}


void sysmsg(int msgid, const char *s) {
	gen_sysmsg(msgid, s, MSG_RUN, NULL);
}


void alt_sysmsg(int msgid, const char *s, parse_rec *new_dobjrec, parse_rec *new_iobjrec) {
	parse_rec *save_dobjrec, *save_iobjrec;
	integer save_dobj, save_iobj;

	save_dobj = dobj;
	save_dobjrec = dobj_rec;
	dobj = p_obj(new_dobjrec);
	dobj_rec = new_dobjrec;

	save_iobj = iobj;
	save_iobjrec = iobj_rec;
	iobj = p_obj(new_iobjrec);
	iobj_rec = new_iobjrec;

	gen_sysmsg(msgid, s, MSG_RUN, NULL);

	dobj = save_dobj;
	dobj_rec = save_dobjrec;
	iobj = save_iobj;
	iobj_rec = save_iobjrec;
}


void sysmsgd(int msgid, const char *s, parse_rec *new_dobjrec)
/* Front end for sysmsg w/alternative direct object */
{
	alt_sysmsg(msgid, s, new_dobjrec, NULL);
}


/* -------------------------------------------------------------------- */
/*  QUESTION and ANSWER processing                                      */
/* -------------------------------------------------------------------- */


static char *match_string(char *ans, char *corr_ans, int n)
/* Searches for s (w/ surrounding whitespace removed) inside ans */
/* looking at only n characters of s */
{
	char *s;
	char *corr;
	int i;

	s = rstrdup(corr_ans);
	for (i = n - 1; i > 0 && isspace(s[i]); i--); /* Kill trailing whitespace */
	s[i + 1] = 0;
	for (i = 0; s[i] != 0; i++) s[i] = tolower(s[i]);
	for (i = 0; isspace(s[i]); i++); /* Kill leading whitespace */
	corr = strstr(ans, s + i);
	rfree(s);
	return corr;
}


static rbool check_answer(char *ans, long start, long size)
/* qnum has already been fixed to start from 0 */
/*   Master's edition answer checker. Master's edition answers can */
/* be seperate by AND and OR characters. If there is one OR in the */
/* answer, all ANDs will also be treated as ORs */
/*   Furthermore, AND-delimited strings must appear in the correct order */
/* unless PURE_ANSWER is false */
{
	char *corr, *corr2; /* Pointer into answer to match correct answers */
	int match_mode; /* 0=AND mode, 1=OR mode */
	descr_line *astr; /* Holds the answer string */
	int i; /* Index to line of astr we're on. */
	char *p, *q, *r;  /* Used to break astr up into pieces and
        loop over them */

	astr = read_descr(start, size);
	if (astr == NULL) {
		if (!PURE_ERROR)
			writeln("GAME ERROR: Empty answer field.");
		return 1;
	}

	match_mode = 0;
	for (i = 0; astr[i] != NULL; i++)
		if (strstr(astr[i], "OR") != NULL) {
			match_mode = 1;
			break;
		}

	corr = ans;
	for (i = 0; astr[i] != NULL; i++) { /* loop over all lines of the answer */
		p = astr[i];
		do {
			q = strstr(p, "OR");
			r = strstr(p, "AND");
			if (q == NULL || (r != NULL && r < q)) q = r;
			if (q == NULL) q = p + strlen(p); /* i.e. points at the concluding null */
			corr2 = match_string(corr, p, q - p);
			if (corr2 == NULL && match_mode == 0) {
				free_descr(astr);
				return 0;
			}
			if (corr2 != NULL && match_mode == 1) {
				free_descr(astr);
				return 1;
			}
			if (PURE_ANSWER && match_mode == 0) corr = corr2;
			if (*q == 'O') p = q + 2;
			else if (*q == 'A') p = q + 3;
			else assert(*q == 0);
		} while (*q != 0);
	}
	free_descr(astr);
	if (match_mode == 0) return 1; /* AND: Matched them all */
	else return 0;  /* OR: Didn't find a single match */
}


/* Does the answer in string ans match answer anum? */
/* Warning: this changes and then rfrees ans */
rbool match_answer(char *ans, int anum) {
	char *corr;
	rbool ans_corr;

	for (corr = ans; *corr != 0; corr++)
		*corr = tolower(*corr);
	if (answer != NULL) {
		/* corr=strstr(ans,answer[anum]); */
		corr = match_string(ans, answer[anum], strlen(answer[anum]));
		rfree(ans);
		if (corr == NULL) return 0;
	} else if (ans_ptr != NULL) {
		ans_corr = check_answer(ans, ans_ptr[anum].start, ans_ptr[anum].size);
		rfree(ans);
		return ans_corr;
	} else writeln("INT ERR: Invalid answer pointer.");
	return 1;

}


rbool ask_question(int qnum)
/* 1=got it right, 0=got it wrong */
{
	char *ans;

	qnum--;

	/* Now actually ask the question and compare the answers */
	if (question != NULL)
		writeln(question[qnum]);
	else if (quest_ptr != NULL)
		print_descr(quest_ptr[qnum], 1);
	else {
		writeln("INT ERR: Invalid question pointer");
		return 1;
	}
	ans = agt_readline(2);
	return match_answer(ans, qnum);
}


/* -------------------------------------------------------------------- */
/*  Miscellaneous support routines */
/* -------------------------------------------------------------------- */

long read_number(void) {
	char *s, *err;
	long n;

	n = 1;
	do {
		if (n != 1) gen_sysmsg(218, "Please enter a *number*. ", MSG_MAIN, NULL);
		s = agt_readline(1);
		n = strtol(s, &err, 10);
		if (err == s) err = NULL;
		rfree(s);
	} while (err == NULL);
	return n;
}




void runptr(int i, descr_ptr dp_[], const char *msg, int msgid,
            parse_rec *nounrec, parse_rec *objrec)
/* Prints out description unless it doesn't exist, in which
   case it prints out either system message #msgid or the message
   contained in msg. */
{
	if (dp_[i].size > 0) print_descr(dp_[i], 1);
	else alt_sysmsg(msgid, msg, nounrec, objrec);
}



/* Score modes:
      S:Score, R:Room  +=list '(out of..'), -=don't list at all.
    0-- S+ R+
    1-- S+ R
    2-- S  R+
    3-- S  R
    4-- S+ R-
    5-- S  R-
    6-- S- R+
    7-- S- R
    8-- S- R- and disable SCORE.
    */


void print_score(void) {
	char s[80];
	int i, rmcnt, totroom;

	if (score_mode < 5) {
		if (score_mode == 0 || score_mode == 1 || score_mode == 4)
			sprintf(s, "Your score is %ld (out of %ld possible).", tscore, max_score);
		else sprintf(s, "Your score is %ld.", tscore);
		writeln(s);
	}

	if (score_mode < 4 || score_mode == 6 || score_mode == 7) {
		rmcnt = 0;
		totroom = 0;
		for (i = 0; i <= maxroom - first_room; i++)
			if (!room[i].unused) {
				if (room[i].seen) rmcnt++;
				/* Should really compute this once at the beginning, but */
				/* I don't want to add yet another global variable, particulary */
				/* since this is only used here. */
				totroom++;
			}
		if (score_mode % 2 == 0)
			sprintf(s, "You have visited %d locations (out of %d in the game)", rmcnt,
			        totroom);
		else sprintf(s, "You have visited %d locations.", rmcnt);
		writeln(s);
	}
}


int normalize_time(int tnum) { /* Convert hhmm so mm<60 */
	int min, hr;

	min = tnum % 100; /* The minutes */
	hr = tnum / 100; /* The hours */
	hr += min / 60;
	min = min % 60;
	while (hr < 0) hr += 24;
	hr = hr % 24;
	return hr * 100 + min;
}


void add_time(int dt) {
	int min, hr;

	min = curr_time % 100; /* The minutes */
	hr = curr_time / 100; /* The hours */
	if (aver == AGT183) min += dt; /* AGT 1.83 version */
	else {  /* Normal version */
		min += dt % 100;
		hr += dt / 100;
	}
	while (min < 0) {
		min = min + 60;
		hr++;
	}
	hr += min / 60;
	min = min % 60;
	while (hr < 0) hr += 24;
	hr = hr % 24;
	curr_time = hr * 100 + min;
}


void look_room(void) {
	compute_seen();
	writeln("");
	if (islit()) {
		if (room[loc].name != NULL && room[loc].name[0] != 0 &&
		        (!PURE_ROOMTITLE)) {
			agt_textcolor(-1);  /* Emphasized text on */
			writestr(room[loc].name);
			agt_textcolor(-2);
			writeln("");
		}  /* Emphasized text off */
		if (room_firstdesc && room[loc].initdesc != 0)
			msgout(room[loc].initdesc, 1);
		else if (room_ptr[loc].size > 0)
			print_descr(room_ptr[loc], 1);
		print_contents(loc + first_room, 1);
		if (listexit_flag)
			v_listexit();
	} else
		sysmsg(room[loc].light == 1 ? 6 : 7,
		       "It is dark. $You$ can't see anything.");
	room_firstdesc = 0;
	do_look = 0;
}


static void run_autoverb(void) {
	int v0; /* Will hold the verb number of the autoverb */
	int savevb;
	integer saveactor, savedobj, saveiobj;
	parse_rec *save_actor_rec, *save_dobj_rec, *save_iobj_rec;
	word saveprep;


	beforecmd = 1;

	/* This is the penalty for vb, actor, etc being global variables. */
	savevb = vb;
	saveactor = actor;
	savedobj = dobj;
	saveprep = prep;
	saveiobj = iobj;
	save_actor_rec = copy_parserec(actor_rec);
	save_dobj_rec = copy_parserec(dobj_rec);
	save_iobj_rec = copy_parserec(iobj_rec);

	if (room[loc].autoverb != 0) {
		v0 = verb_code(room[loc].autoverb);
		(void)scan_metacommand(0, v0, 0, 0, 0, NULL);
	}
	free_all_parserec();
	vb = savevb;
	actor = saveactor;
	dobj = savedobj;
	iobj = saveiobj;
	actor_rec = save_actor_rec;
	dobj_rec = save_dobj_rec;
	iobj_rec = save_iobj_rec;
	prep = saveprep;
}



/* ------------------------------------------------------------------- */
/* MAIN COMMAND EXECUTION ROUTINES-- */
/*  These routines handle the execution of player commands  */
/*  Then they change the status line, update counters, etc. */
/* ------------------------------------------------------------------- */

static void creat_initdesc(void) {
	int i;

	creatloop(i)
	if (creature[i].location == loc + first_room &&
	        creature[i].initdesc != 0) {
		msgout(creature[i].initdesc, 1);
		creature[i].initdesc = 0;
	}
}

/* Print out picture names, remember to put intro before first one. */
/* This should be called with s==NULL before and after:
   before to reset it, after to put the trailing newline on. */
void listpictname(const char *s) {
	static rbool first_pict = 1; /* True until we output first picture */

	if (s == NULL) {
		if (!first_pict) writeln(""); /* Trailing newline */
		first_pict = 1;
		return;
	}
	if (first_pict) {
		writeln(""); /* Skip a line */
		sysmsg(219, "  Illustrations:");
		first_pict = 0;
	}
	writestr("  ");
	writestr(s);
}


void listpict(int obj) {
	char *s;

	if (it_pict(obj) != 0) {
		s = objname(obj);
		listpictname(s);
		rfree(s);
	}
}


void list_viewable(void)
/* List pictures that can be viewed, if any */
{
	int i;

	listpictname(NULL);

	if (room[loc].pict != 0)
		listpictname("scene");
	contloop(i, 1)
	listpict(i);
	contloop(i, 1000)
	listpict(i);
	contloop(i, loc + first_room)
	listpict(i);

	for (i = 0; i < maxpix; i++)
		if (room[loc].PIX_bits & (1L << i))
			listpictname(dict[pix_name[i]]);
	listpictname(NULL);
}



void newroom(void) {
	rbool save_do_look;
	integer prevloc;

	do {
		save_do_look = do_look;
		if (do_look == 1) look_room();
		creat_initdesc();
		if (save_do_look == 1 && aver >= AGTME10)
			list_viewable();   /* Print out picts that can be viewed here. */
		do_look = 0;

		prevloc = loc;
		if (do_autoverb) {
			do_autoverb = 0;
			run_autoverb();
		}

		if (!room[loc].seen) { /* This only runs on the first turn */
			room[loc].seen = 1;
			tscore += room[loc].points;
		}
	} while (prevloc != loc); /* Autoverb could move player */
}


static int min_delta(void) {
	return (aver == AGT183) ? 1 : 0 ;
}


void increment_turn(void) {
	int i;

	compute_seen();

	newlife_flag = 0;

	if (quitflag) return;

	newroom();

	if (winflag || deadflag || endflag) return;

	if (was_metaverb) return;  /* No time should pass during a metaverb. */

	turncnt++;
	/* Now increment the time counter */
	if (delta_time > 0) {
		if (PURE_TIME)
			add_time(agt_rand(min_delta(), delta_time));
		else /* if !PURE_TIME */
			add_time(delta_time);
	}

	for (i = 0; i <= CNT_NUM; i++)
		if (agt_counter[i] >= 0) ++agt_counter[i];
	creatloop(i)
	if (creature[i].location == loc + first_room && creature[i].hostile &&
	        creature[i].timethresh > 0) {
		parse_rec tmpcreat; /* Used for creature messages */
		make_parserec(i + first_creat, &tmpcreat);
		curr_creat_rec = &tmpcreat;

		if (++creature[i].timecounter >= creature[i].timethresh) {
			/* Creature attacks */
			sysmsg(16, "$The_c$$c_name$ suddenly attacks $you_obj$!");
			sysmsg(creature[i].gender == 0 ? 17 : 18,
			       "    $You$ try to defend $your$self, but $the_c$$c_name$ "
			       "kills $you_obj$ anyhow.");
			deadflag = 1;
		} else /* 'Angrier' messages */
			if (creature[i].timethresh > 0 &&
			        creature[i].timecounter > creature[i].timethresh - 3)
				sysmsg(15, "$The_c$$c_name$ seems to be getting angrier.");
	}
}


/* Wrapper for increment_turn used by exec routines below.
   This just checks to make sure we're not one of the 1.8x versions
   (which call increment turn from elsewhere) */
static void exec_increment_turn(void) {
	if (PURE_AFTER) increment_turn();
}

static void end_turn(void) {
	if (textbold) agt_textcolor(-2);
	textbold = 0;
	set_statline();

	if (quitflag) return;

	if (notify_flag && !was_metaverb) {
		if (old_score < tscore)
			sysmsg(227, "  [Your score just went up]");
		else if (old_score > tscore)
			sysmsg(228, "  [Your score just went down]");
	}
	old_score = tscore;

}



static void set_pronoun(int item) {
	if (item == 0) return;
	switch (it_gender(item)) {
	case 0:
		if (it_plur(item))
			last_they = item;
		last_it = item;  /* Change: last_it will be set even if the
               noun is plural */
		break;
	case 1:
		last_she = item;
		break;
	case 2:
		last_he = item;
		break;
	default:
		break;
	}
}


/* True if the current noun is the last one in the list. */
static rbool lastnoun(parse_rec *list) {
	if (list->info == D_END) return 1;
	list++;
	while (list->info == D_AND) list++;
	return (list->info == D_END);
}




static void runverbs(parse_rec *actor0, int vnum,
                     parse_rec *lnoun, word prep0, parse_rec *iobj0)
/* The zeros are postpended simply to avoid a name conflict */
{
	parse_rec *currnoun;

	textbold = 0;
	do_look = 0;
	do_autoverb = 0;
	was_metaverb = 0;
	actor = actor0->obj;
	actor_rec = copy_parserec(actor0);
	vb = vnum;
	dobj = lnoun[0].obj;
	dobj_rec = copy_parserec(lnoun);
	prep = prep0;
	iobj = iobj0->obj;
	iobj_rec = copy_parserec(iobj0);
	set_pronoun(actor);  /* Basically the last one that isn't 0 will stick */
	set_pronoun(iobj0->obj);
	was_metaverb = 0; /* Most verbs are not metaverbs; assume this by default */
	start_of_turn = 1;
	end_of_turn = 0;

	if (lnoun[0].info == D_END || lnoun[0].info == D_ALL) {
		end_of_turn = 1;
		exec_verb();
		if (doing_restore) {
			free_all_parserec();
			return;
		}
		if (PURE_AND) exec_increment_turn();
	} else for (currnoun = lnoun; currnoun->info != D_END; currnoun++)
			if (currnoun->info != D_AND) {
				free_all_parserec();
				end_of_turn = lastnoun(currnoun);
				actor = actor0->obj;
				actor_rec = copy_parserec(actor0);
				vb = vnum;
				dobj = currnoun->obj;
				dobj_rec = copy_parserec(currnoun);
				iobj = iobj0->obj;
				iobj_rec = copy_parserec(iobj0);
				set_pronoun(dobj);
				exec_verb();
				if (doing_restore) return;
				if (PURE_AND)
					exec_increment_turn();
				else
					start_of_turn = 0;
				if (quitflag || winflag || deadflag || endflag)
					break;
			}
	assert(end_of_turn);
	if (!PURE_AND) exec_increment_turn();
	end_turn();
	free_all_parserec();
}


/* The following store values for use by AGAIN */
/* (so AGAIN can be implemented just by executing runverbs w/ the saved
   values) */
static int save_vnum;
static word save_prep;
static parse_rec save_actor;
static parse_rec save_obj;
parse_rec *save_lnoun = NULL;



void exec(parse_rec *actor_, int vnum,
          parse_rec *lnoun, word prep_, parse_rec *iobj_) {

	cmd_saveable = 0;
	pronoun_mode = !PURE_PROSUB;

	if (vnum == verb_code(ext_code[wagain]) && lnoun[0].info == D_END
	        && iobj_->info == D_END &&
	        (actor_->info == D_END || actor_->obj == save_actor.obj))
		if (save_lnoun == NULL) {
			rfree(lnoun);
			sysmsg(186,
			       "You can't use AGAIN until you've entered at least one command.");
			return;
		} else {
			memcpy(actor_, &save_actor, sizeof(parse_rec));
			vnum = save_vnum;
			prep_ = save_prep;
			memcpy(iobj_, &save_obj, sizeof(parse_rec));
			rfree(lnoun);
			lnoun = save_lnoun;
			save_lnoun = NULL;
		}
	else
		realverb = input[vp];


	runverbs(actor_, vnum, lnoun, prep_, iobj_);

	if (cmd_saveable) {
		if (save_lnoun != NULL) rfree(save_lnoun);

		memcpy(&save_actor, actor_, sizeof(parse_rec));
		save_vnum = vnum;
		save_lnoun = lnoun;
		lnoun = NULL;
		save_prep = prep_;
		memcpy(&save_obj, iobj_, sizeof(parse_rec));
	} else
		rfree(lnoun);
}

} // End of namespace AGT
} // End of namespace Glk
