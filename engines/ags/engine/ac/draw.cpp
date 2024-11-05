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

#include "common/std/algorithm.h"
#include "ags/lib/aastr-0.1.1/aastr.h"
#include "ags/shared/core/platform.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/util/compress.h"
#include "ags/shared/util/wgt2_allg.h"
#include "ags/shared/ac/view.h"
#include "ags/engine/ac/character_extras.h"
#include "ags/shared/ac/character_info.h"
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/draw_software.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_gui.h"
#include "ags/engine/ac/global_region.h"
#include "ags/engine/ac/gui.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/move_list.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/room_object.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/screen_overlay.h"
#include "ags/engine/ac/sprite.h"
#include "ags/engine/ac/sprite_list_entry.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/ac/view_frame.h"
#include "ags/engine/ac/walkable_area.h"
#include "ags/engine/ac/dynobj/script_system.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/gui/gui_object.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/plugins/ags_plugin_evts.h"
#include "ags/plugins/plugin_engine.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/gfx/gfx_util.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/engine/gfx/blender.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/ags.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;


ObjTexture::ObjTexture(ObjTexture &&o) {
	*this = std::move(o);
}

ObjTexture::~ObjTexture() {
	Bmp.reset();
	if (Ddb) {
		assert(_G(gfxDriver));
		_G(gfxDriver)->DestroyDDB(Ddb);
	}
}

ObjTexture &ObjTexture::operator=(ObjTexture &&o) {
	SpriteID = o.SpriteID;
	if (Ddb) {
		assert(_G(gfxDriver));
		_G(gfxDriver)->DestroyDDB(Ddb);
	}
	Bmp = std::move(o.Bmp);
	Ddb = o.Ddb;
	o.Ddb = nullptr;
	Pos = o.Pos;
	Off = o.Off;
	return *this;
}


void setpal() {
	set_palette_range(_G(palette), 0, 255, 0);
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

// NOTE: Some of these conversions are required even when using
// D3D and OpenGL rendering, for two reasons:
// 1) certain raw drawing operations are still performed by software
// Allegro methods, hence bitmaps should be kept compatible to any native
// software operations, such as blitting two bitmaps of different formats.
// 2) OpenGL renderer assumes native bitmaps are in OpenGL-compatible format,
// so that it could copy them to texture without additional changes.
//
// TODO: make _G(gfxDriver)->GetCompatibleBitmapFormat describe all necessary
// conversions, so that we did not have to guess.
//
Bitmap *AdjustBitmapForUseWithDisplayMode(Bitmap *bitmap, bool has_alpha) {
	const int bmp_col_depth = bitmap->GetColorDepth();
	const int game_col_depth = _GP(game).GetColorDepth();
	const int compat_col_depth = _G(gfxDriver)->GetCompatibleBitmapFormat(game_col_depth);

	const bool must_switch_palette = bitmap->GetColorDepth() == 8 && game_col_depth > 8;
	if (must_switch_palette)
		select_palette(_G(palette));

	Bitmap *new_bitmap = bitmap;

	//
	// The only special case when bitmap needs to be prepared for graphics driver
	//
	// In 32-bit display mode, 32-bit bitmaps may require component conversion
	// to match graphics driver expectation about pixel format.
	// TODO: make GetCompatibleBitmapFormat tell this somehow
#if defined (AGS_INVERTED_COLOR_ORDER)
	const int sys_col_depth = _G(gfxDriver)->GetDisplayMode().ColorDepth;
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
			BitmapHelper::ReplaceAlphaWithRGBMask(new_bitmap);
	}
	// In 32-bit game hicolor bitmaps must be converted to the true color
	else if (game_col_depth == 32 && (bmp_col_depth > 8 && bmp_col_depth <= 16)) {
		new_bitmap = BitmapHelper::CreateBitmapCopy(bitmap, compat_col_depth);
	}
	// In non-32-bit game truecolor bitmaps must be downgraded
	else if (game_col_depth <= 16 && bmp_col_depth > 16) {
		if (has_alpha) // if has valid alpha channel, convert it to regular transparency mask
			new_bitmap = remove_alpha_channel(bitmap);
		else // else simply convert bitmap
			new_bitmap = BitmapHelper::CreateBitmapCopy(bitmap, compat_col_depth);
	}

	// Finally, if we did not create a new copy already, - convert to driver compatible format
	if (new_bitmap == bitmap)
		new_bitmap = GfxUtil::ConvertBitmap(bitmap, _G(gfxDriver)->GetCompatibleBitmapFormat(bitmap->GetColorDepth()));

	if (must_switch_palette)
		unselect_palette();

	return new_bitmap;
}

Bitmap *CreateCompatBitmap(int width, int height, int col_depth) {
	return new Bitmap(width, height,
		_G(gfxDriver)->GetCompatibleBitmapFormat(col_depth == 0 ? _GP(game).GetColorDepth() : col_depth));
}

Bitmap *ReplaceBitmapWithSupportedFormat(Bitmap *bitmap) {
	return GfxUtil::ConvertBitmap(bitmap, _G(gfxDriver)->GetCompatibleBitmapFormat(bitmap->GetColorDepth()));
}

Bitmap *PrepareSpriteForUse(Bitmap *bitmap, bool has_alpha) {
	Bitmap *new_bitmap = AdjustBitmapForUseWithDisplayMode(bitmap, has_alpha);
	if (new_bitmap != bitmap)
		delete bitmap;
	return new_bitmap;
}

PBitmap PrepareSpriteForUse(PBitmap bitmap, bool has_alpha) {
	Bitmap *new_bitmap = AdjustBitmapForUseWithDisplayMode(bitmap.get(), has_alpha);
	return new_bitmap == bitmap.get() ? bitmap : PBitmap(new_bitmap); // if bitmap is same, don't create new smart ptr!
}

Bitmap *CopyScreenIntoBitmap(int width, int height, const Rect *src_rect, bool at_native_res, uint32_t batch_skip_filter) {
	Bitmap *dst = new Bitmap(width, height, _GP(game).GetColorDepth());
	GraphicResolution want_fmt;
	// If the size and color depth are supported, then we may copy right into our final bitmap
	if (_G(gfxDriver)->GetCopyOfScreenIntoBitmap(dst, src_rect, at_native_res, &want_fmt, batch_skip_filter))
		return dst;

	// Otherwise we might need to copy between few bitmaps...
	// Get screenshot in the suitable format
	std::unique_ptr<Bitmap> buf_screenfmt(new Bitmap(want_fmt.Width, want_fmt.Height, want_fmt.ColorDepth));
	_G(gfxDriver)->GetCopyOfScreenIntoBitmap(buf_screenfmt.get(), src_rect, at_native_res, nullptr, batch_skip_filter);
	// If color depth does not match, and we must stretch-blit, then we need another helper bmp,
	// because Allegro does not support stretching with mismatching color depths
	std::unique_ptr<Bitmap> buf_fixdepth;
	Bitmap *blit_from = buf_screenfmt.get();
	if ((dst->GetSize() != blit_from->GetSize()) && (want_fmt.ColorDepth != _GP(game).GetColorDepth())) {
		buf_fixdepth.reset(new Bitmap(want_fmt.Width, want_fmt.Height, _GP(game).GetColorDepth()));
		buf_fixdepth->Blit(buf_screenfmt.get());
		blit_from = buf_fixdepth.get();
	}
	// Now either blit or stretch-blit
	if (dst->GetSize() == blit_from->GetSize()) {
		dst->Blit(blit_from);
	} else {
		dst->StretchBlt(blit_from, RectWH(dst->GetSize()));
	}
	return dst;
}


// Begin resolution system functions

// Multiplies up the number of pixels depending on the current
// resolution, to give a relatively fixed size at any game res
int get_fixed_pixel_size(int pixels) {
	return pixels * _GP(game).GetRelativeUIMult();
}

int data_to_game_coord(int coord) {
	return coord * _GP(game).GetDataUpscaleMult();
}

void data_to_game_coords(int *x, int *y) {
	const int mul = _GP(game).GetDataUpscaleMult();
	x[0] *= mul;
	y[0] *= mul;
}

void data_to_game_round_up(int *x, int *y) {
	const int mul = _GP(game).GetDataUpscaleMult();
	x[0] = x[0] * mul + (mul - 1);
	y[0] = y[0] * mul + (mul - 1);
}

int game_to_data_coord(int coord) {
	return coord / _GP(game).GetDataUpscaleMult();
}

void game_to_data_coords(int &x, int &y) {
	const int mul = _GP(game).GetDataUpscaleMult();
	x /= mul;
	y /= mul;
}

int game_to_data_round_up(int coord) {
	const int mul = _GP(game).GetDataUpscaleMult();
	return (coord / mul) + (mul - 1);
}

void ctx_data_to_game_coord(int &x, int &y, bool hires_ctx) {
	if (hires_ctx && !_GP(game).IsLegacyHiRes()) {
		x /= HIRES_COORD_MULTIPLIER;
		y /= HIRES_COORD_MULTIPLIER;
	} else if (!hires_ctx && _GP(game).IsLegacyHiRes()) {
		x *= HIRES_COORD_MULTIPLIER;
		y *= HIRES_COORD_MULTIPLIER;
	}
}

void ctx_data_to_game_size(int &w, int &h, bool hires_ctx) {
	if (hires_ctx && !_GP(game).IsLegacyHiRes()) {
		w = MAX(1, (w / HIRES_COORD_MULTIPLIER));
		h = MAX(1, (h / HIRES_COORD_MULTIPLIER));
	} else if (!hires_ctx && _GP(game).IsLegacyHiRes()) {
		w *= HIRES_COORD_MULTIPLIER;
		h *= HIRES_COORD_MULTIPLIER;
	}
}

int ctx_data_to_game_size(int size, bool hires_ctx) {
	if (hires_ctx && !_GP(game).IsLegacyHiRes())
		return MAX(1, (size / HIRES_COORD_MULTIPLIER));
	if (!hires_ctx && _GP(game).IsLegacyHiRes())
		return size * HIRES_COORD_MULTIPLIER;
	return size;
}

int game_to_ctx_data_size(int size, bool hires_ctx) {
	if (hires_ctx && !_GP(game).IsLegacyHiRes())
		return size * HIRES_COORD_MULTIPLIER;
	else if (!hires_ctx && _GP(game).IsLegacyHiRes())
		return MAX(1, (size / HIRES_COORD_MULTIPLIER));
	return size;
}

void defgame_to_finalgame_coords(int &x, int &y) {
	// Note we support only upscale now
	x *= _GP(game).GetScreenUpscaleMult();
	y *= _GP(game).GetScreenUpscaleMult();
}

// End resolution system functions

// Create blank (black) images used to repaint borders around game frame
void create_blank_image(int coldepth) {
	// this is the first time that we try to use the graphics driver,
	// so it's the most likey place for a crash
	//try
	//{
	Bitmap *blank = CreateCompatBitmap(16, 16, coldepth);
	blank->Clear();
	_G(blankImage) = _G(gfxDriver)->CreateDDBFromBitmap(blank, false, true);
	_G(blankSidebarImage) = _G(gfxDriver)->CreateDDBFromBitmap(blank, false, true);
	delete blank;
	/*}
	catch (Ali3DException gfxException)
	{
		quit(gfxException.Message.GetCStr());
	}*/
}

