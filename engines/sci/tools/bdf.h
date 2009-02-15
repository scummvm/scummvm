/*
 * Copyright 2001 Computing Research Labs, New Mexico State University
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COMPUTING RESEARCH LAB OR NEW MEXICO STATE UNIVERSITY BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
 * OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef _h_bdf
#define _h_bdf

/*
 * $Id: bdf.h 1284 2004-04-02 07:42:44Z jameson $
 */

#include <stdio.h>
#include <stdlib.h>
#ifndef __digital__
#include <unistd.h>
#endif
#include <string.h>

#ifdef HAVE_XLIB
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#endif /* HAVE_XLIB */

#ifdef HAVE_FREETYPE
#include <freetype.h>
#include <ftxsbit.h>
#endif /* HAVE_FREETYPE */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A macro for prototypes.
 */
#undef __
#ifdef __STDC__
#define __(x) x
#else
#define __(x) ()
#endif

/**************************************************************************
 *
 * BDF font options macros and types.
 *
 **************************************************************************/

#define BDF_UNIX_EOL 1           /* Save fonts with Unix LF.              */
#define BDF_DOS_EOL  2           /* Save fonts with DOS CRLF.             */
#define BDF_MAC_EOL  3           /* Save fonts with Mac CR.               */

#define BDF_CORRECT_METRICS 0x01 /* Correct invalid metrics when loading. */
#define BDF_KEEP_COMMENTS   0x02 /* Preserve the font comments.           */
#define BDF_KEEP_UNENCODED  0x04 /* Keep the unencoded glyphs.            */
#define BDF_PROPORTIONAL    0x08 /* Font has proportional spacing.        */
#define BDF_MONOWIDTH       0x10 /* Font has mono width.                  */
#define BDF_CHARCELL        0x20 /* Font has charcell spacing.            */

#define BDF_ALL_SPACING (BDF_PROPORTIONAL|BDF_MONOWIDTH|BDF_CHARCELL)

#define BDF_DEFAULT_LOAD_OPTIONS \
    (BDF_CORRECT_METRICS|BDF_KEEP_COMMENTS|BDF_KEEP_UNENCODED|BDF_PROPORTIONAL)

typedef struct {
    int ttf_hint;
    int correct_metrics;
    int keep_unencoded;
    int keep_comments;
    int pad_cells;
    int font_spacing;
    long point_size;
    unsigned long resolution_x;
    unsigned long resolution_y;
    int bits_per_pixel;
    int eol;
} bdf_options_t;

/*
 * Callback function type for unknown configuration options.
 */
typedef int (*bdf_options_callback_t) __((bdf_options_t *opts,
                                          char **params,
                                          unsigned long nparams,
                                          void *client_data));

/**************************************************************************
 *
 * BDF font property macros and types.
 *
 **************************************************************************/

#define BDF_ATOM     1
#define BDF_INTEGER  2
#define BDF_CARDINAL 3

/*
 * This structure represents a particular property of a font.
 * There are a set of defaults and each font has their own.
 */
typedef struct {
    char *name;         /* Name of the property.                        */
    int format;         /* Format of the property.                      */
    int builtin;        /* A builtin property.                          */
    union {
        char *atom;
        long int32;
        unsigned long card32;
    } value;            /* Value of the property.                       */
} bdf_property_t;

/**************************************************************************
 *
 * SBIT metrics specific structures.
 *
 **************************************************************************/

/*
 * Boolean flags for SBIT metrics files.
 */
#define BDF_SBIT_MONO_ADVANCE 0x0001
#define BDF_SBIT_ADD_EBLC     0x0002
#define BDF_SBIT_APPLE_COMPAT 0x0004

/*
 * Direction macros (inclusive, can be combined).
 */
#define BDF_SBIT_HORIZONTAL   0x0008
#define BDF_SBIT_VERTICAL     0x0010

/*
 * Bitmap storage options (exclusive, cannot be combined).
 */
