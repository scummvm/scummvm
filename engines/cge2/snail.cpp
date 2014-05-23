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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge2/snail.h"
#include "cge2/fileio.h"
#include "cge2/hero.h"

namespace CGE2 {

const char *CommandHandler::_commandText[] = {
	"NOP", "USE", "PAUSE", "INF", "CAVE",
	"SLAVE", "FOCUS", "SETX", "SETY", "SETZ",
	"ADD", "SUB", "MUL", "DIV", "IF", "FLAG",
	"FLASH", "LIGHT", "CYCLE",
	"CLEAR", "TALK", "MOUSE",
	"MAP", "COUNT", "MIDI",
	"SETDLG", "MSKDLG",
	".DUMMY.",
	"WAIT", "HIDE", "ROOM",
	"SAY", "SOUND", "TIME", "KILL",
	"RSEQ", "SEQ", "SEND", "SWAP",
	"KEEP", "GIVE",
	"GETPOS", "GOTO", "MOVEX", "MOVEY",
	"MOVEZ", "TRANS", "PORT",
	"NEXT", "NNEXT", "MTNEXT", "FTNEXT",
	"RNNEXT", "RMTNEXT", "RFTNEXT",
	"RMNEAR", "RMMTAKE", "RMFTAKE",
	"SETREF", "BACKPT",
	"WALKTO", "REACH", "COVER", "UNCOVER",
	NULL };

CommandHandler::CommandHandler(CGE2Engine *vm, bool turbo)
	: _turbo(turbo), _textDelay(false), _timerExpiry(0), _talkEnable(true),
      _head(0), _tail(0), _commandList((Command *)malloc(sizeof(Command)* 256)),
      _count(1), _vm(vm) {
}

CommandHandler::~CommandHandler() {
	free(_commandList);
}

void CommandHandler::runCommand() {
	warning("STUB: CommandHandler::runCommand()");
}

void CommandHandler::addCommand(CommandType com, int ref, int val, void *ptr) {
	if (ref == 2)
		ref = 142 - _vm->_sex;
	Command *headCmd = &_commandList[_head++];
	headCmd->_commandType = com;
	headCmd->_ref = ref;
	headCmd->_val = val;
	headCmd->_spritePtr = ptr;
	headCmd->_cbType = kNullCB;
	if (headCmd->_commandType == kCmdClear) {
		clear();
	}
}

void CommandHandler::addCallback(CommandType com, int ref, int val, CallbackType cbType) {
	warning("STUB: CommandHandler::addCallback()");
}

void CommandHandler::insertCommand(CommandType com, int ref, int val, void *ptr) {
	warning("STUB: CommandHandler::insertCommand()");
}

bool CommandHandler::idle() {
	warning("STUB: CommandHandler::idle()");
	return false;
}

void CommandHandler::reset() {
	warning("STUB: CommandHandler::reset()");
}

void CommandHandler::clear() {
	_tail = _head;
	_vm->killText();
	_timerExpiry = 0;
}

int CommandHandler::com(const char *com) {
	int i = _vm->takeEnum(_commandText, com);
	return (i < 0) ? i : i + kCmdCom0 + 1;
}

void CGE2Engine::feedSnail(Sprite *spr, Action snq, Hero *hero) {
	if (!spr || !spr->active())
		return;

	int cnt = spr->_actionCtrl[snq]._cnt;
	if (cnt) {
		byte ptr = spr->_actionCtrl[snq]._ptr;
		CommandHandler::Command *comtab = spr->snList(snq);
		CommandHandler::Command *c = &comtab[ptr];
		CommandHandler::Command *p;
		CommandHandler::Command *q = &comtab[cnt];

		warning("STUB: CGE2Engine::feedSnail()");
		// Dont bother with pockets (inventory system) for now... TODO: Implement it later!!!
		/*
		int pocFre = freePockets(hero->_ref & 1);
		int pocReq = 0;
		for (p = c; p < q && p->_commandType != kCmdNext; p++) { // scan commands
			// drop from pocket?
			if ((p->_commandType == kCmdSend && p->_val != _now)
				|| p->_commandType == kCmdGive) {
				int ref = p->_ref;
				if (ref < 0)
					ref = spr->_ref;
				if (findActivePocket(ref) >= 0)
					--pocReq;
			}
			// make/dispose additional room?
			if (p->_commandType == kCmdRoom) {
				if (p->_val == 0)
					++pocReq;
				else
					--pocReq;
			}
			// put into pocket?
			if (p->_commandType == kCmdKeep)
				++pocReq;
			// overloaded?
			if (pocReq > pocFre) {
				pocFul();
				return;
			}
		}*/

		while (c < q) {
			if (c->_commandType == kCmdTalk) {
				if ((_commandHandler->_talkEnable = (c->_val != 0)) == false)
					killText();
			}
			if (c->_commandType == kCmdWalk || c->_commandType == kCmdReach) {
				if (c->_val == -1)
					c->_val = spr->_ref;
			}
			if (c->_commandType == kCmdNext) {
				Sprite *s;

				switch (c->_ref) {
				case -2:
					s = hero;
					break;
				case -1:
					s = spr;
					break;
				default:
					s = _vga->_showQ->locate(c->_ref);
					break;
				}

				if (s) {
					if (s->_actionCtrl[snq]._cnt) {
						int v;
						switch (c->_val) {
						case -1:
							v = int(c - comtab + 1);
							break;
						case -2:
							v = int(c - comtab);
							break;
						case -3:
							v = -1;
							break;
						default:
							v = c->_val;
							if ((v > 255) && s)
								v = s->labVal(snq, v >> 8);
							break;
						}
						if (v >= 0)
							s->_actionCtrl[snq]._ptr = v;
					}
				}
				if (s == spr)
					break;
			}
			if (c->_commandType == kCmdIf) {
				Sprite *s = (c->_ref < 0) ? spr : _vga->_showQ->locate(c->_ref);
				if (s) { // sprite extsts 
					if (!s->seqTest(-1)) { // not parked
						int v = c->_val;
						if (v > 255) if (s) v = s->labVal(snq, v >> 8);
						c = comtab + (v - 1);
					}
				}
			} else
				_commandHandler->addCommand(c->_commandType, c->_ref, c->_val, spr);

			++c;
		}
	}
	
}

} // End of namespace CGE2.
