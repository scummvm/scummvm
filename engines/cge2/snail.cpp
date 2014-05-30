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
#include "cge2/text.h"
#include "cge2/sound.h"

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
	if (!_turbo && _vm->_commandStat._wait) {
		if (*(_vm->_commandStat._wait))
			return;
		else {
			++_vm->_commandStat._ref[0];
			warning("STUB: CommandHandler::runCommand() - Sound code missing!");
			_vm->_commandStat._wait = nullptr;
		}
	}

	uint8 tmpHead = _head;
	while (_tail != tmpHead) {
		Command tailCmd = _commandList[_tail];

		if (!_turbo) { // only for the slower one
			if (_vm->_waitRef)
				break;
			if (_timerExpiry) {
				// Delay in progress
				if (_timerExpiry > g_system->getMillis())
					// Delay not yet ended
					break;

				// Delay is finished
				_timerExpiry = 0;
			} else if (_textDelay) {
				if (_vm->_talk) {
					_vm->snKill((Sprite *)_vm->_talk);
					_vm->_talk = nullptr;
				}
				_textDelay = false;
			}
			
			if (_vm->_talk && tailCmd._commandType != kCmdPause)
				break;
		}
		++_tail;
		_vm->_taken = false;
		Sprite *spr = nullptr;
		if (tailCmd._commandType > kCmdSpr)
			spr = (tailCmd._ref < 0) ? ((Sprite *)tailCmd._spritePtr) : _vm->locate(tailCmd._ref);

		switch (tailCmd._commandType) {
		case kCmdUse:
			break;
		case kCmdPause:
			_timerExpiry = g_system->getMillis() + tailCmd._val * kCommandFrameDelay;
			if (_vm->_talk)
				_textDelay = true;
			break;
		case kCmdWait:
			if (spr && spr->active() && (spr->_scene == _vm->_now || spr->_scene == 0)) {
				_vm->_waitSeq = tailCmd._val;
				_vm->_waitRef = spr->_ref;
			}
			break;
		case kCmdHide:
			_vm->snHide(spr, tailCmd._val);
			break;
		case kCmdSay:
			if (spr && spr->active() && _talkEnable) //-- mouth animation
				warning("STUB: CommandHandler::runCommand() - Mouth animation missing!");
			break;
		case kCmdInf:
			if (_talkEnable)
				_vm->inf(((tailCmd._val) >= 0) ? _vm->_text->getText(tailCmd._val) : (const char *)tailCmd._spritePtr);
			break;
		case kCmdTime:
			warning("STUB: CommandHandler::runCommand() - Something missing connected to kCmdTime!");
			break;
		case kCmdCave:
			_vm->switchCave(tailCmd._val);
			break;
		case kCmdMidi:
			_vm->snMidi(tailCmd._val);
			break;
		case kCmdSetDlg:
			_vm->snSetDlg(tailCmd._ref, tailCmd._val);
			break;
		case kCmdMskDlg:
			_vm->snMskDlg(tailCmd._ref, tailCmd._val);
			break;
		case kCmdKill:
			_vm->snKill(spr);
			break;
		case kCmdSeq:
			_vm->snSeq(spr, tailCmd._val);
			break;
		case kCmdRSeq:
			_vm->snRSeq(spr, tailCmd._val);
			break;
		case kCmdSend:
			_vm->snSend(spr, tailCmd._val);
			break;
		case kCmdSwap:
			_vm->snSwap(spr, tailCmd._val);
			break;
		case kCmdCover:
			_vm->snCover(spr, tailCmd._val);
			break;
		case kCmdUncover:
			_vm->snUncover(spr, (tailCmd._val >= 0) ? _vm->locate(tailCmd._val) : ((Sprite *)tailCmd._spritePtr));
			break;
		case kCmdFocus:
			_vm->snFocus(tailCmd._val);
			break;
		case kCmdKeep:
			_vm->snKeep(spr, tailCmd._val);
			break;
		case kCmdGive:
			_vm->snGive(spr, tailCmd._val);
			break;
		case kCmdSetX:
			_vm->_point[tailCmd._val]->_x = tailCmd._ref;
			break;
		case kCmdSetY:
			_vm->_point[tailCmd._val]->_y = tailCmd._ref;
			break;
		case kCmdSetZ:
			_vm->_point[tailCmd._val]->_z = tailCmd._ref;
			break;
		case kCmdAdd:
			*(_vm->_point[tailCmd._ref]) = *(_vm->_point[tailCmd._ref]) + *(_vm->_point[tailCmd._val]);
			break;
		case kCmdSub:
			*(_vm->_point[tailCmd._ref]) = *(_vm->_point[tailCmd._ref]) - *(_vm->_point[tailCmd._val]);
			break;
		case kCmdMul:
			*(_vm->_point[tailCmd._ref]) = *(_vm->_point[tailCmd._ref]) * tailCmd._val;
			break;
		case kCmdDiv:
			*(_vm->_point[tailCmd._ref]) = *(_vm->_point[tailCmd._ref]) / tailCmd._val;
			break;
		case kCmdGetPos:
			if (spr)
				*(_vm->_point[tailCmd._val]) = spr->_pos3D;
			break;
		case kCmdGoto:
			_vm->snGoto(spr, tailCmd._val);
			break;
		case kCmdMoveX:
			_vm->snMove(spr, V3D(tailCmd._val, 0, 0));
			break;
		case kCmdMoveY:
			_vm->snMove(spr, V3D(0, tailCmd._val, 0));
			break;
		case kCmdMoveZ:
			_vm->snMove(spr, V3D(0, 0, tailCmd._val));
			break;
		case kCmdSlave:
			_vm->snSlave(spr, tailCmd._val);
			break;
		case kCmdTrans:
			_vm->snTrans(spr, tailCmd._val);
			break;
		case kCmdPort:
			_vm->snPort(spr, tailCmd._val);
			break;
		case kCmdNext:
			break;
		case kCmdIf:
			break;
		case kCmdTalk:
			break;
		case kCmdMouse:
			_vm->snMouse(tailCmd._val != 0);
			break;
		case kCmdNNext:
			_vm->snNNext(spr, kNear, tailCmd._val);
			break;
		case kCmdMTNext:
			_vm->snNNext(spr, kMTake, tailCmd._val);
			break;
		case kCmdFTNext:
			_vm->snNNext(spr, kFTake, tailCmd._val);
			break;
		case kCmdRNNext:
			_vm->snRNNext(spr, tailCmd._val);
			break;
		case kCmdRMTNext:
			_vm->snRMTNext(spr, tailCmd._val);
			break;
		case kCmdRFTNext:
			_vm->snRFTNext(spr, tailCmd._val);
			break;
		case kCmdRMNear:
			_vm->snRmNear(spr);
			break;
		case kCmdRMMTake:
			_vm->snRmMTake(spr);
			break;
		case kCmdRMFTake:
			_vm->snRmFTake(spr);
			break;
		case kCmdFlag:
			_vm->snFlag(tailCmd._ref & 3, tailCmd._val);
			break;
		case kCmdSetRef:
			_vm->snSetRef(spr, tailCmd._val);
			break;
		case kCmdBackPt:
			_vm->snBackPt(spr, tailCmd._val);
			break;
		case kCmdFlash:
			_vm->snFlash(tailCmd._val != 0);
			break;
		case kCmdLight:
			_vm->snLight(tailCmd._val != 0);
			break;
		case kCmdCycle:
			warning("Unhandled command - kCmdCycle");
			break;
		case kCmdWalk:
			_vm->snWalk(spr, tailCmd._val);
			break;
		case kCmdReach:
			_vm->snReach(spr, tailCmd._val);
			break;
		case kCmdSound:
			_vm->snSound(spr, tailCmd._val);
			_count = 1;
			break;
		case kCmdMap:
			_vm->_heroTab[tailCmd._ref & 1]->_ptr->_ignoreMap = tailCmd._val == 0;
			break;
		case kCmdCount:
			_count = tailCmd._val;
			break;
		case kCmdRoom:
			_vm->snRoom(spr, tailCmd._val);
			break;
		case kCmdDim:
			warning("Unhandled command - kCmdDim");
			break;
		case kCmdExec:
			warning("Unhandled command - kCmdExec");
			break;
		case kCmdStep:
			spr->step();
			break;
		case kCmdGhost:
			_vm->snGhost((Bitmap *)tailCmd._spritePtr);
			break;
		default:
			warning("Unhandled command");
			break;
		}

		if (_vm->_taken && spr)
			_vm->_spare->dispose(spr);

		if (!_turbo)
			break;
	}
}