#define BDF_SBIT_STORE_SMALL  0x0020
#define BDF_SBIT_STORE_FAST   0x0040

typedef struct {
    short cnum;         /* Caret slope numerator.                       */
    short cdenom;       /* Caret slope denominator.                     */
    short coff;         /* Caret offset.                                */
    short sx;           /* Scaled version horizontal PPEM size.         */
    short sy;           /* Scaled version vertical PPEM size (optional).*/
    short flags;        /* Booleans and other non-numeric values.       */
} bdf_sbit_t;

/**************************************************************************
 *
 * BDF opaque undo information types.
 *
 **************************************************************************/

typedef struct _bdf_undo_struct *bdf_undo_t;

/**************************************************************************
 *
 * BDF font metric and glyph types.
 *
 **************************************************************************/

/*
 * A general bitmap type, mostly used when the glyph bitmap is being edited.
 */
typedef struct {
    short x;
    short y;
    unsigned short width;
    unsigned short height;
    unsigned short bpp;
    unsigned short pad;
    unsigned char *bitmap;
    unsigned long bytes;
} bdf_bitmap_t;

typedef struct {
    int font_spacing;
    unsigned short swidth;
    unsigned short dwidth;
    unsigned short width;
    unsigned short height;
    short x_offset;
    short y_offset;
    short ascent;
    short descent;
} bdf_metrics_t;

typedef struct {
    unsigned short width;
    unsigned short height;
    short x_offset;
    short y_offset;
    short ascent;
    short descent;
} bdf_bbx_t;

typedef struct {
    char *name;                 /* Glyph name.                          */
    long encoding;              /* Glyph encoding.                      */
    unsigned short swidth;      /* Scalable width.                      */
    unsigned short dwidth;      /* Device width.                        */
    bdf_bbx_t bbx;              /* Glyph bounding box.                  */
    unsigned char *bitmap;      /* Glyph bitmap.                        */
    unsigned short bytes;       /* Number of bytes used for the bitmap. */
} bdf_glyph_t;

typedef struct {
    unsigned short pad;         /* Pad to 4-byte boundary.              */
    unsigned short bpp;         /* Bits per pixel.                      */
    long start;                 /* Beginning encoding value of glyphs.  */
    long end;                   /* Ending encoding value of glyphs.     */
    bdf_glyph_t *glyphs;        /* Glyphs themselves.                   */
    unsigned long glyphs_size;  /* Glyph structures allocated.          */
    unsigned long glyphs_used;  /* Glyph structures used.               */
    bdf_bbx_t bbx;              /* Overall bounding box of glyphs.      */
} bdf_glyphlist_t;

typedef struct {
    char *name;                 /* Name of the font.                     */
    bdf_bbx_t bbx;              /* Font bounding box.                    */

    long point_size;            /* Point size of the font.               */
    unsigned long resolution_x; /* Font horizontal resolution.           */
    unsigned long resolution_y; /* Font vertical resolution.             */

    int hbf;                    /* Font came from an HBF font.           */

    int spacing;                /* Font spacing value.                   */

    unsigned short monowidth;   /* Logical width for monowidth font.     */

    long default_glyph;         /* Encoding of the default glyph.        */

    long font_ascent;           /* Font ascent.                          */
    long font_descent;          /* Font descent.                         */

    long glyphs_size;           /* Glyph structures allocated.           */
    long glyphs_used;           /* Glyph structures used.                */
    bdf_glyph_t *glyphs;        /* Glyphs themselves.                    */

    long unencoded_size;        /* Unencoded glyph structures allocated. */
    long unencoded_used;        /* Unencoded glyph structures used.      */
    bdf_glyph_t *unencoded;     /* Unencoded glyphs themselves.          */

    unsigned long props_size;   /* Font properties allocated.            */
    unsigned long props_used;   /* Font properties used.                 */
    bdf_property_t *props;      /* Font properties themselves.           */

    char *comments;             /* Font comments.                        */
    unsigned long comments_len; /* Length of comment string.             */

    char *acmsgs;               /* Auto-correction messages.             */
    unsigned long acmsgs_len;   /* Length of auto-correction messages.   */

    bdf_glyphlist_t overflow;   /* Storage used for glyph insertion.     */

    void *internal;             /* Internal data for the font.           */

    unsigned long nmod[2048];   /* Bitmap indicating modified glyphs.    */
    unsigned long umod[2048];   /* Bitmap indicating modified unencoded. */

    unsigned short modified;    /* Boolean indicating font modified.     */
    unsigned short bpp;         /* Bits per pixel.                       */

    bdf_sbit_t *sbits;          /* Associcated SBIT metrics.             */
    unsigned long sbits_used;   /* Number of SBIT metrics entries.       */
    unsigned long sbits_size;   /* Amount of entries allocated.          */

    bdf_undo_t *undo_stack;     /* Record of undoable operations.        */
    unsigned long undo_used;    /* Amount of undo stack used.            */
    unsigned long undo_size;    /* Amount of undo stack allocated.       */
} bdf_font_t;

