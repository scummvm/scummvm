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

#include "glk/alan2/acode.h"
#include "glk/alan2/alan2.h"
#include "glk/alan2/execute.h"
#include "glk/alan2/interpreter.h"
#include "glk/alan2/saveload.h"
#include "glk/alan2/types.h"
#include "common/debug.h"

namespace Glk {
namespace Alan2 {

// TODO: Refactor these into debug flags
const bool trcflg = false;
const bool stpflg = false;

void Interpreter::if_(Aword v) {
	int lev = 1;
	Aword i;

	if (!v) {
		// Skip to next ELSE or ENDIF on same level
		while (true) {
			i = _vm->memory[_vm->pc++];
			if (I_CLASS(i) == (Aword)C_STMOP) {
				switch (I_OP(i)) {
				case I_ELSE:
					if (lev == 1)
						return;
					break;
				case I_IF:
					lev++;
					break;
				case I_ENDIF:
					lev--;
					if (lev == 0)
						return;
					break;
				default:
					break;
				}
			}
		}
	}
}

void Interpreter::else_() {
	int lev = 1;
	Aword i;

	while (true) {
		// Skip to ENDIF on the same level
		i = _vm->memory[_vm->pc++];
		if (I_CLASS(i) == (Aword)C_STMOP) {
			switch (I_OP(i)) {
			case I_ENDIF:
				lev--;
				if (lev == 0)
					return;
				break;
			case I_IF:
				lev++;
				break;
			default:
				break;
			}
		}
	}
}

void Interpreter::depstart() {
	// A DEPSTART was executed, so skip across the redundant DEPCASE to
	// start at the first expression
	_vm->pc++;
}

void Interpreter::swap() {
	Aptr v1 = _stack->pop();
	Aptr v2 = _stack->pop();

	_stack->push(v1);
	_stack->push(v2);
}

void Interpreter::depexec(Aword v) {
	int lev = 1;
	Aword i;

	if (!v) {
		// The expression was not true, skip to next CASE on the same
		//   level which could be a DEPCASE or DEPELSE
		while (true) {
			i = _vm->memory[_vm->pc++];
			if (I_CLASS(i) == (Aword)C_STMOP) {
				switch (I_OP(i)) {
				case I_DEPSTART:
					lev++;
					break;
				case I_DEPEND:
					if (lev == 1)
						return;
					lev--;
					break;
				case I_DEPCASE:
				case I_DEPELSE:
					if (lev == 1)
						return;
					break;
				default:
					break;
				}
			}
		}
	}
}

void Interpreter::depcase() {
	int lev = 1;
	Aword i;

	// Skip to end of DEPENDING block (next DEPEND on same level) because
	// we have just executed a DEPCASE/DEPELSE statement as a result of a DEPCASE
	// catching

	while (true) {
		i = _vm->memory[_vm->pc++];
		if (I_CLASS(i) == (Aword)C_STMOP) {
			switch (I_OP(i)) {
			case I_DEPSTART:
				lev++;
				break;
			case I_DEPEND:
				lev--;
				if (lev == 0) return;
				break;
			default:
				break;
			}
		}
	}
}

void Interpreter::curVar(Aword i) {
	switch (I_OP(i)) {
	case V_PARAM:
		if (stpflg)
			debug("PARAM \t%5ld\t\t(%ld)", _stack->top(), _vm->params[_stack->top() - 1].code);
		_stack->push(_vm->params[_stack->pop() - 1].code);
		break;
	case V_CURLOC:
		if (stpflg)
			debug("CURLOC \t\t\t(%d)", _vm->cur.loc);
		_stack->push(_vm->cur.loc);
		break;
	case V_CURACT:
		if (stpflg)
			debug("CURACT \t\t\t(%d)", _vm->cur.act);
		_stack->push(_vm->cur.act);
		break;
	case V_CURVRB:
		if (stpflg)
			debug("CURVRB \t\t\t(%d)", _vm->cur.vrb);
		_stack->push(_vm->cur.vrb);
		break;
	case V_SCORE:
		if (stpflg)
			debug("CURSCORE \t\t\t(%d)", _vm->cur.score);
		_stack->push(_vm->cur.score);
		break;
	default:
		error("Unknown CURVAR instruction.");
	}
}

void Interpreter::adaptOldOpcodes(Aword i) {
	switch (I_OP(i)) {
	case I_AND:
	case I_OR:
	case I_NE:
	case I_EQ:
	case I_STREQ:
	case I_STREXACT:
	case I_LE:
	case I_GE:
	case I_LT:
	case I_GT:
	case I_PLUS:
	case I_MINUS:
	case I_MULT:
	case I_DIV:
		if (_vm->header->vers[0] == 2 && _vm->header->vers[1] == 7) // Check for 2.7 version
			swap();
	}
}

void Interpreter::stMop(Aword i, Aaddr oldpc) {
	Aptr arg1, arg2, arg3, lh, rh;

	adaptOldOpcodes(i);

	switch (I_OP(i)) {
	case I_PRINT:
		arg1 = _stack->pop();	// fpos
		arg2 = _stack->pop();	// len
		if (stpflg) {
			debug("PRINT \t%5ld, %5ld\t\"", arg1, arg2);
			_vm->col = 34;		// To format it better!
		}
		_execute->print(arg1, arg2);
		if (stpflg)
			debug("\"");
		break;
	case I_SYSTEM:
		arg1 = _stack->pop();	// fpos
		arg2 = _stack->pop();	// len
		if (stpflg) {
			debug("SYSTEM \t%5ld, %5ld\t\"", arg1, arg2);
			_vm->col = 34;		// To format it better!
		}
		_execute->sys(arg1, arg2);
		break;
	case I_GETSTR:
		arg1 = _stack->pop();	// fpos
		arg2 = _stack->pop();	// len
		if (stpflg)
			debug("GETSTR\t%5ld, %5ld", arg1, arg2);
		_execute->getstr(arg1, arg2);
		if (stpflg)
			debug("\t(%ld)", _stack->top());
		break;
	case I_QUIT:
		if (stpflg)
			debug("QUIT");
		_execute->quit();
		break;
	case I_LOOK:
		if (stpflg)
			debug("LOOK");
		_execute->look();
		break;
	case I_SAVE:
		if (stpflg)
			debug("SAVE");
		_saveLoad->save();
		break;
	case I_RESTORE:
		if (stpflg)
			debug("RESTORE");
		_saveLoad->restore();
		break;
	case I_RESTART:
		if (stpflg)
			debug("RESTART");
		_execute->restart();
		break;
	case I_LIST:
		arg1 = _stack->pop();	// cnt
		if (stpflg)
			debug("LIST \t%5ld", arg1);
		_execute->list(arg1);
		break;
	case I_EMPTY:
		arg1 = _stack->pop();	// cnt
		arg2 = _stack->pop();	// whr
		if (stpflg)
			debug("EMPTY \t%5ld, %5ld", arg1, arg2);
		_execute->empty(arg1, arg2);
		break;
	case I_SCORE:
		arg1 = _stack->pop();	// score
		if (stpflg)
			debug("SCORE \t%5ld\t\t(%ld)", arg1, _vm->scores[arg1 - 1]);
		_execute->score(arg1);
		break;
	case I_VISITS:
		arg1 = _stack->pop();	// visits
		if (stpflg)
			debug("VISITS \t%5ld", arg1);
		_execute->visits(arg1);
		break;
	case I_SCHEDULE:
		arg1 = _stack->pop();	// evt
		arg2 = _stack->pop();	// whr
		arg3 = _stack->pop();	// aft
		if (stpflg)
			debug("SCHEDULE \t%5ld, %5ld, %5ld", arg1, arg2, arg3);
		_execute->schedule(arg1, arg2, arg3);
		break;
	case I_CANCEL:
		arg1 = _stack->pop();	// evt
		if (stpflg)
			debug("CANCEL \t%5ld", arg1);
		_execute->cancl(arg1);
		break;
	case I_MAKE:
		arg1 = _stack->pop();	// id
		arg2 = _stack->pop();	// atr
		arg3 = _stack->pop();	// val
		if (stpflg) {
			debug("MAKE \t%5ld, %5ld, ", arg1, arg2);
			if (arg3)
				debug("TRUE");
			else
				debug("FALSE");
		}
		_execute->make(arg1, arg2, arg3);
		break;
	case I_SET:
		arg1 = _stack->pop();	// id
		arg2 = _stack->pop();	// atr
		arg3 = _stack->pop();	// val
		if (stpflg)
			debug("SET \t%5ld, %5ld, %5ld", arg1, arg2, arg3);
		_execute->set(arg1, arg2, arg3);
		break;
	case I_STRSET:
		arg1 = _stack->pop();	// id
		arg2 = _stack->pop();	// atr
		arg3 = _stack->pop();	// str
		if (stpflg)
			debug("STRSET\t%5ld, %5ld, %5ld", arg1, arg2, arg3);
		_execute->setstr(arg1, arg2, arg3);
		break;
	case I_INCR:
		arg1 = _stack->pop();	// id
		arg2 = _stack->pop();	// atr
		arg3 = _stack->pop();	// step
		if (stpflg)
			debug("INCR\t%5ld, %5ld, %5ld", arg1, arg2, arg3);
		_execute->incr(arg1, arg2, arg3);
		break;
	case I_DECR:
		arg1 = _stack->pop();	// id
		arg2 = _stack->pop();	// atr
		arg3 = _stack->pop();	// step
		if (stpflg)
			debug("DECR\t%5ld, %5ld, %5ld", arg1, arg2, arg3);
		_execute->decr(arg1, arg2, arg3);
		break;
	case I_ATTRIBUTE:
		arg1 = _stack->pop();	// id
		arg2 = _stack->pop();	// atr
		if (stpflg)
			debug("ATTRIBUTE %5ld, %5ld", arg1, arg2);
		_stack->push(_execute->attribute(arg1, arg2));
		if (stpflg)
			debug("\t(%ld)", _stack->top());
		break;
	case I_STRATTR:
		arg1 = _stack->pop();	// id
		arg2 = _stack->pop();	// atr
		if (stpflg)
			debug("STRATTR \t%5ld, %5ld", arg1, arg2);
		_stack->push(_execute->strattr(arg1, arg2));
		if (stpflg)
			debug("\t(%ld)", _stack->top());
		break;
	case I_LOCATE:
		arg1 = _stack->pop();	// id
		arg2 = _stack->pop();	// whr
		if (stpflg)
			debug("LOCATE \t%5ld, %5ld", arg1, arg2);
		_execute->locate(arg1, arg2);
		break;
	case I_WHERE:
		arg1 = _stack->pop();	// id
		if (stpflg)
			debug("WHERE \t%5ld", arg1);
		_stack->push(_execute->where(arg1));
		if (stpflg)
			debug("\t\t(%ld)", _stack->top());
		break;
	case I_HERE:
		arg1 = _stack->pop();	// id
		if (stpflg)
			debug("HERE \t%5ld", arg1);
		_stack->push(_execute->isHere(arg1));
		if (stpflg) {
			if (_stack->top())
				debug("\t(TRUE)");
			else
				debug("\t(FALSE)");
		}
		break;
	case I_NEAR:
		arg1 = _stack->pop();	// id
		if (stpflg)
			debug("NEAR \t%5ld", arg1);
		_stack->push(_execute->isNear(arg1));
		if (stpflg) {
			if (_stack->top())
				debug("\t(TRUE)");
			else
				debug("\t(FALSE)");
		}
		break;
	case I_USE:
		arg1 = _stack->pop();	// act
		arg2 = _stack->pop();	// scr
		if (stpflg)
			debug("USE \t%5ld, %5ld", arg1, arg2);
		_execute->use(arg1, arg2);
		break;
	case I_IN:
		arg1 = _stack->pop();	// obj
		arg2 = _stack->pop();	// cnt
		if (stpflg)
			debug("IN \t%5ld, %5ld ", arg1, arg2);
		_stack->push(_execute->in(arg1, arg2));
		if (stpflg)
			if (_stack->top()) debug("\t(TRUE)"); else debug("\t(FALSE)");
		break;
	case I_DESCRIBE:
		arg1 = _stack->pop();	// id
		if (stpflg) {
			debug("DESCRIBE \t%5ld\t", arg1);
			_vm->col = 34;		// To format it better!
		}
		_execute->describe(arg1);
		break;
	case I_SAY:
		arg1 = _stack->pop();	// id
		if (stpflg)
			debug("SAY \t%5ld\t\t\"", arg1);
		_execute->say(arg1);
		if (stpflg)
			debug("\"");
		break;
	case I_SAYINT:
		arg1 = _stack->pop();	// val
		if (stpflg)
			debug("SAYINT\t%5ld\t\t\"", arg1);
		_execute->sayint(arg1);
		if (stpflg)
			debug("\"");
		break;
	case I_SAYSTR:
		arg1 = _stack->pop();	// adr
		if (stpflg)
			debug("SAYSTR\t%5ld\t\t\"", arg1);
		_execute->saystr((char *)arg1);
		if (stpflg)
			debug("\"");
		break;
	case I_IF:
		arg1 = _stack->pop();	// v
		if (stpflg) {
			debug("IF \t");
			if (arg1) debug(" TRUE"); else debug("FALSE");
		}
		if_(arg1);
		break;
	case I_ELSE:
		if (stpflg)
			debug("ELSE");
		else_();
		break;
	case I_ENDIF:
		if (stpflg)
			debug("ENDIF");
		break;
	case I_AND:
		rh = _stack->pop();
		lh = _stack->pop();
		if (stpflg) {
			debug("AND \t");
			if (lh) debug("TRUE, "); else debug("FALSE, ");
			if (rh) debug("TRUE"); else debug("FALSE");
		}
		_stack->push(lh && rh);
		if (stpflg)
			if (_stack->top()) debug("\t(TRUE)"); else debug("\t(FALSE)");
		break;
	case I_OR:
		rh = _stack->pop();
		lh = _stack->pop();
		if (stpflg) {
			debug("OR \t");
			if (lh) debug("TRUE, "); else debug("FALSE, ");
			if (rh) debug("TRUE"); else debug("FALSE");
		}
		_stack->push(lh || rh);
		if (stpflg) {
			if (_stack->top())
				debug("\t(TRUE)");
			else
				debug("\t(FALSE)");
		}
		break;
	case I_NE:
		rh = _stack->pop();
		lh = _stack->pop();
		if (stpflg)
			debug("NE \t%5ld, %5ld", lh, rh);
		_stack->push(lh != rh);
		if (stpflg)
			if (_stack->top()) debug("\t(TRUE)"); else debug("\t(FALSE)");
		break;
	case I_EQ:
		rh = _stack->pop();
		lh = _stack->pop();
		if (stpflg)
			debug("EQ \t%5ld, %5ld", lh, rh);
		_stack->push(lh == rh);
		if (stpflg) {
			if (_stack->top())
				debug("\t(TRUE)");
			else
				debug("\t(FALSE)");
		}
		break;
	case I_STREQ:
		rh = _stack->pop();
		lh = _stack->pop();
		if (stpflg)
			debug("STREQ \t%5ld, %5ld", lh, rh);
		_stack->push(_execute->streq((char *)lh, (char *)rh));
		if (stpflg)
			if (_stack->top()) debug("\t(TRUE)"); else debug("\t(FALSE)");
		break;
	case I_STREXACT:
		rh = _stack->pop();
		lh = _stack->pop();
		if (stpflg)
			debug("STREXACT \t%5ld, %5ld", lh, rh);
		_stack->push(strcmp((char *)lh, (char *)rh) == 0);
		if (stpflg)
			if (_stack->top()) debug("\t(TRUE)"); else debug("\t(FALSE)");
		free((void *)lh);
		free((void *)rh);
		break;
	case I_LE:
		rh = _stack->pop();
		lh = _stack->pop();
		if (stpflg)
			debug("LE \t%5ld, %5ld", lh, rh);
		_stack->push(lh <= rh);
		if (stpflg)
			if (_stack->top()) debug("\t(TRUE)"); else debug("\t(FALSE)");
		break;
	case I_GE:
		rh = _stack->pop();
		lh = _stack->pop();
		if (stpflg)
			debug("GE \t%5ld, %5ld", lh, rh);
		_stack->push(lh >= rh);
		if (stpflg)
			if (_stack->top()) debug("\t(TRUE)"); else debug("\t(FALSE)");
		break;
	case I_LT:
		rh = _stack->pop();
		lh = _stack->pop();
		if (stpflg)
			debug("LT \t%5ld, %5ld", lh, rh);
		_stack->push((signed int)lh < (signed int)rh);
		if (stpflg)
			if (_stack->top()) debug("\t(TRUE)"); else debug("\t(FALSE)");
		break;
	case I_GT:
		rh = _stack->pop();
		lh = _stack->pop();
		if (stpflg)
			debug("GT \t%5ld, %5ld", lh, rh);
		_stack->push(lh > rh);
		if (stpflg)
			if (_stack->top()) debug("\t(TRUE)"); else debug("\t(FALSE)");
		break;
	case I_PLUS:
		rh = _stack->pop();
		lh = _stack->pop();
		if (stpflg)
			debug("PLUS \t%5ld, %5ld", lh, rh);
		_stack->push(lh + rh);
		if (stpflg)
			debug("\t(%ld)", _stack->top());
		break;
	case I_MINUS:
		rh = _stack->pop();
		lh = _stack->pop();
		if (stpflg)
			debug("MINUS \t%5ld, %5ld", lh, rh);
		_stack->push(lh - rh);
		if (stpflg)
			debug("\t(%ld)", _stack->top());
		break;
	case I_MULT:
		rh = _stack->pop();
		lh = _stack->pop();
		if (stpflg)
			debug("MULT \t%5ld, %5ld", lh, rh);
		_stack->push(lh * rh);
		if (stpflg)
			debug("\t(%ld)", _stack->top());
		break;
	case I_DIV:
		rh = _stack->pop();
		lh = _stack->pop();
		if (stpflg)
			debug("DIV \t%5ld, %5ld", lh, rh);
		_stack->push(lh / rh);
		if (stpflg)
			debug("\t(%ld)", _stack->top());
		break;
	case I_NOT:
		arg1 = _stack->pop();	// val
		if (stpflg) {
			debug("NOT \t");
			if (arg1) debug("TRUE"); else debug("FALSE");
		}
		_stack->push(!arg1);
		if (stpflg)
			if (_stack->top()) debug("\t\t(TRUE)"); else debug("\t\t(FALSE)");
		break;
	case I_MAX:
		arg1 = _stack->pop();	// atr
		arg2 = _stack->pop();	// whr
		if (stpflg)
			debug("MAX \t%5ld, %5ld", arg1, arg2);
		_stack->push(_execute->agrmax(arg1, arg2));
		if (stpflg)
			debug("\t(%ld)", _stack->top());
		break;
	case I_SUM:
		arg1 = _stack->pop();	// atr
		arg2 = _stack->pop();	// whr
		if (stpflg)
			debug("SUM \t%5ld, %5ld", arg1, arg2);
		_stack->push(_execute->agrsum(arg1, arg2));
		if (stpflg)
			debug("\t(%ld)", _stack->top());
		break;
	case I_COUNT:
		arg1 = _stack->pop();	// whr
		if (stpflg)
			debug("COUNT \t%5ld", arg1);
		_stack->push(_execute->agrcount(arg1));
		if (stpflg)
			debug("\t(%ld)", _stack->top());
		break;
	case I_RND:
		arg1 = _stack->pop();	// from
		arg2 = _stack->pop();	// to
		if (stpflg)
			debug("RANDOM \t%5ld, %5ld", arg1, arg2);
		_stack->push(_execute->rnd(arg1, arg2));
		if (stpflg)
			debug("\t(%ld)", _stack->top());
		break;
	case I_BTW:
		arg1 = _stack->pop();	// high
		arg2 = _stack->pop();	// low
		arg3 = _stack->pop();	// val
		if (stpflg)
			debug("BETWEEN \t%5ld, %5ld, %5ld", arg1, arg2, arg3);
		_stack->push(_execute->btw(arg1, arg2, arg3));
		if (stpflg)
			debug("\t(%ld)", _stack->top());
		break;
	case I_CONTAINS:
		arg1 = _stack->pop();	// substring
		arg2 = _stack->pop();	// string
		if (stpflg)
			debug("CONTAINS \t%5ld, %5ld", arg2, arg1);
		_stack->push(_execute->contains(arg2, arg1));
		if (stpflg)
			debug("\t(%ld)", _stack->top());
		break;
	case I_DEPSTART:
		if (stpflg)
			debug("DEPSTART");
		depstart();
		break;
	case I_DEPCASE:
		if (stpflg)
			debug("DEPCASE");
		depcase();
		break;
	case I_DEPEXEC:
		arg1 = _stack->pop();	// v
		if (stpflg) {
			debug("DEPEXEC \t");
			if (arg1) debug(" TRUE"); else debug("FALSE");
		}
		depexec(arg1);
		break;
	case I_DEPELSE:
		if (stpflg)
			debug("DEPELSE");
		depcase();
		break;
	case I_DEPEND:
		if (stpflg)
			debug("DEPEND");
		break;
	case I_RETURN:
		if (stpflg)
			debug("RETURN\n--------------------------------------------------\n");
		_vm->pc = oldpc;
		return;
	default:
		error("Unknown STMOP instruction.");
	}

	if (_vm->fail) {
		_vm->pc = oldpc;
		return;		// TODO: Return true/false, and stop execution if necessary
	}
}

void Interpreter::interpret(Aaddr adr) {
	Aaddr oldpc = _vm->pc;
	Aword i;
 	
	if (stpflg)
		debug("\n++++++++++++++++++++++++++++++++++++++++++++++++++");
 	
	_vm->pc = adr;

	while(true) {
		if (stpflg)
			debug("\n%4x: ", _vm->pc);

		if (_vm->pc > _vm->memTop)
			error("Interpreting outside program.");

		i = _vm->memory[_vm->pc++];
    
		switch (I_CLASS(i)) {
		case C_CONST:
			if (stpflg)
				debug("PUSH  \t%5ld", I_OP(i));
			_stack->push(I_OP(i));
			break;
		case C_CURVAR:
			curVar(i);
			break;
		case C_STMOP: 
			stMop(i, oldpc);
			break;
	    default:
			error("Unknown instruction class.");
    }
  }
}

} // End of namespace Alan2
} // End of namespace Glk
