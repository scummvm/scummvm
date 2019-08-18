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

/*
 * Defines TADS vocabulary (player command parser) functionality
 */

#ifndef GLK_TADS_TADS2_VOCABULARY
#define GLK_TADS_TADS2_VOCABULARY

#include "common/util.h"
#include "glk/tads/tads2/lib.h"
#include "glk/tads/tads2/object.h"
#include "glk/tads/tads2/property.h"
#include "glk/tads/tads2/run.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/*
 *   Cover macro for parser errors.  Any parser error should be covered
 *   with this macro for documentation and search purposes.  (The macro
 *   doesn't do anything - this is just something to search for when we're
 *   trying to enumerate parser error codes.) 
 */
#define VOCERR(errcode) errcode

/* maximum number of objects matching an ambiguous word */
#define VOCMAXAMBIG 200

/* size of input buffer */
#define VOCBUFSIZ 128

/*
 *   Vocabulary relation structure - this structure relates a vocabulary
 *   word to an object and part of speech.  A list of these structures is
 *   attached to each vocabulary word structure to provide the word's
 *   meanings.
 */
struct vocwdef {
    uint   vocwnxt;      /* index of next vocwdef attached to the same word */
    objnum vocwobj;                      /* object associated with the word */
    uchar  vocwtyp;   /* property associated with the word (part of speech) */
    uchar  vocwflg;                                   /* flags for the word */
#define VOCFCLASS  1                          /* word is for a class object */
#define VOCFINH    2                 /* word is inherited from a superclass */
#define VOCFNEW    4                          /* word was added at run-time */
#define VOCFDEL    8                               /* word has been deleted */
};

/* vocabulary word structure */
struct vocdef {
    vocdef *vocnxt;                         /* next word at same hash value */
    uchar   voclen;                                   /* length of the word */
    uchar   vocln2;          /* length of second word (0 if no second word) */
    uint    vocwlst;      /* head of list of vocwdef's attached to the word */
    uchar   voctxt[1];                                  /* text of the word */
};

/* vocabulary inheritance cell */
struct vocidef {
    uchar            vocinsc;   /* # of superclasses (gives size of record) */
    union {
        struct {
            uchar    vociusflg;                          /* flags for entry */
#define VOCIFCLASS  1  /* entry refers to a class object (loc records only) */
#define VOCIFVOC    2                 /* entry has vocabulary words defined */
#define VOCIFXLAT   4  /* superclasses must be translated from portable fmt */
#define VOCIFLOCNIL 8                  /* location is explicitly set to nil */
#define VOCIFNEW    16       /* object was allocated at run-time with "new" */
            objnum   vociusloc;                   /* location of the object */
            objnum   vociusilc;                       /* inherited location */
            objnum   vociussc[1];                  /* array of superclasses */
        } vocius;
        vocidef     *vociunxt;
    } vociu;
#define   vociflg vociu.vocius.vociusflg
#define   vociloc vociu.vocius.vociusloc
#define   vociilc vociu.vocius.vociusilc
#define   vocisc  vociu.vocius.vociussc
#define   vocinxt vociu.vociunxt
};

/* size of a page in a vocabulary pool */
#define VOCPGSIZ  8192

/* number of bytes in an inheritance cell page */
#define VOCISIZ 8192

/* maximum number of inheritance pages */
#define VOCIPGMAX 32

/* maximum number of inheritance pages (256 objects per page) */
#define VOCINHMAX 128

/* size of vocabulary hash table */
#define VOCHASHSIZ  256

/* size of a template structure */
#define VOCTPLSIZ 10

/* new-style template structure */
#define VOCTPL2SIZ  16


/*
 *   vocwdef's are fixed in size.  They're allocated in a set of arrays
 *   (the voccxwp member of the voc context has the list of arrays).  Each
 *   array is of a fixed number of vocwdef entries; a maximum number of
 *   vocwdef arrays is possible. 
 */
#define VOCWPGSIZ   2000                   /* number of vocwdef's per array */
#define VOCWPGMAX   16                  /* maximum number of vocwdef arrays */

