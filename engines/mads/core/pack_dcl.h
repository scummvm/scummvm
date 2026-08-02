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

#ifndef MADS_CORE_PACK_DCL_H
#define MADS_CORE_PACK_DCL_H

#include "common/stream.h"
#include "mads/core/general.h"

namespace MADS {

/*
 * pack_dcl.cpp -- genuine PKWare Data Compression Library ("explode")
 * decompressor.
 *
 * MADSPACK 1.0 (used by early builds such as the Rex Nebular demo) used
 * this real PKWare algorithm; MADSPACK 2.0 replaced it with MicroProse's
 * own "pFAB" compressor (see pfab.cpp). The two formats are unrelated
 * despite the shared "MADSPACK" branding, and this decoder is deliberately
 * kept independent of the shared pack_data()/PackList machinery in
 * pack.cpp (which every other MADS game relies on), so that adding support
 * for this older format carries no risk of regressing anything else.
 *
 * Ported from the public-domain reference implementation "blast.c" by
 * Mark Adler (zlib contrib/blast), which documents the format originally
 * described by Ben Rudiak-Gould.
 */

/**
 * Decompresses a PKWare DCL "explode" stream.
 *
 * @param src       Compressed data stream, positioned at the start of the block.
 *                  Only as many bytes as the format actually needs are consumed;
 *                  the stream is not required to end where the block does.
 * @param dest      Destination buffer; must be at least destSize bytes.
 * @param destSize  Expected number of decompressed bytes.
 * @return true if exactly destSize bytes were decompressed successfully.
 */
extern bool pack_dcl_explode(Common::SeekableReadStream *src, byte *dest, long destSize);

} // namespace MADS

#endif
