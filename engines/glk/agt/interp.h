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

#include "common/file.h"

#ifndef GLK_AGT_INTERP
#define GLK_AGT_INTERP

namespace Glk {
namespace AGT {

/* This file contains variables and data structures used
    by the interpreter but not read in from the gamefile.
   For the rest of the data structures, see agility.h and
    agtdata.c */

#ifndef global
#define uagt_defined_global
#define global extern
#endif

/* -------------------------------------------------------------------- */
/* The following are debugging and diagnostic flags.                */
/* They are mainly intended for debugging the interpreter, but      */
/*  they could concievable be used for debugging games under the    */
/* interpreter                              */
/* -------------------------------------------------------------------- */

global rbool stable_random;
/* --Force random numbers to be repeatable. */


global rbool DEBUG_MEM; /* prints out information on memory allocation */

global rbool debug_parse; /* Prints out long parse diagnostic information
             after the sentence has been parse but before
             disambiguation */
global rbool DEBUG_EXEC_VERB; /* This prints out a line indicating each command
             that exec_verb() is asked to run */
global rbool DEBUG_DISAMBIG; /* Print out dismabiguation debugging info */
global rbool DEBUG_SMSG;    /* Print out STANDARD message info */

#define DEBUG_AGT_CMD flag[0]  /* This sends metacommand execution information
            to either the screen or debugfile, depending on 
            whether DEBUG_OUT is true or false. */

global rbool debug_disambig, debug_any;
/* These determine if metacommands are traced during disambiguation
 or during the scanning of ANY commands */

global rbool DEBUG_OUT; /* True if debugging output is redirected somewhere
            other than the screen */
global Common::DumpFile *debugfile; /* Where debugging output should be sent */

#define def_DEBUG_MEM 1    /* parser.c */


/* -------------------------------------------------------------------- */
/* The following are AGT 'purity' flags; they turn off features of  */
/*   my interpreter that are not fully consistent with the original AGT.*/
/* More are defined in agility.h, and you should look there for general */
/*  notes                               */
/* -------------------------------------------------------------------- */

/* The following are defined (and described) in agil.c */
extern rbool PURE_INPUT, PURE_TONE;

#define PURE_WEAR 1  /* If this is cleared, then things taken off
            still stay in the player's inventory.
            The support for this isn't quite complete yet
            (there are one or two checks that need to be done
            but aren't) and so right now this should be 1. */

global rbool PURE_DOT; /* Treats period as a letter-character and not
              as punctuation. This should be set automatically
              during initialization based on whether any words
              in the dictionary have dots. */

#define FORCE_PURE_DOT 0  /* This forces the period to be treated as a letter
                 even if there are no words in the dictionary
                 containing periods. The only reason to set
                 this was if you were concerned that knowledge
                 about the presence or absence of periods in the 
                 dictionary would give puzzles away. */




#define MEM_MARGIN (16*1024) /* 16K should be enough (SOGGY, the largest AGT
                game, uses around 12K) */
#define PICT_SUPPORT 0  /* Graphics not supported */
#define TAB_SIZE 3     /* Number of spaces in a tab */




/* -------------------------------------------------------------------- */
/*  Variables and Flags related to Metaverbs                */
/* -------------------------------------------------------------------- */

global rbool notify_flag, listexit_flag, menu_mode;

global rbool cmd_saveable; /* set indicates that this command can be repeated
             with AGAIN. */
global rbool can_undo;  /* Can we UNDO the last turn? */

global uchar *restart_state, *undo_state; /* Store old game states for
                     RESTART and UNDO */
global char doing_restore; /* Have we been asked to RESTORE? */

global rbool do_look;  /* True if we should print a room description */
global rbool do_autoverb; /* True if we should run the autoexec verb
              for the current room */

/* The following are used for scripting and logging */
global rbool script_on;
global genfile scriptfile;
global signed char logflag; /* 1=logging, 2=replaying, 0=neither, 3=both */
global int logdelay; /* -1=wait for keypress, >=0 is numerical delay */
global genfile log_in, log_out;

global rbool fast_replay; /* If true, don't print MORE prompts. */

global rbool sound_on; /* Manipulated by music_cmd; can be used by interface
             to determine if a tone should be made */

global integer *pictable; /* Used to decode picture numbers */
global fc_type hold_fc; /* Needed to print instructions on demand */

global unsigned short compass_rose; /* Used to pass compass info to
                    the os layer */


/* -------------------------------------------------------------------- */
/*  Game State                              */
/* -------------------------------------------------------------------- */

global rbool quitflag, deadflag, winflag, endflag;
global rbool first_visit_flag, newlife_flag, room_firstdesc;

global rbool start_of_turn; /* True if running the command on the first
                  noun in the list */
global rbool end_of_turn;  /* True if running command on last noun in
                  the list. */

global rbool actor_in_scope; /* Used to determine if the actor was in
                   scope when the command was originally
                   given */

global integer loc;   /* Player's location */
global integer player_contents, player_worn; /* Worn and carried objects*/

global long totwt, totsize; /* Size and wt player is carrying around */

global integer curr_lives; /* Number of lives left. */

global long tscore, old_score; /* Total score */
global long objscore;  /* Portion of tscore that comes from the POINTS
            field of nouns and creatures. */

global integer turncnt;  /* Number of turns that have passed */
global integer curr_time;   /* The time in the game; in the format
               1243==12:43 */

global rbool *flag; /* AGT Flags */
global short *agt_counter; /* AGT counters */
#ifdef AGT_16BIT
global short *agt_var; /*AGT variables */
#else
global long *agt_var;
#endif

global long agt_number; /* The number entered by the player */
global rbool agt_answer; /* Did the player get the answer to the last question
            right? */

global tline l_stat, r_stat;  /* Left and right parts of status line */
/* If r_stat is the empty string, l_stat should be
centered to create a Trinity-like status line */

global rbool nomatch_aware;  /* Does the game use the nomatch extension
                   to the metacommand format?
                   (which allow <none> and ANY to be
                   distingused) */

global rbool smart_look;  /* If true, then LOOK <object> will be converted
                to EXAMINE. This is set automatically in agil.c,
                based on whether the game file uses
                LOOK <object> in any of the metacommands; if it
                does, then smart_look is set to 0. */

/* -------------------------------------------------------------------- */
/* Menu data structures                         */
/* -------------------------------------------------------------------- */

#define MENU_WIDTH 50
typedef char menuentry[MENU_WIDTH];

global int  vm_width; /* Width of widest element */
global menuentry *verbmenu;



/* -------------------------------------------------------------------- */
/* Parser Data Structures                       */
/*  This also includes "parser-related" variables like dobj and iobj    */
/* -------------------------------------------------------------------- */

/* This extracts the object number from a parse rec */
#define p_obj(objrec) ((objrec) ? (objrec)->obj : 0)

/* The following data structures are used for disambiguation of nouns */
struct parse_rec {
	long num;     /* Numeric value of object; 0 if object doesn't have one */
	int obj;     /* Object number; negative values point into the dictionary */
	int info;    /* Disambiguation info */
	/* -1=last record; ignore obj field. */
	word noun, adj;  /* Used for printing out error messages */
	short score;   /* Disambiguation score */
}; /* Stores objects that have been found during parse */


/* In an ideal world, the following would all be local variables. */
/*   Unfortunately, they're used in too many different places for this
     to be practical */

global int vb;
global integer actor, dobj, iobj;
global parse_rec *actor_rec, *dobj_rec, *iobj_rec;
global word prep;
global parse_rec *curr_creat_rec;
/* Creature currently behaving in a hostile way:
used to fill in $c_name$ messages */

global int disambig_score; /* Used to rank objects during disambiguation */

#define DISAMBIG_SUCC 1000   /* Score given to an object that triggers a
                built-in verb or an action token */


#define MAXINPUT 200   /* Max number of words input */

global word input[MAXINPUT];  /* 200 words of input should be enough */
global words in_text[MAXINPUT];
/* The corrospoinding strings, for error reporting purposes */

global short ip, ep; /* input pointer and error pointer */
global short ap, vp, np, pp, op; /* Points to first word in actor, verb, noun,
            and object resp. */



/* The following needs to be kept consistant with ext_voc[] in
   agil.c */
typedef enum {wthe, wmy, wa, wan, wthen, wp, wsc, wand, wc, wits, wall, wundo, wlook, wg,
              wpick, wgo, wexits, wtalk, wtake, wdoor, wagain, wbut, wexcept,
              wscene, weverything, wlistexit, wlistexits, wclose,
              wdverb, wdnoun, wdadjective, wdprep, wdobject, wdname,
              wstep, w_any, weither, wboth, weveryone, weverybody,
              whe, wshe, wit, wthey, whim, wher, wthem, wis, ware, woops,
              wwas, wwere,
              win, wout, winto, wat, wto, wacross, winside, wwith, wnear, wfor,
              wof, wbehind, wbeside, won, woff, wunder, wfrom, wthrough,
              wtoward, wtowards, wbetween, waround, wupon, wthru,
              wby, wover, wup, wdown,
              wabout
             } wtype;
global word ext_code[wabout + 1]; /* Codes for the above */
global short last_he, last_she, last_it, last_they;
/* Used for pronoun support */



/* -------------------------------------------------------------------- */
/* Noun List Data structures and constants              */
/* -------------------------------------------------------------------- */


/* The following are used in noun lists */
#define AND_MARK (-ext_code[wand])
#define ALL_MARK (-ext_code[wall])

#define D_END  50    /* Marks end of disambiguation list */
#define D_AND  51    /* Used to seperate multiple objects during disambig */
#define D_NOUN 0   /* Noun match */
#define D_SYN 1    /* Adjective/synonym only match */
#define D_ADJ 2    /* Adj only match */
#define D_FLAG 3   /* Flag noun */
#define D_GLOBAL 4 /* Global noun */
#define D_PIX 5     /* PIX name */
#define D_PRO 6      /* Pronoun */
#define D_ALL  7     /* ALL, or a header to an ALL EXCEPT _ AND _ ... list */
#define D_INTERN 8     /* Internal nouns: DOOR, SCENE */
#define D_NUM  9    /* A number, value is in obj */
#define D_EITHER 10  /* EITHER or ANY, used only to resolve disambiguation */

#define D_MARK 0x80   /* Used as a temporary marker, usually to indicate
             this noun is being considered for elimination */



/* -------------------------------------------------------------------- */
/* These are used for text boxes (quotes and the title)                 */
/* -------------------------------------------------------------------- */
#define TB_TTL 1   /* We're printing the title */
#define TB_BOLD 2   /* Print it bold */
#define TB_BORDER 4 /* Give it a border */
#define TB_CENTER 8  /* Center the text inside */
#define TB_NOCENT 16 /* Don't center the whole box */


/* -------------------------------------------------------------------- */
/* In AGIL.C                                */
/* -------------------------------------------------------------------- */
extern void print_instructions(fc_type fc);
extern void run_game(fc_type fc);

/* -------------------------------------------------------------------- */
/* In PARSER.C                              */
/* -------------------------------------------------------------------- */
extern rbool parse(void);  /* Returns true unless there is ambiguity */
extern void menu_cmd(void);


/* -------------------------------------------------------------------- */
/* In EXEC.C                                */
/* -------------------------------------------------------------------- */

/* Legal values for gen_sysmsg context; they indicate who is calling it */
#define MSG_PARSE 0   /* The parser */
#define MSG_MAIN 1    /* The main execution loop */
#define MSG_RUN  2   /* The routines that execute the player's commands */
#define MSG_DESC 3   /* Printing out description. */

extern void gen_sysmsg(int msgid, const char *s, int context, const char *pword);
/* Prints either STANDARD message <msgid> or default msg <s>;
   <context> determines what $$ substitutions are meaningful
   <parseword> gives the $pword$ substitution for MSG_PARSE messages */

extern void exec(parse_rec *actor, int vnum, parse_rec *lnoun,
                 word prep, parse_rec *iobj);
extern void set_statline(void);
extern void newroom(void);
extern void print_descr(descr_ptr dp, rbool nl);
extern void quote(int msgnum);
extern void print_score(void);
extern long read_number(void);


/* -------------------------------------------------------------------- */
/* In TOKEN.C                                   */
/* -------------------------------------------------------------------- */
extern void init_stack(void);  /* Set up expression stack */
extern void clear_stack(void);  /* Set stack back to empty state */

/* -------------------------------------------------------------------- */
/* In OBJECT.C                              */
/* -------------------------------------------------------------------- */
extern rbool player_has(int item);
extern rbool visible(int item);
extern rbool genvisible(parse_rec *dobj);
extern int *get_nouns(void);  /* Returns list of in scope nouns */
extern void add_object(int loc, int item); /* Adds item to loc's contents list */
extern void tmpobj(parse_rec *objrec);
extern void compute_scope(void); /* Sets scope flags for all of the objects */
extern void compute_seen(void);  /* Determine HAS_SEEN flag for nouns and creatures */

extern void init_creat_fix(void);
extern void free_creat_fix(void);

/* -------------------------------------------------------------------  */
/* The following are intended as building blocks to construct macros    */
/*  to extract information about general objects, regardless of whether */
/*  they are nouns, creatures, or virtual nouns with no associated  */
/*  data structure.                             */
/* -------------------------------------------------------------------  */
/* nounattr(item,attr) -- returns 0 if not noun.
   creatattr(item,attr) -- returns 0 if not creature
   objattr(item,attr) -- Returns attribute for noun or creature, 0 otherwise
   anyattr(item,attr) -- Returns attribute for noun, creature, or room,
                            0 otherwise.
*/

#define creatattr2(item,attr,op3) (tcreat(item)? \
                                   creature[(item)-first_creat].attr:\
                                   (op3))
#define creatattr(item,attr) creatattr2(item,attr,0)
#define nounattr2(item,attr,alt) (tnoun(item)? \
                                  noun[(item)-first_noun].attr:(alt))
