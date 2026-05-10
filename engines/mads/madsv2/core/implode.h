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

#ifndef MADS_CORE_IMPLODE_H
#define MADS_CORE_IMPLODE_H

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

extern word implode(
	word (*read_buff)(char *buffer, word *size),
	word (*write_buff)(char *buffer, word *size),
	char *work_buff,
	word *type,
	word *dsize);

extern word explode(
	word (*read_buff)(char *buffer, word *size),
	word (*write_buff)(char *buffer, word *size),
	char *work_buff);

/*
 * CRC32
 * Standard CRC-32 (ISO 3309, polynomial 0xEDB88320, reflected).
 * Not present in the two ASM files but declared in the shared header.
 *
 * @param buffer	
 * @param size	
 * @param old_crc	
 * @return 
 */
extern long crc32(unsigned char *buffer, word *size, long *old_crc);


#define CMP_BINARY             0
#define CMP_ASCII              1

#define CMP_NO_ERROR           0
#define CMP_INVALID_DICTSIZE   1
#define CMP_INVALID_MODE       2
#define CMP_BAD_DATA           3
#define CMP_ABORT              4

} // namespace MADSV2
} // namespace MADS

#endif
