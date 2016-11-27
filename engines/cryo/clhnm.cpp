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

#include "common/debug.h"

#include "cryo/cryolib.h"

namespace Cryo {

static bool use_mono = false;

void CLHNM_Done() {
}

hnm_t *CLHNM_New(int preload_size) {
	hnm_t *hnm = (hnm_t *)malloc(sizeof(*hnm));

	hnm->_frameNum = 0;
	hnm->ff_4 = 0;
	hnm->_file = nullptr;
	hnm->tmpBuffer[0] = nullptr;
	hnm->tmpBuffer[1] = nullptr;
	hnm->finalBuffer = nullptr;
	hnm->_readBuffer = nullptr;
	hnm->ff_896 = 0;
	hnm->_totalRead = 0;
	for (int i = 0; i < 256; i++) {
		hnm->_palette[i].a = 0;
		hnm->_palette[i].r = 0;
		hnm->_palette[i].g = 0;
		hnm->_palette[i].b = 0;
	}

	return hnm;
}

void CLHNM_Dispose(hnm_t *hnm) {
	free(hnm);
}


void CLHNM_SetFile(hnm_t *hnm, file_t *file) {
	hnm->_file = file;
}

void CLHNM_GiveTime(hnm_t *hnm) {
}

void CLHNM_CanLoop(hnm_t *hnm, bool canLoop) {
	hnm->_canLoop = canLoop;
}

void CLHNM_SoundMono(bool isMono) {
	use_mono = isMono;
}

} // End of namespace Cryo
