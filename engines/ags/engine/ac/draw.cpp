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

#include "ags/lib/std/algorithm.h"
#include "ags/lib/std/math.h"
#include "ags/lib/aastr-0.1.1/aastr.h"
#include "ags/shared/core/platform.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/util/compress.h"
#include "ags/shared/ac/view.h"
#include "ags/engine/ac/charactercache.h"
#include "ags/engine/ac/characterextras.h"
#include "ags/shared/ac/characterinfo.h"
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/draw_software.h"
#include "ags/engine/ac/gamesetup.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_gui.h"
#include "ags/engine/ac/global_region.h"
#include "ags/engine/ac/gui.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/objectcache.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/roomobject.h"
#include "ags/engine/ac/roomstatus.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/screenoverlay.h"
#include "ags/engine/ac/sprite.h"
#include "ags/engine/ac/spritelistentry.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/ac/viewframe.h"
#include "ags/engine/ac/walkablearea.h"
#include "ags/engine/ac/walkbehind.h"
#include "ags/engine/ac/dynobj/scriptsystem.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/gui/guimain.h"
#include "ags/engine/platform/base/agsplatformdriver.h"
#include "ags/engine/plugin/agsplugin.h"
#include "ags/engine/plugin/plugin_engine.h"
#include "ags/shared/ac/spritecache.h"
#include "ags/engine/gfx/gfx_util.h"
#include "ags/engine/gfx/graphicsdriver.h"
#include "ags/engine/gfx/ali3dexception.h"
#include "ags/engine/gfx/blender.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/ac/game.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

#if AGS_PLATFORM_OS_ANDROID
//include <sys/stat.h>
//include <android/log.h>

extern "C" void android_render();
#endif

#if AGS_PLATFORM_OS_IOS
extern "C" void ios_render();
#endif

extern GameSetup usetup;
extern GameSetupStruct game;
extern GameState play;
extern int convert_16bit_bgr;
extern ScriptSystem scsystem;
extern AGSPlatformDriver *platform;
extern RoomStruct thisroom;
extern char noWalkBehindsAtAll;
extern unsigned int loopcounter;
extern char *walkBehindExists;  // whether a WB area is in this column
extern int *walkBehindStartY, *walkBehindEndY;
extern int walkBehindLeft[MAX_WALK_BEHINDS], walkBehindTop[MAX_WALK_BEHINDS];
extern int walkBehindRight[MAX_WALK_BEHINDS], walkBehindBottom[MAX_WALK_BEHINDS];
extern IDriverDependantBitmap *walkBehindBitmap[MAX_WALK_BEHINDS];
extern int walkBehindsCachedForBgNum;
extern WalkBehindMethodEnum walkBehindMethod;
extern int walk_behind_baselines_changed;
extern SpriteCache spriteset;
extern RoomStatus *croom;
extern int our_eip;
extern int in_new_room;
extern RoomObject *objs;
extern ViewStruct *views;
extern CharacterCache *charcache;
extern ObjectCache objcache[MAX_ROOM_OBJECTS];
extern int displayed_room;
extern CharacterExtras *charextra;
extern CharacterInfo *playerchar;
extern int eip_guinum;
extern int is_complete_overlay;
extern int cur_mode, cur_cursor;
extern int mouse_frame, mouse_delay;
extern int lastmx, lastmy;
extern IDriverDependantBitmap *mouseCursor;
extern int hotx, hoty;
extern int bg_just_changed;

color palette[256];

COLOR_MAP maincoltable;

IGraphicsDriver *gfxDriver;
IDriverDependantBitmap *blankImage = nullptr;
IDriverDependantBitmap *blankSidebarImage = nullptr;
IDriverDependantBitmap *debugConsole = nullptr;

// actsps is used for temporary storage of the bitamp image
// of the latest version of the sprite
int actSpsCount = 0;
Bitmap **actsps;
IDriverDependantBitmap * *actspsbmp;
// temporary cache of walk-behind for this actsps image
Bitmap **actspswb;
IDriverDependantBitmap * *actspswbbmp;
CachedActSpsData *actspswbcache;

bool current_background_is_dirty = false;

// Room background sprite
IDriverDependantBitmap *roomBackgroundBmp = nullptr;
// Buffer and info flags for viewport/camera pairs rendering in software mode
struct RoomCameraDrawData {
	// Intermediate bitmap for the software drawing method.
	// We use this bitmap in case room camera has scaling enabled, we draw dirty room rects on it,
	// and then pass to software renderer which draws sprite on top and then either blits or stretch-blits
	// to the virtual screen.
	// For more details see comment in ALSoftwareGraphicsDriver::RenderToBackBuffer().
	PBitmap Buffer;      // this is the actual bitmap
	PBitmap Frame;       // this is either same bitmap reference or sub-bitmap of virtual screen
	bool    IsOffscreen; // whether room viewport was offscreen (cannot use sub-bitmap)
	bool    IsOverlap;   // whether room viewport overlaps any others (marking dirty rects is complicated)
};
std::vector<RoomCameraDrawData> CameraDrawData;

std::vector<SpriteListEntry> sprlist;
std::vector<SpriteListEntry> thingsToDrawList;

Bitmap **guibg = nullptr;
IDriverDependantBitmap **guibgbmp = nullptr;


Bitmap *debugConsoleBuffer = nullptr;

// whether there are currently remnants of a DisplaySpeech
bool screen_is_dirty = false;

Bitmap *raw_saved_screen = nullptr;
Bitmap *dynamicallyCreatedSurfaces[MAX_DYNAMIC_SURFACES];


SpriteListEntry::SpriteListEntry()
	: bmp(nullptr)
	, pic(nullptr)
	, baseline(0), x(0), y(0)
	, transparent(0)
	, takesPriorityIfEqual(false), hasAlphaChannel(false) {
}

void setpal() {
	set_palette_range(palette, 0, 255, 0);
}

int _places_r = 3, _places_g = 2, _places_b = 3;

// convert RGB to BGR for strange graphics cards
Bitmap *convert_16_to_16bgr(Bitmap *tempbl) {

	int x, y;
	unsigned short c, r, ds, b;

	for (y = 0; y < tempbl->GetHeight(); y++) {
		unsigned short *p16 = (unsigned short *)tempbl->GetScanLine(y);

		for (x = 0; x < tempbl->GetWidth(); x++) {
			c = p16[x];
			if (c != MASK_COLOR_16) {
				b = _rgb_scale_5[c & 0x1F];
				ds = _rgb_scale_6[(c >> 5) & 0x3F];
				r = _rgb_scale_5[(c >> 11) & 0x1F];
				// allegro assumes 5-6-5 for 16-bit
				p16[x] = (((r >> _places_r) << _rgb_r_shift_16) |
				          ((ds >> _places_g) << _rgb_g_shift_16) |
				          ((b >> _places_b) << _rgb_b_shift_16));

			}
		}
	}

	return tempbl;
}

// PSP: convert 32 bit RGB to BGR.
Bitmap *convert_32_to_32bgr(Bitmap *tempbl) {

	int i = 0;
	int j = 0;
	unsigned char *current;
	while (i < tempbl->GetHeight()) {
		current = tempbl->GetScanLineForWriting(i);
		while (j < tempbl->GetWidth()) {
			current[0] ^= current[2];
			current[2] ^= current[0];
			current[0] ^= current[2];
			current += 4;
			j++;
		}
		i++;
		j = 0;
	}

	return tempbl;
}

// NOTE: Some of these conversions are required  even when using
// D3D and OpenGL rendering, for two reasons:
// 1) certain raw drawing operations are still performed by software
// Allegro methods, hence bitmaps should be kept compatible to any native
// software operations, such as blitting two bitmaps of different formats.
// 2) mobile ports feature an OpenGL renderer built in Allegro library,
// that assumes native bitmaps are in OpenGL-compatible format, so that it
// could copy them to texture without additional changes.
// AGS own OpenGL renderer tries to sync its behavior with the former one.
//
// TODO: make gfxDriver->GetCompatibleBitmapFormat describe all necessary
// conversions, so that we did not have to guess.
//
Bitmap *AdjustBitmapForUseWithDisplayMode(Bitmap *bitmap, bool has_alpha) {
	const int bmp_col_depth = bitmap->GetColorDepth();
	//const int sys_col_depth = System_GetColorDepth();
	const int game_col_depth = game.GetColorDepth();
	Bitmap *new_bitmap = bitmap;

	//
	// The only special case when bitmap needs to be prepared for graphics driver
	//
	// In 32-bit display mode, 32-bit bitmaps may require component conversion
	// to match graphics driver expectation about pixel format.
	// TODO: make GetCompatibleBitmapFormat tell this somehow
#if defined (AGS_INVERTED_COLOR_ORDER)
	if (sys_col_depth > 16 && bmp_col_depth == 32) {
		// Convert RGB to BGR.
		new_bitmap = convert_32_to_32bgr(bitmap);
	}
#endif

	//
	// The rest is about bringing bitmaps to the native game's format
	// (has no dependency on display mode).
	//
	// In 32-bit game 32-bit bitmaps should have transparent pixels marked
	// (this adjustment is probably needed for DrawingSurface ops)
	if (game_col_depth == 32 && bmp_col_depth == 32) {
		if (has_alpha)
			set_rgb_mask_using_alpha_channel(new_bitmap);
	}
	// In 32-bit game hicolor bitmaps must be converted to the true color
	else if (game_col_depth == 32 && (bmp_col_depth > 8 && bmp_col_depth <= 16)) {
		new_bitmap = BitmapHelper::CreateBitmapCopy(bitmap, game_col_depth);
	}
	// In non-32-bit game truecolor bitmaps must be downgraded
	else if (game_col_depth <= 16 && bmp_col_depth > 16) {
		if (has_alpha) // if has valid alpha channel, convert it to regular transparency mask
			new_bitmap = remove_alpha_channel(bitmap);
		else // else simply convert bitmap
			new_bitmap = BitmapHelper::CreateBitmapCopy(bitmap, game_col_depth);
	}
	// Special case when we must convert 16-bit RGB to BGR
	else if (convert_16bit_bgr == 1 && bmp_col_depth == 16) {
		new_bitmap = convert_16_to_16bgr(bitmap);
	}
	return new_bitmap;
}

Bitmap *ReplaceBitmapWithSupportedFormat(Bitmap *bitmap) {
	Bitmap *new_bitmap = GfxUtil::ConvertBitmap(bitmap, gfxDriver->GetCompatibleBitmapFormat(bitmap->GetColorDepth()));
	if (new_bitmap != bitmap)
		delete bitmap;
	return new_bitmap;
}

Bitmap *PrepareSpriteForUse(Bitmap *bitmap, bool has_alpha) {
	bool must_switch_palette = bitmap->GetColorDepth() == 8 && game.GetColorDepth() > 8;
	if (must_switch_palette)
		select_palette(palette);

	Bitmap *new_bitmap = AdjustBitmapForUseWithDisplayMode(bitmap, has_alpha);
	if (new_bitmap != bitmap)
		delete bitmap;
	new_bitmap = ReplaceBitmapWithSupportedFormat(new_bitmap);

	if (must_switch_palette)
		unselect_palette();
	return new_bitmap;
}

PBitmap PrepareSpriteForUse(PBitmap bitmap, bool has_alpha) {
	bool must_switch_palette = bitmap->GetColorDepth() == 8 && System_GetColorDepth() > 8;
	if (must_switch_palette)
		select_palette(palette);

	Bitmap *new_bitmap = AdjustBitmapForUseWithDisplayMode(bitmap.get(), has_alpha);
	new_bitmap = ReplaceBitmapWithSupportedFormat(new_bitmap);

	if (must_switch_palette)
		unselect_palette();
	return new_bitmap == bitmap.get() ? bitmap : PBitmap(new_bitmap); // if bitmap is same, don't create new smart ptr!
}

