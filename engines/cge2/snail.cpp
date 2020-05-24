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
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#include "cge2/snail.h"
#include "cge2/fileio.h"
#include "cge2/hero.h"
#include "cge2/text.h"
#include "cge2/sound.h"
#include "cge2/events.h"

namespace CGE2 {

const char *CommandHandler::_commandText[] = {
	"NOP", "USE", "PAUSE", "INF", "CAVE", "SETX", "SETY", "SETZ", "ADD",
	"FLASH", "CYCLE", "CLEAR", "MOUSE", "MAP", "MIDI", ".DUMMY.", "WAIT",
	"HIDE", "ROOM", "SAY", "SOUND", "KILL", "RSEQ", "SEQ", "SEND", "SWAP",
	"KEEP", "GIVE", "GETPOS", "GOTO", "PORT", "NEXT", "NNEXT", "MTNEXT",
	"FTNEXT", "RNNEXT", "RMTNEXT", "RFTNEXT", "RMNEAR", "RMMTAKE", "RMFTAKE",
	"SETREF", "WALKTO", "REACH", "COVER", "UNCOVER", "EXEC", "GHOST",
	nullptr };

CommandHandler::CommandHandler(CGE2Engine *vm, bool turbo)
	: _turbo(turbo), _textDelay(false), _timerExpiry(0), _talkEnable(true),
      _head(0), _tail(0), _commandList((Command *)malloc(sizeof(Command)* 256)),
      _vm(vm) {
}

CommandHandler::~CommandHandler() {
	free(_commandList);
}

void CommandHandler::runCommand() {
	if (!_turbo && _vm->_soundStat._wait) {
		if (*(_vm->_soundStat._wait))
			return;

		++_vm->_soundStat._ref[0];
		if (_vm->_fx->exist(_vm->_soundStat._ref[1], _vm->_soundStat._ref[0])) {
			int16 oldRepeat = _vm->_sound->getRepeat();
			_vm->_sound->setRepeat(1);
			_vm->_sound->play(Audio::Mixer::kSpeechSoundType, _vm->_fx->load(_vm->_soundStat._ref[1], _vm->_soundStat._ref[0]), _vm->_sound->_smpinf._span);
			_vm->_sound->setRepeat(oldRepeat);
			return;
		}
		_vm->_soundStat._wait = nullptr;
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

		Common::String sprStr;
		if (spr && *spr->_file && (tailCmd._commandType != kCmdGhost))
			// In case of kCmdGhost _spritePtr stores a pointer to a Bitmap, not to a Sprite...
			sprStr = Common::String(spr->_file);
		else
			sprStr = "None";

		if (sprStr.empty())
			sprStr = "None";
		debugC(1, kCGE2DebugOpcode, "Command: %s; Ref: %d; Val: %d; Sprite: %s;", getComStr(tailCmd._commandType), tailCmd._ref, tailCmd._val, sprStr.c_str());

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
			_vm->snSay(spr, tailCmd._val);
			break;
		case kCmdInf:
			if (_talkEnable)
				_vm->inf(((tailCmd._val) >= 0) ? _vm->_text->getText(tailCmd._val) : (const char *)tailCmd._spritePtr);
			break;
		case kCmdCave:
			_vm->switchScene(tailCmd._val);
			break;
		case kCmdMidi:
			_vm->snMidi(tailCmd._val);
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
		case kCmdGetPos:
			if (spr)
				*(_vm->_point[tailCmd._val]) = spr->_pos3D;
			break;
		case kCmdGoto:
			_vm->snGoto(spr, tailCmd._val);
			break;
		case kCmdPort:
			_vm->snPort(spr, tailCmd._val);
			break;
		case kCmdNext:
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
		case kCmdSetRef:
			_vm->snSetRef(spr, tailCmd._val);
			break;
		case kCmdFlash:
			_vm->snFlash(tailCmd._val != 0);
			break;
		case kCmdCycle:
			_vm->snCycle(tailCmd._val);
			break;
		case kCmdWalk:
			_vm->snWalk(spr, tailCmd._val);
			break;
		case kCmdReach:
			_vm->snReach(spr, tailCmd._val);
			break;
		case kCmdSound:
			_vm->snSound(spr, tailCmd._val);
			_vm->_sound->setRepeat(1);
			break;
		case kCmdMap:
			_vm->_heroTab[tailCmd._ref & 1]->_ptr->_ignoreMap = tailCmd._val == 0;
			break;
		case kCmdRoom:
			_vm->snRoom(spr, tailCmd._val);
			break;
		case kCmdExec:
			switch (tailCmd._cbType) {
			case kQGame:
				_vm->qGame();
				break;
			case kXScene:
				_vm->xScene();
				break;
			default:
				error("Unknown Callback Type in SNEXEC");
				break;
			}
			break;
		case kCmdGhost:
			_vm->snGhost((Bitmap *)tailCmd._spritePtr);
			break;
		case kCmdNop: // Do nothing.
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
	if (spr) {
		if (spr->_flags._kept)
			releasePocket(spr);
		Sprite *nx = spr->_next;
		hide1(spr);
		_vga->_showQ->remove(spr);
		_eventManager->clearEvent(spr);
		if (spr->_flags._kill) {
			_spare->take(spr->_ref);
			delete spr;
		} else {
			spr->setScene(-1);
			_spare->dispose(spr);
		}
		if (nx && nx->_flags._slav)
			snKill(nx);
	}
}

void CGE2Engine::snHide(Sprite *spr, int val) {
	if (spr) {
		spr->_flags._hide = (val >= 0) ? (val != 0) : (!spr->_flags._hide);
		if (spr->_flags._shad)
			spr->_prev->_flags._hide = spr->_flags._hide;
	}
}

void CGE2Engine::snMidi(int val) {
	if (val < 0)
		_midiPlayer->killMidi();
	else if (_music)
		_midiPlayer->loadMidi(val);
}

void CGE2Engine::snSeq(Sprite *spr, int val) {
	if (spr) {
		if (isHero(spr) && (val == 0))
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

	// Sending", spr->_file
	// from scene", spr->_scene
	// to scene", val
	bool was1 = (_vga->_showQ->locate(spr->_ref) != nullptr);
	bool val1 = (val == 0 || val == _now);
	spr->_scene = val;
	releasePocket(spr);
	if (val1 != was1) {
		if (was1) {
			// deactivating
			hide1(spr);
			spr->_flags._slav = false;
			if ((spr == _heroTab[_sex]->_ptr) && (_heroTab[!_sex]->_ptr->_scene == _now))
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
			_bitmapPalette = nullptr;
		}
	}
}

void CGE2Engine::snSwap(Sprite *spr, int val) {
	bool tak = _taken;
	Sprite *xspr = locate(val);
	if (spr && xspr) {
		bool was1 = (_vga->_showQ->locate(spr->_ref) != nullptr);
		bool xwas1 = (_vga->_showQ->locate(val) != nullptr);

		int tmp = spr->_scene;
		spr->setScene(xspr->_scene);
		xspr->setScene(tmp);

		SWAP(spr->_pos2D, xspr->_pos2D);
		SWAP(spr->_pos3D, xspr->_pos3D);
		if (spr->_flags._kept)
			swapInPocket(spr, xspr);
		if (xwas1 != was1) {
			if (was1) {
				hide1(spr);
				_spare->dispose(spr);
			} else
				expandSprite(spr);
			if (xwas1) {
				hide1(xspr);
				_spare->dispose(xspr);
			} else {
				expandSprite(xspr);
				_taken = false;
			}
		}
	}
	if (_taken)
		_spare->dispose(xspr);
	_taken = tak;
}

void CGE2Engine::snCover(Sprite *spr, int val) {
	bool tak = _taken;
	Sprite *xspr = locate(val);
	if (spr && xspr) {
		spr->_flags._hide = true;
		xspr->setScene(spr->_scene);
		xspr->gotoxyz(spr->_pos3D);
		expandSprite(xspr);
		if ((xspr->_flags._shad = spr->_flags._shad) == true) {
			_vga->_showQ->insert(_vga->_showQ->remove(spr->_prev), xspr);
			spr->_flags._shad = false;
		}
		feedSnail(xspr, kNear, _heroTab[_sex]->_ptr);
		_taken = false;
	}
	if (_taken)
		_spare->dispose(xspr);
	_taken = tak;
}

void CGE2Engine::snUncover(Sprite *spr, Sprite *spr2) {
	if (spr && spr2) {
		spr->_flags._hide = false;
		spr->setScene(spr2->_scene);
		if ((spr->_flags._shad = spr2->_flags._shad) == true) {
			_vga->_showQ->insert(_vga->_showQ->remove(spr2->_prev), spr);
			spr2->_flags._shad = false;
		}
		spr->gotoxyz(spr2->_pos3D);
		snSend(spr2, -1);
		if (spr->_time == 0)
			++spr->_time;
	}
}

void CGE2Engine::snKeep(Sprite *spr, int stp) {
	int sex = _sex;
	if (stp > 127) {
		_sex = stp & 1; // for another hero
		stp = -1;
	}
	HeroTab *ht = _heroTab[_sex];
	selectPocket(-1);
	int pp = ht->_pocPtr;

	if (spr && !spr->_flags._kept && ht->_pocket[pp] == nullptr) {
		V3D pos(14, -10, -1);
		int16 oldRepeat = _sound->getRepeat();
		_sound->setRepeat(1);
		snSound(ht->_ptr, 3);
		_sound->setRepeat(oldRepeat);
		if (_taken) {
			_vga->_showQ->insert(spr);
			_taken = false;
		}
		ht->_pocket[pp] = spr;
		spr->setScene(0);
		spr->_flags._kept = true;
		if (!_sex)
			pos._x += kScrWidth - 58;
		if (pp & 1)
			pos._x += 29;
		if (pp >> 1)
			pos._y -= 20;
		pos._y -= (spr->_siz.y / 2);
		spr->gotoxyz(pos);
		if (stp >= 0)
			spr->step(stp);
	}
	_sex = sex;
	selectPocket(-1);
}

void CGE2Engine::snGive(Sprite *spr, int val) {
	if (spr) {
		int p = findActivePocket(spr->_ref);
		if (p >= 0) {
			releasePocket(spr);
			spr->setScene(_now);
			if (val >= 0)
				spr->step(val);
		}
	}
	selectPocket(-1);
}

void CGE2Engine::snGoto(Sprite *spr, int val) {
	if (spr) {
		V3D eye = *_eye;
		if (spr->_scene > 0)
			setEye(*_eyeTab[spr->_scene]);
		spr->gotoxyz(*_point[val]);
		setEye(eye);
	}
}

void CGE2Engine::snPort(Sprite *spr, int port) {
	if (spr)
		spr->_flags._port = (port < 0) ? !spr->_flags._port : (port != 0);
}

void CGE2Engine::snMouse(bool on) {
	if (on)
		_mouse->on();
	else
		_mouse->off();
}

void CGE2Engine::snNNext(Sprite *spr, Action act, int val) {
	if (spr) {
		if (val > 255)
			val = spr->labVal(act, val >> 8);
		spr->_actionCtrl[act]._ptr = val;
	}
}

void CGE2Engine::snRNNext(Sprite *spr, int val) {
	if (spr)
		spr->_actionCtrl[kNear]._ptr += val;
}

void CGE2Engine::snRMTNext(Sprite *spr, int val) {
	if (spr)
		spr->_actionCtrl[kMTake]._ptr += val;
}

void CGE2Engine::snRFTNext(Sprite * spr, int val) {
	if (spr)
		spr->_actionCtrl[kFTake]._ptr += val;
}

void CGE2Engine::snRmNear(Sprite *spr) {
	if (spr)
		spr->_actionCtrl[kNear]._cnt = 0;
}

void CGE2Engine::snRmMTake(Sprite *spr) {
	if (spr)
		spr->_actionCtrl[kMTake]._cnt = 0;
}

void CGE2Engine::snRmFTake(Sprite *spr) {
	if (spr)
		spr->_actionCtrl[kFTake]._cnt = 0;
}

void CGE2Engine::snSetRef(Sprite *spr, int val) {
	if (spr)
		spr->_ref = val;
}

void CGE2Engine::snFlash(bool on) {
	if (on) {
		Dac *pal = (Dac *)malloc(sizeof(Dac) * kPalCount);
		if (pal) {
			memcpy(pal, _vga->_sysPal, kPalSize);
			for (int i = 0; i < kPalCount; i++) {
				int c;
				c = pal[i]._r << 1;
				pal[i]._r = (c < 64) ? c : 63;
				c = pal[i]._g << 1;
				pal[i]._g = (c < 64) ? c : 63;
				c = pal[i]._b << 1;
				pal[i]._b = (c < 64) ? c : 63;
			}
			_vga->setColors(pal, 64);
		}

		free(pal);
	} else
		_vga->setColors(_vga->_sysPal, 64);
	_dark = false;
}

void CGE2Engine::snCycle(int cnt) {
	_vga->_rot._len = cnt;
}

void CGE2Engine::snWalk(Sprite *spr, int val) {
	if (isHero(spr)) {
		if (val < kMaxPoint)
			((Hero *)spr)->walkTo(*_point[val]);
		else {
			Sprite *s = _vga->_showQ->locate(val);
			if (s)
				((Hero *)spr)->walkTo(s);
		}
		((Hero *)spr)->_time = 1;
	}
}

void CGE2Engine::snReach(Sprite *spr, int val) {
	if (isHero(spr))
		((Hero *)spr)->reach(val);
}

void CGE2Engine::snSound(Sprite *spr, int wav, Audio::Mixer::SoundType soundType) {
	if (wav == -1)
		_sound->stop();
	else {
		if (_sound->_smpinf._counter && wav < 20)
			return;
		if (_soundStat._wait && ((wav & 255) > 80))
			return;

		_soundStat._ref[1] = wav;
		_soundStat._ref[0] = !_fx->exist(_soundStat._ref[1]);
		_sound->play(soundType, _fx->load(_soundStat._ref[1], _soundStat._ref[0]),
			(spr) ? (spr->_pos2D.x / (kScrWidth / 16)) : 8);
	}
}

void CGE2Engine::snRoom(Sprite *spr, bool on) {
	if (!isHero(spr))
		return;

	int sex = spr->_ref & 1;
	Sprite **p = _heroTab[sex]->_pocket;
	if (on) {
		if (freePockets(sex) == 0 && p[kPocketMax] == nullptr) {
			SWAP(p[kPocketMax], p[kPocketMax - 1]);
			snHide(p[kPocketMax], 1);
		}
	} else if (p[kPocketMax]) {
		for (int i = 0; i < kPocketMax; i++) {
			if (p[i] == nullptr) {
				snHide(p[kPocketMax], 0);
				SWAP(p[kPocketMax], p[i]);
				break;
			}
		}
	}
}

void CGE2Engine::snGhost(Bitmap *bmp) {
	V2D p(this, bmp->_map & 0xFFFF, bmp->_map >> 16);
	bmp->hide(p);
	bmp->release();
	delete[] bmp->_b;
	bmp->_b = nullptr;
	delete bmp;
	bmp = nullptr;
}

void CGE2Engine::snSay(Sprite *spr, int val) {
	if (spr && spr->active() && _commandHandler->_talkEnable) {
		//-- mouth animation
		if (isHero(spr) && spr->seqTest(-1))
			((Hero *)spr)->say();
		if (_sayCap)
			_text->say(_text->getText(val), spr);
		if (_sayVox) {
			int i = val;
			if (i < 256)
				i -= 100;
			int16 oldRepeat = _sound->getRepeat();
			_sound->setRepeat(1);
			snSound(spr, i, Audio::Mixer::kSpeechSoundType);
			_sound->setRepeat(oldRepeat);
			_soundStat._wait = &_sound->_smpinf._counter;
		}
	}
}

void CGE2Engine::hide1(Sprite *spr) {
	_commandHandlerTurbo->addCommand(kCmdGhost, -1, 0, spr->ghost());
}

void CGE2Engine::swapInPocket(Sprite *spr, Sprite *xspr) {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < kPocketMax; j++) {
			Sprite *&poc = _heroTab[i]->_pocket[j];
			if (poc == spr) {
				spr->_flags._kept = false;
				poc = xspr;
				xspr->_flags._kept = true;
				xspr->_flags._port = false;
				return;
			}
		}
	}
}

Sprite *CGE2Engine::expandSprite(Sprite *spr) {
	if (spr)
		_vga->_showQ->insert(spr);
	return spr;
}

void CGE2Engine::qGame() {
	// Write out the user's progress
	saveGame(0, Common::String("Automatic Savegame"));

	busy(false);
	_vga->sunset();
	_endGame = true;
}

void CGE2Engine::xScene() {
	sceneDown();
	sceneUp(_req);
}

void CommandHandler::addCommand(CommandType com, int ref, int val, void *ptr) {
	if (ref == -2)
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
	Command *headCmd = &_commandList[_head++];
	headCmd->_commandType = com;
	headCmd->_ref = ref;
	headCmd->_val = val;
	headCmd->_spritePtr = nullptr;
	headCmd->_cbType = cbType;
	if (headCmd->_commandType == kCmdClear) {
		_tail = _head;
		_vm->killText();
		_timerExpiry = 0;
	}
}

void CommandHandler::insertCommand(CommandType com, int ref, int val, void *ptr) {
	if (ref == -2)
		ref = 142 - _vm->_sex;
	--_tail;
	Command *tailCmd = &_commandList[_tail];
	tailCmd->_commandType = com;
	tailCmd->_ref = ref;
	tailCmd->_val = val;
	tailCmd->_spritePtr = ptr;
	tailCmd->_cbType = kNullCB;
	if (com == kCmdClear) {
		_tail = _head;
		_vm->killText();
		_timerExpiry = 0;
	}
}

bool CommandHandler::idle() {
	return (!_vm->_waitRef && _head == _tail);
}

void CommandHandler::clear() {
	_tail = _head;
	_vm->killText();
	_timerExpiry = 0;
}

int CommandHandler::getComId(const char *com) {
	int i = _vm->takeEnum(_commandText, com);
	return (i < 0) ? i : i + kCmdCom0 + 1;
}

const char *CommandHandler::getComStr(CommandType cmdType) {
	return _commandText[cmdType - kCmdNop];
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

		if (hero != nullptr) {
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
			}
		}

		while (c < q) {
			if ((c->_val == -1) && (c->_commandType == kCmdWalk || c->_commandType == kCmdReach))
				c->_val = spr->_ref;

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

				if (s && s->_actionCtrl[snq]._cnt) {
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
					if (v >= 0) {
						s->_actionCtrl[snq]._ptr = v;
						if (spr->_ref == 1537 && s->_actionCtrl[snq]._ptr == 26)
						{
							debug(1, "Carpet Clothes Horse Rehanging Workaround Triggered!");
							s->_actionCtrl[snq]._ptr = 8;
						}
					}
				}

				if (s == spr)
					break;
			}

			_commandHandler->addCommand(c->_commandType, c->_ref, c->_val, spr);

			++c;
		}
	}

}

} // End of namespace CGE2.
