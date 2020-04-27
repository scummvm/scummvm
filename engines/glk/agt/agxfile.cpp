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

/* NOTES ON CHANGING THE AGX FILE FORMAT

     First of all, don't.

     One of the benefits of adventure creation systems like this is
   that the same game files can be played on a variety of different
   platforms without any extra effort on the part of the game
   author.  If you change the file format, this is no longer true:  games
   created under the new format won't run on the old interpreters.

     Even if you distribute a new interpreter with your game, there are two
   problems:

     i) People on other platforms won't be able to play your game unless
    and until your modified interpreter is ported to their machine. Since
    I-F players as a group tend to use a wider range of different computers
    and operating systems than the population at large, this is bad.

     ii) Even for machines that you port your modified interpreter to,
    people will now need to maintain two interpreters: the original one
     (for most of the games) and your modified one (for your new game).
    This is not only a nuisance but it wastes disk space.


    If you *do* decide to change the file format anyhow, please adhere to
  the following guidelines, to minimize confusion.

GUIDLINES FOR NEW FILE FORMAT VERSIONS

  File format version are labled by a series of four bytes near the
beginning of the file. (They are actually the fifth, sixth, seventh,
and eight bytes-- the first four bytes are the file format signature
that indicate the file is an AGX file and not, say, a PCX file)

  In order, they are the version owner id and number, and the
extension owner id and number.  In "vanilla" AGX, both owner id's are
'R', the version number is 2, and the extension number is 1 (the
extension associated with AGiliTy 1.0).  (There are AGX files with
version numbers 0 and 1 created by earlier releases of agt2agx.  In
fact, for downward compatibility, agt2agx will sometimes create a file
of version 1 and extnum 7 if later features aren't being used.)

  I will discuss the difference between "versions" and "extensions"
further below, but briefly: extensions are minor changes whereas versions
represent vast reoganizations of the file format.  The routines below
will still try to read in a file with an unrecognized extension, but
they will give up on an unrecognized version.

  If you create a new extension, then you should first change the
extension owner id to something else; the owner id is intended to
indicate who is responsible for defining this extension; 'R' indicates
Robert Masenten (the author of this file) and so shouldn't be used by anyone
else. The id isn't required to be a printable character.

  You can then define the extension number however you want. The
extension number is intended to differentiate between different
extensions defined by the same source (e.g. two extensions both
defined by me would have the same owner id but different extension
numbers). The extensions that I define are numbered sequentially
starting at 0, but you don't have to follow this convention if you
don't want to.

  Finally, send me an e-mail note telling me what you've done so I can
keep track of the different extensions and prevent conflicts between
owner-ids.

  Creating a new version works the same way: change the version owner-id to
something no one is using and set the version number however you want.
If you're defining a new version, you can do whatever you want with
the two extension bytes.


EXTENSIONS AND VERSIONS

  For purposes of the file format, an 'extension' is a change to the
format that follows certain restrictions given below; a 'version' is one that
violates one or more of these restrictions.

  If at all possible you should try to fit your changes to the format
within the limitations of an 'extension': it is more likely that other
programs will work with your new file format and it is also more
likely that your modified interpreter will still be able to understand
the original file format.

  An extension shouldn't change any of the existing data fields; nor
should it insert new data fields into the middle of records.  An
extension *may* add new fields onto the end of one or more of the
records and it can define new blocks.

  Examples of things that would be extensions (create a new extension
id and number, but keep the version the same):

--Adding a new field onto the end of the creature record, containing
    the code for a sound file that should be played whenever
    the creature is in the room.

--Adding a new block to the file containing debugging information for
    the new AGT compiler you've just written, numbered 35.


  Things that would *not* be extensions (create a new version id and
number; do what you want with the extension id and number)

  --Going to 32-bit object numbers for everything. (There *are*
sneaky ways you could make this an extension; but not if you just do this
by converting all the int16 fields in the file to int32)

  --Changing metacommands to accept an arbitrary string of tokens
instead of just  ACTOR,VERB NOUN PREP OBJECT.



A FEW NOTES ON BACKWARD COMPATIBILITY

  (These notes only apply if you are creating an extension; if you are
creating a new version, then anything goes)

  If you add a new field onto an existing record (like the creature
soundtrack example above) and read in an old-format file, then the new
data fields will be automatically initialized to zero so long as none
of them are individually longer than 81 bytes (if any *are* longer than
81 bytes then the file routines may break). (There is nothing magic
about 81 bytes; it just happens to be the length of the largest data
structure that shows up in 'vanilla' AGX files).

  If you add a new block, then you should check the extension number
of the file and if the block doesn't exists be prepared to either
initialize the data to something sensible or to exit cleanly with an
error message.

 */

/* AGX File format versions and corresponding versions of AGiliTy
   and Magx: (versions given as 'Version-Extension')

 AGX  AGiliTy    Magx
 0-0    0.5
 0-1    0.7       0.1
 1-0    0.7.2     0.2
 1-1    0.8
 1-2    0.8.1     0.3
 1-3    0.8.3     0.4
 1-4    0.8.5
 1-5    0.8.6     0.5
 1-6    0.8.7     0.5.2
 1-7    0.8.8     0.6
 2-0    0.8.9     0.6.1
 2-1    1.0       0.6.3
 2-2    1.1       0.6.4
 */

/* Changes is AGX File format versions:
  0-0: Original format.
  0-1: Add
    PURE_TIME, PURE_OBJ_DESC, exitmsg_base
    noun.related_name
    command.noun_adj, command.obj_adj
  1-0: Change index file format, fixing a bug
  1-1: Add
      Multi-word verb block(28)
      Preposition block(29)
      (room|noun|creature).oclass
  1-2: Add (room|noun|creature).unused
  1-3: Add PURE_GRAMMAR
  1-4: Add (noun|creature).isglobal and (noun|creature).flagnum
       Add TWO_CYCLE
  1-5: Add min_ver
       Add PURE_AFTER
  1-6: Add (noun|creature).seen
  1-7: Add objflag and objprop blocks (with corrosponding
         support data in the gameinfo block).
  2-0: No change in file format; version upped to protect against
        a bug in early versions of the AGX file code.
  2-1: Added (noun|creature).proper
  2-2: Added noun_obj and obj_obj to cmd header
       Added objflag.ystr, objflag.nstr, objprop.str_cnt, objprop.str_list
       Added propstr block.
       Added fallback_ext to file header.
*/

#define AGX_NUMBLOCK 37
#define AGT_FILE_SIG 0x51C1C758L