Bitmap *CopyScreenIntoBitmap(int width, int height, bool at_native_res) {
	Bitmap *dst = new Bitmap(width, height, game.GetColorDepth());
	GraphicResolution want_fmt;
	// If the size and color depth are supported we may copy right into our bitmap
	if (gfxDriver->GetCopyOfScreenIntoBitmap(dst, at_native_res, &want_fmt))
		return dst;
	// Otherwise we might need to copy between few bitmaps...
	Bitmap *buf_screenfmt = new Bitmap(want_fmt.Width, want_fmt.Height, want_fmt.ColorDepth);
	gfxDriver->GetCopyOfScreenIntoBitmap(buf_screenfmt, at_native_res);
	// If at least size matches then we may blit
	if (dst->GetSize() == buf_screenfmt->GetSize()) {
		dst->Blit(buf_screenfmt);
	}
	// Otherwise we need to go through another bitmap of the matching format
	else {
		Bitmap *buf_dstfmt = new Bitmap(buf_screenfmt->GetWidth(), buf_screenfmt->GetHeight(), dst->GetColorDepth());
		buf_dstfmt->Blit(buf_screenfmt);
		dst->StretchBlt(buf_dstfmt, RectWH(dst->GetSize()));
		delete buf_dstfmt;
	}
	delete buf_screenfmt;
	return dst;
}


// Begin resolution system functions

// Multiplies up the number of pixels depending on the current
// resolution, to give a relatively fixed size at any game res
AGS_INLINE int get_fixed_pixel_size(int pixels) {
	return pixels * game.GetRelativeUIMult();
}

AGS_INLINE int data_to_game_coord(int coord) {
	return coord * game.GetDataUpscaleMult();
}

AGS_INLINE void data_to_game_coords(int *x, int *y) {
	const int mul = game.GetDataUpscaleMult();
	x[0] *= mul;
	y[0] *= mul;
}

AGS_INLINE void data_to_game_round_up(int *x, int *y) {
	const int mul = game.GetDataUpscaleMult();
	x[0] = x[0] * mul + (mul - 1);
	y[0] = y[0] * mul + (mul - 1);
}

AGS_INLINE int game_to_data_coord(int coord) {
	return coord / game.GetDataUpscaleMult();
}

AGS_INLINE void game_to_data_coords(int &x, int &y) {
	const int mul = game.GetDataUpscaleMult();
	x /= mul;
	y /= mul;
}

AGS_INLINE int game_to_data_round_up(int coord) {
	const int mul = game.GetDataUpscaleMult();
	return (coord / mul) + (mul - 1);
}

AGS_INLINE void ctx_data_to_game_coord(int &x, int &y, bool hires_ctx) {
	if (hires_ctx && !game.IsLegacyHiRes()) {
		x /= HIRES_COORD_MULTIPLIER;
		y /= HIRES_COORD_MULTIPLIER;
	} else if (!hires_ctx && game.IsLegacyHiRes()) {
		x *= HIRES_COORD_MULTIPLIER;
		y *= HIRES_COORD_MULTIPLIER;
	}
}

AGS_INLINE void ctx_data_to_game_size(int &w, int &h, bool hires_ctx) {
	if (hires_ctx && !game.IsLegacyHiRes()) {
		w = Math::Max(1, (w / HIRES_COORD_MULTIPLIER));
		h = Math::Max(1, (h / HIRES_COORD_MULTIPLIER));
	} else if (!hires_ctx && game.IsLegacyHiRes()) {
		w *= HIRES_COORD_MULTIPLIER;
		h *= HIRES_COORD_MULTIPLIER;
	}
}

AGS_INLINE int ctx_data_to_game_size(int size, bool hires_ctx) {
	if (hires_ctx && !game.IsLegacyHiRes())
		return Math::Max(1, (size / HIRES_COORD_MULTIPLIER));
	if (!hires_ctx && game.IsLegacyHiRes())
		return size * HIRES_COORD_MULTIPLIER;
	return size;
}

AGS_INLINE int game_to_ctx_data_size(int size, bool hires_ctx) {
	if (hires_ctx && !game.IsLegacyHiRes())
		return size * HIRES_COORD_MULTIPLIER;
	else if (!hires_ctx && game.IsLegacyHiRes())
		return Math::Max(1, (size / HIRES_COORD_MULTIPLIER));
	return size;
}

AGS_INLINE void defgame_to_finalgame_coords(int &x, int &y) {
	// Note we support only upscale now
	x *= game.GetScreenUpscaleMult();
	y *= game.GetScreenUpscaleMult();
}

// End resolution system functions

// Create blank (black) images used to repaint borders around game frame
void create_blank_image(int coldepth) {
	// this is the first time that we try to use the graphics driver,
	// so it's the most likey place for a crash
//	try {
		Bitmap *blank = BitmapHelper::CreateBitmap(16, 16, coldepth);
		blank = ReplaceBitmapWithSupportedFormat(blank);
		blank->Clear();
		blankImage = gfxDriver->CreateDDBFromBitmap(blank, false, true);
		blankSidebarImage = gfxDriver->CreateDDBFromBitmap(blank, false, true);
		delete blank;
/*	} catch (Ali3DException gfxException) {
		quit((char *)gfxException._message);
	}*/
}

void destroy_blank_image() {
	if (blankImage)
		gfxDriver->DestroyDDB(blankImage);
	if (blankSidebarImage)
		gfxDriver->DestroyDDB(blankSidebarImage);
	blankImage = nullptr;
	blankSidebarImage = nullptr;
}

int MakeColor(int color_index) {
	color_t real_color = 0;
	__my_setcolor(&real_color, color_index, game.GetColorDepth());
	return real_color;
}

void init_draw_method() {
	if (gfxDriver->HasAcceleratedTransform()) {
		walkBehindMethod = DrawAsSeparateSprite;
		create_blank_image(game.GetColorDepth());
	} else {
		walkBehindMethod = DrawOverCharSprite;
	}

	on_mainviewport_changed();
	init_room_drawdata();
	if (gfxDriver->UsesMemoryBackBuffer())
		gfxDriver->GetMemoryBackBuffer()->Clear();
}

void dispose_draw_method() {
	dispose_room_drawdata();
	dispose_invalid_regions(false);
	destroy_blank_image();
}

void dispose_room_drawdata() {
	CameraDrawData.clear();
	dispose_invalid_regions(true);
}

void on_mainviewport_changed() {
	if (!gfxDriver->RequiresFullRedrawEachFrame()) {
		init_invalid_regions(-1, play.GetMainViewport().GetSize(), RectWH(play.GetMainViewport().GetSize()));
		if (game.GetGameRes().ExceedsByAny(play.GetMainViewport().GetSize()))
			clear_letterbox_borders();
	}
}

// Allocates a bitmap for rendering camera/viewport pair (software render mode)
void prepare_roomview_frame(Viewport *view) {
	const int view_index = view->GetID();
	const Size view_sz = view->GetRect().GetSize();
	const Size cam_sz = view->GetCamera()->GetRect().GetSize();
	RoomCameraDrawData &draw_dat = CameraDrawData[view_index];
	// We use intermediate bitmap to render camera/viewport pair in software mode under these conditions:
	// * camera size and viewport size are different (this may be suboptimal to paint dirty rects stretched,
	//   and also Allegro backend cannot stretch background of different colour depth).
	// * viewport is located outside of the virtual screen (even if partially): subbitmaps cannot contain
	//   regions outside of master bitmap, and we must not clamp surface size to virtual screen because
	//   plugins may want to also use viewport bitmap, therefore it should retain full size.
	if (cam_sz == view_sz && !draw_dat.IsOffscreen) {
		// note we keep the buffer allocated in case it will become useful later
		draw_dat.Frame.reset();
	} else {
		PBitmap &camera_frame = draw_dat.Frame;
		PBitmap &camera_buffer = draw_dat.Buffer;
		if (!camera_buffer || camera_buffer->GetWidth() < cam_sz.Width || camera_buffer->GetHeight() < cam_sz.Height) {
			// Allocate new buffer bitmap with an extra size in case they will want to zoom out
			int room_width = data_to_game_coord(thisroom.Width);
			int room_height = data_to_game_coord(thisroom.Height);
			Size alloc_sz = Size::Clamp(cam_sz * 2, Size(1, 1), Size(room_width, room_height));
			camera_buffer.reset(new Bitmap(alloc_sz.Width, alloc_sz.Height, gfxDriver->GetMemoryBackBuffer()->GetColorDepth()));
		}

		if (!camera_frame || camera_frame->GetSize() != cam_sz) {
			camera_frame.reset(BitmapHelper::CreateSubBitmap(camera_buffer.get(), RectWH(cam_sz)));
		}
	}
}

// Syncs room viewport and camera in case either size has changed
void sync_roomview(Viewport *view) {
	if (view->GetCamera() == nullptr)
		return;
	init_invalid_regions(view->GetID(), view->GetCamera()->GetRect().GetSize(), play.GetRoomViewportAbs(view->GetID()));
	prepare_roomview_frame(view);
}

void init_room_drawdata() {
	if (gfxDriver->RequiresFullRedrawEachFrame())
		return;
	// Make sure all frame buffers are created for software drawing
	int view_count = play.GetRoomViewportCount();
	CameraDrawData.resize(view_count);
	for (int i = 0; i < play.GetRoomViewportCount(); ++i)
		sync_roomview(play.GetRoomViewport(i).get());
}

void on_roomviewport_created(int index) {
	if (!gfxDriver || gfxDriver->RequiresFullRedrawEachFrame())
		return;
	if ((size_t)index < CameraDrawData.size())
		return;
	CameraDrawData.resize(index + 1);
}

void on_roomviewport_deleted(int index) {
	if (gfxDriver->RequiresFullRedrawEachFrame())
		return;
	CameraDrawData.erase(CameraDrawData.begin() + index);
	delete_invalid_regions(index);
}

void on_roomviewport_changed(Viewport *view) {
	if (gfxDriver->RequiresFullRedrawEachFrame())
		return;
	if (!view->IsVisible() || view->GetCamera() == nullptr)
		return;
	const bool off = !IsRectInsideRect(RectWH(gfxDriver->GetMemoryBackBuffer()->GetSize()), view->GetRect());
	const bool off_changed = off != CameraDrawData[view->GetID()].IsOffscreen;
	CameraDrawData[view->GetID()].IsOffscreen = off;
	if (view->HasChangedSize())
		sync_roomview(view);
	else if (off_changed)
		prepare_roomview_frame(view);
	// TODO: don't have to do this all the time, perhaps do "dirty rect" method
	// and only clear previous viewport location?
	invalidate_screen();
	gfxDriver->GetMemoryBackBuffer()->Clear();
}

void detect_roomviewport_overlaps(size_t z_index) {
	if (gfxDriver->RequiresFullRedrawEachFrame())
		return;
	// Find out if we overlap or are overlapped by anything;
	const auto &viewports = play.GetRoomViewportsZOrdered();
	for (; z_index < viewports.size(); ++z_index) {
		auto this_view = viewports[z_index];
		const int this_id = this_view->GetID();
		bool is_overlap = false;
		if (!this_view->IsVisible()) continue;
		for (size_t z_index2 = 0; z_index2 < z_index; ++z_index) {
			if (!viewports[z_index2]->IsVisible()) continue;
			if (AreRectsIntersecting(this_view->GetRect(), viewports[z_index2]->GetRect())) {
				is_overlap = true;
				break;
			}
		}
		if (CameraDrawData[this_id].IsOverlap != is_overlap) {
			CameraDrawData[this_id].IsOverlap = is_overlap;
			prepare_roomview_frame(this_view.get());
		}
	}
}

void on_roomcamera_changed(Camera *cam) {
	if (gfxDriver->RequiresFullRedrawEachFrame())
		return;
	if (cam->HasChangedSize()) {
		auto viewrefs = cam->GetLinkedViewports();
		for (auto vr : viewrefs) {
			PViewport vp = vr.lock();
			if (vp)
				sync_roomview(vp.get());
		}
	}
	// TODO: only invalidate what this particular camera sees
	invalidate_screen();
}

void mark_screen_dirty() {
	screen_is_dirty = true;
}

bool is_screen_dirty() {
	return screen_is_dirty;
}

void invalidate_screen() {
	invalidate_all_rects();
}

void invalidate_camera_frame(int index) {
	invalidate_all_camera_rects(index);
}

void invalidate_rect(int x1, int y1, int x2, int y2, bool in_room) {
	//if (!in_room)
	invalidate_rect_ds(x1, y1, x2, y2, in_room);
}

