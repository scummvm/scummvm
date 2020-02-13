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

#include "glk/tads/tads2/file_io.h"
#include "glk/tads/tads2/appctx.h"
#include "glk/tads/tads2/character_map.h"
#include "glk/tads/tads2/error.h"
#include "glk/tads/tads2/memory_cache_heap.h"
#include "glk/tads/tads2/os.h"
#include "glk/tads/tads2/run.h"
#include "glk/tads/tads2/tokenizer.h"
#include "glk/tads/tads2/vocabulary.h"
#include "glk/tads/os_glk.h"

namespace Glk {
namespace TADS {
namespace TADS2 {


/* compare a resource string */
/* int fioisrsc(uchar *filbuf, char *refnam); */
#define fioisrsc(filbuf, refnam) \
  (((filbuf)[0] == strlen(refnam)) && \
   !memcmp(filbuf+1, refnam, (size_t)((filbuf)[0])))

/* callback to load an object on demand */
void OS_LOADDS fioldobj(void *ctx0, mclhd handle, uchar *ptr, ushort siz)
{
    fiolcxdef  *ctx = (fiolcxdef *)ctx0;
    ulong       seekpos = (ulong)handle;
    osfildef   *fp = ctx->fiolcxfp;
    char        buf[7];
    errcxdef   *ec = ctx->fiolcxerr;
    uint        rdsiz = 0;
    
    /* figure out what type of object is to be loaded */
    osfseek(fp, seekpos + ctx->fiolcxst, OSFSK_SET);
    if (osfrb(fp, buf, 7)) errsig(ec, ERR_LDGAM);
    switch(buf[0])
    {
    case TOKSTFUNC:
        rdsiz = osrp2(buf + 3);
        break;

    case TOKSTOBJ:
        rdsiz = osrp2(buf + 5);
        break;
        
    case TOKSTFWDOBJ:
    case TOKSTFWDFN:
    default:
        errsig(ec, ERR_UNKOTYP);
    }
    
    if (siz < rdsiz) errsig(ec, ERR_LDBIG);
    if (osfrb(fp, ptr, rdsiz)) errsig(ec, ERR_LDGAM);
    if (ctx->fiolcxflg & FIOFCRYPT)
        fioxor(ptr, rdsiz, ctx->fiolcxseed, ctx->fiolcxinc);
}

/* shut down load-on-demand subsystem (close load file) */
void fiorcls(fiolcxdef *ctx)
{
    if (ctx != 0 && ctx->fiolcxfp != 0)
    {
        /* close the file */
        osfcls(ctx->fiolcxfp);

        /* forget the file object */
        ctx->fiolcxfp = 0;
    }
}

/*
 *   Read an HTMLRES resource map 
 */
static void fiordhtml(errcxdef *ec, osfildef *fp, appctxdef *appctx,
                      int resfileno, const char *resfilename)
{
    uchar buf[256];

    /* 
     *   resource map - if the host system is interested, tell it about it 
     */
    if (appctx != 0)
    {
        ulong entry_cnt;
        ulong i;
        
        /* read the index table header */
        if (osfrb(fp, buf, 8))
            errsig1(ec, ERR_RDRSC, ERRTSTR,
                    errstr(ec, resfilename, strlen(resfilename)));

        /* get the number of entries in the table */
        entry_cnt = osrp4(buf);
        
        /* read the index entries */
        for (i = 0 ; i < entry_cnt ; ++i)
        {
            ulong res_ofs;
            ulong res_siz;
            ushort res_namsiz;
            
            /* read this entry */
            if (osfrb(fp, buf, 10))
                errsig1(ec, ERR_RDRSC, ERRTSTR,
                        errstr(ec, resfilename, strlen(resfilename)));

            /* get the entry header */
            res_ofs = osrp4(buf);
            res_siz = osrp4(buf + 4);
            res_namsiz = osrp2(buf + 8);
            
            /* read this entry's name */
            if (osfrb(fp, buf, res_namsiz))
                errsig1(ec, ERR_RDRSC, ERRTSTR,
                        errstr(ec, resfilename, strlen(resfilename)));
            
            /* tell the host system about this entry */
            if (appctx->add_resource)
                (*appctx->add_resource)(appctx->add_resource_ctx,
                                        res_ofs, res_siz,
                                        (char *)buf,
                                        (size_t)res_namsiz,
                                        resfileno);
        }
        
        /* tell the host system where the resources start */
        if (appctx->set_resmap_seek != 0)
        {
            long pos = osfpos(fp);
            (*appctx->set_resmap_seek)(appctx->set_resmap_seek_ctx,
                                       pos, resfileno);
        }
    }
}

/*
 *   Read an external resource file.  This is a limited version of the
 *   general file reader that can only read resource files, not full game
 *   files.  
 */
static void fiordrscext(errcxdef *ec, osfildef *fp, appctxdef *appctx,
                        int resfileno, char *resfilename)
{
    uchar buf[TOKNAMMAX + 50];
    unsigned long endpos;
    unsigned long startofs;

    /* note the starting offset */
    startofs = osfpos(fp);
    
    /* check file and version headers, and get flags and timestamp */
    if (osfrb(fp, buf, (int)(sizeof(FIOFILHDR) + sizeof(FIOVSNHDR) + 2)))
        errsig1(ec, ERR_RDRSC, ERRTSTR,
                errstr(ec, resfilename, strlen(resfilename)));
    if (memcmp(buf, FIOFILHDRRSC, (size_t)sizeof(FIOFILHDRRSC)))
        errsig1(ec, ERR_BADHDRRSC, ERRTSTR,
                errstr(ec, resfilename, strlen(resfilename)));
    if (memcmp(buf + sizeof(FIOFILHDR), FIOVSNHDR,
               (size_t)sizeof(FIOVSNHDR))
        && memcmp(buf + sizeof(FIOFILHDR), FIOVSNHDR2,
                  (size_t)sizeof(FIOVSNHDR2))
        && memcmp(buf + sizeof(FIOFILHDR), FIOVSNHDR3,
                  (size_t)sizeof(FIOVSNHDR3)))
        errsig(ec, ERR_BADVSN);
    if (osfrb(fp, buf, (size_t)26))
        errsig1(ec, ERR_RDRSC, ERRTSTR,
                errstr(ec, resfilename, strlen(resfilename)));

    /* now read resources from the file */
    for (;;)
    {
        /* read resource type and next-resource pointer */
        if (osfrb(fp, buf, 1)
            || osfrb(fp, buf + 1, (int)(buf[0] + 4)))
            errsig1(ec, ERR_RDRSC, ERRTSTR,
                    errstr(ec, resfilename, strlen(resfilename)));
        endpos = osrp4(buf + 1 + buf[0]);

        /* check the resource type */
        if (fioisrsc(buf, "HTMLRES"))
        {
            /* read the HTML resource map */
            fiordhtml(ec, fp, appctx, resfileno, resfilename);

            /* 
             *   skip the resources - they're entirely for the host
             *   application's use 
             */
            osfseek(fp, endpos + startofs, OSFSK_SET);
        }
        else if (fioisrsc(buf, "$EOF"))
        {
            /* we're done reading the file */
            break;
        }
        else
            errsig(ec, ERR_UNKRSC);
    }
}

/*
 *   read a game from a binary file
 *
 *   flags:
 *      &1 ==> run preinit
 *      &2 ==> preload objects
 */
static void fiord1(mcmcxdef *mctx, voccxdef *vctx, tokcxdef *tctx,
                   osfildef *fp, const char *fname,
                   fiolcxdef *setupctx, ulong startofs,
                   objnum *preinit, uint *flagp, tokpdef *path,
                   uchar **fmtsp, uint *fmtlp, uint *pcntptr, int flags,
                   appctxdef *appctx, char *argv0)
{
    int         i;
    int         siz;
    uchar       buf[TOKNAMMAX + 50];
    errcxdef   *ec = vctx->voccxerr;
    ulong       endpos;
    int         obj;
    ulong       curpos;
    runxdef    *ex;
    ulong       eof_reset = 0;             /* reset here at EOF if non-zero */
#if 0  // XFCNs are obsolete
    int         xfcns_done = FALSE;                 /* already loaded XFCNs */
#endif
    ulong       xfcn_pos = 0;          /* location of XFCN's if preloadable */
    uint        xor_seed = 17;                     /* seed value for fioxor */
    uint        xor_inc = 29;                 /* increment value for fioxor */

    /* set up loader callback context */
    setupctx->fiolcxfp = fp;
    setupctx->fiolcxerr = ec;
    setupctx->fiolcxst = startofs;
    setupctx->fiolcxseed = xor_seed;
    setupctx->fiolcxinc = xor_inc;

    /* check file and version headers, and get flags and timestamp */
    if (osfrb(fp, buf, (int)(sizeof(FIOFILHDR) + sizeof(FIOVSNHDR) + 2)))
        errsig(ec, ERR_RDGAM);
    if (memcmp(buf, FIOFILHDR, (size_t)sizeof(FIOFILHDR)))
         errsig(ec, ERR_BADHDR);
    if (memcmp(buf + sizeof(FIOFILHDR), FIOVSNHDR,
               (size_t)sizeof(FIOVSNHDR))
        && memcmp(buf + sizeof(FIOFILHDR), FIOVSNHDR2,
                  (size_t)sizeof(FIOVSNHDR2))
        && memcmp(buf + sizeof(FIOFILHDR), FIOVSNHDR3,
                  (size_t)sizeof(FIOVSNHDR3)))
        errsig(ec, ERR_BADVSN);
    if (osfrb(fp, vctx->voccxtim, (size_t)26)) errsig(ec, ERR_RDGAM);

    /* 
     *   if the game wasn't compiled with 2.2 or later, make a note,
     *   because we need to ignore certain property flags (due to a bug in
     *   the old compiler) 
     */
    if (memcmp(buf + sizeof(FIOFILHDR), FIOVSNHDR2,
               (size_t)sizeof(FIOVSNHDR2)) == 0
        || memcmp(buf + sizeof(FIOFILHDR), FIOVSNHDR3,
                  (size_t)sizeof(FIOVSNHDR3)) == 0)
        mctx->mcmcxflg |= MCMCXF_NO_PRP_DEL;

    setupctx->fiolcxflg =
        *flagp = osrp2(buf + sizeof(FIOFILHDR) + sizeof(FIOVSNHDR));
    
    /* now read resources from the file */
    for (;;)
    {
        /* read resource type and next-resource pointer */
        if (osfrb(fp, buf, 1)
            || osfrb(fp, buf + 1, (int)(buf[0] + 4)))
            errsig(ec, ERR_RDGAM);
        endpos = osrp4(buf + 1 + buf[0]);
        
        if (fioisrsc(buf, "OBJ"))
        {
            /* skip regular objects if fast-load records are included */
            if (*flagp & FIOFFAST)
            {
                osfseek(fp, endpos + startofs, OSFSK_SET);
                continue;
            }

            curpos = osfpos(fp) - startofs;
            while (curpos != endpos)
            {
                /* read type and object number */
                if (osfrb(fp, buf, 3)) errsig(ec, ERR_RDGAM);
                obj = osrp2(buf+1);

                switch(buf[0])
                {
                case TOKSTFUNC:
                case TOKSTOBJ:
                    if (osfrb(fp, buf + 3, 4)) errsig(ec, ERR_RDGAM);
                    mcmrsrv(mctx, (ushort)osrp2(buf + 3), (mcmon)obj,
                            (mclhd)curpos);
                    curpos += osrp2(buf + 5) + 7;
                    
                    /* load object if preloading */
                    if (flags & 2)
                    {
                        (void)mcmlck(mctx, (mcmon)obj);
                        mcmunlck(mctx, (mcmon)obj);
                    }

                    /* seek past this object */
                    osfseek(fp, curpos + startofs, OSFSK_SET);
                    break;
                    
                case TOKSTFWDOBJ:
                case TOKSTFWDFN:
                {
                    ushort  bsiz;
                    uchar  *p;
                    
                    if (osfrb(fp, buf+3, 2)) errsig(ec, ERR_RDGAM);
                    bsiz = osrp2(buf+3);
                    p = mcmalonum(mctx, bsiz, (mcmon)obj);
                    if (osfrb(fp, p, bsiz)) errsig(ec, ERR_RDGAM);
                    mcmunlck(mctx, (mcmon)obj);
                    curpos += 5 + bsiz;
                    break;
                }
                    
                case TOKSTEXTERN:
                    if (!vctx->voccxrun->runcxext)
                        errsig(ec, ERR_UNXEXT);
                    ex = &vctx->voccxrun->runcxext[obj];

                    if (osfrb(fp, buf + 3, 1)
                        || osfrb(fp, ex->runxnam, (int)buf[3]))
                        errsig(ec, ERR_RDGAM);
                    ex->runxnam[buf[3]] = '\0';
                    curpos += buf[3] + 4;
                    break;
                    
                default:
                    errsig(ec, ERR_UNKOTYP);
                }
            }
        }
        else if (fioisrsc(buf, "FST"))
        {
            uchar *p;
            uchar *bufp;
            ulong  bsiz;
            
            if (!(*flagp & FIOFFAST))
            {
                osfseek(fp, endpos + startofs, OSFSK_SET);
                continue;
            }
            
            curpos = osfpos(fp) - startofs;
            bsiz = endpos - curpos;
            if (bsiz && bsiz < OSMALMAX
                && (bufp = p = (uchar *)osmalloc((size_t)bsiz)) != 0)
            {
                uchar *p1;
                ulong  siz2;
                uint   sizcur;

                for (p1 = p, siz2 = bsiz ; siz2 ; siz2 -= sizcur, p1 += sizcur)
                {
                    sizcur = (siz2 > (uint)0xffff ? (uint)0xffff : siz2);
                    if (osfrb(fp, p1, sizcur)) errsig(ec, ERR_RDGAM);
                }

                while (bsiz)
                {
                    obj = osrp2(p + 1);
                    switch(*p)
                    {
                    case TOKSTFUNC:
                    case TOKSTOBJ:
                        mcmrsrv(mctx, (ushort)osrp2(p + 3), (mcmon)obj,
                                (mclhd)osrp4(p + 7));
                        p += 11;
                        bsiz -= 11;
                        
                        /* preload object if desired */
                        if (flags & 2)
                        {
                            (void)mcmlck(mctx, (mcmon)obj);
                            mcmunlck(mctx, (mcmon)obj);
                        }
                        break;
                        
                    case TOKSTEXTERN:
                        if (!vctx->voccxrun->runcxext)
                            errsig(ec, ERR_UNXEXT);
                        ex = &vctx->voccxrun->runcxext[obj];
                        
                        memcpy(ex->runxnam, p + 4, (size_t)p[3]);
                        ex->runxnam[p[3]] = '\0';
                        bsiz -= p[3] + 4;
                        p += p[3] + 4;
                        break;
                        
                    default:
                        errsig(ec, ERR_UNKOTYP);
                    }
                }
                
                /* done with temporary block; free it */
                osfree(bufp);
                osfseek(fp, endpos + startofs, OSFSK_SET);
            }
            else
            {
                while (curpos != endpos)
                {
                    if (osfrb(fp, buf, 3)) errsig(ec, ERR_RDGAM);
                    obj = osrp2(buf + 1);
                    switch(buf[0])
                    {
                    case TOKSTFUNC:
                    case TOKSTOBJ:
                        if (osfrb(fp, buf + 3, 8)) errsig(ec, ERR_RDGAM);
                        mcmrsrv(mctx, (ushort)osrp2(buf + 3), (mcmon)obj,
                                (mclhd)osrp4(buf + 7));
                        curpos += 11;
                        
                        /* preload object if desired */
                        if (flags & 2)
                        {
                            (void)mcmlck(mctx, (mcmon)obj);
                            mcmunlck(mctx, (mcmon)obj);
                            osfseek(fp, curpos + startofs, OSFSK_SET);
                        }
                        break;
                        
                    case TOKSTEXTERN:
                        if (!vctx->voccxrun->runcxext)
                            errsig(ec, ERR_UNXEXT);
                        ex = &vctx->voccxrun->runcxext[obj];
                        
                        if (osfrb(fp, buf + 3, 1)
                            || osfrb(fp, ex->runxnam, (int)buf[3]))
                            errsig(ec, ERR_RDGAM);
                        ex->runxnam[buf[3]] = '\0';
                        curpos += buf[3] + 4;
                        break;
                        
                    default:
                        errsig(ec, ERR_UNKOTYP);
                    }
                }
            }

            /* if we can preload xfcn's, do so now */
            if (xfcn_pos)
            {
                eof_reset = endpos;    /* remember to return here when done */
                osfseek(fp, xfcn_pos, OSFSK_SET);           /* go to xfcn's */
            }
        }
        else if (fioisrsc(buf, "XFCN"))
        {
            if (!vctx->voccxrun->runcxext) errsig(ec, ERR_UNXEXT);

            /* read length and name of resource */
            if (osfrb(fp, buf, 3) || osfrb(fp, buf + 3, (int)buf[2]))
                errsig(ec, ERR_RDGAM);
            siz = osrp2(buf);

#if 0
/* 
 *   external functions are now obsolete - do not load
 */            

            /* look for an external function with the same name */
            for (i = vctx->voccxrun->runcxexc,  ex = vctx->voccxrun->runcxext
                 ; i ; ++ex, --i)
            {
                j = strlen(ex->runxnam);
                if (j == buf[2] && !memcmp(buf + 3, ex->runxnam, (size_t)j))
                    break;
            }

            /* if we found an external function of this name, load it */
            if (i && !xfcns_done)
            {
                /* load the function */
                ex->runxptr = os_exfld(fp, (unsigned)siz);
            }
            else
            {
                /* this XFCN isn't used; don't bother loading it */
                osfseek(fp, endpos + startofs, OSFSK_SET);
            }
#else
            /* external functions are obsolete; simply skip the data */
            osfseek(fp, endpos + startofs, OSFSK_SET);
#endif
        }
        else if (fioisrsc(buf, "HTMLRES"))
        {
            /* read the resources */
            fiordhtml(ec, fp, appctx, 0, fname);

            /* 
             *   skip the resources - they're entirely for the host
             *   application's use 
             */
            osfseek(fp, endpos + startofs, OSFSK_SET);
        }
        else if (fioisrsc(buf, "INH"))
        {
            uchar *p;
            uchar *bufp;
            ulong  bsiz;
            
            /* do it in a single file read, if we can, for speed */
            curpos = osfpos(fp) - startofs;
            bsiz = endpos - curpos;
            if (bsiz && bsiz < OSMALMAX
                && (bufp = p = (uchar *)osmalloc((size_t)bsiz)) != 0)
            {
                uchar *p1;
                ulong  siz2;
                uint   sizcur;

                for (p1 = p, siz2 = bsiz ; siz2 ; siz2 -= sizcur, p1 += sizcur)
                {
                    sizcur = (siz2 > (uint)0xffff ? (uint)0xffff : siz2);
                    if (osfrb(fp, p1, sizcur)) errsig(ec, ERR_RDGAM);
                }

                while (bsiz)
                {
                    i = osrp2(p + 7);
                    obj = osrp2(p + 1);
                    
                    vociadd(vctx, (objnum)obj, (objnum)osrp2(p+3), i,
                            (objnum *)(p + 9), p[0] | VOCIFXLAT);
                    vocinh(vctx, obj)->vociilc = osrp2(p + 5);
                    
                    p += 9 + (2 * i);
                    bsiz -= 9 + (2 * i);
                }
                
                /* done with temporary block; free it */
                osfree(bufp);
            }
            else
            {
                while (curpos != endpos)
                {
                    if (osfrb(fp, buf, 9)) errsig(ec, ERR_RDGAM);
                    i = osrp2(buf + 7);       /* get number of superclasses */
                    obj = osrp2(buf + 1);              /* get object number */
                    if (i && osfrb(fp, buf + 9, 2 * i)) errsig(ec, ERR_RDGAM);
                    
                    vociadd(vctx, (objnum)obj, (objnum)osrp2(buf+3),
                            i, (objnum *)(buf + 9), buf[0] | VOCIFXLAT);
                    vocinh(vctx, obj)->vociilc = osrp2(buf + 5);
                
                    curpos += 9 + (2 * i);
                }
            }
        }
        else if (fioisrsc(buf, "REQ"))
        {
            curpos = osfpos(fp) - startofs;
            siz = endpos - curpos;

            if (osfrb(fp, buf, (uint)siz)) errsig(ec, ERR_RDGAM);
            vctx->voccxme  = vctx->voccxme_init = osrp2(buf);
            vctx->voccxvtk = osrp2(buf+2);
            vctx->voccxstr = osrp2(buf+4);
            vctx->voccxnum = osrp2(buf+6);
            vctx->voccxprd = osrp2(buf+8);
            vctx->voccxvag = osrp2(buf+10);
            vctx->voccxini = osrp2(buf+12);
            vctx->voccxpre = osrp2(buf+14);
            vctx->voccxper = osrp2(buf+16);
            
            /* if we have a cmdPrompt function, read it */
            if (siz >= 20)
                vctx->voccxprom = osrp2(buf + 18);
            else
                vctx->voccxprom = MCMONINV;

            /* if we have the NLS functions, read them */
            if (siz >= 26)
            {
                vctx->voccxpdis = osrp2(buf + 20);
                vctx->voccxper2 = osrp2(buf + 22);
                vctx->voccxpdef = osrp2(buf + 24);
            }
            else
            {
                /* the new NLS functions aren't defined in this file */
                vctx->voccxpdis = MCMONINV;
                vctx->voccxper2 = MCMONINV;
                vctx->voccxpdef = MCMONINV;
            }

            /* test for parseAskobj separately, as it was added later */
            if (siz >= 28)
                vctx->voccxpask = osrp2(buf + 26);
            else
                vctx->voccxpask = MCMONINV;

            /* test for preparseCmd separately - it's another late comer */
            if (siz >= 30)
                vctx->voccxppc = osrp2(buf + 28);
            else
                vctx->voccxppc = MCMONINV;

            /* check for parseAskobjActor separately - another late comer */
            if (siz >= 32)
                vctx->voccxpask2 = osrp2(buf + 30);
            else
                vctx->voccxpask2 = MCMONINV;

            /* if we have parseErrorParam, read it as well */
            if (siz >= 34)
            {
                vctx->voccxperp = osrp2(buf + 32);
            }
            else
            {
                /* parseErrorParam isn't defined in this file */
                vctx->voccxperp = MCMONINV;
            }

            /* 
             *   if we have commandAfterRead and initRestore, read them as
             *   well 
             */
            if (siz >= 38)
            {
                vctx->voccxpostprom = osrp2(buf + 34);
                vctx->voccxinitrestore = osrp2(buf + 36);
            }
            else
            {
                /* these new functions aren't defined in this game */
                vctx->voccxpostprom = MCMONINV;
                vctx->voccxinitrestore = MCMONINV;
            }

            /* check for and read parseUnknownVerb, parseNounPhrase */
            if (siz >= 42)
            {
                vctx->voccxpuv = osrp2(buf + 38);
                vctx->voccxpnp = osrp2(buf + 40);
            }
            else
            {
                vctx->voccxpuv = MCMONINV;
                vctx->voccxpnp = MCMONINV;
            }

            /* check for postAction, endCommand */
            if (siz >= 48)
            {
                vctx->voccxpostact = osrp2(buf + 42);
                vctx->voccxendcmd = osrp2(buf + 44);
                vctx->voccxprecmd = osrp2(buf + 46);
            }
            else
            {
                vctx->voccxpostact = MCMONINV;
                vctx->voccxendcmd = MCMONINV;
                vctx->voccxprecmd = MCMONINV;
            }

            /* check for parseAskobjIndirect */
            if (siz >= 50)
                vctx->voccxpask3 = osrp2(buf + 48);
            else
                vctx->voccxpask3 = MCMONINV;

            /* check for preparseExt and parseDefaultExt */
            if (siz >= 54)
            {
                vctx->voccxpre2 = osrp2(buf + 50);
                vctx->voccxpdef2 = osrp2(buf + 52);
            }
            else
            {
                vctx->voccxpre2 = MCMONINV;
                vctx->voccxpdef2 = MCMONINV;
            }
        }
        else if (fioisrsc(buf, "VOC"))
        {
            uchar *p;
            uchar *bufp;
            ulong  bsiz;
            int    len1 = 0;
            int    len2 = 0;
            
            /* do it in a single file read, if we can, for speed */
            curpos = osfpos(fp) - startofs;
            bsiz = endpos - curpos;
            if (bsiz && bsiz < OSMALMAX
                && (bufp = p = (uchar *)osmalloc((size_t)bsiz)) != 0)
            {
                uchar *p1;
                ulong  siz2;
                uint   sizcur;

                for (p1 = p, siz2 = bsiz ; siz2 ; siz2 -= sizcur, p1 += sizcur)
                {
                    sizcur = (siz2 > (uint)0xffff ? (uint)0xffff : siz2);
                    if (osfrb(fp, p1, sizcur)) errsig(ec, ERR_RDGAM);
                }
                
                while (bsiz)
                {
                    len1 = osrp2(p);
                    len2 = osrp2(p + 2);
                    if (*flagp & FIOFCRYPT)
                        fioxor(p + 10, (uint)(len1 + len2),
                               xor_seed, xor_inc);
                    vocadd2(vctx, (prpnum)osrp2(p+4), (objnum)osrp2(p+6),
                            osrp2(p+8), p + 10, len1,
                            (len2 ? p + 10 + len1 : (uchar*)0), len2);
                    
                    p += 10 + len1 + len2;
                    bsiz -= 10 + len1 + len2;
                }
                
                /* done with the temporary block; free it up */
                osfree(bufp);
            }
            else
            {
                /* can't do it in one file read; do it the slow way */
                while (curpos != endpos)
                {
                    if (osfrb(fp, buf, 10)
                        || osfrb(fp, buf + 10,
                               (len1 = osrp2(buf)) + (len2 = osrp2(buf + 2))))
                        errsig(ec, ERR_RDGAM);
                
                    if (*flagp & FIOFCRYPT)
                        fioxor(buf + 10, (uint)(len1 + len2),
                               xor_seed, xor_inc);
                    vocadd2(vctx, (prpnum)osrp2(buf+4), (objnum)osrp2(buf+6),
                            osrp2(buf+8), buf + 10, len1,
                            (len2 ? buf + 10 + len1 : (uchar*)0), len2);
                    curpos += 10 + len1 + len2;
                }
            }
        }
        else if (fioisrsc(buf, "FMTSTR"))
        {
            uchar *fmts;
            uint   fmtl;
            
            if (osfrb(fp, buf, 2)) errsig(ec, ERR_RDGAM);
            fmtl = osrp2(buf);
            fmts = mchalo(vctx->voccxerr, fmtl, "fiord1");
            if (osfrb(fp, fmts, fmtl)) errsig(ec, ERR_RDGAM);
            if (*flagp & FIOFCRYPT) fioxor(fmts, fmtl, xor_seed, xor_inc);
            tiosetfmt(vctx->voccxtio, vctx->voccxrun, fmts, fmtl);
            
            if (fmtsp) *fmtsp = fmts;
            if (fmtlp) *fmtlp = fmtl;
        }
        else if (fioisrsc(buf, "CMPD"))
        {
            if (osfrb(fp, buf, 2)) errsig(ec, ERR_RDGAM);
            vctx->voccxcpl = osrp2(buf);
            vctx->voccxcpp = (char *)mchalo(vctx->voccxerr,
                                            vctx->voccxcpl, "fiord1");
            if (osfrb(fp, vctx->voccxcpp, (uint)vctx->voccxcpl))
                errsig(ec, ERR_RDGAM);
            if (*flagp & FIOFCRYPT)
                fioxor((uchar *)vctx->voccxcpp, (uint)vctx->voccxcpl,
                       xor_seed, xor_inc);
        }
        else if (fioisrsc(buf, "SPECWORD"))
        {
            if (osfrb(fp, buf, 2)) errsig(ec, ERR_RDGAM);
            vctx->voccxspl = osrp2(buf);
            vctx->voccxspp = (char *)mchalo(vctx->voccxerr,
                                            vctx->voccxspl, "fiord1");
            if (osfrb(fp, vctx->voccxspp, (uint)vctx->voccxspl))
                errsig(ec, ERR_RDGAM);
            if (*flagp & FIOFCRYPT)
                fioxor((uchar *)vctx->voccxspp, (uint)vctx->voccxspl,
                       xor_seed, xor_inc);
        }
        else if (fioisrsc(buf, "SYMTAB"))
        {
            tokthdef *symtab;
            
            /* if there's no debugger context, don't bother with this */
            if (!vctx->voccxrun->runcxdbg)
            {
                osfseek(fp, endpos + startofs, OSFSK_SET);
                continue;
            }
            
            if (!(symtab = vctx->voccxrun->runcxdbg->dbgcxtab))
            {
                symtab = (tokthdef *)mchalo(ec, sizeof(tokthdef),
                                            "fiord:symtab");
                tokthini(ec, mctx, (toktdef *)symtab);
                vctx->voccxrun->runcxdbg->dbgcxtab = symtab;
            }
            
            /* read symbols until we find a zero-length symbol */
            for (;;)
            {
                int hash;
                
                if (osfrb(fp, buf, 4)) errsig(ec, ERR_RDGAM);
                if (buf[0] == 0) break;
                if (osfrb(fp, buf + 4, (int)buf[0])) errsig(ec, ERR_RDGAM);
                buf[4 + buf[0]] = '\0';
                hash = tokhsh((char *)buf + 4);
                
                (*symtab->tokthsc.toktfadd)((toktdef *)symtab,
                                            (char *)buf + 4,
                                            (int)buf[0], (int)buf[1],
                                            osrp2(buf + 2), hash);
            }
        }
        else if (fioisrsc(buf, "SRC"))
        {
            /* skip source file id's if there's no debugger context */
            if (vctx->voccxrun->runcxdbg == 0)
            {
                osfseek(fp, endpos + startofs, OSFSK_SET);
                continue;
            }
            
            while ((osfpos(fp) - startofs) != endpos)
            {
                /* the only thing we know how to read is linfdef's */
                if (linfload(fp, vctx->voccxrun->runcxdbg, ec, path))
                    errsig(ec, ERR_RDGAM);
            }
        }
        else if (fioisrsc(buf, "SRC2"))
        {
            /* 
             *   this is simply a marker indicating that we have new-style
             *   (line-number-based) source debugging information in the
             *   file -- set the new-style debug info flag 
             */
            if (vctx->voccxrun->runcxdbg != 0)
                vctx->voccxrun->runcxdbg->dbgcxflg |= DBGCXFLIN2;

            /* the contents are empty - skip the block */
            osfseek(fp, endpos + startofs, OSFSK_SET);
        }
        else if (fioisrsc(buf, "PREINIT"))
        {
            if (osfrb(fp, buf, 2)) errsig(ec, ERR_RDGAM);
            *preinit = osrp2(buf);
        }
        else if (fioisrsc(buf, "ERRMSG"))
        {
            errini(ec, fp);
            osfseek(fp, endpos + startofs, OSFSK_SET);
        }
        else if (fioisrsc(buf, "EXTCNT"))
        {
            uchar  *p;
            ushort  len;
            ulong   bsiz;

            curpos = osfpos(fp) - startofs;
            bsiz = endpos - curpos;
            if (osfrb(fp, buf, 2)) errsig(ec, ERR_RDGAM);
            i = osrp2(buf);

            len = i * sizeof(runxdef);
            p = mchalo(ec, len, "fiord:runxdef");
            memset(p, 0, (size_t)len);

            vctx->voccxrun->runcxext = (runxdef *)p;
            vctx->voccxrun->runcxexc = i;

            /* see if start-of-XFCN information is present */
            if (bsiz >= 6)
            {
                /* get location of first XFCN, and seek there */
                if (osfrb(fp, buf, 4)) errsig(ec, ERR_RDGAM);
                xfcn_pos = osrp4(buf);
            }

            /* seek past this resource */
            osfseek(fp, endpos + startofs, OSFSK_SET);
        }
        else if (fioisrsc(buf, "PRPCNT"))
        {
            if (osfrb(fp, buf, 2)) errsig(ec, ERR_RDGAM);
            if (pcntptr) *pcntptr = osrp2(buf);
        }
        else if (fioisrsc(buf, "TADSPP") && tctx != 0)
        {
            tok_read_defines(tctx, fp, ec);
        }
        else if (fioisrsc(buf, "XSI"))
        {
            if (osfrb(fp, buf, 2)) errsig(ec, ERR_RDGAM);
            setupctx->fiolcxseed = xor_seed = buf[0];
            setupctx->fiolcxinc = xor_inc = buf[1];
            osfseek(fp, endpos + startofs, OSFSK_SET);
        }
        else if (fioisrsc(buf, "CHRSET"))
        {
            size_t len;
            
            /* read the character set ID and LDESC */
            if (osfrb(fp, buf, 6)
                || (len = osrp2(buf+4)) > CMAP_LDESC_MAX_LEN
                || osfrb(fp, buf+6, len))
                errsig(ec, ERR_RDGAM);

            /* establish this character set mapping */
            buf[4] = '\0';
            cmap_set_game_charset(ec, (char *)buf, (char *)buf + 6, argv0);
        }
        else if (fioisrsc(buf, "$EOF"))
        {
            if (eof_reset)
            {
                osfseek(fp, eof_reset, OSFSK_SET);     /* back after EXTCNT */
                eof_reset = 0;                   /* really done at next EOF */
#if 0 // XFCNs are obsolete
                xfcns_done = TRUE;                  /* don't do XFCNs again */
#endif
            }
            else
                break;
        }
        else
            errsig(ec, ERR_UNKRSC);
    }
}

/* read binary file */
void fiord(mcmcxdef *mctx, voccxdef *vctx, tokcxdef *tctx, const char *fname,
           const char *exename, fiolcxdef *setupctx, objnum *preinit, uint *flagp,
           tokpdef *path, uchar **fmtsp, uint *fmtlp, uint *pcntptr,
           int flags, struct appctxdef *appctx, char *argv0)
{
    osfildef *fp;
    ulong     startofs;
    const char     *display_fname;
    
    /* presume there will be no need to run preinit */
    *preinit = MCMONINV;

    /* 
     *   get the display filename - use the real filename if one is
     *   provided, otherwise use the name of the executable file itself 
     */
    display_fname = (fname != 0 ? fname : exename);

    /* save the filename in G_os_gamename */
    if (display_fname != 0)
    {
        size_t copylen;

        /* limit the copy to the buffer size */
        if ((copylen = strlen(display_fname)) > sizeof(G_os_gamename) - 1)
            copylen = sizeof(G_os_gamename) - 1;

        /* save it */
        memcpy(G_os_gamename, display_fname, copylen);
        G_os_gamename[copylen] = '\0';
    }
    else
        G_os_gamename[0] = '\0';
    
    /* open the file and read and check file header */
    fp = (fname != 0 ? osfoprb(fname, OSFTGAME)
                     : os_exeseek(exename, "TGAM"));
    if (fp == 0)
        errsig(vctx->voccxerr, ERR_OPRGAM);

    /* 
     *   we've identified the .GAM file source - tell the host system
     *   about it, if it's interested 
     */
    if (appctx != 0 && appctx->set_game_name != 0)
        (*appctx->set_game_name)(appctx->set_game_name_ctx, display_fname);

    /* remember starting location in file */
    startofs = osfpos(fp);

    ERRBEGIN(vctx->voccxerr)

    /* 
     *   Read the game file.  Note that the .GAM file always has resource
     *   file number zero. 
     */
    fiord1(mctx, vctx, tctx, fp, display_fname,
           setupctx, startofs, preinit, flagp, path,
           fmtsp, fmtlp, pcntptr, flags, appctx, argv0);

    /*
     *   If the host system accepts additional resource files, look for
     *   additional resource files.  These are files in the same directory
     *   as the .GAM file, with the .GAM suffix replaced by suffixes from
     *.  RS0 to .RS9.  
     */
    if (appctx != 0 && appctx->add_resfile != 0)
    {
        char suffix_lc[4];
        char suffix_uc[4];
        int i;
        const char *base_name;

        /* use the game or executable filename, as appropriate */
        base_name = display_fname;

        /* build the initial suffixes - try both upper- and lower-case */
        suffix_uc[0] = 'R';
        suffix_uc[1] = 'S';
        suffix_uc[3] = '\0';
        suffix_lc[0] = 'r';
        suffix_lc[1] = 's';
        suffix_lc[3] = '\0';

        /* loop through each possible suffix (.RS0 through .RS9) */
        for (i = 0 ; i < 9 ; ++i)
        {
            char resname[OSFNMAX];
            osfildef *fpres;
            int resfileno;

            /* 
             *   Build the next resource filename.  If there's an explicit
             *   resource path, use it, otherwise use the same directory
             *   that contains the .GAM file. 
             */
            if (appctx->ext_res_path != 0)
            {
                /*
                 *   There's an explicit resource path - append the root
                 *   (filename-only, minus path) portion of the .GAM file
                 *   name to the resource path. 
                 */
                os_build_full_path(resname, sizeof(resname),
                                   appctx->ext_res_path,
                                   os_get_root_name(base_name));
            }
            else
            {
                /* 
                 *   there's no resource path - use the entire .GAM
                 *   filename, including directory, so that we look in the
                 *   same directory that contains the .GAM file 
                 */
                if (base_name != 0)
                    strcpy(resname, base_name);
                else
                    resname[0] = '\0';
            }

            /* add the current extension (replacing any current extension) */
            os_remext(resname);
            suffix_lc[2] = suffix_uc[2] = '0' + i;
            os_addext(resname, suffix_lc);

            /* try opening the file */
            fpres = osfoprb(resname, OSFTGAME);

            /* if that didn't work, try the upper-case name */
            if (fpres == 0)
            {
                /* replace the suffix with the upper-case version */
                os_remext(resname);
                os_addext(resname, suffix_uc);

                /* try again with the new name */
                fpres = osfoprb(resname, OSFTGAME);
            }

            /* if we opened it successfully, read it */
            if (fpres != 0)
            {
                /* tell the host system about it */
                resfileno = (*appctx->add_resfile)
                            (appctx->add_resfile_ctx, resname);

                /* read the file */
                fiordrscext(vctx->voccxerr, fpres, appctx,
                            resfileno, resname);

                /* we're done with the file, so close it */
                osfcls(fpres);
            }
        }
    }

    ERRCLEAN(vctx->voccxerr)
        /* if an error occurs during read, clean up by closing the file */
        osfcls(fp);
    ERRENDCLN(vctx->voccxerr);
}

/* save game header */
#define FIOSAVHDR "TADS2 save\012\015\032"

/* save game header prefix - .GAM file information */
#define FIOSAVHDR_PREFIX "TADS2 save/g\012\015\032"

/* 
 *   Saved game format version string - note that the length of the
 *   version string must be fixed, so when this is updated, it must be
 *   updated to another string of the same length.  This should be updated
 *   whenever a change is made to the format that can't be otherwise
 *   detected from the data stream in the saved game file.  
 */
#define FIOSAVVSN "v2.2.1"

/* old saved game format version strings */
#define FIOSAVVSN1 "v2.2.0"

/* read fuse/daemon/alarm record */
static int fiorfda(osfildef *fp, vocddef *p, uint cnt)
{
    vocddef *q;
    uint     i;
    uchar    buf[14];
    
    /* start by clearing out entire record */
    for (i = 0, q = p ; i < cnt ; ++q, ++i)
        q->vocdfn = MCMONINV;
    
    /* now restore all the records from the file */
    for (;;)
    {
        /* read a record, and quit if it's the last one */
        if (osfrb(fp, buf, 13)) return(TRUE);
        if ((i = osrp2(buf)) == 0xffff) return(FALSE);
        
        /* restore this record */
        q = p + i;
        q->vocdfn = osrp2(buf+2);
        q->vocdarg.runstyp = buf[4];
        switch(buf[4])
        {
        case DAT_NUMBER:
            q->vocdarg.runsv.runsvnum = osrp4s(buf+5);
            break;
        case DAT_OBJECT:
        case DAT_FNADDR:
            q->vocdarg.runsv.runsvobj = osrp2(buf+5);
            break;
        case DAT_PROPNUM:
            q->vocdarg.runsv.runsvprp = osrp2(buf+5);
            break;
        }
        q->vocdprp = osrp2(buf+9);
        q->vocdtim = osrp2(buf+11);
    }
}

/*
 *   Look in a saved game file to determine if it has information on which
 *   GAM file created it.  If the GAM file information is available, this
 *   routine returns true and stores the game file name in the given
 *   buffer; if the information isn't available, we'll return false.  
 */
int fiorso_getgame(char *saved_file, char *fnamebuf, size_t buflen)
{
    osfildef *fp;
    uint      namelen;
    char      buf[sizeof(FIOSAVHDR_PREFIX) + 2];

    /* open the input file */
    if (!(fp = osfoprb(saved_file, OSFTSAVE)))
        return FALSE;

    /* read the prefix header and check */
    if (osfrb(fp, buf, (int)(sizeof(FIOSAVHDR_PREFIX) + 2))
        || memcmp(buf, FIOSAVHDR_PREFIX, sizeof(FIOSAVHDR_PREFIX)) != 0)
    {
        /* 
         *   there's no game file information - close the file and
         *   indicate that we have no information 
         */
        osfcls(fp);
        return FALSE;
    }

    /* get the length of the filename */
    namelen = osrp2(buf + sizeof(FIOSAVHDR_PREFIX));
    if (namelen > buflen - 1)
        namelen = buflen - 1;

    /* read the filename */
    if (osfrb(fp, fnamebuf, namelen))
    {
        osfcls(fp);
        return FALSE;
    }

    /* null-terminate the string */
    fnamebuf[namelen] = '\0';

    /* done with the file */
    osfcls(fp);

    /* indicate that we found the information */
    return TRUE;
}

/* restore game: returns TRUE on failure */
int fiorso(voccxdef *vctx, char *fname)
{
    osfildef   *fp;
    objnum      obj;
    uchar      *p;
    uchar      *mut;
    uint        mutsiz;
    uint        oldmutsiz;
    int         propcnt;
    mcmcxdef   *mctx = vctx->voccxmem;
    uchar       buf[sizeof(FIOSAVHDR) + sizeof(FIOSAVVSN)];
    ushort      newsiz;
    int         err = FALSE;
    char        timestamp[26];
    int         version = 0;            /* version ID - 0 = current version */
    int         result;

    /* presume success */
    result = FIORSO_SUCCESS;

    /* open the input file */
    if (!(fp = osfoprb(fname, OSFTSAVE)))
        return FIORSO_FILE_NOT_FOUND;

    /* check for a prefix header - if it's there, skip it */
    if (!osfrb(fp, buf, (int)(sizeof(FIOSAVHDR_PREFIX) + 2))
        && memcmp(buf, FIOSAVHDR_PREFIX, sizeof(FIOSAVHDR_PREFIX)) == 0)
    {
        ulong skip_len;
        
        /*
         *   The prefix header is present - skip it.  The 2-byte value
         *   following the header is the length of the prefix data block
         *   (not including the header), so simply skip the additional
         *   number of bytes specified.  
         */
        skip_len = (ulong)osrp2(buf + sizeof(FIOSAVHDR_PREFIX));
        osfseek(fp, skip_len, OSFSK_CUR);
    }
    else
    {
        /* 
         *   there's no prefix header - seek back to the start of the file
         *   and read the standard header information
         */
        osfseek(fp, 0, OSFSK_SET);
    }

    
    /* read headers and check */
    if (osfrb(fp, buf, (int)(sizeof(FIOSAVHDR) + sizeof(FIOSAVVSN)))
        || memcmp(buf, FIOSAVHDR, (size_t)sizeof(FIOSAVHDR)))
    {
        /* it's not a saved game file */
        result = FIORSO_NOT_SAVE_FILE;
        goto ret_error;
    }

    /* check the version string */
    if (memcmp(buf + sizeof(FIOSAVHDR), FIOSAVVSN,
               (size_t)sizeof(FIOSAVVSN)) == 0)
    {
        /* it's the current version */
        version = 0;
    }
    else if (memcmp(buf + sizeof(FIOSAVHDR), FIOSAVVSN1,
                    (size_t)sizeof(FIOSAVVSN1)) == 0)
    {
        /* it's old version #1 */
        version = 1;
    }
    else
    {
        /* 
         *   this isn't a recognized version - the file must have been
         *   saved by a newer version of the system, so we can't assume we
         *   will be able to parse the format 
         */
        result = FIORSO_BAD_FMT_VSN;
        goto ret_error;
    }
    
    /* 
     *   Read timestamp and check - the game must have been saved by the
     *   same .GAM file that we are now running, because the .SAV file is
     *   written entirely in terms of the contents of the .GAM file; any
     *   change in the .GAM file invalidates the .SAV file. 
     */
    if (osfrb(fp, timestamp, 26)
        || memcmp(timestamp, vctx->voccxtim, (size_t)26))
    {
        result = FIORSO_BAD_GAME_VSN;
        goto ret_error;
    }
    
    /* first revert every object to original (post-compilation) state */
    vocrevert(vctx);

    /* 
     *   the most common error from here on is simply a file read error,
     *   so presume that this is what will happen; if we are successful or
     *   encounter a different error, we'll change the status at that
     *   point 
     */
    result = FIORSO_READ_ERROR;

    /* go through file and load changed objects */
    for (;;)
    {
        /* get the header */
        if (osfrb(fp, buf, 7))
            goto ret_error;

        /* get the object number from the header, and stop if we're done */
        obj = osrp2(buf+1);
        if (obj == MCMONINV)
            break;

        /* if the object was dynamically allocated, recreate it */
        if (buf[0] == 1)
        {
            int     sccnt;
            objnum  sc;
            
            /* create the object */
            mutsiz = osrp2(buf + 3);
            p = mcmalonum(mctx, (ushort)mutsiz, (mcmon)obj);

            /* read the object's contents */
            if (osfrb(fp, p, mutsiz))
                goto ret_error;

            /* get the superclass data (at most one superclass) */
            sccnt = objnsc(p);
            if (sccnt) sc = osrp2(objsc(p));

            /* create inheritance records for the object */
            vociadd(vctx, obj, MCMONINV, sccnt, &sc, VOCIFNEW | VOCIFVOC);

#if 0
            {
                int     wrdcnt;

                /* read the object's vocabulary and add it back */
                if (osfrb(fp, buf, 2))
                    goto ret_error;
                wrdcnt = osrp2(buf);
                while (wrdcnt--)
                {
                    int   len1;
                    int   len2;
                    char  wrd[80];
                    
                    /* read the header */
                    if (osfrb(fp, buf, 6))
                        goto ret_error;
                    len1 = osrp2(buf+2);
                    len2 = osrp2(buf+4);
                    
                    /* read the word text */
                    if (osfrb(fp, wrd, len1 + len2))
                        goto ret_error;
                    
                    /* add the word */
                    vocadd2(vctx, buf[0], obj, buf[1], wrd, len1,
                            wrd + len1, len2);
                }
            }
#endif

        }
        else
        {
            /* get the remaining data from the header */
            propcnt = osrp2(buf + 3);
            mutsiz = osrp2(buf + 5);
        
            /* expand object if it's not big enough for mutsiz */
            p = mcmlck(mctx, (mcmon)obj);
            oldmutsiz = mcmobjsiz(mctx, (mcmon)obj) - objrst(p);
            if (oldmutsiz < mutsiz)
            {
                newsiz = mutsiz - oldmutsiz;
                p = (uchar *)objexp(mctx, obj, &newsiz);
            }
            
            /* reset statistics, and read mutable part from file */
            mut = p + objrst(p);
            objsnp(p, propcnt);
            objsfree(p, mutsiz + objrst(p));
            if (osfrb(fp, mut, mutsiz))
                err = TRUE;
        
            /* reset ignore flags as needed */
            objsetign(mctx, obj);
        }

        /* touch and unlock the object */
        mcmtch(mctx, (mcmon)obj);
        mcmunlck(mctx, (mcmon)obj);
        if (err)
            goto ret_error;
    }
    
    /* read fuses/daemons/alarms */
    if (fiorfda(fp, vctx->voccxdmn, vctx->voccxdmc)
        || fiorfda(fp, vctx->voccxfus, vctx->voccxfuc)
        || fiorfda(fp, vctx->voccxalm, vctx->voccxalc))
        goto ret_error;

    /* read the dynamically added and deleted vocabulary */
    for (;;)
    {
        int     len1;
        int     len2;
        char    wrd[80];
        int     flags;
        int     typ;
        
        /* read the header */
        if (osfrb(fp, buf, 8))
            goto ret_error;

        typ = buf[0];
        flags = buf[1];
        len1 = osrp2(buf+2);
        len2 = osrp2(buf+4);
        obj = osrp2(buf+6);

        /* check to see if this is the end marker */
        if (obj == MCMONINV) break;
        
        /* read the word text */
        if (osfrb(fp, wrd+2, len1))
            goto ret_error;
        if (len2)
        {
            wrd[len1 + 2] = ' ';
            if (osfrb(fp, &wrd[len1 + 3], len2))
                goto ret_error;
            oswp2(wrd, len1 + len2 + 3);
        }
        else
            oswp2(wrd, len1 + 2);
        
        /* add or delete the word as appropriate */
        if (flags & VOCFDEL)
            vocdel1(vctx, obj, (char *)wrd, (prpnum)typ, FALSE, FALSE, FALSE);
        else
            vocadd2(vctx, buf[0], obj, buf[1], (uchar *)wrd+2, len1,
                    (uchar *)wrd+len1, len2);
    }

    /* 
     *   the following was added in save format version "v2.2.1", so skip
     *   it if the save version is older than that 
     */
    if (version != 1)
    {
        /* read the current "Me" object */
        if (osfrb(fp, buf, 2))
            goto ret_error;
        vctx->voccxme = osrp2(buf);
    }

    /* done - close file and return success indication */
    osfcls(fp);
    return FIORSO_SUCCESS;

    /* come here on failure - close file and return error indication */
ret_error:
    osfcls(fp);
    return result;
}

/* write fuse/daemon/alarm block */
static int fiowfda(osfildef *fp, vocddef *p, uint cnt)
{
    uchar buf[14];
    uint  i;
    
    for (i = 0 ; i < cnt ; ++i, ++p)
    {
        if (p->vocdfn == MCMONINV) continue;            /* not set - ignore */
        
        oswp2(buf, i);                        /* element in array to be set */
        oswp2(buf+2, p->vocdfn);       /* object number for function/target */
        buf[4] = p->vocdarg.runstyp;                    /* type of argument */
        switch(buf[4])
        {
        case DAT_NUMBER:
            oswp4s(buf+5, p->vocdarg.runsv.runsvnum);
            break;
        case DAT_OBJECT:
        case DAT_FNADDR:
            oswp2(buf+5, p->vocdarg.runsv.runsvobj);
            break;
        case DAT_PROPNUM:
            oswp2(buf+5, p->vocdarg.runsv.runsvprp);
            break;
        }
        oswp2(buf+9, p->vocdprp);
        oswp2(buf+11, p->vocdtim);
        
        /* write this record to file */
        if (osfwb(fp, buf, 13)) return(TRUE);
    }
    
    /* write end record - -1 for array element number */
    oswp2(buf, 0xffff);
    return(osfwb(fp, buf, 13));
}

/* context for vocabulary saver callback function */
struct fiosav_cb_ctx
{
    int       err;
    osfildef *fp;
};

#ifdef NEVER
/*
 *   callback for vocabulary saver - called by voc_iterate for each word
 *   defined for a particular object, allowing us to write all the words
 *   attached to a dynamically allocated object to the save file 
 */
static void fiosav_cb(struct fiosav_cb_ctx *ctx,
                      vocdef *voc, vocwdef *vocw)
{
    char buf[10];
    
    /* write the part of speech, flags, and word lengths */
    buf[0] = vocw->vocwtyp;
    buf[1] = vocw->vocwflg;
    oswp2(buf+2, voc->voclen);
    oswp2(buf+4, voc->vocln2);
    if (osfwb(ctx->fp, buf, 6)) ctx->err = TRUE;

    /* write the words */
    if (osfwb(ctx->fp, voc->voctxt, voc->voclen + voc->vocln2))
        ctx->err = TRUE;
}
#endif

/*
 *   Callback for vocabulary saver - called by voc_iterate for every
 *   word.  We'll write the word if it was dynamically added or deleted,
 *   so that we can restore that status when the game is restored.  
 */
static void fiosav_voc_cb(void *ctx0, vocdef *voc, vocwdef *vocw)
{
    struct fiosav_cb_ctx *ctx = (struct fiosav_cb_ctx *)ctx0;
    char buf[10];
    
    /* if the word was dynamically allocated or deleted, save it */
    if ((vocw->vocwflg & VOCFNEW) || (vocw->vocwflg & VOCFDEL))
    {
        /* write the header information */
        buf[0] = vocw->vocwtyp;
        buf[1] = vocw->vocwflg;
        oswp2(buf+2, voc->voclen);
        oswp2(buf+4, voc->vocln2);
        oswp2(buf+6, vocw->vocwobj);
        if (osfwb(ctx->fp, buf, 8)) ctx->err = TRUE;

        /* write the words */
        if (osfwb(ctx->fp, voc->voctxt, voc->voclen + voc->vocln2))
            ctx->err = TRUE;
    }
}


/* save game; returns TRUE on failure */
int fiosav(voccxdef *vctx, char *fname, char *game_fname)
{
    osfildef   *fp;
    vocidef  ***vpg;
    vocidef   **v;
    int         i;
    int         j;
    objnum      obj;
    uchar      *p;
    uchar      *mut;
    uint        mutsiz;
    int         propcnt;
    mcmcxdef   *mctx = vctx->voccxmem;
    uchar       buf[8];
    int         err = FALSE;
    struct fiosav_cb_ctx  fnctx;

    /* open the output file */
    if ((fp = osfopwb(fname, OSFTSAVE)) == 0)
        return TRUE;

    /*
     *   If we have game file information, save the game file information
     *   with the saved game file.  This lets the player start the
     *   run-time and restore the game by specifying only the saved game
     *   file.  
     */
    if (game_fname != 0)
    {
        size_t len;
        
        /* write the prefix header */
        len = strlen(game_fname);
        oswp2(buf, len);
        if (osfwb(fp, FIOSAVHDR_PREFIX, (int)sizeof(FIOSAVHDR_PREFIX))
            || osfwb(fp, buf, 2)
            || osfwb(fp, game_fname, (int)len))
            goto ret_error;
    }
    
    /* write save game header and timestamp */
    if (osfwb(fp, FIOSAVHDR, (int)sizeof(FIOSAVHDR))
        || osfwb(fp, FIOSAVVSN, (int)sizeof(FIOSAVVSN))
        || osfwb(fp, vctx->voccxtim, 26))
        goto ret_error;

    /* go through each object, and write if it's been changed */
    for (vpg = vctx->voccxinh, i = 0 ; i < VOCINHMAX ; ++vpg, ++i)
    {
        if (!*vpg) continue;
        for (v = *vpg, obj = (i << 8), j = 0 ; j < 256 ; ++v, ++obj, ++j)
        {
            if (*v != 0)
            {
                /* write object if it's dirty */
                if (mcmobjdirty(mctx, (mcmon)obj))
                {
                    p = mcmlck(mctx, (mcmon)obj);
                    mut = p + objrst(p);
                    propcnt = objnprop(p);
                    mutsiz = objfree(p) - objrst(p);
                    if ((objflg(p) & OBJFINDEX) != 0)
                        mutsiz += propcnt * 4;

                    /*
                     *   If the object was dynamically allocated, write
                     *   the whole object.  Otherwise, write just the
                     *   mutable part. 
                     */
                    if ((*v)->vociflg & VOCIFNEW)
                    {
                        /* indicate that the object is dynamic */
                        buf[0] = 1;
                        oswp2(buf + 1, obj);

                        /* write the entire object */
                        mutsiz = objfree(p);
                        oswp2(buf + 3, mutsiz);
                        if (osfwb(fp, buf, 7)
                            || osfwb(fp, p, mutsiz))
                            err = TRUE;

#ifdef NEVER
                        {
                            int         wrdcnt;
                            
                            /* count the words, and write the count */
                            voc_count(vctx, obj, 0, &wrdcnt, (int *)0);
                            oswp2(buf, wrdcnt);
                            if (osfwb(fp, buf, 2))
                                err = TRUE;
                            
                            /* write the words */
                            fnctx.err = 0;
                            fnctx.fp = fp;
                            voc_iterate(vctx, obj, fiosav_cb, &fnctx);
                            if (fnctx.err != 0)
                                err = TRUE;
                        }
#endif
                    }
                    else if (mutsiz)
                    {
                        /* write number of properties, size of mut, and mut */
                        buf[0] = 0;   /* indicate that the object is static */
                        oswp2(buf + 1, obj);
                        oswp2(buf + 3, propcnt);
                        oswp2(buf + 5, mutsiz);
                        if (osfwb(fp, buf, 7)
                            || osfwb(fp, mut, mutsiz))
                            err = TRUE;
                    }
                    
                    mcmunlck(mctx, (mcmon)obj);
                    if (err != 0)
                        goto ret_error;
                }
            }
        }
    }

    /* write end-of-objects indication */
    buf[0] = 0;
    oswp2(buf + 1, MCMONINV);
    oswp4(buf + 3, 0);
    if (osfwb(fp, buf, 7))
        goto ret_error;

    /* write fuses/daemons/alarms */
    if (fiowfda(fp, vctx->voccxdmn, vctx->voccxdmc)
        || fiowfda(fp, vctx->voccxfus, vctx->voccxfuc)
        || fiowfda(fp, vctx->voccxalm, vctx->voccxalc))
        goto ret_error;

    /* write run-time vocabulary additions and deletions */
    fnctx.fp = fp;
    fnctx.err = 0;
    voc_iterate(vctx, MCMONINV, fiosav_voc_cb, &fnctx);
    if (fnctx.err)
        goto ret_error;

    /* write end marker for vocabulary additions and deletions */
    oswp2(buf+6, MCMONINV);
    if (osfwb(fp, buf, 8))
        goto ret_error;

    /* write the current "Me" object */
    oswp2(buf, vctx->voccxme);
    if (osfwb(fp, buf, 2))
        goto ret_error;

    /* done - close file and return success indication */
    osfcls(fp);
    os_settype(fname, OSFTSAVE);
    return FALSE;

    /* come here on failure - close file and return error indication */
ret_error:
    osfcls(fp);
    return TRUE;
}

void fioxor(uchar *p, uint siz, uint seed, uint inc)
{
	for (; siz; seed += inc, --siz)
		*p++ ^= (uchar)seed;
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
