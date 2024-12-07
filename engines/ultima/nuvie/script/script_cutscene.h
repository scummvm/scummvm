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

#ifndef NUVIE_SCRIPT_SCRIPT_CUTSCENE_H
#define NUVIE_SCRIPT_SCRIPT_CUTSCENE_H

#include "common/lua/lua.h"

#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/gui/widgets/gui_widget.h"
#include "ultima/nuvie/files/u6_shape.h"
#include "ultima/nuvie/fonts/wou_font.h"
#include "ultima/nuvie/conf/configuration.h"

namespace Ultima {
namespace Nuvie {

class SoundManager;
class Font;
class U6LineWalker;
class Cursor;

class CSImage {
public:
	U6Shape *orig_shp;
	U6Shape *scaled_shp;
	U6Shape *shp;
	uint16 scale;
	uint16 refcount;

	CSImage(U6Shape *shape) {
		orig_shp = shape;
		scaled_shp = nullptr;
		shp = shape;
		scale = 100;
		refcount = 0;
	}
	virtual ~CSImage() {}

	void setScale(uint16 percentage);
	uint16 getScale() {
		return scale;
	}

	virtual void updateEffect() { };
};

#define STAR_FIELD_NUM_STARS 70

class CSStarFieldImage : public CSImage {

private:
	uint16 w;
	uint16 h;
	struct {
		uint8 color;
		U6LineWalker *line;
	} stars[STAR_FIELD_NUM_STARS];
public:
	CSStarFieldImage(U6Shape *shape);
	~CSStarFieldImage() override {}
	void updateEffect() override;
};

struct CSSprite {
	sint16 x;
	sint16 y;
	uint8 opacity;
	CSImage *image;
	bool visible;
	Common::Rect clip_rect;
	Std::string text;
	uint16 text_color;
	uint8 text_align;

	CSSprite() {
		x = 0;
		y = 0;
		opacity = 255;
		image = nullptr;
		visible = false;
		clip_rect = Common::Rect();
		text = "";
		text_color = 0xffff;
		text_align = 0;
	}
};

struct CSMidGameData {
	Std::vector<Std::string> text;
	Std::vector<CSImage *> images;
};

struct TransferSaveData {
	int gameType;
	Common::String name;
	int gender;
	Common::String className;
	int str;
	int dex;
	int intelligence;
	int magic;
	int exp;
	int level;
};

void nscript_init_cutscene(lua_State *L, Configuration *cfg, GUI *gui, SoundManager *sm);

class ScriptCutscene : public GUI_Widget {
private:
	Configuration *config;
	GUI *gui;
	Cursor *cursor;
	Std::list<CSSprite *> sprite_list; // in paint order
	Screen *screen;
	uint8 *palette;
	SoundManager *sound_manager;
	WOUFont *font;
	Common::Rect clip_rect;
	uint16 x_off, y_off;
	uint32 next_time;
	uint32 loop_interval;
	uint8 screen_opacity;
	uint8 bg_color;
	bool solid_bg;
	bool rotate_game_palette;

public:
	ScriptCutscene(GUI *g, Configuration *cfg, SoundManager *sm);
	~ScriptCutscene() override;

	Std::vector<Std::string> load_text(const char *filename, uint8 idx);

	Std::vector<CSMidGameData> load_midgame_file(const char *filename);

	TransferSaveData load_transfer_save();

	CSImage *load_image(const char *filename, int idx, int sub_idx = 0);
	Std::vector<Std::vector<CSImage *> > load_all_images(const char *filename);
	void add_sprite(CSSprite *s) {
		sprite_list.push_back(s);
	}
	void remove_sprite(CSSprite *s) {
		sprite_list.remove(s);
	}

	void load_palette(const char *filename, int idx);
	void set_palette_entry(uint8 idx, uint8 r, uint8 g, uint8 b);
	void rotate_palette(uint8 idx, uint8 length);
	void set_screen_opacity(uint8 new_opacity);
	void enable_game_palette_rotation(bool val) {
		rotate_game_palette = val;
	}

	void set_update_interval(uint16 interval);
	void update();

	void wait();
	void Display(bool full_redraw) override;
	void Hide() override;

	void print_text(CSImage *image, const char *string, uint16 *x, uint16 *y, uint16 startx, uint16 width, uint8 color);
	void print_text_raw(CSImage *image, const char *string, uint16 x, uint16 y, uint8 color) const;

	SoundManager *get_sound_manager() {
		return sound_manager;
	}

	uint16 get_x_off() const {
		return x_off;
	}
	uint16 get_y_off() const {
		return y_off;
	}

	Font *get_font() {
		return (Font *)font;
	}
	Configuration *get_config() {
		return config;
	}

	void hide_sprites();

	void set_bg_color(uint8 new_color) {
		bg_color = new_color;
	}
	void set_solid_bg(bool value) {
		solid_bg = value;
	}

	Screen *get_screen() {
		return screen;
	}

	uint16 get_text_width(const char *text) {
		return font->getStringWidth(text);
	}

private:
	bool is_lzc(const char *filename);
	CSImage *load_image_from_lzc(const Common::Path &filename, uint16 idx, uint16 sub_idx);
	void display_wrapped_text(CSSprite *s);
	int display_wrapped_text_line(Std::string str, uint8 text_color, int x, int y, uint8 align_val);

	bool load_u4_save_file(TransferSaveData &saveData);
	bool load_u5_save_file(TransferSaveData &saveData);
};

ScriptCutscene *get_cutscene();

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
