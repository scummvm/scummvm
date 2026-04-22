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

#ifndef MADS_CORE_INTER_H
#define MADS_CORE_INTER_H

#include "common/serializer.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/anim.h"

namespace MADS {
namespace MADSV2 {

#define inter_columns           5
#define inter_delta_y           8

#define inter_base_x            0
#define inter_size_y            (video_y - display_y)
#define inter_base_y            (video_y - inter_size_y)

#define command_base_x          2
#define command_base_y          3
#define command_delta_x         32

#define inven_base_x            90
#define inven_base_y            3
#define inven_delta_x           69

#define action_base_x           240
#define action_base_y           3
#define action_delta_x          (video_x - action_base_x)

#define dialog_delta_x          318

#define inter_object_base_x     160
#define inter_object_base_y     3
#define inter_object_size_x     72
#define inter_object_size_y     36

#define INTER_SPINNING_OBJECT   200
#define INTER_TEXT_UPDATE       201
#define INTER_OBJECT_SPACE      1200   /* Memory region for objects */

#define INTER_STRING_LENGTH     64

#define INTER_COMMANDS          (inter_columns * 2)

#define INVEN_MAX_OBJECTS       20      /* Can only carry ... objects */

#define INTER_BUILDING_SENTENCES      0       /* Normal sentence building mode */
#define INTER_CONVERSATION            1       /* Special conversation mode     */
#define INTER_LIMITED_SENTENCES       2       /* Build only from room hotspots */

#define ABSOLUTE_MODE   0     /* Hotspot mouse cursor is absolute */
#define RELATIVE_MODE   1     /* Hotspot mouse cursor is relative */

#define NORMAL          0
#define LEFT_SELECT     1
#define RIGHT_SELECT    2

#define VERB_ONLY       0
#define VERB_THIS       1
#define VERB_THAT       2


#define PREP_NONE       0
#define PREP_WITH       1
#define PREP_TO         2
#define PREP_AT         3
#define PREP_FROM       4

#define PREP_ON         5
#define PREP_IN         6
#define PREP_UNDER      7
#define PREP_BEHIND     8

#define PREP_RELATIONAL 255


#define SYNTAX_SINGULAR               0       /* "Give me that  ...." */
#define SYNTAX_PLURAL                 1       /* "Give me those ...." */
#define SYNTAX_PARTITIVE              2       /* "Give me some  ...." */
#define SYNTAX_SINGULAR_MASC          3       /* Singular masculine (him) */
#define SYNTAX_SINGULAR_FEM           4       /* Singular feminine  (her) */
#define SYNTAX_SINGULAR_LIVING        5       /* Singular living    (it)  */
#define SYNTAX_MASC_NOT_PROPER        6       /* Masculine, non-proper    */
#define SYNTAX_FEM_NOT_PROPER         7       /* Feminine, non-proper     */

#define MAX_SYNTAX                    8       /* Number of syntax forms   */

/* Mouse stroke type categories */

#define STROKE_NONE             0       /* Unknown or none          */
#define STROKE_COMMAND          1       /* In main verb list        */
#define STROKE_INVEN            2       /* In user inventory        */
#define STROKE_ACTION           3       /* In object-verb list      */
#define STROKE_INTERFACE        4       /* In room hotspot area     */
#define STROKE_SPECIAL_INVEN    5       /* On spinning object box   */
#define STROKE_DIALOG           6       /* Dialog conversation      */
#define STROKE_SCROLL           7       /* Stroke in scroll box     */

#define STROKE_DYNAMIC          8       /* Special mask for dynamic */

#define STROKE_MASK             7       /* Masks out dynamic        */
#define STROKE_MAX              7       /* Highest stroke class     */

#define SCROLL_UP               1       /* Scroll up button         */
#define SCROLL_DOWN             2       /* Scroll down button       */
#define SCROLL_ELEVATOR         3       /* Elevator bar             */
#define SCROLL_THUMB            4       /* Thumb mark (pseudo-item) */

#define inter_scroll_x1         73      /* Scroll bar hotspot boundaries */
#define inter_scroll_x2         81
#define inter_up_y1             3
#define inter_up_y2             9
#define inter_down_y1           35
#define inter_down_y2           41
#define inter_elevator_y1       11
#define inter_elevator_y2       33
#define inter_thumb_y1          14
#define inter_thumb_y2          31

#define inter_thumb_range       ((inter_thumb_y2 - inter_thumb_y1) + 1)

/* Interface status categories */

#define AWAITING_NONE           0
#define AWAITING_COMMAND        1       /* Virgin: waiting for a verb */
#define AWAITING_THIS           2       /* Waiting for 1st object     */
#define AWAITING_THAT           3       /* Waiting for second object  */
#define AWAITING_RIGHT_MOUSE    4       /* Waiting for button release */

#define INTER_MESSAGE_COLOR     254


#define inter_verb            inter_words[0]
#define inter_main_noun       inter_words[1]
#define inter_second_noun     inter_words[2]



struct VerbBuf {
	word id;
	byte verb_type;
	byte prep_type;

