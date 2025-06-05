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


#include "snatcher/snatcher.h"
#include "snatcher/graphics.h"
#include "snatcher/sound.h"
#include "snatcher/resource.h"
#include "snatcher/scene_imp.h"

namespace Snatcher {

SH_HEAD_BEGIN(D2)
// declarations
SH_DCL_FRM(00)
SH_DCL_FRM(01)
SH_DCL_FRM(02)
SH_DCL_FRM(03)
SH_DCL_FRM(04)
SH_DCL_FRM(05)
SH_DCL_FRM(06)
SH_DCL_FRM(07)
SH_DCL_FRM(08)
SH_DCL_FRM(09)
SH_DCL_FRM(10)
SH_DCL_FRM(11)
SH_DCL_FRM(12)

// extra functions
void updateAnimBlink();

// local vars
int16 _option;
uint16 _counter2;
bool _loadCancelled;

SH_HEAD_END(D2)

SH_IMP_FRMTBL(D2) {
	SH_FRM(D2, 00),
	SH_FRM(D2, 01),
	SH_FRM(D2, 02),
	SH_FRM(D2, 03),
	SH_FRM(D2, 04),
	SH_FRM(D2, 05),
	SH_FRM(D2, 06),
	SH_FRM(D2, 07),
	SH_FRM(D2, 08),
	SH_FRM(D2, 09),
	SH_FRM(D2, 10),
	SH_FRM(D2, 11),
	SH_FRM(D2, 12)
};

SH_IMP_CTOR(D2), _option(0), _counter2(0), _loadCancelled(false) {
	SH_CTOR_MAKEPROCS(D2);
}

SH_IMP_DTOR(D2)	{
	SH_DTOR_DELPROCS(D2);
}

SH_IMPL_UPDT(D2) {
	if (state.frameNo >= (int)_frameProcs.size() || !_frameProcs[state.frameNo]->isValid())
		error("%s(): Invalid call to frame proc %d", __FUNCTION__, state.frameNo);
	else
		(*_frameProcs[state.frameNo])(state);
}

int _hasSaveSlotFlag = 15;
int _saveFileCurID = 0;
uint8 _buram_0 = 0;
bool _buram_2 = true;

// functions
SH_IMPL_FRM(D2, 00) {
	switch (state.frameState) {
	case 0:
		_vm->sound()->fmSendCommand(state.conf.disableStereo ? 243 : 244, 0);
		_vm->sound()->pcmSendCommand(state.conf.disableStereo ? 252 : 253, -1);
		//_d2state = 0;
		//buramCheck();
		++state.frameState;
		break;
	case 1:
		if (state.conf.hasRAMCart) {
			++state.frameNo;
			state.frameState = 0;
		} else {
			_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
			_vm->gfx()->runScript(_module->getGfxData(), 1); // RAM Cart select screen
			++state.frameState;
		}
		break;
	case 2:
		if (_vm->input().controllerFlags & 3) {
			_vm->gfx()->clearAnimParameterFlags(16, GraphicsEngine::kAnimParaControlFlags, ~GraphicsEngine::kAnimHide);
			_buram_0 ^= 1;
		}
		if (_vm->input().controllerFlags & 0x80) {
			state.counter = 10;
			_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x290F2));
			++state.frameState;
		}
		break;
	default:
		if (--state.counter)
			return;
		if (_buram_0)
			state.conf.hasRAMCart = _buram_2;
		++state.frameNo;
		state.frameState = 0;
		break;
	}
}

SH_IMPL_FRM(D2, 01) {
	if (!state.frameState) {
		_vm->sound()->cdaPlay(2);
		++state.frameState;
	} else if (_vm->sound()->cdaIsPlaying()) {
		++state.frameNo;
		state.frameState = 0;
	}	
}

SH_IMPL_FRM(D2, 02) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->transitionCommand(1);
	_vm->gfx()->setScrollStep(GraphicsEngine::kVertA | GraphicsEngine::kSingleStep, 0x100);
	_vm->gfx()->runScript(_module->getGfxData(), 0);
	state.counter = 24;
	++state.frameNo;
	state.frameState = 0;
}

SH_IMPL_FRM(D2, 03) {
	if (--state.counter)
		return;
	_vm->gfx()->transitionCommand(7);
	state.counter = 4;
	++state.frameNo;
	state.frameState = 0;
}

SH_IMPL_FRM(D2, 04) {
	if (--state.counter)
		return;
	_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x288E6));
	state.counter = 86;
	++state.frameNo;
	state.frameState = 0;
}

SH_IMPL_FRM(D2, 05) {
	if (--state.counter)
		return;
	++state.frameNo;
	state.frameState = 0;
}

