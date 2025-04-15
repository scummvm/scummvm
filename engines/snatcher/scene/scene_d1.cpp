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
#include "common/endian.h"

namespace Snatcher {

SH_HEAD_BEGIN(D1)
// declarations
SH_DCL_FRM(00)
SH_DCL_FRM(01)
SH_DCL_FRM(02)
SH_DCL_FRM(04)
SH_DCL_FRM(05)
SH_DCL_FRM(06)
SH_DCL_FRM(07)
SH_DCL_FRM(08)
SH_DCL_FRM(09)
SH_DCL_FRM(11)
SH_DCL_FRM(13)
SH_DCL_FRM(15)
SH_DCL_FRM(17)
SH_DCL_FRM(19)
SH_DCL_FRM(21)
SH_DCL_FRM(22)
SH_DCL_FRM(23)
SH_DCL_FRM(25)
SH_DCL_FRM(27)
SH_DCL_FRM(28)

const uint32 *_timeStamp;

SH_HEAD_END(D1)

SH_IMP_FRMTBL(D1) {
	SH_FRM(00),
	SH_FRM(01),
	SH_FRM(02),
	SH_FRM(01),
	SH_FRM(04),
	SH_FRM(05),
	SH_FRM(06),
	SH_FRM(07),
	SH_FRM(08),
	SH_FRM(09),
	SH_FRM(01),
	SH_FRM(11),
	SH_FRM(01),
	SH_FRM(13),
	SH_FRM(01),
	SH_FRM(15),
	SH_FRM(01),
	SH_FRM(17),
	SH_FRM(01),
	SH_FRM(19),
	SH_FRM(01),
	SH_FRM(21),
	SH_FRM(22),
	SH_FRM(23),
	SH_FRM(22),
	SH_FRM(25),
	SH_FRM(22),
	SH_FRM(27),
	SH_FRM(28)
};

SH_IMP_CTOR(D1) {
	SH_CTOR_MAKEPROCS(D1);

	uint32 *t = new uint32[29];
	const uint8 *in = _module->getPtr(0x2929A)();
	for (int i = 0; i < 29; ++i) {
		t[i] = READ_BE_UINT32(in);
		in += 6;
	}
	_timeStamp = t;
}

SH_IMP_DTOR(D1)	{
	SH_DTOR_DELPROCS(D1);
	delete[] _timeStamp;
}

SH_IMPL_UPDT(D1) {
	if (state.frameNo == -1) {
		if (state.frameState == 0) {
			_vm->sound()->musicPlay(3);
			++state.frameState;
		} else if (_vm->sound()->musicIsPlaying()) {
			state.frameNo = 0;
			state.frameState = 0;
		}
		return;
	}

	if (state.frameNo >= (int)_frameProcs.size() || !_frameProcs[state.frameNo]->isValid()) {
		error("%s(): Invalid call to frame proc %d", __FUNCTION__, state.frameNo);
	} else if (_vm->sound()->musicGetTime() >= _timeStamp[state.frameNo]) {
		(*_frameProcs[state.frameNo])(state);
		++state.frameNo;
	} else if (_vm->inputFlag() & 0x80) {
		state.finish = -1;
		state.frameNo = 0;
	}
}

SH_IMPL_FRM(D1, 00) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 0);
}

SH_IMPL_FRM(D1, 01) {
	_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x142C));
}

SH_IMPL_FRM(D1, 02) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 1);
}

SH_IMPL_FRM(D1, 04) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 2);
	_vm->gfx()->setScrollStep(GraphicsEngine::kVertA | GraphicsEngine::kSingleStep, 0x110);
	_vm->gfx()->scrollCommand(1);
}

SH_IMPL_FRM(D1, 05) {
	_vm->gfx()->scrollCommand(8);
}

SH_IMPL_FRM(D1, 06) {
	_vm->gfx()->scrollCommand(9);
}

SH_IMPL_FRM(D1, 07) {
	_vm->gfx()->scrollCommand(10);
}

SH_IMPL_FRM(D1, 08) {
	_vm->gfx()->scrollCommand(11);
}

SH_IMPL_FRM(D1, 09) {
	_vm->gfx()->scrollCommand(12);
}

SH_IMPL_FRM(D1, 11) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 3);
}

SH_IMPL_FRM(D1, 13) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 4);
}

SH_IMPL_FRM(D1, 15) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 5);
}

SH_IMPL_FRM(D1, 17) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 6);
}

SH_IMPL_FRM(D1, 19) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 7);
}

SH_IMPL_FRM(D1, 21) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 8);
}

SH_IMPL_FRM(D1, 22) {
	_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x143A));
}

SH_IMPL_FRM(D1, 23) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 9);
}

SH_IMPL_FRM(D1, 25) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 10);
}

SH_IMPL_FRM(D1, 27) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 11);
}

SH_IMPL_FRM(D1, 28) {
	state.finish = 1;
	state.frameNo = 0;
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
}

} // End of namespace Snatcher