/*
 *   To find a vocwdef entry given its index, divide the index by the
 *   number of entries per array to find the array number, and use the
 *   remainder to find the index within that array. 
 */
/*#define VOCW_IN_CACHE*/
#ifdef VOCW_IN_CACHE
vocwdef *vocwget(struct voccxdef *ctx, uint idx);
#else
#define vocwget(ctx, idx) \
    ((idx) == VOCCXW_NONE ? (vocwdef *)0 : \
      ((ctx)->voccxwp[(idx)/VOCWPGSIZ] + ((idx) % VOCWPGSIZ)))
#endif

/*
 *   Special values for vocdtim - these values indicate that the daemon
 *   does not have a normal turn-based expiration time. 
 */
#define VOCDTIM_EACH_TURN  0xffff            /* the daemon fires every turn */

/* daemon/fuse/alarm slot */
struct vocddef {
    objnum   vocdfn;             /* object number of function to be invoked */
    runsdef  vocdarg;                  /* argument for daemon/fuse function */
    prpnum   vocdprp;             /* property number (used only for alarms) */
    uint     vocdtim;  /* time for fuses/alarms (0xffff -> each-turn alarm) */
};

/* vocabulary object list entry */
struct vocoldef {
    objnum  vocolobj;                           /* object matching the word */
    const char *vocolfst;     /* first word in cmd[] that identified object */
    const char *vocollst;      /* last word in cmd[] that identified object */
    char   *vocolhlst;      /* hypothetical last word, if we trimmed a prep */
    int     vocolflg;                           /* special flags (ALL, etc) */
};

/* vocabulary context */
struct voccxdef {
    errcxdef  *voccxerr;                          /* error handling context */
    tiocxdef  *voccxtio;                                /* text i/o context */
    runcxdef  *voccxrun;                               /* execution context */
    mcmcxdef  *voccxmem;                          /* memory manager context */
    objucxdef *voccxundo;                                   /* undo context */
    uchar     *voccxpool;                  /* next free byte in vocdef pool */
    vocdef    *voccxfre;                        /* head of vocdef free list */
    char      *voccxcpp;                   /* pointer to compound word area */
    int        voccxcpl;                    /* length of compound word area */
    char      *voccxspp;                    /* pointer to special word area */
    int        voccxspl;                     /* length of special word area */
    uint       voccxrem;        /* number of bytes remaining in vocdef pool */
    vocidef  **voccxinh[VOCINHMAX];     /* vocidef page table: 256 per page */
    uchar     *voccxip[VOCIPGMAX];                 /* inheritance cell pool */
    vocidef   *voccxifr;              /* head of inheritance cell free list */
    uint       voccxiplst;          /* last inheritance cell page allocated */
    uint       voccxilst;      /* next unused byte in last inheritance page */
    int        voccxredo;                   /* flag: redo command in buffer */

    /* 
     *   redo buffer - if voccxredo is set, and this buffer is not empty,
     *   we'll redo the command in this buffer rather than the one in our
     *   internal stack buffer 
     */
    char       voccxredobuf[VOCBUFSIZ];

    /*
     *   "again" buffer - when we save the last command for repeating via
     *   the "again" command, we'll save the direct and indirect object
     *   words here, so that they can be recovered if "again" is used 
     */
    char       voccxagainbuf[VOCBUFSIZ];

    vocdef    *voccxhsh[VOCHASHSIZ];                          /* hash table */

#ifdef VOCW_IN_CACHE
    mcmon      voccxwp[VOCWPGMAX];        /* list of pages of vocab records */
    mcmon      voccxwplck;                  /* locked page of vocab records */
    vocwdef   *voccxwpgptr;             /* pointer to currently locked page */
#else
    vocwdef   *voccxwp[VOCWPGMAX];                  /* vocabulary word pool */
#endif

    uint       voccxwalocnt;             /* number of vocwdef's used so far */
    uint       voccxwfre;            /* index of first vocwdef in free list */
#define VOCCXW_NONE  ((uint)(-1))     /* index value indicating end of list */

