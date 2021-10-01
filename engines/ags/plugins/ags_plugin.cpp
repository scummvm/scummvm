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

#include "ags/lib/allegro.h"
#include "ags/lib/std/vector.h"
#include "ags/shared/core/platform.h"
#include "ags/plugins/ags_plugin.h"
#include "ags/plugins/core/core.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/view.h"
#include "ags/engine/ac/character_cache.h"
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/dynamic_sprite.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/global_walkable_area.h"
#include "ags/shared/ac/keycode.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/move_list.h"
#include "ags/engine/ac/object_cache.h"
#include "ags/engine/ac/parser.h"
#include "ags/engine/ac/path_helper.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/string.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/ac/dynobj/cc_dynamic_object_addr_and_manager.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/shared/font/fonts.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/device/mouse_w32.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/engine/gfx/gfx_util.h"
#include "ags/engine/gfx/gfxfilter.h"
#include "ags/shared/gui/gui_defines.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/main/graphics_mode.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/plugins/plugin_engine.h"
#include "ags/plugins/plugin_object_reader.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/engine/script/script.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/shared/util/file_stream.h"
#include "ags/engine/util/library.h"
#include "ags/engine/util/library_scummvm.h"
#include "ags/shared/util/memory.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string_compat.h"
#include "ags/shared/util/wgt2_allg.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Shared::Memory;
using namespace AGS::Engine;

const int PLUGIN_API_VERSION = 25;

// On save/restore, the Engine will provide the plugin with a handle. Because we only ever save to one file at a time,
// we can reuse the same handle.

void PluginSimulateMouseClick(int pluginButtonID) {
	_G(pluginSimulatedClick) = pluginButtonID - 1;
}

void IAGSEngine::AbortGame(const char *reason) {
	quit(reason);
}
const char *IAGSEngine::GetEngineVersion() {
	return get_engine_version();
}
void IAGSEngine::RegisterScriptFunction(const char *name, Plugins::ScriptContainer *instance) {
	ccAddExternalPluginFunction(name, instance);
}
void IAGSEngine::RegisterBuiltInFunction(const char *name, Plugins::ScriptContainer *instance) {
	ccAddExternalFunctionForPlugin(name, instance);
}
const char *IAGSEngine::GetGraphicsDriverID() {
	if (_G(gfxDriver) == nullptr)
		return nullptr;

	return _G(gfxDriver)->GetDriverID();
}

BITMAP *IAGSEngine::GetScreen() {
	// TODO: we could actually return stage buffer here, will that make a difference?
	if (!_G(gfxDriver)->UsesMemoryBackBuffer())
		quit("!This plugin requires software graphics driver.");

	Bitmap *buffer = _G(gfxDriver)->GetMemoryBackBuffer();
	return buffer ? (BITMAP *)buffer->GetAllegroBitmap() : nullptr;
}

BITMAP *IAGSEngine::GetVirtualScreen() {
	Bitmap *stage = _G(gfxDriver)->GetStageBackBuffer(true);
	return stage ? (BITMAP *)stage->GetAllegroBitmap() : nullptr;
}

void IAGSEngine::RequestEventHook(int32 event) {
	if (event >= AGSE_TOOHIGH)
		quit("!IAGSEngine::RequestEventHook: invalid event requested");

	if ((event & AGSE_SCRIPTDEBUG) &&
	        ((_GP(plugins)[this->pluginId].wantHook & AGSE_SCRIPTDEBUG) == 0)) {
		_G(pluginsWantingDebugHooks)++;
		ccSetDebugHook(scriptDebugHook);
	}

	if (event & AGSE_AUDIODECODE) {
		quit("Plugin requested AUDIODECODE, which is no longer supported");
	}

	_GP(plugins)[this->pluginId].wantHook |= event;
}

void IAGSEngine::UnrequestEventHook(int32 event) {
	if (event >= AGSE_TOOHIGH)
		quit("!IAGSEngine::UnrequestEventHook: invalid event requested");

	if ((event & AGSE_SCRIPTDEBUG) &&
	        (_GP(plugins)[this->pluginId].wantHook & AGSE_SCRIPTDEBUG)) {
		_G(pluginsWantingDebugHooks)--;
		if (_G(pluginsWantingDebugHooks) < 1)
			ccSetDebugHook(nullptr);
	}

	_GP(plugins)[this->pluginId].wantHook &= ~event;
}

