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

namespace Common {
class SeekableReadStream;
class Path;
}

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

// A2 disk image values and helpers for AgiLoader_A2 and AgiMetaEngineDetection

// Disk image detection requires that image files have a known extension
static const char * const a2DiskImageExtensions[] = { ".do", ".dsk", ".img", ".nib", ".woz" };

#define A2_DISK_SIZE                     (35 * 16 * 256)
#define A2_DISK_POSITION(t, s, o)        ((((t * 16) + s) * 256) + o)

#define A2_INITDIR_POSITION              A2_DISK_POSITION(1, 3, 0)
#define A2_INITDIR_ENTRY_SIZE            4

#define A2_INITDIR_LOGDIR_INDEX          3
#define A2_INITDIR_PICDIR_INDEX          4
#define A2_INITDIR_VIEWDIR_INDEX         5
#define A2_INITDIR_SOUNDDIR_INDEX        6
#define A2_INITDIR_OBJECTS_INDEX         1
#define A2_INITDIR_WORDS_INDEX           2
#define A2_INITDIR_VOLUME_MAP_POSITION   (A2_INITDIR_POSITION + 5 + 33)

#define A2_KQ2_LOGDIR_POSITION           A2_DISK_POSITION(1,  0,  0)
#define A2_KQ2_PICDIR_POSITION           A2_DISK_POSITION(1,  3,  0)
#define A2_KQ2_VIEWDIR_POSITION          A2_DISK_POSITION(1,  6,  0)
#define A2_KQ2_SOUNDDIR_POSITION         A2_DISK_POSITION(1,  9,  0)
#define A2_KQ2_OBJECTS_POSITION          A2_DISK_POSITION(2,  9,  0)
#define A2_KQ2_WORDS_POSITION            A2_DISK_POSITION(3,  0,  0)
#define A2_KQ2_VOL0_POSITION             A2_DISK_POSITION(26, 0,  0)
#define A2_KQ2_VOL1_POSITION             A2_DISK_POSITION(18, 0,  0)
#define A2_KQ2_DISK_COUNT                5

#define A2_BC_LOGDIR_POSITION            A2_DISK_POSITION(1,  7,  0)
#define A2_BC_PICDIR_POSITION            A2_DISK_POSITION(1,  12, 0)
#define A2_BC_VIEWDIR_POSITION           A2_DISK_POSITION(1,  9,  0)
#define A2_BC_SOUNDDIR_POSITION          A2_DISK_POSITION(1,  13, 0)
#define A2_BC_OBJECTS_POSITION           A2_DISK_POSITION(1,  3,  0)
#define A2_BC_WORDS_POSITION             A2_DISK_POSITION(1,  5,  0)
#define A2_BC_VOLUME_MAP_POSITION        A2_DISK_POSITION(7,  11, 254)
#define A2_BC_DISK_COUNT                 5
#define A2_BC_VOLUME_COUNT               9

Common::SeekableReadStream *openPCDiskImage(const Common::Path &path, const Common::FSNode &node);
Common::SeekableReadStream *openA2DiskImage(const Common::Path &path, const Common::FSNode &node, bool loadAllTracks = true);

} // End of namespace Agi

#endif /* AGI_DISK_IMAGE_H */
