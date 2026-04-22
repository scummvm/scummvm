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

#ifndef MADS_CORE_KERNEL_H
#define MADS_CORE_KERNEL_H

#include "common/serializer.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/anim.h"
#include "mads/madsv2/core/color.h"
#include "mads/madsv2/core/room.h"

namespace MADS {
namespace MADSV2 {

/* Some handy location macros */

#define NOWHERE                 1               /* Does not exist     */
#define PLAYER                  2               /* Player's posession */
#define HERE                    room_id         /* In current room    */

#define KERNEL_INTERRUPT_STACK_SIZE    256    /* Size of interrupt stack */

#define KERNEL_RESERVED_LOW_COLORS      26    /* Colors to reserve at bottom of palette */
#define KERNEL_RESERVED_HIGH_COLORS     10    /* Colors to reserve at top of palette    */

#define KERNEL_MESSAGE_COLOR_BASE       16    /* 1st color to use for messages          */
#define KERNEL_MESSAGE_COLOR_BASE_2     252   /* 1st color to use for second message    */
#define KERNEL_MESSAGE_COLOR_BASE_3     254   /* 1st color to use for third  message    */

#define KERNEL_OBJECT_COLOR_BASE        246   /* 1st color to use for rotating objects  */

#define KERNEL_SEGMENT_SYSTEM           0xff  /* System related images  */
#define KERNEL_SEGMENT_PLAYER           0xfe  /* Player sprite images   */
#define KERNEL_SEGMENT_ANIMATION_HIGH   0xfd  /* Highest possible anim  */
#define KERNEL_SEGMENT_ANIMATION        0x80  /* Animation based images */

#define KERNEL_MAX_SEQUENCES            30    /* Max concurrent sequences    */

#define KERNEL_MAX_TRIGGERS             5     /* Max triggers for a sequence */

#define KERNEL_TRIGGER_EXPIRE           0     /* Trigger when sequence expires        */
#define KERNEL_TRIGGER_LOOP             1     /* Trigger when sequence loops around   */
#define KERNEL_TRIGGER_SPRITE           2     /* Trigger when sequence reaches sprite */

#define KERNEL_TRIGGER_PARSER     0     /* Sequence triggers parser code */
#define KERNEL_TRIGGER_DAEMON     1     /* Sequence triggers daemon code */
#define KERNEL_TRIGGER_PREPARSE   2     /* Sequence triggers preparser   */

#define KERNEL_DEFAULT                 0      /* Default for parameter      */
#define KERNEL_FIRST                   -1     /* Use first sprite in series */
#define KERNEL_LAST                    -2     /* Use last sprite in series  */

#define KERNEL_SPECIAL_TIMING         0xff    /* Special sequence type for */
					  /* "timing only" triggers    */

#define KERNEL_MESSAGE_ANIM           0x100   /* Message attached to anim  */
#define KERNEL_MESSAGE_ACTIVE         0x80    /* Flag if message is active */
#define KERNEL_MESSAGE_EXPIRED        0x40    /* Flag if message expired   */
#define KERNEL_MESSAGE_CENTER         0x20    /* Message centered at coord */
#define KERNEL_MESSAGE_RIGHT          0x10    /* Message to right of coord */
#define KERNEL_MESSAGE_TELETYPE       0x08    /* Message teletypes in      */
#define KERNEL_MESSAGE_ATTACHED       0x04    /* Message attached to sprite*/
#define KERNEL_MESSAGE_PLAYER         0x02    /* Message attached to player*/
#define KERNEL_MESSAGE_QUOTE          0x01    /* Message has trailing quote*/

#define KERNEL_NONE                   -1      /* No file extension  */
#define KERNEL_SS                     1       /* Sprite series .SS  */
#define KERNEL_AA                     2       /* Animation     .AA  */
#define KERNEL_DAT                    3       /* Room data     .DAT */
#define KERNEL_HH                     4       /* Hot spot list .HH  */
#define KERNEL_ART                    5       /* Room artwork  .ART */
#define KERNEL_INT                    6       /* Interface     .INT */
#define KERNEL_TT                     7       /* Tile          .TT  */
#define KERNEL_MM                     8       /* Map           .MM  */
#define KERNEL_WW                     9       /* Walk codes    .WW? */

#define KERNEL_MAX_MESSAGES           10      /* Maximum messages on screen at once */
#define KERNEL_MAX_MESSAGE_LEN        40      /* Maximum length of text messages    */
#define KERNEL_MESSAGE_INTERVAL       3       /* Ticks between message updates      */

#define KERNEL_GAME_LOAD              0       /* Kernel load state */
#define KERNEL_SECTION_PRELOAD        1
#define KERNEL_SECTION_INIT           2
#define KERNEL_ROOM_PRELOAD           3
#define KERNEL_ROOM_INIT              4
#define KERNEL_ACTIVE_CODE            5

#define KERNEL_MOTION                 0x01    /* Series is in motion                */
#define KERNEL_MOTION_OFFSCREEN       0x02    /* Series expires if moves off screen */

#define KERNEL_MAX_DYNAMIC            16      /* Maximum dynamic hotspots */

#define KERNEL_DYNAMIC_MAX_SEGMENTS   10      /* Max segments triggered by dynamic hotspot */
#define KERNEL_DYNAMIC_NO_ANIM        0xff    /* When no animation segment                 */

#define KERNEL_STARTING_GAME          -1      /* Previous room when starting fresh */
#define KERNEL_RESTORING_GAME         -2      /* Previous room when restoring game */

#define KERNEL_SERIES                 1       /* Synchronize sprite sequence      */
#define KERNEL_PLAYER                 2       /* Synchronize PLAYER sprite series */
#define KERNEL_ANIM                   3       /* Synchronize animation            */
#define KERNEL_NOW                    4       /* Synchronize with current time    */

#define KERNEL_MAX_RANDOM_MESSAGES    4       /* Max # of idle chatter msgs at once     */
#define KERNEL_MAX_RANDOM_QUOTES      8       /* Max # of available idle chatter quotes */
#define KERNEL_RANDOM_MESSAGE_TRIGGER 240     /* Trigger base for idle chatter          */

#define KERNEL_MAX_CONVERSATION       16      /* Max items in a conversation */

#define KERNEL_MAX_ANIMATIONS         10       /* Max number of animations    */

#define KERNEL_SCRATCH_SIZE           256     /* Size of game scratch area   */

#define KERNEL_HOME                   -32000  /* Sprite home location        */

#define KERNEL_STARTUP_POPUP          0x8000
#define KERNEL_STARTUP_FONT           0x4000
#define KERNEL_STARTUP_INTERRUPT      0x2000
#define KERNEL_STARTUP_DEMO           0x1000
#define KERNEL_STARTUP_OBJECTS        0x0800
#define KERNEL_STARTUP_PLAYER         0x0400
#define KERNEL_STARTUP_CURSOR         0x0200
#define KERNEL_STARTUP_CURSOR_SHOW    0x0100
#define KERNEL_STARTUP_VOCAB          0x0080
//#define KERNEL_STARTUP_INTERFACE      0x0040
#define KERNEL_STARTUP_ALL_FLAGS      0xffc0

typedef struct {
	byte active_flag;                   /* Sequence is active */

	byte series_id;                     /* Series list handle */
	byte mirror;                        /* Mirroring flag     */

	int sprite;                         /* Current sprite of sequence */

	int start_sprite;                   /* Lowest sprite to be used */
	int end_sprite;                     /* Highest sprite to be used */

	int loop_mode;                      /* Looping mode (AA_LINEAR/AA_PINGPONG) */
	int loop_direction;                 /* Looping direction (1,-1) */

	byte depth;                         /* Sequence depth (0-15)  */
	byte scale;                         /* Sequence scale (0-100) */

	int dynamic_hotspot;                /* Dynamic hotspot (if any) */

	byte auto_locating;                 /* Sequence runs at home location */
	byte motion;                        /* Sequence moves                 */
	int x, y;                           /* Specific location for sequence */
	int delta_x, delta_y;               /* Delta values (*100)            */
	int sign_x, sign_y;                 /* Delta signs                    */
	int accum_x, accum_y;               /* Delta accumulators (*100)      */

	byte expire;                        /* Loops left till expiration */
	byte expired;                       /* Expiration flag            */

	byte num_triggers;                          /* Number of triggers   */
	byte trigger_type[KERNEL_MAX_TRIGGERS];   /* Trigger info */
	int  trigger_sprite[KERNEL_MAX_TRIGGERS];
	byte trigger_code[KERNEL_MAX_TRIGGERS];

	byte trigger_dest;                  /* Type of code to activate for trigger          */
	int  trigger_words[3];              /* vocabulary words for reactivating parser code */

	word ticks;                         /* Ticks between frames */
	word interval_ticks;                /* Ticks between loops  */

	long base_time;                     /* Clock time of next scheduled update */

	Image last_image;                   /* Copy of last image entered          */
} Sequence;

typedef Sequence *SequencePtr;


typedef struct {
	word flags;                         /* Message flags                        */
	byte sequence_id;                   /* Attached sequence number             */
	byte segment_id;                    /* Animation segment id                 */
	byte strobe_save;                   /* Saved strobe character               */
	byte strobe_save_2;                 /* Second saved strobe character        */
	int color;                          /* Message color                        */
	int x, y;                           /* X and Y position                     */
	int matte_message_handle;           /* Handle in matte message list         */
	int strobe_marker;                  /* Marker in strobe message             */
	int strobe_rate;                    /* Number of ticks between strobes      */
	long strobe_time;                   /* Time for next strobe                 */
	long update_time;                   /* Clock time of next update            */
	long expire_ticks;                  /* Number of ticks until expiration     */
	byte trigger_code;                  /* Trigger code, if any.                */
	byte trigger_dest;                  /* Type of code to activate for trigger */
	int  trigger_words[3];              /* Vocabulary words for reactivating parser code */
	char *message;                  /* Pointer to message text              */
} KernelMessage;

typedef KernelMessage *KernelMessagePtr;


typedef struct {
	byte flags;
	int auto_sequence;
	int x, y, xs, ys;
	int feet_x, feet_y;
	byte facing;
	int vocab_id;
	int verb_id;
	byte valid;
	byte prep;
	byte cursor;
	byte syntax;
	char auto_anim;
	byte auto_segment[KERNEL_DYNAMIC_MAX_SEGMENTS];
} KernelDynamicHotSpot;

typedef KernelDynamicHotSpot *KernelDynamicHotSpotPtr;



typedef struct {
	AnimPtr anim;                       /* Animation pointer                    */
	int     repeat;                     /* Animation should repeat continuously */
	int     cycled;                     /* Animation has cycled back to start   */
	int     sprite_loaded;              /* Current sprite loaded (for streamers)*/
	byte *buffer[2];                /* Double buffering pointers (streamers)*/
	int     buffer_id;                  /* Double buffering index    (streamers)*/
	int     messages;                   /* Messages currently active from anim  */
	int     frame;                      /* Current frame                        */
	int     image;                      /* Animation image marker               */
	int     doomed;                     /* Animation expires this round         */

	int     view_changes;               /* Animation can change picture view    */

	int     trigger_code;               /* Trigger code on expiration           */
	int     trigger_mode;               /* Trigger mode (parser/daemon/etc.)    */
	int     trigger_words[3];           /* Trigger parser words                 */

	long    next_clock;                 /* Next animation update                */

	int     dynamic_hotspot;            /* Attached dynamic hotspot             */

	int     last_frame;                 /* Most recent frame viewed             */
} Animation;


typedef struct {
	long clock;                 /* Current game timing clock        */
	byte fx;                    /* Special FX for this frame        */

	byte video_mode;            /* Video mode for game              */
	byte translating;           /* Flag if translating to 16 colors */

	char sound_card;            /* Game sound card character        */

	byte force_restart;         /* Forces restart of room if true   */

	int  trigger;               /* Game trigger code, if any        */
	int  trigger_setup_mode;    /* Mode which sets up trigger       */
	int  trigger_mode;          /* Mode which activated trigger     */

	byte cheating;              /* Flag if cheating enabled.        */

	int  cursor_x[2];           /* Keyboard cursor locs.            */
	int  cursor_y[2];

	byte paused;                /* Flag if game paused at current frame */
	byte cause_pause;           /* Flag if game should pause this frame */
	byte frame_by_frame;        /* Flag if game should run frame/frame  */

	byte mouse_cursor_point;    /* Flag if mouse cursor loc should be displayed */
	byte memory_tracking;       /* Flag if memory amount should be displayed    */

	byte teleported_in;         /* Flag if player teleported to room    */

	char interface[20];         /* Name of current interface animation  */
	char sound_driver[20];      /* Name of current sound driver         */

	char *quotes;           /* Quote list                           */

	/*    char global_message[80]; */   /* Global message storage area          */

	int activate_menu;          /* Flag to activate a game menu         */

	byte player_tracking;       /* Flag if tracking player location     */
	byte disable_fastwalk;      /* Flag if fastwalk disabled            */

} Kernel;


struct KernelGame {
	byte going;							/* Game is running OK          */
	int16 scratch[KERNEL_SCRATCH_SIZE];	/* Scratch variables for room  */
	int8 difficulty;					/* Difficulty level            */
	int16  last_save;					/* Most recent save slot #     */