    vocddef   *voccxdmn;                           /* array of daemon slots */
    uint       voccxdmc;                 /* number of slots in daemon array */
    vocddef   *voccxfus;                             /* array of fuse slots */
    uint       voccxfuc;                   /* number of slots in fuse array */
    vocddef   *voccxalm;                            /* array of alarm slots */
    uint       voccxalc;                  /* number of slots in alarm array */
    char       voccxtim[26];            /* game's timestamp (asctime value) */
    
    objnum     voccxvtk;                /* object number of "take" deepverb */
    objnum     voccxme;                      /* object number of "Me" actor */
    objnum     voccxme_init;                     /* initial setting of "Me" */
    objnum     voccxstr;                       /* object number of "strObj" */
    objnum     voccxnum;                       /* object number of "numObj" */
    objnum     voccxit;                                  /* last "it" value */
    objnum     voccxhim;                                /* last "him" value */
    objnum     voccxher;                                /* last "her" value */
    objnum     voccxthc;                   /* count of items in "them" list */
    objnum     voccxthm[VOCMAXAMBIG];            /* list of items in "them" */
    objnum     voccxprd;                 /* "pardon" function object number */
    objnum     voccxpre;               /* "preparse" function object number */
    objnum     voccxppc;            /* "preparseCmd" function object number */
    objnum     voccxpre2;           /* "preparseExt" function object number */
    objnum     voccxvag;                             /* "again" verb object */
    objnum     voccxini;                                 /* "init" function */
    objnum     voccxper;             /* "parseError" function object number */
    objnum     voccxprom;             /* "cmdPrompt" function object number */
    objnum     voccxpostprom;     /* "cmdPostPrompt" function object number */
    objnum     voccxpdis;                         /* parseDisambig function */
    objnum     voccxper2;                           /* parseError2 function */
    objnum     voccxperp;                       /* parseErrorParam function */
    objnum     voccxpdef;                          /* parseDefault function */
    objnum     voccxpdef2;                      /* parseDefaultExt function */
    objnum     voccxpask;                           /* parseAskobj function */
    objnum     voccxpask2;                     /* parseAskobjActor function */
    objnum     voccxpask3;                  /* parseAskobjIndirect function */
    objnum     voccxinitrestore;    /* "initRestore" function object number */
    objnum     voccxpuv;         /* parseUnknownVerb function object number */
    objnum     voccxpnp;          /* parseNounPhrase function object number */
    objnum     voccxpostact;           /* postAction function object number */
    objnum     voccxprecmd;            /* preCommand function object number */
    objnum     voccxendcmd;            /* endCommand function object number */

    /* current command word list values */
    vocoldef  *voccxdobj;                /* current direct object word list */
    vocoldef  *voccxiobj;              /* current indirect object word list */

    /* current command objects */
    objnum     voccxactor;                                 /* current actor */
    objnum     voccxverb;                       /* current command deepverb */
    objnum     voccxprep;                    /* current command preposition */
    
    /* previous command values - used by "again" */
    objnum     voccxlsa;                                  /* previous actor */
    objnum     voccxlsv;                                   /* previous verb */
    vocoldef   voccxlsd;                          /* previous direct object */
    vocoldef   voccxlsi;                        /* previous indirect object */
    objnum     voccxlsp;                                     /* preposition */
    int        voccxlssty;              /* style (new/old) of last template */
    uchar      voccxlst[VOCTPL2SIZ];                            /* template */

    objnum     voccxpreinit;                            /* preinit function */

    /* special flags */
    uchar      voccxflg;
#define VOCCXFCLEAR    1      /* ignore remainder of command line (restore) */
#define VOCCXFVWARN    2                /* generate redundant verb warnings */
#define VOCCXFDBG      4           /* debug mode:  show parsing information */
#define VOCCXAGAINDEL  8             /* "again" lost due to object deletion */

    /* number of remaining unresolved unknown words in the command */
    int        voccxunknown;

    /* total number of unresolved words in the last command */
    int        voccxlastunk;

    /* parser stack area */
    uchar *voc_stk_ptr;
    uchar *voc_stk_cur;
    uchar *voc_stk_end;
};

/* allocate and push a list, returning a pointer to the list's memory */
uchar *voc_push_list_siz(voccxdef *ctx, uint lstsiz);