int IAGSEngine::GetSavedData(char *buffer, int32 bufsize) {
	int savedatasize = _GP(plugins)[this->pluginId].savedatasize;

	if (bufsize < savedatasize)
		quit("!IAGSEngine::GetSavedData: buffer too small");

	if (savedatasize > 0)
		memcpy(buffer, _GP(plugins)[this->pluginId].savedata, savedatasize);

	return savedatasize;
}

void IAGSEngine::DrawText(int32 x, int32 y, int32 font, int32 color, const char *text) {
	Bitmap *ds = _G(gfxDriver)->GetStageBackBuffer(true);
	if (!ds)
		return;
	color_t text_color = ds->GetCompatibleColor(color);
	draw_and_invalidate_text(ds, x, y, font, text_color, text);
}

void IAGSEngine::GetScreenDimensions(int32 *width, int32 *height, int32 *coldepth) {
	if (width != nullptr)
		width[0] = _GP(play).GetMainViewport().GetWidth();
	if (height != nullptr)
		height[0] = _GP(play).GetMainViewport().GetHeight();
	if (coldepth != nullptr)
		coldepth[0] = _GP(scsystem).coldepth;
}

int IAGSEngine::GetBitmapPitch(BITMAP *bmp) {
	return bmp->pitch;
}

uint8 *IAGSEngine::GetRawBitmapSurface(BITMAP *bmp) {
	Bitmap *stage = _G(gfxDriver)->GetStageBackBuffer(true);
	if (stage && bmp == stage->GetAllegroBitmap())
		_GP(plugins)[this->pluginId].invalidatedRegion = 0;

	return (uint8 *)bmp->getPixels();
}

void IAGSEngine::ReleaseBitmapSurface(BITMAP *bmp) {
	Bitmap *stage = _G(gfxDriver)->GetStageBackBuffer(true);
	if (stage && bmp == stage->GetAllegroBitmap()) {
		// plugin does not manaually invalidate stuff, so
		// we must invalidate the whole screen to be safe
		if (!_GP(plugins)[this->pluginId].invalidatedRegion)
			invalidate_screen();
	}
}

void IAGSEngine::GetMousePosition(int32 *x, int32 *y) {
	if (x) x[0] = _G(mousex);
	if (y) y[0] = _G(mousey);
}

int IAGSEngine::GetCurrentRoom() {
	return _G(displayed_room);
}

int IAGSEngine::GetNumBackgrounds() {
	return _GP(thisroom).BgFrameCount;
}

int IAGSEngine::GetCurrentBackground() {
	return _GP(play).bg_frame;
}

BITMAP *IAGSEngine::GetBackgroundScene(int32 index) {
	return (BITMAP *)_GP(thisroom).BgFrames[index].Graphic->GetAllegroBitmap();
}

void IAGSEngine::GetBitmapDimensions(BITMAP *bmp, int32 *width, int32 *height, int32 *coldepth) {
	if (bmp == nullptr)
		return;

	if (width != nullptr)
		width[0] = bmp->w;
	if (height != nullptr)
		height[0] = bmp->h;
	if (coldepth != nullptr)
		coldepth[0] = bitmap_color_depth(bmp);
}

void pl_set_file_handle(long data, Stream *stream) {
	_G(pl_file_handle) = data;
	_G(pl_file_stream) = stream;
}

void pl_clear_file_handle() {
	_G(pl_file_handle) = -1;
	_G(pl_file_stream) = nullptr;
}

int IAGSEngine::FRead(void *buffer, int32 len, int32 handle) {
	if (handle != _G(pl_file_handle)) {
		quitprintf("IAGSEngine::FRead: invalid file handle: %d", handle);
	}
	if (!_G(pl_file_stream)) {
		quit("IAGSEngine::FRead: file stream not set");
	}
	return _G(pl_file_stream)->Read(buffer, len);
}

int IAGSEngine::FWrite(void *buffer, int32 len, int32 handle) {
	if (handle != _G(pl_file_handle)) {
		quitprintf("IAGSEngine::FWrite: invalid file handle: %d", handle);
	}
	if (!_G(pl_file_stream)) {
		quit("IAGSEngine::FWrite: file stream not set");
	}
	return _G(pl_file_stream)->Write(buffer, len);
}

bool IAGSEngine::FSeek(soff_t offset, int origin, int32 handle) {
	if (handle != _G(pl_file_handle)) {
		quitprintf("IAGSEngine::FWrite: invalid file handle: %d", handle);
	}
	if (!_G(pl_file_stream)) {
		quit("IAGSEngine::FWrite: file stream not set");
	}
	return _G(pl_file_stream)->Seek(offset, (AGS::Shared::StreamSeek)origin);
}

