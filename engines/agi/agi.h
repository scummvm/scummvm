/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef AGI_H
#define AGI_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/advancedDetector.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/hash-str.h"

#include "engines/engine.h"

namespace Agi {

typedef signed int Err;

/*
 * Version and other definitions
 */
#define VERSION __DATE__ " " __TIME__

#define USE_IIGS_SOUND

#define	TITLE		"AGI engine"

#define DIR_		"dir"
#define LOGDIR		"logdir"
#define PICDIR		"picdir"
#define VIEWDIR		"viewdir"
#define	SNDDIR		"snddir"
#define OBJECTS		"object"
#define WORDS		"words.tok"

#define	MAX_DIRS	256
#define	MAX_VARS	256
#define	MAX_FLAGS	(256 >> 3)
#define MAX_VIEWTABLE	255	/* KQ3 uses o255! */
#define MAX_WORDS	20
#define	MAX_STRINGS	24		/* MAX_STRINGS + 1 used for get.num */
#define MAX_STRINGLEN	40
#ifndef MAX_PATH
#define MAX_PATH	260
#endif

#define	_EMPTY		0xfffff
#define	EGO_OWNED	0xff

#define	CRYPT_KEY_SIERRA	"Avis Durgan"
#define CRYPT_KEY_AGDS		"Alex Simkin"

#ifndef INLINE
#define INLINE
#endif

#define	MSG_BOX_COLOUR	0x0f	/* White */
#define MSG_BOX_TEXT	0x00	/* Black */
#define MSG_BOX_LINE	0x04	/* Red */
#define STATUS_FG	0x00		/* Black */
#define	STATUS_BG	0x0f		/* White */
#define PATCH_LOGIC				/* disable copy protection on some games */

} // End of namespace Agi

/* AGI resources */
#include "agi/console.h"
#include "agi/view.h"
#include "agi/picture.h"
#include "agi/logic.h"
#include "agi/sound.h"

namespace Agi {


enum AgiGameType {
	GType_V2 = 1,
	GType_V3
};

enum AgiGameFeatures {
	AGI_AMIGA = 1 << 0,
	AGI_MOUSE = 1 << 1,
	AGI_AGDS = 1 << 2

};

struct AGIGameDescription {
	Common::ADGameDescription desc;

	int gameType;
	uint32 features;
	uint16 version;
};




enum {
	NO_GAMEDIR = 0,
	GAMEDIR
};

enum AGIErrors {
	err_OK = 0,
	err_DoNothing,
	err_BadCLISwitch,
	err_InvalidAGIFile,
	err_BadFileOpen,
	err_NotEnoughMemory,
	err_BadResource,
	err_UnknownAGIVersion,
	err_RestartGame,
	err_NoLoopsInView,
	err_ViewDataError,
	err_NoGameList,

