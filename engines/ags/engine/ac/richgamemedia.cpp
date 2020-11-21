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

#include "ac/richgamemedia.h"
#include "util/stream.h"

using AGS::Common::Stream;

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
