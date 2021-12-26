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

#include "ags/engine/ac/rich_game_media.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {

using AGS::Shared::Stream;

void RICH_GAME_MEDIA_HEADER::ReadFromFile(Stream *in) {
	dwMagicNumber = in->ReadInt32();
	dwHeaderVersion = in->ReadInt32();
	dwHeaderSize = in->ReadInt32();
	dwThumbnailOffsetLowerDword = in->ReadInt32();
	dwThumbnailOffsetHigherDword = in->ReadInt32();
	dwThumbnailSize = in->ReadInt32();
	in->Read(guidGameId, 16);
	in->ReadArrayOfInt16((int16_t *)szGameName, RM_MAXLENGTH);
	in->ReadArrayOfInt16((int16_t *)szSaveName, RM_MAXLENGTH);
	in->ReadArrayOfInt16((int16_t *)szLevelName, RM_MAXLENGTH);
	in->ReadArrayOfInt16((int16_t *)szComments, RM_MAXLENGTH);
}

void RICH_GAME_MEDIA_HEADER::WriteToFile(Stream *out) {
	out->WriteInt32(dwMagicNumber);
	out->WriteInt32(dwHeaderVersion);
	out->WriteInt32(dwHeaderSize);
	out->WriteInt32(dwThumbnailOffsetLowerDword);
	out->WriteInt32(dwThumbnailOffsetHigherDword);
	out->WriteInt32(dwThumbnailSize);
	out->Write(guidGameId, 16);
	out->WriteArrayOfInt16((int16_t *)szGameName, RM_MAXLENGTH);
	out->WriteArrayOfInt16((int16_t *)szSaveName, RM_MAXLENGTH);
	out->WriteArrayOfInt16((int16_t *)szLevelName, RM_MAXLENGTH);
	out->WriteArrayOfInt16((int16_t *)szComments, RM_MAXLENGTH);
}

void RICH_GAME_MEDIA_HEADER::setSaveName(const Common::String &saveName) {
	uconvert<char, uint16>(saveName.c_str(), szSaveName, RM_MAXLENGTH);
}

Common::String RICH_GAME_MEDIA_HEADER::getSaveName() const {
	char buf[RM_MAXLENGTH];
	uconvert<uint16, char>(szSaveName, buf, RM_MAXLENGTH);

	return Common::String(buf);
}

} // namespace AGS3
