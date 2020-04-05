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

#ifndef ULTIMA4_CORE_LZW_U4DECODE_H
#define ULTIMA4_CORE_LZW_U4DECODE_H

#include "common/stream.h"

namespace Ultima {
namespace Ultima4 {
namespace LZW {

long decompress_u4_file(Common::SeekableReadStream *in, long filesize, void **out);
long getFilesize(Common::SeekableReadStream *input_file);
byte mightBeValidCompressedFile(Common::SeekableReadStream *compressed_file);
long decompress_u4_memory(void *in, long inlen, void **out);

} // End of namespace LZW
} // End of namespace Ultima4
} // End of namespace Ultima

#endif