	void synchronize(Common::Serializer &s);
};
typedef struct VerbBuf Verb;

extern AnimInterPtr inter_anim;         /* Background animation script    */

extern int inter_auxiliary_click;       /* Clicks during downtime        */

extern int inter_mouse_x;               /* Cooked mouse location X       */
extern int inter_mouse_y;               /* Cooked mouse location Y       */
extern int inter_mouse_type;            /* Hotspot list identifier       */

extern int inter_object_sprite;
extern int inter_object_series;

extern int inter_spinning_objects;      /* Flag if objects rotating     */
extern int inter_animation_running;     /* Flag if background animation */

extern byte *inter_objects_block;   /* Place for loading object series */
extern long inter_objects_block_size;


extern int inter_input_mode;            /* Interface input mode */

extern char *inter_dialog_strings[inter_columns];   /* String pointers for dialog mode */
extern int       inter_dialog_results[inter_columns];   /* Result numbers for dialog mode  */


extern Verb command[INTER_COMMANDS];    /* Main Verb List                 */

extern int inven[INVEN_MAX_OBJECTS];    /* Inventory index                */
extern int inven_num_objects;           /* Inventory size                 */

extern int active_inven;                /* The selected inventory item    */
extern int first_inven;

extern int right_command;              /* Right mouse main verb      */
extern int left_command;               /* Left mouse main verb       */
extern int left_inven;                 /* Left mouse inventory       */
extern int left_action;                /* Left mouse secondary verb  */
extern int right_action;               /* Right mouse secondary verb */


extern int inter_command;               /* Command # index                  */
extern int inter_main_object;           /* Object # index                   */
extern int inter_second_object;         /* Object # index                   */
extern int inter_prep;                  /* Prep # of sentence's preposition */

extern int inter_main_syntax;           /* Syntax form of first noun        */
extern int inter_second_syntax;         /* Syntax form of second noun       */

extern int inter_command_source;        /* Stroke types for sentence components */
extern int inter_main_object_source;
extern int inter_second_object_source;

extern int  inter_look_around;          /* "Look around" command            */

extern int inter_point_established;     /* Clicked point for possible walk-to */
extern int inter_point_x;
extern int inter_point_y;

extern int inter_awaiting;              /* Current awaiting status          */


extern int inter_words[3];              /* Vocabulary words for sentence    */
extern char inter_sentence[64];         /* Sentence building buffer         */

extern int inter_sentence_ready;        /* Flag if a sentence ready to go   */
extern int inter_report_hotspots;       /* True = show hotspots even with no button pressed */

extern int inter_force_rescan;          /* Force mouse rescan even if not changed */
extern int inter_base_hotspots;         /* Number of basic room hotspots          */

extern long inter_base_time;            /* Timing marker for interface      */


extern int  scrollbar_active;           /* Active scrollbar item            */
extern int  scrollbar_elevator;
extern int  scrollbar_old_active;
extern int  scrollbar_old_elevator;

extern int  scrollbar_stroke_type;
extern int  scrollbar_quickly;
extern long scrollbar_base_timing;


extern int inter_spot_class;
extern int inter_spot_index;
extern int inter_spot_id;

extern byte spot_base[STROKE_MAX];      /* Hotspot list base markers (by stroke type) */


extern char istring_prep_names[9][7];               /* Preposition strings */

extern char istring_look_around[12];                /* Misc. hard coded    */
extern char istring_use[5];
extern char istring_to[4];
extern char istring_walk_to[9];
extern char istring_space[2];

extern char istring_object_pronoun[MAX_SYNTAX][8];  /* Object pronouns     */
extern char istring_subject_pronoun[MAX_SYNTAX][8]; /* Subject pronouns    */
extern char istring_demonstrative[MAX_SYNTAX][8];   /* Demonstratives      */

extern int  inter_object_id;
extern char inter_object_buf[20];
extern void (*inter_object_routine)();


/*
 * inter_prepare_background()
 * Writes all relevant words to the interface work buffer.
 */
extern void inter_prepare_background(void);
/*
 * inter_setup_hotspots()
 * Sets up the hotspot list for the interface module.  Hotspots only
 * need to be set up once for each room.
 */
extern void inter_setup_hotspots(void);
/*
 * inter_set_active_inven()
 * Picks a new active object from the inventory list.  This involves
 * changing the list of object-specific verbs.
 *
 * @param new_active	
 */
extern void inter_set_active_inven(int inven_id);
/*
 * inter_give_to_player ()
 * Adds the specified object to the player's inventory.
 *
 * @param this_	
 */
extern void inter_give_to_player(int this_);
/*
 * inter_move_object()
 * Moves an object to a new location, taking into account any
 * player_inventory problems.
 *
 * @param object_id	
 * @param location	
 */
extern void inter_move_object(int object_id, int location);
/*
 * inter_init_sentence()
 * Begins accepting a new sentence.
 */
extern void inter_init_sentence(void);
/*
 * inter_main_loop()
 * The main interface loop.  Called by run-time kernel to perform one
 * round of interface checking.  Provides mouse interface for the whole
 * program.
 *
 * @param allow_input	
 */
extern void inter_main_loop(int allow_input);

extern void inter_screen_update(void);

extern void inter_turn_off_object(void);
extern void inter_spin_object(int object_id);
extern int  inter_allocate_objects(void);
extern void inter_deallocate_objects(void);

extern void inter_reset_dialog(void);
extern int inter_add_dialog(char *string, int result);

extern int inter_load_background(const char *name, Buffer *target);

} // namespace MADSV2
} // namespace MADS

#endif
