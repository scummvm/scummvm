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

#include "common/lua/lauxlib.h"

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/files/u6_lib_n.h"
#include "ultima/nuvie/files/nuvie_io.h"
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/files/u6_lzw.h"
#include "ultima/nuvie/misc/u6_line_walker.h"
#include "ultima/nuvie/screen/game_palette.h"
#include "ultima/nuvie/sound/sound_manager.h"
#include "ultima/nuvie/fonts/font.h"
#include "ultima/nuvie/fonts/wou_font.h"
#include "ultima/nuvie/core/cursor.h"
#include "ultima/nuvie/keybinding/keys.h"
#include "ultima/nuvie/script/script_cutscene.h"
#include "common/system.h"

namespace Ultima {
namespace Nuvie {

#define DELUXE_PAINT_MAGIC 0x4d524f46 // "FORM"

#define INPUT_KEY_RIGHT 79 | (1<<30)
#define INPUT_KEY_LEFT 80 | (1<<30)
#define INPUT_KEY_DOWN  81 | (1<<30)
#define INPUT_KEY_UP  82 | (1<<30)

static ScriptCutscene *cutScene = NULL;
ScriptCutscene *get_cutscene() {
	return cutScene;
}

static int nscript_image_set(lua_State *L);
static int nscript_image_get(lua_State *L);

bool nscript_new_image_var(lua_State *L, CSImage *image);
static int nscript_image_gc(lua_State *L);

static sint32 nscript_dec_image_ref_count(CSImage *image);

static int nscript_image_new(lua_State *L);
static int nscript_image_new_starfield(lua_State *L);
static int nscript_image_copy(lua_State *L);
static int nscript_image_load(lua_State *L);
static int nscript_image_load_all(lua_State *L);
static int nscript_image_print(lua_State *L);
static int nscript_image_draw_line(lua_State *L);
static int nscript_image_blit(lua_State *L);
static int nscript_image_update_effect(lua_State *L);
static int nscript_image_static(lua_State *L);
static int nscript_image_set_transparency_colour(lua_State *L);
static int nscript_image_bubble_effect_add_color(lua_State *L);
static int nscript_image_bubble_effect(lua_State *L);

static const struct luaL_Reg nscript_imagelib_m[] = {
	{ "__index", nscript_image_get },
	{ "__newindex", nscript_image_set },
	{ "__gc", nscript_image_gc },
	{ NULL, NULL }
};

static int nscript_sprite_set(lua_State *L);
static int nscript_sprite_get(lua_State *L);

bool nscript_new_sprite_var(lua_State *L, CSSprite *image);
static int nscript_sprite_gc(lua_State *L);

static const struct luaL_Reg nscript_spritelib_m[] = {
	{ "__index", nscript_sprite_get },
	{ "__newindex", nscript_sprite_set },
	{ "__gc", nscript_sprite_gc },
	{ NULL, NULL }
};

static int nscript_sprite_new(lua_State *L);
static int nscript_sprite_move_to_front(lua_State *L);

static int nscript_text_load(lua_State *L);
static int nscript_midgame_load(lua_State *L);

static int nscript_canvas_set_bg_color(lua_State *L);
static int nscript_canvas_set_palette(lua_State *L);
static int nscript_canvas_set_palette_entry(lua_State *L);
static int nscript_canvas_rotate_palette(lua_State *L);
static int nscript_canvas_set_update_interval(lua_State *L);
static int nscript_canvas_set_opacity(lua_State *L);
static int nscript_canvas_set_solid_bg(lua_State *L);
static int nscript_canvas_update(lua_State *L);
static int nscript_canvas_show(lua_State *L);
static int nscript_canvas_hide(lua_State *L);
static int nscript_canvas_hide_all_sprites(lua_State *L);
static int nscript_canvas_string_length(lua_State *L);
static int nscript_canvas_rotate_game_palette(lua_State *L);

static int nscript_music_play(lua_State *L);
static int nscript_music_stop(lua_State *L);
static int nscript_get_mouse_x(lua_State *L);
static int nscript_get_mouse_y(lua_State *L);
static int nscript_input_poll(lua_State *L);

static int nscript_config_set(lua_State *L);

static int nscript_engine_should_quit(lua_State *L);

void nscript_init_cutscene(lua_State *L, Configuration *cfg, GUI *gui, SoundManager *sm) {
	cutScene = new ScriptCutscene(gui, cfg, sm);

	luaL_newmetatable(L, "nuvie.Image");
	luaL_register(L, NULL, nscript_imagelib_m);

	luaL_newmetatable(L, "nuvie.Sprite");
	luaL_register(L, NULL, nscript_spritelib_m);

	lua_pushcfunction(L, nscript_image_new);
	lua_setglobal(L, "image_new");

	lua_pushcfunction(L, nscript_image_new_starfield);
	lua_setglobal(L, "image_new_starfield");

	lua_pushcfunction(L, nscript_image_copy);
	lua_setglobal(L, "image_copy");

	lua_pushcfunction(L, nscript_image_load);
	lua_setglobal(L, "image_load");

	lua_pushcfunction(L, nscript_image_load_all);
	lua_setglobal(L, "image_load_all");

	lua_pushcfunction(L, nscript_image_print);
	lua_setglobal(L, "image_print");

	lua_pushcfunction(L, nscript_image_static);
	lua_setglobal(L, "image_static");

	lua_pushcfunction(L, nscript_image_set_transparency_colour);
	lua_setglobal(L, "image_set_transparency_colour");

	lua_pushcfunction(L, nscript_image_update_effect);
	lua_setglobal(L, "image_update_effect");

	lua_pushcfunction(L, nscript_sprite_new);
	lua_setglobal(L, "sprite_new");

	lua_pushcfunction(L, nscript_sprite_move_to_front);
	lua_setglobal(L, "sprite_move_to_front");

	lua_pushcfunction(L, nscript_image_bubble_effect_add_color);
	lua_setglobal(L, "image_bubble_effect_add_color");

	lua_pushcfunction(L, nscript_image_bubble_effect);
	lua_setglobal(L, "image_bubble_effect");

	lua_pushcfunction(L, nscript_image_draw_line);
	lua_setglobal(L, "image_draw_line");

	lua_pushcfunction(L, nscript_image_blit);
	lua_setglobal(L, "image_blit");

	lua_pushcfunction(L, nscript_text_load);
	lua_setglobal(L, "text_load");

	lua_pushcfunction(L, nscript_midgame_load);
	lua_setglobal(L, "midgame_load");

	lua_pushcfunction(L, nscript_canvas_set_bg_color);
	lua_setglobal(L, "canvas_set_bg_color");

	lua_pushcfunction(L, nscript_canvas_set_palette);
	lua_setglobal(L, "canvas_set_palette");

	lua_pushcfunction(L, nscript_canvas_set_palette_entry);
	lua_setglobal(L, "canvas_set_palette_entry");

	lua_pushcfunction(L, nscript_canvas_rotate_palette);
	lua_setglobal(L, "canvas_rotate_palette");

	lua_pushcfunction(L, nscript_canvas_set_update_interval);
	lua_setglobal(L, "canvas_set_update_interval");

	lua_pushcfunction(L, nscript_canvas_set_solid_bg);
	lua_setglobal(L, "canvas_set_solid_bg");

	lua_pushcfunction(L, nscript_canvas_set_opacity);
	lua_setglobal(L, "canvas_set_opacity");

	lua_pushcfunction(L, nscript_canvas_update);
	lua_setglobal(L, "canvas_update");

	lua_pushcfunction(L, nscript_canvas_show);
	lua_setglobal(L, "canvas_show");

	lua_pushcfunction(L, nscript_canvas_hide);
	lua_setglobal(L, "canvas_hide");

	lua_pushcfunction(L, nscript_canvas_hide_all_sprites);
	lua_setglobal(L, "canvas_hide_all_sprites");

	lua_pushcfunction(L, nscript_canvas_string_length);
	lua_setglobal(L, "canvas_string_length");

	lua_pushcfunction(L, nscript_canvas_rotate_game_palette);
	lua_setglobal(L, "canvas_rotate_game_palette");

	lua_pushcfunction(L, nscript_music_play);
	lua_setglobal(L, "music_play");

	lua_pushcfunction(L, nscript_music_stop);
	lua_setglobal(L, "music_stop");

	lua_pushcfunction(L, nscript_get_mouse_x);
	lua_setglobal(L, "get_mouse_x");

	lua_pushcfunction(L, nscript_get_mouse_y);
	lua_setglobal(L, "get_mouse_y");

	lua_pushcfunction(L, nscript_input_poll);
	lua_setglobal(L, "input_poll");

	lua_pushcfunction(L, nscript_config_set);
	lua_setglobal(L, "config_set");

	lua_pushcfunction(L, nscript_engine_should_quit);
	lua_setglobal(L, "engine_should_quit");
}

bool nscript_new_image_var(lua_State *L, CSImage *image) {
	CSImage **userdata;

	userdata = (CSImage **)lua_newuserdata(L, sizeof(CSImage *));

	luaL_getmetatable(L, "nuvie.Image");
	lua_setmetatable(L, -2);

	*userdata = image;

	if (image)
		image->refcount++;

	return true;
}

CSImage *nscript_get_image_from_args(lua_State *L, int lua_stack_offset) {
	CSImage **s_image = (CSImage **)luaL_checkudata(L, lua_stack_offset, "nuvie.Image");
	if (s_image == NULL)
		return NULL;

	return *s_image;
}

static int nscript_image_set(lua_State *L) {
	CSImage **s_image;
	CSImage *image;
	const char *key;

	s_image = (CSImage **)lua_touserdata(L, 1);
	if (s_image == NULL)
		return 0;

	image = *s_image;
	if (image == NULL)
		return 0;

	key = lua_tostring(L, 2);

	if (!strcmp(key, "scale")) {
		image->setScale((uint16)lua_tointeger(L, 3));
		return 0;
	}

	return 0;
}


static int nscript_image_get(lua_State *L) {
	CSImage **s_image;
	CSImage *image;
	const char *key;

	s_image = (CSImage **)lua_touserdata(L, 1);
	if (s_image == NULL)
		return 0;

	image = *s_image;
	if (image == NULL)
		return 0;

	key = lua_tostring(L, 2);

	if (!strcmp(key, "w")) {
		uint16 w, h;
		image->shp->get_size(&w, &h);
		lua_pushinteger(L, w);
		return 1;
	}

	if (!strcmp(key, "h")) {
		uint16 w, h;
		image->shp->get_size(&w, &h);
		lua_pushinteger(L, h);
		return 1;
	}

	if (!strcmp(key, "scale")) {
		lua_pushinteger(L, image->getScale());
		return 1;
	}

	return 0;
}

static sint32 nscript_dec_image_ref_count(CSImage *image) {
	if (image == NULL)
		return -1;

	image->refcount--;

	return image->refcount;
}

static int nscript_image_gc(lua_State *L) {
	//DEBUG(0, LEVEL_INFORMATIONAL, "\nImage garbage Collection!\n");

	CSImage **p_image = (CSImage **)lua_touserdata(L, 1);
	CSImage *image;

	if (p_image == NULL)
		return false;

	image = *p_image;

	if (nscript_dec_image_ref_count(image) == 0) {
		if (image->shp)
			delete image->shp;
		delete image;
	}

	return 0;
}

static int nscript_image_new(lua_State *L) {
	uint16 width = lua_tointeger(L, 1);
	uint16 height = lua_tointeger(L, 2);

	U6Shape *shp = new U6Shape();
	if (shp->init(width, height) == false)
		return 0;

	if (lua_gettop(L) >= 3)
		shp->fill((uint8)lua_tointeger(L, 3));

	CSImage *image = new CSImage(shp);

	nscript_new_image_var(L, image);
	return 1;
}

static int nscript_image_new_starfield(lua_State *L) {
	uint16 width = lua_tointeger(L, 1);
	uint16 height = lua_tointeger(L, 2);

	U6Shape *shp = new U6Shape();
	if (shp->init(width, height) == false)
		return 0;

	CSStarFieldImage *image = new CSStarFieldImage(shp);

	nscript_new_image_var(L, image);
	return 1;
}

static int nscript_image_copy(lua_State *L) {
	CSImage *img = nscript_get_image_from_args(L, 1);
	uint16 w, h;

	U6Shape *shp = new U6Shape();
	img->shp->get_size(&w, &h);
	if (shp->init(w, h) == false) {
		return 0;
	}
	shp->blit(img->shp, 0, 0);

	CSImage *image = new CSImage(shp);

	nscript_new_image_var(L, image);
	return 1;
}

static int nscript_image_load(lua_State *L) {
	const char *filename = lua_tostring(L, 1);
	int idx = -1;
	int sub_idx = 0;

	if (lua_gettop(L) >= 2)
		idx = lua_tointeger(L, 2);

	if (lua_gettop(L) >= 3)
		sub_idx = lua_tointeger(L, 3);

	CSImage *image = cutScene->load_image(filename, idx, sub_idx);

	if (image) {
		nscript_new_image_var(L, image);
		return 1;
	}

	return 0;
}

static int nscript_image_load_all(lua_State *L) {
	const char *filename = lua_tostring(L, 1);
	Std::vector<Std::vector<CSImage *> > images = cutScene->load_all_images(filename);

	if (images.empty()) {
		return 0;
	}

	lua_newtable(L);

	for (uint16 i = 0; i < images.size(); i++) {
		lua_pushinteger(L, i);
		if (images[i].size() > 1) {
			lua_newtable(L);
			for (uint16 j = 0; j < images[i].size(); j++) {
				lua_pushinteger(L, j);
				nscript_new_image_var(L, images[i][j]);
				lua_settable(L, -3);
			}
		} else {
			nscript_new_image_var(L, images[i][0]);
		}
		lua_settable(L, -3);
	}

	return 1;
}

static int nscript_image_print(lua_State *L) {
	CSImage *img = nscript_get_image_from_args(L, 1);
	const char *text = lua_tostring(L, 2);
	uint16 startx = lua_tointeger(L, 3);
	uint16 width = lua_tointeger(L, 4);
	uint16 x = lua_tointeger(L, 5);
	uint16 y = lua_tointeger(L, 6);
	uint8 color = lua_tointeger(L, 7);

	cutScene->print_text(img, text, &x, &y, startx, width, color);

	lua_pushinteger(L, x);
	lua_pushinteger(L, y);

	return 2;
}

static int nscript_image_draw_line(lua_State *L) {
	CSImage *img = nscript_get_image_from_args(L, 1);
	uint16 sx = lua_tointeger(L, 2);
	uint16 sy = lua_tointeger(L, 3);
	uint16 ex = lua_tointeger(L, 4);
	uint16 ey = lua_tointeger(L, 5);
	uint8 color = lua_tointeger(L, 6);

	if (img) {
		img->shp->draw_line(sx, sy, ex, ey, color);
	}

	return 0;
}

static int nscript_image_blit(lua_State *L) {
	CSImage *dest_img = nscript_get_image_from_args(L, 1);
	CSImage *src_img = nscript_get_image_from_args(L, 2);
	uint16 x = lua_tointeger(L, 3);
	uint16 y = lua_tointeger(L, 4);

	if (dest_img && src_img) {
		dest_img->shp->blit(src_img->shp, x, y);
	}

	return 0;
}
static int nscript_image_update_effect(lua_State *L) {
	CSImage *img = nscript_get_image_from_args(L, 1);
	if (img) {
		img->updateEffect();
	}

	return 0;
}

static uint8 u6_flask_num_colors = 0;
static uint8 u6_flask_liquid_colors[8];

static int nscript_image_bubble_effect_add_color(lua_State *L) {
	if (u6_flask_num_colors != 8) {
		u6_flask_liquid_colors[u6_flask_num_colors++] = lua_tointeger(L, 1);
	}

	return 0;
}

static int nscript_image_bubble_effect(lua_State *L) {
	CSImage *img = nscript_get_image_from_args(L, 1);

	if (img && u6_flask_num_colors > 0) {
		unsigned char *data = img->shp->get_data();
		uint16 w, h;
		img->shp->get_size(&w, &h);

		for (int i = 0; i < w * h; i++) {
			if (*data != 0xff) {
				*data = u6_flask_liquid_colors[NUVIE_RAND() % u6_flask_num_colors];
			}

			data++;
		}

	}
	return 0;
}

static int nscript_image_set_transparency_colour(lua_State *L) {
	CSImage *img = nscript_get_image_from_args(L, 1);
	uint8 pal_index = lua_tointeger(L, 2);
	if (img) {
		unsigned char *data = img->shp->get_data();
		uint16 w, h;
		img->shp->get_size(&w, &h);
		for (int i = 0; i < w * h; i++) {
			if (data[i] == pal_index)
				data[i] = 0xff;
		}
	}
	return 0;
}

static int nscript_image_static(lua_State *L) {
	CSImage *img = nscript_get_image_from_args(L, 1);

	if (img) {
		unsigned char *data = img->shp->get_data();
		uint16 w, h;
		img->shp->get_size(&w, &h);
		memset(data, 16, w * h);
		for (int i = 0; i < 1000; i++) {
			data[NUVIE_RAND() % (w * h)] = 0;
		}
	}
	return 0;
}

CSSprite *nscript_get_sprite_from_args(lua_State *L, int lua_stack_offset) {
	CSSprite **s_sprite;
	CSSprite *sprite;

	s_sprite = (CSSprite **)lua_touserdata(L, 1);
	if (s_sprite == NULL)
		return NULL;

	sprite = *s_sprite;
	return sprite;
}

bool nscript_new_sprite_var(lua_State *L, CSSprite *sprite) {
	CSSprite **userdata;

	userdata = (CSSprite **)lua_newuserdata(L, sizeof(CSSprite *));

	luaL_getmetatable(L, "nuvie.Sprite");
	lua_setmetatable(L, -2);

	*userdata = sprite;

	return true;
}

static int nscript_sprite_set(lua_State *L) {
	CSSprite **s_sprite;
	CSSprite *sprite;
	const char *key;

	s_sprite = (CSSprite **)lua_touserdata(L, 1);
	if (s_sprite == NULL)
		return 0;

	sprite = *s_sprite;
	if (sprite == NULL)
		return 0;

	key = lua_tostring(L, 2);

	if (!strcmp(key, "x")) {
		sprite->x = lua_tointeger(L, 3);
		return 0;
	}

	if (!strcmp(key, "y")) {
		sprite->y = lua_tointeger(L, 3);
		return 0;
	}

	if (!strcmp(key, "opacity")) {
		int opacity = lua_tointeger(L, 3);
		sprite->opacity = (uint8)clamp(opacity, 0, 255);
		return 0;
	}

	if (!strcmp(key, "visible")) {
		sprite->visible = lua_toboolean(L, 3);
		return 0;
	}

	if (!strcmp(key, "image")) {
		if (sprite->image) {
			sprite->image->refcount--;
			if (sprite->image->refcount == 0)
				delete sprite->image;
		}

		sprite->image = nscript_get_image_from_args(L, 3);
		if (sprite->image)
			sprite->image->refcount++;

		return 0;
	}

	if (!strcmp(key, "clip_x")) {
		sprite->clip_rect.left = lua_tointeger(L, 3) + cutScene->get_x_off();
		return 0;
	}

	if (!strcmp(key, "clip_y")) {
		sprite->clip_rect.top = lua_tointeger(L, 3) + cutScene->get_y_off();
		return 0;
	}

	if (!strcmp(key, "clip_w")) {
		sprite->clip_rect.setWidth(lua_tointeger(L, 3));
		return 0;
	}

	if (!strcmp(key, "clip_h")) {
		sprite->clip_rect.setHeight(lua_tointeger(L, 3));
		return 0;
	}
	if (!strcmp(key, "text")) {
		const char *text = lua_tostring(L, 3);
		sprite->text = Std::string(text);
	}
	if (!strcmp(key, "text_color")) {
		sprite->text_color = lua_tointeger(L, 3);
		return 0;
	}
	if (!strcmp(key, "text_align")) {
		int align_val = lua_tointeger(L, 3);
		sprite->text_align = (uint8) align_val;
		return 0;
	}
	return 0;
}


static int nscript_sprite_get(lua_State *L) {
	CSSprite **s_sprite;
	CSSprite *sprite;
	const char *key;

	s_sprite = (CSSprite **)lua_touserdata(L, 1);
	if (s_sprite == NULL)
		return 0;

	sprite = *s_sprite;
	if (sprite == NULL)
		return 0;

	key = lua_tostring(L, 2);

	if (!strcmp(key, "x")) {
		lua_pushinteger(L, sprite->x);
		return 1;
	}

	if (!strcmp(key, "y")) {
		lua_pushinteger(L, sprite->y);
		return 1;
	}

	if (!strcmp(key, "opacity")) {
		lua_pushinteger(L, sprite->opacity);
		return 1;
	}

	if (!strcmp(key, "visible")) {
		lua_pushboolean(L, sprite->visible);
		return 1;
	}

	if (!strcmp(key, "image")) {
		if (sprite->image) {
			nscript_new_image_var(L, sprite->image);
			return 1;
		}
	}

	if (!strcmp(key, "text")) {
		lua_pushstring(L, sprite->text.c_str());
		return 1;
	}

	if (!strcmp(key, "text_color")) {
		lua_pushinteger(L, sprite->text_color);
		return 1;
	}

	if (!strcmp(key, "text_width")) {
		lua_pushinteger(L, cutScene->get_text_width(sprite->text.c_str()));
		return 1;
	}

	return 0;
}

static int nscript_sprite_gc(lua_State *L) {
	//DEBUG(0, LEVEL_INFORMATIONAL, "\nSprite garbage Collection!\n");

	CSSprite **p_sprite = (CSSprite **)lua_touserdata(L, 1);
	CSSprite *sprite;

	if (p_sprite == NULL)
		return false;

	sprite = *p_sprite;

	if (sprite) {
		if (sprite->image) {
			if (nscript_dec_image_ref_count(sprite->image) == 0)
				delete sprite->image;
		}
		cutScene->remove_sprite(sprite);
		delete sprite;
	}

	return 0;
}

static int nscript_sprite_new(lua_State *L) {
	CSSprite *sprite = new CSSprite();

	if (lua_gettop(L) >= 1 && !lua_isnil(L, 1)) {
		sprite->image = nscript_get_image_from_args(L, 1);
		if (sprite->image)
			sprite->image->refcount++;
	}

	if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
		sprite->x = lua_tointeger(L, 2);
	}