/* AGX File format:
  (This tends to lag a little behind the code below;
  you should double check against the actual file_info definitions
  below)
All integer values stored little-endian.
desc_ptrs: int32 ofs, int32 leng (both in lines)
dictionary word: int16
slist ptr: int16
tline: char[81]
filename: char[10]
rbool values are packed into bytes, 1 bit per value, from lsb to msb.
cfgopt: 0=false, 1=true, 2=leave alone

Mandatory blocks are marked with astericks.

*File header: 16  bytes
   uint 32 File ID [AGT_FILE_SIG, 4 bytes]
   byte Version owner: 'R'
   byte Version 0
   byte Extension owner 'R'
   byte Extension 0
   char[2]: '\n\r'  -- to catch download errors
   byte Extension fallback. For non-'R' extensions, this gives the
           'R' extension to fall back to.
   char[5] Reserved for future use (should be 0 right now)
*0-File index:
   For each block (including itself): [16 bytes]
        uint32 starting offset
        uint32 block size
    uint32 number of records
    uint32 size of a record  (recsize*numrec == blocksize)
11-Description strings (block of tline)
12-Command text (block of int16)
*1-Game header
   uint16 AGT_version_code;  +1 for "big/soggy" games
   uint32 game_sig  (game signature, used to check save files and debug info)
   rbool debug_mode, freeze_mode, milltime_mode, bold_mode,
         have_meta, mars_fix, intro_first, TWO_CYCLE;
   uchar score_mode, statusmode;
   uint16 max_lives
   uint32 max_score;
   uint16 startup_time, delta_time;
   descr_ptr intro_ptr, title_ptr, ins_ptr;
   int16 start_room, treas_room, resurrect_room
   int16 first_room, first_noun, first_creat
   int16 FLAG_NUM, CNT_NUM, VAR_NUM
   int16 BASE_VERB
   cfgopt PURE_ANSWER, PURE_TIME, PURE_ROOMTITLE;
   cfgopt PURE_AND, PURE_METAVERB;
   cfgopt PURE_SYN, PURE_NOUN, PURE_ADJ;
   cfgopt PURE_DUMMY, PURE_SUBNAME, PURE_PROSUB;
   cfgopt PURE_HOSTILE, PURE_GETHOSTILE;
   cfgopt PURE_DISAMBIG, PURE_ALL;
   cfgopt irun_mode, verboseflag;
   cfgopt PURE_GRAMMAR  (Extension R1-R3)
   rbool TWO_CYCLE (R1-R4)
   PURE_AFTER (R1-R5)
   int16 min_ver
   uchar font_status;
   int16 num_rflags, num_nflags, num_cflags;
   int16 num_rprops, num_nprops, num_cprops;
2-Room data (room_rec format, pointers->int ref into static string)
   include help, desc, special ptrs
3-Noun data (noun_rec format)
   include noun, text, turn, push, pull, play ptrs
4-Creature data (creat_rec format)
   include creature, talk, ask ptrs
5-Command headers (cmd_rec format), pointers into command text
     must be in increasing order.
6-Standard error message ptrs (array of descptr
7-Message ptrs   (array of descptr)
8-Question pointers (array of descptr)
9-Answer pointers (array of descptr)
10-User strings  (array of tline)
*13-Static string block (block of chars)
14-Subroutine dictionary ids (array of word:int16)
*15-Synlist (for verbs) (array of slist:int16)
16-Pix names (array of word:int16 -- pointers into dictionary)
17-Global nouns (array of word:int16 -- ptrs into dictionary)
18-Flag nouns (array of word:int16)
*19-Syntbl (block of word:int16)
*20-Dictionary text (block of char)
*21-Dictionary 'index' (array of uint32)
22-OPT block (14 bytes)
23-Picture filename ptrs
24-Pix filename ptrs
25-Font filename ptrs
26-Sound filename ptrs
27-VOC block, an array of verbinfo_rec
28-Multi-word verbs  (Extension R1-R1)
29-Prep table (Extension R1-R1)
30- ObjFlag Data (Extension R1-R7)
31- ObjProp Data (Extension R1-R7)
32- ObjFlag Lookup (Extension R1-R7)
33- ObjProp Lookup (Extension R1-R7)
34- ObjProp string pointers (array of FT_STR) (Extension R2-R2)
35- Variable itemization array  (Extension R2-R2)
36- Flag itemization array (Extension R2-R2)

*/

/* AGT Version IDs; +1 for LARGE/SOGGY
        00000=v1.0
    01800=v1.18
    01900=v1.19
    02000=v1.20           ("Early Classic")
    03200=v1.32/COS
    03500=v1.35           ("Classic")
    05000=v1.5/H
    05050=v1.5/F (MDT)
        05070=v1.6   (PORK)
    08200=v1.82
    08300=v1.83
        10000=ME/1.0
    15000=ME/1.5
    15500=ME/1.55
    16000=ME/1.6
    20000=Magx/0.0
    etc.
*/




/* ------------------------------------------------------------- */
/*        AGX Block Descriptions                                 */
/* ------------------------------------------------------------- */


static integer old_base_verb;

/* AGX file info blocks */

#define g(ft,dt,var) {ft,dt,&var,0}
#define r(ft,dt,str,f) {ft,dt,NULL,offsetof(str,f)}
#define dptype {FT_DESCPTR,DT_DESCPTR,NULL,0}
#define xx DT_DEFAULT
#define u16 FT_UINT16
#define u32 FT_UINT32
#define bb  FT_BOOL
#define i16 FT_INT16

static file_info fi_gameinfo[] = {
	/* 0  */
	g(FT_VERSION, xx, aver), /* FT_VERSION converter also sets ver */
	g(u32, DT_LONG, game_sig),
	/* 6 */
	g(bb, xx, debug_mode), g(bb, xx, freeze_mode), g(bb, xx, milltime_mode),
	g(bb, xx, bold_mode), g(bb, xx, have_meta), g(bb, xx, mars_fix),
	g(bb, xx, intro_first), g(bb, xx, box_title),
	/* 7 */
	g(FT_BYTE, xx, score_mode), g(FT_BYTE, xx, statusmode),
	g(i16, xx, max_lives), g(u32, DT_LONG, max_score),
	/* 15 */
	g(i16, xx, startup_time), g(i16, xx, delta_time),
	/* 19 */
	g(FT_DESCPTR, xx, intro_ptr), g(FT_DESCPTR, xx, title_ptr),
	g(FT_DESCPTR, xx, ins_ptr),
	/* 43 */
	g(i16, xx, treas_room),
	g(i16, xx, start_room), g(i16, xx, resurrect_room),
	g(i16, xx, first_room),  g(i16, xx, first_noun),
	g(i16, xx, first_creat), g(i16, xx, FLAG_NUM),
	g(i16, xx, CNT_NUM), g(i16, xx, VAR_NUM),
	g(i16, xx, old_base_verb),
	/* 63 */
	g(FT_CFG, xx, PURE_ANSWER), g(FT_CFG, xx, PURE_ROOMTITLE),
	g(FT_CFG, xx, PURE_AND), g(FT_CFG, xx, PURE_METAVERB),
	g(FT_CFG, xx, PURE_SYN), g(FT_CFG, xx, PURE_NOUN), g(FT_CFG, xx, PURE_ADJ),
	g(FT_CFG, xx, PURE_DUMMY), g(FT_CFG, xx, PURE_SUBNAME),
	g(FT_CFG, xx, PURE_PROSUB), g(FT_CFG, xx, PURE_HOSTILE),
	g(FT_CFG, xx, PURE_GETHOSTILE), g(FT_CFG, xx, PURE_DISAMBIG),
	g(FT_CFG, xx, PURE_ALL),
	g(FT_CFG, xx, irun_mode), g(FT_CFG, xx, verboseflag),
	g(FT_CFG, xx, PURE_TIME),    /* Ext R0-1 */
	g(FT_CFG, xx, PURE_OBJ_DESC), /* Ext R0-1 */
	/* 81 */
	g(i16, xx, exitmsg_base),         /* Ext R0-1 */
	/* 83 */
	g(FT_CFG, xx, PURE_GRAMMAR),   /* Ext R1-3 */
	g(bb, xx, TWO_CYCLE),          /* Ext R1-4 */
	g(bb, xx, PURE_AFTER),         /* Ext R1-5 */
	g(i16, xx, min_ver),           /* Ext R1-5 */
	g(FT_BYTE, xx, font_status),   /* Ext R1-5 */
	g(i16, xx, num_rflags), g(i16, xx, num_nflags), g(i16, xx, num_cflags), /* Ext R1-7 */
	g(i16, xx, num_rprops), g(i16, xx, num_nprops), g(i16, xx, num_cprops), /* Ext R1-7 */
	endrec
};

static file_info fi_room[] = {
	dptype, /* help */
	dptype, /* desc */
	dptype, /* special */
	r(FT_STR, xx, room_rec, name),
	r(FT_INT32, xx, room_rec, flag_noun_bits),
	r(FT_INT32, xx, room_rec, PIX_bits),
	r(FT_SLIST, xx, room_rec, replacing_word),
	r(FT_WORD, xx, room_rec, replace_word),
	r(FT_WORD, xx, room_rec, autoverb),
	r(FT_PATHARRAY, xx, room_rec, path),
	r(FT_INT16, xx, room_rec, key),
	r(FT_INT16, xx, room_rec, points),
	r(FT_INT16, xx, room_rec, light),
	r(FT_INT16, xx, room_rec, pict),
	r(FT_INT16, xx, room_rec, initdesc),
	r(bb, xx, room_rec, seen), r(bb, xx, room_rec, locked_door),
	r(bb, xx, room_rec, end), r(bb, xx, room_rec, win), r(bb, xx, room_rec, killplayer),
	r(bb, xx, room_rec, unused),    /* Ext R1-2: Can add here since rbool */
	r(FT_INT16, xx, room_rec, oclass), /* Ext R1-1 */
	endrec
};

