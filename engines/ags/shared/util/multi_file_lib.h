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

//=============================================================================
//
// A packed data file header and functions for reading it from the stream.
//
//-----------------------------------------------------------------------------
//
// The code is based on CLIB32, by Chris Jones (1998-99), DJGPP implementation
// of the CLIB reader.
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_MULTI_FILE_LIB_H
#define AGS_SHARED_UTIL_MULTI_FILE_LIB_H

#include "ags/shared/core/asset.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

//
// MultiFileLib utilities: (de)serialization of asset library in MFL format
//
namespace MFLUtil {
enum MFLError {
	kMFLNoError = 0,
	kMFLErrNoLibSig = -1, // library signature does not match
	kMFLErrLibVersion = -2, // library version unsupported
	kMFLErrNoLibBase = -3, // file is not library base (head)
	kMFLErrLibAssetCount = -4, // too many assets in library
	kMFLErrAssetNameLong = -5  // asset name is too long (old formats only)
};

enum MFLVersion {
	kMFLVersion_SingleLib = 6,
	kMFLVersion_MultiV10 = 10,
	kMFLVersion_MultiV11 = 11,
	kMFLVersion_MultiV15 = 15, // unknown differences
	kMFLVersion_MultiV20 = 20,
	kMFLVersion_MultiV21 = 21,
	kMFLVersion_MultiV30 = 30  // 64-bit file support, loose limits
};

// Maximal number of the data files in one library chain (1-byte index)
const size_t MaxMultiLibFiles = 256;

MFLError TestIsMFL(Stream *in, bool test_is_main = false);
MFLError ReadHeader(AssetLibInfo &lib, Stream *in);

void     WriteHeader(const AssetLibInfo &lib, MFLVersion lib_version, int lib_index, Stream *out);
void     WriteEnder(soff_t lib_offset, MFLVersion lib_index, Stream *out);
} // namespace MFLUtil

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