	err_Unk = 127
};

enum kDebugLevels {
	kDebugLevelMain =      1 << 0,
	kDebugLevelResources = 1 << 1,
	kDebugLevelSprites =   1 << 2,
	kDebugLevelInventory = 1 << 3,
	kDebugLevelInput =     1 << 4,
	kDebugLevelMenu =      1 << 5,
	kDebugLevelScripts =   1 << 6,
	kDebugLevelSound =     1 << 7,
	kDebugLevelText =      1 << 8
};

/**
 * AGI resources.
 */
enum {
	rLOGIC = 1,
	rSOUND,
	rVIEW,
	rPICTURE
};

enum {
	RES_LOADED = 1,
	RES_COMPRESSED = 0x40
};

enum {
	lCOMMAND_MODE = 1,
	lTEST_MODE
};

struct game_id_list {
	struct game_id_list *next;
	uint32 version;
	uint32 crc;
	char *gName;
	char *switches;
};

struct Mouse {
	int button;
	unsigned int x;
	unsigned int y;
};

/**
 * Command-line options.
 */
struct agi_options {
#define GAMERUN_RUNGAME 0
#define GAMERUN_PICVIEW 1
#define GAMERUN_WORDS	2
#define GAMERUN_OBJECTS	3
#define GAMERUN_GAMES	4
#define GAMERUN_CRC	5
	int gamerun;		/**< game run mode*/
	int emuversion;		/**< AGI version to emulate */
	int agds;			/**< enable AGDS mode */
	int amiga;			/**< enable Amiga mode */
	int nosound;		/**< disable sound */
	Common::RenderMode renderMode;
	int soundemu;		/**< sound emulation mode */
	int agimouse;		/**< AGI Mouse 1.0 emulation */
};

#define report printf

enum GameId {
	GID_AGI = 1
};

#define WIN_TO_PIC_X(x) ((x) / 2)
#define WIN_TO_PIC_Y(y) ((y) < 8 ? 999 : (y) >= (8 + _HEIGHT) ? 999 : (y) - 8)

/**
 * AGI variables.
 */
enum {
	V_cur_room = 0,		/* 0 */
	V_prev_room,
	V_border_touch_ego,
	V_score,
	V_border_code,
	V_border_touch_obj,	/* 5 */
	V_ego_dir,
	V_max_score,
	V_free_pages,
	V_word_not_found,
	V_time_delay,		/* 10 */
	V_seconds,
	V_minutes,
	V_hours,
	V_days,
	V_joystick_sensitivity,	/* 15 */
	V_ego_view_resource,
	V_agi_err_code,
	V_agi_err_code_info,
	V_key,
	V_computer,		/* 20 */
	V_window_reset,
	V_soundgen,
	V_volume,
	V_max_input_chars,
	V_sel_item,		/* 25 */
	V_monitor
};

/**
 * AGI flags
 */
enum {
	F_ego_water = 0,	/* 0 */
	F_ego_invisible,
	F_entered_cli,
	F_ego_touched_p2,
	F_said_accepted_input,
	F_new_room_exec,	/* 5 */
	F_restart_game,
	F_script_blocked,
	F_joy_sensitivity,
	F_sound_on,
	F_debugger_on,		/* 10 */
	F_logic_zero_firsttime,
	F_restore_just_ran,
	F_status_selects_items,
	F_menus_work,
	F_output_mode,		/* 15 */
	F_auto_restart
};

struct agi_event {
	uint16 data;
	uint8 occured;
};

struct agi_object {
	int location;
	char *name;
};

struct agi_word {
	int id;
	char *word;
};

struct agi_dir {
	uint8 volume;
	uint32 offset;
	uint32 len;
	uint32 clen;
	uint8 flags;
	/* 0 = not in mem, can be freed
	 * 1 = in mem, can be released
	 * 2 = not in mem, cant be released
	 * 3 = in mem, cant be released
	 * 0x40 = was compressed
	 */
};

struct agi_block {
	int active;
	int x1, y1;
	int x2, y2;
	uint8 *buffer;		/* used for window background */
};

#define EGO_VIEW_TABLE	0
#define	HORIZON		36
#define _WIDTH		160
#define _HEIGHT		168

/**
 * AGI game structure.
 * This structure contains all global data of an AGI game executed
 * by the interpreter.
 */
struct agi_game {
#define STATE_INIT	0x00
#define STATE_LOADED	0x01
#define STATE_RUNNING	0x02
	int state;		/**< state of the interpreter */

	char name[8];	/**< lead in id (e.g. `GR' for goldrush) */
	char id[8];		/**< game id */
	uint32 crc;		/**< game CRC */

	/* game flags and variables */
	uint8 flags[MAX_FLAGS]; /**< 256 1-bit flags */
	uint8 vars[MAX_VARS];   /**< 256 variables */