/* push a list of objects from a vocoldef array */
void voc_push_vocoldef_list(voccxdef *ctx, vocoldef *objlist, int cnt);

/* push a list of objects from an objnum array */
void voc_push_objlist(voccxdef *ctx, objnum objlist[], int cnt);

/* change the player character ("Me") object */
void voc_set_me(voccxdef *ctx, objnum new_me);

/* add a vocabulary word */
void vocadd(voccxdef *ctx, prpnum p, objnum objn,
            int classflag, char *wrdval);

/* internal addword - must already be split into two words and lengths */
void vocadd2(voccxdef *ctx, prpnum p, objnum objn, int classflg,
             uchar *wrd1, int len1, uchar *wrd2, int len2);

/* delete vocabulary for a given object */
void vocdel(voccxdef *ctx, objnum objn);

/* lower-level vocabulary deletion routine */
void vocdel1(voccxdef *ctx, objnum objn, char *wrd, prpnum prp,
             int really_delete, int revert, int keep_undo);

/* delete all inherited vocabulary */
void vocdelinh(voccxdef *ctx);

/* allocate space for an inheritance record if needed */
void vocialo(voccxdef *ctx, objnum obj);

/* add an inheritance/location record */
void vociadd(voccxdef *ctx, objnum obj, objnum loc,
             int numsc, objnum *sc, int flags);

/* delete inheritance records for an object */
void vocidel(voccxdef *ctx, objnum chi);

/* renumber an object's inheritance records - used for 'modify' */
void vociren(voccxdef *ctx, objnum oldnum, objnum newnum);

/* caller-provided context structure for vocffw/vocfnw searches */
struct vocseadef {
    vocdef  *v;
    vocwdef *vw;
    const uchar *wrd1;
    int      len1;
    const uchar *wrd2;
    int      len2;
};

/* find first word matching a given word */
vocwdef *vocffw(voccxdef *ctx, const char *wrd, int len, const char *wrd2, int len2,
                int p, vocseadef *search_ctx);

/* find next word */
vocwdef *vocfnw(voccxdef *voccx, vocseadef *search_ctx);

/* read a line of input text */
int vocread(voccxdef *ctx, objnum actor, objnum verb,
            char *buf, int bufl, int type);
#define VOCREAD_OK    0
#define VOCREAD_REDO  1

/* compute size of a vocoldef list */
int voclistlen(vocoldef *lst);

/* tokenize an input buffer */
int voctok(voccxdef *ctx, char *cmd, char *outbuf,
           char **wrd, int lower, int cvt_ones, int show_errors);

/* get types for a word list */
int vocgtyp(voccxdef *ctx, char **cmd, int *types, char *orgbuf);

/* execute a player command */
int voccmd(voccxdef *ctx, char *cmd, uint cmdlen);

/* disambiguator */
int vocdisambig(voccxdef *ctx, vocoldef *outlist, vocoldef *inlist,
                prpnum defprop, prpnum accprop, prpnum verprop,
                char *cmd[], objnum otherobj, objnum cmdActor,
                objnum cmdVerb, objnum cmdPrep, char *cmdbuf,
                int silent);

/* display a multiple-object prefix */
void voc_multi_prefix(voccxdef *ctx, objnum objn,
                      int show_prefix, int multi_flags,
                      int cur_index, int count);

/* low-level executor */
int execmd(voccxdef *ctx, objnum actor, objnum prep,
           char *vverb, char *vprep, vocoldef *dolist, vocoldef *iolist,
           char **cmd, int *typelist,
           char *cmdbuf, int wrdcnt, uchar **preparse_list, int *next_start);

/* recursive command execution */
int execmd_recurs(voccxdef *ctx, objnum actor, objnum verb,
                  objnum dobj, objnum prep, objnum iobj,
                  int validate_dobj, int validate_iobj);

/* try running preparseCmd user function */
int try_preparse_cmd(voccxdef *ctx, char **cmd, int wrdcnt,
                     uchar **preparse_list);