void IAGSEngine::DrawTextWrapped(int32 xx, int32 yy, int32 wid, int32 font, int32 color, const char *text) {
	// TODO: use generic function from the engine instead of having copy&pasted code here
	int linespacing = getfontspacing_outlined(font);

	if (break_up_text_into_lines(text, _GP(Lines), wid, font) == 0)
		return;

	Bitmap *ds = _G(gfxDriver)->GetStageBackBuffer(true);
	if (!ds)
		return;
	color_t text_color = ds->GetCompatibleColor(color);
	data_to_game_coords((int *)&xx, (int *)&yy); // stupid! quick tweak
	for (size_t i = 0; i < _GP(Lines).Count(); i++)
		draw_and_invalidate_text(ds, xx, yy + linespacing * i, font, text_color, _GP(Lines)[i].GetCStr());
}

Bitmap glVirtualScreenWrap;
void IAGSEngine::SetVirtualScreen(BITMAP *bmp) {
	if (!_G(gfxDriver)->UsesMemoryBackBuffer()) {
		debug_script_warn("SetVirtualScreen: this plugin requires software graphics driver to work correctly.");
		// we let it continue since gfxDriver is supposed to ignore this request without throwing an exception
	}

	if (bmp) {
		glVirtualScreenWrap.WrapAllegroBitmap(bmp, true);
		_G(gfxDriver)->SetMemoryBackBuffer(&glVirtualScreenWrap);
	} else {
		glVirtualScreenWrap.Destroy();
		_G(gfxDriver)->SetMemoryBackBuffer(nullptr);
	}
}

int IAGSEngine::LookupParserWord(const char *word) {
	return find_word_in_dictionary(word);
}

void IAGSEngine::BlitBitmap(int32 x, int32 y, BITMAP *bmp, int32 masked) {
	Bitmap *ds = _G(gfxDriver)->GetStageBackBuffer(true);
	if (!ds)
		return;
	wputblock_raw(ds, x, y, bmp, masked);
	invalidate_rect(x, y, x + bmp->w, y + bmp->h, false);
}

void IAGSEngine::BlitSpriteTranslucent(int32 x, int32 y, BITMAP *bmp, int32 trans) {
	Bitmap *ds = _G(gfxDriver)->GetStageBackBuffer(true);
	if (!ds)
		return;
	Bitmap wrap(bmp, true);
	if (_G(gfxDriver)->UsesMemoryBackBuffer())
		GfxUtil::DrawSpriteWithTransparency(ds, &wrap, x, y, trans);
	else
		GfxUtil::DrawSpriteBlend(ds, Point(x, y), &wrap, kBlendMode_Alpha, true, false, trans);
}

void IAGSEngine::BlitSpriteRotated(int32 x, int32 y, BITMAP *bmp, int32 angle) {
	Bitmap *ds = _G(gfxDriver)->GetStageBackBuffer(true);
	if (!ds)
		return;
	// FIXME: call corresponding Graphics Blit
	rotate_sprite(ds->GetAllegroBitmap(), bmp, x, y, itofix(angle));
}

extern void domouse(int);

void IAGSEngine::PollSystem() {
	domouse(DOMOUSE_NOCURSOR);
	update_polled_stuff_if_runtime();
	int mbut, mwheelz;
	if (run_service_mb_controls(mbut, mwheelz) && mbut >= 0 && !_GP(play).IsIgnoringInput())
		pl_run_plugin_hooks(AGSE_MOUSECLICK, mbut);
	KeyInput kp;
	if (run_service_key_controls(kp) && !_GP(play).IsIgnoringInput()) {
		pl_run_plugin_hooks(AGSE_KEYPRESS, kp.Key);
	}
}

