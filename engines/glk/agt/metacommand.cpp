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

/* This contains the code for scanning and running metacommands.
   Note that while the code here deals with all of the flow-of-execution
   details, the code for executing individual tokens is actually
   in token.c (With a few exceptions for commands that impact
   the order of execution).  */

#define DEBUG_SCAN 1

#define MAX_REDIR 50000L  /* Maximum number of redirects, to catch
                infinite loops. If this is 0, allow infinitely
                many */

#define MAX_SUBCALL 2047  /* Maximum number of subroutine calls.
                 If this is 0, no limit (except for the 
                 program's stack size). */


/*

scan_metacommand
 -2=end of cycle, something happened (disambiguation only)
  0=end of this cycle   (disambig: end of cycle, nothing happened)
  1=end of all commands (disambig: end of cycle, nothing happened)
  2=end of turn         (disambig: nothing happened)

run_metacommand
    0 to go on to next metacommand,
    1 to stop running metacommands,  and
    2 to end the turn.
    3 indicates that redirection has just occured
    4 indicates a subcall has just occured.
    5 to go on to next metacommand after a return has occured.
    -2 means we're doing disambiguation and just hit an action token.

*/


/* ====================================================================*/
/*  RUN METACOMMAND: The following are the routines used to execute */
/*   a single metacommand block.  run_metacommand is invoked by */
/*   scan_metacommand, which is further down in this file.  */
/* ====================================================================*/

/* ------------------------------------------------------------------- */
/*  TYPE CHECKING ROUTINES        */
/*  Routines used to do type checking for metacommands. */
/* ------------------------------------------------------------------- */


rbool argvalid(int argtype, int arg) {
	if (argtype & AGT_VAR) { /* We have a variable */
		/* First, verify that arg actually indexes a variable */
		if (arg < 0 || arg > VAR_NUM) return 0; /* Nope */

		if (argtype == AGT_VAR) return 1; /* Pure variable; contents don't matter */

		/* Next, verify its contents, using the rest of this routine */
		arg = agt_var[arg];
		argtype &= ~AGT_VAR; /* Mask off AGT_VAR */
	}

	if (argtype < 128) {
		if (tnoun(arg)) return (argtype & AGT_ITEM) != 0;
		if (troom(arg)) return (argtype & AGT_ROOM) != 0;
		if (arg == 0) return (argtype & AGT_NONE) != 0;
		if (arg == 1) return (argtype & AGT_SELF) != 0;
		if (tcreat(arg)) return (argtype & AGT_CREAT) != 0;
		if (arg == 1000) return (argtype & AGT_WORN) != 0;
		return 0;
	} else switch (argtype) {
		case AGT_NUM:
			return 1;
		case AGT_DIR:
			return (arg >= 1 && arg <= 12);
		case AGT_FLAG:
			return (arg >= 0 && arg <= FLAG_NUM);
		case AGT_CNT:
			return (arg >= 0 && arg <= CNT_NUM);
		case AGT_QUEST:
			return (arg >= 1 && arg <= MaxQuestion);
		case AGT_MSG:
			return (arg >= 1 && arg <= last_message);
		case AGT_ERR:
			return (arg >= 1 && arg <= NUM_ERR);
		case AGT_STR:
			return (arg >= 1 && arg <= MAX_USTR);
		case AGT_SUB:
			return (arg >= 1 && arg <= MAX_SUB);
		case AGT_PIC:
			return (arg >= 1 && arg <= maxpict);
		case AGT_PIX:
			return (arg >= 1 && arg <= maxpix);
		case AGT_FONT:
			return (arg >= 1 && arg <= maxfont);
		case AGT_SONG:
			return (arg >= 1 && arg <= maxsong);
		case AGT_ROOMFLAG:
			return (arg >= 1 && arg <= 32);
		case AGT_EXIT:
			return (argvalid(AGT_ROOM | AGT_NONE, arg)
			        || argvalid(AGT_MSG, arg - exitmsg_base)
			        || (arg < 0 && aver >= AGX00)); /* Treat as verb */
		case AGT_OBJFLAG:
			return (arg >= 0 && arg < oflag_cnt);
		case AGT_OBJPROP:
			return (arg >= 0 && arg < oprop_cnt);
		case AGT_ATTR:  /* ATTR and PROP are type-checked elsewhere */
		case AGT_PROP:
			return 1;
		default:
			writeln("INTERNAL ERROR:Unrecognized type specifier.");
		}
	return 0;
}

/* <special> is set true for NOUN, OBJECT, NAME variables that are 0 */
/* (In this case, some error handling is suppressed) */

static rbool argfix(int argtype, int *arg, int optype, rbool *special) {
	*special = 0;
	switch (optype) {
	case 0:
		break;  /* Direct: The easy case */
	case 1:  /* Variable */
		if (*arg == -1) { /* Top-of-stack */
			*arg = pop_expr_stack();
			break;
		}
		if (!argvalid(AGT_VAR, *arg)) return 0;
		*arg = (int)agt_var[*arg];
		break;
	case 2:
		*arg = dobj;
		*special = (dobj == 0);
		break; /* NOUN */
	case 3:
		*arg = iobj;
		*special = (iobj == 0);
		break; /* OBJECT */
	default:
		rprintf("Internal error: Invalid optype.");
		return 0;
	}
	if (!(optype & 2)) {
		/* i.e. we have direct or variable type */
		/* The noun and object types below are useless for direct use,
		   but may be useful when used as values of variables. */
		if (argtype < 64) {
			if (*arg == -1) { /* NAME */
				*arg = actor;
				*special = (actor == 0);
			} else if (*arg == -2) { /* NOUN */
				*arg = dobj;
				*special = (dobj == 0);
			} else if (*arg == -3) { /* OBJECT */
				*arg = iobj;
				*special = (iobj == 0);
			}
		}
	}
	return argvalid(argtype, *arg);
}


/* These are handled in the order ARG2 then ARG1 so that
   top-of-stack references will pop the stack in that order
   (so that the push-order will corrospond to the argument order) */
/* <grammer_arg> is true if "bad" argument is NOUN/OBJECT/etc. and
   is 0. */
static int argok(const opdef *opdata, int *arg1, int *arg2, int optype,
                 rbool *grammer_arg) {
	if ((opdata->argnum) > 1 && !argfix(opdata->arg2, arg2, optype % 4, grammer_arg))
		return 0;
	if ((opdata->argnum) > 0 && !argfix(opdata->arg1, arg1, optype / 4, grammer_arg))
		return 0;
	return 1;
}

/* ------------------------------------------------------------------- */
/*  INSTRUCTION DECODING ROUTINES  */
/*  Routines for decoding opcodes and their arguments */
/* ------------------------------------------------------------------- */

static int decode_instr(op_rec *oprec, const integer *data, int maxleng) {
	integer op_;
	int optype;
	int leng;
	rbool special_arg1;  /* Is the first argument a special 0-length argument? */

	oprec->negate = oprec->failmsg = oprec->disambig = 0;
	oprec->errmsg = NULL;
	oprec->op = -1;
	oprec->opdata = &illegal_def;
	oprec->argcnt = 0;
	oprec->endor = 1;

	special_arg1 = 0;

	if (maxleng <= 0) {
		oprec->errmsg = "GAME ERROR: Unexpected end of token sequence.";
		return 1;
	}
	op_ = data[0];
	if (op_ < 0) {
		oprec->errmsg = "GAME ERROR: Negative token found.";
		return 1;
	}
	oprec->optype = optype = op_ / 2048; /* Split op_ into operand proper and optype */
	oprec->op = op_ = op_ % 2048;
	oprec->opdata = get_opdef(op_);

	if (oprec->opdata == &illegal_def) {
		if (op_ < START_ACT)
			oprec->errmsg = "GAME ERROR: Illegal condition token encountered.";
		else
			oprec->errmsg = "GAME ERROR: Illegal action token encountered.";
		return 1;
	}

	if (op_ < 1000) oprec->endor = 0; /* Conditional tokens don't end OR block */

	/* Recall that oprec->disambig is initialized to 0 */
	switch (op_) {
	case 89:
	case 95:
	case 96:
	case 97:
		oprec->disambig = 1;
		break; /* YesNo and Chance */
	case WIN_ACT:
	case WIN_ACT+1:
		oprec->disambig = 1;
		break; /* WinGame, EndGame */

	case 1037:
	case 1038: /* DoSubroutine, Return */
	case 1062:
	case 1115: /* RedirectTo, SetDisambigPriority */
	case 1132:            /* AND */
	case 1149:
	case 1150: /* Goto and OnFailGoto */
	case 1151:            /* EndDisambig */
	case 1152:            /* XRedirect */
		break;   /* Accept default of 0: these tokens don' trigger disambig */

	case 1135:
	case 1137:
	case 1138:
	case 1139:
	case 1140:
	case 1141:
	case 1142:
	case 1143:
	case 1147:
	case 1159:
		oprec->endor = 0;
		break;  /* Operations that only affect the stack don't
         stop disambiguation, either. They also
         don't mark the end of an OR block */

	default:
		/* Aside from the above exceptions, all actions will stop
		disambiguation (with success) and all conditions will let it
		 continue. */
		oprec->disambig = (op_ >= START_ACT && op_ < WIN_ACT);
	}

	if (op_ >= 1128 && op_ <= 1131) /* FailMessage group */
		oprec->failmsg = 1;

	leng = oprec->opdata->argnum + 1;
	if (optype != 0) { /* Correct leng for NOUN and OBJECT args */
		special_arg1 = ((optype & 8) == 8);
		leng -= special_arg1 + ((optype & 2) == 2);
		if (leng < 1) {
			oprec->errmsg = "GAME ERROR: Token list corrupted.";
			return 1;
		}
	}
	if (leng > maxleng) {
		oprec->errmsg = "GAME ERROR: Unexpected end of token sequence";
		return 1;
	}

	if (op_ == 108) { /* NOT */
		leng = 1 + decode_instr(oprec, data + 1, maxleng - 1);
		oprec->negate = !oprec->negate;
		return leng;
	}
	oprec->argcnt = leng - 1;
	oprec->arg1 = oprec->arg2 = 0;
	if (leng >= 2) {
		if (special_arg1) {
			assert(leng == 2);
			oprec->arg2 = data[1];
			oprec->arg1 = 0;
		} else oprec->arg1 = data[1];
	}
	if (leng >= 3) oprec->arg2 = data[2];
	if (leng >= 4) writeln("INTERNAL ERROR: Too many token arguments.");
	return leng;
}


/* decode_args checks and decodes the arguments to metacommand tokens */
/* Returns false on an error */
static rbool decode_args(int ip_, op_rec *oprec) {
	rbool grammer_arg; /* Have NOUN/OBJECT that is 0 and so failed argok tests */

	if (oprec->errmsg != NULL) {
		if (!PURE_ERROR)
			writeln(oprec->errmsg);
		return 0;
	}
	if (DEBUG_AGT_CMD && !supress_debug) {
		if (oprec->negate) { /* Output NOT */
			debug_cmd_out(ip_, 108, 0, 0, 0);
			ip_++;
		}
	}

	if (DEBUG_AGT_CMD && !supress_debug)
		debug_cmd_out(ip_, oprec->op, oprec->arg1, oprec->arg2, oprec->optype);

	/* This checks and translates the arguments */
	if (!argok(oprec->opdata, &(oprec->arg1), &(oprec->arg2),
	           oprec->optype, &grammer_arg)) {
		/* Don't report errors for null NOUN/OBJECT/ACTOR arguments
		   used in conditional tokens */
		if (grammer_arg && oprec->op <= MAX_COND)
			return 0;
		if (!PURE_ERROR) {
			if (DEBUG_AGT_CMD && !supress_debug) debugout("\n");
			writeln("GAME ERROR: Invalid argument to metacommand token.");
		}
		return 0;
	}
	return 1;
}




/* ------------------------------------------------------------------- */
/*  Subroutine Call Stack routines                                     */
/* ------------------------------------------------------------------- */
/* Note: run_metacommand() passes subroutine calls up to it's parent,
   but it processes Returns on its own (and is the routine responsible
   for maintaining the subcall stack--  scan_metacommand treats
   a subroutine call just like RedirecTo) */
/* The progression for subroutine calls goes like this:
    run_metacommand hits a DoSubroutine token;
      the subroutine id is saved in subcall_arg by exec_token.
    run_metacommand does push_subcall, saving cnum and ip,
      and then returns 4 to scan_metacommand.
    scan_metacommand saves grammar state to the new stack entry
      with push_subcall and then starts scanning SUBROUTINEnn

    Many tokens are executed.

    run_metacommand hits Return. It sets restart_state and
      returns 5 to its parent.
    scan_metacommand then runs pop_subcall_grammar and restores
      the original scanning grammer. It subtracts one from cnum
      so the original cnum will be rerun.
    run_metacommand sees that restart_state is set and pops the
      rest of the information (cnum and ip) off of the stack.
    Things continue as usual.
    */



typedef struct {
	/* run_metacommand state */
	short cnum, ip, failaddr;
	/* scan_metacommand state */
	integer mactor, mdobj, miobj;
	word mprep;
	short vcode;
	/* Global state (is this really saved?) */
	short vb;
	word prep;
} subcall_rec;


static subcall_rec *substack = NULL;
static short subcnt = 0;
static short subsize = 0;


static rbool push_subcall(int cnum, int ip_, int failaddr) {
	subcall_rec *savestack; /* In case something goes wrong. */

	if (MAX_SUBCALL != 0 && ++subcnt > MAX_SUBCALL)
		return 0;
	if (subcnt > subsize) {
		subsize += 5;
		savestack = substack;
		rm_trap = 0;
		substack = (subcall_rec *)rrealloc(substack, subsize * sizeof(subcall_rec));
		rm_trap = 1;
		if (substack == NULL) { /* out of memory */
			substack = savestack;
			return 0;
		}
	}
	substack[subcnt - 1].cnum = cnum;
	substack[subcnt - 1].ip = ip_;
	substack[subcnt - 1].failaddr = failaddr;
	return 1;
}


/* pop_subcall_grammar is called before this */
static void pop_subcall(int *rcnum, int *rip, int *rfailaddr) {
	assert(*rcnum == substack[subcnt - 1].cnum);
	/* *rcnum=substack[subcnt-1].cnum; */
	*rip = substack[subcnt - 1].ip;
	*rfailaddr = substack[subcnt - 1].failaddr;
	subcnt--;
}

/* This is called after push_subcall */
static void push_subcall_grammar(int m_actor, int vcode, int m_dobj, word m_prep,
                                 int m_iobj, int cnum) {
	/* run_metacommand should already have pushed cnum on the stack */
	substack[subcnt - 1].vb = vb;
	substack[subcnt - 1].prep = prep;
	substack[subcnt - 1].mactor = m_actor;
	substack[subcnt - 1].vcode = vcode;
	substack[subcnt - 1].mdobj = m_dobj;
	substack[subcnt - 1].mprep = m_prep;
	substack[subcnt - 1].miobj = m_iobj;
}

/* Return false if something goes wrong-- such as stack underflow. */
/* This is called *before* pop_subcall */
static rbool pop_subcall_grammar(integer *m_actor, int *vcode,
                                 integer *m_dobj, word *m_prep, integer *m_iobj,
                                 int *cnum) {
	if (subcnt == 0) return 0;
	vb = substack[subcnt - 1].vb;
	prep = substack[subcnt - 1].prep;
	*cnum = substack[subcnt - 1].cnum;
	*m_actor = substack[subcnt - 1].mactor;
	*vcode = substack[subcnt - 1].vcode;
	*m_dobj = substack[subcnt - 1].mdobj;
	*m_prep = substack[subcnt - 1].mprep;
	*m_iobj = substack[subcnt - 1].miobj;
	return 1;
}




/* ------------------------------------------------------------------- */
/*  Run Metacommand                                                    */
/* ------------------------------------------------------------------- */

static int run_metacommand(int cnum, int *redir_offset)
/* cnum=command number to run. */
/* *redir_offset=offset of redirect header, if we exit with redirection. */
/* Return
      0 to go on to next metacommand,
      1 to stop running metacommands,  and
      2 to end the turn.
      3 indicates that redirection has just occured
      4 indicates a subcall has just occured.
      5 Is used to go on to the next metacommand after a Return.
      -2 means we're doing disambiguation and just hit an action token. */
{
	int ip_, oip;  /* ip_=Instruction pointer, oip=Old instruction pointer */
	int r;        /* Used to hold return value from token execution */
	int fail_addr;  /* What address to jump to on failure */
	rbool fail;    /* Last token was a conditional token that failed */
	rbool ortrue, blocktrue, orflag; /* OR stuff
                     orflag: Are we in an OR group?
                     ortrue: Is current OR group true?
                     blocktrue: Is current block w/in OR true?
                     */
	static rbool restart = 0; /* Restarting after subroutine?  */
	op_rec currop;          /* Information on the current token and its args */

	fail_addr = 32000; /* Fall off the end when we fail */
	fail = 0;
	ip_ = 0;
	orflag = blocktrue = ortrue = 0;
	*redir_offset = 1;  /* Default: This is what RedirectTo does.
               Only XRedirect can send a different value */


	if (restart)  /* finish up Return from subroutine */
		pop_subcall(&cnum, &ip_, &fail_addr);

	if (DEBUG_AGT_CMD && !supress_debug) {
		debug_head(cnum);
		if (restart) debugout("   (Resuming after subroutine)\n");
	}

	restart = 0;


	/* ==========  Main Loop ================= */
	while (ip_ < command[cnum].cmdsize) {

		oip = ip_;
		ip_ += decode_instr(&currop, command[cnum].data + ip_, command[cnum].cmdsize - ip_);

		/* -------  OR Logic --------------- */
		if (currop.op == 109) { /* OR */
			if (!orflag) { /* First OR; set things up */
				orflag = 1;
				ortrue = 0;
				blocktrue = 1;
			}
			blocktrue = blocktrue && !fail; /* Was the previous token true? */
			fail = 0;
			ortrue = ortrue || blocktrue; /* OR in last block */
			blocktrue = 1; /* New block starts out true. */
		} else if (orflag) { /* we're in the middle of a block */
			blocktrue = blocktrue && !fail; /* Add in previous token */
			fail = 0;
			if (currop.endor) {  /* i.e. not a conditional token */
				orflag = 0;                /* End of OR block */
				ortrue = ortrue || blocktrue; /* OR in last block */
				fail = !ortrue; /* Success of whole group */
			}
		}

		/* ------------  FAILMESSAGE handling ------------- */
		if (currop.failmsg) {  /* Is the current token a Fail... token? */
			if (!fail) continue;  /* Skip it; look at next instruction */
			/* ErrMessage and ErrStdMessage: set disambiguation score */
			if (do_disambig) {
				if (currop.op == 1130 || currop.op == 1131) {
					if (!decode_args(oip, &currop)) return 2;
					disambig_score = currop.arg1;
					return 2;
				} else return -2; /* FailMessage counts as an action token */
			}
			/* Then run the failmessage, skipping the following step... */
		}
		/* -------- Failure routines -------------------- */
		else if (fail) {  /* ... and not failmessage */
			/* consequences of failure */
			fail = 0; /* In case fail_addr doesn't point off the edge of the world */
			ip_ = fail_addr;
			fail_addr = 32000; /* Reset fail_addr */
			continue; /* Usually fail_addr will fall off the end, causing this to
           return 0 */
		}

		/* - Finish decoding arguments and print out debugging message - */
		if (!decode_args(oip, &currop)) {
			if (currop.op < 1000) fail = currop.negate ? 0 : 1;
			continue;
			/* return 2;*/
		}

		/* -------- Commands that need to be handled specially -------------- */
		if (currop.op == 109) { /* OR */
			if (DEBUG_AGT_CMD && !supress_debug) debug_newline(op, 0);
			continue; /* OR: skip further processing */
		}

		if (currop.op == 1037) { /* DoSubroutine */
			if (!push_subcall(cnum, ip_, fail_addr)) {
				writeln("GAME ERROR: Subroutine stack overflow.");
				return 2;
			}
			subcall_arg = currop.arg1;
			if (DEBUG_AGT_CMD && !supress_debug) debugout("--> Call\n");
			return 4;
		}

		if (currop.op == 1038) { /* Return */
			restart = 1;
			if (DEBUG_AGT_CMD && !supress_debug) debugout("--> Return\n");
			return 5;
		}

		if (currop.op == 1149) { /* Goto */
			ip_ = currop.arg1;
			if (DEBUG_AGT_CMD && !supress_debug) debugout("\n");
			continue;
		}

		if (currop.op == 1150) { /* OnFailGoto */
			fail_addr = currop.arg1;
			if (DEBUG_AGT_CMD && !supress_debug) debugout("\n");
			continue;
		}

		if (currop.op == 1152) /* XRedirect */
			*redir_offset = currop.arg1;

		/* ---------- Disambiguation Success -------------- */
		if (do_disambig && currop.disambig) {
			if (DEBUG_AGT_CMD && !supress_debug) debugout("==> ACTION\n");
			return -2;
		}

		/* ---------- Run normal metacommands -------------- */
		switch (r = exec_instr(&currop)) {
		case 0:  /* Normal action token or successful conditional token */
			if (DEBUG_AGT_CMD && !supress_debug) debug_newline(op, 0);
			continue;
		case 1: /* Conditional token: fail */
			if (DEBUG_AGT_CMD && !supress_debug) {
				if (orflag) debugout("  (-->FAIL)\n");
				else debugout("--->FAIL\n");
			}
			fail = 1;
			continue;
		default: /* Return explicit value */
			if (DEBUG_AGT_CMD && !supress_debug) {
				if (r == 103) debugout("-->Redirect\n");
				else debugout("==> END\n");
			}
			return r - 100;
		}
	}
	return 0;
}



/* ====================================================================*/
/*  SCAN METACOMMAND: These are the routines that scan through the  */
/*    metacommand headers and find the appropriate ones to execute */
/*    Redirection is also handled at this level  */
/* ====================================================================*/


/* ------------------------------------------------------------------- */
/*  Support routines for extracting object information */
/* ------------------------------------------------------------------- */

/* For $ strings. Returns object number if there is one, or negative
   the dictionary index.
   This is used by the metacommand redirection routines */

static integer expand_redirect(word w) {
	assert(w != -1); /* <*NONE*> object shouldn't make it this far */
	if (w == 0 || aver < AGTME10) return -w;
	if (w == ext_code[wdverb]) return -syntbl[auxsyn[vb]];
	if (w == ext_code[wdnoun]) return dobj;
	if (w == ext_code[wdobject]) return iobj;
	if (w == ext_code[wdname]) return actor;
	if (w == ext_code[wdadjective]) return -it_adj(dobj);
	if (w == ext_code[wdprep]) return -prep;
	return -w;
}


static int extract_actor(int actnum) {
	if (actnum < 0) actnum = -actnum; /* Erase redirection stuff */
	if (tcreat(actnum)) return actnum;
	else return 0;
}

/* Basically, we need to find an object with a matching noun
   and adj to our choice. */
static int extract_obj(word name, word adj) {
	int i, obj;

	/* We just take the first one. We split this into separate noun and
	 creature loops for performance reaons */

	if (name == -1) /* <*NONE*> */
		return 0;

	obj = expand_redirect(name);
	adj = it_name(expand_redirect(adj));

	if (obj > 0) { /* $noun$, $object$, or $name$ */
		if (adj == 0 || adj == it_adj(obj))
			return obj; /* We're done */
		name = it_name(obj);
	} else
		name = -obj;

	if (adj == 0) return -name; /* Adjectives required for CLASS redirect */
	nounloop(i)
	if (noun[i].name == name && noun[i].adj == adj) return i + first_noun;
	creatloop(i)
	if (creature[i].name == name && creature[i].adj == adj)
		return i + first_creat;
	/* Hmm... just hope it's an internal noun. */
	writeln("GAME ERROR: Redirect statement with bad object name.");
	return -name;
}


/* ------------------------------------------------------------------- */
/*  Redirection Routines     */
/* ------------------------------------------------------------------- */


#define wordcode_fix(w) it_name(expand_redirect(w));

/* 'real_obj' below is the dobj_obj/iobj_obj field; it takes
   precedence over anything else if it is nonzero.
   It represents an *explicitly* declared object in
   the header */

static void fix_objnum(integer *objnum, word match,
                       int real_obj,
                       int actor_, int dobj_, int iobj_) {
	if (real_obj) *objnum = real_obj;
	else if (match == ext_code[wdobject]) *objnum = iobj_;
	else if (match == ext_code[wdnoun]) *objnum = dobj_;
	else if (match == ext_code[wdname]) *objnum = actor_;
}

/* Returns TRUE if we changed *objrec, FALSE otherwise */
/*  (This is needed for memory allocation purposes) */
static rbool fix_objrec(parse_rec **objrec, word match,
                        int real_obj,
                        parse_rec *actrec, parse_rec *dobjrec,
                        parse_rec *iobjrec) {
	if (real_obj) *objrec = make_parserec(real_obj, NULL);
	else if (match == ext_code[wdobject]) *objrec = copy_parserec(iobjrec);
	else if (match == ext_code[wdnoun]) *objrec = copy_parserec(dobjrec);
	else if (match == ext_code[wdname]) *objrec = copy_parserec(actrec);
	else return 0; /* *objrec unchanged */

	return 1;  /* *objrec changed */
}

static void objcode_fix(cmd_rec *cmd)
/* For $ strings. Fixes object redirection if neccessary */
{
	int actorword;
	word nounword, objword;
	int dobj_obj, iobj_obj;
	int savedobj, saveactor;
	parse_rec *savedrec, *saveactrec, *saveirec;
	rbool achange, dchange, ichange; /* Did the given _rec ptr change? */

	/* dobj_obj/iobj_obj take precedence over anything else */
	actorword = cmd->actor;
	nounword = cmd->nouncmd;
	objword = cmd->objcmd;
	dobj_obj = cmd->noun_obj;
	iobj_obj = cmd->obj_obj;

	/* Make temporary copies of things for when more than one thing is
	   being shuffled around; we don't need to save iobj since
	   it's processed last */
	saveactor = actor;
	saveactrec = actor_rec;
	savedobj = dobj;
	savedrec = dobj_rec;
	saveirec = iobj_rec; /* Saved only so it can be freed */

	/* Fix object numbers... */
	fix_objnum(&actor, actorword, 0, saveactor, savedobj, iobj);
	fix_objnum(&dobj, nounword, dobj_obj, saveactor, savedobj, iobj);
	fix_objnum(&iobj, objword, iobj_obj, saveactor, savedobj, iobj);

	/* ... and records */
	achange = fix_objrec(&actor_rec, actorword, 0, saveactrec, savedrec, iobj_rec);
	dchange = fix_objrec(&dobj_rec, nounword, dobj_obj, saveactrec, savedrec, iobj_rec);
	ichange = fix_objrec(&iobj_rec, objword, iobj_obj, saveactrec, savedrec, iobj_rec);

	/* Free up whatever needs freeing */
	if (achange) rfree(saveactrec);
	if (dchange) rfree(savedrec);
	if (ichange) rfree(saveirec);
}


/* Redirection is very superficial-- normally all it does is */
/* change the matching pattern, not the underlying objects */
/* The one exception is when we use the special redirection tokens */
/* NOUN or OBJECT */

void redirect_exec(cmd_rec *cmd, word *m_actor, int *vcode,
                   word *m_dobj, word *m_prep, word *m_iobj) {
	*m_actor = extract_actor(cmd->actor);
	vb = *vcode = verb_code(it_name(expand_redirect(cmd->verbcmd)));
	*m_dobj = extract_obj(cmd->nouncmd, cmd->noun_adj);
	if (cmd->prep == -1)
		*m_prep = 0;
	else
		*m_prep = it_name(expand_redirect(cmd->prep));
	*m_iobj = extract_obj(cmd->objcmd, cmd->obj_adj);

	/* This shuffles the _real_ objects if $noun$ forms are being
	   used */
	objcode_fix(cmd);
}




/* ------------------------------------------------------------------- */
/*  Scan Metacommand and the matching function it uses                 */
/* ------------------------------------------------------------------- */

/* This is used to match the elements of metacommand trigger patterns */
/* Sees if w2 matches COMMMAND pattern word w1; w1==0 corresponds to ANY */
#define cmatch(w1,w2) ((w1)==0 || (w1)==(w2) || ((w1)==-1 && (w2)==0))

static int cm_actor(int actnum, int actor_)
/* cmd: actnum,  player entry: actor_ */
{
	if (aver < AGX00) return 1; /* Bit of AGT brain-deadness. */
	if (actnum == 1) return actor_ == 0; /* No actor_: just the player  */
	if (tcreat(actnum))
		return (creat_fix[actor_ - first_creat] == creat_fix[actnum - first_creat]);
	if (actnum == 2) return (actor_ != 0); /* ANYBODY? */
	return (actor_ == 0);
}


/* Check that the explicit object matches */
static rbool cm_x_obj(int x_obj, int real_obj) {
	if (x_obj == 0) return 1; /* No explicit object; automatically match. */
	/* Explicit object case */
	/* In this case, we match against the _real_ object */
	/* However, we also require a "normal" match */
	do {
		if (x_obj == real_obj) return 1;
		real_obj = it_class(real_obj);
	} while (real_obj != 0);
	return 0;
}

/* Does [obj] match <adj> <noun> [x_obj]? */
/*  --[obj] must match up with <adj> <noun> */
/*  --If x_obj(the explicit object) is defined, it must match with
      the "real" object-- that is, the global dobj or iobj value. */
static rbool cm_obj(word name, word adj, int x_obj, int obj, int real_obj) {
	if (name == -1) return (obj == 0); /* <NONE> */

	if (x_obj && !cm_x_obj(x_obj, real_obj)) return 0;

	/* (Note that ANY does not match ALL) */
	if (obj == -ext_code[wall])
		return (name == ext_code[wall] && adj == 0);

	do {  /* Work our way up the class hierarchy */
		if (cmatch(name, it_name(obj)) && cmatch(adj, it_adj(obj)))
			return 1;
		obj = it_class(obj);
	} while (obj != 0);

	return 0;
}



static void scan_dbg(int vcode) {
	char buff[220];
	word w;

	if (vcode >= BASE_VERB && vcode < BASE_VERB + DUMB_VERB
	        && syntbl[synlist[vcode]] != 0)
		w = syntbl[synlist[vcode]];
	else w = syntbl[auxsyn[vcode]];

	if (strlen(dict[w]) > 200) return; /* Just in case... */
	sprintf(buff, "+++++Scanning %s\n", dict[w]);
	debugout(buff);
}

#define not_any(n,a) (n!=0 || a!=0)

/* This returns true if we redirect from VERB OBJ {PREP OBJ}
   to something that has fewer objects or no (explicit) preposition.
   This is less perfect than I would like since there is currently
   no way of distinguishing between ANY and an empty slot unless
   the new "NOMATCH" extension is used. */

static rbool redir_narrows_grammar(cmd_rec *cmd1, cmd_rec *cmd2) {
	/* Check inward from obj to prep to noun; if in any of these
	   fields cmd2 has ANY and cmd1 doesn't, return 1.
	   Stop as soon as we find a non-ANY field in either one. */

	/* If we *are* using the new extension, we can just use that info */
	if (cmd2->objcmd == -1) {
		if (cmd1->objcmd != -1) return 1;
		if (cmd1->prep == -1) {
			if (cmd1->prep != -1) return 1;
			if (cmd2->nouncmd == -1 && cmd1->objcmd != -1) return 1;
		}
	}
	if (nomatch_aware) return 0; /* If we are using nomatch, don't need
                  to go through the rest of this nonsense. */

	if (not_any(cmd2->objcmd, cmd2->obj_adj)) return 0;
	if (not_any(cmd1->objcmd, cmd1->obj_adj)) return 1;

	if (cmd2->prep != 0) return 0;
	if (cmd1->prep != 0) return 1;

	if (not_any(cmd2->nouncmd, cmd2->noun_adj)) return 0;
	if (not_any(cmd1->nouncmd, cmd1->noun_adj)) return 1;

	return 0; /* They are both all ANY. */
}



static rbool cm_command(cmd_rec *cmd,
                        integer m_actor, int m_verb,
                        integer m_dobj, word m_prep, integer m_iobj) {
	if (cmd->verbcmd == 0) { /* ANY */
		if (cmd->actor == 0 && aver >= AGX00)
			return (m_verb == 0); /* ANY command: rest of line ignored */
		/* Else ANY matchs; go on to test other things. */
	} else if (cmd->verbcmd != m_verb) return 0;

	return
	    cm_actor(cmd->actor, m_actor)
	    && cm_obj(cmd->nouncmd, cmd->noun_adj, cmd->noun_obj, m_dobj, dobj)
	    && cmatch(cmd->prep, m_prep)
	    && cm_obj(cmd->objcmd, cmd->obj_adj, cmd->obj_obj, m_iobj, iobj);
}



static void scan_for_actor(integer m_actor, int *start, int *end) {
	int i;

	assert(m_actor != 0);

	if (aver >= AGX00) {
		if (start != NULL) *start = verbptr[DIR_ADDR_CODE];
		*end = verbend[DIR_ADDR_CODE];
		return;
	}
	for (i = verbend[DIR_ADDR_CODE]; i > verbptr[DIR_ADDR_CODE]; i--)
		if (creat_fix[command[i].actor - first_creat]
		        == creat_fix[m_actor - first_creat]) {
			i++;
			break;
		}
	*end = i;

	if (start == NULL) return;

	for (i = verbptr[DIR_ADDR_CODE]; i <= *end; i++)
		if (creat_fix[command[i].actor - first_creat]
		        == creat_fix[m_actor - first_creat])
			break;
	*start = i;
}


/* m_<word> are the matching criterion; they have no *neccessary*
  connection to dobj, iobj, etc. */

int scan_metacommand(integer m_actor, int vcode,
                     integer m_dobj, word m_prep, integer m_iobj,
                     int *redir_flag)
/* Return codes:  0=end of this cycle, 1=end of all commands
   2=end of turn */
/* If doing disambiguation, then -2=end of cycle, something happened;
   0 or 1=end of cycle; nothing happened; 2=end of turn, nothing happened. */
/* If redir_flag is non-NULL, it is set when redirection occurs:
   1+=Redirection occured
   2=Grammar-changing redirection occured. */
{
	int i, oldi;
	word m_verb;
	int scanend;
	int redir_offset;   /* Used for multiple redirects in the same
             metacommand (which can occur in AGATE-style
             commands)-- this is used to hold the offset
             of the given redirect. */
	long redirect_count;  /* This is a safety measure: this keeps track of how
            many redirections have occured on a single turn, and
            if there are "too many" it will issue an error message
            and stop. This is to prevent the system from getting
            into a redirection loop. The number should be set
            high enough not to prevent deliberate loops,
            however. */

	rfree(substack);
	subcnt = 0;
	subsize = 0;
	redirect_count = 0;

	if (mars_fix)
		if (vcode == 0 || m_actor == 2) return 0;
	/* Don't explicity scan ANY metacommands if MARS fix is active. */
	if (m_actor == -ext_code[weverybody]) m_actor = 2;


	if (DEBUG_AGT_CMD && DEBUG_SCAN && !supress_debug) scan_dbg(vcode);

	m_verb = syntbl[auxsyn[vcode]];
	if (m_actor == 0) {
		i = verbptr[vcode];
		scanend = verbend[vcode];
	} else
		scan_for_actor(m_actor, &i, &scanend);
	for (; i < scanend; i++)
		if (command[i].actor < 0) {
			/* REDIRECT data; skip over it */;
		} else if (cm_command(&command[i], m_actor, m_verb, m_dobj, m_prep, m_iobj))
			switch (run_metacommand(i, &redir_offset)) {
			case -2:
				rfree(substack);
				return -2;
			/* We are doing disambiguation and reached
			   an action token */
			case 0:
			default:
				break; /* Go onto next metacommand */
			case 1:
				rfree(substack);
				return 1;  /* Done with metacommands */
			case 2:
				rfree(substack);
				return 2;  /* Done with turn */


			/* -------- REDIRECTION  ------------ */
			/* This handles RedirectTo tokens */
			case 3:
				oldi = i;
				i += redir_offset;
				if (i == last_cmd || command[i].actor > 0) {
					if (!PURE_ERROR) writeln("GAME ERROR: Invalid REDIRECT token.");
					rfree(substack);
					return 2;
				}
				if (MAX_REDIR != 0 && ++redirect_count > MAX_REDIR) {
					if (!PURE_ERROR) writeln("GAME ERROR: Infinite REDIRECT loop.");
					rfree(substack);
					return 2;
				}
				if (DEBUG_AGT_CMD && !supress_debug) {
					debugout("   ==>");
					debug_head(i);
				}

				/* REDIRECT :If we do a redirect from a broader grammar to a
				   narrower grammer, it will be noted so that certain types
				   of grammer checking can be disabled. */
				if (redir_flag != NULL) {
					if (*redir_flag < 2
					        && redir_narrows_grammar(&command[oldi], &command[i]))
						*redir_flag = 2;

					/* Set *redir_flag to at least 1 if we do *any* redirection. */
					if (!*redir_flag) *redir_flag = 1;
				}

				/* REDIRECT: Do the actual redirection, building the new command
				   header and shuffling around nouns and verbs as
				   neccessary */
				redirect_exec(&command[i], &m_actor, &vcode,
				              &m_dobj, &m_prep, &m_iobj);

				/* REDIRECT: Start scanning again from the beginning */
				if (!mars_fix) {/* In MARS, we *don't* go back to the top */
					if (m_actor != 0)
						scan_for_actor(m_actor, &i, &scanend);
					else {
						i = verbptr[vcode];
						scanend = verbend[vcode];
					}
					i--; /* Back up one so that the following i++ we'll
            be at the right location */
				}

				/* So when i is incremented, we start back at the correct start: i.e.
				   we start scanning again from the beginning. It's even possible
				   to use REDIRECT to run verb commands from an AFTER command,
				   although it precludes other AFTER commands from running. */
				m_verb = syntbl[auxsyn[vcode]];
				break;



			/* -------- SUBROUTINE CALL  ------------ */
			case 4:  /* Subroutine Call -- same idea as RedirectTo,
          but less complicated */
				push_subcall_grammar(m_actor, vcode, m_dobj, m_prep, m_iobj, i);
				vcode = verb_code(sub_name[subcall_arg - 1]);
				m_actor = m_dobj = m_iobj = 0;
				m_prep = 0;

				if (!mars_fix) /* In MARS, we *don't* go back to the top */
					i = verbptr[vcode] - 1;
				scanend = verbend[vcode];
				m_verb = syntbl[auxsyn[vcode]];
				break;


			/* -------- RETURN  ------------ */
			case 5: /* Return: pop grammar state, then ... ? */
				if (!pop_subcall_grammar(&m_actor, &vcode,
				                         &m_dobj, &m_prep, &m_iobj, &i)) {
					writeln("GAME ERROR: Return without DoSubroutine.");
					rfree(substack);
					return 2;
				}

				if (m_actor == 0)
					scanend = verbend[vcode];
				else
					scan_for_actor(m_actor, NULL, &scanend);
				m_verb = syntbl[auxsyn[vcode]];

				i--; /* Cause the last command to restart,
          at which point run_command will pop the rest of the
          stack. */

				break;
			}
	rfree(substack);
	return 0; /* Done with this cycle of metacommands */
}

/* ====================================================================*/

#undef cm

} // End of namespace AGT
} // End of namespace Glk
