/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef _DEBUG

#include "bagel/hodjnpodj/metagame/bgen/bgen.h"
#include "bagel/hodjnpodj/metagame/bgen/bdbg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define TRACE_ERRORS 0

#undef OUTFILE
#define REOPENCOUNT 100
/*
#define OUTFILE "jxdebug.txt"
#define REOPENCOUNT 10
#define OUTFILE "aux"
#define REOPENCOUNT MAXPOSINT
*/

#define MAXRECURSION 100

#define CWCBRK SCANAF(10)   /* break is ALT F10 (5 times) */

#define CWCTRON SCANAF(1)   /* trace on -- ALT-F1 */
#define CWCTROFF SCANAF(2)  /* trace off -- ALT-F2 */
#define CWCNL CCHCR     /* new line (enter) */

#ifdef JX_DEBUG

static const char    dbgxx1[7] = "DBGXX1" ;

static const char    dbgxx2[5] = "TRC." ;
int dbgtrc = 0 ;        /* %trace -- trace calls */

static const char    dbgxx3[5] = "TRK." ;
int dbgtrk = TRACE_ERRORS ; /* %track -- track calls */

static const char    dbgxx4[5] = "MSG." ;
int dbgmsg = 0 ;        /* %msg */

static const char    dbgxx5[5] = "MTK." ;
int dbgmtk = 0 ;        /* %memtrk -- track memory */

static const char    dbgxx6[5] = "DP1." ;
int dbgdp1 = 0 ;        /* %dump1 */

static const char    dbgxx7[5] = "DP2." ;
int dbgdp2 = 0 ;        /* %dump2 */

static const char    dbgxx8[5] = "FOC." ;
int dbgfoc = 0 ;        /* %filoc -- file open/close */

static const char    dbgxx9[5] = "FRW." ;
int dbgfrw = 0 ;        /* %filrw -- file read/write */

static const char    dbgxxa[5] = "MTC." ;
int dbgmtc = 0 ;        /* %memtrc -- trace memory */

static const char    dbgxxb[5] = "MFC." ;
int dbgmfc = 0 ;        /* %mfc -- trace msg interface */

static const char    dbgxxc[5] = "SWI." ;
int dbgswi = 0 ;        /* %swi -- trace msg interface */

static const char    dbgxxd[5] = "CKN." ; /* %cknull -- check null ptr */
int dbgckn = 0 ;

static const char    dbgxxe[5] = "CKF." ;
int dbgckf = 0 ;        /* %cknull:arg -- check # words
                        following first 4 */

static const char    dbgxxf[5] = "TRR." ;
int dbgtrr = TRACE_ERRORS ; /* %trcerr -- trace errors */

static const char    dbgxxg[5] = "FIL." ;
int dbgreopen = MAXPOSINT ; /* reopen size */
char    dbgfile[DBGFILESIZE] = "" ; /* output file */

static const char    dbgxxz[7] = "DBGXXX" ;


#define NAMEL 40    /* max name length, including terminating null */
#define NNAMES 40   /* max name depth */

/* data base */
char dbgxxx[] = "DBGLST" ;
int dbgdep = 0 ;    /* depth of names */
int dbganc = 0 ;    /* anchor point for screen display */
char dbglst[NNAMES][NAMEL] ;    /* array of names */

char dbgabt = 0 ;   /* abort flag -- set to 1 to abort */


LPVOID dbgptr = NULL ;
int dbgptrlen = 4 ;
// long dbgvalue = 0 ;
char dbgvalue[5] = "h.bm" ;