SH_IMPL_FRM(D2, 06) {
	switch (state.frameState) {
	case 0:
		if (state.conf.hasRAMCart) {
			++state.frameNo;
			state.frameState = 0;
		} else {
			_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x28944));
			++state.frameState;
			state.counter = 17;
		}
		break;
	case 1:
		if (--state.counter)
			return;
		_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
		_vm->gfx()->runScript(_module->getGfxData(), 2);
		++state.frameState;
		break;
	default:
		//if (_vm->_commandsFromMain & 0x80)
		/**/
		break;
	}
}

SH_IMPL_FRM(D2, 07) {
	switch (state.frameState) {
	case 0:
		// loadSaveFile();
		++state.frameState;
		break;
	default:
		++state.frameNo;
		if (_hasSaveSlotFlag)
			++state.frameNo;
		state.frameState = 0;
		break;
	}
}

SH_IMPL_FRM(D2, 08) {
	switch (state.frameState) {
	case 0:
		// buramCheckSpace();
		++state.frameState;
		break;
	case 1:
		if (state.conf.hasFreeBuram) {
			++state.frameNo;
			state.frameState = 0;
		} else {
			_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x28944));
			state.counter = 17;
			++state.frameState;
		}
		break;
	case 2:
		if (--state.counter)
			return;
		_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
		_vm->gfx()->runScript(_module->getGfxData(), 3);
		++state.frameState;
		break;
	default:
		break;
	}
}

