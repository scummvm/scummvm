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

// local vars
int16 _option;
bool _loadCancelled;

SH_HEAD_END(D2)

SH_IMP_FRMTBL(D2) {
	SH_FRM(00),
	SH_FRM(01),
	SH_FRM(02),
	SH_FRM(03),
	SH_FRM(04),
	SH_FRM(05),
	SH_FRM(06),
	SH_FRM(07),
	SH_FRM(08),
	SH_FRM(09),
	SH_FRM(10),
	SH_FRM(11),
	SH_FRM(12)
};

SH_IMP_CTOR(D2), _option(0), _loadCancelled(false) {
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

bool hasRAMCart = true; 
int _state_ua_1 = 0;
int _hasSaveSlotFlag = 15;
int _saveFileCurID = 0;
bool hasFreeBuram = true;
uint8 _bua3 = 0;
uint8 _buram_0 = 0;
bool _buram_2 = true;

int16 _sceneState_ua_2 = 0;
int16 _sceneState_ua_3 = 0;

// functions
SH_IMPL_FRM(D2, 00) {
	switch (state.frameState) {
	case 0:
		//enqueueCommunicationStatus2Command(_state_ua_1 ? 0xF3 : 0xF4, 0);
		_vm->sound()->pcmDoCommand(_state_ua_1 ? 0xFC : 0xFD, -1);
		//_d2state = 0;
		//buramCheck();
		++state.frameState;
		break;
	case 1:
		if (hasRAMCart) {
			++state.frameNo;
			state.frameState = 0;
		} else {
			_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
			_vm->gfx()->runScript(_module->getPtr(0), 1); // RAM Cart select screen
			++state.frameState;
		}
		break;
	case 2:
		if (_vm->inputFlag() & 3) {
			_vm->gfx()->clearAnimControlFlags(16, ~GraphicsEngine::kAnimHide);
			_buram_0 ^= 1;
		}
		if (_vm->inputFlag() & 0x80) {
			state.counter = 10;
			_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x10F2));
			++state.frameState;
		}
		break;
	default:
		if (--state.counter)
			return;
		if (_buram_0)
			hasRAMCart = _buram_2;
		++state.frameNo;
		state.frameState = 0;
		break;
	}
}

SH_IMPL_FRM(D2, 01) {
	if (!state.frameState) {
		_vm->sound()->musicPlay(2);
		++state.frameState;
	} else if (_vm->sound()->musicIsPlaying()) {
		++state.frameNo;
		state.frameState = 0;
	}	
}

SH_IMPL_FRM(D2, 02) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->scrollCommand(1);
	_vm->gfx()->setScrollStep(GraphicsEngine::kVertA | GraphicsEngine::kSingleStep, 0x100);
	_vm->gfx()->runScript(_module->getPtr(0), 0);
	state.counter = 24;
	++state.frameNo;
	state.frameState = 0;
}

SH_IMPL_FRM(D2, 03) {
	if (--state.counter)
		return;
	_vm->gfx()->scrollCommand(7);
	state.counter = 4;
	++state.frameNo;
	state.frameState = 0;
}

SH_IMPL_FRM(D2, 04) {
	if (--state.counter)
		return;
	_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x08E6));
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
		if (hasRAMCart) {
			++state.frameNo;
			state.frameState = 0;
		} else {
			_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x0944));
			++state.frameState;
			state.counter = 17;
		}
		break;
	case 1:
		if (--state.counter)
			return;
		_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
		_vm->gfx()->runScript(_module->getPtr(0), 2);
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
		if (hasFreeBuram) {
			++state.frameNo;
			state.frameState = 0;
		} else {
			_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x0944));
			state.counter = 17;
			++state.frameState;
		}
		break;
	case 2:
		if (--state.counter)
			return;
		_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
		_vm->gfx()->runScript(_module->getPtr(0), 3);
		++state.frameState;
		break;
	default:
		//if (_vm->_commandsFromMain & 0x80)
		/**/
		break;
	}
}

