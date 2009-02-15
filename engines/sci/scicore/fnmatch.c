/* fnmatch.c -- ksh-like extended pattern matching for the shell and filename
		globbing. */

/* Copyright (C) 1991, 1997 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.
   
   Bash is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.
	      
   Bash is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.
			 
   You should have received a copy of the GNU General Public License along
   with Bash; see the file COPYING.  If not, write to the Free Software
   Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA. */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef HAVE_FNMATCH

#include <stdio.h>	/* for debugging */
				
#include <ctype.h>

#if defined (HAVE_STRING_H)
#  include <string.h>
#else
#  include <strings.h>
#endif /* HAVE_STRING_H */

static int gmatch ();
static char *brackmatch ();
#ifdef EXTENDED_GLOB
static int extmatch ();
static char *patscan ();
#endif
  
#if !defined (isascii)
#  define isascii(c)	((unsigned int)(c) <= 0177)
#endif

/* Note that these evaluate C many times.  */

#ifndef isblank
#  define isblank(c)	((c) == ' ' || (c) == '\t')
#endif

#ifndef isgraph
#  define isgraph(c)	((c) != ' ' && isprint((c)))
#endif

#ifndef isxdigit
#  define isxdigit(c)	(((c) >= '0' && (c) <= '9') || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#endif

/* The result of FOLD is an `unsigned char' */
# define FOLD(c) ((flags & FNM_CASEFOLD) && isupper ((unsigned char)c) \
	? tolower ((unsigned char)c) \
	: ((unsigned char)c))

#ifndef STREQ
#define STREQ(a, b) ((a)[0] == (b)[0] && strcmp(a, b) == 0)
#define STREQN(a, b, n) ((a)[0] == (b)[0] && strncmp(a, b, n) == 0)
#endif

/* We use strcoll(3) for range comparisons in bracket expressions,
   even though it can have unwanted side effects in locales
   other than POSIX or US.  For instance, in the de locale, [A-Z] matches
   all characters. */

#if defined (HAVE_STRCOLL)
/* Helper function for collating symbol equivalence. */
static int rangecmp (c1, c2)
     int c1, c2;
{
  static char s1[2] = { ' ', '\0' };
  static char s2[2] = { ' ', '\0' };
  int ret;

  /* Eight bits only.  Period. */
  c1 &= 0xFF;
  c2 &= 0xFF;

  if (c1 == c2)
    return (0);

  s1[0] = c1;
  s2[0] = c2;

  if ((ret = strcoll (s1, s2)) != 0)
    return ret;
  return (c1 - c2);
}
#else /* !HAVE_STRCOLL */
#  define rangecmp(c1, c2)	((int)(c1) - (int)(c2))
#endif /* !HAVE_STRCOLL */

#if defined (HAVE_STRCOLL)
static int collequiv (c1, c2)
     int c1, c2;
{
  return (rangecmp (c1, c2) == 0);
}
#else
#  define collequiv(c1, c2)	((c1) == (c2))
#endif

static int
collsym (s, len)
     char *s;
     int len;
{
  register struct _collsym *csp;

  for (csp = posix_collsyms; csp->name; csp++)
    {
      if (STREQN(csp->name, s, len) && csp->name[len] == '\0')
	return (csp->code);
    }
  if (len == 1)
    return s[0];
  return -1;
}

int
fnmatch (pattern, string, flags)
     char *pattern;
     char *string;
     int flags;
{
  char *se, *pe;

  if (string == 0 || pattern == 0)
    return FNM_NOMATCH;

  se = string + strlen (string);
  pe = pattern + strlen (pattern);

  return (gmatch (string, se, pattern, pe, flags));
}

/* Match STRING against the filename pattern PATTERN, returning zero if
   it matches, FNM_NOMATCH if not.  */
