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

#include "common/stack.h"
#include "glk/alan2/alan2.h"
#include "glk/alan2/execute.h"
#include "glk/alan2/interpreter.h"
#include "glk/alan2/parse.h"
#include "glk/alan2/types.h"
#include "glk/alan2/util.h"
#include "common/debug.h"
#include "common/file.h"
#include "decode.h"

namespace Glk {
namespace Alan2 {
	
uint32 litCount = 0;

// All procedures for getting a command and turning it into a list of
// dictionary entries are placed here.

Parser::Parser() {
	wrds[0] = EOF;
	plural = false;

	// TODO
}

void Parser::unknown(char *inputStr) {
	Common::String str = Common::String::format("'%s'?", inputStr);

	// TODO
#if 0
#if ISO == 0
  fromIso(str, str);
#endif
#endif

	_vm->output(str);
	eol = true;
	_vm->printError(M_UNKNOWN_WORD);
}

int Parser::lookup(char *wrd) {
#if 0
	for (int i = 0; !endOfTable(&dict[i]); i++) {
		if (strcmp(wrd, (char *)addrTo(dict[i].wrd)) == 0)
			return i;
	}
#endif

	unknown(wrd);
	return EOF;
}

char *Parser::gettoken(char *tokenBuffer) {
	static char *marker;
	static char oldch;

	if (tokenBuffer == NULL)
		*marker = oldch;
	else
		marker = tokenBuffer;

	while (*marker != '\0' && Common::isSpace(*marker) && *marker != '\n') marker++;
	tokenBuffer = marker;

	if (Common::isAlpha(*marker))
		while (*marker && (Common::isAlpha(*marker) || Common::isDigit(*marker) || *marker == '\'')) marker++;
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
	
	return tokenBuffer;
}

void Parser::agetline() {
	static char buf[LISTLEN + 1];	// The input buffer
	static char isobuf[LISTLEN + 1];	// The input buffer in ISO

	_vm->paragraph();

	// TODO
#if 0

	do {
#if defined(HAVE_ANSI) || defined(GLK)
		statusline();
#endif
		debug("> ");

#if 0
		if (logflg)
			fprintf(logfil, "> ");
#endif

#ifdef USE_READLINE
		if (!readline(buf)) {
			newline();
			quit();
		}
#else
		if (fgets(buf, LISTLEN, stdin) == NULL) {
			newline();
			quit();
		}
#endif

		getPageSize();
		anyOutput = FALSE;

		if (logflg)
			fprintf(logfil, "%s\n", buf);

#if ISO == 0
		toIso(isobuf, buf, NATIVECHARSET);
#else
		strcpy(isobuf, buf);
#endif

		token = gettoken(isobuf);

		if (token != NULL && strcmp("debug", token) == 0 && _vm->header->debug) {
			dbgflg = true;
			debug();
			token = NULL;
		}
	} while (token == NULL);

  eol = false;
  lin = 1;

#endif
}

void Parser::scan() {
	int i;
	int w;
	char *str;

	agetline();
	wrds[0] = 0;

	for (i = 0; i < litCount; i++)
		if (litValues[i].type == TYPSTR && litValues[i].value != 0)
			free((char *) litValues[i].value);

	litCount = 0;

	do {
		if (Common::isAlpha(token[0])) {
			Common::String tmp = token;
			tmp.toLowercase();
			strcpy(token, tmp.c_str());

			w = lookup(token);

			// TODO
			//if (!isNoise(w))
			//	wrds[i++] = w;
		} else if (Common::isDigit(token[0])) {
			if (litCount > MAXPARAMS)
				error("Too many parameters.");

			wrds[i++] = dictsize + litCount; // Word outside dictionary = literal
			litValues[litCount].type = TYPNUM;
			litValues[litCount++].value = atoi(token);
		} else if (token[0] == '\"') {
			if (litCount > MAXPARAMS)
				error("Too many parameters.");
  
			wrds[i++] = dictsize + litCount; // Word outside dictionary = literal
			litValues[litCount].type = TYPSTR;

			// Remove the string quotes while copying
			Common::String tmp = token;
			tmp.deleteChar(0);
			tmp.deleteLastChar();
			strcpy(str, tmp.c_str());

			litValues[litCount++].value = (Aptr) str;
		} else if (token[0] == ',') {
			//wrds[i++] = conjWord;		// TODO
		} else
			unknown(token);

		wrds[i] = EOF;
		eol = (token = gettoken(NULL)) == NULL;
	} while (!eol);
}

void Parser::nonverb() {
	if (isDir(wrds[wrdidx])) {
		wrdidx++;
		if (wrds[wrdidx] != EOF && !isConj(wrds[wrdidx]))
			_vm->printError(M_WHAT);
// TODO
#if 0
		else
			go(dict[wrds[wrdidx-1]].code);
#endif

		if (wrds[wrdidx] != EOF)
			wrdidx++;
	} else
		_vm->printError(M_WHAT);
}

Abool Parser::objhere(Aword obj) {
	if (isCnt(objs[obj - OBJMIN].loc)) {	// In something?
		if (isObj(objs[obj - OBJMIN].loc) || isAct(objs[obj - OBJMIN].loc))
			return(isHere(objs[obj - OBJMIN].loc));
// TODO
#if 0
		else // If the container wasn't anywhere, assume where HERO is!
			return(where(HERO) == _vm->cur.loc);
#endif

	} else
		return(objs[obj - OBJMIN].loc == _vm->cur.loc);
}


Aword Parser::acthere(Aword act) {
	return(acts[act - ACTMIN].loc == _vm->cur.loc);
}

Abool Parser::isHere(Aword id) {
	if (isObj(id))
		return objhere(id);
	else if (isAct(id))
		return acthere(id);
	else
		error("Can't HERE item (%ld).", (unsigned long)id);
}

// ----------------------------------------------------------------------------

void Parser::buildall(ParamElem list[]) {
	int o, i = 0;
	bool found = false;
  
	for (o = OBJMIN; o <= OBJMAX; o++) {
		if (isHere(o)) {
			found = true;
			list[i].code = o;
			list[i++].firstWord = EOF;
		}
	}

	if (!found)
		_vm->printError(M_WHAT_ALL);
	else
		list[i].code = EOF;
}

void Parser::listCopy(ParamElem a[], ParamElem b[]) {
	int i;

	for (i = 0; b[i].code != EOF; i++)
		a[i] = b[i];

	a[i].code = EOF;
}

bool Parser::listContains(ParamElem l[], Aword e) {
	int i;

	for (i = 0; l[i].code != EOF && l[i].code != e; i++);

	return (l[i].code == e);
}

void Parser::listIntersection(ParamElem a[], ParamElem b[]) {
	int i, last = 0;

	for (i = 0; a[i].code != EOF; i++)
		if (listContains(b, a[i].code))
			a[last++] = a[i];

	a[last].code = EOF;
}

void Parser::listCopyFromDictionary(ParamElem p[], Aword r[]) {
	int i;

	for (i = 0; r[i] != EOF; i++) {
		p[i].code = r[i];
		p[i].firstWord = EOF;
	}

	p[i].code = EOF;
}

int Parser::listLength(ParamElem a[]) {
	int i = 0;

	while (a[i].code != EOF)
		i++;

	return (i);
}

void Parser::listCompact(ParamElem a[]) {
	int i, j;

	for (i = 0, j = 0; a[j].code != EOF; j++)
		if (a[j].code != 0)
			a[i++] = a[j];

	a[i].code = EOF;
}

void Parser::listMerge(ParamElem a[], ParamElem b[]) {
	int i, last;

	for (last = 0; a[last].code != EOF; last++); // Find end of list

	for (i = 0; b[i].code != EOF; i++) {
		if (!listContains(a, b[i].code)) {
			a[last++] = b[i];
			a[last].code = EOF;
		}
	}
}

void Parser::listSubtract(ParamElem a[], ParamElem b[]) {
	for (int i = 0; a[i].code != EOF; i++)
		if (listContains(b, a[i].code))
			a[i].code = 0;		// Mark empty

	listCompact(a);
}

void Parser::unambig(ParamElem plst[]) {
	int i;
	bool found = false;		// Adjective or noun found?
	static ParamElem *refs;	// Entities referenced by word
	static ParamElem *savlst;	// Saved list for backup at EOF
	int firstWord, lastWord;	// The words the player used

	if (refs == NULL)
		refs = new ParamElem[MAXENTITY + 1];

	if (savlst == NULL)
		savlst = new ParamElem[MAXENTITY + 1];

	if (isLiteral(wrds[wrdidx])) {
		// Transform the word into a reference to the literal value
		plst[0].code = wrds[wrdidx++] - dictsize + LITMIN;
		plst[0].firstWord = EOF;	// No words used!
		plst[1].code = EOF;
		return;
	}

	plst[0].code = EOF;		// Make empty

	if (isIt(wrds[wrdidx])) {
		wrdidx++;
		
		// Use last object in previous command!
		for (i = listLength(pparams)-1; i >= 0 && (pparams[i].code == 0 || pparams[i].code >= LITMIN); i--);
		
		if (i < 0)
			_vm->printError(M_WHAT_IT);

		if (!isHere(pparams[i].code)) {
			params[0].code = pparams[i].code;
			params[0].firstWord = EOF;
			params[1].code = EOF;
			_vm->printError(M_NO_SUCH);
		}

		plst[0] = pparams[i];
		plst[0].firstWord = EOF;	// No words used!
		plst[1].code = EOF;
		return;
	}

	firstWord = wrdidx;

	while (wrds[wrdidx] != EOF && isAdj(wrds[wrdidx])) {
		// If this word can be a noun and there is no noun following break loop
		if (isNoun(wrds[wrdidx]) && (wrds[wrdidx+1] == EOF || !isNoun(wrds[wrdidx+1])))
			break;

		listCopyFromDictionary(refs, (Aword *)addrTo(dict[wrds[wrdidx]].adjrefs));
		listCopy(savlst, plst);	// To save it for backtracking

		if (found)
			listIntersection(plst, refs);
		else {
			listCopy(plst, refs);
			found = true;
		}

		wrdidx++;
	}

	if (wrds[wrdidx] != EOF) {
		if (isNoun(wrds[wrdidx])) {
			listCopyFromDictionary(refs, (Aword *)addrTo(dict[wrds[wrdidx]].nounrefs));
			if (found)
				listIntersection(plst, refs);
			else {
				listCopy(plst, refs);
				found = true;
			}

			wrdidx++;
		} else
			_vm->printError(M_NOUN);
	} else if (found) {
		if (isNoun(wrds[wrdidx-1])) {
			// Perhaps the last word was also a noun?
			listCopy(plst, savlst);	// Restore to before last adjective
			listCopyFromDictionary(refs, (Aword *)addrTo(dict[wrds[wrdidx-1]].nounrefs));
			
			if (plst[0].code == EOF)
				listCopy(plst, refs);
			else
				listIntersection(plst, refs);
		} else
			_vm->printError(M_NOUN);
	}

	lastWord = wrdidx - 1;

	// Allow remote objects, but resolve ambiguities by presence
	if (listLength(plst) > 1) {
		for (i=0; plst[i].code != EOF; i++)
			if (!isHere(plst[i].code))
				plst[i].code = 0;

		listCompact(plst);
	}

	if (listLength(plst) > 1 || (found && listLength(plst) == 0)) {
		params[0].code = 0;		/* Just make it anything != EOF */
		params[0].firstWord = firstWord; /* Remember words for errors below */
		params[0].lastWord = lastWord;
		params[1].code = EOF;	/* But be sure to terminate */

		if (listLength(plst) > 1)
			_vm->printError(M_WHICH_ONE);
		else if (found && listLength(plst) == 0)
			_vm->printError(M_NO_SUCH);
	} else {
		plst[0].firstWord = firstWord;
		plst[0].lastWord = lastWord;
	}
}

void Parser::simple(ParamElem olst[]) {
	static ParamElem *tlst = NULL;
	int savidx = wrdidx;
	bool savplur = false;
	int i;

	if (tlst == NULL)
		tlst = new ParamElem[MAXENTITY + 1];

	tlst[0].code = EOF;

	for (;;) {
		if (isThem(wrds[wrdidx])) {
			plural = true;

			for (i = 0; pmlst[i].code != EOF; i++)
				if (!isHere(pmlst[i].code))
					pmlst[i].code = 0;

			listCompact(pmlst);

			if (listLength(pmlst) == 0)
				_vm->printError(M_WHAT_THEM);

			listCopy(olst, pmlst);
			olst[0].firstWord = EOF;	// No words used
			wrdidx++;
		} else {
			unambig(olst);		// Look for unambigous noun phrase

			if (listLength(olst) == 0) {	// Failed!
				listCopy(olst, tlst);
				wrdidx = savidx;
				plural = savplur;
				return;
			}
		}

		listMerge(tlst, olst);

		if (wrds[wrdidx] != EOF
			&& (isConj(wrds[wrdidx])
			&& (isAdj(wrds[wrdidx+1]) || isNoun(wrds[wrdidx+1])))) {
			// More parameters in a conjunction separated list ?
			savplur = plural;
			savidx = wrdidx;
			wrdidx++;
			plural = true;
		} else {
			listCopy(olst, tlst);
			return;
		}
	}
}

void Parser::complex(ParamElem olst[]) {
	// Above this procedure we can use the is* tests, but not below since
	// they work on words.Below all is converted to indices into the
	// entity tables.Particularly this goes for literals...

	static ParamElem *alst = NULL;

	if (alst == NULL)
		alst = new ParamElem[MAXENTITY + 1];

	if (isAll(wrds[wrdidx])) {
		plural = true;
		buildall(alst);		// Build list of all objects
		wrdidx++;
		if (wrds[wrdidx] != EOF && isBut(wrds[wrdidx])) {
			wrdidx++;
			simple(olst);

			if (listLength(olst) == 0)
				_vm->printError(M_AFTER_BUT);

			listSubtract(alst, olst);
			if (listLength(alst) == 0)
				_vm->printError(M_NOT_MUCH);
		}

		listCopy(olst, alst);
		allLength = listLength(olst);
	} else
		simple(olst);		// Look for simple noun group
}

bool Parser::claCheck(ClaElem *cla) {
	bool ok = false;

	if ((cla->classes&(Aword)CLA_OBJ) != 0)
		ok = ok || isObj(params[cla->code-1].code);
	if ((cla->classes&(Aword)CLA_CNT) != 0)
		ok = ok || isCnt(params[cla->code-1].code);
	if ((cla->classes&(Aword)CLA_ACT) != 0)
		ok = ok || isAct(params[cla->code-1].code);
	if ((cla->classes&(Aword)CLA_NUM) != 0)
		ok = ok || isNum(params[cla->code-1].code);
	if ((cla->classes&(Aword)CLA_STR) != 0)
		ok = ok || isStr(params[cla->code-1].code);
	if ((cla->classes&(Aword)CLA_COBJ) != 0)
		ok = ok || (isCnt(params[cla->code-1].code) && isObj(params[cla->code-1].code));
	if ((cla->classes&(Aword)CLA_CACT) != 0)
		ok = ok || (isCnt(params[cla->code-1].code) && isAct(params[cla->code-1].code));

	return ok;
}

void Parser::resolve(ParamElem plst[]) {
	if (allLength > 0)
		return;	// ALL has already done this

	// Resolve ambiguities by presence
	for (int i = 0; plst[i].code != EOF; i++)  {
		if (plst[i].code < LITMIN)	// Literals are always 'here'
			if (!isHere(plst[i].code)) {
				params[0] = plst[i];	// Copy error param as first one for message
				params[1].code = EOF;	// But be sure to terminate
				_vm->printError(M_NO_SUCH);
			}
	}
}

bool Parser::endOfTable(StxElem *addr) {
	Aword *x = (Aword *)addr;
	return *x == EOF;
}

bool Parser::endOfTable(ElmElem *addr) {
	Aword *x = (Aword *)addr;
	return *x == EOF;
}

bool Parser::endOfTable(ClaElem *addr) {
	Aword *x = (Aword *)addr;
	return *x == EOF;
}

bool Parser::endOfTable(VrbElem *addr) {
	Aword *x = (Aword *)addr;
	return *x == EOF;
}

bool Parser::endOfTable(AltElem *addr) {
	Aword *x = (Aword *)addr;
	return *x == EOF;
}

bool Parser::endOfTable(ChkElem *addr) {
	Aword *x = (Aword *)addr;
	return *x == EOF;
}

AltElem *Parser::findalt(Aword vrbsadr, Aword param) {
	VrbElem *vrb;
	AltElem *alt;

	if (vrbsadr == 0)
		return NULL;

	for (vrb = (VrbElem *)addrTo(vrbsadr); !endOfTable(vrb); vrb++) {
		if (vrb->code == _vm->cur.vrb) {
			for (alt = (AltElem *)addrTo(vrb->alts); !endOfTable(alt); alt++)
				if (alt->param == param || alt->param == 0)
					return alt;
			return NULL;
		}
	}

	return NULL;
}

bool Parser::trycheck(Aaddr adr, bool act) {
	ChkElem *chk = (ChkElem *)addrTo(adr);

	if (chk->exp == 0) {
		_vm->_interpreter->interpret(chk->stms);
		return false;
	} else {
		while (!endOfTable(chk)) {
			_vm->_interpreter->interpret(chk->exp);
			if (!(Abool)_vm->_stack->pop()) {
				if (act)
					_vm->_interpreter->interpret(chk->stms);
				return false;
			}
			chk++;
		}
		return true;
	}
}

bool Parser::possible() {
	AltElem *alt[MAXPARAMS + 2];	// List of alt-pointers, one for each param
	int i;			// Parameter index

	_vm->fail = false;
	alt[0] = findalt(_vm->header->vrbs, 0);

	// Perform global checks
	if (alt[0] != 0 && alt[0]->checks != 0) {
		if (!trycheck(alt[0]->checks, false))
			return false;
		if (_vm->fail)
			return false;
	}

	// Now CHECKs in this location
	alt[1] = findalt(locs[_vm->cur.loc - LOCMIN].vrbs, 0);
	if (alt[1] != 0 && alt[1]->checks != 0)
		if (!trycheck(alt[1]->checks, false))
			return false;

	for (i = 0; params[i].code != EOF; i++) {
		alt[i + 2] = findalt(objs[params[i].code - OBJMIN].vrbs, i + 1);
		// CHECKs in a possible parameter
		if (alt[i + 2] != 0 && alt[i + 2]->checks != 0)
			if (!trycheck(alt[i + 2]->checks, false))
				return false;
	}

	for (i = 0; i < 2 || params[i - 2].code != EOF; i++)
		if (alt[i] != 0 && alt[i]->action != 0)
			break;
	if (i >= 2 && params[i - 2].code == EOF)
		// Didn't find any code for this verb/object combination
		return false;
	else
		return true;
}

void Parser::tryMatch(ParamElem mlst[]) {
	ElmElem *elms;		// Pointer to element list
	StxElem *stx;			// Pointer to syntax list
	ClaElem *cla;			// Pointer to class definitions
	bool anyPlural = false;	// Any parameter that was plural?
	int i, p;
	static ParamElem *tlst = NULL; // List of params found by complex()
	static bool *checked = NULL; // Corresponding parameter checked?

	if (tlst == NULL) {
		tlst = new ParamElem[MAXENTITY + 1];
		checked = new bool[MAXENTITY + 1];
	}

	for (stx = stxs; !endOfTable(stx); stx++)
		if (stx->code == vrbcode)
			break;

	if (endOfTable(stx))
		_vm->printError(M_WHAT);

	elms = (ElmElem *) addrTo(stx->elms);

	while (true) {
		// End of input?
		if (wrds[wrdidx] == EOF || isConj(wrds[wrdidx])) {
			while (!endOfTable(elms) && elms->code != EOS)
				elms++;

				if (endOfTable(elms))
					_vm->printError(M_WHAT);
				else
					break;
		} else {
			// A preposition?
			if (isPrep(wrds[wrdidx])) {
				while (!endOfTable(elms) && elms->code != dict[wrds[wrdidx]].code)
					elms++;

				if (endOfTable(elms))
					_vm->printError(M_WHAT);
				else
					wrdidx++;
			} else {
				// Must be a parameter!
				while (!endOfTable(elms) && elms->code != 0)
					elms++;

				if (endOfTable(elms))
					_vm->printError(M_WHAT);

				// Get it!
				plural = false;
				complex(tlst);

				if (listLength(tlst) == 0) // No object!?
					_vm->printError(M_WHAT);

				if ((elms->flags & OMNIBIT) == 0) // Omnipotent parameter?
					resolve(tlst);	// If its not an omnipotent parameter, resolve by presence

				if (plural) {
					if ((elms->flags & MULTIPLEBIT) == 0)	// Allowed multiple?
						_vm->printError(M_MULTIPLE);
					else {
						// Mark this as the multiple position in which to insert
						// actual parameter values later
						params[paramidx++].code = 0;
						listCopy(mlst, tlst);
						anyPlural = true;
					}
				} else
					params[paramidx++] = tlst[0];
				
				params[paramidx].code = EOF;
			}

			elms = (ElmElem *) addrTo(elms->next);
		}
	}
  
	// Now perform class checks
	if (elms->next == 0)	// No verb code, verb not declared!
		_vm->printError(M_CANT0);

	for (p = 0; params[p].code != EOF; p++) /* Mark all parameters unchecked */
		checked[p] = false;

	for (cla = (ClaElem *) addrTo(elms->next); !endOfTable(cla); cla++) {
		if (params[cla->code - 1].code == 0) {
			// This was a multiple parameter, so check all and remove failing
			for (i = 0; mlst[i].code != EOF; i++) {
				params[cla->code-1] = mlst[i];
				if (!claCheck(cla)) {
					// Multiple could be both an explicit list of params and an ALL
					if (allLength == 0) {
						char marker[80];
						// It wasn't ALL, we need to say something about it, so
						// prepare a printout with $1/2/3
						sprintf(marker, "($%ld)", (unsigned long) cla->code); 
						_vm->output(marker);
						_vm->_interpreter->interpret(cla->stms);
						_vm->paragraph();
					}

					mlst[i].code = 0;	  // In any case remove it from the list
				}
			}

			params[cla->code - 1].code = 0;
		} else {
			if (!claCheck(cla)) {
				_vm->_interpreter->interpret(cla->stms);
				_vm->printError(MSGMAX);		// Return to player without saying anything
			}
		}

		checked[cla->code - 1] = true; // Remember that it's already checked
	}

	// Now check the rest of the parameters, must be objects
	for (p = 0; params[p].code != EOF; p++) {
		if (!checked[p]) {
			if (params[p].code == 0) {
				// This was a multiple parameter, check all and remove failing
				for (i = 0; mlst[i].code != EOF; i++) {
					if (mlst[i].code != 0 && !isObj(mlst[i].code)) // Skip any empty slots
						mlst[i].code = 0;
				}
			} else if (!isObj(params[p].code))
				_vm->printError(M_CANT0);
		}
	}

	// Set verb code
	_vm->cur.vrb = ((Aword *) cla)[1];	// Take first word after end of table!

	// Finally, if ALL was used, try to find out what was applicable
	if (allLength > 0) {
		for (p = 0; params[p].code != 0; p++); // Find multiple marker

		for (i = 0; i < allLength; i++) {
			if (mlst[i].code != 0) {	// Already empty?
				params[p] = mlst[i];

				if (!possible())
					mlst[i].code = 0;	// Remove this from list
			}
		}

		params[p].code = 0;		// Restore multiple marker
		listCompact(mlst);

		if (listLength(mlst) == 0) {
			params[0].code = EOF;
			_vm->printError(M_WHAT_ALL);
		}
	} else if (anyPlural) {
		listCompact(mlst);

		// If there where multiple parameters but non left, exit without a
		// word, assuming we have already said enough
		if (listLength(mlst) == 0)
			_vm->printError(MSGMAX);
	}

	plural = anyPlural;		// Remember that we found plural objects
}

void Parser::match(ParamElem *mlst) {
	tryMatch(mlst);			// try to understand what the user said

	if (wrds[wrdidx] != EOF && !isConj(wrds[wrdidx]))
		_vm->printError(M_WHAT);
	if (wrds[wrdidx] != EOF)	// More on this line?
		wrdidx++;			// If so skip the AND
}

void Parser::action(ParamElem plst[]) {
	int i, mpos;
	char marker[10];

	if (plural) {
		// The code == 0 means this is a multiple position. We must loop
		// over this position (and replace it by each present in the plst)
		for (mpos = 0; params[mpos].code != 0; mpos++); // Find multiple position
		
		sprintf(marker, "($%d)", mpos + 1); // Prepare a printout with $1/2/3

		for (i = 0; plst[i].code != EOF; i++) {
			params[mpos] = plst[i];
			_vm->output(marker);
			//do_it();	// TODO

			if (plst[i + 1].code != EOF)
				_vm->paragraph();
		}

		params[mpos].code = 0;
	} //else // TODO
		//do_it();
}

void Parser::parse() {
	if (mlst == NULL) {		// Allocate large enough paramlists
		mlst = new ParamElem[MAXENTITY + 1];
		mlst[0].code = EOF;
		pmlst = new ParamElem[MAXENTITY + 1];
		params = new ParamElem[MAXENTITY + 1];
		params[0].code = EOF;
		pparams = new ParamElem[MAXENTITY + 1];
	}

	if (wrds[wrdidx] == EOF) {
		wrdidx = 0;
		scan();
	} else if (false/*anyOutput*/)	// TODO
		_vm->paragraph();

	allLength = 0;
	paramidx = 0;
	listCopy(pparams, params);
	params[0].code = EOF;
	listCopy(pmlst, mlst);
	mlst[0].code = EOF;

	if (isVerb(wrds[wrdidx])) {
		vrbwrd = wrds[wrdidx];
		vrbcode = dict[vrbwrd].code;
		wrdidx++;
		match(mlst);
		action(mlst);		// mlst contains possible multiple params
	} else {
		params[0].code = EOF;
		pmlst[0].code = EOF;
		nonverb();
	}
}

} // End of namespace Alan2
} // End of namespace Glk