AGSCharacter *IAGSEngine::GetCharacter(int32 charnum) {
	if (charnum >= _GP(game).numcharacters)
		quit("!AGSEngine::GetCharacter: invalid character request");

	return (AGSCharacter *)&_GP(game).chars[charnum];
}
AGSGameOptions *IAGSEngine::GetGameOptions() {
	return (AGSGameOptions *)&_GP(play);
}
AGSColor *IAGSEngine::GetPalette() {
	return (AGSColor *)&_G(palette)[0];
}
void IAGSEngine::SetPalette(int32 start, int32 finish, AGSColor *cpl) {
	set_palette_range((RGB *)cpl, start, finish, 0);
}
int IAGSEngine::GetNumCharacters() {
	return _GP(game).numcharacters;
}
int IAGSEngine::GetPlayerCharacter() {
	return _GP(game).playercharacter;
}
void IAGSEngine::RoomToViewport(int32 *x, int32 *y) {
	Point scrp = _GP(play).RoomToScreen(x ? data_to_game_coord(*x) : 0, y ? data_to_game_coord(*y) : 0);
	if (x)
		*x = scrp.X;
	if (y)
		*y = scrp.Y;
}
void IAGSEngine::ViewportToRoom(int32 *x, int32 *y) {
	// NOTE: This is an old function that did not account for custom/multiple viewports
	// and does not expect to fail, therefore we always use primary viewport here.
	// (Not sure if it's good though)
	VpPoint vpt = _GP(play).ScreenToRoom(x ? game_to_data_coord(*x) : 0, y ? game_to_data_coord(*y) : 0);
	if (x)
		*x = vpt.first.X;
	if (y)
		*y = vpt.first.Y;
}
int IAGSEngine::GetNumObjects() {
	return _G(croom)->numobj;
}
AGSObject *IAGSEngine::GetObject(int32 num) {
	if (num >= _G(croom)->numobj)
		quit("!IAGSEngine::GetObject: invalid object");

	return (AGSObject *)&_G(croom)->obj[num];
}
BITMAP *IAGSEngine::CreateBlankBitmap(int32 width, int32 height, int32 coldep) {
	// [IKM] We should not create Bitmap object here, because
	// a) we are returning raw allegro bitmap and therefore loosing control over it
	// b) plugin won't use Bitmap anyway
	BITMAP *tempb = create_bitmap_ex(coldep, width, height);
	clear_to_color(tempb, bitmap_mask_color(tempb));
	return tempb;
}
void IAGSEngine::FreeBitmap(BITMAP *tofree) {
	if (tofree)
		destroy_bitmap(tofree);
}
BITMAP *IAGSEngine::GetSpriteGraphic(int32 num) {
	return (BITMAP *)_GP(spriteset)[num]->GetAllegroBitmap();
}
BITMAP *IAGSEngine::GetRoomMask(int32 index) {
	if (index == MASK_WALKABLE)
		return (BITMAP *)_GP(thisroom).WalkAreaMask->GetAllegroBitmap();
	else if (index == MASK_WALKBEHIND)
		return (BITMAP *)_GP(thisroom).WalkBehindMask->GetAllegroBitmap();
	else if (index == MASK_HOTSPOT)
		return (BITMAP *)_GP(thisroom).HotspotMask->GetAllegroBitmap();
	else if (index == MASK_REGIONS)
		return (BITMAP *)_GP(thisroom).RegionMask->GetAllegroBitmap();
	else
		quit("!IAGSEngine::GetRoomMask: invalid mask requested");
	return nullptr;
}
AGSViewFrame *IAGSEngine::GetViewFrame(int32 view, int32 loop, int32 frame) {
	view--;
	if ((view < 0) || (view >= _GP(game).numviews))
		quit("!IAGSEngine::GetViewFrame: invalid view");
	if ((loop < 0) || (loop >= _G(views)[view].numLoops))
		quit("!IAGSEngine::GetViewFrame: invalid loop");
	if ((frame < 0) || (frame >= _G(views)[view].loops[loop].numFrames))
		return nullptr;

	return (AGSViewFrame *)&_G(views)[view].loops[loop].frames[frame];
}

int IAGSEngine::GetRawPixelColor(int32 color) {
	// Convert the standardized colour to the local gfx mode color
	// NOTE: it is unclear whether this has to be game colour depth or display color depth.
	// there was no difference in the original engine, but there is now.
	int result;
	__my_setcolor(&result, color, _GP(game).GetColorDepth());
	return result;
}