/*
 *   Handle an unknown verb or sentence structure.  We'll call this when
 *   we encounter a sentence where we don't know the verb word, or we
 *   don't know the combination of verb and verb preposition, or we don't
 *   recognize the sentence structure (for example, an indirect object is
 *   present, but we don't have a template defined using an indirect
 *   object for the verb).
 *   
 *   'wrdcnt' is the number of words in the cmd[] array.  If wrdcnt is
 *   zero, we'll automatically count the array entries, with the end of
 *   the array indicated by a null pointer entry.
 *   
 *   If do_fuses is true, we'll execute the fuses and daemons if the
 *   function exists and doesn't throw an ABORT error, or any other
 *   run-time error other than EXIT.
 *   
 *   This function calls the game-defined function parseUnknownVerb, if it
 *   exists.  If the function doesn't exist, we'll simply display the
 *   given error message, using the normal parseError mechanism.  The
 *   function should use "abort" or "exit" if it wants to cancel further
 *   processing of the command.
 *   
 *   We'll return true if the function exists and executes successfully,
 *   in which case normal processing should continue with any remaining
 *   command on the command line.  We'll return false if the function
 *   doesn't exist or throws an error other than EXIT, in which case the
 *   remainder of the command should be aborted.  
 */
int try_unknown_verb(voccxdef *ctx, objnum actor,
                     char **cmd, int *typelist, int wrdcnt, int *next_start,
                     int do_fuses, int err, const char *msg, ...);

/* find a template */
int voctplfnd(voccxdef *ctx, objnum verb_in, objnum prep,
              uchar *tplout, int *newstyle);

/* build a printable name for an object from the words in a command list */
void voc_make_obj_name(voccxdef *ctx, char *namebuf, char *cmd[],
                       int firstwrd, int lastwrd);
void voc_make_obj_name_from_list(voccxdef *ctx, char *namebuf,
                                 char *cmd[], const char *firstwrd, const char *lastwrd);

/*
 *   check noun - determines whether the next set of words is a valid noun
 *   phrase.  No complaint is issued if not; this check is generally made
 *   to figure out what type of sentence we're dealing with.  This is
 *   simple; we just call vocgobj() with the complaint flag turned off.
 */
/* int vocchknoun(voccxdef *ctx, char **cmd, int *typelist, int cur,
                  int *next, vocoldef *nounlist, int chkact); */
#define vocchknoun(ctx, cmd, typelist, cur, next, nounlist, chkact) \
 vocgobj(ctx, cmd, typelist, cur, next, FALSE, nounlist, TRUE, chkact, 0)
#define vocchknoun2(ctx, cmd, typlst, cur, next, nounlist, chkact, nomatch) \
 vocgobj(ctx, cmd, typlst, cur, next, FALSE, nounlist, TRUE, chkact, nomatch)

/*
 *   get noun - reads an object list.  We simply call vocgobj() with the
 *   complaint and multiple-noun flags turned on.
 */
/* int vocgetnoun(voccxdef *ctx, char **cmd, int *typelist, int cur,
                  int *next, vocoldef *nounlist); */
#define vocgetnoun(ctx, cmd, typelist, cur, next, nounlist) \
 vocgobj(ctx, cmd, typelist, cur, next, TRUE, nounlist, TRUE, FALSE, 0)

/* get object */
int vocgobj(voccxdef *ctx, char **cmd, int *typelist, int cur,
            int *next, int complain, vocoldef *nounlist,
            int multi, int chkact, int *nomatch);

/* tokenize a string - TADS program code interface */
void voc_parse_tok(voccxdef *ctx);

/* get token types - TADS program code interface */
void voc_parse_types(voccxdef *ctx);

/* get objects matching all of the given words - TADS program code interface */
void voc_parse_dict_lookup(voccxdef *ctx);

/* parse a noun list - TADS program code interface */
void voc_parse_np(voccxdef *ctx);

/* disambiguate a noun list - TADS program code interface */
void voc_parse_disambig(voccxdef *ctx);

/* replace the current command - TADS program code interface */
void voc_parse_replace_cmd(voccxdef *ctx);

/* check access to an object */
int vocchkaccess(voccxdef *ctx, objnum obj, prpnum verprop,
                 int seqno, objnum actor, objnum verb);

