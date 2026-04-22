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

#ifndef MADS_CORE_SPRITE_H
#define MADS_CORE_SPRITE_H

#include "common/stream.h"
#include "mads/madsv2/core/color.h"
#include "mads/madsv2/core/loader.h"

namespace MADS {
namespace MADSV2 {

#define sprite_file_version   "3.02"

#define misc_largest_block            misc[0]
/* LONG ... uses misc[1] also */
#define misc_is_a_walker              misc[2]

/* Flags to be passed to sprite_series_load().  Note that all of the   */
/* PAL_MAP_... flags can also be passed to this routine (see pal.mac). */

#define SPRITE_LOAD_TRANSLATE         0x0001  /* Translate to 16 colors  */
#define SPRITE_LOAD_HEADER_ONLY       0x0002  /* Load nothing but header */
#define SPRITE_LOAD_WALKER_INFO       0x0004  /* Load walker information */
#define SPRITE_LOAD_SPINNING_OBJECT   0x0008  /* Load spinning object    */


/* Sprite ID masking values for mirroring */

#define SPRITE_MASK     0x7fff  /* Sprite ID mask     */
#define MIRROR_MASK     0x8000  /* Sprite mirror mask */

#define HALF_MIRROR_MASK  0x80  /* Mirror mask for bytes */
#define HALF_SPRITE_MASK  0x7f  /* Sprite mask for bytes */


/* Sprite line packing types:  */
#define SS_DUMP         251     /* Defines a line as DUMP type */
#define SS_RLE          253     /* Defines a line as RLE type */
#define SS_IRLE         254     /* Defines a line as IRLE type */
#define SS_LASTVALID    252     /* Last valid non-macro byte value */

/* Control characters:  */
#define SS_EOI          252     /* End Of Image */
#define SS_EOL          255     /* End of line */
#define SS_RUN          254     /* Marks begining of a run in IRLE */
#define SS_SKIP         253     /* Used as SKIP color */
				/*  -- Never set SS_SKIP to 255! -- (RBR)   */
				/*  -- Would explode color list crap --     */
				/*  -- SS_SKIP must not = COLOR_TRANSPARENT */


#define SPRITE_COLOR_TABLE_SIZE       252

#define MAX_SECONDARY         16      /* Maximum secondary walker sequences */




	/* Legal values for x->source_type */
#define         SS_LBM          1       /* Source file is one frame LBM */
#define         SS_LBM_MULT     2       /* Source file is several LBMs  */
#define         SS_ANM          3       /* Source file is an .ANM file  */
#define         SS_FLI          4       /* Source file is a .FLI file   */

	/* Legal values for x->base_mode */
#define         SS_FIRSTFRAME   1       /* If relative to first frame of ANM */
#define         SS_STATICLBM    2       /* If relative to an LBM */
#define         SS_BACKCOL      3       /* If relative to background color */
#define         SS_INDIVIDUAL   4       /* Individual frames relative to first */
#define         SS_PANNING_ANM  5       /* Relative to panning ANM */
#define         SS_PANNING_FLI  6       /* Relative to panning FLI */


/* STRUCTURES FOR ALL SPRITE LEVELS           */

struct WalkerInfo {
	uint16  num_primary;                   /* Number of primary sequence frames */
	uint16  num_secondary;                 /* Number of secondary sequences     */
	uint16  sequence_start[MAX_SECONDARY]; /* Secondary sequence start frames   */
	uint16  sequence_stop[MAX_SECONDARY];  /* Secondary sequence stop frames    */
	uint16  sequence_chance[MAX_SECONDARY];/* Secondary sequence probability    */

	int16  velocity;                      /* Pixel velocity for walker         */
	byte   frame_rate;                    /* Frame rate for walker             */
	byte   center_of_gravity;             /* Center of gravity displacement    */

	static constexpr int SIZE = 2 + 2 + (2 + 2 + 2) * MAX_SECONDARY + 2 + 1 + 1;
	void load(Load &load_handle);
	void load(Common::SeekableReadStream *src);
};

typedef WalkerInfo *WalkerInfoPtr;



/* DEVELOP TIME STRUCTURES (.DSS Files)       */

/* Develop-time definition of a single sprite */

typedef struct {
	char name[9];           /* Sprite name */
	int x, y;               /* Sprite location */
	int xs, ys;             /* Size of sprite  */
} HagSprite;

typedef HagSprite *HagSpritePtr;


/* Develop-time definition of a sprite series */

typedef struct {
	char name[80];                      /* Name        */
	char desc[80];                      /* Description */

	int  source_type;                   /* Type of source data (see above) */
	char source_name[80];               /* Filename of source file (LBM or ANM) */

	int  delta_series;                  /* Boolean: delta vice absolute */

	int  base_mode;                     /* Sprites relative to what? (see above) */
	char base_lbm[80];                  /* Name of LBM if base_mode==SS_STATICLBM */

	int  pack_by_sprite;                /* Packed by sprite vice series? */

	WalkerInfo walker;                  /* Walker information.               */

	int  offset_x_view;                 /* View offsets for sprites intended */
	int  offset_y_view;                 /* for extra-wide screen animations  */

	int  series_is_font;                /* Flag if series is a font          */

	int  misc[16];                      /* 16 words of miscellaneous data    */

	int  num_sprites;                   /* Number of sprites in series       */

	byte transparent;                   /* Transparent color */
	byte border;                        /* Border color      */

	int  num_translated;                /* Number of colors translated */
	int  color_list_valid;              /* Color list is valid flag    */

	ColorList color_list;               /* Series master color list */

	HagSprite index[1];                 /* Sprites */
} HagSeries;

typedef HagSeries *HagSeriesPtr;


/* LOADABLE FILE STRUCTURES (.SS files) */

/**
 * Loadable sprite record
 */
struct FileSprite {
	uint32 file_offset;           /* Location of sprite data in file   */
	uint32 memory_needed;         /* Memory needed to hold sprite data */
	int16 x, y;
	int16 xs, ys;