static file_info fi_noun[] = {
	dptype, /* Noun */
	dptype, /* Text */
	dptype, dptype, dptype, dptype, /* Turn, push, pull, play */
	r(FT_STR, xx, noun_rec, shortdesc),
	r(FT_STR, xx, noun_rec, position),
	r(FT_SLIST, xx, noun_rec, syns),
	r(FT_WORD, xx, noun_rec, name),  r(FT_WORD, xx, noun_rec, adj),
	/* r(FT_WORD,xx,noun_rec,pos_prep),  r(FT_WORD,xx,noun_rec,pos_name),*/
	r(FT_INT16, xx, noun_rec, nearby_noun),
	r(FT_INT16, xx, noun_rec, num_shots), r(FT_INT16, xx, noun_rec, points),
	r(FT_INT16, xx, noun_rec, weight), r(FT_INT16, xx, noun_rec, size),
	r(FT_INT16, xx, noun_rec, key),
	r(FT_INT16, xx, noun_rec, initdesc), r(FT_INT16, xx, noun_rec, pict),
	r(FT_INT16, xx, noun_rec, location),
	r(bb, xx, noun_rec, plural),
	r(bb, xx, noun_rec, something_pos_near_noun),
	r(bb, xx, noun_rec, has_syns),
	r(bb, xx, noun_rec, pushable),  r(bb, xx, noun_rec, pullable),
	r(bb, xx, noun_rec, turnable),  r(bb, xx, noun_rec, playable),
	r(bb, xx, noun_rec, readable),  r(bb, xx, noun_rec, on),
	r(bb, xx, noun_rec, closable),  r(bb, xx, noun_rec, open),
	r(bb, xx, noun_rec, lockable),  r(bb, xx, noun_rec, locked),
	r(bb, xx, noun_rec, edible),  r(bb, xx, noun_rec, wearable),
	r(bb, xx, noun_rec, drinkable),  r(bb, xx, noun_rec, poisonous),
	r(bb, xx, noun_rec, movable),  r(bb, xx, noun_rec, light),
	r(bb, xx, noun_rec, shootable),  r(bb, xx, noun_rec, win),
	r(bb, xx, noun_rec, unused),  /* Ext R1-2: Can add here since packed rbool*/
	r(bb, xx, noun_rec, isglobal), /* Ext R1-4: ditto (&room for 1 more). */
	r(FT_WORD, xx, noun_rec, related_name), /* Ext R0-1 */
	r(FT_INT16, xx, noun_rec, oclass), /* Ext R1-1 */
	r(FT_INT16, xx, noun_rec, flagnum), /* Ext R1-4 */
	r(bb, xx, noun_rec, seen),       /* Ext R1-6 */
	r(bb, xx, noun_rec, proper),     /* Ext R2-1 */
	endrec
};

static file_info fi_creat[] = {
	dptype,  /* Creature */
	dptype, dptype,  /* Talk, ask */
	r(FT_STR, xx, creat_rec, shortdesc),
	r(FT_SLIST, xx, creat_rec, syns),
	r(FT_WORD, xx, creat_rec, name), r(FT_WORD, xx, creat_rec, adj),
	r(FT_INT16, xx, creat_rec, location),
	r(FT_INT16, xx, creat_rec, weapon), r(FT_INT16, xx, creat_rec, points),
	r(FT_INT16, xx, creat_rec, counter), r(FT_INT16, xx, creat_rec, threshold),
	r(FT_INT16, xx, creat_rec, timethresh), r(FT_INT16, xx, creat_rec, timecounter),
	r(FT_INT16, xx, creat_rec, pict), r(FT_INT16, xx, creat_rec, initdesc),
	r(bb, xx, creat_rec, has_syns), r(bb, xx, creat_rec, groupmemb),
	r(bb, xx, creat_rec, hostile),
	r(bb, xx, creat_rec, unused),    /* Ext R1-2: Can add since packed rbool */
	r(bb, xx, creat_rec, isglobal),  /* Ext R1-4: ditto (& space for 3 more) */
	r(FT_BYTE, xx, creat_rec, gender),
	r(FT_INT16, xx, creat_rec, oclass), /* Ext R1-1 */
	r(FT_INT16, xx, creat_rec, flagnum), /* Ext R1-4 */
	r(bb, xx, creat_rec, seen),       /* Ext R1-6 */
	r(bb, xx, creat_rec, proper),     /* Ext R2-1 */
	endrec
};

static file_info fi_cmdhead[] = {
	{FT_CMDPTR, DT_CMDPTR, NULL, 0},
	r(FT_INT16, xx, cmd_rec, actor),
	r(FT_WORD, xx, cmd_rec, verbcmd), r(FT_WORD, xx, cmd_rec, nouncmd),
	r(FT_WORD, xx, cmd_rec, objcmd), r(FT_WORD, xx, cmd_rec, prep),
	r(FT_INT16, xx, cmd_rec, cmdsize),
	r(FT_WORD, xx, cmd_rec, noun_adj), r(FT_WORD, xx, cmd_rec, obj_adj), /* Ext R0-1*/
	r(FT_INT16, xx, cmd_rec, noun_obj), /* Ext R2-2 */
	r(FT_INT16, xx, cmd_rec, obj_obj), /* Ext R2-2 */
	endrec
};

static file_info fi_verbentry[] = {
	r(FT_WORD, xx, verbentry_rec, verb),
	r(FT_WORD, xx, verbentry_rec, prep),
	r(FT_INT16, xx, verbentry_rec, objnum),
	endrec
};


static file_info fi_descptr[] = {
	r(FT_INT32, xx, descr_ptr, start),
	r(FT_INT32, xx, descr_ptr, size),
	endrec
};

static file_info fi_tline[] = {
	{FT_TLINE, xx, NULL, 0},
	endrec
};

static file_info fi_attrrec[] = { /* Ext R1-R7 */
	r(FT_INT32, xx, attrdef_rec, r),
	r(FT_INT32, xx, attrdef_rec, n),
	r(FT_INT32, xx, attrdef_rec, c),
	r(FT_BYTE, xx, attrdef_rec, rbit),
	r(FT_BYTE, xx, attrdef_rec, nbit),
	r(FT_BYTE, xx, attrdef_rec, cbit),
	r(FT_STR, xx, attrdef_rec, ystr), /* Ext R2-R2 */
	r(FT_STR, xx, attrdef_rec, nstr), /* Ext R2-R2 */
	endrec
};

static file_info fi_proprec[] = {   /* Ext R1-R7 */
	r(FT_INT32, xx, propdef_rec, r),
	r(FT_INT32, xx, propdef_rec, n),
	r(FT_INT32, xx, propdef_rec, c),
	r(FT_INT32, xx, propdef_rec, str_cnt), /* Ext R2-R2 */
	r(FT_INT32, xx, propdef_rec, str_list), /* Ext R2-R2 */
	endrec
};

static file_info fi_varrec[] = {        /* Ext R2-R2 */
	r(FT_INT32, xx, vardef_rec, str_cnt),
	r(FT_INT32, xx, vardef_rec, str_list),
	endrec
};

static file_info fi_flagrec[] = { /* Ext R2-R2 */
	r(FT_STR, xx, flagdef_rec, ystr),
	r(FT_STR, xx, flagdef_rec, nstr),
	endrec
};

#undef g
#undef r
#undef xx
#undef u16
#undef u32
#undef bb
#undef i16
#undef dptype

static void set_endrec(file_info *fi, int index) {
	fi[index].ftype = FT_END;
	fi[index].dtype = 0;
	fi[index].ptr = NULL;
	fi[index].offset = 0;
}

/* ------------------------------------------------------------- */

/* If <to_intern> is true, convert "0" string ptrs to "yes/no" ptrs.
   If it is false, convert the other way. */