#define nounattr(item,attr) nounattr2(item,attr,0)
#define objattr(item,attr) nounattr2(item,attr,creatattr(item,attr))
#define objattr2(item,attr,op3) nounattr2(item,attr,creatattr2(item,attr,op3))
#define roomattr2(item,attr,op3) (troom(item)?\
                                  room[(item)-first_room].attr:(op3))
#define anyattr(item,attr) roomattr2(item,attr,objattr(item,attr))

#define it_scratch(item) objattr(item,scratch)
#define it_loc(item) objattr2(item,location,\
                              (tdoor(item)) ? loc+first_room : 0)


/* -------------------------------------------------------------------- */
/* In RUNVERB.C                             */
/* -------------------------------------------------------------------- */
extern int check_obj(parse_rec *act, int verbid,
                     parse_rec *donum, word prep, parse_rec *ionum);


/* -------------------------------------------------------------------- */
/* In AGTDBG.C                              */
/* -------------------------------------------------------------------- */
extern void debug_cmd_out(int ip, integer op, int arg1, int arg2, int optype);
extern void debug_head(int);
extern void debug_newline(integer op, rbool first_nl);

/* -------------------------------------------------------------------- */
/* In SAVEGAME.C                            */
/* -------------------------------------------------------------------- */
extern Common::Error savegame(Common::WriteStream *savefile);
extern Common::Error loadgame(Common::SeekableReadStream *loadfile);
extern void init_state_sys(void);  /* Must be called before either of the following */
extern uchar *getstate(uchar *gs);
/* Returns malloc'd block containing game state. */
extern void putstate(uchar *gs); /* Restores games state. */
extern void init_vals(void);  /* Compute dependent variables
              such as totwt, totsize, etc. */
