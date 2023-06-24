#ifndef __FT2_1_3_INTERNAL_PFR_H__
#define __FT2_1_3_INTERNAL_PFR_H__

#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_FREETYPE_H

FT2_1_3_BEGIN_HEADER

typedef FT2_1_3_Error  (*FT2_1_3_PFR_GetMetricsFunc)( FT2_1_3_Face    face,
        FT2_1_3_UInt   *aoutline,
        FT2_1_3_UInt   *ametrics,
        FT2_1_3_Fixed  *ax_scale,
        FT2_1_3_Fixed  *ay_scale );

typedef FT2_1_3_Error  (*FT2_1_3_PFR_GetKerningFunc)( FT2_1_3_Face     face,
        FT2_1_3_UInt     left,
        FT2_1_3_UInt     right,
        FT2_1_3_Vector  *avector );

typedef FT2_1_3_Error  (*FT2_1_3_PFR_GetAdvanceFunc)( FT2_1_3_Face   face,
        FT2_1_3_UInt   gindex,
        FT2_1_3_Pos   *aadvance );

typedef struct FT2_1_3_PFR_ServiceRec_ {
	FT2_1_3_PFR_GetMetricsFunc    get_metrics;
	FT2_1_3_PFR_GetKerningFunc    get_kerning;
	FT2_1_3_PFR_GetAdvanceFunc    get_advance;

} FT2_1_3_PFR_ServiceRec, *FT2_1_3_PFR_Service;

#define  FT2_1_3_PFR_SERVICE_NAME  "pfr"

FT2_1_3_END_HEADER

#endif /* __FT2_1_3_INTERNAL_PFR_H__ */
