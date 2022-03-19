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

#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/view_frame.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/engine/ac/dynobj/cc_audio_clip.h"
#include "ags/engine/ac/draw.h"
#include "ags/shared/ac/game_version.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/globals.h"

namespace AGS3 {

using AGS::Shared::Bitmap;
using AGS::Shared::Graphics;

int ViewFrame_GetFlipped(ScriptViewFrame *svf) {
	if (_GP(views)[svf->view].loops[svf->loop].frames[svf->frame].flags & VFLG_FLIPSPRITE)
		return 1;
	return 0;
}

int ViewFrame_GetGraphic(ScriptViewFrame *svf) {
	return _GP(views)[svf->view].loops[svf->loop].frames[svf->frame].pic;
}

void ViewFrame_SetGraphic(ScriptViewFrame *svf, int newPic) {
	_GP(views)[svf->view].loops[svf->loop].frames[svf->frame].pic = newPic;
}

ScriptAudioClip *ViewFrame_GetLinkedAudio(ScriptViewFrame *svf) {
	int soundIndex = _GP(views)[svf->view].loops[svf->loop].frames[svf->frame].sound;
	if (soundIndex < 0)
		return nullptr;

	return &_GP(game).audioClips[soundIndex];
}

void ViewFrame_SetLinkedAudio(ScriptViewFrame *svf, ScriptAudioClip *clip) {
	int newSoundIndex = -1;
	if (clip != nullptr)
		newSoundIndex = clip->id;

	_GP(views)[svf->view].loops[svf->loop].frames[svf->frame].sound = newSoundIndex;
}

int ViewFrame_GetSound(ScriptViewFrame *svf) {
	// convert audio clip to old-style sound number
	return get_old_style_number_for_sound(_GP(views)[svf->view].loops[svf->loop].frames[svf->frame].sound);
}

void ViewFrame_SetSound(ScriptViewFrame *svf, int newSound) {
	if (newSound < 1) {
		_GP(views)[svf->view].loops[svf->loop].frames[svf->frame].audioclip = -1;
	} else {
		// convert sound number to audio clip
		ScriptAudioClip *clip = GetAudioClipForOldStyleNumber(_GP(game), false, newSound);
		if (clip == nullptr)
			quitprintf("!SetFrameSound: audio clip aSound%d not found", newSound);

		_GP(views)[svf->view].loops[svf->loop].frames[svf->frame].sound =
			_GP(game).IsLegacyAudioSystem() ? newSound : clip->id;
		_GP(views)[svf->view].loops[svf->loop].frames[svf->frame].audioclip = clip->id;
	}
}

int ViewFrame_GetSpeed(ScriptViewFrame *svf) {
	return _GP(views)[svf->view].loops[svf->loop].frames[svf->frame].speed;
}

int ViewFrame_GetView(ScriptViewFrame *svf) {
	return svf->view + 1;
}

int ViewFrame_GetLoop(ScriptViewFrame *svf) {
	return svf->loop;
}

int ViewFrame_GetFrame(ScriptViewFrame *svf) {
	return svf->frame;
}

//=============================================================================

void precache_view(int view) {
	if (view < 0)
		return;

	for (int i = 0; i < _GP(views)[view].numLoops; i++) {
		for (int j = 0; j < _GP(views)[view].loops[i].numFrames; j++)
			_GP(spriteset).Precache(_GP(views)[view].loops[i].frames[j].pic);
	}
}

// the specified frame has just appeared, see if we need
// to play a sound or whatever
void CheckViewFrame(int view, int loop, int frame, int sound_volume) {
	ScriptAudioChannel *channel = nullptr;
	if (_GP(game).IsLegacyAudioSystem()) {
		// sound field contains legacy sound num, so we also need an actual clip index
		const int sound = _GP(views)[view].loops[loop].frames[frame].sound;
		int &clip_id = _GP(views)[view].loops[loop].frames[frame].audioclip;
		if (sound > 0) {
			if (clip_id < 0) {
				ScriptAudioClip *clip = GetAudioClipForOldStyleNumber(_GP(game), false, sound);
				if (!clip)
					return;
				clip_id = clip->id;
			}
			channel = play_audio_clip_by_index(clip_id);
		}
	} else {
		if (_GP(views)[view].loops[loop].frames[frame].sound >= 0) {
			// play this sound (eg. footstep)
			channel = play_audio_clip_by_index(_GP(views)[view].loops[loop].frames[frame].sound);
		}
	}
	if (sound_volume != SCR_NO_VALUE && channel != nullptr) {
		auto *ch = AudioChans::GetChannel(channel->id);

		if (ch)
			ch->set_volume100(ch->get_volume100() * sound_volume / 100);
	}
}

// draws a view frame, flipped if appropriate
void DrawViewFrame(Bitmap *ds, const ViewFrame *vframe, int x, int y, bool alpha_blend) {
	// NOTE: DrawViewFrame supports alpha blending only since OPT_SPRITEALPHA;
	// this is why there's no sense in blending if it's not set (will do no good anyway).
	if (alpha_blend && _GP(game).options[OPT_SPRITEALPHA] == kSpriteAlphaRender_Proper) {
		Bitmap *vf_bmp = _GP(spriteset)[vframe->pic];
		Bitmap *src = vf_bmp;
		if (vframe->flags & VFLG_FLIPSPRITE) {
			src = new Bitmap(vf_bmp->GetWidth(), vf_bmp->GetHeight(), vf_bmp->GetColorDepth());
			src->FlipBlt(vf_bmp, 0, 0, Shared::kBitmap_HFlip);
		}
		draw_sprite_support_alpha(ds, true, x, y, src, (_GP(game).SpriteInfos[vframe->pic].Flags & SPF_ALPHACHANNEL) != 0);
		if (src != vf_bmp)
			delete src;
	} else {
		if (vframe->flags & VFLG_FLIPSPRITE)
			ds->FlipBlt(_GP(spriteset)[vframe->pic], x, y, Shared::kBitmap_HFlip);
		else
			ds->Blit(_GP(spriteset)[vframe->pic], x, y, Shared::kBitmap_Transparency);
	}
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// int (ScriptViewFrame *svf)
RuntimeScriptValue Sc_ViewFrame_GetFlipped(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptViewFrame, ViewFrame_GetFlipped);
}

// int (ScriptViewFrame *svf)
RuntimeScriptValue Sc_ViewFrame_GetFrame(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptViewFrame, ViewFrame_GetFrame);
}
// int (ScriptViewFrame *svf)
RuntimeScriptValue Sc_ViewFrame_GetGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptViewFrame, ViewFrame_GetGraphic);
}