/* check to see if an object is visible */
int vocchkvis(voccxdef *ctx, objnum obj, objnum cmdActor);

/* display an appropriate message for an unreachable object */
void vocnoreach(voccxdef *ctx, objnum *list1, int cnt,
                objnum actor, objnum verb, objnum prep, prpnum defprop,
                int show_multi_prefix, int multi_flags,
                int multi_base_index, int multi_total_count);

/* set {numObj | strObj}.value, as appropriate */
void vocsetobj(voccxdef *ctx, objnum obj, dattyp typ, const void *val,
               vocoldef *inobj, vocoldef *outobj);

/* macros to read values out of templates */
#define voctplpr(tpl) ((objnum)osrp2(((uchar *)tpl)))        /* preposition */
#define voctplvi(tpl) ((prpnum)osrp2(((uchar *)tpl) + 2))      /* verIoVerb */
#define voctplio(tpl) ((prpnum)osrp2(((uchar *)tpl) + 4))         /* ioVerb */
#define voctplvd(tpl) ((prpnum)osrp2(((uchar *)tpl) + 6))      /* verDoVerb */
#define voctpldo(tpl) ((prpnum)osrp2(((uchar *)tpl) + 8))         /* doVerb */
#define voctplflg(tpl) (*(((uchar *)tpl) + 10))                    /* flags */

/* flag values for the voctplflg */
#define VOCTPLFLG_DOBJ_FIRST   0x01     /* disambiguate direct object first */


/* word type flags */
#define VOCT_ARTICLE  1
#define VOCT_ADJ      2
#define VOCT_NOUN     4
#define VOCT_PREP     8
#define VOCT_VERB     16
#define VOCT_SPEC     32            /* special words - "of", ",", ".", etc. */
#define VOCT_PLURAL   64
#define VOCT_UNKNOWN  128                                /* word is unknown */

/* special type flags */
#define VOCS_ALL     1                                             /* "all" */
#define VOCS_EXCEPT  2                                          /* "except" */
#define VOCS_IT      4                                              /* "it" */
#define VOCS_THEM    8                                            /* "them" */
#define VOCS_NUM     16                                         /* a number */
#define VOCS_COUNT   32                   /* a number being used as a count */
#define VOCS_PLURAL  64                                           /* plural */
#define VOCS_ANY     128                                           /* "any" */
#define VOCS_HIM     256                                           /* "him" */
#define VOCS_HER     512                                           /* "her" */
#define VOCS_STR     1024                                /* a quoted string */
#define VOCS_UNKNOWN 2048           /* noun phrase contains an unknown word */
#define VOCS_ENDADJ  4096        /* word matched adjective at end of phrase */
#define VOCS_TRUNC   8192    /* truncated match - word is leading substring */
#define VOCS_TRIMPREP 16384 /* trimmed prep phrase: assumed it was for verb */

/* special internally-defined one-character word flags */
#define VOCW_AND     ','
#define VOCW_THEN    '.'
#define VOCW_OF      'O'
#define VOCW_ALL     'A'
#define VOCW_BOTH    'B'
#define VOCW_IT      'I'
#define VOCW_HIM     'M'
#define VOCW_ONE     'N'
#define VOCW_ONES    'P'
#define VOCW_HER     'R'
#define VOCW_THEM    'T'
#define VOCW_BUT     'X'
#define VOCW_ANY     'Y'

/* structure for special internal word table */
struct vocspdef {
    const char *vocspin;
    char  vocspout;
};

/* check if a word is a special word - true if word is given special word */
/* int vocspec(char *wordptr, int speccode); */
#define vocspec(w, s) (*(w) == (s))

/* 
 *   Set a fuse/daemon/notifier.
 */
void vocsetfd(voccxdef *ctx, vocddef *what, objnum func, prpnum prop,
              uint tm, runsdef *val, int err);

/* remove a fuse/daemon/notifier */
void vocremfd(voccxdef *ctx, vocddef *what, objnum func, prpnum prop,
              runsdef *val, int err);

/* count a turn (down all fuse/notifier timers) */
void vocturn(voccxdef *ctx, int turncnt, int do_fuses);