void CGE2Engine::snKill(Sprite *spr) {
	warning("STUB: CGE2Engine::snKill()");
}

void CGE2Engine::snHide(Sprite *spr, int val) {
	if (spr) {
		spr->_flags._hide = (val >= 0) ? (val != 0) : (!spr->_flags._hide);
		if (spr->_flags._shad)
			spr->_prev->_flags._hide = spr->_flags._hide;
	}
}

void CGE2Engine::snMidi(int val) {
	warning("STUB: CGE2Engine::snMidi()");
}

void CGE2Engine::snSetDlg(int clr, int set) {
	warning("STUB: CGE2Engine::snSetDlg()");
}

void CGE2Engine::snMskDlg(int clr, int set) {
	warning("STUB: CGE2Engine::snMskDlg()");
}

void CGE2Engine::snSeq(Sprite *spr, int val) {
	if (spr) {
		if (isHero(spr) && val == 0)
			((Hero*)spr)->park();
		else
			spr->step(val);
	}
}

void CGE2Engine::snRSeq(Sprite *spr, int val) {
	if (spr)
		snSeq(spr, spr->_seqPtr + val);
}

void CGE2Engine::snSend(Sprite *spr, int val) {
	if (!spr)
		return;
	
	// Sending", spr->File
	// from cave", spr->Cave
	// to cave", val
	bool was1 = (_vga->_showQ->locate(spr->_ref) != nullptr);
	bool val1 = (val == 0 || val == _now);
	spr->_scene = val;
	releasePocket(spr);
	if (val1 != was1) {
		if (was1) {
			// deactivating
			hide1(spr);
			spr->_flags._slav = false;
			if (spr == _heroTab[_sex]->_ptr)
				if (_heroTab[!_sex]->_ptr->_scene == _now)
					switchHero(!_sex);
			_spare->dispose(spr);
		} else {
			// activating
			if (byte(spr->_ref) == 0)
				_bitmapPalette = _vga->_sysPal;
			_vga->_showQ->insert(spr);
			if (isHero(spr)) {
				V2D p = *_heroTab[spr->_ref & 1]->_posTab[val];
				spr->gotoxyz(V3D(p.x, 0, p.y));
				((Hero*)spr)->setCurrent();
			}
			_taken = false;
			_bitmapPalette = NULL;
		}
	}
}

