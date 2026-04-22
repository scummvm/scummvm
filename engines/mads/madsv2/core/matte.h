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

#ifndef MADS_CORE_MATTE_H
#define MADS_CORE_MATTE_H

#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/anim.h"
#include "mads/madsv2/core/tile.h"

namespace MADS {
namespace MADSV2 {

#define SERIES_LIST_SIZE        50

#define SERIES_BONUS_SIZE       1
#define SERIES_BONUS_OBJECT     1     /* Bonus number of spinning objects */


#define IMAGE_LIST_SIZE         50
#define MESSAGE_LIST_SIZE       40

#define IMAGE_INTER_LIST_SIZE   50

#define MATTE_LIST_SIZE         (IMAGE_LIST_SIZE + MESSAGE_LIST_SIZE)
#define FIRST_MESSAGE_MATTE     IMAGE_LIST_SIZE

#define MATTE_FX_FADE_FROM_BLACK      1
#define MATTE_FX_FADE_THRU_BLACK      2
#define MATTE_FX_CORNER_LOWER_LEFT    3
#define MATTE_FX_CORNER_LOWER_RIGHT   4
#define MATTE_FX_CORNER_UPPER_LEFT    5
#define MATTE_FX_CORNER_UPPER_RIGHT   6
#define MATTE_FX_EDGE_LEFT            7
#define MATTE_FX_EDGE_RIGHT           8
#define MATTE_FX_CIRCLE_OUT_SLOW      9
#define MATTE_FX_CIRCLE_IN_SLOW       10
#define MATTE_FX_CIRCLE_OUT_FAST      11
#define MATTE_FX_CIRCLE_IN_FAST       12

#define MATTE_FX_FAST_THRU_BLACK      20
#define MATTE_FX_FAST_AND_FANCY       21


struct Matte {
	int x, y;             /* upper left coords    */
	int xs, ys;             /* sizes                */
	int xh, yh;             /* half sizes           */
	int xc, yc;             /* centers              */
	byte changed;           /* changed this update? */
	byte valid;             /* contains valid matte?*/
	Matte *linked_matte;
};

typedef Matte *MattePtr;

extern SeriesPtr series_list[SERIES_LIST_SIZE + SERIES_BONUS_SIZE]; /* Master sprite list     */

extern char series_name[SERIES_LIST_SIZE][9];   /* Master sprite name list*/
extern byte series_user[SERIES_LIST_SIZE];      /* Master sprite user list*/

extern Image image_list[IMAGE_LIST_SIZE];      /* Master image list      */
extern Matte matte_list[MATTE_LIST_SIZE];      /* Master matte list      */

extern byte depth_list_id[MATTE_LIST_SIZE];            /* Depth list for sorting */
extern word depth_list[MATTE_LIST_SIZE];

extern Message message_list[MESSAGE_LIST_SIZE];

extern ImageInter image_inter_list[IMAGE_INTER_LIST_SIZE]; /* Interface lists */
extern Matte      matte_inter_list[IMAGE_INTER_LIST_SIZE];

extern int series_list_marker;          /* # of series list entries in use */

extern int picture_view_x;              /* Current viewing offset for upper left */
extern int picture_view_y;

extern int viewing_at_x;                /* Screen X coordinate of left  window column  */
extern int viewing_at_y;                /* Screen Y coordinate of upper window line    */
extern int inter_viewing_at_y;          /* Screen Y coordinate of upper interface line */

extern int matte_disable_screen_update; /* Matte driver updates live screen  */

extern byte attr_packed;                /* Flag if attributes packed         */

extern byte image_marker;               /* Number of active images           */
extern byte image_inter_marker;         /* Number of active interface images */

extern Buffer scr_main;                 /* Full-screen sized work buffer     */
extern Buffer scr_work;                 /* Background picture sized buffer   */
extern Buffer scr_inter;                /* Interface screen sized buffer     */

extern Buffer scr_orig;                 /* Buffer to hold full background picture */
extern Buffer scr_depth;                /* Buffer to hold depth codes             */
extern Buffer scr_walk;                 /* Buffer to hold walk codes              */
extern Buffer scr_special;              /* Buffer to hold special codes           */

extern Buffer scr_inter_orig;           /* Buffer to hold original interface      */

extern int matte_guard_depth_0;         /* Guard popup window depth  */

extern int work_screen_ems_handle;      /* Work screen in EMS        */

extern TileMapHeader picture_map, depth_map;      /* Tile maps         */
extern TileResource  picture_resource, depth_resource; /* Tile resources    */




void matte_init(int init_series);
int  matte_map_work_screen(void);


void matte_frame(int special_effect, int full_screen);

//void matte_init_series(void);
int  matte_load_series(const char *name, int load_flags, int bonus_series_number);
void matte_deallocate_series(int id, int free_memory);
int  matte_allocate_series(SeriesPtr series,
	int bonus_series_number);

//void matte_init_messages(void);
int  matte_add_message(FontPtr font, char *text,
	int x, int y, int message_color,
	int auto_spacing);
void matte_clear_message(int handle);

int matte_allocate_image(void);

void matte_refresh_work(void);

void matte_inter_frame(int update_live, int clear_chaff);
int  matte_allocate_inter_image(void);
void matte_refresh_inter(void);

} // namespace MADSV2
} // namespace MADS

#endif