/**************************************************************************
 *
 * BDF glyph grid structures for editing glyph bitmaps.
 *
 **************************************************************************/

typedef struct {
    char *name;
    long encoding;              /* The glyph encoding.                  */
    unsigned short unencoded;   /* Whether the glyph was unencoded.     */
    unsigned short bpp;         /* Bits per pixel.                      */
    int spacing;                /* Font spacing.                        */
    long resolution_x;          /* Horizontal resolution.               */
    long resolution_y;          /* Vertical resolution.                 */
    unsigned long point_size;   /* Font point size.                     */
    unsigned short swidth;      /* Scalable width.                      */
    unsigned short dwidth;      /* Device width.                        */
    bdf_bbx_t font_bbx;         /* Font bounding box.                   */
    bdf_bbx_t glyph_bbx;        /* Glyph bounding box.                  */
    unsigned char *bitmap;      /* The grid bitmap.                     */
    unsigned short bytes;       /* Number of bytes in the grid bitmap.  */
    short grid_width;           /* Width of the grid.                   */
    short grid_height;          /* Height of the grid.                  */
    short base_x;               /* Baseline X coordinate.               */
    short base_y;               /* Baseline Y coordinate.               */
    short glyph_x;              /* Top-left X position of glyph.        */
    short glyph_y;              /* Top-left Y position of glyph.        */
    unsigned short modified;    /* Flag indicating if bitmap modified.  */
    short cap_height;           /* Font CAP_HEIGHT if it exists.        */
    short x_height;             /* Font X_HEIGHT if it exists.          */
    bdf_bitmap_t sel;           /* Selected portion of the glyph bitmap.*/
} bdf_glyph_grid_t;

/**************************************************************************
 *
 * Types for load/save callbacks.
 *
 **************************************************************************/

/*
 * Callback reasons.
 */
#define BDF_LOAD_START       1
#define BDF_LOADING          2
#define BDF_SAVE_START       3
#define BDF_SAVING           4
#define BDF_TRANSLATE_START  5
#define BDF_TRANSLATING      6
#define BDF_ROTATE_START     7
#define BDF_ROTATING         8
#define BDF_SHEAR_START      9
#define BDF_SHEARING         10
#define BDF_GLYPH_NAME_START 11
#define BDF_GLYPH_NAME       12
#define BDF_EXPORT_START     13
#define BDF_EXPORTING        14
#define BDF_EMBOLDEN_START   15
#define BDF_EMBOLDENING      16
#define BDF_WARNING          20
#define BDF_ERROR            21

/*
 * Error codes.
 */
#define BDF_OK                 0
#define BDF_MISSING_START     -1
#define BDF_MISSING_FONTNAME  -2
#define BDF_MISSING_SIZE      -3
#define BDF_MISSING_FONTBBX   -4
#define BDF_MISSING_CHARS     -5
#define BDF_MISSING_STARTCHAR -6
#define BDF_MISSING_ENCODING  -7
#define BDF_MISSING_BBX       -8

