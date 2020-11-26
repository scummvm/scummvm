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

#include "ags/std/vector.h"
#include "ags/shared/core/platform.h"
#if AGS_PLATFORM_OS_WINDOWS
#include "ags/shared/platform/windows/winapi_exclusive.h"
#endif
#include "ags/shared/util/wgt2allg.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/view.h"
#include "ags/engine/ac/charactercache.h"
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/dynamicsprite.h"
#include "ags/engine/ac/gamesetup.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/global_plugin.h"
#include "ags/engine/ac/global_walkablearea.h"
#include "ags/engine/ac/keycode.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/movelist.h"
#include "ags/engine/ac/objectcache.h"
#include "ags/engine/ac/parser.h"
#include "ags/engine/ac/path_helper.h"
#include "ags/engine/ac/roomstatus.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/dynobj/cc_dynamicobject_addr_and_manager.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/util/string_compat.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/device/mousew32.h"
#include "ags/shared/gui/guidefines.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/plugin/agsplugin.h"
#include "ags/engine/plugin/plugin_engine.h"
#include "ags/engine/plugin/plugin_builtin.h"
#include "ags/engine/plugin/pluginobjectreader.h"
#include "ags/engine/script/script.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/shared/ac/spritecache.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/graphicsdriver.h"
#include "ags/engine/gfx/gfxfilter.h"
#include "ags/engine/script/runtimescriptvalue.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/ac/dynobj/scriptstring.h"
#include "ags/engine/main/graphics_mode.h"
#include "ags/engine/gfx/gfx_util.h"
#include "ags/shared/util/memory.h"
#include "ags/shared/util/filestream.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/util/library.h"

#if defined(BUILTIN_PLUGINS)
#include "ags/shared/../Plugins/AGSflashlight/agsflashlight.h"
#include "ags/shared/../Plugins/agsblend/agsblend.h"
#include "ags/shared/../Plugins/ags_snowrain/ags_snowrain.h"
#include "ags/shared/../Plugins/ags_parallax/ags_parallax.h"
#include "ags/shared/../Plugins/agspalrender/agspalrender.h"
#if AGS_PLATFORM_OS_IOS
#include "ags/shared/../Plugins/agstouch/agstouch.h"
#endif // AGS_PLATFORM_OS_IOS
#endif // BUILTIN_PLUGINS

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Shared::Memory;
using namespace AGS::Engine;

extern IGraphicsDriver *gfxDriver;
extern int mousex, mousey;
extern int displayed_room;
extern RoomStruct thisroom;
extern GameSetupStruct game;
extern RoomStatus *croom;
extern SpriteCache spriteset;
extern ViewStruct *views;
extern int game_paused;
extern GameSetup usetup;
extern int inside_script;
extern ccInstance *gameinst, *roominst;
extern CharacterCache *charcache;
extern ObjectCache objcache[MAX_ROOM_OBJECTS];
extern MoveList *mls;
extern color palette[256];
extern PluginObjectReader pluginReaders[MAX_PLUGIN_OBJECT_READERS];
extern int numPluginReaders;
extern RuntimeScriptValue GlobalReturnValue;
extern ScriptString myScriptStringImpl;

// **************** PLUGIN IMPLEMENTATION ****************




struct EnginePlugin {
	char        filename[PLUGIN_FILENAME_MAX + 1];
	AGS::Engine::Library   library;
	bool       available;
	char *savedata;
	int         savedatasize;
	int         wantHook;
	int         invalidatedRegion;
	void (*engineStartup)(IAGSEngine *) = nullptr;
	void (*engineShutdown)() = nullptr;
	int (*onEvent)(int, int) = nullptr;
	void (*initGfxHook)(const char *driverName, void *data) = nullptr;
	int (*debugHook)(const char *whichscript, int lineNumber, int reserved) = nullptr;
	IAGSEngine  eiface;
	bool        builtin;

	EnginePlugin() {
		filename[0] = 0;
		wantHook = 0;
		invalidatedRegion = 0;
		savedata = nullptr;
		savedatasize = 0;
		builtin = false;
		available = false;
		eiface.version = 0;
		eiface.pluginId = 0;
	}
};
#define MAXPLUGINS 20
EnginePlugin plugins[MAXPLUGINS];
int numPlugins = 0;
int pluginsWantingDebugHooks = 0;

std::vector<InbuiltPluginDetails> _registered_builtin_plugins;

void IAGSEngine::AbortGame(const char *reason) {
	quit((const char *)reason);
}
const char *IAGSEngine::GetEngineVersion() {
	return get_engine_version();
}
void IAGSEngine::RegisterScriptFunction(const char *name, void *addy) {
	ccAddExternalPluginFunction(name, addy);
}
const char *IAGSEngine::GetGraphicsDriverID() {
	if (gfxDriver == nullptr)
		return nullptr;

	return gfxDriver->GetDriverID();
}

