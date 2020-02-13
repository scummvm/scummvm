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

uchar DIAG, interp_arg, debug_da1, RAW_CMD_OUT;
int ERR_LEVEL;

rbool agx_file;
rbool have_opt;
rbool skip_descr;
rbool no_auxsyn;
rbool BATCH_MODE, make_test;
rbool have_meta;
rbool debug_mode, freeze_mode, milltime_mode, bold_mode;
uchar score_mode, statusmode;
rbool intro_first;
rbool box_title;
rbool mars_fix;
rbool fix_ascii_flag;
rbool dbg_nomsg;
rbool irun_mode;
rbool verboseflag;
int font_status;

rbool MASTERS_OR;
integer FLAG_NUM, CNT_NUM, VAR_NUM;
integer MAX_USTR;
integer MAX_SUB;
integer DVERB;
integer NUM_ERR;
integer num_rflags, num_nflags, num_cflags;
integer num_rprops, num_nprops, num_cprops;
integer oprop_cnt, oflag_cnt;
integer maxroom, maxnoun, maxcreat;
long MaxQuestion;
integer first_room, first_noun, first_creat, last_obj;
long last_message, last_cmd;
long numglobal;
long maxpict, maxpix, maxfont, maxsong;
long num_prep;
int num_auxcomb;
int num_comb;
integer exitmsg_base;
integer start_room, treas_room, resurrect_room, max_lives;
long max_score;
integer startup_time, delta_time;
int ver, aver;
long game_sig;
int vm_size;
int min_ver;
room_rec *room;
creat_rec *creature;
noun_rec *noun;
cmd_rec *command;
unsigned char *objflag;
long *objprop;
attrdef_rec *attrtable;
propdef_rec *proptable;
vardef_rec *vartable;
flagdef_rec *flagtable;
const char **propstr;
long propstr_size;
tline *userstr;
word *sub_name;
verbentry_rec *verbinfo;
short *verbptr, *verbend;
slist *synlist;
slist *comblist;
word *old_agt_verb;
slist *userprep;
word flag_noun[MAX_FLAG_NOUN], *globalnoun;
word pix_name[MAX_PIX];
filename *pictlist, *pixlist, *fontlist, *songlist;
uchar opt_data[14];
slist *auxsyn;
slist *preplist;
uchar *verbflag;
slist *auxcomb;

#ifdef PATH_SEP
char **gamepath;
#endif

rbool stable_random;
rbool DEBUG_MEM;
rbool debug_parse;
rbool DEBUG_EXEC_VERB;
rbool DEBUG_DISAMBIG;
rbool DEBUG_SMSG;
rbool debug_disambig, debug_any;
rbool DEBUG_OUT;
Common::DumpFile *debugfile;

rbool notify_flag, listexit_flag, menu_mode;
rbool cmd_saveable;
rbool can_undo;
uchar *restart_state, *undo_state;
char doing_restore;
rbool do_look;
rbool do_autoverb;
rbool script_on;
genfile scriptfile;
signed char logflag;
int logdelay;
genfile log_in, log_out;
rbool fast_replay;
rbool sound_on;
integer *pictable;
fc_type hold_fc;
unsigned short compass_rose;

rbool quitflag, deadflag, winflag, endflag;
rbool first_visit_flag, newlife_flag, room_firstdesc;
rbool start_of_turn;
rbool end_of_turn;
rbool actor_in_scope;
integer loc;
integer player_contents, player_worn;
long totwt, totsize;
integer curr_lives;
long tscore, old_score;
long objscore;
integer turncnt;
integer curr_time;
rbool *flag;
short *agt_counter;

#ifdef AGT_16BIT
short *agt_var;
#else
long *agt_var;
#endif

long agt_number;
rbool agt_answer;
tline l_stat, r_stat;
rbool nomatch_aware;
rbool smart_look;
int vm_width;
menuentry *verbmenu;

int vb;
integer actor, dobj, iobj;
parse_rec *actor_rec, *dobj_rec, *iobj_rec;
word prep;
parse_rec *curr_creat_rec;
int disambig_score;
word input[MAXINPUT];
words in_text[MAXINPUT];
short ip, ep;
short ap, vp, np, pp, op;
word ext_code[wabout + 1];
short last_he, last_she, last_it, last_they;

volatile int screen_width, status_width;
int screen_height;
volatile int curr_x;
rbool par_fill_on, center_on;
rbool textbold;

uchar trans_ascii[256];
rbool rm_acct;
long rfree_cnt, ralloc_cnt;
long rm_size, rm_freesize;
words *verblist;

/*-----------------------------------------------------------------*/

uchar do_disambig;
rbool beforecmd;
rbool supress_debug;
rbool was_metaverb;
integer oldloc;
integer subcall_arg;
integer *creat_fix;

/*-----------------------------------------------------------------*/

rbool *dbgflagptr;
long *dbgvarptr;
short *dbgcntptr;
rbool PURE_DOT;

} // End of namespace AGT
} // End of namespace Glk