void invalidate_sprite(int x1, int y1, IDriverDependantBitmap *pic, bool in_room) {
	//if (!in_room)
	invalidate_rect_ds(x1, y1, x1 + pic->GetWidth(), y1 + pic->GetHeight(), in_room);
}

void mark_current_background_dirty() {
	current_background_is_dirty = true;
}


void draw_and_invalidate_text(Bitmap *ds, int x1, int y1, int font, color_t text_color, const char *text) {
	wouttext_outline(ds, x1, y1, font, text_color, (const char *)text);
	invalidate_rect(x1, y1, x1 + wgettextwidth_compensate(text, font), y1 + getfontheight_outlined(font) + get_fixed_pixel_size(1), false);
}

// Renders black borders for the legacy boxed game mode,
// where whole game screen changes size between large and small rooms
void render_black_borders() {
	if (gfxDriver->UsesMemoryBackBuffer())
		return;
	{
		gfxDriver->BeginSpriteBatch(RectWH(game.GetGameRes()), SpriteTransform());
		const Rect &viewport = play.GetMainViewport();
		if (viewport.Top > 0) {
			// letterbox borders
			blankImage->SetStretch(game.GetGameRes().Width, viewport.Top, false);
			gfxDriver->DrawSprite(0, 0, blankImage);
			gfxDriver->DrawSprite(0, viewport.Bottom + 1, blankImage);
		}
		if (viewport.Left > 0) {
			// sidebar borders for widescreen
			blankSidebarImage->SetStretch(viewport.Left, viewport.GetHeight(), false);
			gfxDriver->DrawSprite(0, 0, blankSidebarImage);
			gfxDriver->DrawSprite(viewport.Right + 1, 0, blankSidebarImage);
		}
	}
}


void render_to_screen() {
	// Stage: final plugin callback (still drawn on game screen
	if (pl_any_want_hook(AGSE_FINALSCREENDRAW)) {
		gfxDriver->BeginSpriteBatch(play.GetMainViewport(), SpriteTransform(), Point(0, play.shake_screen_yoff), (GlobalFlipType)play.screen_flipped);
		gfxDriver->DrawSprite(AGSE_FINALSCREENDRAW, 0, nullptr);
	}
	// Stage: engine overlay
	construct_engine_overlay();

	// only vsync in full screen mode, it makes things worse in a window
	gfxDriver->EnableVsyncBeforeRender((scsystem.vsync > 0) && (!scsystem.windowed));

	bool succeeded = false;
	while (!succeeded) {
//		try {
			// For software renderer, need to blacken upper part of the game frame when shaking screen moves image down
			const Rect &viewport = play.GetMainViewport();
			if (play.shake_screen_yoff > 0 && !gfxDriver->RequiresFullRedrawEachFrame())
				gfxDriver->ClearRectangle(viewport.Left, viewport.Top, viewport.GetWidth() - 1, play.shake_screen_yoff, nullptr);
			gfxDriver->Render(0, play.shake_screen_yoff, (GlobalFlipType)play.screen_flipped);

#if AGS_PLATFORM_OS_ANDROID
			if (game.color_depth == 1)
				android_render();
#elif AGS_PLATFORM_OS_IOS
			if (game.color_depth == 1)
				ios_render();
#endif

			succeeded = true;
/*		} catch (Ali3DFullscreenLostException) {
			platform->Delay(500);
		}*/
	}
}

// Blanks out borders around main viewport in case it became smaller (e.g. after loading another room)
void clear_letterbox_borders() {
	const Rect &viewport = play.GetMainViewport();
	gfxDriver->ClearRectangle(0, 0, game.GetGameRes().Width - 1, viewport.Top - 1, nullptr);
	gfxDriver->ClearRectangle(0, viewport.Bottom + 1, game.GetGameRes().Width - 1, game.GetGameRes().Height - 1, nullptr);
}

void draw_game_screen_callback() {
	construct_game_scene(true);
	construct_game_screen_overlay(false);
}



void putpixel_compensate(Bitmap *ds, int xx, int yy, int col) {
	if ((ds->GetColorDepth() == 32) && (col != 0)) {
		// ensure the alpha channel is preserved if it has one
		int alphaval = geta32(ds->GetPixel(xx, yy));
		col = makeacol32(getr32(col), getg32(col), getb32(col), alphaval);
	}
	ds->FillRect(Rect(xx, yy, xx + get_fixed_pixel_size(1) - 1, yy + get_fixed_pixel_size(1) - 1), col);
}




void draw_sprite_support_alpha(Bitmap *ds, bool ds_has_alpha, int xpos, int ypos, Bitmap *image, bool src_has_alpha,
                               BlendMode blend_mode, int alpha) {
	if (alpha <= 0)
		return;

	if (game.options[OPT_SPRITEALPHA] == kSpriteAlphaRender_Proper) {
		GfxUtil::DrawSpriteBlend(ds, Point(xpos, ypos), image, blend_mode, ds_has_alpha, src_has_alpha, alpha);
	}
	// Backwards-compatible drawing
	else if (src_has_alpha && alpha == 0xFF) {
		set_alpha_blender();
		ds->TransBlendBlt(image, xpos, ypos);
	} else {
		GfxUtil::DrawSpriteWithTransparency(ds, image, xpos, ypos, alpha);
	}
}

void draw_sprite_slot_support_alpha(Bitmap *ds, bool ds_has_alpha, int xpos, int ypos, int src_slot,
                                    BlendMode blend_mode, int alpha) {
	draw_sprite_support_alpha(ds, ds_has_alpha, xpos, ypos, spriteset[src_slot], (game.SpriteInfos[src_slot].Flags & SPF_ALPHACHANNEL) != 0,
	                          blend_mode, alpha);
}


IDriverDependantBitmap *recycle_ddb_bitmap(IDriverDependantBitmap *bimp, Bitmap *source, bool hasAlpha, bool opaque) {
	if (bimp != nullptr) {
		// same colour depth, width and height -> reuse
		if (((bimp->GetColorDepth() + 1) / 8 == source->GetBPP()) &&
		        (bimp->GetWidth() == source->GetWidth()) && (bimp->GetHeight() == source->GetHeight())) {
			gfxDriver->UpdateDDBFromBitmap(bimp, source, hasAlpha);
			return bimp;
		}

		gfxDriver->DestroyDDB(bimp);
	}
	bimp = gfxDriver->CreateDDBFromBitmap(source, hasAlpha, opaque);
	return bimp;
}

void invalidate_cached_walkbehinds() {
	memset(&actspswbcache[0], 0, sizeof(CachedActSpsData) * actSpsCount);
}

// sort_out_walk_behinds: modifies the supplied sprite by overwriting parts
// of it with transparent pixels where there are walk-behind areas
// Returns whether any pixels were updated
int sort_out_walk_behinds(Bitmap *sprit, int xx, int yy, int basel, Bitmap *copyPixelsFrom = nullptr, Bitmap *checkPixelsFrom = nullptr, int zoom = 100) {
	if (noWalkBehindsAtAll)
		return 0;

	if ((!thisroom.WalkBehindMask->IsMemoryBitmap()) ||
	        (!sprit->IsMemoryBitmap()))
		quit("!sort_out_walk_behinds: wb bitmap not linear");

	int rr, tmm, toheight; //,tcol;
	// precalculate this to try and shave some time off
	int maskcol = sprit->GetMaskColor();
	int spcoldep = sprit->GetColorDepth();
	int screenhit = thisroom.WalkBehindMask->GetHeight();
	short *shptr, *shptr2;
	int *loptr, *loptr2;
	int pixelsChanged = 0;
	int ee = 0;
	if (xx < 0)
		ee = 0 - xx;

	if ((checkPixelsFrom != nullptr) && (checkPixelsFrom->GetColorDepth() != spcoldep))
		quit("sprite colour depth does not match background colour depth");

	for (; ee < sprit->GetWidth(); ee++) {
		if (ee + xx >= thisroom.WalkBehindMask->GetWidth())
			break;

		if ((!walkBehindExists[ee + xx]) ||
		        (walkBehindEndY[ee + xx] <= yy) ||
		        (walkBehindStartY[ee + xx] > yy + sprit->GetHeight()))
			continue;

		toheight = sprit->GetHeight();

		if (walkBehindStartY[ee + xx] < yy)
			rr = 0;
		else
			rr = (walkBehindStartY[ee + xx] - yy);

		// Since we will use _getpixel, ensure we only check within the screen
		if (rr + yy < 0)
			rr = 0 - yy;
		if (toheight + yy > screenhit)
			toheight = screenhit - yy;
		if (toheight + yy > walkBehindEndY[ee + xx])
			toheight = walkBehindEndY[ee + xx] - yy;
		if (rr < 0)
			rr = 0;

		for (; rr < toheight; rr++) {

			// we're ok with _getpixel because we've checked the screen edges
			//tmm = _getpixel(thisroom.WalkBehindMask,ee+xx,rr+yy);
			// actually, _getpixel is well inefficient, do it ourselves
			// since we know it's 8-bit bitmap
			tmm = thisroom.WalkBehindMask->GetScanLine(rr + yy)[ee + xx];
			if (tmm < 1) continue;
			if (croom->walkbehind_base[tmm] <= basel) continue;

			if (copyPixelsFrom != nullptr) {
				if (spcoldep <= 8) {
					if (checkPixelsFrom->GetScanLine((rr * 100) / zoom)[(ee * 100) / zoom] != maskcol) {
						sprit->GetScanLineForWriting(rr)[ee] = copyPixelsFrom->GetScanLine(rr + yy)[ee + xx];
						pixelsChanged = 1;
					}
				} else if (spcoldep <= 16) {
					shptr = (short *)&sprit->GetScanLine(rr)[0];
					shptr2 = (short *)&checkPixelsFrom->GetScanLine((rr * 100) / zoom)[0];
					if (shptr2[(ee * 100) / zoom] != maskcol) {
						shptr[ee] = ((short *)(&copyPixelsFrom->GetScanLine(rr + yy)[0]))[ee + xx];
						pixelsChanged = 1;
					}
				} else if (spcoldep == 24) {
					char *chptr = (char *)&sprit->GetScanLine(rr)[0];
					char *chptr2 = (char *)&checkPixelsFrom->GetScanLine((rr * 100) / zoom)[0];
					if (memcmp(&chptr2[((ee * 100) / zoom) * 3], &maskcol, 3) != 0) {
						memcpy(&chptr[ee * 3], &copyPixelsFrom->GetScanLine(rr + yy)[(ee + xx) * 3], 3);
						pixelsChanged = 1;
					}
				} else if (spcoldep <= 32) {
					loptr = (int *)&sprit->GetScanLine(rr)[0];
					loptr2 = (int *)&checkPixelsFrom->GetScanLine((rr * 100) / zoom)[0];
					if (loptr2[(ee * 100) / zoom] != maskcol) {
						loptr[ee] = ((int *)(&copyPixelsFrom->GetScanLine(rr + yy)[0]))[ee + xx];
						pixelsChanged = 1;
					}
				}
			} else {
				pixelsChanged = 1;
				if (spcoldep <= 8)
					sprit->GetScanLineForWriting(rr)[ee] = maskcol;
				else if (spcoldep <= 16) {
					shptr = (short *)&sprit->GetScanLine(rr)[0];
					shptr[ee] = maskcol;
				} else if (spcoldep == 24) {
					char *chptr = (char *)&sprit->GetScanLine(rr)[0];
					memcpy(&chptr[ee * 3], &maskcol, 3);
				} else if (spcoldep <= 32) {
					loptr = (int *)&sprit->GetScanLine(rr)[0];
					loptr[ee] = maskcol;
				} else
					quit("!Sprite colour depth >32 ??");
			}
		}
	}
	return pixelsChanged;
}

