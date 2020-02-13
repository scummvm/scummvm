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
#include "common/str.h"

namespace Glk {
namespace AGT {

/* ------------------------------------------------------------------- */
/*  Description Pointers   */


descr_ptr intro_ptr;
descr_ptr title_ptr, ins_ptr; /* Only defined if agx_file is true */
descr_ptr *err_ptr; /* [NUM_ERR];*/

descr_ptr *msg_ptr; /* [MAX_MSG];*/
descr_ptr *help_ptr, *room_ptr, *special_ptr; /*[ROOM] */
descr_ptr *noun_ptr, *text_ptr, *turn_ptr, /* [NOUN] */
          *push_ptr, *pull_ptr, *play_ptr;
descr_ptr *talk_ptr, *ask_ptr, *creat_ptr; /* [CREAT] */

descr_ptr *quest_ptr, *ans_ptr; /* [MAX_QUEST] */
tline *question, *answer; /* [MAX_QUEST] */



/* ------------------------------------------------------------------------ */
/* Dynamically allocated data blocks (which are pointed to from elsewhere)  */

char **dict;  /* dict[n] points to the nth dictionary word */
long dp;  /* Dictionary pointer: number of words in dict */

#define DICT_INIT 12*1024 /* Starting size of dictstr */
#define DICT_GRAN 1024  /* Granularity of dictstr size requests 
               must be at least 4. */
char *dictstr;  /* Pointer to memory block containing dict words */
long dictstrptr, dictstrsize;
/* dictstrptr points to the first unused byte in dictstr.
   dictstrsize points to the end of the space currently allocated for
      dictstr.
*/

char *static_str; /*Static string space */
long ss_end; /* Pointer to end of used space in above */
long ss_size; /* Current size of static string space */

word *syntbl;  /* Synonym list space */
slist synptr; /* Points to end of used space */
long syntbl_size; /* Long so we can catch overflows */

long descr_maxmem;
char *mem_descr; /* Copy of descriptor in memory */



/* ------------------------------------------------------------------------ */
/*  Data structures used internally by agtread.c   */

/*The following are all set to NULL after agtread finishes. */
long *cmd_ptr; /* ME only;Points to cmd start locs in gamefile.*/
long *room_name, *noun_sdesc, *noun_pos, *creat_sdesc;
long *t_pictlist, *t_pixlist, *t_songlist, *t_fontlist;

/* These are only used by agtout (to allow the AGT reading routines to
   pass back the count of nouns inside the given object) */
integer *room_inside, *noun_inside, *creat_inside;

/* This is used to translate ASCII codes */
uchar fixchar[256];

rbool text_file;  /* Set if we are currently opening a binary file. */
#ifdef OPEN_AS_TEXT
rbool open_as_binary;  /* Open text files as binary, anyhow. */
#endif

/*   The following are AGT 'purity' flags; they turn off features of */
/* my interpreter that are not fully consistent with the original AGT */
/* and so could break some games. Some of these are trivial improvements; */
/* some are more radical and should be used with caution. Several are */
/* only useful if a game was designed with them in mind. */
/*   In all cases, setting the flag to 1 more closely follows the */
/* behavior of the original interpreters */
/* WARNING: Most of these haven't been tested in the non-default state. */
/* Most of these will eventually become variables settable by the user */
/*   or from a (possibly game-specific) configuration file */

rbool PURE_INPUT = 1; /* Is the input line bold? */

rbool PURE_TONE = 0; /* Is low level sound enabled? */


/*-------------------------------------------------------------------*/
/*  Misc. things to support the tokenizer and the dictionry.         */
/*-------------------------------------------------------------------*/

/* The following should not be changed without also changing the
   wtype enum statement in interp.h to match */
static const char *ext_voc[] = {
	"the", "my", "a", "an", /* These 4 are ignored in canonical AGT */
	"then", ".", ";", "and", "," , "its", "all", "undo", "look", "g",
	"pick", "go", "exits", "talk", "take", "door", "again", "but", "except",
	"scene", "everything", "listexit", "listexits", "close",
	"verb", "noun", "adjective", "prep", "object", "name", "step",
	" any", "either", "both", "everyone", "everybody",
	"he", "she", "it", "they", "him", "her", "them", "is", "are", "oops",
	"was", "were",
	/* Everything between 'in' and 'about' should be a preposition */
	"in", "out", "into", "at", "to", "across", "inside", "with", "near", "for",
	"of", "behind", "beside", "on", "off", "under", "from", "through",
	"toward", "towards", "between", "around", "upon", "thru",
	"by", "over", "up", "down",
	"about"
};
/* 'about' must be the last element of this list */


/*-------------------------------------------------------------------*/
/* Routines to read in and use various auxilary files.               */
/*   (.TTL, .INS, .VOC, .CFG)                                        */
/*-------------------------------------------------------------------*/

static rbool emptyline(unsigned char *s)
/* Check if s consists only of white space and control characters */
{
	unsigned char *p;

	for (p = s; *p != 0; p++)
		if (!rspace(*p) && *p > 26) return 0;
	return 1;
}

static void print_title(fc_type fc) {
	int height;
	signed char center_mode; /* Center title? */
	descr_line *buff;
	char *s;
	rbool skip_line; /* Skip first line: it has COLORS */

	if (agx_file)
		buff = read_descr(title_ptr.start, title_ptr.size);
	else
		buff = read_ttl(fc);

	if (buff == NULL) {
		writeln("");
		writeln("");
		s = formal_name(fc, fNONE);
		if (s != NULL) {
			s[0] = toupper(s[0]);
			agt_center(1);
			agt_textcolor(-1);
			writeln(s);
			agt_textcolor(-2);
			agt_center(0);
			rfree(s);
		}
		writeln("");
		writeln("");

		if (aver < AGX00)
			writeln("This game was created with Malmberg and Welch's Adventure "
			        "Game Toolkit; it is being executed by");
		else writeln("This game is being executed by ");
		writeln("");
		height = 0;
	} else {
		if (buff[0] != NULL && strncasecmp(buff[0], "COLORS", 6) == 0) {
			/* Do screen colors */
			skip_line = 1;
		} else skip_line = 0;
		/* Compute height and count the number of non-empty lines
		   starting with spaces. We use height as a loop variable
		   and center_mode to store the count temporarily. */
		center_mode = 0;
		for (height = skip_line; buff[height] != NULL; height++)
			if (!emptyline((uchar *)buff[height])) {
				if (rspace(buff[height][0])) center_mode++;
				else center_mode--;
			}

		if (box_title || aver == AGTCOS) center_mode = TB_CENTER;
		else /* includes aver==AGT135 */
			if (center_mode <= 0) center_mode = TB_CENTER;
			else center_mode = TB_NOCENT;

		if (!bold_mode) agt_textcolor(-1);
		agt_clrscr();
		textbox(buff + skip_line, height - skip_line, center_mode |
		        (bold_mode ? 0 : TB_BOLD) | TB_TTL |
		        (box_title ? TB_BORDER : 0));
		if (!bold_mode) agt_textcolor(-2); /* Bold off */
	}  /* End printing of title proper */

	if (agx_file)
		free_descr(buff);
	else
		free_ttl(buff);

	agt_textcolor(7);
	agt_center(1);
	if (buff != NULL) {
		if (aver < AGX00 && height <= screen_height - 6)
			writeln("[Created with Malmberg and Welch's Adventure Game Toolkit]");
		if (height <= screen_height - 9) writeln("");
		if (height <= screen_height - 5) writeln("This game is being executed by");
	}
	agt_textcolor(-1);
	s = (char *)rmalloc(80);
	if (height <= screen_height - 5)
		sprintf(s, "AGiliTy: "
		        "The (Mostly) Universal AGT Interpreter  %s", version_str);
	else
		sprintf(s, "Being run by AGiliTy  %s, "
		        "Copyright (C) 1996-99,2001 Robert Masenten",
		        version_str);
	writeln(s);
	rfree(s);
	agt_textcolor(-2);
	if (height <= screen_height - 5)
		writeln("Copyright (C) 1996-99,2001 by Robert Masenten");
	if (height <= screen_height - 3) writeln(portstr);
	if (height <= screen_height - 10) writeln("");
	agt_center(0);
}



/* .INS reading routines -------------------------------------- */

void print_instructions(fc_type fc) {
	char *buffer;
	uchar *s;

	writeln("INSTRUCTIONS:");
	if (open_ins_file(fc, 1)) {  /* Instruction file exists */
		while (NULL != (buffer = read_ins_line())) {
			for (s = (uchar *)buffer; *s != 0; s++) *s = trans_ascii[*s];
			writeln(buffer);
		}
	}
	writeln("");
}

/* Routines to build the verb menu from the .VOC information */

static void build_verbmenu(void) {
	int i, n;
	char *p, *d;

	verbmenu = (menuentry *)rmalloc(vm_size * sizeof(menuentry));
	vm_width = 0;
	for (i = 0; i < vm_size; i++) {
		p = verbmenu[i];
		d = dict[verbinfo[i].verb];
		n = 0;
		for (; n < MENU_WIDTH && *d != 0; p++, d++, n++) *p = *d;
		if (verbinfo[i].prep != 0 && n + 1 < MENU_WIDTH) {
			*p++ = ' ';
			d = dict[verbinfo[i].prep];
			*p++ = toupper(*d++);
			for (; n < MENU_WIDTH && *d != 0; p++, d++, n++) *p = *d;
		}
		verbmenu[i][0] = toupper(verbmenu[i][0]);
		*p = 0;
		if (n > vm_width) vm_width = n;
	}
}

/* .CFG reading routines  -------------------------------------------- */

#define opt(s) (strcasecmp(optstr[0],s)==0)

/* These are the interpreter specific options; this is called
   from cfg_option in agtdata.c. */
void agil_option(int optnum, char *optstr[], rbool setflag, rbool lastpass) {
	if (opt("ibm_char")) fix_ascii_flag = !setflag;
	else if (!lastpass) return; /* On the first pass through the game specific
                 file, we ignore all but the above options */
	else if (opt("tone")) PURE_TONE = setflag;
	else if (opt("input_bold")) PURE_INPUT = setflag;
	else if (opt("force_load")) FORCE_VERSION = setflag;
	else if (!agt_option(optnum, optstr, setflag)) /* Platform specific options */
		rprintf("Invalid option %s\n", optstr[0]);
}

/*-------------------------------------------------------------------*/
/* Tokeniser: Split input into words and look them up in dictionary  */
/*-------------------------------------------------------------------*/

static rbool noise_word(word w) {
	if (w == ext_code[wthe] || w == ext_code[wa] || w == ext_code[wan]) return 1;
	if (w == ext_code[wmy]) return 1;
	if (aver >= AGT18 && aver <= AGT18MAX && w == ext_code[wis]) return 1;
	return 0;
}


static rbool check_dot(char *prevtext, int prevcnt, char *lookahead)
/* This routine is devoted to trying to figure out whether
 we should treat '.' as punctuation or as a letter. */
/* It returns true if '.' should be treated as punctuation. */
/* prevtext=the current word, as far as it has been parsed.
 prevcnt=the number of letters in prevtext
 [which is *not* 0 terminated] */
/* lookahead=the rest of the current input line *after* the period. */
{
	int i, endword, restcnt;

	if (!PURE_DOT) return 1;  /* No words with periods in them, so it must
                   be punctuation. */
	/*  We just start scanning the dictionary to see if any of them
	    are possible matches, looking ahead as neccessary. */

	/* Find the next unambiguous word end. This ignores possible
	   word ends caused by periods. */
	for (endword = 0; lookahead[endword] != 0; endword++)
		if (isspace(lookahead[endword]) ||
		        lookahead[endword] == ',' || lookahead[endword] == ';')
			break;

	for (i = 0; i < dp; i++) {
		if (i == ext_code[wp]) continue; /* Ignore matches with the word ".". */

		/* If it doesn't contain a '.' at the right location, there is no
		   point in continuing. */
		restcnt = strlen(dict[i]);
		if (restcnt <= prevcnt || dict[i][prevcnt] != '.') continue;

		/* Now make sure the previous characters are correct */
		if (strncasecmp(prevtext, dict[i], prevcnt) != 0) continue;

		/* Finally, compare the trailing text. This is complicated by
		 the fact that the trailing text could itself contain ambiguous '.'s */
		restcnt -= prevcnt + 1; /* Number of characters in dict entry after '.' */
		if (restcnt > endword) continue; /* Dictionary entry is longer than
                       following text */

		/* Check to see if the dictionary entry can be found in the lookahead
		   buffer */
		if (strncasecmp(lookahead, dict[i] + prevcnt + 1, restcnt) != 0) continue;

		if (restcnt == endword) return 0; /* We have a match */
		/* At this point, we know that restcnt<endword and the dictionary
		   entry matches as far as restcnt. */
		/* endword ignores '.', though, so it could be we have a match
		   but are missing it because it is period-terminated. Check this. */
		if (lookahead[restcnt] == '.')  return 0;

		/* Otherwise, no match... try again with the next word... */
	}
	return 1; /* No matches: treat it as punctuation. */
}






static void tokenise(char *buff)
/* Convert input string into vocabulary codes */
/* 0 here denotes an unrecognized word and -1 marks the end. */
{
	int ip_, j, k;
	rbool punctuation;

	j = 0;
	ip_ = 0;
	k = 0; /* k is the character pointer */
	for (k = 0;; k++) {
		/* If PURE_DOT is set, then there are periods in some of the dictionary
		   words, so '.' could be a letter or punctuation-- we have to examine
		   context to figure out which. */
		if (buff[k] == '.' && PURE_DOT)
			/* Note: check_dot is in agtdata.c, since it needs to access
			internal implementation details of the dictionary */
			punctuation = check_dot(in_text[ip_], j, buff + k + 1);
		else
			punctuation = (buff[k] == ',' || buff[k] == ';' || buff[k] == '.');
		if (buff[k] != 0 && !isspace(buff[k]) && !punctuation) {
			if (j < WORD_LENG - 1)
				in_text[ip_][j++] = buff[k];
		} else if (j > 0) { /* End of word: add it to input */
			in_text[ip_][j] = 0;
			input[ip_] = search_dict(in_text[ip_]);
			if (input[ip_] == -1) input[ip_] = 0;
			else if (input[ip_] == 0) input[ip_] = ext_code[w_any]; /* _Real_ 'ANY' */
			if (!noise_word(input[ip_])) ip_ += 1;
			/* i.e. if not one of the four ignored words, advance */
			j = 0;
		} /* If j=0 and not punct, then no new word; just skip the whitespace */
		if (punctuation) {
			in_text[ip_][0] = buff[k];
			in_text[ip_][1] = 0;
			input[ip_] = search_dict(in_text[ip_]);
			if (input[ip_] == -1) input[ip_] = 0;
			j = 0;
			ip_++;
		}
		if (ip_ >= MAXINPUT - 1) {
			writeln("Too many words in input; ignoring rest of line.");
			break;
		}
		if (buff[k] == 0) break;
	}
	input[ip_] = -1;
	in_text[ip_][0] = 0;
}



/*-------------------------------------------------------------------*/
/* Main game loop: Get player input and call the parser.             */
/*-------------------------------------------------------------------*/

static void game_end(void) {
	rbool done_flag;
	char *s;

	if (winflag || deadflag) {
		writeln("");
		writeln("");
		agt_center(1);
		if (winflag)
			gen_sysmsg(148, "***** $You$ have won! *****", MSG_MAIN, NULL);
		if (deadflag)
			gen_sysmsg(147, "***** $You$ have died! *****", MSG_MAIN, NULL);
		writeln("");
		writeln("");
		agt_center(0);
	}
	if (deadflag && !endflag) {
		if (curr_lives > 1) { /* Resurrection code */
			if (curr_lives == max_lives)
				gen_sysmsg(151, "Hmmm.... so $you$'ve gotten $your$self killed. "
				           "Would you like me to try a resurrection?", MSG_MAIN, NULL);
			else gen_sysmsg(152, "<Sigh>  $You$'ve died *again*. "
				                "Would you like me to try another resurrection?",
				                MSG_MAIN, NULL);
			if (yesno("? ")) {  /* Now do resurrection */
				curr_lives--;
				quitflag = deadflag = 0;
				gen_sysmsg(154,
				           "$You$ emerge coughing from a cloud of dark green smoke.",
				           MSG_MAIN, NULL);
				writeln("");
				loc = resurrect_room - first_room;
				newlife_flag = 1;
				set_statline();
				do_look = do_autoverb = 1;
				newroom();
				return;
			} else writeln("As you wish...");
		} else if (max_lives > 1)
			gen_sysmsg(153, "$You$'ve used up all of $your$ lives.", MSG_MAIN, NULL);
	}
	writeln("");
	print_score();
	writeln("");
	done_flag = quitflag; /* If player has QUIT, don't ask again */
	while (!done_flag && !quitflag) {
		writestr("Would you like to ");
		if (restart_state != NULL) writestr("restart, ");
		writestr("restore");
		if (undo_state != NULL && can_undo)
			writestr(", undo,");
		else if (restart_state != NULL) writestr(",");
		writestr(" or quit? ");
		s = agt_readline(5);
		if (strncasecmp(s, "RESTART", 7) == 0)
			if (restart_state != NULL) {
				restart_game();
				done_flag = 1;
			} else writeln("Sorry, I'm unable to do that because of limited memory.");
		else if (strncasecmp(s, "RESTORE", 7) == 0)
			if (g_vm->loadGame().getCode() == Common::kNoError) {
				done_flag = 1;
			} else writeln("(RESTORE failed)");
		else if (strncasecmp(s, "UNDO", 4) == 0)
			if (can_undo && undo_state != NULL) {
				putstate(undo_state);
				done_flag = 1;
			} else writeln("Insufficiant memory to support UNDO");
		else if (toupper(s[0]) == 'Q') {
			quitflag = 1;
			done_flag = 1;
		}
	}
	set_statline();
}


static void parse_loop(void)
/* This exists to deal with THEN lists; parse() handles the indiviudual
 commands */
{
	for (ip = 0; ip >= 0 && ip < MAXINPUT && input[ip] != -1;) {
		if (!parse() || quitflag || winflag || deadflag || endflag) break;
		if (doing_restore) break;
		if (ip >= 0 && ip < MAXINPUT && input[ip] != -1)
			writeln(""); /* Insert blank lines between commands when dealing
              with THEN lists */
	}
}


static long rm_start_size;
static char memstr[100];

static void mainloop(void) {
	char *s;

	doing_restore = 0;
	while (!quitflag) {
		if (DEBUG_MEM) {
			sprintf(memstr,
			        "A:%ld F:%ld  Delta:%ld   Size:%ld+%ld=%ld (%ld left)\n",
			        ralloc_cnt, rfree_cnt, ralloc_cnt - rfree_cnt,
			        rm_start_size, rm_size - rm_start_size, rm_size,
			        rm_freesize);
			writeln(memstr);
		}
		rm_size = 0; /* Reset it to zero */
		rm_freesize = get_rm_freesize();
		if (!menu_mode) {
			prompt_out(1);
			s = agt_readline(0);
			if (g_vm->shouldQuit())
				return;

			agt_newline();
			if (!doing_restore) tokenise(s);   /* Tokenizes into input */
			rfree(s);
			if (!doing_restore) parse_loop();
		} else
			menu_cmd();
		if (doing_restore) {
			if (doing_restore == 1)
				g_vm->loadGame();
			else if (doing_restore == 2)
				restart_game();
			else if (doing_restore == 3 || doing_restore == 4)
				return; /* Quit or New game requested */
			doing_restore = 0;
		}
		if (winflag || deadflag || endflag || quitflag)
			game_end();
	}
}


/*-------------------------------------------------------------------*/
/* Start up and shut down: Routines to initialise the game state and */
/*   clean up after the game ends.                                   */
/*-------------------------------------------------------------------*/

static int init(void) {
	int i, can_save;
	uchar *tmp1, *tmp2;

	init_vals();
	init_creat_fix();
	if (!agx_file) dict[0][0] = 0; /*  Turn "ANY" into "" */
	l_stat[0] = r_stat[0] = 0; /* Clear the status line */
	/*  lactor=lobj=lnoun=NULL;*/
	tscore = old_score = objscore = 0;
	turncnt = 0;
	curr_time = startup_time;
	loc = start_room - first_room;
	cmd_saveable = 0;
	first_visit_flag = newlife_flag = room_firstdesc = 1;
	curr_lives = max_lives;

	/* Note: flag[0] is the debugging flag and is set elsewhere */
	if (FLAG_NUM < 0) FLAG_NUM = 0;
	dbgflagptr = flag = (rbool *)rrealloc(flag, sizeof(rbool) * (FLAG_NUM + 1));
	for (i = 1; i <= FLAG_NUM; i++)
		flag[i] = 0;
	dbgcntptr = agt_counter = (short *)rmalloc(sizeof(short) * (CNT_NUM + 1));
	for (i = 0; i <= CNT_NUM; i++) {
		agt_counter[i] = -1;
	}
	dbgvarptr = agt_var = (long *)rmalloc(sizeof(*agt_var) * (VAR_NUM + 1));
	for (i = 0; i <= VAR_NUM; i++)
		agt_var[i] = 0;

	for (i = 0; i <= maxnoun - first_noun; i++) {
		if (noun[i].position == NULL || noun[i].position[0] == '\0')
			noun[i].pos_prep = 0;
		else noun[i].pos_prep = -1;
		noun[i].pos_name = 0;
		noun[i].initpos = noun[i].position;
	}

	nomatch_aware = 0; /* By default, not aware. */
	smart_look = 1; /*  By default, LOOK <x> --> EXAMINE */
	for (i = 0; i < last_cmd; i++) {
		if (command[i].nouncmd == -1 || command[i].objcmd == -1
		        || command[i].noun_adj == -1 || command[i].obj_adj == -1
		        || command[i].prep == -1)
			nomatch_aware = 1;
		if (command[i].verbcmd == ext_code[wlook] &&
		        (command[i].nouncmd > 0 || command[i].noun_adj > 0
		         || command[i].objcmd > 0 || command[i].obj_adj > 0
		         || command[i].prep > 0))
			smart_look = 0;
	}

	pictable = (integer *)rmalloc(sizeof(int) * maxpict);
	for (i = 0; i < maxpict; i++) pictable[i] = i;
	init_state_sys(); /* Initialize the system for saving and restoring
               game states */
	tmp1 = (uchar *)rmalloc(MEM_MARGIN); /* Preserve some work space */

	tmp2 = getstate(NULL); /* Make sure we have space to save */
	if (tmp2 == NULL) can_save = 0;
	else can_save = 1;

	if (tmp2 != NULL)
		undo_state = getstate(NULL);
	else undo_state = NULL;

	if (undo_state != NULL)
		restart_state = getstate(NULL);
	else restart_state = NULL;

	rfree(tmp1);
	rfree(tmp2);
	rm_start_size = get_rm_size();
	rm_freesize = get_rm_freesize();
	return can_save;
}


static void ext_dict(void)
/* Enter the vocabulary extensions into the dictionary */
{
	wtype i;
	for (i = wthe; i <= wabout; i = (wtype)((int)i + 1))
		ext_code[i] = add_dict(ext_voc[i]);
}


static void fix_dummy(void) {
	int i;

	/* At this point, all occurances in the game file of the dictionary
	   words have been converted to dictionary indices, and so as long as
	   we don't change the dictionary index values, we can change the contents
	   without interfering with the metacommand scanner (since it compares
	   dictionary indices, not actual strings) */

	if (!PURE_DUMMY) {
		for (i = 0; i < DUMB_VERB; i++)
			dict[ syntbl[auxsyn[i + BASE_VERB]] ][5] = ' ';
		/* Convert underscores into spaces:
		   i.e. 'dummy_verb5' -> 'dummy verb5' */
		dict[ syntbl[auxsyn[21]] ][6] = ' '; /* change_locations */
		dict[ syntbl[auxsyn[55]] ][5] = ' '; /* magic_word */
	}

	if (!PURE_SUBNAME)     /* Replace the 'e' by a space */
		for (i = 0; i < MAX_SUB; i++)
			sprintf(dict[sub_name[i]], "subroutin %d", i + 1);
	/* This must be no longer than 25 characters with the terminating null */

	/* Now set PURE_DOT based on whether any dictionary word
	   contains a period. */
	if (aver >= AGT18 && aver <= AGT18MAX) PURE_DOT = 0;
	else {
		PURE_DOT = FORCE_PURE_DOT;
		for (i = 0; i < dp && !PURE_DOT; i++)
			if (strchr(dict[i], '.') != NULL && /* i.e. dict[i] contains period */
			        i != ext_code[wp])   /* The period itself _is_ a dictionary word:
                avoid this false match */
				PURE_DOT = 1;
	}
}


/* This is a hack to get rid of the "What Now?" prompt. */
static void fix_prompt(void) {
	descr_line *d;

	if (err_ptr == NULL) return;
	d = read_descr(err_ptr[0].start, err_ptr[0].size);
	if (d == NULL) return;
	if (strncasecmp(d[0], "What Now?", 9) == 0)
		err_ptr[0].size = err_ptr[0].start = 0;
	free_descr(d);
}



void close_game(void); /* Called by setup_game, and so needs
                 to be defined here. */

static fc_type setup_game(fc_type fc)
/* game_name is the common filename of the AGT game files */
{
	int can_save;
	char choice;
	rbool have_ins;

	bold_mode = 0;
	rm_acct = 1;
	rm_trap = 1;
	rm_size = ralloc_cnt = rfree_cnt = 0;
	mars_fix = 0;
	no_auxsyn = 0;
	debug_disambig = 0;
	debug_any = 1;
	dbg_nomsg = 1; /* Supress output of MSG arguments to metacommands */
	textbold = 0;
	debug_mode = 0;
	aver = 0;
	verboseflag = 1;
	notify_flag = 0;
	logflag = 0;
	menu_mode = 0;
	fast_replay = 0;
	stable_random = BATCH_MODE || make_test;
	if (make_test) BATCH_MODE = 0;
	hold_fc = fc;
	set_default_filenames(fc);

	init_stack();
	read_config(agt_globalfile(0), 1);  /* Global configuration file */

	/* Now that we *have* PATH information, go looking for the games */
	/* At the very least, it creates an rmalloc'd copy of game_name */
	read_config(openfile(fc, fCFG, NULL, 0), 0);
	text_file = 0;
	/* First pass through game specific config file */
	build_trans_ascii();
#ifdef PROFILE
	resetwatch();
#endif
	writeln("Loading game...");
	if (!read_agx(fc, 0) && !readagt(fc, 0))
		fatal("Unable to load game.");
#ifdef PROFILE
	writeln(stopwatch());
	agt_waitkey();
#endif
	if (have_opt)
		menu_mode = opt_data[5];   /* See agtread.c for discussion of OPT file
                format */
	text_file = 1;
	read_config(openfile(fc, fCFG, NULL, 0), 1); /*Game specific config file*/
	text_file = 0;
	if (min_ver > AGIL_VERID) {
		if (FORCE_VERSION)
			agtwarn("This game requires a later version of AGiliTy.", 0);
		else
			fatal("This game requires a later version of AGiliTy.");
	}
	sort_cmd();
	ext_dict();
	build_verbmenu();
	fix_dummy(); /* Prevent player from calling dummy verbs or subroutines by
            typing 'Subroutine n' on the command line */
	can_save = init();
	if (!agx_file) open_descr(fc);
	fix_prompt(); /* Kill off 'What Now?' prompt. */
	if (BATCH_MODE || make_test)
		set_test_mode(fc);
	start_interface(fc);
	fontcmd(2, 0);  /* Set initial font */
	if (intro_first && intro_ptr.size > 0) {
		agt_clrscr();
		print_descr(intro_ptr, 1);
		wait_return();
	}
	if (aver >= AGTME10)
		pictcmd(3, 0); /* Show title image, if there is one */
	print_title(fc);
	have_ins = open_ins_file(fc, 0);

	if (have_ins) {
		do {
			writestr("Choose <I>nstructions, or <other> to start the game");

			choice = tolower(agt_getchar()); /* Wait for keypress */
			if (g_vm->shouldQuit())
				return nullptr;

			agt_clrscr();
			if (have_ins && choice == 'i')
				print_instructions(fc);
		} while (choice == 'i');
	}
	close_ins_file();

	if (!intro_first && intro_ptr.size > 0) {
		print_descr(intro_ptr, 1);
		wait_return();
		agt_clrscr();
	}
	if (maxroom < first_room) {
		close_game();
		error("Invalid first room");
	}
	set_statline();
	if (can_save == 0) {
		writeln("[Insufficiant memory to support SAVE, RESTORE, or UNDO]");
	} else if (undo_state == NULL)
		writeln("[Insufficiant memory to support UNDO]");
	do_look = do_autoverb = 1;
	newroom();
	rm_acct = 1; /* Turn on memory allocation accounting */
	return fc;
}


/* We need to import save_lnoun from exec.c so that we can free it. */
extern parse_rec *save_lnoun;

void close_game(void) {
	if (agx_file)
		agx_close_descr();
	else
		close_descr();
	fontcmd(1, -1); /* Restore original font */
	musiccmd(7, -1); /* Clean up */
	close_interface();

	/* Now free everything in sight; this _shouldn't_ be necessary,
	   but why take chances? */
	free_all_agtread();
	rfree(restart_state);
	rfree(undo_state);
	rfree(pictable);
	rfree(save_lnoun);
	rfree(verbptr);
	rfree(verbend);
	rfree(agt_counter);
	rfree(agt_var);
	free_creat_fix();
	flag = (rbool *)rrealloc(flag, sizeof(rbool)); /* Preserve the debugging flag */

	if (DEBUG_MEM)
		debug("\n\nAlloc:%ld  Freed:%ld  Difference:%ld\n", ralloc_cnt,
		      rfree_cnt, ralloc_cnt - rfree_cnt);
}


void run_game(fc_type fc) {
	doing_restore = 0;
	rm_acct = 1;
	rm_trap = 1;
	rm_size = ralloc_cnt = rfree_cnt = 0;
	read_config(agt_globalfile(0), 1);   /* Global configuration file:
                         get PATH information*/
	fix_file_context(fc, fDA1);
	do {
		if (doing_restore == 3) {
			release_file_context(&fc);
			fc = setup_game(new_game());
		} else setup_game(fc);
		doing_restore = 0;

		if (!g_vm->shouldQuit())
			mainloop();
		close_game();

		if (g_vm->shouldQuit())
			break;
	} while (doing_restore == 3);
	release_file_context(&fc);
}

} // End of namespace AGT
} // End of namespace Glk
