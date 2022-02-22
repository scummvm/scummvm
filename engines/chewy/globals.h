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

#ifndef CHEWY_GLOBAL_H
#define CHEWY_GLOBAL_H

#include "common/array.h"
#include "chewy/chewy.h"
#include "chewy/types.h"
#include "chewy/atds.h"
#include "chewy/room.h"
#include "chewy/movclass.h"
#include "chewy/timer.h"
#include "chewy/gedclass.h"
#include "chewy/bitclass.h"
#include "chewy/text.h"
#include "chewy/object.h"
#include "chewy/effect.h"
#include "chewy/sound_player.h"

namespace Chewy {

#define MAX_ZOBJ 60
#define MAX_RAND_NO_USE 6
#define R45_MAX_PERSON 9

extern const uint8 RAND_NO_USE[MAX_RAND_NO_USE];

class ChewyFont;
class Cursor;
class FontMgr;
class InputMgr;
class IOGame;
class McgaGraphics;
class Memory;
class SoundPlayer;

class Globals {
	class CurrentScreen : public Graphics::Surface {
	public:
		CurrentScreen() {
			w = pitch = SCREEN_WIDTH;
			h = SCREEN_HEIGHT;
			format = Graphics::PixelFormat::createFormatCLUT8();
		}
		CurrentScreen &operator=(byte *p) {
			setPixels(p);
			return *this;
		}
		byte *getPixels() { return (byte *)Graphics::Surface::getPixels(); }
	};

	struct ZObjSort {
		uint8 ObjArt;
		uint8 ObjNr;
		int16 ObjZ;
	};
public:
	Globals();
	~Globals();
public:
	int16 _ani_invent_anf[3] = { 38, 39, 21 };
	int16 _ani_invent_delay[3][2] = {
		{ 12, 12 }, { 10, 10 }, { 11, 11 }
	};
	int16 _ani_count[3] = { 38, 39, 21 };
	int16 _timer_nr[MAX_TIMER_OBJ] = { 0 };
public:
	CurrentScreen _currentScreen;
	Spieler _spieler;

	int16 _pfeil_ani = 0;
	int16 _pfeil_delay = 0;
	int16 _cur_hide_flag = 0;
	int16 _auto_p_nr = 0;

	int16 _zoom_horizont = 0;
	int16 _zoom_mov_fak = 0;

	int16 _auto_obj = 0;
	int16 _ged_mov_ebene = 0;

	bool _cur_display = false;
	int16 _maus_links_click = 0;
	Common::String _calc_inv_text_str1, _calc_inv_text_str2;
	bool _calc_inv_text_set = false;
	bool _stopAutoMove[3] = { false };
	bool _e_streifen = false;
	int16 _r45_delay = 0;
	int16 _r56koch_flug = 0;
	int16 _r62Delay = 0;
	int16 _r62TalkAni = 0;
	int16 _r64TalkAni = 0;
	int16 _r63ChewyAni = 0;
	int16 _r63Schalter = 0;
	int16 _r63RunDia = 0;
	int16 _r65tmp_scrollx = 0;
	int16 _r65tmp_scrolly = 0;
	int16 _r65tmp_ch_x = 0;
	int16 _r65tmp_ch_y = 0;
	int16 _r65tmp_ho_x = 0;
	int16 _r65tmp_ho_y = 0;
	int16 _r68HohesC = 0;

	// global.cpp
	const uint8 *_chewy_ph_anz;
	const uint8 *_chewy_ph;
	byte *_pal = nullptr;
	byte *_screen0 = nullptr;
	int16 _scr_width = 0;
	BlendMode _fx_blend = BLEND_NONE;
	void (*_SetUpScreenFunc)() = nullptr;

	byte *_font6x8 = nullptr;
	byte *_font8x8 = nullptr;
	int16 _fvorx6x8 = 0;
	int16 _fvorx8x8 = 0;
	int16 _fvory6x8 = 0;
	short _fvory8x8 = 0;
	int16 _FrameSpeed = 0;
	int16 _FrameSpeedTmp = 0;

	byte **_ablage = nullptr;
	byte *_workpage = nullptr;
	byte *_workptr = nullptr;
	byte *_cur_back = nullptr;
	byte **_ged_mem = nullptr;
	byte *_spblende = nullptr;
	char **_ads_item_ptr = nullptr;