void sort_out_char_sprite_walk_behind(int actspsIndex, int xx, int yy, int basel, int zoom, int width, int height) {
	if (noWalkBehindsAtAll)
		return;

	if ((!actspswbcache[actspsIndex].valid) ||
	        (actspswbcache[actspsIndex].xWas != xx) ||
	        (actspswbcache[actspsIndex].yWas != yy) ||
	        (actspswbcache[actspsIndex].baselineWas != basel)) {
		actspswb[actspsIndex] = recycle_bitmap(actspswb[actspsIndex], thisroom.BgFrames[play.bg_frame].Graphic->GetColorDepth(), width, height, true);
		Bitmap *wbSprite = actspswb[actspsIndex];

		actspswbcache[actspsIndex].isWalkBehindHere = sort_out_walk_behinds(wbSprite, xx, yy, basel, thisroom.BgFrames[play.bg_frame].Graphic.get(), actsps[actspsIndex], zoom);
		actspswbcache[actspsIndex].xWas = xx;
		actspswbcache[actspsIndex].yWas = yy;
		actspswbcache[actspsIndex].baselineWas = basel;
		actspswbcache[actspsIndex].valid = 1;

		if (actspswbcache[actspsIndex].isWalkBehindHere) {
			actspswbbmp[actspsIndex] = recycle_ddb_bitmap(actspswbbmp[actspsIndex], actspswb[actspsIndex], false);
		}
	}

	if (actspswbcache[actspsIndex].isWalkBehindHere) {
		add_to_sprite_list(actspswbbmp[actspsIndex], xx, yy, basel, 0, -1, true);
	}
}

void clear_draw_list() {
	thingsToDrawList.clear();
}
void add_thing_to_draw(IDriverDependantBitmap *bmp, int x, int y, int trans, bool alphaChannel) {
	SpriteListEntry sprite;
	sprite.pic = nullptr;
	sprite.bmp = bmp;
	sprite.x = x;
	sprite.y = y;
	sprite.transparent = trans;
	sprite.hasAlphaChannel = alphaChannel;
	thingsToDrawList.push_back(sprite);
}

// the sprite list is an intermediate list used to order
// objects and characters by their baselines before everything
// is added to the Thing To Draw List
void clear_sprite_list() {
	sprlist.clear();
}
void add_to_sprite_list(IDriverDependantBitmap *spp, int xx, int yy, int baseline, int trans, int sprNum, bool isWalkBehind) {

	if (spp == nullptr)
		quit("add_to_sprite_list: attempted to draw NULL sprite");
	// completely invisible, so don't draw it at all
	if (trans == 255)
		return;

	SpriteListEntry sprite;
	if ((sprNum >= 0) && ((game.SpriteInfos[sprNum].Flags & SPF_ALPHACHANNEL) != 0))
		sprite.hasAlphaChannel = true;
	else
		sprite.hasAlphaChannel = false;

	sprite.bmp = spp;
	sprite.baseline = baseline;
	sprite.x = xx;
	sprite.y = yy;
	sprite.transparent = trans;

	if (walkBehindMethod == DrawAsSeparateSprite)
		sprite.takesPriorityIfEqual = !isWalkBehind;
	else
		sprite.takesPriorityIfEqual = isWalkBehind;

	sprlist.push_back(sprite);
}

void repair_alpha_channel(Bitmap *dest, Bitmap *bgpic) {
	// Repair the alpha channel, because sprites may have been drawn
	// over it by the buttons, etc
	int theWid = (dest->GetWidth() < bgpic->GetWidth()) ? dest->GetWidth() : bgpic->GetWidth();
	int theHit = (dest->GetHeight() < bgpic->GetHeight()) ? dest->GetHeight() : bgpic->GetHeight();
	for (int y = 0; y < theHit; y++) {
		unsigned int *destination = ((unsigned int *)dest->GetScanLineForWriting(y));
		unsigned int *source = ((unsigned int *)bgpic->GetScanLineForWriting(y));
		for (int x = 0; x < theWid; x++) {
			destination[x] |= (source[x] & 0xff000000);
		}
	}
}


// used by GUI renderer to draw images
void draw_gui_sprite(Bitmap *ds, int pic, int x, int y, bool use_alpha, BlendMode blend_mode) {
	Bitmap *sprite = spriteset[pic];
	const bool ds_has_alpha  = ds->GetColorDepth() == 32;
	const bool src_has_alpha = (game.SpriteInfos[pic].Flags & SPF_ALPHACHANNEL) != 0;

	if (use_alpha && game.options[OPT_NEWGUIALPHA] == kGuiAlphaRender_Proper) {
		GfxUtil::DrawSpriteBlend(ds, Point(x, y), sprite, blend_mode, ds_has_alpha, src_has_alpha);
	}
	// Backwards-compatible drawing
	else if (use_alpha && ds_has_alpha && game.options[OPT_NEWGUIALPHA] == kGuiAlphaRender_AdditiveAlpha) {
		if (src_has_alpha)
			set_additive_alpha_blender();
		else
			set_opaque_alpha_blender();
		ds->TransBlendBlt(sprite, x, y);
	} else {
		GfxUtil::DrawSpriteWithTransparency(ds, sprite, x, y);
	}
}

void draw_gui_sprite_v330(Bitmap *ds, int pic, int x, int y, bool use_alpha, BlendMode blend_mode) {
	draw_gui_sprite(ds, pic, x, y, use_alpha && (loaded_game_file_version >= kGameVersion_330), blend_mode);
}

// function to sort the sprites into baseline order
bool spritelistentry_less(const SpriteListEntry &e1, const SpriteListEntry &e2) {
	if (e1.baseline == e2.baseline) {
		if (e1.takesPriorityIfEqual)
			return false;
		if (e2.takesPriorityIfEqual)
			return true;
	}
	return e1.baseline < e2.baseline;
}




void draw_sprite_list() {

	if (walkBehindMethod == DrawAsSeparateSprite) {
		for (int ee = 1; ee < MAX_WALK_BEHINDS; ee++) {
			if (walkBehindBitmap[ee] != nullptr) {
				add_to_sprite_list(walkBehindBitmap[ee], walkBehindLeft[ee], walkBehindTop[ee],
				                   croom->walkbehind_base[ee], 0, -1, true);
			}
		}
	}

	std::sort(sprlist.begin(), sprlist.end(), spritelistentry_less);

	if (pl_any_want_hook(AGSE_PRESCREENDRAW))
		add_thing_to_draw(nullptr, AGSE_PRESCREENDRAW, 0, TRANS_RUN_PLUGIN, false);

	// copy the sorted sprites into the Things To Draw list
	thingsToDrawList.insert(thingsToDrawList.end(), sprlist.begin(), sprlist.end());
}

// Avoid freeing and reallocating the memory if possible
Bitmap *recycle_bitmap(Bitmap *bimp, int coldep, int wid, int hit, bool make_transparent) {
	if (bimp != nullptr) {
		// same colour depth, width and height -> reuse
		if ((bimp->GetColorDepth() == coldep) && (bimp->GetWidth() == wid)
		        && (bimp->GetHeight() == hit)) {
			if (make_transparent) {
				bimp->ClearTransparent();
			}
			return bimp;
		}

		delete bimp;
	}
	bimp = make_transparent ? BitmapHelper::CreateTransparentBitmap(wid, hit, coldep) :
	       BitmapHelper::CreateBitmap(wid, hit, coldep);
	return bimp;
}


// Get the local tint at the specified X & Y co-ordinates, based on
// room regions and SetAmbientTint
// tint_amnt will be set to 0 if there is no tint enabled
// if this is the case, then light_lev holds the light level (0=none)
void get_local_tint(int xpp, int ypp, int nolight,
                    int *tint_amnt, int *tint_r, int *tint_g,
                    int *tint_b, int *tint_lit,
                    int *light_lev) {

	int tint_level = 0, light_level = 0;
	int tint_amount = 0;
	int tint_red = 0;
	int tint_green = 0;
	int tint_blue = 0;
	int tint_light = 255;

	if (nolight == 0) {

		int onRegion = 0;

		if ((play.ground_level_areas_disabled & GLED_EFFECTS) == 0) {
			// check if the player is on a region, to find its
			// light/tint level
			onRegion = GetRegionIDAtRoom(xpp, ypp);
			if (onRegion == 0) {
				// when walking, he might just be off a walkable area
				onRegion = GetRegionIDAtRoom(xpp - 3, ypp);
				if (onRegion == 0)
					onRegion = GetRegionIDAtRoom(xpp + 3, ypp);
				if (onRegion == 0)
					onRegion = GetRegionIDAtRoom(xpp, ypp - 3);
				if (onRegion == 0)
					onRegion = GetRegionIDAtRoom(xpp, ypp + 3);
			}
		}

		if ((onRegion > 0) && (onRegion < MAX_ROOM_REGIONS)) {
			light_level = thisroom.Regions[onRegion].Light;
			tint_level = thisroom.Regions[onRegion].Tint;
		} else if (onRegion <= 0) {
			light_level = thisroom.Regions[0].Light;
			tint_level = thisroom.Regions[0].Tint;
		}

		int tint_sat = (tint_level >> 24) & 0xFF;
		if ((game.color_depth == 1) || ((tint_level & 0x00ffffff) == 0) ||
		        (tint_sat == 0))
			tint_level = 0;

		if (tint_level) {
			tint_red = (unsigned char)(tint_level & 0x000ff);
			tint_green = (unsigned char)((tint_level >> 8) & 0x000ff);
			tint_blue = (unsigned char)((tint_level >> 16) & 0x000ff);
			tint_amount = tint_sat;
			tint_light = light_level;
		}

		if (play.rtint_enabled) {
			if (play.rtint_level > 0) {
				// override with room tint
				tint_red = play.rtint_red;
				tint_green = play.rtint_green;
				tint_blue = play.rtint_blue;
				tint_amount = play.rtint_level;
				tint_light = play.rtint_light;
			} else {
				// override with room light level
				tint_amount = 0;
				light_level = play.rtint_light;
			}
		}
	}

	// copy to output parameters
	*tint_amnt = tint_amount;
	*tint_r = tint_red;
	*tint_g = tint_green;
	*tint_b = tint_blue;
	*tint_lit = tint_light;
	if (light_lev)
		*light_lev = light_level;
}




// Applies the specified RGB Tint or Light Level to the actsps
// sprite indexed with actspsindex
void apply_tint_or_light(int actspsindex, int light_level,
                         int tint_amount, int tint_red, int tint_green,
                         int tint_blue, int tint_light, int coldept,
                         Bitmap *blitFrom) {

// In a 256-colour game, we cannot do tinting or lightening
// (but we can do darkening, if light_level < 0)
	if (game.color_depth == 1) {
		if ((light_level > 0) || (tint_amount != 0))
			return;
	}

// we can only do tint/light if the colour depths match
	if (game.GetColorDepth() == actsps[actspsindex]->GetColorDepth()) {
		Bitmap *oldwas;
		// if the caller supplied a source bitmap, ->Blit from it
		// (used as a speed optimisation where possible)
		if (blitFrom)
			oldwas = blitFrom;
		// otherwise, make a new target bmp
		else {
			oldwas = actsps[actspsindex];
			actsps[actspsindex] = BitmapHelper::CreateBitmap(oldwas->GetWidth(), oldwas->GetHeight(), coldept);
		}
		Bitmap *active_spr = actsps[actspsindex];

		if (tint_amount) {
			// It is an RGB tint
			tint_image(active_spr, oldwas, tint_red, tint_green, tint_blue, tint_amount, tint_light);
		} else {
			// the RGB values passed to set_trans_blender decide whether it will darken
			// or lighten sprites ( <128=darken, >128=lighten). The parameter passed
			// to LitBlendBlt defines how much it will be darkened/lightened by.

			int lit_amnt;
			active_spr->FillTransparent();
			// It's a light level, not a tint
			if (game.color_depth == 1) {
				// 256-col
				lit_amnt = (250 - ((-light_level) * 5) / 2);
			} else {
				// hi-color
				if (light_level < 0)
					set_my_trans_blender(8, 8, 8, 0);
				else
					set_my_trans_blender(248, 248, 248, 0);
				lit_amnt = abs(light_level) * 2;
			}

			active_spr->LitBlendBlt(oldwas, 0, 0, lit_amnt);
		}

		if (oldwas != blitFrom)
			delete oldwas;

	} else if (blitFrom) {
		// sprite colour depth != game colour depth, so don't try and tint
		// but we do need to do something, so copy the source
		Bitmap *active_spr = actsps[actspsindex];
		active_spr->Blit(blitFrom, 0, 0, 0, 0, active_spr->GetWidth(), active_spr->GetHeight());
	}

}

