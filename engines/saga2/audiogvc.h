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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_AUDIOGVC_H
#define SAGA2_AUDIOGVC_H

namespace Saga2 {

#define BUGFIX_RELEASE "22"

#define HAVE_STDARG_H 1


# define OLD_MAGIC          "ajkg"
# define MAGIC          "dgvc"

// file formats

enum decompFTypes {
	dft_au,                  // Unix .AU file
	dft_ulaw,                // mu-law encoding
	dft_s8,                  //   signed 8  bit PCM
	dft_u8,                  // unsigned 8  bit PCM
	dft_s16,                 //   signed 16 bit PCM
	dft_u16,                 // unsigned 16 bit PCM
	dft_s16x,                //   signed 16 bit PCM
	dft_u16x,                // unsigned 16 bit PCM
	dft_s16hl,               //   signed 16 bit PCM 680x0 ordering
	dft_u16hl,               // unsigned 16 bit PCM 680x0 ordering
	dft_s16lh,               //   signed 16 bit PCM 80x86 ordering
	dft_u16lh,               // unsigned 16 bit PCM 80x86 ordering
};

#define DEFAULT_FTYPE ( dft_s16 )




# define FORMAT_VERSION     1
# define MIN_SUPPORTED_VERSION  0
# define MAX_SUPPORTED_VERSION  2
# define MAX_VERSION        7
# define UNDEFINED_UINT     -1
# define DEFAULT_BLOCK_SIZE 256
# define DEFAULT_V0NMEAN    0
# define DEFAULT_V2NMEAN    4
# define DEFAULT_MAXNLPC    0
# define DEFAULT_NCHAN      1
# define DEFAULT_NSKIP      0
# define DEFAULT_NDISCARD   0
# define NBITPERLONG        32
# define DEFAULT_MINSNR         99
# define DEFAULT_MAXRESNSTR "32.0"
# define DEFAULT_QUANTERROR 0
# define MINBITRATE     2.5

# define MAX_LPC_ORDER  64
# define CHANSIZE   0
# define ENERGYSIZE 3
# define BITSHIFTSIZE   2
# define NWRAP      3

# define FNSIZE     2
# define FN_DIFF0   0
# define FN_DIFF1   1
# define FN_DIFF2   2
# define FN_DIFF3   3
# define FN_QUIT    4
# define FN_BLOCKSIZE   5
# define FN_BITSHIFT    6
# define FN_QLPC    7
# define FN_ZERO    8

# define ULONGSIZE  2
# define NSKIPSIZE  1
# define LPCQSIZE   2
# define LPCQUANT   5
# define XBYTESIZE  7

# define TYPESIZE   4
# define TYPE_AU    0
# define TYPE_S8    1
# define TYPE_U8    2
# define TYPE_S16HL 3
# define TYPE_U16HL 4
# define TYPE_S16LH 5
# define TYPE_U16LH 6
# define TYPE_ULAW  7
# define TYPE_EOF   8

#ifndef MIN
# define MIN( a, b ) ((( a )<( b ))?( a ):( b ))
#endif

#ifndef MAX
# define MAX( a, b ) ((( a )>( b ))?( a ):( b ))
#endif

#if defined( unix ) && !defined( linux )
# define labs abs
#endif

# define ROUNDEDSHIFTDOWN( x, n ) ((( n ) == 0 ) ? ( x ) : (( x ) >> (( n ) - 1 )) >> 1 )

#ifndef M_LN2
#define M_LN2   0.69314718055994530942
#endif

/* BUFSIZ must be a multiple of four to contain a whole number of words */
#ifndef BUFSIZ
# define BUFSIZ 1024
#endif

#define putc_exit( val, stream )\
	{ char rval;\
		if(( rval = buffputc(( val ), ( stream ))) != ( char ) ( val ))\
			SegFatal( serrCompPutFailed );\
	}
//update_exit( 1, "write failed: putc returns EOF\n");

extern int getc_exit_val;
#define getc_exit( stream )\
	((( getc_exit_val = buffgetc( stream )) == EOF ) ? \
	 update_exit( 1, "read failed: getc returns EOF\n"), 0: getc_exit_val )

#undef  uchar
#define uchar   unsigned char
#undef  ushort
#define ushort  unsigned short
#undef  ulong
#define ulong   unsigned long

#if defined( __STDC__ ) || defined( __GNUC__ ) || defined( sgi ) || !defined( unix )
typedef signed char schar;
#define PROTO( ARGS )   ARGS
#else
typedef char        schar;
#define PROTO( ARGS )   ()
#endif

#ifdef NEED_OLD_PROTOTYPES
/*******************************************/
/* this should be in string.h or strings.h */
extern int  strcmp      PROTO((const char *, const char *));
extern char    *strcpy      PROTO((char *, const char *));
extern char    *strcat      PROTO((char *, const char *));
extern int  strlen      PROTO((const char *));

/**************************************/
/* defined in stdlib.h if you have it */
extern void    *malloc      PROTO((unsigned long));
extern void free        PROTO((void *));
extern int  atoi        PROTO((const char *));
extern void swab        PROTO((char *, char *, int));
extern int  buffseek        PROTO((doubleBuffer *, long, int));

/***************************/
/* other misc system calls */
extern int  unlink      PROTO((const char *));
extern void exit        PROTO((int));
#endif

/************************/
/* defined in shorten.c */
extern long     init_offset     PROTO((long **, int, int, int));

/*********************/
/* defined in ulaw.c */
extern uchar    linear2ulaw     PROTO((long));
extern int      ulaw2linear     PROTO((uchar));

/********************/
/* defined in lpc.c */
extern int  wav2lpc     PROTO((long *, int, long, int *, int, int, float *, float *));

/*********************/
/* defined in poly.c */
extern int  wav2poly    PROTO((long *, int, long, int, float *, float *));

/*********************/
/* defined in exit.c */

extern void basic_exit  PROTO((int));

#ifdef HAVE_STDARG_H
extern void perror_exit PROTO((char *, ...));
extern void usage_exit  PROTO((int, char *, ...));
extern void update_exit PROTO((int, char *, ...));
# else
extern void perror_exit PROTO(());
extern void usage_exit  PROTO(());
extern void update_exit PROTO(());
# endif

/***********************/
/* defined in hsgetopt.c */
extern void hs_resetopt PROTO((void));
extern int  hs_getopt   PROTO((int, char **, char *));
extern int  hs_optind;
extern char    *hs_optarg;

/**********************/
/* defined in array.c */
extern void *pmalloc    PROTO((ulong));
extern long **long2d    PROTO((ulong, ulong));

/****************************/
/* defined in dupfileinfo.c */
extern int  dupfileinfo PROTO((char *, char *));


// PROTO() is an annoying parameter definition macro to allow compatibility
//         with the K&R style parameter declarations

/**********************/
/* defined in fixio.c */
extern void init_sizeof_sample PROTO((void));
extern void fread_type_init PROTO((void));
extern void fread_type_quit PROTO((void));
extern void fwrite_type_init PROTO((void));
extern int  fread_type  PROTO((long **, int, int, int, Buffer *));
extern void fwrite_type PROTO((long **, int, int, int, Buffer *));
extern void fwrite_type_quit PROTO((void));
extern int  find_bitshift   PROTO((long *, int, int));
extern void fix_bitshift    PROTO((long *, int, int, int));

/**********************/
/* defined in vario.c */
extern void var_put_init    PROTO((void));
extern void uvar_put    PROTO((ulong, int, Buffer *));
extern void var_put     PROTO((long, int, Buffer *));
extern void ulong_put   PROTO((ulong, Buffer *));
extern void var_put_quit    PROTO((Buffer *));

extern void var_get_init    PROTO((void));
extern long uvar_get    PROTO((int, Buffer *));
extern long var_get     PROTO((int, Buffer *));
extern ulong    ulong_get   PROTO((Buffer *));
extern void var_get_quit    PROTO((void));

extern int  sizeof_uvar PROTO((ulong, int));
extern int  sizeof_var  PROTO((long, int));

extern void     mkmasktab       PROTO((void));
extern void     word_put        PROTO((ulong, Buffer *));
extern ulong    word_get        PROTO((Buffer *));


extern int  shorten     PROTO((Buffer *, Buffer *, int, char **));


long init_offset(long **offset, int nchan, int nblock, int ftype) ;
float Satof(char *string) ;
float *parseList(char *maxresnstr, int nchan) ;
int16 getFType(decompFTypes dft);
int unShorten(Buffer *stdi, Buffer *stdo, int, char **);

# define V2LPCQOFFSET ( 1 << LPCQUANT );

# define UINT_PUT( val, nbit, file ) \
	if ( version == 0 ) uvar_put(( unsigned long ) val, nbit, file ); \
	else ulong_put(( unsigned long ) val, file )

# define UINT_GET( nbit, file ) \
	(( version == 0 ) ? uvar_get( nbit, file ) : ulong_get( file ))

# define VAR_PUT( val, nbit, file ) \
	if ( version == 0 ) var_put(( unsigned long ) val, nbit - 1, file ); \
	else var_put(( unsigned long ) val, nbit, file )

} // end of namespace Saga2

#endif