#define BDF_NOT_CONSOLE_FONT  -10
#define BDF_NOT_MF_FONT       -11
#define BDF_NOT_PSF_FONT      -12
#define BDF_EMPTY_FONT        -99
#define BDF_INVALID_LINE      -100

typedef struct {
    unsigned long reason;
    unsigned long current;
    unsigned long total;
    unsigned long errlineno;
} bdf_callback_struct_t;

typedef void (*bdf_callback_t) __((bdf_callback_struct_t *call_data,
                                   void *client_data));

/**************************************************************************
 *
 * BDF font API.
 *
 **************************************************************************/

/*
 * Startup and shutdown functions.
 */
extern void bdf_setup __((void));
extern void bdf_cleanup __((void));

/*
 * Configuration file loading and saving.
 */
extern void bdf_load_options __((FILE *in, bdf_options_t *opts,
                                 bdf_options_callback_t callback,
                                 void *client_data));
extern void bdf_save_options __((FILE *out, bdf_options_t *opts));

/*
 * Font options functions.
 */
extern void bdf_default_options __((bdf_options_t *opts));

/*
 * Font load, create, save and free functions.
 */
extern bdf_font_t *bdf_new_font __((char *name, long point_size,
                                    long resolution_x, long resolution_y,
                                    long spacing, int bpp));
extern bdf_font_t *bdf_load_font __((FILE *in, bdf_options_t *opts,
                                     bdf_callback_t callback, void *data));
extern bdf_font_t *bdf_load_hbf_font __((char *filename, bdf_options_t *opts,
                                         bdf_callback_t callback, void *data));

#ifdef HAVE_XLIB
extern bdf_font_t *bdf_load_server_font __((Display *d, XFontStruct *f,
                                            char *name, bdf_options_t *opts,
                                            bdf_callback_t callback,
                                            void *data));
#endif /* HAVE_XLIB */

extern int bdf_load_console_font __((FILE *in, bdf_options_t *opts,
                                     bdf_callback_t callback, void *data,
                                     bdf_font_t *fonts[3], int *nfonts));

extern int bdf_load_mf_font __((FILE *in, bdf_options_t *opts,
                                bdf_callback_t callback, void *data,
                                bdf_font_t **font));

extern void bdf_save_font __((FILE *out, bdf_font_t *font,
                              bdf_options_t *opts, bdf_callback_t callback,
                              void *data));

extern void bdf_save_sbit_metrics __((FILE *out, bdf_font_t *font,
                                      bdf_options_t *opts, char *appname));

extern void bdf_export_hex __((FILE *out, bdf_font_t *font,
                               bdf_callback_t callback, void *data));

extern int bdf_export_psf __((FILE *out, bdf_font_t *font));

extern void bdf_free_font __((bdf_font_t *font));

#ifdef HAVE_FREETYPE

/*
 * TrueType related macros and functions.
 */

/*
 * ID numbers of the strings that can appear in a TrueType font.
 */
#define BDFTTF_COPYRIGHT_STRING  0
#define BDFTTF_FAMILY_STRING     1
#define BDFTTF_SUBFAMILY_STRING  2
#define BDFTTF_UNIQUEID_STRING   3
#define BDFTTF_FULLNAME_STRING   4
#define BDFTTF_VENDOR_STRING     5
#define BDFTTF_POSTSCRIPT_STRING 6
#define BDFTTF_TRADEMARK_STRING  7

extern char *bdfttf_platform_name __((short pid));
extern char *bdfttf_encoding_name __((short pid, short eid));
extern int bdfttf_get_english_string __((TT_Face face, int nameID,
                                         int dash_to_space, char *name));

extern int bdfttf_load_font __((TT_Face face, TT_Face_Properties *properties,
                                short pid, short eid, bdf_options_t *opts,
                                bdf_callback_t callback, void *data,
                                bdf_font_t **font));