/* initialize voc context */
void vocini(voccxdef *vocctx, errcxdef *errctx, mcmcxdef *memctx,
            runcxdef *runctx, objucxdef *undoctx, int fuses,
            int daemons, int notifiers);

/* clean up the voc context - frees memory allocated by vocini() */
void vocterm(voccxdef *vocctx);

/* allocate/free fuse/daemon/notifier array for voc ctx initialization */
void vocinialo(voccxdef *ctx, vocddef **what, int cnt);
void voctermfree(vocddef *what);

/* get a vocidef given an object number */
/* vocidef *vocinh(voccxdef *ctx, objnum obj); */
#define vocinh(ctx, obj) ((ctx)->voccxinh[(obj) >> 8][(obj) & 255])

/* revert all objects back to original state, using inheritance records */
void vocrevert(voccxdef *ctx);

/* clear all fuses/daemons/notifiers (useful for restarting) */
void vocdmnclr(voccxdef *ctx);

/* display a parser error message */
void vocerr(voccxdef *ctx, int err, const char *f, ...);

/* 
 *   display a parser informational error message - this will display the
 *   message whether or not we're suppressing messages due to unknown
 *   words, and should be used when providing information, such as objects
 *   we're assuming by default 
 */
void vocerr_info(voccxdef *ctx, int err, const char *f, ...);

/* client undo callback - undoes a daemon/fuse/notifier */
void vocdundo(void *ctx, uchar *data);

/* client undo size figuring callback - return size of client undo record */
ushort OS_LOADDS vocdusz(void *ctx, uchar *data);

/* save undo for object creation */
void vocdusave_newobj(voccxdef *ctx, objnum objn);

/* save undo for adding a word */
void vocdusave_addwrd(voccxdef *ctx, objnum objn, prpnum typ, int flags,
                      char *wrd);

/* save undo for deleting a word */
void vocdusave_delwrd(voccxdef *ctx, objnum objn, prpnum typ, int flags,
                      char *wrd);

/* save undo for object deletion */
void vocdusave_delobj(voccxdef *ctx, objnum objn);

/* save undo for changing the "Me" object */
void vocdusave_me(voccxdef *ctx, objnum old_me);

/* compute vocabulary word hash value */
uint vochsh(const uchar *t, int len);

/* TADS versions of isalpha, isspace, isdigit, etc */
#define vocisupper(c) ((uchar)(c) <= 127 && Common::isUpper((uchar)(c)))
#define vocislower(c) ((uchar)(c) <= 127 && Common::isLower((uchar)(c)))
#define vocisalpha(c) ((uchar)(c) > 127 || Common::isAlpha((uchar)(c)))
#define vocisspace(c) ((uchar)(c) <= 127 && Common::isSpace((uchar)(c)))
#define vocisdigit(c) ((uchar)(c) <= 127 && Common::isDigit((uchar)(c)))


/*
 *   Undo types for voc subsystem 
 */
#define VOC_UNDO_DAEMON   1                    /* fuse/daemon status change */
#define VOC_UNDO_NEWOBJ   2                              /* object creation */
#define VOC_UNDO_DELOBJ   3                              /* object deletion */
#define VOC_UNDO_ADDVOC   4                  /* add vocabulary to an object */
#define VOC_UNDO_DELVOC   5             /* delete vocabulary from an object */
#define VOC_UNDO_SETME    6                          /* set the "Me" object */


/*
 *   Our own stack.  We need to allocate some fairly large structures
 *   (for the disambiguation lists, mostly) in a stack-like fashion, and
 *   we don't want to consume vast quantities of the real stack, because
 *   some machines have relatively restrictive limitations on stack usage.
 *   To provide some elbow room, we'll use a stack-like structure of our
 *   own: we'll allocate out of this structure as needed, and whenever we
 *   leave a C stack frame, we'll also leave our own stack frame. 
 */

/* re-initialize the stack, allocating space for it if needed */
void  voc_stk_ini(voccxdef *ctx, uint siz);

/* enter a stack frame, marking our current position */
#define voc_enter(ctx, marker)  (*(marker) = (ctx)->voc_stk_cur)