void CGE2Engine::snSwap(Sprite *spr, int val) {
	warning("STUB: CGE2Engine::snSwap()");
}
void CGE2Engine::snCover(Sprite *spr, int val) {
	warning("STUB: CGE2Engine::snCover()");
}

void CGE2Engine::snUncover(Sprite *spr, Sprite *spr2) {
	warning("STUB: CGE2Engine::snUncover()");
}

void CGE2Engine::snFocus(int val) {
	warning("STUB: CGE2Engine::snFocus()");
}

void CGE2Engine::snKeep(Sprite *spr, int val) {
	warning("STUB: CGE2Engine::snKeep()");
}

void CGE2Engine::snGive(Sprite *spr, int val) {
	warning("STUB: CGE2Engine::snGive()");
}

void CGE2Engine::snGoto(Sprite *spr, int val) {
	warning("STUB: CGE2Engine::snGoto()");
}

void CGE2Engine::snMove(Sprite *spr, V3D pos) {
	warning("STUB: CGE2Engine::snMove()");
}

void CGE2Engine::snSlave(Sprite *spr, int val) {
	warning("STUB: CGE2Engine::snSlave()");
}

void CGE2Engine::snTrans(Sprite *spr, int val) {
	warning("STUB: CGE2Engine::snTrans()");
}