int IAGSEngine::GetWalkbehindBaseline(int32 wa) {
	if ((wa < 1) || (wa >= MAX_WALK_BEHINDS))
		quit("!IAGSEngine::GetWalkBehindBase: invalid walk-behind area specified");
	return _G(croom)->walkbehind_base[wa];
}
Plugins::PluginMethod IAGSEngine::GetScriptFunctionAddress(const char *funcName) {
	return ccGetSymbolAddressForPlugin(funcName);
}
int IAGSEngine::GetBitmapTransparentColor(BITMAP *bmp) {
	return bitmap_mask_color(bmp);
}
// get the character scaling level at a particular point
int IAGSEngine::GetAreaScaling(int32 x, int32 y) {
	return GetScalingAt(x, y);
}
int IAGSEngine::IsGamePaused() {
	return _G(game_paused);
}
int IAGSEngine::GetSpriteWidth(int32 slot) {
	return _GP(game).SpriteInfos[slot].Width;
}
int IAGSEngine::GetSpriteHeight(int32 slot) {
	return _GP(game).SpriteInfos[slot].Height;
}
void IAGSEngine::GetTextExtent(int32 font, const char *text, int32 *width, int32 *height) {
	if ((font < 0) || (font >= _GP(game).numfonts)) {
		if (width != nullptr) width[0] = 0;
		if (height != nullptr) height[0] = 0;
		return;
	}

	if (width != nullptr)
		width[0] = wgettextwidth_compensate(text, font);
	if (height != nullptr)
		height[0] = wgettextheight(text, font);
}
void IAGSEngine::PrintDebugConsole(const char *text) {
	debug_script_log("[PLUGIN] %s", text);
}
int IAGSEngine::IsChannelPlaying(int32 channel) {
	return AGS3::IsChannelPlaying(channel);
}
void IAGSEngine::PlaySoundChannel(int32 channel, int32 soundType, int32 volume, int32 loop, const char *filename) {
	stop_and_destroy_channel(channel);
	// Not sure if it's right to let it play on *any* channel, but this is plugin so let it go...
	// we must correctly stop background voice speech if it takes over speech chan
	if (channel == SCHAN_SPEECH && _GP(play).IsNonBlockingVoiceSpeech())
		stop_voice_nonblocking();

	SOUNDCLIP *newcha = nullptr;

	if (((soundType == PSND_MP3STREAM) || (soundType == PSND_OGGSTREAM))
	        && (loop != 0))
		quit("IAGSEngine::PlaySoundChannel: streamed samples cannot loop");

	// TODO: find out how engine was supposed to decide on where to load the sound from
	AssetPath asset_name(filename, "audio");

	if (soundType == PSND_WAVE)
		newcha = my_load_wave(asset_name, volume, (loop != 0));
	else if (soundType == PSND_MP3STREAM)
		newcha = my_load_mp3(asset_name, volume);
	else if (soundType == PSND_OGGSTREAM)
		newcha = my_load_ogg(asset_name, volume);
	else if (soundType == PSND_MP3STATIC)
		newcha = my_load_static_mp3(asset_name, volume, (loop != 0));
	else if (soundType == PSND_OGGSTATIC)
		newcha = my_load_static_ogg(asset_name, volume, (loop != 0));
	else if (soundType == PSND_MIDI) {
		if (_GP(play).silent_midi != 0 || _G(current_music_type) == MUS_MIDI)
			quit("!IAGSEngine::PlaySoundChannel: MIDI already in use");
		newcha = my_load_midi(asset_name, (loop != 0));
		newcha->set_volume(volume);
	} else if (soundType == PSND_MOD) {
		newcha = my_load_mod(asset_name, (loop != 0));
		newcha->set_volume(volume);
	} else
		quit("!IAGSEngine::PlaySoundChannel: unknown sound type");

	set_clip_to_channel(channel, newcha);
}
// Engine interface 12 and above are below
void IAGSEngine::MarkRegionDirty(int32 left, int32 top, int32 right, int32 bottom) {
	invalidate_rect(left, top, right, bottom, false);
	_GP(plugins)[this->pluginId].invalidatedRegion++;
}
AGSMouseCursor *IAGSEngine::GetMouseCursor(int32 cursor) {
	if ((cursor < 0) || (cursor >= _GP(game).numcursors))
		return nullptr;

	return (AGSMouseCursor *)&_GP(game).mcurs[cursor];
}
void IAGSEngine::GetRawColorComponents(int32 coldepth, int32 color, int32 *red, int32 *green, int32 *blue, int32 *alpha) {
	if (red)
		*red = getr_depth(coldepth, color);
	if (green)
		*green = getg_depth(coldepth, color);
	if (blue)
		*blue = getb_depth(coldepth, color);
	if (alpha)
		*alpha = geta_depth(coldepth, color);
}
int IAGSEngine::MakeRawColorPixel(int32 coldepth, int32 red, int32 green, int32 blue, int32 alpha) {
	return makeacol_depth(coldepth, red, green, blue, alpha);
}
int IAGSEngine::GetFontType(int32 fontNum) {
	if ((fontNum < 0) || (fontNum >= _GP(game).numfonts))
		return FNT_INVALID;

	if (font_supports_extended_characters(fontNum))
		return FNT_TTF;

	return FNT_SCI;
}
int IAGSEngine::CreateDynamicSprite(int32 coldepth, int32 width, int32 height) {

	// TODO: why is this implemented right here, should not an existing
	// script handling implementation be called instead?

	int gotSlot = _GP(spriteset).GetFreeIndex();
	if (gotSlot <= 0)
		return 0;

	if ((width < 1) || (height < 1))
		quit("!IAGSEngine::CreateDynamicSprite: invalid width/height requested by plugin");

	// resize the sprite to the requested size
	Bitmap *newPic = BitmapHelper::CreateTransparentBitmap(width, height, coldepth);
	if (newPic == nullptr)
		return 0;

	// add it into the sprite set
	add_dynamic_sprite(gotSlot, newPic);
	return gotSlot;
}
void IAGSEngine::DeleteDynamicSprite(int32 slot) {
	free_dynamic_sprite(slot);
}
int IAGSEngine::IsSpriteAlphaBlended(int32 slot) {
	if (_GP(game).SpriteInfos[slot].Flags & SPF_ALPHACHANNEL)
		return 1;
	return 0;
}