	if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) {
		sprite->y = lua_tointeger(L, 3);
	}

	if (lua_gettop(L) >= 4 && !lua_isnil(L, 4)) {
		sprite->visible = lua_toboolean(L, 4);
	}

	cutScene->add_sprite(sprite);

	nscript_new_sprite_var(L, sprite);
	return 1;
}

static int nscript_sprite_move_to_front(lua_State *L) {
	CSSprite *sprite = nscript_get_sprite_from_args(L, 1);
	if (sprite) {
		cutScene->remove_sprite(sprite);
		cutScene->add_sprite(sprite);
	}

	return 0;
}

static int nscript_text_load(lua_State *L) {
	const char *filename = lua_tostring(L, 1);
	uint8 idx = lua_tointeger(L, 2);

	Std::vector<Std::string> text = cutScene->load_text(filename, idx);

	if (text.empty()) {
		return 0;
	}

	lua_newtable(L);

	for (uint16 i = 0; i < text.size(); i++) {
		lua_pushinteger(L, i);
		lua_pushstring(L, text[i].c_str());
		lua_settable(L, -3);
	}

	return 1;
}

static int nscript_midgame_load(lua_State *L) {
	const char *filename = lua_tostring(L, 1);
	Std::vector<CSMidGameData> v = cutScene->load_midgame_file(filename);

	if (v.empty()) {
		return 0;
	}

	lua_newtable(L);

	for (uint16 i = 0; i < v.size(); i++) {
		lua_pushinteger(L, i);

		lua_newtable(L);

		lua_pushstring(L, "text");
		lua_newtable(L);
		for (uint16 j = 0; j < v[i].text.size(); j++) {
			lua_pushinteger(L, j);
			lua_pushstring(L, v[i].text[j].c_str());
			lua_settable(L, -3);
		}
		lua_settable(L, -3);

		lua_pushstring(L, "images");
		lua_newtable(L);
		for (uint16 j = 0; j < v[i].images.size(); j++) {
			lua_pushinteger(L, j);
			nscript_new_image_var(L, v[i].images[j]);
			lua_settable(L, -3);
		}
		lua_settable(L, -3);

		lua_settable(L, -3);
	}

	return 1;
}

