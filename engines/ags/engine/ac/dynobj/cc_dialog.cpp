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

#include "ags/engine/ac/dynobj/cc_dialog.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/engine/ac/dialog.h"
#include "ags/shared/ac/dialog_topic.h"
#include "ags/shared/ac/game_struct_defines.h"
#include "ags/shared/util/stream.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

// return the type name of the object
const char *CCDialog::GetType() {
	return "Dialog";
}

size_t CCDialog::CalcSerializeSize(const void * /*address*/) {
	return sizeof(int32_t);
}

// serialize the object into BUFFER (which is BUFSIZE bytes)
// return number of bytes used
void CCDialog::Serialize(const void *address, Stream *out) {
	const ScriptDialog *shh = static_cast<const ScriptDialog *>(address);
	out->WriteInt32(shh->id);
}

void CCDialog::Unserialize(int index, Stream *in, size_t data_sz) {
	int num = in->ReadInt32();
	ccRegisterUnserializedObject(index, &_GP(scrDialog)[num], this);
}

} // namespace AGS3
