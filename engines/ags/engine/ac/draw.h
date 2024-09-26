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

#ifndef AGS_ENGINE_AC_DRAW_H
#define AGS_ENGINE_AC_DRAW_H

#include "common/std/memory.h"
#include "ags/shared/core/types.h"
#include "ags/shared/ac/common_defines.h"
#include "ags/shared/gfx/gfx_def.h"
#include "ags/shared/gfx/allegro_bitmap.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/game/room_struct.h"
#include "ags/engine/ac/runtime_defines.h"

namespace AGS3 {
namespace AGS {
namespace Shared {
typedef std::shared_ptr<Shared::Bitmap> PBitmap;
} // namespace Shared

namespace Engine {
class IDriverDependantBitmap;
} // namespace Engine
} // namespace AGS

using namespace AGS; // FIXME later

#define IS_ANTIALIAS_SPRITES _GP(usetup).enable_antialiasing && (_GP(play).disable_antialiasing == 0)

// Render stage flags, for filtering out certain elements
// during room transitions, capturing screenshots, etc.
// NOTE: these values are internal and purely arbitrary atm.
#define RENDER_BATCH_ENGINE_OVERLAY 0x0001
#define RENDER_BATCH_MOUSE_CURSOR   0x0002
#define RENDER_SHOT_SKIP_ON_FADE    (RENDER_BATCH_ENGINE_OVERLAY | RENDER_BATCH_MOUSE_CURSOR)

/**
 * Buffer and info flags for viewport/camera pairs rendering in software mode
 */
struct RoomCameraDrawData {
	// Intermediate bitmap for the software drawing method.
	// We use this bitmap in case room camera has scaling enabled, we draw dirty room rects on it,
	// and then pass to software renderer which draws sprite on top and then either blits or stretch-blits
	// to the virtual screen.
	// For more details see comment in ALSoftwareGraphicsDriver::RenderToBackBuffer().
	AGS::Shared::PBitmap Buffer;      // this is the actual bitmap
	AGS::Shared::PBitmap Frame;       // this is either same bitmap reference or sub-bitmap of virtual screen
	bool    IsOffscreen; // whether room viewport was offscreen (cannot use sub-bitmap)
	bool    IsOverlap;   // whether room viewport overlaps any others (marking dirty rects is complicated)
};

// ObjTexture is a helper struct that pairs a raw bitmap with
// a renderer's texture and an optional position
struct ObjTexture {
	// Sprite ID
	uint32_t SpriteID = UINT32_MAX;
	// Raw bitmap; used for software render mode,
	// or when particular object types require generated image.
	std::unique_ptr<Shared::Bitmap> Bmp;
	// Corresponding texture, created by renderer
	Engine::IDriverDependantBitmap *Ddb = nullptr;
	// Sprite's position
	Point Pos;
	// Texture's offset, *relative* to the logical sprite's position;
	// may be used in case the texture's size is different for any reason
	Point Off;

	ObjTexture() = default;
	ObjTexture(uint32_t sprite_id, Shared::Bitmap *bmp, Engine::IDriverDependantBitmap *ddb, int x, int y, int xoff = 0, int yoff = 0)
		: SpriteID(sprite_id), Bmp(bmp), Ddb(ddb), Pos(x, y), Off(xoff, yoff) {
	}
	ObjTexture(ObjTexture &&o);
	~ObjTexture();

	ObjTexture &operator =(ObjTexture &&o);
};

// ObjectCache stores cached object data, used to determine
// if active sprite / texture should be reconstructed
struct ObjectCache {
	std::unique_ptr<AGS::Shared::Bitmap> image;
	bool  in_use = false;  // CHECKME: possibly may be removed
	int   sppic = 0;
	short tintr = 0, tintg = 0, tintb = 0, tintamnt = 0, tintlight = 0;
	short lightlev = 0, zoom = 0;
	bool  mirrored = 0;
	int   x = 0, y = 0;