//* dbgntr -- enter a subroutine
void dbgntr(const char * name)
// char *name ;     /* name of subroutine */
{
	char *p ;       /* string pointer */
	int i ;     /* character counter */

	if (dbgabt)     /* test abort flag */
		error("Abort"); //_exit(1) ;      /* exit if on */
	if (dbgtrc || dbgtrk) { /* if tracing or tracking enabled */
		if (dbgptr) {
			if (*(long *)dbgptr != *(long *)&dbgvalue[0])
				DebugBreak() ;
//	    if (*(long *)dbgptr != dbgvalue)
//		DebugBreak() ;
		}
		if (dbgckn)     /* if null pointer check wanted */
			dbgcnp("dbgntr", name) ;    /* do null pointer check */

		if (++dbgdep < NNAMES) { /* if no array overflow */
			p = &dbglst[dbgdep - 1][0]; /* point to correct entry */
			i = 0 ;     /* no characters yet */
			while (++i < NAMEL && (*(p++) = *(name++)) != 0)
				;       /* copy to null char */
			while (++i <= NAMEL)
				*(p++) = '\0' ; /* rest of string is nulls */
		}
		if (dbgdep > MAXRECURSION)
			/* if too much recursion */
		{
//	    ifcprf(">"); /* indicate enter */
//	    dbganc = -1 ;    /* force dump of entire list */
			dbgtyp("\nToo much recursion.\n>", TRUE) ;
			/* type list of names */
			if (dbgdep > MAXRECURSION + 10)
				error("Abort"); //_exit(1) ;      /* exit if on */
//		DebugBreak() ;
		} else if (dbgtrc) { /* if tracing enabled */
//	    ifcprf(">"); /* indicate enter */
			dbgtyp(">", FALSE) ;        /* type list of names */
		}
		if (kybtst() == CWCTRON) /* if trace on toggle hit */
			dbgttg() ;      /* go handle */
	}
}

//* dbgxit -- exit a subroutine
void dbgxit(const char * name, int iError)
// char *name ;     /* name of subroutine */
{
	int i ;     /* loop index */
	char *p, *q ;   /* string pointers */
	int depth ;     /* depth level */

	if (dbgabt)     /* test abort flag */
		error("Abort") ; //_exit(1) ;      /* exit if on */
	if (iError && dbgtrr) { /* if tracing errors */
		ifcprf("\nError %d in function %s.\n", iError, name) ;
		dbganc = -1 ;   // force dump of entire call list
	}
	if (dbgtrc || dbgtrk) { /* if tracing or tracking enabled */
		if (dbgptr) {
			if (*(long *)dbgptr != *(long *)&dbgvalue[0])
				DebugBreak() ;
//	    if (*(long *)dbgptr != dbgvalue)
//		DebugBreak() ;
		}
		if (dbgckn)     /* if null pointer check wanted */
			dbgcnp("dbgxit", name) ;    /* do null pointer check */

		if (dbgdep > 0 && dbgdep < NNAMES) { /* if we haven't overflowed */
			depth = dbgdep ;        /* copy current depth */
			i = 0 ;         /* loop initialization */
			while (i < NAMEL && --depth > 0) { /* look for name */
				p = &dbglst[depth][0] ; /* point to correct entry */
				q = name ;      /* input name */
				i = 0 ;     /* no characters yet */
				for (; ++i < NAMEL && *p == *q ; /* search for name */
				        p++, q++)   /* increment pointers */
					if (*p == '\0')
						/* if we found a terminating null char */
						i = NAMEL ; /* force both loop
                termination -- the strings compare equal */
			}
			if (depth == 0 && dbgdep > 1)
				ifcprf("\ndbgxit -- exit name %s not found.\n", name) ;
			else        /* name found */
				dbgdep = depth ;    /* set global depth value */
		}
		if (dbgtrc || (iError && dbgtrr)) { /* if tracing enabled */
//	    ifcprf("<"); /* indicate exit */
			dbgtyp("<", FALSE) ;        /* type list of names */
		}

		if (kybtst() == CWCTRON) /* if trace on toggle hit */
			dbgttg() ;      /* go handle */
	}
}

//* dbgtyp -- type status line
void dbgtyp(const char * ps, BOOL bReset) {
	int k ;         /* name index */
	int len ;           /* line length */

	if (bReset || dbganc < 0)   /* get typeout anchor point, and test
                special value to dump entire list */
		dbganc = 0 ;        /* reset to beginning of list */
	else {      /* standard case */
		if (dbgdep < NNAMES && dbgdep - dbganc > 8)
			/* if too many on line */
			dbganc = dbgdep - 4 ;   /* reset anchor */
		if (dbgdep - dbganc < 2)        /* if too few on line */
			dbganc = (dbgdep < 6) ? 0 : dbgdep - 6; /* reset anchor */
	}

	if (ps)
		ifcprf(ps) ;

	ifcprf("%d ", dbgdep) ;
	if (dbganc > 0)     /* if anchor is not at beginning */
		ifcprf("...");
	len = 0 ;
	for (k = dbganc; k < dbgdep && k < NNAMES; k++)
		/* loop thru names in list */
	{
		if ((len += strlen(&dbglst[k][0])) > 80)
			ifcprf("\n++ "), len = 0 ;
		ifcprf("%s ", dbglst + k);  /* print entry */
	}

	if (dbgdep >= NNAMES)   /* if too many names */
		ifcprf("...");
	ifcprf("\n");
//    DoPendingEvents() ;   /// ********
}