static int nscript_canvas_set_bg_color(lua_State *L) {
	uint8 color = lua_tointeger(L, 1);
	cutScene->set_bg_color(color);

	return 0;
}

static int nscript_canvas_set_palette(lua_State *L) {
	const char *filename = lua_tostring(L, 1);
	uint8 idx = lua_tointeger(L, 2);

	cutScene->load_palette(filename, idx);

	return 0;
}

static int nscript_canvas_set_palette_entry(lua_State *L) {
	uint8 idx = lua_tointeger(L, 1);
	uint8 r = lua_tointeger(L, 2);
	uint8 g = lua_tointeger(L, 3);
	uint8 b = lua_tointeger(L, 4);

	cutScene->set_palette_entry(idx, r, g, b);

	return 0;
}

static int nscript_canvas_rotate_palette(lua_State *L) {
	uint8 idx = lua_tointeger(L, 1);
	uint8 len = lua_tointeger(L, 2);

	cutScene->rotate_palette(idx, len);

	return 0;
}

static int nscript_canvas_set_update_interval(lua_State *L) {
	uint16 loop_interval = lua_tointeger(L, 1);

	cutScene->set_update_interval(loop_interval);
	return 0;
}

static int nscript_canvas_set_solid_bg(lua_State *L) {
	bool solid_bg = lua_toboolean(L, 1);
	cutScene->set_solid_bg(solid_bg);
	return 0;
}