// disable AGS's sound engine
void IAGSEngine::DisableSound() {
	shutdown_sound();
	_GP(usetup).audio_backend = 0;
}
int IAGSEngine::CanRunScriptFunctionNow() {
	if (_G(inside_script))
		return 0;
	return 1;
}
int IAGSEngine::CallGameScriptFunction(const char *name, int32 globalScript, int32 numArgs, long arg1, long arg2, long arg3) {
	if (_G(inside_script))
		return -300;

	ccInstance *toRun = GetScriptInstanceByType(globalScript ? kScInstGame : kScInstRoom);

	RuntimeScriptValue params[3];
	params[0].SetPluginArgument(arg1);
	params[1].SetPluginArgument(arg2);
	params[2].SetPluginArgument(arg3);
	int toret = RunScriptFunctionIfExists(toRun, name, numArgs, params);
	return toret;
}

void IAGSEngine::NotifySpriteUpdated(int32 slot) {
	game_sprite_updated(slot);
}

void IAGSEngine::SetSpriteAlphaBlended(int32 slot, int32 isAlphaBlended) {

	_GP(game).SpriteInfos[slot].Flags &= ~SPF_ALPHACHANNEL;

	if (isAlphaBlended)
		_GP(game).SpriteInfos[slot].Flags |= SPF_ALPHACHANNEL;
}

void IAGSEngine::QueueGameScriptFunction(const char *name, int32 globalScript, int32 numArgs, long arg1, long arg2) {
	if (!_G(inside_script)) {
		this->CallGameScriptFunction(name, globalScript, numArgs, arg1, arg2, 0);
		return;
	}

	if (numArgs < 0 || numArgs > 2)
		quit("IAGSEngine::QueueGameScriptFunction: invalid number of arguments");

	_G(curscript)->run_another(name, globalScript ? kScInstGame : kScInstRoom, numArgs,
	                           RuntimeScriptValue().SetPluginArgument(arg1), RuntimeScriptValue().SetPluginArgument(arg2));
}

int IAGSEngine::RegisterManagedObject(const void *object, IAGSScriptManagedObject *callback) {
	_GP(GlobalReturnValue).SetPluginObject(const_cast<void *>(object), (ICCDynamicObject *)callback);
	return ccRegisterManagedObject(object, (ICCDynamicObject *)callback, true);
}

void IAGSEngine::AddManagedObjectReader(const char *typeName, IAGSManagedObjectReader *reader) {
	if (_G(numPluginReaders) >= MAX_PLUGIN_OBJECT_READERS)
		quit("Plugin error: IAGSEngine::AddObjectReader: Too many object readers added");

	if ((typeName == nullptr) || (typeName[0] == 0))
		quit("Plugin error: IAGSEngine::AddObjectReader: invalid name for type");

	for (int ii = 0; ii < _G(numPluginReaders); ii++) {
		if (strcmp(_G(pluginReaders)[ii].type, typeName) == 0)
			quitprintf("Plugin error: IAGSEngine::AddObjectReader: type '%s' has been registered already", typeName);
	}

	_G(pluginReaders)[_G(numPluginReaders)].reader = reader;
	_G(pluginReaders)[_G(numPluginReaders)].type = typeName;
	_G(numPluginReaders)++;
}

void IAGSEngine::RegisterUnserializedObject(int key, const void *object, IAGSScriptManagedObject *callback) {
	_GP(GlobalReturnValue).SetPluginObject(const_cast<void *>(object), (ICCDynamicObject *)callback);
	ccRegisterUnserializedObject(key, object, (ICCDynamicObject *)callback, true);
}