	void synchronize(Common::Serializer &s);
};


#ifdef old_animation
#define kernel_animation                kernel_anim[0].anim
#define kernel_animation_cycled         kernel_anim[0].cycled
#define kernel_repeat_animation         kernel_anim[0].repeat
#define kernel_animation_sprite_loaded  kernel_anim[0].sprite_loaded
#define kernel_animation_buffer_id      kernel_anim[0].buffer_id
#define kernel_animation_buffer         kernel_anim[0].buffer
#define kernel_animation_frame          kernel_anim[0].frame
#define kernel_animation_image          kernel_anim[0].image
#define kernel_animation_doomed         kernel_anim[0].doomed
#define kernel_animation_trigger_code   kernel_anim[0].trigger_code
#define kernel_animation_trigger_mode   kernel_anim[0].trigger_mode
#define kernel_animation_trigger_words  kernel_anim[0].trigger_words
#define kernel_animation_next_clock     kernel_anim[0].next_clock
#endif


#define kernel_init_dialog() inter_reset_dialog()
#define kernel_dialog(q)     inter_add_dialog(quote_string(kernel.quotes, q), q)


extern RoomPtr room;            /* Pointer to current room's data */
extern int room_id;             /* Current room #                 */
extern int section_id;          /* Current section #              */
extern int room_variant;        /* Current room attribute variant */

extern int16 new_room;          /* New room # to enter            */
extern int16 new_section;       /* New section # to enter         */

extern int16 previous_room;     /* Last room #                    */
extern int16 previous_section;  /* Last section #                 */

extern int kernel_initial_variant; /* Initial variant to load     */

extern HotPtr room_spots;       /* Pointer to hotspots for this room */
extern int room_num_spots;      /* Number of hotspots for this room  */

extern int kernel_room_series_marker;   /* Flag for room background series */

extern int kernel_room_bound_dif;
extern int kernel_room_scale_dif;

extern int kernel_allow_peel;   /* Flag if peeling buffers allowed */

extern int kernel_panning_speed;
extern int kernel_screen_fade;


extern Animation kernel_anim[KERNEL_MAX_ANIMATIONS];

/*  Old animation stuff
extern AnimPtr kernel_animation;
extern int     kernel_repeat_animation;

extern int     kernel_animation_sprite_loaded;
extern int     kernel_animation_buffer_id;
extern byte *kernel_animation_buffer[2];

extern int     kernel_animation_messages;

extern int     kernel_animation_cycled;
extern int     kernel_animation_frame;
extern int     kernel_animation_image;
extern int     kernel_animation_doomed;
extern int     kernel_animation_trigger_code;
extern int     kernel_animation_trigger_mode;
extern int     kernel_animation_trigger_words[3];
extern long    kernel_animation_next_clock;
*/

extern ShadowList kernel_shadow_main;
extern ShadowList kernel_shadow_inter;

extern int kernel_ok_to_fail_load;

extern int kernel_mode;

extern char kernel_cheating_password[16];
extern int  kernel_cheating_allowed;
extern int  kernel_cheating_forbidden;

extern KernelDynamicHotSpot kernel_dynamic_hot[KERNEL_MAX_DYNAMIC];
extern int kernel_num_dynamic;
extern int kernel_dynamic_changed;

extern SeriesPtr cursor;                /* Mouse cursor series              */

extern int cursor_id;
extern int cursor_last;

extern Kernel kernel;
extern KernelGame game;                 /* Kernel level game data */
extern KernelMessage kernel_message[KERNEL_MAX_MESSAGES];
extern FontPtr kernel_message_font;
extern int kernel_message_spacing;

#define OMR 40  /* OUAF_MAX_ROOMS in global.mac */
extern int16 room_state[OMR];


extern int kernel_load_vocab();

/**
 * Game level system shutdown.
 */
extern void kernel_game_shutdown();

extern void kernel_force_refresh();

/**
 * Game level system startup.
 */
extern int kernel_game_startup(int game_video_mode, int load_flag,
	const char *release_version, const char *release_date);

/**
 * Section level system shutdown.
 */
extern void kernel_section_shutdown();

/**
 * Section level system startup.
 */
extern int kernel_section_startup(int new_section);

/**
 * Room level system shutdown.
 */
extern void kernel_room_shutdown();

extern int kernel_room_startup(int new_room, int initial_variant = 0,
	const char *interface = nullptr, bool new_palette = true, bool barebones = false);

/**
 * Unloads all room series.
 */
extern void kernel_unload_all_series();

/**
 * Loads a sprite series for the room.
 */
extern int kernel_load_series(const char *name, int load_flags);

/*
 * kernel_flip_hotspot()
 * Toggles an interface hotspot (referenced by its vocabulary word)
 * on or off.  Hotspots that are off do not interact with the mouse
 * cursor.
 *
 * @param vocab_code	
 * @param active	
 */
extern void kernel_flip_hotspot(int vocab_code, int active);

/**
 * Toggles an interface hotspot (referenced by its vocabulary word)
 * on or off.  Hotspots that are off do not interact with the mouse cursor.
 *
 * Only hotspots which contain the point (X, Y) are affected.
 */
extern void kernel_flip_hotspot_loc(int vocab_code, int active, int x, int y);

extern Sequence sequence_list[KERNEL_MAX_SEQUENCES];
extern void kernel_seq_init();
extern void kernel_seq_correction(long old_clock, long new_clock);
extern int kernel_seq_add(int series_id, int mirror, int initial_sprite,
	int low_sprite, int high_sprite, int loop_mode, int loop_direction,
	int depth, int scale, int auto_locating, int x, int y,
	word ticks, word interval_ticks, word start_ticks, int expire);
extern int kernel_seq_forward(int series_id, int mirror,
	word ticks, word interval_ticks, word start_ticks, int expire);
extern int kernel_seq_forward_scroll(int series_id, int mirror,
	word ticks, word interval_ticks, word start_ticks, int expire);
extern int kernel_seq_pingpong(int series_id, int mirror,
	word ticks, word interval_ticks, word start_ticks, int expire);
extern int kernel_seq_pingpong_scroll(int series_id, int mirror,
	word ticks, word interval_ticks, word start_ticks, int expire);
extern int kernel_seq_backward(int series_id, int mirror,
	word ticks, word interval_ticks, word start_ticks, int expire);
extern int kernel_seq_backward_scroll(int series_id, int mirror,
	word ticks, word interval_ticks, word start_ticks, int expire);
extern int kernel_seq_stamp(int series_id, int mirror, int sprite);
extern int kernel_seq_stamp_scroll(int series_id, int mirror, int sprite);
extern void kernel_synch(int slave_type, int slave_id,
	int master_type, int master_id);
extern void kernel_player_expire(int sequence_id);
extern void kernel_seq_depth(int sequence_id, int depth);
extern void kernel_seq_scale(int sequence_id, int scale);
extern void kernel_seq_loc(int sequence_id, int x, int y);
extern void kernel_seq_range(int sequence_id, int first, int last);
void kernel_seq_motion(int sequence_id, int flags,
	int delta_x_times_100, int delta_y_times_100);
extern int kernel_seq_trigger(int sequence_id, int trigger_type,
	int trigger_sprite, int trigger_code);
extern int kernel_timing_trigger(int ticks, int trigger_code);
extern int kernel_seq_purge(int sequence_id);
extern void kernel_seq_full_update();
extern void kernel_draw_to_background(int series_id, int sprite_id,
	int x, int y, int depth, int scale);
extern void kernel_seq_delete(int sequence_id);
extern void kernel_seq_update_all();
extern void kernel_seq_player(int sequence_id, int synch_me);
extern void kernel_animation_init();
extern int  kernel_run_animation(const char *name, int trigger_code);

/**
 * Run a displacement animation
 */
extern int kernel_run_animation_disp(char thing, int num, int trigger_code);

/**
 * Run a talking animation
 */
extern int kernel_run_animation_talk(char thing, int num, int trigger_code);

/**
 * Run a writing anim (always edgar)
 */
extern int kernel_run_animation_write(int trigger_code);

/**
 * Run a pointing animation (always abigail)
 */
extern int kernel_run_animation_point(int num, int trigger_code);
extern void kernel_reset_animation(int handle, int frame);
extern void kernel_abort_animation(int handle);
extern void kernel_abort_all_animations();
extern void kernel_doom_all_animations();
extern void kernel_abort_doomed_animations();
extern void kernel_process_all_animations();
extern void kernel_message_init();
extern int  kernel_message_add(char *text, int x, int y, int color,
	long time_on_screen, int trigger_code, int flags);
extern void kernel_message_teletype(int id, int rate, int quote);
extern void kernel_message_attach(int id, int sequence);
extern void kernel_message_delete(int id);
extern void kernel_message_purge();
extern void kernel_message_anim(int id, int anim, int segment);
extern int kernel_message_player(int quote_id, long delay, int trigger);
extern void kernel_message_update_all();
extern void kernel_message_correction(long old_clock, long new_clock);
extern int  kernel_add_dynamic(int vocab_id, int verb_id, byte syntax,
	int auto_sequence, int x, int y, int xs, int ys);
extern int  kernel_dynamic_walk(int id, int feet_x, int feet_y, int facing);
extern void kernel_dynamic_anim(int id, int anim_id, int segment);
extern int  kernel_dynamic_cursor(int id, int cursor);
extern void kernel_delete_dynamic(int id);
extern void kernel_purge_dynamic();
extern void kernel_init_dynamic();
extern int kernel_dynamic_consecutive(int id);
extern void kernel_refresh_dynamic();
extern char *kernel_full_name(int my_room, char type, int num, char *text, int ext);
extern char *kernel_name(char type, int num);
extern char *kernel_interface_name(int num);
extern void kernel_unload_sound_driver();
extern int  kernel_load_sound_driver(const char *name, char sound_card_,
	int sound_board_address_, int sound_board_type_, int sound_board_irq_);
extern void kernel_load_variant(int variant);
extern void kernel_new_palette();
extern void kernel_dump_quotes();
extern void kernel_dump_all();
extern void kernel_dump_walker_only();

/**
 * Initializes a random chatter sequence.  (Parameters end with
 * a zero-terminated list of already loaded quote id's to be
 * used for messages).
 */
extern void kernel_random_messages_init(int max_messages_at_once,
	int min_x, int max_x, int min_y, int max_y,
	int min_y_spacing, int teletype_rate, int color,
	int duration, int quote_id, ...);

/**
 * Returns the number of idle chatter messages currently being displayed.
 */
extern int kernel_check_random();

/**
 * Should be called regularly from daemon code whenever a random
 * message sequence is running (intercepts triggers from terminating
 * messages to determine when to free up control space).
 */
extern void kernel_random_message_server();

/**
 * Called occasionally to (possibly) generate a random message.
 *
 * (generated whenever random(chance_major) <= chance_minor)
 */
extern int kernel_generate_random_message(int chance_major, int chance_minor);

extern void kernel_random_purge();

/**
 * Switches interface modes (INTER_BUILDING_SENTENCES is the normal mode;
 * INTER_CONVERSATION is for dialog scenes), and properly updates the graphic structures.
 */
extern void kernel_set_interface_mode(int mode);

extern void kernel_room_scale(int front_y, int front_scale, int back_y, int back_scale);

/**
 * Room level system shutdown.
 */
extern void kernel_background_shutdown();
extern int kernel_background_startup(int new_room, int initial_variant);

} // namespace MADSV2
} // namespace MADS

#endif