static int nscript_canvas_set_opacity(lua_State *L) {
	uint8 opacity = lua_tointeger(L, 1);

	cutScene->set_screen_opacity(opacity);
	return 0;
}

static int nscript_canvas_update(lua_State *L) {
	cutScene->update();
	return 0;
}

static int nscript_canvas_show(lua_State *L) {
	cutScene->moveToFront();
	return 0;
}

static int nscript_canvas_hide(lua_State *L) {
	cutScene->Hide();
	return 0;
}

static int nscript_canvas_hide_all_sprites(lua_State *L) {
	cutScene->hide_sprites();
	return 0;
}

static int nscript_canvas_string_length(lua_State *L) {
	Font *font = cutScene->get_font();
	const char *str = lua_tostring(L, 1);

	lua_pushinteger(L, font->getStringWidth(str));
	return 1;
}

static int nscript_canvas_rotate_game_palette(lua_State *L) {
	bool val = lua_toboolean(L, 1);
	cutScene->enable_game_palette_rotation(val);
	return 0;
}

static int nscript_music_play(lua_State *L) {
	uint16 song_num = 0;
	const char *filename = lua_tostring(L, 1);

	if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
		song_num = lua_tointeger(L, 2);
	}

	cutScene->get_sound_manager()->musicPlay(filename, song_num);

	return 0;
}