extern void restart_game(void);


/* -------------------------------------------------------------------- */
/* In OS_<whatever>.C                           */
/* -------------------------------------------------------------------- */
global volatile int screen_width, status_width;
global int screen_height;
global volatile int curr_x;

extern void init_interface();
extern void start_interface(fc_type fc);
extern void close_interface(void);
extern char *agt_input(int in_type); /* read line, return malloc'd string */
extern char agt_getkey(rbool echo_char);
extern void agt_clrscr(void);
extern void agt_textcolor(int c);
extern void agt_delay(int n); /* n in seconds */
extern int agt_rand(int a, int b); /* Return random number from a to b, inclusive */
extern void agt_newline(void);
extern void agt_puts(const char *s); /* Output string */
extern void agt_statline(const char *s); /* Prints s out on status line */
extern void agt_tone(int hz, int ms);
extern void agt_makebox(int width, int height, unsigned long flags);
extern void agt_qnewline(void);
extern void agt_endbox(void);
extern genfile agt_globalfile(int fid); /* When fid=0, return global config file */
extern rbool agt_option(int optnum, char *optstr[], rbool setflag);

/* These have stubs defined in interface.c that would ened to be
    commented out if you actually wanted to support these */
extern void fontcmd(int cmd, int font); /* fontlist[font] */
extern void pictcmd(int cmd, int pict); /* pictlist[pict] or pixlist[pict] */
extern int musiccmd(int cmd, int song); /* songlist[song] */


