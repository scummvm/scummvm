/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

/* GFX Settings. Sound & Music only works properly with SIMON1WIN */
#define USE_SOUND
#define USE_MUSIC

/* Various other settings */
//#define DUMP_CONTINOUS_MAINSCRIPT
//#define DUMP_START_MAINSCRIPT
//#define DUMP_CONTINOUS_VGASCRIPT
//#define USE_TEXT_HACK
//#define DRAW_IMAGES_DEBUG
//#define DRAW_THREE_STARS
//#define DUMP_START_VGASCRIPT
//#define DUMP_FILE_NR 8
//#define DUMP_BITMAPS_FILE_NR 8
//#define DUMP_DRAWN_BITMAPS

uint fileReadByte(FILE *in);
uint fileReadBE16(FILE *in);
uint fileReadLE16(FILE *in);
uint32 fileReadBE32(FILE *in);
uint32 fileReadLE32(FILE *in);
void fileWriteBE32(FILE *in, uint32 value);
void fileWriteBE16(FILE *in, uint16 value);


#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))
#define CHECK_BOUNDS(x,y) assert((uint)(x)<ARRAYSIZE(y))

enum {
       CHILD1_SIZE = 12,
       CHILD2_SIZE = 16
};

struct Child {
	Child *next;
	uint16 type;
};

struct Child2 {
	Child hdr;

	uint16 string_id;
	uint32 avail_props;
	int16 array[1];
};

struct Child1 {
	Child hdr;

	uint16 subroutine_id;
	uint16 fr2;
	uint16 array[1];
};

struct Child9 {
	Child hdr;
	
	uint16 array[4];
};

struct Child3 {
	Child hdr;
};

struct ThreeValues {
	uint16 a, b, c;
};


struct Item {
	uint16 parent;
	uint16 child;
	uint16 sibling;
	int16 unk1;
	int16 unk2;
	int16 unk3; /* signed int */
	uint16 unk4;
	uint16 xxx_1; /* unused? */
	Child *children;
};

struct Subroutine {
	uint16 id;				/* subroutine ID */
	uint16 first;			/* offset from subroutine start to first subroutine line */
	Subroutine *next;	/* next subroutine in linked list */
};

struct FillOrCopyDataEntry {
	Item *item;
	uint16 hit_area;
	uint16 xxx_1;
};

struct FillOrCopyData {
	int16 unk1;
	Item *item_ptr;
	FillOrCopyDataEntry e[64];
	int16 unk3, unk4;
	uint16 unk2;
};

struct FillOrCopyStruct {
	byte mode;
	byte flags;
	uint16 x, y;
	uint16 width,height;
	uint16 unk1, unk2;
	uint8 unk3, unk6, unk7, fill_color, text_color, unk5;
	FillOrCopyData *fcs_data;
};


enum {
	SUBROUTINE_LINE_SMALL_SIZE = 2,
	SUBROUTINE_LINE_BIG_SIZE = 8,
};

struct SubroutineLine {
	uint16 next;
	int16 cond_a;
	int16 cond_b;
	int16 cond_c;
};

struct TimeEvent {
	uint32 time;
	uint16 subroutine_id;
	TimeEvent *next;
};

struct HitArea {
	uint16 x, y;
	uint16 width, height;
	uint16 flags;
	uint16 id;
	FillOrCopyStruct *fcs;
	Item *item_ptr;
	uint16 unk3;
	uint16 layer;
};

struct VgaPointersEntry {
	byte *vgaFile1;
	byte *vgaFile2;
	uint32 dd;
};

struct VgaSprite {
	uint16 id;
	uint16 image;
	uint16 base_color;
	uint16 x,y; /* actually signed numbers */
	uint16 unk4,unk5,unk6,unk7;
};

struct VgaSleepStruct {
	uint16 ident;
	byte *code_ptr;
	uint16 sprite_id;
	uint16 cur_vga_file;
};