static int nscript_music_stop(lua_State *L) {
	cutScene->get_sound_manager()->musicStop();

	return 0;
}

static int nscript_get_mouse_x(lua_State *L) {
	int x, y;
	cutScene->get_screen()->get_mouse_location(&x, &y);
	x -= cutScene->get_x_off();
	lua_pushinteger(L, x);
	return 1;
}

static int nscript_get_mouse_y(lua_State *L) {
	int x, y;
	cutScene->get_screen()->get_mouse_location(&x, &y);
	y -= cutScene->get_y_off();
	lua_pushinteger(L, y);
	return 1;
}

static int nscript_input_poll(lua_State *L) {
	Common::Event event;
	bool poll_mouse_motion;
	if (lua_isnil(L, 1))
		poll_mouse_motion = false;
	else
		poll_mouse_motion = lua_toboolean(L, 1);

	while (Events::get()->pollEvent(event)) {
		//FIXME do something here.
		KeyBinder *keybinder = Game::get_game()->get_keybinder();

		if (event.type == Common::EVENT_JOYAXIS_MOTION ||
				event.type == Common::EVENT_JOYBUTTON_DOWN ||
				event.type == Common::EVENT_JOYBUTTON_UP) {
			event.kbd.flags = 0;
			event.kbd.keycode = keybinder->get_key_from_joy_events(&event);
			if (event.kbd.keycode == Common::KEYCODE_INVALID)
				// button isn't mapped or was in deadzone
				return 0; // pretend nothing happened

			event.type = Common::EVENT_KEYDOWN;
		}

		if (event.type == Common::EVENT_KEYDOWN) {
			Common::KeyState key = event.kbd;

			if ((((key.flags & Common::KBD_CAPS) == Common::KBD_CAPS
					&& (key.flags & Common::KBD_SHIFT) == 0) || ((key.flags & Common::KBD_CAPS) == 0 && (key.flags & Common::KBD_SHIFT)))
			        && key.keycode >= Common::KEYCODE_a && key.keycode <= Common::KEYCODE_z)
				key.keycode = (Common::KeyCode)(key.keycode - 32);
			if (key.keycode > 0xFF || !Common::isPrint((char)key.keycode) || (key.flags & Common::KBD_ALT) || (key.flags & Common::KBD_CTRL)) {
				ActionType a = keybinder->get_ActionType(key);
				switch (keybinder->GetActionKeyType(a)) {
				case WEST_KEY:
					lua_pushinteger(L, INPUT_KEY_LEFT);
					return 1;
				case EAST_KEY:
					lua_pushinteger(L, INPUT_KEY_RIGHT);
					return 1;
				case SOUTH_KEY:
					lua_pushinteger(L, INPUT_KEY_DOWN);
					return 1;
				case NORTH_KEY:
					lua_pushinteger(L, INPUT_KEY_UP);
					return 1;
				case CANCEL_ACTION_KEY:
					key.keycode = Common::KEYCODE_ESCAPE;
					break;
				case DO_ACTION_KEY:
					key.keycode = Common::KEYCODE_RETURN;
					break;
				default:
					if (keybinder->handle_always_available_keys(a)) return 0;
					break;
				}
			}
			lua_pushinteger(L, key.keycode);
			return 1;
		}
		if (event.type == Common::EVENT_QUIT) {
			lua_pushinteger(L, 'Q');
			return 1;
		}

		if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN) {
			lua_pushinteger(L, 0);
			return 1;
		}
		if (poll_mouse_motion && event.type == Common::EVENT_MOUSEMOVE) {
			lua_pushinteger(L, 1);
			return 1;
		}
	}

	return 0;
}

static int nscript_config_set(lua_State *L) {
	const char *config_key = lua_tostring(L, 1);

	if (lua_isstring(L, 2)) {
		cutScene->get_config()->set(config_key, lua_tostring(L, 2));
	} else if (lua_isnumber(L, 2)) {
		cutScene->get_config()->set(config_key, (int)lua_tointeger(L, 2));
	} else if (lua_isboolean(L, 2)) {
		cutScene->get_config()->set(config_key, (bool)lua_toboolean(L, 2));
	}

	return 0;
}

static int nscript_engine_should_quit(lua_State *L) {
	int x = g_engine->shouldQuit();
	lua_pushinteger(L, x);
	return 1;
}

ScriptCutscene::ScriptCutscene(GUI *g, Configuration *cfg, SoundManager *sm) : GUI_Widget(NULL) {
	config = cfg;
	gui = g;

	cursor = Game::get_game()->get_cursor();
	x_off = Game::get_game()->get_game_x_offset();
	y_off = Game::get_game()->get_game_y_offset();

	x_off += (Game::get_game()->get_game_width() - 320) / 2; // center it
	y_off += (Game::get_game()->get_game_height() - 200) / 2; // center it

	nuvie_game_t game_type = Game::get_game()->get_game_type();

	GUI_Widget::Init(NULL, 0, 0, g->get_width(), g->get_height());

	clip_rect = Common::Rect(x_off, y_off, x_off + 320, y_off + 200);
	screen = g->get_screen();
	gui->AddWidget(this);
	Hide();
	sound_manager = sm;

	//FIXME this should be loaded by script.
	Std::string path;


	font = new WOUFont();

	if (game_type == NUVIE_GAME_U6) {
		config_get_path(config, "u6.set", path);
		font->init(path.c_str());
	}
	//FIXME load other fonts for MD / SE if needed here.
	if (game_type == NUVIE_GAME_SE) {
		Std::string filePath;
		U6Lib_n lib_file;

		config_get_path(config, "savage.fnt", filePath);

		lib_file.open(filePath, 4, NUVIE_GAME_SE); //can be either SE or MD just as long as it isn't set to U6 type.

		unsigned char *buf = lib_file.get_item(0);
		font->initWithBuffer(buf, lib_file.get_item_size(0)); //buf will be freed by ~Font()
	}

	if (game_type == NUVIE_GAME_MD) {
		Std::string filePath;
		U6Lib_n lib_file;

		config_get_path(config, "fonts.lzc", filePath);

		lib_file.open(filePath, 4, NUVIE_GAME_MD);

		unsigned char *buf = lib_file.get_item(0);
		font->initWithBuffer(buf, lib_file.get_item_size(0)); //buf will be freed by ~Font()
	}
	next_time = 0;
	loop_interval = 40;

	screen_opacity = 255;
	bg_color = 0;
	solid_bg = true;
	rotate_game_palette = false;
	palette = NULL;
}