// void (ScriptViewFrame *svf, int newPic)
RuntimeScriptValue Sc_ViewFrame_SetGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptViewFrame, ViewFrame_SetGraphic);
}

// ScriptAudioClip* (ScriptViewFrame *svf)
RuntimeScriptValue Sc_ViewFrame_GetLinkedAudio(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(ScriptViewFrame, ScriptAudioClip, _GP(ccDynamicAudioClip), ViewFrame_GetLinkedAudio);
}

// void (ScriptViewFrame *svf, ScriptAudioClip* clip)
RuntimeScriptValue Sc_ViewFrame_SetLinkedAudio(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(ScriptViewFrame, ViewFrame_SetLinkedAudio, ScriptAudioClip);
}

// int (ScriptViewFrame *svf)
RuntimeScriptValue Sc_ViewFrame_GetLoop(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptViewFrame, ViewFrame_GetLoop);
}

// int (ScriptViewFrame *svf)
RuntimeScriptValue Sc_ViewFrame_GetSound(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptViewFrame, ViewFrame_GetSound);
}

// void (ScriptViewFrame *svf, int newSound)
RuntimeScriptValue Sc_ViewFrame_SetSound(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptViewFrame, ViewFrame_SetSound);
}

// int (ScriptViewFrame *svf)
RuntimeScriptValue Sc_ViewFrame_GetSpeed(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptViewFrame, ViewFrame_GetSpeed);
}

// int (ScriptViewFrame *svf)
RuntimeScriptValue Sc_ViewFrame_GetView(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptViewFrame, ViewFrame_GetView);
}


void RegisterViewFrameAPI() {
	ccAddExternalObjectFunction("ViewFrame::get_Flipped", Sc_ViewFrame_GetFlipped);
	ccAddExternalObjectFunction("ViewFrame::get_Frame", Sc_ViewFrame_GetFrame);
	ccAddExternalObjectFunction("ViewFrame::get_Graphic", Sc_ViewFrame_GetGraphic);
	ccAddExternalObjectFunction("ViewFrame::set_Graphic", Sc_ViewFrame_SetGraphic);
	ccAddExternalObjectFunction("ViewFrame::get_LinkedAudio", Sc_ViewFrame_GetLinkedAudio);
	ccAddExternalObjectFunction("ViewFrame::set_LinkedAudio", Sc_ViewFrame_SetLinkedAudio);
	ccAddExternalObjectFunction("ViewFrame::get_Loop", Sc_ViewFrame_GetLoop);
	ccAddExternalObjectFunction("ViewFrame::get_Sound", Sc_ViewFrame_GetSound);
	ccAddExternalObjectFunction("ViewFrame::set_Sound", Sc_ViewFrame_SetSound);
	ccAddExternalObjectFunction("ViewFrame::get_Speed", Sc_ViewFrame_GetSpeed);
	ccAddExternalObjectFunction("ViewFrame::get_View", Sc_ViewFrame_GetView);
}

} // namespace AGS3
