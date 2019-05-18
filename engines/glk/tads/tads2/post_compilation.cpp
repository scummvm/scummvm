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

#include "glk/tads/tads2/post_compilation.h"
#include "glk/tads/tads2/error.h"
#include "glk/tads/tads2/os.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/*
 *   Special character sequence description table for TADS language.
 *   Note that operators that start with common sequences must be grouped
 *   together, with the shorter sequences preceding the longer sequences.
 *   For example, ":" and ":=" must be adjacent, and ":" must precede
 *   ":=".  Other than this restriction, the order of tokens doesn't
 *   matter.  
 */
tokldef supsctab[] =
{
    { TOKTCOLON,   ":"  },
    { TOKTASSIGN,  ":=" },
    { TOKTLT,      "<"  },
    { TOKTLE,      "<=" },
    { TOKTSHL,     "<<" },
    { TOKTSHLEQ,   "<<="},
    { TOKTNE,      "<>" },
    { TOKTGT,      ">"  },
    { TOKTSHR,     ">>" },
    { TOKTSHREQ,   ">>="},
    { TOKTGE,      ">=" },
    { TOKTLPAR,    "("  },
    { TOKTRPAR,    ")"  },
    { TOKTPLUS,    "+"  },
    { TOKTINC,     "++" },
    { TOKTPLEQ,    "+=" },
    { TOKTMINUS,   "-"  },
    { TOKTPOINTER, "->" },
    { TOKTDEC,     "--" },
    { TOKTMINEQ,   "-=" },
    { TOKTDIV,     "/"  },
    { TOKTMOD,     "%"  },
    { TOKTMODEQ,   "%=" },
    { TOKTDIVEQ,   "/=" },
    { TOKTTIMES,   "*"  },
    { TOKTTIMEQ,   "*=" },
    { TOKTEQ,      "="  },
    { TOKTEQEQ,    "==" },
    { TOKTLBRACK,  "["  },
    { TOKTRBRACK,  "]"  },
    { TOKTLBRACE,  "{"  },
    { TOKTRBRACE,  "}"  },
    { TOKTSEM,     ";"  },
    { TOKTCOMMA,   ","  },
    { TOKTDOT,     "."  },
    { TOKTELLIPSIS,"..." },
    { TOKTPOUND,   "#"  },
    { TOKTBAND,    "&"  },
    { TOKTBANDEQ,  "&=" },
    { TOKTAND,     "&&" },
    { TOKTBOR,     "|"  },
    { TOKTBOREQ,   "|=" },
    { TOKTOR,      "||" },
    { TOKTQUESTION,"?"  },
    { TOKTDSTRING, "\"" },
    { TOKTSSTRING, "'"  },
    { TOKTNOT,     "!"  },
    { TOKTNE,      "!=" },
    { TOKTXOR,     "^"  },
    { TOKTXOREQ,   "^=" },
    { TOKTTILDE,   "~"  },
    { 0,           ""   }
};

typedef struct supkwdef supkwdef;
struct supkwdef
{
    char *supkwnam;
    int   supkwtok;
};

static supkwdef supkwtab[] =
{
    { "not", TOKTNOT },
    { "if", TOKTIF },
    { "else", TOKTELSE },
    { "while", TOKTWHILE },
    { "break", TOKTBREAK },
    { "continue", TOKTCONTINUE },
    { "exit", TOKTEXIT },
    { "abort", TOKTABORT },
    { "and", TOKTAND },
    { "or", TOKTOR },
    { "function", TOKTFUNCTION },
    { "return", TOKTRETURN },
    { "local", TOKTLOCAL },
    { "object", TOKTOBJECT },
    { "nil", TOKTNIL },
    { "true", TOKTTRUE },
    { "pass", TOKTPASS },
    { "askdo", TOKTASKDO },
    { "askio", TOKTASKIO },
    { "ioSynonym", TOKTIOSYN },
    { "doSynonym", TOKTDOSYN },
    { "external", TOKTEXTERN },
    { "formatstring", TOKTFORMAT },
    { "compoundWord", TOKTCOMPOUND },
    { "specialWords", TOKTSPECIAL },
    { "class", TOKTCLASS },
        
    /* new keywords for V2 */
    { "\002", 0 },                  /* special flag for start of v2 section */
    { "for", TOKTFOR },
    { "\001", 0 },                /* special flag that "do" is next keyword */
    { "do", TOKTDO },
    { "switch", TOKTSWITCH },
    { "case", TOKTCASE },
    { "default", TOKTDEFAULT },
    { "goto", TOKTGOTO },
    { "replace", TOKTREPLACE },
    { "modify", TOKTMODIFY },

    { "new", TOKTNEW },
    { "delete", TOKTDELETE },
    { (char *)0, 0 }
};

