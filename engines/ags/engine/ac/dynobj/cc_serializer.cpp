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

#include "ags/shared/util/memory_stream.h"
#include "ags/engine/ac/dynobj/cc_serializer.h"
#include "ags/engine/ac/dynobj/all_dynamic_classes.h"
#include "ags/engine/ac/dynobj/all_script_classes.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/engine/ac/dynobj/cc_dynamic_array.h"
#include "ags/engine/ac/dynobj/script_user_object.h"
#include "ags/engine/ac/dynobj/script_camera.h"
#include "ags/engine/ac/dynobj/script_containers.h"
#include "ags/engine/ac/dynobj/script_file.h"
#include "ags/engine/ac/dynobj/script_viewport.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/plugins/plugin_engine.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

// *** De-serialization of script objects

void AGSDeSerializer::Unserialize(int32_t index, const char *objectType, const char *serializedData, int dataSize) {
	if (dataSize < 0) {
		quitprintf("Unserialise: invalid data size (%d) for object type '%s'", dataSize, objectType);
		return; // TODO: don't quit, return error
	}
	// Note that while our builtin classes may accept Stream object,
	// classes registered by plugin cannot, because streams are not (yet)
	// part of the plugin API.
	size_t data_sz = static_cast<size_t>(dataSize);
	assert(data_sz <= INT32_MAX); // dynamic object API does not support size > int32
	MemoryStream mems(reinterpret_cast<const uint8_t *>(serializedData), dataSize);

	// TODO: consider this: there are object types that are part of the
	// script's foundation, because they are created by the bytecode ops:
	// such as DynamicArray and UserObject. *Maybe* these should be moved
	// to certain "base serializer" class which guarantees their restoration.
	//
	// TODO: should we support older save versions here (DynArray, UserObj)?
	// might have to use older class names to distinguish save formats
	if (strcmp(objectType, CCDynamicArray::TypeName) == 0) {
		_GP(globalDynamicArray).Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, ScriptUserObject::TypeName) == 0) {
		ScriptUserObject *suo = new ScriptUserObject();
		suo->Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "GUIObject") == 0) {
		_GP(ccDynamicGUIObject).Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "Character") == 0) {
		_GP(ccDynamicCharacter).Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "Hotspot") == 0) {
		_GP(ccDynamicHotspot).Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "Region") == 0) {
		_GP(ccDynamicRegion).Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "Inventory") == 0) {
		_GP(ccDynamicInv).Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "Dialog") == 0) {
		_GP(ccDynamicDialog).Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "GUI") == 0) {
		_GP(ccDynamicGUI).Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "Object") == 0) {
		_GP(ccDynamicObject).Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "String") == 0) {
		_GP(myScriptStringImpl).Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "File") == 0) {
		// files cannot be restored properly -- so just recreate
		// the object; attempting any operations on it will fail
		sc_File *scf = new sc_File();
		ccRegisterUnserializedObject(index, scf, scf);
	} else if (strcmp(objectType, "Overlay") == 0) {
		ScriptOverlay *scf = new ScriptOverlay();
		scf->Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "DateTime") == 0) {
		ScriptDateTime *scf = new ScriptDateTime();
		scf->Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "ViewFrame") == 0) {
		ScriptViewFrame *scf = new ScriptViewFrame();
		scf->Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "DynamicSprite") == 0) {
		ScriptDynamicSprite *scf = new ScriptDynamicSprite();
		scf->Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "DrawingSurface") == 0) {
		ScriptDrawingSurface *sds = new ScriptDrawingSurface();
		sds->Unserialize(index, &mems, data_sz);

		if (sds->isLinkedBitmapOnly) {
			_G(dialogOptionsRenderingSurface) = sds;
		}
	} else if (strcmp(objectType, "DialogOptionsRendering") == 0) {
		_GP(ccDialogOptionsRendering).Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "StringDictionary") == 0) {
		Dict_Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "StringSet") == 0) {
		Set_Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "Viewport2") == 0) {
		Viewport_Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "Camera2") == 0) {
		Camera_Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "AudioChannel") == 0) {
		_GP(ccDynamicAudio).Unserialize(index, &mems, data_sz);
	} else if (strcmp(objectType, "AudioClip") == 0) {
		_GP(ccDynamicAudioClip).Unserialize(index, &mems, data_sz);
	} else {
		// check if the type is read by a plugin
		for (const auto &pr : _GP(pluginReaders)) {
			if (pr.Type == objectType) {
				if (dataSize == 0) { // avoid unserializing stubbed plugins
					debug(0, "Skipping %s plugin unserialization (dataSize = 0)", objectType);
					return;
				}
				pr.Reader->Unserialize(index, serializedData, dataSize);
				return;
			}
		}
		quitprintf("Unserialise: unknown object type: '%s'", objectType);
	}
}

} // namespace AGS3
