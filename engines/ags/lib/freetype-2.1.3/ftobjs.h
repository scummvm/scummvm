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

/***************************************************************************/
/*                                                                         */
/*  ftobjs.h                                                               */
/*    The FreeType private base classes (specification).                   */
/*                                                                         */
/***************************************************************************/


/*************************************************************************/
/*                                                                       */
/*  This file contains the definition of all internal FreeType classes.  */
/*                                                                       */
/*************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTOBJS_H
#define AGS_LIB_FREETYPE_FTOBJS_H

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/config/ftstdlib.h"   /* for ft_setjmp and ft_longjmp */
#include "engines/ags/lib/freetype-2.1.3/ftrender.h"
#include "engines/ags/lib/freetype-2.1.3/ftsizes.h"
#include "engines/ags/lib/freetype-2.1.3/ftmemory.h"
#include "engines/ags/lib/freetype-2.1.3/ftgloadr.h"
#include "engines/ags/lib/freetype-2.1.3/ftdriver.h"
#include "engines/ags/lib/freetype-2.1.3/modules/autohint/autohint.h"
#include "engines/ags/lib/freetype-2.1.3/ftobject.h"

#ifdef FT2_1_3_CONFIG_OPTION_INCREMENTAL
#include "engines/ags/lib/freetype-2.1.3/ftincrem.h"
#endif

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE  0
#endif

#ifndef NULL
#define NULL  (void*)0
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a) ((a) < 0 ? -(a) : (a))
#endif


/****  VALIDATION ****/

/* handle to a validation object */
typedef struct FT_ValidatorRec_ *FT_Validator;

typedef enum  FT_ValidationLevel_ {
	FT2_1_3_VALIDATE_DEFAULT = 0,
	FT2_1_3_VALIDATE_TIGHT,
	FT2_1_3_VALIDATE_PARANOID
} FT_ValidationLevel;

/* validator structure */
typedef struct  FT_ValidatorRec_ {
	const FT_Byte		*base;       /* address of table in memory       */
	const FT_Byte		*limit;      /* `base' + sizeof(table) in memory */
	FT_ValidationLevel  level;       /* validation level                 */
	FT_Error            error;       /* error returned. 0 means success  */

	ft_jmp_buf          jump_buffer; /* used for exception handling      */
} FT_ValidatorRec;

#define FT2_1_3_VALIDATOR(x) ((FT_Validator)(x))

FT2_1_3_BASE(void)
ft_validator_init(FT_Validator valid, const FT_Byte *base, const FT_Byte *limit, FT_ValidationLevel level);

FT2_1_3_BASE(FT_Int)
ft_validator_run(FT_Validator valid);

/* Sets the error field in a validator, then calls `longjmp' to return */
/* to high-level caller.  Using `setjmp/longjmp' avoids many stupid    */
/* error checks within the validation routines.                        */
/*                                                                     */
FT2_1_3_BASE(void)
ft_validator_error(FT_Validator valid, FT_Error error);

/* Calls ft_validate_error.  Assumes that the `valid' local variable */
/* holds a pointer to the current validator object.                  */
/*                                                                   */
#define FT2_1_3_INVALID(_error) ft_validator_error(valid, _error)

/* called when a broken table is detected */
#define FT2_1_3_INVALID_TOO_SHORT FT2_1_3_INVALID(FT2_1_3_Err_Invalid_Table)

/* called when an invalid offset is detected */
#define FT2_1_3_INVALID_OFFSET FT2_1_3_INVALID(FT2_1_3_Err_Invalid_Offset)

/* called when an invalid format/value is detected */
#define FT2_1_3_INVALID_FORMAT FT2_1_3_INVALID(FT2_1_3_Err_Invalid_Table)

/* called when an invalid glyph index is detected */
#define FT2_1_3_INVALID_GLYPH_ID FT2_1_3_INVALID(FT2_1_3_Err_Invalid_Glyph_Index)

/* called when an invalid field value is detected */
#define FT2_1_3_INVALID_DATA FT2_1_3_INVALID(FT2_1_3_Err_Invalid_Table)


/**** CHARMAPS ****/

/* handle to internal charmap object */
typedef struct FT_CMapRec_ *FT_CMap;

/* handle to charmap class structure */
typedef const struct FT_CMap_ClassRec_ *FT_CMap_Class;

/* internal charmap object structure */
typedef struct  FT_CMapRec_ {
	FT_CharMapRec  charmap;
	FT_CMap_Class  clazz;
} FT_CMapRec;

/* typecase any pointer to a charmap handle */
#define FT2_1_3_CMAP(x) ((FT_CMap)(x))