	static constexpr int SIZE = 4 + 4 + 2 + 2 + 2 + 2;
	void load(Common::SeekableReadStream *src);
};

typedef FileSprite *FileSpritePtr;


/**
 * Loadable sprite series record
 */
struct FileSeries {
	byte pack_by_sprite;                /* Overall series packing strategy   */
	byte compression;                   /* Type of compression               */
	uint16 delta_series;                   /* Flag if series is a delta series  */
	uint16 base_mode;                      /* Base mode optimization type       */

	uint16 misc[16];                       /* Miscellaneous data for updates    */

	uint16 num_sprites;                    /* Number of sprites in series       */

	WalkerInfo walker;                  /* Walker information                */

	uint16 offset_x_view;                  /* View offsets for sprites intended */
	uint16 offset_y_view;                  /* for extra-wide animations         */

	uint32 total_data_size;               /* Total uncompressed data size      */
	FileSprite index[1];                /* Sprite records begin here         */

	static constexpr int HEADER_SIZE = 1 + 1 + 2 + 2 + 2 * 16 + 2 +
		WalkerInfo::SIZE + 2 + 2 + 4;
	bool loadHeader(Load &load_handle);
};

typedef FileSeries *FileSeriesPtr;


/* RUNTIME STRUCTURES -- MEMORY */

/* Global paging information for a stream-loaded sprite series */

typedef struct {
	byte paging_source;         /* Is source EMS, XMS, or hard disk? */
	byte packing_mode;          /* Type of compression involved      */
	Common::SeekableReadStream *handle; /* File handle for hard disk load    */
	long base_sprite_offset;    /* Base file offset for sprite data  */
	int  ems_handle;            /* EMS handle for preloaded sprites  */
	int  ems_page_marker;       /* EMS page number of current read   */
	int  ems_page_offset;       /* EMS page offset of current read   */
	int  xms_handle;            /* XMS handle for preloaded sprites  */
	long xms_offset;            /* XMS offset for current read       */
} SpritePageInfo;

typedef SpritePageInfo *SpritePageInfoPtr;


/* Sprite-specific paging informatio for stream-loaded series */

typedef struct {
	long file_offset;           /* Offset in file for sprite's data   */
	long memory_needed;         /* Amount of memory needed for sprite */
} SpritePageTable;

typedef SpritePageTable *SpritePageTablePtr;


/* Runtime memory structure for an individual sprite */

typedef struct {
	byte *data;             /* Pointer to sprite's actual data */
	int  x, y;                  /* Home position on screen         */
	int  xs, ys;                /* Sprite box size                 */
} Sprite;

typedef Sprite *SpritePtr;


/* Runtime memory structure for a sprite series */

typedef struct {
	int  delta_series;          /* Flag if series is a delta series         */
	int  base_mode;             /* Series base mode optimization scheme     */

	int  num_sprites;           /* Number of sprites in series              */

	int  color_handle;          /* Palette handle for loaded colors         */
	int  misc[16];              /* Miscellaneous data for future updates    */

	int  offset_x_view;         /* View offsets for sprites intended for    */
	int  offset_y_view;         /* ... extra wide background pictures.      */

	byte pack_by_sprite;        /* Overall sprite packing strategy          */
	SpritePageInfoPtr  page_info;  /* Pointer to global paging data, if any */
	SpritePageTablePtr page_table; /* Pointer to sprite paging table,if any */
	byte *color_table;      /* Pointer to sprite color table    ,if any */
	byte *arena;            /* Pointer to sprite data load arena,if any */

	WalkerInfoPtr walker;       /* Pointer to walker information    ,if any */

	Sprite index[1];            /* Individual sprite records begin here     */
} Series;

typedef Series *SeriesPtr;

#define SS_ERR_OPENFILE         -1
#define SS_ERR_READFILE         -2
#define SS_ERR_WRITEFILE        -3
#define SS_ERR_NOMOREMEMORY     -4
#define SS_ERR_MAKEDIALOG       -5
#define SS_ERR_PALETTEMISMATCH  -6
#define SS_ERR_BOUNDARY         -7
#define SS_ERR_TOOMANYSPRITES   -8
#define SS_ERR_TOOMANYCOLORS    -9
#define SS_ERR_BADDEFINITION    -10
#define SS_ERR_OLDDEFINITION    -11


extern byte *sprite_force_memory;
extern long      sprite_force_size;
extern int sprite_error;


void sprite_draw(SeriesPtr series, int id, Buffer *buf, int target_x, int target_y);
void sprite_draw_scaled(SeriesPtr series, int id, Buffer *buf,
	int target_x, int target_y, int scale_factor);
void sprite_draw_3d_scaled(SeriesPtr series, int id, Buffer *buf, Buffer *attr,
	int target_x, int target_y, int target_depth, int scale_factor);
void sprite_draw_3d_big(SeriesPtr series, int id, Buffer *buf, Buffer *attr,
	int target_x, int target_y, int target_depth, int view_port_x, int view_port_y);
void sprite_draw_3d_scaled_big(SeriesPtr series, int id, Buffer *buf, Buffer *attr,
	int target_x, int target_y, int target_depth, int scale_factor,
	int view_port_x, int view_port_y);
void sprite_draw_3d_x16(SeriesPtr series, int id, Buffer *buf, Buffer *attr,
	int target_x, int target_y, int target_depth);
void sprite_draw_3d_scaled_x16(SeriesPtr series, int id, Buffer *buf, Buffer *attr,
	int target_x, int target_y, int target_depth, int scale_factor);
void sprite_draw_3d_big_x16(SeriesPtr series, int id, Buffer *buf, Buffer *attr,
	int target_x, int target_y, int target_depth, int view_port_x, int view_port_y);
void sprite_draw_3d_scaled_big_x16(SeriesPtr series, int id, Buffer *buf, Buffer *attr,
	int target_x, int target_y, int target_depth, int scale_factor,
	int view_port_x, int view_port_y);
void sprite_draw_x16(SeriesPtr series, int id, Buffer *buf, int target_x, int target_y);
void sprite_draw_interface(SeriesPtr series, int id, Buffer *buf, int target_x, int target_y);
void sprite_draw_3d_scaled_to_attr(SeriesPtr series, int id, Buffer *buf, Buffer *attr,
	int target_x, int target_y, int target_depth, int scale_factor,
	int view_port_x, int view_port_y);
void sprite_draw_3d_scaled_mono(SeriesPtr series, int id, Buffer *buf, Buffer *attr,
	int target_x, int target_y, int target_depth, int scale_factor, byte color);
SeriesPtr sprite_series_load(const char *filename, int load_flags);
void sprite_get_scaled_matte(SeriesPtr series, int id, int target_x, int target_y,
	int scale_factor, SpritePtr matte);

/**
 * Packs line (read from source) to buffer pointer to by BUF,
 * and returns size.  X1,X2 and Y are line on screen to read.
 * "Transparent" defines the background "skip" color.  "Palette_map"
 * defines the palette-to-color-list mappings.
 */
word sprite_pack_line_rle(byte *target, Buffer *source, byte *palette_map, byte transparent,
	int x1, int x2, int y);

/**
 * Packs line (read from source) to buffer pointer to by BUF,
 * and returns size.  X1,X2 and Y are line of buffer to read.
 * "Transparent" defines transparent color (instances of which
 * are converted to "SS_SKIP").  "Palette_map" defines the
 * palette-to-color-list mappings for the series.
 */
word sprite_pack_line_irle(byte *target, Buffer *source,
	byte *palette_map, byte transparent, int x1, int x2, int y);

void sprite_set_bounds(HagSpritePtr sprite, int x, int y, int xs, int ys);

/**
 * Given a foreground buffer and a background buffer, merges
 * background image into transparent areas of foreground image.
 */
void sprite_merge_background(Buffer *source, Buffer *background, byte transparent);

/**
 * Finds differences between "source" and "delta" buffers.  When pixels
 * are different, "source" pixel is copied down to "delta" buffer (to
 * prepare buffer for next round), and "source" buffer is whited out
 * to show changes.  "sprite"'s boundaries are set to conform to minimum
 * rectangle needed to contain all changes.
 */
void sprite_delta_compute(HagSpritePtr sprite, Buffer *source, Buffer *delta);

/**
 * Compares "source" and "delta" buffers.  If a pixel is the same,
 * "source" buffer is changed to "transparent"; if pixel is different,
 * "source" pixel is copied to "delta" pixel, which prepares the delta
 * buffer for use on next round.
 */
void sprite_remove_non_delta(Buffer *source, Buffer *delta, byte transparent);

/**
 * Given sprite, packs it and writes it to buffer target.
 * Returns # of bytes output.
 */
long sprite_pack_image(byte *target, FileSpritePtr sprite, Buffer *source,
	byte *palette_map, byte transparent);

void sprite_color_translate(SeriesPtr series, ColorListPtr list);
void sprite_single_color_translate(SeriesPtr series, int id);
int sprite_data_load(SeriesPtr series, int id, byte *target);

void dont_frag_the_palette();
void go_ahead_and_frag_the_palette();
void sprite_free(SeriesPtr *series, int free_memory);

} // namespace MADSV2
} // namespace MADS

#endif