struct VgaTimerEntry {
	uint16 delay;
	byte *script_pointer;
	uint16 sprite_id;
	uint16 cur_vga_file;
};

struct VgaFile1Header {
	uint16 x_1, x_2;
	uint16 hdr2_start;
	uint16 x_3, x_4;
};

struct VgaFile1Header2 {
	uint16 x_1;
	uint16 unk1;
	uint16 x_2;
	uint16 id_count;
	uint16 x_3;
	uint16 unk2_offs;
	uint16 x_4;
	uint16 id_table;
	uint16 x_5;
};

struct VgaFile1Struct0x8 {
	uint16 id;
	uint16 x_1;
	uint16 x_2;
	uint16 script_offs;
};

struct VgaFile1Struct0x6 {
	uint16 id;
	uint16 x_2;
	uint16 script_offs;
};

/* dummy typedefs to make it compile in *nix */
#if defined(UNIX) || defined(UNIX_X11) || defined(__MORPHOS__) || defined(__DC__) || defined(__APPLE__CW)
typedef void* HMIDISTRM;
typedef void* HMIDIOUT;
typedef uint32 UINT;
typedef void* MIDIHDR;
typedef uint32 MMRESULT;
#define CALLBACK
typedef uint32 DWORD;

enum {
	VK_F5 = 1,
	VK_LBUTTON = 2,
	VK_SHIFT = 3,

};

int GetAsyncKeyState(int key);

#endif

class MidiDriver;
struct MidiEvent;

class MidiPlayer {
public:
	void read_all_songs(FILE *in);
	void read_all_songs_old(FILE *in);
	void initialize();
	void shutdown();
	void play();
	void set_driver(MidiDriver *md);
		
private:
	struct Track {
		uint32 a;
		uint32 data_size;
		uint32 data_cur_size;
		byte *data_ptr;
		byte *data_cur_ptr;
		uint32 delay;
		byte last_cmd;
	};

	struct Song {
		uint ppqn;
		uint midi_format;
		uint num_tracks;
		Track *tracks;
	};

	struct NoteRec {
		uint32 delay;
		byte cmd;
		byte param_1;
		byte param_2;
		uint cmd_length;
		byte *sysex_data;
	};

	MidiDriver *_md;

	FILE *_input;

	uint _midi_var10, _midi_5;
	bool _midi_var9;
	byte _midi_var1;
	bool _shutting_down;
	uint _midi_var8;

	uint _midi_var11;

	uint32 _midi_tempo;
	
	Track *_midi_tick_track_ptr;
	Track *_midi_track_ptr;
	int16 _midi_song_id;
	int16 _midi_song_id_2;
	int16 _midi_var2;

	Song *_midi_cur_song_ptr;

	uint32 _midi_volume_table[16];

	Song _midi_songs[8];

	void read_mthd(Song *s, bool old);

	void read_from_file(void *dst, uint size);
	void read_one_song(Song *s);
	byte read_byte_from_file();
	uint32 read_uint32_from_file();
	uint16 read_uint16_from_file();

	static uint32 track_read_gamma(Track *t);
	static byte track_read_byte(Track *t);

	int fill(MidiEvent *me, int num_event);
	bool fill_helper(NoteRec *nr, MidiEvent *me);

	void reset_tracks();
	void read_next_note(Track *t, NoteRec *nr);

	void unload();

	static int on_fill(void *param, MidiEvent *ev, int num);

};


struct GameSpecificSettings {
	uint VGA_DELAY_BASE;
	uint TABLE_INDEX_BASE;
	uint TEXT_INDEX_BASE;
	uint NUM_GAME_OFFSETS;
	uint NUM_VIDEO_OP_CODES;
	uint VGA_MEM_SIZE;
	uint TABLES_MEM_SIZE;
	uint NUM_VOICE_RESOURCES;
	uint MUSIC_INDEX_BASE;
	uint SOUND_INDEX_BASE;
	const char *gme_filename;
	const char *wav_filename;
	const char *gamepc_filename;
};