//* dbgntu -- enter a subroutine -- unconditional message
void dbgntu(const char * name)
// char *name ;     /* name of subroutine */
{
	ifcprf("Entering %s\n", name);
}

//* dbgxiu -- exit a subroutine -- unconditional message
void dbgxiu(const char * name)
// char *name ;     /* name of subroutine */
{
	ifcprf("Leaving %s\n", name);
}

//* dbgttg -- keyboard trace toggle hit
void dbgttg(void) {

	kybrd() ;       /* remove char from keyboard buffer */
	if (dbgtrc)         /* if trace flag already on */
		dbgtrc = 0 ;        /* turn it off */
	else dbgtrc = 1, dbganc = -1 ;   /* else turn it on,
            and also force dump of entire list */
	dbgtrk = 1 ;        /* make sure track flag is on */
}

//* dbgcnp -- check null pointer storage area clobbered
void dbgcnp(char * caller, char * name)
// char * caller ;      /* caller -- dbgntr, dbgxit */
// char * name ;        /* name of routine calling caller */
{
	int *pint ;     /* storage clobbered pointer check */
	static char firstcall = 1 ; /* flag -- first call to here */
	static int words[20] ;  /* check at most 20 more words */
	int i ;     /* loop variable */

	pint = NULL ;       /* storage clobbered check */
	if (*pint++ || *pint++ || *pint++ || *pint) {
		ifcprf("%s %s -- null pointer storage clobbered.", caller, name) ;
		dbgtyp(NULL, TRUE) ;
		error("Abort") ; //_exit(1) ;      /* exit if on */
	}

	if (dbgckf > 0) { /* if we want to check more words */
		if (firstcall) {    /* if this is first call */
			firstcall = 0 ; /* no longer first call */
			if (dbgckf > 20)    /* if too many */
				dbgckf = 20 ;
			for (i = 0 ; i < dbgckf ; ++i)
				words[i] = *pint++ ;    /* copy a word */
		} else      /* not the first call */
			for (i = 0 ; i < dbgckf ; ++i)
				if (words[i] != *pint++) {  /* if word wrong */
					ifcprf("%s %s -- word # %d + 4 clobbered.",
					       caller, name, i) ;
					dbgtyp(NULL, TRUE) ;
					error("Abort") ; //_exit(1) ;      /* exit if on */
				}
	}
}

#ifndef JXKEYBOARD
int kybtst(void) {
	return 0;
}
int kybrd(void) {
	return 0;
}

#else

/* kyb1.c -- keyboard routines */
/* Copyright (C) 1984 by John J. Xenakis Company */
// Modified by John J. Xenakis for Papyrus

#include <conio.h>
#include <dos.h>

#define INTKYB 0x16 /* keyboard interrupt code */
#define QSIZE 100   /* input queue size */

char kybxx1[6] = "KYBXX1";
int kybccc = 0 ;    /* break char count */
int *kybqbg, *kybqnd ;  /* queue beginning and end */
int kybqsz ;        /* queue size */
int kybque[QSIZE] ; /* keyboard input queue */
char kybxx2[6] = "KYBXX2";

//* kybupq -- input chars, update keyboard input queue
void kybupq(void) {
	if (kybqsz == 0)    /* if queues are empty */
		kybqbg = kybqnd = kybque ;  /* initialize queue pointers */
	while (kybqsz < QSIZE && kbhit() != 0)  /* loop as long as there
                    are chars in operating system queue,
                    and there's room in our queue */
	{
		/* get char, add to end of queue, and test for control-c */
		if ((*(kybqnd++) = kybord()) == CWCBRK) {
			if (++kybccc >= 5)  /* if five consecutive breaks */
				error("Abort") ; //_exit(1) ;      /* exit */
		} else kybccc = 0 ; /* else reset cc count */
		kybqsz++;       /* increment queue size */
		if (kybqnd >= kybque + QSIZE) /* if ptr past end of queue */
			kybqnd = kybque ;   /* reset queue end pointer */
	}
}

