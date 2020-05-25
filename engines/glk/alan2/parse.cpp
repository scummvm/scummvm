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

#include "glk/alan2/alan2.h"
#include "glk/alan2/debug.h"
#include "glk/alan2/exe.h"
#include "glk/alan2/glkio.h"
#include "glk/alan2/inter.h"
#include "glk/alan2/main.h"
#include "glk/alan2/params.h"
#include "glk/alan2/parse.h"
#include "glk/alan2/types.h"

namespace Glk {
namespace Alan2 {

#define LISTLEN 100


/* PUBLIC DATA */

Aword wrds[LISTLEN / 2] = {EOD};  // List of parsed words
int wrdidx;				// and an index into it

Boolean plural = FALSE;


/* Syntax Parameters */
int paramidx;           /* Index in params */
ParamElem *params;      /* List of params */
static ParamElem *pparams;  /* Previous parameter list */
static ParamElem *mlst;     /* Multiple objects list */
static ParamElem *pmlst;    /* Previous multiple list */

/* Literals */
LitElem litValues[MAXPARAMS + 1];
int litCount;

/* What did the user say? */
int vrbwrd;         /* The word he used */
int vrbcode;            /* The code for that verb */


/*----------------------------------------------------------------------*\

  SCAN DATA & PROCEDURES

  All procedures for getting a command and turning it into a list of
  dictionary entries are placed here.

  buf
  unknown()
  lookup()
  token
  agetline()
  scan()

\*----------------------------------------------------------------------*/


/* PRIVATE DATA */

static char buf[LISTLEN + 1]; /* The input buffer */
static char isobuf[LISTLEN + 1]; /* The input buffer in ISO */


static Boolean eol = TRUE;  /* Looking at End of line? Yes, initially */


static void unknown(CONTEXT, char token[]) {
	char *str = (char *)allocate((int)strlen(token) + 4);

	str[0] = '\'';
	strcpy(&str[1], token);
	strcat(str, "'?");
	output(str);
	free(str);
	eol = TRUE;
	CALL1(error, M_UNKNOWN_WORD)
}


static char *token;


static int lookup(CONTEXT, char wrd[]) {
	int i;

	for (i = 0; !endOfTable(&dict[i]); i++) {
		if (strcmp(wrd, (char *) addrTo(dict[i].wrd)) == 0)
			return (i);
	}
	R0CALL1(unknown, wrd)
	return EOD;
}

/* IN - The string to convert to a number */
static int number(char tok[]) {
	int i;

	(void)sscanf(tok, "%d", &i);
	return i;
}

static char *gettoken(char *tokBuf) {
	static char *marker;
	static char oldch;

	if (tokBuf == NULL)
		*marker = oldch;
	else
		marker = tokBuf;
	while (*marker != '\0' && isSpace(*marker) && *marker != '\n') marker++;
	tokBuf = marker;
	if (isISOLetter(*marker))
		while (*marker && (isISOLetter(*marker) || Common::isDigit(*marker) || *marker == '\'')) marker++;
	else if (Common::isDigit(*marker))
		while (Common::isDigit(*marker)) marker++;
	else if (*marker == '\"') {
		marker++;
		while (*marker != '\"') marker++;
		marker++;
	} else if (*marker == '\0' || *marker == '\n')
		return NULL;
	else
		marker++;
	oldch = *marker;
	*marker = '\0';
	return tokBuf;
}

static void agetline(CONTEXT) {
	para();
	do {
		statusline();

		printf("> ");
		if (logflg)
			fprintf(logfil, "> ");

		if (!readline(buf)) {
			if (g_vm->shouldQuit())
				return;

			newline();
			CALL0(quit)
		}

		anyOutput = FALSE;
		if (logflg)
			fprintf(logfil, "%s\n", buf);
		strcpy(isobuf, buf);

		token = gettoken(isobuf);
		if (token != NULL && strcmp("debug", token) == 0 && header->debug) {
			dbgflg = TRUE;
			debug();
			token = NULL;
		}
	} while (token == NULL);
	eol = FALSE;
	lin = 1;
}

static void scan(CONTEXT) {
	int i;
	int w;
	char *str;

	CALL0(agetline)
	if (g_vm->shouldQuit())
		return;

	wrds[0] = 0;
	for (i = 0; i < litCount; i++)
		if (litValues[i].type == TYPSTR && litValues[i].value != 0)
			free((char *) litValues[i].value);
	i = 0;
	litCount = 0;
	do {
		if (isISOLetter(token[0])) {
			(void) stringLower(token);
			FUNC1(lookup, w, token);

			if (!isNoise(w))
				wrds[i++] = w;
		} else if (Common::isDigit(token[0])) {
			if (litCount > MAXPARAMS)
				syserr("Too many parameters.");
			wrds[i++] = dictsize + litCount; /* Word outside dictionary = literal */
			litValues[litCount].type = TYPNUM;
			litValues[litCount++].value = number(token);
		} else if (token[0] == '\"') {
			if (litCount > MAXPARAMS)
				syserr("Too many parameters.");
			wrds[i++] = dictsize + litCount; /* Word outside dictionary = literal */
			litValues[litCount].type = TYPSTR;
			/* Remove the string quotes while copying */
			str = scumm_strdup(&token[1]);
			str[strlen(token) - 2] = '\0';
			litValues[litCount++].value = (Aptr) str;
		} else if (token[0] == ',') {
			wrds[i++] = conjWord;
		} else {
			CALL1(unknown, token)
		}
		wrds[i] = EOD;
		eol = (token = gettoken(NULL)) == NULL;
	} while (!eol);
}


/*----------------------------------------------------------------------*\

  PARSE DATA & PROCEDURES

  All procedures and data for getting a command and parsing it

  nonverb() - search for a non-verb command
  buildall()    - build a list of objects matching 'all'
  unambig() - match an unambigous object reference
  simple()  - match a simple verb command
  complex() - match a complex -"-
  tryMatch()- to match a verb command
  match()   - find the verb class (not used currently) and 'tryMatch()'

\*---------------------------------------------------------------------- */

static int allLength;       /* No. of objects matching 'all' */


static void nonverb(CONTEXT) {
	if (isDir(wrds[wrdidx])) {
		wrdidx++;
		if (wrds[wrdidx] != EOD && !isConj(wrds[wrdidx])) {
			CALL1(error, M_WHAT)
		} else {
			CALL1(go, dict[wrds[wrdidx - 1]].code)
		}
		if (wrds[wrdidx] != EOD)
			wrdidx++;
	} else {
		CALL1(error, M_WHAT)
	}
}

static void buildall(CONTEXT, ParamElem list[]) {
	int i = 0;
	Boolean found = FALSE;

	for (uint o = OBJMIN; o <= OBJMAX; o++)
		if (isHere(o)) {
			found = TRUE;
			list[i].code = o;
			list[i++].firstWord = EOD;
		}
	if (!found) {
		CALL1(error, M_WHAT_ALL)
	} else {
		list[i].code = EOD;
	}
}

static void unambig(CONTEXT, ParamElem plst[]) {
	int i;
	Boolean found = FALSE;    /* Adjective or noun found ? */
	static ParamElem *refs;   /* Entities referenced by word */
	static ParamElem *savlst; /* Saved list for backup at EOD */
	int firstWord, lastWord;  /* The words the player used */

	if (refs == NULL)
		refs = (ParamElem *)allocate((MAXENTITY + 1) * sizeof(ParamElem));

	if (savlst == NULL)
		savlst = (ParamElem *)allocate((MAXENTITY + 1) * sizeof(ParamElem));

	if (isLiteral(wrds[wrdidx])) {
		/* Transform the word into a reference to the literal value */
		plst[0].code = wrds[wrdidx++] - dictsize + LITMIN;
		plst[0].firstWord = EOD;    /* No words used! */
		plst[1].code = EOD;
		return;
	}

	plst[0].code = EOD;       /* Make empty */
	if (isIt(wrds[wrdidx])) {
		wrdidx++;
		/* Use last object in previous command! */
		for (i = lstlen(pparams) - 1; i >= 0 && (pparams[i].code == 0 || pparams[i].code >= LITMIN); i--);
		if (i < 0) {
			CALL1(error, M_WHAT_IT)
		}
		if (!isHere(pparams[i].code)) {
			params[0].code = pparams[i].code;
			params[0].firstWord = EOD;
			params[1].code = EOD;
			CALL1(error, M_NO_SUCH)
		}
		plst[0] = pparams[i];
		plst[0].firstWord = EOD;    /* No words used! */
		plst[1].code = EOD;
		return;
	}

	firstWord = wrdidx;
	while (wrds[wrdidx] != EOD && isAdj(wrds[wrdidx])) {
		/* If this word can be a noun and there is no noun following break loop */
		if (isNoun(wrds[wrdidx]) && (wrds[wrdidx + 1] == EOD || !isNoun(wrds[wrdidx + 1])))
			break;
		cpyrefs(refs, (Aword *)addrTo(dict[wrds[wrdidx]].adjrefs));
		lstcpy(savlst, plst);   /* To save it for backtracking */
		if (found)
			isect(plst, refs);
		else {
			lstcpy(plst, refs);
			found = TRUE;
		}
		wrdidx++;
	}
	if (wrds[wrdidx] != EOD) {
		if (isNoun(wrds[wrdidx])) {
			cpyrefs(refs, (Aword *)addrTo(dict[wrds[wrdidx]].nounrefs));
			if (found)
				isect(plst, refs);
			else {
				lstcpy(plst, refs);
				found = TRUE;
			}
			wrdidx++;
		} else
			CALL1(error, M_NOUN)
	} else if (found) {
		if (isNoun(wrds[wrdidx - 1])) {
			/* Perhaps the last word was also a noun? */
			lstcpy(plst, savlst); /* Restore to before last adjective */
			cpyrefs(refs, (Aword *)addrTo(dict[wrds[wrdidx - 1]].nounrefs));
			if (plst[0].code == EOD)
				lstcpy(plst, refs);
			else
				isect(plst, refs);
		} else {
			CALL1(error, M_NOUN)
		}
	}
	lastWord = wrdidx - 1;

	/* Allow remote objects, but resolve ambiguities by presence */
	if (lstlen(plst) > 1) {
		for (i = 0; plst[i].code != EOD; i++)
			if (!isHere(plst[i].code))
				plst[i].code = 0;
		compact(plst);
	}

	if (lstlen(plst) > 1 || (found && lstlen(plst) == 0)) {
		params[0].code = 0;     /* Just make it anything != EOD */
		params[0].firstWord = firstWord; /* Remember words for errors below */
		params[0].lastWord = lastWord;
		params[1].code = EOD;   /* But be sure to terminate */
		if (lstlen(plst) > 1) {
			CALL1(error, M_WHICH_ONE)
		} else if (found && lstlen(plst) == 0) {
			CALL1(error, M_NO_SUCH)
		}
	} else {
		plst[0].firstWord = firstWord;
		plst[0].lastWord = lastWord;
	}
}

static void simple(CONTEXT, ParamElem olst[]) {
	static ParamElem *tlst = NULL;
	int savidx = wrdidx;
	Boolean savplur = FALSE;
	int i;

	if (tlst == NULL)
		tlst = (ParamElem *) allocate(sizeof(ParamElem) * (MAXENTITY + 1));
	tlst[0].code = EOD;

	for (;;) {
		if (isThem(wrds[wrdidx])) {
			plural = TRUE;
			for (i = 0; pmlst[i].code != EOD; i++)
				if (!isHere(pmlst[i].code))
					pmlst[i].code = 0;
			compact(pmlst);
			if (lstlen(pmlst) == 0) {
				CALL1(error, M_WHAT_THEM)
			}

			lstcpy(olst, pmlst);
			olst[0].firstWord = EOD;  /* No words used */
			wrdidx++;
		} else {
			// Look for unambigous noun phrase
			CALL1(unambig, olst)
			if (lstlen(olst) == 0) {  /* Failed! */
				lstcpy(olst, tlst);
				wrdidx = savidx;
				plural = savplur;
				return;
			}
		}
		mrglst(tlst, olst);
		if (wrds[wrdidx] != EOD
		        && (isConj(wrds[wrdidx]) &&
		            (isAdj(wrds[wrdidx + 1]) || isNoun(wrds[wrdidx + 1])))) {
			/* More parameters in a conjunction separated list ? */
			savplur = plural;
			savidx = wrdidx;
			wrdidx++;
			plural = TRUE;
		} else {
			lstcpy(olst, tlst);
			return;
		}
	}
}


/*----------------------------------------------------------------------

  complex()

  Above this procedure we can use the is* tests, but not below since
  they work on words. Below all is converted to indices into the
  entity tables. Particularly this goes for literals...

*/
static void complex(CONTEXT, ParamElem olst[]) {
	static ParamElem *alst = NULL;

	if (alst == NULL)
		alst = (ParamElem *) allocate((MAXENTITY + 1) * sizeof(ParamElem));

	if (isAll(wrds[wrdidx])) {
		plural = TRUE;
		// Build list of all objects
		CALL1(buildall, alst)
		wrdidx++;
		if (wrds[wrdidx] != EOD && isBut(wrds[wrdidx])) {
			wrdidx++;
			CALL1(simple, olst)
			if (lstlen(olst) == 0)
				CALL1(error, M_AFTER_BUT)
			sublst(alst, olst);
			if (lstlen(alst) == 0)
				CALL1(error, M_NOT_MUCH)
		}
		lstcpy(olst, alst);
		allLength = lstlen(olst);
	} else {
		// Look for simple noun group
		CALL1(simple, olst)
	}
}

static Boolean claCheck(ClaElem *cla /* IN - The cla elem to check */) {
	Boolean ok = FALSE;

	if ((cla->classes & (Aword)CLA_OBJ) != 0)
		ok = ok || isObj(params[cla->code - 1].code);
	if ((cla->classes & (Aword)CLA_CNT) != 0)
		ok = ok || isCnt(params[cla->code - 1].code);
	if ((cla->classes & (Aword)CLA_ACT) != 0)
		ok = ok || isAct(params[cla->code - 1].code);
	if ((cla->classes & (Aword)CLA_NUM) != 0)
		ok = ok || isNum(params[cla->code - 1].code);
	if ((cla->classes & (Aword)CLA_STR) != 0)
		ok = ok || isStr(params[cla->code - 1].code);
	if ((cla->classes & (Aword)CLA_COBJ) != 0)
		ok = ok || (isCnt(params[cla->code - 1].code) && isObj(params[cla->code - 1].code));
	if ((cla->classes & (Aword)CLA_CACT) != 0)
		ok = ok || (isCnt(params[cla->code - 1].code) && isAct(params[cla->code - 1].code));
	return ok;
}


/*----------------------------------------------------------------------

  resolve()

  In case the syntax did not indicate omnipotent powers (allowed
  access to remote object), we need to remove non-present parameters

*/
static void resolve(CONTEXT, ParamElem plst[]) {
	int i;

	if (allLength > 0) return;    /* ALL has already done this */

	/* Resolve ambiguities by presence */
	for (i = 0; plst[i].code != EOD; i++)
		if (plst[i].code < LITMIN)  /* Literals are always 'here' */
			if (!isHere(plst[i].code)) {
				params[0] = plst[i];    /* Copy error param as first one for message */
				params[1].code = EOD;   /* But be sure to terminate */
				CALL1(error, M_NO_SUCH)
			}
}

/* OUT - List of params allowed by multiple */
static void tryMatch(CONTEXT, ParamElem matchLst[]) {
	ElmElem *elms;        /* Pointer to element list */
	StxElem *stx;         /* Pointer to syntax list */
	ClaElem *cla;         /* Pointer to class definitions */
	Boolean anyPlural = FALSE;    /* Any parameter that was plural? */
	int i, p;
	static ParamElem *tlst = NULL; /* List of params found by complex() */
	static Boolean *checked = NULL; /* Corresponding parameter checked? */

	if (tlst == NULL) {
		tlst = (ParamElem *) allocate((MAXENTITY + 1) * sizeof(ParamElem));
		checked = (Boolean *) allocate((MAXENTITY + 1) * sizeof(Boolean));
	}

	for (stx = stxs; !endOfTable(stx); stx++)
		if ((int)stx->code == vrbcode)
			break;
	if (endOfTable(stx)) {
		CALL1(error, M_WHAT)
	}

	elms = (ElmElem *) addrTo(stx->elms);

	while (TRUE) {
		/* End of input? */
		if (wrds[wrdidx] == EOD || isConj(wrds[wrdidx])) {
			while (!endOfTable(elms) && elms->code != EOS)
				elms++;
			if (endOfTable(elms)) {
				CALL1(error, M_WHAT)
			} else
				break;
		} else {
			/* A preposition? */
			if (isPrep(wrds[wrdidx])) {
				while (!endOfTable(elms) && elms->code != dict[wrds[wrdidx]].code)
					elms++;
				if (endOfTable(elms)) {
					CALL1(error, M_WHAT)
				} else
					wrdidx++;
			} else {
				/* Must be a parameter! */
				while (!endOfTable(elms) && elms->code != 0)
					elms++;
				if (endOfTable(elms)) {
					CALL1(error, M_WHAT)
				}
				/* Get it! */
				plural = FALSE;
				CALL1(complex, tlst)
				if (lstlen(tlst) == 0) {
					/* No object!? */
					CALL1(error, M_WHAT)
				}
				/* Omnipotent parameter? */
				if ((elms->flags & OMNIBIT) == 0) {
					/* If its not an omnipotent parameter, resolve by presence */
					CALL1(resolve, tlst)
				}
				if (plural) {
					/* Allowed multiple? */
					if ((elms->flags & MULTIPLEBIT) == 0) {
						CALL1(error, M_MULTIPLE)
					} else {
						/*
						   Mark this as the multiple position in which to insert
						   actual parameter values later
						 */
						params[paramidx++].code = 0;
						lstcpy(matchLst, tlst);
						anyPlural = TRUE;
					}
				} else
					params[paramidx++] = tlst[0];
				params[paramidx].code = EOD;
			}
			elms = (ElmElem *) addrTo(elms->next);
		}
	}

	/* Now perform class checks */
	if (elms->next == 0) {
		/* No verb code, verb not declared! */
		CALL1(error, M_CANT0)
	}

	for (p = 0; params[p].code != EOD; p++) /* Mark all parameters unchecked */
		checked[p] = FALSE;
	for (cla = (ClaElem *) addrTo(elms->next); !endOfTable(cla); cla++) {
		if (params[cla->code - 1].code == 0) {
			/* This was a multiple parameter, so check all and remove failing */
			for (i = 0; matchLst[i].code != EOD; i++) {
				params[cla->code - 1] = matchLst[i];
				if (!claCheck(cla)) {
					/* Multiple could be both an explicit list of params and an ALL */
					if (allLength == 0) {
						char marker[80];
						/*
						   It wasn't ALL, we need to say something about it, so
						   prepare a printout with $1/2/3
						 */
						sprintf(marker, "($%ld)", (unsigned long) cla->code);
						output(marker);
						interpret(cla->stms);
						para();
					}
					matchLst[i].code = 0;   /* In any case remove it from the list */
				}
			}
			params[cla->code - 1].code = 0;
		} else {
			if (!claCheck(cla)) {
				/* Return to player without saying anything */
				interpret(cla->stms);
				CALL1(error, MSGMAX)
			}
		}
		checked[cla->code - 1] = TRUE; /* Remember that it's already checked */
	}
	/* Now check the rest of the parameters, must be objects */
	for (p = 0; params[p].code != EOD; p++)
		if (!checked[p]) {
			if (params[p].code == 0) {
				/* This was a multiple parameter, check all and remove failing */
				for (i = 0; matchLst[i].code != EOD; i++)
					if (matchLst[i].code != 0) /* Skip any empty slots */
						if (!isObj(matchLst[i].code))
							matchLst[i].code = 0;
			} else if (!isObj(params[p].code)) {
				CALL1(error, M_CANT0)
			}
		}

	/* Set verb code */
	cur.vrb = ((Aword *) cla)[1]; /* Take first word after end of table! */

	/* Finally, if ALL was used, try to find out what was applicable */
	if (allLength > 0) {
		for (p = 0; params[p].code != 0; p++); /* Find multiple marker */
		for (i = 0; i < allLength; i++) {
			if (matchLst[i].code != 0) {  /* Already empty? */
				params[p] = matchLst[i];
				if (!possible())
					matchLst[i].code = 0; /* Remove this from list */
			}
		}
		params[p].code = 0;     /* Restore multiple marker */
		compact(matchLst);
		if (lstlen(matchLst) == 0) {
			params[0].code = EOD;
			CALL1(error, M_WHAT_ALL)
		}
	} else if (anyPlural) {
		compact(matchLst);
		if (lstlen(matchLst) == 0)
			/* If there where multiple parameters but non left, exit without a */
			/* word, assuming we have already said enough */
			CALL1(error, MSGMAX)
	}
	plural = anyPlural;       /* Remember that we found plural objects */
}

/* OUT - List of params allowed by multiple */
static void match(CONTEXT, ParamElem *matchLst) {
	/* ... to understand what he said */
	CALL1(tryMatch, matchLst)
	if (wrds[wrdidx] != EOD && !isConj(wrds[wrdidx])) {
		CALL1(error, M_WHAT)
	}
	if (wrds[wrdidx] != EOD)  /* More on this line? */
		wrdidx++;           /* If so skip the AND */
}

void parse(CONTEXT) {
	if (mlst == NULL) {       /* Allocate large enough paramlists */
		mlst = (ParamElem *) allocate(sizeof(ParamElem) * (MAXENTITY + 1));
		mlst[0].code = EOD;
		pmlst = (ParamElem *) allocate(sizeof(ParamElem) * (MAXENTITY + 1));
		params = (ParamElem *) allocate(sizeof(ParamElem) * (MAXENTITY + 1));
		params[0].code = EOD;
		pparams = (ParamElem *) allocate(sizeof(ParamElem) * (MAXENTITY + 1));
	}

	if (wrds[wrdidx] == EOD) {
		wrdidx = 0;
		CALL0(scan)

		if (g_vm->shouldQuit())
			return;
	} else if (anyOutput)
		para();

	allLength = 0;
	paramidx = 0;
	lstcpy(pparams, params);
	params[0].code = EOD;
	lstcpy(pmlst, mlst);
	mlst[0].code = EOD;
	if (isVerb(wrds[wrdidx])) {
		vrbwrd = wrds[wrdidx];
		vrbcode = dict[vrbwrd].code;
		wrdidx++;
		CALL1(match, mlst)
		/* mlst contains possible multiple params */
		CALL1(action, mlst)
	} else {
		params[0].code = EOD;
		pmlst[0].code = EOD;
		CALL0(nonverb)
	}
}

} // End of namespace Alan2
} // End of namespace Glk