void destroy_blank_image() {
	if (_G(blankImage))
		_G(gfxDriver)->DestroyDDB(_G(blankImage));
	if (_G(blankSidebarImage))
		_G(gfxDriver)->DestroyDDB(_G(blankSidebarImage));
	_G(blankImage) = nullptr;
	_G(blankSidebarImage) = nullptr;
}

int MakeColor(int color_index) {
	color_t real_color = 0;
	__my_setcolor(&real_color, color_index, _GP(game).GetColorDepth());
	return real_color;
}

void init_draw_method() {
	_G(drawstate).SoftwareRender = !_G(gfxDriver)->HasAcceleratedTransform();
	_G(drawstate).FullFrameRedraw = _G(gfxDriver)->RequiresFullRedrawEachFrame();

	if (_G(drawstate).SoftwareRender) {
		_G(drawstate).SoftwareRender = true;
		_G(drawstate).WalkBehindMethod = DrawOverCharSprite;
	} else {
		_G(drawstate).WalkBehindMethod = DrawAsSeparateSprite;
		create_blank_image(_GP(game).GetColorDepth());
		size_t tx_cache_size = _GP(usetup).TextureCacheSize * 1024;
		// If graphics driver can report available texture memory,
		// then limit the setting by, let's say, 66% of it (we use it for other things)
		size_t avail_tx_mem = _G(gfxDriver)->GetAvailableTextureMemory();
		if (avail_tx_mem > 0)
			tx_cache_size = std::min<size_t>(tx_cache_size, avail_tx_mem * 0.66);
		// texturecache.SetMaxCacheSize(tx_cache_size);
	}

	on_mainviewport_changed();
	init_room_drawdata();
	if (_G(gfxDriver)->UsesMemoryBackBuffer())
		_G(gfxDriver)->GetMemoryBackBuffer()->Clear();
}

void dispose_draw_method() {
	dispose_room_drawdata();
	dispose_invalid_regions(false);
	destroy_blank_image();
}

void init_game_drawdata() {
	// character and object caches
	_GP(charcache).resize(_GP(game).numcharacters);

	for (int i = 0; i < MAX_ROOM_OBJECTS; ++i)
		_G(objcache)[i] = ObjectCache();

	size_t actsps_num = _GP(game).numcharacters + MAX_ROOM_OBJECTS;
	_GP(actsps).resize(actsps_num);

	_GP(guibg).resize(_GP(game).numgui);

	size_t guio_num = 0;
	// Prepare GUI cache lists and build the quick reference for controls cache
	_GP(guiobjddbref).resize(_GP(game).numgui);
	for (const auto &gui : _GP(guis)) {
		_GP(guiobjddbref)[gui.ID] = guio_num;
		guio_num += gui.GetControlCount();
	}
	_GP(guiobjbg).resize(guio_num);
}

extern void dispose_engine_overlay();

void dispose_game_drawdata() {
	clear_drawobj_cache();

	_GP(charcache).clear();
	_GP(actsps).clear();
	_GP(walkbehindobj).clear();

	_GP(guibg).clear();
	_GP(guiobjbg).clear();
	_GP(guiobjddbref).clear();

	dispose_engine_overlay();
}

static void dispose_debug_room_drawdata() {
	_GP(debugRoomMaskObj) = ObjTexture();
	_GP(debugMoveListObj) = ObjTexture();
}

void dispose_room_drawdata() {
	_GP(CameraDrawData).clear();
	dispose_invalid_regions(true);
}

void clear_drawobj_cache() {
	// clear the character cache
	for (auto &cc : _GP(charcache)) {
		cc = ObjectCache();
	}

	// clear the object cache
	for (int i = 0; i < MAX_ROOM_OBJECTS; ++i) {
		_G(objcache)[i] = ObjectCache();
	}

	// room overlays cache
	_GP(overcache).clear();

	// cleanup Character + Room object textures
	for (auto &o : _GP(actsps)) o = ObjTexture();
	for (auto &o : _GP(walkbehindobj)) o = ObjTexture();
	// cleanup GUI and controls textures
	for (auto &o : _GP(guibg)) o = ObjTexture();
	for (auto &o : _GP(guiobjbg)) o = ObjTexture();
	_GP(overtxs).clear();

	// Clear sprite update notification blocks
	_G(drawstate).SpriteNotifyMap.clear();

	dispose_debug_room_drawdata();
}

void on_mainviewport_changed() {
	if (!_G(drawstate).FullFrameRedraw) {
		const auto &view = _GP(play).GetMainViewport();
		set_invalidrects_globaloffs(view.Left, view.Top);
		// the black background region covers whole game screen
		init_invalid_regions(-1, _GP(game).GetGameRes(), RectWH(_GP(game).GetGameRes()));
		if (_GP(game).GetGameRes().ExceedsByAny(view.GetSize()))
			clear_letterbox_borders();
	}
}

// Allocates a bitmap for rendering camera/viewport pair (software render mode)
void prepare_roomview_frame(Viewport *view) {
	if (!view->GetCamera()) return; // no camera link
	const int view_index = view->GetID();
	const Size view_sz = view->GetRect().GetSize();
	const Size cam_sz = view->GetCamera()->GetRect().GetSize();
	RoomCameraDrawData &draw_dat = _GP(CameraDrawData)[view_index];
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
			int room_width = data_to_game_coord(_GP(thisroom).Width);
			int room_height = data_to_game_coord(_GP(thisroom).Height);
			Size alloc_sz = Size::Clamp(cam_sz * 2, Size(1, 1), Size(room_width, room_height));
			camera_buffer.reset(new Bitmap(alloc_sz.Width, alloc_sz.Height, _G(gfxDriver)->GetMemoryBackBuffer()->GetColorDepth()));
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
	// Note the dirty regions' viewport is found using absolute offset on game screen
	init_invalid_regions(view->GetID(),
		view->GetCamera()->GetRect().GetSize(),
		_GP(play).GetRoomViewportAbs(view->GetID()));
	prepare_roomview_frame(view);
}

void init_room_drawdata() {
	if (_G(displayed_room) < 0)
		return; // not loaded yet

	if (_G(drawstate).WalkBehindMethod == DrawAsSeparateSprite) {
		walkbehinds_generate_sprites();
	}
	// Update debug overlays, if any were on
	debug_draw_room_mask(_G(debugRoomMask));
	debug_draw_movelist(_G(debugMoveListChar));

	// Following data is only updated for software renderer
	if (_G(drawstate).FullFrameRedraw)
		return;
	// Make sure all frame buffers are created for software drawing
	int view_count = _GP(play).GetRoomViewportCount();
	_GP(CameraDrawData).resize(view_count);
	for (int i = 0; i < _GP(play).GetRoomViewportCount(); ++i)
		sync_roomview(_GP(play).GetRoomViewport(i).get());
}

void on_roomviewport_created(int index) {
	if (_G(drawstate).FullFrameRedraw || (_G(displayed_room) < 0))
		return;
	if ((size_t)index < _GP(CameraDrawData).size())
		return;
	_GP(CameraDrawData).resize(index + 1);
}

void on_roomviewport_deleted(int index) {
	if (_G(drawstate).FullFrameRedraw || (_G(displayed_room) < 0))
		return;
	_GP(CameraDrawData).erase(_GP(CameraDrawData).begin() + index);
	delete_invalid_regions(index);
}

void on_roomviewport_changed(Viewport *view) {
	if (_G(drawstate).FullFrameRedraw || (_G(displayed_room) < 0))
		return;
	if (!view->IsVisible() || view->GetCamera() == nullptr)
		return;
	const bool off = !IsRectInsideRect(RectWH(_G(gfxDriver)->GetMemoryBackBuffer()->GetSize()), view->GetRect());
	const bool off_changed = off != _GP(CameraDrawData)[view->GetID()].IsOffscreen;
	_GP(CameraDrawData)[view->GetID()].IsOffscreen = off;
	if (view->HasChangedSize())
		sync_roomview(view);
	else if (off_changed)
		prepare_roomview_frame(view);
	// TODO: don't have to do this all the time, perhaps do "dirty rect" method
	// and only clear previous viewport location?
	invalidate_screen();
	_G(gfxDriver)->GetMemoryBackBuffer()->Clear();
}

void detect_roomviewport_overlaps(size_t z_index) {
	if (_G(drawstate).FullFrameRedraw || (_G(displayed_room) < 0))
		return;
	// Find out if we overlap or are overlapped by anything;
	const auto &viewports = _GP(play).GetRoomViewportsZOrdered();
	for (; z_index < viewports.size(); ++z_index) {
		auto this_view = viewports[z_index];
		const int this_id = this_view->GetID();
		bool is_overlap = false;
		if (!this_view->IsVisible()) continue;
		for (size_t z_index2 = 0; z_index2 < z_index; ++z_index2) {
			if (!viewports[z_index2]->IsVisible()) continue;
			if (AreRectsIntersecting(this_view->GetRect(), viewports[z_index2]->GetRect())) {
				is_overlap = true;
				break;
			}
		}
		if (_GP(CameraDrawData)[this_id].IsOverlap != is_overlap) {
			_GP(CameraDrawData)[this_id].IsOverlap = is_overlap;
			prepare_roomview_frame(this_view.get());
		}
	}
}