//* kybrd -- keyboard read -- wait if no char available
int kybrd(void) {
	int ch ;        /* output value: returned char */

	if (kybqsz == 0) return (kybord()); /* if keyboard queue is empty,
                return o/s keyboard char */
	ch = *(kybqbg++) ;  /* get char from beginning of queue */
	kybqsz-- ;      /* decrement queue size */
	if (kybqbg >= kybque + QSIZE) /* if pointer past end of queue */
		kybqbg = kybque ;   /* reset to beginning of queue */
	kybupq();       /* update queue */
	return (ch);    /* give caller the char */
}

//* kybrdz -- keyboard read, return zero if no char
int kybrdz(void) {
	kybupq() ;      /* update queue */
	if (kybqsz == 0) return (0) ;
	return kybrd();
}

//* kybord -- o/s keyboard read -- returns char or scan code + 1000
int kybord(void) {
	union REGS rr, rrr /* register sets */ ;

	rr.h.ah = 0 /* read char function */;

	int86(INTKYB, &rr, &rrr) /* do keyboard input */ ;
	if (rrr.h.al == 0) /* scan code in ah */
		return (1000 + rrr.h.ah) ;
	return (rrr.h.al) /* give char to caller */;
}

//* kybtst -- test for keyboard input, return char, leave in queue
// Quick test for keyboard input -- returns next char, but
//		doesn't remove it from queue
int kybtst(void) {
	kybupq();   /* update queue */
	return ((kybqsz > 0) ? *kybqbg : 0); /* return char in queue
                    if there is one */
}

//* kybnlt -- new line test -- test if there's a newline in queue
int kybnlt(void) {
	int flag ;      /* return value -- indicates new line */
	int n ;     /* queue size count */
	int *p ;        /* current queue pointer */

	kybupq() ;      /* update queue */

	flag = 0 ;      /* no newline yet */
	for (n = kybqsz, p = kybqbg ;
	        (n-- > 0) && (!(flag = (*p == CWCNL))) ;)
		if (++p > kybque + QSIZE)   /* end of ring buffer? */
			p = kybque ;    /* beginning of ring buffer */
	return (flag) ;     /* tell the caller */
}

#endif /* JXKEYBOARD */
#endif /* JX_DEBUG */


/* ifc2.c -- replacement for "printf" and "Common::sprintf_s" functions */
/* Copyright (C) 1984 by John J. Xenakis Company */

/** ifcprf -- formatted write to stdout **/
/* name         ifcprf -- formatted write to stdout
*
* synopsis     ifcprf(cs, arglist);
*              char *cs;       control string specifying formatting
*              "arglist"       items (if any) to be formatted
*
* description  This function generates formatted output to "stdout".
*              The actual field image construction is performed by
*              "ifcfmt".  Refer to Kernighan and Ritchie for format
*              specifications.
*              This version is for target machines which pass function
*              parameters on the stack in the standard call-by-value
*              mechanism.
**/

//* ifcprf -- printf replacement
void ifcprf(char * cs, ...)
// void ifcprf(char * cs, char * args)
// char *cs, *args;
{
	int i, n;
	char c, *p, **na, *ifcfmt();
	char work[256];

//    na =  &args;           /* point to first arg */
	na =  &cs;
	++na ;           /* point to first arg */
	while (*cs != '\0') {
		c = *cs++;
		if (c == '%')
			if (*cs == '%') {
				c = *cs++;
				ifcwtt(c);
			} else {
				p = ifcfmt(cs, work, &na, &n);
				if (p != NULL) {
					cs = p;
					for (i = 0; i < n; i++)
						ifcwtt(work[i]);
				}
			} else
			ifcwtt(c);
	}
	return;
}

/** ifcspf -- formatted write to string **/
/* name         ifcspf -- formatted write to string
*
* synopsis     n = ifcspf(ds, cs, arglist);
*              int n;          number of characters written
*              char *ds;       string for resulting image
*              char *cs;       control string specifying formatting
*              "arglist"       items (if any) to be formatted
* description  This function generates formatted output to the specified
*              string.  The actual field image construction is performed by
*              "ifcfmt".  Refer to Kernighan and Ritchie for format
*              specifications.
*              This version is for target machines which pass function
*              parameters on the stack in the standard call-by-value
*              mechanism.
* returns      n = number of characters placed in "ds"
**/

