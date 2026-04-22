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

#ifndef MADS_CORE_ANIM_H
#define MADS_CORE_ANIM_H

#include "common/stream.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/font.h"
#include "mads/madsv2/core/image.h"
#include "mads/madsv2/core/room.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/tile.h"

namespace MADS {
namespace MADSV2 {


#define anim_file_version   "3.02"      /* Animate definition file version */

/* Flags to be passed to anim_load().  Note that all PAL_MAP... flags */
/* can be passed through as well.                                     */

#define ANIM_LOAD_TRANSLATE   0x0001    /* Translate to 16 colors     */
#define ANIM_LOAD_BACKGROUND  0x0100    /* Load background picture    */
#define ANIM_LOAD_BACK_ONLY   0x0200    /* Load background only       */
#define ANIM_INTERFACE        0x4000

/* these are mapped to misc[] in the AnimFile structure below */

#define depth_coded         misc[0]
#define misc_any_packed     misc[1]
#define misc_packed_series  misc[2]
#define misc_peel_x         misc[3]
#define misc_peel_y         misc[4]
#define misc_peel_rate      misc[5]
#define misc_slow_fade      misc[6]

#define misc_no_catchup     misc[7]

#define seg_misc_coordinate_style       misc[0]
#define seg_misc_displace_x             misc[1]
#define seg_misc_displace_y             misc[2]

#define seg_misc_disp_good_1            misc[3]  /* holds magic number (problem w/ uninitialized data) */
#define seg_misc_disp_good_2            misc[4]
#define SEG_MAGIC_1                     0x1234
#define SEG_MAGIC_2                     0x5678

#define seg_misc_accel_x                misc[5]
#define seg_misc_accel_y                misc[6]

#define seg_misc_special_scale          misc[7]

#define SEG_SPECIAL_SCALE_UP          0x8000
#define SEG_SPECIAL_SCALE_DOWN        0x4000
#define SEG_SPECIAL_SCALE_MASK        0xf000

#define AA_MAX_SERIES            50     /* Maximum # of series        */
#define AA_MAX_SEGMENTS          25     /* Maximum # of segments      */
#define AA_MAX_FRAMES           500     /* Maximum # of frames        */
#define AA_MAX_IMAGES           500     /* Maximum # of images        */
#define AA_MAX_SPEECH            25     /* Maximum # of speech queues */

#define AA_MAX_IMAGES_PER_FRAME 250     /* Maximum images in one frame */

#define AA_MAX_SPAWNED          2       /* Maximum # of spawns in random */

#define AA_MAX_CHANGES          100     /* Maximum # of speech changes   */


#define AA_ROOM                 1       /* Use MADS room as background */
#define AA_FULLSIZE             2       /* Use MADS full screen room   */
#define AA_BLACKSCREEN          3       /* Black screen background     */
#define AA_INTERFACE            4       /* Use MADS interface screen   */


#define AA_DONTCARE             -1

#define AA_LINEAR               1       /* loop_mode settings */
#define AA_PINGPONG             2
#define AA_STAMP                9       /* FOR KERNEL SEQUENCES ONLY */

#define AA_ONCE                 1       /* repeat_mode settings */
#define AA_MULTIPLE             2

#define AA_ROOMBASED            -1      /* depth/scale settings */
#define AA_MANUAL               -2

#define AA_ABSOLUTEFRAME        1       /* framing settings */
#define AA_DELTAFIRST           2
#define AA_DELTALAST            3
#define AA_ABSOLUTEFIRST        4
#define AA_ABSOLUTELAST         5
#define AA_RANDOMACCESS         6

#define AA_NOTHING              0             /* No timing marks      */
#define AA_MARK                 0x8000        /* Timing MARK portion  */
#define AA_TIMING               0x7fff        /* Timing TICKS portion */

#define AA_LOAD_SOUND         0x8000          /* Load sound file  */
#define AA_LOAD_SPEECH        0x4000          /* Load speech file */
#define AA_LOAD_FONT          0x2000          /* Load font file   */

#define AA_NO_SPEECH          (-1)            /* No speech queue       */

#define AA_SPEECH             0x8000          /* Use speech always     */
#define AA_TEXT               0x4000          /* Use text always       */
#define AA_SPEECH_XOR_TEXT    0x2000          /* Use speech or text    */
#define AA_SPEECH_AND_TEXT    0x1000          /* Use speech and text   */
#define AA_SPEECH_NOR_TEXT    0x0800          /* Use neither           */

#define AA_ABORTABLE          0x8000          /* Segment can be aborted           */
#define AA_CONVENTIONAL       0x4000          /* Segment uses conventional memory */
#define AA_PRELOAD            0x2000          /* Segment preloads (conv. only)    */


#define AA_ERR_OPENFILE                 -128
#define AA_ERR_WRITEFILE                -129
#define AA_ERR_READFILE                 -130
#define AA_ERR_NOMOREMEMORY             -131
#define AA_ERR_MAKEDIALOG               -132
#define AA_ERR_TOOMANYIMAGES            -133
#define AA_ERR_TOOMANYFRAMES            -134
#define AA_ERR_TOOMANYSEGMENTS          -135
#define AA_ERR_TOOMANYSPEECH            -136
#define AA_ERR_NODAMNFONT               -137
#define AA_ERR_BADDEFINITION            -138
#define AA_ERR_OLDDEFINITION            -139


#define IMAGE_UNSCALED                  0xff

#define IMAGE_UPDATE_READY              0x40  /* Ready for live screen   */

#define IMAGE_UPDATE                     1    /* Image needs redraw      */
#define IMAGE_STATIC                     0    /* Image remains on screen */
#define IMAGE_ERASE                     -1    /* Image erased & removed  */
#define IMAGE_REFRESH                   -2    /* Full buffer refresh     */
#define IMAGE_OVERPRINT                 -3    /* Interface overprint     */
#define IMAGE_DELTA                     -4    /* Image drawn to scr_orig */
#define IMAGE_FULLUPDATE                -5    /* Interface area erasure  */

#define IMAGE_UPDATE_ONLY               -20   /* Update live screen only */


struct SegmentBuf {
	int    seg_id;

