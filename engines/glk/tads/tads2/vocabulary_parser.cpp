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

#include "glk/tads/tads2/error.h"
#include "glk/tads/tads2/list.h"
#include "glk/tads/tads2/memory_cache_heap.h"
#include "glk/tads/tads2/os.h"
#include "glk/tads/tads2/run.h"
#include "glk/tads/tads2/vocabulary.h"

namespace Glk {
namespace TADS {
namespace TADS2 {


static const char *type_names[] =
{
    "article", "adj", "noun", "prep", "verb", "special", "plural",
    "unknown"
};

/* array of flag values for words by part of speech */
static int voctype[] =
{ 0, 0, VOCT_VERB, VOCT_NOUN, VOCT_ADJ, VOCT_PREP, VOCT_ARTICLE };

/* ------------------------------------------------------------------------ */
/*
 *   Allocate and push a list, given the number of bytes needed for the
 *   elements of the list.  
 */
uchar *voc_push_list_siz(voccxdef *ctx, uint lstsiz)
{
    runcxdef *rcx = ctx->voccxrun;
    runsdef val;
    uchar *lstp;

    /* add in the size needed for the list's length prefix */
    lstsiz += 2;

    /* allocate space in the heap */
    runhres(rcx, lstsiz, 0);

    /* set up a stack value to push */
    val.runstyp = DAT_LIST;
    val.runsv.runsvstr = lstp = ctx->voccxrun->runcxhp;

    /* set up the list's length prefix */
    oswp2(lstp, lstsiz);
    lstp += 2;

    /* commit the space in the heap */
    rcx->runcxhp += lstsiz;

    /* push the list value (repush, since we can use the original copy) */
    runrepush(rcx, &val);

    /* return the list element pointer */
    return lstp;
}

/*
 *   Allocate and push a list.  Returns a pointer to the space for the
 *   list's first element in the heap.  
 */
static uchar *voc_push_list(voccxdef *ctx, int ele_count, int ele_size)
{
    uint lstsiz;

    /* 
     *   Figure the list size - we need space for the given number of
     *   elements of the given size; in addition, each element requires
     *   one byte of overhead for its type prefix.  
     */
    lstsiz = (uint)(ele_count * (1 + ele_size));

    /* allocate and return the list */
    return voc_push_list_siz(ctx, lstsiz);
}

/*
 *   Push a list of numbers 
 */
static void voc_push_numlist(voccxdef *ctx, uint numlist[], int cnt)
{
    int i;
    uchar *lstp;

    /* allocate space for the list of numbers */
    lstp = voc_push_list(ctx, cnt, 4);

    /* enter the list elements */
    for (i = 0 ; i < cnt ; ++i)
    {
        /* add the type prefix */
        *lstp++ = DAT_NUMBER;

        /* add the value */
        oswp4(lstp, numlist[i]);
        lstp += 4;
    }
}

/*
 *   Push a list of object ID's obtained from a vocoldef array 
 */
void voc_push_vocoldef_list(voccxdef *ctx, vocoldef *objlist, int cnt)
{
    int i;
    uchar *lstp;
    uint lstsiz;

    /* 
     *   count the size - we need 3 bytes per object (1 for type plus 2
     *   for the value), and 1 byte per nil 
     */
    for (lstsiz = 0, i = 0 ; i < cnt ; ++i)
        lstsiz += (objlist[i].vocolobj == MCMONINV ? 1 : 3);

    /* allocate space for the list */
    lstp = voc_push_list_siz(ctx, lstsiz);

    /* enter the list elements */
    for (i = 0 ; i < cnt ; ++i)
    {
        if (objlist[i].vocolobj == MCMONINV)
        {
            /* store the nil */
            *lstp++ = DAT_NIL;
        }
        else
        {
            /* add the type prefix */
            *lstp++ = DAT_OBJECT;

            /* add the value */
            oswp2(lstp, objlist[i].vocolobj);
            lstp += 2;
        }
    }
}

/*
 *   Push a list of object ID's 
 */
void voc_push_objlist(voccxdef *ctx, objnum objlist[], int cnt)
{
    int i;
    uchar *lstp;
    uint lstsiz;

    /* 
     *   count the size - we need 3 bytes per object (1 for type plus 2
     *   for the value), and 1 byte per nil 
     */
    for (lstsiz = 0, i = 0 ; i < cnt ; ++i)
        lstsiz += (objlist[i] == MCMONINV ? 1 : 3);

    /* allocate space for the list */
    lstp = voc_push_list_siz(ctx, lstsiz);

    /* enter the list elements */
    for (i = 0 ; i < cnt ; ++i)
    {
        if (objlist[i] == MCMONINV)
        {
            /* store the nil */
            *lstp++ = DAT_NIL;
        }
        else
        {
            /* add the type prefix */
            *lstp++ = DAT_OBJECT;
            
            /* add the value */
            oswp2(lstp, objlist[i]);
            lstp += 2;
        }
    }
}

/*
 *   Push a list of strings, where the strings are stored in memory, one
 *   after the other, each string separated from the next with a null
 *   byte.  The list is bounded by firstwrd and lastwrd, inclusive of
 *   both.  
 */
static void voc_push_strlist(voccxdef *ctx, const char *firstwrd, const char *lastwrd)
{
    size_t curlen;
    const char *p;
    uint lstsiz;
    uchar *lstp;

    /*
     *   Determine how much space we need for the word list.  For each
     *   entry, we need one byte for the type prefix, two bytes for the
     *   length prefix, and the bytes of the string itself.  
     */
    for (lstsiz = 0, p = firstwrd ; p != 0 && p <= lastwrd ; p += curlen + 1)
    {
        curlen = strlen(p);
        lstsiz += curlen + (1+2);
    }

    /* allocate space for the word list */
    lstp = voc_push_list_siz(ctx, lstsiz);

    /* enter the list elements */
    for (p = firstwrd ; p != 0 && p <= lastwrd ; p += curlen + 1)
    {
        /* add the type prefix */
        *lstp++ = DAT_SSTRING;

        /* add the length prefix for this string */
        curlen = strlen(p);
        oswp2(lstp, curlen + 2);
        lstp += 2;

        /* add this string */
        memcpy(lstp, p, curlen);
        lstp += curlen;
    }
}

/*
 *   Push a list of strings, taking the strings from an array.  
 */
static void voc_push_strlist_arr(voccxdef *ctx, char *wordlist[], int cnt)
{
    int i;
    char **p;
    uint lstsiz;
    uchar *lstp;

    /* 
     *   Add up the lengths of the strings in the array.  For each
     *   element, we need space for the string's bytes, plus two bytes for
     *   the length prefix, plus one byte for the type prefix.  
     */
    for (lstsiz = 0, p = wordlist, i = 0 ; i < cnt ; ++i, ++p)
        lstsiz += strlen(*p) + 3;

    /* allocate space for the list */
    lstp = voc_push_list_siz(ctx, lstsiz);

    /* enter the list elements */
    for (p = wordlist, i = 0 ; i < cnt ; ++i, ++p)
    {
        size_t curlen;

        /* add the type prefix */
        *lstp++ = DAT_SSTRING;

        /* add the length prefix for this string */
        curlen = strlen(*p);
        oswp2(lstp, curlen + 2);
        lstp += 2;

        /* add this string */
        memcpy(lstp, *p, curlen);
        lstp += curlen;
    }
}

/*
 *   Push a list of strings, taking the strings from an array that was
 *   prepared by the parser tokenizer.  This is almost the same as pushing
 *   a regular string array, with the difference that we must recognize
 *   the special format that the tokenizer uses to store string tokens.  
 */
static void voc_push_toklist(voccxdef *ctx, char *wordlist[], int cnt)
{
    int i;
    char **p;
    uint lstsiz;
    uchar *lstp;
    size_t cur_len;

    /* 
     *   Add up the lengths of the strings in the array.  For each
     *   element, we need space for the string's bytes, plus two bytes for
     *   the length prefix, plus one byte for the type prefix.  
     */
    for (lstsiz = 0, p = wordlist, i = 0 ; i < cnt ; ++i, ++p)
    {
        /* 
         *   get the length of the current token - check what kind of
         *   token we have, since we must sense the length of different
         *   token types in different ways 
         */
        if (**p == '"')
        {
            /* 
             *   It's a string token - the string follows with a two-byte
             *   length prefix; add two bytes for the open and close quote
             *   characters that we'll add to the output string.  Note
             *   that we must deduct two bytes from the prefix length,
             *   because the prefix includes the size of the prefix
             *   itself, which we're not copying and will account for
             *   separately in the result string.
             */
            cur_len = osrp2(*p + 1) - 2 + 2;
        }
        else
        {
            /* for anything else, it's just a null-terminated string */
            cur_len = strlen(*p);
        }

        /* add the current length to the total so far */
        lstsiz += cur_len + 3;
    }

    /* allocate space for the list */
    lstp = voc_push_list_siz(ctx, lstsiz);

    /* enter the list elements */
    for (p = wordlist, i = 0 ; i < cnt ; ++i, ++p)
    {
        char *cur_ptr;
        size_t copy_len;
        
        /* add the type prefix */
        *lstp++ = DAT_SSTRING;

        /* get the information for the string based on the type */
        if (**p == '"')
        {
            /* 
             *   it's a string - use the length prefix (deducting two
             *   bytes for the prefix itself, which we're not copying) 
             */
            copy_len = osrp2(*p + 1) - 2;

            /* add space in the result for the open and close quotes */
            cur_len = copy_len + 2;

            /* the string itself follows the length prefix and '"' flag */
            cur_ptr = *p + 3;
        }
        else
        {
            /* for anything else, it's just a null-terminated string */
            cur_len = copy_len = strlen(*p);
            cur_ptr = *p;
        }

        /* write the length prefix for this string */
        oswp2(lstp, cur_len + 2);
        lstp += 2;

        /* add the open quote if this is a quoted string */
        if (**p == '"')
            *lstp++ = '"';

        /* add this string */
        memcpy(lstp, cur_ptr, copy_len);
        lstp += copy_len;

        /* add the close quote if it's a quoted string */
        if (**p == '"')
            *lstp++ = '"';
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   Read a command from the keyboard, doing all necessary output flushing
 *   and prompting.
 */
int vocread(voccxdef *ctx, objnum actor, objnum verb,
            char *buf, int bufl, int type)
{
    const char *prompt;
    int ret;

    /* presume we'll return success */
    ret = VOCREAD_OK;

    /* make sure output capturing is off */
    tiocapture(ctx->voccxtio, (mcmcxdef *)0, FALSE);
    tioclrcapture(ctx->voccxtio);

    /* 
     *   Clear out the command buffer.  This is important for the
     *   timeout-based command reader, since it will take what's in the
     *   buffer as the initial contents of the command line; this lets us
     *   remember any partial line that the player entered before a
     *   timeout interrupted their typing and redisplay the original
     *   partial line on the next command line.  Initially, there's no
     *   partial line, so clear it out.  
     */
    buf[0] = '\0';

    /* show the game-defined prompt, if appropriate */
    if (ctx->voccxprom != MCMONINV)
    {
        runpnum(ctx->voccxrun, (long)type);
        runfn(ctx->voccxrun, ctx->voccxprom, 1);
        tioflushn(ctx->voccxtio, 0);
        prompt = "";
    }
    else
    {
        /* there's no game-defined prompt - use our default */
        tioblank(tio);
        prompt = ">";
    }
    
    /* get a line of input */
    if (tiogets(ctx->voccxtio, prompt, buf, bufl))
        errsig(ctx->voccxerr, ERR_RUNQUIT);
    
    /* abort immediately if we see the special panic command */
    if (!strcmp(buf, "$$ABEND"))
    {
        /* make sure any script file is closed */
        qasclose();
        
        /* use the OS-level termination */
        os_term(OSEXFAIL);

        /* if that returned, signal a quit */
        errsig(ctx->voccxerr, ERR_RUNQUIT);
    }
    
    /* call the post-prompt function if defined */
    if (ctx->voccxpostprom != MCMONINV)
    {
        runpnum(ctx->voccxrun, (long)type);
        runfn(ctx->voccxrun, ctx->voccxpostprom, 1);
    }

    /* 
     *   If this isn't a type "0" input, and preparseExt() is defined, call
     *   it.  Don't call preparseExt() for type "0" inputs, since these will
     *   be handled via the conventional preparse().  
     */
    if (ctx->voccxpre2 != MCMONINV && type != 0)
    {
        uchar *s;
        size_t len;

        /* push the arguments - actor, verb, str, type */
        runpnum(ctx->voccxrun, (long)type);
        runpstr(ctx->voccxrun, buf, (int)strlen(buf), 0);
        runpobj(ctx->voccxrun, verb);
        runpobj(ctx->voccxrun, actor);

        /* call preparseExt() */
        runfn(ctx->voccxrun, ctx->voccxpre2, 4);

        /* check the result */
        switch(runtostyp(ctx->voccxrun))
        {
        case DAT_SSTRING:
            /* 
             *   They returned a string.  Replace the input buffer we read
             *   with the new string.  Pop the new string and scan its length
             *   prefix.  
             */
            s = runpopstr(ctx->voccxrun);
            len = osrp2(s) - 2;
            s += 2;

            /* 
             *   limit the size we copy to our buffer length (leaving space
             *   for null termination) 
             */
            if (len > (size_t)bufl - 1)
                len = bufl - 1;

            /* copy the new command string into our buffer */
            memcpy(buf, s, len);

            /* null-terminate the result */
            buf[len] = '\0';

            /* proceed as normal with the new string */
            break;

        case DAT_TRUE:
            /* 
             *   they simply want to keep the current string as it is -
             *   proceed as normal 
             */
            break;

        case DAT_NIL:
            /* 
             *   They want to skip the special interpretation of the input
             *   and proceed directly to treating the input as a brand new
             *   command.  The caller will have to take care of the details;
             *   we need only indicate this to the caller through our "redo"
             *   result code.  
             */
            ret = VOCREAD_REDO;
            break;
        }
    }

    /* return our result */
    return ret;
}

/*
 *   Compare a pair of words, truncated to six characters or the
 *   length of the first word, whichever is longer.  (The first word is
 *   the user's entry, the second is the reference word in the dictionary.)
 *   Returns TRUE if the words match, FALSE otherwise.
 */
static int voceq(const uchar *s1, uint l1, uchar *s2, uint l2)
{
    uint i;

    if (l1 == 0 && l2 == 0)  return(TRUE);           /* both NULL - a match */
    if (l1 == 0 || l2 == 0)  return(FALSE);  /* one NULL only - not a match */
    if (l1 >= 6 && l2 >= l1) l2 = l1;
    if (l1 != l2)            return(FALSE);                /* ==> not equal */
    for (i = 0 ; i < l1 ; i++)
        if (*s1++ != *s2++)  return(FALSE);
    return(TRUE);                                          /* strings match */
}

/* find the next word in a search */
vocwdef *vocfnw(voccxdef *voccx, vocseadef *search_ctx)
{
    vocdef  *v, *vf;
    vocwdef *vw, *vwf = nullptr;
    vocdef  *c = search_ctx->v;
    int      first;

    /* continue with current word's vocwdef list if anything is left */
    first = TRUE;
    vw = vocwget(voccx, search_ctx->vw->vocwnxt);

    /* keep going until we run out of hash chain entries or find a match */
    for (v = c, vf = 0 ; v != 0 && vf == 0 ; v = v->vocnxt, first = FALSE)
    {
        /* if this word matches, look at the objects in its list */
        if (first
            || (voceq(search_ctx->wrd1, search_ctx->len1,
                      v->voctxt, v->voclen)
                && voceq(search_ctx->wrd2, search_ctx->len2,
                         v->voctxt + v->voclen, v->vocln2)))
        {
            /*
             *   on the first time through, vw has already been set up
             *   with the next vocwdef in the current list; on subsequent
             *   times through the loop, start at the head of the current
             *   word's list 
             */
            if (!first)
                vw = vocwget(voccx, v->vocwlst);

            /* search the list from vw forward */
            for ( ; vw ; vw = vocwget(voccx, vw->vocwnxt))
            {
                if (search_ctx->vw->vocwtyp == vw->vocwtyp
                    && !(vw->vocwflg & VOCFCLASS)
                    && !(vw->vocwflg & VOCFDEL))
                {
                    /*
                     *   remember the first vocdef that we found, and
                     *   remember this, the first matching vocwdef, then
                     *   stop scanning 
                     */
                    vf = v;
                    vwf = vw;
                    break;
                }
            }
        }
    }

    /* return the first vocwdef in this word's list */
    search_ctx->v = vf;
    search_ctx->vw = (vf ? vwf : 0);
    return(search_ctx->vw);
}

/* find the first vocdef matching a set of words */
vocwdef *vocffw(voccxdef *ctx, const char *wrd, int len, const char *wrd2, int len2,
                int p, vocseadef *search_ctx)
{
    uint     hshval;
    vocdef  *v, *vf;
    vocwdef *vw, *vwf = nullptr;

    /* get the word's hash value */
    hshval = vochsh((const uchar *)wrd, len);

    /* scan the hash list until we run out of entries, or find a match */
    for (v = ctx->voccxhsh[hshval], vf = 0 ; v != 0 && vf == 0 ;
         v = v->vocnxt)
    {
        /* if this word matches, look at the objects in its list */
        if (voceq((const uchar *)wrd, len, v->voctxt, v->voclen)
            && voceq((const uchar *)wrd2, len2, v->voctxt + v->voclen, v->vocln2))
        {
            /* look for a suitable object in the vocwdef list */
            for (vw = vocwget(ctx, v->vocwlst) ; vw ;
                 vw = vocwget(ctx, vw->vocwnxt))
            {
                if (vw->vocwtyp == p && !(vw->vocwflg & VOCFCLASS)
                    && !(vw->vocwflg & VOCFDEL))
                {
                    /*
                     *   remember the first vocdef that we found, and
                     *   remember this, the first matching vocwdef; then
                     *   stop scanning, since we have a match 
                     */
                    vf = v;
                    vwf = vw;
                    break;
                }
            }
        }
    }

    /* set up the caller-provided search structure for next time */
    vw = (vf != 0 ? vwf : 0);
    if (search_ctx)
    {
        /* save the search position */
        search_ctx->v = vf;
        search_ctx->vw = vw;

        /* save the search criteria */
        search_ctx->wrd1 = (const uchar *)wrd;
        search_ctx->len1 = len;
        search_ctx->wrd2 = (const uchar *)wrd2;
        search_ctx->len2 = len2;
    }

    /* return the match */
    return vw;
}

/* ------------------------------------------------------------------------ */
/* 
 *   vocerr_va information structure.  This is initialized in the call to
 *   vocerr_va_prep(), and must then be passed to vocerr_va().  
 */
struct vocerr_va_info
{
    /* parseError/parseErrorParam result */
    char user_msg[400];

    /* the sprintf-style format string to display */
    const char *fmt;

    /* 
     *   Pointer to the output buffer to use to format the string 'fmt' with
     *   its arguments, using vsprintf.  The prep function will set this up
     *   to point to user_msg[].  
     */
    char *outp;

    /* size of the output buffer, in bytes */
    size_t outsiz;
};

/*
 *   General parser error formatter - preparation.  This must be called to
 *   initialize the context before the message can be displayed with
 *   vocerr_va().  
 */
static void vocerr_va_prep(voccxdef *ctx, struct vocerr_va_info *info,
                           int err, const char *f, va_list argptr)
{
    /* 
     *   presume that we'll use the given format string, instead of one
     *   provided by the program 
     */
    info->fmt = f;

    /* use the output buffer from the info structure */
    info->outp = info->user_msg;
    info->outsiz = sizeof(info->user_msg);

    /* 
     *   if the user has a parseError or parseErrorParam function, see if it
     *   provides a msg 
     */
    if (ctx->voccxper != MCMONINV || ctx->voccxperp != MCMONINV)
    {
        runcxdef *rcx = ctx->voccxrun;
        dattyp    typ;
        size_t    len;
        int       argc;

        /* start off with the two arguments that are always present */
        argc = 2;

        /* 
         *   if we're calling parseErrorParam, and we have additional
         *   arguments, push them as well 
         */
        if (ctx->voccxperp != MCMONINV)
        {
            enum typ_t
            {
                ARGBUF_STR, ARGBUF_INT, ARGBUF_CHAR
            };
            struct argbuf_t
            {
                enum typ_t typ;
                union
                {
                    char *strval;
                    int   intval;
                    char  charval;
                } val;
            };
            struct argbuf_t  args[5];
            struct argbuf_t *argp;
            const char  *p;

            /* 
             *   Retrieve the arguments by examining the format string.  We
             *   must buffer up the arguments before pushing them, because
             *   we need to push them in reverse order (last to first); so,
             *   we must scan all arguments before we push the first one.  
             */
            for (p = f, argp = args ; *p != '\0' ; ++p)
            {
                /* check if this is a parameter */
                if (*p == '%')
                {
                    /* find out what type it is */
                    switch(*++p)
                    {
                    case 's':
                        /* string - save the char pointer */
                        argp->val.strval = va_arg(argptr, char *);
                        argp->typ = ARGBUF_STR;

                        /* consume an argument slot */
                        ++argp;
                        break;

                    case 'd':
                        /* integer - save the integer */
                        argp->val.intval = va_arg(argptr, int);
                        argp->typ = ARGBUF_INT;

                        /* consume an argument slot */
                        ++argp;
                        break;

                    case 'c':
                        /* character */
                        argp->val.charval = (char)va_arg(argptr, int);
                        argp->typ = ARGBUF_CHAR;

                        /* consume an argument slot */
                        ++argp;
                        break;

                    default:
                        /* 
                         *   ignore other types (there shouldn't be any
                         *   other types anyway) 
                         */
                        break;
                    }
                }
            }

            /*
             *   Push the arguments - keep looping until we get back to the
             *   first argument slot 
             */
            while (argp != args)
            {
                /* move to the next argument, working backwards */
                --argp;

                /* push this argument */
                switch(argp->typ)
                {
                case ARGBUF_STR:
                    /* push the string value */
                    runpstr(rcx, argp->val.strval,
                            (int)strlen(argp->val.strval), 0);
                    break;

                case ARGBUF_INT:
                    /* push the number value */
                    runpnum(rcx, argp->val.intval);
                    break;

                case ARGBUF_CHAR:
                    /* push the character as a one-character string */
                    runpstr(rcx, &argp->val.charval, 1, 0);
                    break;
                }

                /* count the argument */
                ++argc;
            }
        }

        /* push standard arguments: error code and default message */
        runpstr(rcx, f, (int)strlen(f), 0);         /* 2nd arg: default msg */
        runpnum(rcx, (long)err);                   /* 1st arg: error number */

        /* invoke parseErrorParam if it's defined, otherwise parseError */
        runfn(rcx, (objnum)(ctx->voccxperp == MCMONINV
                            ? ctx->voccxper : ctx->voccxperp), argc);

        /* see what the function returned */
        typ = runtostyp(rcx);
        if (typ == DAT_SSTRING)
        {
            char *p;
            
            /* 
             *   they returned a string - use it as the error message
             *   instead of the default message 
             */
            p = (char *)runpopstr(rcx);
            len = osrp2(p) - 2;
            p += 2;
            if (len > sizeof(info->user_msg) - 1)
                len = sizeof(info->user_msg) - 1;
            memcpy(info->user_msg, p, len);
            info->user_msg[len] = '\0';

            /* use the returned string as the message to display */
            info->fmt = info->user_msg;

            /* use the remainder of the buffer for the final formatting */
            info->outp = info->user_msg + len + 1;
            info->outsiz = sizeof(info->user_msg) - len - 1;
        }
        else
        {
            /* ignore other return values */
            rundisc(rcx);
        }
    }

}

/* 
 *   General parser error formatter.
 *   
 *   Before calling this routine, callers MUST invoke vocerr_va_prep() to
 *   prepare the information structure.  Because both this routine and the
 *   prep routine need to look at the varargs list ('argptr'), the caller
 *   must call va_start/va_end around the prep call, and then AGAIN on this
 *   call.  va_start/va_end must be used twice to ensure that the argptr is
 *   property re-initialized for the call to this routine.  
 */
static void vocerr_va(voccxdef *ctx, struct vocerr_va_info *info,
                      int err, const char *f, va_list argptr)
{
    char *buf;
    
    /* turn on output */
    (void)tioshow(ctx->voccxtio);

    /* build the string to display */
    if (os_vasprintf(&buf, info->fmt, argptr) >= 0)
    {
        /* display it */
        tioputs(ctx->voccxtio, buf);

        /* free the buffer */
        osfree(buf);
    }
}

/* ------------------------------------------------------------------------ */
/* 
 *   display a parser informational message 
 */
void vocerr_info(voccxdef *ctx, int err, const char *f, ...)
{
    va_list argptr;
    struct vocerr_va_info info;

    /* prepare to format the message */
    va_start(argptr, f);
    vocerr_va_prep(ctx, &info, err, f, argptr);
    va_end(argptr);

   /* call the general vocerr formatter */
    va_start(argptr, f);
    vocerr_va(ctx, &info, err, f, argptr);
    va_end(argptr);
}

/* 
 *   display a parser error 
 */
void vocerr(voccxdef *ctx, int err, const char *f, ...)
{
    va_list argptr;
    struct vocerr_va_info info;

    /*
     *   If the unknown word flag is set, suppress this error, because
     *   we're going to be trying the whole parsing from the beginning
     *   again anyway.  
     */
    if (ctx->voccxunknown > 0)
        return;

    /* prepare to format the message */
    va_start(argptr, f);
    vocerr_va_prep(ctx, &info, err, f, argptr);
    va_end(argptr);

    /* call the general vocerr formatter */
    va_start(argptr, f);
    vocerr_va(ctx, &info, err, f, argptr);
    va_end(argptr);
}

/*
 *   Handle an unknown verb or sentence structure.  We'll call this when
 *   we encounter a sentence where we don't know the verb word, or we
 *   don't know the combination of verb and verb preposition, or we don't
 *   recognize the sentence structure (for example, an indirect object is
 *   present, but we don't have a template defined using an indirect
 *   object for the verb).
 *   
 *   This function calls the game-defined function parseUnknownVerb, if it
 *   exists.  If the function doesn't exist, we'll simply display the
 *   given error message, using the normal parseError mechanism.  The
 *   function should use "abort" or "exit" if it wants to cancel further
 *   processing of the command.
 *   
 *   We'll return true if the function exists, in which case normal
 *   processing should continue with any remaining command on the command
 *   line.  We'll return false if the function doesn't exist, in which
 *   case the remainder of the command should be aborted.  
 *   
 *   'wrdcnt' is the number of words in the cmd[] array.  If wrdcnt is
 *   zero, we'll automatically count the array entries, with the end of
 *   the array indicated by a null pointer entry.
 *   
 *   'next_start' is a variable that we may fill in with the index of the
 *   next word in the command to be parsed.  If the user function
 *   indicates the number of words it consumes, we'll use 'next_start' to
 *   communicate this back to the caller, so that the caller can resume
 *   parsing after the part parsed by the function.
 */
int try_unknown_verb(voccxdef *ctx, objnum actor,
                     char **cmd, int *typelist, int wrdcnt, int *next_start,
                     int do_fuses, int vocerr_err, const char *vocerr_msg, ...)
{
    int  show_msg;
    va_list  argptr;

    /* presume we won't show the message */
    show_msg = FALSE;

    /* determine the word count if the caller passed in zero */
    if (wrdcnt == 0)
    {
        /* count the words before the terminating null entry */
        for ( ; cmd[wrdcnt] != 0 ; ++wrdcnt) ;
    }

    /* if parseUnknownVerb exists, call it */
    if (ctx->voccxpuv != MCMONINV)
    {
        int  err;
        int  i;
        //int  do_fuses;

        /* no error has occurred yet */
        err = 0;

        /* presume we will run the fuses */
        do_fuses = TRUE;

        /* push the error number argument */
        runpnum(ctx->voccxrun, (long)vocerr_err);

        /* push a list of the word types */
        voc_push_numlist(ctx, (uint *)typelist, wrdcnt);

        /* push a list of the words */
        voc_push_toklist(ctx, cmd, wrdcnt);

        /* use "Me" as the default actor */
        if (actor == MCMONINV)
            actor = ctx->voccxme;

        /* push the actor argument */
        runpobj(ctx->voccxrun, actor);

        /* catch any errors that occur while calling the function */
        ERRBEGIN(ctx->voccxerr)
        {
            /* invoke the function */
            runfn(ctx->voccxrun, ctx->voccxpuv, 4);

            /* get the return value */
            switch(runtostyp(ctx->voccxrun))
            {
            case DAT_TRUE:
                /* the command was handled */
                rundisc(ctx->voccxrun);

                /* consume the entire command */
                *next_start = wrdcnt;

                /* 
                 *   since the command has now been handled, forget about
                 *   any unknown words 
                 */
                ctx->voccxunknown = 0;
                break;

            case DAT_NUMBER:
                /* 
                 *   The command was handled, and the function indicated
                 *   the number of words it wants to skip.  Communicate
                 *   this information back to the caller in *next_start.
                 *   Since the routine returns the 1-based index of the
                 *   next entry, we must subtract one to get the number of
                 *   words actually consumed.  
                 */
                *next_start = runpopnum(ctx->voccxrun);
                if (*next_start > 0)
                    --(*next_start);

                /* make sure the value is in range */
                if (*next_start < 0)
                    *next_start = 0;
                else if (*next_start > wrdcnt)
                    *next_start = wrdcnt;

                /* 
                 *   forget about any unknown words in the list up to the
                 *   next word 
                 */
                for (i = 0 ; i < *next_start ; ++i)
                {
                    /* if this word was unknown, forget about that now */
                    if ((typelist[i] & VOCT_UNKNOWN) != 0
                        && ctx->voccxunknown > 0)
                        --(ctx->voccxunknown);
                }
                break;

            default:
                /* treat anything else like nil */

            case DAT_NIL:
                /* nil - command not handled; show the message */
                rundisc(ctx->voccxrun);
                show_msg = TRUE;
                break;
            }
        }
        ERRCATCH(ctx->voccxerr, err)
        {
            /* check the error */
            switch(err)
            {
            case ERR_RUNEXIT:
            case ERR_RUNEXITOBJ:
                /* 
                 *   Exit or exitobj was executed - skip to the fuses.
                 *   Forget about any unknown words, since we've finished
                 *   processing this command and we don't want to allow
                 *   "oops" processing.  
                 */
                ctx->voccxunknown = 0;
                break;

            case ERR_RUNABRT:
                /* 
                 *   abort was executed - skip to the end of the command,
                 *   but do not execute the fuses 
                 */
                do_fuses = FALSE;

                /*
                 *   Since we're aborting the command, ignore any
                 *   remaining unknown words - we're skipping out of the
                 *   command entirely, so we don't care that there were
                 *   unknown words in the command.  
                 */
                ctx->voccxunknown = 0;
                break;

            default:
                /* re-throw any other errors */
                errrse(ctx->voccxerr);
            }
        }
        ERREND(ctx->voccxerr);

        /* if we're not showing the message, process fuses and daemons */
        if (!show_msg)
        {
            /* execute fuses and daemons */
            if (exe_fuses_and_daemons(ctx, err, do_fuses,
                                      actor, MCMONINV, 0, 0,
                                      MCMONINV, MCMONINV) != 0)
            {
                /* 
                 *   aborted from fuses and daemons - return false to tell
                 *   the caller not to execute anything left on the
                 *   command line 
                 */
                return FALSE;
            }

            /* indicate that the game code successfully handled the command */
            return TRUE;
        }
    }

    /* 
     *   If we made it here, it means we're showing the default message.
     *   If we have unknown words, suppress the message so that we show
     *   the unknown word error instead after returning.
     */
    if (ctx->voccxunknown == 0)
    {
        struct vocerr_va_info info;

        /* prepare to format the message */
        va_start(argptr, vocerr_msg);
        vocerr_va_prep(ctx, &info, vocerr_err, vocerr_msg, argptr);
        va_end(argptr);

        /* format the mesage */
        va_start(argptr, vocerr_msg);
        vocerr_va(ctx, &info, vocerr_err, vocerr_msg, argptr);
        va_end(argptr);
    }

    /* indicate that the remainder of the command should be aborted */
    return FALSE;
}


/* determine if a tokenized word is a special internal word flag */
/* int vocisspec(char *wrd); */
#define vocisspec(wrd) \
   (vocisupper(*wrd) || (!vocisalpha(*wrd) && *wrd != '\'' && *wrd != '-'))

static const vocspdef vocsptab[] =
{
    { "of",     VOCW_OF   },
    { "and",    VOCW_AND  },
    { "then",   VOCW_THEN },
    { "all",    VOCW_ALL  },
    { "everyt", VOCW_ALL  },
    { "both",   VOCW_BOTH },
    { "but",    VOCW_BUT  },
    { "except", VOCW_BUT  },
    { "one",    VOCW_ONE  },
    { "ones",   VOCW_ONES },
    { "it",     VOCW_IT   },
    { "them",   VOCW_THEM },
    { "him",    VOCW_HIM  },
    { "her",    VOCW_HER  },
    { "any",    VOCW_ANY  },
    { "either", VOCW_ANY  },
    { 0,        0         }
};

/* test a word to see if it's a particular special word */
static int voc_check_special(voccxdef *ctx, const char *wrd, int checktyp)
{
    /* search the user or built-in special table, as appropriate */
    if (ctx->voccxspp)
    {
        char  *p;
        char  *endp;
        char   typ;
        int    len;
        int    wrdlen = strlen(wrd);
        
        for (p = ctx->voccxspp, endp = p + ctx->voccxspl ;
             p < endp ; )
        {
            typ = *p++;
            len = *p++;

            /* if this word matches in type and text, we have a match */
            if (typ == checktyp
                && len == wrdlen && !memcmp(p, wrd, (size_t)len))
                return TRUE;

            /* no match - keep going */
            p += len;
        }
    }
    else
    {
        const vocspdef *x;
        
        for (x = vocsptab ; x->vocspin ; ++x)
        {
            /* if it matches in type and text, we have a match */
            if (x->vocspout == checktyp
                && !strncmp((const char *)wrd, x->vocspin, (size_t)6))
                return TRUE;
        }
    }

    /* didn't find a match for the text and type */
    return FALSE;
}


/* tokenize a command line - returns number of words in command */
int voctok(voccxdef *ctx, char *cmd, char *outbuf, char **wrd,
           int lower, int cvt_ones, int show_errors)
{
    int       i;
    const vocspdef *x;
    int       l;
    char     *p;
    char     *w;
    uint      len;

    for (i = 0 ;; )
    {
        while (vocisspace(*cmd)) cmd++;
        if (!*cmd)
        {
            wrd[i] = outbuf;
            *outbuf = '\0';
            return(i);
        }

        wrd[i++] = outbuf;
        if (vocisalpha(*cmd) || *cmd == '-')
        {
            while(vocisalpha(*cmd) || vocisdigit(*cmd) ||
                  *cmd=='\'' || *cmd=='-')
            {
                *outbuf++ = (vocisupper(*cmd) && lower) ? tolower(*cmd) : *cmd;
                ++cmd;
            }
            
            /*
             *   Check for a special case:  abbreviations that end in a
             *   period.  For example, "Mr. Patrick J. Wayne."  We wish
             *   to absorb the period after "Mr" and the one after "J"
             *   into the respective words; we detect this condition by
             *   actually trying to find a word in the dictionary that
             *   has the period.
             */
            w = wrd[i-1];
            len = outbuf - w;
            if (*cmd == '.')
            {
                *outbuf++ = *cmd++;           /* add the period to the word */
                *outbuf = '\0';                        /* null-terminate it */
                ++len;
                if (!vocffw(ctx, (char *)w, len, 0, 0, PRP_NOUN,
                            (vocseadef *)0)
                    && !vocffw(ctx, (char *)w, len, 0, 0, PRP_ADJ,
                               (vocseadef *)0))
                {
                    /* no word with period in dictionary - remove period */
                    --outbuf;
                    --cmd;
                    --len;
                }
            }

            /* null-terminate the buffer */
            *outbuf = '\0';

            /* find compound words and glue them together */
            for (p = ctx->voccxcpp, l = ctx->voccxcpl ; l ; )
            {
                uint   l1 = osrp2(p);
                char  *p2 = p + l1;                      /* get second word */
                uint   l2 = osrp2(p2);
                char  *p3 = p2 + l2;                   /* get compound word */
                uint   l3 = osrp2(p3);
                
                if (i > 1 && len == (l2 - 2)
                    && !memcmp(w, p2 + 2, (size_t)len)
                    && strlen((char *)wrd[i-2]) == (l1 - 2)
                    && !memcmp(wrd[i-2], p + 2, (size_t)(l1 - 2)))
                {
                    memcpy(wrd[i-2], p3 + 2, (size_t)(l3 - 2));
                    *(wrd[i-2] + l3 - 2) = '\0';
                    --i;
                    break;
                }

                /* move on to the next word */
                l -= l1 + l2 + l3;
                p = p3 + l3;
            }

            /*
             *   Find any special keywords, and set to appropriate flag
             *   char.  Note that we no longer convert "of" in this
             *   fashion; "of" is now handled separately in order to
             *   facilitate its use as an ordinary preposition. 
             */
            if (ctx->voccxspp)
            {
                //char  *p;
                char  *endp;
                char   typ;
                //int    len;
                uint    wrdlen = strlen((char *)wrd[i-1]);
                
                for (p = ctx->voccxspp, endp = p + ctx->voccxspl ;
                     p < endp ; )
                {
                    typ = *p++;
                    len = *p++;
                    if (len == wrdlen && !memcmp(p, wrd[i-1], (size_t)len)
                        && (cvt_ones || (typ != VOCW_ONE && typ != VOCW_ONES))
                        && typ != VOCW_OF)
                    {
                        *wrd[i-1] = typ;
                        *(wrd[i-1] + 1) = '\0';
                        break;
                    }
                    p += len;
                }
            }
            else
            {
                for (x = vocsptab ; x->vocspin ; ++x)
                {
                    if (!strncmp((char *)wrd[i-1], x->vocspin, (size_t)6)
                        && (cvt_ones ||
                            (x->vocspout != VOCW_ONE
                             && x->vocspout != VOCW_ONES))
                        && x->vocspout != VOCW_OF)
                    {
                        *wrd[i-1] = x->vocspout;
                        *(wrd[i-1] + 1) = '\0';
                        break;
                    }
                }
            }

            /* make sure the output pointer is fixed up to the right spot */
            outbuf = wrd[i-1];
            outbuf += strlen((char *)outbuf);
        }
        else if (vocisdigit( *cmd ))
        {
            while(vocisdigit(*cmd) || vocisalpha(*cmd)
                  || *cmd == '\'' || *cmd == '-')
                *outbuf++ = *cmd++;
        }
        else switch( *cmd )
        {
        case '.':
        case '!':
        case '?':
        case ';':
            *outbuf++ = VOCW_THEN;
            ++cmd;
            break;

        case ',':
        case ':':
            *outbuf++ = VOCW_AND;
            ++cmd;
            break;

        case '"':
        case '\'':
            {
                char  *lenptr;
                char   quote = *cmd++;

                /* 
                 *   remember that this is a quoted string (it doesn't
                 *   matter whether they're actually using single or
                 *   double quotes - in either case, we use '"' as the
                 *   flag to indicate that it's a quote string)
                 */
                *outbuf++ = '"';

                /* make room for the length prefix */
                lenptr = outbuf;
                outbuf += 2;

                /* copy up to the matching close quote */
                while (*cmd && *cmd != quote)
                {
                    char c;

                    /* get this character */
                    c = *cmd++;
                    
                    /* escape the character if necessary */
                    switch(c)
                    {
                    case '\\':
                        *outbuf++ = '\\';
                        break;
                    }

                    /* copy this character */
                    *outbuf++ = c;
                }
                
                oswp2(lenptr, ((int)(outbuf - lenptr)));
                if (*cmd == quote) cmd++;
                break;
            }

        default:
            /* display an error if appropriate */
            if (show_errors)
            {
                int hmode = tio_is_html_mode();

                /* 
                 *   if we're in HTML mode, switch out momentarily, so that
                 *   we show the character literally, even if it's a
                 *   markup-significant character (such as '<' or '&') 
                 */
                if (hmode)
                    tioputs(ctx->voccxtio, "\\H-");
                
                /* show the message */
                vocerr(ctx, VOCERR(1),
                       "I don't understand the punctuation \"%c\".", *cmd);

                /* restore HTML mode if appropriate */
                if (hmode)
                    tioputs(ctx->voccxtio, "\\H+");
            }

            /* return failure */
            return -1;
        }

        /* null-terminate the result */
        *outbuf++ = '\0';
    }
}


/* ------------------------------------------------------------------------ */
/*
 *   Look up a word's type.  If 'of_is_spec' is true, we'll treat OF as
 *   being of type special if it's not otherwise defined.  
 */
static int voc_lookup_type(voccxdef *ctx, char *p, int len, int of_is_spec)
{
    int t;
    
    /* check for a special word */
    if (vocisspec(p))
    {
        /* it's a special word - this is its type */
        t = VOCT_SPEC;
    }
    else
    {
        vocwdef *vw;
        vocdef  *v;

        /*
         *   Now check the various entries of this word to get the word
         *   type flag bits.  The Noun and Adjective flags can be set for
         *   any word which matches this word in the first six letters (or
         *   more if more were provided by the player), but the Plural
         *   flag can only be set if the plural word matches exactly.
         *   Note that this pass only matches the first word in two-word
         *   verbs; the second word is considered later during the
         *   semantic analysis.  
         */
        for (t = 0, v = ctx->voccxhsh[vochsh((uchar *)p, len)] ; v != 0 ;
             v = v->vocnxt)
        {
            /* if this hash chain entry matches, add it to our types */
            if (voceq((uchar *)p, len, v->voctxt, v->voclen))
            {
                /* we have a match - look through relation list for word */
                for (vw = vocwget(ctx, v->vocwlst) ; vw != 0 ;
                     vw = vocwget(ctx, vw->vocwnxt))
                {
                    /* skip this word if it's been deleted */
                    if (vw->vocwflg & VOCFDEL)
                        continue;

                    /* we need a special check for plurals */
                    if (vw->vocwtyp == PRP_PLURAL)
                    {
                        /* plurals must be exact (non-truncated) match */
                        if (len == v->voclen)
                        {
                            /* plurals also count as nouns */
                            t |= (VOCT_NOUN | VOCT_PLURAL);
                        }
                    }
                    else
                    {
                        /* add this type bit to our type value */
                        t |= voctype[vw->vocwtyp];
                    }
                }
            }
        }
    }

    /* check for "of" if the caller wants us to */
    if (of_is_spec && t == 0 && voc_check_special(ctx, p, VOCW_OF))
        t = VOCT_SPEC;

    /* return the type */
    return t;
}


/* ------------------------------------------------------------------------ */
/*
 *   Display an unknown word error, and read a new command, allowing the
 *   user to respond with the special OOPS command to correct the unknown
 *   word.  Returns a pointer to the start of the replacement text if the
 *   player entered a correction via OOPS, or a null pointer if the player
 *   simply entered a new command.  
 */
static char *voc_read_oops(voccxdef *ctx, char *oopsbuf, size_t oopsbuflen,
                           const char *unknown_word)
{
    char *p;
    
    /* display the error */
    vocerr(ctx, VOCERR(2), "I don't know the word \"%s\".", unknown_word);

    /* read a new command */
    if (vocread(ctx, MCMONINV, MCMONINV,
                oopsbuf, (int)oopsbuflen, 1) == VOCREAD_REDO)
    {
        /* 
         *   we've already decided it's not an OOPS input - return null to
         *   indicate to the caller that we have a new command 
         */
        return 0;
    }

    /* lower-case the string */
    for (p = oopsbuf ; *p != '\0' ; ++p)
        *p = (vocisupper(*p) ? tolower(*p) : *p);

    /* skip leading spaces */
    for (p = oopsbuf ; vocisspace(*p) ; ++p) ;

    /* 
     *   See if they are saying "oops".  Allow "oops" or simply "o",
     *   followed by either a space or a comma.  
     */
    if ((strlen(p) > 5 && memcmp(p, "oops ", 5) == 0)
        || (strlen(p) > 5 && memcmp(p, "oops,", 5) == 0))
    {
        /* we found "OOPS" - move to the next character */
        p += 5;
    }
    else if ((strlen(p) > 2 && memcmp(p, "o ", 2) == 0)
             || (strlen(p) > 2 && memcmp(p, "o,", 2) == 0))
    {
        /* we found "O" - move to the next character */
        p += 2;
    }
    else
    {
        /* 
         *   we didn't find any form of "OOPS" response - return null to
         *   indicate to the caller that the player entered a new command 
         */
        return 0;
    }

    /* skip spaces before the replacement text */
    for ( ; vocisspace(*p) ; ++p) ;

    /* return a pointer to the start of the replacement text */
    return p;
}

/* ------------------------------------------------------------------------ */
/*
 *   figure out what parts of speech are associated with each
 *   word in a tokenized command list
 */
int vocgtyp(voccxdef *ctx, char *cmd[], int types[], char *orgbuf)
{
    int      cur;
    int      t;
    char    *p;
    int      len;
    int      unknown_count = 0;
    
startover:
    if (ctx->voccxflg & VOCCXFDBG)
        tioputs(ctx->vocxtio, ". Checking words:\\n");

    for (cur = 0 ; cmd[cur] ; ++cur)
    {
        /* get the word */
        p = cmd[cur];
        len = strlen(p);

        /* look it up */
        t = voc_lookup_type(ctx, p, len, FALSE);
        
        /* see if the word was found */
        if (t == 0 && !voc_check_special(ctx, p, VOCW_OF))
        {
            /* 
             *   We didn't find the word.  For now, set its type to
             *   "unknown".
             */
            t = VOCT_UNKNOWN;

            /*
             *   If the unknown word count is already non-zero, it means
             *   that we've tried to let the game resolve this word using
             *   the parseUnknownDobj/Iobj mechanism, but it wasn't able
             *   to do so, thus we've come back here to use the normal
             *   "oops" processing instead.
             *   
             *   Don't generate a message until we get to the first
             *   unknown word from the original list that we weren't able
             *   to resolve.  We may have been able to handle one or more
             *   of the original list of unknown words (through
             *   parseNounPhrase or other means), so we don't want to
             *   generate a message for any words we ended up handling.
             *   The number we resolved is the last full unknown count
             *   minus the remaining unknown count.  
             */
            if (ctx->voccxunknown != 0
                && unknown_count >= ctx->voccxlastunk - ctx->voccxunknown)
            {
                char  oopsbuf[VOCBUFSIZ];
                char *p1;
                
                /* 
                 *   we can try using the parseUnknownDobj/Iobj again
                 *   after this, so clear the unknown word count for now
                 */
                ctx->voccxunknown = 0;

                /* display an error, and ask for a new command */
                p1 = voc_read_oops(ctx, oopsbuf, sizeof(oopsbuf), p);

                /* if they responded with replacement text, apply it */
                if (p1 != 0)
                {
                    char   redobuf[200];
                    char  *q;
                    int    i;
                    int    wc;
                    char **w;
                    char  *outp;
                    
                    /* 
                     *   copy words from the original string, replacing
                     *   the unknown word with what follows the "oops" in
                     *   the new command 
                     */
                    for (outp = redobuf, i = 0, w = cmd ; *w != 0 ; ++i, ++w)
                    {
                        
                        /* see what we have */
                        if (i == cur)
                        {
                            /* 
                             *   We've reached the word to be replaced.
                             *   Ignore the original token, and replace it
                             *   with the word or words from the OOPS
                             *   command 
                             */
                            for (q = p1, len = 0 ;
                                 *q != '\0' && *q != '.' && *q != ','
                                     && *q != '?' && *q != '!' ; ++q, ++len) ;
                            memcpy(outp, p1, (size_t)len);
                            outp += len;
                        }
                        else if (**w == '"')
                        {
                            char *strp;
                            char *p2;
                            char qu;
                            int rem;

                            /* 
                             *   It's a string - add a quote mark, then
                             *   copy the string as indicated by the
                             *   length prefix, then add another quote
                             *   mark.  Get the length by reading the
                             *   length prefix following the quote mark,
                             *   and get a pointer to the text of the
                             *   string, which immediately follows the
                             *   length prefix.  
                             */
                            len = osrp2(*w + 1) - 2;
                            strp = *w + 3;

                            /*
                             *   We need to figure out what kind of quote
                             *   mark to use.  If the string contains any
                             *   embedded double quotes, use single quotes
                             *   to delimit the string; otherwise, use
                             *   double quotes.  Presume we'll use double
                             *   quotes as the delimiter, then scan the
                             *   string for embedded double quotes.  
                             */
                            for (qu = '"', p2 = strp, rem = len ; rem != 0 ;
                                 --rem, ++p2)
                            {
                                /* 
                                 *   if this is an embedded double quote,
                                 *   use single quotes to delimite the
                                 *   string 
                                 */
                                if (*p2 == '"')
                                {
                                    /* use single quotes as delimiters */
                                    qu = '\'';

                                    /* no need to look any further */
                                    break;
                                }
                            }

                            /* add the open quote */
                            *outp++ = qu;

                            /* copy the string */
                            memcpy(outp, strp, len);
                            outp += len;

                            /* add the close quote */
                            *outp++ = qu;
                        }
                        else
                        {
                            /* 
                             *   it's an ordinary token - copy the
                             *   null-terminated string for the token from
                             *   the original command list 
                             */
                            len = strlen(*w);
                            memcpy(outp, *w, (size_t)len);
                            outp += len;
                        }

                        /* add a space between words */
                        *outp++ = ' ';
                    }
                    
                    /* terminate the new string */
                    *outp = '\0';
                    
                    /* try tokenizing the string */
                    *(cmd[0]) = '\0';
                    if ((wc = voctok(ctx, redobuf, cmd[0],
                                     cmd, FALSE, FALSE, TRUE)) <= 0)
                        return 1;
                    cmd[wc] = 0;

                    /* start over with the typing */
                    goto startover;
                }
                else
                {
                    /* 
                     *   They didn't start the command with "oops", so
                     *   this must be a brand new command.  Replace the
                     *   original command with the new command.  
                     */
                    strcpy(orgbuf, oopsbuf);

                    /* 
                     *   forget we had an unknown word so that we're sure
                     *   to start over with a new command 
                     */
                    ctx->voccxunknown = 0;
                    
                    /* 
                     *   set the "redo" flag to start over with the new
                     *   command 
                     */
                    ctx->voccxredo = 1;
                    
                    /* 
                     *   return an error to indicate the current command
                     *   has been aborted 
                     */
                    return 1;
                }
            }
            else
            {
                /*
                 *   We've now encountered an unknown word, and we're
                 *   going to defer resolution.  Remember this; we'll
                 *   count the unknown word in the context when we return
                 *   (do so only locally for now, since we may encounter
                 *   more unknown words before we return, in which case we
                 *   want to know that this is still the first pass).  
                 */
                ++unknown_count;
            }
        }

        /* display if in debug mode */
        if (ctx->voccxflg & VOCCXFDBG)
        {
            char    buf[128];
            size_t  i;
            //char   *p;
            int     cnt;
            
            (void)tioshow(ctx->voccxtio);
            sprintf(buf, "... %s (", cmd[cur]);
            p = buf + strlen(buf);
            cnt = 0;
            for (i = 0 ; i < sizeof(type_names)/sizeof(type_names[0]) ; ++i)
            {
                if (t & (1 << i))
                {
                    if (cnt) *p++ = ',';
                    strcpy(p, type_names[i]);
                    p += strlen(p);
                    ++cnt;
                }
            }
            *p++ = ')';
            *p++ = '\\';
            *p++ = 'n';
            *p = '\0';
            tioputs(ctx->voccxtio, buf);
        }
        
        types[cur] = t;                         /* record type of this word */
    }

    /* if we found any unknown words, note this in our context */
    ctx->voccxunknown = unknown_count;
    ctx->voccxlastunk = unknown_count;
    
    /* successful acquisition of types */
    return 0;
}

/*
 *   intersect - takes two lists and puts the intersection of them into
 *   the first list.
 */
static int vocisect(objnum *list1, objnum *list2)
{
    int i, j, k;

    for (i = k = 0 ; list1[i] != MCMONINV ; ++i)
    {
        for (j = 0 ; list2[j] != MCMONINV ; ++j)
        {
            if (list1[i] == list2[j])
            {
                list1[k++] = list1[i];
                break;
            }
        }
    }
    list1[k] = MCMONINV;
    return(k);
}

/*
 *   Intersect lists, including parallel flags lists.  The flags from the
 *   two lists for any matching object are OR'd together. 
 */
static int vocisect_flags(objnum *list1, uint *flags1,
                          objnum *list2, uint *flags2)
{
    int i, j, k;

    for (i = k = 0 ; list1[i] != MCMONINV ; ++i)
    {
        for (j = 0 ; list2[j] != MCMONINV ; ++j)
        {
            if (list1[i] == list2[j])
            {
                list1[k] = list1[i];
                flags1[k] = flags1[i] | flags2[j];
                ++k;
                break;
            }
        }
    }
    list1[k] = MCMONINV;
    return(k);
}

/*
 *   get obj list: build a list of the objects that are associated with a
 *   given word of player input.
 */
static int vocgol(voccxdef *ctx, objnum *list, uint *flags, char **wrdlst,
                  int *typlst, int first, int cur, int last, int ofword)
{
    const char *wrd;
    int        typ;
    vocwdef   *v;
    int        cnt;
    int        len;
    vocseadef  search_ctx;
    int        try_plural;
    int        try_noun_before_num;
    int        try_endadj;
    int        trying_endadj;
    int        wrdtyp;

    /* get the current word and its type */
    wrd = wrdlst[cur];
    typ = typlst[cur];

    /* get the length of the word */
    len = strlen(wrd);

    /*
     *   Get word type: figure out the correct part of speech, given by
     *   context, for a given word.  If it could count as only a
     *   noun/plural or only an adjective, we use that.  If it could count
     *   as either a noun/plural or an adjective, we will treat it as a
     *   noun/plural if it is the last word in the name or the last word
     *   before "of", otherwise as an adjective.
     *   
     *   If the word is unknown, treat it as a noun or adjective - treat
     *   it as part of the current noun phrase.  One unknown word renders
     *   the whole noun phrase unknown.  
     */
    try_plural = (typ & VOCT_PLURAL);

    /* presume we won't retry this word as an adjective */
    try_endadj = FALSE;

    /* presume we won't retry this as a noun before a number */
    try_noun_before_num = FALSE;

    /* we're not yet trying with adjective-at-end */
    trying_endadj = FALSE;

    /* check to see what parts of speech are defined for this word */
    if ((typ & (VOCT_NOUN | VOCT_PLURAL)) && (typ & VOCT_ADJ))
    {
        /*
         *   This can be either an adjective or a plural/noun.  If this is
         *   the last word in the noun phrase, treat it as a noun/plural if
         *   possible.  Otherwise, treat it as an adjective.  
         */
        if (cur + 1 == last || cur == ofword - 1)
        {
            /* 
             *   This is the last word in the entire phrase, or the last word
             *   before an 'of' (which makes it the last word of its
             *   subphrase).  Treat it as a noun if possible, otherwise as a
             *   plural 
             */
            wrdtyp = ((typ & VOCT_NOUN) ? PRP_NOUN : PRP_PLURAL);

            /* 
             *   If this can be an adjective, too, make a note to come back
             *   and try it again as an adjective.  We prefer not to end a
             *   noun phrase with an adjective, but we allow it, since it's
             *   often convenient to abbreviate a noun phrase to just the
             *   adjectives (as in TAKE RED, where there's only one object
             *   nearby to which RED applies).  
             */
            if ((typ & VOCT_ADJ) != 0)
                try_endadj = TRUE;
        }
        else if ((cur + 2 == last || cur == ofword - 2)
                 && vocisdigit(wrdlst[cur+1][0]))
        {
            /*
             *   This is the second-to-last word, and the last word is
             *   numeric.  In this case, try this word as BOTH a noun and an
             *   adjective.  Try it as an adjective first, but make a note to
             *   go back and try it again as a noun. 
             */
            wrdtyp = PRP_ADJ;
            try_noun_before_num = TRUE;
        }
        else
        {
            /* 
             *   This isn't the last word, so it can only be an adjective.
             *   Look at it only as an adjective.  
             */
            wrdtyp = PRP_ADJ;
        }
    }
    else if (typ & VOCT_NOUN)
        wrdtyp = PRP_NOUN;
    else if (typ & VOCT_UNKNOWN)
        wrdtyp = PRP_UNKNOWN;
    else
    {
        /* it's just an adjective */
        wrdtyp = PRP_ADJ;

        /* 
         *   if this is the last word in the phrase, flag it as an ending
         *   adjective 
         */
        if (cur + 1 == last || cur == ofword - 1)
            trying_endadj = TRUE;
    }

    /* display debugger information if appropriate */
    if (ctx->voccxflg & VOCCXFDBG)
    {
        char buf[128];

        sprintf(buf, "... %s (treating as %s%s)\\n", wrd,
                (wrdtyp == PRP_ADJ ? "adjective" :
                 wrdtyp == PRP_NOUN ? "noun" :
                 wrdtyp == PRP_INVALID ? "unknown" : "plural"),
                (wrdtyp == PRP_NOUN && try_plural ? " + plural" : ""));
        tioputs(ctx->vocxtio, buf);
    }

    /* if this is an unknown word, it doesn't have any objects */
    if (wrdtyp == PRP_UNKNOWN)
    {
        list[0] = MCMONINV;
        return 0;
    }

    /* we have nothing in the list yet */
    cnt = 0;

add_words:
    for (v = vocffw(ctx, wrd, len, (char *)0, 0, wrdtyp, &search_ctx)
         ; v != 0 ; v = vocfnw(ctx, &search_ctx))
    {
        int i;

        /* add the matching object to the output list */
        list[cnt] = v->vocwobj;

        /* clear the flags */
        flags[cnt] = 0;

        /* set the PLURAL flag if this is the plural vocabulary usage */
        if (wrdtyp == PRP_PLURAL)
            flags[cnt] |= VOCS_PLURAL;

        /* set the ADJECTIVE AT END flag if appropriate */
        if (wrdtyp == PRP_ADJ && trying_endadj)
            flags[cnt] |= VOCS_ENDADJ;

        /* 
         *   if this is not an exact match for the word, but is merely a
         *   long-enough leading substring, flag it as truncated 
         */
        if (len < search_ctx.v->voclen)
            flags[cnt] |= VOCS_TRUNC;

        /* count the additional word in the list */
        ++cnt;

        /* 
         *   if this object is already in the list with the same flags,
         *   don't add it again 
         */
        for (i = 0 ; i < cnt - 1 ; ++i)
        {
            /* check for an identical entry */
            if (list[i] == list[cnt-1] && flags[i] == flags[cnt-1])
            {
                /* take it back out of the list */
                --cnt;

                /* no need to continue looking for the duplicate */
                break;
            }
        }

        /* make sure we haven't overflowed the list */
        if (cnt >= VOCMAXAMBIG)
        {
            vocerr(ctx, VOCERR(3),
                   "The word \"%s\" refers to too many objects.", wrd);
            list[0] = MCMONINV;
            return -1;
        }
    }

    /*
     *   if we want to go back and try the word again as a noun before a
     *   number (as in "button 5"), do so now 
     */
    if (try_noun_before_num && wrdtyp == PRP_ADJ)
    {
        /* change the word type to noun */
        wrdtyp = PRP_NOUN;

        /* don't try this again */
        try_noun_before_num = FALSE;

        /* add the words for the noun usage */
        goto add_words;
    }

    /*
     *   if we're interpreting the word as a noun, and the word can be a
     *   plural, add in the plural interpretation as well 
     */
    if (try_plural && wrdtyp != PRP_PLURAL)
    {
        /* change the word type to plural */
        wrdtyp = PRP_PLURAL;

        /* don't try plurals again */
        try_plural = FALSE;

        /* add the words for the plural usage */
        goto add_words;
    }

    /* 
     *   if this was the last word in the phrase, and it could have been
     *   an adjective, try it again as an adjective 
     */
    if (try_endadj && wrdtyp != PRP_ADJ)
    {
        /* change the word type to adjective */
        wrdtyp = PRP_ADJ;

        /* note that we're retrying as an adjective */
        trying_endadj = TRUE;

        /* don't try this again */
        try_endadj = FALSE;

        /* add the words for the adjective usage */
        goto add_words;
    }

    /*
     *   If we're interpreting the word as an adjective, and it's
     *   numeric, include objects with "#" in their adjective list --
     *   these objects allow arbitrary numbers as adjectives.  Don't do
     *   this if there's only the one word.  
     */
    if (vocisdigit(wrd[0]) && wrdtyp == PRP_ADJ && first + 1 != last)
    {
        wrd = "#";
        len = 1;
        goto add_words;
    }

    list[cnt] = MCMONINV;
    return cnt;
}

/*
 *   Add the user-defined word for "of" to a buffer.  If no such word is
 *   defined by the user (with the specialWords construct), add "of".  
 */
static void vocaddof(voccxdef *ctx, char *buf)
{
    if (ctx->voccxspp)
    {
        size_t len = ctx->voccxspp[1];
        size_t oldlen = strlen(buf);
        memcpy(buf + oldlen, ctx->voccxspp + 2, len);
        buf[len + oldlen] = '\0';
    }
    else
        strcat(buf, "of");
}

/* ------------------------------------------------------------------------ */
/*
 *   Call the parseNounPhrase user function, if defined, to attempt to
 *   parse a noun phrase.
 *   
 *   Returns VOC_PNP_ERROR if the hook function indicates that an error
 *   occurred; PNP_DEFAULT if the hook function told us to use the default
 *   list; or PNP_SUCCESS to indicate that the hook function provided a
 *   list to use.  
 */
static int voc_pnp_hook(voccxdef *ctx, char *cmd[], int typelist[],
                        int cur, int *next, int complain,
                        vocoldef *out_nounlist, int *out_nouncount,
                        int chkact, int *no_match)
{
    runcxdef *rcx = ctx->voccxrun;
    runsdef val;
    int wordcnt;
    char **cmdp;
    int outcnt;
    vocoldef *outp;
    int i;

    /* if parseNounPhrase isn't defined, use the default handling */
    if (ctx->voccxpnp == MCMONINV)
        return VOC_PNP_DEFAULT;

    /* push the actor-check flag */
    val.runstyp = (chkact ? DAT_TRUE : DAT_NIL);
    runpush(rcx, val.runstyp, &val);

    /* push the complain flag */
    val.runstyp = (complain ? DAT_TRUE : DAT_NIL);
    runpush(rcx, val.runstyp, &val);

    /* push the current index (adjusted to 1-based user convention) */
    runpnum(rcx, cur + 1);

    /* count the entries in the command list */
    for (wordcnt = 0, cmdp = cmd ; *cmdp != 0 && **cmdp != '\0' ;
         ++wordcnt, ++cmdp) ;

    /* push the type list */
    voc_push_numlist(ctx, (uint *)typelist, wordcnt);

    /* push the command word list */
    voc_push_strlist_arr(ctx, cmd, wordcnt);

    /* call the method */
    runfn(rcx, ctx->voccxpnp, 5);

    /* check the return value */
    if (runtostyp(rcx) == DAT_NUMBER)
    {
        /* return the status code directly from the hook function */
        return (int)runpopnum(rcx);
    }
    else if (runtostyp(rcx) == DAT_LIST)
    {
        uchar *lstp;
        uint lstsiz;
        
        /* pop the list */
        lstp = runpoplst(rcx);

        /* read and skip the size prefix */
        lstsiz = osrp2(lstp);
        lstsiz -= 2;
        lstp += 2;

        /* the first element should be the next index */
        if (lstsiz > 1 && *lstp == DAT_NUMBER)
        {
            /* set the 'next' pointer, adjusting to 0-based indexing */
            *next = osrp4(lstp+1) - 1;

            /* 
             *   If 'next' is out of range, force it into range.  We can't
             *   go backwards (so 'next' must always be at least 'cur'),
             *   and we can't go past the null element at the end of the
             *   list. 
             */
            if (*next < cur)
                *next = cur;
            else if (*next > wordcnt)
                *next = wordcnt;

            /* skip the list entry */
            lstadv(&lstp, &lstsiz);
        }
        else
        {
            /* ignore the list and use the default parsing */
            return VOC_PNP_DEFAULT;
        }

        /* read the list entries and store them in the output array */
        for (outcnt = 0, outp = out_nounlist ; lstsiz > 0 ; )
        {
            /* make sure we have room for another entry */
            if (outcnt >= VOCMAXAMBIG - 1)
                break;
            
            /* get the next list entry, and store it in the output array */
            if (*lstp == DAT_NIL)
            {
                /* set the list entry */
                outp->vocolobj = MCMONINV;

                /* skip the entry */
                lstadv(&lstp, &lstsiz);
            }
            else if (*lstp == DAT_OBJECT)
            {
                /* set the list entry */
                outp->vocolobj = osrp2(lstp+1);

                /* skip the list entry */
                lstadv(&lstp, &lstsiz);
            }
            else
            {
                /* ignore other types in the list */
                lstadv(&lstp, &lstsiz);
                continue;
            }

            /* check for a flag entry */
            if (lstsiz > 0 && *lstp == DAT_NUMBER)
            {
                /* set the flags */
                outp->vocolflg = (int)osrp4(lstp+1);
                
                /* skip the number */
                lstadv(&lstp, &lstsiz);
            }
            else
            {
                /* no flags were specified - use the default */
                outp->vocolflg = 0;
            }

            /* set the word list boundaries */
            outp->vocolfst = cmd[cur];
            outp->vocollst = cmd[*next - 1];

            /* count the entry */
            ++outp;
            ++outcnt;
        }

        /* terminate the list */
        outp->vocolobj = MCMONINV;
        outp->vocolflg = 0;

        /* set the output count */
        *out_nouncount = outcnt;

        /* 
         *   set "no_match" appropriately -- set "no_match" true if we're
         *   returning an empty list and we parsed one or more words 
         */
        if (no_match != 0)
            *no_match = (outcnt == 0 && *next > cur);

        /*
         *   Adjust the unknown word count in the context.  If the routine
         *   parsed any unknown words, decrement the unknown word count in
         *   the context by the number of unknown words parsed, since
         *   these have now been dealt with.  If the return list contains
         *   any objects flagged as having unknown words, add the count of
         *   such objects back into the context, since we must still
         *   resolve these at disambiguation time. 
         */
        for (i = cur ; i < *next ; ++i)
        {
            /* if this parsed word was unknown, remove it from the count */
            if ((typelist[i] & VOCT_UNKNOWN) != 0)
                --(ctx->voccxunknown);
        }
        for (i = 0, outp = out_nounlist ; i < outcnt ; ++i)
        {
            /* if this object has the unknown flag, count it */
            if ((outp->vocolflg & VOCS_UNKNOWN) != 0)
                ++(ctx->voccxunknown);
        }

        /* indicate that the hook provided a list */
        return VOC_PNP_SUCCESS;
    }
    else
    {
        /* 
         *   ignore any other return value - consider others equivalent to
         *   DEFAULT 
         */
        rundisc(rcx);
        return VOC_PNP_DEFAULT;
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   Build an object name from the words in a command 
 */
void voc_make_obj_name(voccxdef *ctx, char *namebuf, char *cmd[],
                       int firstwrd, int lastwrd)
{
    int i;
    
    /* run through the range of words, and add them to the buffer */
    for (i = firstwrd, namebuf[0] = '\0' ; i < lastwrd ; ++i)
    {
        if (voc_check_special(ctx, cmd[i], VOCW_OF))
            vocaddof(ctx, namebuf);
        else
            strcat(namebuf, cmd[i]);
        
        if (cmd[i][strlen(cmd[i])-1] == '.' && i + 1 < lastwrd)
            strcat(namebuf, "\\");

        if (i + 1 < lastwrd)
            strcat(namebuf, " ");
    }
}

/*
 *   Make an object name from a list entry 
 */
void voc_make_obj_name_from_list(voccxdef *ctx, char *namebuf,
                                 char *cmd[], const char *firstwrd, const char *lastwrd)
{
    int i, i1, i2;
    
    /* find the cmd indices */
    for (i = i1 = i2 = 0 ; cmd[i] != 0 && *cmd[i] != 0 ; ++i)
    {
        if (cmd[i] == firstwrd)
            i1 = i;
        if (cmd[i] == lastwrd)
            i2 = i + 1;
    }

    /* build the name */
    voc_make_obj_name(ctx, namebuf, cmd, i1, i2);
}


/* ------------------------------------------------------------------------ */
/*
 *   get 1 obj - attempts to figure out the limits of a single noun
 *   phrase.  Aside from dealing with special words here ("all", "it",
 *   "them", string objects, numeric objects), we will accept a basic noun
 *   phrase of the form [article][adjective*][noun]["of" [noun-phrase]].
 *   (Note that this is not actually recursive; only one "of" can occur in
 *   a noun phrase.)  If successful, we will construct a list of all
 *   objects that have all the adjectives and nouns in the noun phrase.
 *   Note that plurals are treated basically like nouns, except that we
 *   will flag them so that the disambiguator knows to include all objects
 *   that work with the plural.
 *   
 *   Note that we also allow the special constructs "all [of]
 *   <noun-phrase>" and "both [of] <noun-phrase>"; these are treated
 *   identically to normal plurals.
 *   
 *   If no_match is not null, we'll set it to true if we found valid
 *   syntax but no matching objects, false otherwise.  
 */
static int vocg1o(voccxdef *ctx, char *cmd[], int typelist[],
                  int cur, int *next, int complain, vocoldef *nounlist,
                  int chkact, int *no_match)
{
    int     l1;
    int     firstwrd;
    int     i;
    int     ofword = -1;
    int     hypothetical_last = -1;
    int     trim_flags = 0;
    int     outcnt = 0;
    objnum *list1;
    uint   *flags1;
    objnum *list2;
    uint   *flags2;
    char   *namebuf;
    int     has_any = FALSE;
    uchar  *save_sp;
    int     found_plural;
    int     unknown_count;
    int     trying_count = FALSE;
    int     retry_with_count;

    voc_enter(ctx, &save_sp);
    VOC_MAX_ARRAY(ctx, objnum, list1);
    VOC_MAX_ARRAY(ctx, uint,   flags1);
    VOC_MAX_ARRAY(ctx, objnum, list2);
    VOC_MAX_ARRAY(ctx, uint,   flags2);
    VOC_STK_ARRAY(ctx, char,   namebuf, VOCBUFSIZ);

    /* presume we'll find a match */
    if (no_match != 0)
        *no_match = FALSE;

    /* start at the first word */
    *next = cur;

    /* if we're at the end of the command, return no objects in list */
    if (cur == -1 || !cmd[cur]) { VOC_RETVAL(ctx, save_sp, 0); }

    /* show trace message if in debug mode */
	if (ctx->voccxflg & VOCCXFDBG) {
		tioputs(ctx->vocxtio, chkact ? ". Checking for actor\\n" : ". Reading noun phrase\\n");
	}

    /* try the user parseNounPhrase hook */
    switch(voc_pnp_hook(ctx, cmd, typelist, cur, next, complain,
                        nounlist, &outcnt, chkact, no_match))
    {
    case VOC_PNP_DEFAULT:
        /* continue on to the default processing */
        break;

    case VOC_PNP_ERROR:
    default:
        /* return an error */
        VOC_RETVAL(ctx, save_sp, -1);

    case VOC_PNP_SUCCESS:
        /* use their list */
        VOC_RETVAL(ctx, save_sp, outcnt);
    }

    /* check for a quoted string */
    if (*cmd[cur] == '"')
    {
        /* can't use a quoted string as an actor */
        if (chkact) { VOC_RETVAL(ctx, save_sp, 0); }
        
        if (ctx->voccxflg & VOCCXFDBG)
            tioputs(ctx->vocxtio, "... found quoted string\\n");

        nounlist[outcnt].vocolobj = MCMONINV;
        nounlist[outcnt].vocolflg = VOCS_STR;
        nounlist[outcnt].vocolfst = nounlist[outcnt].vocollst = cmd[cur];
        *next = ++cur;
        ++outcnt;
        VOC_RETVAL(ctx, save_sp, outcnt);
    }

    /* check for ALL/ANY/BOTH/EITHER [OF] <plural> contruction */
    if ((vocspec(cmd[cur], VOCW_ALL)
         || vocspec(cmd[cur], VOCW_BOTH)
         || vocspec(cmd[cur], VOCW_ANY)) &&
        cmd[cur+1] != (char *)0)
    {
        int nxt;
        int n = cur+1;
        int has_of;

        /* can't use ALL as an actor */
        if (chkact) { VOC_RETVAL(ctx, save_sp, 0); }

        /* remember whether we have "any" or "either" */
        has_any = vocspec(cmd[cur], VOCW_ANY);

        /* check for optional 'of' */
        if (voc_check_special(ctx, cmd[n], VOCW_OF))
        {
            if (ctx->voccxflg & VOCCXFDBG)
                tioputs(ctx->vocxtio, "... found ALL/ANY/BOTH/EITHER OF\\n");

            has_of = TRUE;
            n++;
            if (!cmd[n])
            {
                const char *p;
                int   ver;
                
                if (vocspec(cmd[cur], VOCW_ALL))
                {
                    ver = VOCERR(4);
                    p = "I think you left something out after \"all of\".";
                }
                else if (vocspec(cmd[cur], VOCW_ANY))
                {
                    ver = VOCERR(29);
                    p = "I think you left something out after \"any of\".";
                }
                else
                {
                    ver = VOCERR(5);
                    p = "There's something missing after \"both of\".";
                }
                vocerr(ctx, ver, p);
                VOC_RETVAL(ctx, save_sp, -1);
            }
        }
        else
            has_of = FALSE;

        nxt = n;
        if (typelist[n] & VOCT_ARTICLE) ++n;        /* skip leading article */
        for ( ;; )
        {
            if (!cmd[n])
                break;

            if (voc_check_special(ctx, cmd[n], VOCW_OF))
            {
                ++n;
                if (!cmd[n])
                {
                    vocerr(ctx, VOCERR(6), "I expected a noun after \"of\".");
                    VOC_RETVAL(ctx, save_sp, -1);
                }
                if (*cmd[n] & VOCT_ARTICLE) ++n;
            }
            else if (typelist[n] & (VOCT_ADJ | VOCT_NOUN))
                ++n;
            else
                break;
        }

        /*
         *   Accept the ALL if the last word is a plural.  Accept the ANY
         *   if either we don't have an OF (ANY NOUN is okay even without
         *   a plural), or if we have OF and a plural.  (More simply put,
         *   accept the ALL or ANY if the last word is a plural, or if we
         *   have ANY but not OF).  
         */
        if (n > cur && ((typelist[n-1] & VOCT_PLURAL)
                        || (has_any && !has_of)))
        {
            if (ctx->voccxflg & VOCCXFDBG)
                tioputs(ctx->vocxtio,
                        "... found ALL/ANY/BOTH/EITHER + noun phrase\\n");

            cur = nxt;
        }
    }
    
    if (vocspec(cmd[cur], VOCW_ALL) && !has_any)
    {
        /* can't use ALL as an actor */
        if (chkact)
        {
            VOC_RETVAL(ctx, save_sp, 0);
        }
        
        if (ctx->voccxflg & VOCCXFDBG)
            tioputs(ctx->vocxtio, "... found ALL\\n");

        nounlist[outcnt].vocolobj = MCMONINV;
        nounlist[outcnt].vocolflg = VOCS_ALL;
        nounlist[outcnt].vocolfst = nounlist[outcnt].vocollst = cmd[cur];
        ++outcnt;
        ++cur;

        if (cmd[cur] && vocspec(cmd[cur], VOCW_BUT))
        {
            int       cnt;
            //int       i;
            vocoldef *xlist;
            uchar    *inner_save_sp;

            if (ctx->voccxflg & VOCCXFDBG)
                tioputs(ctx->vocxtio, "... found ALL EXCEPT\\n");

            voc_enter(ctx, &inner_save_sp);
            VOC_MAX_ARRAY(ctx, vocoldef, xlist);

            cur++;
            cnt = vocgobj(ctx, cmd, typelist, cur, next, complain, xlist, 1, 
                          chkact, 0);
            if (cnt < 0)
            {
                /* 
                 *   An error occurred - return it.  Note that, since
                 *   we're returning from the entire function, we're
                 *   popping the save_sp frame, NOT the inner_save_sp
                 *   frame -- the inner frame is nested within the save_sp
                 *   frame, and we want to pop the entire way out since
                 *   we're exiting the entire function. 
                 */
                VOC_RETVAL(ctx, save_sp, cnt);
            }
            cur = *next;
            for (i = 0 ; i < cnt ; ++i)
            {
                OSCPYSTRUCT(nounlist[outcnt], xlist[i]);
                nounlist[outcnt].vocolflg |= VOCS_EXCEPT;
                ++outcnt;
            }

            voc_leave(ctx, inner_save_sp);
        }
        *next = cur;
        nounlist[outcnt].vocolobj = MCMONINV;
        nounlist[outcnt].vocolflg = 0;
        VOC_RETVAL(ctx, save_sp, outcnt);
    }
    
    switch(*cmd[cur])
    {
    case VOCW_IT:
        nounlist[outcnt].vocolflg = VOCS_IT;
        goto do_special;
    case VOCW_THEM:
        nounlist[outcnt].vocolflg = VOCS_THEM;
        goto do_special;
    case VOCW_HIM:
        nounlist[outcnt].vocolflg = VOCS_HIM;
        goto do_special;
    case VOCW_HER:
        nounlist[outcnt].vocolflg = VOCS_HER;
        /* FALLTHRU */
    do_special:
        if (ctx->voccxflg & VOCCXFDBG)
            tioputs(ctx->vocxtio, "... found pronoun\\n");

        *next = cur + 1;
        nounlist[outcnt].vocolobj = MCMONINV;
        nounlist[outcnt].vocolfst = nounlist[outcnt].vocollst = cmd[cur];
        ++outcnt;
        VOC_RETVAL(ctx, save_sp, outcnt);
    default:
        break;
    }

    if (((typelist[cur]
          & (VOCT_ARTICLE | VOCT_ADJ | VOCT_NOUN | VOCT_UNKNOWN)) == 0)
        && !vocisdigit(*cmd[cur]))
    {
        VOC_RETVAL(ctx, save_sp, 0);
    }

    if (typelist[cur] & VOCT_ARTICLE)
    {
        ++cur;
        if (cmd[cur] == (char *)0
            || ((typelist[cur] & (VOCT_ADJ | VOCT_NOUN | VOCT_UNKNOWN)) == 0
                && !vocisdigit(*cmd[cur])))
        {
            vocerr(ctx, VOCERR(7), "An article must be followed by a noun.");
            *next = cur;
            VOC_RETVAL(ctx, save_sp, -1);
        }
    }

    /* start at the current word */
    firstwrd = cur;

    /* scan words for inclusion in this noun phrase */
    for (found_plural = FALSE, unknown_count = 0, l1 = 0 ; ; )
    {
        if (cmd[cur] == (char *)0)
            break;

        if (typelist[cur] & VOCT_ADJ)
            ++cur;
        else if (typelist[cur] & VOCT_UNKNOWN)
        {
            /* 
             *   Remember that we found an unknown word, but include it in
             *   the noun phrase - this will render the entire noun phrase
             *   unknown, but we'll resolve that later.
             */
            ++unknown_count;
            ++cur;
        }
        else if (typelist[cur] & VOCT_NOUN)
        {
            ++cur;
            if (cmd[cur] == (char *)0) break;
            if (vocisdigit(*cmd[cur])) ++cur;
            if (cmd[cur] == (char *)0) break;
            if (!voc_check_special(ctx, cmd[cur], VOCW_OF)) break;
        }
        else if (vocisdigit(*cmd[cur]))
            ++cur;
        else if (voc_check_special(ctx, cmd[cur], VOCW_OF))
        {
            ++cur;
            if (ofword != -1)
            {
                /* there's already one 'of' - we must be done */
                --cur;
                break;
            }
            ofword = cur-1;
            if (typelist[cur] & VOCT_ARTICLE)   /* allow article after "of" */
                ++cur;
        }
        else
            break;

        /* note whether we found anything that might be a plural */
        if (cmd[cur] != 0 && (typelist[cur] & VOCT_PLURAL) != 0)
            found_plural = TRUE;
    }

try_again:
    /* 
     *   build a printable string consisting of the words in the noun
     *   phrase, for displaying error messages 
     */
    voc_make_obj_name(ctx, namebuf, cmd, firstwrd, cur);

    /* remember the next word after this noun phrase */
    *next = cur;

    /*
     *   If we have any unknown words, we won't be able to match any
     *   objects for the noun phrase.  Return with one entry in the list,
     *   but use an invalid object and mark the object as containing
     *   unknown words.  
     */
    if (unknown_count > 0)
    {
        /*
         *   Add one unknown object for each unknown word.  This lets us
         *   communicate the number of unknown words that we found to the
         *   disambiguator, which will later attempt to resolve the
         *   reference.  Each object we add is the same; they're here only
         *   for the word count.  
         */
        for ( ; unknown_count > 0 ; --unknown_count)
        {
            nounlist[outcnt].vocolobj = MCMONINV;
            nounlist[outcnt].vocolflg = VOCS_UNKNOWN;
            nounlist[outcnt].vocolfst = cmd[firstwrd];
            nounlist[outcnt].vocollst = cmd[cur-1];
            ++outcnt;
        }
        nounlist[outcnt].vocolobj = MCMONINV;
        nounlist[outcnt].vocolflg = 0;
        VOC_RETVAL(ctx, save_sp, outcnt);
    }

    /* get the list of objects that match the first word */
    l1 = vocgol(ctx, list1, flags1, cmd, typelist,
                firstwrd, firstwrd, cur, ofword);

    /*
     *   Allow retrying with a count plus a plural if the first word is a
     *   number, and we have something plural in the list.  Only treat "1"
     *   this way if more words follow in the noun phrase.  
     */
    retry_with_count = ((vocisdigit(*cmd[firstwrd]) && found_plural)
                        || (vocisdigit(*cmd[firstwrd])
                            && cur != firstwrd+1
                            && atoi(cmd[firstwrd]) == 1));

    /* see if we found anything on the first word */
    if (l1 <= 0)
    {
        if (chkact) { VOC_RETVAL(ctx, save_sp, 0); }

        if (vocisdigit(*cmd[firstwrd]))
        {
            if (retry_with_count)
            {
                /* interpret it as a count plus a plural */
                trying_count = TRUE;

                /* don't try this again */
                retry_with_count = FALSE;
            }
            else
            {
                /* not a plural - take the number as the entire noun phrase */
                nounlist[outcnt].vocolobj = MCMONINV;
                nounlist[outcnt].vocolflg = VOCS_NUM;
                nounlist[outcnt].vocolfst = nounlist[outcnt].vocollst =
                    cmd[firstwrd];
                *next = firstwrd + 1;
                ++outcnt;
                nounlist[outcnt].vocolobj = MCMONINV;
                nounlist[outcnt].vocolflg = 0;
                VOC_RETVAL(ctx, save_sp, outcnt);
            }
        }
        else
        {
            /* 
             *   display a message if we didn't already (if vocgol
             *   returned less than zero, it already displayed its own
             *   error message) 
             */
            if (l1 == 0)
                vocerr(ctx, VOCERR(9), "I don't see any %s here.", namebuf);

            /* return failure */
            VOC_RETVAL(ctx, save_sp, -1);
        }
    }

retry_exclude_first:
    for (i = firstwrd + 1 ; i < cur ; ++i)
    {
        int l2;
        
        if (voc_check_special(ctx, cmd[i], VOCW_OF)
            || (typelist[i] & VOCT_ARTICLE))
            continue;

        /* get the list for this new object */
        l2 = vocgol(ctx, list2, flags2, cmd, typelist,
                    firstwrd, i, cur, ofword);

        /* if that failed and displayed an error, return failure */
        if (l2 < 0)
        {
            /* return failure */
            VOC_RETVAL(ctx, save_sp, -1);
        }

        /*
         *   Intersect the last list with the new list.  If the previous
         *   list didn't have anything in it, it must mean that the word
         *   list started with a number, in which case we're trying to
         *   interpret this as a count plus a plural.  So, don't intersect
         *   the list if there was nothing in the first list. 
         */
        if (l1 == 0)
        {
            /* just copy the new list */
            l1 = l2;
            memcpy(list1, list2, (size_t)((l1+1) * sizeof(list1[0])));
            memcpy(flags1, flags2, (size_t)(l1 * sizeof(flags1[0])));
        }       
        else
        {
            /* intersect the two lists */
            l1 = vocisect_flags(list1, flags1, list2, flags2);
        }

        /*
         *   If there's nothing in the list, it means that there's no
         *   object that defines all of these words.  
         */
        if (l1 == 0)
        {
            if (ctx->voccxflg & VOCCXFDBG)
                tioputs(ctx->vocxtio,
                        "... can't find any objects matching these words\\n");
            /*
             *   If there's an "of", remove the "of" and everything that
             *   follows, and go back and reprocess the part up to the
             *   "of" -- treat it as a sentence that has two objects, with
             *   "of" as the preposition introducing the indirect object.
             */
            if (ofword != -1)
            {
                if (ctx->voccxflg & VOCCXFDBG)
                    tioputs(ctx->vocxtio,
                            "... dropping the part after OF and retrying\\n");

                /* 
                 *   drop the part from 'of' on - scan only from firstwrd
                 *   to the word before 'of' 
                 */
                hypothetical_last = cur;
                trim_flags |= VOCS_TRIMPREP;
                cur = ofword;

                /* forget that we have an 'of' phrase at all */
                ofword = -1;

                /* retry with the new, shortened phrase */
                goto try_again;
            }

            /*
             *   Try again with the count + plural interpretation, if
             *   possible 
             */
            if (retry_with_count)
            {
                if (ctx->voccxflg & VOCCXFDBG)
                    tioputs(ctx->vocxtio,
                         "... treating the number as a count and retrying\\n");

                /* we've exhausted our retries */
                retry_with_count = FALSE;
                trying_count = TRUE;

                /* go try it */
                goto retry_exclude_first;
            }

            /*
             *   If one of the words will work as a preposition, and we
             *   took it as an adjective, go back and try the word again
             *   as a preposition.  
             */
            for (i = cur - 1; i > firstwrd ; --i)
            {
                if (typelist[i] & VOCT_PREP)
                {
                    if (ctx->voccxflg & VOCCXFDBG)
                        tioputs(ctx->vocxtio,
                                "... changing word to prep and retrying\\n");

                    hypothetical_last = cur;
                    trim_flags |= VOCS_TRIMPREP;
                    cur = i;
                    goto try_again;
                }
            }

            /* if just checking actor, don't display an error */
            if (chkact) { VOC_RETVAL(ctx, save_sp, 0); }

            /* 
             *   tell the player about it unless supressing complaints,
             *   and return an error 
             */
            if (complain)
                vocerr(ctx, VOCERR(9), "I don't see any %s here.", namebuf);
            if (no_match != 0)
                *no_match = TRUE;
            VOC_RETVAL(ctx, save_sp, 0);
        }
    }

    /*
     *   We have one or more objects, so make a note of how we found
     *   them.
     */
    if (ctx->voccxflg & VOCCXFDBG)
        tioputs(ctx->vocxtio, "... found objects matching vocabulary:\\n");

    /* store the list of objects that match all of our words */
    for (i = 0 ; i < l1 ; ++i)
    {
        if (ctx->voccxflg & VOCCXFDBG)
        {
            tioputs(ctx->voccxtio, "..... ");
            runppr(ctx->voccxrun, list1[i], PRP_SDESC, 0);
            tioflushn(ctx->voccxtio, 1);
        }

        nounlist[outcnt].vocolfst = cmd[firstwrd];
        nounlist[outcnt].vocollst = cmd[cur-1];
        nounlist[outcnt].vocolflg =
            flags1[i] | (trying_count ? VOCS_COUNT : 0) | trim_flags;
        if (trim_flags)
            nounlist[outcnt].vocolhlst = cmd[hypothetical_last - 1];
        if (has_any)
            nounlist[outcnt].vocolflg |= VOCS_ANY;
        nounlist[outcnt++].vocolobj = list1[i];
        if (outcnt > VOCMAXAMBIG)
        {
            vocerr(ctx, VOCERR(10),
                   "You're referring to too many objects with \"%s\".",
                   namebuf);
            VOC_RETVAL(ctx, save_sp, -2);
        }
    }

    /*
     *   If we have a one-word noun phrase, and the word is a number, add
     *   the number object into the list of objects we're considering,
     *   even though we found an object that matches.  We'll probably
     *   easily disambiguate this later, but we need to keep open the
     *   possibility that they're just referring to a number rather than a
     *   numbered adjective for now.
     */
    if (firstwrd + 1 == cur && vocisdigit(*cmd[firstwrd]))
    {
        /* add just the number as an object */
        nounlist[outcnt].vocolobj = ctx->voccxnum;
        nounlist[outcnt].vocolflg = VOCS_NUM;
        nounlist[outcnt].vocolfst = nounlist[outcnt].vocollst =
            cmd[firstwrd];
        ++outcnt;
    }

    /* terminate the list */
    nounlist[outcnt].vocolobj = MCMONINV;
    nounlist[outcnt].vocolflg = 0;

    /* return the number of objects in our match list */
    VOC_RETVAL(ctx, save_sp, outcnt);
}

/*
 *   get obj - gets one or more noun lists (a flag, "multi", says whether
 *   we should allow multiple lists).  We use vocg1o() to read noun lists
 *   one at a time, and keep going (if "multi" is true) as long as there
 *   are more "and <noun-phrase>" clauses.
 *   
 *   If no_match is not null, we'll set it to true if the syntax was okay
 *   but we didn't find any match for the list of words, false otherwise.  
 */
int vocgobj(voccxdef *ctx, char *cmd[], int typelist[],
            int cur, int *next, int complain, vocoldef *nounlist,
            int multi, int chkact, int *no_match)
{
    int       cnt;
    int       outcnt = 0;
    int       i;
    int       again = FALSE;
	int       lastcur = 0;
    vocoldef *tmplist;
    uchar    *save_sp;

    voc_enter(ctx, &save_sp);
    VOC_MAX_ARRAY(ctx, vocoldef, tmplist);

    for ( ;; )
    {
        /* try getting a single object */
        cnt = vocg1o(ctx, cmd, typelist, cur, next, complain,
                     tmplist, chkact, no_match);

        /* if we encountered a syntax error, return failure */
        if (cnt < 0)
        {
            VOC_RETVAL(ctx, save_sp, cnt);
        }

        /* if we got any objects, store them in our output list */
        if (cnt > 0)
        {
            for (i = 0 ; i < cnt ; ++i)
            {
                OSCPYSTRUCT(nounlist[outcnt], tmplist[i]);
                if (++outcnt > VOCMAXAMBIG)
                {
                    vocerr(ctx, VOCERR(11),
                           "You're referring to too many objects.");
                    VOC_RETVAL(ctx, save_sp, -1);
                }
            }
        }

        /* if we didn't find any objects, stop looking */
        if (cnt == 0)
        {
            if (again)
                *next = lastcur;
            break;
        }

        /* 
         *   if the caller only wanted a single object (or is getting an
         *   actor, in which case they implicitly want only a single
         *   object), stop looking for additional noun phrases 
         */
        if (!multi || chkact)
            break;

        /* skip past the previous noun phrase */
        cur = *next;

        /* 
         *   if we're looking at a noun phrase separator ("and" or a
         *   comma), get the next noun phrase; otherwise, we're done 
         */
        if (cur != -1 && cmd[cur] != 0 && vocspec(cmd[cur], VOCW_AND))
        {
            lastcur = cur;
            while (cmd[cur] && vocspec(cmd[cur], VOCW_AND)) ++cur;
            again = TRUE;
            if (complain) complain = 2;
        }
        else
        {
            /* end of line, or not at a separator - we're done */
            break;
        }
    }

    /* terminate the list and return the number of objects we found */
    nounlist[outcnt].vocolobj = MCMONINV;
    nounlist[outcnt].vocolflg = 0;
    VOC_RETVAL(ctx, save_sp, outcnt);
}


/* ------------------------------------------------------------------------ */
/*
 *   TADS program code interface - tokenize a string.  Returns a list of
 *   strings, with each string giving a token in the command. 
 */
void voc_parse_tok(voccxdef *ctx)
{
    uchar  *save_sp;
    runcxdef *rcx = ctx->voccxrun;
    char **cmd;
    char *inbuf;
    char *outbuf;
    uchar *p;
    uint len;
    int cnt;

    /* enter our stack frame */
    voc_enter(ctx, &save_sp);

    /* get the string argument */
    p = runpopstr(rcx);

    /* get and skip the length prefix */
    len = osrp2(p) - 2;
    p += 2;

    /* 
     *   Allocate space for the original string, and space for the token
     *   pointers and the tokenized string buffer.  We could potentially
     *   have one token per character in the original string, and we could
     *   potentially need one extra null terminator for each character in
     *   the original string; allocate accordingly.  
     */
    VOC_STK_ARRAY(ctx, char,   inbuf,  len + 1);
    VOC_STK_ARRAY(ctx, char,   outbuf, len*2);
    VOC_STK_ARRAY(ctx, char *, cmd,    len*2);

    /* copy the string into our buffer, and null-terminate it */
    memcpy(inbuf, p, len);
    inbuf[len] = '\0';

    /* tokenize the string */
    cnt = voctok(ctx, inbuf, outbuf, cmd, TRUE, FALSE, FALSE);

    /* check the result */
    if (cnt < 0)
    {
        /* negative count - it's an error, so return nil */
        runpnil(rcx);
    }
    else
    {
        /* push the return list */
        voc_push_toklist(ctx, cmd, cnt);
    }

    /* leave our stack frame */
    voc_leave(ctx, save_sp);
}

/* ------------------------------------------------------------------------ */
/*
 *   TADS program code interface - get the list of types for a list words.
 *   The words are simply strings of the type returned from the tokenizer.
 *   The return value is a list of types, with each entry in the return
 *   list giving the types of the corresponding entry in the word list. 
 */
void voc_parse_types(voccxdef *ctx)
{
    runcxdef *rcx = ctx->voccxrun;
    uchar *wrdp;
    uint wrdsiz;
    uchar *typp;
    uchar *lstp;
    uint lstsiz;
    int wrdcnt;

    /* get the list of words from the stack */
    wrdp = runpoplst(rcx);

    /* read and skip the size prefix */
    wrdsiz = osrp2(wrdp) - 2;
    wrdp += 2;

    /* scan the list and count the number of string entries */
    for (wrdcnt = 0, lstp = wrdp, lstsiz = wrdsiz ; lstsiz != 0 ;
         lstadv(&lstp, &lstsiz))
    {
        /* if this is a string, count it */
        if (*lstp == DAT_SSTRING)
            ++wrdcnt;
    }

    /* allocate the return list - one number entry per word */
    typp = voc_push_list(ctx, wrdcnt, 4);

    /* look up each word and set the corresponding element in the type list */
    for (lstp = wrdp, lstsiz = wrdsiz ; lstsiz != 0 ; lstadv(&lstp, &lstsiz))
    {
        /* if this is a string, look it up in the dictionary */
        if (*lstp == DAT_SSTRING)
        {
            char buf[256];
            int curtyp;
            uint len;

            /* make sure it fits in our buffer */
            len = osrp2(lstp+1) - 2;
            if (len < sizeof(buf))
            {
                /* extract the word into our buffer */
                memcpy(buf, lstp+3, len);

                /* null-terminate it */
                buf[len] = '\0';

                /* get the type */
                curtyp = voc_lookup_type(ctx, buf, len, TRUE);

                /* if they didn't find a type at all, set it to UNKNOWN */
                if (curtyp == 0)
                    curtyp = VOCT_UNKNOWN;
            }
            else
            {
                /* the string is too big - just mark it as unknown */
                curtyp = VOCT_UNKNOWN;
            }

            /* add this type to the return list */
            *typp++ = DAT_NUMBER;
            oswp4s(typp, curtyp);
            typp += 4;
        }
    }
}


/* ------------------------------------------------------------------------ */
/*
 *   Parse a noun phrase from TADS program code.  Takes a list of words
 *   and a list of types from the stack, uses the standard noun phrase
 *   parser, and returns a list of matching objects.  The object list is
 *   not disambiguated, but merely reflects all matching objects.  The
 *   entire standard parsing algorithm applies, including parseNounPhrase
 *   invocation if appropriate.  
 */
void voc_parse_np(voccxdef *ctx)
{
    runcxdef *rcx = ctx->voccxrun;
    vocoldef *objlist;
    uchar *save_sp;
    uchar *wordp;
    uint wordsiz;
    uchar *typp;
    uint typsiz;
    int cnt;
    char **wordarr;
    int wordcnt;
    char *wordchararr;
    uint wordcharsiz;
    int *typarr;
    int complain;
    int chkact;
    int multi;
    int no_match;
    int next;
    int cur;
    uchar *lstp;
    uint lstsiz;
    char *p;
    int i;
    int old_unknown, old_lastunk;

    /* allocate stack arrays */
    voc_enter(ctx, &save_sp);
    VOC_MAX_ARRAY(ctx, vocoldef, objlist);

    /* 
     *   Save the original context unknown values, since we don't want to
     *   affect the context information in this game-initiated call, then
     *   clear the unknown word count for the duration of the call.  
     */
    old_unknown = ctx->voccxunknown;
    old_lastunk = ctx->voccxlastunk;
    ctx->voccxunknown = ctx->voccxlastunk = 0;

    /* get the list of words, and read its length prefix */
    wordp = runpoplst(rcx);
    wordsiz = osrp2(wordp) - 2;
    wordp += 2;

    /* get the list of types, and read its length prefix */
    typp = runpoplst(rcx);
    typsiz = osrp2(typp) - 2;
    typp += 2;

    /* get the starting index (adjusting for zero-based indexing) */
    cur = runpopnum(rcx) - 1;
    next = cur;

    /* get the flag arguments */
    complain = runpoplog(rcx);
    multi = runpoplog(rcx);
    chkact = runpoplog(rcx);

    /* count the words in the word list */
    for (wordcnt = 0, lstp = wordp, wordcharsiz = 0, lstsiz = wordsiz ;
         lstsiz != 0 ; lstadv(&lstp, &lstsiz))
    {
        /* count the word */
        ++wordcnt;

        /* 
         *   count the space needed for the word - count the bytes of the
         *   string plus a null terminator 
         */
        if (*lstp == DAT_SSTRING)
            wordcharsiz += osrp2(lstp+1) + 1;
    }

    /* allocate space for the word arrays */
    VOC_STK_ARRAY(ctx, char,   wordchararr, wordcharsiz);
    VOC_STK_ARRAY(ctx, char *, wordarr,     wordcnt+1);
    VOC_STK_ARRAY(ctx, int,    typarr,      wordcnt+1);

    /* build the word array */
    for (i = 0, p = wordchararr, lstp = wordp, lstsiz = wordsiz ;
         lstsiz != 0 ; lstadv(&lstp, &lstsiz))
    {
        /* add the word to the word array */
        if (*lstp == DAT_SSTRING)
        {
            uint len;
            
            /* add this entry to the word array */
            wordarr[i] = p;

            /* copy the word to the character array and null-terminate it */
            len = osrp2(lstp + 1) - 2;
            memcpy(p, lstp + 3, len);
            p[len] = '\0';

            /* move the write pointer past this entry */
            p += len + 1;

            /* bump the index */
            ++i;
        }
    }

    /* store an empty last entry */
    wordarr[i] = 0;

    /* build the type array */
    for (i = 0, lstp = typp, lstsiz = typsiz ;
         lstsiz != 0 && i < wordcnt ; lstadv(&lstp, &lstsiz))
    {
        /* add this entry to the type array */
        if (*lstp == DAT_NUMBER)
        {
            /* set the entry */
            typarr[i] = (int)osrp4(lstp + 1);

            /* bump the index */
            ++i;
        }
    }

    /* parse the noun phrase */
    cnt = vocgobj(ctx, wordarr, typarr, cur, &next, complain, objlist,
                  multi, chkact, &no_match);

    /* restore context unknown values */
    ctx->voccxunknown = old_unknown;
    ctx->voccxlastunk = old_lastunk;

    /* check the return value */
    if (cnt < 0)
    {
        /* syntax error; return nil to indicate an error */
        runpnil(rcx);
    }
    else if (cnt == 0)
    {
        /* 
         *   No objects found.  Return a list consisting only of the next
         *   index.  If the next index is equal to the starting index,
         *   this will tell the caller that no noun phrase is
         *   syntactically present; otherwise, it will tell the caller
         *   that a noun phrase is present but there are no matching
         *   objects.
         *   
         *   Note that we must increment the returned element index to
         *   conform with the 1-based index values that the game function
         *   uses.  
         */
        ++next;
        voc_push_numlist(ctx, (uint *)&next, 1);
    }
    else
    {
        /*
         *   We found one or more objects.  Return a list whose first
         *   element is the index of the next word to be parsed, and whose
         *   remaining elements are sublists.  Each sublist contains a
         *   match for one noun phrase; for each AND adding another noun
         *   phrase, there's another sublist.  Each sublist contains the
         *   index of the first word of its noun phrase, the index of the
         *   last word of its noun phrase, and then the objects.  For each
         *   object, there is a pair of entries: the object itself, and
         *   the flags for the object.
         *   
         *   First, figure out how much space we need by scanning the
         *   return list.  
         */
        for (lstsiz = 0, i = 0 ; i < cnt ; )
        {
            int j;

            /* 
             *   count the entries in this sublist by looking for the next
             *   entry whose starting word is different 
             */
            for (j = i ;
                 j < cnt && objlist[j].vocolfst == objlist[i].vocolfst ;
                 ++j)
            {
                /* 
                 *   for this entry, we need space for the object (1 + 2
                 *   for an object, or just 1 for nil) and flags (1 + 4) 
                 */
                if (objlist[j].vocolobj == MCMONINV)
                    lstsiz += 1;
                else
                    lstsiz += 3;
                lstsiz += 5;
            }

            /* 
             *   For this sublist, we need space for the first index (type
             *   prefix + number = 1 + 4 = 5) and the last index (5).
             *   We've already counted space for the objects in the list.
             *   Finally, we need space for the list type and length
             *   prefixes (1 + 2) for the sublist itself.  
             */
            lstsiz += (5 + 5) + 3;

            /* skip to the next element */
            i = j;
        }

        /* 
         *   finally, we need space for the first element of the list,
         *   which is the index of the next word to be parsed (1+4)
         */
        lstsiz += 5;

        /* allocate space for the list */
        lstp = voc_push_list_siz(ctx, lstsiz);

        /* 
         *   store the first element - the index of the next word to parse
         *   (adjusting to 1-based indexing) 
         */
        *lstp++ = DAT_NUMBER;
        oswp4s(lstp, next + 1);
        lstp += 4;

        /* build the list */
        for (i = 0 ; i < cnt ; )
        {
            uchar *sublstp;
            int j;
            int firstidx = 0;
			int lastidx = 0;

            /* store the list type prefix */
            *lstp++ = DAT_LIST;

            /* 
             *   remember where the length prefix is, then skip it - we'll
             *   come back and fill it in when we're done with the sublist 
             */
            sublstp = lstp;
            lstp += 2;

            /* search the array to find the indices of the boundary words */
            for (j = 0 ; j < wordcnt ; ++j)
            {
                /* if this is the first word, remember the index */
                if (wordarr[j] == objlist[i].vocolfst)
                    firstidx = j;

                /* if this is the last word, remember the index */
                if (wordarr[j] == objlist[i].vocollst)
                {
                    /* remember the index */
                    lastidx = j;

                    /* 
                     *   we can stop looking now, since the words are
                     *   always in order (so the first index will never be
                     *   after the last index) 
                     */
                    break;
                }
            }

            /* add the first and last index, adjusting to 1-based indexing */
            *lstp++ = DAT_NUMBER;
            oswp4s(lstp, firstidx + 1);
            lstp += 4;
            *lstp++ = DAT_NUMBER;
            oswp4s(lstp, lastidx + 1);
            lstp += 4;

            /* add the objects */
            for (j = i ; 
                 j < cnt && objlist[j].vocolfst == objlist[i].vocolfst ;
                 ++j)
            {
                /* add this object */
                if (objlist[j].vocolobj == MCMONINV)
                {
                    /* just store a nil */
                    *lstp++ = DAT_NIL;
                }
                else
                {
                    /* store the object */
                    *lstp++ = DAT_OBJECT;
                    oswp2(lstp, objlist[j].vocolobj);
                    lstp += 2;
                }

                /* add the flags */
                *lstp++ = DAT_NUMBER;
                oswp4s(lstp, objlist[i].vocolflg);
                lstp += 4;
            }

            /* fix up the sublist's length prefix */
            oswp2(sublstp, lstp - sublstp);

            /* move on to the next sublist */
            i = j;
        }
    }

    /* exit the stack frame */
    voc_leave(ctx, save_sp);
}


/* ------------------------------------------------------------------------ */
/*
 *   TADS program code interface - given a list of words and a list of
 *   their types, produce a list of objects that match all of the words.  
 */
void voc_parse_dict_lookup(voccxdef *ctx)
{
    uchar *save_sp;
    runcxdef *rcx = ctx->voccxrun;
    uchar *wrdp;
    uint wrdsiz;
    uchar *typp;
    uint typsiz;
    objnum *list1;
    objnum *list2;
    int cnt1;
    int cnt2;

    /* enter our stack frame and allocate stack arrays */
    voc_enter(ctx, &save_sp);
    VOC_MAX_ARRAY(ctx, objnum, list1);
    VOC_MAX_ARRAY(ctx, objnum, list2);
    
    /* get the word list, and read and skip its size prefix */
    wrdp = runpoplst(rcx);
    wrdsiz = osrp2(wrdp) - 2;
    wrdp += 2;

    /* get the type list, and read and skip its size prefix */
    typp = runpoplst(rcx);
    typsiz = osrp2(typp) - 2;
    typp += 2;

    /* nothing in the main list yet */
    cnt1 = 0;

    /* go through the word list */
    while (wrdsiz > 0)
    {
        int curtyp;
        int type_prop;
        char *curword;
        uint curwordlen;
        char *curword2;
        uint curwordlen2;
        vocwdef *v;
        char *p;
        uint len;
        vocseadef  search_ctx;
        
        /* if this entry is a string, consider it */
        if (*wrdp == DAT_SSTRING)
        {
            /* get the current word's text string */
            curword = (char *)(wrdp + 3);
            curwordlen = osrp2(wrdp+1) - 2;

            /* check for an embedded space */
            for (p = curword, len = curwordlen ; len != 0 && !t_isspace(*p) ;
                 ++p, --len) ;
            if (len != 0)
            {
                /* get the second word */
                curword2 = p + 1;
                curwordlen2 = len - 1;

                /* truncate the first word accordingly */
                curwordlen -= len;
            }
            else
            {
                /* no embedded space -> no second word */
                curword2 = 0;
                curwordlen2 = 0;
            }

            /* presume we won't find a valid type property */
            type_prop = PRP_INVALID;

            /* 
             *   get this type entry, if there's another entry in the
             *   list, and it's of the appropriate type 
             */
            if (typsiz > 0 && *typp == DAT_NUMBER)
            {
                /*
                 *   Figure out what type property we'll be using.  We'll
                 *   consider only one meaning for each word, and we'll
                 *   arbitrarily pick one if the type code has more than
                 *   one type, because we expect the caller to provide
                 *   exactly one type per word.  
                 */
                int i;
                struct typemap_t
                {
                    int flag;
                    int prop;
                };
                static struct typemap_t typemap[] =
                {
                    { VOCT_ARTICLE, PRP_ARTICLE },
                    { VOCT_ADJ,     PRP_ADJ },
                    { VOCT_NOUN,    PRP_NOUN },
                    { VOCT_PREP,    PRP_PREP },
                    { VOCT_VERB,    PRP_VERB },
                    { VOCT_PLURAL,  PRP_PLURAL }
                };
                struct typemap_t *mapp;

                /* get the type */
                curtyp = (int)osrp4(typp+1);

                /* search for a type */
                for (mapp = typemap, i = sizeof(typemap)/sizeof(typemap[0]) ;
                     i != 0 ; ++mapp, --i)
                {
                    /* if this flag is set, use this type property */
                    if ((curtyp & mapp->flag) != 0)
                    {
                        /* use this one */
                        type_prop = mapp->prop;
                        break;
                    }
                }
            }

            /* nothing in the new list yet */
            cnt2 = 0;

            /* scan for matching words */
            for (v = vocffw(ctx, curword, curwordlen, curword2, curwordlen2,
                            type_prop, &search_ctx) ;
                 v != 0 ;
                 v = vocfnw(ctx, &search_ctx))
            {
                int i;
                
                /* make sure we have room in our list */
                if (cnt2 >= VOCMAXAMBIG - 1)
                    break;

                /* make sure that this entry isn't already in our list */
                for (i = 0 ; i < cnt2 ; ++i)
                {
                    /* if this entry matches, stop looking */
                    if (list2[i] == v->vocwobj)
                        break;
                }

                /* if it's not already in the list, add it now */
                if (i == cnt2)
                {
                    /* add it to our list */
                    list2[cnt2++] = v->vocwobj;
                }
            }

            /* terminate the list */
            list2[cnt2] = MCMONINV;

            /* 
             *   if there's nothing in the first list, simply copy this
             *   into the first list; otherwise, intersect the two lists 
             */
            if (cnt1 == 0)
            {
                /* this is the first list -> copy it into the main list */
                memcpy(list1, list2, (cnt2+1)*sizeof(list2[0]));
                cnt1 = cnt2;
            }
            else
            {
                /* intersect the two lists */
                cnt1 = vocisect(list1, list2);
            }

            /* 
             *   if there's nothing in the result list now, there's no
             *   need to look any further, because further intersection
             *   will yield nothing 
             */
            if (cnt1 == 0)
                break;
        }
        
        /* advance the word list */
        lstadv(&wrdp, &wrdsiz);

        /* if there's anything left in the type list, advance it as well */
        if (typsiz > 0)
            lstadv(&typp, &typsiz);
    }

    /* push the result list */
    voc_push_objlist(ctx, list1, cnt1);

    /* exit our stack frame */
    voc_leave(ctx, save_sp);
}

/* ------------------------------------------------------------------------ */
/*
 *   TADS program code interface - disambiguate a noun list.  We take the
 *   kind of complex object list returned by voc_parse_np(), and produce a
 *   fully-resolved list of objects.  
 */
void voc_parse_disambig(voccxdef *ctx)
{
    voccxdef ctx_copy;
    uchar *save_sp;
    runcxdef *rcx = ctx->voccxrun;
    vocoldef *inlist;
    vocoldef *outlist;
    int objcnt;
    char **cmd;
    char *cmdbuf;
    char *oopsbuf;
    objnum actor;
    objnum verb;
    objnum prep;
    objnum otherobj;
    prpnum defprop;
    prpnum accprop;
    prpnum verprop;
    uchar *tokp;
    uint toksiz;
    uchar *objp;
    uint objsiz;
    uchar *lstp;
    uint lstsiz;
    int tokcnt;
    char *p;
    int i;
    int silent;
    int err;
    int unknown_count;

    /* allocate our stack arrays */
    voc_enter(ctx, &save_sp);
    VOC_MAX_ARRAY(ctx, vocoldef, outlist);
    VOC_STK_ARRAY(ctx, char,     cmdbuf, VOCBUFSIZ);
    VOC_STK_ARRAY(ctx, char,     oopsbuf, VOCBUFSIZ);

    /* get the actor, verb, prep, and otherobj arguments */
    actor = runpopobj(rcx);
    verb = runpopobj(rcx);
    prep = runpopobjnil(rcx);
    otherobj = runpopobjnil(rcx);

    /* 
     *   get the usage parameter, and use it to select the appropriate
     *   defprop and accprop 
     */
    switch(runpopnum(rcx))
    {
    case VOC_PRO_RESOLVE_DOBJ:
    default:
        defprop = PRP_DODEFAULT;
        accprop = PRP_VALIDDO;
        break;

    case VOC_PRO_RESOLVE_IOBJ:
        defprop = PRP_IODEFAULT;
        accprop = PRP_VALIDIO;
        break;

    case VOC_PRO_RESOLVE_ACTOR:
        defprop = PRP_DODEFAULT;
        accprop = PRP_VALIDACTOR;
        break;
    }
    
    /* get the verprop argument */
    verprop = runpopprp(rcx);

    /* pop the token list, and read and skip the length prefix */
    tokp = runpoplst(rcx);
    toksiz = osrp2(tokp) - 2;
    tokp += 2;

    /* pop the object list, and read and skip the length prefix */
    objp = runpoplst(rcx);
    objsiz = osrp2(objp) - 2;
    objp += 2;

    /* pop the "silent" flag */
    silent = runpoplog(rcx);

    /* count the tokens */
    for (lstp = tokp, lstsiz = toksiz, tokcnt = 0 ;
         lstsiz != 0 ; lstadv(&lstp, &lstsiz))
    {
        /* if this is a string, count it */
        if (*lstp == DAT_SSTRING)
            ++tokcnt;
    }

    /* allocate stack space for the command pointers and buffer */
    VOC_STK_ARRAY(ctx, char *, cmd, tokcnt + 1);

    /* extract the tokens into our pointer buffer */
    for (lstp = tokp, lstsiz = toksiz, i = 0, p = cmdbuf ;
         lstsiz != 0 ; lstadv(&lstp, &lstsiz))
    {
        /* if this is a string, count and size it */
        if (*lstp == DAT_SSTRING)
        {
            uint len;
            
            /* store a pointer to the word in the command buffer */
            cmd[i++] = p;

            /* copy the token into the command buffer and null-terminate it */
            len = osrp2(lstp + 1) - 2;
            memcpy(p, lstp + 3, len);
            p[len] = '\0';

            /* move the buffer pointer past this word */
            p += len + 1;
        }
    }

    /* store a null at the end of the command pointer list */
    cmd[i] = 0;

    /*
     *   The object list is provided in the same format as the list
     *   returned by voc_parse_np(), but the leading index number is
     *   optional.  We don't need the leading index for anything, so if
     *   it's there, simply skip it so that we can start with the first
     *   sublist.  
     */
    if (objsiz > 0 && *objp == DAT_NUMBER)
        lstadv(&objp, &objsiz);

    /*
     *   Count the objects in the object list, so that we can figure out
     *   how much we need to allocate for the input object list.  
     */
    for (lstp = objp, lstsiz = objsiz, objcnt = 0 ; lstsiz != 0 ;
         lstadv(&lstp, &lstsiz))
    {
        /* if this is a sublist, parse it */
        if (*lstp == DAT_LIST)
        {
            uchar *subp;
            uint subsiz;

            /* get the sublist pointer, and read and skip the size prefix */
            subp = lstp + 1;
            subsiz = osrp2(subp) - 2;
            subp += 2;

            /* scan the sublist */
            while (subsiz > 0)
            {
                /* if this is an object, count it */
                if (*subp == DAT_OBJECT || *subp == DAT_NIL)
                    ++objcnt;

                /* skip this element */
                lstadv(&subp, &subsiz);
            }
        }
    }

    /* allocate space for the input list */
    VOC_STK_ARRAY(ctx, vocoldef, inlist, objcnt + 1);

    /* we don't have any unknown words yet */
    unknown_count = 0;

    /* parse the list, filling in the input array */
    for (lstp = objp, lstsiz = objsiz, i = 0 ; lstsiz != 0 ;
         lstadv(&lstp, &lstsiz))
    {
        /* if this is a sublist, parse it */
        if (*lstp == DAT_LIST)
        {
            uchar *subp;
            uint subsiz;
            int firstwrd, lastwrd;

            /* get the sublist pointer, and read and skip the size prefix */
            subp = lstp + 1;
            subsiz = osrp2(subp) - 2;
            subp += 2;

            /* in case we don't find token indices, clear them */
            firstwrd = 0;
            lastwrd = 0;

            /* 
             *   the first two elements of the list are the token indices
             *   of the first and last words of this object's noun phrase 
             */
            if (subsiz > 0 && *subp == DAT_NUMBER)
            {
                /* read the first index, adjusting to zero-based indexing */
                firstwrd = (int)osrp4(subp+1) - 1;

                /* make sure it's in range */
                if (firstwrd < 0)
                    firstwrd = 0;
                else if (firstwrd > tokcnt)
                    firstwrd = tokcnt;

                /* skip it */
                lstadv(&subp, &subsiz);
            }
            if (subsiz > 0 && *subp == DAT_NUMBER)
            {
                /* read the last index, adjusting to zero-based indexing */
                lastwrd = (int)osrp4(subp+1) - 1;

                /* make sure it's in range */
                if (lastwrd < firstwrd)
                    lastwrd = firstwrd;
                else if (lastwrd > tokcnt)
                    lastwrd = tokcnt;

                /* skip it */
                lstadv(&subp, &subsiz);
            }

            /* scan the sublist */
            while (subsiz > 0)
            {
                /* if this is an object, store it */
                if (*subp == DAT_OBJECT || *subp == DAT_NIL)
                {
                    /* store the object */
                    if (*subp == DAT_OBJECT)
                        inlist[i].vocolobj = osrp2(subp+1);
                    else
                        inlist[i].vocolobj = MCMONINV;

                    /* set the first and last word pointers */
                    inlist[i].vocolfst = cmd[firstwrd];
                    inlist[i].vocollst = cmd[lastwrd];

                    /* skip the object */
                    lstadv(&subp, &subsiz);

                    /* check for flags */
                    if (subsiz > 0 && *subp == DAT_NUMBER)
                    {
                        /* get the flags value */
                        inlist[i].vocolflg = (int)osrp4(subp+1);

                        /* skip the number in the list */
                        lstadv(&subp, &subsiz);
                    }
                    else
                    {
                        /* clear the flags */
                        inlist[i].vocolflg = 0;
                    }

                    /* if an unknown word was involved, note it */
                    if ((inlist[i].vocolflg & VOCS_UNKNOWN) != 0)
                        ++unknown_count;

                    /* consume the element */
                    ++i;
                }
                else
                {
                    /* skip this element */
                    lstadv(&subp, &subsiz);
                }
            }
        }
    }

    /* terminate the list */
    inlist[i].vocolobj = MCMONINV;
    inlist[i].vocolflg = 0;

    /* 
     *   make a copy of our context, so the disambiguation can't make any
     *   global changes 
     */
    memcpy(&ctx_copy, ctx, sizeof(ctx_copy));

    /*
     *   Count the unknown words and set the count in the context.  This
     *   will allow us to determine after we call the resolver whether the
     *   resolution process cleared up the unknown words (via
     *   parseUnknownDobj/Iobj). 
     */
    ctx_copy.voccxunknown = ctx_copy.voccxlastunk = unknown_count;

    /* disambiguate the noun list */
    err = vocdisambig(&ctx_copy, outlist, inlist,
                      defprop, accprop, verprop, cmd,
                      otherobj, actor, verb, prep, cmdbuf, silent);

    /*
     *   If the error was VOCERR(2) - unknown word - check the input list
     *   to see if it contained any unknown words.  If it does, and we're
     *   not in "silent" mode, flag the error and then give the user a
     *   chance to use "oops" to correct the problem.  If we're in silent
     *   mode, don't display an error and don't allow interactive
     *   correction via "oops."
     *   
     *   It is possible that the unknown word is not in the input list,
     *   but in the user's response to an interactive disambiguation
     *   query.  This is why we must check to see if the unknown word is
     *   in the original input list or not.
     */
    if (err == VOCERR(2) && ctx_copy.voccxunknown != 0 && !silent)
    {
        char *unk;
		int unk_idx = 0;
        char *rpl_text;

        /* 
         *   forget we have unknown words, since we're going to handle
         *   them now
         */
        ctx_copy.voccxunknown = 0;

        /* 
         *   find the unknown word - look up each word until we find one
         *   that's not in the dictionary 
         */
        for (i = 0, unk = 0 ; cmd[i] != 0 ; ++i)
        {
            int t;
            
            /* 
             *   get this word's type - if the word has no type, it's an
             *   unknown word 
             */
            t = voc_lookup_type(ctx, cmd[i], strlen(cmd[i]), TRUE);
            if (t == 0)
            {
                /* this is it - note it and stop searching */
                unk_idx = i;
                unk = cmd[i];
                break;
            }
        }

        /* 
         *   if we didn't find any unknown words, assume the first word
         *   was unknown 
         */
        if (unk == 0)
        {
            unk_idx = 0;
            unk = cmd[0];
        }

        /* display an error, and read a new command */
        rpl_text = voc_read_oops(&ctx_copy, oopsbuf, VOCBUFSIZ, unk);

        /* 
         *   if they didn't respond with "oops," treat the response as a
         *   brand new command to replace the current command 
         */
        if (rpl_text == 0)
        {
            /* 
             *   it's a replacement command - set the redo flag to
             *   indicate that we should process the replacement command 
             */
            ctx_copy.voccxredo = TRUE;

            /* copy the response into the command buffer */
            strcpy(cmdbuf, oopsbuf);
        }
        else
        {
            /* indicate the correction via the result code */
            err = VOCERR(45);

            /* 
             *   Build the new command string.  The new command string
             *   consists of all of the tokens up to the unknown token,
             *   then the replacement text, then all of the remaining
             *   tokens. 
             */
            for (p = cmdbuf, i = 0 ; cmd[i] != 0 ; ++i)
            {
                size_t needed;
                
                /* figure the size needs for this token */
                if (i == unk_idx)
                {
                    /* we need to insert the replacement text */
                    needed = strlen(rpl_text);
                }
                else
                {
                    /* we need to insert this token string */
                    needed = strlen(cmd[i]);
                }
                
                /* 
                 *   if more tokens follow, we need a space after the
                 *   replacement text to separate it from what follows 
                 */
                if (cmd[i+1] != 0 && needed != 0)
                    needed += 1;

                /* leave room for null termination */
                needed += 1;

                /* if we don't have room for this token, stop now */
                if (needed > (size_t)(VOCBUFSIZ - (p - cmdbuf)))
                    break;

                /* 
                 *   if we've reached the unknown token, insert the
                 *   replacement text; otherwise, insert this token
                 */
                if (i == unk_idx)
                {
                    /* insert the replacement text */
                    strcpy(p, rpl_text);
                }
                else if (*cmd[i] == '"')
                {
                    char *p1;
                    char qu;
                    
                    /* 
                     *   Scan the quoted string for embedded double quotes
                     *   - if it has any, use single quotes as the
                     *   delimiter; otherwise, use double quotes as the
                     *   delimiter.  Note that we ignore the first and
                     *   last characters in the string, since these are
                     *   always the delimiting double quotes in the
                     *   original token text.  
                     */
                    for (qu = '"', p1 = cmd[i] + 1 ;
                         *p1 != '\0' && *(p1 + 1) != '\0' ; ++p1)
                    {
                        /* check for an embedded double quote */
                        if (*p1 == '"')
                        {
                            /* switch to single quotes as delimiters */
                            qu = '\'';

                            /* no need to look any further */
                            break;
                        }
                    }

                    /* add the open quote */
                    *p++ = qu;

                    /* 
                     *   add the text, leaving out the first and last
                     *   characters (which are the original quotes) 
                     */
                    if (strlen(cmd[i]) > 2)
                    {
                        memcpy(p, cmd[i] + 1, strlen(cmd[i]) - 2);
                        p += strlen(cmd[i]) - 2;
                    }

                    /* add the closing quote */
                    *p++ = qu;

                    /* null-terminate here so we don't skip any further */
                    *p = '\0';
                }
                else
                {
                    /* copy this word */
                    strcpy(p, cmd[i]);
                }

                /* move past this token */
                p += strlen(p);

                /* add a space if another token follows */
                if (cmd[i+1] != 0)
                    *p++ = ' ';
            }

            /* null-terminate the replacement text */
            *p = '\0';
        }
    }

    /* 
     *   Count the objects.  An object list is returned only on success or
     *   VOCERR(44), which indicates that the list is still ambiguous. 
     */
    if (err == 0 || err == VOCERR(44))
    {
        /* count the objects in the list */
        for (i = 0 ; outlist[i].vocolobj != MCMONINV ; ++i) ;
        objcnt = i;
    }
    else
    {
        /* there's nothing in the list */
        objcnt = 0;
    }

    /* figure out how much space we need for the objects */
    lstsiz = (1+2) * objcnt;
        
    /* add space for the first element, which contains the status code */
    lstsiz += (1 + 4);

    /* if there's a new command string, we'll store it, so make room */
    if (ctx_copy.voccxredo || err == VOCERR(45))
    {
        /* 
         *   add space for the type prefix (1), length prefix (2), and the
         *   string bytes (with no null terminator, of course) 
         */
        lstsiz += (1 + 2 + strlen(cmdbuf));

        /* 
         *   if we're retrying due to the redo flag, always return the
         *   RETRY error code, regardless of what caused us to retry the
         *   command 
         */
        if (ctx_copy.voccxredo)
            err = VOCERR(43);
    }

    /* push a list with space for the objects */
    lstp = voc_push_list_siz(ctx, lstsiz);

    /* store the status code in the first element */
    *lstp++ = DAT_NUMBER;
    oswp4s(lstp, err);
    lstp += 4;

    /* store the remainder of the list */
    if (err == 0 || err == VOCERR(44))
    {
        /* fill in the list with the objects */
        for (i = 0 ; i < objcnt ; ++i)
        {
            /* set this element */
            *lstp++ = DAT_OBJECT;
            oswp2(lstp, outlist[i].vocolobj);
            lstp += 2;
        }
    }
    else if (ctx_copy.voccxredo || err == VOCERR(45))
    {
        uint len;
        
        /* there's a new command - return it as the second element */
        *lstp++ = DAT_SSTRING;

        /* store the length */
        len = strlen(cmdbuf);
        oswp2(lstp, len + 2);
        lstp += 2;

        /* store the string */
        memcpy(lstp, cmdbuf, len);
    }

    /* leave the stack frame */
    voc_leave(ctx, save_sp);
}


/* ------------------------------------------------------------------------ */
/*
 *   TADS program code interface - replace the current command line with a
 *   new string, aborting the current command. 
 */
void voc_parse_replace_cmd(voccxdef *ctx)
{
    runcxdef *rcx = ctx->voccxrun;
    uchar *p;
    uint len;

    /* get the string */
    p = runpopstr(rcx);

    /* read and skip the length prefix */
    len = osrp2(p) - 2;
    p += 2;

    /* make sure it fits in the redo buffer - truncate it if necessary */
    if (len + 1 > VOCBUFSIZ)
        len = VOCBUFSIZ - 1;

    /* copy the string and null-terminate it */
    memcpy(ctx->voccxredobuf, p, len);
    ctx->voccxredobuf[len] = '\0';

    /* set the "redo" flag so that we execute what's in the buffer */
    ctx->voccxredo = TRUE;

    /* abort the current command so that we start anew with the replacement */
    errsig(ctx->voccxerr, ERR_RUNABRT);
}


/* ------------------------------------------------------------------------ */
/*
 *   This routine gets an actor, which is just a single object reference at
 *   the beginning of a sentence.  We return 0 if we fail to find an actor;
 *   since this can be either a harmless or troublesome condition, we must
 *   return additional information.  The method used to return back ERROR/OK
 *   is to set *next != cur if there is an error, *next == cur if not.  So,
 *   getting back (objdef*)0 means that you should check *next.  If the return
 *   value is nonzero, then that object is the actor.
 */
static objnum vocgetactor(voccxdef *ctx, char *cmd[], int typelist[],
                          int cur, int *next, char *cmdbuf)
{
    int       l;
    vocoldef *nounlist;
    vocoldef *actlist;
    int       cnt;
    uchar    *save_sp;
    prpnum    valprop, verprop;

    voc_enter(ctx, &save_sp);
    VOC_MAX_ARRAY(ctx, vocoldef, nounlist);
    VOC_MAX_ARRAY(ctx, vocoldef, actlist);
    
    *next = cur;                              /* assume no error will occur */
    cnt = vocchknoun(ctx, cmd, typelist, cur, next, nounlist, TRUE);
    if (cnt > 0 && *next != -1 && cmd[*next] && vocspec(cmd[*next], VOCW_AND))
    {
        int  have_unknown;

        /* make a note as to whether the list contains an unknown word */
        have_unknown = ((nounlist[0].vocolflg & VOCS_UNKNOWN) != 0);
        
        /*
         *   If validActor is defined for any of the actors, use it;
         *   otherwise, for compatibility with past versions, use the
         *   takeVerb disambiguation mechanism.  If we have a pronoun, we
         *   can't decide yet how to do this, so presume that we'll use
         *   the new mechanism and switch later if necessary.
         *   
         *   If we have don't have a valid object (which will be the case
         *   for a pronoun), we can't decide until we get into the
         *   disambiguation process, so presume we'll use validActor for
         *   now.  
         */
        verprop = PRP_VERACTOR;
        if (nounlist[0].vocolobj == MCMONINV
            || objgetap(ctx->voccxmem, nounlist[0].vocolobj, PRP_VALIDACTOR,
                        (objnum *)0, FALSE))
            valprop = PRP_VALIDACTOR;
        else
            valprop = PRP_VALIDDO;
        
        /* disambiguate it using the selected properties */
        if (vocdisambig(ctx, actlist, nounlist, PRP_DODEFAULT, valprop,
                        verprop, cmd, MCMONINV, ctx->voccxme,
                        ctx->voccxvtk, MCMONINV, cmdbuf, FALSE))
        {
            /* 
             *   if we have an unknown word in the list, assume for the
             *   moment that this isn't an actor phrase after all, but a
             *   verb 
             */
            if (have_unknown)
            {
                /* indicate that we didn't find a noun phrase syntactically */
                *next = cur;
            }

            /* return no actor */
            VOC_RETVAL(ctx, save_sp, MCMONINV);
        }

        if ((l = voclistlen(actlist)) > 1)
        {
            vocerr(ctx, VOCERR(12),
                   "You can only speak to one person at a time.");
            *next = cur + 1;   /* error flag - return invalid but next!=cur */
            VOC_RETVAL(ctx, save_sp, MCMONINV);
        }
        else if (l == 0)
            return(MCMONINV);

        if (cmd[*next] && vocspec(cmd[*next], VOCW_AND))
        {
            ++(*next);
            VOC_RETVAL(ctx, save_sp, actlist[0].vocolobj);
        }
    }
    if (cnt < 0)
    {
        /* error - make *next != cur */
        *next = cur + 1;
    }
    else
        *next = cur;               /* no error condition, but nothing found */

    VOC_RETVAL(ctx, save_sp, MCMONINV);    /* so return invalid and *next == cur */
}

/* figure out how many objects are in an object list */
int voclistlen(vocoldef *lst)
{
    int i;
    
    for (i = 0 ; lst->vocolobj != MCMONINV || lst->vocolflg != 0 ;
         ++lst, ++i) ;
    return(i);
}

/*
 *   check access - evaluates cmdVerb.verprop(actor, obj, seqno), and
 *   returns whatever it returns.  The seqno parameter is used for special
 *   cases, such as "ask", when the validation routine wishes to return
 *   "true" on the first object and "nil" on all subsequent objects which
 *   correspond to a particular noun phrase.  We expect to be called with
 *   seqno==0 on the first object, non-zero on others; we will pass
 *   seqno==1 on the first object to the validation property, higher on
 *   subsequent objects, to maintain consistency with the TADS language
 *   convention of indexing from 1 up (as seen by the user in indexing
 *   functions).  Note that if we're checking an actor, we'll just call
 *   obj.validActor() for the object itself (not the verb).
 */
int vocchkaccess(voccxdef *ctx, objnum obj, prpnum verprop,
                 int seqno, objnum cmdActor, objnum cmdVerb)
{
    /* 
     *   special case: the special "string" and "number" objects are
     *   always accessible 
     */
    if (obj == ctx->voccxstr || obj == ctx->voccxnum)
        return TRUE;

    /*
     *   If the access method is validActor, make sure the object in fact
     *   has a validActor method defined; if it doesn't, we must be
     *   running a game from before validActor's invention, so use the old
     *   ValidXo mechanism instead. 
     */
    if (verprop == PRP_VALIDACTOR)
    {
        /* checking an actor - check to see if ValidActor is defined */
        if (objgetap(ctx->voccxmem, obj, PRP_VALIDACTOR, (objnum *)0, FALSE))
        {
            /* ValidActor is present - call ValidActor in the object itself */
            runppr(ctx->voccxrun, obj, verprop, 0);

            /* return the result */
            return runpoplog(ctx->voccxrun);
        }
        else
        {
            /* there's no ValidActor - call ValidXo in the "take" verb */
            cmdVerb = ctx->voccxvtk;
            verprop = PRP_VALIDDO;
        }
    }

    /* call ValidXo in the verb */
    runpnum(ctx->voccxrun, (long)(seqno + 1));
    runpobj(ctx->voccxrun, obj);
    runpobj(ctx->voccxrun,
            (objnum)(cmdActor == MCMONINV ? ctx->voccxme : cmdActor));
    runppr(ctx->voccxrun, cmdVerb, verprop, 3);

    /* return the result */
    return runpoplog(ctx->voccxrun);
}

/* ask game if object is visible to the actor */
int vocchkvis(voccxdef *ctx, objnum obj, objnum cmdActor)
{
    runpobj(ctx->voccxrun,
            (objnum)(cmdActor == MCMONINV ? ctx->voccxme : cmdActor));
    runppr(ctx->voccxrun, obj, (prpnum)PRP_ISVIS, 1);
    return(runpoplog(ctx->voccxrun));
}

/* set {numObj | strObj}.value, as appropriate */
void vocsetobj(voccxdef *ctx, objnum obj, dattyp typ, const void *val,
               vocoldef *inobj, vocoldef *outobj)
{
    *outobj = *inobj;
    outobj->vocolobj = obj;
    objsetp(ctx->voccxmem, obj, PRP_VALUE, typ, val, ctx->voccxundo);
}

/* set up a vocoldef */
static void vocout(vocoldef *outobj, objnum obj, int flg,
                   const char *fst, const char *lst)
{
    outobj->vocolobj = obj;
    outobj->vocolflg = flg;
    outobj->vocolfst = fst;
    outobj->vocollst = lst;
}

/*
 *   Generate an appropriate error message saying that the objects in the
 *   command are visible, but can't be used with the command for some
 *   reason.  Use the cantReach method of the verb (the new way), or if
 *   there is no cantReach in the verb, of each object in the list. 
 */
void vocnoreach(voccxdef *ctx, objnum *list1, int cnt,
                objnum actor, objnum verb, objnum prep, prpnum defprop,
                int show_multi_prefix,
                int multi_flags, int multi_base_index, int multi_total_count)
{
    /* see if the verb has a cantReach method - use it if so */
    if (objgetap(ctx->voccxmem, verb, PRP_NOREACH, (objnum *)0, FALSE))
    {
        /* push arguments:  (actor, dolist, iolist, prep) */
        runpobj(ctx->voccxrun, prep);
        if (defprop == PRP_DODEFAULT)
        {
            runpnil(ctx->voccxrun);
            voc_push_objlist(ctx, list1, cnt);
        }
        else
        {
            voc_push_objlist(ctx, list1, cnt);
            runpnil(ctx->voccxrun);
        }
        runpobj(ctx->voccxrun, actor);

        /* invoke the method in the verb */
        runppr(ctx->voccxrun, verb, PRP_NOREACH, 4);
    }
    else
    {
        int  i;

        /* use the old way - call obj.cantReach() for each object */
        for (i = 0 ; i < cnt ; ++i)
        {
            /* 
             *   display this object's name if there's more than one, so
             *   that the player can tell to which object this message
             *   applies 
             */
            voc_multi_prefix(ctx, list1[i], show_multi_prefix, multi_flags,
                             multi_base_index + i, multi_total_count);

            /* call cantReach on the object */
            runpobj(ctx->voccxrun,
                    (objnum)(actor == MCMONINV ? ctx->voccxme : actor));
            runppr(ctx->voccxrun, list1[i], (prpnum)PRP_NOREACH, 1);
            tioflush(ctx->voccxtio);
        }
    }
}

/*
 *   Get the specialWords string for a given special word entry.  Returns
 *   the first string if multiple strings are defined for the entry.  
 */
static void voc_get_spec_str(voccxdef *ctx, char vocw_id,
                             char *buf, size_t buflen,
                             const char *default_name)
{
    int found;

    /* presume we won't find it */
    found = FALSE;

    /* if there's a special word list, search it for this entry */
    if (ctx->voccxspp != 0)
    {
        char   *p;
        char   *endp;
        size_t  len;

        /* find appropriate user-defined word in specialWords list */
        for (p = ctx->voccxspp, endp = p + ctx->voccxspl ; p < endp ; )
        {
            /* if this one matches, get its first word */
            if (*p++ == vocw_id)
            {
                /* note that we found it */
                found = TRUE;

                /* 
                 *   get the length, and limit it to the buffer size,
                 *   leaving room for null termination 
                 */
                len = *p++;
                if (len + 1 > buflen)
                    len = buflen - 1;

                /* copy it and null-terminate the string */
                memcpy(buf, p, len);
                buf[len] = '\0';

                /* we found it - no need to look any further */
                break;
            }

            /* 
             *   move on to the next one - skip the length prefix plus the
             *   length 
             */
            p += *p + 1;
        }
    }

    /* if we didn't find it, use the default */
    if (!found)
    {
        strncpy(buf, default_name, (size_t)buflen);
        buf[buflen - 1] = '\0';
    }
}

/* set it/him/her */
static int vocsetit(voccxdef *ctx, objnum obj, int accprop,
                    objnum actor, objnum verb, objnum prep,
                    vocoldef *outobj, const char *default_name, char vocw_id,
                    prpnum defprop, int silent)
{
    if (obj == MCMONINV || !vocchkaccess(ctx, obj, (prpnum)accprop,
                                         0, actor, verb))
    {
        char nambuf[40];

        /* get the display name for this specialWords entry */
        voc_get_spec_str(ctx, vocw_id, nambuf, sizeof(nambuf), default_name);
        
        /* show the error if appropriate */
        if (!silent)
        {
            /* use 'noreach' if possible, otherwise use a default message */
            if (obj == MCMONINV)
                vocerr(ctx, VOCERR(13),
                       "I don't know what you're referring to with '%s'.",
                       nambuf);
            else
                vocnoreach(ctx, &obj, 1, actor, verb, prep,
                           defprop, FALSE, 0, 0, 1);
        }

        /* indicate that the antecedent is inaccessible */
        return VOCERR(13);
    }

    /* set the object */
    vocout(outobj, obj, 0, default_name, default_name);
    return 0;
}

/*
 *   Get a new numbered object, given a number.  This is used for objects
 *   that define '#' as one of their adjectives; we call the object,
 *   asking it to create an object with a particular number.  The object
 *   can return nil, in which case we'll reject the command.  
 */
static objnum voc_new_num_obj(voccxdef *ctx, objnum objn,
                              objnum actor, objnum verb,
                              long num, int plural)
{
    /* push the number - if we need a plural object, use nil instead */
    if (plural)
        runpnil(ctx->voccxrun);
    else
        runpnum(ctx->voccxrun, num);

    /* push the other arguments and call the method */
    runpobj(ctx->voccxrun, verb);
    runpobj(ctx->voccxrun, actor);
    runppr(ctx->voccxrun, objn, PRP_NEWNUMOBJ, 3);

    /* if it was rejected, return an invalid object, else return the object */
    if (runtostyp(ctx->voccxrun) == DAT_NIL)
    {
        rundisc(ctx->voccxrun);
        return MCMONINV;
    }
    else
        return runpopobj(ctx->voccxrun);
}

/* check if an object defines the special adjective '#' */
static int has_gen_num_adj(voccxdef *ctx, objnum objn)
{
    vocwdef   *v;
    vocseadef  search_ctx;

    /* scan the list of objects defined '#' as an adjective */
    for (v = vocffw(ctx, "#", 1, (char *)0, 0, PRP_ADJ, &search_ctx) ;
         v ; v = vocfnw(ctx, &search_ctx))
    {
        /* if this is the object, return positive indication */
        if (v->vocwobj == objn)
            return TRUE;
    }

    /* didn't find it */
    return FALSE;
}


/* ------------------------------------------------------------------------ */
/*
 *   Call the deepverb's disambigDobj or disambigIobj method to perform
 *   game-controlled disambiguation.  
 */
static int voc_disambig_hook(voccxdef *ctx, objnum verb, objnum actor,
                             objnum prep, objnum otherobj,
                             prpnum accprop, prpnum verprop,
                             objnum *objlist, uint *flags, int *objcount,
                             const char *firstwrd, const char *lastwrd,
                             int num_wanted, int is_ambig, char *resp,
                             int silent)
{
    runcxdef *rcx = ctx->voccxrun;
    prpnum call_prop;
    runsdef val;
    uchar *lstp;
    uint lstsiz;
    int ret;
    int i;
    
    /* check for actor disambiguation */
    if (verprop == PRP_VERACTOR)
    {
        /* do nothing on actor disambiguation */
        return VOC_DISAMBIG_CONT;
    }

    /* figure out whether this is a dobj method or an iobj method */
    call_prop = (accprop == PRP_VALIDDO ? PRP_DISAMBIGDO : PRP_DISAMBIGIO);

    /* if the method isn't defined, we can skip this entirely */
    if (objgetap(ctx->voccxmem, verb, call_prop, (objnum *)0, FALSE) == 0)
        return VOC_DISAMBIG_CONT;

    /* push the "silent" flag */
    val.runstyp = (silent ? DAT_TRUE : DAT_NIL);
    runpush(rcx, val.runstyp, &val);

    /* push the "is_ambiguous" flag */
    val.runstyp = (is_ambig ? DAT_TRUE : DAT_NIL);
    runpush(rcx, val.runstyp, &val);

    /* push the "numWanted" count */
    runpnum(rcx, num_wanted);

    /* push the flag list */
    voc_push_numlist(ctx, flags, *objcount);

    /* push the object list */
    voc_push_objlist(ctx, objlist, *objcount);

    /* push the word list */
    voc_push_strlist(ctx, firstwrd, lastwrd);

    /* push the verification property */
    val.runstyp = DAT_PROPNUM;
    val.runsv.runsvprp = verprop;
    runpush(rcx, DAT_PROPNUM, &val);

    /* push the other object */
    runpobj(rcx, otherobj);

    /* push the preposition and the actor objects */
    runpobj(rcx, prep);
    runpobj(rcx, actor);

    /* call the method */
    runppr(rcx, verb, call_prop, 10);

    /* check the return value */
    switch(runtostyp(rcx))
    {
    case DAT_LIST:
        /* get the list */
        lstp = runpoplst(rcx);

        /* read the list size prefix */
        lstsiz = osrp2(lstp) - 2;
        lstp += 2;

        /* check for the status code */
        if (lstsiz > 0 && *lstp == DAT_NUMBER)
        {
            /* get the status code */
            ret = osrp4s(lstp+1);

            /* skip the element */
            lstadv(&lstp, &lstsiz);
        }
        else
        {
            /* there's no status code - assume CONTINUE */
            ret = VOC_DISAMBIG_CONT;
        }

        /* check for a PARSE_RESP return */
        if (ret == VOC_DISAMBIG_PARSE_RESP)
        {
            /* the second element is the string */
            if (*lstp == DAT_SSTRING)
            {
                uint len;
                
                /* get the length, and limit it to our buffer size */
                len = osrp2(lstp+1) - 2;
                if (len > VOCBUFSIZ - 1)
                    len = VOCBUFSIZ - 1;

                /* copy the string into the caller's buffer */
                memcpy(resp, lstp+3, len);
                resp[len] = '\0';
            }
            else
            {
                /* there's no string - ignore it */
                ret = VOC_DISAMBIG_CONT;
            }
        }
        else
        {
            /* store the object list in the caller's list */
            for (i = 0 ; lstsiz > 0 && i < VOCMAXAMBIG-1 ; ++i)
            {
                /* get this object */
                if (*lstp == DAT_OBJECT)
                    objlist[i] = osrp2(lstp+1);
                else
                    objlist[i] = MCMONINV;
                
                /* skip the list entry */
                lstadv(&lstp, &lstsiz);
                
                /* check for flags */
                if (lstsiz > 0 && *lstp == DAT_NUMBER)
                {
                    /* store the flags */
                    flags[i] = (int)osrp4(lstp+1);
                    
                    /* skip the flags elements */
                    lstadv(&lstp, &lstsiz);
                }
                else
                {
                    /* no flags - use zero by default */
                    flags[i] = 0;
                }
            }

            /* store a terminator at the end of the list */
            objlist[i] = MCMONINV;
            flags[i] = 0;
            
            /* store the output count for the caller */
            *objcount = i;
        }

        /* return the result */
        return ret;

    case DAT_NUMBER:
        /* get the status code */
        ret = runpopnum(rcx);

        /* ignore raw PARSE_RESP codes, since they need to return a string */
        if (ret == VOC_DISAMBIG_PARSE_RESP)
            ret = VOC_DISAMBIG_CONT;

        /* return the status */
        return ret;

    default:
        /* treat anything else as CONTINUE */
        rundisc(rcx);
        return VOC_DISAMBIG_CONT;
    }
}


/* ------------------------------------------------------------------------ */
/*
 *   Prune a list of matches by keeping only the matches without the given
 *   flag value, if we have a mix of entries with and without the flag.
 *   This is a service routine for voc_prune_matches.
 *   
 *   The flag indicates a lower quality of matching, so this routine can
 *   be used to reduce ambiguity by keeping only the best-quality matches
 *   when matches of mixed quality are present.  
 */
static int voc_remove_objs_with_flag(voccxdef *ctx,
                                     objnum *list, uint *flags, int cnt,
                                     int flag_to_remove)
{
    int i;
    int flag_cnt;
    int special_cnt;

    /* first, count the number of objects with the flag */
    for (i = 0, flag_cnt = special_cnt = 0 ; i < cnt ; ++i)
    {
        /* if this object exhibits the flag, count it */
        if ((flags[i] & flag_to_remove) != 0)
            ++flag_cnt;

        /* if it's numObj or strObj, count it separately */
        if (list[i] == ctx->voccxnum || list[i] == ctx->voccxstr)
            ++special_cnt;
    }

    /* 
     *   If all of the objects didn't have the flag, omit the ones that
     *   did, so that we reduce the ambiguity to those without the flag.
     *   Don't include the special objects (numObj and strObj) in the
     *   count, since they will never have any of these flags set.  
     */
    if (flag_cnt != 0 && flag_cnt < cnt - special_cnt)
    {
        int dst;

        /* 
         *   Remove the flagged objects.  Note that we can make this
         *   adjustment to the arrays in place, because they can only
         *   shrink - there's no need to make an extra temporary copy.  
         */
        for (i = 0, dst = 0 ; i < cnt ; ++i)
        {
            /* 
             *   If this one doesn't have the flag, keep it.  Always keep
             *   the special objects (numObj and strObj). 
             */
            if ((flags[i] & flag_to_remove) == 0
                || list[i] == ctx->voccxnum
                || list[i] == ctx->voccxstr)
            {
                /* copy this one to the output location */
                list[dst] = list[i];
                flags[dst] = flags[i];

                /* count the new element of the output */
                ++dst;
            }
        }

        /* set the updated count */
        cnt = dst;
        list[cnt] = MCMONINV;
    }

    /* return the new count */
    return cnt;
}

/*
 *   Prune a list of matches by keeping only the best matches when matches
 *   of different qualities are present.
 *   
 *   If we have a mix of objects matching noun phrases that end in
 *   adjectives and phrases ending in nouns with the same words, remove
 *   those elements that end in adjectives, keeping only the better
 *   matches that end in nouns.
 *   
 *   If we have a mix of objects where the words match exactly, and others
 *   where the words are only leading substrings of longer dictionary
 *   words, keep only the exact matches.
 *   
 *   Returns the number of elements in the result list.  
 */
static int voc_prune_matches(voccxdef *ctx,
                             objnum *list, uint *flags, int cnt)
{
    /* remove matches that end with an adjective */
    cnt = voc_remove_objs_with_flag(ctx, list, flags, cnt, VOCS_ENDADJ);

    /* remove matches that use truncated words */
    cnt = voc_remove_objs_with_flag(ctx, list, flags, cnt, VOCS_TRUNC);

    /* return the new list size */
    return cnt;
}

/* ------------------------------------------------------------------------ */
/*
 *   Count indistinguishable items.
 *   
 *   If 'keep_all' is true, we'll keep all of the items, whether or not
 *   some are indistinguishable from one another.  If 'keep_all' is false,
 *   we'll keep only one item from each set of indistinguishable items. 
 */
static int voc_count_diff(voccxdef *ctx, objnum *list, uint *flags, int *cnt,
                          int keep_all)
{
    int i;
    int diff_cnt;
    
    /* 
     *   Presume all items will be distinguishable from one another.  As
     *   we scan the list for indistinguishable items, we'll decrement
     *   this count each time we find an item that can't be distinguished
     *   from another item.  
     */
    diff_cnt = *cnt;

    /* 
     *   Look for indistinguishable items.
     *   
     *   An object is distinguishable if it doesn't have the special
     *   property marking it as one of a group of equivalent objects
     *   (PRP_ISEQUIV), or if it has the property but there is no object
     *   following it in the list which has the same immediate superclass.
     *   
     *   Note that we want to keep the duplicates if we're looking for
     *   plurals, because the player is explicitly referring to all
     *   matching objects.  
     */
    for (i = 0 ; i < *cnt ; ++i)
    {
        /* 
         *   check to see if this object might have indistinguishable
         *   duplicates - it must be marked with isEquiv for this to be
         *   possible 
         */
        runppr(ctx->voccxrun, list[i], PRP_ISEQUIV, 0);
        if (runpoplog(ctx->voccxrun))
        {
            int     j;
            int     dst;
            objnum  sc;
            
            /* get the superclass, if possible */
            sc = objget1sc(ctx->voccxmem, list[i]);
            if (sc == MCMONINV)
                continue;
            
            /* 
             *   run through the remainder of the list, and remove any
             *   duplicates of this item 
             */
            for (j = i + 1, dst = i + 1 ; j < *cnt ; ++j)
            {
                /* 
                 *   see if it matches our object - if not, keep it in the
                 *   list by copying it to our destination position 
                 */
                if (objget1sc(ctx->voccxmem, list[j]) != sc)
                {
                    /* it's distinguishable - keep it */
                    list[dst] = list[j];
                    flags[dst++] = flags[j];
                }
                else
                {
                    /*
                     *   This item is indistinguishable from the list[i].
                     *   First, reduce the count of different items.  
                     */
                    --diff_cnt;
                    
                    /*
                     *   Keep this object only if we're keeping all
                     *   redundant indistinguishable items. 
                     */
                    if (keep_all)
                    {
                        /* keep all items -> keep this item */
                        list[dst] = list[j];
                        flags[dst++] = flags[j];
                    }
                }
            }
            
            /* adjust the count to reflect the updated list */
            *cnt = dst;

            /* add a terminator */
            list[dst] = MCMONINV;
            flags[dst] = 0;
        }
    }

    /* return the number of distinguishable items */
    return diff_cnt;
}

/* ------------------------------------------------------------------------ */
/*
 *   vocdisambig - determines which nouns in a noun list apply.  When this
 *   is called, we must know the verb that we are processing, so we delay
 *   disambiguation until quite late in the parsing of a sentence, opting
 *   to keep all relevant information around until such time as we can
 *   meaningfully disambiguate.
 *
 *   This routine resolves any "all [except...]", "it", and "them"
 *   references.  We determine if all of the objects listed are accessible
 *   (via verb.validDo, verb.validIo).  We finally try to determine which
 *   nouns apply when there are ambiguous nouns by using do.verDo<Verb>
 *   and io.verIo<Verb>.
 */
int vocdisambig(voccxdef *ctx, vocoldef *outlist, vocoldef *inlist,
                prpnum defprop, prpnum accprop, prpnum verprop,
                char *cmd[], objnum otherobj, objnum cmdActor,
                objnum cmdVerb, objnum cmdPrep, char *cmdbuf,
                int silent)
{
    int       inpos;
    int       outpos;
    int       listlen = voclistlen(inlist);
    int       noreach = FALSE;
    prpnum    listprop;
    uchar    *save_sp;
    int       old_unknown, old_lastunk;
    int       err;
    int       still_ambig;
	static char one_name[] = "ones";

    voc_enter(ctx, &save_sp);

    ERRBEGIN(ctx->voccxerr)

    /* presume we will not leave any ambiguity in the result */
    still_ambig = FALSE;

    /* loop through all of the objects in the input list */
    for (inpos = outpos = 0 ; inpos < listlen ; ++inpos)
    {
        /* 
         *   reset the stack to our entrypoint value, since our stack
         *   variables are all temporary for a single iteration 
         */
        voc_leave(ctx, save_sp);
        voc_enter(ctx, &save_sp);

        if (inlist[inpos].vocolflg == VOCS_STR)
        {
            vocsetobj(ctx, ctx->voccxstr, DAT_SSTRING,
                      inlist[inpos].vocolfst + 1,
                      &inlist[inpos], &outlist[outpos]);
            ++outpos;
        }
        else if (inlist[inpos].vocolflg == VOCS_NUM)
        {
            long v1;
            char vbuf[4];
            
            v1 = atol(inlist[inpos].vocolfst);
            oswp4s(vbuf, v1);
            vocsetobj(ctx, ctx->voccxnum, DAT_NUMBER, vbuf,
                      &inlist[inpos], &outlist[outpos]);
            ++outpos;
        }
        else if (inlist[inpos].vocolflg == VOCS_IT ||
                 (inlist[inpos].vocolflg == VOCS_THEM && ctx->voccxthc == 0))
        {
            err = vocsetit(ctx, ctx->voccxit, accprop, cmdActor,
                           cmdVerb, cmdPrep, &outlist[outpos],
                           inlist[inpos].vocolflg == VOCS_IT ? "it" : "them",
                           (char)(inlist[inpos].vocolflg == VOCS_IT
                                  ? VOCW_IT : VOCW_THEM), defprop, silent);
            if (err != 0)
                goto done;
            ++outpos;
        }
        else if (inlist[inpos].vocolflg == VOCS_HIM)
        {
            err = vocsetit(ctx, ctx->voccxhim, accprop, cmdActor, cmdVerb,
                           cmdPrep, &outlist[outpos], "him", VOCW_HIM,
                           defprop, silent);
            if (err != 0)
                goto done;
            ++outpos;
        }
        else if (inlist[inpos].vocolflg == VOCS_HER)
        {
            err = vocsetit(ctx, ctx->voccxher, accprop, cmdActor, cmdVerb,
                           cmdPrep, &outlist[outpos], "her", VOCW_HER,
                           defprop, silent);
            if (err != 0)
                goto done;
            ++outpos;
        }
        else if (inlist[inpos].vocolflg == VOCS_THEM)
        {
            int i;
            int thempos = outpos;
            static char them_name[] = "them";

            for (i = 0 ; i < ctx->voccxthc ; ++i)
            {
                if (outpos >= VOCMAXAMBIG)
                {
                    if (!silent)
                        vocerr(ctx, VOCERR(11),
                               "You're referring to too many objects.");
                    err = VOCERR(11);
                    goto done;
                }
                
                /* add object only if it's still accessible */
                if (vocchkaccess(ctx, ctx->voccxthm[i], accprop, 0,
                                 cmdActor, cmdVerb))
                {
                    /* it's still accessible - add it to the list */
                    vocout(&outlist[outpos++], ctx->voccxthm[i], VOCS_THEM,
                           them_name, them_name);
                }
                else
                {
                    /* it's not accessible - complain about it */
                    vocnoreach(ctx, &ctx->voccxthm[i], 1,
                               cmdActor, cmdVerb, cmdPrep,
                               defprop, TRUE, VOCS_THEM, i, ctx->voccxthc);
                    tioflush(ctx->voccxtio);
                }
            }
            
            /* make sure we found at least one acceptable object  */
            if (outpos == thempos)
            {
                if (!silent)
                    vocerr(ctx, VOCERR(14),
                           "I don't know what you're referring to.");
                err = VOCERR(14);
                goto done;
            }
        }
        else if (inlist[inpos].vocolflg == VOCS_ALL)
        {
            uchar    *l;
            int       exccnt = 0;
            int       allpos = outpos;
            int       k;
            uint      len;
            static    char all_name[] = "all";
            vocoldef *exclist;
            vocoldef *exclist2;

            VOC_MAX_ARRAY(ctx, vocoldef, exclist);
            VOC_MAX_ARRAY(ctx, vocoldef, exclist2);

            if (defprop != PRP_IODEFAULT)
                runpobj(ctx->voccxrun, otherobj);
            runpobj(ctx->voccxrun, cmdPrep);
            runpobj(ctx->voccxrun, cmdActor);
            runppr(ctx->voccxrun, cmdVerb, defprop,
                   defprop == PRP_DODEFAULT ? 3 : 2);
            
            if (runtostyp(ctx->voccxrun) == DAT_LIST)
            {
                l = runpoplst(ctx->voccxrun);
                len = osrp2(l) - 2;
                l += 2;
                
                while (len)
                {
                    /* add list element to output if it's an object */
                    if (*l == DAT_OBJECT)
                        vocout(&outlist[outpos++], (objnum)osrp2(l+1), 0,
                               all_name, all_name);

                    /* move on to next list element */
                    lstadv(&l, &len);
                }
                
                vocout(&outlist[outpos], MCMONINV, 0, (char *)0, (char *)0);
            }
            else
                rundisc(ctx->voccxrun);           /* discard non-list value */

            /* if we didn't get anything, complain about it and quit */
            if (outpos <= allpos)
            {
                if (!silent)
                    vocerr(ctx, VOCERR(15),
                           "I don't see what you're referring to.");
                err = VOCERR(15);
                goto done;
            }

            /* remove any items in "except" list */
            while (inlist[inpos + 1].vocolflg & VOCS_EXCEPT)
            {
                OSCPYSTRUCT(exclist[exccnt], inlist[++inpos]);
                exclist[exccnt++].vocolflg &= ~VOCS_EXCEPT;
            }
            exclist[exccnt].vocolobj = MCMONINV;
            exclist[exccnt].vocolflg = 0;

            /* disambiguate "except" list */
            if (exccnt)
            {
                err = vocdisambig(ctx, exclist2, exclist, defprop, accprop,
                                  verprop, cmd, otherobj, cmdActor,
                                  cmdVerb, cmdPrep, cmdbuf, silent);
                if (err != 0)
                    goto done;

                exccnt = voclistlen(exclist2);
                for (k = 0 ; k < exccnt ; ++k)
                {
                    int i;
                    for (i = allpos ; i < outpos ; ++i)
                    {
                        if (outlist[i].vocolobj == exclist2[k].vocolobj)
                        {
                            int j;
                            for (j = i ; j < outpos ; ++j)
                                outlist[j].vocolobj = outlist[j+1].vocolobj;
                            --i;
                            --outpos;
                            if (outpos <= allpos)
                            {
                                if (!silent)
                                    vocerr(ctx,  VOCERR(15),
                                      "I don't see what you're referring to.");
                                err = VOCERR(15);
                                goto done;
                            }
                        }
                    }
                }
            }
        }
        else                         /* we have a (possibly ambiguous) noun */
        {
            int       lpos = inpos;
            int       i = 0;
            int       cnt;
            const char *p;
            int       cnt2, cnt3;
            int       trying_again;
            int       user_count = 0;
            objnum   *cantreach_list;
            int       unknown_count;
            int       use_all_objs;
            objnum   *list1;
            uint     *flags1;
            objnum   *list2;
            uint     *flags2;
            objnum   *list3;
            uint     *flags3;
            char     *usrobj;
            uchar    *lstbuf;
            char     *newobj;
            char     *disnewbuf;
            char     *disbuffer;
            char    **diswordlist;
            int      *distypelist;
            vocoldef *disnounlist;
            int       dst;

            VOC_MAX_ARRAY(ctx, objnum,   list1);
            VOC_MAX_ARRAY(ctx, objnum,   list2);
            VOC_MAX_ARRAY(ctx, objnum,   list3);
            VOC_MAX_ARRAY(ctx, uint,     flags1);
            VOC_MAX_ARRAY(ctx, uint,     flags2);
            VOC_MAX_ARRAY(ctx, uint,     flags3);
            VOC_MAX_ARRAY(ctx, vocoldef, disnounlist);
            VOC_STK_ARRAY(ctx, char,     disnewbuf,   VOCBUFSIZ);
            VOC_STK_ARRAY(ctx, char,     disbuffer,   2*VOCBUFSIZ);
            VOC_STK_ARRAY(ctx, char *,   diswordlist, VOCBUFSIZ);
            VOC_STK_ARRAY(ctx, int,      distypelist, VOCBUFSIZ);
            VOC_STK_ARRAY(ctx, char,     usrobj,      VOCBUFSIZ);
            VOC_STK_ARRAY(ctx, char,     newobj,      VOCBUFSIZ);
            VOC_STK_ARRAY(ctx, uchar,    lstbuf,      2 + VOCMAXAMBIG*3);

            /* presume we won't resolve any unknown words */
            unknown_count = 0;

            /*
             *   Presume that we won't use all the objects that match
             *   these words, since we normally want to try to find a
             *   single, unambiguous match for a given singular noun
             *   phrase.  Under certain circumstances, we'll want to keep
             *   all of the words that match the noun phrase, in which
             *   case we'll set this flag accordingly. 
             */
            use_all_objs = FALSE;

            /* 
             *   go through the objects matching the current noun phrase
             *   and add them into our list 
             */
            while (inlist[lpos].vocolfst == inlist[inpos].vocolfst
                   && lpos < listlen)
            {
                /* add this object to the list of nouns */
                list1[i] = inlist[lpos].vocolobj;

                /* 
                 *   note whether this object matched a plural, whether it
                 *   matched adjective-at-end usage, and whether it
                 *   matched a truncated dictionary word 
                 */
                flags1[i] = inlist[lpos].vocolflg
                    & (VOCS_PLURAL | VOCS_ANY | VOCS_COUNT
                       | VOCS_ENDADJ | VOCS_TRUNC);

                /* if this is a valid object, count it */
                if (list1[i] != MCMONINV)
                    ++i;

                /* if there's a user count, note it */
                if ((inlist[lpos].vocolflg & VOCS_COUNT) != 0)
                    user_count = atoi(inlist[lpos].vocolfst);

                /* if an unknown word was involved, note it */
                if ((inlist[lpos].vocolflg & VOCS_UNKNOWN) != 0)
                    ++unknown_count;

                /* move on to the next entry */
                ++lpos;
            }

            /* terminate the list */
            list1[i] = MCMONINV;
            cnt = i;

            /*
             *   If this noun phrase contained an unknown word, check to
             *   see if the verb defines the parseUnknownXobj() method.
             *   If so, call the method and check the result. 
             */
            if (unknown_count > 0)
            {
                prpnum prp;

                /* 
                 *   figure out which method to call - use
                 *   parseUnknownDobj if we're disambiguating the direct
                 *   object, parseUnknownIobj for the indirect object 
                 */
                prp = (defprop == PRP_DODEFAULT
                       ? PRP_PARSEUNKNOWNDOBJ : PRP_PARSEUNKNOWNIOBJ);

                /* check if the verb defines this method */
                if (objgetap(ctx->voccxmem, cmdVerb, prp, (objnum *)0, FALSE))
                {
                    uchar *lstp;
                    uint lstlen;

                    /* trace the event for debugging */
                    if (ctx->voccxflg & VOCCXFDBG)
                        tioputs(ctx->voccxtio,
                                "... unknown word: calling "
                                "parseUnknownXobj\\n");

                    /* push the list of words in the noun phrase */
                    voc_push_strlist(ctx, inlist[inpos].vocolfst,
                                     inlist[inpos].vocollst);

                    /* push the other arguments */
                    runpobj(ctx->voccxrun, otherobj);
                    runpobj(ctx->voccxrun, cmdPrep);
                    runpobj(ctx->voccxrun, cmdActor);

                    /* call the method */
                    runppr(ctx->voccxrun, cmdVerb, prp, 4);

                    /* see what they returned */
                    switch(runtostyp(ctx->voccxrun))
                    {
                    case DAT_OBJECT:
                        /* 
                         *   use the object they returned as the match for
                         *   the noun phrase 
                         */
                        list1[cnt++] = runpopobj(ctx->voccxrun);

                        /* terminate the new list */
                        list1[cnt] = MCMONINV;
                        break;

                    case DAT_LIST:
                        /*
                         *   use the list of objects they returned as the
                         *   match for the noun phrase 
                         */
                        lstp = runpoplst(ctx->voccxrun);

                        /* get the length of the list */
                        lstlen = osrp2(lstp) - 2;
                        lstp += 2;

                        /* run through the list's elements */
                        while (lstlen != 0)
                        {
                            /* if this is an object, add it */
                            if (*lstp == DAT_OBJECT
                                && i < VOCMAXAMBIG)
                                list1[cnt++] = osrp2(lstp+1);
                            
                            /* move on to the next element */
                            lstadv(&lstp, &lstlen);
                        }

                        /* 
                         *   Note that we want to use all of these objects
                         *   without disambiguation, since the game code
                         *   has explicitly said that this is the list
                         *   that matches the given noun phrase. 
                         */
                        use_all_objs = TRUE;
                        
                        /* terminate the new list */
                        list1[cnt] = MCMONINV;
                        break;

                    case DAT_TRUE:
                        /*
                         *   A 'true' return value indicates that the
                         *   parseUnknownXobj routine has fully handled
                         *   the command.  They don't want anything more
                         *   to be done with these words.  Simply remove
                         *   the unknown words and continue with any other
                         *   words in the list.  
                         */
                        rundisc(ctx->voccxrun);

                        /* we're done with this input phrase */
                        continue;

                    default:
                        /*
                         *   For anything else, use the default mechanism.
                         *   Simply return an error; since the "unknown
                         *   word" flag is set, we'll reparse the
                         *   sentence, this time rejecting unknown words
                         *   from the outset.
                         *   
                         *   Return error 2, since that's the generic "I
                         *   don't know the word..." error code.  
                         */
                        rundisc(ctx->voccxrun);
                        err = VOCERR(2);
                        goto done;
                    }

                    /*
                     *   If we made it this far, it means that they've
                     *   resolved the object for us, so we can consider
                     *   the previously unknown words to be known now. 
                     */
                    ctx->voccxunknown -= unknown_count;
                }
                else
                {
                    /* trace the event for debugging */
                    if (ctx->voccxflg & VOCCXFDBG)
                        tioputs(ctx->voccxtio,
                                "... unknown word: no parseUnknownXobj - "
                                "restarting parsing\\n");

                    /*
                     *   The verb doesn't define this method, so we should
                     *   use the traditional method; simply return
                     *   failure, and we'll reparse the sentence to reject
                     *   the unknown word in the usual fashion.  Return
                     *   error 2, since that's the generic "I don't know
                     *   the word..." error code.  
                     */
                    err = VOCERR(2);
                    goto done;
                }
            }
            
            /*
             *   Use a new method to cut down on the time it will take to
             *   iterate through the verprop's on all of those words.
             *   We'll call the verb's validXoList method - it should
             *   return a list containing all of the valid objects for the
             *   verb (it's sort of a Fourier transform of validDo).
             *   We'll intersect that list with the list we're about to
             *   disambiguate, which should provide a list of objects that
             *   are already qualified, in that validDo should return true
             *   for every one of them.  
             * 
             *   The calling sequence is:
             *       verb.validXoList(actor, prep, otherobj)
             * 
             *   For reverse compatibility, if the return value is nil,
             *   we use the old algorithm and consider all objects
             *   that match the vocabulary.  The return value must be
             *   a list to be considered.
             *
             *   If disambiguating the actor, skip this phase, since
             *   we don't have a verb yet.
             */
            if (accprop != PRP_VALIDACTOR && cnt != 0)
            {
                if (defprop == PRP_DODEFAULT)
                    listprop = PRP_VALDOLIST;
                else
                    listprop = PRP_VALIOLIST;
                
                /* push the arguments:  the actor, prep, and other object */
                runpobj(ctx->voccxrun, otherobj);
                runpobj(ctx->voccxrun, cmdPrep);
                runpobj(ctx->voccxrun, cmdActor);
                runppr(ctx->voccxrun, cmdVerb, listprop, 3);
                if (runtostyp(ctx->voccxrun) == DAT_LIST)
                {
                    uchar *l;
                    uint   len;
                    int    kept_numobj;

                    /* presume we won't keep numObj */
                    kept_numobj = FALSE;
                    
                    /* read the list length prefix, and skip it */
                    l = runpoplst(ctx->voccxrun);
                    len = osrp2(l) - 2;
                    l += 2;
                    
                    /*
                     *   For each element of the return value, see if
                     *   it's in list1.  If so, copy the object into
                     *   list2, unless it's already in list2.  
                     */
                    for (cnt2 = 0 ; len != 0 ; )
                    {
                        if (*l == DAT_OBJECT)
                        {
                            objnum o = osrp2(l+1);
                            
                            for (i = 0 ; i < cnt ; ++i)
                            {
                                if (list1[i] == o)
                                {
                                    int j;
                                    
                                    /* check to see if o is already in list2 */
                                    for (j = 0 ; j < cnt2 ; ++j)
                                        if (list2[j] == o) break;
                                    
                                    /* if o is not in list2 yet, add it */
                                    if (j == cnt2)
                                    {
                                        /* add it */
                                        list2[cnt2] = o;
                                        flags2[cnt2] = flags1[i];
                                        ++cnt2;

                                        /* 
                                         *   if it's numObj, note that
                                         *   we've already included it in
                                         *   the output list, so that we
                                         *   don't add it again later 
                                         */
                                        if (o == ctx->voccxnum)
                                            kept_numobj = TRUE;
                                    }
                                    break;
                                }
                            }
                        }
                        
                        /* move on to next element */
                        lstadv(&l, &len);
                    }

                    /*
                     *   If the original list included numObj, keep it in
                     *   the accessible list for now - we consider numObj
                     *   to be always accessible.  The noun phrase matcher
                     *   will include numObj whenever the player enters a
                     *   single number as a noun phrase, even when the
                     *   number matches an object.  Note that we can skip
                     *   this special step if we already kept numObj in
                     *   the valid list.  
                     */
                    if (!kept_numobj)
                    {
                        /* search the original list for numObj */
                        for (i = 0 ; i < cnt ; ++i)
                        {
                            /* if this original entry is numObj, keep it */
                            if (list1[i] == ctx->voccxnum)
                            {
                                /* keep it in the accessible list */
                                list2[cnt2++] = ctx->voccxnum;
                                
                                /* no need to look any further */
                                break;
                            }
                        }
                    }
                    
                    /* copy list2 into list1 */
                    memcpy(list1, list2, (size_t)(cnt2 * sizeof(list1[0])));
                    memcpy(flags1, flags2, (size_t)cnt2 * sizeof(flags1[0]));
                    cnt = cnt2;
                    list1[cnt] = MCMONINV;
                }
                else
                    rundisc(ctx->voccxrun);
            }

            /*
             *   Determine accessibility and visibility.  First, limit
             *   list1 to those objects that are visible OR accessible,
             *   and limit list3 to those objects that are visible.  
             */
            for (cnt = cnt3 = i = 0 ; list1[i] != MCMONINV ; ++i)
            {
                int is_vis;
                int is_acc;

                /* determine if the object is visible */
                is_vis = vocchkvis(ctx, list1[i], cmdActor);

                /* determine if it's accessible */
                is_acc = vocchkaccess(ctx, list1[i], accprop, i,
                                      cmdActor, cmdVerb);

                /* keep items that are visible OR accessible in list1 */
                if (is_acc || is_vis)
                {
                    list1[cnt] = list1[i];
                    flags1[cnt] = flags1[i];
                    ++cnt;
                }
                
                /* 
                 *   put items that are visible (regardless of whether or
                 *   not they're accessible) in list3
                 */
                if (is_vis)
                {
                    list3[cnt3] = list1[i];
                    flags3[cnt3] = flags1[i];
                    ++cnt3;
                }
            }

            /*
             *   If some of our accessible objects matched with an
             *   adjective at the end of the noun phrase, and others
             *   didn't (i.e., the others matched with a noun or plural at
             *   the end of the noun phrase), eliminate the ones that
             *   matched with an adjective at the end.  Ending a noun
             *   phrase with an adjective is really a kind of short-hand;
             *   if we have matches for both the full name version (with a
             *   noun at the end) and a short-hand version, we want to
             *   discard the short-hand version so that we don't treat it
             *   as ambiguous with the long-name version.  Likewise, if we
             *   have some exact matches and some truncations, keep only
             *   the exact matches.  
             */
            cnt = voc_prune_matches(ctx, list1, flags1, cnt);
            cnt3 = voc_prune_matches(ctx, list3, flags3, cnt3);

            /*
             *   Now, reduce list1 to objects that are accessible.  The
             *   reason for this multi-step process is to ensure that we
             *   prune the list with respect to every object in scope
             *   (visible or accessible for the verb), so that we get the
             *   most sensible pruning behavior.  This is more sensible
             *   than pruning by accessibility only, because sometimes we
             *   may have objects that are visible but are not accessible;
             *   as far as the player is concerned, the visible objects
             *   are part of the current location, so the player should be
             *   able to refer to them regardless of whether they're
             *   accessible.  
             */
            for (dst = 0, i = 0 ; i < cnt ; ++i)
            {
                /* check this object for accessibility */
                if (vocchkaccess(ctx, list1[i], accprop, i,
                                 cmdActor, cmdVerb))
                {
                    /* keep it in the final list */
                    list1[dst] = list1[i];
                    flags1[dst] = flags1[i];
                    
                    /* count the new list entry */
                    ++dst;
                }
            }

            /* terminate list1 */
            cnt = dst;
            list1[dst] = MCMONINV;

            /*
             *   Go through the list of accessible objects, and perform
             *   the sensible-object (verXoVerb) check on each.  Copy each
             *   sensible object to list2.  
             */
            for (i = 0, cnt2 = 0 ; i < cnt ; ++i)
            {
                /* run it by the appropriate sensible-object check */
                if (accprop == PRP_VALIDACTOR)
                {
                    /* run it through preferredActor */
                    runppr(ctx->voccxrun, list1[i], PRP_PREFACTOR, 0);
                    if (runpoplog(ctx->voccxrun))
                    {
                        list2[cnt2] = list1[i];
                        flags2[cnt2] = flags1[i];
                        ++cnt2;
                    }
                }
                else
                {
                    /* run it through verXoVerb */
                    tiohide(ctx->voccxtio);
                    if (otherobj != MCMONINV)
                        runpobj(ctx->voccxrun, otherobj);
                    runpobj(ctx->voccxrun, cmdActor);
                    runppr(ctx->voccxrun, list1[i], verprop,
                           (otherobj != MCMONINV ? 2 : 1));
                    
                    /*
                     *   If that didn't result in a message, this object
                     *   passed the tougher test of ver?oX, so include it
                     *   in list2.  
                     */
                    if (!tioshow(ctx->voccxtio))
                    {
                        list2[cnt2] = list1[i];
                        flags2[cnt2] = flags1[i];
                        ++cnt2;
                    }
                }
            }

            /*
             *   Construct a string consisting of the words the user typed
             *   to reference this object, in case we need to complain.
             */
            usrobj[0] = '\0';
            if (inlist[inpos].vocolfst != 0 && inlist[inpos].vocollst != 0)
            {
                for (p = inlist[inpos].vocolfst ; p <= inlist[inpos].vocollst
                     ; p += strlen(p) + 1)
                {
                    /* add a space if we have a prior word */
                    if (usrobj[0] != '\0')
                    {
                        /* quote the space if the last word ended with '.' */
                        if (p[strlen(p)-1] == '.')
                            strcat(usrobj, "\\");

                        /* add the space */
                        strcat(usrobj, " ");
                    }

                    /* add the current word, or "of" if it's "of" */
                    if (voc_check_special(ctx, p, VOCW_OF))
                        vocaddof(ctx, usrobj);
                    else
                        strcat(usrobj, p);
                }
            }

            /*
             *   If there's nothing in the YES list, and we have just a
             *   single number as our word, act as though they are talking
             *   about the number itself, rather than one of the objects
             *   that happened to use the number -- none of those objects
             *   make any sense, it seems, so fall back on the number.
             *   
             *   Note that we may also have only numObj in the YES list,
             *   because the noun phrase parser normally adds numObj when
             *   the player types a noun phrase consisting only of a
             *   number.  Do the same thing in this case -- just return
             *   the number object.  
             */
            if ((cnt2 == 0
                 || (cnt2 == 1 && list2[0] == ctx->voccxnum))
                && inlist[inpos].vocolfst != 0
                && inlist[inpos].vocolfst == inlist[inpos].vocollst
                && vocisdigit(*inlist[inpos].vocolfst))
            {
                long  v1;
                char  vbuf[4];

                v1 = atol(inlist[inpos].vocolfst);
                oswp4s(vbuf, v1);
                vocsetobj(ctx, ctx->voccxnum, DAT_NUMBER, vbuf,
                          &inlist[inpos], &outlist[outpos]);
                outlist[outpos].vocolflg = VOCS_NUM;
                ++outpos;

                /* skip all objects that matched the number */
                for ( ; inlist[inpos+1].vocolobj != MCMONINV
                      && inlist[inpos+1].vocolfst == inlist[inpos].vocolfst
                      ; ++inpos) ;
                continue;
            }

            /*
             *   Check if we found anything in either the YES (list2) or
             *   MAYBE (list1) lists.  If there's nothing in either list,
             *   complain and return.  
             */
            if (cnt2 == 0 && cnt == 0)
            {
                /*
                 *   We have nothing sensible, and nothing even
                 *   accessible.  If there's anything merely visible,
                 *   complain about those items. 
                 */
                if (cnt3 != 0)
                {
                    /* there are visible items - complain about them */
                    cnt = cnt3;
                    cantreach_list = list3;
                    noreach = TRUE;

                    /* give the cantReach message, even for multiple objects */
                    goto noreach1;
                }
                else
                {
                    /* 
                     *   explain that there's nothing visible or
                     *   accessible matching the noun phrase, and abort
                     *   the command with an error 
                     */
                    if (!silent)
                        vocerr(ctx, VOCERR(9),
                               "I don't see any %s here.", usrobj);
                    err = VOCERR(9);
                    goto done;
                }
            }

            /*
             *   If anything passed the stronger test (objects passing are
             *   in list2), use this as our proposed resolution for the
             *   noun phrase.  If nothing passed the stronger test (i.e.,
             *   list2 is empty), simply keep the list of accessible
             *   objects in list1. 
             */
            if (cnt2 != 0)
            {
                /* 
                 *   we have items passing the stronger test -- copy the
                 *   stronger list (list2) to list1 
                 */
                cnt = cnt2;
                memcpy(list1, list2, (size_t)(cnt2 * sizeof(list1[0])));
                memcpy(flags1, flags2, (size_t)(cnt2 * sizeof(flags1[0])));
            }

            /*
             *   Check for redundant objects in the list.  If the same
             *   object appears multiple times in the list, remove the
             *   extra occurrences.  Sometimes, a game can inadvertantly
             *   define the same vocabulary word several times for the
             *   same object, because of the parser's leniency with
             *   matching leading substrings of 6 characters or longer.
             *   To avoid unnecessary "which x do you mean..." errors,
             *   simply discard any duplicates in the list.  
             */
            for (dst = 0, i = 0 ; i < cnt ; ++i)
            {
                int dup;
                int j;
                
                /* presume we won't find a duplicate of this object */
                dup = FALSE;
                
                /* 
                 *   look for duplicates of this object in the remainder
                 *   of the list 
                 */
                for (j = i + 1 ; j < cnt ; ++j)
                {
                    /* check for a duplicate */
                    if (list1[i] == list1[j])
                    {
                        /* note that this object has a duplicate */
                        dup = TRUE;
                        
                        /* we don't need to look any further */
                        break;
                    }
                }

                /* 
                 *   if this object has no duplicate, retain it in the
                 *   output list 
                 */
                if (!dup)
                {
                    /* copy the element to the output */
                    list1[dst] = list1[i];
                    flags1[dst] = flags1[i];
                    
                    /* count the output */
                    ++dst;
                }
            }
            
            /* update the count to the new list's size */
            cnt = dst;
            list1[cnt] = MCMONINV;

            /* 
             *   If we have more than one object in the list, and numObj
             *   is still in the list, remove numObj - we don't want to
             *   consider numObj to be considered ambiguous with another
             *   object when the other object passes access and validation
             *   tests.
             */
            if (cnt > 1)
            {
                /* scan the list for numObj */
                for (i = 0, dst = 0 ; i < cnt ; ++i)
                {
                    /* if this isn't numObj, keep this element */
                    if (list1[i] != ctx->voccxnum)
                        list1[dst++] = list1[i];
                }

                /* update the final count */
                cnt = dst;
                list1[cnt] = MCMONINV;
            }

            /*
             *   Check for a generic numeric adjective ('#' in the
             *   adjective list for the object) in each object.  If we
             *   find it, we need to make sure there's a number in the
             *   name of the object.  
             */
            for (i = 0 ; i < cnt ; ++i)
            {
                if (has_gen_num_adj(ctx, list1[i]))
                {
                    /*
                     *   If they specified a count, create the specified
                     *   number of objects.  Otherwise, if the object is
                     *   plural, they mean to use all of the objects, so a
                     *   numeric adjective isn't required -- set the
                     *   numeric adjective property in the object to nil
                     *   to so indicate.  Otherwise, look for the number,
                     *   and set the numeric adjective property
                     *   accordingly.  
                     */
                    if ((flags1[i] & (VOCS_ANY | VOCS_COUNT)) != 0)
                    {
                        int     n = (user_count ? user_count : 1);
                        int     j;
                        objnum  objn = list1[i];
                        
                        /* 
                         *   They specified a count, so we want to create
                         *   n-1 copies of the numbered object.  Make room
                         *   for the n-1 new copies of this object by
                         *   shifting any elements that follow up n-1
                         *   slots.  
                         */
                        if (i + 1 != cnt && n > 1)
                        {
                            memmove(&list1[i + n - 1], &list1[i],
                                    (cnt - i) * sizeof(list1[i]));
                            memmove(&flags1[i + n - 1], &flags1[i],
                                    (cnt - i) * sizeof(flags1[i]));
                        }
                        
                        /* create n copies of this object */
                        for (j = 0 ; j < n ; ++j)
                        {
                            long l;
                            
                            /* 
                             *   Generate a number for the new object,
                             *   asking the object to tell us what value
                             *   to use for an "any".  
                             */
                            runpnum(ctx->voccxrun, (long)(j + 1));
                            runppr(ctx->voccxrun, objn, PRP_ANYVALUE, 1);
                            l = runpopnum(ctx->voccxrun);
                            
                            /* try creating the new object */
                            list1[i+j] =
                                voc_new_num_obj(ctx, objn,
                                                cmdActor, cmdVerb,
                                                l, FALSE);
                            if (list1[i+j] == MCMONINV)
                            {
                                err = VOCERR(40);
                                goto done;
                            }
                        }
                    }
                    else if ((flags1[i] & VOCS_PLURAL) != 0)
                    {
                        /*
                         *   get the plural object by asking for the
                         *   numbered object with a nil number parameter 
                         */
                        list1[i] =
                            voc_new_num_obj(ctx, list1[i], cmdActor, cmdVerb,
                                            (long)0, TRUE);
                        if (list1[i] == MCMONINV)
                        {
                            err = VOCERR(40);
                            goto done;
                        }
                    }
                    else
                    {
                        //char *p;
                        int   found;
                        
                        /* 
                         *   No plural, no "any" - we just want to create
                         *   one numbered object, using the number that
                         *   the player must have specified.  Make sure
                         *   the player did, in fact, specify a number. 
                         */
                        for (found = FALSE, p = inlist[inpos].vocolfst ;
                             p != 0 && p <= inlist[inpos].vocollst ;
                             p += strlen(p) + 1)
                        {
                            /* did we find it? */
                            if (vocisdigit(*p))
                            {
                                long l;

                                /* get the number */
                                l = atol(p);
                                
                                /* create the object with this number */
                                list1[i] = voc_new_num_obj(ctx, list1[i],
                                    cmdActor, cmdVerb,
                                    l, FALSE);
                                if (list1[i] == MCMONINV)
                                {
                                    err = VOCERR(40);
                                    goto done;
                                }
                                
                                /* the command looks to be valid */
                                found = TRUE;
                                break;
                            }
                        }
                        
                        /* if we didn't find it, stop now */
                        if (!found)
                        {
                            if (!silent)
                                vocerr(ctx, VOCERR(160),
                    "You'll have to be more specific about which %s you mean.",
                                       usrobj);
                            err = VOCERR(160);
                            goto done;
                        }
                    }
                }
            }

            /*
             *   We still have an ambiguous word - ask the user which of
             *   the possible objects they meant to use 
             */
            trying_again = FALSE;
            for (;;)
            {
                int    wrdcnt;
                int    next;
                uchar *pu;
                int    cleared_noun;
                int    diff_cnt;
                int    stat;
                int    num_wanted;
                int    is_ambig;
                int    all_plural;
                    
                /* 
                 *   check for usage - determine if we have singular
                 *   definite, singular indefinite, counted, or plural
                 *   usage 
                 */
                if ((flags1[0] & (VOCS_PLURAL | VOCS_ANY | VOCS_COUNT)) != 0)
                {
                    //int i;

                    /* 
                     *   loop through the objects to AND together the
                     *   flags from all of the objects; we only care about
                     *   the plural flags (PLURAL, ANY, and COUNT), so
                     *   start out with only those, then AND off any that
                     *   aren't in all of the objects 
                     */
                    for (all_plural = VOCS_PLURAL | VOCS_ANY | VOCS_COUNT,
                         i = 0 ; i < cnt ; ++i)
                    {
                        /* AND out this object's flags */
                        all_plural &= flags1[i];
                        
                        /* 
                         *   if we've ANDed down to zero, there's no need
                         *   to look any further 
                         */
                        if (!all_plural)
                            break;
                    }
                }
                else
                {
                    /* 
                     *   it looks like we want just a single object -
                     *   clear the various plural flags 
                     */
                    all_plural = 0;
                }

                /*
                 *   Count the distinguishable items.
                 *   
                 *   If we're looking for a single object, don't keep
                 *   duplicate indistinguishable items (i.e., keep only
                 *   one item from each set of mutually indistinguishable
                 *   items), since we could equally well use any single
                 *   one of those items.  If we're looking for multiple
                 *   objects, keep all of the items, since the user is
                 *   referring to all of them. 
                 */
                diff_cnt = voc_count_diff(ctx, list1, flags1, &cnt,
                                          all_plural != 0 || use_all_objs);

                /*
                 *   Determine how many objects we'd like to find.  If we
                 *   have a count specified, we'd like to find the given
                 *   number of objects.  If we have "ANY" specified, we
                 *   just want to pick one object arbitrarily.  If we have
                 *   all plurals, we can keep all of the objects.  If the
                 *   'use_all_objs' flag is true, it means that we can use
                 *   everything in the list.  
                 */
                if (use_all_objs)
                {
                    /* we want to use all of the objects */
                    num_wanted = cnt;
                    is_ambig = FALSE;
                }
                else if ((all_plural & VOCS_COUNT) != 0)
                {
                    /* 
                     *   we have a count - we want exactly the given
                     *   number of objects, but we can pick an arbitrary
                     *   subset, so it's not ambiguous even if we have too
                     *   many at the moment 
                     */
                    num_wanted = user_count;
                    is_ambig = FALSE;
                }
                else if ((all_plural & VOCS_ANY) != 0)
                {
                    /* 
                     *   they specified "any", so we want exactly one, but
                     *   we can pick one arbitrarily, so there's no
                     *   ambiguity 
                     */
                    num_wanted = 1;
                    is_ambig = FALSE;
                }
                else if (all_plural != 0)
                {
                    /* 
                     *   we have a simple plural, so we can use all of the
                     *   provided objects without ambiguity 
                     */
                    num_wanted = cnt;
                    is_ambig = FALSE;
                }
                else
                {
                    /* 
                     *   it's a singular, definite usage, so we want
                     *   exactly one item; if we have more than one in our
                     *   list, it's ambiguous 
                     */
                    num_wanted = 1;
                    is_ambig = (cnt != 1);
                }

                /* call the disambiguation hook */
                stat = voc_disambig_hook(ctx, cmdVerb, cmdActor, cmdPrep,
                                         otherobj, accprop, verprop,
                                         list1, flags1, &cnt,
                                         inlist[inpos].vocolfst,
                                         inlist[inpos].vocollst,
                                         num_wanted, is_ambig, disnewbuf,
                                         silent);

                /* check the status */
                if (stat == VOC_DISAMBIG_DONE)
                {
                    /* that's it - copy the result */
                    for (i = 0 ; i < cnt ; ++i)
                        vocout(&outlist[outpos++], list1[i], flags1[i],
                               inlist[inpos].vocolfst,
                               inlist[inpos].vocollst);
                    
                    /* we're done */
                    break;
                }
                else if (stat == VOC_DISAMBIG_CONT)
                {
                    /* 
                     *   Continue with the new list (which is the same as
                     *   the old list, if it wasn't actually updated by
                     *   the hook routine) - proceed with remaining
                     *   processing, but using the new list.
                     *   
                     *   Because the list has been updated, we must once
                     *   again count the number of distinguishable items,
                     *   since that may have changed.  
                     */
                    diff_cnt = voc_count_diff(ctx, list1, flags1, &cnt, TRUE);
                }
                else if (stat == VOC_DISAMBIG_PARSE_RESP
                         || stat == VOC_DISAMBIG_PROMPTED)
                {
                    /* 
                     *   The status indicates one of the following:
                     *   
                     *   - the hook prompted for more information and read
                     *   a response from the player, but decided not to
                     *   parse it; we will continue with the current list,
                     *   and parse the player's response as provided by
                     *   the hook.
                     *   
                     *   - the hook prompted for more information, but
                     *   left the reading to us.  We'll proceed with the
                     *   current list and read a response as normal, but
                     *   without displaying another prompt.
                     *   
                     *   In any case, just continue processing; we'll take
                     *   appropriate action on the prompting and reading
                     *   when we reach those steps.  
                     */
                }
                else
                {
                    /* anything else is an error */
                    err = VOCERR(41);
                    goto done;
                }

                /*
                 *   If we found only one word, or a plural/ANY, we are
                 *   finished.  If we found a count, use that count if
                 *   possible.  
                 */
                if (cnt == 1 || all_plural || use_all_objs)
                {
                    int flags;

                    /* keep only one of the objects if ANY was used */
                    if ((all_plural & VOCS_COUNT) != 0)
                    {
                        if (user_count > cnt)
                        {
                            if (!silent)
                                vocerr(ctx, VOCERR(30),
                                       "I only see %d of those.", cnt);
                            err = VOCERR(30);
                            goto done;
                        }
                        cnt = user_count;
                        flags = VOCS_ALL;
                    }
                    else if ((all_plural & VOCS_ANY) != 0)
                    {
                        cnt = 1;
                        flags = VOCS_ALL;
                    }
                    else
                        flags = 0;

                    /* put the list */
                    for (i = 0 ; i < cnt ; ++i)
                        vocout(&outlist[outpos++], list1[i], flags,
                               inlist[inpos].vocolfst,
                               inlist[inpos].vocollst);

                    /* we're done */
                    break;
                }

                /* make sure output capturing is off */
                tiocapture(ctx->voccxtio, (mcmcxdef *)0, FALSE);
                tioclrcapture(ctx->voccxtio);

                /* 
                 *   if we're in "silent" mode, we can't ask the player
                 *   for help, so return an error 
                 */
                if (silent)
                {
                    /* 
                     *   We can't disambiguate the list.  Fill in the
                     *   return list with what's left, which is still
                     *   ambiguous, and note that we need to return an
                     *   error code indicating that the list remains
                     *   ambiguous.  
                     */
                    for (i = 0 ; i < cnt && outpos < VOCMAXAMBIG ; ++i)
                        vocout(&outlist[outpos++], list1[i], 0,
                               inlist[inpos].vocolfst,
                               inlist[inpos].vocollst);

                    /* note that we have ambiguity remaining */
                    still_ambig = TRUE;

                    /* we're done with this sublist */
                    break;
                }

                /*
                 *   We need to prompt for more information interactively.
                 *   Figure out how we're going to display the prompt.
                 *   
                 *   - If the disambigXobj hook status (stat) indicates
                 *   that the hook already displayed a prompt of its own,
                 *   we don't need to add anything here.
                 *   
                 *   - Otherwise, if there's a parseDisambig function
                 *   defined in the game, call it to display the prompt.
                 *   
                 *   - Otherwise, display our default prompt.  
                 */
                if (stat == VOC_DISAMBIG_PARSE_RESP
                    || stat == VOC_DISAMBIG_PROMPTED)
                {
                    /* 
                     *   the disambigXobj hook already asked for a
                     *   response, so don't display any prompt of our own 
                     */
                }
                else if (ctx->voccxpdis != MCMONINV)
                {
                    uint l;
                    
                    /* 
                     *   There's a parseDisambig function defined in the
                     *   game - call it to display the prompt, passing the
                     *   list of possible objects and the player's
                     *   original noun phrase text as parameters.  
                     */
                    for (i = 0, pu = lstbuf+2 ; i < cnt ; ++i, pu += 2)
                    {
                        *pu++ = DAT_OBJECT;
                        oswp2(pu, list1[i]);
                    }
                    l = pu - lstbuf;
                    oswp2(lstbuf, l);
                    runpbuf(ctx->voccxrun, DAT_LIST, lstbuf);
                    runpstr(ctx->voccxrun, usrobj, (int)strlen(usrobj), 1);
                    runfn(ctx->voccxrun, ctx->voccxpdis, 2);
                }
                else
                {
                    /* display "again" message, if necessary */
                    if (trying_again)
                        vocerr_info(ctx, VOCERR(100), "Let's try it again: ");

                    /* ask the user about it */
                    vocerr_info(ctx, VOCERR(101),
                                "Which %s do you mean, ", usrobj);
                    for (i = 0 ; i < cnt ; )
                    {
                        int    eqcnt;
                        int    j;
                        objnum sc;
                        
                        /*
                         *   See if we have multiple instances of an
                         *   identical object.  All such instances should
                         *   be grouped together (this was done above), so
                         *   we can just count the number of consecutive
                         *   equivalent objects. 
                         */
                        eqcnt = 1;
                        runppr(ctx->voccxrun, list1[i], PRP_ISEQUIV, 0);
                        if (runpoplog(ctx->voccxrun))
                        {
                            /* get the superclass, if possible */
                            sc = objget1sc(ctx->voccxmem, list1[i]);
                            if (sc != MCMONINV)
                            {
                                /* count equivalent objects that follow */
                                for (j = i + 1 ; j < cnt ; ++j)
                                {
                                    if (objget1sc(ctx->voccxmem, list1[j])
                                        == sc)
                                        ++eqcnt;
                                    else
                                        break;
                                }
                            }
                        }

                        /*
                         *   Display this object's name.  If we have only
                         *   one such object, display its thedesc,
                         *   otherwise display its adesc. 
                         */
                        runppr(ctx->voccxrun, list1[i],
                               (prpnum)(eqcnt == 1 ?
                                        PRP_THEDESC : PRP_ADESC), 0);

                        /* display the separator as appropriate */
                        if (i + 1 < diff_cnt)
                            vocerr_info(ctx, VOCERR(102), ", ");
                        if (i + 2 == diff_cnt)
                            vocerr_info(ctx, VOCERR(103), "or ");

                        /* skip all equivalent items */
                        i += eqcnt;
                    }
                    vocerr_info(ctx, VOCERR(104), "?");
                }

                /* 
                 *   Read the response.  If the disambigXobj hook already
                 *   read the response, we don't need to read anything
                 *   more. 
                 */
                if (stat != VOC_DISAMBIG_PARSE_RESP
                    && vocread(ctx, cmdActor, cmdVerb, disnewbuf,
                               (int)VOCBUFSIZ, 2) == VOCREAD_REDO)
                {
                    /* they want to treat the input as a new command */
                    strcpy(cmdbuf, disnewbuf);
                    ctx->voccxunknown = 0;
                    ctx->voccxredo = TRUE;
                    err = VOCERR(43);
                    goto done;
                }

                /*
                 *   parse the response 
                 */

                /* tokenize the list */
                wrdcnt = voctok(ctx, disnewbuf, disbuffer, diswordlist,
                                TRUE, TRUE, TRUE);
                if (wrdcnt == 0)
                {
                    /* empty response - run pardon() function and abort */
                    runfn(ctx->voccxrun, ctx->voccxprd, 0);
                    err = VOCERR(42);
                    goto done;
                }
                if (wrdcnt < 0)
                {
                    /* return the generic punctuation error */
                    err = VOCERR(1);
                    goto done;
                }

                /*
                 *   Before we tokenize the sentence, remember the current
                 *   unknown word count, then momentarily set the count to
                 *   zero.  This will cause the tokenizer to absorb any
                 *   unknown words; if there are any unknown words, the
                 *   tokenizer will parse them and set the unknown count.
                 *   If we find any unknown words in the input, we'll
                 *   simply treat the input as an entirely new command.  
                 */
                old_unknown = ctx->voccxunknown;
                old_lastunk = ctx->voccxlastunk;
                ctx->voccxunknown = 0;

                /* clear our internal type list */
                memset(distypelist, 0, VOCBUFSIZ * sizeof(distypelist[0]));

                /* tokenize the sentence */
                diswordlist[wrdcnt] = 0;
                if (vocgtyp(ctx, diswordlist, distypelist, cmdbuf)
                    || ctx->voccxunknown != 0)
                {
                    /* 
                     *   there's an unknown word or other problem - retry
                     *   the input as an entirely new command 
                     */
                    strcpy(cmdbuf, disnewbuf);
                    ctx->voccxunknown = 0;
                    ctx->voccxredo = TRUE;
                    err = VOCERR(2);
                    goto done;
                }

                /* restore the original unknown word count */
                ctx->voccxunknown = old_unknown;
                ctx->voccxlastunk = old_lastunk;

                /*
                 *   Find the last word that can be an adj and/or a noun.
                 *   If it can be either (i.e., both bits are set), clear
                 *   the noun bit and make it just an adjective.  This is
                 *   because we're asking for an adjective for clarification,
                 *   and we most likely want it to be an adjective in this
                 *   context; if the noun bit is set, too, the object lister
                 *   will think it must be a noun, being the last word.
                 */
                for (i = 0 ; i < wrdcnt ; ++i)
                {
                    if (!(distypelist[i] &
                          (VOCT_ADJ | VOCT_NOUN | VOCT_ARTICLE)))
                        break;
                }
                
                if (i && (distypelist[i-1] & VOCT_ADJ)
                    && (distypelist[i-1] & VOCT_NOUN))
                {
                    /*
                     *   Note that we're clearing the noun flag.  If
                     *   we're unsuccessful in finding the object with the
                     *   noun flag cleared, we'll put the noun flag back
                     *   in and give it another try (by adding VOCT_NOUN
                     *   back into distypelist[cleared_noun], and coming
                     *   back to the label below). 
                     */
                    cleared_noun = i-1;
                    distypelist[i-1] &= ~VOCT_NOUN;
                }
                else
                    cleared_noun = -1;

            try_current_flags:
                /* start with the first word */
                if (vocspec(diswordlist[0], VOCW_ALL)
                    || vocspec(diswordlist[0], VOCW_BOTH))
                {
                    char *nam;
                    static char all_name[] = "all";
                    static char both_name[] = "both";

                    if (vocspec(diswordlist[0], VOCW_ALL))
                        nam = all_name;
                    else
                        nam = both_name;
                    
                    for (i = 0 ; i < cnt ; ++i)
                        vocout(&outlist[outpos++], list1[i], 0, nam, nam);
                    if (noreach)
                    {
                        cantreach_list = list1;
                        goto noreach1;
                    }
                    break;
                }
                else if (vocspec(diswordlist[0], VOCW_ANY))
                {
                    /* choose the first object arbitrarily */
					vocout(&outlist[outpos++], list1[i], VOCS_ALL, "any", "any");
                    break;
                }
                else
                {
                    /* check for a word matching the phrase */
                    cnt2 = vocchknoun(ctx, diswordlist, distypelist,
                                      0, &next, disnounlist, FALSE);
                    if (cnt2 > 0)
                    {
                        /* 
                         *   if that didn't consume the entire phrase, or
                         *   at least up to "one" or "ones" or a period,
                         *   disallow it, since they must be entering
                         *   something more complicated 
                         */
                        if (diswordlist[next] != 0
                            && !vocspec(diswordlist[next], VOCW_ONE)
                            && !vocspec(diswordlist[next], VOCW_ONES)
                            && !vocspec(diswordlist[next], VOCW_THEN))
                        {
                            cnt2 = 0;
                        }
                    }
                    else if (cnt2 < 0)
                    {
                        /* 
                         *   There was a syntax error in the phrase.
                         *   vocchknoun() will have displayed a message in
                         *   this case, so we're done parsing this command. 
                         */
                        err = VOCERR(45);
                        goto done;
                    }

                    /* proceed only if we got a valid phrase */
                    if (cnt2 > 0)
                    {
                        //int cnt3;
                        int newcnt;

                        /* build the list of matches for the new phrase */
                        for (i = 0, newcnt = 0 ; i < cnt2 ; ++i)
                        {
                            int j;
                            int found;

                            /* 
                             *   make sure this object isn't already in
                             *   our list - we want each object only once 
                             */
                            for (j = 0, found = FALSE ; j < newcnt ; ++j)
                            {
                                /* if this is in the list, note it */
                                if (list2[j] == disnounlist[i].vocolobj)
                                {
                                    found = TRUE;
                                    break;
                                }
                            }

                            /* 
                             *   add it to our list only if it wasn't
                             *   already there 
                             */
                            if (!found)
                            {
                                /* copy the object ID */
                                list2[newcnt] = disnounlist[i].vocolobj;

                                /* copy the flags that we care about */
                                flags2[newcnt] = disnounlist[i].vocolflg
                                    & (VOCS_PLURAL | VOCS_ANY
                                       | VOCS_COUNT);

                                /* count the entry */
                                ++newcnt;
                            }
                        }

                        /* terminate the list */
                        list2[newcnt] = MCMONINV;

                        /* intersect the new list with the old list */
                        newcnt = vocisect(list2, list1);

                        /* count the noun phrases in the new list */
                        for (i = cnt3 = 0 ; i < cnt2 ; ++i)
                        {
                            /* we have one more noun phrase */
                            ++cnt3;

                            /* if we have a noun phrase, skip matching objs */
                            if (disnounlist[i].vocolfst != 0)
                            {
                                int j;

                                /* skip objects matching this noun phrase */
                                for (j = i + 1 ; disnounlist[i].vocolfst ==
                                         disnounlist[j].vocolfst ; ++j) ;
                                i = j - 1;
                            }
                        }
                        
                        /*
                         *   If the count of items in the intersection of
                         *   the original list and the typed-in list is no
                         *   bigger than the number of items specified in
                         *   the typed-in list, we've successfully
                         *   disambiguated the object, because the user's
                         *   new list matches only one object for each set
                         *   of words the user typed.  
                         */
                        if (newcnt
                            && (newcnt <= cnt3
                                || (diswordlist[next]
                                    && vocspec(diswordlist[next],
                                               VOCW_ONES))))
                        {                            
                            for (i = 0 ; i < cnt ; ++i)
                                vocout(&outlist[outpos++], list2[i], 0,
                                       one_name, one_name);
                            
                            if (noreach)
                            {
                                cnt = newcnt;
                                cantreach_list = list2;
                            noreach1:
                                if (accprop == PRP_VALIDACTOR)
                                {
                                    /* for actors, show a special message */
                                    vocerr(ctx, VOCERR(31),
                                           "You can't talk to that.");
                                }
                                else
                                {
                                    /* use the normal no-reach message */
                                    vocnoreach(ctx, cantreach_list, cnt,
                                               cmdActor, cmdVerb, cmdPrep,
                                               defprop, cnt > 1, 0, 0, cnt);
                                }
                                err = VOCERR(31);
                                goto done;
                            }
                            break;
                        }
                        else if (newcnt == 0)
                        {
                            /*
                             *   If we cleared the noun, maybe we actually
                             *   need to treat the word as a noun, so add
                             *   the noun flag back in and give it another
                             *   go.  If we didn't clear the noun, there's
                             *   nothing left to try, so explain that we
                             *   don't see any such object and give up.  
                             */
                            if (cleared_noun != -1)
                            {
                                distypelist[cleared_noun] |= VOCT_NOUN;
                                cleared_noun = -1;
                                goto try_current_flags;
                            }

                            /* find the first object with a noun phrase */
                            for (i = 0 ; i < cnt2 ; ++i)
                            {
                                /* if we have a noun phrase, stop scanning */
                                if (disnounlist[i].vocolfst != 0)
                                    break;
                            }

                            /* 
                             *   if we found a noun phrase, build a string
                             *   out of the words used; otherwise, just
                             *   use "such" 
                             */
                            if (i != cnt2) {
                                const char *last;

                                /* clear the word buffer */
                                newobj[0] = '\0';
                                
                                /* build a string out of the words */
                                p = disnounlist[i].vocolfst;
                                last = disnounlist[i].vocollst;
                                for ( ; p <= last ; p += strlen(p) + 1)
                                {
                                    /* 
                                     *   If this is a special word, we
                                     *   probably can't construct a
                                     *   sensible sentence - special words
                                     *   are special parts of speech that
                                     *   will look weird if inserted into
                                     *   our constructed noun phrase.  In
                                     *   these cases, turn the entire
                                     *   thing into "I don't see any
                                     *   *such* object" rather than trying
                                     *   to make do with pronouns or other
                                     *   special words.  
                                     */
                                    if (vocisspec(p))
                                    {
                                        /* 
                                         *   replace the entire adjective
                                         *   phrase with "such" 
                                         */
                                        strcpy(newobj, "such");

                                        /* 
                                         *   stop here - don't add any
                                         *   more, since "such" is the
                                         *   whole thing 
                                         */
                                        break;
                                    }
                                    
                                    /* add a space if we have a prior word */
                                    if (newobj[0] != '\0')
                                        strcat(newobj, " ");

                                    /* add this word */
                                    strcat(newobj, p);
                                }
                            }
                            else
                            {
                                /* no noun phrase found */
                                strcpy(newobj, "such");
                            }

                            /* didn't find anything - complain and give up */
                            vocerr(ctx, VOCERR(16),
                                   "You don't see any %s %s here.",
                                   newobj, usrobj);
                            err = VOCERR(16);
                            goto done;
                        }
                        
                        /*
                         *   If we get here, it means that we have still
                         *   more than one object per noun phrase typed in
                         *   the latest sentence.  Limit the list to the
                         *   intersection (by copying list2 to list1), and
                         *   try again.  
                         */
                        memcpy(list1, list2,
                               (size_t)((newcnt + 1) * sizeof(list1[0])));
                        cnt = newcnt;
                        trying_again = TRUE;
                    }
                    else
                    {
                        /* 
                         *   We didn't find a noun phrase, so it's probably a
                         *   new command.  However, check first to see if we
                         *   were making a trial run with the noun flag
                         *   cleared: if so, go back and make another pass
                         *   with the noun flag added back in to see if that
                         *   works any better.  
                         */
                        if (cleared_noun != -1)
                        {
                            distypelist[cleared_noun] |= VOCT_NOUN;
                            cleared_noun = -1;
                            goto try_current_flags;
                        }
                        
                        /* retry as an entire new command */
                        strcpy(cmdbuf, disnewbuf);
                        ctx->voccxunknown = 0;
                        ctx->voccxredo = TRUE;
                        err = VOCERR(43);
                        goto done;
                    }
                }
            }
            inpos = lpos - 1;
        }
    }

    /* terminate the output list */
    vocout(&outlist[outpos], MCMONINV, 0, (char *)0, (char *)0);

    /* 
     *   If we still have ambiguous objects, so indicate.  This can only
     *   happen when we operate in "silent" mode, because only then can we
     *   give up without fully resolving a list. 
     */
    if (still_ambig)
        err = VOCERR(44);

    /* no error */
    err = 0;

done:
    ERRCLEAN(ctx->voccxerr)
    {
        /* 
         *   reset the stack before we return, in case the caller handles
         *   the error without aborting the command 
         */
        voc_leave(ctx, save_sp);
    }
    ERRENDCLN(ctx->voccxerr);

    /* return success */
    VOC_RETVAL(ctx, save_sp, err);
}

/* vocready - see if at end of command, execute & return TRUE if so */
static int vocready(voccxdef *ctx, char *cmd[], int *typelist, int cur,
                    objnum cmdActor, objnum cmdPrep, char *vverb, char *vprep,
                    vocoldef *dolist, vocoldef *iolist, int *errp,
                    char *cmdbuf, int first_word, uchar **preparse_list,
                    int *next_start)
{
    if (cur != -1
        && (cmd[cur] == (char *)0
            || vocspec(cmd[cur], VOCW_AND) || vocspec(cmd[cur], VOCW_THEN)))
    {
        if (ctx->voccxflg & VOCCXFDBG)
        {
            char buf[128];
            
            sprintf(buf, ". executing verb:  %s %s\\n",
                    vverb, vprep ? vprep : "");
            tioputs(ctx->vocxtio, buf);
        }

        *errp = execmd(ctx, cmdActor, cmdPrep, vverb, vprep, dolist, iolist,
                       &cmd[first_word], &typelist[first_word],cmdbuf,
                       cur - first_word, preparse_list, next_start);
        return(TRUE);
    }
    return(FALSE);
}

/* execute a single command */
static int voc1cmd(voccxdef *ctx, char *cmd[], char *cmdbuf,
                   objnum *cmdActorp, int first)
{
    int       cur;
    int       next;
    objnum    o;
    vocwdef  *v;
    char     *vverb;
    int       vvlen;
    char     *vprep;
    int       cnt;
    int       err;
    vocoldef *dolist;
    vocoldef *iolist;
    int      *typelist;
    objnum    cmdActor = *cmdActorp;
    objnum    cmdPrep;
    int       swapObj;                        /* TRUE -> swap dobj and iobj */
    int       again;
    int       first_word;
	uchar    *preparse_list = nullptr;
    int       next_start;
    struct
    {
        int    active;
        int    cur;
        char **cmd;
        char  *cmdbuf;
    } preparseCmd_stat;
    char    **newcmd;
    char     *origcmdbuf;
    char     *newcmdbuf;
    uchar    *save_sp;
    int       no_match;
    int       retval;

    voc_enter(ctx, &save_sp);
    VOC_MAX_ARRAY(ctx, vocoldef, dolist);
    VOC_MAX_ARRAY(ctx, vocoldef, iolist);
    VOC_STK_ARRAY(ctx, int,      typelist,  VOCBUFSIZ);
    VOC_STK_ARRAY(ctx, char *,   newcmd,    VOCBUFSIZ);
    VOC_STK_ARRAY(ctx, char,     newcmdbuf, VOCBUFSIZ);

	preparseCmd_stat.active = 0;
	preparseCmd_stat.cur = 0;
	preparseCmd_stat.cmd = nullptr;
	preparseCmd_stat.cmdbuf = nullptr;

    /* save the original command buf in case we need to redo the command */
    origcmdbuf = cmdbuf;

    /* clear out the type list */
    memset(typelist, 0, VOCBUFSIZ*sizeof(typelist[0]));

    /* get the types of the words in the command */
    if (vocgtyp(ctx, cmd, typelist, cmdbuf))
    {
        retval = 1;
        goto done;
    }

    /* start off at the first word */
    cur = next = first_word = 0;

    /* 
     *   Presume we will be in control of the next word - when execmd() or
     *   another routine we call decides where the command ends, it will
     *   fill in a new value here.  When this value is non-zero, it will
     *   tell us where the next sentence start is relative to the previous
     *   sentence start.  
     */
    next_start = 0;

    /* we don't have a preparseCmd result yet */
    preparseCmd_stat.active = FALSE;

    /* keep going until we run out of work to do */
    for (again = FALSE, err = 0 ; ; again = TRUE)
    {
        /*
         *   if preparseCmd sent us back a list, parse that list as a new
         *   command 
         */
        if (err == ERR_PREPRSCMDREDO)
        {
            uchar  *src;
            size_t  len;
            size_t  curlen;
            char   *dst;
            //int     cnt;

            /* don't allow a preparseCmd to loop */
            if (preparseCmd_stat.active)
            {
                vocerr(ctx, VOCERR(34),
                       "Internal game error: preparseCmd loop");
                retval = 1;
                goto done;
            }
            
            /* save our status prior to processing the preparseCmd list */
            preparseCmd_stat.active = TRUE;
            preparseCmd_stat.cur = cur;
            preparseCmd_stat.cmd = cmd;
            preparseCmd_stat.cmdbuf = cmdbuf;

            /* set up with the new command */
            cmd = newcmd;
            cmdbuf = newcmdbuf;
            cur = 0;

            /* break up the list into the new command buffer */
            src = preparse_list;
            len = osrp2(src) - 2;
            for (src += 2, dst = cmdbuf, cnt = 0 ; len ; )
            {
                /* make sure the next element is a string */
                if (*src != DAT_SSTRING)
                {
                    vocerr(ctx, VOCERR(32),
                 "Internal game error: preparseCmd returned an invalid list");
                    retval = 1;
                    goto done;
                }

                /* get the string */
                ++src;
                curlen = osrp2(src) - 2;
                src += 2;

                /* make sure it will fit in the buffer */
                if (dst + curlen + 1 >= cmdbuf + VOCBUFSIZ)
                {
                    vocerr(ctx, VOCERR(33),
                          "Internal game error: preparseCmd command too long");
                    retval = 1;
                    goto done;
                }

                /* store the word */
                cmd[cnt++] = dst;
                memcpy(dst, src, curlen);
                dst[curlen] = '\0';

                /* move on to the next word */
                len -= 3 + curlen;
                src += curlen;
                dst += curlen + 1;
            }

            /* enter a null last word */
            cmd[cnt] = 0;

            /* generate the type list for the new list */
            if (vocgtyp(ctx, cmd, typelist, cmdbuf))
            {
                /* return an error */
                retval = 1;
                goto done;
            }

            /*
             *   this is not a new command - it's just further processing
             *   of the current command 
             */
            again = FALSE;

            /* clear the error */
            err = 0;
        }

        /* initialize locals */
        cmdPrep  = MCMONINV;                       /* assume no preposition */
        swapObj  = FALSE;                      /* assume no object swapping */
        dolist[0].vocolobj = iolist[0].vocolobj = MCMONINV;
        dolist[0].vocolflg = iolist[0].vocolflg = 0;

        /* check error return from vocready (which returns from execmd) */
        if (err)
        {
            /* return the error */
            retval = err;
            goto done;
        }

    skip_leading_stuff:
        /* 
         *   If someone updated the sentence start point, jump there.  The
         *   sentence start is relative to the previous sentence start. 
         */
        if (next_start != 0)
            cur = first_word + next_start;

        /* clear next_start, so we can tell if someone updates it */
        next_start = 0;
        
        /* skip any leading THEN's and AND's */
        while (cmd[cur] && (vocspec(cmd[cur], VOCW_THEN)
                            || vocspec(cmd[cur], VOCW_AND)))
            ++cur;

        /* see if there's anything left to parse */
        if (cmd[cur] == 0)
        {
            /*
             *   if we've been off doing preparseCmd work, return to the
             *   original command list 
             */
            if (preparseCmd_stat.active)
            {
                /* restore the original status */
                cur = preparseCmd_stat.cur;
                cmd = preparseCmd_stat.cmd;
                cmdbuf = preparseCmd_stat.cmdbuf;
                preparseCmd_stat.active = FALSE;
                
                /* get the type list for the original list again */
                if (vocgtyp(ctx, cmd, typelist, cmdbuf))
                {
                    /* return the error */
                    retval = 1;
                    goto done;
                }

                /* try again */
                goto skip_leading_stuff;
            }
            else
            {
                /* nothing to pop - we must be done */
                retval = 0;
                goto done;
            }
        }

        /* 
         *   display a blank line if this is not the first command on this
         *   command line, so that we visually separate the results of the
         *   new command from the results of the previous command 
         */
        if (again)
            outformat("\\b");                   /* tioblank(ctx->voccxtio); */

        {
            /* look for an explicit actor in the command */
            if ((o = vocgetactor(ctx, cmd, typelist, cur, &next, cmdbuf))
                != MCMONINV)
            {
                /* skip the actor noun phrase in the input */
                cur = next;

                /* remember the actor internally */
                cmdActor = *cmdActorp = o;

                /* set the actor in the context */
                ctx->voccxactor = o;
            }

            /* if the actor parsing failed, return an error */
            if (cur != next)
            {
                /* error getting actor */
                retval = 1;
                goto done;
            }
        }

        /* remember where the sentence starts */
        first_word = cur;

        /* make sure we have a verb */
        if ((cmd[cur] == (char *)0) || !(typelist[cur] & VOCT_VERB))
        {
            /* unknown verb - handle it with parseUnknownVerb if possible */
            if (!try_unknown_verb(ctx, cmdActor, &cmd[cur], &typelist[cur],
                                  0, &next_start, TRUE, VOCERR(17),
                                  "There's no verb in that sentence!"))
            {
                /* error - abort the command */
                retval = 1;
                goto done;
            }
            else
            {
                /* go back for more */
                continue;
            }
        }
        vverb = cmd[cur++];                             /* this is the verb */
        vvlen = strlen(vverb);                   /* remember length of verb */
        vprep = 0;                            /* assume no verb-preposition */

        /* execute if the command is just a verb */
        if (vocready(ctx, cmd, typelist, cur, cmdActor, cmdPrep,
                     vverb, vprep, dolist, iolist, &err, cmdbuf,
                     first_word, &preparse_list, &next_start))
            continue;

        /*
         *   If the next word is a preposition, and it makes sense to be
         *   aggregated with this verb, use it as such.
         */
        if (typelist[cur] & VOCT_PREP)
        {
            if (vocffw(ctx, vverb, vvlen, cmd[cur], (int)strlen(cmd[cur]),
                       PRP_VERB, (vocseadef *)0))
            {
                vprep = cmd[cur++];
                if (vocready(ctx, cmd, typelist, cur, cmdActor, cmdPrep,
                             vverb, vprep, dolist, iolist, &err, cmdbuf,
                             first_word, &preparse_list, &next_start))
                    continue;
            }
            else
            {
                /*
                 *   If we have a preposition which can NOT be aggregated
                 *   with the verb, take command of this form: "verb prep
                 *   iobj dobj".  Note that we do *not* do this if the
                 *   word is also a noun, or it's an adjective and a noun
                 *   (possibly separated by one or more adjectives)
                 *   follows.  
                 */
                if ((v = vocffw(ctx, cmd[cur], (int)strlen(cmd[cur]),
                                (char *)0, 0, PRP_PREP, (vocseadef *)0)) != 0)
                {
                    int swap_ok;

                    /* if it can be an adjective, check further */
                    if (typelist[cur] & VOCT_NOUN)
                    {
                        /* don't allow the swap */
                        swap_ok = FALSE;
                    }
                    else if (typelist[cur] & VOCT_ADJ)
                    {
                        int i;

                        /* look for a noun, possibly preceded by adj's */
                        for (i = cur + 1 ;
                             cmd[i] && (typelist[i] & VOCT_ADJ)
                             && !(typelist[i] & VOCT_NOUN) ; ++i) ;
                        swap_ok = (!cmd[i] || !(typelist[i] & VOCT_NOUN));
                    }
                    else
                    {
                        /* we can definitely allow this swap */
                        swap_ok = TRUE;
                    }

                    if (swap_ok)
                    {
                        cmdPrep = v->vocwobj;
                        swapObj = TRUE;
                        ++cur;
                    }
                }
            }
        }

    retry_swap:
        /* get the direct object if there is one */
        if ((cnt = vocchknoun2(ctx, cmd, typelist, cur, &next, dolist,
                               FALSE, &no_match)) > 0)
        {
            /* we found a noun phrase matching one or more objects */
            cur = next;
        }
        else if (no_match)
        {
            /* 
             *   we found a valid noun phrase, but we didn't find any
             *   objects that matched the words -- get the noun again,
             *   this time showing the error 
             */
            vocgetnoun(ctx, cmd, typelist, cur, &next, dolist);

            /* return the error */
            retval = 1;
            goto done;
        }
        else if (cnt < 0)
        {
            /* invalid syntax - return failure */
            retval = 1;
            goto done;
        }
        else
        {
            /*
             *   If we thought we were going to get a two-object
             *   sentence, and we got a zero-object sentence, and it looks
             *   like the word we took as a preposition is also an
             *   adjective or noun, go back and treat it as such. 
             */
            if (swapObj &&
                ((typelist[cur-1] & VOCT_NOUN)
                 || (typelist[cur-1] & VOCT_ADJ)))
            {
                --cur;
                swapObj = FALSE;
                cmdPrep = MCMONINV;
                goto retry_swap;
            }

        bad_sentence:
            /* find the last word */
            while (cmd[cur]) ++cur;
            
            /* try running the sentence through preparseCmd */
            err = try_preparse_cmd(ctx, &cmd[first_word], cur - first_word,
                                   &preparse_list);
            switch(err)
            {
            case 0:
                /* preparseCmd didn't do anything - the sentence fails */
                if (!try_unknown_verb(ctx, cmdActor, &cmd[first_word],
                                      &typelist[first_word], 0, &next_start,
                                      TRUE, VOCERR(18),
                                      "I don't understand that sentence."))
                {
                    /* error - abort the command */
                    retval = 1;
                    goto done;
                }
                else
                {
                    /* success - go back for more */
                    continue;
                }

            case ERR_PREPRSCMDCAN:
                /* they cancelled - we're done with the sentence */
                retval = 0;
                goto done;

            case ERR_PREPRSCMDREDO:
                /* reparse with the new sentence */
                continue;
            }
        }

        /* see if we want to execute the command now */
        if (vocready(ctx, cmd, typelist, cur, cmdActor, cmdPrep,
                     vverb, vprep,
                     swapObj ? iolist : dolist,
                     swapObj ? dolist : iolist,
                     &err, cmdbuf, first_word, &preparse_list,
                     &next_start))
            continue;
        
        /*
         *   Check for an indirect object, which may or may not be preceded
         *   by a preposition.  (Note that the lack of a preposition implies
         *   that the object we already found is the indirect object, and the
         *   next object is the direct object.  It also implies a preposition
         *   of "to.")
         */
        if (cmdPrep == MCMONINV && (typelist[cur] & VOCT_PREP))
        {
            char *p1 = cmd[cur++];

            /* if this is the end of the sentence, add the prep to the verb */
            if (cmd[cur] == (char *)0
                || vocspec(cmd[cur], VOCW_AND)
                || vocspec(cmd[cur], VOCW_THEN))
            {
                if (vocffw(ctx, vverb, vvlen, p1, (int)strlen(p1), PRP_VERB,
                           (vocseadef *)0)
                    && !vprep)
                    vprep = p1;
                else
                {
                    /* call parseUnknownVerb */
                    if (!try_unknown_verb(ctx, cmdActor, &cmd[first_word],
                                          &typelist[first_word], 0,
                                          &next_start, TRUE, VOCERR(19),
                        "There are words after your command I couldn't use."))
                    {
                        /* error - abandon the command */
                        retval = 1;
                        goto done;
                    }
                    else
                    {
                        /* go back for more */
                        continue;
                    }
                }
                
                if ((err = execmd(ctx, cmdActor, cmdPrep, vverb, vprep,
                                  dolist, iolist,
                                  &cmd[first_word], &typelist[first_word],
                                  cmdbuf, cur - first_word,
                                  &preparse_list, &next_start)) != 0)
                {
                    retval = 1;
                    goto done;
                }
                continue;
            }

            /*
             *   If we have no verb preposition already, and we have
             *   another prep-capable word following this prep-capable
             *   word, and this preposition aggregates with the verb, take
             *   it as a sentence of the form "pry box open with crowbar"
             *   (where the current word is "with").  We also must have at
             *   least one more word after that, since there will have to
             *   be an indirect object.  
             */
            if (cmd[cur] && (typelist[cur] & VOCT_PREP) && cmd[cur+1]
                && vprep == 0
                && vocffw(ctx, vverb, vvlen, p1, (int)strlen(p1), PRP_VERB,
                          (vocseadef *)0))
            {
                /* 
                 *   check to make sure that the next word, which we're
                 *   about to take for a prep (the "with" in "pry box open
                 *   with crowbar") is actually not part of an object name
                 *   - if it is, use it as the object name rather than as
                 *   the prep 
                 */
                if (vocgobj(ctx, cmd, typelist, cur, &next,
                            FALSE, iolist, FALSE, FALSE, 0) <= 0)
                {
                    /* aggregate the first preposition into the verb */
                    vprep = p1;

                    /* use the current word as the object-introducing prep */
                    p1 = cmd[cur++];
                }
            }

            /* try for an indirect object */
            cnt = vocgobj(ctx, cmd, typelist, cur, &next, TRUE, iolist,
                          TRUE, FALSE, &no_match);
            if (cnt > 0)
            {
                cur = next;
                v = vocffw(ctx, p1, (int)strlen(p1), (char *)0, 0, PRP_PREP,
                           (vocseadef *)0);
                if (v == (vocwdef *)0)
                {
                    /* let parseUnknownVerb handle it */
                    if (!try_unknown_verb(ctx, cmdActor, &cmd[first_word],
                                          &typelist[first_word], 0,
                                          &next_start, TRUE, VOCERR(20),
                   "I don't know how to use the word \"%s\" like that.", p1))
                    {
                        /* error - abort the command */
                        retval = 1;
                        goto done;
                    }
                    else
                    {
                        /* go on to the next command */
                        continue;
                    }
                }
                cmdPrep = v->vocwobj;

                if (vocready(ctx, cmd, typelist, cur, cmdActor, cmdPrep,
                             vverb, vprep, dolist, iolist, &err, cmdbuf,
                             first_word, &preparse_list, &next_start))
                    continue;
                else if ((typelist[cur] & VOCT_PREP) &&
                         vocffw(ctx, vverb, vvlen, cmd[cur],
                                (int)strlen(cmd[cur]), PRP_VERB,
                                (vocseadef *)0) && !vprep)
                {
                    vprep = cmd[cur++];
                    if (vocready(ctx, cmd, typelist, cur, cmdActor,
                                 cmdPrep, vverb, vprep, dolist, iolist,
                                 &err, cmdbuf, first_word, &preparse_list,
                                 &next_start))
                        continue;
                    else
                    {
                        /* let parseUnknownVerb handle it */
                        if (!try_unknown_verb(ctx, cmdActor, &cmd[first_word],
                                              &typelist[first_word], 0,
                                              &next_start, TRUE, VOCERR(19),
                         "There are words after your command I couldn't use."))
                        {
                            /* error - abandon the command */
                            retval = 1;
                            goto done;
                        }
                        else
                        {
                            /* go on to the next command */
                            continue;
                        }
                    }
                }
                else
                {
                    /*
                     *   If the latter object phrase is flagged with the
                     *   "trimmed preposition" flag, meaning that we could
                     *   have used the preposition in the noun phrase but
                     *   assumed instead it was part of the verb, reverse
                     *   this assumption now: add the preposition back to the
                     *   noun phrase and explain that there's no such thing
                     *   present.
                     *   
                     *   Otherwise, we simply have an unknown verb phrasing,
                     *   so let parseUnknownVerb handle it.  
                     */
                    vocoldef *np1 =
                        (dolist[0].vocolflg & VOCS_TRIMPREP) != 0
                        ? &dolist[0]
                        : (iolist[0].vocolflg & VOCS_TRIMPREP) != 0
                        ? &iolist[0]
                        : 0;
                    if (np1 != 0)
                    {
                        char namebuf[VOCBUFSIZ];
                        
                        /* show the name, adding the prep back in */
                        voc_make_obj_name_from_list(
                            ctx, namebuf, cmd, np1->vocolfst, np1->vocolhlst);
                        vocerr(ctx, VOCERR(9), "I don't see any %s here.",
                               namebuf);

                        /* error - abort */
                        retval = 1;
                        goto done;
                    }
                    else if (!try_unknown_verb(
                        ctx, cmdActor,
                        &cmd[first_word], &typelist[first_word],
                        0, &next_start, TRUE, VOCERR(19),
                  "There are words after your command that I couldn't use."))
                    {
                        /* error - abort */
                        retval = 1;
                        goto done;
                    }
                    else
                    {
                        /* continue with the next command */
                        continue;
                    }
                }
            }
            else if (cnt < 0)
            {
                /* 
                 *   the noun phrase syntax was invalid - we've already
                 *   displayed an error about it, so simply return failure 
                 */
                retval = 1;
                goto done;
            }
            else if (no_match)
            {
                /* 
                 *   we found a valid noun phrase, but we didn't find any
                 *   matching objects - we've already generated an error,
                 *   so simply return failure 
                 */
                retval = 1;
                goto done;
            }
            else
            {
                goto bad_sentence;
            }
        }
        else if ((cnt = vocchknoun(ctx, cmd, typelist, cur,
                                   &next, iolist, FALSE)) > 0)
        {
            /* look for prep at end of command */
            cur = next;
            if (cmd[cur])
            {
                if ((typelist[cur] & VOCT_PREP) &&
                    vocffw(ctx, vverb, vvlen, cmd[cur],
                           (int)strlen(cmd[cur]), PRP_VERB,
                           (vocseadef *)0) && !vprep)
                {
                    vprep = cmd[cur++];
                }
            }

            /* the command should definitely be done now */
            if (cmd[cur] != 0)
            {
                /* let parseUnknownVerb handle it */
                if (!try_unknown_verb(ctx, cmdActor, &cmd[first_word],
                                      &typelist[first_word], 0,
                                      &next_start, TRUE, VOCERR(21),
                        "There appear to be extra words after your command."))
                {
                    /* error - stop the command */
                    retval = 1;
                    goto done;
                }
                else
                {
                    /* go on to the next command */
                    continue;
                }
            }
                
            /*
             *   If we don't have a preposition yet, we need to find the
             *   verb's default.  If the verb object has a nilPrep
             *   property defined, use that prep object; otherwise, look
             *   up the word "to" and use that.  
             */
            if (cmdPrep == MCMONINV &&
                (v = vocffw(ctx, vverb, vvlen,
                            vprep, (vprep ? (int)strlen(vprep) : 0),
                            PRP_VERB, (vocseadef *)0)) != 0)
            {
                runppr(ctx->voccxrun, v->vocwobj, PRP_NILPREP, 0);
                if (runtostyp(ctx->voccxrun) == DAT_OBJECT)
                    cmdPrep = runpopobj(ctx->voccxrun);
                else
                    rundisc(ctx->voccxrun);
            }

            /* if we didn't find anything with nilPrep, find "to" */
            if (cmdPrep == MCMONINV)
            {
                v = vocffw(ctx, "to", 2, (char *)0, 0, PRP_PREP,
                           (vocseadef *)0);
                if (v) cmdPrep = v->vocwobj;
            }

            /* execute the command */
            err = execmd(ctx, cmdActor, cmdPrep, vverb, vprep,
                         iolist, dolist,
                         &cmd[first_word], &typelist[first_word], cmdbuf,
                         cur - first_word, &preparse_list, &next_start);
            continue;
        }
        else if (cnt < 0)
        {
            retval = 1;
            goto done;
        }
        else
        {
            goto bad_sentence;
        }
    }

done:
    /* copy back the command if we need to redo */
    if (ctx->voccxredo && cmdbuf != origcmdbuf)
        strcpy(origcmdbuf, cmdbuf);
                    
    /* return the status */
    VOC_RETVAL(ctx, save_sp, retval);
}

/* execute a player command */
int voccmd(voccxdef *ctx, char *cmd, uint cmdlen)
{
    int      wrdcnt;
	int      cur = 0;
    int      next;
    char    *buffer;
    char   **wordlist;
    objnum   cmdActor;
    int      first;

    /* 
     *   Make sure the stack is set up, resetting the stack on entry. Note
     *   that this makes this routine non-reentrant - recursively calling
     *   this routine will wipe out the enclosing caller's stack. 
     */
    voc_stk_ini(ctx, (uint)VOC_STACK_SIZE);

    /* allocate our stack arrays */
    VOC_STK_ARRAY(ctx, char,   buffer,   2*VOCBUFSIZ);
    VOC_STK_ARRAY(ctx, char *, wordlist, VOCBUFSIZ);
    
    /* until further notice, the actor for formatStrings is Me */
    tiosetactor(ctx->voccxtio, ctx->voccxme);

    /* clear the 'ignore' flag */
    ctx->voccxflg &= ~VOCCXFCLEAR;

    /* send to game function 'preparse' */
    if (ctx->voccxpre != MCMONINV)
    {
        int      typ;
        uchar   *s;
        size_t   len;
        int      err;

        /* push the argument string */
        runpstr(ctx->voccxrun, cmd, (int)strlen(cmd), 0);

        /* presume no error will occur */
        err = 0;

        /* catch errors that occur during preparse() */
        ERRBEGIN(ctx->voccxerr)
        {
            /* call preparse() */
            runfn(ctx->voccxrun, ctx->voccxpre, 1);
        }
        ERRCATCH(ctx->voccxerr, err)
        {
            /* see what we have */
            switch(err)
            {
            case ERR_RUNEXIT:
            case ERR_RUNEXITOBJ:
            case ERR_RUNABRT:
                /* 
                 *   if we encountered abort, exit, or exitobj, treat it
                 *   the same as a nil result code - simply cancel the
                 *   entire current command 
                 */
                break;

            default:
                /* re-throw any other error */
                errrse(ctx->voccxerr);
            }
        }
        ERREND(ctx->voccxerr);

        /* if an error occurred, abort the command */
        if (err != 0)
            return 0;

        /* check the return value's type */
        typ = runtostyp(ctx->voccxrun);
        if (typ == DAT_SSTRING)
        {
            /* 
             *   It's a string - replace the command with the new string.
             *   Pop the new string and scan its length prefix. 
             */
            s = runpopstr(ctx->voccxrun);
            len = osrp2(s) - 2;
            s += 2;

            /* 
             *   limit the size of the command to our buffer length,
             *   leaving space for null termination
             */
            if (len > cmdlen - 1)
                len = cmdlen - 1;

            /* copy the new command string into our command buffer */
            memcpy(cmd, s, len);

            /* null-terminate the command buffer */
            cmd[len] = '\0';
        }
        else
        {
            /* the value isn't important for any other type */
            rundisc(ctx->voccxrun);

            /* if they returned nil, we're done processing the command */
            if (typ == DAT_NIL)
                return 0;
        }
    }

    /* break up into individual words */
    if ((wrdcnt = voctok(ctx, cmd, buffer, wordlist, TRUE, FALSE, TRUE)) > 0)
    {
        /* skip any leading "and" and "then" separators */
        for (cur = 0 ; cur < wrdcnt ; ++cur)
        {
            /* if this isn't "and" or "then", we're done scanning */
            if (!vocspec(wordlist[cur], VOCW_THEN)
                && !vocspec(wordlist[cur], VOCW_AND))
                break;
        }
    }

    /* 
     *   if we found no words, or we found only useless leading "and" and
     *   "then" separators, run the "pardon" function to tell the player
     *   that we didn't find any command on the line 
     */
    if (wrdcnt == 0 || (wrdcnt > 0 && cur >= wrdcnt))
    {
        runfn(ctx->voccxrun, ctx->voccxprd, 0);
        return 0;
    }

    /* 
     *   if we got an error tokenizing the word list, return - we've
     *   already displayed an error message, so there's nothing more for
     *   us to do here 
     */
    if (wrdcnt < 0)
        return 0;

    /* process the commands on the line */
    for (first = TRUE, cmdActor = ctx->voccxactor = MCMONINV ; cur < wrdcnt ;
         ++cur, first = FALSE)
    {
        /* presume we won't find an unknown word */
        ctx->voccxunknown = 0;

        /* find the THEN that ends the command, if there is one */
        for (next = cur ; cur < wrdcnt && !vocspec(wordlist[cur], VOCW_THEN)
             ; ++cur) ;
        wordlist[cur] = (char *)0;

        /* process until we run out of work to do */
        for (;;)
        {
            /* try processing the command */
            if (voc1cmd(ctx, &wordlist[next], cmd, &cmdActor, first))
            {
                /* 
                 *   If the unknown word flag is set, try the command
                 *   again from the beginning.  This flag means that we
                 *   tried using parseUnknownDobj/Iobj to resolve the
                 *   unknown word, but that failed and we need to try
                 *   again with the normal "oops" processing.  
                 */
                if (ctx->voccxunknown > 0)
                    continue;
            
                /* return an error */
                return 1;
            }

            /* success - we're done with the command */
            break;
        }

        /* if the rest of the command is to be ignored, we're done */
        if (ctx->voccxflg & VOCCXFCLEAR)
            return 0;

        /* scan past any separating AND's and THEN's */
        while (cur + 1 < wrdcnt
               && (vocspec(wordlist[cur+1], VOCW_THEN)
                   || vocspec(wordlist[cur+1], VOCW_AND)))
            ++cur;

        /* 
         *   if another command follows, add a blank line to separate the
         *   results from the previous command and those from the next
         *   command 
         */
        if (cur + 1 < wrdcnt)
            outformat("\\b");
    }

    /* done */
    return 0;
}


/*
 *   Off-stack stack management 
 */

/* allocate/reset the stack */
void voc_stk_ini(voccxdef *ctx, uint siz)
{
    /* allocate it if it's not already allocated */
    if (ctx->voc_stk_ptr == 0)
    {
        ctx->voc_stk_ptr = mchalo(ctx->voccxerr, siz, "voc_stk_ini");
        ctx->voc_stk_end = ctx->voc_stk_ptr + siz;
    }
    
    /* reset the stack */
    ctx->voc_stk_cur = ctx->voc_stk_ptr;
}

/* allocate space from the off-stack stack */
void *voc_stk_alo(voccxdef *ctx, uint siz)
{
    void *ret;
    
    /* round size up to allocation units */
    siz = osrndsz(siz);

    /* if there's not space, signal an error */
    if (ctx->voc_stk_cur + siz > ctx->voc_stk_end)
        errsig(ctx->voccxerr, ERR_VOCSTK);

    /* save the return pointer */
    ret = ctx->voc_stk_cur;

    /* consume the space */
    ctx->voc_stk_cur += siz;

/*#define SHOW_HI*/
#ifdef SHOW_HI
{
    static uint maxsiz;
    if (ctx->voc_stk_cur - ctx->voc_stk_ptr > maxsiz)
    {
        char buf[20];
        
        maxsiz = ctx->voc_stk_cur - ctx->voc_stk_ptr;
        sprintf(buf, "%u\n", maxsiz);
        os_printz(buf);
    }
}
#endif


    /* return the space */
    return ret;
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