//* ifcspf -- Common::sprintf_s replacement
int ifcspf(char * ds, char * cs, ...)
// void ifcspf(char * ds, char * cs, char * args)
// char *ds;
// char *cs, *args;
{
	int i, c, n;
	char *p, *q, **na, *ifcfmt();
	char work[256];

	na =  &cs;
	++na ;           /* point to first arg */
	q = ds;                /* save destination string pointer */
	while (*cs != '\0') {
		c = *cs++;
		if (c == '%')
			if (*cs == '%')
				*ds++ = *cs++;
			else {
				p = ifcfmt(cs, work, &na, &n);
				if (p != NULL) {
					cs = p;
					for (i = 0; i < n; i++)
						*ds++ = work[i];
				}
			} else {
			*ds++ = (char)c;
		}
	}
	*ds = '\0';

	return ds - q;
}

/** ifcfmt -- generated formatted print string **/
/* name         ifcfmt -- generate formatted print string
*
* synopsis     p = ifcfmt(cs, ds, pv, flen);
*              char *p;        pointer to next byte in control string,
*                              or NULL if error
*              char *cs;       pointer to first char after % in format spec
*              char *ds;       destination string for field image
*              ---- **pv;      pointer to ptr to value to be formatted
*              int *flen;      location for return of field image size
*
* description  This function is called by the various formatted print
*              routines whenever a format descriptor (beginning with a %)
*              is encountered.  The "cs" pointer indicates the character
*              immediately following the % in the format control string.
*              The destination string receives an image without a terminating
*              null byte.  The "pv" pointer can point to any of the
*              various items which can legally appear as a function
*              parameter (see below); ifcfmt determines what the target
*              object is from the field descriptor.  This pointer is
*              incremented according to the size of that target object.
*              The length of the resulting field image is returned through
*              "flen".
*
* returns      p = pointer to first byte after field descriptor
*                = NULL if error (invalid field descriptor)
*
**/

#define MAXDIG 20      /* maximum decimal digits from CXVSD */