ScriptCutscene::~ScriptCutscene() {
	delete font;
}

bool ScriptCutscene::is_lzc(const char *filename) {
	if (strlen(filename) > 4 && scumm_stricmp((const char *)&filename[strlen(filename) - 4], ".lzc") == 0)
		return true;

	return false;
}

CSImage *ScriptCutscene::load_image_from_lzc(Std::string filename, uint16 idx, uint16 sub_idx) {
	CSImage *image = nullptr;
	U6Lib_n lib_n;
	unsigned char *buf = NULL;

	if (!lib_n.open(filename, 4, NUVIE_GAME_MD)) {
		return NULL;
	}

	if (idx >= lib_n.get_num_items()) {
		return NULL;
	}

	buf = lib_n.get_item(idx, NULL);
	NuvieIOBuffer io;
	io.open(buf, lib_n.get_item_size(idx), false);
	U6Lib_n lib1;
	lib1.open(&io, 4, NUVIE_GAME_MD);

	if (sub_idx >= lib1.get_num_items()) {
		return NULL;
	}

	U6Shape *shp = new U6Shape();
	if (shp->load(&lib1, (uint32)sub_idx)) {
		image = new CSImage(shp);
	}

	free(buf);

	return image;
}

CSImage *ScriptCutscene::load_image(const char *filename, int idx, int sub_idx) {
	U6Lib_n lib_n;
	Std::string path;
	CSImage *image = NULL;

	config_get_path(config, filename, path);

	if (is_lzc(filename)) {
		return load_image_from_lzc(path, (uint16)idx, (uint16)sub_idx);
	}

	U6Shape *shp = new U6Shape();

	if (idx >= 0) {
		U6Lzw lzw;

		uint32 decomp_size;
		unsigned char *buf = lzw.decompress_file(path.c_str(), decomp_size);
		NuvieIOBuffer io;
		io.open(buf, decomp_size, false);
		{
			// Note: libN needs to be destroyed before the io object.
			U6Lib_n libN;
			if (libN.open(&io, 4, NUVIE_GAME_MD)) {
				if (shp->load(&libN, (uint32)idx)) {
					image = new CSImage(shp);
				}
			}
		}
		free(buf);
	} else {
		if (shp->load(path)) {
			image = new CSImage(shp);
		}
	}

	if (image == NULL)
		delete shp;

	return image;
}

Std::vector<Std::vector<CSImage *> > ScriptCutscene::load_all_images(const char *filename) {
	Std::string path;
	CSImage *image = NULL;

	config_get_path(config, filename, path);

	Std::vector<Std::vector<CSImage *> > v;
	U6Lzw lzw;

	U6Lib_n lib_n;
	unsigned char *buf = NULL;

	if (is_lzc(filename)) {
		if (!lib_n.open(path, 4, NUVIE_GAME_MD)) {
			return v;
		}
		for (uint32 idx = 0; idx < lib_n.get_num_items(); idx++) {
			buf = lib_n.get_item(idx, NULL);
			NuvieIOBuffer io;
			io.open(buf, lib_n.get_item_size(idx), false);
			U6Lib_n lib1;
			lib1.open(&io, 4, NUVIE_GAME_MD);
			//printf("lib_size = %d\n", lib1.get_num_items());
			Std::vector<CSImage *> v1;
			for (uint32 idx1 = 0; idx1 < lib1.get_num_items(); idx1++) {
				U6Shape *shp = new U6Shape();
				if (shp->load(&lib1, (uint32)idx1)) {
					image = new CSImage(shp);
					v1.push_back(image);
				}
			}
			free(buf);
			buf = NULL;
			v.push_back(v1);
		}
	} else {
		uint32 decomp_size;
		buf = lzw.decompress_file(path.c_str(), decomp_size);
		NuvieIOBuffer io;
		io.open(buf, decomp_size, false);
		if (!lib_n.open(&io, 4, NUVIE_GAME_MD)) {
			free(buf);
			return v;
		}

		for (uint32 idx = 0; idx < lib_n.get_num_items(); idx++) {
			Std::vector<CSImage *> v1;
			U6Shape *shp = new U6Shape();
			if (shp->load(&lib_n, (uint32)idx)) {
				image = new CSImage(shp);
				v1.push_back(image);
				v.push_back(v1);
			}
		}

		lib_n.close();
	}

	if (buf)
		free(buf);

	return v;

}

void load_images_from_lib(Std::vector<CSImage *> *images, U6Lib_n *lib, uint32 index) {
	unsigned char *buf = lib->get_item(index, NULL);
	if (buf == NULL) {
		return;
	}

	NuvieIOBuffer io;
	io.open(buf, lib->get_item_size(index), false);
	U6Lib_n lib1;
	lib1.open(&io, 4, NUVIE_GAME_MD);

	for (uint16 i = 0; i < lib1.get_num_items(); i++) {
		U6Shape *shp = new U6Shape();
		if (shp->load(&lib1, i)) {
			images->push_back(new CSImage(shp));
		}
	}
	free(buf);
}

Std::vector<CSMidGameData> ScriptCutscene::load_midgame_file(const char *filename) {
	Std::string path;
	U6Lib_n lib_n;
	Std::vector<CSMidGameData> v;
	nuvie_game_t game_type = Game::get_game()->get_game_type();

	config_get_path(config, filename, path);

	if (!lib_n.open(path, 4, NUVIE_GAME_MD)) {
		return v;
	}

	for (uint32 idx = 0; idx < lib_n.get_num_items();) {
		if (game_type == NUVIE_GAME_MD && idx == 0) { //Skip the first entry in the lib for MD it is *bogus*
			idx++;
			continue;
		}

		CSMidGameData data;
		for (int i = 0; i < 3; i++, idx++) {
			unsigned char *buf = lib_n.get_item(idx, NULL);
			data.text.push_back(string((const char *)buf));
			free(buf);
		}

		load_images_from_lib(&data.images, &lib_n, idx++);

		if (game_type == NUVIE_GAME_MD) {
			load_images_from_lib(&data.images, &lib_n, idx++);
		}

		v.push_back(data);
	}

	return v;
}