/* This is done for the sake of downward compatibility.
   It *does* mean that the first string in static_str cannot
   be an attribute's yes/no string. */

/* "0" pointers in this case will actually be equal to static_str
   (since that is the base point for all pointers to static strings.) */

const char base_yesstr[] = "yes";
const char base_nostr[] = "no";

static void conv_fstr(const char **s, rbool yes, rbool to_intern) {
	if (to_intern) {  /* Convert to internal form */
		assert(*s != NULL);
		if (*s == static_str) *s = yes ? base_yesstr : base_nostr;
	} else { /* convert to external form */
		if (*s == NULL || *s == base_yesstr || *s == base_nostr)
			*s = static_str;
	}
}

static void fix_objflag_str(rbool to_intern) {
	int i;
	for (i = 0; i < oflag_cnt; i++) {
		conv_fstr(&attrtable[i].ystr, 1, to_intern);
		conv_fstr(&attrtable[i].nstr, 0, to_intern);
	}
	if (flagtable)
		for (i = 0; i <= FLAG_NUM; i++) {
			conv_fstr(&flagtable[i].ystr, 1, to_intern);
			conv_fstr(&flagtable[i].nstr, 0, to_intern);
		}
}

/* ------------------------------------------------------------- */
/*        AGX Reading Code                                       */
/* ------------------------------------------------------------- */


static long descr_ofs;

void agx_close_descr(void) {
	if (mem_descr != NULL)
		rfree(mem_descr);
	else if (descr_ofs != -1)
		buffclose(); /* This closes the whole AGX file */
}

descr_line *agx_read_descr(long start, long size) {
	long i, line, len;
	descr_line *txt;
	char *buff;

	if (size <= 0) return NULL;

	if (mem_descr == NULL && descr_ofs != -1)
		buff = (char *)read_recblock(NULL, FT_CHAR, size,
		                             descr_ofs + start, size * ft_leng[FT_CHAR]);
	else
		buff = mem_descr + start;

	len = 0;
	for (i = 0; i < size; i++) /* Count the number of lines */
		if (buff[i] == 0) len++;
	txt = (descr_line *)rmalloc(sizeof(descr_line) * (len + 1));
	txt[0] = buff;
	i = 0;
	for (line = 1; line < len;) /* Determine where each of the lines is */
		if (buff[i++] == 0)
			txt[line++] = buff + i;
	txt[len] = NULL; /* Mark the end of the array */
	return txt;
}


/* We need to read in command text and use cmd_rec[] values to
   rebuild command[].data. We are guaranteed that cmd_rec[] is in
   increasing order */

static void read_command(long cmdcnt, long cmdofs, rbool diag) {
	long i;

	for (i = 0; i < last_cmd; i++) {
		command[i].data = (integer *)rmalloc(sizeof(integer) * command[i].cmdsize);
		read_recblock(command[i].data, FT_INT16, command[i].cmdsize,
		              cmdofs + 2 * cmd_ptr[i], 2 * command[i].cmdsize);
	}
	if (!diag) rfree(cmd_ptr);
}


/* Correct for differences between old_base_verb and BASE_VERB.
   This means that the interpreter's set of built-inv verbs has changed
   since the file was created. */
static void correct_synlist(void) {
	int i;
	if (BASE_VERB == old_base_verb) return; /* Nothing needs to be done */

	/* Need to move everything >= old_base_verb to BASE_VERB */
	memmove(synlist + BASE_VERB, synlist + old_base_verb,
	        sizeof(slist) * (DVERB + MAX_SUB));

	if (BASE_VERB < old_base_verb) /* We've _lost_ verbs */
		agtwarn("Missing built-in verbs.", 0);

	/* Now we need to give the "new" verbs empty synonym lists */
	for (i = old_base_verb; i < BASE_VERB; i++)
		synlist[i] = synptr;
	addsyn(-1);
}



static void set_roomdesc(file_info fi[]) {
	fi[0].ptr = help_ptr = (descr_ptr *)rmalloc(sizeof(descr_ptr) * (maxroom - first_room + 1));
	fi[1].ptr = room_ptr = (descr_ptr *)rmalloc(sizeof(descr_ptr) * (maxroom - first_room + 1));
	fi[2].ptr = special_ptr = (descr_ptr *)rmalloc(sizeof(descr_ptr) * (maxroom - first_room + 1));
}

static void wset_roomdesc(file_info fi[]) {
	fi[0].ptr = help_ptr;
	fi[1].ptr = room_ptr;
	fi[2].ptr = special_ptr;
}

static void set_noundesc(file_info *fi) {
	fi[0].ptr = noun_ptr = (descr_ptr *)rmalloc(sizeof(descr_ptr) * (maxnoun - first_noun + 1));
	fi[1].ptr = text_ptr = (descr_ptr *)rmalloc(sizeof(descr_ptr) * (maxnoun - first_noun + 1));
	fi[2].ptr = turn_ptr = (descr_ptr *)rmalloc(sizeof(descr_ptr) * (maxnoun - first_noun + 1));
	fi[3].ptr = push_ptr = (descr_ptr *)rmalloc(sizeof(descr_ptr) * (maxnoun - first_noun + 1));
	fi[4].ptr = pull_ptr = (descr_ptr *)rmalloc(sizeof(descr_ptr) * (maxnoun - first_noun + 1));
	fi[5].ptr = play_ptr = (descr_ptr *)rmalloc(sizeof(descr_ptr) * (maxnoun - first_noun + 1));
}

static void wset_noundesc(file_info *fi) {
	fi[0].ptr = noun_ptr;
	fi[1].ptr = text_ptr;
	fi[2].ptr = turn_ptr;
	fi[3].ptr = push_ptr;
	fi[4].ptr = pull_ptr;
	fi[5].ptr = play_ptr;
}

static void set_creatdesc(file_info *fi) {
	fi[0].ptr = creat_ptr = (descr_ptr *)rmalloc(sizeof(descr_ptr) * (maxcreat - first_creat + 1));
	fi[1].ptr = talk_ptr = (descr_ptr *)rmalloc(sizeof(descr_ptr) * (maxcreat - first_creat + 1));
	fi[2].ptr = ask_ptr = (descr_ptr *)rmalloc(sizeof(descr_ptr) * (maxcreat - first_creat + 1));
}

static void wset_creatdesc(file_info *fi) {
	fi[0].ptr = creat_ptr;
	fi[1].ptr = talk_ptr;
	fi[2].ptr = ask_ptr;
}

static void set_cmdptr(file_info *fi) {
	fi[0].ptr = cmd_ptr = (long *)rmalloc(sizeof(long) * last_cmd);
}

static void wset_cmdptr(file_info *fi) {
	fi[0].ptr = cmd_ptr;
}


typedef struct {  /* Entries in the index header of the AGX file */
	uint32 file_offset;
	uint32 blocksize;
	uint32 numrec;
	uint32 recsize;
} index_rec;

static file_info fi_index[] = {
	{FT_UINT32, DT_DEFAULT, NULL, offsetof(index_rec, file_offset)},
	{FT_UINT32, DT_DEFAULT, NULL, offsetof(index_rec, blocksize)},
	{FT_UINT32, DT_DEFAULT, NULL, offsetof(index_rec, numrec)},
	{FT_UINT32, DT_DEFAULT, NULL, offsetof(index_rec, recsize)},
	endrec
};


/*
   uint32 File ID ['....' 4 bytes]
   byte Version owner: 'R'
   byte Version 0
   byte Extension owner 'R'
   byte Extension 0
   */

struct file_head_rec  {
	uint32 fileid;
	uint32 res1; /* Reserved for future use */
	uchar res2;
	uchar eol_chk1;  /* Catch non-binary upload errors */
	uchar eol_chk2;
	uchar ver_own;
	uchar version;
	uchar ext_own;
	uchar extnum;
	uchar fallback_ext;  /* For non-'R' extensions, this is the 'R' extension
              to fall back to. */
};