//* ifcfmt -- formatting routine
char *ifcfmt(char * cs, char * ds, void * pparg, int * flen)
// char *cs, *ds;
// int *flen;
{
//    int i,j,n,lj,lf,sign,prec,fill,mfw,expt,fpf;
	int i, n, lj, lf, sign, prec, fill, mfw ;
	char *p;
	long v;
	char buf[MAXDIG];
	static char hex[16] = "0123456789ABCDEF";
	union   {
		int *pi;
		long *pl;
		//double *pd;
		char **ps;
	} *pp = pparg ;

	lj = mfw = lf = sign = 0;
	prec = -1;
	fill = ' ';
	if (*cs == '-') {
		/* left justify */
		lj = 1;
		++cs;
	}
	if (isdigit(*cs)) {
		/* field width specified */
		if (*cs == '0') fill = '0';
		mfw = *cs++ & 15;
		while (isdigit(*cs)) mfw = 10 * mfw + (*cs++ & 15);
	}
	if (*cs == '.') {
		/* precision specified */
		++cs;
		prec = 0;
		while (isdigit(*cs)) prec = 10 * prec + (*cs++ & 15);
	}
	if (*cs == 'l') {
		/* long item */
		lf = 1;
		++cs;
	}
	p = NULL;              /* reset buffer pointer */
	switch (*cs) {
	/* switch on descriptor type */

	case 'd':           /* decimal integer */
		if (lf) v = *pp->pl++;
		else v = *pp->pi++;
		if (v < 0) {
			v = -v;
			sign = 1;
		}
		goto dec;

	case 'u':           /* unsigned integer */
		if (lf) v = *pp->pl++;
		else v = (unsigned) * pp->pi++;
dec:
		i = 11;
		do {
			buf[--i] = (char)('0' + v % 10) ;
			v /= 10;
		} while (v != 0);
		if (sign == 1) buf[--i] = '-';
		p = &buf[i];
		n = 11 - i;
		break;

	case 'x':           /* hexadecimal integer */
		if (lf) v = *pp->pl++;
		else v = (unsigned) * pp->pi++;
		i = 8;
		do {
			buf[--i] = hex[v & 15];
			v >>= 4;
			v &= 0xfffffff;
		} while (v != 0);
		p = &buf[i];
		n = 8 - i;
		break;

	case 'o':           /* octal integer */
		if (lf) v = *pp->pl++;
		else v = (unsigned) * pp->pi++;
		i = 11;
		do {
			buf[--i] = (char)('0' + (v & 7)) ;
			v >>= 3;
			v &= 0x1fffffff;
		} while (v != 0);
		p = &buf[i];
		n = 11 - i;
		break;

	case 's':           /* string */
		if (prec == -1) prec = 200;
		for (n = 0; n < prec && (*pp->ps)[n] != '\0'; n++);
		p = *pp->ps++;
		break;

	case 'c':           /* single character */
		p = buf;
		n = 1;
		buf[0] = (char) * pp->pi++;
		break;

		#ifdef FLOAT_PT     /* floating point code doesn't work */
	case 'g':           /* general floating point format */
		fpf = 2;
		break;

	case 'e':           /* exponential floating point */
		fpf = 0;
		break;

	case 'f':           /* fixed point float */
		fpf = 1;
		break;
		#endif /* FLOAT_PT */

	default:
		return NULL;
	}

	if (p != NULL) {
		/* not floating point */
		if (mfw == 0 || mfw < n) mfw = n;
		mfw -= n;
		i = 0;
		if (lj) {
			/* left justified */
			while (--n >= 0)
				ds[i++] = *p++;
			while (--mfw >= 0)
				ds[i++] = (char)fill;
		} else {
			/* right justified */
			while (--mfw >= 0)
				ds[i++] = (char)fill;
			while (--n >= 0)
				ds[i++] = *p++;
		}
		*flen = i;
		return cs + 1;
	}

	#ifdef FLOAT_PT     /* floating point code doesn't work */
	if (prec == -1) prec = 6;
	n = (prec >= MAXDIG) ? MAXDIG - 1 : prec;
	n = CXVFD(pp->pd, n + 1, fpf, &expt, &sign, buf);
	/* convert to decimal digits */
	pp->pd++;
	p = buf;
	i = expt;
	if (i < 0) i = -i;
	if (fpf == 2) fpf = (n == 0 || i < 6);
	if (n != 0) --expt;
	i = 0;
	if (sign) i++;
	if (fpf != 0) {
		/* f format */
		i += prec + 1;
		if (expt >= 0) i += expt;
		if (prec != 0) i++;
	} else {
		i += prec + 6;
		j = expt < 0 ? -expt : expt;
		if (j > 99) i++;
		if (j > 999) i++;
	}
	if (lj == 0 && mfw > i)
		for (mfw -= i; --mfw >= 0; i++) *ds++ = fill;
	if (sign) *ds++ = '-';
	if (fpf != 0) {
		/* f format */
		if (expt < 0) {
			*ds++ = '0';
			*ds++ = '.';
			while (--prec >= 0)
				if (++expt < 0)
					*ds++ = '0';
				else if (--n >= 0)
					*ds++ = *p++;
				else
					*ds++ = '0';
		} else {
			while (expt-- >= 0)
				if (--n >= 0)
					*ds++ = *p++;
				else
					*ds++ = '0';
			if (prec != 0) *ds++ = '.';
			while (--prec >= 0)
				if (--n >= 0)
					*ds++ = *p++;
				else
					*ds++ = '0';
		}
	} else {
		/* e format */
		if (--n >= 0) *ds++ = *p++;
		else *ds++ = '0';
		*ds++ = '.';
		while (--prec >= 0)
			if (--n >= 0)
				*ds++ = *p++;
			else
				*ds++ = '0';
		*ds++ = 'E';
		if (expt < 0) {
			*ds++ = '-';
			expt = -expt;
		} else *ds++ = '+';
		n = 11;
		do  {
			buf[--n] = '0' + expt % 10;
			expt /= 10;
		} while (n > 9 || expt != 0);
		while (n < 11) *ds++ = buf[n++];
	}
	if (lj == 1 && mfw > i)
		for (mfw -= i; --mfw >= 0; i++) *ds++ = fill;
	*flen = i;
	return cs + 1;
	#endif /* FLOAT_PT */
}


//* ifcwst -- write character string to screen
void ifcwst(LPCSTR xpStr) {
	char ch;
	if (dbgfile[0]) {
		while ((ch = *xpStr++) != '\0')
			ifcwtt(ch);
	} else {
		OutputDebugString(xpStr);
	}
}