static int
gmatch (string, se, pattern, pe, flags)
     char *string, *se;
     char *pattern, *pe;
     int flags;
{
  register char *p, *n;		/* pattern, string */
  register char c;		/* current pattern character */
  register char sc;		/* current string character */

  p = pattern;
  n = string;

  if (string == 0 || pattern == 0)
    return FNM_NOMATCH;

#if DEBUG_MATCHING
fprintf(stderr, "gmatch: string = %s; se = %s\n", string, se);
fprintf(stderr, "gmatch: pattern = %s; pe = %s\n", pattern, pe);
#endif

  while (p < pe)
    {
      c = *p++;
      c = FOLD (c);

      sc = n < se ? *n : '\0';

#ifdef EXTENDED_GLOB
      /* extmatch () will handle recursively calling gmatch, so we can
	 just return what extmatch() returns. */
      if ((flags & FNM_EXTMATCH) && *p == '(' &&
	  (c == '+' || c == '*' || c == '?' || c == '@' || c == '!')) /* ) */
	{
	  int lflags;
	  /* If we're not matching the start of the string, we're not
	     concerned about the special cases for matching `.' */
	  lflags = (n == string) ? flags : (flags & ~FNM_PERIOD);
	  return (extmatch (c, n, se, p, pe, lflags));
	}
#endif

      switch (c)
	{
	case '?':		/* Match single character */
	  if (sc == '\0')
	    return FNM_NOMATCH;
	  else if ((flags & FNM_PATHNAME) && sc == '/')
	    /* If we are matching a pathname, `?' can never match a `/'. */
	    return FNM_NOMATCH;
	  else if ((flags & FNM_PERIOD) && sc == '.' &&
		   (n == string || ((flags & FNM_PATHNAME) && n[-1] == '/')))
	    /* `?' cannot match a `.' if it is the first character of the
	       string or if it is the first character following a slash and
	       we are matching a pathname. */
	    return FNM_NOMATCH;
	  break;

	case '\\':		/* backslash escape removes special meaning */
	  if (p == pe)
	    return FNM_NOMATCH;

	  if ((flags & FNM_NOESCAPE) == 0)
	    {
	      c = *p++;
	      /* A trailing `\' cannot match. */
	      if (p > pe)
		return FNM_NOMATCH;
	      c = FOLD (c);
	    }
	  if (FOLD (sc) != (unsigned char)c)
	    return FNM_NOMATCH;
	  break;

	case '*':		/* Match zero or more characters */
	  if (p == pe)
	    return 0;
	  
	  if ((flags & FNM_PERIOD) && sc == '.' &&
	      (n == string || ((flags & FNM_PATHNAME) && n[-1] == '/')))
	    /* `*' cannot match a `.' if it is the first character of the
	       string or if it is the first character following a slash and
	       we are matching a pathname. */
	    return FNM_NOMATCH;

	  /* Collapse multiple consecutive, `*' and `?', but make sure that
	     one character of the string is consumed for each `?'. */
	  for (c = *p++; (c == '?' || c == '*'); c = *p++)
	    {
	      if ((flags & FNM_PATHNAME) && sc == '/')
		/* A slash does not match a wildcard under FNM_PATHNAME. */
		return FNM_NOMATCH;
	      else if (c == '?')
		{
		  if (sc == '\0')
		    return FNM_NOMATCH;
		  /* One character of the string is consumed in matching
		     this ? wildcard, so *??? won't match if there are
		     fewer than three characters. */
		  n++;
		  sc = n < se ? *n : '\0';
		}

#ifdef EXTENDED_GLOB
	      /* Handle ******(patlist) */
	      if ((flags & FNM_EXTMATCH) && c == '*' && *p == '(')  /*)*/
		{
		  char *newn;
		  /* We need to check whether or not the extended glob
		     pattern matches the remainder of the string.
		     If it does, we match the entire pattern. */
		  for (newn = n; newn < se; ++newn)
		    {
		      if (extmatch (c, newn, se, p, pe, flags) == 0)
			return (0);
		    }
		  /* We didn't match the extended glob pattern, but
		     that's OK, since we can match 0 or more occurrences.
		     We need to skip the glob pattern and see if we
		     match the rest of the string. */
		  newn = patscan (p + 1, pe, 0);
		  p = newn;
		}
#endif
	      if (p == pe)
		break;
	    }

	  /* If we've hit the end of the pattern and the last character of
	     the pattern was handled by the loop above, we've succeeded.
	     Otherwise, we need to match that last character. */
	  if (p == pe && (c == '?' || c == '*'))
	    return (0);

	  /* General case, use recursion. */
	  {
	    unsigned char c1;

	    c1 = (unsigned char)((flags & FNM_NOESCAPE) == 0 && c == '\\') ? *p : c;
	    c1 = FOLD (c1);
	    for (--p; n < se; ++n)
	      {
		/* Only call fnmatch if the first character indicates a
		   possible match.  We can check the first character if
		   we're not doing an extended glob match. */
		if ((flags & FNM_EXTMATCH) == 0 && c != '[' && FOLD (*n) != c1) /*]*/
		  continue;

		/* If we're doing an extended glob match and the pattern is not
		   one of the extended glob patterns, we can check the first
		   character. */
		if ((flags & FNM_EXTMATCH) && p[1] != '(' && /*)*/
		    strchr ("?*+@!", *p) == 0 && c != '[' && FOLD (*n) != c1) /*]*/
		  continue;

		/* Otherwise, we just recurse. */
		if (gmatch (n, se, p, pe, flags & ~FNM_PERIOD) == 0)
		  return (0);
	      }
	    return FNM_NOMATCH;
	  }

	case '[':
	  {
	    if (sc == '\0' || n == se)
	      return FNM_NOMATCH;

	    /* A character class cannot match a `.' if it is the first
	       character of the string or if it is the first character
	       following a slash and we are matching a pathname. */
	    if ((flags & FNM_PERIOD) && sc == '.' &&
		(n == string || ((flags & FNM_PATHNAME) && n[-1] == '/')))
	      return (FNM_NOMATCH);

	    p = brackmatch (p, sc, flags);
	    if (p == 0)
	      return FNM_NOMATCH;
	  }
	  break;

	default:
	  if ((unsigned char)c != FOLD (sc))
	    return (FNM_NOMATCH);
	}

      ++n;
    }

  if (n == se)
    return (0);

  if ((flags & FNM_LEADING_DIR) && *n == '/')
    /* The FNM_LEADING_DIR flag says that "foo*" matches "foobar/frobozz".  */
    return 0;
	  
  return (FNM_NOMATCH);
}