class SimonState {
public:
	OSystem *_system;

	char *_game_path;

	byte *_vc_ptr; /* video code ptr */

	uint32 *_game_offsets_ptr;

	const GameSpecificSettings *gss;

	byte _game;
	byte _key_pressed;

	enum {
		GAME_SIMON2 = 1,
		GAME_WIN = 2,

		GAME_SIMON1DOS = 0,
		GAME_SIMON1WIN = 2,
		GAME_SIMON2DOS = 1,
		GAME_SIMON2WIN = 3,
	};


	FILE *_game_file;
	FILE *_voice_file;
	uint32 *_voice_offsets;

	byte *_stripped_txt_mem;
	uint _text_size;
	uint _stringtab_num, _stringtab_pos, _stringtab_numalloc;
	byte **_stringtab_ptr;

	Item **_itemarray_ptr;
	uint _itemarray_size;
	uint _itemarray_inited;

	byte *_itemheap_ptr;
	uint _itemheap_curpos;
	uint _itemheap_size;

	byte *_icon_file_ptr;

	byte *_tbl_list;

	byte *_code_ptr;
	

	byte **_local_stringtable;
	uint _string_id_local_min, _string_id_local_max;

	byte *_tablesheap_ptr, *_tablesheap_ptr_org, *_tablesheap_ptr_new;
	uint _tablesheap_size,_tablesheap_curpos,_tablesheap_curpos_org;
	uint _tablesheap_curpos_new;

	Subroutine *_subroutine_list, *_subroutine_list_org;

	uint _dx_surface_pitch;

	uint _recursion_depth;

	uint32 _last_vga_tick;

//#ifdef SIMON2
	uint16 _op_189_flags;
//#endif

	bool _scriptvar_2;
	bool _run_script_return_1;
	bool _skip_vga_wait;
	bool _no_parent_notify;
	bool _vga_res_328_loaded;
	bool _hitarea_unk_3;
	bool _mortal_flag;
	bool _sync_flag_1;
	byte _video_var_8;
	bool _use_palette_delay;
	bool _sync_flag_2;
	bool _hitarea_unk_6;
	bool _in_callback;
	bool _cepe_flag;
	byte _copy_partial_mode;
	bool _fast_mode;
	bool _dx_use_3_or_4_for_lock;

	bool _mouse_pos_changed;

//#ifdef SIMON2
	bool _vk_t_toggle;
	byte _mouse_cursor;
	bool _vga_var9;
//#endif

//#ifdef SIMON2
	int16 _script_unk_1;
	bool _vga_var6;
	int _x_scroll,_vga_var1,_vga_var2,_vga_var3,_vga_var5;
	byte _vga_var8;

	uint16 _vc72_var1, _vc72_var2, _vc72_var3;
	uint16 _vc70_var1, _vc70_var2;
	byte *_vga_var7;
//#else
//	int _script_unk_1;
//#endif

	int16 _script_cond_a, _script_cond_b, _script_cond_c;

	uint16 _fcs_unk_1;
	FillOrCopyStruct *_fcs_ptr_1;

	Item *_subject_item, *_object_item;
	Item *_item_1_ptr, *_item_ptr_B;
	Item *_item_1;

	byte *_sfx_heap;

	Item *_hitarea_object_item;
	HitArea *_last_hitarea;
	HitArea*_last_hitarea_2_ptr;
	HitArea*_last_hitarea_3;
	byte _left_button_down;
	Item *_hitarea_subject_item;
	HitArea *_hitarea_ptr_5, *_hitarea_ptr_7;
	uint _need_hitarea_recalc;
	uint _verb_hitarea;
	uint16 _hitarea_unk_4;
	uint _lock_counter;

	uint16 _video_palette_mode;

	uint _print_char_unk_1, _print_char_unk_2;
	uint _num_letters_to_print;

	uint _last_time;

	TimeEvent *_first_time_struct, *_pending_delete_time_event;

	uint _base_time;

