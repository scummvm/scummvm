#ifndef AGS_LIB_FREETYPE_FTSTROKER_H
#define AGS_LIB_FREETYPE_FTSTROKER_H

#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftoutln.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER

/*@*************************************************************
 *
 * @type: FT2_1_3_Stroker
 *
 * @description:
 *    opaque handler to a path stroker object
 */
typedef struct FT2_1_3_StrokerRec_*    FT2_1_3_Stroker;


/*@*************************************************************
 *
 * @enum: FT2_1_3_Stroker_LineJoin
 *
 * @description:
 *    these values determine how two joining lines are rendered
 *    in a stroker.
 *
 * @values:
 *    FT2_1_3_STROKER_LINEJOIN_ROUND ::
 *      used to render rounded line joins. circular arcs are used
 *      to join two lines smoothly
 *
 *    FT2_1_3_STROKER_LINEJOIN_BEVEL ::
 *      used to render beveled line joins; i.e. the two joining lines
 *      are extended until they intersect
 *
 *    FT2_1_3_STROKER_LINEJOIN_MITER ::
 *      same as beveled rendering, except that an additional line
 *      break is added if the angle between the two joining lines
 *      is too closed (this is useful to avoid unpleasant spikes
 *      in beveled rendering).
 */
typedef enum {
	FT2_1_3_STROKER_LINEJOIN_ROUND = 0,
	FT2_1_3_STROKER_LINEJOIN_BEVEL,
	FT2_1_3_STROKER_LINEJOIN_MITER

} FT2_1_3_Stroker_LineJoin;


/*@*************************************************************
 *
 * @enum: FT2_1_3_Stroker_LineCap
 *
 * @description:
 *    these values determine how the end of opened sub-paths are
 *    rendered in a stroke
 *
 * @values:
 *    FT2_1_3_STROKER_LINECAP_BUTT ::
 *      the end of lines is rendered as a full stop on the last
 *      point itself
 *
 *    FT2_1_3_STROKER_LINECAP_ROUND ::
 *      the end of lines is rendered as a half-circle around the
 *      last point
 *
 *    FT2_1_3_STROKER_LINECAP_SQUARE ::
 *      the end of lines is rendered as a square around the
 *      last point
 */
typedef enum {
	FT2_1_3_STROKER_LINECAP_BUTT = 0,
	FT2_1_3_STROKER_LINECAP_ROUND,
	FT2_1_3_STROKER_LINECAP_SQUARE

} FT2_1_3_Stroker_LineCap;

/* */

FT2_1_3_EXPORT( FT_Error )
FT2_1_3_Stroker_New( FT2_1_3_Memory    memory,
				FT2_1_3_Stroker  *astroker );

FT2_1_3_EXPORT( void )
FT2_1_3_Stroker_Set( FT2_1_3_Stroker           stroker,
				FT_Fixed             radius,
				FT2_1_3_Stroker_LineCap   line_cap,
				FT2_1_3_Stroker_LineJoin  line_join,
				FT_Fixed             miter_limit );


FT2_1_3_EXPORT( FT_Error )
FT2_1_3_Stroker_ParseOutline( FT2_1_3_Stroker   stroker,
						 FT_Outline*  outline,
						 FT_Bool      opened );

FT2_1_3_EXPORT( FT_Error )
FT2_1_3_Stroker_BeginSubPath( FT2_1_3_Stroker  stroker,
						 FT_Vector*  to,
						 FT_Bool     open );

FT2_1_3_EXPORT( FT_Error )
FT2_1_3_Stroker_EndSubPath( FT2_1_3_Stroker  stroker );


FT2_1_3_EXPORT( FT_Error )
FT2_1_3_Stroker_LineTo( FT2_1_3_Stroker  stroker,
				   FT_Vector*  to );

FT2_1_3_EXPORT( FT_Error )
FT2_1_3_Stroker_ConicTo( FT2_1_3_Stroker  stroker,
					FT_Vector*  control,
					FT_Vector*  to );

FT2_1_3_EXPORT( FT_Error )
FT2_1_3_Stroker_CubicTo( FT2_1_3_Stroker  stroker,
					FT_Vector*  control1,
					FT_Vector*  control2,
					FT_Vector*  to );


FT2_1_3_EXPORT( FT_Error )
FT2_1_3_Stroker_GetCounts( FT2_1_3_Stroker  stroker,
					  FT_UInt    *anum_points,
					  FT_UInt    *anum_contours );

FT2_1_3_EXPORT( void )
FT2_1_3_Stroker_Export( FT2_1_3_Stroker   stroker,
				   FT_Outline*  outline );

FT2_1_3_EXPORT( void )
FT2_1_3_Stroker_Done( FT2_1_3_Stroker  stroker );


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTSTROKER_H */