	int16 _ads_dia_nr = 0;
	int16 _ads_item_anz = 0;
	int16 _ads_blk_nr = 0;
	int16 _ads_push = 0;

	int16 _ads_tmp_dsp = 0;
	int8 _menu_display = 0;
	int16 _menu_lauflicht = 0;
	int16 _menu_item = 0;
	int16 _menu_item_vorwahl = 0;
	int16 _maus_menu_x = 0;
	int16 _maus_old_x = 0;
	int16 _maus_old_y = 0;
	int16 _inventar_nr = 0;
	int16 _invent_cur_mode = 0;
	byte *_inv_spr[MAX_MOV_OBJ] = { nullptr };
	int16 _person_tmp_hide[MAX_PERSON] = { 0 };
	int16 _person_tmp_room[MAX_PERSON] = { 0 };

	int16 _talk_start_ani = -1;
	int16 _talk_hide_static = -1;
	int16 _frequenz = 0;
	int16 _currentSong = -1;
	bool _savegameFlag = false;
	Stream *_music_handle = nullptr;
	int16 _EndOfPool = 0;
	int _timer_action_ctr = 0;

	AutoMov _auto_mov[MAX_AUTO_MOV];
	int16 _HowardMov = 0;

	TafSeqInfo *_spz_tinfo = nullptr;
	int16 _SpzDelay = 0;
	int16 _spz_spr_nr[MAX_SPZ_PHASEN] = { 0 };
	int16 _spz_start = 0;
	int16 _spz_akt_id = 0;

	int16 _spz_p_nr = 0;
	int16 _spz_delay[MAX_PERSON] = { 0 };
	int16 _spz_count = 0;
	int16 _spz_ani[MAX_PERSON] = { 0 };

	int16 _AkChewyTaf = 0;
	int16 _PersonAni[MAX_PERSON] = { 0 };
	TafSeqInfo *_PersonTaf[MAX_PERSON] = { nullptr };
	uint8 _PersonSpr[MAX_PERSON][8] = { 0 };

	RoomDetailInfo *_Rdi = nullptr;
	StaticDetailInfo *_Sdi = nullptr;
	AniDetailInfo *_Adi = nullptr;
	TafInfo *_curtaf = nullptr;
	TafInfo *_menutaf = nullptr;
	TafSeqInfo *_howard_taf = nullptr;
	TafInfo *_chewy = nullptr;
	int16 *_chewy_kor = nullptr;
	BitClass *_bit = nullptr;
	GedClass *_ged = nullptr;
	Text *_txt = nullptr;
	Room *_room = nullptr;
	Object *_obj = nullptr;
	Timer *_uhr = nullptr;
	Detail *_det = nullptr;
	Effect *_fx = nullptr;
	Atdsys *_atds = nullptr;
	Flic *_flc = nullptr;
	MovClass *_mov = nullptr;

	ObjMov _spieler_vector[MAX_PERSON];
	SprInfo _spr_info[MAX_PROG_ANI];
	MovInfo _spieler_mi[MAX_PERSON];
	ObjMov _auto_mov_vector[MAX_OBJ_MOV];
	MovInfo _auto_mov_obj[MAX_OBJ_MOV];
	MovPhasen _mov_phasen[MAX_OBJ_MOV];
	MovLine _mov_line[MAX_OBJ_MOV][5];
	TimerBlk _ani_timer[MAX_TIMER_OBJ];
	int16 _person_end_phase[MAX_PERSON] = { 0 };
	int16 _ani_stand_count[MAX_PERSON] = { 0 };
	bool _ani_stand_flag[MAX_PERSON] = { false };

	MouseInfo _minfo;
	KbdInfo _kbinfo;
	CurBlk _curblk;
	CurAni _curani;
	IogInit _ioptr;
	RaumBlk _room_blk;
	Flags _flags = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0 };
	CustomInfo _Ci;
	GotoPkt _gpkt;

	SplitStringInit _ssi[AAD_MAX_PERSON] = {
		{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8 },
	};
	int16 _r45_pinfo[R45_MAX_PERSON][4] = {
	   { 0, 4, 1490, 1500 },
	   { 0, 1, 180, 190 },
	   { 0, 1, 40, 60 },
	   { 0, 1, 40, 150 },
	   { 0, 1, 40, 230 },
	   { 0, 1, 40, 340 },
	   { 0, 1, 49, 50 },
	   { 0, 1, 5, 90 },
	   { 0, 1, 7, 190 }
	};

