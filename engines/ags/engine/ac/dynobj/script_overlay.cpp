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

#include "ags/engine/ac/dynobj/script_overlay.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/util/stream.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/screen_overlay.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

int ScriptOverlay::Dispose(void * /*address*/, bool force) {
	// since the managed object is being deleted, remove the
	// reference so it doesn't try and dispose something else
	// with that handle later
	if (overlayId >= 0) {
		auto *over = get_overlay(overlayId);
		if (over) {
			over->associatedOverlayHandle = 0;
		}
	}

	// if this is being removed voluntarily (ie. pointer out of
	// scope) then remove the associateed overlay
	// Otherwise, it's a Restore Game or something so don't
	if ((!force) && (Overlay_GetValid(this))) {
		Remove();
	}

	delete this;
	return 1;
}

const char *ScriptOverlay::GetType() {
	return "Overlay";
}

size_t ScriptOverlay::CalcSerializeSize(const void * /*address*/) {
	return sizeof(int32_t) * 4;
}

void ScriptOverlay::Serialize(const void * /*address*/, Stream *out) {
	out->WriteInt32(overlayId);
	out->WriteInt32(0); // unused (was text window x padding)
	out->WriteInt32(0); // unused (was text window y padding)
	out->WriteInt32(0); // unused (was internal ref flag)
}

void ScriptOverlay::Unserialize(int index, Stream *in, size_t data_sz) {
	overlayId = in->ReadInt32();
	in->ReadInt32(); // unused (was text window x padding)
	in->ReadInt32(); // unused (was text window y padding)
	in->ReadInt32(); // unused (was internal ref flag)
	ccRegisterUnserializedObject(index, this, this);
}

void ScriptOverlay::Remove() {
	if (overlayId < 0) {
		debug_script_warn("Overlay.Remove: overlay is invalid, could have been removed earlier.");
		return;
	}
	remove_screen_overlay(overlayId);
	overlayId = -1;
}

} // namespace AGS3
