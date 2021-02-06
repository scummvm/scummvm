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

#ifndef AGS_ENGINE_AC_RICHGAMEMEDIA_H
#define AGS_ENGINE_AC_RICHGAMEMEDIA_H

#include "common/algorithm.h"
#include "common/str.h"

namespace AGS3 {

// Windows Vista Rich Save Games, modified to be platform-agnostic

#define RM_MAXLENGTH    1024
#define RM_MAGICNUMBER  MKTAG('H', 'M', 'G', 'R')

// Forward declaration
namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

#pragma pack(push)
#pragma pack(1)
struct RICH_GAME_MEDIA_HEADER {
private:
	template<class SRC, class DEST>
	static void uconvert(const SRC *src, DEST *dest, size_t maxSize) {
		do {
			*dest++ = *src;
		} while (*src++ != 0 && --maxSize > 1);

		*dest = '\0';
	}
public:
	uint32    dwMagicNumber;
	int       dwHeaderVersion;
	int       dwHeaderSize;
	uint32    dwThumbnailOffsetLowerDword;
	uint32    dwThumbnailOffsetHigherDword;
	int       dwThumbnailSize;
	byte guidGameId[16];
	uint16 szGameName[RM_MAXLENGTH];
	uint16 szSaveName[RM_MAXLENGTH];
	uint16 szLevelName[RM_MAXLENGTH];
	uint16 szComments[RM_MAXLENGTH];

	void ReadFromFile(Shared::Stream *in);
	void WriteToFile(Shared::Stream *out);

	void setSaveName(const Common::String &saveName);
	Common::String getSaveName() const;
};
#pragma pack(pop)

} // namespace AGS3

#endif
