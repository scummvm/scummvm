#ifndef AGS_LIB_FREETYPE_FTEXCEPT_H
#define AGS_LIB_FREETYPE_FTEXCEPT_H

#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER



/* I can't find a better place for this for now */


	/* the size of a cleanup chunk in bytes is FT2_1_3_CLEANUP_CHUNK_SIZE*12 + 4 */
	/* this must be a small power of 2 whenever possible..                  */
	/*                                                                      */
	/* with a value of 5, we have a byte size of 64 bytes per chunk..       */
	/*                                                                      */
#define  FT2_1_3_CLEANUP_CHUNK_SIZE   5



typedef struct FT2_1_3_CleanupItemRec_ {
	FT2_1_3_Pointer      item;
	FT2_1_3_CleanupFunc  item_func;
	FT2_1_3_Pointer      item_data;

} FT2_1_3_CleanupItemRec;

typedef struct FT2_1_3_CleanupChunkRec_*   FT2_1_3_CleanupChunk;

typedef struct FT2_1_3_CleanupChunkRec_ {
	FT2_1_3_CleanupChunk    link;
	FT2_1_3_CleanupItemRec  items[ FT2_1_3_CLEANUP_CHUNK_SIZE ];

} FT2_1_3_CleanupChunkRec;


typedef struct FT2_1_3_CleanupStackRec_ {
	FT2_1_3_CleanupItem     top;
	FT2_1_3_CleanupItem     limit;
	FT2_1_3_CleanupChunk    chunk;
	FT2_1_3_CleanupChunkRec chunk_0;  /* avoids stupid dynamic allocation */
	FT2_1_3_Memory          memory;

} FT2_1_3_CleanupStackRec, *FT2_1_3_CleanupStack;


FT2_1_3_BASE( void )
FT2_1_3_cleanup_stack_push( FT2_1_3_CleanupStack  stack,
					   FT2_1_3_Pointer       item,
					   FT2_1_3_CleanupFunc   item_func,
					   FT2_1_3_Pointer       item_data );

FT2_1_3_BASE( void )
FT2_1_3_cleanup_stack_pop( FT2_1_3_CleanupStack   stack,
					  FT2_1_3_Int            destroy );

FT2_1_3_BASE( FT2_1_3_CleanupItem )
FT2_1_3_cleanup_stack_peek( FT2_1_3_CleanupStack  stack );

FT2_1_3_BASE( void )
FT2_1_3_xhandler_enter( FT2_1_3_XHandler  xhandler,
				   FT2_1_3_Memory    memory );

FT2_1_3_BASE( void )
FT2_1_3_xhandler_exit( FT2_1_3_XHandler  xhandler );


FT2_1_3_BASE( void )
FT2_1_3_cleanup_throw( FT2_1_3_CleanupStack  stack,
				  FT2_1_3_Error         error );

FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTEXCEPT_H */
