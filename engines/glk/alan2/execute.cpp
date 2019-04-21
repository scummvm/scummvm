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
#include "glk/alan2/decode.h"
#include "glk/alan2/execute.h"
#include "glk/alan2/interpreter.h"
#include "glk/alan2/saveload.h"
#include "glk/alan2/types.h"
#include "glk/alan2/util.h"
#include "common/debug.h"
#include "common/file.h"

namespace Glk {
namespace Alan2 {

#define WIDTH 80

// Is there an exit from one location to another ?
bool Execute::exitto(int to, int from) {
	if (_locs[from - LOCMIN].exts == 0)
		return false; // No exits

	for (ExtElem *ext = (ExtElem *)addrTo(_locs[from - LOCMIN].exts); !endOfTable(ext); ext++)
		if ((int)ext->next == to)
			return true;

	return false;
}

int Execute::count(int cnt) {
	int j = 0;

	for (uint i = OBJMIN; i <= OBJMAX; i++)
		if (in(i, cnt))
			j++;	// Then it's in this container also

	return j;
}

int Execute::sumAttributes(Aword atr, Aword cnt) {
	int sum = 0;

	for (uint i = OBJMIN; i <= OBJMAX; i++) {
		if (_objs[i - OBJMIN].loc == cnt) {	// Then it's in this container
			if (_objs[i - OBJMIN].cont != 0)	// This is also a container!
				sum = sum + sumAttributes(atr, i);
			sum = sum + attribute(i, atr);
		}
	}

	return sum;
}

bool Execute::checkContainerLimit(Aword cnt, Aword obj) {
	LimElem *lim;
	Aword props;

	_vm->fail = true;
	if (!isCnt(cnt))
		error("Checking limits for a non-container.");

	// Find the container properties
	if (isObj(cnt))
		props = _objs[cnt - OBJMIN].cont;
	else if (isAct(cnt))
		props = _acts[cnt - ACTMIN].cont;
	else
		props = cnt;


	if (_cnts[props - CNTMIN].lims != 0) { /* Any limits at all? */
		for (lim = (LimElem *)addrTo(_cnts[props - CNTMIN].lims); !endOfTable(lim); lim++) {
			if (lim->atr == 0) {
				if (count(cnt) >= (int)lim->val) {
					_vm->_interpreter->interpret(lim->stms);
					return true;		// Limit check failed
				}
			} else {
				if (sumAttributes(lim->atr, cnt) + attribute(obj, lim->atr) > lim->val) {
					_vm->_interpreter->interpret(lim->stms);
					return true;
				}
			}
		}
	}

	_vm->fail = false;

	return false;
}

void Execute::print(Aword fpos, Aword len) {
	char str[2 * WIDTH];			// String buffer
	int outlen = 0;					// Current output length
	int ch = 0;
	int i;
	long savfp = 0;					// Temporary saved text file position
	bool printFlag = false;			// Printing already?
	bool savedPrintFlag = printFlag;
	DecodeInfo *info = nullptr;		// Saved decoding info


	if (len == 0)
		return;

	if (isHere(HERO)) {           // Check if the player will see it
		if (printFlag) {            // Already printing?
			// Save current text file position and/or decoding info
			if (_vm->header->pack)
				info = _vm->_decode->pushDecode();
			else
				savfp = _vm->_txtFile->pos();
		}

		printFlag = true;           // We're printing now!
		_vm->_txtFile->seek(fpos, SEEK_CUR);	// Position to start of text
		if (_vm->header->pack)
			_vm->_decode->startDecoding();

		for (outlen = 0; outlen != (int)len; outlen = outlen + strlen(str)) {
			// Fill the buffer from the beginning
			for (i = 0; i <= WIDTH || (i > WIDTH && ch != ' '); i++) {
				if (outlen + i == (int)len)  // No more characters?
					break;
				if (_vm->header->pack)
					ch = _vm->_decode->decodeChar();
				else
					ch = _vm->_txtFile->readSByte();
				if (ch == EOFChar)      // Or end of text?
					break;
				str[i] = ch;
			}

			str[i] = '\0';

			// TODO
	/*
#if ISO == 0
		fromIso(str, str);
#endif
	*/
			_vm->output(str);
		}

		// And restore
		printFlag = savedPrintFlag;
		if (printFlag) {
			if (_vm->header->pack)
				_vm->_decode->popDecode(info);
			else
				_vm->_txtFile->seek(savfp, SEEK_CUR);
		}
	}
}

void Execute::sys(Aword fpos, Aword len) {
	char *command;

	getstr(fpos, len);            // Returns address to string on stack
	command = (char *)_vm->_stack->pop();

	warning("Request to execute system command %s", command);
	free(command);
}

void Execute::getstr(Aword fpos, Aword len) {
	char *buf = new char[len + 1];

	_vm->_stack->push((Aptr) buf);            // Push the address to the string
	_vm->_txtFile->seek(fpos, SEEK_CUR);	// Position to start of text
	if (_vm->header->pack)
		_vm->_decode->startDecoding();
	while (len--) {
		if (_vm->header->pack)
			*(buf++) = _vm->_decode->decodeChar();
		else
			*(buf++) = _vm->_txtFile->readSByte();
	}
	*buf = '\0';
}

void Execute::score(Aword sc) {
	char buf[80];

	if (sc == 0) {
		_vm->printMessage(M_SCORE1);
		sprintf(buf, "%d", _vm->cur.score);
		_vm->output(buf);
		_vm->printMessage(M_SCORE2);
		sprintf(buf, "%ld.", (unsigned long)_vm->header->maxscore);
		_vm->output(buf);
	} else {
		_vm->cur.score += _vm->scores[sc - 1];
		_vm->scores[sc - 1] = 0;
	}
}

void Execute::visits(Aword v) {
	_vm->cur.visits = v;
}

bool Execute::confirm(MsgKind msgno) {
	char buf[80];

	// This is a bit of a hack since we really want to compare the input,
	// it could be affirmative, but for now any input is NOT!
	_vm->printMessage(msgno);

	// TODO
#if 0
	//_vm->glk_request_line_event(_bottomWindow, buf, 80 - 1, 0);

#ifdef USE_READLINE
	if (!readline(buf)) return true;
#else
	if (gets(buf) == NULL) return true;
#endif

#endif
	_vm->col = 1;

	return (buf[0] == '\0');
}


void Execute::quit() {
	char buf[80];
//	char choices[10];

	_vm->paragraph();
	
	while (true) {
		_vm->col = 1;
		_vm->statusLine();
		_vm->printMessage(M_QUITACTION);

		// TODO
#if 0
#ifdef USE_READLINE
	if (!readline(buf)) terminate(0);
#else
	if (gets(buf) == NULL) terminate(0);
#endif
#endif

	if (strcmp(buf, "restart") == 0) {
		//longjmp(restart_label, true);	// TODO
	} else if (strcmp(buf, "restore") == 0) {
		_vm->_saveLoad->restore();
		return;
	} else if (strcmp(buf, "quit") == 0) {
		_vm->quitGame();
	}
  }
  error("Fallthrough in QUIT");
}

void Execute::restart() {
	_vm->paragraph();
	if (confirm(M_REALLY)) {
		//longjmp(restart_label, true);	// TODO
	} else
		return;

	error("Fallthrough in RESTART");
}

void Execute::eventchk() {
	while (etop != 0 && eventq[etop - 1].time == _vm->cur.tick) {
		etop--;
		if (isLoc(eventq[etop].where))
			_vm->cur.loc = eventq[etop].where;
		else
			_vm->cur.loc = where(eventq[etop].where);

		// TODO
#if 0
		if (trcflg) {
			debug("\n<EVENT %d (at ", eventq[etop].event);
			debugsay(_vm->cur.loc);
			debug("):>\n");
		}
#endif
		_vm->_interpreter->interpret(_vm->evts[eventq[etop].event - EVTMIN].code);
	}
}

void Execute::cancl(Aword evt) {
	int i;

	for (i = etop - 1; i >= 0; i--) {
		if (eventq[i].event == (int)evt) {
			while (i < etop - 1) {
				eventq[i].event = eventq[i + 1].event;
				eventq[i].time = eventq[i + 1].time;
				eventq[i].where = eventq[i + 1].where;
				i++;
			}

			etop--;
			return;
		}
	}
}

void Execute::schedule(Aword evt, Aword whr, Aword aft) {
	int i;
	int time;
  
	cancl(evt);
	// Check for overflow
	if (etop == N_EVTS)
		error("Out of event space.");
  
	time = _vm->cur.tick+aft;
  
	// Bubble this event down
	for (i = etop; i >= 1 && eventq[i-1].time <= time; i--) {
		eventq[i].event = eventq[i-1].event;
		eventq[i].time = eventq[i-1].time;
		eventq[i].where = eventq[i-1].where;
	}
  
	eventq[i].time = time;
	eventq[i].where = whr;
	eventq[i].event = evt;
	etop++;
}

Aptr Execute::getAttribute(Aaddr atradr, Aaddr atr) {
	AtrElem *at = (AtrElem *)addrTo(atradr);
	return at[atr - 1].val;
}

void Execute::setAttribute(Aaddr atradr, Aword atr, Aword val) {
	AtrElem *at = (AtrElem *)addrTo(atradr);
	at[atr - 1].val = val;
}

void Execute::make(Aword id, Aword atr, Aword val) {
	if (isObj(id))
		setAttribute(_objs[id - OBJMIN].atrs, atr, val);
	else if (isLoc(id))
		setAttribute(_locs[id - LOCMIN].atrs, atr, val);
	else if (isAct(id))
		setAttribute(_acts[id - ACTMIN].atrs, atr, val);
	else
		error("Can't MAKE item (%ld).", (unsigned long)id);
}

void Execute::set(Aword id, Aword atr, Aword val) {
	if (isObj(id))
		setAttribute(_objs[id - OBJMIN].atrs, atr, val);
	else if (isLoc(id)) {
		setAttribute(_locs[id - LOCMIN].atrs, atr, val);
		_locs[id - LOCMIN].describe = 0;
	} else if (isAct(id))
		setAttribute(_acts[id - ACTMIN].atrs, atr, val);
	else
		error("Can't SET item (%ld).", (unsigned long)id);
}

void Execute::setstr(Aword id, Aword atr, Aword str) {
	free((char *)attribute(id, atr));
	set(id, atr, str);
}

void Execute::incAttribute(Aaddr atradr, Aword atr, Aword step) {
	AtrElem *at = (AtrElem *) addrTo(atradr);
	at[atr - 1].val += step;
}

void Execute::incLocation(Aword loc, Aword atr, Aword step) {
	incAttribute(_locs[loc - LOCMIN].atrs, atr, step);
	_locs[loc - LOCMIN].describe = 0;
}

void Execute::incObject(Aword obj, Aword atr, Aword step) {
	incAttribute(_objs[obj - OBJMIN].atrs, atr, step);
}

void Execute::incract(Aword act, Aword atr, Aword step) {
	incAttribute(_acts[act - ACTMIN].atrs, atr, step);
}

void Execute::incr(Aword id, Aword atr, Aword step) {
	if (isObj(id))
		incObject(id, atr, step);
	else if (isLoc(id))
		incLocation(id, atr, step);
	else if (isAct(id))
		incract(id, atr, step);
	else
		error("Can't INCR item (%ld).", (unsigned long)id);
}

void Execute::decr(Aword id, Aword atr, Aword step) {
	if (isObj(id))
		incObject(id, atr, -(int)step);
	else if (isLoc(id))
		incLocation(id, atr, -(int)step);
	else if (isAct(id))
		incract(id, atr, -(int)step);
	else
		error("Can't DECR item (%ld).", (unsigned long)id);
}

Aptr Execute::attribute(Aword id, Aword atr) {
	if (isObj(id))
		return getAttribute(_objs[id - OBJMIN].atrs, atr);
	else if (isLoc(id))
		return getAttribute(_locs[id - LOCMIN].atrs, atr);
	else if (isAct(id))
		return getAttribute(_acts[id - ACTMIN].atrs, atr);
	else if (isLit(id)) {
		if (atr == 1)
			return litValues[id - LITMIN].value;
		else
			error("Unknown attribute for literal (%ld).", (unsigned long)atr);
	} else
		error("Can't ATTRIBUTE item (%ld).", (unsigned long) id);
}

Aptr Execute::strattr(Aword id, Aword atr) {
	Common::String result = (char *)attribute(id, atr);
	return (Aptr)result.c_str();
}

Aword Execute::objloc(Aword obj) {
	if (isCnt(_objs[obj - OBJMIN].loc)) { // In something ?
		if (isObj(_objs[obj - OBJMIN].loc) || isAct(_objs[obj - OBJMIN].loc))
			return(where(_objs[obj - OBJMIN].loc));
		else // Containers not anywhere is where the hero is!
			return(where(HERO));
	} else {
		return(_objs[obj - OBJMIN].loc);
	}
}

Aword Execute::actloc(Aword act) {
	return(_acts[act - ACTMIN].loc);
}

Aword Execute::where(Aword id) {
	if (isObj(id))
		return objloc(id);
	else if (isAct(id))
		return actloc(id);
	else
		error("Can't WHERE item (%ld).", (unsigned long) id);
}

Aint Execute::agrmax(Aword atr, Aword whr) {
	Aword i;
	uint max = 0;

	for (i = OBJMIN; i <= OBJMAX; i++) {
		if (isLoc(whr)) {
			if (where(i) == whr && attribute(i, atr) > max)
				max = attribute(i, atr);
		} else if (_objs[i - OBJMIN].loc == whr && attribute(i, atr) > max)
			max = attribute(i, atr);
	}

	return(max);
}

Aint Execute::agrsum(Aword atr, Aword whr) {
	Aword i;
	uint sum = 0;

	for (i = OBJMIN; i <= OBJMAX; i++) {
		if (isLoc(whr)) {
			if (where(i) == whr)
				sum += attribute(i, atr);
		} else if (_objs[i-OBJMIN].loc == whr)
			sum += attribute(i, atr);
	}

	return(sum);
}

Aint Execute::agrcount(Aword whr) {
	Aword i;
	Aword count = 0;

	for (i = OBJMIN; i <= OBJMAX; i++) {
		if (isLoc(whr)) {
			if (where(i) == whr)
				count++;
		} else if (_objs[i-OBJMIN].loc == whr)
			count++;
	}

	return(count);
}

void Execute::locobj(Aword obj, Aword whr) {
	if (isCnt(whr)) { // Into a container
		if (whr == obj)
			error("Locating something inside itself.");
		if (checkContainerLimit(whr, obj))
			return;
		else
			_objs[obj-OBJMIN].loc = whr;
	} else {
		_objs[obj-OBJMIN].loc = whr;
		// Make sure the location is described since it's changed
		_locs[whr-LOCMIN].describe = 0;
	}
}

void Execute::locact(Aword act, Aword whr) {
	Aword prevact = _vm->cur.act;
	Aword prevloc = _vm->cur.loc;

	_vm->cur.loc = whr;
	_acts[act - ACTMIN].loc = whr;

	if (act == HERO) {
		if (_locs[_acts[act - ACTMIN].loc-LOCMIN].describe % (_vm->cur.visits+1) == 0)
			look();
		else {
			if (_vm->_anyOutput)
				_vm->paragraph();

			say(where(HERO));
			_vm->printMessage(M_AGAIN);
			_vm->newLine();
			dscrobjs();
			dscracts();
		}

		_locs[where(HERO)-LOCMIN].describe++;
		_locs[where(HERO)-LOCMIN].describe %= (_vm->cur.visits+1);
	} else
		_locs[whr-LOCMIN].describe = 0;

	if (_locs[_vm->cur.loc-LOCMIN].does != 0) {
		_vm->cur.act = act;
		_vm->_interpreter->interpret(_locs[_vm->cur.loc-LOCMIN].does);
		_vm->cur.act = prevact;
	}

	if (_vm->cur.act != (int)act)
		_vm->cur.loc = prevloc;
}


void Execute::locate(Aword id, Aword whr) {
	if (isObj(id))
		locobj(id, whr);
	else if (isAct(id))
		locact(id, whr);
	else
		error("Can't LOCATE item (%ld).", (unsigned long)id);
}

Abool Execute::objhere(Aword obj) {
	if (isCnt(_objs[obj - OBJMIN].loc)) {	// In something?
		if (isObj(_objs[obj - OBJMIN].loc) || isAct(_objs[obj - OBJMIN].loc))
			return(isHere(_objs[obj - OBJMIN].loc));
		else // If the container wasn't anywhere, assume where HERO is!
			return((int)where(HERO) == _vm->cur.loc);
	} else
		return((int)_objs[obj - OBJMIN].loc == _vm->cur.loc);
}

Aword Execute::acthere(Aword act) {
	return (int)_acts[act - ACTMIN].loc == _vm->cur.loc;
}

Abool Execute::isHere(Aword id) {
	if (isObj(id))
		return objhere(id);
	else if (isAct(id))
		return acthere(id);
	else
		error("Can't HERE item (%ld).", (unsigned long)id);
}

Aword Execute::objnear(Aword obj) {
	if (isCnt(_objs[obj-OBJMIN].loc)) {		// In something?
		if (isObj(_objs[obj-OBJMIN].loc) || isAct(_objs[obj-OBJMIN].loc))
			return(isNear(_objs[obj-OBJMIN].loc));
		else  // If the container wasn't anywhere, assume here, so not nearby!
			return(false);
	} else {
		return(exitto(where(obj), _vm->cur.loc));
	}
}

Aword Execute::actnear(Aword act) {
	return(exitto(where(act), _vm->cur.loc));
}


Abool Execute::isNear(Aword id) {
	if (isObj(id))
		return objnear(id);
	else if (isAct(id))
		return actnear(id);
	else
		error("Can't NEAR item (%ld).", (unsigned long) id);
}

Abool Execute::in(Aword obj, Aword cnt) {
	if (!isObj(obj))
		return(false);
	if (!isCnt(cnt))
		error("IN in a non-container.");

	return(_objs[obj - OBJMIN].loc == cnt);
}

void Execute::sayarticle(Aword id) {
	if (!isObj(id))
		error("Trying to say article of something *not* an object.");
	if (_objs[id - OBJMIN].art != 0)
		_vm->_interpreter->interpret(_objs[id - OBJMIN].art);
	else
		_vm->printMessage(M_ARTICLE);
}

void Execute::say(Aword id) {
	if (isHere(HERO)) {
		if (isObj(id))
			_vm->_interpreter->interpret(_objs[id - OBJMIN].dscr2);
		else if (isLoc(id))
			_vm->_interpreter->interpret(_locs[id - LOCMIN].nams);
		else if (isAct(id))
			_vm->_interpreter->interpret(_acts[id - ACTMIN].nam);
		else if (isLit(id)) {
			if (isNum(id))
				_vm->output(Common::String::format("%ld", litValues[id - LITMIN].value));
			else
				_vm->output((char *)litValues[id - LITMIN].value);
		} else
			error("Can't SAY item (%ld).", (unsigned long)id);
	}
}

void Execute::dscrloc(Aword loc) {
	if (_locs[loc - LOCMIN].dscr != 0)
		_vm->_interpreter->interpret(_locs[loc - LOCMIN].dscr);
}

void Execute::dscrobj(Aword obj) {
	_objs[obj - OBJMIN].describe = false;
	if (_objs[obj - OBJMIN].dscr1 != 0)
		_vm->_interpreter->interpret(_objs[obj - OBJMIN].dscr1);
	else {
		_vm->printMessage(M_SEEOBJ1);
		sayarticle(obj);
		say(obj);
		_vm->printMessage(M_SEEOBJ4);
		if (_objs[obj - OBJMIN].cont != 0)
			list(obj);
	}
}


void Execute::dscract(Aword act) {
	ScrElem *scr = NULL;

	if (_acts[act - ACTMIN].script != 0) {
		for (scr = (ScrElem *) addrTo(_acts[act - ACTMIN].scradr); !endOfTable(scr); scr++)
			if (scr->code == _acts[act - ACTMIN].script)
				break;
		if (endOfTable(scr))
			scr = NULL;
	}

	if (scr != NULL && scr->dscr != 0)
		_vm->_interpreter->interpret(scr->dscr);
	else if (_acts[act - ACTMIN].dscr != 0)
		_vm->_interpreter->interpret(_acts[act - ACTMIN].dscr);
	else {
		_vm->_interpreter->interpret(_acts[act - ACTMIN].nam);
		_vm->printMessage(M_SEEACT);
	}

	_acts[act - ACTMIN].describe = false;
}

void Execute::describe(Aword id) {
	for (uint i = 0; i < _describeStack.size(); i++) {
		_describeStack.push(id);

		if (isObj(id))
			dscrobj(id);
		else if (isLoc(id))
			dscrloc(id);
		else if (isAct(id))
			dscract(id);
		else
			error("Can't DESCRIBE item (%ld).", (unsigned long)id);
	}

	_describeStack.pop();
}

void Execute::use(Aword act, Aword scr) {
	if (!isAct(act))
		error("Item is not an Actor (%ld).", (unsigned long) act);

	_acts[act - ACTMIN].script = scr;
	_acts[act - ACTMIN].step = 0;
}

void Execute::list(Aword cnt) {
	uint i;
	Aword props;
	Aword prevobj = 0;
	bool found = false;
	bool multiple = false;

	// Find container properties
	if (isObj(cnt))
		props = _objs[cnt-OBJMIN].cont;
	else if (isAct(cnt))
		props = _acts[cnt-ACTMIN].cont;
	else
		props = cnt;

	for (i = OBJMIN; i <= OBJMAX; i++) {
		if (in(i, cnt)) { // Yes, it's in this container
			if (!found) {
				found = true;
				if (_cnts[props-CNTMIN].header != 0)
					_vm->_interpreter->interpret(_cnts[props-CNTMIN].header);
				else {
					_vm->printMessage(M_CONTAINS1);
					if (_cnts[props-CNTMIN].nam != 0) // It has it's own name
						_vm->_interpreter->interpret(_cnts[props-CNTMIN].nam);
					else
						say(_cnts[props-CNTMIN].parent); // It is actually an object or actor

					_vm->printMessage(M_CONTAINS2);
				}
			} else {
				if (multiple) {
					_vm->_needSpace = false;
					_vm->printMessage(M_CONTAINS3);
				}

				multiple = true;
				sayarticle(prevobj);
				say(prevobj);
			}

			prevobj = i;
		}
	}

	if (found) {
		if (multiple)
			_vm->printMessage(M_CONTAINS4);

		sayarticle(prevobj);
		say(prevobj);
		_vm->printMessage(M_CONTAINS5);
	} else {
		if (_cnts[props-CNTMIN].empty != 0)
			_vm->_interpreter->interpret(_cnts[props-CNTMIN].empty);
		else {
			_vm->printMessage(M_EMPTY1);

			if (_cnts[props-CNTMIN].nam != 0) // It has it's own name
				_vm->_interpreter->interpret(_cnts[props-CNTMIN].nam);
			else
				say(_cnts[props-CNTMIN].parent);	// It is actually an actor or object

			_vm->printMessage(M_EMPTY2);
		}
	}

	_vm->_needSpace = true;
}

void Execute::empty(Aword cnt, Aword whr) {
	for (uint i = OBJMIN; i <= OBJMAX; i++)
		if (in(i, cnt))
			locate(i, whr);
}

void Execute::dscrobjs() {
	uint i;
	int prevobj = 0;
	bool found = false;
	bool multiple = false;

	// First describe everything here with its own description
	for (i = OBJMIN; i <= OBJMAX; i++) {
		if ((int)_objs[i - OBJMIN].loc == _vm->cur.loc && _objs[i - OBJMIN].describe && _objs[i - OBJMIN].dscr1)
			describe(i);
	}

	// Then list everything else here
	for (i = OBJMIN; i <= OBJMAX; i++) {
		if ((int)_objs[i - OBJMIN].loc == _vm->cur.loc && _objs[i - OBJMIN].describe) {
			if (!found) {
				_vm->printMessage(M_SEEOBJ1);
				sayarticle(i);
				say(i);
				found = true;
			} else {
				if (multiple) {
					_vm->_needSpace = false;
					_vm->printMessage(M_SEEOBJ2);
					sayarticle(prevobj);
					say(prevobj);
				}

				multiple = true;
			}

			prevobj = i;
		}
	}

	if (found) {
		if (multiple) {
			_vm->printMessage(M_SEEOBJ3);
			sayarticle(prevobj);
			say(prevobj);
		}

		_vm->printMessage(M_SEEOBJ4);
	}
  
	// Set describe flag for all objects
	for (i = OBJMIN; i <= OBJMAX; i++)
		_objs[i-OBJMIN].describe = true;
}


void Execute::dscracts() {
	for (uint i = HERO+1; i <= ACTMAX; i++)
		if ((int)_acts[i-ACTMIN].loc == _vm->cur.loc &&
			_acts[i-ACTMIN].describe)
			describe(i);

	// Set describe flag for all actors
	for (uint i = HERO; i <= ACTMAX; i++)
		_acts[i-ACTMIN].describe = true;
}

void Execute::look() {
	uint i;
	_vm->looking = true;

	// Set describe flag for all objects and actors
	for (i = OBJMIN; i <= OBJMAX; i++)
		_objs[i-OBJMIN].describe = true;
	for (i = ACTMIN; i <= ACTMAX; i++)
		_acts[i-ACTMIN].describe = true;

	if (_vm->_anyOutput)
		_vm->paragraph();

	//glk_set_style(style_Subheader);	// TODO
	_vm->_needSpace = false;
	say(_vm->cur.loc);

	_vm->_needSpace = false;
	_vm->output(".");

	//glk_set_style(style_Normal);	// TODO
	_vm->newLine();
	_vm->_needSpace = false;
	describe(_vm->cur.loc);
	dscrobjs();
	dscracts();
	_vm->looking = false;
}

Aword Execute::rnd(Aword from, Aword to) {
	if (to == from)
		return to;
	else if (to > from)
		return to;	// TODO
		//return (rand()/10)%(to-from+1)+from;
	else
		return to;	// TODO
		//return (rand()/10)%(from-to+1)+to;
}

// Between
Abool Execute::btw(Aint val, Aint low, Aint high) {
	if (high > low)
		return low <= val && val <= high;
	else
		return high <= val && val <= low;
}

// TODO: Replace this with Common::String functionality
Aword Execute::contains(Aptr string, Aptr substring) {
	Common::String str = (char *)string;
	char *substr = (char *)substring;
	bool result = str.contains(substr);

	free((char *)string);
	free((char *)substring);

	return result;
}

// TODO: Replace this with Common::String functionality
// Compare two strings approximately, ignore case
Abool Execute::streq(char *a, char *b) {
	Common::String str1 = a;
	Common::String str2 = b;
	bool result = str1.equalsIgnoreCase(str2);

	free(a);
	free(b);

	return result;
}

} // End of namespace Alan2
} // End of namespace Glk