typedef struct supprdef supprdef;
struct supprdef
{
    char   *supprnam;
    prpnum  supprval;
};

static supprdef supprtab[] =
{
    { "verb", PRP_VERB },
    { "noun", PRP_NOUN },
    { "adjective", PRP_ADJ },
    { "preposition", PRP_PREP },
    { "article", PRP_ARTICLE },
    { "plural", PRP_PLURAL },

    /* add some more built-in properties */
    { "doAction", PRP_DOACTION },
    { "ioAction", PRP_IOACTION },
    { "sdesc", PRP_SDESC },
    { "thedesc", PRP_THEDESC },
    { "ioDefault", PRP_IODEFAULT },
    { "doDefault", PRP_DODEFAULT },
    { "location", PRP_LOCATION },
    { "value", PRP_VALUE },
    { "roomAction", PRP_ROOMACTION },
    { "actorAction", PRP_ACTORACTION },
    { "contents", PRP_CONTENTS },
    { "prepDefault", PRP_PREPDEFAULT },
    { "verActor", PRP_VERACTOR },
    { "validDo", PRP_VALIDDO },
    { "validIo", PRP_VALIDIO },
    { "lookAround", PRP_LOOKAROUND },
    { "roomCheck", PRP_ROOMCHECK },
    { "statusLine", PRP_STATUSLINE },
    { "locationOK", PRP_LOCOK },
    { "isVisible", PRP_ISVIS },
    { "cantReach", PRP_NOREACH },
    { "isHim", PRP_ISHIM },
    { "isHer", PRP_ISHER },
    { "action", PRP_ACTION },
    { "validDoList", PRP_VALDOLIST },
    { "validIoList", PRP_VALIOLIST },
    { "dobjGen", PRP_DOBJGEN },
    { "iobjGen", PRP_IOBJGEN },
    { "nilPrep", PRP_NILPREP },
    { "rejectMultiDobj", PRP_REJECTMDO },
    { "moveInto", PRP_MOVEINTO },
    { "construct", PRP_CONSTRUCT },
    { "destruct", PRP_DESTRUCT },
    { "validActor", PRP_VALIDACTOR },
    { "preferredActor", PRP_PREFACTOR },
    { "isEquivalent", PRP_ISEQUIV },
    { "adesc", PRP_ADESC },
    { "multisdesc", PRP_MULTISDESC },
    { "anyvalue", PRP_ANYVALUE },
    { "newNumbered", PRP_NEWNUMOBJ },
    { "parseUnknownDobj", PRP_PARSEUNKNOWNDOBJ },
    { "parseUnknownIobj", PRP_PARSEUNKNOWNIOBJ },
    { "dobjCheck", PRP_DOBJCHECK },
    { "iobjCheck", PRP_IOBJCHECK },
    { "verbAction", PRP_VERBACTION },
    { "disambigDobj", PRP_DISAMBIGDO },
    { "disambigIobj", PRP_DISAMBIGIO },
    { "prefixdesc", PRP_PREFIXDESC },
    { "isThem", PRP_ISTHEM },

    /* still more - TADS/Graphic properties */
    { "gp_picture", PRP_GP_PIC },
    { "gp_name", PRP_GP_NAME },
    { "gp_defverb", PRP_GP_DEFVERB },
    { "gp_active", PRP_GP_ACTIVE },
    { "gp_hotlist", PRP_GP_HOTLIST },
    { "gp_icon", PRP_GP_ICON },
    { "gp_defverb2", PRP_GP_DEFVERB2 },
    { "gp_defprep", PRP_GP_DEFPREP },
    { "gp_hotid", PRP_GP_HOTID },
    { "gp_overlay", PRP_GP_OVERLAY },
    { "gp_hotx", PRP_GP_HOTX },
    { "gp_hoty", PRP_GP_HOTY },

    /* flag end of list with null property name */
    { (char *)0, 0 }
};

/* define a built-in symbol */
static void supaddsym(toktdef *tab, char *txt, int styp, int sval,
                      int casefold)
{
    char buf[40];

    if (casefold)
    {
        strcpy(buf, txt);
        os_strlwr(buf);
        txt = buf;
    }
    (*tab->toktfadd)(tab, txt, (int)strlen(txt), styp, sval, tokhsh(txt));
}