// Draws the specified 'sppic' sprite onto actsps[useindx] at the
// specified width and height, and flips the sprite if necessary.
// Returns 1 if something was drawn to actsps; returns 0 if no
// scaling or stretching was required, in which case nothing was done
int scale_and_flip_sprite(int useindx, int coldept, int zoom_level,
                          int sppic, int newwidth, int newheight,
                          int isMirrored) {

	int actsps_used = 1;

	// create and blank out the new sprite
	actsps[useindx] = recycle_bitmap(actsps[useindx], coldept, newwidth, newheight, true);
	Bitmap *active_spr = actsps[useindx];

	if (zoom_level != 100) {
		// Scaled character

		our_eip = 334;

		// Ensure that anti-aliasing routines have a palette to
		// use for mapping while faded out
		if (in_new_room)
			select_palette(palette);


		if (isMirrored) {
			Bitmap *tempspr = BitmapHelper::CreateBitmap(newwidth, newheight, coldept);
			tempspr->Fill(actsps[useindx]->GetMaskColor());
			if ((IS_ANTIALIAS_SPRITES) && ((game.SpriteInfos[sppic].Flags & SPF_ALPHACHANNEL) == 0))
				tempspr->AAStretchBlt(spriteset[sppic], RectWH(0, 0, newwidth, newheight), Shared::kBitmap_Transparency);
			else
				tempspr->StretchBlt(spriteset[sppic], RectWH(0, 0, newwidth, newheight), Shared::kBitmap_Transparency);
			active_spr->FlipBlt(tempspr, 0, 0, Shared::kBitmap_HFlip);
			delete tempspr;
		} else if ((IS_ANTIALIAS_SPRITES) && ((game.SpriteInfos[sppic].Flags & SPF_ALPHACHANNEL) == 0))
			active_spr->AAStretchBlt(spriteset[sppic], RectWH(0, 0, newwidth, newheight), Shared::kBitmap_Transparency);
		else
			active_spr->StretchBlt(spriteset[sppic], RectWH(0, 0, newwidth, newheight), Shared::kBitmap_Transparency);

		/*  AASTR2 version of code (doesn't work properly, gives black borders)
		if (IS_ANTIALIAS_SPRITES) {
		int aa_mode = AA_MASKED;
		if (game.spriteflags[sppic] & SPF_ALPHACHANNEL)
		aa_mode |= AA_ALPHA | AA_RAW_ALPHA;
		if (isMirrored)
		aa_mode |= AA_HFLIP;

		aa_set_mode(aa_mode);
		->AAStretchBlt(actsps[useindx],spriteset[sppic],0,0,newwidth,newheight);
		}
		else if (isMirrored) {
		Bitmap *tempspr = BitmapHelper::CreateBitmap_ (coldept, newwidth, newheight);
		->Clear (tempspr, ->GetMaskColor(actsps[useindx]));
		->StretchBlt (tempspr, spriteset[sppic], 0, 0, newwidth, newheight);
		->FlipBlt(Shared::kBitmap_HFlip, (actsps[useindx], tempspr, 0, 0);
		wfreeblock (tempspr);
		}
		else
		->StretchBlt(actsps[useindx],spriteset[sppic],0,0,newwidth,newheight);
		*/
		if (in_new_room)
			unselect_palette();

	} else {
		// Not a scaled character, draw at normal size

		our_eip = 339;

		if (isMirrored)
			active_spr->FlipBlt(spriteset[sppic], 0, 0, Shared::kBitmap_HFlip);
		else
			actsps_used = 0;
		//->Blit (spriteset[sppic], actsps[useindx], 0, 0, 0, 0, actsps[useindx]->GetWidth(), actsps[useindx]->GetHeight());
	}

	return actsps_used;
}



// create the actsps[aa] image with the object drawn correctly
// returns 1 if nothing at all has changed and actsps is still
// intact from last time; 0 otherwise
int construct_object_gfx(int aa, int *drawnWidth, int *drawnHeight, bool alwaysUseSoftware) {
	int useindx = aa;
	bool hardwareAccelerated = !alwaysUseSoftware && gfxDriver->HasAcceleratedTransform();

	if (spriteset[objs[aa].num] == nullptr)
		quitprintf("There was an error drawing object %d. Its current sprite, %d, is invalid.", aa, objs[aa].num);

	int coldept = spriteset[objs[aa].num]->GetColorDepth();
	int sprwidth = game.SpriteInfos[objs[aa].num].Width;
	int sprheight = game.SpriteInfos[objs[aa].num].Height;

	int tint_red, tint_green, tint_blue;
	int tint_level, tint_light, light_level;
	int zoom_level = 100;

	// calculate the zoom level
	if ((objs[aa].flags & OBJF_USEROOMSCALING) == 0) {
		zoom_level = objs[aa].zoom;
	} else {
		int onarea = get_walkable_area_at_location(objs[aa].x, objs[aa].y);
		if ((onarea <= 0) && (thisroom.WalkAreas[0].ScalingFar == 0)) {
			// just off the edge of an area -- use the scaling we had
			// while on the area
			zoom_level = objs[aa].zoom;
		} else
			zoom_level = get_area_scaling(onarea, objs[aa].x, objs[aa].y);
	}
	if (zoom_level != 100)
		scale_sprite_size(objs[aa].num, zoom_level, &sprwidth, &sprheight);
	objs[aa].zoom = zoom_level;

	// save width/height into parameters if requested
	if (drawnWidth)
		*drawnWidth = sprwidth;
	if (drawnHeight)
		*drawnHeight = sprheight;

	objs[aa].last_width = sprwidth;
	objs[aa].last_height = sprheight;

	tint_red = tint_green = tint_blue = tint_level = tint_light = light_level = 0;

	if (objs[aa].flags & OBJF_HASTINT) {
		// object specific tint, use it
		tint_red = objs[aa].tint_r;
		tint_green = objs[aa].tint_g;
		tint_blue = objs[aa].tint_b;
		tint_level = objs[aa].tint_level;
		tint_light = objs[aa].tint_light;
		light_level = 0;
	} else if (objs[aa].flags & OBJF_HASLIGHT) {
		light_level = objs[aa].tint_light;
	} else {
		// get the ambient or region tint
		int ignoreRegionTints = 1;
		if (objs[aa].flags & OBJF_USEREGIONTINTS)
			ignoreRegionTints = 0;

		get_local_tint(objs[aa].x, objs[aa].y, ignoreRegionTints,
		               &tint_level, &tint_red, &tint_green, &tint_blue,
		               &tint_light, &light_level);
	}

	// check whether the image should be flipped
	int isMirrored = 0;
	if ((objs[aa].view >= 0) &&
	        (views[objs[aa].view].loops[objs[aa].loop].frames[objs[aa].frame].pic == objs[aa].num) &&
	        ((views[objs[aa].view].loops[objs[aa].loop].frames[objs[aa].frame].flags & VFLG_FLIPSPRITE) != 0)) {
		isMirrored = 1;
	}

	if ((hardwareAccelerated) &&
	        (walkBehindMethod != DrawOverCharSprite) &&
	        (objcache[aa].image != nullptr) &&
	        (objcache[aa].sppic == objs[aa].num) &&
	        (actsps[useindx] != nullptr)) {
		// HW acceleration
		objcache[aa].tintamntwas = tint_level;
		objcache[aa].tintredwas = tint_red;
		objcache[aa].tintgrnwas = tint_green;
		objcache[aa].tintbluwas = tint_blue;
		objcache[aa].tintlightwas = tint_light;
		objcache[aa].lightlevwas = light_level;
		objcache[aa].zoomWas = zoom_level;
		objcache[aa].mirroredWas = isMirrored;

		return 1;
	}

	if ((!hardwareAccelerated) && (gfxDriver->HasAcceleratedTransform())) {
		// They want to draw it in software mode with the D3D driver,
		// so force a redraw
		objcache[aa].sppic = -389538;
	}

	// If we have the image cached, use it
	if ((objcache[aa].image != nullptr) &&
	        (objcache[aa].sppic == objs[aa].num) &&
	        (objcache[aa].tintamntwas == tint_level) &&
	        (objcache[aa].tintlightwas == tint_light) &&
	        (objcache[aa].tintredwas == tint_red) &&
	        (objcache[aa].tintgrnwas == tint_green) &&
	        (objcache[aa].tintbluwas == tint_blue) &&
	        (objcache[aa].lightlevwas == light_level) &&
	        (objcache[aa].zoomWas == zoom_level) &&
	        (objcache[aa].mirroredWas == isMirrored)) {
		// the image is the same, we can use it cached!
		if ((walkBehindMethod != DrawOverCharSprite) &&
		        (actsps[useindx] != nullptr))
			return 1;
		// Check if the X & Y co-ords are the same, too -- if so, there
		// is scope for further optimisations
		if ((objcache[aa].xwas == objs[aa].x) &&
		        (objcache[aa].ywas == objs[aa].y) &&
		        (actsps[useindx] != nullptr) &&
		        (walk_behind_baselines_changed == 0))
			return 1;
		actsps[useindx] = recycle_bitmap(actsps[useindx], coldept, sprwidth, sprheight);
		actsps[useindx]->Blit(objcache[aa].image, 0, 0, 0, 0, objcache[aa].image->GetWidth(), objcache[aa].image->GetHeight());
		return 0;
	}

	// Not cached, so draw the image

	int actspsUsed = 0;
	if (!hardwareAccelerated) {
		// draw the base sprite, scaled and flipped as appropriate
		actspsUsed = scale_and_flip_sprite(useindx, coldept, zoom_level,
		                                   objs[aa].num, sprwidth, sprheight, isMirrored);
	} else {
		// ensure actsps exists
		actsps[useindx] = recycle_bitmap(actsps[useindx], coldept, game.SpriteInfos[objs[aa].num].Width, game.SpriteInfos[objs[aa].num].Height);
	}

	// direct read from source bitmap, where possible
	Bitmap *comeFrom = nullptr;
	if (!actspsUsed)
		comeFrom = spriteset[objs[aa].num];

	// apply tints or lightenings where appropriate, else just copy
	// the source bitmap
	if (!hardwareAccelerated && ((tint_level > 0) || (light_level != 0))) {
		apply_tint_or_light(useindx, light_level, tint_level, tint_red,
		                    tint_green, tint_blue, tint_light, coldept,
		                    comeFrom);
	} else if (!actspsUsed) {
		actsps[useindx]->Blit(spriteset[objs[aa].num], 0, 0, 0, 0, game.SpriteInfos[objs[aa].num].Width, game.SpriteInfos[objs[aa].num].Height);
	}

	// Re-use the bitmap if it's the same size
	objcache[aa].image = recycle_bitmap(objcache[aa].image, coldept, sprwidth, sprheight);
	// Create the cached image and store it
	objcache[aa].image->Blit(actsps[useindx], 0, 0, 0, 0, sprwidth, sprheight);
	objcache[aa].sppic = objs[aa].num;
	objcache[aa].tintamntwas = tint_level;
	objcache[aa].tintredwas = tint_red;
	objcache[aa].tintgrnwas = tint_green;
	objcache[aa].tintbluwas = tint_blue;
	objcache[aa].tintlightwas = tint_light;
	objcache[aa].lightlevwas = light_level;
	objcache[aa].zoomWas = zoom_level;
	objcache[aa].mirroredWas = isMirrored;
	return 0;
}




