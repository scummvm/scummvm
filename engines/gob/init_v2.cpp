/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/init.h"
#include "gob/global.h"
#include "gob/timer.h"
#include "gob/video.h"
#include "gob/draw.h"

namespace Gob {

Init_v2::Init_v2(GobEngine *vm) : Init_v1(vm) {
}

void Init_v2::soundVideo(int32 smallHeap, int16 flag) {
	if (_vm->_global->_videoMode != 0x13 && _vm->_global->_videoMode != 0x14 &&
			_vm->_global->_videoMode != 0)
		error("soundVideo: Video mode 0x%x is not supported!",
		    _vm->_global->_videoMode);

	_vm->_draw->_frontSurface = &_vm->_global->_primarySurfDesc;
	_vm->_video->initSurfDesc(_vm->_global->_videoMode, _vm->_video->_surfWidth,
			_vm->_video->_surfHeight, 0x80);

	_vm->_global->_mousePresent = 1;

	_vm->_global->_inVM = 0;

	_vm->_global->_sprAllocated = 0;
	_vm->_gtimer->enableTimer();

	if ((_vm->_global->_videoMode == 0x13) || (_vm->_global->_videoMode == 0x14))
		_vm->_global->_colorCount = 256;

	_vm->_global->_pPaletteDesc = &_vm->_global->_paletteStruct;
	_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaPalette;
	_vm->_global->_pPaletteDesc->unused1 = _vm->_global->_unusedPalette1;
	_vm->_global->_pPaletteDesc->unused2 = _vm->_global->_unusedPalette2;

	if (_vm->_global->_videoMode != 0)
		_vm->_video->initSurfDesc(_vm->_global->_videoMode, _vm->_video->_surfWidth,
				_vm->_video->_surfHeight, PRIMARY_SURFACE);
}

} // End of namespace Gob