/* Parse a bracket expression collating symbol ([.sym.]) starting at P, find
   the value of the symbol, and move P past the collating symbol expression.
   The value is returned in *VP, if VP is not null. */
static char *
parse_collsym (p, vp)
     char *p;
     int *vp;
{
  register int pc;
  int val;

  p++;				/* move past the `.' */
	  
  for (pc = 0; p[pc]; pc++)
    if (p[pc] == '.' && p[pc+1] == ']')
      break;
   val = collsym (p, pc);
   if (vp)
     *vp = val;
   return (p + pc + 2);
}

static char *
brackmatch (p, test, flags)
     char *p;
     unsigned char test;
     int flags;
{
  register char cstart, cend, c;
  register int not;    /* Nonzero if the sense of the character class is inverted.  */
  int pc, brcnt;
  char *savep;

  test = FOLD (test);

  savep = p;

  /* POSIX.2 3.13.1 says that an exclamation mark (`!') shall replace the
     circumflex (`^') in its role in a `nonmatching list'.  A bracket
     expression starting with an unquoted circumflex character produces
     unspecified results.  This implementation treats the two identically. */
  if (not = (*p == '!' || *p == '^'))
    ++p;

  c = *p++;
  for (;;)
    {
      /* Initialize cstart and cend in case `-' is the last
	 character of the pattern. */
      cstart = cend = c;

      /* POSIX.2 equivalence class:  [=c=].  See POSIX.2 2.8.3.2.  Find
	 the end of the equivalence class, move the pattern pointer past
	 it, and check for equivalence.  XXX - this handles only
	 single-character equivalence classes, which is wrong, or at
	 least incomplete. */
      if (c == '[' && *p == '=' && p[2] == '=' && p[3] == ']')
	{
	  pc = FOLD (p[1]);
	  p += 4;
	  if (collequiv (test, pc))
	    {
/*[*/	      /* Move past the closing `]', since the first thing we do at
		 the `matched:' label is back p up one. */
	      p++;
	      goto matched;
	    }
	  else
	    {
	      c = *p++;
	      if (c == '\0')
		return ((test == '[') ? savep : (char *)0); /*]*/
	      c = FOLD (c);
	      continue;
	    }
	}

      /* POSIX.2 character class expression.  See POSIX.2 2.8.3.2. */
      if (c == '[' && *p == ':')	/*]*/
	{
	  pc = 0;	/* make sure invalid char classes don't match. */
	  if (STREQN (p+1, "alnum:]", 7))
	    { pc = isalnum (test); p += 8; }
	  else if (STREQN (p+1, "alpha:]", 7))
	    { pc = isalpha (test); p += 8; }
	  else if (STREQN (p+1, "blank:]", 7))
	    { pc = isblank (test); p += 8; }
	  else if (STREQN (p+1, "cntrl:]", 7))
	    { pc = iscntrl (test); p += 8; }
	  else if (STREQN (p+1, "digit:]", 7))
	    { pc = isdigit (test); p += 8; }
	  else if (STREQN (p+1, "graph:]", 7))
	    { pc = isgraph (test); p += 8; }
	  else if (STREQN (p+1, "lower:]", 7))
	    { pc = islower (test); p += 8; }
	  else if (STREQN (p+1, "print:]", 7))
	    { pc = isprint (test); p += 8; }
	  else if (STREQN (p+1, "punct:]", 7))
	    { pc = ispunct (test); p += 8; }
	  else if (STREQN (p+1, "space:]", 7))
	    { pc = isspace (test); p += 8; }
	  else if (STREQN (p+1, "upper:]", 7))
	    { pc = isupper (test); p += 8; }
	  else if (STREQN (p+1, "xdigit:]", 8))
	    { pc = isxdigit (test); p += 9; }
	  else if (STREQN (p+1, "ascii:]", 7))
	    { pc = isascii (test); p += 8; }
	  if (pc)
	    {
/*[*/	      /* Move past the closing `]', since the first thing we do at
		 the `matched:' label is back p up one. */
	      p++;
	      goto matched;
	    }
	  else
	    {
	      /* continue the loop here, since this expression can't be
		 the first part of a range expression. */
	      c = *p++;
	      if (c == '\0')
		return ((test == '[') ? savep : (char *)0);
	      else if (c == ']')
		break;
	      c = FOLD (c);
	      continue;
	    }
	}
 
      /* POSIX.2 collating symbols.  See POSIX.2 2.8.3.2.  Find the end of
	 the symbol name, make sure it is terminated by `.]', translate
	 the name to a character using the external table, and do the
	 comparison. */
      if (c == '[' && *p == '.')
	{
	  p = parse_collsym (p, &pc);
	  /* An invalid collating symbol cannot be the first point of a
	     range.  If it is, we set cstart to one greater than `test',
	     so any comparisons later will fail. */
	  cstart = (pc == -1) ? test + 1 : pc;
	}

      if (!(flags & FNM_NOESCAPE) && c == '\\')
	{
	  if (*p == '\0')
	    return (char *)0;
	  cstart = cend = *p++;
	}

      cstart = cend = FOLD (cstart);

      /* POSIX.2 2.8.3.1.2 says: `An expression containing a `[' that
	 is not preceded by a backslash and is not part of a bracket
	 expression produces undefined results.'  This implementation
	 treats the `[' as just a character to be matched if there is
	 not a closing `]'. */
      if (c == '\0')
	return ((test == '[') ? savep : (char *)0);

      c = *p++;
      c = FOLD (c);

      if ((flags & FNM_PATHNAME) && c == '/')
	/* [/] can never match when matching a pathname.  */
	return (char *)0;

      /* This introduces a range, unless the `-' is the last
	 character of the class.  Find the end of the range
	 and move past it. */
      if (c == '-' && *p != ']')
	{
	  cend = *p++;
	  if (!(flags & FNM_NOESCAPE) && cend == '\\')
	    cend = *p++;
	  if (cend == '\0')
	    return (char *)0;
	  if (cend == '[' && *p == '.')
	    {
	      p = parse_collsym (p, &pc);
	      /* An invalid collating symbol cannot be the second part of a
		 range expression.  If we get one, we set cend to one fewer
		 than the test character to make sure the range test fails. */
	      cend = (pc == -1) ? test - 1 : pc;
	    }
	  cend = FOLD (cend);

	  c = *p++;

	  /* POSIX.2 2.8.3.2:  ``The ending range point shall collate
	     equal to or higher than the starting range point; otherwise
	     the expression shall be treated as invalid.''  Note that this
	     applies to only the range expression; the rest of the bracket
	     expression is still checked for matches. */
	  if (rangecmp (cstart, cend) > 0)
	    {
	      if (c == ']')
		break;
	      c = FOLD (c);
	      continue;
	    }
	}

      if (rangecmp (test, cstart) >= 0 && rangecmp (test, cend) <= 0)
	goto matched;

      if (c == ']')
	break;
    }
  /* No match. */
  return (!not ? (char *)0 : p);

matched:
  /* Skip the rest of the [...] that already matched.  */
#if 0
  brcnt = (c != ']') + (c == '[' && (*p == '=' || *p == ':' || *p == '.'));
#else
  c = *--p;
  brcnt = 1;
#endif
  while (brcnt > 0)
    {
      /* A `[' without a matching `]' is just another character to match. */
      if (c == '\0')
	return ((test == '[') ? savep : (char *)0);

      c = *p++;
      if (c == '[' && (*p == '=' || *p == ':' || *p == '.'))
	brcnt++;
      else if (c == ']')
	brcnt--;
      else if (!(flags & FNM_NOESCAPE) && c == '\\')
	{
	  if (*p == '\0')
	    return (char *)0;
	  /* XXX 1003.2d11 is unclear if this is right. */
	  ++p;
	}
    }
  return (not ? (char *)0 : p);
}