Std::vector<Std::string> ScriptCutscene::load_text(const char *filename, uint8 idx) {
	Std::string path;
	U6Lib_n lib_n;
	Std::vector<string> v;
	unsigned char *buf = NULL;

	config_get_path(config, filename, path);

	if (!lib_n.open(path, 4, NUVIE_GAME_MD) || idx >= lib_n.get_num_items()) {
		return v;
	}

	buf = lib_n.get_item(idx, NULL);
	uint16 len = lib_n.get_item_size(idx);
	if (buf != NULL) {
		uint16 start = 0;
		for (uint16 i = 0; i < len; i++) {
			if (buf[i] == '\r') {
				buf[i] = '\0';
				v.push_back(string((const char *)&buf[start]));
				i++;
				buf[i] = '\0'; // skip the '\n' character
				start = i + 1;
			}
		}

		free(buf);
	}

	return v;
}

void ScriptCutscene::print_text(CSImage *image, const char *s, uint16 *x, uint16 *y, uint16 startx, uint16 width, uint8 color) {
	int len = *x - startx;
	size_t start = 0;
	size_t found;
	Std::string str = s;
	Std::list<Std::string> tokens;
	int space_width = font->getStringWidth(" ");
	//uint16 x1 = startx;

	found = str.findFirstOf(" ", start);
	while (found != string::npos) {
		Std::string token = str.substr(start, found - start);

		int token_len = font->getStringWidth(token.c_str());

		if (len + token_len + space_width > width) {
			//FIXME render line here.
			list<Std::string>::iterator it;
			int new_space = 0;
			if (tokens.size() > 1)
				new_space = floor((width - (len - space_width * (tokens.size() - 1))) / (tokens.size() - 1));

			for (it = tokens.begin() ; it != tokens.end() ; it++) {
				*x = ((WOUFont *)font)->drawStringToShape(image->shp, (*it).c_str(), *x, *y, color);
				*x += new_space;
			}
			*y += 8;
			*x = startx;
			len = token_len + space_width;
			tokens.clear();
			tokens.push_back(token);
		} else {
			tokens.push_back(token);
			len += token_len + space_width;
		}

		start = found + 1;
		found = str.findFirstOf(" ", start);
	}

	list<Std::string>::iterator it;

	for (it = tokens.begin() ; it != tokens.end() ; it++) {
		*x = ((WOUFont *)font)->drawStringToShape(image->shp, (*it).c_str(), *x, *y, color);
		*x += space_width;
	}

	if (start < str.length()) {
		Std::string token = str.substr(start, str.length() - start);
		if (len + font->getStringWidth(token.c_str()) > width) {
			*y += 8;
			*x = startx;
		}
		*x = ((WOUFont *)font)->drawStringToShape(image->shp, token.c_str(), *x, *y, color);
	}


	//font->drawStringToShape(image->shp, string, x, y, color);
}

void ScriptCutscene::load_palette(const char *filename, int idx) {
	NuvieIOFileRead file;
	uint8 buf[0x240 + 1];
	uint8 unpacked_palette[0x300];
	Std::string path;

	config_get_path(config, filename, path);


	if (file.open(path.c_str()) == false) {
		DEBUG(0, LEVEL_ERROR, "loading palette.\n");
		return;
	}

	if (file.read4() == DELUXE_PAINT_MAGIC || has_file_extension(filename, ".lbm")) {
		//deluxe paint palette file.
		file.seek(0x30);
		file.readToBuf(unpacked_palette, 0x300);
	} else if (has_file_extension(filename, ".pal")) {
		U6Lib_n lib;
		lib.open(path, 4, NUVIE_GAME_MD);
		unsigned char *decomp_buf = lib.get_item(0, NULL);
		memcpy(unpacked_palette, &decomp_buf[idx * 0x300], 0x300);

		free(decomp_buf);
	} else {
		file.seek(idx * 0x240);

		file.readToBuf(buf, 0x240);
		buf[0x240] = 0; //protect from buf[byte_pos+1] overflow when processing last byte of data.

		for (int i = 0; i < 0x100; i++) {
			for (int j = 0; j < 3; j++) {
				int byte_pos = (i * 3 * 6 + j * 6) / 8;
				int shift_val = (i * 3 * 6 + j * 6) % 8;
				int color = ((buf[byte_pos] +
				              (buf[byte_pos + 1] << 8))
				             >> shift_val) & 0x3F;
				unpacked_palette[i * 3 + j] = (uint8)(color << 2);
			}
		}
	}

	/*
	printf("GIMP Palette\nName: U6 ending\n#\n");
	for (int i = 0; i < 0x100; i++)
	{
	    for (int j = 0; j < 3; j++)
	    {
	        printf("% 3d ", unpacked_palette[i*3+j]);
	    }
	    printf(" untitled\n");
	}
	*/
	screen->set_palette(unpacked_palette);
}

void ScriptCutscene::set_palette_entry(uint8 idx, uint8 r, uint8 g, uint8 b) {
	screen->set_palette_entry(idx, r, g, b);
}

void ScriptCutscene::rotate_palette(uint8 idx, uint8 length) {
	screen->rotate_palette(idx, length);
}

void ScriptCutscene::set_update_interval(uint16 interval) {
	loop_interval = interval;
}

void ScriptCutscene::set_screen_opacity(uint8 new_opacity) {
	screen_opacity = new_opacity;
}

void ScriptCutscene::hide_sprites() {
	for (Std::list<CSSprite *>::iterator it = sprite_list.begin(); it != sprite_list.end(); it++) {
		CSSprite *s = *it;
		if (s->visible)
			s->visible = false;
	}
}

void ScriptCutscene::update() {
	if (cutScene->Status() == WIDGET_HIDDEN) {
		cutScene->Show();
		gui->force_full_redraw();
	}

	if (rotate_game_palette) {
		GamePalette *pal = Game::get_game()->get_palette();
		if (pal) {
			pal->rotatePalette();
		}
	}
	gui->Display();
	screen->preformUpdate();
	sound_manager->update();
	wait();
}

void ScriptCutscene::wait() {
	uint32 now = SDL_GetTicks();
	if (next_time <= now) {
		next_time = now + loop_interval;
		return;
	}

	uint32 delay = next_time - now;
	next_time += loop_interval;
	g_system->delayMillis(delay);
}

