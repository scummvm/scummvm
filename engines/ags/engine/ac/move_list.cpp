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
#include "ags/shared/util/bbop.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

float MoveList::GetStepLength() const {
	assert(numstage > 0);
	float permove_x = fixtof(xpermove[onstage]);
	float permove_y = fixtof(ypermove[onstage]);
	return sqrt(permove_x * permove_x + permove_y * permove_y);
}

float MoveList::GetPixelUnitFraction() const {
	assert(numstage > 0);
	float distance = GetStepLength() * onpart;
	return distance - floor(distance);
}

void MoveList::SetPixelUnitFraction(float frac) {
	assert(numstage > 0);
	float permove_dist = GetStepLength();
	onpart = permove_dist > 0.f ? (1.f / permove_dist) * frac : 0.f;
}

void MoveList::ReadFromSavegame_Legacy(Stream *in) {
	*this = MoveList(); // reset struct
	for (int i = 0; i < MAXNEEDSTAGES_LEGACY; ++i) {
		// X & Y was packed as high/low shorts, and hence reversed in lo-end
		pos[i].Y = in->ReadInt16();
		pos[i].X = in->ReadInt16();
	}
	numstage = in->ReadInt32();
	in->ReadArrayOfInt32(xpermove, MAXNEEDSTAGES_LEGACY);
	in->ReadArrayOfInt32(ypermove, MAXNEEDSTAGES_LEGACY);
	from.X = in->ReadInt32();
	from.Y = in->ReadInt32();
	onstage = in->ReadInt32();
	onpart = static_cast<float>(in->ReadInt32());
	in->ReadInt32(); // UNUSED
	in->ReadInt32(); // UNUSED
	doneflag = in->ReadInt8();
	direct = in->ReadInt8();
	in->ReadInt16(); // alignment padding to int32 (finalize struct)
}

HSaveError MoveList::ReadFromSavegame(Stream *in, int32_t cmp_ver) {
	if (cmp_ver < kMoveSvgVersion_350) {
		return new SavegameError(kSvgErr_UnsupportedComponentVersion,
								 String::FromFormat("Movelist format %d is no longer supported", cmp_ver));
	}

	*this = MoveList(); // reset struct
	numstage = in->ReadInt32();
	if ((numstage == 0) && cmp_ver >= kMoveSvgVersion_36109) {
		return HSaveError::None();
	}
	// TODO: reimplement MoveList stages as vector to avoid these limits
	if (numstage > MAXNEEDSTAGES) {
		return new SavegameError(kSvgErr_IncompatibleEngine,
		                         String::FromFormat("Incompatible number of movelist steps (count: %d, max : %d).", numstage, MAXNEEDSTAGES));
	}

	from.X = in->ReadInt32();
	from.Y = in->ReadInt32();
	onstage = in->ReadInt32();
	BBOp::IntFloatSwap onpart_u(in->ReadInt32());
	in->ReadInt32(); // UNUSED
	in->ReadInt32(); // UNUSED
	doneflag = in->ReadInt8();
	direct = in->ReadInt8();

	for (int i = 0; i < numstage; ++i) {
		// X & Y was packed as high/low shorts, and hence reversed in lo-end
		pos[i].Y = in->ReadInt16();
		pos[i].X = in->ReadInt16();
	}
	in->ReadArrayOfInt32(xpermove, numstage);
	in->ReadArrayOfInt32(ypermove, numstage);

	// Some variables require conversion depending on a save version
	if (cmp_ver < kMoveSvgVersion_36109)
		onpart = static_cast<float>(onpart_u.val.i32);
	else
		onpart = onpart_u.val.f;

	return HSaveError::None();
}

void MoveList::WriteToSavegame(Stream *out) const {
	out->WriteInt32(numstage);
	if (numstage == 0)
		return;

	out->WriteInt32(from.X);
	out->WriteInt32(from.Y);
	out->WriteInt32(onstage);
	out->WriteInt32(BBOp::IntFloatSwap(onpart).val.i32);
	out->WriteInt32(0); // UNUSED
	out->WriteInt32(0); // UNUSED
	out->WriteInt8(doneflag);
	out->WriteInt8(direct);

	for (int i = 0; i < numstage; ++i) {
		// X & Y was packed as high/low shorts, and hence reversed in lo-end
		out->WriteInt16(pos[i].Y);
		out->WriteInt16(pos[i].X);
	}
	out->WriteArrayOfInt32(xpermove, numstage);
	out->WriteArrayOfInt32(ypermove, numstage);
}

} // namespace AGS3
