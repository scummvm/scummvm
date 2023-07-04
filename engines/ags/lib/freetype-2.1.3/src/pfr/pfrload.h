/***************************************************************************/
/*                                                                         */
/*  pfrload.h                                                              */
/*                                                                         */
/*    FreeType PFR loader (specification).                                 */
/*                                                                         */
/*  Copyright 2002 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_PFRLOAD_H
#define AGS_LIB_FREETYPE_PFRLOAD_H

#include "pfrobjs.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftstream.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER

#ifdef PFR_CONFIG_NO_CHECKS
#define PFR_CHECK( x )  do { } while ( 0 )
#else
#define PFR_CHECK(x)         \
	do {                     \
		if (p + (x) > limit) \
			goto Too_Short;  \
	} while (0)
#endif

#define PFR_NEXT_BYTE( p )    FT2_1_3_NEXT_BYTE( p )
#define PFR_NEXT_INT8( p )    FT2_1_3_NEXT_CHAR( p )
#define PFR_NEXT_SHORT( p )   FT2_1_3_NEXT_SHORT( p )
#define PFR_NEXT_USHORT( p )  FT2_1_3_NEXT_USHORT( p )
#define PFR_NEXT_LONG( p )    FT2_1_3_NEXT_OFF3( p )
#define PFR_NEXT_ULONG( p )   FT2_1_3_NEXT_UOFF3( p )


/* handling extra items */

typedef FT2_1_3_Error (*PFR_ExtraItem_ParseFunc)(FT2_1_3_Byte *p, FT2_1_3_Byte *limit, FT2_1_3_Pointer data);

typedef struct  PFR_ExtraItemRec_ {
	FT2_1_3_UInt                  type;
	PFR_ExtraItem_ParseFunc  parser;

} PFR_ExtraItemRec;

typedef const struct PFR_ExtraItemRec_*  PFR_ExtraItem;

FT2_1_3_LOCAL(FT2_1_3_Error)
pfr_extra_items_skip(FT2_1_3_Byte **pp, FT2_1_3_Byte *limit);

FT2_1_3_LOCAL(FT2_1_3_Error)
pfr_extra_items_parse(FT2_1_3_Byte **pp, FT2_1_3_Byte *limit, PFR_ExtraItem item_list, FT2_1_3_Pointer item_data);

/* load a PFR header */
FT2_1_3_LOCAL(FT2_1_3_Error)
pfr_header_load(PFR_Header header, FT2_1_3_Stream stream);

/* check a PFR header */
FT2_1_3_LOCAL(FT2_1_3_Bool)
pfr_header_check(PFR_Header header);

/* return number of logical fonts in this file */
FT2_1_3_LOCAL(FT2_1_3_Error)
pfr_log_font_count(FT2_1_3_Stream stream, FT2_1_3_UInt32 log_section_offset, FT2_1_3_UInt *acount);

/* load a pfr logical font entry */
FT2_1_3_LOCAL(FT2_1_3_Error)
pfr_log_font_load(PFR_LogFont log_font, FT2_1_3_Stream stream, FT2_1_3_UInt face_index, FT2_1_3_UInt32 section_offset, FT2_1_3_Bool size_increment);

/* load a physical font entry */
FT2_1_3_LOCAL(FT2_1_3_Error)
pfr_phy_font_load(PFR_PhyFont phy_font, FT2_1_3_Stream stream, FT2_1_3_UInt32 offset, FT2_1_3_UInt32 size);

/* finalize a physical font */
FT2_1_3_LOCAL(void)
pfr_phy_font_done(PFR_PhyFont phy_font, FT2_1_3_Memory memory);

/* */

FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_PFRLOAD_H */


/* END */
