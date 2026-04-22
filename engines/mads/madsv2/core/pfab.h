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

#ifndef MADS_CORE_PFAB_H
#define MADS_CORE_PFAB_H

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

#define CMP_BINARY             0
#define CMP_ASCII              1

#define CMP_NO_ERROR           0
#define CMP_INVALID_DICTSIZE   1
#define CMP_INVALID_MODE       2
#define CMP_BAD_DATA           3
#define CMP_ABORT              4


/*
 * Public API: pFABcomp
 *
 * @param read_buff	
 * @param write_buff	
 * @param work_buff	
 * @param type	
 * @param dsize	
 * @return 
 */
extern word pFABcomp(
	word (*read_buff)(char *buffer, word *size),
	word (*write_buff)(char *buffer, word *size),
	char *work_buff,
	word *type,
	word *dsize);

extern word pFABexp0(
	word (*read_buff)(char *buffer, word *size),
	word (*write_buff)(char *buffer, word *size),
	char *work_buff);

extern word pFABexp1(
	word (*read_buff)(char *buffer, word *size),
	char *write_buf,
	char *work_buff);

/*
 * pFABexp2  --  memory-to-memory decompressor  (PFABEXP2.ASM)
 * read_buf  points to compressed data.
 * write_buf points to destination buffer (must be large enough).
 * work_buff must be at least 4 bytes (for the output length result),
 * or NULL to query required size.
 * Returns 0 on success.  Decompressed byte count stored at work_buff[0..3].
 *
 * @param read_buf	
 * @param write_buf	
 * @param work_buff	
 * @return 
 */
extern word pFABexp2(
	byte *read_buf,
	byte *write_buf,
	char *work_buff);

} // namespace MADSV2
} // namespace MADS

#endif
