/***************************************************************************/
/*                                                                         */
/*  ftheader.h                                                             */
/*                                                                         */
/*    Build macros of the FreeType 2 library.                              */
/*                                                                         */
/*  Copyright 1996-2001, 2002 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#if 0

#ifndef __FT2_1_3_HEADER_H__
#define __FT2_1_3_HEADER_H__

/*@***********************************************************************/
/*                                                                       */
/* <Macro>                                                               */
/*    FT_BEGIN_HEADER                                                    */
/*                                                                       */
/* <Description>                                                         */
/*    This macro is used in association with @FT_END_HEADER in header    */
/*    files to ensure that the declarations within are properly          */
/*    encapsulated in an `extern "C" { .. }' block when included from a  */
/*    C++ compiler.                                                      */
/*                                                                       */
#ifdef __cplusplus
#define FT_BEGIN_HEADER  extern "C" {
#else
#define FT_BEGIN_HEADER  /* nothing */
#endif


/*@***********************************************************************/
/*                                                                       */
/* <Macro>                                                               */
/*    FT_END_HEADER                                                      */
/*                                                                       */
/* <Description>                                                         */
/*    This macro is used in association with @FT_BEGIN_HEADER in header  */
/*    files to ensure that the declarations within are properly          */
/*    encapsulated in an `extern "C" { .. }' block when included from a  */
/*    C++ compiler.                                                      */
/*                                                                       */
#ifdef __cplusplus
#define FT_END_HEADER  }
#else
#define FT_END_HEADER  /* nothing */
#endif


/*************************************************************************/
/*                                                                       */
/* Aliases for the FreeType 2 public and configuration files.            */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* <Section>                                                             */
/*    header_file_macros                                                 */
/*                                                                       */
/* <Title>                                                               */
/*    Header File Macros                                                 */
/*                                                                       */
/* <Abstract>                                                            */
/*    Macro definitions used to #include specific header files.          */
/*                                                                       */
/* <Description>                                                         */
/*    The following macros are defined to the name of specific           */
/*    FreeType 2 header files.  They can be used directly in #include    */
/*    statements as in:                                                  */
/*                                                                       */
/*    {                                                                  */
/*      #include FT2_1_3_FREETYPE_H                                           */
/*      #include FT2_1_3_MULTIPLE_MASTERS_H                                   */
/*      #include FT_GLYPH_H                                              */
/*    }                                                                  */
/*                                                                       */
/*    There are several reasons why we are now using macros to name      */
/*    public header files.  The first one is that such macros are not    */
/*    limited to the infamous 8.3 naming rule required by DOS (and       */
/*    `FT2_1_3_MULTIPLE_MASTERS_H' is a lot more meaningful than `ftmm.h').   */
/*                                                                       */
/*    The second reason is that is allows for more flexibility in the    */
/*    way FreeType 2 is installed on a given system.                     */
/*                                                                       */
/*************************************************************************/

/* configuration files */

/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_CONFIG_CONFIG_H                                                 */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    FreeType 2 configuration data.                                     */
/*                                                                       */
#ifndef FT2_1_3_CONFIG_CONFIG_H
#define FT2_1_3_CONFIG_CONFIG_H  "engines/ags/lib/freetype-2.1.3/config/ftconfig.h"
#endif


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_CONFIG_STANDARD_LIBRARY_H                                       */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    FreeType 2 configuration data.                                     */
/*                                                                       */
#ifndef FT2_1_3_CONFIG_STANDARD_LIBRARY_H
#define FT2_1_3_CONFIG_STANDARD_LIBRARY_H  "engines/ags/lib/freetype-2.1.3/config/ftstdlib.h"
#endif


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_CONFIG_OPTIONS_H                                                */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    FreeType 2 project-specific configuration options.                 */
/*                                                                       */
#ifndef FT2_1_3_CONFIG_OPTIONS_H
#define FT2_1_3_CONFIG_OPTIONS_H  "engines/ags/lib/freetype-2.1.3/config/ftoption.h"
#endif


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_CONFIG_MODULES_H                                                */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the list of FreeType 2 modules that are statically linked to new   */
/*    library instances in @FT2_1_3_Init_FreeType.                            */
/*                                                                       */
#ifndef FT2_1_3_CONFIG_MODULES_H
#define FT2_1_3_CONFIG_MODULES_H  "engines/ags/lib/freetype-2.1.3/config/ftmodule.h"
#endif

/* public headers */