/* obvious macros */
#define FT2_1_3_CMAP_PLATFORM_ID(x)	 FT2_1_3_CMAP(x)->charmap.platform_id
#define FT2_1_3_CMAP_ENCODING_ID(x)  FT2_1_3_CMAP(x)->charmap.encoding_id
#define FT2_1_3_CMAP_ENCODING(x)	 FT2_1_3_CMAP(x)->charmap.encoding
#define FT2_1_3_CMAP_FACE(x)		 FT2_1_3_CMAP(x)->charmap.face

/* class method definitions */
typedef FT_Error (*FT_CMap_InitFunc)(FT_CMap cmap, FT_Pointer init_data);
typedef void (*FT_CMap_DoneFunc)(FT_CMap cmap);
typedef FT_UInt (*FT_CMap_CharIndexFunc)(FT_CMap cmap, FT_UInt32 char_code);
typedef FT_UInt (*FT_CMap_CharNextFunc)(FT_CMap cmap, FT_UInt32 *achar_code);

typedef struct  FT_CMap_ClassRec_ {
	FT_UInt                size;
	FT_CMap_InitFunc       init;
	FT_CMap_DoneFunc       done;
	FT_CMap_CharIndexFunc  char_index;
	FT_CMap_CharNextFunc   char_next;
} FT_CMap_ClassRec;

/* create a new charmap and add it to charmap->face */
FT2_1_3_BASE(FT_Error)
FT_CMap_New(FT_CMap_Class clazz, FT_Pointer init_data, FT_CharMap charmap, FT_CMap *acmap);

/* destroy a charmap (don't remove it from face's list though) */
FT2_1_3_BASE(void)
FT_CMap_Done(FT_CMap cmap);

typedef struct  FT_Face_InternalRec_ {
	FT_UShort    max_points;
	FT_Short     max_contours;

	FT_Matrix    transform_matrix;
	FT_Vector    transform_delta;
	FT_Int       transform_flags;

	FT_UInt32    hint_flags;

	const char	 *postscript_name;

#ifdef FT2_1_3_CONFIG_OPTION_INCREMENTAL
	FT_Incremental_InterfaceRec*  incremental_interface;
#endif
} FT_Face_InternalRec;

typedef struct  FT_Slot_InternalRec_ {
	FT_GlyphLoader  loader;
	FT_Bool         glyph_transformed;
	FT_Matrix       glyph_matrix;
	FT_Vector       glyph_delta;
	void			*glyph_hints;
} FT_GlyphSlot_InternalRec;


/**** MODULES ****/

typedef struct  FT_ModuleRec_ {
	FT_Module_Class   *clazz;
	FT_Library        library;
	FT_Memory         memory;
	FT_Generic        generic;
} FT_ModuleRec;

/* typecast an object to a FT_Module */
#define FT2_1_3_MODULE(x)			((FT_Module)(x))
#define FT2_1_3_MODULE_CLASS(x)		FT2_1_3_MODULE(x)->clazz
#define FT2_1_3_MODULE_LIBRARY(x)	FT2_1_3_MODULE(x)->library
#define FT2_1_3_MODULE_MEMORY(x)	FT2_1_3_MODULE(x)->memory

#define FT2_1_3_MODULE_IS_DRIVER(x) 	(FT2_1_3_MODULE_CLASS(x)->module_flags & ft_module_font_driver)
#define FT2_1_3_MODULE_IS_RENDERER(x) 	(FT2_1_3_MODULE_CLASS(x)->module_flags & ft_module_renderer)
#define FT2_1_3_MODULE_IS_HINTER(x) 	(FT2_1_3_MODULE_CLASS(x)->module_flags & ft_module_hinter)
#define FT2_1_3_MODULE_IS_STYLER(x) 	(FT2_1_3_MODULE_CLASS(x)->module_flags & ft_module_styler)
#define FT2_1_3_DRIVER_IS_SCALABLE(x) 	(FT2_1_3_MODULE_CLASS(x)->module_flags & ft_module_driver_scalable)
#define FT2_1_3_DRIVER_USES_OUTLINES(x)	!(FT2_1_3_MODULE_CLASS(x)->module_flags & ft_module_driver_no_outlines)
#define FT2_1_3_DRIVER_HAS_HINTER(x) 	(FT2_1_3_MODULE_CLASS(x)->module_flags & ft_module_driver_has_hinter)

FT2_1_3_BASE(const void *)
FT_Get_Module_Interface(FT_Library library, const char *mod_name);


/**** FACE, SIZE & GLYPH SLOT OBJECTS ****/

/* a few macros used to perform easy typecasts with minimal brain damage */

#define FT2_1_3_FACE(x)	((FT_Face)(x))
#define FT2_1_3_SIZE(x) ((FT_Size)(x))
#define FT2_1_3_SLOT(x) ((FT_GlyphSlot)(x))