// This is only called from draw_screen_background, but it's seperated
// to help with profiling the program
void prepare_objects_for_drawing() {
	our_eip = 32;

	for (int aa = 0; aa < croom->numobj; aa++) {
		if (objs[aa].on != 1) continue;
		// offscreen, don't draw
		if ((objs[aa].x >= thisroom.Width) || (objs[aa].y < 1))
			continue;

		const int useindx = aa;
		int tehHeight;
		int actspsIntact = construct_object_gfx(aa, nullptr, &tehHeight, false);

		// update the cache for next time
		objcache[aa].xwas = objs[aa].x;
		objcache[aa].ywas = objs[aa].y;
		int atxp = data_to_game_coord(objs[aa].x);
		int atyp = data_to_game_coord(objs[aa].y) - tehHeight;

		int usebasel = objs[aa].get_baseline();

		if (objs[aa].flags & OBJF_NOWALKBEHINDS) {
			// ignore walk-behinds, do nothing
			if (walkBehindMethod == DrawAsSeparateSprite) {
				usebasel += thisroom.Height;
			}
		} else if (walkBehindMethod == DrawAsSeparateCharSprite) {
			sort_out_char_sprite_walk_behind(useindx, atxp, atyp, usebasel, objs[aa].zoom, objs[aa].last_width, objs[aa].last_height);
		} else if ((!actspsIntact) && (walkBehindMethod == DrawOverCharSprite)) {
			sort_out_walk_behinds(actsps[useindx], atxp, atyp, usebasel);
		}

		if ((!actspsIntact) || (actspsbmp[useindx] == nullptr)) {
			bool hasAlpha = (game.SpriteInfos[objs[aa].num].Flags & SPF_ALPHACHANNEL) != 0;

			if (actspsbmp[useindx] != nullptr)
				gfxDriver->DestroyDDB(actspsbmp[useindx]);
			actspsbmp[useindx] = gfxDriver->CreateDDBFromBitmap(actsps[useindx], hasAlpha);
		}

		if (gfxDriver->HasAcceleratedTransform()) {
			actspsbmp[useindx]->SetFlippedLeftRight(objcache[aa].mirroredWas != 0);
			actspsbmp[useindx]->SetStretch(objs[aa].last_width, objs[aa].last_height);
			actspsbmp[useindx]->SetTint(objcache[aa].tintredwas, objcache[aa].tintgrnwas, objcache[aa].tintbluwas, (objcache[aa].tintamntwas * 256) / 100);

			if (objcache[aa].tintamntwas > 0) {
				if (objcache[aa].tintlightwas == 0)  // luminance of 0 -- pass 1 to enable
					actspsbmp[useindx]->SetLightLevel(1);
				else if (objcache[aa].tintlightwas < 250)
					actspsbmp[useindx]->SetLightLevel(objcache[aa].tintlightwas);
				else
					actspsbmp[useindx]->SetLightLevel(0);
			} else if (objcache[aa].lightlevwas != 0)
				actspsbmp[useindx]->SetLightLevel((objcache[aa].lightlevwas * 25) / 10 + 256);
			else
				actspsbmp[useindx]->SetLightLevel(0);
		}

		add_to_sprite_list(actspsbmp[useindx], atxp, atyp, usebasel, objs[aa].transparent, objs[aa].num);
	}
}



// Draws srcimg onto destimg, tinting to the specified level
// Totally overwrites the contents of the destination image
void tint_image(Bitmap *ds, Bitmap *srcimg, int red, int grn, int blu, int light_level, int luminance) {

	if ((srcimg->GetColorDepth() != ds->GetColorDepth()) ||
	        (srcimg->GetColorDepth() <= 8)) {
		debug_script_warn("Image tint failed - images must both be hi-color");
		// the caller expects something to have been copied
		ds->Blit(srcimg, 0, 0, 0, 0, srcimg->GetWidth(), srcimg->GetHeight());
		return;
	}

	// For performance reasons, we have a seperate blender for
	// when light is being adjusted and when it is not.
	// If luminance >= 250, then normal brightness, otherwise darken
	if (luminance >= 250)
		set_blender_mode(_myblender_color15, _myblender_color16, _myblender_color32, red, grn, blu, 0);
	else
		set_blender_mode(_myblender_color15_light, _myblender_color16_light, _myblender_color32_light, red, grn, blu, 0);

	if (light_level >= 100) {
		// fully colourised
		ds->FillTransparent();
		ds->LitBlendBlt(srcimg, 0, 0, luminance);
	} else {
		// light_level is between -100 and 100 normally; 0-100 in
		// this case when it's a RGB tint
		light_level = (light_level * 25) / 10;

		// Copy the image to the new bitmap
		ds->Blit(srcimg, 0, 0, 0, 0, srcimg->GetWidth(), srcimg->GetHeight());
		// Render the colourised image to a temporary bitmap,
		// then transparently draw it over the original image
		Bitmap *finaltarget = BitmapHelper::CreateTransparentBitmap(srcimg->GetWidth(), srcimg->GetHeight(), srcimg->GetColorDepth());
		finaltarget->LitBlendBlt(srcimg, 0, 0, luminance);

		// customized trans blender to preserve alpha channel
		set_my_trans_blender(0, 0, 0, light_level);
		ds->TransBlendBlt(finaltarget, 0, 0);
		delete finaltarget;
	}
}




void prepare_characters_for_drawing() {
	int zoom_level, newwidth, newheight, onarea, sppic;
	int light_level, coldept;
	int tint_red, tint_green, tint_blue, tint_amount, tint_light = 255;

	our_eip = 33;

	// draw characters
	for (int aa = 0; aa < game.numcharacters; aa++) {
		if (game.chars[aa].on == 0) continue;
		if (game.chars[aa].room != displayed_room) continue;
		eip_guinum = aa;
		const int useindx = aa + MAX_ROOM_OBJECTS;

		CharacterInfo *chin = &game.chars[aa];
		our_eip = 330;
		// if it's on but set to view -1, they're being silly
		if (chin->view < 0) {
			quitprintf("!The character '%s' was turned on in the current room (room %d) but has not been assigned a view number.",
			           chin->name, displayed_room);
		}

		if (chin->frame >= views[chin->view].loops[chin->loop].numFrames)
			chin->frame = 0;

		if ((chin->loop >= views[chin->view].numLoops) ||
		        (views[chin->view].loops[chin->loop].numFrames < 1)) {
			quitprintf("!The character '%s' could not be displayed because there were no frames in loop %d of view %d.",
			           chin->name, chin->loop, chin->view + 1);
		}

		sppic = views[chin->view].loops[chin->loop].frames[chin->frame].pic;
		if (sppic < 0)
			sppic = 0;  // in case it's screwed up somehow
		our_eip = 331;
		// sort out the stretching if required
		onarea = get_walkable_area_at_character(aa);
		our_eip = 332;

		// calculate the zoom level
		if (chin->flags & CHF_MANUALSCALING)  // character ignores scaling
			zoom_level = charextra[aa].zoom;
		else if ((onarea <= 0) && (thisroom.WalkAreas[0].ScalingFar == 0)) {
			zoom_level = charextra[aa].zoom;
			// NOTE: room objects don't have this fix
			if (zoom_level == 0)
				zoom_level = 100;
		} else
			zoom_level = get_area_scaling(onarea, chin->x, chin->y);

		charextra[aa].zoom = zoom_level;

		tint_red = tint_green = tint_blue = tint_amount = tint_light = light_level = 0;

		if (chin->flags & CHF_HASTINT) {
			// object specific tint, use it
			tint_red = charextra[aa].tint_r;
			tint_green = charextra[aa].tint_g;
			tint_blue = charextra[aa].tint_b;
			tint_amount = charextra[aa].tint_level;
			tint_light = charextra[aa].tint_light;
			light_level = 0;
		} else if (chin->flags & CHF_HASLIGHT) {
			light_level = charextra[aa].tint_light;
		} else {
			get_local_tint(chin->x, chin->y, chin->flags & CHF_NOLIGHTING,
			               &tint_amount, &tint_red, &tint_green, &tint_blue,
			               &tint_light, &light_level);
		}

		our_eip = 3330;
		int isMirrored = 0, specialpic = sppic;
		bool usingCachedImage = false;

		coldept = spriteset[sppic]->GetColorDepth();

		// adjust the sppic if mirrored, so it doesn't accidentally
		// cache the mirrored frame as the real one
		if (views[chin->view].loops[chin->loop].frames[chin->frame].flags & VFLG_FLIPSPRITE) {
			isMirrored = 1;
			specialpic = -sppic;
		}

		our_eip = 3331;

		// if the character was the same sprite and scaling last time,
		// just use the cached image
		if ((charcache[aa].inUse) &&
		        (charcache[aa].sppic == specialpic) &&
		        (charcache[aa].scaling == zoom_level) &&
		        (charcache[aa].tintredwas == tint_red) &&
		        (charcache[aa].tintgrnwas == tint_green) &&
		        (charcache[aa].tintbluwas == tint_blue) &&
		        (charcache[aa].tintamntwas == tint_amount) &&
		        (charcache[aa].tintlightwas == tint_light) &&
		        (charcache[aa].lightlevwas == light_level)) {
			if (walkBehindMethod == DrawOverCharSprite) {
				actsps[useindx] = recycle_bitmap(actsps[useindx], charcache[aa].image->GetColorDepth(), charcache[aa].image->GetWidth(), charcache[aa].image->GetHeight());
				actsps[useindx]->Blit(charcache[aa].image, 0, 0, 0, 0, actsps[useindx]->GetWidth(), actsps[useindx]->GetHeight());
			} else {
				usingCachedImage = true;
			}
		} else if ((charcache[aa].inUse) &&
		           (charcache[aa].sppic == specialpic) &&
		           (gfxDriver->HasAcceleratedTransform())) {
			usingCachedImage = true;
		} else if (charcache[aa].inUse) {
			//destroy_bitmap (charcache[aa].image);
			charcache[aa].inUse = 0;
		}

		our_eip = 3332;

		if (zoom_level != 100) {
			// it needs to be stretched, so calculate the new dimensions

			scale_sprite_size(sppic, zoom_level, &newwidth, &newheight);
			charextra[aa].width = newwidth;
			charextra[aa].height = newheight;
		} else {
			// draw at original size, so just use the sprite width and height
			// TODO: store width and height always, that's much simplier to use for reference!
			charextra[aa].width = 0;
			charextra[aa].height = 0;
			newwidth = game.SpriteInfos[sppic].Width;
			newheight = game.SpriteInfos[sppic].Height;
		}

		our_eip = 3336;

		// Calculate the X & Y co-ordinates of where the sprite will be
		const int atxp = (data_to_game_coord(chin->x)) - newwidth / 2;
		const int atyp = (data_to_game_coord(chin->y) - newheight)
		                 // adjust the Y positioning for the character's Z co-ord
		                 - data_to_game_coord(chin->z);

		charcache[aa].scaling = zoom_level;
		charcache[aa].sppic = specialpic;
		charcache[aa].tintredwas = tint_red;
		charcache[aa].tintgrnwas = tint_green;
		charcache[aa].tintbluwas = tint_blue;
		charcache[aa].tintamntwas = tint_amount;
		charcache[aa].tintlightwas = tint_light;
		charcache[aa].lightlevwas = light_level;

		// If cache needs to be re-drawn
		if (!charcache[aa].inUse) {

			// create the base sprite in actsps[useindx], which will
			// be scaled and/or flipped, as appropriate
			int actspsUsed = 0;
			if (!gfxDriver->HasAcceleratedTransform()) {
				actspsUsed = scale_and_flip_sprite(
				                 useindx, coldept, zoom_level, sppic,
				                 newwidth, newheight, isMirrored);
			} else {
				// ensure actsps exists
				actsps[useindx] = recycle_bitmap(actsps[useindx], coldept, game.SpriteInfos[sppic].Width, game.SpriteInfos[sppic].Height);
			}

			our_eip = 335;

			if (((light_level != 0) || (tint_amount != 0)) &&
			        (!gfxDriver->HasAcceleratedTransform())) {
				// apply the lightening or tinting
				Bitmap *comeFrom = nullptr;
				// if possible, direct read from the source image
				if (!actspsUsed)
					comeFrom = spriteset[sppic];

				apply_tint_or_light(useindx, light_level, tint_amount, tint_red,
				                    tint_green, tint_blue, tint_light, coldept,
				                    comeFrom);
			} else if (!actspsUsed) {
				// no scaling, flipping or tinting was done, so just blit it normally
				actsps[useindx]->Blit(spriteset[sppic], 0, 0, 0, 0, actsps[useindx]->GetWidth(), actsps[useindx]->GetHeight());
			}

			// update the character cache with the new image
			charcache[aa].inUse = 1;
			//charcache[aa].image = BitmapHelper::CreateBitmap_ (coldept, actsps[useindx]->GetWidth(), actsps[useindx]->GetHeight());
			charcache[aa].image = recycle_bitmap(charcache[aa].image, coldept, actsps[useindx]->GetWidth(), actsps[useindx]->GetHeight());
			charcache[aa].image->Blit(actsps[useindx], 0, 0, 0, 0, actsps[useindx]->GetWidth(), actsps[useindx]->GetHeight());

		} // end if !cache.inUse

		int usebasel = chin->get_baseline();

		our_eip = 336;

		const int bgX = atxp + chin->pic_xoffs;
		const int bgY = atyp + chin->pic_yoffs;

		if (chin->flags & CHF_NOWALKBEHINDS) {
			// ignore walk-behinds, do nothing
			if (walkBehindMethod == DrawAsSeparateSprite) {
				usebasel += thisroom.Height;
			}
		} else if (walkBehindMethod == DrawAsSeparateCharSprite) {
			sort_out_char_sprite_walk_behind(useindx, bgX, bgY, usebasel, charextra[aa].zoom, newwidth, newheight);
		} else if (walkBehindMethod == DrawOverCharSprite) {
			sort_out_walk_behinds(actsps[useindx], bgX, bgY, usebasel);
		}

		if ((!usingCachedImage) || (actspsbmp[useindx] == nullptr)) {
			bool hasAlpha = (game.SpriteInfos[sppic].Flags & SPF_ALPHACHANNEL) != 0;

			actspsbmp[useindx] = recycle_ddb_bitmap(actspsbmp[useindx], actsps[useindx], hasAlpha);
		}

		if (gfxDriver->HasAcceleratedTransform()) {
			actspsbmp[useindx]->SetStretch(newwidth, newheight);
			actspsbmp[useindx]->SetFlippedLeftRight(isMirrored != 0);
			actspsbmp[useindx]->SetTint(tint_red, tint_green, tint_blue, (tint_amount * 256) / 100);

			if (tint_amount != 0) {
				if (tint_light == 0) // tint with 0 luminance, pass as 1 instead
					actspsbmp[useindx]->SetLightLevel(1);
				else if (tint_light < 250)
					actspsbmp[useindx]->SetLightLevel(tint_light);
				else
					actspsbmp[useindx]->SetLightLevel(0);
			} else if (light_level != 0)
				actspsbmp[useindx]->SetLightLevel((light_level * 25) / 10 + 256);
			else
				actspsbmp[useindx]->SetLightLevel(0);

		}

		our_eip = 337;

		chin->actx = atxp;
		chin->acty = atyp;

		add_to_sprite_list(actspsbmp[useindx], bgX, bgY, usebasel, chin->transparency, sppic);
	}
}