SH_IMPL_FRM(D2, 09) {
	bool fin = false;
	switch (state.frameState) {
	case 0:
		_vm->gfx()->transitionCommand(0xFF);
		_vm->gfx()->clearAnimParameterFlags(_hasSaveSlotFlag ? 17 : 16, GraphicsEngine::kAnimParaControlFlags, ~GraphicsEngine::kAnimHide);
		state.frameState += (_hasSaveSlotFlag ? 1 : 2);
		break;
	case 1:
		if (_vm->input().controllerFlags & 3)
			_vm->gfx()->clearAnimParameterFlags(17, GraphicsEngine::kAnimParaControlFlags, ~GraphicsEngine::kAnimHide);
		if (_vm->input().controllerFlags & 0x80) {
			_vm->gfx()->setAnimParameter(17, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
			state.menuSelect = 1;
			if (_vm->gfx()->getAnimParameter(17, GraphicsEngine::kAnimParaFrame) == 3) {
				state.menuSelect = 0;
				++state.frameNo;
			}
			fin = true;
		}
		break;
	default:
		if (_vm->input().controllerFlags & 0x80) {
			++state.frameNo;
			state.frameState = 0;
			fin = true;
		}
		break;
	}
	if (fin) {
		++state.frameNo;
		state.frameState = 0;
		_vm->sound()->fmSendCommand(56, 0, 2);
		_vm->gfx()->setAnimParameter(16, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
		_vm->gfx()->setAnimParameter(17, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
		_option = 0;
	}
}

SH_IMPL_FRM(D2, 10) {
	bool fin = false;
	switch (state.frameState) {
	case 0:
		if (state.counter == 3) {

		}
		if (state.counter != 21) {;
			_vm->gfx()->enqueueDrawCommands(_module->getPtr(0x29142).getDataFromTable(state.counter));
			if (state.counter != 0)
				_vm->gfx()->enqueueDrawCommands(_module->getPtr(0x29412).getDataFromTable(state.counter));
			++state.counter;
			return;
		}
		_vm->gfx()->clearAnimParameterFlags(24, GraphicsEngine::kAnimParaControlFlags, 0xFF);
		_vm->gfx()->clearAnimParameterFlags(25, GraphicsEngine::kAnimParaControlFlags, 0xFF);		

		do {
			if (_vm->input().controllerFlags & 1)
				--_saveFileCurID;
			if (_vm->input().controllerFlags & 2)
				++_saveFileCurID;
			_saveFileCurID &= 3;
		} while (!(_hasSaveSlotFlag & (1 << _saveFileCurID)));

		_vm->gfx()->setAnimParameter(24, GraphicsEngine::kAnimParaFrame, _saveFileCurID << 2);
		_vm->gfx()->setAnimParameter(25, GraphicsEngine::kAnimParaFrame, _saveFileCurID << 2);

		if (_vm->input().controllerFlags & 0x80) {
			_vm->sound()->fmSendCommand(56, 0, 2);
			_loadCancelled = false;
			fin = true;
		} else if (_vm->input().controllerFlags & 0x10) {
			_loadCancelled = true;
			fin = true;
		}
		if (fin) {
			_vm->gfx()->setAnimParameter(24, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
			_vm->gfx()->setAnimParameter(25, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
			++state.frameState;
			state.counter = 0;
		}
		break;
	case 1:
		if (state.counter == 20) {
			if (_loadCancelled) {
				state.counter = 2;
				++state.frameState;
				_vm->gfx()->enqueueDrawCommands(_module->getPtr(0x29806));
			} else {
				state.counter = 0;
				++state.frameNo;
				state.frameState = 0;
			}
		} else {
			_vm->gfx()->enqueueDrawCommands(_module->getPtr(0x296C6).getDataFromTable(state.counter++));
		}
		break;
	case 2:
		if (--state.counter)
			return;
		--state.frameNo;
		state.frameState = 1;
		_vm->gfx()->setAnimParameter(17, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimPause);
		_vm->gfx()->setAnimParameter(17, GraphicsEngine::kAnimParaFrame, 0);
		break;
	case 3:
		break;
	case 4:
		break;
	default:
		break;
	}
}

SH_IMPL_FRM(D2, 11) {
	int fin = 0;
	switch (state.frameState) {
	case 0:
		if (state.counter < 24) {
			_vm->gfx()->enqueueDrawCommands(_module->getPtr(0x29292).getDataFromTable(state.counter));
			if (state.counter != 0)
				_vm->gfx()->enqueueDrawCommands(_module->getPtr(0x29554).getDataFromTable(state.counter));
			++state.counter;
			return;
		}

		for (int i = 32; i < 37; ++i)
			_vm->gfx()->clearAnimParameterFlags(i, GraphicsEngine::kAnimParaControlFlags, 0xFF);

		do {
			if (_vm->input().controllerFlags & 1)
				--_option;
			if (_vm->input().controllerFlags & 2)
				++_option;
			_option = (_option + 5) % 5;
		} while (_option == 3 && !state.conf.useLightGun);

		if (!state.conf.lightGunAvailable) {
			state.conf.useLightGun = 0;
			if (_option == 3)
				_option = 4;
		}

		if (_option == 0) {
			if (state.conf.lightGunAvailable && _vm->input().controllerFlags & 0x0C)
				state.conf.useLightGun ^= 1;
		} else if (_option == 1) {
			if (_vm->input().controllerFlags & 8)
				++state.conf.controllerSetup;
			if (_vm->input().controllerFlags & 4)
				--state.conf.controllerSetup;
			state.conf.controllerSetup = (state.conf.controllerSetup + 5) % 5;
		} else if (_option == 2) {
			if (_vm->input().controllerFlags & 0x0C)
				state.conf.disableStereo ^= 1;
		}		

		_vm->gfx()->setAnimParameter(32, GraphicsEngine::kAnimParaFrame, _option << 2);
		_vm->gfx()->setAnimParameter(33, GraphicsEngine::kAnimParaFrame, ((_option ? 2 : 0) + state.conf.useLightGun) << 1);
		_vm->gfx()->setAnimParameter(36, GraphicsEngine::kAnimParaFrame, state.conf.useLightGun << 1);
		_vm->gfx()->setAnimParameter(34, GraphicsEngine::kAnimParaFrame, ((_option != 1 ? 6 : 0) + state.conf.controllerSetup) << 1);
		_vm->gfx()->setAnimParameter(35, GraphicsEngine::kAnimParaFrame, ((_option != 2 ? 2 : 0) + state.conf.disableStereo) << 1);

		if (_vm->input().controllerFlags & 0x80)
			fin = (_option != 3) ? 1 : 3;
		else if (_vm->input().controllerFlags & 0x70)
			fin = (_option == 4) ? 1 : (_option == 3 ? 3 : 0);
		if (fin) {
			_vm->sound()->pcmSendCommand(55 + fin, -1);
			_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x290F2));
			state.counter = 10;
			if (fin == 1) {
				state.frameState = 0;
				++state.frameNo;
			} else {
				++state.frameState;
			}
		}
		break;
	case 1:
		if (--state.counter)
			return;
		_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
		_vm->gfx()->runScript(_module->getGfxData(), 7);
		_vm->gfx()->createMouseCursor(true);
		++state.frameState;
		break;
	case 2:
		state.counter  = 0;
		_counter2 = 0;
		_vm->allowLightGunInput(true);
		_vm->gfx()->setAnimParameter(16, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
		for (int i = 0; i < 4; ++i)
			_vm->gfx()->setAnimParameter(16 + i, GraphicsEngine::kAnimParaFrame, 0);
		for (int i = 0; i < 3; ++i) {
			_vm->gfx()->setAnimParameter(16 + i, GraphicsEngine::kAnimParaBlink, 0);
			_vm->gfx()->setAnimParameter(16 + i, GraphicsEngine::kAnimParaAbsSpeedX, 0);
		}

		++state.frameState;
		break;
	case 3:
		if (_vm->input().controllerFlags & 0x80) {
			_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x290F2));
			state.counter = 10;
			++state.frameState;
			_vm->allowLightGunInput(false);
			return;
		}

		updateAnimBlink();

		switch (state.counter) {
		case 0:
			_vm->gfx()->setAnimParameter(18, GraphicsEngine::kAnimParaBlink, 1);
			_vm->gfx()->setAnimParameter(18, GraphicsEngine::kAnimParaAbsSpeedX, 1);
			++state.counter;
			break;
		case 1:
			if (_vm->input().controllerFlags & 0x100) {
				_vm->sound()->pcmSendCommand(57, -1);
				_vm->gfx()->setAnimParameter(18, GraphicsEngine::kAnimParaBlink, 0);
				_vm->gfx()->setAnimParameter(18, GraphicsEngine::kAnimParaAbsSpeedX, 0);
				_vm->gfx()->setAnimParameter(19, GraphicsEngine::kAnimParaBlink, 1);
				_vm->gfx()->setAnimParameter(19, GraphicsEngine::kAnimParaAbsSpeedX, 1);
				++state.counter;
			}
			break;
		case 2:
			_vm->calibrateLightGun(state);
			_vm->gfx()->setAnimParameter(16, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimPause);
			_vm->gfx()->setAnimParameter(16, GraphicsEngine::kAnimParaPosX, 117);
			_vm->gfx()->setAnimParameter(16, GraphicsEngine::kAnimParaPosY, 93);
			++state.counter;
			break;
		case 3:
			if (_vm->input().controllerFlags & 0x200) {
				_vm->gfx()->setAnimParameter(17, GraphicsEngine::kAnimParaBlink, 1);
				_vm->gfx()->setAnimParameter(17, GraphicsEngine::kAnimParaAbsSpeedX, 1);
				_vm->gfx()->setAnimParameter(19, GraphicsEngine::kAnimParaBlink, 0);
				_vm->gfx()->setAnimParameter(19, GraphicsEngine::kAnimParaAbsSpeedX, 0);
				_vm->allowLightGunInput(false);
				state.counter += 2;
				_vm->sound()->fmSendCommand(29, 0, 2);
			} else if (_vm->input().controllerFlags & 0x100) {
				_vm->sound()->pcmSendCommand(57, -1);
				state.counter++;
			}
			break;
		case 4:
			_vm->gfx()->setAnimParameter(16, GraphicsEngine::kAnimParaPosX, _vm->input().lightGunPos.x - 11);
			_vm->gfx()->setAnimParameter(16, GraphicsEngine::kAnimParaPosY, _vm->input().lightGunPos.y - 35);
			--state.counter;
			break;
		case 5:
			if (++_counter2 < 180)
				return;
			state.counter = 10;
			_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x290F2));
			++state.frameState;
			break;
		default:
			break;
		}
		break;
	case 4:
		if (--state.counter)
			return;
		_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
		_vm->gfx()->runScript(_module->getGfxData(), 8);
		state.frameState = 0;
		break;
	default:
		break;
	}
}