#define FT2_1_3_FACE_DRIVER(x) 	FT2_1_3_FACE(x)->driver
#define FT2_1_3_FACE_LIBRARY(x) FT2_1_3_FACE_DRIVER(x)->root.library
#define FT2_1_3_FACE_MEMORY(x) 	FT2_1_3_FACE(x)->memory
#define FT2_1_3_FACE_STREAM(x) 	FT2_1_3_FACE(x)->stream

#define FT2_1_3_SIZE_FACE(x) FT2_1_3_SIZE(x)->face
#define FT2_1_3_SLOT_FACE(x) FT2_1_3_SLOT(x)->face

#define FT2_1_3_FACE_SLOT(x) FT2_1_3_FACE(x)->glyph
#define FT2_1_3_FACE_SIZE(x) FT2_1_3_FACE(x)->size

FT2_1_3_BASE(FT_Error)
FT_New_GlyphSlot(FT_Face face, FT_GlyphSlot *aslot);

FT2_1_3_BASE(void)
FT2_1_3_Done_GlyphSlot(FT_GlyphSlot slot);


/**** RENDERERS ****/

#define FT2_1_3_RENDERER(x)		 ((FT_Renderer)(x))
#define FT2_1_3_GLYPH(x)		 ((FT_Glyph)(x))
#define FT2_1_3_BITMAP_GLYPH(x)	 ((FT_BitmapGlyph)(x))
#define FT2_1_3_OUTLINE_GLYPH(x) ((FT_OutlineGlyph)(x))

typedef struct  FT_RendererRec_ {
	FT_ModuleRec            root;
	FT_Renderer_Class		*clazz;
	FT_Glyph_Format         glyph_format;
	FT_Glyph_Class          glyph_class;

	FT_Raster               raster;
	FT_Raster_Render_Func   raster_render;
	FT_Renderer_RenderFunc  render;
} FT_RendererRec;


/**** FONT DRIVERS ****/

/* typecast a module into a driver easily */
#define FT2_1_3_DRIVER(x) ((FT_Driver)(x))

/* typecast a module as a driver, and get its driver class */
#define FT2_1_3_DRIVER_CLASS(x) FT2_1_3_DRIVER(x)->clazz

typedef struct  FT_DriverRec_ {
	FT_ModuleRec     root;
	FT_Driver_Class  clazz;

	FT_ListRec       faces_list;
	void			 *extensions;

	FT_GlyphLoader   glyph_loader;
} FT_DriverRec;


/**** LIBRARIES ****/

#define FT2_1_3_DEBUG_HOOK_TRUETYPE  0
#define FT2_1_3_DEBUG_HOOK_TYPE1     1

typedef struct  FT_LibraryRec_ {
	FT_Memory          memory;           /* library's memory manager */

	FT_Generic         generic;

	FT_Int             version_major;
	FT_Int             version_minor;
	FT_Int             version_patch;

	FT_UInt            num_modules;
	FT_Module          modules[FT2_1_3_MAX_MODULES];  /* module objects  */

	FT_ListRec         renderers;        /* list of renderers        */
	FT_Renderer        cur_renderer;     /* current outline renderer */
	FT_Module          auto_hinter;

	FT_Byte			   *raster_pool;      /* scan-line conversion */
	/* render pool          */
	FT_ULong           raster_pool_size; /* size of render pool in bytes */

	FT_DebugHook_Func  debug_hooks[4];

	FT_MetaClassRec    meta_class;
} FT_LibraryRec;

FT2_1_3_BASE(FT_Renderer)
FT_Lookup_Renderer(FT_Library library, FT_Glyph_Format format, FT_ListNode *node);

FT2_1_3_BASE(FT_Error)
FT_Render_Glyph_Internal(FT_Library library, FT_GlyphSlot slot, FT_Render_Mode render_mode);

typedef const char *(*FT_Face_GetPostscriptNameFunc)(FT_Face face);
typedef FT_Error (*FT_Face_GetGlyphNameFunc)(FT_Face face, FT_UInt glyph_index, FT_Pointer buffer, FT_UInt buffer_max);
typedef FT_UInt (*FT_Face_GetGlyphNameIndexFunc)(FT_Face face, FT_String *glyph_name);


#ifndef FT2_1_3_CONFIG_OPTION_NO_DEFAULT_SYSTEM

FT2_1_3_EXPORT(FT_Memory)
FT_New_Memory(void);

FT2_1_3_EXPORT(void)
FT_Done_Memory(FT_Memory memory);

#endif /* !FT2_1_3_CONFIG_OPTION_NO_DEFAULT_SYSTEM */


/* Define default raster's interface.  The default raster is located in  */
/* `src/base/ftraster.c'.                                                */
/*                                                                       */
/* Client applications can register new rasters through the              */
/* FT2_1_3_Set_Raster() API.                                             */

#ifndef FT2_1_3_NO_DEFAULT_RASTER
FT2_1_3_EXPORT_VAR(FT_Raster_Funcs)
ft_default_raster;
#endif


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTOBJS_H */