#define INTVIO 0x10     /* video interrupt code */

//* ifcwtt -- write character to screen
void ifcwtt(char ch) {
//    if (ch == '\n')   // for newline (0x0a)
//	ifcwwf('\r') ;   // write carriage return before newline (0x0d)
	ifcwwf(ch) ;    // write char to windows file
//    putchar(ch) ;
//	// eliminate putchar, since it brings in other C library routines

	#if 0
	if (ch == '\n') // for newline (0x0a)
		_bdos(0x02, '\r', 0) ;  // do carriage return first (0x0d)
	_bdos(0x02, ch, 0) ;    // DOS system call to output char
	// (the char may be redirected; the call below
	// using INTVIO doesn't allow redirection)
	#endif

	#if 0
	union REGS rr, rrr ; /* register sets */

	/* set up for video_io interrupt */
	rr.h.ah = 14 ; /* write teletype to active page */
	rr.h.bl = 0 ; /* foreground color */
	rr.h.bh = 0 ;   /* needed for COMPAQ */
	if (ch == '\xa') {  /* if char is a line feed */
		rr.h.al = '\xd' ;   /* set char to carriage return */
		int86(INTVIO, &rr, &rrr); /* and do carriage ret first */
	}
	rr.h.al = ch ; /* set char to write */
	int86(INTVIO, &rr, &rrr) ; /* do video_io interrupt */
	#endif
}

//* ifcwnl -- write newline
void ifcwnl(void) {
	ifcwtt('\xa') ;
}

//* ifcbep -- "display" a beep
void ifcbep(void) {
	ifcwtt('\7') ;  /* write out control-g */
}

//* ifcwwf -- write character to windows file
void ifcwwf(int iChar)
// iChar -- output char, or -1 to close output file
{
	static FILE * xpOutput = NULL ; // output file stream
//    char szFilename[100] ;    // name of output file
	static int iCount = 0 ; // count checkpoint chars
	char cOut[2] ;

	if (dbgfile[0]) {
		if ((iChar & 0xff) == 0xff) {
			if (xpOutput)
				fclose(xpOutput) ;
			xpOutput = NULL ;
		} else {
			if (!xpOutput)
				xpOutput = fopen(dbgfile, "a") ;

			if (xpOutput)
				fputc(iChar, xpOutput) ;
			if (++iCount > dbgreopen) {
				fclose(xpOutput) ;
				xpOutput = fopen(dbgfile, "a") ;
				iCount = 0 ;
			}

		}
	} else if ((iChar & 0xff) != 0xff) {
		cOut[0] = (char)iChar ;
		cOut[1] = '\0';
		if (iChar > 0)
			OutputDebugString(cOut) ;
	}

}

// ===============================

// WDDB -- window dump block, used by following subroutines
typedef struct WDDB {
	int iDepth ;    // depth
	HWND hCurrentParent ;   // current parent window
	HWND hFocusWnd ;    // window with focus
} FAR *LPWDDB ;


//* dbgWindowDump -- dump current window tree
VOID FAR PASCAL dbgWindowDump(HWND hStartWnd)
// hStartWnd -- starting point; if NULL, dump all windows on screen
// returns: void
{
	int iError ;
	HWND hWnd /*, hParentWnd */ ;   // current/parent window
	struct WDDB stWddb ;    // window dump block
	int iSaveDbgtrc ;

	JXENTER(dbgWindowDump) ;
	iSaveDbgtrc = dbgtrc ;
	dbgtrc = FALSE ;
	iError = 0 ;
	stWddb.hFocusWnd = GetFocus() ; // get current focus

	// is window arg valid?
	if (!IsWindow((hWnd = hStartWnd))) {
		hWnd = (HWND)NULL ;     // if not, do all windows
	}

	stWddb.iDepth = 0 ; // clear the parameter block
	stWddb.hCurrentParent = hWnd ;  // set current parent

	// if we're doing children of one window
	if (hWnd) {
		// dump info on top window
		dbgDumpWindowInfo(hWnd, (DWORD)(LPWDDB)&stWddb) ;
		EnumChildWindows(hWnd, (FARPROC)dbgDumpEnumProc, (DWORD)(LPWDDB)&stWddb) ; // do children

		// doing all windows on screen
	} else {
		EnumWindows((FARPROC)dbgDumpEnumProc, (DWORD)(LPWDDB)&stWddb) ; // do parents
	}
	dbgtrc = iSaveDbgtrc ;
	JXELEAVE(dbgWindowDump) ;
	RETURN_VOID ;
}