// Compiles a list of room sprites (characters, objects, background)
void prepare_room_sprites() {
	// Background sprite is required for the non-software renderers always,
	// and for software renderer in case there are overlapping viewports.
	// Note that software DDB is just a tiny wrapper around bitmap, so overhead is negligible.
	if (roomBackgroundBmp == nullptr) {
		update_polled_stuff_if_runtime();
		roomBackgroundBmp = gfxDriver->CreateDDBFromBitmap(thisroom.BgFrames[play.bg_frame].Graphic.get(), false, true);
	} else if (current_background_is_dirty) {
		update_polled_stuff_if_runtime();
		gfxDriver->UpdateDDBFromBitmap(roomBackgroundBmp, thisroom.BgFrames[play.bg_frame].Graphic.get(), false);
	}
	if (gfxDriver->RequiresFullRedrawEachFrame()) {
		if (current_background_is_dirty || walkBehindsCachedForBgNum != play.bg_frame) {
			if (walkBehindMethod == DrawAsSeparateSprite) {
				update_walk_behind_images();
			}
		}
		add_thing_to_draw(roomBackgroundBmp, 0, 0, 0, false);
	}
	current_background_is_dirty = false; // Note this is only place where this flag is checked

	clear_sprite_list();

	if ((debug_flags & DBG_NOOBJECTS) == 0) {
		prepare_objects_for_drawing();
		prepare_characters_for_drawing();

		if ((debug_flags & DBG_NODRAWSPRITES) == 0) {
			our_eip = 34;
			draw_sprite_list();
		}
	}
	our_eip = 36;
}

// Draws the black surface behind (or rather between) the room viewports
void draw_preroom_background() {
	if (gfxDriver->RequiresFullRedrawEachFrame())
		return;
	update_black_invreg_and_reset(gfxDriver->GetMemoryBackBuffer());
}

// Draws the room background on the given surface.
//
// NOTE that this is **strictly** for software rendering.
// ds is a full game screen surface, and roomcam_surface is a surface for drawing room camera content to.
// ds and roomcam_surface may be the same bitmap.
// no_transform flag tells to copy dirty regions on roomcam_surface without any coordinate conversion
// whatsoever.
PBitmap draw_room_background(Viewport *view, const SpriteTransform &room_trans) {
	our_eip = 31;

	// For the sake of software renderer, if there is any kind of camera transform required
	// except screen offset, we tell it to draw on separate bitmap first with zero transformation.
	// There are few reasons for this, primary is that Allegro does not support StretchBlt
	// between different colour depths (i.e. it won't correctly stretch blit 16-bit rooms to
	// 32-bit virtual screen).
	// Also see comment to ALSoftwareGraphicsDriver::RenderToBackBuffer().
	const int view_index = view->GetID();
	Bitmap *ds = gfxDriver->GetMemoryBackBuffer();
	// If separate bitmap was prepared for this view/camera pair then use it, draw untransformed
	// and blit transformed whole surface later.
	const bool draw_to_camsurf = CameraDrawData[view_index].Frame != nullptr;
	Bitmap *roomcam_surface = draw_to_camsurf ? CameraDrawData[view_index].Frame.get() : ds;
	{
		// For software renderer: copy dirty rects onto the virtual screen.
		// TODO: that would be SUPER NICE to reorganize the code and move this operation into SoftwareGraphicDriver somehow.
		// Because basically we duplicate sprite batch transform here.

		auto camera = view->GetCamera();
		set_invalidrects_cameraoffs(view_index, camera->GetRect().Left, camera->GetRect().Top);

		// TODO: (by CJ)
		// the following line takes up to 50% of the game CPU time at
		// high resolutions and colour depths - if we can optimise it
		// somehow, significant performance gains to be had
		update_room_invreg_and_reset(view_index, roomcam_surface, thisroom.BgFrames[play.bg_frame].Graphic.get(), draw_to_camsurf);
	}

	return CameraDrawData[view_index].Frame;
}


void draw_fps(const Rect &viewport) {
	// TODO: make allocated "fps struct" instead of using static vars!!
	static IDriverDependantBitmap *ddb = nullptr;
	static Bitmap *fpsDisplay = nullptr;
	const int font = FONT_NORMAL;
	if (fpsDisplay == nullptr) {
		fpsDisplay = BitmapHelper::CreateBitmap(viewport.GetWidth(), (getfontheight_outlined(font) + get_fixed_pixel_size(5)), game.GetColorDepth());
		fpsDisplay = ReplaceBitmapWithSupportedFormat(fpsDisplay);
	}
	fpsDisplay->ClearTransparent();

	color_t text_color = fpsDisplay->GetCompatibleColor(14);

	char base_buffer[20];
	if (!isTimerFpsMaxed()) {
		sprintf(base_buffer, "%d", frames_per_second);
	} else {
		sprintf(base_buffer, "unlimited");
	}

	char fps_buffer[60];
	// Don't display fps if we don't have enough information (because loop count was just reset)
	if (!std::isnan(fps)) {
		snprintf(fps_buffer, sizeof(fps_buffer), "FPS: %2.1f / %s", fps, base_buffer);
	} else {
		snprintf(fps_buffer, sizeof(fps_buffer), "FPS: --.- / %s", base_buffer);
	}
	wouttext_outline(fpsDisplay, 1, 1, font, text_color, fps_buffer);

	char loop_buffer[60];
	sprintf(loop_buffer, "Loop %u", loopcounter);
	wouttext_outline(fpsDisplay, viewport.GetWidth() / 2, 1, font, text_color, loop_buffer);

	if (ddb)
		gfxDriver->UpdateDDBFromBitmap(ddb, fpsDisplay, false);
	else
		ddb = gfxDriver->CreateDDBFromBitmap(fpsDisplay, false);
	int yp = viewport.GetHeight() - fpsDisplay->GetHeight();
	gfxDriver->DrawSprite(1, yp, ddb);
	invalidate_sprite(1, yp, ddb, false);
}

// Draw GUI and overlays of all kinds, anything outside the room space
void draw_gui_and_overlays() {
	if (pl_any_want_hook(AGSE_PREGUIDRAW))
		add_thing_to_draw(nullptr, AGSE_PREGUIDRAW, 0, TRANS_RUN_PLUGIN, false);

	// draw overlays, except text boxes and portraits
	for (const auto &over : screenover) {
		// complete overlay draw in non-transparent mode
		if (over.type == OVER_COMPLETE)
			add_thing_to_draw(over.bmp, over.x, over.y, TRANS_OPAQUE, false);
		else if (over.type != OVER_TEXTMSG && over.type != OVER_PICTURE) {
			int tdxp, tdyp;
			get_overlay_position(over, &tdxp, &tdyp);
			add_thing_to_draw(over.bmp, tdxp, tdyp, 0, over.hasAlphaChannel);
		}
	}

	// Draw GUIs - they should always be on top of overlays like
	// speech background text
	our_eip = 35;
	if (((debug_flags & DBG_NOIFACE) == 0) && (displayed_room >= 0)) {
		int aa;

		if (playerchar->activeinv >= MAX_INV) {
			quit("!The player.activeinv variable has been corrupted, probably as a result\n"
			     "of an incorrect assignment in the game script.");
		}
		if (playerchar->activeinv < 1) gui_inv_pic = -1;
		else gui_inv_pic = game.invinfo[playerchar->activeinv].pic;
		our_eip = 37;
		if (guis_need_update) {
			guis_need_update = 0;
			for (aa = 0; aa < game.numgui; aa++) {
				if (!guis[aa].IsDisplayed()) continue;

				if (guibg[aa] == nullptr)
					recreate_guibg_image(&guis[aa]);

				eip_guinum = aa;
				our_eip = 370;
				guibg[aa]->ClearTransparent();
				our_eip = 372;
				guis[aa].DrawAt(guibg[aa], 0, 0);
				our_eip = 373;

				bool isAlpha = false;
				if (guis[aa].HasAlphaChannel()) {
					isAlpha = true;

					if ((game.options[OPT_NEWGUIALPHA] == kGuiAlphaRender_Legacy) && (guis[aa].BgImage > 0)) {
						// old-style (pre-3.0.2) GUI alpha rendering
						repair_alpha_channel(guibg[aa], spriteset[guis[aa].BgImage]);
					}
				}

				if (guibgbmp[aa] != nullptr) {
					gfxDriver->UpdateDDBFromBitmap(guibgbmp[aa], guibg[aa], isAlpha);
				} else {
					guibgbmp[aa] = gfxDriver->CreateDDBFromBitmap(guibg[aa], isAlpha);
				}
				our_eip = 374;
			}
		}
		our_eip = 38;
		// Draw the GUIs
		for (int gg = 0; gg < game.numgui; gg++) {
			aa = play.gui_draw_order[gg];
			if (!guis[aa].IsDisplayed()) continue;

			// Don't draw GUI if "GUIs Turn Off When Disabled"
			if ((game.options[OPT_DISABLEOFF] == 3) &&
			        (all_buttons_disabled > 0) &&
			        (guis[aa].PopupStyle != kGUIPopupNoAutoRemove))
				continue;

			add_thing_to_draw(guibgbmp[aa], guis[aa].X, guis[aa].Y, guis[aa].Transparency, guis[aa].HasAlphaChannel());

			// only poll if the interface is enabled (mouseovers should not
			// work while in Wait state)
			if (IsInterfaceEnabled())
				guis[aa].Poll();
		}
	}

	// draw speech and portraits (so that they appear over GUIs)
	for (const auto &over : screenover) {
		if (over.type == OVER_TEXTMSG || over.type == OVER_PICTURE) {
			int tdxp, tdyp;
			get_overlay_position(over, &tdxp, &tdyp);
			add_thing_to_draw(over.bmp, tdxp, tdyp, 0, false);
		}
	}

	our_eip = 1099;
}