void CGE2Engine::snPort(Sprite *spr, int val) {
	warning("STUB: CGE2Engine::snPort()");
}

void CGE2Engine::snMouse(int val) {
	warning("STUB: CGE2Engine::snMouse()");
}

void CGE2Engine::snNNext(Sprite *spr, Action act, int val) {
	warning("STUB: CGE2Engine::snNNext()");
}

void CGE2Engine::snRNNext(Sprite *spr, int val) {
	warning("STUB: CGE2Engine::snRNNext()");
}

void CGE2Engine::snRMTNext(Sprite *spr, int val) {
	warning("STUB: CGE2Engine::snRMTNext()");
}

void CGE2Engine::snRFTNext(Sprite *spr, int val) {
	warning("STUB: CGE2Engine::snRFTNext()");
}

void CGE2Engine::snRmNear(Sprite *spr) {
	warning("STUB: CGE2Engine::snRmNear()");
}

void CGE2Engine::snRmMTake(Sprite *spr) {
	warning("STUB: CGE2Engine::snRmMTake()");
}

void CGE2Engine::snRmFTake(Sprite *spr) {
	warning("STUB: CGE2Engine::snRmFTake()");
}

void CGE2Engine::snFlag(int ref, int val) {
	warning("STUB: CGE2Engine::snFlag()");
}

void CGE2Engine::snSetRef(Sprite *spr, int val) {
	warning("STUB: CGE2Engine::snSetRef()");
}

void CGE2Engine::snBackPt(Sprite *spr, int val) {
	warning("STUB: CGE2Engine::snBackPt()");
}

void CGE2Engine::snFlash(int val) {
	warning("STUB: CGE2Engine::snFlash()");
}

void CGE2Engine::snLight(int val) {
	warning("STUB: CGE2Engine::snLight()");
}

void CGE2Engine::snWalk(Sprite *spr, int val) {
	warning("STUB: CGE2Engine::snWalk()");
}

void CGE2Engine::snReach(Sprite *spr, int val) {
	warning("STUB: CGE2Engine::snReach()");
}

void CGE2Engine::snSound(Sprite *spr, int wav) {
	if (wav == -1)
		_sound->stop();
	else {
		if (_sound->_smpinf._counter && wav < 20)
			return;
		if (_commandStat._wait && ((wav & 255) > 80))
			return;

		_commandStat._ref[1] = wav;
		_commandStat._ref[0] = !_fx->exist(_commandStat._ref[1]);
		_sound->play(_fx->load(_commandStat._ref[1], _commandStat._ref[0]),
			(spr) ? (spr->_pos2D.x / (kScrWidth / 16)) : 8);
	}
	
}

void CGE2Engine::snRoom(Sprite *spr, int val) {
	warning("STUB: CGE2Engine::snRoom()");
}

void CGE2Engine::snGhost(Bitmap *bmp) {
	V2D p(this, *bmp->_v & 0xFFFF, *bmp->_v >> 16);
	bmp->hide(p.x, p.y);
	delete bmp->_b;
	bmp->_v = nullptr;
	bmp->_b = nullptr;
	delete bmp;
}

void CGE2Engine::hide1(Sprite *spr) {
	_commandHandlerTurbo->addCommand(kCmdGhost, -1, 0, spr->ghost());
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
	return (!_vm->_waitRef && _head == _tail);
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
		CommandHandler::Command *q = &comtab[cnt];

		warning("STUB: CGE2Engine::feedSnail()");
		// Dont bother with pockets (inventory system) for now... TODO: Implement it later!!!
		/*
		int pocFre = freePockets(hero->_ref & 1);
		int pocReq = 0;
		CommandHandler::Command *p = c;
		for (; p < q && p->_commandType != kCmdNext; p++) { // scan commands
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
