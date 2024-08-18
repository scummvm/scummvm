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

#ifndef AGI_DISK_IMAGE_H
#define AGI_DISK_IMAGE_H

namespace Agi {

// PC disk image values and helpers for AgiLoader_v1 and AgiMetaEngineDetection

// Disk image detection requires that image files have a known extension
static const char * const pcDiskImageExtensions[] = { ".ima", ".img" };

#define PC_DISK_SIZE                     (2 * 40 * 9 * 512)
#define PC_DISK_POSITION(h,t,s,o)        (((((h + (t * 2)) * 9) + (s - 1)) * 512) + o)

#define PC_INITDIR_POSITION_V1           PC_DISK_POSITION(0, 0, 9, 0)
#define PC_INITDIR_ENTRY_SIZE_V1         8
#define PC_INITDIR_SIZE_V1               (PC_INITDIR_ENTRY_SIZE_V1 * 10)

#define PC_INITDIR_LOGDIR_INDEX_V1       3
#define PC_INITDIR_PICDIR_INDEX_V1       4
#define PC_INITDIR_VIEWDIR_INDEX_V1      5
#define PC_INITDIR_SOUNDDIR_INDEX_V1     6
#define PC_INITDIR_OBJECTS_INDEX_V1      1
#define PC_INITDIR_WORDS_INDEX_V1        2

#define PC_INITDIR_POSITION_V2001        PC_DISK_POSITION(0, 0, 2, 0)
#define PC_INITDIR_ENTRY_SIZE_V2001      3

#define PC_INITDIR_LOGDIR_INDEX_V2001    10
#define PC_INITDIR_PICDIR_INDEX_V2001    11
#define PC_INITDIR_VIEWDIR_INDEX_V2001   12
#define PC_INITDIR_SOUNDDIR_INDEX_V2001  13
#define PC_INITDIR_OBJECTS_INDEX_V2001   8
#define PC_INITDIR_WORDS_INDEX_V2001     9
#define PC_INITDIR_VOL0_INDEX_V2001      14

} // End of namespace Agi

#endif /* AGI_DISK_IMAGE_H */