BITMAP *IAGSEngine::GetScreen() {
	// TODO: we could actually return stage buffer here, will that make a difference?
	if (!gfxDriver->UsesMemoryBackBuffer())
		quit("!This plugin requires software graphics driver.");

	Bitmap *buffer = gfxDriver->GetMemoryBackBuffer();
	return buffer ? (BITMAP *)buffer->GetAllegroBitmap() : nullptr;
}

BITMAP *IAGSEngine::GetVirtualScreen() {
	Bitmap *stage = gfxDriver->GetStageBackBuffer();
	return stage ? (BITMAP *)stage->GetAllegroBitmap() : nullptr;
}

void IAGSEngine::RequestEventHook(int32 event) {
	if (event >= AGSE_TOOHIGH)
		quit("!IAGSEngine::RequestEventHook: invalid event requested");

	if (plugins[this->pluginId].onEvent == nullptr)
		quit("!IAGSEngine::RequestEventHook: no callback AGS_EngineOnEvent function exported from plugin");

	if ((event & AGSE_SCRIPTDEBUG) &&
		((plugins[this->pluginId].wantHook & AGSE_SCRIPTDEBUG) == 0)) {
		pluginsWantingDebugHooks++;
		ccSetDebugHook(scriptDebugHook);
	}

	if (event & AGSE_AUDIODECODE) {
		quit("Plugin requested AUDIODECODE, which is no longer supported");
	}


	plugins[this->pluginId].wantHook |= event;
}

void IAGSEngine::UnrequestEventHook(int32 event) {
	if (event >= AGSE_TOOHIGH)
		quit("!IAGSEngine::UnrequestEventHook: invalid event requested");

	if ((event & AGSE_SCRIPTDEBUG) &&
		(plugins[this->pluginId].wantHook & AGSE_SCRIPTDEBUG)) {
		pluginsWantingDebugHooks--;
		if (pluginsWantingDebugHooks < 1)
			ccSetDebugHook(nullptr);
	}

	plugins[this->pluginId].wantHook &= ~event;
}

int IAGSEngine::GetSavedData(char *buffer, int32 bufsize) {
	int savedatasize = plugins[this->pluginId].savedatasize;

	if (bufsize < savedatasize)
		quit("!IAGSEngine::GetSavedData: buffer too small");

	if (savedatasize > 0)
		memcpy(buffer, plugins[this->pluginId].savedata, savedatasize);

	return savedatasize;
}

void IAGSEngine::DrawText(int32 x, int32 y, int32 font, int32 color, char *text) {
	Bitmap *ds = gfxDriver->GetStageBackBuffer();
	if (!ds)
		return;
	color_t text_color = ds->GetCompatibleColor(color);
	draw_and_invalidate_text(ds, x, y, font, text_color, text);
}

void IAGSEngine::GetScreenDimensions(int32 *width, int32 *height, int32 *coldepth) {
	if (width != nullptr)
		width[0] = play.GetMainViewport().GetWidth();
	if (height != nullptr)
		height[0] = play.GetMainViewport().GetHeight();
	if (coldepth != nullptr)
		coldepth[0] = scsystem.coldepth;
}

unsigned char **IAGSEngine::GetRawBitmapSurface(BITMAP *bmp) {
	if (!is_linear_bitmap(bmp))
		quit("!IAGSEngine::GetRawBitmapSurface: invalid bitmap for access to surface");
	acquire_bitmap(bmp);

	Bitmap *stage = gfxDriver->GetStageBackBuffer();
	if (stage && bmp == stage->GetAllegroBitmap())
		plugins[this->pluginId].invalidatedRegion = 0;

	return (unsigned char **)bmp->getPixels();
}

void IAGSEngine::ReleaseBitmapSurface(BITMAP *bmp) {
	release_bitmap(bmp);

	Bitmap *stage = gfxDriver->GetStageBackBuffer();
	if (stage && bmp == stage->GetAllegroBitmap()) {
		// plugin does not manaually invalidate stuff, so
		// we must invalidate the whole screen to be safe
		if (!plugins[this->pluginId].invalidatedRegion)
			invalidate_screen();
	}
}