static file_info fi_header[] = {
	{FT_UINT32, DT_LONG, NULL, offsetof(file_head_rec, fileid)}, /* File ID */
	{FT_BYTE, DT_DEFAULT, NULL, offsetof(file_head_rec, ver_own)}, /* Owner */
	{FT_BYTE, DT_DEFAULT, NULL, offsetof(file_head_rec, version)}, /* Version */
	{FT_BYTE, DT_DEFAULT, NULL, offsetof(file_head_rec, ext_own)}, /*Ext owner*/
	{FT_BYTE, DT_DEFAULT, NULL, offsetof(file_head_rec, extnum)}, /* Ext vers */
	{FT_BYTE, DT_DEFAULT, NULL, offsetof(file_head_rec, eol_chk1)},
	{FT_BYTE, DT_DEFAULT, NULL, offsetof(file_head_rec, eol_chk2)},
	{FT_BYTE, DT_DEFAULT, NULL, offsetof(file_head_rec, fallback_ext)},
	{FT_BYTE, DT_DEFAULT, NULL, offsetof(file_head_rec, res2)},
	{FT_UINT32, DT_DEFAULT, NULL, offsetof(file_head_rec, res1)},
	endrec
};

static const char *block_name[AGX_NUMBLOCK] = {
	"Index", "Game Info", "Room(DA2)", "Noun(DA3)", "Creature(DA4)",
	"Command Header(DA5)", "Error Message(STD)", "Message",
	"Question", "Answer", "User String", "Description Text(D$$)",
	"Command Tokens(DA6)", "Static String", "Subroutine ID",
	"Verb Synonym", "RoomPIX", "Global Noun", "Flag Noun", "Word Lists(Syntbl)",
	"Dictionary Text", "Dictionary Index", "OPT",
	"Picture Filename", "RoomPIX Filename", "Font Filename", "Sound Filename",
	"Menu(VOC)", "Multi-word Verb", "Preposition", "ObjFlag", "ObjProp",
	"Attrtable", "PropTable", "PropStr", "Itemized Variables",
	"Itemized Flags"
};