// Push the gathered list of sprites into the active graphic renderer
void put_sprite_list_on_screen(bool in_room) {
	// *** Draw the Things To Draw List ***

	SpriteListEntry *thisThing;

	for (size_t i = 0; i < thingsToDrawList.size(); ++i) {
		thisThing = &thingsToDrawList[i];

		if (thisThing->bmp != nullptr) {
			// mark the image's region as dirty
			invalidate_sprite(thisThing->x, thisThing->y, thisThing->bmp, in_room);
		} else if ((thisThing->transparent != TRANS_RUN_PLUGIN) &&
		           (thisThing->bmp == nullptr)) {
			quit("Null pointer added to draw list");
		}

		if (thisThing->bmp != nullptr) {
			if (thisThing->transparent <= 255) {
				thisThing->bmp->SetTransparency(thisThing->transparent);
			}

			gfxDriver->DrawSprite(thisThing->x, thisThing->y, thisThing->bmp);
		} else if (thisThing->transparent == TRANS_RUN_PLUGIN) {
			// meta entry to run the plugin hook
			gfxDriver->DrawSprite(thisThing->x, thisThing->y, nullptr);
		} else
			quit("Unknown entry in draw list");
	}

	our_eip = 1100;
}

bool GfxDriverNullSpriteCallback(int x, int y) {
	if (displayed_room < 0) {
		// if no room loaded, various stuff won't be initialized yet
		return 1;
	}
	return (pl_run_plugin_hooks(x, y) != 0);
}

void GfxDriverOnInitCallback(void *data) {
	pl_run_plugin_init_gfx_hooks(gfxDriver->GetDriverID(), data);
}

// Schedule room rendering: background, objects, characters
static void construct_room_view() {
	draw_preroom_background();
	prepare_room_sprites();
	// reset the Baselines Changed flag now that we've drawn stuff
	walk_behind_baselines_changed = 0;

	for (const auto &viewport : play.GetRoomViewportsZOrdered()) {
		if (!viewport->IsVisible())
			continue;
		auto camera = viewport->GetCamera();
		if (!camera)
			continue;
		const Rect &view_rc = play.GetRoomViewportAbs(viewport->GetID());
		const Rect &cam_rc = camera->GetRect();
		SpriteTransform room_trans(-cam_rc.Left, -cam_rc.Top,
		                           (float)view_rc.GetWidth() / (float)cam_rc.GetWidth(),
		                           (float)view_rc.GetHeight() / (float)cam_rc.GetHeight(),
		                           0.f);
		if (gfxDriver->RequiresFullRedrawEachFrame()) {
			// we draw everything as a sprite stack
			gfxDriver->BeginSpriteBatch(view_rc, room_trans, Point(0, play.shake_screen_yoff), (GlobalFlipType)play.screen_flipped);
		} else {
			if (CameraDrawData[viewport->GetID()].Frame == nullptr && CameraDrawData[viewport->GetID()].IsOverlap) {
				// room background is prepended to the sprite stack
				// TODO: here's why we have blit whole piece of background now:
				// if we draw directly to the virtual screen overlapping another
				// viewport, then we'd have to also mark and repaint every our
				// region located directly over their dirty regions. That would
				// require to update regions up the stack, converting their
				// coordinates (cam1 -> screen -> cam2).
				// It's not clear whether this is worth the effort, but if it is,
				// then we'd need to optimise view/cam data first.
				gfxDriver->BeginSpriteBatch(view_rc, room_trans);
				gfxDriver->DrawSprite(0, 0, roomBackgroundBmp);
			} else {
				// room background is drawn by dirty rects system
				PBitmap bg_surface = draw_room_background(viewport.get(), room_trans);
				gfxDriver->BeginSpriteBatch(view_rc, room_trans, Point(), kFlip_None, bg_surface);
			}
		}
		put_sprite_list_on_screen(true);
	}

	clear_draw_list();
}

// Schedule ui rendering
static void construct_ui_view() {
	gfxDriver->BeginSpriteBatch(play.GetUIViewportAbs(), SpriteTransform(), Point(0, play.shake_screen_yoff), (GlobalFlipType)play.screen_flipped);
	draw_gui_and_overlays();
	put_sprite_list_on_screen(false);
	clear_draw_list();
}

void construct_game_scene(bool full_redraw) {
	gfxDriver->ClearDrawLists();

	if (play.fast_forward)
		return;

	our_eip = 3;

	// React to changes to viewports and cameras (possibly from script) just before the render
	play.UpdateViewports();

	gfxDriver->UseSmoothScaling(IS_ANTIALIAS_SPRITES);
	gfxDriver->RenderSpritesAtScreenResolution(usetup.RenderAtScreenRes, usetup.Supersampling);

	pl_run_plugin_hooks(AGSE_PRERENDER, 0);

	// Possible reasons to invalidate whole screen for the software renderer
	if (full_redraw || play.screen_tint > 0 || play.shakesc_length > 0)
		invalidate_screen();

	// TODO: move to game update! don't call update during rendering pass!
	// IMPORTANT: keep the order same because sometimes script may depend on it
	if (displayed_room >= 0)
		play.UpdateRoomCameras();

	// Stage: room viewports
	if (play.screen_is_faded_out == 0 && is_complete_overlay == 0) {
		if (displayed_room >= 0) {
			construct_room_view();
			update_polled_mp3();
		} else if (!gfxDriver->RequiresFullRedrawEachFrame()) {
			// black it out so we don't get cursor trails
			// TODO: this is possible to do with dirty rects system now too (it can paint black rects outside of room viewport)
			gfxDriver->GetMemoryBackBuffer()->Fill(0);
		}
	}

	our_eip = 4;

	// Stage: UI overlay
	if (play.screen_is_faded_out == 0) {
		construct_ui_view();
	}
}

void construct_game_screen_overlay(bool draw_mouse) {
	gfxDriver->BeginSpriteBatch(play.GetMainViewport(), SpriteTransform(), Point(0, play.shake_screen_yoff), (GlobalFlipType)play.screen_flipped);
	if (pl_any_want_hook(AGSE_POSTSCREENDRAW))
		gfxDriver->DrawSprite(AGSE_POSTSCREENDRAW, 0, nullptr);

	// TODO: find out if it's okay to move cursor animation and state update
	// to the update loop instead of doing it in the drawing routine
	// update animating mouse cursor
	if (game.mcurs[cur_cursor].view >= 0) {
		ags_domouse(DOMOUSE_NOCURSOR);
		// only on mousemove, and it's not moving
		if (((game.mcurs[cur_cursor].flags & MCF_ANIMMOVE) != 0) &&
		        (mousex == lastmx) && (mousey == lastmy));
		// only on hotspot, and it's not on one
		else if (((game.mcurs[cur_cursor].flags & MCF_HOTSPOT) != 0) &&
		         (GetLocationType(game_to_data_coord(mousex), game_to_data_coord(mousey)) == 0))
			set_new_cursor_graphic(game.mcurs[cur_cursor].pic);
		else if (mouse_delay > 0) mouse_delay--;
		else {
			int viewnum = game.mcurs[cur_cursor].view;
			int loopnum = 0;
			if (loopnum >= views[viewnum].numLoops)
				quitprintf("An animating mouse cursor is using view %d which has no loops", viewnum + 1);
			if (views[viewnum].loops[loopnum].numFrames < 1)
				quitprintf("An animating mouse cursor is using view %d which has no frames in loop %d", viewnum + 1, loopnum);

			mouse_frame++;
			if (mouse_frame >= views[viewnum].loops[loopnum].numFrames)
				mouse_frame = 0;
			set_new_cursor_graphic(views[viewnum].loops[loopnum].frames[mouse_frame].pic);
			mouse_delay = views[viewnum].loops[loopnum].frames[mouse_frame].speed + 5;
			CheckViewFrame(viewnum, loopnum, mouse_frame);
		}
		lastmx = mousex;
		lastmy = mousey;
	}

	ags_domouse(DOMOUSE_NOCURSOR);

	// Stage: mouse cursor
	if (draw_mouse && !play.mouse_cursor_hidden && play.screen_is_faded_out == 0) {
		gfxDriver->DrawSprite(mousex - hotx, mousey - hoty, mouseCursor);
		invalidate_sprite(mousex - hotx, mousey - hoty, mouseCursor, false);
	}

	if (play.screen_is_faded_out == 0) {
		// Stage: screen fx
		if (play.screen_tint >= 1)
			gfxDriver->SetScreenTint(play.screen_tint & 0xff, (play.screen_tint >> 8) & 0xff, (play.screen_tint >> 16) & 0xff);
		// Stage: legacy letterbox mode borders
		render_black_borders();
	}

	if (play.screen_is_faded_out != 0 && gfxDriver->RequiresFullRedrawEachFrame()) {
		const Rect &main_viewport = play.GetMainViewport();
		gfxDriver->BeginSpriteBatch(main_viewport, SpriteTransform());
		gfxDriver->SetScreenFade(play.fade_to_red, play.fade_to_green, play.fade_to_blue);
	}
}

void construct_engine_overlay() {
	const Rect &viewport = RectWH(game.GetGameRes());
	gfxDriver->BeginSpriteBatch(viewport, SpriteTransform());

	// draw the debug console, if appropriate
	if ((play.debug_mode > 0) && (display_console != 0)) {
		const int font = FONT_NORMAL;
		int ypp = 1;
		int txtspacing = getfontspacing_outlined(font);
		int barheight = getheightoflines(font, DEBUG_CONSOLE_NUMLINES - 1) + 4;

		if (debugConsoleBuffer == nullptr) {
			debugConsoleBuffer = BitmapHelper::CreateBitmap(viewport.GetWidth(), barheight, game.GetColorDepth());
			debugConsoleBuffer = ReplaceBitmapWithSupportedFormat(debugConsoleBuffer);
		}

		color_t draw_color = debugConsoleBuffer->GetCompatibleColor(15);
		debugConsoleBuffer->FillRect(Rect(0, 0, viewport.GetWidth() - 1, barheight), draw_color);
		color_t text_color = debugConsoleBuffer->GetCompatibleColor(16);
		for (int jj = first_debug_line; jj != last_debug_line; jj = (jj + 1) % DEBUG_CONSOLE_NUMLINES) {
			wouttextxy(debugConsoleBuffer, 1, ypp, font, text_color, debug_line[jj]);
			ypp += txtspacing;
		}

		if (debugConsole == nullptr)
			debugConsole = gfxDriver->CreateDDBFromBitmap(debugConsoleBuffer, false, true);
		else
			gfxDriver->UpdateDDBFromBitmap(debugConsole, debugConsoleBuffer, false);

		gfxDriver->DrawSprite(0, 0, debugConsole);
		invalidate_sprite(0, 0, debugConsole, false);
	}

	if (display_fps != kFPS_Hide)
		draw_fps(viewport);
}

static void update_shakescreen() {
	// TODO: unify blocking and non-blocking shake update
	play.shake_screen_yoff = 0;
	if (play.shakesc_length > 0) {
		if ((loopcounter % play.shakesc_delay) < (play.shakesc_delay / 2))
			play.shake_screen_yoff = play.shakesc_amount;
	}
}

// Draw everything
void render_graphics(IDriverDependantBitmap *extraBitmap, int extraX, int extraY) {
	// Don't render if skipping cutscene
	if (play.fast_forward)
		return;
	// Don't render if we've just entered new room and are before fade-in
	// TODO: find out why this is not skipped for 8-bit games
	if ((in_new_room > 0) & (game.color_depth > 1))
		return;

	// TODO: find out if it's okay to move shake to update function
	update_shakescreen();

	construct_game_scene(false);
	our_eip = 5;
	// NOTE: extraBitmap will always be drawn with the UI render stage
	if (extraBitmap != nullptr) {
		invalidate_sprite(extraX, extraY, extraBitmap, false);
		gfxDriver->DrawSprite(extraX, extraY, extraBitmap);
	}
	construct_game_screen_overlay(true);
	render_to_screen();

	if (!play.screen_is_faded_out) {
		// always update the palette, regardless of whether the plugin
		// vetos the screen update
		if (bg_just_changed) {
			setpal();
			bg_just_changed = 0;
		}
	}

	screen_is_dirty = false;
}

} // namespace AGS3
