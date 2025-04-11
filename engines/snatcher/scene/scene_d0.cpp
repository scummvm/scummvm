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

SH_HEAD_BEGIN(D0)
// declarations
SH_DCL_FRM(00)
SH_DCL_FRM(01)
SH_DCL_FRM(02)
SH_DCL_FRM(03)
SH_DCL_FRM(04)
SH_DCL_FRM(05)
SH_DCL_FRM(07)
SH_DCL_FRM(08)
SH_DCL_FRM(09)
SH_DCL_FRM(10)
SH_DCL_FRM(11)
SH_DCL_FRM(12)
SH_DCL_FRM(14)
SH_DCL_FRM(16)
SH_DCL_FRM(18)
SH_DCL_FRM(20)
SH_DCL_FRM(22)
SH_DCL_FRM(24)
SH_DCL_FRM(26)
SH_DCL_FRM(28)
SH_DCL_FRM(30)
SH_DCL_FRM(31)
SH_DCL_FRM(32)
SH_DCL_FRM(34)

const uint32 *_timeStamp;

SH_HEAD_END(D0)

SH_IMP_FRMTBL(D0) {
	SH_FRM(00),
	SH_FRM(01),
	SH_FRM(02),
	SH_FRM(03),
	SH_FRM(04),
	SH_FRM(05),
	SH_FRM(01),
	SH_FRM(07),
	SH_FRM(08),
	SH_FRM(09),
	SH_FRM(10),
	SH_FRM(11),
	SH_FRM(12),
	SH_FRM(08),
	SH_FRM(14),
	SH_FRM(08),
	SH_FRM(16),
	SH_FRM(08),
	SH_FRM(18),
	SH_FRM(08),
	SH_FRM(20),
	SH_FRM(08),
	SH_FRM(22),
	SH_FRM(08),
	SH_FRM(24),
	SH_FRM(08),
	SH_FRM(26),
	SH_FRM(08),
	SH_FRM(28),
	SH_FRM(08),
	SH_FRM(30),
	SH_FRM(31),
	SH_FRM(32),
	SH_FRM(01),
	SH_FRM(34)
};

SH_IMP_CTOR(D0) {
	SH_CTOR_MAKEPROCS(D0);

	uint32 *t = new uint32[35];
	const uint8 *in = _module->getPtr(0x2A85C)();
	for (int i = 0; i < 35; ++i) {
		t[i] = READ_BE_UINT32(in);
		in += 6;
	}
	_timeStamp = t;
}

SH_IMP_DTOR(D0)	{
	SH_DTOR_DELPROCS(D0);
	delete[] _timeStamp;
}

SH_IMPL_UPDT(D0) {
	if (state.frameNo == -1) {
		if (state.frameState == 0) {
			_vm->sound()->musicPlay(5);
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

SH_IMPL_FRM(D0, 00) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 0);
}

SH_IMPL_FRM(D0, 01) {
	_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x2A4A));
}

SH_IMPL_FRM(D0, 02) {
	/**/
}

SH_IMPL_FRM(D0, 03) {
	_vm->sound()->musicPlay(4);
}

SH_IMPL_FRM(D0, 04) {
	--state.frameNo;
	if (_vm->sound()->musicIsPlaying()) {
		++state.frameNo;
		state.frameState = 0;
	}
}

SH_IMPL_FRM(D0, 05) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 1);
}

SH_IMPL_FRM(D0, 07) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 2);
}

SH_IMPL_FRM(D0, 08) {
	_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x2A66));
}

SH_IMPL_FRM(D0, 09) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 3);
}

SH_IMPL_FRM(D0, 10) {
	_vm->gfx()->enqueuePaletteEvent(_module->getPtr(0x2A58));
}

SH_IMPL_FRM(D0, 11) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 4);
}

SH_IMPL_FRM(D0, 12) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 5);
}

SH_IMPL_FRM(D0, 14) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 6);
}

SH_IMPL_FRM(D0, 16) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 7);
}

SH_IMPL_FRM(D0, 18) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 8);
}

SH_IMPL_FRM(D0, 20) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 9);
}

SH_IMPL_FRM(D0, 22) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 10);
}

SH_IMPL_FRM(D0, 24) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 11);
}

SH_IMPL_FRM(D0, 26) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 12);
}

SH_IMPL_FRM(D0, 28) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 13);
}

SH_IMPL_FRM(D0, 30) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 14);
}

SH_IMPL_FRM(D0, 31) {
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
	_vm->gfx()->runScript(_module->getPtr(0), 15);
}

SH_IMPL_FRM(D0, 32) {
	//_sprites[16].controlFlags = 1;
}

SH_IMPL_FRM(D0, 34) {
	state.finish = 1;
	state.frameNo = 0;
	_vm->gfx()->reset(GraphicsEngine::kResetSetDefaultsExt);
}

} // End of namespace Snatcher