	char   name[32];

	int    series_id;
	int    mirror_series;
	int    start_sprite;
	int    end_sprite;
	int    sprite_loop_mode;
	int    sprite_repeat_mode;
	double loop_direction;

	int    framing;
	int    first_frame;
	int    last_frame;

	byte   depth_mode;
	byte   low_level_change;

	int    scale_mode;
	int    front_y, back_y;
	int    front_scale, back_scale;

	int    probability;

	byte   spawn[AA_MAX_SPAWNED];
	int    spawn_frame[AA_MAX_SPAWNED];
	byte   sound;
	int    sound_frame;

	int    start_x, start_y;
	int    end_x, end_y;

	double delta_x, delta_y;

	int    auto_locating;

	int    speech;

	word   misc[10];

	int    num_sprite_changes;
	word   sprite_change_frame[AA_MAX_CHANGES];
	word   sprite_change_target[AA_MAX_CHANGES];
};

typedef struct SegmentBuf Segment;
typedef Segment *SegmentPtr;


#define zone_low      spawn_frame[0]
#define zone_high     spawn_frame[1]


struct ImageEditBuf {
	int frame_id;
	byte segment_id;

	byte series_id;
	int sprite_id;
	int x, y;
	byte depth;
	byte scale;
};

typedef struct ImageEditBuf ImageEdit;
typedef ImageEdit *ImageEditPtr;

struct ImageInter {
	int16 flags;
	byte segment_id;
	byte series_id;
	byte sprite_id;
	int16 x;
	byte y;

	static constexpr int SIZE = 2 + 1 + 1 + 1 + 2 + 1;
	void load(Common::SeekableReadStream *src);
};
typedef ImageInter *ImageInterPtr;


struct MessageBuf {
	char active;
	char spacing;
	int x, y;
	int xs, ys;
	int status;
	int main_color;
	FontPtr font;
	char *text;
};

typedef struct MessageBuf Message;
typedef Message *MessagePtr;



struct FrameEditBuf {
	byte sound;
	word ticks;
	word timing;
	word view_x;
	word view_y;
	char yank_x;
	char yank_y;
};

typedef struct FrameEditBuf FrameEdit;
typedef FrameEdit *FrameEditPtr;


struct Frame {
	byte sound;         /* what sound cue to play                             */
	byte speech;        /* what speech record to activate                     */
	word ticks;         /* how many ticks for this frame (before? after?)     */
	word view_x;        /* where the pan set currently                        */
	word view_y;
	int8 yank_x;        /* for backgrounds which wrap around, like starfields */
	int8 yank_y;

	static constexpr int SIZE = 1 + 1 + 2 + 2 + 2 + 1 + 1;
	void load(Common::SeekableReadStream *src);
};
typedef Frame *FramePtr;


struct SegmentInter {
	int16 probability;
	int16 num_images;
	int16 first_image;
	int16 last_image;
	int16 counter;
	byte spawn[AA_MAX_SPAWNED];
	int16 spawn_frame[AA_MAX_SPAWNED];
	byte sound;
	int16 sound_frame;