/* add a built-in function to a symbol table */
static void supaddbi(void (*bif[])(bifcxdef *, int),
                     toktdef *tab, char *txt,
                     void (*fn)(bifcxdef *, int), int num, int casefold)
{
    supaddsym(tab, txt, TOKSTBIFN, num, casefold);
    bif[num] = fn;
}

/* set up reserved words: built-in functions and properties, keywords, etc */
void suprsrv(supcxdef *sup, void (*bif[])(bifcxdef *, int),
             toktdef *tab, int max, int v1compat, char *new_do,
             int casefold)
{
    supkwdef *kw;
    supbidef *p;
    int       i;
    supprdef *pr;
    extern    supbidef osfar_t supbitab[];
    int       do_kw = FALSE;
    char     *kwname;
    char      buf[40];

    /* add built-in functions */
    for (p = supbitab, i = 0 ; p->supbinam ; ++i, ++p)
    {
        if (i >= max) errsig(sup->supcxerr, ERR_MANYBIF);
        supaddbi(bif, tab, p->supbinam, p->supbifn, i, casefold);
    }

    /* add keywords */
    for (kw = supkwtab ; kw->supkwnam ; ++kw)
    {
        if (kw->supkwnam[0] == '\002')
        {
            if (v1compat) break;               /* no v2 keywords - quit now */
            else continue;          /* keep going, but skip this flag entry */
        }
        
        /* if this is the "do" keyword, change to user-supplied value */
        if (do_kw && new_do)
            kwname = new_do;
        else
            kwname = kw->supkwnam;
        
        if (kw->supkwnam[0] == '\001')
        {
            do_kw = TRUE;
            continue;
        }
        else
            do_kw = FALSE;

        if (casefold)
        {
            strcpy(buf, kwname);
            os_strlwr(buf);
            kwname = buf;
        }
        (*tab->toktfadd)(tab, kwname, (int)strlen(kwname),
                         TOKSTKW, kw->supkwtok, tokhsh(kwname));
    }

    /* add pseudo-variables */
    supaddsym(tab, "self", TOKSTSELF, 0, casefold);
    supaddsym(tab, "inherited", TOKSTINHERIT, 0, casefold);
    supaddsym(tab, "argcount", TOKSTARGC, 0, casefold);
    
    /* add built-in properties */
    for (pr = supprtab ; pr->supprnam ; ++pr)
        supaddsym(tab, pr->supprnam, TOKSTPROP, pr->supprval, casefold);
}

/* get name of an object out of symbol table */
void supgnam(char *buf, tokthdef *tab, objnum objn)
{
    toksdef sym;
    
    if (!tab)
    {
        strcpy(buf, "<NO SYMBOL TABLE>");
        return;
    }
    
    if (tokthfind((toktdef *)tab, TOKSTOBJ, (uint)objn, &sym)
        || tokthfind((toktdef *)tab, TOKSTFWDOBJ, (uint)objn, &sym))
    {
        memcpy(buf, sym.toksnam, (size_t)sym.tokslen);
        buf[sym.tokslen] = '\0';
        return;
    }

    strcpy(buf, "<UNKNOWN>");
}

/* set up inherited vocabulary */
void supivoc(supcxdef *ctx)
{
    vocidef ***vpg;
    vocidef  **v;
    voccxdef  *voc = ctx->supcxvoc;
    int        i;
    int        j;
    objnum     obj;
    
    /* delete all existing inherited words */
    vocdelinh(voc);
    
    for (vpg = voc->voccxinh, i = 0 ; i < VOCINHMAX ; ++vpg, ++i)
    {
        if (!*vpg) continue;                     /* no entries on this page */
        for (v = *vpg, obj = (i << 8), j = 0 ; j < 256 ; ++v, ++obj, ++j)
        {
            /* if it's not a class, inherit vocabulary for the object */
            if (!*v) continue;
            if (!((*v)->vociflg & VOCIFCLASS))
            {
                (*v)->vociilc = MCMONINV;      /* no inherited location yet */
                supivoc1(ctx, ctx->supcxvoc, *v, obj, FALSE, 0);
            }
        }
    }
}