	ObjectCache() = default;
	ObjectCache(int pic_, int tintr_, int tintg_, int tintb_, int tint_amnt_, int tint_light_,
				int light_, int zoom_, bool mirror_, int posx_, int posy_)
		: sppic(pic_), tintr(tintr_), tintg(tintg_), tintb(tintb_), tintamnt(tint_amnt_), tintlight(tint_light_)
		, lightlev(light_), zoom(zoom_), mirrored(mirror_), x(posx_), y(posy_) {}
};

// Converts AGS color index to the actual bitmap color using game's color depth
int MakeColor(int color_index);

class Viewport;
class Camera;

// Initializes drawing methods and optimisation
void init_draw_method();
// Initializes global game drawing resources
void init_game_drawdata();
// Initializes drawing resources upon entering new room
void init_room_drawdata();
// Disposes resources related to the current drawing methods
void dispose_draw_method();
// Disposes global game drawing resources
void dispose_game_drawdata();
// Disposes any temporary resources on leaving current room
void dispose_room_drawdata();
// Releases all the cached textures of game objects
void clear_drawobj_cache();
// Updates drawing settings depending on main viewport's size and position on screen
void on_mainviewport_changed();
// Notifies that a new room viewport was created
void on_roomviewport_created(int index);
// Notifies that a new room viewport was deleted
void on_roomviewport_deleted(int index);
// Updates drawing settings if room viewport's position or size has changed
void on_roomviewport_changed(Viewport *view);
// Detects overlapping viewports, starting from the given index in z-sorted array
void detect_roomviewport_overlaps(size_t z_index);
// Updates drawing settings if room camera's size has changed
void on_roomcamera_changed(Camera *cam);
// Marks particular object as need to update the texture
void mark_object_changed(int objid);
// Resets all object caches which reference this sprite
void reset_objcache_for_sprite(int sprnum, bool deleted);
// TODO: write a generic drawable/objcache system where each object
// allocates a drawable for itself, and disposes one if being removed.
void reset_drawobj_for_overlay(int objnum);

// whether there are currently remnants of a DisplaySpeech
void mark_screen_dirty();
bool is_screen_dirty();

// marks whole screen as needing a redraw
void invalidate_screen();
// marks all the camera frame as needing a redraw
void invalidate_camera_frame(int index);
// marks certain rectangle on screen as needing a redraw
// in_room flag tells how to interpret the coordinates: as in-room coords or screen viewport coordinates.
void invalidate_rect(int x1, int y1, int x2, int y2, bool in_room);

void mark_current_background_dirty();

// Avoid freeing and reallocating the memory if possible
Shared::Bitmap *recycle_bitmap(Shared::Bitmap *bimp, int coldep, int wid, int hit, bool make_transparent = false);
void recycle_bitmap(std::unique_ptr<Shared::Bitmap> &bimp, int coldep, int wid, int hit, bool make_transparent = false);
Engine::IDriverDependantBitmap* recycle_ddb_sprite(Engine::IDriverDependantBitmap *ddb, uint32_t sprite_id, Shared::Bitmap *source, bool has_alpha = false, bool opaque = false);
inline Engine::IDriverDependantBitmap* recycle_ddb_bitmap(Engine::IDriverDependantBitmap *ddb, Shared::Bitmap *source, bool has_alpha = false, bool opaque = false) {
	return recycle_ddb_sprite(ddb, UINT32_MAX, source, has_alpha, opaque);
}
// Draw everything
void render_graphics(Engine::IDriverDependantBitmap *extraBitmap = nullptr, int extraX = 0, int extraY = 0);
// Construct game scene, scheduling drawing list for the renderer
void construct_game_scene(bool full_redraw = false);
// Construct final game screen elements; updates and draws mouse cursor
void construct_game_screen_overlay(bool draw_mouse = true);
// Construct engine overlay with debugging tools (fps, console)
void construct_engine_overlay();
// Clears black game borders in legacy letterbox mode
void clear_letterbox_borders();

void debug_draw_room_mask(RoomAreaMask mask);
void debug_draw_movelist(int charnum);
void update_room_debug();

void tint_image(Shared::Bitmap *g, Shared::Bitmap *source, int red, int grn, int blu, int light_level, int luminance = 255);
void draw_sprite_support_alpha(Shared::Bitmap *ds, bool ds_has_alpha, int xpos, int ypos, Shared::Bitmap *image, bool src_has_alpha,
                               Shared::BlendMode blend_mode = Shared::kBlendMode_Alpha, int alpha = 0xFF);
void draw_sprite_slot_support_alpha(Shared::Bitmap *ds, bool ds_has_alpha, int xpos, int ypos, int src_slot,
                                    Shared::BlendMode blend_mode = Shared::kBlendMode_Alpha, int alpha = 0xFF);
void draw_gui_sprite(Shared::Bitmap *ds, int pic, int x, int y, bool use_alpha = true, Shared::BlendMode blend_mode = Shared::kBlendMode_Alpha);
void draw_gui_sprite_v330(Shared::Bitmap *ds, int pic, int x, int y, bool use_alpha = true, Shared::BlendMode blend_mode = Shared::kBlendMode_Alpha);
void draw_gui_sprite(Shared::Bitmap *ds, bool use_alpha, int xpos, int ypos,
	Shared::Bitmap *image, bool src_has_alpha, Shared::BlendMode blend_mode = Shared::kBlendMode_Alpha, int alpha = 0xFF);

// Render game on screen
void render_to_screen();
// Callbacks for the graphics driver
void draw_game_screen_callback();
void GfxDriverOnInitCallback(void *data);
bool GfxDriverSpriteEvtCallback(int evt, int data);
void putpixel_compensate(Shared::Bitmap *g, int xx, int yy, int col);
// Create the actsps[objid] image with the object drawn correctly.
// Returns true if nothing at all has changed and actsps is still
// intact from last time; false otherwise.
// Hardware-accelerated do not require altering the raw bitmap itself,
// so they only detect whether the sprite ID itself has changed.
// Software renderers modify the cached bitmap whenever any visual
// effect changes (scaling, tint, etc).
// * force_software option forces HW renderers to  construct the image
// in software mode as well.
bool construct_object_gfx(int objid, bool force_software);
bool construct_char_gfx(int charid, bool force_software);
// Returns a cached character image prepared for the render
Shared::Bitmap *get_cached_character_image(int charid);
// Returns a cached object image prepared for the render
Shared::Bitmap *get_cached_object_image(int objid);
// Adds a walk-behind sprite to the list for the given slot
// (reuses existing texture if possible)
void add_walkbehind_image(size_t index, Shared::Bitmap *bmp, int x, int y);

void draw_and_invalidate_text(Shared::Bitmap *ds, int x1, int y1, int font, color_t text_color, const char *text);

void setpal();

// These functions are converting coordinates between data resolution and
// game resolution units. The first are units used by game data and script,
// and second define the game's screen resolution, sprite and font sizes.
// This conversion is done before anything else (like moving from room to
// viewport on screen, or scaling game further in the window by the graphic
// renderer).
int get_fixed_pixel_size(int pixels);
// coordinate conversion data,script ---> final game resolution
extern int data_to_game_coord(int coord);
extern void data_to_game_coords(int *x, int *y);
extern void data_to_game_round_up(int *x, int *y);
// coordinate conversion final game resolution ---> data,script
extern int game_to_data_coord(int coord);
extern void game_to_data_coords(int &x, int &y);
extern int game_to_data_round_up(int coord);
// convert contextual data coordinates to final game resolution
extern void ctx_data_to_game_coord(int &x, int &y, bool hires_ctx);
extern void ctx_data_to_game_size(int &x, int &y, bool hires_ctx);
extern int ctx_data_to_game_size(int size, bool hires_ctx);
extern int game_to_ctx_data_size(int size, bool hires_ctx);
// This function converts game coordinates coming from script to the actual game resolution.
extern void defgame_to_finalgame_coords(int &x, int &y);

// Creates bitmap of a format compatible with the gfxdriver;
// if col_depth is 0, uses game's native color depth.
Shared::Bitmap *CreateCompatBitmap(int width, int height, int col_depth = 0);
// Checks if the bitmap is compatible with the gfxdriver;
// returns same bitmap or its copy of a compatible format.
Shared::Bitmap *ReplaceBitmapWithSupportedFormat(Shared::Bitmap *bitmap);
// Checks if the bitmap needs any kind of adjustments before it may be used
// in AGS sprite operations. Also handles number of certain special cases
// (old systems or uncommon gfx modes, and similar stuff).
// Original bitmap **gets deleted** if a new bitmap had to be created.
Shared::Bitmap *PrepareSpriteForUse(Shared::Bitmap *bitmap, bool has_alpha);
// Same as above, but compatible for std::shared_ptr.
Shared::PBitmap PrepareSpriteForUse(Shared::PBitmap bitmap, bool has_alpha);
// Makes a screenshot corresponding to the last screen render and returns it as a bitmap
// of the requested width and height and game's native color depth.
Shared::Bitmap *CopyScreenIntoBitmap(int width, int height, const Rect *src_rect = nullptr,
									 bool at_native_res = false, uint32_t batch_skip_filter = 0u);

} // namespace AGS3

#endif
