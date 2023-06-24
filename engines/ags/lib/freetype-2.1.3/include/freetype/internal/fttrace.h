/***************************************************************************/
/*                                                                         */
/*  fttrace.h                                                              */
/*                                                                         */
/*    Tracing handling (specification only).                               */
/*                                                                         */
/*  Copyright 2002 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


/* definitions of trace levels for FreeType 2 */

/* the first level must always be `trace_any' */
FT2_1_3_TRACE_DEF( any )

/* base components */
FT2_1_3_TRACE_DEF( calc )      /* calculations            (ftcalc.c)   */
FT2_1_3_TRACE_DEF( memory )    /* memory manager          (ftobjs.c)   */
FT2_1_3_TRACE_DEF( stream )    /* stream manager          (ftstream.c) */
FT2_1_3_TRACE_DEF( io )        /* i/o interface           (ftsystem.c) */
FT2_1_3_TRACE_DEF( list )      /* list management         (ftlist.c)   */
FT2_1_3_TRACE_DEF( init )      /* initialization          (ftinit.c)   */
FT2_1_3_TRACE_DEF( objs )      /* base objects            (ftobjs.c)   */
FT2_1_3_TRACE_DEF( outline )   /* outline management      (ftoutln.c)  */
FT2_1_3_TRACE_DEF( glyph )     /* glyph management        (ftglyph.c)  */

FT2_1_3_TRACE_DEF( raster )    /* monochrome rasterizer   (ftraster.c) */
FT2_1_3_TRACE_DEF( smooth )    /* anti-aliasing raster    (ftgrays.c)  */
FT2_1_3_TRACE_DEF( mm )        /* MM interface            (ftmm.c)     */

/* Cache sub-system */
FT2_1_3_TRACE_DEF( cache )     /* cache sub-system        (ftcache.c, etc..) */

/* SFNT driver components */
FT2_1_3_TRACE_DEF( sfobjs )    /*  SFNT object handler     (sfobjs.c)   */
FT2_1_3_TRACE_DEF( ttcmap )    /* charmap handler         (ttcmap.c)    */
FT2_1_3_TRACE_DEF( ttload )    /* basic TrueType tables   (ttload.c)    */
FT2_1_3_TRACE_DEF( ttpost )    /* PS table processing     (ttpost.c)   */
FT2_1_3_TRACE_DEF( ttsbit )    /* TrueType sbit handling  (ttsbit.c)   */

/* TrueType driver components */
FT2_1_3_TRACE_DEF( ttdriver )  /* TT font driver          (ttdriver.c) */
FT2_1_3_TRACE_DEF( ttgload )   /* TT glyph loader         (ttgload.c)  */
FT2_1_3_TRACE_DEF( ttinterp )  /* bytecode interpreter    (ttinterp.c) */
FT2_1_3_TRACE_DEF( ttobjs )    /* TT objects manager      (ttobjs.c)   */
FT2_1_3_TRACE_DEF( ttpload )   /* TT data/program loader  (ttpload.c)  */

/* Type 1 driver components */
FT2_1_3_TRACE_DEF( t1driver )
FT2_1_3_TRACE_DEF( t1gload )
FT2_1_3_TRACE_DEF( t1hint )
FT2_1_3_TRACE_DEF( t1load )
FT2_1_3_TRACE_DEF( t1objs )
FT2_1_3_TRACE_DEF( t1parse )

/* PostScript helper module `psaux' */
FT2_1_3_TRACE_DEF( t1decode )
FT2_1_3_TRACE_DEF( psobjs )

/* PostScript hinting module `pshinter' */
FT2_1_3_TRACE_DEF( pshrec )
FT2_1_3_TRACE_DEF( pshalgo1 )
FT2_1_3_TRACE_DEF( pshalgo2 )

/* Type 2 driver components */
FT2_1_3_TRACE_DEF( cffdriver )
FT2_1_3_TRACE_DEF( cffgload )
FT2_1_3_TRACE_DEF( cffload )
FT2_1_3_TRACE_DEF( cffobjs )
FT2_1_3_TRACE_DEF( cffparse )

/* Type 42 driver component */
FT2_1_3_TRACE_DEF( t42 )

/* CID driver components */
FT2_1_3_TRACE_DEF( cidafm )
FT2_1_3_TRACE_DEF( ciddriver )
FT2_1_3_TRACE_DEF( cidgload )
FT2_1_3_TRACE_DEF( cidload )
FT2_1_3_TRACE_DEF( cidobjs )
FT2_1_3_TRACE_DEF( cidparse )

/* Windows fonts component */
FT2_1_3_TRACE_DEF( winfnt )

/* PCF fonts components */
FT2_1_3_TRACE_DEF( pcfdriver )
FT2_1_3_TRACE_DEF( pcfread )

/* BDF fonts component */
FT2_1_3_TRACE_DEF( bdfdriver )
FT2_1_3_TRACE_DEF( bdflib )

/* PFR fonts component */
FT2_1_3_TRACE_DEF( pfr )


/* END */