/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_FREETYPE_H                                                      */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the base FreeType 2 API.                                           */
/*                                                                       */
#define FT2_1_3_FREETYPE_H  "engines/ags/lib/freetype-2.1.3/freetype.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT_ERRORS_H                                                        */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the list of FreeType 2 error codes (and messages).                 */
/*                                                                       */
/*    It is included by @FT2_1_3_FREETYPE_H.                                  */
/*                                                                       */
#define FT_ERRORS_H  "engines/ags/lib/freetype-2.1.3/fterrors.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_MODULE_ERRORS_H                                                 */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the list of FreeType 2 module error offsets (and messages).        */
/*                                                                       */
#define FT2_1_3_MODULE_ERRORS_H  "engines/ags/lib/freetype-2.1.3/ftmoderr.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_SYSTEM_H                                                        */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the FreeType 2 interface to low-level operations (i.e. memory      */
/*    management and stream i/o).                                        */
/*                                                                       */
/*    It is included by @FT2_1_3_FREETYPE_H.                                  */
/*                                                                       */
#define FT2_1_3_SYSTEM_H  "engines/ags/lib/freetype-2.1.3/ftsystem.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_IMAGE_H                                                         */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    types definitions related to glyph images (i.e. bitmaps, outlines, */
/*    scan-converter parameters).                                        */
/*                                                                       */
/*    It is included by @FT2_1_3_FREETYPE_H.                                  */
/*                                                                       */
#define FT2_1_3_IMAGE_H  "engines/ags/lib/freetype-2.1.3/ftimage.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_TYPES_H                                                         */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the basic data types defined by FreeType 2.                        */
/*                                                                       */
/*    It is included by @FT2_1_3_FREETYPE_H.                                  */
/*                                                                       */
#define FT2_1_3_TYPES_H  "engines/ags/lib/freetype-2.1.3/fttypes.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_LIST_H                                                          */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the list management API of FreeType 2.                             */
/*                                                                       */
/*    (Most applications will never need to include this file.)          */
/*                                                                       */
#define FT2_1_3_LIST_H  "engines/ags/lib/freetype-2.1.3/ftlist.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT_OUTLINE_H                                                       */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the scalable outline management API of FreeType 2.                 */
/*                                                                       */
#define FT_OUTLINE_H  "engines/ags/lib/freetype-2.1.3/ftoutln.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_SIZES_H                                                         */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the API used to manage multiple @FT_Size objects per face.         */
/*                                                                       */
#define FT2_1_3_SIZES_H  "engines/ags/lib/freetype-2.1.3/ftsizes.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_MODULE_H                                                        */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the module management API of FreeType 2.                           */
/*                                                                       */
#define FT2_1_3_MODULE_H  "engines/ags/lib/freetype-2.1.3/ftmodule.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT_RENDER_H                                                        */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the renderer module management API of FreeType 2.                  */
/*                                                                       */
#define FT_RENDER_H  "engines/ags/lib/freetype-2.1.3/ftrender.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_TYPE1_TABLES_H                                                  */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the types and API specific to the Type 1 format.                   */
/*                                                                       */
#define FT2_1_3_TYPE1_TABLES_H  "engines/ags/lib/freetype-2.1.3/t1tables.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_TRUETYPE_IDS_H                                                  */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the enumeration values used to identify name strings, languages,   */
/*    encodings, etc.  This file really contains a _large_ set of        */
/*    constant macro definitions, taken from the TrueType and OpenType   */
/*    specifications.                                                    */
/*                                                                       */
#define FT2_1_3_TRUETYPE_IDS_H  "engines/ags/lib/freetype-2.1.3/ttnameid.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_TRUETYPE_TABLES_H                                               */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the types and API specific to the TrueType (as well as OpenType)   */
/*    format.                                                            */
/*                                                                       */
#define FT2_1_3_TRUETYPE_TABLES_H  "engines/ags/lib/freetype-2.1.3/tttables.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_TRUETYPE_TAGS_H                                                 */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the definitions of TrueType 4-byte `tags' used to identify blocks  */
/*    in SFNT-based font formats (i.e. TrueType and OpenType).           */
/*                                                                       */
#define FT2_1_3_TRUETYPE_TAGS_H  "engines/ags/lib/freetype-2.1.3/tttags.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_BDF_H                                                           */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the definitions of an API to access BDF-specific strings from a    */
/*    face.                                                              */
/*                                                                       */
#define FT2_1_3_BDF_H  "engines/ags/lib/freetype-2.1.3/ftbdf.h"

