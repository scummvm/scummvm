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


namespace Glk {
namespace AGT {

#ifndef global    /* Don't touch this */
#define global extern
#define global_defined_exec
#endif



/* This contains the decoding of the current instruction */
struct op_rec {
	integer op;
	int arg1;
	int arg2;
	int optype;
	int argcnt;  /* Actual number of argument words present */
	const opdef *opdata;
	const char *errmsg;
	rbool disambig; /* Trigger disambiguation? */
	rbool negate;  /* NOT? (cond token only) */
	rbool failmsg;  /* Run only on failure? */
	rbool endor;  /* End any OR blocks?  (action tokens, mainly) */
} ;



/* The following determines if we are doing disambiguation
   or actually executing a verb */
global uchar do_disambig;  /* 0= execution
                      1= disambiguating noun
                      2= disambiguating object */


/* Flags used during turn execution */
global rbool beforecmd;     /* Only used by 1.8x games */
global rbool supress_debug; /* Causes debugging info to _not_ be printed
                  even if debugging is on; used by disambiguator
                  and to supress ANY commands */
global rbool was_metaverb; /* Was the verb that just executed a metaverb? */
/* Metaverbs are commands that should not take game time
to execute: SAVE, RESTORE, RESTART, QUIT, SCRIPT, UNSCRIPT,
NOTIFY, SCORE, etc. */
global integer oldloc;  /* Save old location for NO_BLOCK_HOSTILE purposes */

/* This is a hack to pass the subroutine number from exec_token
   back to scan_metacommand when a DoSubroutine is done */
global integer subcall_arg;

/* This fixes a bug in the original AGT spec, causing "actor, verb ..."
   commands to misfire if there is more than one creature of the same
   name. */
global integer *creat_fix;


/* -------------------------------------------------------------------- */
/* Defined in EXEC.C                            */
/* -------------------------------------------------------------------- */
extern void raw_lineout(const char *s, rbool do_repl,
                        int context, const char *pword);
extern void msgout(int msgnum, rbool add_nl);
extern void sysmsg(int msgid, const char *s);
extern void alt_sysmsg(int msgid, const char *s, parse_rec *new_dobjrec,
	parse_rec *new_iobjrec);
extern void sysmsgd(int msgid, const char *s, parse_rec *new_dobj_rec);

rbool ask_question(int qnum);
extern void increment_turn(void);

/* Warning: the following function rfrees <ans> */
extern rbool match_answer(char *ans, int anum);

extern void look_room(void);
extern void runptr(int i, descr_ptr dp[], const char *msg, int msgid,
                   parse_rec *nounrec, parse_rec *objrec);

extern int normalize_time(int tnum); /* Convert hhmm so mm<60 */
extern void add_time(int dt);


/* -------------------------------------------------------------------- */
/* Defined in OBJECT.C                          */
/* -------------------------------------------------------------------- */
extern parse_rec *make_parserec(int obj, parse_rec *rec);
extern parse_rec *copy_parserec(parse_rec *rec);
extern void free_all_parserec(void); /* Freeds doj_rec, iobj_rec, and actor_rec */

extern rbool in_scope(int item);
extern rbool islit(void);
extern rbool it_possess(int item);
extern rbool it_proper(int item);
extern rbool it_isweapon(int item);
extern rbool it_door(int obj, word noun); /* Is obj a door? */
extern rbool is_within(integer obj1, integer obj2, rbool stop_if_closed);

extern integer it_room(int item); /* Returns the room that the item is in */

extern int lightcheck(int parent, int roomlight, rbool active);
/* If active is false, we don't care if the light is actually working. */

#define it_move(a,b) it_reposition(a,b,0)
#define it_destroy(item) it_move(item,0)
#define get_obj(dobj) it_move(dobj,1)
#define drop_obj(dobj) it_move(dobj,loc+first_room)

extern void it_reposition(int item, int newloc, rbool save_pos);
extern void goto_room(int newroom);

extern void it_describe(int dobj);
extern int print_contents(int obj, int ind_lev);

extern void recompute_score(void);

extern int check_fit(int obj1, int obj2);

/* And its possible return values: */

#define FIT_OK 0     /* Fits */
#define FIT_WEIGHT 1   /* Too heavy [*]  */
#define FIT_NETWEIGHT 2  /* With other stuff is too heavy [*] */
#define FIT_SIZE 3    /* Too big */
#define FIT_NETSIZE 4   /* With other stuff is too big */
/* [*]-- These can only occur if obj2==1 or for ME/1.5-1.7 */


extern long getprop(int obj, int prop);
extern void setprop(int obj, int prop, long val);
extern rbool getattr(int obj, int prop);
extern void setattr(int obj, int prop, rbool val);

extern rbool matchclass(int obj, int oclass);

/* ---------------------------------------------------------------------- */
/* Define in RUNVERB.C                                                    */
/* ---------------------------------------------------------------------- */

/* Verbs actually used elsewhere in th interpreter */
extern void v_inventory(void);
extern void v_look(void);
extern void v_listexit(void);

/* The routine that actually runs the current player command */
extern void exec_verb(void);


/* ---------------------------------------------------------------------- */
/* In METACOMMAND.C                               */
/* ---------------------------------------------------------------------- */
/* The main routine to search the metacommand list and run the appropriate
   meta-commands */
extern int scan_metacommand(integer m_actor, int vcode,
                            integer m_dobj, word m_prep, integer m_iobj,
                            int *redir_flag);

/* The type checking routine */
rbool argvalid(int argtype, int arg);

/* ---------------------------------------------------------------------- */
/* In TOKEN.C                                 */
/* ---------------------------------------------------------------------- */
extern int exec_instr(op_rec *oprec); /* Execute instruction */
extern long pop_expr_stack(void);  /* Wrapper around routine to access TOS */

/* ---------------------------------------------------------------------- */
/* Defined in DEBUGCMD.C                          */
/* ---------------------------------------------------------------------- */
extern void get_debugcmd(void);  /* Get and execute debugging commands */


/* -------------------------------------------------------------------  */
/* Macros for getting information about items               */
/* (mainly used to blackbox the difference between nouns and creatures) */
/* -------------------------------------------------------------------- */

/* A note on object codes:
       <0                 obj is a 'virtual' object, existing only as the word
                           dict[-obj], e.g. DOOR, flag nouns, global nouns
       0                  No object (or any object)
       1                  Self(i.e. the player)
   first_room..last_room  Rooms
   first_noun..last_noun  Nouns
   first_creat..last_creat Creatures
      1000                Being worn by the player          */


/* The following macro loops over the contents of an object */
#define contloop(i,obj)   for(i=it_contents(obj);i!=0;i=it_next(i))
#define safecontloop(i,j,obj) for(i=it_contents(obj),j=it_next(i); \
                                  i!=0;i=j,j=it_next(i))

#define cnt_val(c) ((c)==-1 ? 0 : (c))


/* -------------------------------------------------------------------- */
/* These are the macros that should usually be used to determine    */
/*  information about the objects in the game, unless the object type   */
/*  is definitely known                         */
/* -------------------------------------------------------------------  */

#define it_on(item) nounattr(item,on)
#define it_group(item) creatattr(item,groupmemb)
#define it_adj(item) objattr(item,adj)
#define it_pushable(item) nounattr(item,pushable)
#define it_pullable(item) nounattr(item,pullable)
#define it_turnable(item) nounattr(item,turnable)
#define it_playable(item) nounattr(item,playable)
#define it_plur(item) nounattr(item,plural)
#define it_gender(item) creatattr(item,gender)

#define it_pict(item) objattr(item,pict)
#define it_class(item) anyattr(item,oclass)
#define it_next(item) objattr(item,next)
#define it_isglobal(item) objattr(item,isglobal)
#define it_flagnum(item) objattr(item,flagnum)
#define it_seen(item) anyattr(item,seen)


#define it_name(item) objattr2(item,name,(item<0) ? -item : 0)
#define it_open(item) nounattr2(item,open, tcreat(item) || \
                                (tdoor(item) && !room[loc].locked_door))

/* This checks to make sure the object isn't unmovable. */
/* (As such, all non-nouns automatically pass) */
#define it_canmove(item) (!tnoun(item) || noun[(item)-first_noun].movable)


#ifdef IT_MACRO
#define it_contents(item) objattr2(item,contents,\
                                   roomattr2(item,contents,\
                                           (item==1) ? player_contents : \
                                           (item==1000) ? player_worn : 0))
#define it_lockable(item)  nounattr2(item,lockable, (tdoor(item) ? 1 : 0) )
#define it_locked(item,name) nounattr2(item,locked,\
                                       (tdoor(item) && room[loc].locked_door ? \
                                        1 : 0))
#else
extern int it_contents(integer obj);
extern rbool it_lockable(integer obj, word noun);
extern rbool it_locked(integer obj, word noun);
#endif


#ifdef global_defined_exec
#undef global
#undef global_defined_exec
#endif

} // End of namespace AGT
} // End of namespace Glk