	/* internal variables */
	int horizon;			/**< horizon y coordinate */
	int line_status;		/**< line number to put status on */
	int line_user_input;	/**< line to put user input on */
	int line_min_print;		/**< num lines to print on */
	int cursor_pos;			/**< column where the input cursor is */
	uint8 input_buffer[40]; /**< buffer for user input */
	uint8 echo_buffer[40]; 	/**< buffer for echo.line */
	int keypress;
#define INPUT_NORMAL	0x01
#define INPUT_GETSTRING	0x02
#define INPUT_MENU	0x03
#define INPUT_NONE	0x04
	int input_mode;			/**< keyboard input mode */
	int input_enabled;		/**< keyboard input enabled */
	int lognum;				/**< current logic number */

	/* internal flags */
	int player_control;		/**< player is in control */
	int quit_prog_now;		/**< quit now */
	int status_line;		/**< status line on/off */
	int clock_enabled;		/**< clock is on/off */
	int exit_all_logics;	/**< break cycle after new.room */
	int picture_shown;		/**< show.pic has been issued */
	int has_prompt;			/**< input prompt has been printed */
#define ID_AGDS		0x00000001
#define ID_AMIGA	0x00000002
	int game_flags;			/**< agi options flags */

	uint8 pri_table[_HEIGHT];/**< priority table */

	/* windows */
	uint32 msg_box_ticks;	/**< timed message box tick counter */
	struct agi_block block;
	struct agi_block window;
	int has_window;

	/* graphics & text */
	int gfx_mode;
	char cursor_char;
	unsigned int color_fg;
	unsigned int color_bg;
	uint8 *sbuf;			/**< 160x168 AGI screen buffer */

	/* player command line */
	struct agi_word ego_words[MAX_WORDS];
	int num_ego_words;

	unsigned int num_objects;

	struct agi_event ev_keyp[MAX_DIRS];  /**< keyboard keypress events */
	char strings[MAX_STRINGS + 1][MAX_STRINGLEN]; /**< strings */

	/* directory entries for resources */
	struct agi_dir dir_logic[MAX_DIRS];
	struct agi_dir dir_pic[MAX_DIRS];
	struct agi_dir dir_view[MAX_DIRS];
	struct agi_dir dir_sound[MAX_DIRS];

	/* resources */
	struct agi_picture pictures[MAX_DIRS]; 	/**< AGI picture resources */
	struct agi_logic logics[MAX_DIRS];		/**< AGI logic resources */
	struct agi_view views[MAX_DIRS];		/**< AGI view resources */
	struct agi_sound sounds[MAX_DIRS];		/**< AGI sound resources */

	/* view table */
	struct vt_entry view_table[MAX_VIEWTABLE];

	int32 ver;								/**< detected game version */

	int simple_save;						/**< select simple savegames */
};

class AgiLoader {
private:
	int int_version;
	AgiEngine *_vm;

public:

	AgiLoader() {}
	virtual ~AgiLoader() {}

	virtual int init() = 0;
	virtual int deinit() = 0;
	virtual int detect_game() = 0;
	virtual int load_resource(int, int) = 0;
	virtual int unload_resource(int, int) = 0;
	virtual int load_objects(const char *) = 0;
	virtual int load_words(const char *) = 0;
	virtual int version() = 0;
	virtual void setIntVersion(int) = 0;
	virtual int getIntVersion() = 0;
};

class AgiLoader_v2 : public AgiLoader {
private:
	int int_version;
	AgiEngine *_vm;

	int load_dir(struct agi_dir *agid, const char *fname);
	uint8 *load_vol_res(struct agi_dir *agid);

public:

	AgiLoader_v2(AgiEngine *vm) {
		_vm = vm;
		int_version = 0;
	}

	virtual int init();
	virtual int deinit();
	virtual int detect_game();
	virtual int load_resource(int, int);
	virtual int unload_resource(int, int);
	virtual int load_objects(const char *);
	virtual int load_words(const char *);
	virtual int version();
	virtual void setIntVersion(int);
	virtual int getIntVersion();
};

class AgiLoader_v3 : public AgiLoader {
private:
	int int_version;
	AgiEngine *_vm;