/* leave a stack frame, restoring the entry position */
#define voc_leave(ctx, marker)  ((ctx)->voc_stk_cur = marker)

/* return a value */
#define VOC_RETVAL(ctx, marker, retval) \
       voc_leave(ctx, marker); return retval

/* allocate space from the stack */
void *voc_stk_alo(voccxdef *ctx, uint siz);

/* allocation cover macros */
#define VOC_STK_ARRAY(ctx, typ, var, cnt) \
    (var = (typ *)voc_stk_alo(ctx, (uint)((cnt) * sizeof(typ))))

#define VOC_MAX_ARRAY(ctx, typ, var) \
    VOC_STK_ARRAY(ctx, typ, var, VOCMAXAMBIG)

/*
 *   Stack size for the vocab stack.  We'll scale our stack needs based
 *   on the size of the vocoldef structure, since this is the most common
 *   item to be allocated on the vocab stack.  We'll also scale based on
 *   the defined VOCMAXAMBIG parameter, since it is the number of elements
 *   usually allocated.  The actual amount of space needed depends on how
 *   the functions in vocab.c and execmd.c work, so this parameter may
 *   need to be adjusted for changes to the player command parser. 
 */
#define VOC_STACK_SIZE  (16 * VOCMAXAMBIG * sizeof(vocoldef))

/*
 *   Execute all fuses and daemons, then execute the endCommand user
 *   function.  Returns zero on success, or ERR_ABORT if 'abort' was
 *   thrown during execution.  This is a convenient cover single function
 *   to do all end-of-turn processing; this calls exefuse() and exedaem()
 *   as needed, trapping any 'abort' or 'exit' errors that occur.
 *   
 *   If 'do_fuses' is true, we'll run fuses and daemons.  Otherwise, 
 */
int exe_fuses_and_daemons(voccxdef *ctx, int err, int do_fuses,
                          objnum actor, objnum verb,
                          vocoldef *dobj_list, int do_cnt,
                          objnum prep, objnum iobj);

/*
 *   Execute any pending fuses.  Return TRUE if any fuses were executed,
 *   FALSE otherwise.  
 */
int exefuse(voccxdef *ctx, int do_run);

/*
 *   Execute daemons 
 */
void exedaem(voccxdef *ctx);

/*
 *   Get the number and size of words defined for an object.  The size
 *   returns the total byte count from all the words involved.  Do not
 *   include deleted words in the count.  
 */
void voc_count(voccxdef *ctx, objnum objn, prpnum prp, int *cnt, int *siz);

/*
 *   Iterate through all words for a particular object, calling a
 *   function with each vocwdef found.  If objn == MCMONINV, we'll call
 *   the callback for every word.  
 */
void voc_iterate(voccxdef *ctx, objnum objn,
                 void (*fn)(void *, vocdef *, vocwdef *), void *fnctx);

/* ------------------------------------------------------------------------ */
/*
 *   disambiguation status codes - used for disambigDobj and disambigIobj
 *   methods in the deepverb
 */

/* continue with disambiguation process (using possibly updated list) */
#define VOC_DISAMBIG_CONT     1

/* done - the list is fully resolved; return with (possibly updated) list */
#define VOC_DISAMBIG_DONE     2

/* error - abort the command */
#define VOC_DISAMBIG_ERROR    3

/* parse string returned in second element of list as interactive response */
#define VOC_DISAMBIG_PARSE_RESP  4

/* already asked for an interactive response, but didn't read it yet */
#define VOC_DISAMBIG_PROMPTED 5


/* ------------------------------------------------------------------------ */
/*
 *   parseNounPhrase status codes 
 */

/* parse error occurred */
#define VOC_PNP_ERROR    1

/* use built-in default parser */
#define VOC_PNP_DEFAULT  2

/* successful parse */
#define VOC_PNP_SUCCESS  3


/* ------------------------------------------------------------------------ */
/*
 *   parserResolveObjects usage codes 
 */
#define VOC_PRO_RESOLVE_DOBJ  1                            /* direct object */
#define VOC_PRO_RESOLVE_IOBJ  2                          /* indirect object */
#define VOC_PRO_RESOLVE_ACTOR 3                                    /* actor */

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
