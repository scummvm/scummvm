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

#ifndef CHAMBER_IFGM_H
#define CHAMBER_IFGM_H

namespace Chamber {

extern byte ifgm_loaded;

extern uint16 ifgm_flag1;
extern byte ifgm_flag2;

void IFGM_Init(void);

void IFGM_Shutdown(void);

void IFGM_Poll(void);

void IFGM_PlaySample(byte index);
void IFGM_StopSample(void);

int16 IFGM_PlaySound(byte index);
void IFGM_PlaySfx(byte index);

} // end of namespace Chamber

#endif