	int load_dir(agi_dir *agid, Common::File *fp, uint32 offs, uint32 len);
	uint8 *load_vol_res(agi_dir *agid);

public:

	AgiLoader_v3(AgiEngine *vm) {
		_vm = vm;
		int_version = 0;
	}

	virtual int init();
	virtual int deinit();
	virtual int detect_game();
	virtual int load_resource(int, int);
	virtual int unload_resource(int, int);
	virtual int load_objects(const char *);
	virtual int load_words(const char *);
	virtual int version();
	virtual void setIntVersion(int);
	virtual int getIntVersion();
};

class GfxMgr;
class SpritesMgr;
class Menu;
class SaveGameMgr;

extern struct Mouse g_mouse;

/* Image stack support */
struct image_stack_element {
	uint8 type;
	uint8 pad;
	int16 parm1;
	int16 parm2;
	int16 parm3;
	int16 parm4;
	int16 parm5;
	int16 parm6;
	int16 parm7;
};

struct string_data {
	int x;
	int y;
	int len;
	int str;
};

#define TICK_SECONDS 20

#define KEY_QUEUE_SIZE 16

class AgiEngine : public ::Engine {
	int _gameId;

protected:
	int init();
	int go();
	void shutdown();
	void initialize();

	bool initGame();

public:
	AgiEngine(OSystem *syst);
	virtual ~AgiEngine();
	int getGameId() {
		return _gameId;
	}

	const AGIGameDescription *_gameDescription;

private:

	int _key_queue[KEY_QUEUE_SIZE];
	int _key_queue_start;
	int _key_queue_end;

	int check_priority(struct vt_entry *v);
	int check_collision(struct vt_entry *v);
	int check_position(struct vt_entry *v);

	uint32 match_version(uint32 crc);

public:
	struct agi_game game;
	struct agi_object *objects;	/* objects in the game */

	struct string_data stringdata;

	AgiLoader *loader;	/* loader */

	Common::RandomSource *_rnd;
	const char *_savePath;

	volatile uint32 clock_count;

	uint8 *intobj;

	Menu* menu;

	char last_sentence[40];

	SpritesMgr *_sprites;
	GfxMgr *_gfx;
	SoundMgr *_sound;
	PictureMgr *_picture;
	SaveGameMgr *_saveGameMgr;

	#define INITIAL_IMAGE_STACK_SIZE 32

	int stack_size;
	struct image_stack_element *image_stack;
	int image_stack_pointer;

	void clear_image_stack();
	void record_image_stack_call(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7);
	void replay_image_stack_call(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7);
	void release_image_stack();

	struct agi_debug _debug;
	struct agi_options opt;

	int _key_control;
	int _key_alt;

	Console *_console;

	int agiInit();
	int agiDeinit();
	int agiVersion();
	int agiGetRelease();
	void agiSetRelease(int);
	int agiDetectGame();
	int agiLoadResource(int, int);
	int agiUnloadResource(int, int);
	void agiUnloadResources();

	void agiTimerLow();
	int agiGetKeypressLow();
	int agiIsKeypressLow();
	static void agiTimerFunctionLow(void *refCon);
	void initPriTable();

	void new_input_mode(int);
	void old_input_mode();

	int getflag(int);
	void setflag(int, int);
	void flipflag(int);
	int getvar(int);
	void setvar(int, int);
	void decrypt(uint8 * mem, int len);
	void release_sprites();
	int main_cycle();
	int view_pictures();
	int parse_cli(int, char **);
	int run_game();
	void inventory();
	void list_games();
	uint32 match_crc(uint32, char *, int);
	int v2id_game();
	int v3id_game();
	int v4id_game(uint32 ver);
	void update_timer();
	int get_app_dir(char *app_dir, unsigned int size);

	int setup_v2_game(int ver, uint32 crc);
	int setup_v3_game(int ver, uint32 crc);