/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_GZIP_H                                                          */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the definitions of an API to support for gzip-compressed files.    */
/*                                                                       */
#define FT2_1_3_GZIP_H  "engines/ags/lib/freetype-2.1.3/ftgzip.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT_GLYPH_H                                                         */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the API of the optional glyph management component.                */
/*                                                                       */
#define FT_GLYPH_H  "engines/ags/lib/freetype-2.1.3/ftglyph.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_BBOX_H                                                          */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the API of the optional exact bounding box computation routines.   */
/*                                                                       */
#define FT2_1_3_BBOX_H  "engines/ags/lib/freetype-2.1.3/ftbbox.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_CACHE_H                                                         */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the API of the optional FreeType 2 cache sub-system.               */
/*                                                                       */
#define FT2_1_3_CACHE_H  "engines/ags/lib/freetype-2.1.3/cache/ftcache.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_CACHE_IMAGE_H                                                   */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the `glyph image' API of the FreeType 2 cache sub-system.          */
/*                                                                       */
/*    It is used to define a cache for @FT_Glyph elements.  You can also */
/*    see the API defined in @FT2_1_3_CACHE_SMALL_BITMAPS_H if you only need  */
/*    to store small glyph bitmaps, as it will use less memory.          */
/*                                                                       */
#define FT2_1_3_CACHE_IMAGE_H  "engines/ags/lib/freetype-2.1.3/cache/ftcimage.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_CACHE_SMALL_BITMAPS_H                                           */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the `small bitmaps' API of the FreeType 2 cache sub-system.        */
/*                                                                       */
/*    It is used to define a cache for small glyph bitmaps in a          */
/*    relatively memory-efficient way.  You can also use the API defined */
/*    in @FT2_1_3_CACHE_IMAGE_H if you want to cache arbitrary glyph images,  */
/*    including scalable outlines.                                       */
/*                                                                       */
#define FT2_1_3_CACHE_SMALL_BITMAPS_H  "engines/ags/lib/freetype-2.1.3/cache/ftcsbits.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_CACHE_CHARMAP_H                                                 */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the `charmap' API of the FreeType 2 cache sub-system.              */
/*                                                                       */
#define FT2_1_3_CACHE_CHARMAP_H  "engines/ags/lib/freetype-2.1.3/cache/ftccmap.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_MAC_H                                                           */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the Macintosh-specific FreeType 2 API. The latter is used to       */
/*    access fonts embedded in resource forks.                           */
/*                                                                       */
/*    This header file must be explicitly included by client             */
/*    applications compiled on the Mac (note that the base API still     */
/*    works though).                                                     */
/*                                                                       */
#define FT2_1_3_MAC_H  "engines/ags/lib/freetype-2.1.3/ftmac.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_MULTIPLE_MASTERS_H                                              */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the optional multiple-masters management API of FreeType 2.        */
/*                                                                       */
#define FT2_1_3_MULTIPLE_MASTERS_H  "engines/ags/lib/freetype-2.1.3/ftmm.h"


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_SFNT_NAMES_H                                                    */
/*                                                                       */
/* @description:                                                         */
/*    A macro used in #include statements to name the file containing    */
/*    the optional FreeType 2 API used to access embedded `name' strings */
/*    in SFNT-based font formats (i.e. TrueType and OpenType).           */
/*                                                                       */
#define FT2_1_3_SFNT_NAMES_H  "engines/ags/lib/freetype-2.1.3/ftsnames.h"

/* */

#define FT2_1_3_TRIGONOMETRY_H          "engines/ags/lib/freetype-2.1.3/fttrigon.h"
#define FT2_1_3_STROKER_H               "engines/ags/lib/freetype-2.1.3/ftstroker.h"
#define FT2_1_3_SYNTHESIS_H             "engines/ags/lib/freetype-2.1.3/include/freetype/ftsynth.h"
#define FT_ERROR_DEFINITIONS_H     "engines/ags/lib/freetype-2.1.3/fterrdef.h"

#define FT2_1_3_CACHE_MANAGER_H         "engines/ags/lib/freetype-2.1.3/cache/ftcmanag.h"

#define FT2_1_3_CACHE_INTERNAL_LRU_H    "engines/ags/lib/freetype-2.1.3/cache/ftlru.h"
#define FT2_1_3_CACHE_INTERNAL_GLYPH_H  "engines/ags/lib/freetype-2.1.3/cache/ftcglyph.h"
#define FT2_1_3_CACHE_INTERNAL_CACHE_H  "engines/ags/lib/freetype-2.1.3/cache/ftccache.h"

#define FT2_1_3_XFREE86_H               "engines/ags/lib/freetype-2.1.3/ftxf86.h"

#define FT2_1_3_INCREMENTAL_H           "engines/ags/lib/freetype-2.1.3/ftincrem.h"

/* now include internal headers definitions from <freetype/internal/...> */

#define  FT2_1_3_INTERNAL_INTERNAL_H    "engines/ags/lib/freetype-2.1.3/include/freetype/internal/internal.h"
#include FT2_1_3_INTERNAL_INTERNAL_H


#endif /* __FT2_BUILD_H__ */

#endif
/* END */