#if defined (EXTENDED_GLOB)
/* ksh-like extended pattern matching:

	[?*+@!](pat-list)

   where pat-list is a list of one or patterns separated by `|'.  Operation
   is as follows:

	?(patlist)	match zero or one of the given patterns
	*(patlist)	match zero or more of the given patterns
	+(patlist)	match one or more of the given patterns
	@(patlist)	match exactly one of the given patterns
	!(patlist)	match anything except one of the given patterns
*/

/* Scan a pattern starting at STRING and ending at END, keeping track of
   embedded () and [].  If DELIM is 0, we scan until a matching `)'
   because we're scanning a `patlist'.  Otherwise, we scan until we see
   DELIM.  In all cases, we never scan past END.  The return value is the
   first character after the matching DELIM. */
static char *
patscan (string, end, delim)
     char *string, *end;
     int delim;
{
  int pnest, bnest, cchar;
  char *s, c, *bfirst;

  pnest = bnest = cchar = 0;
  bfirst = 0;
  for (s = string; c = *s; s++)
    {
      if (s >= end)
	return (s);
      switch (c)
	{
	case '\0':
	  return ((char *)0);

	/* `[' is not special inside a bracket expression, but it may
	   introduce one of the special POSIX bracket expressions
	   ([.SYM.], [=c=], [: ... :]) that needs special handling. */
	case '[':
	  if (bnest == 0)
	    {
	      bfirst = s + 1;
	      if (*bfirst == '!' || *bfirst == '^')
		bfirst++;
	      bnest++;
	    }
	  else if (s[1] == ':' || s[1] == '.' || s[1] == '=')
	    cchar = s[1];
	  break;

	/* `]' is not special if it's the first char (after a leading `!'
	   or `^') in a bracket expression or if it's part of one of the
	   special POSIX bracket expressions ([.SYM.], [=c=], [: ... :]) */
	case ']':
	  if (bnest)
	    {
	      if (cchar && s[-1] == cchar)
		cchar = 0;
	      else if (s != bfirst)
		{
		  bnest--;
		  bfirst = 0;
		}
	    }
	  break;

	case '(':
	  if (bnest == 0)
	    pnest++;
	  break;

	case ')':
#if 0
	  if (bnest == 0)
	    pnest--;
	  if (pnest <= 0)
	    return ++s;
#else
	  if (bnest == 0 && pnest-- <= 0)
	    return ++s;
#endif
	  break;

	case '|':
	  if (bnest == 0 && pnest == 0 && delim == '|')
	    return ++s;
	  break;
	}
    }

  return (char *)0;
}