	uint _mouse_x, _mouse_y;
	uint _mouse_x_old, _mouse_y_old;

	Item _dummy_item_1;
	Item _dummy_item_2;
	Item _dummy_item_3;

	uint16 _lock_word;
	uint16 _scroll_up_hit_area;
	uint16 _scroll_down_hit_area;

	uint16 _video_var_7;
	uint16 _palette_color_count;

	byte _video_var_4;
	bool _video_var_5;
	bool _video_var_3;
	bool _unk_pal_flag;
	bool _exit_cutscene;
	byte _video_var_9;

	uint _last_music_played;

	bool _show_preposition;
	bool _showmessage_flag;

	uint _video_num_pal_colors;

	uint _invoke_timer_callback;

//	uint32 _voice_size;
	
//	uint32 _sound_size;
//	byte *_sound_ptr;

	uint _vga_sprite_changed;
	
	byte *_vga_buf_free_start, *_vga_buf_end, *_vga_buf_start;
	byte *_vga_file_buf_org, *_vga_file_buf_org_2;

	byte *_cur_vga_file_1;
	byte *_cur_vga_file_2;

	uint16 _timer_1, _timer_5, _timer_4;

	uint16 _vga_base_delay;

	uint16 _vga_cur_file_2;
	uint16 _vga_wait_for, _vga_cur_file_id;
	uint16 _vga_cur_sprite_id;

	VgaTimerEntry *_next_vga_timer_to_process;
	
	Item *_vc_item_array[20];
	Item *_item_array_6[20];

	uint16 _stringid_array_2[20];
	uint16 _stringid_array_3[20];
	uint16 _array_4[20];

	uint16 _bit_array[32];
	int16 _variableArray[256];

	FillOrCopyStruct *_fcs_ptr_array_3[8];

	byte _fcs_data_1[8];
	bool _fcs_data_2[8];

	SoundMixer _mixer[1];

	ThreeValues _threevalues_1, _threevalues_2, _threevalues_3, _threevalues_4;

	int _free_string_slot;

	byte _stringReturnBuffer[2][180];

	HitArea _hit_areas[90];

	VgaPointersEntry _vga_buffer_pointers[180];
	VgaSprite _vga_sprites[180]; 
	VgaSleepStruct _vga_sleep_structs[30];

//	uint16 _unk21_word_array[32]; /* should be initialized to ones */

	uint16 *_pathfind_array[20];

	uint8 _palette_backup[1024];
	uint8 _palette[1024];

	byte _video_buf_1[3000];

	VgaTimerEntry _vga_timer_list[95];

	FillOrCopyStruct _fcs_list[16];

	byte _letters_to_print_buf[80];

	MidiPlayer midi;

	int _num_screen_updates;
	int _vga_tick_counter;

	PlayingSoundHandle _playing_sound;
	PlayingSoundHandle _voice_sound;

	int _timer_id;

	FILE *_dump_file;

	int _number_of_savegames;

	int _saveload_row_curpos;
	int _num_savegame_rows;
	bool _savedialog_flag;
	bool _save_or_load;
	bool _saveload_flag;

	int allocGamePcVars(FILE *in);
	Item *allocItem1();
	void loginPlayerHelper(Item *item, int a, int b);
	void loginPlayer();
	void allocateStringTable(int num);
	void setupStringTable(byte *mem, int num);
	void setupLocalStringTable(byte *mem, int num);
	void readGamePcText(FILE *in);
	void readItemChildren(FILE *in, Item *item, uint tmp);
	void readItemFromGamePc(FILE *in, Item *item);
	bool loadGamePcFile(const char *filename);

	byte *allocateItem(uint size);
	byte *allocateTable(uint size);
	void alignTableMem();

	Child *findChildOfType(Item *i, uint child);
	Child *allocateChildBlock(Item *i, uint type, uint size);

	void allocItemHeap();
	void allocTablesHeap();