/* Return 0 on failure, 1 on success */
int read_agx(fc_type fc, rbool diag) {
	file_head_rec filehead;
	unsigned long fsize;
	index_rec *index;
	long i;
	int index_recsize;
	int index_start;

	agx_file = 1;
	fsize = buffopen(fc, fAGX, 16, NULL, 1);
	if (fsize == 0) {
		agx_file = 0;
		return 0;
	}

	/* Read header */
	read_recarray(&filehead, sizeof(file_head_rec), 1, fi_header,
	              "File Header", 0, compute_recsize(fi_header));
	if (filehead.fileid != AGT_FILE_SIG) {
		buffclose();
		return 0;
	}
	if (DIAG) {
		rprintf("AGX file format");
		if (isprint(filehead.ver_own) && isprint(filehead.ext_own))
			rprintf("   Version:%c%d\tExtension:%c%d\n",
			        filehead.ver_own, filehead.version,
			        filehead.ext_own, filehead.extnum);
		else
			rprintf("   Version:%d:%d\tExtension:%d:%d\n",
			        filehead.ver_own, filehead.version,
			        filehead.ext_own, filehead.extnum);
	}
	if (filehead.ver_own != 'R' || filehead.version > 2) {
		rprintf("Unsupported AGX file version.\n");
		rprintf("  Either the file is corrupted or or you need a more recent "
		        "version of AGiliTy.\n");
		rprintf("\n");
		fatal("Can't read AGX file.");
	}

	index_recsize = compute_recsize(fi_index);
	if (filehead.version == 0) {
		if (debug_da1)
			rprintf("[AGX version 0: obsolete.]\n");
		index_recsize += 8; /* Extra junk block in version 0. */
		index_start = 8;
	} else {
		index_start = 16;
		if (filehead.eol_chk1 != '\n' || filehead.eol_chk2 != '\r')
			fatal("File apparently downloaded as non-binary file.");
	}
	if (filehead.ext_own != 'R'
	        || (filehead.version == 0 && filehead.extnum > 1)
	        || (filehead.version == 1 && filehead.extnum > 7)
	        || (filehead.version == 2 && filehead.extnum > 2))
		agtwarn("Unrecognized extension to AGX file format.", 0);
	if (filehead.ext_own != 'R') { /* Assume lowest common denomenator */
		if (filehead.version < 2)
			fatal("Extensions of AGX beta versions not supported.");
		if (filehead.fallback_ext < 1) filehead.fallback_ext = 1;
	}

	/* Now read master index */
	/* This assumes that the file is long enough to absorb any
	   'extra' blocks we read in in early versions with fewer blocks. */
	/* (Right now, this must be true: the next block alone is big enough) */
	index = (index_rec *)read_recarray(NULL, sizeof(index_rec), AGX_NUMBLOCK,
	                                   fi_index, "File Index", index_start,
	                                   index_recsize * AGX_NUMBLOCK);

	/* Zero index entries for any blocks that are beyond the bounds of the
	   file's index */
	if (AGX_NUMBLOCK > index[0].numrec)
		memset(index + index[0].numrec, 0,
		       (AGX_NUMBLOCK - index[0].numrec)*sizeof(index_rec));

	if (DIAG) {
		rprintf("\n");
		rprintf("File Index:\n");
		rprintf("    Offset   Size   NumRec  RecSz\n");
		rprintf("    ------  ------  ------  ------\n");
		for (i = 0; i < AGX_NUMBLOCK; i++)
			rprintf("%2ld: %6d  %6d  %6d  %6d   %s\n", i,
			        index[i].file_offset, index[i].blocksize,
			        index[i].numrec, index[i].recsize, block_name[i]);
	}
	if ((int)index[0].file_offset != index_start)
		fatal("File header corrupted.");

	for (i = 0; i < AGX_NUMBLOCK; i++) { /* Error checking */
#ifdef DEBUG_AGX
		rprintf("  Verifying block %d...\n", i);
#endif
		if (index[i].recsize * index[i].numrec != index[i].blocksize)
			fatal("File header corrupted.");
		if (index[i].file_offset + index[i].blocksize > fsize)
			fatal("File index points past end of file.");
	}

	/* Check for mandatory fields */
	if (!index[0].numrec    /* File index */
	        || !index[1].numrec /* Game header */
	        || !index[13].numrec /* Static string block */
	        || !index[15].numrec /* Synonym list */
	        || !index[19].numrec /* Syntbl */
	        || !index[20].numrec /* Dictionary text */
	        || !index[21].numrec /* Dictionary index */
	   )
		fatal("AGX file missing mandatory block.");


	read_globalrec(fi_gameinfo, "Game Info", index[1].file_offset,
	               index[1].blocksize);
	if (filehead.version == 0 && filehead.extnum == 0) {
		exitmsg_base = 1000;
		if (aver >= AGT15)
			box_title = 1;
	}
	if (index[1].blocksize == 83 && filehead.version == 1 && filehead.extnum >= 5) {
		/* Detect 0.8-compatibility hack */
		filehead.extnum = 2;
	}
	if (filehead.version == 0 || (filehead.version == 1 && filehead.extnum < 5)) {
		if (aver >= AGT182 && aver <= AGT18MAX) {
			if (filehead.extnum < 4) TWO_CYCLE = 1;
		} else
			PURE_AFTER = 1;
	}

	/* Need to read in ss_array before rooms/nouns/creatures */
	ss_size = ss_end = index[13].numrec;
	static_str = (char *)read_recblock(NULL, FT_CHAR,
	                                   index[13].numrec, index[13].file_offset,
	                                   index[13].blocksize);

	synptr = syntbl_size = index[19].numrec;
	syntbl = (word *)read_recblock(NULL, FT_WORD, index[19].numrec, index[19].file_offset,
	                               index[19].blocksize);

	maxroom = first_room + index[2].numrec - 1;
	set_roomdesc(fi_room);
	room = (room_rec *)read_recarray(NULL, sizeof(room_rec), index[2].numrec,
	                                 fi_room, "Room", index[2].file_offset, index[2].blocksize);

	maxnoun = first_noun + index[3].numrec - 1;
	set_noundesc(fi_noun);
	noun = (noun_rec *)read_recarray(NULL, sizeof(noun_rec), index[3].numrec,
	                                 fi_noun, "Noun", index[3].file_offset, index[3].blocksize);

	last_obj = maxcreat = first_creat + index[4].numrec - 1;
	set_creatdesc(fi_creat);
	creature = (creat_rec *)read_recarray(NULL, sizeof(creat_rec), index[4].numrec,
	                                      fi_creat, "Creature", index[4].file_offset,
	                                      index[4].blocksize);

	last_cmd = index[5].numrec;
	set_cmdptr(fi_cmdhead);
	command = (cmd_rec *)read_recarray(NULL, sizeof(cmd_rec), index[5].numrec,
	                                   fi_cmdhead, "Metacommand", index[5].file_offset,
	                                   index[5].blocksize);
	if (filehead.ext_own != 'R' && filehead.fallback_ext <= 1) {
		for (i = 0; i < last_cmd; i++)
			command[i].noun_obj = command[i].obj_obj = 0;
	}

	NUM_ERR = index[6].numrec;
	err_ptr = (descr_ptr *)read_recarray(NULL, sizeof(descr_ptr), index[6].numrec,
	                                     fi_descptr, "Error Message", index[6].file_offset,
	                                     index[6].blocksize);

	last_message = index[7].numrec;
	msg_ptr = (descr_ptr *)read_recarray(NULL, sizeof(descr_ptr), index[7].numrec,
	                                     fi_descptr, "Message", index[7].file_offset,
	                                     index[7].blocksize);

	MaxQuestion = index[8].numrec;
	question = answer = NULL;
	quest_ptr = (descr_ptr *)read_recarray(NULL, sizeof(descr_ptr), index[8].numrec,
	                                       fi_descptr, "Question", index[8].file_offset,
	                                       index[8].blocksize);
	if (index[9].numrec != index[8].numrec)
		fatal("File corrputed: questions and answers don't match.");
	ans_ptr = (descr_ptr *)read_recarray(NULL, sizeof(descr_ptr), index[9].numrec,
	                                     fi_descptr, "Answer", index[9].file_offset,
	                                     index[9].blocksize);

	MAX_USTR = index[10].numrec;
	userstr = (tline *)read_recarray(NULL, sizeof(tline), index[10].numrec,
	                                 fi_tline, "User String", index[10].file_offset,
	                                 index[10].blocksize);

	MAX_SUB = index[14].numrec;
	sub_name = (word *)read_recblock(NULL, FT_WORD, index[14].numrec, index[14].file_offset,
	                                 index[14].blocksize);

	if (index[16].numrec > MAX_PIX) {
		index[16].numrec = MAX_PIX;
		index[16].blocksize = index[16].recsize * index[16].numrec;
	}
	maxpix = index[16].numrec;
	for (i = 0; i < MAX_PIX; i++) pix_name[i] = 0; /* In case there are less than
                       MAX_PIX names */
	read_recblock(pix_name, FT_WORD, index[16].numrec, index[16].file_offset,
	              index[16].blocksize);

	numglobal = index[17].numrec;
	globalnoun = (word *)read_recblock(NULL, FT_WORD,
	                                   index[17].numrec, index[17].file_offset,
	                                   index[17].blocksize);

	if (index[18].numrec > MAX_FLAG_NOUN) {
		index[18].numrec = MAX_FLAG_NOUN;
		index[18].blocksize = index[18].recsize * index[18].numrec;
	}

	for (i = 0; i < MAX_FLAG_NOUN; i++) flag_noun[i] = 0;
	read_recblock(flag_noun, FT_WORD, index[18].numrec, index[18].file_offset,
	              index[18].blocksize);



	DVERB = index[15].numrec - old_base_verb - MAX_SUB;
	synlist = (slist *)read_recblock(NULL, FT_SLIST, index[15].numrec, index[15].file_offset,
	                                 index[15].blocksize);
	correct_synlist();

	num_comb = index[28].numrec;
	comblist = (slist *)read_recblock(NULL, FT_SLIST, index[28].numrec, index[28].file_offset,
	                                  index[28].blocksize);

	num_prep = index[29].numrec;
	userprep = (slist *)read_recblock(NULL, FT_SLIST, index[29].numrec, index[29].file_offset,
	                                  index[29].blocksize);

	/* dicstr must be read in before dict */
	dictstrsize = dictstrptr = index[20].numrec;
	dictstr = (char *)read_recblock(NULL, FT_CHAR, index[20].numrec, index[20].file_offset,
	                                index[20].blocksize);

	dp = index[21].numrec;
	dict = (char **)read_recblock(NULL, FT_DICTPTR,
	                              index[21].numrec, index[21].file_offset,
	                              index[21].blocksize);

	have_opt = (index[22].numrec != 0);
	for (i = 0; i < 14; i++) opt_data[i] = 0;
	if (have_opt) {
		if (index[22].numrec > 14) index[22].numrec = 14;
		read_recblock(opt_data, FT_BYTE, index[22].numrec, index[22].file_offset,
		              index[22].blocksize);
	}

	maxpict = index[23].numrec;
	pictlist = (filename *)read_recblock(NULL, FT_STR, index[23].numrec, index[23].file_offset,
	                                     index[23].blocksize);
	maxpix = index[24].numrec;
	pixlist = (filename *)read_recblock(NULL, FT_STR, index[24].numrec, index[24].file_offset,
	                                    index[24].blocksize);
	maxfont = index[25].numrec;
	fontlist = (filename *)read_recblock(NULL, FT_STR, index[25].numrec, index[25].file_offset,
	                                     index[25].blocksize);
	maxsong = index[26].numrec;
	songlist = (filename *)read_recblock(NULL, FT_STR, index[26].numrec, index[26].file_offset,
	                                     index[26].blocksize);

	vm_size = index[27].numrec;
	verbinfo = (verbentry_rec *)read_recarray(NULL, sizeof(verbentry_rec), index[27].numrec,
	           fi_verbentry, "Menu Vocabulary", index[27].file_offset,
	           index[27].blocksize);

	/* Check that objflag and objprop fields are of correct size */
	if (index[30].numrec != (uint32)objextsize(0))
		fatal("Object flag block not of the correct size.");

	if (index[31].numrec != (uint32)objextsize(1))
		fatal("Object property block not of the correct size.");

	objflag = (uchar *)read_recblock(NULL, FT_BYTE, index[30].numrec, index[30].file_offset,
	                                 index[30].blocksize);
	objprop = (long *)read_recblock(NULL, FT_INT32, index[31].numrec, index[31].file_offset,
	                                index[31].blocksize);

	oflag_cnt = index[32].numrec;
	attrtable = (attrdef_rec *)read_recarray(NULL, sizeof(attrdef_rec), index[32].numrec,
	            fi_attrrec, "Object Flag Table",
	            index[32].file_offset,
	            index[32].blocksize);
	/* Objflags are converted to internal form later, after
	   block 36 has been read in. */

	oprop_cnt = index[33].numrec;
	proptable = (propdef_rec *)read_recarray(NULL, sizeof(propdef_rec), index[33].numrec,
	            fi_proprec, "Object Property Table",
	            index[33].file_offset,
	            index[33].blocksize);

	if (filehead.ext_own != 'R' && filehead.fallback_ext <= 1) {
		/* Non-standard extension */
//    int i;
		for (i = 0; i < oflag_cnt; i++) /* These are converted later */
			attrtable[i].ystr = NULL;
		attrtable[i].nstr = NULL;
		for (i = 0; i < oprop_cnt; i++)
			proptable[i].str_cnt = 0;
		propstr_size = 0;
		propstr = NULL;
		vartable = NULL;
		flagtable = NULL;
	} else { /* Normal case */
		propstr_size = index[34].numrec;
		propstr = (const char **)read_recblock(NULL, FT_STR, index[34].numrec,
		                                       index[34].file_offset, index[34].blocksize);

		if (index[35].numrec && index[35].numrec != (uint32)VAR_NUM + 1)
			fatal("AGX file corrupted: variable itemization table size mismatch.");
		vartable = (vardef_rec *)read_recarray(NULL, sizeof(vardef_rec), index[35].numrec,
		                                       fi_varrec, "Variable Itemization Table",
		                                       index[35].file_offset,
		                                       index[35].blocksize);

		if (index[36].numrec && index[36].numrec != (uint32)FLAG_NUM + 1)
			fatal("AGX file corrupted: flag itemization table size mismatch.");
		flagtable = (flagdef_rec *)read_recarray(NULL, sizeof(flagdef_rec), index[36].numrec,
		            fi_flagrec, "Flag Itemization Table",
		            index[36].file_offset,
		            index[36].blocksize);
	}

	fix_objflag_str(1); /* Convert flags and objflags to internal form */


	/* Block 12: Command text */
	read_command(index[12].numrec, index[12].file_offset, diag);

	/* Block 11 is description block; it doesn't get read in by
	   agxread() but during play */
	if ((long)index[11].blocksize <= descr_maxmem) {
		/* ... if we decided to load descriptions into memory */
		mem_descr = (char *)read_recblock(NULL, FT_CHAR, index[11].numrec,
		                                  index[11].file_offset,
		                                  index[11].blocksize);
		buffclose(); /* Don't need to keep it open */
		descr_ofs = -1;
	} else {
		descr_ofs = index[11].file_offset;
		mem_descr = NULL;
	}
	reinit_dict();
	return 1;
}