void on_roomcamera_changed(Camera *cam) {
	if (_G(drawstate).FullFrameRedraw || (_G(displayed_room) < 0))
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

void mark_object_changed(int objid) {
	_G(objcache)[objid].y = -9999;
}

void reset_drawobj_for_overlay(int objnum) {
	if (objnum > 0 && static_cast<size_t>(objnum) < _GP(overtxs).size()) {
		_GP(overtxs)[objnum] = ObjTexture();
		if (_G(drawstate).SoftwareRender)
			_GP(overcache)[objnum] = Point(INT32_MIN, INT32_MIN);
	}
}

void notify_sprite_changed(int sprnum, bool deleted) {
	assert(sprnum >= 0 && sprnum < (int)_GP(game).SpriteInfos.size());

	// software renderer
	// will need to know to redraw active cached sprite for objects.
	// We have this notification for both kinds of renderers though,
	// because it makes the code simpler, and also it makes it simpler to
	// notify texture-based ones in a specific case when a deleted sprite
	// was replaced by another of same ID.

	auto it_notify = _G(drawstate).SpriteNotifyMap.find(sprnum);
	if (it_notify != _G(drawstate).SpriteNotifyMap.end()) {
		*it_notify->_value = UINT32_MAX;
		_G(drawstate).SpriteNotifyMap.erase(sprnum);
	}
}

void mark_screen_dirty() {
	_G(drawstate).ScreenIsDirty = true;
}

bool is_screen_dirty() {
	return _G(drawstate).ScreenIsDirty;
}

void invalidate_screen() {
	invalidate_all_rects();
}

void invalidate_camera_frame(int index) {
	invalidate_all_camera_rects(index);
}

void invalidate_rect(int x1, int y1, int x2, int y2, bool in_room) {
	invalidate_rect_ds(x1, y1, x2, y2, in_room);
}

void invalidate_sprite(int x1, int y1, IDriverDependantBitmap *pic, bool in_room) {
	invalidate_rect_ds(x1, y1, x1 + pic->GetWidth(), y1 + pic->GetHeight(), in_room);
}

void invalidate_sprite_glob(int x1, int y1, IDriverDependantBitmap *pic) {
	invalidate_rect_global(x1, y1, x1 + pic->GetWidth(), y1 + pic->GetHeight());
}

void mark_current_background_dirty() {
	_G(current_background_is_dirty) = true;
}


void draw_and_invalidate_text(Bitmap *ds, int x1, int y1, int font, color_t text_color, const char *text) {
	wouttext_outline(ds, x1, y1, font, text_color, text);
	invalidate_rect(x1, y1, x1 + get_text_width_outlined(text, font),
		y1 + get_font_height_outlined(font) + get_fixed_pixel_size(1), false);
}

// Renders black borders for the legacy boxed game mode,
// where whole game screen changes size between large and small rooms
static void render_black_borders() {
	const Rect &viewport = _GP(play).GetMainViewport();
	if (viewport.Top > 0) {
		// letterbox borders
		_G(blankImage)->SetStretch(_GP(game).GetGameRes().Width, viewport.Top, false);
		_G(gfxDriver)->DrawSprite(0, 0, _G(blankImage));
		_G(gfxDriver)->DrawSprite(0, viewport.Bottom + 1, _G(blankImage));
	}
	if (viewport.Left > 0) {
		// sidebar borders for widescreen
		_G(blankSidebarImage)->SetStretch(viewport.Left, viewport.GetHeight(), false);
		_G(gfxDriver)->DrawSprite(0, 0, _G(blankSidebarImage));
		_G(gfxDriver)->DrawSprite(viewport.Right + 1, 0, _G(blankSidebarImage));
	}
}

void render_to_screen() {
	// Stage: final plugin callback (still drawn on game screen)
	if (pl_any_want_hook(AGSE_FINALSCREENDRAW)) {
		_G(gfxDriver)->BeginSpriteBatch(_GP(play).GetMainViewport(),
										_GP(play).GetGlobalTransform(_G(drawstate).FullFrameRedraw), (GraphicFlip)_GP(play).screen_flipped);
		_G(gfxDriver)->DrawSprite(AGSE_FINALSCREENDRAW, 0, nullptr);
		_G(gfxDriver)->EndSpriteBatch();
	}
	// Stage: engine overlay
	construct_engine_overlay();

	// Try set new vsync value, and remember the actual result
	if (isTimerFpsMaxed()) {
		_G(gfxDriver)->SetVsync(false);
	} else {
		bool new_vsync = _G(gfxDriver)->SetVsync(_GP(scsystem).vsync > 0);
		if (new_vsync != (_GP(scsystem).vsync != 0))
			System_SetVSyncInternal(new_vsync);
	}

	bool succeeded = false;
	while (!succeeded && !_G(want_exit) && !_G(abort_engine)) {
		//     try
		//     {
		if (_G(drawstate).FullFrameRedraw) {
			_G(gfxDriver)->Render();
		}
		else {
			// NOTE: the shake yoff and global flip here will only be used by a software renderer;
			// as hw renderers have these as transform parameters for the parent scene nodes.
			// This may be a matter for the future code improvement.
			//
			// For software renderer, need to blacken upper part of the game frame when shaking screen moves image down
			if (_GP(play).shake_screen_yoff > 0) {
				const Rect &viewport = _GP(play).GetMainViewport();
				_G(gfxDriver)->ClearRectangle(viewport.Left, viewport.Top, viewport.GetWidth() - 1, _GP(play).shake_screen_yoff, nullptr);
			}
			_G(gfxDriver)->Render(0, _GP(play).shake_screen_yoff, (GraphicFlip)_GP(play).screen_flipped);
		}

#if AGS_PLATFORM_OS_ANDROID
		if (_GP(game).color_depth == 1)
			android_render();
#elif AGS_PLATFORM_OS_IOS
		if (_GP(game).color_depth == 1)
			ios_render();
#endif

		succeeded = true;
		/*}
		catch (Ali3DFullscreenLostException e) {
			Debug::Printf("Renderer exception: %s", e.Message.GetCStr());
			do {
				sys_evt_process_pending();
				platform->Delay(300);
			} while (_G(game_update_suspend) && (!_G(want_exit)) && (!_G(abort_engine)));
		}*/
	}
}

// Blanks out borders around main viewport in case it became smaller (e.g. after loading another room)
void clear_letterbox_borders() {
	const Rect &viewport = _GP(play).GetMainViewport();
	_G(gfxDriver)->ClearRectangle(0, 0, _GP(game).GetGameRes().Width - 1, viewport.Top - 1, nullptr);
	_G(gfxDriver)->ClearRectangle(0, viewport.Bottom + 1, _GP(game).GetGameRes().Width - 1, _GP(game).GetGameRes().Height - 1, nullptr);
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

	if (_GP(game).options[OPT_SPRITEALPHA] == kSpriteAlphaRender_Proper) {
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
	draw_sprite_support_alpha(ds, ds_has_alpha, xpos, ypos, _GP(spriteset)[src_slot], (_GP(game).SpriteInfos[src_slot].Flags & SPF_ALPHACHANNEL) != 0,
	                          blend_mode, alpha);
}

Engine::IDriverDependantBitmap* recycle_ddb_sprite(Engine::IDriverDependantBitmap *ddb, uint32_t sprite_id, Shared::Bitmap *source, bool has_alpha, bool opaque) {
	// no ddb, - get or create shared object
	if (!ddb)
		return _G(gfxDriver)->GetSharedDDB(sprite_id, source, has_alpha, opaque);
	// same sprite id, - use existing
	if ((sprite_id != UINT32_MAX) && (ddb->GetRefID() == sprite_id))
		return ddb;
	// not related to a sprite ID, but has same resolution, -
	// repaint directly from the given bitmap
	if ((sprite_id == UINT32_MAX) && (ddb->GetColorDepth() == source->GetColorDepth()) &&
		(ddb->GetWidth() == source->GetWidth()) && (ddb->GetHeight() == source->GetHeight())) {
		_G(gfxDriver)->UpdateDDBFromBitmap(ddb, source, has_alpha);
		return ddb;
	}
	// have to recreate ddb
	_G(gfxDriver)->DestroyDDB(ddb);
	return _G(gfxDriver)->GetSharedDDB(sprite_id, source, has_alpha, opaque);
}

// FIXME: make has_alpha and opaque properties of ObjTexture?!
static void sync_object_texture(ObjTexture &obj, bool has_alpha = false, bool opaque = false) {
	Bitmap *use_bmp = obj.Bmp.get() ? obj.Bmp.get() : _GP(spriteset)[obj.SpriteID];
	obj.Ddb = recycle_ddb_sprite(obj.Ddb, obj.SpriteID, use_bmp, has_alpha, opaque);
	// Handle notification control block for the dynamic sprites
	if ((obj.SpriteID != UINT32_MAX) && _GP(game).SpriteInfos[obj.SpriteID].IsDynamicSprite()) {
		// For dynamic sprite: check and update a notification block for this drawable
		if (!obj.SpriteNotify || (*obj.SpriteNotify != obj.SpriteID)) {
			auto it_notify = _G(drawstate).SpriteNotifyMap.find(obj.SpriteID);
			if (it_notify != _G(drawstate).SpriteNotifyMap.end()) { // assign existing
				obj.SpriteNotify = it_notify->_value;
			}
		} else { // if does not exist, then create and share one
			obj.SpriteNotify.reset(new (uint32_t)(obj.SpriteID));
			_G(drawstate).SpriteNotifyMap.insert(std::make_pair((sprkey_t)obj.SpriteID, obj.SpriteNotify));
		}
	} else {
		obj.SpriteNotify = nullptr; // reset, for static sprite or without ID
	}
}

//------------------------------------------------------------------------
// Functions for filling the lists of sprites to render

static void clear_draw_list() {
	_GP(thingsToDrawList).clear();
}

static void add_thing_to_draw(IDriverDependantBitmap *ddb, int x, int y) {
	assert(ddb != nullptr);
	SpriteListEntry sprite;
	sprite.ddb = ddb;
	sprite.x = x;
	sprite.y = y;
	_GP(thingsToDrawList).push_back(sprite);
}

static void add_render_stage(int stage) {
	SpriteListEntry sprite;
	sprite.renderStage = stage;
	_GP(thingsToDrawList).push_back(sprite);
}

static void clear_sprite_list() {
	_GP(sprlist).clear();
}

static void add_to_sprite_list(IDriverDependantBitmap *ddb, int x, int y, int zorder, int id = -1) {
	assert(ddb);
	// completely invisible, so don't draw it at all
	if (ddb->GetAlpha() == 0)
		return;

	SpriteListEntry sprite;
	sprite.id = id;
	sprite.ddb = ddb;
	sprite.zorder = zorder;
	sprite.x = x;
	sprite.y = y;

	_GP(sprlist).push_back(sprite);
}

// Sprite drawing order sorting function,
// where equal zorder is resolved by comparing optional IDs too.
static bool spritelistentry_less(const SpriteListEntry &e1, const SpriteListEntry &e2) {
	return (e1.zorder < e2.zorder) ||
		   ((e1.zorder == e2.zorder) && (e1.id < e2.id));
}

// copy the sorted sprites into the Things To Draw list
static void draw_sprite_list() {
	std::sort(_GP(sprlist).begin(), _GP(sprlist).end(), spritelistentry_less);
	_GP(thingsToDrawList).insert(_GP(thingsToDrawList).end(),
		_GP(sprlist).begin(), _GP(sprlist).end());
}

// Push the gathered list of sprites into the active graphic renderer
void put_sprite_list_on_screen(bool in_room);
//
//------------------------------------------------------------------------

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
// NOTE: use_alpha arg is for backward compatibility (legacy draw modes)
void draw_gui_sprite(Bitmap *ds, int pic, int x, int y, bool use_alpha, BlendMode blend_mode) {
	draw_gui_sprite(ds, use_alpha, x, y, _GP(spriteset)[pic],
		(_GP(game).SpriteInfos[pic].Flags & SPF_ALPHACHANNEL) != 0, blend_mode);
}

void draw_gui_sprite(Bitmap *ds, bool use_alpha, int x, int y, Bitmap *sprite, bool src_has_alpha,
	BlendMode blend_mode, int alpha) {
	if (alpha <= 0)
		return;

	const bool ds_has_alpha = (ds->GetColorDepth() == 32);
	if (use_alpha && _GP(game).options[OPT_NEWGUIALPHA] == kGuiAlphaRender_Proper) {
		GfxUtil::DrawSpriteBlend(ds, Point(x, y), sprite, blend_mode, ds_has_alpha, src_has_alpha, alpha);
	}
	// Backwards-compatible drawing
	else if (use_alpha && ds_has_alpha && (_GP(game).options[OPT_NEWGUIALPHA] == kGuiAlphaRender_AdditiveAlpha) && (alpha == 0xFF)) {
		if (src_has_alpha)
			set_additive_alpha_blender();
		else
			set_opaque_alpha_blender();
		ds->TransBlendBlt(sprite, x, y);
	} else {
		GfxUtil::DrawSpriteWithTransparency(ds, sprite, x, y, alpha);
	}
}

void draw_gui_sprite_v330(Bitmap *ds, int pic, int x, int y, bool use_alpha, BlendMode blend_mode) {
	draw_gui_sprite(ds, pic, x, y, use_alpha && (_G(loaded_game_file_version) >= kGameVersion_330), blend_mode);
}

// Avoid freeing and reallocating the memory if possible
Bitmap *recycle_bitmap(Bitmap *bimp, int coldep, int wid, int hit, bool make_transparent) {
	if (bimp != nullptr) {
		// same colour depth, width and height -> reuse
		if ((bimp->GetColorDepth() == coldep) && (bimp->GetWidth() == wid)
		        && (bimp->GetHeight() == hit)) {
			bimp->ResetClip();
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

void recycle_bitmap(std::unique_ptr<Shared::Bitmap> &bimp, int coldep, int wid, int hit, bool make_transparent) {
	bimp.reset(recycle_bitmap(bimp.release(), coldep, wid, hit, make_transparent));
}

// Get the local tint at the specified X & Y co-ordinates, based on
// room regions and SetAmbientTint
// tint_amnt will be set to 0 if there is no tint enabled
// if this is the case, then light_lev holds the light level (0=none)
void get_local_tint(int xpp, int ypp, bool use_region_tint,
                    int *tint_amnt, int *tint_r, int *tint_g,
                    int *tint_b, int *tint_lit,
                    int *light_lev) {

	int tint_level = 0, light_level = 0;
	int tint_amount = 0;
	int tint_red = 0;
	int tint_green = 0;
	int tint_blue = 0;
	int tint_light = 255;

	if (use_region_tint) {

		int onRegion = 0;

		if ((_GP(play).ground_level_areas_disabled & GLED_EFFECTS) == 0) {
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
			light_level = _GP(thisroom).Regions[onRegion].Light;
			tint_level = _GP(thisroom).Regions[onRegion].Tint;
		} else if (onRegion <= 0) {
			light_level = _GP(thisroom).Regions[0].Light;
			tint_level = _GP(thisroom).Regions[0].Tint;
		}

		int tint_sat = (tint_level >> 24) & 0xFF;
		if ((_GP(game).color_depth == 1) || ((tint_level & 0x00ffffff) == 0) ||
		        (tint_sat == 0))
			tint_level = 0;

		if (tint_level) {
			tint_red = (unsigned char)(tint_level & 0x000ff);
			tint_green = (unsigned char)((tint_level >> 8) & 0x000ff);
			tint_blue = (unsigned char)((tint_level >> 16) & 0x000ff);
			tint_amount = tint_sat;
			tint_light = light_level;
		}

		if (_GP(play).rtint_enabled) {
			if (_GP(play).rtint_level > 0) {
				// override with room tint
				tint_red = _GP(play).rtint_red;
				tint_green = _GP(play).rtint_green;
				tint_blue = _GP(play).rtint_blue;
				tint_amount = _GP(play).rtint_level;
				tint_light = _GP(play).rtint_light;
			} else {
				// override with room light level
				tint_amount = 0;
				light_level = _GP(play).rtint_light;
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




// Applies the specified RGB Tint or Light Level to the ObjTexture 'actsp'.
// Used for software render mode only.
static void apply_tint_or_light(ObjTexture &actsp, int light_level,
								int tint_amount, int tint_red, int tint_green,
								int tint_blue, int tint_light, int coldept,
								Bitmap *blitFrom) {

	// In a 256-colour game, we cannot do tinting or lightning
	// (but we can do darkening, if light_level < 0)
	if (_GP(game).color_depth == 1) {
		if ((light_level > 0) || (tint_amount != 0))
			return;
	}

	// we can only do tint/light if the colour depths match
	if (_GP(game).GetColorDepth() == actsp.Bmp->GetColorDepth()) {
		std::unique_ptr<Bitmap> oldwas;
		// if the caller supplied a source bitmap, ->Blit from it
		// (used as a speed optimisation where possible)
		if (blitFrom)
			oldwas.reset(blitFrom);
		// otherwise, make a new target bmp
		else {
			oldwas = std::move(actsp.Bmp);
			actsp.Bmp.reset(BitmapHelper::CreateBitmap(oldwas->GetWidth(), oldwas->GetHeight(), coldept));
		}
		Bitmap *active_spr = actsp.Bmp.get();

		if (tint_amount) {
			// It is an RGB tint
			tint_image(active_spr, oldwas.get(), tint_red, tint_green, tint_blue, tint_amount, tint_light);
		} else {
			// the RGB values passed to set_trans_blender decide whether it will darken
			// or lighten sprites ( <128=darken, >128=lighten). The parameter passed
			// to LitBlendBlt defines how much it will be darkened/lightened by.

			int lit_amnt;
			active_spr->FillTransparent();
			// It's a light level, not a tint
			if (_GP(game).color_depth == 1) {
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

			active_spr->LitBlendBlt(oldwas.get(), 0, 0, lit_amnt);
		}

		if (oldwas.get() == blitFrom)
			oldwas.release();

	} else if (blitFrom) {
		// sprite colour depth != game colour depth, so don't try and tint
		// but we do need to do something, so copy the source
		Bitmap *active_spr = actsp.Bmp.get();
		active_spr->Blit(blitFrom, 0, 0, 0, 0, active_spr->GetWidth(), active_spr->GetHeight());
	}
}

// Generates a transformed sprite, using src image and parameters;

// * if transformation is necessary - writes into dst and returns dst;
// * if no transformation is necessary - simply returns src;
// Used for software render mode only.
static Bitmap *transform_sprite(Bitmap *src, bool src_has_alpha, std::unique_ptr<Bitmap> &dst,
								const Size dst_sz, GraphicFlip flip = Shared::kFlip_None) {
	if ((src->GetSize() == dst_sz) && (flip == kFlip_None))
		return src; // No transform: return source image

	recycle_bitmap(dst, src->GetColorDepth(), dst_sz.Width, dst_sz.Height, true);
	set_our_eip(339);

	// If scaled: first scale then optionally mirror
	if (src->GetSize() != dst_sz) {
		// 8-bit support: ensure that anti-aliasing routines have a palette
		// to use for mapping while faded out.
		// TODO: find out if this may be moved out and not repeated?
		if (_G(in_new_room) > 0)
			select_palette(_G(palette));

		if (flip != kFlip_None) {
			Bitmap tempbmp;
			tempbmp.CreateTransparent(dst_sz.Width, dst_sz.Height, src->GetColorDepth());
			if ((IS_ANTIALIAS_SPRITES) && !src_has_alpha)
				tempbmp.AAStretchBlt(src, RectWH(dst_sz), kBitmap_Transparency);
			else
				tempbmp.StretchBlt(src, RectWH(dst_sz), kBitmap_Transparency);
			dst->FlipBlt(&tempbmp, 0, 0, kFlip_Horizontal);
		} else {
			if ((IS_ANTIALIAS_SPRITES) && !src_has_alpha)
				dst->AAStretchBlt(src, RectWH(dst_sz), kBitmap_Transparency);
			else
				dst->StretchBlt(src, RectWH(dst_sz), kBitmap_Transparency);
		}

		if (_G(in_new_room) > 0)
			unselect_palette();
	} else {
		// If not scaled, then simply blit mirrored
		dst->FlipBlt(src, 0, 0, kFlip_Horizontal);
	}
	return dst.get(); // return transformed result
}

// Draws the specified 'sppic' sprite onto ObjTexture 'actsp' at the
// specified width and height, and flips the sprite if necessary.
// Returns 1 if something was drawn to actsps; returns 0 if no
// scaling or stretching was required, in which case nothing was done.
// Used for software render mode only.
static bool scale_and_flip_sprite(ObjTexture &actsp, int sppic, int width, int height, bool hmirror) {
	Bitmap *src = _GP(spriteset)[sppic];
	Bitmap *result = transform_sprite(src, (_GP(game).SpriteInfos[sppic].Flags & SPF_ALPHACHANNEL) != 0,
		actsp.Bmp, Size(width, height), hmirror ? kFlip_Horizontal : kFlip_None);
	return result != src;
}

// Create the actsps[objid] image with the object drawn correctly.
// Returns true if nothing at all has changed and actsps is still
// intact from last time; false otherwise.
// Hardware-accelerated renderers always return true, because they do not
// require altering the raw bitmap itself.
// Except if alwaysUseSoftware is set, in which case even HW renderers
// construct the image in software mode as well.
static bool construct_object_gfx(const ViewFrame *vf, int pic,
								 const Size &scale_size,
								 int tint_flags,            // OBJF_* flags related to using tint and light fx
								 const ObjectCache &objsrc, // source item to acquire values from
								 ObjectCache &objsav,       // cache item to use
								 ObjTexture &actsp,			// object texture to draw upon
								 bool optimize_by_position, // allow to optimize walk-behind merging using object's pos
								 bool force_software) {
	const bool use_hw_transform = !force_software && !_G(drawstate).SoftwareRender;

	int tint_red, tint_green, tint_blue;
	int tint_level, tint_light, light_level;
	tint_red = tint_green = tint_blue = tint_level = tint_light = light_level = 0;

	if (tint_flags & OBJF_HASTINT) {
		// object specific tint, use it
		tint_red = objsrc.tintr;
		tint_green = objsrc.tintg;
		tint_blue = objsrc.tintb;
		tint_level = objsrc.tintamnt;
		tint_light = objsrc.tintlight;
		light_level = 0;
	} else if (tint_flags & OBJF_HASLIGHT) {
		light_level = objsrc.tintlight;
	} else {
		// get the ambient or region tint
		get_local_tint(objsrc.x, objsrc.y, (tint_flags & OBJF_USEREGIONTINTS) != 0,
		               &tint_level, &tint_red, &tint_green, &tint_blue,
		               &tint_light, &light_level);
	}

	// check whether the image should be flipped
	bool is_mirrored = false;
	int specialpic = pic;
	if (vf && (vf->pic == pic) && ((vf->flags & VFLG_FLIPSPRITE) != 0)) {
		is_mirrored = true;
		specialpic = -pic;
	}

	actsp.SpriteID = pic; // for texture sharing

	// Hardware accelerated mode: always use original sprite and apply texture transform
	if (use_hw_transform) {
		// HW acceleration
		const bool is_texture_intact = (objsav.sppic == specialpic) && !actsp.IsChangeNotified();
		objsav.sppic = specialpic;
		objsav.tintamnt = tint_level;
		objsav.tintr = tint_red;
		objsav.tintg = tint_green;
		objsav.tintb = tint_blue;
		objsav.tintlight = tint_light;
		objsav.lightlev = light_level;
		objsav.zoom = objsrc.zoom;
		objsav.mirrored = is_mirrored;
		return is_texture_intact;
	}

	//
	// Software mode below
	//
	// They want to draw it in software mode with the hw driver, so force a redraw (???)
	if (!_G(drawstate).SoftwareRender) {
		objsav.sppic = INT32_MIN;
	}

	// If we have the image cached, use it
	if ((objsav.image != nullptr) &&
	        (objsav.sppic == specialpic) &&
			// not a dynamic sprite, or not sprite modified lately
			(!actsp.IsChangeNotified()) &&
			(objsav.tintamnt == tint_level) &&
			(objsav.tintlight == tint_light) &&
			(objsav.tintr == tint_red) &&
			(objsav.tintg == tint_green) &&
			(objsav.tintb == tint_blue) &&
			(objsav.lightlev == light_level) &&
			(objsav.zoom == objsrc.zoom) &&
			(objsav.mirrored == is_mirrored)) {
		// if the image is the same, we can use it cached
		if ((_G(drawstate).WalkBehindMethod != DrawOverCharSprite) &&
			(actsp.Bmp != nullptr))
			return true;
		// Check if the X & Y co-ords are the same, too -- if so, there
		// is scope for further optimisations
		if (optimize_by_position &&
			(objsav.x == objsrc.x) &&
				(objsav.y == objsrc.y) &&
				(actsp.Bmp != nullptr) &&
				(_G(walk_behind_baselines_changed) == 0))
			return true;
		recycle_bitmap(actsp.Bmp, objsav.image->GetColorDepth(), objsav.image->GetWidth(), objsav.image->GetHeight());
		actsp.Bmp->Blit(objsav.image.get(), 0, 0);
		return false; // image was modified
	}

	// Not cached, so draw the image
	Bitmap *sprite = _GP(spriteset)[pic];
	const int coldept = sprite->GetColorDepth();
	const int src_sprwidth = sprite->GetWidth();
	const int src_sprheight = sprite->GetHeight();
	bool actsps_used = false;
	// draw the base sprite, scaled and flipped as appropriate
	actsps_used = scale_and_flip_sprite(actsp, pic, scale_size.Width, scale_size.Height, is_mirrored);
	if (!actsps_used) {
		// ensure actsps exists // CHECKME: why do we need this in hardware accel mode too?
		recycle_bitmap(actsp.Bmp, coldept, src_sprwidth, src_sprheight);
	}

	// apply tints or lightenings where appropriate, else just copy
	// the source bitmap
	if ((tint_level > 0) || (light_level != 0)) {
		// direct read from source bitmap, where possible
		Bitmap *blit_from = nullptr;
		if (!actsps_used)
			blit_from = sprite;

		apply_tint_or_light(actsp, light_level, tint_level, tint_red,
			tint_green, tint_blue, tint_light, coldept,
			blit_from);
	} else if (!actsps_used) {
		// no scaling, flipping or tinting was done, so just blit it normally
		actsp.Bmp->Blit(sprite, 0, 0);
	}

	// Create the cached image and store it
	objsav.in_use = true;
	recycle_bitmap(objsav.image, actsp.Bmp->GetColorDepth(), actsp.Bmp->GetWidth(), actsp.Bmp->GetHeight());
	objsav.image->Blit(actsp.Bmp.get(), 0, 0);
	objsav.sppic = specialpic;
	objsav.tintamnt = tint_level;
	objsav.tintr = tint_red;
	objsav.tintg = tint_green;
	objsav.tintb = tint_blue;
	objsav.tintlight = tint_light;
	objsav.lightlev = light_level;
	objsav.zoom = objsrc.zoom;
	objsav.mirrored = is_mirrored;
	objsav.x = objsrc.x;
	objsav.y = objsrc.y;
	return false; // image was modified
}

// Generate object's raw sprite bitmap, update the object's texture
// from the sprite, add the object's texture to the draw list.
// - atx and aty are coordinates of the top-left object's corner in the room;
// - usebasel is object's z-order, it may be modified within the function;
// TODO: possibly makes sense to split this function into parts later.
void prepare_and_add_object_gfx(const ObjectCache &objsav, ObjTexture &actsp, bool actsp_modified, const Size &scale_size,
								int atx, int aty, int &usebasel, bool use_walkbehinds, int transparency, bool hw_accel) {
	// Handle the walk-behinds, according to the walkBehindMethod.
	// This potentially may edit actsp's raw bitmap if actsp_modified is set.
	if (use_walkbehinds) {
		// Only merge sprite with the walk-behinds in software mode
		if ((_G(drawstate).WalkBehindMethod == DrawOverCharSprite) && (actsp_modified)) {
			walkbehinds_cropout(actsp.Bmp.get(), atx, aty, usebasel);
		}
	} else {
		// Ignore walk-behinds by shifting baseline to a larger value
		// CHECKME: may this fail if WB somehow got larger than room baseline?
		if (_G(drawstate).WalkBehindMethod == DrawAsSeparateSprite) {
			usebasel += _GP(thisroom).Height;
		}
	}

	// Sync object texture with the raw sprite bitmap.
	if ((actsp.Ddb == nullptr) || (actsp_modified)) {
		sync_object_texture(actsp, (_GP(game).SpriteInfos[actsp.SpriteID].Flags & SPF_ALPHACHANNEL) != 0);
	}

	// Now when we have a ready texture, assign texture properties
	// (transform, effects, and so forth)
	if (hw_accel) {
		actsp.Ddb->SetStretch(scale_size.Width, scale_size.Height);
		actsp.Ddb->SetFlippedLeftRight(objsav.mirrored);
		actsp.Ddb->SetTint(objsav.tintr, objsav.tintg, objsav.tintb, (objsav.tintamnt * 256) / 100);

		if (objsav.tintamnt > 0) {
			if (objsav.tintlight == 0) // luminance of 0 -- pass 1 to enable
				actsp.Ddb->SetLightLevel(1);
			else if (objsav.tintlight < 250)
				actsp.Ddb->SetLightLevel(objsav.tintlight);
			else
				actsp.Ddb->SetLightLevel(0);
		} else if (objsav.lightlev != 0)
			actsp.Ddb->SetLightLevel((objsav.lightlev * 25) / 10 + 256);
		else
			actsp.Ddb->SetLightLevel(0);
	}

	actsp.Ddb->SetAlpha(GfxDef::LegacyTrans255ToAlpha255(transparency));
}

// Generates RoomObject's raw bitmap and saves in actsps; updates object cache.
bool construct_object_gfx(int objid, bool force_software) {
	const RoomObject &obj = _G(objs)[objid];
	if (!_GP(spriteset).DoesSpriteExist(obj.num))
		quitprintf("There was an error drawing object %d. Its current sprite, %d, is invalid.", objid, obj.num);

	ObjectCache objsrc(obj.num, obj.tint_r, obj.tint_g, obj.tint_b,
					   obj.tint_level, obj.tint_light, 0 /* skip */, obj.zoom, false /* skip */,
					   obj.x, obj.y);

	return construct_object_gfx(
		(obj.view != UINT16_MAX) ? &_GP(views)[obj.view].loops[obj.loop].frames[obj.frame] : nullptr,
		obj.num,
		Size(obj.last_width, obj.last_height),
		obj.flags & OBJF_TINTLIGHTMASK,
		objsrc,
		_G(objcache)[objid],
		_GP(actsps)[objid],
		true,
		force_software);
}

void prepare_objects_for_drawing() {
	set_our_eip(32);

	const bool hw_accel = !_G(drawstate).SoftwareRender;

	for (uint32_t objid = 0; objid < _G(croom)->numobj; ++objid) {
		const RoomObject &obj = _G(objs)[objid];
		if (obj.on != 1) // WARNING: 'on' may have other values than 0 and 1 !!
			continue;    // disabled
		// offscreen, don't draw
		if ((obj.x >= _GP(thisroom).Width) || (obj.y < 1))
			continue; // offscreen

		_G(eip_guinum) = objid;
		const ObjectCache &objsav = _G(objcache)[objid];
		ObjTexture &actsp = _GP(actsps)[objid];

		// Calculate sprite top-left position in the room and baseline
		const int atx = data_to_game_coord(obj.x);
		const int aty = data_to_game_coord(obj.y) - obj.last_height;
		int usebasel = obj.get_baseline();

		// Generate raw bitmap in ObjTexture and store parameters in ObjectCache.
		bool actsp_modified = !construct_object_gfx(objid, false);
		// Prepare the object texture
		prepare_and_add_object_gfx(objsav, actsp, actsp_modified,
								   Size(obj.last_width, obj.last_height), atx, aty, usebasel,
								   (obj.flags & OBJF_NOWALKBEHINDS) == 0, obj.transparent, hw_accel);
		// Finally, add the texture to the draw list
		add_to_sprite_list(actsp.Ddb, atx, aty, usebasel);
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

	// Some games have incorrect data that result in a negative luminance.
	// Do the same as the accelerated drivers that use 255 luminance for that case.
	if (luminance < 0)
		luminance = 255;

	// For performance reasons, we have a separate blender for
	// when light is being adjusted and when it is not.
	// If luminance >= 250, then normal brightness, otherwise darken
	if (luminance >= 250)
		set_blender_mode(kTintBlenderMode, red, grn, blu, 0);
	else
		set_blender_mode(kTintLightBlenderMode, red, grn, blu, 0);

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

// Generates Character's raw bitmap and saves in actsps; updates character cache.
bool construct_char_gfx(int charid, bool force_software) {
	// const bool use_hw_transform = !force_software && _G(gfxDriver)->HasAcceleratedTransform();

	const CharacterInfo &chin = _GP(game).chars[charid];
	const CharacterExtras &chex = _GP(charextra)[charid];
	const ViewFrame *vf = &_GP(views)[chin.view].loops[chin.loop].frames[chin.frame];
	const int pic = vf->pic;
	if (!_GP(spriteset).DoesSpriteExist(pic))
		quitprintf("There was an error drawing character %d. Its current frame's sprite, %d, is invalid.", charid, pic);

	ObjectCache chsrc(pic, chex.tint_r, chex.tint_g, chex.tint_b,
					  chex.tint_level, chex.tint_light, 0 /* skip */, chex.zoom, false /* skip */,
					  chin.x, chin.y);

	return construct_object_gfx(
		vf,
		pic,
		Size(chex.width, chex.height),
		CharFlagsToObjFlags(chin.flags) & OBJF_TINTLIGHTMASK,
		chsrc,
		_GP(charcache)[charid],
		_GP(actsps)[charid + ACTSP_OBJSOFF],
		false,                  // characters cannot optimize by pos, probably because of z coord and view offsets (?)
		force_software);
}

void prepare_characters_for_drawing() {
	set_our_eip(33);
	const bool hw_accel = !_G(drawstate).SoftwareRender;

	// draw characters
	for (int charid = 0; charid < _GP(game).numcharacters; ++charid) {
		const CharacterInfo &chin = _GP(game).chars[charid];
		if (chin.on == 0)
			continue;  // disabled
		if (chin.room != _G(displayed_room))
			continue;  // in another room

		_G(eip_guinum) = charid;
		const CharacterExtras &chex = _GP(charextra)[charid];
		const ObjectCache &chsav = _GP(charcache)[charid];
		ObjTexture &actsp = _GP(actsps)[charid + ACTSP_OBJSOFF];

		// Calculate sprite top-left position in the room and baseline
		const int atx = chin.actx + chin.pic_xoffs * chex.zoom_offs / 100;
		const int aty = chin.acty + chin.pic_yoffs * chex.zoom_offs / 100;
		int usebasel = chin.get_baseline();

        // Generate raw bitmap in ObjTexture and store parameters in ObjectCache.
        bool actsp_modified = !construct_char_gfx(charid, false);
        // Prepare the object texture
        prepare_and_add_object_gfx(chsav, actsp, actsp_modified,
            Size(chex.width, chex.height), atx, aty, usebasel,
            (chin.flags & CHF_NOWALKBEHINDS) == 0, chin.transparency, hw_accel);
        // Finally, add the texture to the draw list
        add_to_sprite_list(actsp.Ddb, atx, aty, usebasel);
	}
}

Bitmap *get_cached_character_image(int charid) {
	return _GP(actsps)[charid + ACTSP_OBJSOFF].Bmp.get();
}

Bitmap *get_cached_object_image(int objid) {
	return _GP(actsps)[objid].Bmp.get();
}

void add_walkbehind_image(size_t index, Shared::Bitmap *bmp, int x, int y) {
	if (_GP(walkbehindobj).size() <= index)
		_GP(walkbehindobj).resize(index + 1);
	_GP(walkbehindobj)[index].Bmp.reset(); // don't store bitmap if added this way
	_GP(walkbehindobj)[index].Ddb = recycle_ddb_bitmap(_GP(walkbehindobj)[index].Ddb, bmp);
	_GP(walkbehindobj)[index].Pos = Point(x, y);
}

// Add active room overlays to the sprite list
static void add_roomovers_for_drawing() {
	const auto &overs = get_overlays();
	for (const auto &over : overs) {
		if (over.type < 0) continue; // empty slot
		if (!over.IsRoomLayer()) continue; // not a room layer
		if (over.transparency == 255) continue; // skip fully transparent
		Point pos = get_overlay_position(over);
		add_to_sprite_list(_GP(overtxs)[over.type].Ddb, pos.X, pos.Y, over.zorder, over.creation_id);
	}
}

// Compiles a list of room sprites (characters, objects, background)
void prepare_room_sprites() {
	// Background sprite is required for the non-software renderers always,
	// and for software renderer in case there are overlapping viewports.
	// Note that software DDB is just a tiny wrapper around bitmap, so overhead is negligible.
	if (_G(current_background_is_dirty) || !_G(roomBackgroundBmp)) {
		_G(roomBackgroundBmp) =
			recycle_ddb_bitmap(_G(roomBackgroundBmp), _GP(thisroom).BgFrames[_GP(play).bg_frame].Graphic.get(), false, true);

	}
	if (_G(drawstate).FullFrameRedraw) {
		if (_G(current_background_is_dirty) || _G(walkBehindsCachedForBgNum) != _GP(play).bg_frame) {
			if (_G(drawstate).WalkBehindMethod == DrawAsSeparateSprite) {
				walkbehinds_generate_sprites();
			}
		}
		add_thing_to_draw(_G(roomBackgroundBmp), 0, 0);
	}
	_G(current_background_is_dirty) = false; // Note this is only place where this flag is checked

	clear_sprite_list();

	if ((_G(debug_flags) & DBG_NOOBJECTS) == 0) {
		prepare_objects_for_drawing();
		prepare_characters_for_drawing();
		add_roomovers_for_drawing();

		if ((_G(debug_flags) & DBG_NODRAWSPRITES) == 0) {
			set_our_eip(34);

			if (_G(drawstate).WalkBehindMethod == DrawAsSeparateSprite) {
				for (size_t wb = 1 /* 0 is "no area" */;
					(wb < MAX_WALK_BEHINDS) && (wb < (size_t)_GP(walkbehindobj).size()); ++wb) {
					const auto &wbobj = _GP(walkbehindobj)[wb];
					if (wbobj.Ddb) {
						// when baselines are equal, walk-behinds must be sorted back, so tag as INT32_MIN
						add_to_sprite_list(wbobj.Ddb, wbobj.Pos.X, wbobj.Pos.Y, _G(croom)->walkbehind_base[wb], INT32_MIN);
					}
				}
			}

			if (pl_any_want_hook(AGSE_PRESCREENDRAW))
				add_render_stage(AGSE_PRESCREENDRAW);

			draw_sprite_list();
		}
	}
	set_our_eip(36);

	// Debug room overlay
	update_room_debug();
	if ((_G(debugRoomMask) != kRoomAreaNone) && _GP(debugRoomMaskObj).Ddb)
		add_thing_to_draw(_GP(debugRoomMaskObj).Ddb, 0, 0);
	if ((_G(debugMoveListChar) >= 0) && _GP(debugMoveListObj).Ddb)
		add_thing_to_draw(_GP(debugMoveListObj).Ddb, 0, 0);

	if (pl_any_want_hook(AGSE_POSTROOMDRAW))
		add_render_stage(AGSE_POSTROOMDRAW);
}

// Draws the black surface behind (or rather between) the room viewports
void draw_preroom_background() {
	if (_G(drawstate).FullFrameRedraw)
		return;
	update_black_invreg_and_reset(_G(gfxDriver)->GetMemoryBackBuffer());
}

// Draws the room background on the given surface.
//
// NOTE that this is **strictly** for software rendering.
// ds is a full game screen surface, and roomcam_surface is a surface for drawing room camera content to.
// ds and roomcam_surface may be the same bitmap.
// no_transform flag tells to copy dirty regions on roomcam_surface without any coordinate conversion
// whatsoever.
PBitmap draw_room_background(Viewport *view) {
	set_our_eip(31);

	// For the sake of software renderer, if there is any kind of camera transform required
	// except screen offset, we tell it to draw on separate bitmap first with zero transformation.
	// There are few reasons for this, primary is that Allegro does not support StretchBlt
	// between different colour depths (i.e. it won't correctly stretch blit 16-bit rooms to
	// 32-bit virtual screen).
	// Also see comment to ALSoftwareGraphicsDriver::RenderToBackBuffer().
	const int view_index = view->GetID();
	Bitmap *ds = _G(gfxDriver)->GetMemoryBackBuffer();
	// If separate bitmap was prepared for this view/camera pair then use it, draw untransformed
	// and blit transformed whole surface later.
	const bool draw_to_camsurf = _GP(CameraDrawData)[view_index].Frame != nullptr;
	Bitmap *roomcam_surface = draw_to_camsurf ? _GP(CameraDrawData)[view_index].Frame.get() : ds;
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
		update_room_invreg_and_reset(view_index, roomcam_surface, _GP(thisroom).BgFrames[_GP(play).bg_frame].Graphic.get(), draw_to_camsurf);
	}

	return _GP(CameraDrawData)[view_index].Frame;
}

void dispose_engine_overlay() {
	_G(gl_DrawFPS).bmp.reset();
	if (_G(gl_DrawFPS).ddb)
		_G(gfxDriver)->DestroyDDB(_G(gl_DrawFPS).ddb);
	_G(gl_DrawFPS).ddb = nullptr;
	_G(gl_DrawFPS).font = -1;
}

void draw_fps(const Rect &viewport) {
	const int font = FONT_NORMAL;
	auto &fpsDisplay = _G(gl_DrawFPS).bmp;
	if (fpsDisplay == nullptr || _G(gl_DrawFPS).font != font) {
		recycle_bitmap(fpsDisplay, _GP(game).GetColorDepth(), viewport.GetWidth(), (get_font_surface_height(font) + get_fixed_pixel_size(5)));
		_G(gl_DrawFPS).font = font;
	}

	fpsDisplay->ClearTransparent();
	const color_t text_color = fpsDisplay->GetCompatibleColor(14);
	char base_buffer[20];
	if (!isTimerFpsMaxed()) {
		snprintf(base_buffer, sizeof(base_buffer), "%d", _G(frames_per_second));
	} else {
		snprintf(base_buffer, sizeof(base_buffer), "unlimited");
	}

	char fps_buffer[60];
	// Don't display fps if we don't have enough information (because loop count was just reset)
	float fps = get_real_fps();
	if (!isnan(fps)) {
		snprintf(fps_buffer, sizeof(fps_buffer), "FPS: %2.1f / %s", fps, base_buffer);
	} else {
		snprintf(fps_buffer, sizeof(fps_buffer), "FPS: --.- / %s", base_buffer);
	}
	char loop_buffer[60];
	snprintf(loop_buffer, sizeof(loop_buffer), "Loop %u", _G(loopcounter));

	int text_off = get_font_surface_extent(font).first; // TODO: a generic function that accounts for this?
	wouttext_outline(fpsDisplay.get(), 1, 1 - text_off, font, text_color, fps_buffer);
	wouttext_outline(fpsDisplay.get(), viewport.GetWidth() / 2, 1 - text_off, font, text_color, loop_buffer);
	_G(gl_DrawFPS).ddb = recycle_ddb_bitmap(_G(gl_DrawFPS).ddb, _G(gl_DrawFPS).bmp.get());
	int yp = viewport.GetHeight() - fpsDisplay->GetHeight();
	_G(gfxDriver)->DrawSprite(1, yp, _G(gl_DrawFPS).ddb);
	invalidate_sprite_glob(1, yp, _G(gl_DrawFPS).ddb);
}

// Draw GUI controls as separate sprites
void draw_gui_controls(GUIMain &gui) {
	if (_G(all_buttons_disabled >= 0) && (GUI::Options.DisabledStyle == kGuiDis_Blackout))
		return; // don't draw GUI controls

	int draw_index = _GP(guiobjddbref)[gui.ID];
	for (int i = 0; i < gui.GetControlCount(); ++i, ++draw_index) {
		GUIObject *obj = gui.GetControl(i);
		if (!obj->IsVisible() ||
			(obj->GetSize().IsNull()) ||
			(!obj->IsEnabled() && (GUI::Options.DisabledStyle == kGuiDis_Blackout)))
			continue;
		if (!obj->HasChanged())
			continue;

		auto &objbg = _GP(guiobjbg)[draw_index];
		Rect obj_surf = obj->CalcGraphicRect(GUI::Options.ClipControls);
		recycle_bitmap(objbg.Bmp, _GP(game).GetColorDepth(), obj_surf.GetWidth(), obj_surf.GetHeight(), true);
		obj->Draw(objbg.Bmp.get(), -obj_surf.Left, -obj_surf.Top);

		sync_object_texture(objbg, obj->HasAlphaChannel());
		objbg.Off = Point(obj_surf.GetLT());
		obj->ClearChanged();
	}
}

// Draw GUI and overlays of all kinds, anything outside the room space
void draw_gui_and_overlays() {
	// Draw gui controls on separate textures if:
	// - it is a 3D renderer (software one may require adjustments -- needs testing)
	// - not legacy alpha blending (may we implement specific texture blend?)
	const bool draw_controls_as_textures =
		_G(gfxDriver)->HasAcceleratedTransform()
		&& (_GP(game).options[OPT_NEWGUIALPHA] == kGuiAlphaRender_Proper);

	if (pl_any_want_hook(AGSE_PREGUIDRAW))
		_G(gfxDriver)->DrawSprite(AGSE_PREGUIDRAW, 0, nullptr); // render stage

	clear_sprite_list();

	// Add active overlays to the sprite list
	const auto &overs = get_overlays();
	for (const auto &over : overs) {
		if (over.type < 0) continue; // empty slot
		if (over.IsRoomLayer()) continue; // not a ui layer
		if (over.transparency == 255) continue; // skip fully transparent
		Point pos = get_overlay_position(over);
		add_to_sprite_list(_GP(overtxs)[over.type].Ddb, pos.X, pos.Y, over.zorder, over.creation_id);
	}

	// Add GUIs
	set_our_eip(35);
	if (((_G(debug_flags) & DBG_NOIFACE) == 0) && (_G(displayed_room) >= 0)) {
		if (_G(playerchar)->activeinv >= MAX_INV) {
			quit("!The player.activeinv variable has been corrupted, probably as a result\n"
				"of an incorrect assignment in the game script.");
		}
		if (_G(playerchar)->activeinv < 1) _G(gui_inv_pic) = -1;
		else _G(gui_inv_pic) = _GP(game).invinfo[_G(playerchar)->activeinv].pic;
		set_our_eip(37);
		// Prepare and update GUI textures
		{
			for (int index = 0; index < _GP(game).numgui; ++index) {
				auto &gui = _GP(guis)[index];
				if (!gui.IsDisplayed()) continue; // not on screen
				if (!gui.HasChanged() && !gui.HasControlsChanged()) continue; // no changes: no need to update image
				if (gui.Transparency == 255) continue; // 100% transparent

				_G(eip_guinum) = index;
				set_our_eip(372);
				const bool draw_with_controls = !draw_controls_as_textures;
				if (gui.HasChanged() || (draw_with_controls && gui.HasControlsChanged())) {
					auto &gbg = _GP(guibg)[index];
                    recycle_bitmap(gbg.Bmp, _GP(game).GetColorDepth(), gui.Width, gui.Height, true);
                    if (draw_with_controls)
                        gui.DrawWithControls(gbg.Bmp.get());
                    else
                        gui.DrawSelf(gbg.Bmp.get());

                    const bool is_alpha = gui.HasAlphaChannel();
                    if (is_alpha)
                    {
                        if ((_GP(game).options[OPT_NEWGUIALPHA] == kGuiAlphaRender_Legacy) && (gui.BgImage > 0))
                        {
                            // old-style (pre-3.0.2) GUI alpha rendering
                            repair_alpha_channel(gbg.Bmp.get(), _GP(spriteset)[gui.BgImage]);
                        }
                    }
                    sync_object_texture(gbg, is_alpha);
				}

				set_our_eip(373);
				if (!draw_with_controls && gui.HasControlsChanged()) {
					draw_gui_controls(gui);
				}
				set_our_eip(374);

				gui.ClearChanged();
			}
		}
		set_our_eip(38);
		// Draw the GUIs
		for (int index = 0; index < _GP(game).numgui; ++index) {
			const auto &gui = _GP(guis)[index];
			if (!gui.IsDisplayed()) continue; // not on screen
			if (gui.Transparency == 255) continue; // 100% transparent

			// Don't draw GUI if "GUIs Turn Off When Disabled"
			if ((_GP(game).options[OPT_DISABLEOFF] == kGuiDis_Off) &&
				(_G(all_buttons_disabled) >= 0) &&
				(gui.PopupStyle != kGUIPopupNoAutoRemove))
				continue;

			auto *gui_ddb = _GP(guibg)[index].Ddb;
			assert(gui_ddb); // Test for missing texture, might happen if not marked for update
			if (!gui_ddb) continue;
			gui_ddb->SetAlpha(GfxDef::LegacyTrans255ToAlpha255(gui.Transparency));
			add_to_sprite_list(gui_ddb, gui.X, gui.Y, gui.ZOrder, index);
		}
	}

	// If not adding gui controls as textures, simply move the resulting sprlist to render
	if (!draw_controls_as_textures ||
		(_G(all_buttons_disabled >= 0) && (GUI::Options.DisabledStyle == kGuiDis_Blackout))) {
		draw_sprite_list();
		put_sprite_list_on_screen(false);
		return;
	}
	// If adding control textures, sort the ui list, and then pass into renderer,
	// adding controls and creating sub-batches as necessary
	std::sort(_GP(sprlist).begin(), _GP(sprlist).end(), spritelistentry_less);
	for (const auto &s : _GP(sprlist)) {
		invalidate_sprite(s.x, s.y, s.ddb, false);
		_G(gfxDriver)->DrawSprite(s.x, s.y, s.ddb);
		if (s.id < 0) continue; // not a group parent (gui)
		// Create a sub-batch
		_G(gfxDriver)->BeginSpriteBatch(RectWH(s.x, s.y, s.ddb->GetWidth(), s.ddb->GetHeight()),
			SpriteTransform(s.x, s.y, 1.f, 1.f, 0.f, s.ddb->GetAlpha()));
		const int draw_index = _GP(guiobjddbref)[s.id];
		for (const auto &obj_id : _GP(guis)[s.id].GetControlsDrawOrder()) {
			GUIObject *obj = _GP(guis)[s.id].GetControl(obj_id);
			if (!obj->IsVisible() ||
				(obj->GetSize().IsNull()) ||
				(!obj->IsEnabled() && (GUI::Options.DisabledStyle == kGuiDis_Blackout)))
				continue;
			const auto &obj_tx = _GP(guiobjbg)[draw_index + obj_id];
			auto *obj_ddb = obj_tx.Ddb;
			assert(obj_ddb); // Test for missing texture, might happen if not marked for update
			if (!obj_ddb) continue;
			obj_ddb->SetAlpha(GfxDef::LegacyTrans255ToAlpha255(obj->GetTransparency()));
			_G(gfxDriver)->DrawSprite(obj->X + obj_tx.Off.X, obj->Y + obj_tx.Off.Y, obj_ddb);
		}
		_G(gfxDriver)->EndSpriteBatch();
	}

	set_our_eip(1099);
}

// Push the gathered list of sprites into the active graphic renderer
void put_sprite_list_on_screen(bool in_room) {
	for (const auto &t : _GP(thingsToDrawList)) {
		assert(t.ddb || (t.renderStage >= 0));
		if (t.ddb) {
			if (t.ddb->GetAlpha() == 0)
				continue; // skip completely invisible things
			// mark the image's region as dirty
			invalidate_sprite(t.x, t.y, t.ddb, in_room);
			// push to the graphics driver
			_G(gfxDriver)->DrawSprite(t.x, t.y, t.ddb);
		} else if (t.renderStage >= 0) {
			// meta entry to run the plugin hook
			_G(gfxDriver)->DrawSprite(t.renderStage, 0, nullptr);
		}
	}

	set_our_eip(1100);
}

bool GfxDriverSpriteEvtCallback(int evt, int data) {
	if (_G(displayed_room) < 0) {
		// if no room loaded, various stuff won't be initialized yet
		return false;
	}
	return (pl_run_plugin_hooks(evt, data) != 0);
}

void GfxDriverOnInitCallback(void *data) {
	pl_run_plugin_init_gfx_hooks(_G(gfxDriver)->GetDriverID(), data);
}

// Schedule room rendering: background, objects, characters
static void construct_room_view() {
	draw_preroom_background();
	prepare_room_sprites();
	// reset the zorders Changed flag now that we've drawn stuff
	_G(walk_behind_baselines_changed) = 0;

	for (const auto &viewport : _GP(play).GetRoomViewportsZOrdered()) {
		if (!viewport->IsVisible())
			continue;
		auto camera = viewport->GetCamera();
		if (!camera)
			continue;
		const Rect &view_rc = viewport->GetRect();
		const Rect &cam_rc = camera->GetRect();
		const float view_sx = (float)view_rc.GetWidth() / (float)cam_rc.GetWidth();
		const float view_sy = (float)view_rc.GetHeight() / (float)cam_rc.GetHeight();
		const SpriteTransform view_trans(view_rc.Left, view_rc.Top, view_sx, view_sy);
		const SpriteTransform cam_trans(-cam_rc.Left, -cam_rc.Top);

		if (_G(drawstate).FullFrameRedraw) {
			// For hw renderer we draw everything as a sprite stack;
			// viewport-camera pair is done as 2 nested scene nodes,
			// where first defines how camera's image translates into the viewport on screen,
			// and second - how room's image translates into the camera.
			_G(gfxDriver)->BeginSpriteBatch(view_rc, view_trans);
			_G(gfxDriver)->BeginSpriteBatch(Rect(), cam_trans);
			_G(gfxDriver)->SetStageScreen(cam_rc.GetSize(), cam_rc.Left, cam_rc.Top);
			put_sprite_list_on_screen(true);
			_G(gfxDriver)->EndSpriteBatch();
			_G(gfxDriver)->EndSpriteBatch();
		} else {
			// For software renderer - combine viewport and camera in one batch,
			// due to how the room drawing is implemented currently in the software mode.
			// TODO: review this later?
			_G(gfxDriver)->BeginSpriteBatch(view_rc, view_trans);

			if (_GP(CameraDrawData)[viewport->GetID()].Frame == nullptr && _GP(CameraDrawData)[viewport->GetID()].IsOverlap) {
				// room background is prepended to the sprite stack
				// TODO: here's why we have blit whole piece of background now:
				// if we draw directly to the virtual screen overlapping another
				// viewport, then we'd have to also mark and repaint every our
				// region located directly over their dirty regions. That would
				// require to update regions up the stack, converting their
				// coordinates (cam1 -> screen -> cam2).
				// It's not clear whether this is worth the effort, but if it is,
				// then we'd need to optimise view/cam data first.
				_G(gfxDriver)->BeginSpriteBatch(Rect(), cam_trans);
				_G(gfxDriver)->DrawSprite(0, 0, _G(roomBackgroundBmp));
			} else {
				// room background is drawn by dirty rects system
				PBitmap bg_surface = draw_room_background(viewport.get());
				_G(gfxDriver)->BeginSpriteBatch(Rect(), cam_trans, kFlip_None, bg_surface);
			}
		put_sprite_list_on_screen(true);
		_G(gfxDriver)->EndSpriteBatch();
		_G(gfxDriver)->EndSpriteBatch();
		}
	}

	clear_draw_list();
}

// Schedule ui rendering
static void construct_ui_view() {
	_G(gfxDriver)->BeginSpriteBatch(_GP(play).GetUIViewport());
	draw_gui_and_overlays();
	_G(gfxDriver)->EndSpriteBatch();
	clear_draw_list();
}

// Prepares overlay textures;
// but does not put them on screen yet - that's done in respective construct_*_view functions
static void construct_overlays() {
	const bool is_software_mode = _G(drawstate).SoftwareRender;
	const bool crop_walkbehinds = (_G(drawstate).WalkBehindMethod == DrawOverCharSprite);

	auto &overs = get_overlays();
	if ( _GP(overtxs).size() < overs.size()) {
		_GP(overtxs).resize(overs.size());
		if (is_software_mode)
			_GP(overcache).resize(overs.size(), Point(INT32_MIN, INT32_MIN));
	}
	for (size_t i = 0; i < overs.size(); ++i) {
		auto &over = overs[i];
		if (over.type < 0) continue; // empty slot
		if (over.transparency == 255) continue; // skip fully transparent

		auto &overtx = _GP(overtxs)[i];
		bool has_changed = over.HasChanged();
		// If walk behinds are drawn over the cached object sprite, then check if positions were updated
		if (crop_walkbehinds && over.IsRoomLayer()) {
			Point pos = get_overlay_position(over);
			has_changed |= (pos.X != _GP(overcache)[i].X || pos.Y != _GP(overcache)[i].Y);
			_GP(overcache)[i].X = pos.X; _GP(overcache)[i].Y = pos.Y;
		}

		if (has_changed || overtx.IsChangeNotified()) {
			overtx.SpriteID = over.GetSpriteNum();
			// For software mode - prepare transformed bitmap if necessary;
			// for hardware-accelerated - use the sprite ID if possible, to avoid redundant sprite load
			// TODO: find a way to unify this code with the character & object ObjTexture preparation;
			// they use practically same approach, except of different fields cache.
			Bitmap *use_bmp = nullptr;
			if (is_software_mode) {
				use_bmp = transform_sprite(over.GetImage(), over.HasAlphaChannel(), overtx.Bmp, Size(over.scaleWidth, over.scaleHeight));
				if (crop_walkbehinds && over.IsRoomLayer()) {
					if (use_bmp != overtx.Bmp.get()) {
						recycle_bitmap(overtx.Bmp, use_bmp->GetColorDepth(), use_bmp->GetWidth(), use_bmp->GetHeight(), true);
						overtx.Bmp->Blit(use_bmp);
					}
					Point pos = get_overlay_position(over);
					walkbehinds_cropout(overtx.Bmp.get(), pos.X, pos.Y, over.zorder);
					use_bmp = overtx.Bmp.get();
				}
			}
			sync_object_texture(overtx, over.HasAlphaChannel());
			over.ClearChanged();
		}

		assert(overtx.Ddb); // Test for missing texture, might happen if not marked for update
		if (!overtx.Ddb) continue;
		overtx.Ddb->SetStretch(over.scaleWidth, over.scaleHeight);
		overtx.Ddb->SetAlpha(GfxDef::LegacyTrans255ToAlpha255(over.transparency));
	}
}

void construct_game_scene(bool full_redraw) {
	_G(gfxDriver)->ClearDrawLists();

	if (_GP(play).fast_forward)
		return;

	set_our_eip(3);

	// React to changes to viewports and cameras (possibly from script) just before the render
	_GP(play).UpdateViewports();

	_G(gfxDriver)->UseSmoothScaling(IS_ANTIALIAS_SPRITES);
	_G(gfxDriver)->RenderSpritesAtScreenResolution(_GP(usetup).RenderAtScreenRes);

	pl_run_plugin_hooks(AGSE_PRERENDER, 0);

	// Possible reasons to invalidate whole screen for the software renderer
	if (full_redraw || _GP(play).screen_tint > 0 || _GP(play).shakesc_length > 0)
		invalidate_screen();

	// Overlays may be both in rooms and ui layer, prepare their textures beforehand
	construct_overlays();

	// TODO: move to game update! don't call update during rendering pass!
	// IMPORTANT: keep the order same because sometimes script may depend on it
	if (_G(displayed_room) >= 0)
		_GP(play).UpdateRoomCameras();

	// Begin with the parent scene node, defining global offset and flip
	_G(gfxDriver)->BeginSpriteBatch(_GP(play).GetMainViewport(),
									_GP(play).GetGlobalTransform(_G(drawstate).FullFrameRedraw),
									(GraphicFlip)_GP(play).screen_flipped);

	// Stage: room viewports
	if (_GP(play).screen_is_faded_out == 0 && _GP(play).complete_overlay_on == 0) {
		if (_G(displayed_room) >= 0) {
			construct_room_view();
		} else if (!_G(drawstate).FullFrameRedraw) {
			// black it out so we don't get cursor trails
			// TODO: this is possible to do with dirty rects system now too (it can paint black rects outside of room viewport)
			_G(gfxDriver)->GetMemoryBackBuffer()->Fill(0);
		}
	}

	set_our_eip(4);

	// Stage: UI overlay
	if (_GP(play).screen_is_faded_out == 0) {
		construct_ui_view();
	}

	// End the parent scene node
	_G(gfxDriver)->EndSpriteBatch();
}

void construct_game_screen_overlay(bool draw_mouse) {
	_G(gfxDriver)->BeginSpriteBatch(_GP(play).GetMainViewport(),
									_GP(play).GetGlobalTransform(_G(drawstate).FullFrameRedraw),
									(GraphicFlip)_GP(play).screen_flipped);
	if (pl_any_want_hook(AGSE_POSTSCREENDRAW)) {
		_G(gfxDriver)->DrawSprite(AGSE_POSTSCREENDRAW, 0, nullptr);
	}

	// Mouse cursor
	if (_GP(play).screen_is_faded_out == 0) {
		if (draw_mouse && !_GP(play).mouse_cursor_hidden) {
			// Exclusive sub-batch for mouse cursor, to let filter it out (CHECKME later?)
			_G(gfxDriver)->BeginSpriteBatch(Rect(), SpriteTransform(), kFlip_None, nullptr, RENDER_BATCH_MOUSE_CURSOR);
			_G(gfxDriver)->DrawSprite(_G(mousex) - _G(hotx), _G(mousey) - _G(hoty), _G(mouseCursor));
			invalidate_sprite(_G(mousex) - _G(hotx), _G(mousey) - _G(hoty), _G(mouseCursor), false);
			_G(gfxDriver)->EndSpriteBatch();
		}
	}
	// Full screen tint fx, covers everything except for fade fx(?) and engine overlay
	if ((_GP(play).screen_tint >= 1) && (_GP(play).screen_is_faded_out == 0))
		_G(gfxDriver)->SetScreenTint(_GP(play).screen_tint & 0xff, (_GP(play).screen_tint >> 8) & 0xff, (_GP(play).screen_tint >> 16) & 0xff);
	_G(gfxDriver)->EndSpriteBatch();

	// For hardware-accelerated renderers: legacy letterbox and global screen fade effect
	if (_G(drawstate).FullFrameRedraw) {
		_G(gfxDriver)->BeginSpriteBatch(_GP(play).GetMainViewport(), SpriteTransform());
		// Stage: legacy letterbox mode borders
		if (_GP(play).screen_is_faded_out == 0)
			render_black_borders();
		// Stage: full screen fade fx
		if (_GP(play).screen_is_faded_out != 0)
			_G(gfxDriver)->SetScreenFade(_GP(play).fade_to_red, _GP(play).fade_to_green, _GP(play).fade_to_blue);
		_G(gfxDriver)->EndSpriteBatch();
	}
}

void construct_engine_overlay() {
	const Rect &viewport = RectWH(_GP(game).GetGameRes());
	_G(gfxDriver)->BeginSpriteBatch(viewport, SpriteTransform(), kFlip_None, nullptr, RENDER_BATCH_ENGINE_OVERLAY);

	if (_G(display_fps) != kFPS_Hide)
		draw_fps(viewport);

	_G(gfxDriver)->EndSpriteBatch();
}

static void update_shakescreen() {
	// TODO: unify blocking and non-blocking shake update
	_GP(play).shake_screen_yoff = 0;
	if (_GP(play).shakesc_length > 0) {
		if ((_G(loopcounter) % _GP(play).shakesc_delay) < (_GP(play).shakesc_delay / 2))
			_GP(play).shake_screen_yoff = _GP(play).shakesc_amount;
	}
}

void debug_draw_room_mask(RoomAreaMask mask) {
	_G(debugRoomMask) = mask;
	if (mask == kRoomAreaNone)
		return;

	Bitmap *bmp;
	switch (mask) {
	case kRoomAreaHotspot: bmp = _GP(thisroom).HotspotMask.get(); break;
	case kRoomAreaWalkBehind: bmp = _GP(thisroom).WalkBehindMask.get(); break;
	case kRoomAreaWalkable: bmp = prepare_walkable_areas(-1); break;
	case kRoomAreaRegion: bmp = _GP(thisroom).RegionMask.get(); break;
	default: return;
	}

	// Software mode scaling
	// note we don't use transparency in software mode - may be slow in hi-res games
	if (_G(drawstate).SoftwareRender &&
		(mask != kRoomAreaWalkBehind) &&
		(bmp->GetSize() != Size(_GP(thisroom).Width, _GP(thisroom).Height))) {
		recycle_bitmap(_GP(debugRoomMaskObj).Bmp,
			bmp->GetColorDepth(), _GP(thisroom).Width, _GP(thisroom).Height);
		_GP(debugRoomMaskObj).Bmp->StretchBlt(bmp, RectWH(0, 0, _GP(thisroom).Width, _GP(thisroom).Height));
		bmp = _GP(debugRoomMaskObj).Bmp.get();
	}

	_GP(debugRoomMaskObj).Ddb = recycle_ddb_bitmap(_GP(debugRoomMaskObj).Ddb, bmp, false, true);
	_GP(debugRoomMaskObj).Ddb->SetAlpha(150);
	_GP(debugRoomMaskObj).Ddb->SetStretch(_GP(thisroom).Width, _GP(thisroom).Height);
}

void debug_draw_movelist(int charnum) {
	_G(debugMoveListChar) = charnum;
}

void update_room_debug() {
	if (_G(debugRoomMask) == kRoomAreaWalkable) {
		Bitmap *bmp = prepare_walkable_areas(-1);
		// Software mode scaling
		if (_G(drawstate).SoftwareRender && (_GP(thisroom).MaskResolution > 1)) {
			recycle_bitmap(_GP(debugRoomMaskObj).Bmp,
				bmp->GetColorDepth(), _GP(thisroom).Width, _GP(thisroom).Height);
			_GP(debugRoomMaskObj).Bmp->StretchBlt(bmp, RectWH(0, 0, _GP(thisroom).Width, _GP(thisroom).Height));
			bmp = _GP(debugRoomMaskObj).Bmp.get();
		}
		_GP(debugRoomMaskObj).Ddb = recycle_ddb_bitmap(_GP(debugRoomMaskObj).Ddb, bmp, false, true);
		_GP(debugRoomMaskObj).Ddb->SetAlpha(150);
		_GP(debugRoomMaskObj).Ddb->SetStretch(_GP(thisroom).Width, _GP(thisroom).Height);
	}
	if (_G(debugMoveListChar) >= 0) {
		const int mult = _G(drawstate).SoftwareRender ? 1 : _GP(thisroom).MaskResolution;
		if (_G(drawstate).SoftwareRender)
			recycle_bitmap(_GP(debugMoveListObj).Bmp, _GP(game).GetColorDepth(),
				_GP(thisroom).Width, _GP(thisroom).Height, true);
		else
			recycle_bitmap(_GP(debugMoveListObj).Bmp, _GP(game).GetColorDepth(),
				_GP(thisroom).WalkAreaMask->GetWidth(), _GP(thisroom).WalkAreaMask->GetHeight(), true);

		if (_GP(game).chars[_G(debugMoveListChar)].walking > 0) {
			int mlsnum = _GP(game).chars[_G(debugMoveListChar)].walking;
			if (_GP(game).chars[_G(debugMoveListChar)].walking >= TURNING_AROUND)
				mlsnum %= TURNING_AROUND;
			const MoveList &cmls = _GP(mls)[mlsnum];
			for (int i = 0; i < cmls.numstage - 1; i++) {
				short srcx = cmls.pos[i].X;
				short srcy = cmls.pos[i].Y;
				short targetx = cmls.pos[i + 1].X;
				short targety = cmls.pos[i + 1].Y;
				_GP(debugMoveListObj).Bmp->DrawLine(Line(srcx / mult, srcy / mult, targetx / mult, targety / mult),
					MakeColor(i + 1));
			}
		}
		sync_object_texture(_GP(debugMoveListObj));
		_GP(debugMoveListObj).Ddb->SetAlpha(150);
		_GP(debugMoveListObj).Ddb->SetStretch(_GP(thisroom).Width, _GP(thisroom).Height);
	}
}

// Draw everything
void render_graphics(IDriverDependantBitmap *extraBitmap, int extraX, int extraY) {
	// Don't render if skipping cutscene
	if (_GP(play).fast_forward)
		return;
	// Don't render if we've just entered new room and are before fade-in
	// TODO: find out why this is not skipped for 8-bit games
	if ((_G(in_new_room) > 0) & (_GP(game).color_depth > 1))
		return;

	// TODO: find out if it's okay to move shake to update function
	update_shakescreen();

	construct_game_scene(false);
	set_our_eip(5);
	// TODO: extraBitmap is a hack, used to place an additional gui element
	// on top of the screen. Normally this should be a part of the game UI stage.
	if (extraBitmap != nullptr) {
		_G(gfxDriver)->BeginSpriteBatch(_GP(play).GetMainViewport(), _GP(play).GetGlobalTransform(_G(drawstate).FullFrameRedraw),
										(GraphicFlip)_GP(play).screen_flipped);
		invalidate_sprite(extraX, extraY, extraBitmap, false);
		_G(gfxDriver)->DrawSprite(extraX, extraY, extraBitmap);
		_G(gfxDriver)->EndSpriteBatch();
	}
	construct_game_screen_overlay(true);
	render_to_screen();

	if (!SHOULD_QUIT && !_GP(play).screen_is_faded_out) {
		// always update the palette, regardless of whether the plugin
		// vetos the screen update
		if (_G(bg_just_changed)) {
			setpal();
			_G(bg_just_changed) = 0;
		}
	}

	_G(drawstate).ScreenIsDirty = false;
}

} // namespace AGS3