	// main.cpp
	int16 _menu_flag = 0;
	bool _inv_disp_ok = 0;
	int16 _txt_aus_click = 0;
	int16 _txt_nummer = 0;
	int16 _tmp_menu_item = 0;
	int16 _cur_ausgang_flag = 0;
	int16 _room_start_nr = 0;
	int16 _TmpFrameSpeed = 0;
	InputMgr *_in = nullptr;
	Memory *_mem = nullptr;
	IOGame *_iog = nullptr;
	McgaGraphics *_out = nullptr;
	Cursor *_cur = nullptr;
	SoundPlayer *_sndPlayer = nullptr;
	ChewyFont *_font6 = nullptr;
	ChewyFont *_font8 = nullptr;
	FontMgr *_fontMgr = nullptr;
	int16 _scroll_delay = 0;

	// mcga.cpp
	byte _saved_palette[PALETTE_SIZE] = { 0 };
	bool _screenHasDefault = false;
	byte *_screenDefaultP = nullptr;
	int _spriteWidth = 0;
	byte *_fontAddr = nullptr;
	size_t _fontWidth = 0, _fontHeight = 0;
	int _fontFirst = 0, _fontLast = 0;

	// mcga_graphics.cpp
	int16 _clipx1 = 0, _clipx2 = 0, _clipy1 = 0, _clipy2 = 0;
	int16 _gcurx = 0, _gcury = 0, _fvorx = 0, _fvory = 0;
	int16 _scr_w = 0, _scr_h = SCREEN_HEIGHT;

	// menus.cpp
	int8 _m_flip = 0;
	int16 _tmp_menu = 0;
	int16 _show_invent_menu = 0;

	// r_event.cpp
	int16 _flic_val1 = 0, _flic_val2 = 0;

	// sprite.cpp
	int16 _z_count = 0;
	ZObjSort _z_obj_sort[MAX_ZOBJ];
	char _new_vector = false;
	int16 _tmp_maus_links = 0;

	// timer.cpp
	bool _timer_int = false;
	int _timer_count = 0;
	bool _timer_suspend = false;
};

extern Globals *g_globals;

#define _G(FIELD) g_globals->_##FIELD

void cursorChoice(int16 nr);

void hideCur();

void showCur();

void alloc_buffers();
void free_buffers();

bool mainLoop(int16 mode);

void set_up_screen(SetupScreenMode mode);

void kb_mov(int16 mode);

void kb_cur_action(int16 key, int16 mode);
void mouseAction();

void obj_auswerten(int16 test_nr, int16 txt_nr);

void swap_if_l(int16 *x1, int16 *x2);

void print_shad(int16 x, int16 y, int16 fcol, int16 bcol, int16 scol,
	int16 scr_w, char *txtptr);

bool auto_move(int16 mov_nr, int16 p_nr);
void get_user_key(int16 mode);
void clear_prog_ani();

void set_ani_screen();

void del_inventar(int16 nr);

bool is_cur_inventar(int16 nr);

void check_shad(int16 g_idx, int16 mode);

void get_scroll_off(int16 x, int16 y, int16 pic_x, int16 pic_y,
                    int16 *sc_x, int16 *sc_y);

void calc_scroll(int16 x, int16 y, int16 pic_x, int16 pic_y,
                 int16 *sc_x, int16 *sc_y);

void auto_scroll(int16 scrx, int16 scry);

int16 calc_mouse_mov_obj(int16 *auto_nr);

void check_mouse_ausgang(int16 x, int16 y);

void calc_ausgang(int16 x, int16 y);

void go_auto_xy(int16 x, int16 y, int16 p_nr, int16 mode);

void calc_auto_go();

void disable_timer();

void enable_timer();

int16 is_mouse_person(int16 x, int16 y);

void calc_mouse_person(int16 x, int16 y);

void hide_person();

void show_person();

void save_person_rnr();

void set_person_rnr();

bool is_chewy_busy();

void standard_init();

void tidy();

void init_load();

void var_init();

void new_game();
void sound_init();
void sound_exit();
void show_intro();
void register_cutscene(int cutsceneNum);
void getCutscenes(Common::Array<int> &cutscenes);

void init_atds();

void init_room();

void build_menu(int16 x, int16 y, int16 xanz, int16 yanz, int16 col, int16 mode);

