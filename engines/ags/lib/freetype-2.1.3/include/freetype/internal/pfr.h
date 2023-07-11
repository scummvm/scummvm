#ifndef AGS_LIB_FREETYPE_PFR_H
#define AGS_LIB_FREETYPE_PFR_H

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER

typedef FT_Error  (*FT2_1_3_PFR_GetMetricsFunc)( FT_Face    face,
		FT_UInt   *aoutline,
		FT_UInt   *ametrics,
		FT_Fixed  *ax_scale,
		FT_Fixed  *ay_scale );

typedef FT_Error  (*FT2_1_3_PFR_GetKerningFunc)( FT_Face     face,
		FT_UInt     left,
		FT_UInt     right,
		FT_Vector  *avector );

typedef FT_Error  (*FT2_1_3_PFR_GetAdvanceFunc)( FT_Face   face,
		FT_UInt   gindex,
		FT_Pos   *aadvance );

typedef struct FT2_1_3_PFR_ServiceRec_ {
	FT2_1_3_PFR_GetMetricsFunc    get_metrics;
	FT2_1_3_PFR_GetKerningFunc    get_kerning;
	FT2_1_3_PFR_GetAdvanceFunc    get_advance;

} FT2_1_3_PFR_ServiceRec, *FT2_1_3_PFR_Service;

#define  FT2_1_3_PFR_SERVICE_NAME  "pfr"

FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_PFR_H */