SH_IMPL_FRM(D2, 12) {
	if (--state.counter)
		return;
	state.finish = 1;
	state.frameNo = 0;
	state.frameState = 0;
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
}

void Scene_D2::updateAnimBlink() {
	for (int i = 0; i < 3; ++i) {
		_vm->gfx()->setAnimParameter(17 + i, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
		int speedx = _vm->gfx()->getAnimParameter(17 + i, GraphicsEngine::kAnimParaAbsSpeedX);
		if (speedx)
			_vm->gfx()->setAnimParameter(17 + i, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimPause);
		if (_vm->gfx()->getAnimParameter(17 + i, GraphicsEngine::kAnimParaBlink)) {
			int bc = _vm->gfx()->getAnimParameter(17 + i, GraphicsEngine::kAnimParaBlinkCounter);
			if (++bc >= 16) {
				bc = 0;
				int bd = _vm->gfx()->getAnimParameter(17 + i, GraphicsEngine::kAnimParaBlinkDuration) + 1;
				_vm->gfx()->setAnimParameter(17 + i, GraphicsEngine::kAnimParaAbsSpeedX, 0);
				_vm->gfx()->setAnimParameter(17 + i, GraphicsEngine::kAnimParaAbsSpeedX, speedx ^ 1);
				_vm->gfx()->setAnimParameter(17 + i, GraphicsEngine::kAnimParaBlinkDuration, bd);
				if (bd >= 6)
					_vm->gfx()->setAnimParameter(17 + i, GraphicsEngine::kAnimParaBlink, 0);
			}
			_vm->gfx()->setAnimParameter(17 + i, GraphicsEngine::kAnimParaBlinkCounter, bc);
		}
	}
}

} // End of namespace Snatcher