int IAGSEngine::GetManagedObjectKeyByAddress(const char *address) {
	return ccGetObjectHandleFromAddress(address);
}

void *IAGSEngine::GetManagedObjectAddressByKey(int key) {
	void *object;
	ICCDynamicObject *manager;
	ScriptValueType obj_type = ccGetObjectAddressAndManagerFromHandle(key, object, manager);
	if (obj_type == kScValPluginObject) {
		_GP(GlobalReturnValue).SetPluginObject(object, manager);
	} else {
		_GP(GlobalReturnValue).SetDynamicObject(object, manager);
	}
	return object;
}

const char *IAGSEngine::CreateScriptString(const char *fromText) {
	const char *string = CreateNewScriptString(fromText);
	// Should be still standard dynamic object, because not managed by plugin
	_GP(GlobalReturnValue).SetDynamicObject(const_cast<char *>(string), &_GP(myScriptStringImpl));
	return string;
}

int IAGSEngine::IncrementManagedObjectRefCount(const char *address) {
	return ccAddObjectReference(GetManagedObjectKeyByAddress(address));
}

int IAGSEngine::DecrementManagedObjectRefCount(const char *address) {
	return ccReleaseObjectReference(GetManagedObjectKeyByAddress(address));
}

void IAGSEngine::SetMousePosition(int32 x, int32 y) {
	_GP(mouse).SetPosition(Point(x, y));
	RefreshMouse();
}

void IAGSEngine::SimulateMouseClick(int32 button) {
	PluginSimulateMouseClick(button);
}

int IAGSEngine::GetMovementPathWaypointCount(int32 pathId) {
	return _G(mls)[pathId % TURNING_AROUND].numstage;
}

int IAGSEngine::GetMovementPathLastWaypoint(int32 pathId) {
	return _G(mls)[pathId % TURNING_AROUND].onstage;
}

void IAGSEngine::GetMovementPathWaypointLocation(int32 pathId, int32 waypoint, int32 *x, int32 *y) {
	*x = (_G(mls)[pathId % TURNING_AROUND].pos[waypoint] >> 16) & 0x0000ffff;
	*y = (_G(mls)[pathId % TURNING_AROUND].pos[waypoint] & 0x0000ffff);
}

void IAGSEngine::GetMovementPathWaypointSpeed(int32 pathId, int32 waypoint, int32 *xSpeed, int32 *ySpeed) {
	*xSpeed = _G(mls)[pathId % TURNING_AROUND].xpermove[waypoint];
	*ySpeed = _G(mls)[pathId % TURNING_AROUND].ypermove[waypoint];
}

int IAGSEngine::IsRunningUnderDebugger() {
	return (_G(editor_debugging_enabled) != 0) ? 1 : 0;
}

void IAGSEngine::GetPathToFileInCompiledFolder(const char *fileName, char *buffer) {
	// TODO: this is very unsafe, deprecate and make a better API function if still necessary
	strcpy(buffer, PathFromInstallDir(fileName).GetCStr());
}

void IAGSEngine::BreakIntoDebugger() {
	_G(break_on_next_script_step) = 1;
}

IAGSFontRenderer *IAGSEngine::ReplaceFontRenderer(int fontNumber, IAGSFontRenderer *newRenderer) {
	auto *old_render = font_replace_renderer(fontNumber, newRenderer);
	GUI::MarkForFontUpdate(fontNumber);
	return old_render;
}

void IAGSEngine::GetRenderStageDesc(AGSRenderStageDesc *desc) {
	if (desc->Version >= 25) {
		_G(gfxDriver)->GetStageMatrixes((RenderMatrixes &)desc->Matrixes);
	}
}


// *********** General plugin implementation **********

void pl_stop_plugins() {
	uint a;
	ccSetDebugHook(nullptr);

	for (a = 0; a < _GP(plugins).size(); a++) {
		if (_GP(plugins)[a].available) {
			_GP(plugins)[a]._plugin->AGS_EngineShutdown();
			_GP(plugins)[a].wantHook = 0;
			if (_GP(plugins)[a].savedata) {
				free(_GP(plugins)[a].savedata);
				_GP(plugins)[a].savedata = nullptr;
			}
			if (!_GP(plugins)[a].builtin) {
				_GP(plugins)[a].library.Unload();
			}
		}
	}

	_GP(plugins).clear();
	_GP(plugins).reserve(MAXPLUGINS);
}

