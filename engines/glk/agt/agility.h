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

#ifndef GLK_AGT_AGILITY
#define GLK_AGT_AGILITY

#include "glk/agt/config.h"
#include "common/str.h"

namespace Glk {
namespace AGT {

/* This indicates the AGiliTy version code for the current version. */
/*  0=0.8.7 and the numbers are assigned sequentially from there.
    1=0.8.8
    2=1.0
    (pre-0.8.7 versions of AGiliTy don't have version codes) */
/* Don't touch this unless you know exactly what you're doing. */
#define AGIL_VERID 3

/*
    This is the master header file for all of the AGT stuff.
      It includes the global variables, the data types, etc.
      (everything that is read in from the game file).
    Variables not read in from the game file but used internally
      by the AGiliTy interpreter are declared in interp.h.
    Magx specific things are in comp.h.
*/

/* ------------------------------------------------------------------- */
/* This is a preprocessor trick to ensure that space is only allocated
   for global variables once.  'global' should only be defined in one
   source file; for all of the other modules, it will be converted to
   extern by the following lines */
/* ------------------------------------------------------------------- */
#ifndef global    /* Don't touch this */
#define global extern
#define global_defined_agtread
#endif


/* ------------------------------------------------------------------- */
/* DEFINITIONS OF SPECIAL DATA TYPES                       */
/* These should by platform-independent.                               */
/* ------------------------------------------------------------------- */

#ifdef force16  /* This is for debugging purposes */
#define int short int
#endif

/* General data types */

typedef byte uchar;
typedef int8 schar;
typedef int16 integer;
typedef uchar rbool;

enum { INT8_MAX_VAL = 127, BYTE_MAX_VAL = 255 };

#define WORD_LENG 25

/* Game-specific data type */
typedef char tline[81]; /* Both of these must include terminating null */
typedef char words[WORD_LENG]; /* ...23 in classic, 16 in master's */
typedef short word;     /* A pointer into the dictionary */
typedef short slist; /* Index into synlist marking beginning of this
            synonym list.  [SYNCNT]
            list of word pointers -- eg synonyms */
typedef char *descr_line; /* This is the type used to return descriptions.
                 They are actually returned as an array of
                 pointers to char, one for each line.
                 It is NULL terminated. */
typedef char *filename;  /* Datatype used for picture, sound, etc. names*/


typedef enum {fNONE,
              fDA1, fDA2, fDA3, fDA4, fDA5, fDA6, fDSS,
              fHNT, fOPT, fTTL,
              fSAV, fSCR, fLOG,
              fAGX, fINS, fVOC, fCFG,
              fAGT, fDAT, fMSG, fCMD, fSTD, fAGT_STD
             } filetype;


/* ------------------------------------------------------------------- */
/* GLOBAL FLAGS                                                        */
/* Many of the following should be made into command line options      */
/* ------------------------------------------------------------------- */

/* #define AGT_16BIT */ /* Force interpreter to use 16-bit quantities */
/* #define DUMP_VLIST  */ /* Dump out the verb list info */
/* #define USE_CMD_INDEX */  /* Read in metacommand index data for objects */


#define SS_GRAIN 1024  /* Granularity of size request for static string
              array (to avoid calling rrealloc too often) */
#define SYN_GRAIN 32 /* Granularity of requests for synonym array; this
               is in units of sizeof(word) */
#define MAX_BADTOK 10  /* How many bad tokens to report before giving
              up */



/* The following are defaults that can be overridden from the command line */
/* The real variable has the name without the "def_"  */

#define def_DIAG 0   /* Print out diagnostic data? */
#define def_interp_arg 1  /* Decipher arguments to opcodes? */
#define def_debug_da1 0  /* used to analyse .DA1 files */
#define def_RAW_CMD_OUT 0  /*Print CMDs out raw, undecoded; sometimes useful
            when trying to understand new gamefile version */
#define def_ERR_LEVEL 2    /* Level of error reporting. */
/* 0== almost none */
/* 1== Report possibly serious conditions */
/* 2== Report any fall from perfection */







/* ------------------------------------------------------------------- */
/* DEFINES OF GLOBAL PARAMETERS                                        */
/* ------------------------------------------------------------------- */

#define FORMAT_CODE 0xFF  /* Character used for special formatting codes:
                 --in 1.8x games, it designates bold
                 --in AGX+ games, it will prefix format codes
                 --otherwise it designates black (replacing
                 0, which has obvious problems) */
#define LAST_TEXTCODE 8  /* Last ascii code to be used for text attributes */



#define MAX_PIX 31
#define MAX_FLAG_NOUN 31



#define OLD_VERB 59    /* Number of built in verbs in original interpreters-- 
              this number includes ANY, which is verb 0. */
#define DIR_ADDR_CODE (OLD_VERB+17)  /* Verb ID used for direct address */
#define AUX_VERB 18    /* Additional verbs supported by the interpreter */
#define BASE_VERB (OLD_VERB+AUX_VERB)  /* Total number of built-in verbs */
#define DUMB_VERB (DVERB+MAX_SUB)  /* Number of dummy verbs and subroutines */
#define TOTAL_VERB (BASE_VERB+DUMB_VERB)  /* Total count of verbs */


/* The following numbers refer to the ideal code we are translating into,
   not the parameters for the actual data file we're reading. */
#define MAX_COND 143     /* Last condition token id */
#define START_ACT 1000  /* First action code */
#define PREWIN_ACT 1161 /* Last action code before WinGame */
#define WIN_ACT 2000    /* Value of WinGame opcode */
#define END_ACT (WIN_ACT+2)  /* Lowest command-terminating action code */
#define MAX_ACT (WIN_ACT+4)  /* Highest action code */

/* Note: the following values are used by Magx internally:
   3000-- Used to mark 'no entry' in the opcode hash table.
   8000-- Used for NEXT
   8001-- Used for PREV
 */

#define MAX_TOKEN_ID 250   /* Upper limit on legal (raw) token values
                  read from AGT files. Doesn't need to be exact. */

/* Number of built in properties and attributes. */
#define NUM_PROP 14
#define NUM_ATTR 26


/*
   None of these are used any more, but I leave them here for reference
   #define MAX_ROOM 300
   #define MAX_NOUN 300
   #define MAX_CREAT 200
   #define MAX_CMD 1500
   #define MAX_QUEST 100
   #define MAX_MSG 1000
   #define MAX_PICT 250
   #define ABS_MAX_REC_CMD 34
   #define MAX_OBJ (MAX_ROOM+MAX_NOUN+MAX_CREAT)
   #define SYNCNT 15
*/



/* --------------------------------------------------------------------- */
/* DATA STRUCTURE DEFINITIONS                                            */
/*                                                                       */
/* All of the internal data structures used to store the contents of the */
/*   game file                                                           */
/* --------------------------------------------------------------------- */

/* First, pointers to game descriptions */
/*    start and size may be measured in units of characters or in units */
/*    of tline  */
typedef struct {
	long start;
	long size;
} descr_ptr;


/* Entries in the opcode tables:  the name of the opcode, the number of
   arguments, and the types of those arguments */
typedef struct {
	const char *opcode;
	integer argnum;
	integer arg1, arg2;
} opdef;  /* Opcode table entry */



/* This is the data type for opcode correction entries */
/*   These are used to translate the opcodes from the various versions */
/*   to a uniform internal format. */
/*   The actual translation tables built with this are in agtdata.c */
typedef struct {
	integer told, tnew; /* Old and new values. */
} cmd_fix_rec;

typedef const cmd_fix_rec *fix_array;


/* ROOMS */
typedef struct {
	const char *name;  /* [31] */
	int32 flag_noun_bits, PIX_bits; /* Master's Ed. only */
	slist replacing_word;  /* List of words to be replaced */
	word replace_word;   /* Word that does replacing */
	word autoverb;     /* Verb automatically executed on entry to room */
	integer path[13];
	integer key;
	integer contents;   /* Used internally by the interpreter; not read in
             from the game file */
	integer points;
	integer light;  /* Object that lights room; 0=none needed, 1=any */
	integer pict, initdesc;
	integer oclass;
	rbool seen, locked_door;
	rbool end, win, killplayer;
	rbool unused; /* True if this room entry is unused */
} room_rec;


/* NOUNS */
typedef struct {
	const char *shortdesc; /* tline */
	const char *position;
	/* 23 characters in position for classic ed, 31 for ME */
	const char *initpos; /*Starting value of position;used for RESTART/RESTORE*/
	int scratch;  /* Scratch space for use by various parser routines. */
	slist syns;   /* List of synonyms */
	word name, adj;
	word related_name;  /* Word that should appear on menu when this noun
              is in scope */
	word pos_prep, pos_name;   /* Used internally by the interpreter */
	/* pos_prep==-1 means use noun.position */
	integer nearby_noun; /* Noun this noun is behind */
	integer num_shots;
	integer points;
	integer weight, size;
	integer key;
	integer initdesc, pict;
	integer location;    /* 1=carried, 1000=worn */
	integer contents, next;   /* Used internally by the interpreter; not read in
             from the game file */
	integer oclass;
	integer flagnum; /* Number of associated flag + 1. 0=no flag. */
	rbool scope;   /* Used internally by the interpreter */
	rbool isglobal; /* True if global object */
	uchar plural;
	rbool something_pos_near_noun;  /* Anybody behind us? */
	rbool has_syns;
	rbool pushable, pullable, turnable, playable, readable;
	rbool on, closable, open, lockable, locked, edible, wearable;
	rbool drinkable, poisonous, movable, light;
	rbool shootable;
	rbool win;
	rbool unused; /* True if this noun entry is unused */
	rbool seen;  /* True if this noun has been seen by the player */
	rbool proper;  /* True if noun's name is to be treated as proper noun. */
} noun_rec;


/* CREATURES */
typedef struct {
	const char *shortdesc; /* tline */
	int scratch;  /* Scratch space for use by various parser routines. */
	slist syns;
	word name;
	word adj;
	integer location;
	integer contents, next;   /* Used internally by the interpreter; not read in
                   from the game file */
	integer weapon;  /* Killed by this */
	integer points;
	integer counter;  /* How many times has player been nasty to it? */
	integer threshold, timethresh, timecounter;
	integer pict, initdesc;
	integer oclass;
	integer flagnum; /* Number of associated flag + 1. 0=no flag. */
	rbool scope; /* Used internally by the interpreter */
	rbool isglobal; /* True if global object */
	rbool has_syns;
	rbool groupmemb;
	rbool hostile;
	uchar gender;
	rbool unused; /* True if this creature entry is unused */
	rbool seen;  /* True if this creature has been seen by the player */
	rbool proper; /* True if this creature's name is to be treated as a proper
           noun (i.e. not prepended with "the") */
} creat_rec;

/* These records are used to hold reference information for
   user-defined flags and properties */
/* These give the base index offset in objflag/objprop of each flag/prop
 for rooms, nouns, and creatures.  The flag record also contains the
 bit offset in the given byte. */
/* This approach allows for a single property to be located in different
   places for each of the three types of objects. */
/* A value of -1 for any of the three fields indicates that this
   property doesn't exist for the given type of object. */

typedef struct {
	long r, n, c;
	long str_cnt;  /* values from 0..(str_cnt-1) have assoc strings */
	long str_list; /* Index into propstr[] array */
} propdef_rec;

typedef struct {
	long r, n, c;
	char rbit, nbit, cbit;
	const char *ystr, *nstr; /* Yes and no strings */
} attrdef_rec;

typedef struct {
	long str_cnt;
	long str_list;
} vardef_rec;


typedef struct {
	const char *ystr, *nstr;
} flagdef_rec;

/*  Metacommand headers and a pointer to the actual sequence of tokens
    to execute if the metacommand is run. */
typedef struct {
	integer actor;  /* Contains the actor object number;
               1==self(no explicit actor) 2=anybody
               It is multiplied by negative one for redirected
               commands */
	/* 0 in any of the following word variables denotes ANY;
	   -1 denotes <*none*> and will only occur in Magx-generated files.
	   (verbcmd cannot be equal to -1).  Support for -1 is still experimental */
	word verbcmd, nouncmd, objcmd, prep;  /* prep only in ME games */
	word noun_adj, obj_adj;  /* Adjectives for noun and obj; not
                  supported in original AGT games */
	integer noun_obj, obj_obj; /* Actual object numbers,
                or 0 if none specified */
	integer *data; /* MaxSizeCommand */
	integer cmdsize; /* Number of commands */
	/*  integer ptr; */ /* In ME games-- see below for replacement */
} cmd_rec;


/* FRS=file record size; these are the sizes to be allocated to the buffer
   used to read in the various records from the files; they should be at
   least as big as the worst case scenario. */
#define FRS_ROOM 220
#define FRS_NOUN 310
#define FRS_CREAT 240
#define FRS_CMD 150


/* This is the record used to hold menu information for verbs */
typedef struct {  /*verb menu entry */
	word verb; /* Verb word */
	word prep; /* Associated preposition */
	short objnum; /* Number of objects */
} verbentry_rec;


/* This is the datatype that will be used (tenatively) for hint info */
/* This isn't implemented yet. */
typedef struct {
	integer dtype;  /* The hint element type */
	integer child;  /* The hint element value */
	const char *name; /* The hint element name */
} hint_rec;


/* This is the data type used to hold information about
   built-in attributes and properties */
struct prop_struct {
	const char *name; /* Property name. */
	int room, noun, creature; /* Offsets for the various object types. */
};


/* The following data structure is used to store info on fields of a struct
   that may need to be read from/written to a file. */
/* They are used by both the AGX and the Savefile routines */
/* They should be organized in ftype==0 terminated arrays,
   in the order they occur in the file (the file is assumed to have
   no padding, so we don't need file offsets: they can be computed) */
/* The following is used for both global variables and fields in
   structures. For global variables, ptr is set to point at the variable
   and offset is 0. For fields, offset is set to the offset of the field
   in the structure and ptr is set internally */
struct file_info {
	int ftype; /* Type in file */
	int dtype; /* Data type of field in memory; often ignored */
	void *ptr;   /* Pointer to variable */
	size_t offset; /* Offset of field in structure */
} ;



/* This contains all of the information needed to find files. */
#ifndef REPLACE_FC
struct file_context_rec {
	char *gamename;   /* Name as entered by user */
	char *path;       /* The path */
	char *shortname;  /* The filename w/o directory information */
	char *ext;        /* The preexisting extension/prefix */
	filetype ft;      /* The filetype corresponding to ext */
	int special;      /* Used to mark special files, such as UNIX pipes */
};

typedef file_context_rec *fc_type;

#endif


/* ------------------------------------------------------------------- */
/* GLOBAL VARIABLES                                                    */
/* ------------------------------------------------------------------- */

/* ------------------------------------------------------------------- */
/* Flags used internally by the interpreter and reading routines */

global uchar DIAG, interp_arg, debug_da1, RAW_CMD_OUT;
global int ERR_LEVEL;

global rbool agx_file;  /* Are we reading an AGX file? */
global rbool have_opt;      /* Do we have an OPT file? */
global rbool skip_descr; /* Causes read_filerec() to skip over description
               pointers without actually reading them in.
               Used to support RESTORE for multi-part games
               such as Klaustrophobia */
global rbool no_auxsyn; /* Prevents building of auxsyn and preplist
          synonym lists; used by agt2agx. */


global rbool BATCH_MODE, make_test;
/* These indicates we are in testing mode:
     -- The random number generator should be set to a standard state.
     -- Automatically send output to <gamename>.scr
     -- Automatically read/write input from <gamename>.log
      (depending on whether we are in BATCH_MODE or make_test mode,
      respectivly).
*/


/* ------------------------------------------------------------------- */
/* Flags reflecting game version and configuration */

global rbool have_meta; /* Do we have any metacommands? */
global rbool debug_mode, freeze_mode, milltime_mode, bold_mode;
global uchar score_mode, statusmode;
global rbool intro_first;
global rbool box_title;
global rbool mars_fix;
global rbool fix_ascii_flag;  /* Translate IBM characters?
                   Defaults to fix_ascii #define'd above */
global rbool dbg_nomsg;  /* Causes printing of <msg> arguments by
               debug disassembler to be supressed */

global rbool irun_mode;  /* If true, all messages will be in 1st person */
global rbool verboseflag;

global int font_status; /* 0=unknown; 1=force fixed font,
               2=allow proportional font. */


/*   The following are AGT 'purity' flags; they turn off features of */
/* my interpreter that are not fully consistent with the original AGT. */
/* They are defined in auxfile.c (see also interp.h and agil.c for */
/* interpreter-specific flags)  */
/* Anything added here should also be correctly initialized in agt2agx */

extern rbool PURE_ANSWER, PURE_TIME, PURE_ROOMTITLE;
extern rbool PURE_AND, PURE_METAVERB, PURE_ERROR;
extern rbool PURE_SYN, PURE_NOUN, PURE_ADJ, PURE_SIZE;
extern rbool PURE_DUMMY, PURE_SUBNAME, PURE_PROSUB;
extern rbool PURE_HOSTILE, PURE_GETHOSTILE;
extern rbool PURE_DISAMBIG, PURE_ALL, PURE_OBJ_DESC;
extern rbool PURE_GRAMMAR, PURE_SYSMSG, PURE_AFTER;
extern rbool PURE_PROPER;

extern rbool TWO_CYCLE, FORCE_VERSION;
extern rbool MASTERS_OR;

/* ------------------------------------------------------------------- */
/*  Variables containing limits and counts of objects  */

global integer FLAG_NUM, CNT_NUM, VAR_NUM;  /* (255, 50, 50) */
global integer MAX_USTR;  /* Maximum number of user strings (25)  */
global integer MAX_SUB;   /* Number of subroutines (15) */
global integer DVERB;     /* Number of real dummy_verbs (50) */
global integer NUM_ERR;  /* For ME is 185 */

/* Number of objflags and objprops for each type of object */
/*  The flag counts are for groups of 8 flags. */
global integer num_rflags, num_nflags, num_cflags;
global integer num_rprops, num_nprops, num_cprops;
global integer oprop_cnt, oflag_cnt; /* Size of flag and property tables */

global integer maxroom, maxnoun, maxcreat;
global long MaxQuestion;
global integer first_room, first_noun, first_creat, last_obj;
global long last_message, last_cmd;
global long numglobal; /* Number of global nouns */
global long maxpict, maxpix, maxfont, maxsong;
global long num_prep; /* Number of user-defined prepositions */
global int num_auxcomb;
global int num_comb;

global integer exitmsg_base; /* Number added to messages used as
             "illegal direction" messages */


/* ------------------------------------------------------------------- */
/*   Miscellaneous other variables read in from the game file */

global integer start_room, treas_room, resurrect_room, max_lives;
global long max_score;
global integer startup_time, delta_time;

/* ver contains the size of the game, aver indicates its version */
/*  See the #define's below for details */
global int ver, aver; /* ver: 0=unknown, 1=small, 2=big, 4=masters1.5 */
global long game_sig; /* 2-byte quantity used to identify game files */
/* (It's declared long to avoid overflow problems when
computing it) */
global int vm_size;  /* Size of verb menu */

global int min_ver;  /* Lowest version of AGiliTy this will run on. */

/* ------------------------------------------------------------------- */
/*  Miscellaneous Game Data Structures */

/* All of the following are allocated dynamically */
global room_rec *room; /* [MAX_ROOM]; */
global creat_rec *creature; /* [MAX_CREAT]; */
global noun_rec *noun; /* [MAX_NOUN]; */
global cmd_rec *command;

global unsigned char *objflag;
global long *objprop;

/* Next we have tables linking abstract flag/property values to
   the indices with the objflag/objprop arrays. */
global attrdef_rec *attrtable;
global propdef_rec *proptable;
global vardef_rec *vartable; /* For itemized variables */
global flagdef_rec *flagtable;

/* Output strings associated with various property values. */
/*    See propdef_rec */
global const char **propstr;
global long propstr_size;

global tline *userstr; /*[MAX_USTR];*/
global word *sub_name;  /* [MAX_SUB] Dictionary id's of all subroutines */

/* Verb information */
global verbentry_rec *verbinfo; /* Verb information */
global short *verbptr, *verbend; /* [TOTAL_VERB] */
global slist *synlist;  /* [MAX_VERBS+1];*/
global slist *comblist; /* Global combination list */
global word *old_agt_verb; /* List of non-canonical verb synonyms in the
                  original AGT; these are not allowed to be
                  expanded as dummy verbs. */


global slist *userprep; /* Array of user-defined prepostions */

global word flag_noun[MAX_FLAG_NOUN], *globalnoun;
global word pix_name[MAX_PIX];
global filename *pictlist, *pixlist, *fontlist, *songlist;

global uchar opt_data[14];  /* Contents of OPT file. For the format of this
                   block, see the comments to read_opt() in
                   agtread.c */

/* These are built by reinit_dict */

global slist *auxsyn; /* [TOTAL_VERB]  Built-in synonym list */
global slist *preplist;  /* [TOTAL_VERB] */
global uchar *verbflag;  /* [TOTAL_VERB]  Verb flags; see below */
global slist *auxcomb; /* Built-in combination lists (for multi-word
              verbs) */

#ifdef PATH_SEP
global char **gamepath;
#endif

/* ------------------------------------------------------------------- */
/*  Description Pointers   */


global descr_ptr intro_ptr;
global descr_ptr title_ptr, ins_ptr; /* Only defined if agx_file is true */
global descr_ptr *err_ptr; /* [NUM_ERR];*/

global descr_ptr *msg_ptr; /* [MAX_MSG];*/
global descr_ptr *help_ptr, *room_ptr, *special_ptr; /*[ROOM] */
global descr_ptr *noun_ptr, *text_ptr, *turn_ptr, /* [NOUN] */
       *push_ptr, *pull_ptr, *play_ptr;
global descr_ptr *talk_ptr, *ask_ptr, *creat_ptr; /* [CREAT] */

global descr_ptr *quest_ptr, *ans_ptr; /* [MAX_QUEST] */
global tline *question, *answer; /* [MAX_QUEST] */



/* ------------------------------------------------------------------------ */
/* Dynamically allocated data blocks (which are pointed to from elsewhere)  */

global char **dict;  /* dict[n] points to the nth dictionary word */
global long dp;  /* Dictionary pointer: number of words in dict */

#define DICT_INIT 12*1024 /* Starting size of dictstr */
#define DICT_GRAN 1024  /* Granularity of dictstr size requests 
               must be at least 4. */
global char *dictstr;  /* Pointer to memory block containing dict words */
global long dictstrptr, dictstrsize;
/* dictstrptr points to the first unused byte in dictstr.
   dictstrsize points to the end of the space currently allocated for
      dictstr.
*/

global char *static_str; /*Static string space */
global long ss_end; /* Pointer to end of used space in above */
global long ss_size; /* Current size of static string space */

global word *syntbl;  /* Synonym list space */
global slist synptr; /* Points to end of used space */
global long syntbl_size; /* Long so we can catch overflows */

global long descr_maxmem;
global char *mem_descr; /* Copy of descriptor in memory */



/* ------------------------------------------------------------------------ */
/*  Data structures used internally by agtread.c   */

/*The following are all set to NULL after agtread finishes. */
global long *cmd_ptr; /* ME only;Points to cmd start locs in gamefile.*/
global long *room_name, *noun_sdesc, *noun_pos, *creat_sdesc;
global long *t_pictlist, *t_pixlist, *t_songlist, *t_fontlist;

/* These are only used by agtout (to allow the AGT reading routines to
   pass back the count of nouns inside the given object) */
global integer *room_inside, *noun_inside, *creat_inside;

/* This is used to translate ASCII codes */
global uchar fixchar[256];

global rbool text_file;  /* Set if we are currently opening a binary file. */
#ifdef OPEN_AS_TEXT
global rbool open_as_binary;  /* Open text files as binary, anyhow. */
#endif


/* ------------------------------------------------------------------ */
/* SYMBOLIC CONSTANTS: VERSION CODES                                  */
/*   These are the values stored in the variable 'aver'.              */
/* ------------------------------------------------------------------ */

#define AGT10 1     /* SPA */
#define AGT118 2    /* TAMORET, PORK II  */
#define AGT12  3   /* SOS,... */
#define AGTCOS 4   /* COSMOS and SOGGY: enhanced versions of 1.3x */
#define AGT135 5   /* By far the most common version; includes practically
              every version of Classic AGT from 1.19 to 1.7 */
#define AGT182 6
#define AGT183 7
#define AGT15 8   /* HOTEL */
#define AGT15F 9  /* MDTHIEF */
#define AGT16 10  /* PORK  */
#define AGTME10 11  /* CLIFF2, ELF20  */
#define AGTME10A 12  /* HURRY */
#define AGTME15 13    /* WOK */
#define AGTME155 14 /* TJA */
#define AGTME16 15 /* also includes v1.56 and 1.7 */
#define AGX00 16   /* Tenative */

#define AGTMAST AGTME16
#define AGTCLASS AGT16  /* Dividing line between master's ed and classic */
#define AGT18 AGT182  /* Defines lowest 1.8x version */
#define AGT18MAX AGT183 /* Defines the highest 1.8x version */
#define AGTSTD AGT135  /* "Default" version of AGT */



/* ------------------------------------------------------------------ */
/* SYMBOLIC CONSTANTS: ARGUMENT TYPES                                 */
/*   These are used to encode the argument types of metacommands for  */
/*   opcode tables.                                                   */
/* ------------------------------------------------------------------ */

#define AGT_UNK 0     /* Unknown argument type */

/* The following can all mix and match in various ways and so are
   put together as powers of two. */
#define AGT_NONE 1  /* 0 is allowed */
#define AGT_SELF 2  /* 1 is allowed */
#define AGT_WORN 4  /* 1000 is allowed */
#define AGT_ROOM 8  /* A room # is allowed */
#define AGT_ITEM 16 /* An item # is allowed */
#define AGT_CREAT 32  /* A creature # is allowed */

/* AGT_VAR is special, since it is always combined with another type--
   the type that the variable is expected to be */
#define AGT_VAR 64

/* The rest of the values are always distinct; they never mix and so
   they can be given simple consecutive indices */
#define AGT_NUM 128
#define AGT_FLAG 129
#define AGT_QUEST 130   /* Question */
#define AGT_MSG 131     /* Message */
#define AGT_STR 132    /* String */
#define AGT_CNT 133  /* Counter */
#define AGT_DIR 134   /* Direction */
#define AGT_SUB 135   /* Subroutine */
#define AGT_PIC 136    /* Picture */
#define AGT_PIX 137   /* Room picture */
#define AGT_FONT 138
#define AGT_SONG 139
#define AGT_ROOMFLAG 140
#define AGT_TIME 141
#define AGT_ERR 142
#define AGT_OBJFLAG 143  /* User defined object flags */
#define AGT_OBJPROP 144  /* User defined object properties */
#define AGT_ATTR    145  /* Built-in attribute */
#define AGT_PROP    146  /* Built-in property */


/* These next three may not occur as operand types */
#define AGT_EXIT 147  /* Valid values for an exit: room, msg+msgbase, 0 */
#define AGT_GENFLAG 148 /* PIX or Room Flag; used internally by compiler */
#define AGT_GENPROP 149  /* ObjProp/Property */

/* certain restrictions.  Used internally
                by the compiler in the parsing
                of "[obj].[prop].[prop].[flag]"
                constructions. */

#define AGT_LVAL   150  /* Used by the compiler in certain psuedo-ops */


#define AGT_DEFINE 256



/* ------------------------------------------------------------------ */
/* Verb flags for verbflag[]; these should be powers of 2 */

#define VERB_TAKEOBJ 1
#define VERB_META 2
#define VERB_MULTI 4  /* Can the verb take mulitple objects? */
#define VERB_GLOBAL 8  /* Does the verb have global scope? */

/* ------------------------------------------------------------------ */
/* SYMBOLIC CONSTANTS: FILE DATA TYPES                                */
/*   Data type specifiers for file i/o routines                       */
/*   The FT_* constants specify file data types                       */
/*   The DT_* constants specify internal data types                   */
/* ------------------------------------------------------------------ */

#define FT_COUNT 17 /* Number of file data types */

#define FT_END 0  /* End of list of fields/variables in file */
#define FT_INT16 1   /* DT_SHORT */
#define FT_UINT16 2  /* DT_LONG */
#define FT_INT32 3   /* DT_LONG */
#define FT_UINT32 4
#define FT_BYTE 5   /* aka uchar; DT_UCHAR */
#define FT_VERSION 6 /* Game version */
#define FT_BOOL 7   /* DT_BOOL. Adjacent rbooleans are packed */
#define FT_DESCPTR 8  /* DT_DESCPTR */
#define FT_STR 9  /* Integer pointer into static string array */
#define FT_SLIST 10 /* Synonym list index */
#define FT_WORD FT_INT16 /* Index into dictionary */
#define FT_PATHARRAY 11 /* 13 integers in an array of directions */
#define FT_CMDPTR 12   /* Pointer into command block */
#define FT_DICTPTR 13  /* Pointer into dictionary text */
#define FT_TLINE 14    /* TextLine */
#define FT_CHAR 15     /* Characters. */
#define FT_CFG 16      /* Configuration byte; 0=false, 1=true, 
              2=none (don't change) */

#define DT_DEFAULT 0  /* Default internal type for <ftype> */
#define DT_LONG 1
#define DT_DESCPTR 2 /* Description pointer, which are treated specially */
#define DT_CMDPTR 3  /* Command block pointer, also treated specially */

/* This is the end marker for the file definitions used by the file I/O
   routines */
#define endrec {FT_END,0,NULL,0}


/* ------------------------------------------------------------------- */
/* Date type macros                                                    */
/* ------------------------------------------------------------------- */
#define troom(x) ((x)>=first_room && (x)<=maxroom)
#define tnoun(x) ((x)>=first_noun && (x)<=maxnoun)
#define tcreat(x) ((x)>=first_creat && (x)<=maxcreat)


/* ------------------------------------------------------------------- */
/* FUNCTION PROTOTYPES AND INITIALIZED TABLES                          */
/* ------------------------------------------------------------------- */

/* This is intended for removing whitespace in AGT data files. */
#define rspace(c) ((c)==' ' || (c)=='\t')

/* ------------------------------------------------------------------- */
/* In GAMEDATA.C                                                       */
/* This module contains the major initialized data structures and      */
/*  routines to manipulate game data structures, in particular the     */
/*  game's dictionary                              */
/* ------------------------------------------------------------------- */

void init_dict(void); /* 1=set of verblist, 0=don't */
void build_verblist(void);  /* Creates verblist */
void reinit_dict(void);
void free_dict(void);
word search_dict(const char *);
word add_dict(const char *);
const char *gdict(word w);  /* Almost equivalent to dict[w], but with
                   some error checking and handling for
                   the w==-1 case. */

int verb_code(word);
int verb_builtin(word);
int verb_authorsyn(word);
void addsyn(word);
slist add_multi_word(word);

/* Commands to manipulate object flags */
long objextsize(char op);
long op_objprop(int op, int obj, int propid, long val);
rbool op_objflag(int op, integer obj, int flagid);
long lookup_objflag(int id, int t, char *ofs);
long lookup_objprop(int id, int t);
rbool have_objattr(rbool prop, integer obj, int id);
int num_oattrs(int t, rbool isflag);
rbool op_simpflag(uchar *flag, char ofs, int op);
/* op: 0=clear, 1=set, 2=nop, 3=toggle    two bits: <ab> */
const char *get_objattr_str(int dtype, int id, long val);


const opdef *get_opdef(integer op);
char *objname(int);
void sort_cmd(void);

void agtwarn(const char *, int elev);
void agtnwarn(const char *, int, int elev);

void init_flags(void);

#ifdef ZIP
#define fatal agil_fatal
#endif
void fatal(const char *);

long new_str(char *buff, int max_leng, rbool pasc);

descr_line *read_descr(long start, long size);
void free_descr(descr_line *p);

extern const char trans_ibm[];
extern const char nonestr[];

/* Tables of opcodes */
extern const opdef cond_def[], act_def[], end_def[], illegal_def;

/* Table of built in properties and attributes */
extern const prop_struct proplist[NUM_PROP];
extern const prop_struct attrlist[NUM_ATTR];

global words *verblist; /* List of prexisting words, intialized by init_dict */
extern const fix_array FIX_LIST[];
extern const char *exitname[13];
extern const char *verstr[], *averstr[];
extern const char *version_str, *portstr;


/* ------------------------------------------------------------------- */
/* In AGTREAD.C                                                        */
/* Routines to read in AGT data files                                  */
/* ------------------------------------------------------------------- */

void open_descr(fc_type fc);
void close_descr(void);
descr_line *agt_read_descr(long start, long len);
rbool readagt(fc_type fc, rbool diag);
void free_all_agtread(void); /* Cleans up everything allocated in agtread
                  should only be called at the very end of
                  the program */
void free_ttl(descr_line *title);


/* ------------------------------------------------------------------- */
/* In AGXFILE.C                                                    */
/*   Routines to read and write AGX files                  */
/* ------------------------------------------------------------------- */

int read_agx(fc_type fc, rbool diag);
descr_line *agx_read_descr(long start, long size);
void agx_close_descr(void);

/* The following are in the order they should be called */
void agx_create(fc_type fc);
void write_descr(descr_ptr *dp, descr_line *txt);
void agx_write(void);
void agx_wclose(void);
void agx_wabort(void);

/* ------------------------------------------------------------------- */
/*  In AUXFILE.C                                                       */
/*    Routines to read VOC, OPT, CFG, TTL, INS, etc. files             */
/* ------------------------------------------------------------------- */
void read_opt(fc_type fc);
void read_config(genfile cfgfile, rbool lastpass);
rbool parse_config_line(char *s, rbool lastpass);

descr_line *read_ttl(fc_type fc); /* This returns the title. The return string
                      must be freed with free_ttl() and not
                      with free_descr */
void free_ttl(descr_line *title);

void read_voc(fc_type fc);
void init_verbrec(void);
void add_verbrec(const char *verbline, rbool addnew); /* addnew should be 0 */
void finish_verbrec(void);

descr_line *read_ins(fc_type fc);
void free_ins(descr_line *instr);
rbool open_ins_file(fc_type fc, rbool report_error);
char *read_ins_line(void); /* Reuses buffer, so return value should be copied
              if it needs to be used past the next call to
              read_ins_line or close_ins_file */
void close_ins_file(void);

void build_fixchar(void);

/* ------------------------------------------------------------------- */
/* In or used by DISASSEMBLE.C                             */
/*   Routines to disassemble metacommands (used by the interpreter for */
/*   tracing and by agtout to produce the metacommand output).         */
/* ------------------------------------------------------------------- */

global rbool *dbgflagptr;
global long *dbgvarptr;
global short  *dbgcntptr;

void dbgprintf(const char *fmt, ...);
void debugout(const char *s);
int argout(int dtype, int dval, int optype);

/* ------------------------------------------------------------------- */
/* In INTERFACE.C, AGIL.C and/or AGILSTUB.C                            */
/*   agilstub.c provides minimal versions of these routines for use by */
/*   programs other than the interpreter                       */
/* ------------------------------------------------------------------- */

void writeln(const char *s);
void writestr(const char *s);
void agil_option(int optnum, char *optstr[], rbool setflag, rbool lastpass);
void close_interface(void);

void print_tos(void); /* Called by the disassembler; in either TOKEN.C
             or AGTOUT.C */


/* ------------------------------------------------------------------- */
/* In UTIL.C                                   */
/*   Low-level utilites, including memory allocation, string manip.,   */
/*   and buffered file I/O.                        */
/* ------------------------------------------------------------------- */

global uchar trans_ascii[256];  /* Table to translate ascii values read
                   in from file */

void build_trans_ascii(void); /* Set up the above table. */

void rprintf(const char *fmt, ...); /* General output routine, mainly used
                       for diagnostics. There can be a newline
                       at the end, but there shouldn't be
                       one in the middle of the string */



/* Memory management variables and routines */

extern rbool rm_trap; /* Trap memory allocation failures? */
global rbool rm_acct; /* Turn on rmem accounting, to locate memory leaks */
global long rfree_cnt, ralloc_cnt; /* # of allocs/frees since acct turned on */
global long rm_size, rm_freesize; /* These hold worst case values */

long get_rm_size(void);   /* These get the current values */
long get_rm_freesize(void);
void *rmalloc(long size);
void r_free(void *p);
#define rfree(p) (r_free(p),p=NULL) /* Traps errors & catch memory leaks */
void *rrealloc(void *p, long size);
char *rstrdup(const char *s);


/* String utilities */

char *concdup(const char *s1, const char *s2); /* Concacate and duplicate */
char *rstrncpy(char *dest, const char *src, int max);
/* Copy at most max-1 characters */
rbool match_str(const char **pstr, const char *match);

#ifdef NEED_STR_CMP
#undef strcasecmp
//define strcasecmp Common::scumm_strcasecmp
extern int strcasecmp(const char *s1, const char *s2);
#endif
#ifdef NEED_STRN_CMP
#undef strncasecmp
//define strncasecmp Common::scumm_strnicmp
extern int strncasecmp(const char *s1, const char *s2, size_t len);
#endif

#undef isspace
#define isspace Common::isSpace
#undef isprint
#define isprint Common::isPrint
#undef isalpha
#define isalpha Common::isAlpha
#undef isalnum
#define isalnum Common::isAlnum
#undef islower
#define islower Common::isLower
#undef isupper
#define isupper Common::isUpper
#undef ispunct
#define ispunct Common::isPunct

/* The fixsign.. routines put together unsigned bytes to form signed ints */

#ifndef FAST_FIXSIGN
short fixsign16(uchar n1, uchar n2);
long fixsign32(uchar n1, uchar n2, uchar n3, uchar n4);
#else
#define fixsign16(u1,u2) ( (u1) | ((u2)<<8) )
#define fixsign32(u1,u2,u3,u4)  ( ((long)u1) | (((long)u2)<<8) | \
                                  (((long)u3)<<16) | (((long)u4)<<24) )
#endif


/* Miscellaneous */
long rangefix(long n);

/* File routines */
genfile openfile(fc_type fc, filetype ext, const char *err, rbool ferr);
genfile openbin(fc_type fc, filetype ext, const char *err, rbool ferr);

#undef fopen
genfile fopen(const char *name, const char *how);
#undef fseek
int fseek(genfile stream, long int offset, int whence);
#undef fread
size_t fread(void *ptr, size_t size, size_t nmemb, genfile stream);
#undef fwrite
size_t fwrite(const void *ptr, size_t size, size_t nmemb, genfile stream);
#undef fclose
#define fclose(f) delete f
#undef ftell
size_t ftell(genfile f);

char *readln(genfile f, char *buff, int n);
/* Read a line from a 'text' file */

/* (None of the following routines are at all reentrant) */
long buffopen(fc_type fc, filetype ext,
              long minbuffsize, const char *rectype, long recnum);
/* Open file for our buffered routines and make it our current file;
returns the record size. Prints out error message on failure
rectype="noun","room",etc.; recnum=expected # of objects in file */
uchar *buffread(long index);
/* seek to index*recsize, read buff_rsize bytes, return pointer to a
   buffer with them. */
void buffclose(void); /* Close the current file */

void bw_open(fc_type fc, filetype ext); /* Open buffered file for writing */
void bw_close(void); /* Close buffered file */
void bw_abort(void); /* Close and delete buffered file */

/* "Universal" file routines */
extern const size_t ft_leng[FT_COUNT];  /* File lengths of the data types */
long compute_recsize(file_info *recinfo);
void *read_recblock(void *base, int ftype, long numrec, long offset,
                    long blocksize);
/* Only works for FT_BYTE, FT_SLIST, FT_WORD, FT_DICTTEXT, FT_INT16 */
void *read_recarray(void *base, long eltsize, long numelts,
                    file_info *field_info, const char *rectype,
                    long file_offset, long file_blocksize);
void read_globalrec(file_info *global_info, const char *rectype,
                    long file_offset, long file_blocksize);

long write_recarray(void *base, long eltsize, long numelts,
                    file_info *field_info, long file_offset);
long write_globalrec(file_info *global_info, long file_offset);
long write_recblock(void *base, int ftype, long numrec, long offset);

char *textgets(genfile f, char *buf, size_t n);
char textgetc(genfile f);
void textungetc(genfile f, char c);
bool texteof(genfile f);
void textputs(genfile f, const char *s);

void set_internal_buffer(void *p);
/* Causes all of the above routines to write to the block of memory pointed
   at by p instead of to a file */

#ifdef PROFILE_SUPPORT
/* These are functions to do quick-and-dirty timing of routines;
   I added them to check the performance of the AGX code.
   They aren't likely to work on anything other than a *nix box */
void resetwatch(void);
void startwatch(void);
char *stopwatch(void);
char *timestring(void);
#define runwatch(cmd) do{resetwatch();cmd;printf("%s\n",stopwatch());}while(0)
#else
#define runwatch(cmd) cmd
#endif



/* ------------------------------------------------------------------- */
/* In FILENAME.C                               */
/*   The low-level file routines                                       */
/* ------------------------------------------------------------------- */

fc_type init_file_context(const char *game_name, filetype ft);
/* This sets up the filename system, based around a game with name
   "gamename".  Must be called before any of the following are called
   with relative filetypes (all filetypes that depend on the game's
   name and location-- everything but the global config file.)  */
/* ft indicates the file type class. At the moment, it can be
   fAGX, fSAV, fSCR, fLOG
      */
void fix_file_context(fc_type fc, filetype ft);
fc_type convert_file_context(fc_type fc, filetype ft, const char *name);
void release_file_context(fc_type *pfc);


char *formal_name(fc_type fc, filetype ft); /* Used for messages */
genfile badfile(filetype ft);   /* Bad file of type ft */
rbool fileexist(fc_type fc, filetype ft);

genfile readopen(fc_type fc, filetype ft, const char **errstr);
genfile writeopen(fc_type fc, filetype ft,
                  file_id_type *pfileid, const char **errstr);
rbool filevalid(genfile f, filetype ft);
void readclose(genfile f);
void writeclose(genfile f, file_id_type fileid);

void binremove(genfile f, file_id_type fileid);
void binseek(genfile f, long offset);
rbool binread(genfile f, void *buff, long recsize, long recnum, const char **errstr);
long varread(genfile f, void *buff, long recsize, long recnum, const char **errstr);
rbool binwrite(genfile f, void *buff, long recsize, long recnum, rbool ferr);
long binsize(genfile f); /* Size of an open binary file */

rbool textrewind(genfile f);

char *assemble_filename(const char *path, const char *root,
                        const char *ext);


#ifdef global_defined_agtread
#undef global
#undef global_defined_agtread
#endif

} // End of namespace AGT
} // End of namespace Glk

#endif