/* ------------------------------------------------------------- */
/*        AGX Writing Code                                       */
/* ------------------------------------------------------------- */

static index_rec *gindex;


/* This patches the block descriptions to create AGiliTy-0.8
   compatible files. This is just a quick hack to solve a short-term
   problem. */
void patch_08(void) {
	set_endrec(fi_gameinfo, 48); /* Should give size of 83 */
	set_endrec(fi_noun, 45);
	set_endrec(fi_creat, 23);
}


/* This writes the file header; it needs to be called near the
   end */
void write_header(void) {
	int i;
	rbool simple;
	file_head_rec filehead;

	filehead.fileid = AGT_FILE_SIG;
	filehead.ver_own = filehead.ext_own = 'R';
	/* The following will be converted to 1-7 if advanced features aren't
	   being used. */
	filehead.version = 2;
	filehead.extnum = 2;
	filehead.fallback_ext = 2; /* 'R' extension to fall back to;
                  only meaningful if ext_own is *not* 'R' */
	filehead.eol_chk1 = '\n';
	filehead.eol_chk2 = '\r';
	filehead.res1 = 0;
	filehead.res2 = 0;

	/* This automatically patches the block descriptions to create
	   pre-AGiliTy-0.8.8 compatible files.  If it can't (because the
	   file uses 0.8.8+ features) then it leaves the version at 2;
	   otherwise the version is reduced to 1. */
	/* The files thus created are actually hybrid files-- they
	   have some 0.8.8+ features, just not the ones that might
	   break pre-0.8.8 interpreters. */
	simple = 1;
	for (i = 30; i < AGX_NUMBLOCK; i++)
		if (gindex[i].numrec != 0) simple = 0;
	if (simple) {
		gindex[0].numrec = 30; /* 0.8.7 compatibility */
		gindex[0].blocksize = gindex[0].recsize * gindex[0].numrec;
		filehead.version = 1;
		filehead.extnum = 7;
	}
	write_recarray(&filehead, sizeof(file_head_rec), 1, fi_header, 0);
}


static void agx_compute_index(void)
/* This computes the blocksize and offset values for all blocks */
{
	int i;

	for (i = 0; i < AGX_NUMBLOCK; i++)
		gindex[i].blocksize = gindex[i].recsize * gindex[i].numrec;
	gindex[0].file_offset = 16;
	gindex[11].file_offset = gindex[0].file_offset + gindex[0].blocksize;
	gindex[12].file_offset = gindex[11].file_offset + gindex[11].blocksize;
	gindex[1].file_offset = gindex[12].file_offset + gindex[12].blocksize;
	for (i = 2; i <= AGX_NUMBLOCK - 1; i++)
		if (i == 13)
			gindex[13].file_offset = gindex[10].file_offset + gindex[10].blocksize;
		else if (i != 11 && i != 12)
			gindex[i].file_offset = gindex[i - 1].file_offset + gindex[i - 1].blocksize;
}


/* Create the preliminary gindex for the new file and set it up so we can
 write descriptions to the new file */
void agx_create(fc_type fc) {
	int i;

	bw_open(fc, fAGX);
	gindex = (index_rec *)rmalloc(sizeof(index_rec) * AGX_NUMBLOCK);

	gindex[0].numrec = AGX_NUMBLOCK;
	for (i = 1; i < AGX_NUMBLOCK; i++) /* Initialize the rest to 0 */
		gindex[i].numrec = 0;

	/* This writes random data to the file; their only purpose
	   is to prevent problems with seeking beyond the end of file */
	write_recarray(NULL, sizeof(file_head_rec), 1, fi_header, 0);
	write_recarray(NULL, sizeof(index_rec), AGX_NUMBLOCK, fi_index, 16);

	old_base_verb = BASE_VERB; /* This will be constant for any given version
                  of the interpreter, but may change across
                  versions of the interpreter */
	/* Set record sizes */
	gindex[0].recsize = compute_recsize(fi_index);
	gindex[1].recsize = compute_recsize(fi_gameinfo);
	gindex[2].recsize = compute_recsize(fi_room);
	gindex[3].recsize = compute_recsize(fi_noun);
	gindex[4].recsize = compute_recsize(fi_creat);
	gindex[5].recsize = compute_recsize(fi_cmdhead);
	gindex[6].recsize = gindex[7].recsize = gindex[8].recsize =
	        gindex[9].recsize = compute_recsize(fi_descptr);
	gindex[10].recsize = ft_leng[FT_TLINE];
	gindex[11].recsize = ft_leng[FT_CHAR];
	gindex[12].recsize = ft_leng[FT_INT16];
	gindex[13].recsize = gindex[20].recsize = ft_leng[FT_CHAR];
	gindex[14].recsize = gindex[16].recsize = gindex[17].recsize =
	                         gindex[18].recsize = ft_leng[FT_WORD];
	gindex[15].recsize = ft_leng[FT_SLIST];
	gindex[19].recsize = ft_leng[FT_WORD];
	gindex[21].recsize = ft_leng[FT_DICTPTR];
	gindex[22].recsize = ft_leng[FT_BYTE];
	gindex[23].recsize = gindex[24].recsize = gindex[25].recsize =
	                         gindex[26].recsize = ft_leng[FT_STR];
	gindex[27].recsize = compute_recsize(fi_verbentry);
	gindex[28].recsize = ft_leng[FT_SLIST];
	gindex[29].recsize = ft_leng[FT_SLIST];
	gindex[30].recsize = ft_leng[FT_BYTE];
	gindex[31].recsize = ft_leng[FT_INT32];
	gindex[32].recsize = compute_recsize(fi_attrrec);
	gindex[33].recsize = compute_recsize(fi_proprec);
	gindex[34].recsize = ft_leng[FT_STR];
	gindex[35].recsize = compute_recsize(fi_varrec);
	gindex[36].recsize = compute_recsize(fi_flagrec);

	agx_compute_index(); /* Only the first 10 blocks will be correct */
	/* The important thing is to get the offset of block 11, the desciption
	   text block, so we can write to it. */
	/* Block 11 is the description block; it doesn't get written by agxwrite()
	   but by its own routines. */
}