SH_IMPL_FRM(D2, 09) {
	bool fin = false;
	switch (state.frameState) {
	case 0:
		_vm->gfx()->scrollCommand(0xFF);
		_vm->gfx()->clearAnimControlFlags(_hasSaveSlotFlag ? 17 : 16, ~GraphicsEngine::kAnimHide);
		state.frameState += (_hasSaveSlotFlag ? 1 : 2);
		break;
	case 1:
		if (_vm->inputFlag() & 3)
			_vm->gfx()->clearAnimControlFlags(17, ~GraphicsEngine::kAnimHide);
		if (_vm->inputFlag() & 0x80) {
			_vm->gfx()->setAnimControlFlags(17, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
			state.menuSelect = 1;
			if (_vm->gfx()->getAnimCurFrame(17) == 3) {
				state.menuSelect = 0;
				++state.frameNo;
			}
			fin = true;
		}
		break;
	default:
		if (_vm->inputFlag() & 0x80) {
			++state.frameNo;
			state.frameState = 0;
			fin = true;
		}
		break;
	}
	if (fin) {
		++state.frameNo;
		state.frameState = 0;
		_bua3 = 56;
		//enqueueCommunicationStatus2Command(0);
		_vm->gfx()->setAnimControlFlags(16, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
		_vm->gfx()->setAnimControlFlags(17, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
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
		_vm->gfx()->clearAnimControlFlags(24, 0xFF);
		_vm->gfx()->clearAnimControlFlags(25, 0xFF);
		

		do {
			if (_vm->inputFlag() & 1)
				--_saveFileCurID;
			if (_vm->inputFlag() & 2)
				++_saveFileCurID;
			_saveFileCurID &= 3;
		} while (!(_hasSaveSlotFlag & (1 << _saveFileCurID)));

		_vm->gfx()->setAnimFrame(24, _saveFileCurID << 2);
		_vm->gfx()->setAnimFrame(25, _saveFileCurID << 2);

		if (_vm->inputFlag() & 0x80) {
			_loadCancelled = 0;
			fin = true;
		} else if (_vm->inputFlag() & 0x10) {
			_loadCancelled = 1;
			fin = true;
		}
		if (fin) {
			_vm->gfx()->setAnimControlFlags(24, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
			_vm->gfx()->setAnimControlFlags(25, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
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
		_vm->gfx()->setAnimControlFlags(17, GraphicsEngine::kAnimPause);
		_vm->gfx()->setAnimFrame(17, 0);
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
			_vm->gfx()->clearAnimControlFlags(i, 0xFF);

		do {
		if (_vm->inputFlag() & 1)
			--_option;
		if (_vm->inputFlag() & 2)
			++_option;
		_option = (_option + 5) % 5;

		} while (_option == 3 && !_sceneState_ua_2);

		//sub11_0_Do_sub_28F9C();
		//sub11_0_dd();

		_vm->gfx()->setAnimFrame(32, _option << 2);
		_vm->gfx()->setAnimFrame(33, ((_option ? 2 : 0) + _sceneState_ua_2) << 1);
		_vm->gfx()->setAnimFrame(36, _sceneState_ua_2 & 2);
		_vm->gfx()->setAnimFrame(34, ((_option != 1 ? 6 : 0) + _sceneState_ua_3) << 1);
		_vm->gfx()->setAnimFrame(35, ((_option != 2 ? 2 : 0) + _state_ua_1) << 1);
		
		if (_vm->inputFlag() & 0x80)
			fin = (_option != 3) ? 1 : 3;
		else if (_vm->inputFlag() & 0x70)
			fin = (_option == 4) ? 1 : (_option == 3 ? 3 : 0);
		if (fin) {
			_vm->sound()->pcmDoCommand(55 + fin, -1);
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
		_vm->gfx()->runScript(_module->getPtr(0), 7);
		++state.frameState;
		break;
	case 2:
		state.counter  = 0;
		//
		++state.frameState;
		break;
	case 3:
		if (_vm->inputFlag() & 0x80) {
			_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x10F2));
			state.counter = 10;
			state.frameState = 0;
			++state.frameNo;
			return;
		}

		//animDo:sub_2985E

		switch (state.counter) {
		case 0:
			++state.counter;
			break;
		case 1:
			++state.counter;
			break;
		case 2:
			++state.counter;
			break;
		case 3:
			state.counter += 2;
			break;
		case 4:
			--state.counter;
			break;
		case 5:
			state.counter = 10;
			_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x10F2));
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
		_vm->gfx()->runScript(_module->getPtr(0), 8);
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

} // End of namespace Snatcher