void IAGSEngine::GetMousePosition(int32 *x, int32 *y) {
	if (x) x[0] = mousex;
	if (y) y[0] = mousey;
}
int IAGSEngine::GetCurrentRoom() {
	return displayed_room;
}
int IAGSEngine::GetNumBackgrounds() {
	return thisroom.BgFrameCount;
}
int IAGSEngine::GetCurrentBackground() {
	return play.bg_frame;
}
BITMAP *IAGSEngine::GetBackgroundScene(int32 index) {
	return (BITMAP *)thisroom.BgFrames[index].Graphic->GetAllegroBitmap();
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

// On save/restore, the Engine will provide the plugin with a handle. Because we only ever save to one file at a time,
// we can reuse the same handle.

static long pl_file_handle = -1;
static Stream *pl_file_stream = nullptr;

void pl_set_file_handle(long data, Stream *stream) {
	pl_file_handle = data;
	pl_file_stream = stream;
}

void pl_clear_file_handle() {
	pl_file_handle = -1;
	pl_file_stream = nullptr;
}

int IAGSEngine::FRead(void *buffer, int32 len, int32 handle) {
	if (handle != pl_file_handle) {
		quitprintf("IAGSEngine::FRead: invalid file handle: %d", handle);
	}
	if (!pl_file_stream) {
		quit("IAGSEngine::FRead: file stream not set");
	}
	return pl_file_stream->Read(buffer, len);
}

int IAGSEngine::FWrite(void *buffer, int32 len, int32 handle) {
	if (handle != pl_file_handle) {
		quitprintf("IAGSEngine::FWrite: invalid file handle: %d", handle);
	}
	if (!pl_file_stream) {
		quit("IAGSEngine::FWrite: file stream not set");
	}
	return pl_file_stream->Write(buffer, len);
}

void IAGSEngine::DrawTextWrapped(int32 xx, int32 yy, int32 wid, int32 font, int32 color, const char *text) {
	// TODO: use generic function from the engine instead of having copy&pasted code here
	int linespacing = getfontspacing_outlined(font);

	if (break_up_text_into_lines(text, Lines, wid, font) == 0)
		return;

	Bitmap *ds = gfxDriver->GetStageBackBuffer();
	if (!ds)
		return;
	color_t text_color = ds->GetCompatibleColor(color);
	data_to_game_coords((int *)&xx, (int *)&yy); // stupid! quick tweak
	for (size_t i = 0; i < Lines.Count(); i++)
		draw_and_invalidate_text(ds, xx, yy + linespacing * i, font, text_color, Lines[i]);
}

Bitmap glVirtualScreenWrap;
void IAGSEngine::SetVirtualScreen(BITMAP *bmp) {
	if (!gfxDriver->UsesMemoryBackBuffer()) {
		debug_script_warn("SetVirtualScreen: this plugin requires software graphics driver to work correctly.");
		// we let it continue since gfxDriver is supposed to ignore this request without throwing an exception
	}

	if (bmp) {
		glVirtualScreenWrap.WrapAllegroBitmap(bmp, true);
		gfxDriver->SetMemoryBackBuffer(&glVirtualScreenWrap);
	} else {
		glVirtualScreenWrap.Destroy();
		gfxDriver->SetMemoryBackBuffer(nullptr);
	}
}

int IAGSEngine::LookupParserWord(const char *word) {
	return find_word_in_dictionary((const char *)word);
}

void IAGSEngine::BlitBitmap(int32 x, int32 y, BITMAP *bmp, int32 masked) {
	Bitmap *ds = gfxDriver->GetStageBackBuffer();
	if (!ds)
		return;
	wputblock_raw(ds, x, y, bmp, masked);
	invalidate_rect(x, y, x + bmp->w, y + bmp->h, false);
}

void IAGSEngine::BlitSpriteTranslucent(int32 x, int32 y, BITMAP *bmp, int32 trans) {
	Bitmap *ds = gfxDriver->GetStageBackBuffer();
	if (!ds)
		return;
	Bitmap wrap(bmp, true);
	if (gfxDriver->UsesMemoryBackBuffer())
		GfxUtil::DrawSpriteWithTransparency(ds, &wrap, x, y, trans);
	else
		GfxUtil::DrawSpriteBlend(ds, Point(x, y), &wrap, kBlendMode_Alpha, true, false, trans);
}

void IAGSEngine::BlitSpriteRotated(int32 x, int32 y, BITMAP *bmp, int32 angle) {
	Bitmap *ds = gfxDriver->GetStageBackBuffer();
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
	if (run_service_mb_controls(mbut, mwheelz) && mbut >= 0 && !play.IsIgnoringInput())
		pl_run_plugin_hooks(AGSE_MOUSECLICK, mbut);
	int kp;
	if (run_service_key_controls(kp) && !play.IsIgnoringInput()) {
		pl_run_plugin_hooks(AGSE_KEYPRESS, kp);
	}

}
AGSCharacter *IAGSEngine::GetCharacter(int32 charnum) {
	if (charnum >= game.numcharacters)
		quit("!AGSEngine::GetCharacter: invalid character request");

	return (AGSCharacter *)&game.chars[charnum];
}
AGSGameOptions *IAGSEngine::GetGameOptions() {
	return (AGSGameOptions *)&play;
}
AGSColor *IAGSEngine::GetPalette() {
	return (AGSColor *)&palette[0];
}
void IAGSEngine::SetPalette(int32 start, int32 finish, AGSColor *cpl) {
	set_palette_range((color *)cpl, start, finish, 0);
}
int IAGSEngine::GetNumCharacters() {
	return game.numcharacters;
}
int IAGSEngine::GetPlayerCharacter() {
	return game.playercharacter;
}
void IAGSEngine::RoomToViewport(int32 *x, int32 *y) {
	Point scrp = play.RoomToScreen(x ? data_to_game_coord(*x) : 0, y ? data_to_game_coord(*y) : 0);
	if (x)
		*x = scrp.X;
	if (y)
		*y = scrp.Y;
}
void IAGSEngine::ViewportToRoom(int32 *x, int32 *y) {
	// NOTE: This is an old function that did not account for custom/multiple viewports
	// and does not expect to fail, therefore we always use primary viewport here.
	// (Not sure if it's good though)
	VpPoint vpt = play.ScreenToRoom(x ? game_to_data_coord(*x) : 0, y ? game_to_data_coord(*y) : 0);
	if (x)
		*x = vpt.first.X;
	if (y)
		*y = vpt.first.Y;
}
int IAGSEngine::GetNumObjects() {
	return croom->numobj;
}
AGSObject *IAGSEngine::GetObject(int32 num) {
	if (num >= croom->numobj)
		quit("!IAGSEngine::GetObject: invalid object");

	return (AGSObject *)&croom->obj[num];
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
	return (BITMAP *)spriteset[num]->GetAllegroBitmap();
}
BITMAP *IAGSEngine::GetRoomMask(int32 index) {
	if (index == MASK_WALKABLE)
		return (BITMAP *)thisroom.WalkAreaMask->GetAllegroBitmap();
	else if (index == MASK_WALKBEHIND)
		return (BITMAP *)thisroom.WalkBehindMask->GetAllegroBitmap();
	else if (index == MASK_HOTSPOT)
		return (BITMAP *)thisroom.HotspotMask->GetAllegroBitmap();
	else if (index == MASK_REGIONS)
		return (BITMAP *)thisroom.RegionMask->GetAllegroBitmap();
	else
		quit("!IAGSEngine::GetRoomMask: invalid mask requested");
	return nullptr;
}
AGSViewFrame *IAGSEngine::GetViewFrame(int32 view, int32 loop, int32 frame) {
	view--;
	if ((view < 0) || (view >= game.numviews))
		quit("!IAGSEngine::GetViewFrame: invalid view");
	if ((loop < 0) || (loop >= views[view].numLoops))
		quit("!IAGSEngine::GetViewFrame: invalid loop");
	if ((frame < 0) || (frame >= views[view].loops[loop].numFrames))
		return nullptr;

	return (AGSViewFrame *)&views[view].loops[loop].frames[frame];
}

int IAGSEngine::GetRawPixelColor(int32 color) {
	// Convert the standardized colour to the local gfx mode color
	// NOTE: it is unclear whether this has to be game colour depth or display color depth.
	// there was no difference in the original engine, but there is now.
	int result;
	__my_setcolor(&result, color, game.GetColorDepth());
	return result;
}

int IAGSEngine::GetWalkbehindBaseline(int32 wa) {
	if ((wa < 1) || (wa >= MAX_WALK_BEHINDS))
		quit("!IAGSEngine::GetWalkBehindBase: invalid walk-behind area specified");
	return croom->walkbehind_base[wa];
}
void *IAGSEngine::GetScriptFunctionAddress(const char *funcName) {
	return ccGetSymbolAddressForPlugin((const char *)funcName);
}
int IAGSEngine::GetBitmapTransparentColor(BITMAP *bmp) {
	return bitmap_mask_color(bmp);
}
// get the character scaling level at a particular point
int IAGSEngine::GetAreaScaling(int32 x, int32 y) {
	return GetScalingAt(x, y);
}
int IAGSEngine::IsGamePaused() {
	return game_paused;
}
int IAGSEngine::GetSpriteWidth(int32 slot) {
	return game.SpriteInfos[slot].Width;
}
int IAGSEngine::GetSpriteHeight(int32 slot) {
	return game.SpriteInfos[slot].Height;
}
void IAGSEngine::GetTextExtent(int32 font, const char *text, int32 *width, int32 *height) {
	if ((font < 0) || (font >= game.numfonts)) {
		if (width != nullptr) width[0] = 0;
		if (height != nullptr) height[0] = 0;
		return;
	}

	if (width != nullptr)
		width[0] = wgettextwidth_compensate(text, font);
	if (height != nullptr)
		height[0] = wgettextheight((const char *)text, font);
}
void IAGSEngine::PrintDebugConsole(const char *text) {
	debug_script_log("[PLUGIN] %s", text);
	platform->WriteStdOut("[PLUGIN] %s", text);
}
int IAGSEngine::IsChannelPlaying(int32 channel) {
	return AGS3::IsChannelPlaying(channel);
}
void IAGSEngine::PlaySoundChannel(int32 channel, int32 soundType, int32 volume, int32 loop, const char *filename) {
	stop_and_destroy_channel(channel);
	// Not sure if it's right to let it play on *any* channel, but this is plugin so let it go...
	// we must correctly stop background voice speech if it takes over speech chan
	if (channel == SCHAN_SPEECH && play.IsNonBlockingVoiceSpeech())
		stop_voice_nonblocking();

	SOUNDCLIP *newcha = nullptr;

	if (((soundType == PSND_MP3STREAM) || (soundType == PSND_OGGSTREAM))
		&& (loop != 0))
		quit("IAGSEngine::PlaySoundChannel: streamed samples cannot loop");

	// TODO: find out how engine was supposed to decide on where to load the sound from
	AssetPath asset_name("", filename);

	if (soundType == PSND_WAVE)
		newcha = my_load_wave(asset_name, volume, loop);
	else if (soundType == PSND_MP3STREAM)
		newcha = my_load_mp3(asset_name, volume);
	else if (soundType == PSND_OGGSTREAM)
		newcha = my_load_ogg(asset_name, volume);
	else if (soundType == PSND_MP3STATIC)
		newcha = my_load_static_mp3(asset_name, volume, (loop != 0));
	else if (soundType == PSND_OGGSTATIC)
		newcha = my_load_static_ogg(asset_name, volume, (loop != 0));
	else if (soundType == PSND_MIDI) {
		if (midi_pos >= 0)
			quit("!IAGSEngine::PlaySoundChannel: MIDI already in use");
		newcha = my_load_midi(asset_name, loop);
		newcha->set_volume(volume);
	}
#ifndef PSP_NO_MOD_PLAYBACK
	else if (soundType == PSND_MOD) {
		newcha = my_load_mod(asset_name, loop);
		newcha->set_volume(volume);
	}
#endif
	else
		quit("!IAGSEngine::PlaySoundChannel: unknown sound type");

	set_clip_to_channel(channel, newcha);
}
// Engine interface 12 and above are below
void IAGSEngine::MarkRegionDirty(int32 left, int32 top, int32 right, int32 bottom) {
	invalidate_rect(left, top, right, bottom, false);
	plugins[this->pluginId].invalidatedRegion++;
}
AGSMouseCursor *IAGSEngine::GetMouseCursor(int32 cursor) {
	if ((cursor < 0) || (cursor >= game.numcursors))
		return nullptr;

	return (AGSMouseCursor *)&game.mcurs[cursor];
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
	if ((fontNum < 0) || (fontNum >= game.numfonts))
		return FNT_INVALID;

	if (font_supports_extended_characters(fontNum))
		return FNT_TTF;

	return FNT_SCI;
}
int IAGSEngine::CreateDynamicSprite(int32 coldepth, int32 width, int32 height) {

	// TODO: why is this implemented right here, should not an existing
	// script handling implementation be called instead?

	int gotSlot = spriteset.GetFreeIndex();
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
	if (game.SpriteInfos[slot].Flags & SPF_ALPHACHANNEL)
		return 1;
	return 0;
}

// disable AGS's sound engine
void IAGSEngine::DisableSound() {
	shutdown_sound();
	usetup.digicard = DIGI_NONE;
	usetup.midicard = MIDI_NONE;
	reserve_voices(0, 0);
	install_sound(DIGI_NONE, MIDI_NONE, nullptr);
}
int IAGSEngine::CanRunScriptFunctionNow() {
	if (inside_script)
		return 0;
	return 1;
}
int IAGSEngine::CallGameScriptFunction(const char *name, int32 globalScript, int32 numArgs, long arg1, long arg2, long arg3) {
	if (inside_script)
		return -300;

	ccInstance *toRun = GetScriptInstanceByType(globalScript ? kScInstGame : kScInstRoom);

	RuntimeScriptValue params[3];
	params[0].SetPluginArgument(arg1);
	params[1].SetPluginArgument(arg2);
	params[2].SetPluginArgument(arg3);
	int toret = RunScriptFunctionIfExists(toRun, (const char *)name, numArgs, params);
	return toret;
}

void IAGSEngine::NotifySpriteUpdated(int32 slot) {
	int ff;
	// wipe the character cache when we change rooms
	for (ff = 0; ff < game.numcharacters; ff++) {
		if ((charcache[ff].inUse) && (charcache[ff].sppic == slot)) {
			delete charcache[ff].image;
			charcache[ff].image = nullptr;
			charcache[ff].inUse = 0;
		}
	}

	// clear the object cache
	for (ff = 0; ff < MAX_ROOM_OBJECTS; ff++) {
		if ((objcache[ff].image != nullptr) && (objcache[ff].sppic == slot)) {
			delete objcache[ff].image;
			objcache[ff].image = nullptr;
		}
	}
}

void IAGSEngine::SetSpriteAlphaBlended(int32 slot, int32 isAlphaBlended) {

	game.SpriteInfos[slot].Flags &= ~SPF_ALPHACHANNEL;

	if (isAlphaBlended)
		game.SpriteInfos[slot].Flags |= SPF_ALPHACHANNEL;
}

void IAGSEngine::QueueGameScriptFunction(const char *name, int32 globalScript, int32 numArgs, long arg1, long arg2) {
	if (!inside_script) {
		this->CallGameScriptFunction(name, globalScript, numArgs, arg1, arg2, 0);
		return;
	}

	if (numArgs < 0 || numArgs > 2)
		quit("IAGSEngine::QueueGameScriptFunction: invalid number of arguments");

	curscript->run_another(name, globalScript ? kScInstGame : kScInstRoom, numArgs,
		RuntimeScriptValue().SetPluginArgument(arg1), RuntimeScriptValue().SetPluginArgument(arg2));
}

int IAGSEngine::RegisterManagedObject(const void *object, IAGSScriptManagedObject *callback) {
	GlobalReturnValue.SetPluginObject((void *)object, (ICCDynamicObject *)callback);
	return ccRegisterManagedObject(object, (ICCDynamicObject *)callback, true);
}

void IAGSEngine::AddManagedObjectReader(const char *typeName, IAGSManagedObjectReader *reader) {
	if (numPluginReaders >= MAX_PLUGIN_OBJECT_READERS)
		quit("Plugin error: IAGSEngine::AddObjectReader: Too many object readers added");

	if ((typeName == nullptr) || (typeName[0] == 0))
		quit("Plugin error: IAGSEngine::AddObjectReader: invalid name for type");

	for (int ii = 0; ii < numPluginReaders; ii++) {
		if (strcmp(pluginReaders[ii].type, typeName) == 0)
			quitprintf("Plugin error: IAGSEngine::AddObjectReader: type '%s' has been registered already", typeName);
	}

	pluginReaders[numPluginReaders].reader = reader;
	pluginReaders[numPluginReaders].type = typeName;
	numPluginReaders++;
}

void IAGSEngine::RegisterUnserializedObject(int key_, const void *object, IAGSScriptManagedObject *callback) {
	GlobalReturnValue.SetPluginObject((void *)object, (ICCDynamicObject *)callback);
	ccRegisterUnserializedObject(key_, object, (ICCDynamicObject *)callback, true);
}

int IAGSEngine::GetManagedObjectKeyByAddress(const char *address) {
	return ccGetObjectHandleFromAddress(address);
}

void *IAGSEngine::GetManagedObjectAddressByKey(int key_) {
	void *object;
	ICCDynamicObject *manager;
	ScriptValueType obj_type = ccGetObjectAddressAndManagerFromHandle(key_, object, manager);
	if (obj_type == kScValPluginObject) {
		GlobalReturnValue.SetPluginObject(object, manager);
	} else {
		GlobalReturnValue.SetDynamicObject(object, manager);
	}
	return object;
}

const char *IAGSEngine::CreateScriptString(const char *fromText) {
	const char *string = CreateNewScriptString(fromText);
	// Should be still standard dynamic object, because not managed by plugin
	GlobalReturnValue.SetDynamicObject((void *)string, &myScriptStringImpl);
	return string;
}

int IAGSEngine::IncrementManagedObjectRefCount(const char *address) {
	return ccAddObjectReference(GetManagedObjectKeyByAddress(address));
}

int IAGSEngine::DecrementManagedObjectRefCount(const char *address) {
	return ccReleaseObjectReference(GetManagedObjectKeyByAddress(address));
}

void IAGSEngine::SetMousePosition(int32 x, int32 y) {
	Mouse::SetPosition(Point(x, y));
	RefreshMouse();
}

void IAGSEngine::SimulateMouseClick(int32 button) {
	PluginSimulateMouseClick(button);
}

int IAGSEngine::GetMovementPathWaypointCount(int32 pathId) {
	return mls[pathId % TURNING_AROUND].numstage;
}

int IAGSEngine::GetMovementPathLastWaypoint(int32 pathId) {
	return mls[pathId % TURNING_AROUND].onstage;
}

void IAGSEngine::GetMovementPathWaypointLocation(int32 pathId, int32 waypoint, int32 *x, int32 *y) {
	*x = (mls[pathId % TURNING_AROUND].pos[waypoint] >> 16) & 0x0000ffff;
	*y = (mls[pathId % TURNING_AROUND].pos[waypoint] & 0x0000ffff);
}

void IAGSEngine::GetMovementPathWaypointSpeed(int32 pathId, int32 waypoint, int32 *xSpeed, int32 *ySpeed) {
	*xSpeed = mls[pathId % TURNING_AROUND].xpermove[waypoint];
	*ySpeed = mls[pathId % TURNING_AROUND].ypermove[waypoint];
}

int IAGSEngine::IsRunningUnderDebugger() {
	return (editor_debugging_enabled != 0) ? 1 : 0;
}

void IAGSEngine::GetPathToFileInCompiledFolder(const char *fileName, char *buffer) {
	get_install_dir_path(buffer, fileName);
}

void IAGSEngine::BreakIntoDebugger() {
	break_on_next_script_step = 1;
}

IAGSFontRenderer *IAGSEngine::ReplaceFontRenderer(int fontNumber, IAGSFontRenderer *newRenderer) {
	return font_replace_renderer(fontNumber, newRenderer);
}


// *********** General plugin implementation **********

void pl_stop_plugins() {
	int a;
	ccSetDebugHook(nullptr);

	for (a = 0; a < numPlugins; a++) {
		if (plugins[a].available) {
			if (plugins[a].engineShutdown != nullptr)
				plugins[a].engineShutdown();
			plugins[a].wantHook = 0;
			if (plugins[a].savedata) {
				free(plugins[a].savedata);
				plugins[a].savedata = nullptr;
			}
			if (!plugins[a].builtin) {
				plugins[a].library.Unload();
			}
		}
	}
	numPlugins = 0;
}

void pl_startup_plugins() {
	int i;
	for (i = 0; i < numPlugins; i++) {
		if (plugins[i].available)
			plugins[i].engineStartup(&plugins[i].eiface);
	}
}

int pl_run_plugin_hooks(int event, int data) {
	int i, retval = 0;
	for (i = 0; i < numPlugins; i++) {
		if (plugins[i].wantHook & event) {
			retval = plugins[i].onEvent(event, data);
			if (retval)
				return retval;
		}
	}
	return 0;
}

int pl_run_plugin_debug_hooks(const char *scriptfile, int linenum) {
	int i, retval = 0;
	for (i = 0; i < numPlugins; i++) {
		if (plugins[i].wantHook & AGSE_SCRIPTDEBUG) {
			retval = plugins[i].debugHook(scriptfile, linenum, 0);
			if (retval)
				return retval;
		}
	}
	return 0;
}

void pl_run_plugin_init_gfx_hooks(const char *driverName, void *data) {
	for (int i = 0; i < numPlugins; i++) {
		if (plugins[i].initGfxHook != nullptr) {
			plugins[i].initGfxHook(driverName, data);
		}
	}
}

int pl_register_builtin_plugin(InbuiltPluginDetails const &details) {
	_registered_builtin_plugins.push_back(details);
	return 0;
}

bool pl_use_builtin_plugin(EnginePlugin *apl) {
#if defined(BUILTIN_PLUGINS)
	if (ags_stricmp(apl->filename, "agsflashlight") == 0) {
		apl->engineStartup = agsflashlight::AGS_EngineStartup;
		apl->engineShutdown = agsflashlight::AGS_EngineShutdown;
		apl->onEvent = agsflashlight::AGS_EngineOnEvent;
		apl->debugHook = agsflashlight::AGS_EngineDebugHook;
		apl->initGfxHook = agsflashlight::AGS_EngineInitGfx;
		apl->available = true;
		apl->builtin = true;
		return true;
	} else if (ags_stricmp(apl->filename, "agsblend") == 0) {
		apl->engineStartup = agsblend::AGS_EngineStartup;
		apl->engineShutdown = agsblend::AGS_EngineShutdown;
		apl->onEvent = agsblend::AGS_EngineOnEvent;
		apl->debugHook = agsblend::AGS_EngineDebugHook;
		apl->initGfxHook = agsblend::AGS_EngineInitGfx;
		apl->available = true;
		apl->builtin = true;
		return true;
	} else if (ags_stricmp(apl->filename, "ags_snowrain") == 0) {
		apl->engineStartup = ags_snowrain::AGS_EngineStartup;
		apl->engineShutdown = ags_snowrain::AGS_EngineShutdown;
		apl->onEvent = ags_snowrain::AGS_EngineOnEvent;
		apl->debugHook = ags_snowrain::AGS_EngineDebugHook;
		apl->initGfxHook = ags_snowrain::AGS_EngineInitGfx;
		apl->available = true;
		apl->builtin = true;
		return true;
	} else if (ags_stricmp(apl->filename, "ags_parallax") == 0) {
		apl->engineStartup = ags_parallax::AGS_EngineStartup;
		apl->engineShutdown = ags_parallax::AGS_EngineShutdown;
		apl->onEvent = ags_parallax::AGS_EngineOnEvent;
		apl->debugHook = ags_parallax::AGS_EngineDebugHook;
		apl->initGfxHook = ags_parallax::AGS_EngineInitGfx;
		apl->available = true;
		apl->builtin = true;
		return true;
	} else if (ags_stricmp(apl->filename, "agspalrender") == 0) {
		apl->engineStartup = agspalrender::AGS_EngineStartup;
		apl->engineShutdown = agspalrender::AGS_EngineShutdown;
		apl->onEvent = agspalrender::AGS_EngineOnEvent;
		apl->debugHook = agspalrender::AGS_EngineDebugHook;
		apl->initGfxHook = agspalrender::AGS_EngineInitGfx;
		apl->available = true;
		apl->builtin = true;
		return true;
	}
#if AGS_PLATFORM_OS_IOS
	else if (ags_stricmp(apl->filename, "agstouch") == 0) {
		apl->engineStartup = agstouch::AGS_EngineStartup;
		apl->engineShutdown = agstouch::AGS_EngineShutdown;
		apl->onEvent = agstouch::AGS_EngineOnEvent;
		apl->debugHook = agstouch::AGS_EngineDebugHook;
		apl->initGfxHook = agstouch::AGS_EngineInitGfx;
		apl->available = true;
		apl->builtin = true;
		return true;
	}
#endif // IOS_VERSION
#endif // BUILTIN_PLUGINS

	for (std::vector<InbuiltPluginDetails>::iterator it = _registered_builtin_plugins.begin(); it != _registered_builtin_plugins.end(); ++it) {
		if (ags_stricmp(apl->filename, it->filename) == 0) {
			apl->engineStartup = it->engineStartup;
			apl->engineShutdown = it->engineShutdown;
			apl->onEvent = it->onEvent;
			apl->debugHook = it->debugHook;
			apl->initGfxHook = it->initGfxHook;
			apl->available = true;
			apl->builtin = true;
			return true;
		}
	}
	return false;
}

Engine::GameInitError pl_register_plugins(const std::vector<Shared::PluginInfo> &infos) {
	numPlugins = 0;
	for (size_t inf_index = 0; inf_index < infos.size(); ++inf_index) {
		const Shared::PluginInfo &info = infos[inf_index];
		String name = info.Name;
		if (name.GetLast() == '!')
			continue; // editor-only plugin, ignore it
		if (numPlugins == MAXPLUGINS)
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

		EnginePlugin *apl = &plugins[numPlugins++];
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
			AGS::Shared::Debug::Printf(kDbgMsg_Info, "Plugin '%s' loaded as '%s', resolving imports...", apl->filename, expect_filename.GetCStr());

			if (apl->library.GetFunctionAddress("AGS_PluginV2") == nullptr) {
				quitprintf("Plugin '%s' is an old incompatible version.", apl->filename);
			}
			apl->engineStartup = (void(*)(IAGSEngine *))apl->library.GetFunctionAddress("AGS_EngineStartup");
			apl->engineShutdown = (void(*)())apl->library.GetFunctionAddress("AGS_EngineShutdown");

			if (apl->engineStartup == nullptr) {
				quitprintf("Plugin '%s' is not a valid AGS plugin (no engine startup entry point)", apl->filename);
			}
			apl->onEvent = (int(*)(int, int))apl->library.GetFunctionAddress("AGS_EngineOnEvent");
			apl->debugHook = (int(*)(const char *, int, int))apl->library.GetFunctionAddress("AGS_EngineDebugHook");
			apl->initGfxHook = (void(*)(const char *, void *))apl->library.GetFunctionAddress("AGS_EngineInitGfx");
		} else {
			AGS::Shared::Debug::Printf(kDbgMsg_Info, "Plugin '%s' could not be loaded (expected '%s'), trying built-in plugins...",
				apl->filename, expect_filename.GetCStr());
			if (pl_use_builtin_plugin(apl)) {
				AGS::Shared::Debug::Printf(kDbgMsg_Info, "Build-in plugin '%s' found and being used.", apl->filename);
			} else {
				// Plugin loading has failed at this point, try using built-in plugin function stubs
				if (RegisterPluginStubs((const char *)apl->filename))
					AGS::Shared::Debug::Printf(kDbgMsg_Info, "Placeholder functions for the plugin '%s' found.", apl->filename);
				else
					AGS::Shared::Debug::Printf(kDbgMsg_Info, "No placeholder functions for the plugin '%s' found. The game might fail to load!", apl->filename);
				continue;
			}
		}

		apl->eiface.pluginId = numPlugins - 1;
		apl->eiface.version = 24;
		apl->wantHook = 0;
		apl->available = true;
	}
	return kGameInitErr_NoError;
}

bool pl_is_plugin_loaded(const char *pl_name) {
	if (!pl_name)
		return false;

	for (int i = 0; i < numPlugins; ++i) {
		if (ags_stricmp(pl_name, plugins[i].filename) == 0)
			return plugins[i].available;
	}
	return false;
}

bool pl_any_want_hook(int event) {
	for (int i = 0; i < numPlugins; ++i) {
		if (plugins[i].wantHook & event)
			return true;
	}
	return false;
}

} // namespace AGS3
