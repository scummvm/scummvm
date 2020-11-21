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

#include <string.h>
#include "ac/dynobj/cc_serializer.h"
#include "ac/dynobj/all_dynamicclasses.h"
#include "ac/dynobj/all_scriptclasses.h"
#include "ac/dynobj/scriptcamera.h"
#include "ac/dynobj/scriptcontainers.h"
#include "ac/dynobj/scriptfile.h"
#include "ac/dynobj/scriptuserobject.h"
#include "ac/dynobj/scriptviewport.h"
#include "ac/game.h"
#include "debug/debug_log.h"
#include "plugin/agsplugin.h"
#include "plugin/pluginobjectreader.h"

extern CCGUIObject ccDynamicGUIObject;
extern CCCharacter ccDynamicCharacter;
extern CCHotspot   ccDynamicHotspot;
extern CCRegion    ccDynamicRegion;
extern CCInventory ccDynamicInv;
extern CCGUI       ccDynamicGUI;
extern CCObject    ccDynamicObject;
extern CCDialog    ccDynamicDialog;
extern ScriptDrawingSurface* dialogOptionsRenderingSurface;
extern ScriptDialogOptionsRendering ccDialogOptionsRendering;
extern PluginObjectReader pluginReaders[MAX_PLUGIN_OBJECT_READERS];
extern int numPluginReaders;

// *** De-serialization of script objects



void AGSDeSerializer::Unserialize(int index, const char *objectType, const char *serializedData, int dataSize) {
    if (strcmp(objectType, "GUIObject") == 0) {
        ccDynamicGUIObject.Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "Character") == 0) {
        ccDynamicCharacter.Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "Hotspot") == 0) {
        ccDynamicHotspot.Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "Region") == 0) {
        ccDynamicRegion.Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "Inventory") == 0) {
        ccDynamicInv.Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "Dialog") == 0) {
        ccDynamicDialog.Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "GUI") == 0) {
        ccDynamicGUI.Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "Object") == 0) {
        ccDynamicObject.Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "String") == 0) {
        ScriptString *scf = new ScriptString();
        scf->Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "File") == 0) {
        // files cannot be restored properly -- so just recreate
        // the object; attempting any operations on it will fail
        sc_File *scf = new sc_File();
        ccRegisterUnserializedObject(index, scf, scf);
    }
    else if (strcmp(objectType, "Overlay") == 0) {
        ScriptOverlay *scf = new ScriptOverlay();
        scf->Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "DateTime") == 0) {
        ScriptDateTime *scf = new ScriptDateTime();
        scf->Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "ViewFrame") == 0) {
        ScriptViewFrame *scf = new ScriptViewFrame();
        scf->Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "DynamicSprite") == 0) {
        ScriptDynamicSprite *scf = new ScriptDynamicSprite();
        scf->Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "DrawingSurface") == 0) {
        ScriptDrawingSurface *sds = new ScriptDrawingSurface();
        sds->Unserialize(index, serializedData, dataSize);

        if (sds->isLinkedBitmapOnly)
        {
            dialogOptionsRenderingSurface = sds;
        }
    }
    else if (strcmp(objectType, "DialogOptionsRendering") == 0)
    {
        ccDialogOptionsRendering.Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "StringDictionary") == 0)
    {
        Dict_Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "StringSet") == 0)
    {
        Set_Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "Viewport2") == 0)
    {
        Viewport_Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "Camera2") == 0)
    {
        Camera_Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "UserObject") == 0) {
        ScriptUserObject *suo = new ScriptUserObject();
        suo->Unserialize(index, serializedData, dataSize);
    }
    else if (!unserialize_audio_script_object(index, objectType, serializedData, dataSize)) 
    {
        // check if the type is read by a plugin
        for (int ii = 0; ii < numPluginReaders; ii++) {
            if (strcmp(objectType, pluginReaders[ii].type) == 0) {
                pluginReaders[ii].reader->Unserialize(index, serializedData, dataSize);
                return;
            }
        }
        quitprintf("Unserialise: unknown object type: '%s'", objectType);
    }
}

AGSDeSerializer ccUnserializer;

