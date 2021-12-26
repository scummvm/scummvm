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

//include <string.h>
#include "ags/engine/ac/dynobj/cc_serializer.h"
#include "ags/engine/ac/dynobj/all_dynamic_classes.h"
#include "ags/engine/ac/dynobj/all_script_classes.h"
#include "ags/engine/ac/dynobj/script_camera.h"
#include "ags/engine/ac/dynobj/script_containers.h"
#include "ags/engine/ac/dynobj/script_file.h"
#include "ags/engine/ac/dynobj/script_user_object.h"
#include "ags/engine/ac/dynobj/script_viewport.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/plugins/ags_plugin.h"
#include "ags/plugins/plugin_object_reader.h"
#include "ags/globals.h"

namespace AGS3 {

// *** De-serialization of script objects

void AGSDeSerializer::Unserialize(int index, const char *objectType, const char *serializedData, int dataSize) {
	if (strcmp(objectType, "GUIObject") == 0) {
		_GP(ccDynamicGUIObject).Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "Character") == 0) {
		_GP(ccDynamicCharacter).Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "Hotspot") == 0) {
		_GP(ccDynamicHotspot).Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "Region") == 0) {
		_GP(ccDynamicRegion).Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "Inventory") == 0) {
		_GP(ccDynamicInv).Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "Dialog") == 0) {
		_GP(ccDynamicDialog).Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "GUI") == 0) {
		_GP(ccDynamicGUI).Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "Object") == 0) {
		_GP(ccDynamicObject).Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "String") == 0) {
		ScriptString *scf = new ScriptString();
		scf->Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "File") == 0) {
		// files cannot be restored properly -- so just recreate
		// the object; attempting any operations on it will fail
		sc_File *scf = new sc_File();
		ccRegisterUnserializedObject(index, scf, scf);
	} else if (strcmp(objectType, "Overlay") == 0) {
		ScriptOverlay *scf = new ScriptOverlay();
		scf->Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "DateTime") == 0) {
		ScriptDateTime *scf = new ScriptDateTime();
		scf->Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "ViewFrame") == 0) {
		ScriptViewFrame *scf = new ScriptViewFrame();
		scf->Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "DynamicSprite") == 0) {
		ScriptDynamicSprite *scf = new ScriptDynamicSprite();
		scf->Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "DrawingSurface") == 0) {
		ScriptDrawingSurface *sds = new ScriptDrawingSurface();
		sds->Unserialize(index, serializedData, dataSize);

		if (sds->isLinkedBitmapOnly) {
			_G(dialogOptionsRenderingSurface) = sds;
		}
	} else if (strcmp(objectType, "DialogOptionsRendering") == 0) {
		_GP(ccDialogOptionsRendering).Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "StringDictionary") == 0) {
		Dict_Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "StringSet") == 0) {
		Set_Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "Viewport2") == 0) {
		Viewport_Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "Camera2") == 0) {
		Camera_Unserialize(index, serializedData, dataSize);
	} else if (strcmp(objectType, "UserObject") == 0) {
		ScriptUserObject *suo = new ScriptUserObject();
		suo->Unserialize(index, serializedData, dataSize);
	} else if (!unserialize_audio_script_object(index, objectType, serializedData, dataSize)) {
		// check if the type is read by a plugin
		for (int ii = 0; ii < _G(numPluginReaders); ii++) {
			if (strcmp(objectType, _G(pluginReaders)[ii].type) == 0) {
				_G(pluginReaders)[ii].reader->Unserialize(index, serializedData, dataSize);
				return;
			}
		}
		quitprintf("Unserialise: unknown object type: '%s'", objectType);
	}
}

} // namespace AGS3