	static constexpr int SIZE = 5 * 2 + AA_MAX_SPAWNED + AA_MAX_SPAWNED * 2 + 1 + 2;
	void load(Common::SeekableReadStream *src);
};
typedef SegmentInter *SegmentInterPtr;


struct Speech {
	int16 resource_id;            /* Speech segment id in resource file */
	char text[60];                /* Text to be displayed     */
	byte misc[3];                 /* 3 extra bonus bytes      */
	byte sound;                   /* Sound to be used         */
	/* pl SpeechDirPtr speech;  */      /* Binary speech pointer    */
	int16 x, y;                   /* Text coordinates         */
	int16 display_condition;      /* Condition for display    */
	RGBcolor color[2];            /* Colors for text display  */
	word flags;                   /* Segment flags            */
	int16 speech_loops;           /* Loops if speech active   */
	int16 non_speech_loops;       /* Loops if speech inactive */
	int16 segment_to_loop;        /* Segment ID to be looped  */
	int16 first_frame;            /* First frame of segment   */
	int16 last_frame;             /* Last frame of segment    */
	int16 first_image;            /* First image number       */

	static constexpr int SIZE = 2 + 60 + 3 + 1 + 2 + 2 + 2 + 2 * RGBcolor::SIZE + 7 * 2;
	void load(Common::SeekableReadStream *src);
};

typedef Speech *SpeechPtr;

/* daa file header */

struct AnimDefBuf {
	int num_series;
	int num_segments;
	int num_frames;
	int num_images;
	int num_speech;

	int timing_mode;

	int misc[10];

	word load_flags;
	int font_auto_spacing;

	int background_type;
	int background_xs;
	int background_ys;
	int background_room;
	char background_name[80];

	char sound_file_name[80];

	char name[80];
	char desc[80];

	char series_name[AA_MAX_SERIES][80];

	char background_depth[80];

	char speech_file[80];
	char font_file[80];
};


typedef struct AnimDefBuf AnimDef;
typedef AnimDef *AnimDefPtr;

/* aa file header */

struct AnimFile {
	uint16 num_series;
	uint16 num_frames;
	uint16 num_images;
	uint16 num_speech;
	word load_flags;
	uint16 font_auto_spacing;
	uint16 background_type;                 /* black, or room, or whatever    */
	uint16 background_room;                 /* room number to load            */
	uint16 misc[10];                        /* see MISC_ defines above        */
	char background_name[13];            /* if needed                      */
	char series_name[AA_MAX_SERIES][13]; /* filenames for all your series  */
	char sound_file_name[13];
	char background_depth[13];
	char speech_file[13];
	char font_file[13];

	static constexpr int SIZE = (8 * 2) + (10 * 2) + 13 +
		(AA_MAX_SERIES * 13) + 13 + 13 + 13 + 13 +
		1; // structure padding
	void load(Common::SeekableReadStream *src);
};

/* runtime memory image for an animation */

struct Anim : public AnimFile {
	FontPtr font;
	SpeechPtr speech;

	ImagePtr image;						/* to the array of images */
	FramePtr frame;						/* to the array of frames */
	SeriesPtr series[AA_MAX_SERIES];	/* pointers to series     */

	int series_id[AA_MAX_SERIES];
};
typedef Anim *AnimPtr;

/* the interface version of the above */

struct AnimInter : public AnimFile {
	FontPtr font;
	SpeechPtr speech;

	ImageInterPtr image;
	SegmentInterPtr segment;
	SeriesPtr series[AA_MAX_SERIES];

	int series_id[AA_MAX_SERIES];
};
typedef AnimInter *AnimInterPtr;


extern int anim_error;

int anim_load_background(AnimFile *anim_in,
	Buffer *this_orig,
	Buffer *this_depth,
	TileMapHeader *picture_map,
	TileMapHeader *depth_map,
	TileResource *picture_resource,
	TileResource *depth_resource,
	RoomPtr *room,
	CycleListPtr cycle_list,
	int load_flags, int star_search);


void    anim_unload(AnimPtr anim);

AnimPtr anim_load(const char *file_name,
	Buffer *orig, Buffer *depth,
	TileMapHeader *picture_map,
	TileMapHeader *depth_map,
	TileResource *picture_resource,
	TileResource *depth_resource,
	RoomPtr *room, CycleListPtr cycle_list,
	int load_flags);

int anim_get_sound_info(char *file_name,
	char *sound_file_buffer,
	int *sound_load_flag);

int anim_get_header_info(char *file_name,
	AnimFile *anim_in);


int anim_himem_preload(char *name, int level);

} // namespace MADSV2
} // namespace MADS

#endif