#endif /* HAVE_FREETYPE */

/*
 * FON/FNT related functions.
 */

/*
 * String ID numbers for FON/FNT fonts.
 */
#define BDFFNT_COPYRIGHT 1
#define BDFFNT_TYPEFACE  2

/*
 * Opaque font type.
 */
typedef struct _bdffnt_font_t *bdffnt_font_t;

extern int bdffnt_open_font __((char *path, bdffnt_font_t *font));
extern void bdffnt_close_font __((bdffnt_font_t font));
extern int bdffnt_font_count __((bdffnt_font_t font));
extern int bdffnt_get_copyright __((bdffnt_font_t font, unsigned long fontID,
                                    unsigned char *string));
extern int bdffnt_get_facename __((bdffnt_font_t font, unsigned long fontID,
                                   int for_xlfd, unsigned char *string));
extern int bdffnt_char_count __((bdffnt_font_t font, unsigned long fontID));
extern int bdffnt_font_pointsize __((bdffnt_font_t font,
                                     unsigned long fontID));
extern int bdffnt_load_font __((bdffnt_font_t font, unsigned long fontID,
                                bdf_callback_t callback, void *data,
                                bdf_font_t **out));

/*
 * Font property functions.
 */
extern void bdf_create_property __((char *name, int type));
extern bdf_property_t *bdf_get_property __((char *name));
extern unsigned long bdf_property_list __((bdf_property_t **props));

extern void bdf_add_font_property __((bdf_font_t *font,
                                      bdf_property_t *property));
extern void bdf_delete_font_property __((bdf_font_t *font, char *name));
extern bdf_property_t *bdf_get_font_property __((bdf_font_t *font,
                                                 const char *name));
extern unsigned long bdf_font_property_list __((bdf_font_t *font,
                                                bdf_property_t **props));

/*
 * Font comment functions.
 */
extern int bdf_replace_comments __((bdf_font_t *font, char *comments,
                                    unsigned long comments_len));

/*
 * Other miscellaneous functions.
 */
extern void bdf_set_default_metrics __((bdf_font_t *font));

/*
 * Font glyph editing functions.
 */
extern int bdf_glyph_modified __((bdf_font_t *font, long which,
                                  int unencoded));

extern void bdf_copy_glyphs __((bdf_font_t *font, long start, long end,
                                bdf_glyphlist_t *glyphs, int unencoded));

extern void bdf_delete_glyphs __((bdf_font_t *font, long start, long end,
                                  int unencoded));

extern int bdf_insert_glyphs __((bdf_font_t *font, long start,
                                 bdf_glyphlist_t *glyphs));

extern int bdf_replace_glyphs __((bdf_font_t *font, long start,
                                  bdf_glyphlist_t *glyphs, int unencoded));

extern int bdf_merge_glyphs __((bdf_font_t *font, long start,
                                bdf_glyphlist_t *glyphs, int unencoded));

/**************************************************************************
 *
 * Other API functions.
 *
 **************************************************************************/

extern int bdf_set_font_bbx __((bdf_font_t *font, bdf_metrics_t *metrics));

extern void bdf_set_modified __((bdf_font_t *font, int modified));

extern int bdf_has_xlfd_name __((bdf_font_t *font));

extern char *bdf_make_xlfd_name __((bdf_font_t *font, char *foundry,
                                    char *family));

extern void bdf_update_name_from_properties __((bdf_font_t *font));

extern void bdf_update_properties_from_name __((bdf_font_t *font));

extern int bdf_update_average_width __((bdf_font_t *font));

extern int bdf_set_unicode_glyph_names __((FILE *in, bdf_font_t *font,
                                           bdf_callback_t callback));

extern int bdf_set_adobe_glyph_names __((FILE *in, bdf_font_t *font,
                                         bdf_callback_t callback));