void auto_menu(int16 *x, int16 *y, int16 zeilen_anz, int16 zeilen_hoehe,
               char *txt, int16 mode);

void maus_mov_menu();

void cur_2_inventory();

void inventory_2_cur(int16 nr);

void new_invent_2_cur(int16 inv_nr);

void invent_2_slot(int16 nr);

int16 del_invent_slot(int16 nr);

void remove_inventory(int16 nr);

void get_display_xy(int16 *x, int16 *y, int16 nr);
void calc_txt_xy(int16 *x, int16 *y, char *txt_adr, int16 txt_anz);
void ads_menu();

void stop_ads_dialog();

void play_scene_ani(int16 nr, int16 mode);

void timer_action(int16 t_nr);

void check_ged_action(int16 index);

int16 ged_user_func(int16 idx_nr);

void enter_room(int16 eib_nr);

void exit_room(int16 eib_nr);

void flic_cut(int16 nr);

void print_rows(int16 id);

int16 sib_event_no_inv(int16 sib_nr);

void sib_event_inv(int16 sib_nr);

uint16 exit_flip_flop(int16 ani_nr, int16 eib_nr1, int16 eib_nr2,
                        int16 ats_nr1, int16 ats_nr2, int16 sib_nr,
                        int16 spr_nr1, int16 spr_nr2, int16 flag);

int16 load_ads_dia(int16 dia_nr);

void set_ssi_xy();

int16 ats_action(int16 txt_nr, int16 txt_mode, int16 MODE);

void ads_action(int16 dia_nr, int16 blk_nr, int16 str_end_nr);

void ads_ende(int16 dia_nr, int16 blk_nr, int16 str_end_nr);

void atds_string_start(int16 dia_nr, int16 str_nr, int16 person_nr,
                       int16 mode);
void calc_inv_use_txt(int16 test_nr);
bool calc_inv_no_use(int16 test_nr, int16 mode);
int16 calc_person_txt(int16 p_nr);
int16 calc_person_click(int16 p_nr);
void calc_person_dia(int16 p_nr);
void sprite_engine();

void calc_z_ebene();

void start_ani_block(int16 anz, const AniBlock *ab);

void start_detail_wait(int16 ani_nr, int16 rep, int16 mode);

void start_detail_frame(int16 ani_nr, int16 rep, int16 mode,
                        int16 frame);

void wait_detail(int16 det_nr);

void wait_show_screen(int16 frames);

void zoom_mov_anpass(ObjMov *om, MovInfo *mi);

void calc_zoom(int16 y, int16 zoomfak_x, int16 zoomfak_y,
               ObjMov *om);

void load_chewy_taf(int16 taf_nr);

bool start_ats_wait(int16 txt_nr, int16 txt_mode, int16 col, int16 mode);
void start_aad_wait(int16 dia_nr, int16 str_nr);

void start_aad(int16 dia_nr);
void aad_wait(int16 str_nr);
void start_ads_wait(int16 dia_nr);
void start_aad(int16 dia_nr, int16 ssi_nr);
void wait_auto_obj(int16 nr);

void stop_auto_obj(int16 nr);

void continue_auto_obj(int16 nr, int16 repeat);

void new_auto_line(int16 nr);
void init_auto_obj(int16 auto_nr, const int16 *phasen, int16 lines,
                   const MovLine *pkt);

int16 mouse_auto_obj(int16 nr, int16 xoff, int16 yoff);

int16 auto_obj_status(int16 nr);

int16 mouse_on_prog_ani();

void set_spz_delay(int16 delay);

void start_spz_wait(int16 ani_id, int16 count, bool reverse, int16 p_nr);

bool start_spz(int16 ani_id, int16 count, bool reverse, int16 p_nr);

void stop_spz();

void calc_person_spz_ani(ObjMov *om);

void set_person_pos(int16 x, int16 y, int16 p_nr, int16 richtung);

void set_person_spr(int16 nr, int16 p_nr);

void stop_person(int16 p_nr);

void mov_objekt(ObjMov *om, MovInfo *mi);

void calc_person_end_ani(ObjMov *om, int16 p_nr);

void get_phase(ObjMov *om, MovInfo *mi);

void get_lr_phase(ObjMov *om, int16 obj_mode);

void load_person_ani(int16 ani_id, int16 p_nr);

void calc_person_ani();

void load_room_music(int16 room_nr);

} // namespace Chewy

#endif
