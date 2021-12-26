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

#include "ags/engine/ac/move_list.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

void MoveList::ReadFromFile_Legacy(Stream *in) {
	in->ReadArrayOfInt32(pos, MAXNEEDSTAGES_LEGACY);
	numstage = in->ReadInt32();
	in->ReadArrayOfInt32(xpermove, MAXNEEDSTAGES_LEGACY);
	in->ReadArrayOfInt32(ypermove, MAXNEEDSTAGES_LEGACY);
	fromx = in->ReadInt32();
	fromy = in->ReadInt32();
	onstage = in->ReadInt32();
	onpart = in->ReadInt32();
	lastx = in->ReadInt32();
	lasty = in->ReadInt32();
	doneflag = in->ReadInt8();
	direct = in->ReadInt8();
}

HSaveError MoveList::ReadFromFile(Stream *in, int32_t cmp_ver) {
	if (cmp_ver < 1) {
		ReadFromFile_Legacy(in);
		return HSaveError::None();
	}

	numstage = in->ReadInt32();
	// TODO: reimplement MoveList stages as vector to avoid these limits
	if (numstage > MAXNEEDSTAGES) {
		return new SavegameError(kSvgErr_IncompatibleEngine,
		                         String::FromFormat("Incompatible number of movelist steps (count: %d, max : %d).", numstage, MAXNEEDSTAGES));
	}

	fromx = in->ReadInt32();
	fromy = in->ReadInt32();
	onstage = in->ReadInt32();
	onpart = in->ReadInt32();
	lastx = in->ReadInt32();
	lasty = in->ReadInt32();
	doneflag = in->ReadInt8();
	direct = in->ReadInt8();

	in->ReadArrayOfInt32(pos, numstage);
	in->ReadArrayOfInt32(xpermove, numstage);
	in->ReadArrayOfInt32(ypermove, numstage);
	return HSaveError::None();
}

void MoveList::WriteToFile(Stream *out) {
	out->WriteInt32(numstage);
	out->WriteInt32(fromx);
	out->WriteInt32(fromy);
	out->WriteInt32(onstage);
	out->WriteInt32(onpart);
	out->WriteInt32(lastx);
	out->WriteInt32(lasty);
	out->WriteInt8(doneflag);
	out->WriteInt8(direct);

	out->WriteArrayOfInt32(pos, numstage);
	out->WriteArrayOfInt32(xpermove, numstage);
	out->WriteArrayOfInt32(ypermove, numstage);
}

} // namespace AGS3