	Subroutine *createSubroutine(uint a);
	void readSubroutine(FILE *in, Subroutine *sub);
	SubroutineLine *createSubroutineLine(Subroutine *sub, int a);
	void readSubroutineLine(FILE *in, SubroutineLine *new_table, Subroutine *sub);
	byte *readSingleOpcode(FILE *in, byte *ptr);
	void readSubroutineBlock(FILE *in);

	Subroutine *getSubroutineByID(uint subroutine_id);

	/* used in debugger */
	void dumpSubroutines();
	void dumpSubroutine(Subroutine *sub);
	void dumpSubroutineLine(SubroutineLine *sl, Subroutine *sub);
	byte *dumpOpcode(byte *p);

	int startSubroutine(Subroutine *sub);
	int startSubroutineEx(Subroutine *sub);

	bool checkIfToRunSubroutineLine(SubroutineLine *sl, Subroutine *sub);

	int runScript();

	Item *getNextItemPtr();
	uint getNextItemID();
	uint getItem1ID() { return 1; }
	Item *getItem1Ptr();
	Item *getItemPtrB();

	byte getByte();
	int getNextWord();

	uint getNextVarContents();
	uint getVarOrWord();
	uint getVarOrByte();
	uint readVariable(uint variable);
	void writeNextVarContents(uint16 contents);
	void writeVariable(uint variable, uint16 contents);

	void setItemParent(Item *item, Item *parent);

	uint itemPtrToID(Item *id);

	Item *derefItem(uint item);
	void setItemUnk3(Item *item, int value);

	void showMessageFormat(const char *s, ...);
	const byte *getStringPtrByID(uint string_id);
	const byte *getLocalStringByID(uint string_id);
	uint getNextStringID();

	void addTimeEvent(uint timeout, uint subroutine_id);
	void delTimeEvent(TimeEvent *te);

	bool hasChildOfType1(Item *item);
	bool hasChildOfType2(Item *item);

	Child1 *findChildOfType1(Item *item);
	Child2 *findChildOfType2(Item *item);
	Child3 *findChildOfType3(Item *item);

	void itemChildrenChanged(Item *item);
	void unlinkItem(Item *item);
	void linkItem(Item *item, Item *parent);

	bool o_unk_23(uint a);

	void o_unk_99_simon1(uint a);
	void o_unk_99_simon2(uint a, uint b);

	void o_vga_reset();
	void o_unk_101();
	void fcs_unk_2(uint a);
	void o_unk_103();
	void fcs_delete(uint a);
	void o_unk_108(uint a);
	void clear_hitarea_bit_0x40(uint hitarea);
	void set_hitarea_bit_0x40(uint hitarea);
	void set_hitarea_x_y(uint hitarea, int x, int y);
	bool is_hitarea_0x40_clear(uint hitarea);
	void delete_hitarea(uint hitarea);
	void addNewHitArea(int id, int x, int y, int width, int height, 
		int flags, int unk3,Item *item_ptr);
	HitArea *findEmptyHitArea();
	void hitarea_proc_1();
	void handle_verb_hitarea(HitArea *ha);
	void hitarea_leave(HitArea *ha);
	void leaveHitAreaById(uint hitarea_id);

	void o_unk_114();
	void o_wait_for_vga(uint a);
	void o_unk_120(uint a);
	void o_unk_126();
	void o_unk_127();
	void o_save_game();
	void o_load_game();
	void o_unk_137(uint a);
	void o_unk_138();
	void killAllTimers();
	
	uint getOffsetOfChild2Param(Child2 *child, uint prop);
	void o_unk_160(uint a);
	void o_unk_163(uint a);
	void o_unk_175();
	void o_unk_176();
	void o_pathfind(int x,int y,uint var_1,uint var_2);
	void o_unk_179();
	void o_force_unlock();
	void o_force_lock();
	void o_read_vgares_328();
	void o_read_vgares_23();
	void o_clear_vgapointer_entry(uint a);
	void o_unk_186();
	void o_fade_to_black();
	void o_print_str();
	void o_setup_cond_c();
	void setup_cond_c_helper();