	void new_room(int n);
	void reset_controllers();
	void interpret_cycle();
	int play_game();

	void printItem(int n, int fg, int bg);
	int findItem();
	int showItems();
	void selectItems(int n);

	void processEvents();

	// Objects
	int show_objects();
	int decode_objects(uint8 *mem, uint32 flen);
	int load_objects(const char *fname);
	int alloc_objects(int);
	void unload_objects();
	const char *object_name(unsigned int);
	int object_get_location(unsigned int);
	void object_set_location(unsigned int, int);

	// Logic
	int decode_logic(int);
	void unload_logic(int);
	int run_logic(int);
	void patch_logic(int n);

	void debug_console(int, int, const char *);
	int test_if_code(int);
	void execute_agi_command(uint8, uint8 *);

	// View
private:

	void _set_cel(vt_entry *v, int n);
	void _set_loop(vt_entry *v, int n);
	void update_view(vt_entry *v);

public:

	void set_cel(vt_entry *, int);
	void set_loop(vt_entry *, int);
	void set_view(vt_entry *, int);
	void start_update(vt_entry *);
	void stop_update(vt_entry *);
	void update_viewtable();
	void unload_view(int);
	int decode_view(int);
	void add_to_pic(int, int, int, int, int, int, int);
	void draw_obj(int);
	bool is_ego_view(const vt_entry *v);

	// Words
	int show_words();
	int load_words(const char *);
	void unload_words();
	int find_word(char *word, int *flen);
	void dictionary_words(char *);

	// Motion
private:
	int check_step(int delta, int step);
	int check_block(int x, int y);
	void changepos(struct vt_entry *v);
	void motion_wander(struct vt_entry *v);
	void motion_followego(struct vt_entry *v);
	void motion_moveobj(struct vt_entry *v);
	void check_motion(struct vt_entry *v);
public:
	void check_all_motions();
	void move_obj(vt_entry *);
	void in_destination(vt_entry *);
	void fix_position(int);
	void update_position();
	int get_direction(int x0, int y0, int x, int y, int s);

	// Keyboard
	void init_words();
	void clean_input();
	int do_poll_keyboard();
	void clean_keyboard();
	void handle_keys(int);
	void handle_getstring(int);
	int handle_controller(int);
	void get_string(int, int, int, int);
	uint16 agi_get_keypress();
	int wait_key();
	int wait_any_key();

	// Text
public:
	#define MAXWORDLEN 24

	typedef Common::String String;

	int message_box(const char *);
	int selection_box(const char *, const char **);
	void close_window(void);
	void draw_window(int, int, int, int);
	void print_text(const char *, int, int, int, int, int, int, bool checkerboard = false);
	void print_text_console(const char *, int, int, int, int, int);
	int print(const char *, int, int, int);
	char *word_wrap_string(char *, int *);
	char *agi_sprintf(const char *);
	void write_status(void);
	void write_prompt(void);
	void clear_lines(int, int, int);
	void flush_lines(int, int);
	bool predictiveDialog(void);

private:
	void print_status(const char *message, ...);
	void print_text2(int l, const char *msg, int foff, int xoff, int yoff, int len, int fg, int bg, bool checkerboard = false);
	void blit_textbox(const char *p, int y, int x, int len);
	void erase_textbox();
	char *safe_strcat(char *s, const char *t);
	void loadDict(void);
	bool matchWord(void);

	typedef Common::HashMap<String, String, Common::CaseSensitiveString_Hash, Common::CaseSensitiveString_EqualTo> DictMap;
	DictMap _dict;
	Common::StringList _dictKeys;
	String _currentCode;
	String _currentWord;
	String _matchedWord;
	int _wordNumber;
	uint _wordPosition;
	bool _nextIsActive;
	bool _addIsActive;
public:
	char _predictiveResult[40];
};

} // End of namespace Agi

#endif // AGI_H