/* -------------------------------------------------------------------- */
/* In INTERFACE.C                           */
/* -------------------------------------------------------------------- */
/* init_interface() (in os_?????.c) is responsible for initializing these */
global rbool par_fill_on, center_on;
global rbool textbold;  /* Is the text currently bold? */

extern void wait_return(void);
extern void agt_waitkey(void);

extern void agt_center(rbool b); /* 1=turn on text centering, 0=turn off */
extern void agt_par(rbool b); /* 1=turn on "paragraph" mode, 0=turn off */
extern char *agt_readline(int in_type); /* Front end for agt_input */
extern char agt_getchar(void); /* Front end for some uses of agt_getkey */
extern void prompt_out(int);  /* 1=standard prompt, 2=question prompt */
extern genfile get_user_file(int ft); /* 0=script, 1=save, 2=restore */
extern void set_default_filenames(fc_type fc);
extern void script(uchar); /* 0=turn off, 1=turn on */
extern void logon(void);  /* Turn on logging */
extern int close_pfile(genfile f, int ft); /* ft is the same as for get_user_file */
extern void replay(int delay); /* REPLAY */
extern rbool yesno(const char *);
extern void textbox(char *(txt[]), int len, unsigned long flags);
extern void padout(int padleng); /* Outputs padleng spaces */
extern int agt_menu(const char *header, int size, int width, menuentry *menu);
extern fc_type new_game(void);

extern void set_test_mode(fc_type fc);
/* This sets up scripting and replaying for testing mode */

/* These are intended to be called by the os layer */
extern void print_statline(void);

extern void agt_save(void);
extern void agt_restore(void);
extern void agt_restart(void);
extern void agt_quit(void);
extern void agt_newgame(fc_type fc);

/* -------------------------------------------------------------------- */
/* Object manipulation macros                       */
/* -------------------------------------------------------------------- */
#define objloop(i) for(i=first_noun; i<=maxnoun || i<=maxcreat; \
                       (i<=maxnoun || i>=first_creat) ? (i++) : (i=first_creat) )
#define nounloop(i) for(i=0;i<=maxnoun-first_noun;i++)
#define creatloop(i) for(i=0;i<=maxcreat-first_creat;i++)

#define tdoor(x) ((x)==-ext_code[wdoor])

#ifdef uagt_defined_global
#undef global
#undef uagt_define_global
#endif

} // End of namespace AGT
} // End of namespace Glk

#endif