static void agx_finish_index(void) {
	/* Still have 11, 27-29 */
	/* Block 12 is taken care of elsewhere (in write_command) */

	gindex[1].numrec = 1;
	gindex[2].numrec = rangefix(maxroom - first_room + 1);
	gindex[3].numrec = rangefix(maxnoun - first_noun + 1);
	gindex[4].numrec = rangefix(maxcreat - first_creat + 1);
	gindex[5].numrec = last_cmd;
	gindex[6].numrec = NUM_ERR;
	gindex[7].numrec = last_message;
	gindex[8].numrec = gindex[9].numrec = MaxQuestion;
	if (userstr != NULL)
		gindex[10].numrec = MAX_USTR;
	else gindex[10].numrec = 0;
	gindex[13].numrec = ss_end;
	gindex[14].numrec = MAX_SUB;
	gindex[15].numrec = TOTAL_VERB;
	gindex[16].numrec = maxpix;
	gindex[17].numrec = numglobal;
	gindex[19].numrec = synptr;
	gindex[20].numrec = dictstrptr;
	gindex[21].numrec = dp;
	gindex[23].numrec = maxpict;
	gindex[24].numrec = maxpix;
	gindex[25].numrec = maxfont;
	gindex[26].numrec = maxsong;
	gindex[27].numrec = vm_size;
	gindex[28].numrec = num_comb;
	gindex[29].numrec = num_prep;
	gindex[30].numrec = objextsize(0);
	gindex[31].numrec = objextsize(1);
	gindex[32].numrec = oflag_cnt;
	gindex[33].numrec = oprop_cnt;
	gindex[34].numrec = propstr_size;
	gindex[35].numrec = (vartable ?  VAR_NUM + 1 : 0);
	gindex[36].numrec = (flagtable ? FLAG_NUM + 1 : 0);

	/* These may also be zero (?) */
	gindex[22].numrec = have_opt ? 14 : 0;
	gindex[18].numrec = MAX_FLAG_NOUN;

	agx_compute_index(); /* This time it will be complete except for
            the VOC-TTL-INS blocks at the end */
}



/* The following routine writes a description to disk,
   and stores the size and length in dp */
void write_descr(descr_ptr *dp_, descr_line *txt) {
	long i;
	long size;
	char *buff, *buffptr, *src;

	size = 0;
	if (txt == NULL) {
		dp_->start = 0;
		dp_->size = 0;
		return;
	}

	for (i = 0; txt[i] != NULL; i++) /* Compute size */
		size += strlen(txt[i]) + 1; /* Remember trailing \0 */
	buff = (char *)rmalloc(sizeof(char) * size);

	buffptr = buff;
	for (i = 0; txt[i] != NULL; i++) {
		for (src = txt[i]; *src != 0; src++, buffptr++)
			*buffptr = *src;
		*buffptr++ = 0;
	}
	dp_->start = gindex[11].numrec;
	dp_->size = size;
	gindex[11].numrec +=
	    write_recblock(buff, FT_CHAR, size,
	                   gindex[11].file_offset + gindex[11].numrec);
	rfree(buff);
}

/* Write command text to file and return number of bytes written. */
static long write_command(long cmdofs) {
	long i, cnt;

	cmd_ptr = (long *)rmalloc(sizeof(long) * last_cmd);
	cnt = 0;
	for (i = 0; i < last_cmd; i++) {
		cmd_ptr[i] = cnt;
		write_recblock(command[i].data, FT_INT16, command[i].cmdsize,
		               cmdofs + 2 * cnt);
		cnt += command[i].cmdsize;
	}
	return cnt;
}




/* Write the bulk of the AGX file. This requires that the descriptions,
   etc. have already been written */
void agx_write(void) {
	gindex[11].blocksize = gindex[11].numrec * gindex[11].recsize;
	gindex[12].file_offset = gindex[11].file_offset + gindex[11].blocksize;

	gindex[12].numrec = write_command(gindex[12].file_offset);

	agx_finish_index();

	/* Need to write these blocks in order */

	write_globalrec(fi_gameinfo, gindex[1].file_offset);

	wset_roomdesc(fi_room);
	write_recarray(room, sizeof(room_rec), gindex[2].numrec,
	               fi_room, gindex[2].file_offset);

	wset_noundesc(fi_noun);
	write_recarray(noun, sizeof(noun_rec), gindex[3].numrec,
	               fi_noun, gindex[3].file_offset);

	wset_creatdesc(fi_creat);
	write_recarray(creature, sizeof(creat_rec), gindex[4].numrec,
	               fi_creat, gindex[4].file_offset);

	wset_cmdptr(fi_cmdhead);
	write_recarray(command, sizeof(cmd_rec), gindex[5].numrec,
	               fi_cmdhead, gindex[5].file_offset);

	write_recarray(err_ptr, sizeof(descr_ptr), gindex[6].numrec,
	               fi_descptr, gindex[6].file_offset);
	write_recarray(msg_ptr, sizeof(descr_ptr), gindex[7].numrec,
	               fi_descptr, gindex[7].file_offset);
	write_recarray(quest_ptr, sizeof(descr_ptr), gindex[8].numrec,
	               fi_descptr, gindex[8].file_offset);
	write_recarray(ans_ptr, sizeof(descr_ptr), gindex[9].numrec,
	               fi_descptr, gindex[9].file_offset);

	if (userstr != NULL)
		write_recarray(userstr, sizeof(tline), gindex[10].numrec,
		               fi_tline, gindex[10].file_offset);

	write_recblock(static_str, FT_CHAR,
	               gindex[13].numrec, gindex[13].file_offset);

	write_recblock(sub_name, FT_WORD, gindex[14].numrec, gindex[14].file_offset);
	write_recblock(synlist, FT_SLIST, gindex[15].numrec, gindex[15].file_offset);
	write_recblock(pix_name, FT_WORD, gindex[16].numrec, gindex[16].file_offset);
	write_recblock(globalnoun, FT_WORD, gindex[17].numrec, gindex[17].file_offset);
	write_recblock(flag_noun, FT_WORD, gindex[18].numrec, gindex[18].file_offset);
	write_recblock(syntbl, FT_WORD, gindex[19].numrec, gindex[19].file_offset);
	write_recblock(dictstr, FT_CHAR, gindex[20].numrec, gindex[20].file_offset);
	write_recblock(dict, FT_DICTPTR, gindex[21].numrec, gindex[21].file_offset);
	if (have_opt)
		write_recblock(opt_data, FT_BYTE, gindex[22].numrec, gindex[22].file_offset);

	write_recblock(pictlist, FT_STR, gindex[23].numrec, gindex[23].file_offset);
	write_recblock(pixlist, FT_STR, gindex[24].numrec, gindex[24].file_offset);
	write_recblock(fontlist, FT_STR, gindex[25].numrec, gindex[25].file_offset);
	write_recblock(songlist, FT_STR, gindex[26].numrec, gindex[26].file_offset);

	write_recarray(verbinfo, sizeof(verbentry_rec), gindex[27].numrec,
	               fi_verbentry, gindex[27].file_offset);
	write_recblock(comblist, FT_SLIST, gindex[28].numrec, gindex[28].file_offset);
	write_recblock(userprep, FT_SLIST, gindex[29].numrec, gindex[29].file_offset);
	write_recblock(objflag, FT_BYTE, gindex[30].numrec, gindex[30].file_offset);
	write_recblock(objprop, FT_INT32, gindex[31].numrec, gindex[31].file_offset);
	fix_objflag_str(0); /* Convert to external form */
	write_recarray(attrtable, sizeof(attrdef_rec),
	               gindex[32].numrec, fi_attrrec, gindex[32].file_offset);
	write_recarray(proptable, sizeof(propdef_rec),
	               gindex[33].numrec, fi_proprec, gindex[33].file_offset);
	write_recblock(propstr, FT_STR, gindex[34].numrec, gindex[34].file_offset);
	write_recarray(vartable, sizeof(vardef_rec),
	               gindex[35].numrec, fi_varrec, gindex[35].file_offset);
	write_recarray(flagtable, sizeof(flagdef_rec),
	               gindex[36].numrec, fi_flagrec, gindex[36].file_offset);
	fix_objflag_str(1); /* Restore to internal form */
}


/* Write header and master gindex and then close AGX file */
void agx_wclose(void) {
	write_header();
	write_recarray(gindex, sizeof(index_rec), AGX_NUMBLOCK, fi_index, 16);
	bw_close();
	rfree(gindex);
}


void agx_wabort(void) {
	bw_abort();
	rfree(gindex);
}

} // End of namespace AGT
} // End of namespace Glk