	void o_177();

	void lock();
	void unlock();

	void fcs_unk_proc_1(uint i, Item *item_ptr, int unk1, int unk2);

	void loadTextIntoMem(uint string_id);
	void loadTablesIntoMem(uint subr_id);


	uint loadTextFile(const char *filename, byte *dst);
	FILE *openTablesFile(const char *filename);
	void closeTablesFile(FILE *in);

	uint loadTextFile_simon1(const char *filename, byte *dst);
	FILE *openTablesFile_simon1(const char *filename);
	void closeTablesFile_simon1(FILE *in);

	uint loadTextFile_gme(const char *filename, byte *dst);
	FILE *openTablesFile_gme(const char *filename);
	void closeTablesFile_gme(FILE *in);

	void readSfxFile(const char *filename);

	void invokeTimeEvent(TimeEvent *te);
	bool kickoffTimeEvents();

	void defocusHitarea();
	void startSubroutine170();
	void runSubroutine101();
	void handle_unk2_hitarea(FillOrCopyStruct *fcs);
	void handle_unk_hitarea(FillOrCopyStruct *fcs);
	void hitareaChangedHelper();
	void focusVerb(uint hitarea_id);
	HitArea *findHitAreaByID(uint hitarea_id);

	void showActionString(uint x, const byte *string);
	void video_putchar(FillOrCopyStruct *fcs, byte c);
	void video_fill_or_copy_from_3_to_2(FillOrCopyStruct *fcs);
	void video_toggle_colors(HitArea *ha, byte a, byte b, byte c, byte d);

	void read_vga_from_datfile_1(uint vga_id);

	uint get_fcs_ptr_3_index(FillOrCopyStruct *fcs);

	void setup_hitarea_from_pos(uint x, uint y, uint mode);
	void new_current_hitarea(HitArea *ha);
	bool hitarea_proc_2(uint a);
	bool hitarea_proc_3(Item *item);
	void hitarea_stuff();

	void handle_mouse_moved();
	void pollMouseXY();
	void draw_mouse_pointer();

	void fcs_unk1(uint fcs_index);
	void draw_icon_c(FillOrCopyStruct *fcs, uint icon, uint x, uint y);
	bool has_item_childflag_0x10(Item *item);
	uint item_get_icon_number(Item *item);
	uint setup_icon_hit_area(FillOrCopyStruct *fcs,uint x, uint y, uint icon_number, Item *item_ptr);
	void fcs_unk_proc_2(FillOrCopyStruct *fcs, uint fcs_index);

	void loadIconFile();
	void processSpecialKeys();
	void hitarea_stuff_helper();

	void startUp(uint a);
	void startUp_helper_2();
	void startUp_helper_3();
	void startUp_helper();
	void showmessage_helper_3(uint a, uint b);
	void showmessage_print_char(byte chr);

	void handle_verb_clicked(uint verb);

	void o_set_video_mode(uint mode, uint vga_res);
	void set_video_mode(uint a, uint b);
	void set_video_mode_internal(uint mode, uint vga_res_id);
	
	void ensureVgaResLoadedC(uint vga_res);
	void ensureVgaResLoaded(uint vga_res);

	void start_vga_code(uint b, uint vga_res, uint vga_struct_id, uint c, uint d, uint f);
	void o_unk26_helper(uint a, uint b, uint c, uint d, uint e, uint f, uint g, uint h);
	void talk_with_speech(uint speech_id, uint num_1);
	void talk_with_text(uint num_1, uint num_2, const char *string_ptr, uint a, int b, uint c);
	FillOrCopyStruct *fcs_alloc(uint x, uint y, uint w, uint h, uint flags, uint fill_color, uint unk4);
	
	void render_string(uint num_1, uint color, uint width, uint height, const char *txt);

	void setup_hit_areas(FillOrCopyStruct *fcs, uint fcs_index);