/* Show the widget  */
void ScriptCutscene::Display(bool full_redraw) {
	if (cursor && cursor->is_visible())
		cursor->clear();

	if (solid_bg) {
		if (full_redraw)
			screen->fill(bg_color, 0, 0, area.width(), area.height());
		else
			screen->fill(bg_color, x_off, y_off, 320, 200);
	}

	if (screen_opacity > 0) {
		for (Std::list<CSSprite *>::iterator it = sprite_list.begin(); it != sprite_list.end(); it++) {
			CSSprite *s = *it;
			if (s->visible) {
				if (s->image) {
					uint16 w, h;
					s->image->shp->get_size(&w, &h);
					uint16 x, y;
					s->image->shp->get_hot_point(&x, &y);
					screen->blit(x_off + s->x - x, y_off + s->y - y, s->image->shp->get_data(), 8, w, h, w, true, s->clip_rect.width() != 0 ? &s->clip_rect : &clip_rect, s->opacity);
				}

				if (s->text.length() > 0) {
					if (s->text_align != 0) {
						display_wrapped_text(s);
					} else {
						if (s->text_color == 0xffff) {
							font->drawString(screen, s->text.c_str(), s->x + x_off, s->y + y_off);
						} else {
							font->drawString(screen, s->text.c_str(), s->x + x_off, s->y + y_off, (uint8) s->text_color, (uint8) s->text_color);
						}
					}
				}
			}
		}

		if (screen_opacity < 255) {
			screen->fade(x_off, y_off, 320, 200, screen_opacity, bg_color);
		}
	}

	if (cursor)
		cursor->display();

	if (full_redraw)
		screen->update(0, 0, area.width(), area.height());
	else
		screen->update(x_off, y_off, 320, 200);
}

void ScriptCutscene::Hide() {
	GUI_Widget::Hide();
	gui->force_full_redraw();
}

void ScriptCutscene::display_wrapped_text(CSSprite *s) {
	uint8 text_color = (uint8) s->text_color;

	size_t start = 0;
	size_t found;
	Std::string str = s->text + "^";
	Std::list<Std::string> tokens;
	int y = s->y;


	Std::string line = "";

	found = str.findFirstOf("^", start);
	while (found != string::npos) {
		Std::string token = str.substr(start, found - start);

		y = display_wrapped_text_line(token, text_color, s->x, y, s->text_align);

		start = found + 1;
		found = str.findFirstOf("^", start);
	}
}

int ScriptCutscene::display_wrapped_text_line(Std::string str, uint8 text_color, int x, int y, uint8 align_val) {

	//font->drawString(screen, s->text.c_str(), s->x + x_off, s->y + y_off, text_color, text_color);
	int len = 0;
	size_t start = 0;
	size_t found;
	str = str + " ";
	Std::list<Std::string> tokens;
	int space_width = font->getStringWidth(" ");
	//uint16 x1 = startx;
	int width = 320 - x * 2;

	int char_height = font->getCharHeight();

	Std::string line = "";

	found = str.findFirstOf(" ", start);
	while (found != string::npos) {
		Std::string token = str.substr(start, found - start);

		int token_len = font->getStringWidth(token.c_str());

		if (len + token_len > width) {
			if (len > 0) {
				len -= space_width;
			}
			font->drawString(screen, line.c_str(), x + x_off + (align_val == 1 ? 0 : (width - len)) / 2, y + y_off, text_color, text_color);
			line = "";
			y += char_height + 2;
			len = 0;
		}

		len += token_len + space_width;
		line = line + token + " ";

		start = found + 1;
		found = str.findFirstOf(" ", start);
	}

	if (len > 0) {
		len -= space_width;
		font->drawString(screen, line.c_str(), x + x_off + (align_val == 1 ? 0 : (width - len)) / 2, y + y_off, text_color, text_color);
		y += char_height + 2;
	}

	return y;
}
void CSImage::setScale(uint16 percentage) {
	if (scale == percentage) {
		return;
	}

	if (scaled_shp)
		delete scaled_shp;

	scale = percentage;
	if (scale == 100) {
		scaled_shp = NULL;
		shp = orig_shp;
		return;
	}

	uint16 sw, sh;
	uint16 sx, sy;

	uint16 tw, th;
	uint16 tx, ty;

	float scale_factor = (float)scale / 100;

	orig_shp->get_size(&sw, &sh);
	orig_shp->get_hot_point(&sx, &sy);

	tw = (uint16)((float)sw * scale_factor);
	th = (uint16)((float)sh * scale_factor);
	tx = (uint16)((float)sx * scale_factor);
	ty = (uint16)((float)sy * scale_factor);

	scaled_shp = new U6Shape();
	if (!scaled_shp->init(tw, th, tx, ty)) {
		scale = 100;
		delete scaled_shp;
		scaled_shp = NULL;
		return;
	}

	scale_rect_8bit(orig_shp->get_data(), scaled_shp->get_data(), sw, sh, tw, th);
	shp = scaled_shp;

	return;
}

CSStarFieldImage::CSStarFieldImage(U6Shape *shape) : CSImage(shape) {
	shp->get_size(&w, &h);

	for (int i = 0; i < STAR_FIELD_NUM_STARS; i++) {
		stars[i].color = 2;
		stars[i].line = NULL;
	}
}

void CSStarFieldImage::updateEffect() {
	unsigned char *data = shp->get_data();

	memset(data, 0, w * h);

	for (int i = 0; i < STAR_FIELD_NUM_STARS; i++) {
		if (stars[i].line == NULL) {
			switch (NUVIE_RAND() % 4) {
			case 0 :
				stars[i].line = new U6LineWalker(w / 2, h / 2, 0, NUVIE_RAND() % h);
				break;
			case 1 :
				stars[i].line = new U6LineWalker(w / 2, h / 2, w - 1, NUVIE_RAND() % h);
				break;
			case 2 :
				stars[i].line = new U6LineWalker(w / 2, h / 2, NUVIE_RAND() % w, 0);
				break;
			case 3 :
				stars[i].line = new U6LineWalker(w / 2, h / 2, NUVIE_RAND() % w, h - 1);
				break;
			}

			stars[i].color = NUVIE_RAND() % 10 + 229;
			uint16 start_pos = NUVIE_RAND() % (w / 2);
			for (int j = 0; j < start_pos; j++) {
				if (stars[i].line->step() == false) {
					delete stars[i].line;
					stars[i].line = NULL;
					break;
				}
			}
		} else {
			uint32 cur_x, cur_y;
			if (stars[i].line->next(&cur_x, &cur_y) == false) {
				delete stars[i].line;
				stars[i].line = NULL;
			} else {
				data[cur_y * w + cur_x] = stars[i].color;
			}
		}
	}
}

} // End of namespace Nuvie
} // End of namespace Ultima