/* Return 0 if dequoted pattern matches S in the current locale. */
static int
strcompare (p, pe, s, se)
     char *p, *pe, *s, *se;
{
  int ret;
  char c1, c2;

  c1 = *pe;
  c2 = *se;

  *pe = *se = '\0';
#if defined (HAVE_STRCOLL)
  ret = strcoll (p, s);
#else
  ret = strcmp (p, s);
#endif

  *pe = c1;
  *se = c2;

  return (ret == 0 ? ret : FNM_NOMATCH);
}

/* Match a ksh extended pattern specifier.  Return FNM_NOMATCH on failure or
   0 on success.  This is handed the entire rest of the pattern and string
   the first time an extended pattern specifier is encountered, so it calls
   gmatch recursively. */
static int
extmatch (xc, s, se, p, pe, flags)
     int xc;		/* select which operation */
     char *s, *se;
     char *p, *pe;
     int flags;
{
  char *prest;			/* pointer to rest of pattern */
  char *psub;			/* pointer to sub-pattern */
  char *pnext;			/* pointer to next sub-pattern */
  char *srest;			/* pointer to rest of string */
  int m1, m2;

#if DEBUG_MATCHING
fprintf(stderr, "extmatch: xc = %c\n", xc);
fprintf(stderr, "extmatch: s = %s; se = %s\n", s, se);
fprintf(stderr, "extmatch: p = %s; pe = %s\n", p, pe);
#endif

  prest = patscan (p + (*p == '('), pe, 0); /* ) */
  if (prest == 0)
    /* If PREST is 0, we failed to scan a valid pattern.  In this
       case, we just want to compare the two as strings. */
    return (strcompare (p - 1, pe, s, se));

  switch (xc)
    {
    case '+':			/* match one or more occurrences */
    case '*':			/* match zero or more occurrences */
      /* If we can get away with no matches, don't even bother.  Just
	 call gmatch on the rest of the pattern and return success if
	 it succeeds. */
      if (xc == '*' && (gmatch (s, se, prest, pe, flags) == 0))
	return 0;

      /* OK, we have to do this the hard way.  First, we make sure one of
	 the subpatterns matches, then we try to match the rest of the
	 string. */
      for (psub = p + 1; ; psub = pnext)
	{
	  pnext = patscan (psub, pe, '|');
	  for (srest = s; srest <= se; srest++)
	    {
	      /* Match this substring (S -> SREST) against this
		 subpattern (psub -> pnext - 1) */
	      m1 = gmatch (s, srest, psub, pnext - 1, flags) == 0;
	      /* OK, we matched a subpattern, so make sure the rest of the
		 string matches the rest of the pattern.  Also handle
		 multiple matches of the pattern. */
	      if (m1)
		m2 = (gmatch (srest, se, prest, pe, flags) == 0) ||
		      (s != srest && gmatch (srest, se, p - 1, pe, flags) == 0);
	      if (m1 && m2)
		return (0);
	    }
	  if (pnext == prest)
	    break;
	}
      return (FNM_NOMATCH);

    case '?':		/* match zero or one of the patterns */
    case '@':		/* match exactly one of the patterns */
      /* If we can get away with no matches, don't even bother.  Just
	 call gmatch on the rest of the pattern and return success if
	 it succeeds. */
      if (xc == '?' && (gmatch (s, se, prest, pe, flags) == 0))
	return 0;

      /* OK, we have to do this the hard way.  First, we see if one of
	 the subpatterns matches, then, if it does, we try to match the
	 rest of the string. */
      for (psub = p + 1; ; psub = pnext)
	{
	  pnext = patscan (psub, pe, '|');
	  srest = (prest == pe) ? se : s;
	  for ( ; srest <= se; srest++)
	    {
	      if (gmatch (s, srest, psub, pnext - 1, flags) == 0 &&
		  gmatch (srest, se, prest, pe, flags) == 0)
		return (0);
	    }
	  if (pnext == prest)
	    break;
	}
      return (FNM_NOMATCH);

    case '!':		/* match anything *except* one of the patterns */
      for (srest = s; srest <= se; srest++)
	{
	  m1 = 0;
	  for (psub = p + 1; ; psub = pnext)
	    {
	      pnext = patscan (psub, pe, '|');
	      /* If one of the patterns matches, just bail immediately. */
	      if (m1 = (gmatch (s, srest, psub, pnext - 1, flags) == 0))
		break;
	      if (pnext == prest)
		break;
	    }
	  if (m1 == 0 && gmatch (srest, se, prest, pe, flags) == 0)
	    return (0);
	}
      return (FNM_NOMATCH);
    }

  return (FNM_NOMATCH);
}
#endif /* EXTENDED_GLOB */

#ifdef TEST
main (c, v)
     int c;
     char **v;
{
  char *string, *pat;

  string = v[1];
  pat = v[2];

  if (fnmatch (pat, string, 0) == 0)
    {
      printf ("%s matches %s\n", string, pat);
      exit (0);
    }
  else
    {
      printf ("%s does not match %s\n", string, pat);
      exit (1);
    }
}
#endif

#endif

