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

#ifndef CHEWY_GLOBAL_H
#define CHEWY_GLOBAL_H

#include "chewy/chewy.h"
#include "chewy/types.h"
#include "chewy/atds.h"
#include "chewy/room.h"
#include "chewy/movclass.h"
#include "chewy/timer.h"
#include "chewy/gedclass.h"
#include "chewy/bitclass.h"
#include "chewy/text.h"
#include "chewy/objekte.h"
#include "chewy/effect.h"
#include "chewy/ailclass.h"

#define MAX_RAND_NO_USE 6

extern uint8 rand_no_use[MAX_RAND_NO_USE];

extern int16 pfeil_ani;
extern int16 pfeil_delay;
extern int16 cur_hide_flag;

extern int16 auto_p_nr;

extern int16 timer_nr[MAX_TIMER_OBJ];

extern int16 zoom_horizont;
extern int16 zoom_mov_fak;

extern int16 auto_obj;
extern int16 ged_mov_ebene;

extern int16 person_tmp_hide[MAX_PERSON];
extern int16 person_tmp_room[MAX_PERSON];
extern bool cur_display;
extern int16 maus_links_click;

extern char *err_str;
extern uint32 ram_end;

extern uint32 ram_start;
extern int16 FrameSpeed;
extern int16 FrameSpeedTmp;

extern int16 show_frame;

extern char **ablage;
extern char *workpage;
extern char *workptr;
extern char *cur_back;
extern char **ged_mem;
extern char *pal;
extern char *spblende;

extern char *screen0;
extern int16 scr_width;
extern int16 fx_blende;

extern char *font6x8;
extern char *font8x8;
extern int16 fvorx6x8;
extern int16 fvorx8x8;
extern int16 fvory6x8;
extern int16 fvory8x8;

extern char *invent_txt;
extern char *invent_name;

extern char *str;
extern char **ads_item_ptr;

extern SplitStringInit ssi[AAD_MAX_PERSON];
extern int16 talk_start_ani;

extern int16 talk_hide_static;

extern int16 ads_dia_nr;
extern int16 ads_item_anz;
extern int16 ads_blk_nr;
extern int16 ads_push;

extern int16 ads_tmp_dsp;
extern int8 menu_display;
extern int16 menu_lauflicht;
extern int16 menu_item;
extern int16 menu_item_vorwahl;
extern int16 maus_menu_x;
extern int16 maus_old_x;
extern int16 maus_old_y;
extern int16 inventar_nr;

extern int16 ani_invent_anf [2];
extern int16 ani_invent_end [2];
extern int16 ani_invent_delay[2][2];
extern int16 ani_count[2];
extern int16 invent_cur_mode;
extern int16 ak_invent;
extern char *inv_spr[MAX_MOV_OBJ];

extern int16 life_flag;
extern int16 life_x;
extern int16 life_y;
extern int16 life_anz;
extern char *life_str;
extern int16 life_handler;

extern SprInfo spr_info[MAX_PROG_ANI];
extern DetectInfo detect;
extern maus_info minfo;
extern kb_info kbinfo;
extern cur_blk curblk;
extern cur_ani curani;
extern Spieler spieler;
extern iog_init ioptr;
extern RaumBlk room_blk;
extern ObjMov spieler_vector[MAX_PERSON];

extern MovInfo spieler_mi[MAX_PERSON];

extern ObjMov auto_mov_vector[MAX_OBJ_MOV];
extern MovInfo auto_mov_obj[MAX_OBJ_MOV];

extern MovPhasen mov_phasen[MAX_OBJ_MOV];
extern MovLine mov_line[MAX_OBJ_MOV][5];
extern TimerBlk ani_timer[MAX_TIMER_OBJ];
extern room_detail_info *Rdi;
extern static_detail_info *Sdi;
extern ani_detail_info *Adi;
extern Flags flags;
extern CustomInfo Ci;
extern GotoPkt gpkt;

extern taf_info *curtaf;
extern taf_info *menutaf;
extern taf_seq_info *howard_taf;
extern taf_info *chewy;
extern uint8 *chewy_ph_anz;
extern uint8 *chewy_ph;
extern int16 *chewy_kor;

extern int16 person_end_phase[MAX_PERSON];
extern int16 ani_stand_count[MAX_PERSON];

extern bool ani_stand_flag[MAX_PERSON];

extern char io_pal1[7];

extern bitclass *bit;
extern gedclass *ged;
extern text *txt;
extern Room *room;
extern objekt *obj;
extern timer *uhr;
extern detail *det;
extern effect *fx;
extern atdsys *atds;
extern flic *flc;
extern movclass *mov;
extern ailclass *ailsnd;