//* dbgDumpEnumProc - enumeration function to find child windows
BOOL FAR PASCAL dbgDumpEnumProc(HWND hWnd, DWORD dwlpWddb)
// hWnd -- pointer to child window in enumeration
// dwlpWddb -- ptr to dump block (defined above)
// returns: TRUE to continue enumeration; FALSE to stop enumeration
{
	int iError = 0 ;
	LPWDDB lpWddb ; // pointer to dump block (defined above)

	JXENTER(dbgDumpEnumProc) ;
	lpWddb = (LPWDDB)dwlpWddb ; // copy pointer to control block
	if (GetParent(hWnd) == lpWddb->hCurrentParent)
		// if this is at correct level
	{
		lpWddb->iDepth++ ;  // increment depth
		dbgDumpWindowInfo(hWnd, (DWORD)(LPWDDB)lpWddb) ;
		// dump info on current window
		lpWddb->hCurrentParent = hWnd ; // update current parent
		EnumChildWindows(hWnd, (FARPROC)dbgDumpEnumProc,
		                 (DWORD)(LPWDDB)lpWddb) ;    // do children
		lpWddb->iDepth-- ;  // decrement depth
		lpWddb->hCurrentParent = GetParent(hWnd) ;  // reset parent
	}
	JXELEAVE(dbgDumpEnumProc) ;
	RETURN(TRUE) ;
}

//* dbgDumpWindowInfo - dump info on one window
VOID dbgDumpWindowInfo(HWND hWnd, DWORD dwlpWddb)
// hWnd -- pointer to window to be dump
// dwlpWddb -- ptr to dump block (defined above)
// returns: void
{
	int iError = 0 ;
	LPWDDB lpWddb ; // pointer to dump block (defined above)
	char szMessageClass[25] ;   // message class
	char szCaption[25] ;    // window caption
	char szDebugString[160] ;   // output string
	RECT stRect ;       // rectangle for window dimension

	JXENTER(dbgDumpWindowInfo) ;
	lpWddb = (LPWDDB)dwlpWddb ; // copy pointer to control block

	GetClassName(hWnd, szMessageClass, sizeof(szMessageClass)) ;
	// get name of window class
	GetWindowText(hWnd, szCaption, sizeof(szCaption)) ; // get caption
	GetWindowRect(hWnd, &stRect) ;  // get window coordinates
	lstrcpy(szDebugString,
	        "\n****************************************") ;
	wsprintf(szDebugString + lpWddb->iDepth + 1,
	         " %04x %s:%s (%d,%d,%d,%d)",
	         hWnd, (LPSTR)szMessageClass,
	         (LPSTR)szCaption, stRect.left, stRect.top,
	         stRect.right - stRect.left,
	         stRect.bottom - stRect.top) ;

	if (IsIconic(hWnd)) {
		strncat(szDebugString, " [Iconic]",
		        sizeof(szDebugString) -
		        strlen(szDebugString) - 1) ;
	}

	if (IsZoomed(hWnd)) {
		strncat(szDebugString, " [Maximized]",
		        sizeof(szDebugString) -
		        strlen(szDebugString) - 1) ;
	}

	if (!IsWindowEnabled(hWnd)) {
		strncat(szDebugString, " [Disabled]",
		        sizeof(szDebugString) -
		        strlen(szDebugString) - 1) ;
	}

	if (!IsWindowVisible(hWnd)) {
		strncat(szDebugString, " [Hidden]",
		        sizeof(szDebugString) -
		        strlen(szDebugString) - 1) ;
	}

	if (hWnd == lpWddb->hFocusWnd) {
		strncat(szDebugString, " [Focus]",
		        sizeof(szDebugString) -
		        strlen(szDebugString) - 1) ;
	}

	*(szDebugString + (sizeof(szDebugString) - 1)) = 0 ;
	JXOutputDebugString(szDebugString) ;
	JXELEAVE(dbgDumpWindowInfo) ;
	RETURN_VOID ;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif /* _DEBUG */
