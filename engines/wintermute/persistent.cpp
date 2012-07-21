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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/ad/ad_actor.h"
#include "engines/wintermute/ad/ad_entity.h"
#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/ad/ad_inventory.h"
#include "engines/wintermute/ad/ad_inventory_box.h"
#include "engines/wintermute/ad/ad_item.h"
#include "engines/wintermute/ad/ad_layer.h"
#include "engines/wintermute/ad/ad_node_state.h"
#include "engines/wintermute/ad/ad_object.h"
#include "engines/wintermute/ad/ad_path.h"
#include "engines/wintermute/ad/ad_path_point.h"
#include "engines/wintermute/ad/ad_region.h"
#include "engines/wintermute/ad/ad_response.h"
#include "engines/wintermute/ad/ad_response_box.h"
#include "engines/wintermute/ad/ad_response_context.h"
#include "engines/wintermute/ad/ad_rot_level.h"
#include "engines/wintermute/ad/ad_scale_level.h"
#include "engines/wintermute/ad/ad_scene.h"
#include "engines/wintermute/ad/ad_scene_node.h"
#include "engines/wintermute/ad/ad_scene_state.h"
#include "engines/wintermute/ad/ad_sentence.h"
#include "engines/wintermute/ad/ad_sprite_set.h"
#include "engines/wintermute/ad/ad_talk_def.h"
#include "engines/wintermute/ad/ad_talk_holder.h"
#include "engines/wintermute/ad/ad_talk_node.h"
#include "engines/wintermute/ad/ad_waypoint_group.h"
#include "engines/wintermute/base/base_fader.h"
#include "engines/wintermute/base/font/base_font_bitmap.h"
#include "engines/wintermute/base/font/base_font_storage.h"
#include "engines/wintermute/base/font/base_font_truetype.h"
#include "engines/wintermute/base/base_frame.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_keyboard_state.h"
#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/base/base_point.h"
#include "engines/wintermute/base/base_region.h"
#include "engines/wintermute/base/base_scriptable.h"
#include "engines/wintermute/base/base_script_holder.h"
#include "engines/wintermute/base/sound/base_sound.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/base_sub_frame.h"
#include "engines/wintermute/base/base_viewport.h"
#include "engines/wintermute/base/scriptables/script_engine.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_ext_array.h"
#include "engines/wintermute/base/scriptables/script_ext_date.h"
#include "engines/wintermute/base/scriptables/script_ext_file.h"
#include "engines/wintermute/base/scriptables/script_ext_math.h"
#include "engines/wintermute/base/scriptables/script_ext_mem_buffer.h"
#include "engines/wintermute/base/scriptables/script_ext_object.h"
#include "engines/wintermute/base/scriptables/script_ext_string.h"
#include "engines/wintermute/ui/ui_button.h"
#include "engines/wintermute/ui/ui_edit.h"
#include "engines/wintermute/ui/ui_entity.h"
#include "engines/wintermute/ui/ui_text.h"
#include "engines/wintermute/ui/ui_tiled_image.h"
#include "engines/wintermute/ui/ui_window.h"
#include "engines/wintermute/video/video_theora_player.h"
#include "engines/wintermute/system/sys_class.h"

// CSysClass adds these objects to the registry, thus they aren't as leaked as they look
#define REGISTER_CLASS(class_name, persistent_class)\
	new WinterMute::CSysClass(class_name::_className, class_name::persistBuild, class_name::persistLoad, persistent_class);

namespace WinterMute {

// This is done in a separate file, to avoid including the kitchensink in CSysClassRegistry.
void CSysClassRegistry::registerClasses() {
	REGISTER_CLASS(CAdActor, false)
	REGISTER_CLASS(CAdEntity, false)
	REGISTER_CLASS(CAdGame, true)
	REGISTER_CLASS(CAdInventory, false)
	REGISTER_CLASS(CAdInventoryBox, false)
	REGISTER_CLASS(CAdItem, false)
	REGISTER_CLASS(CAdLayer, false)
	REGISTER_CLASS(CAdNodeState, false)
	REGISTER_CLASS(CAdObject, false)
	REGISTER_CLASS(CAdPath, false)
	REGISTER_CLASS(CAdPathPoint, false)
	REGISTER_CLASS(CAdRegion, false)
	REGISTER_CLASS(CAdResponse, false)
	REGISTER_CLASS(CAdResponseBox, false)
	REGISTER_CLASS(CAdResponseContext, false)
	REGISTER_CLASS(CAdRotLevel, false)
	REGISTER_CLASS(CAdScaleLevel, false)
	REGISTER_CLASS(CAdScene, false)
	REGISTER_CLASS(CAdSceneNode, false)
	REGISTER_CLASS(CAdSceneState, false)
	REGISTER_CLASS(CAdSentence, false)
	REGISTER_CLASS(CAdSpriteSet, false)
	REGISTER_CLASS(CAdTalkDef, false)
	REGISTER_CLASS(CAdTalkHolder, false)
	REGISTER_CLASS(CAdTalkNode, false)
	REGISTER_CLASS(CAdWaypointGroup, false)

	REGISTER_CLASS(CBFader, false)
	REGISTER_CLASS(CBFont, false)
	REGISTER_CLASS(CBFontBitmap, false)
	REGISTER_CLASS(CBFontStorage, true)
	REGISTER_CLASS(CBFontTT, false)
	REGISTER_CLASS(CBFrame, false)
	REGISTER_CLASS(CBGame, true)
	REGISTER_CLASS(CBKeyboardState, false)
	REGISTER_CLASS(CBObject, false)
	REGISTER_CLASS(CBPoint, false)
	REGISTER_CLASS(CBRegion, false)
	REGISTER_CLASS(CBScriptable, false)
	REGISTER_CLASS(CBScriptHolder, false)
	REGISTER_CLASS(CBSound, false)
	REGISTER_CLASS(CBSprite, false)
	REGISTER_CLASS(CBSubFrame, false)

	REGISTER_CLASS(CBViewport, false)
	REGISTER_CLASS(CPartEmitter, false)
	REGISTER_CLASS(CScEngine, true)
	REGISTER_CLASS(CScScript, false)
	REGISTER_CLASS(CScStack, false)
	REGISTER_CLASS(CScValue, false)
	REGISTER_CLASS(CSXArray, false)
	REGISTER_CLASS(CSXDate, false)
	REGISTER_CLASS(CSXFile, false)
	REGISTER_CLASS(CSXMath, true)
	REGISTER_CLASS(CSXMemBuffer, false)
	REGISTER_CLASS(CSXObject, false)
	REGISTER_CLASS(CSXString, false)

	REGISTER_CLASS(CUIButton, false)
	REGISTER_CLASS(CUIEdit, false)
	REGISTER_CLASS(CUIEntity, false)
	REGISTER_CLASS(CUIObject, false)
	REGISTER_CLASS(CUIText, false)
	REGISTER_CLASS(CUITiledImage, false)
	REGISTER_CLASS(CUIWindow, false)
	REGISTER_CLASS(CVidTheoraPlayer, false)
	warning("TODO: Clean this up at some proper time");
}

}