extern char background[];
extern char backged [];
extern char fname [80];
extern AutoMov auto_mov[MAX_AUTO_MOV];
extern int16 HowardMov;

extern int16 AkChewyTaf;

extern uint8 chewy_phasen[8][8];
extern uint8 chewy_bo_phasen[8][8];
extern uint8 chewy_mi_phasen[8][8];
extern uint8 chewy_ro_phasen[8][8];
extern uint8 chewy_jm_phasen[8][8];
extern uint8 chewy_phasen_anz[];
extern uint8 chewy_bo_phasen_anz[];
extern uint8 chewy_mi_phasen_anz[];
extern uint8 chewy_ro_phasen_anz[];

extern int16 PersonAni[MAX_PERSON];
extern taf_seq_info *PersonTaf[MAX_PERSON];
extern uint8 PersonSpr[MAX_PERSON][8];

extern int16 SpzDelay;
// FIXME: was FILE*
extern void *spz_taf_handle;
extern taf_seq_info *spz_tinfo;
extern int16 spz_spr_nr[MAX_SPZ_PHASEN];
extern int16 spz_start;
extern int16 spz_akt_id;

extern int16 spz_count;
extern int16 spz_ani_ph[][2];
extern int16 spz_delay[MAX_PERSON];
extern int16 spz_p_nr;
extern int16 spz_ani[MAX_PERSON];

extern int16 frequenz;
// FIXME: was FILE*
extern void *music_handle;
extern int16 EndOfPool;
// FIXME: was FILE*
extern void *speech_handle;
extern char *SpeechBuf[2];
extern int16 CurrentSong;

void cursor_wahl(int16 nr);

void hide_cur();

void show_cur();

void alloc_buffers();
void free_buffers();

int16 main_loop(int16 mode);

void set_up_screen(int16 mode);

void kb_mov(int16 mode);

void kb_cur_action(int16 key, int16 mode);
void maus_action();

void obj_auswerten(int16 test_nr, int16 txt_nr);

void swap_if_l(int16 *x1, int16 *x2);

void print_shad(int16 x, int16 y, int16 fcol, int16 bcol, int16 scol, int16 scr_w,
                char *txtptr);

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

extern void (*SetUpScreenFunc)(void);

void hide_person();

void show_person();

void save_person_rnr();

void set_person_rnr();

int16 is_chewy_busy();

void standard_init();

void get_detect(char *fname);

void error();
void tidy();

void init_load();

void var_init();

void get_frame_speed(char *work_page, char *source);

void set_speed();

void init_life_handler();

void remove_life_handler();

void life_line();

void new_game();
void sound_init();
void sound_exit();

void init_atds();

void init_room();

int16 call_fileio(int16 palette, int16 mode);

// FIXME: was FILE*
void save(void *handle);

// FIXME: was FILE*
void load(void *handle);

// FIXME: was FILE*
void append_adsh(void *shandle);

// FIXME: was FILE*
void split_adsh(void *shandle);
int16 file_menue();
void option_menue(taf_info *ti);

void look_debug_action(int16 key_nr);

void build_menu(int16 x, int16 y, int16 xanz, int16 yanz, int16 col, int16 mode);

void auto_menu(int16 *x, int16 *y, int16 zeilen_anz, int16 zeilen_hoehe,
               char *txt, int16 mode);

void maus_mov_menu();

void plot_inventar_menu();

void invent_menu();

int16 look_invent(int16 invent_nr, int16 mode, int16 ats_nr);

void look_invent_screen(int16 txt_mode, int16 nr);

int16 calc_use_invent(int16 inv_nr);

void cur_2_inventory();

void inventory_2_cur(int16 nr);

void new_invent_2_cur(int16 inv_nr);

void invent_2_slot(int16 nr);

int16 del_invent_slot(int16 nr);

void get_display_xy(int16 *x, int16 *y, int16 nr);
void calc_txt_xy(int16 *x, int16 *y, char *txt_adr, int16 txt_anz);
void ads_menu();

void stop_ads_dialog();

void calc_person_look();
void switch_room(int16 nr);

void play_scene_ani(int16 nr, int16 mode);

void timer_action(int16 t_nr);

void check_ged_action(int16 index);

int16 ged_user_func(int16 idx_nr);

void enter_room(int16 eib_nr);

void exit_room(int16 eib_nr);

void flic_cut(int16 nr, int16 mode);

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

void start_ani_block(int16 anz, AniBlock *ab);

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
void init_auto_obj(int16 auto_nr, int16 *phasen, int16 lines,
                   MovLine *pkt);

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

#endif
