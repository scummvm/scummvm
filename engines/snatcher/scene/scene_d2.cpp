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

SH_IMP_CTOR(D2) {
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
bool hasSaveFiles = true;
bool hasFreeBuram = true;
uint8 _bua3 = 0;
uint8 _buram_0 = 0;
bool _buram_2 = true;
uint8 wordA = 0;
int16 _moduleWord = -1;

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
		if (hasSaveFiles)
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
	switch (state.frameState) {
	case 0:
		_vm->gfx()->scrollCommand(0xFF);
		_vm->gfx()->clearAnimControlFlags(hasSaveFiles ? 17 : 16, ~GraphicsEngine::kAnimHide);
		state.frameState += (hasSaveFiles ? 1 : 2);
		break;
	case 1:
		if (_vm->inputFlag() & 3)
			_vm->gfx()->clearAnimControlFlags(17, ~GraphicsEngine::kAnimHide);
		if (_vm->inputFlag() & 0x80) {
			_vm->gfx()->setAnimControlFlags(17, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
			wordA = 1;
			if (_vm->gfx()->getAnimCurFrame(17) == 3) {
				wordA = 0;
				++state.frameNo;
			}
			++state.frameNo;
			state.frameState = 0;
			_bua3 = 56;
			//enqueueCommunicationStatus2Command(0);
			_vm->gfx()->setAnimControlFlags(16, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
			_vm->gfx()->setAnimControlFlags(17, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
			_moduleWord = 0;
		}
		break;
	default:
		if (_vm->inputFlag() & 0x80) {
			++state.frameNo;
			state.frameState = 0;
		}
		break;
	}
}

SH_IMPL_FRM(D2, 10) {
	switch (state.frameState) {
	case 0:
		if (state.counter == 3) {

		}
		if (state.counter != 21) {;
			_vm->gfx()->enqueueCopyCommands(_module->getPtr(0x29142).getDataFromTable(state.counter));
			if (state.counter != 0)
				_vm->gfx()->enqueueCopyCommands(_module->getPtr(0x29412).getDataFromTable(state.counter));
			++state.counter;
			return;
		}
		_vm->gfx()->clearAnimControlFlags(24, 0xFF);
		_vm->gfx()->clearAnimControlFlags(25, 0xFF);



		_vm->gfx()->setAnimControlFlags(24, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
		_vm->gfx()->setAnimControlFlags(25, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
		++state.frameState;
		state.counter = 0;

		++state.frameNo;
		state.frameState = 0;
		break;
	case 1:
		if (state.counter == 20) {
			if (1/*_varVar*/) {
				state.counter = 2;
				++state.frameState;
				_vm->gfx()->enqueueCopyCommands(_module->getPtr(29806));
			} else {
				state.counter = 0;
				++state.frameNo;
				state.frameState = 0;
			}
		} else {
			_vm->gfx()->enqueueCopyCommands(_module->getPtr(0x296C6).getDataFromTable(state.counter++));
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
	switch (state.frameState) {
	case 0:
		if (state.counter < 24) {
			_vm->gfx()->enqueueCopyCommands(_module->getPtr(0x29292).getDataFromTable(state.counter));
			if (state.counter != 0)
				_vm->gfx()->enqueueCopyCommands(_module->getPtr(0x29554).getDataFromTable(state.counter));
			++state.counter;
			return;
		}
		/*state.countDown = 10;
		_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x10F2));
		++state.frameState;*/

		_vm->sound()->pcmDoCommand(56, -1);
		_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x10F2));
		state.counter = 10;
		state.frameState = 0;
		++state.frameNo;
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
