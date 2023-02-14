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

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/ifgm.h"
#include "chamber/dialog.h"
#include "chamber/sound.h"

namespace Chamber {

byte ifgm_loaded = 0;
byte ifgm_flag2;

void IFGM_Init(void) {
	if (g_vm->getLanguage() == Common::EN_USA) {
		/*TODO*/
	} else {
		ifgm_loaded = 0;
	}

}

void IFGM_Shutdown(void) {

}

void IFGM_Poll(void) {
	if(!ifgm_loaded)
		return;
/*
xor     ax, ax
int     0F0h
*/

}


/*
Load and play a sound sample.
Return 0 if playback is unavailable
*/
int16 IFGM_PlaySound(byte index) {
	if (!ifgm_loaded)
		return 0;

	/*TODO*/

	return 0;
}

void IFGM_PlaySample(byte index) {
	if (!ifgm_loaded)
		return;
	IFGM_PlaySound(index);
}

void IFGM_StopSample(void) {
	if (!ifgm_loaded)
		return;
}

static byte sfx_sounds[] = {
	0, 5, 10, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void IFGM_PlaySfx(byte index) {
	if (!ifgm_loaded)
		return;
	if (cur_dlg_index == 0)
		return;
	playSound(sfx_sounds[index % 16]);
}

} // end of namespace Chamber