	byte *setup_vga_destination(uint32 size);
	void vga_buf_unk_proc3(byte *end);
	void vga_buf_unk_proc1(byte *end);
	void vga_buf_unk_proc2(uint a,byte *end);
	void delete_memptr_range(byte *end);

	void setup_vga_file_buf_pointers();

	void run_vga_script();

	void vc_1();
	void vc_2();
	void vc_3();
	void vc_4();
	void vc_5();
	void vc_6_maybe_skip_3_inv();
	void vc_7_maybe_skip_3();
	void vc_8_maybe_skip_2();
	void vc_9_maybe_skip();
	void vc_10();
	void vc_11_clear_pathfind_array();
	void vc_12_sleep_variable();
	void vc_13_offset_x();
	void vc_14_offset_y();
	void vc_15_start_funkystruct_by_id();
	void vc_16_setup_funkystruct();
	void vc_17_set_pathfind_item();
	void vc_18_jump_rel();
	void vc_19();
	void vc_20();
	void vc_21();
	void vc_22();
	void vc_23_set_pri();
	void vc_24_set_image_xy();
	void vc_25_del_sprite_and_get_out();
	void vc_26();
	void vc_27_reset();
	void vc_27_reset_simon1();
	void vc_27_reset_simon2();
	void vc_28();
	void vc_29_stop_all_sounds();
	void vc_30_set_base_delay();
	void vc_31_set_palette_mode();
	void vc_32_copy_var();
	void vc_33();
	void vc_34();
	void vc_35();
	void vc_36();
	void vc_37_sprite_unk3_add();
	void vc_38_skip_if_var_zero();
	void vc_39_set_var();
	void vc_40_var_add();
	void vc_41_var_sub();
	void vc_42_delay_if_not_eq();
	void vc_43_skip_if_bit_clear();
	void vc_44_skip_if_bit_set();
	void vc_45_set_x();
	void vc_46_set_y();
	void vc_47_add_var_f();
	void vc_48();
	void vc_49_set_bit();
	void vc_50_clear_bit();
	void vc_51_clear_hitarea_bit_0x40();
	void vc_52();
	void vc_53_no_op();
	void vc_54_no_op();
	void vc_55_offset_hit_area();
	void vc_56_no_op();
	void vc_57_no_op();
	void vc_59();
	void vc_60();
	void vc_61_sprite_change();
	void vc_62();
	void vc_63();

//#ifdef SIMON2
	void vc_64();
	void vc_65();
	void vc_66();
	void vc_67();
	void vc_68();
	void vc_69();
	void vc_70();
	void vc_71();
	void vc_72();
	void vc_73();
	void vc_74();
//#endif

	void delete_vga_timer(VgaTimerEntry *vte);
	void vc_resume_thread(byte *code_ptr, uint16 cur_file, uint16 cur_sprite);
	int vc_read_var_or_word();
	uint vc_read_next_word();
	uint vc_read_next_byte();
	uint vc_read_var(uint var);
	void vc_write_var(uint var, int16 value);
	void vc_skip_next_instruction();

	bool vc_maybe_skip_proc_3(uint16 val);
	bool vc_maybe_skip_proc_2(uint16 a, uint16 b);
	bool vc_maybe_skip_proc_1(uint16 a, int16 b);

	void add_vga_timer(uint num, byte *code_ptr, uint cur_sprite, uint cur_file);
	VgaSprite *find_cur_sprite();
	void vc_set_bit_to(uint bit, bool value);

	bool vc_59_helper();
	void expire_vga_timers();

	bool has_vgastruct_with_id(uint16 id, uint16 file);

	bool vc_get_bit(uint bit);

	void fcs_proc_1(FillOrCopyStruct *fcs, uint value);

	void video_copy_if_flag_0x8_c(FillOrCopyStruct *fcs);
	void delete_hitarea_by_index(uint index);

	void fcs_unk_5(FillOrCopyStruct *fcs, uint fcs_index);
	void fcs_putchar(uint a);

