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
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/screen_overlay.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/globals.h"

namespace AGS3 {

int ScriptOverlay::Dispose(const char *address, bool force) {
	// since the managed object is being deleted, remove the
	// reference so it doesn't try and dispose something else
	// with that handle later
	int overlayIndex = find_overlay_of_type(overlayId);
	if (overlayIndex >= 0) {
		_GP(screenover)[overlayIndex].associatedOverlayHandle = 0;
	}

	// if this is being removed voluntarily (ie. pointer out of
	// scope) then remove the associateed overlay
	// Otherwise, it's a Restre Game or something so don't
	if ((!force) && (!isBackgroundSpeech) && (Overlay_GetValid(this))) {
		Remove();
	}

	delete this;
	return 1;
}

const char *ScriptOverlay::GetType() {
	return "Overlay";
}

int ScriptOverlay::Serialize(const char *address, char *buffer, int bufsize) {
	StartSerialize(buffer);
	SerializeInt(overlayId);
	SerializeInt(borderWidth);
	SerializeInt(borderHeight);
	SerializeInt(isBackgroundSpeech);
	return EndSerialize();
}

void ScriptOverlay::Unserialize(int index, const char *serializedData, int dataSize) {
	StartUnserialize(serializedData, dataSize);
	overlayId = UnserializeInt();
	borderWidth = UnserializeInt();
	borderHeight = UnserializeInt();
	isBackgroundSpeech = UnserializeInt();
	ccRegisterUnserializedObject(index, this, this);
}

void ScriptOverlay::Remove() {
	int overlayIndex = find_overlay_of_type(overlayId);
	if (overlayIndex < 0) {
		debug_script_warn("Overlay.Remove: overlay is invalid, could have been removed earlier.");
		return;
	}
	remove_screen_overlay_index(overlayIndex);
	overlayId = -1;
}


ScriptOverlay::ScriptOverlay() {
	overlayId = -1;
	borderWidth = 0;
	borderHeight = 0;
	isBackgroundSpeech = 0;
}

} // namespace AGS3