void pl_startup_plugins() {
	for (uint i = 0; i < _GP(plugins).size(); i++) {
		if (i == 0)
			_GP(engineExports).AGS_EngineStartup(&_GP(plugins)[0].eiface);

		if (_GP(plugins)[i].available) {
			EnginePlugin &ep = _GP(plugins)[i];
			ep._plugin->AGS_EngineStartup(&ep.eiface);
		}
	}
}

NumberPtr pl_run_plugin_hooks(int event, NumberPtr data) {
	int retval = 0;
	for (uint i = 0; i < _GP(plugins).size(); i++) {
		if (_GP(plugins)[i].wantHook & event) {
			retval = _GP(plugins)[i]._plugin->AGS_EngineOnEvent(event, data);
			if (retval)
				return retval;
		}
	}

	return 0;
}

int pl_run_plugin_debug_hooks(const char *scriptfile, int linenum) {
	int retval = 0;
	for (uint i = 0; i < _GP(plugins).size(); i++) {
		if (_GP(plugins)[i].wantHook & AGSE_SCRIPTDEBUG) {
			retval = _GP(plugins)[i]._plugin->AGS_EngineDebugHook(scriptfile, linenum, 0);
			if (retval)
				return retval;
		}
	}
	return 0;
}

void pl_run_plugin_init_gfx_hooks(const char *driverName, void *data) {
	for (uint i = 0; i < _GP(plugins).size(); i++) {
		_GP(plugins)[i]._plugin->AGS_EngineInitGfx(driverName, data);
	}
}

Engine::GameInitError pl_register_plugins(const std::vector<Shared::PluginInfo> &infos) {
	_GP(plugins).clear();
	_GP(plugins).reserve(MAXPLUGINS);

	for (size_t inf_index = 0; inf_index < infos.size(); ++inf_index) {
		const Shared::PluginInfo &info = infos[inf_index];
		String name = info.Name;
		if (name.GetLast() == '!')
			continue; // editor-only plugin, ignore it
		if (_GP(plugins).size() == MAXPLUGINS)
			return kGameInitErr_TooManyPlugins;
		// AGS Editor currently saves plugin names in game data with
		// ".dll" extension appended; we need to take care of that
		const String name_ext = ".dll";
		if (name.GetLength() <= name_ext.GetLength() || name.GetLength() > PLUGIN_FILENAME_MAX + name_ext.GetLength() ||
		        name.CompareRightNoCase(name_ext, name_ext.GetLength())) {
			return kGameInitErr_PluginNameInvalid;
		}
		// remove ".dll" from plugin's name
		name.ClipRight(name_ext.GetLength());

		_GP(plugins).resize(_GP(plugins).size() + 1);
		EnginePlugin *apl = &_GP(plugins).back();

		// Copy plugin info
		snprintf(apl->filename, sizeof(apl->filename), "%s", name.GetCStr());
		if (info.DataLen) {
			apl->savedata = (char *)malloc(info.DataLen);
			memcpy(apl->savedata, info.Data.get(), info.DataLen);
		}
		apl->savedatasize = info.DataLen;

		// Compatibility with the old SnowRain module
		if (ags_stricmp(apl->filename, "ags_SnowRain20") == 0) {
			strcpy(apl->filename, "ags_snowrain");
		}

		String expect_filename = apl->library.GetFilenameForLib(apl->filename);
		if (apl->library.Load(apl->filename)) {
			apl->_plugin = apl->library.getPlugin();
			AGS::Shared::Debug::Printf(kDbgMsg_Info, "Plugin '%s' loaded as '%s', resolving imports...", apl->filename, expect_filename.GetCStr());

		} else {
			AGS::Shared::Debug::Printf(kDbgMsg_Info, "Plugin '%s' could not be loaded (expected '%s')",
			                           apl->filename, expect_filename.GetCStr());
			_GP(plugins).pop_back();
			continue;
		}

		apl->eiface.pluginId = _GP(plugins).size() - 1;
		apl->eiface.version = PLUGIN_API_VERSION;
		apl->wantHook = 0;
		apl->available = true;
	}
	return kGameInitErr_NoError;
}

bool pl_is_plugin_loaded(const char *pl_name) {
	if (!pl_name)
		return false;

	for (uint i = 0; i < _GP(plugins).size(); ++i) {
		if (ags_stricmp(pl_name, _GP(plugins)[i].filename) == 0)
			return _GP(plugins)[i].available;
	}
	return false;
}

bool pl_any_want_hook(int event) {
	for (uint i = 0; i < _GP(plugins).size(); ++i) {
		if (_GP(plugins)[i].wantHook & event)
			return true;
	}
	return false;
}

} // namespace AGS3
