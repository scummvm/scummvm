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

SH_HEAD_BEGIN(Y16)
// declarations
SH_DCL_FRM(00)
SH_DCL_FRM(01)
SH_DCL_FRM(02)
SH_DCL_FRM(04)
SH_DCL_FRM(06)
SH_DCL_FRM(08)
SH_DCL_FRM(10)
SH_DCL_FRM(12)
SH_DCL_FRM(13)
SH_DCL_FRM(14)
SH_DCL_FRM(15)
SH_DCL_FRM(16)
SH_DCL_FRM(17)
SH_DCL_FRM(18)
SH_DCL_FRM(19)
SH_DCL_FRM(20)
SH_DCL_FRM(21)
SH_DCL_FRM(22)
SH_DCL_FRM(23)
SH_DCL_FRM(25)
SH_DCL_FRM(26)
SH_DCL_FRM(27)

const uint32 *_timeStamp;

SH_HEAD_END(Y16)

SH_IMP_FRMTBL(Y16) {
	SH_FRM(Y16, 00),
	SH_FRM(Y16, 01),
	SH_FRM(Y16, 02),
	SH_FRM(Y16, 01),
	SH_FRM(Y16, 04),
	SH_FRM(Y16, 01),
	SH_FRM(Y16, 06),
	SH_FRM(Y16, 01),
	SH_FRM(Y16, 08),
	SH_FRM(Y16, 01),
	SH_FRM(Y16, 10),
	SH_FRM(Y16, 01),
	SH_FRM(Y16, 12),
	SH_FRM(Y16, 13),
	SH_FRM(Y16, 14),
	SH_FRM(Y16, 15),
	SH_FRM(Y16, 16),
	SH_FRM(Y16, 17),
	SH_FRM(Y16, 18),
	SH_FRM(Y16, 19),
	SH_FRM(Y16, 20),
	SH_FRM(Y16, 21),
	SH_FRM(Y16, 22),
	SH_FRM(Y16, 23),
	SH_FRM(Y16, 22),
	SH_FRM(Y16, 25),
	SH_FRM(Y16, 26),
	SH_FRM(Y16, 27)
};

SH_IMP_CTOR(Y16) {
	SH_CTOR_MAKEPROCS(Y16);

	uint32 *t = new uint32[28];
	const uint8 *in = _module->getPtr(0x290FE)();
	for (int i = 0; i < 28; ++i) {
		t[i] = READ_BE_UINT32(in);
		in += 6;
	}
	_timeStamp = t;
}

SH_IMP_DTOR(Y16)	{
	SH_DTOR_DELPROCS(Y16);
	delete[] _timeStamp;
}

SH_IMPL_UPDT(Y16) {
	if (state.frameNo == -1) {
		if (state.frameState == 0) {
			_vm->sound()->cdaPlay(19);
			++state.frameState;
		} else if (_vm->sound()->cdaIsPlaying()) {
			state.frameNo = 0;
			state.frameState = 0;
		}
		return;
	}

	if (state.frameNo >= (int)_frameProcs.size() || !_frameProcs[state.frameNo]->isValid()) {
		error("%s(): Invalid call to frame proc %d", __FUNCTION__, state.frameNo);
	} else if (_vm->sound()->cdaGetTime() >= _timeStamp[state.frameNo]) {
		(*_frameProcs[state.frameNo])(state);
		++state.frameNo;
	}
}

SH_IMPL_FRM(Y16, 00) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getGfxData(), 0);
}

SH_IMPL_FRM(Y16, 01) {
	_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x2931A));
}

SH_IMPL_FRM(Y16, 02) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getGfxData(), 1);
}

SH_IMPL_FRM(Y16, 04) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getGfxData(), 2);
}

SH_IMPL_FRM(Y16, 06) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getGfxData(), 3);
}

SH_IMPL_FRM(Y16, 08) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getGfxData(), 4);
}

SH_IMPL_FRM(Y16, 10) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getGfxData(), 5);
}

SH_IMPL_FRM(Y16, 12) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getGfxData(), 6);
}

SH_IMPL_FRM(Y16, 13) {
	_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x292F0));
}

SH_IMPL_FRM(Y16, 14) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getGfxData(), 7);
}

SH_IMPL_FRM(Y16, 15) {
	_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x292FE));
}

SH_IMPL_FRM(Y16, 16) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getGfxData(), 8);
}

SH_IMPL_FRM(Y16, 17) {
	if (_vm->sound()->cdaIsPlaying()) {
		--state.frameNo;
		return;
	}
	state.counter = 300;
	if (state.saveCount)
		state.frameNo = 24;
}

SH_IMPL_FRM(Y16, 18) {
	if (--state.counter) {
		--state.frameNo;
		return;
	}
	state.counter = 64;
	_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x292E2));
}

SH_IMPL_FRM(Y16, 19) {
	if (--state.counter) {
		--state.frameNo;
		return;
	}
}

SH_IMPL_FRM(Y16, 20) {
	_vm->sound()->cdaPlay(2);
}

SH_IMPL_FRM(Y16, 21) {
	if (_vm->sound()->cdaIsPlaying())
		state.counter = 24;
	else
		--state.frameNo;
}

SH_IMPL_FRM(Y16, 22) {
	if (--state.counter) {
		--state.frameNo;
		return;
	}
}

SH_IMPL_FRM(Y16, 23) {
	state.counter = 300;
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getGfxData(), 9);
}

SH_IMPL_FRM(Y16, 25) {
	if (!(_vm->input().singleFrameControllerFlags & 0x80)) {
		--state.frameNo;
		return;
	}
	state.counter = 64;
	_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x292E2));
}

SH_IMPL_FRM(Y16, 26) {
	if (--state.counter) {
		--state.frameNo;
		return;
	}
}

SH_IMPL_FRM(Y16, 27) {
	state.frameNo = 0;
	state.finish = 1;
}

} // End of namespace Snatcher