/* find a single required object, by name */
static void supfind1(errcxdef *ec, toktdef *tab, char *nam, objnum *objp,
                     int required, int *errp, int warnlevel, int casefold)
{
    toksdef sym;
    int     namel = strlen(nam);
    char    buf[40];

    if (casefold)
    {
        strcpy(buf, nam);
        os_strlwr(buf);
        nam = buf;
    }
    if ((*tab->toktfsea)(tab, nam, namel, tokhsh(nam), &sym))
    {
        *objp = (objnum)sym.toksval;
    }
    else
    {
        if (required || warnlevel > 1)
            errlog1(ec, (required ? ERR_RQOBJNF : ERR_WRNONF),
                    ERRTSTR, errstr(ec, nam, namel));
        *objp = MCMONINV;
        if (required) *errp = 1;
    }
}

/* find required objects/functions */
void supfind(errcxdef *ec, tokthdef *htab, voccxdef *voc,
             objnum *preinit, int warnlevel, int cf)
{
    int      err = 0;
    toktdef *tab = &htab->tokthsc;

    /* look up the required and optional symbols */
    supfind1(ec, tab, "Me", &voc->voccxme, TRUE, &err, warnlevel, cf);
    supfind1(ec, tab, "takeVerb", &voc->voccxvtk, TRUE, &err, warnlevel, cf);
    supfind1(ec, tab, "strObj", &voc->voccxstr, TRUE, &err, warnlevel, cf);
    supfind1(ec, tab, "numObj", &voc->voccxnum, TRUE, &err, warnlevel, cf);
    supfind1(ec, tab, "pardon", &voc->voccxprd, TRUE, &err, warnlevel, cf);
    supfind1(ec, tab, "againVerb", &voc->voccxvag, TRUE, &err, warnlevel, cf);
    supfind1(ec, tab, "init", &voc->voccxini, TRUE, &err, warnlevel, cf);
    supfind1(ec, tab, "preinit", preinit, FALSE, &err, warnlevel, cf);
    supfind1(ec, tab, "preparse", &voc->voccxpre, FALSE, &err, warnlevel, cf);
    supfind1(ec, tab, "preparseExt", &voc->voccxpre2, FALSE, &err,
             warnlevel, cf);
    supfind1(ec, tab, "parseError", &voc->voccxper, FALSE, &err, warnlevel,
             cf);
    supfind1(ec, tab, "commandPrompt", &voc->voccxprom, FALSE, &err,
             warnlevel, cf);
    supfind1(ec, tab, "parseDisambig", &voc->voccxpdis, FALSE, &err,
             warnlevel, cf);
    supfind1(ec, tab, "parseError2", &voc->voccxper2, FALSE, &err, warnlevel,
             cf);
    supfind1(ec, tab, "parseDefault", &voc->voccxpdef, FALSE, &err, warnlevel,
             cf);
    supfind1(ec, tab, "parseDefaultExt", &voc->voccxpdef2, FALSE, &err,
             warnlevel, cf);
    supfind1(ec, tab, "parseAskobj", &voc->voccxpask, FALSE, &err, warnlevel,
             cf);
    supfind1(ec, tab, "preparseCmd", &voc->voccxppc, FALSE, &err, warnlevel,
             cf);
    supfind1(ec, tab, "parseAskobjActor", &voc->voccxpask2, FALSE,
             &err, warnlevel, cf);
    supfind1(ec, tab, "parseAskobjIndirect", &voc->voccxpask3, FALSE,
             &err, warnlevel, cf);
    supfind1(ec, tab, "parseErrorParam", &voc->voccxperp, FALSE, &err,
             warnlevel, cf);
    supfind1(ec, tab, "commandAfterRead", &voc->voccxpostprom, FALSE,
             &err, warnlevel, cf);
    supfind1(ec, tab, "initRestore", &voc->voccxinitrestore, FALSE,
             &err, warnlevel, cf);
    supfind1(ec, tab, "parseUnknownVerb", &voc->voccxpuv, FALSE,
             &err, warnlevel, cf);
    supfind1(ec, tab, "parseNounPhrase", &voc->voccxpnp, FALSE,
             &err, warnlevel, cf);
    supfind1(ec, tab, "postAction", &voc->voccxpostact, FALSE,
             &err, warnlevel, cf);
    supfind1(ec, tab, "endCommand", &voc->voccxendcmd, FALSE,
             &err, warnlevel, cf);
    supfind1(ec, tab, "preCommand", &voc->voccxprecmd, FALSE,
             &err, warnlevel, cf);

    /* "Me" is always the initial Me object */
    voc->voccxme_init = voc->voccxme;

    /* if we encountered any errors, signal the problem */
    if (err)
        errsig(ec, ERR_UNDEF);
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