extern int bdf_set_glyph_code_names __((int prefix, bdf_font_t *font,
                                        bdf_callback_t callback));

/**************************************************************************
 *
 * Glyph grid API.
 *
 **************************************************************************/

/*
 * Glyph grid allocation and deallocation functions.
 */
extern bdf_glyph_grid_t *bdf_make_glyph_grid __((bdf_font_t *font,
                                                 long code,
                                                 int unencoded));
extern void bdf_free_glyph_grid __((bdf_glyph_grid_t *grid));

/*
 * Glyph grid information functions.
 */
extern void bdf_grid_image __((bdf_glyph_grid_t *grid, bdf_bitmap_t *image));
extern void bdf_grid_origin __((bdf_glyph_grid_t *grid, short *x, short *y));
extern bdf_glyph_t *bdf_grid_glyph __((bdf_glyph_grid_t *grid));

/*
 * Glyph grid editing functions.
 */
extern int bdf_grid_enlarge __((bdf_glyph_grid_t *grid, unsigned short width,
                                unsigned short height));
extern int bdf_grid_resize __((bdf_glyph_grid_t *grid,
                               bdf_metrics_t *metrics));
extern int bdf_grid_crop __((bdf_glyph_grid_t *grid, int grid_modified));

extern int bdf_grid_set_pixel __((bdf_glyph_grid_t *grid, short x, short y,
                                  int val));
extern int bdf_grid_clear_pixel __((bdf_glyph_grid_t *grid, short x, short y));
extern int bdf_grid_invert_pixel __((bdf_glyph_grid_t *grid,
                                     short x, short y, int val));
extern int bdf_grid_shift __((bdf_glyph_grid_t *grid, short xcount,
                              short ycount));
extern int bdf_grid_flip __((bdf_glyph_grid_t *grid, short dir));
extern int bdf_grid_rotate __((bdf_glyph_grid_t *grid, short degrees,
                               int *resize));
extern int bdf_grid_shear __((bdf_glyph_grid_t *grid, short degrees,
                              int *resize));
extern int bdf_grid_embolden __((bdf_glyph_grid_t *grid));

/*
 * Glyph grid selection functions.
 */
extern int bdf_has_selection __((bdf_glyph_grid_t *grid, short *x, short *y,
                                 short *width, short *height));
extern void bdf_set_selection __((bdf_glyph_grid_t *grid, short x, short y,
                                  short width, short height));
extern void bdf_lose_selection __((bdf_glyph_grid_t *grid));
extern void bdf_detach_selection __((bdf_glyph_grid_t *grid));
extern void bdf_attach_selection __((bdf_glyph_grid_t *grid));
extern void bdf_delete_selection __((bdf_glyph_grid_t *grid));
extern int bdf_in_selection __((bdf_glyph_grid_t *grid, short x, short y,
                                short *set));
extern void bdf_add_selection __((bdf_glyph_grid_t *grid, bdf_bitmap_t *sel));

/*
 * Glyph grid misc functions.
 */
extern int bdf_grid_color_at __((bdf_glyph_grid_t *grid, short x, short y));

/*
 * Graphical transformation functions.
 */
extern int bdf_translate_glyphs __((bdf_font_t *font, short dx, short dy,
                                    long start, long end,
                                    bdf_callback_t callback, void *data,
                                    int unencoded));

extern int bdf_rotate_glyphs __((bdf_font_t *font, short degrees,
                                 long start, long end,
                                 bdf_callback_t callback, void *data,
                                 int unencoded));

extern int bdf_shear_glyphs __((bdf_font_t *font, short degrees,
                                long start, long end,
                                bdf_callback_t callback, void *data,
                                int unencoded));

extern int bdf_embolden_glyphs __((bdf_font_t *font, long start, long end,
                                   bdf_callback_t callback, void *data,
                                   int unencoded, int *resize));

extern int bdf_little_endian __((void));

#undef __

#ifdef __cplusplus
}
#endif

#endif /* _h_bdf */