	void copy_img_from_3_to_2(FillOrCopyStruct *fcs);
	void video_erase(FillOrCopyStruct *fcs);

	void dx_copy_rgn_from_3_to_2(uint b, uint r, uint y, uint x);

	byte *dx_lock_2();
	void dx_unlock_2();

	byte *dx_lock_attached();
	void dx_unlock_attached();

	byte *read_vga_from_datfile_2(uint id);

	void resfile_read(void *dst, uint32 offs, uint32 size);

	void go();
	void openGameFile();

	static int CDECL game_thread_proc(void *param);

	void timer_callback();
	void timer_proc1();

	void timer_vga_sprites();
	void timer_vga_sprites_2();

	void dx_clear_surfaces(uint num_lines);
	void dx_update_screen_and_palette();

	void dump_video_script(byte *src, bool one_opcode_only);
	void dump_vga_file(byte *vga);
	void dump_vga_bitmaps(byte *vga, byte *vga1, int res);
	void dump_single_bitmap(int file, int image, byte *offs, int w, int h, byte base);
	
	void dx_clear_attached_from_top(uint lines);
	void dx_copy_from_attached_to_2(uint x, uint y, uint w, uint h);
	void dx_copy_from_attached_to_3(uint lines);
	void dx_copy_from_2_to_attached(uint x, uint y, uint w, uint h);

	void print_char_helper_1(const byte *src, uint len);
	void print_char_helper_5(FillOrCopyStruct *fcs);

	void shutdown();

	byte *vc_10_depack_swap(byte *src, uint w, uint h);

	void dump_vga_script(byte *ptr, uint res, uint sprite_id);
	void dump_vga_script_always(byte *ptr, uint res, uint sprite_id);

	Item *getNextItemPtrStrange();

	bool save_game(uint slot, const char *caption);
	bool load_game(uint slot);

	void showmessage_helper_2();
	void print_char_helper_6(uint i);

	void video_putchar_helper(FillOrCopyStruct *fcs);
	void video_putchar_helper_2(FillOrCopyStruct *fcs, uint x, uint y, byte chr);

	void initSound();
	void playVoice(uint voice);
	void playSound(uint sound);


//	void generateSound(byte *ptr, int len);

	void playMusic(uint music);

	void checkTimerCallback();

	void delay(uint delay);

//#ifdef SIMON2
	void o_190_helper(uint i);
	void vc_58();
	void timer_vga_sprites_helper();

	void vc_10_helper_8(byte *dst, byte *src);
	void scroll_timeout();
	void hitarea_stuff_helper_2();
//#endif
	void realizePalette();

	void vc_kill_thread(uint file, uint sprite);

	static SimonState *create(OSystem *syst, MidiDriver *driver);

	void set_dummy_cursor();

	void set_volume(byte volume);

	FILE *fopen_maybe_lowercase(const char *filename);

	void save_or_load_dialog(bool load);
	void o_unk_132_helper_3();
	int o_unk_132_helper(bool *b, char *buf);
	void o_unk_132_helper_2(FillOrCopyStruct *fcs, int x);
	void savegame_dialog(char *buf);

	int display_savegame_list(int curpos, bool load, char *dst);

	void show_it(void *buf);

	char *gen_savename(int slot);
};

	
void NORETURN CDECL error(const char *errmsg, ...);
void CDECL warning(const char *errmsg, ...);

//uint16 swap16(uint16 a);
//uint32 swap32(uint32 a);


void _2xSaI (uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr, uint8 *dstPtr, uint32 dstPitch, int width, int height);
int Init_2xSaI (uint32 BitFormat);
void Super2xSaI (uint8 *srcPtr, uint32 srcPitch,
		 uint8 *deltaPtr, uint8 *dstPtr, uint32 dstPitch,
		 int width, int height);
void initializeHardware();
void dx_set_palette(uint32 *colors, uint num);

//extern byte *sdl_buf;
//extern byte *sdl_buf_attached;
