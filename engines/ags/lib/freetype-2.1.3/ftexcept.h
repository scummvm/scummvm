#ifndef AGS_LIB_FREETYPE_FTEXCEPT_H
#define AGS_LIB_FREETYPE_FTEXCEPT_H

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"

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



typedef struct FT_CleanupItemRec_ {
	FT_Pointer      item;
	FT_CleanupFunc  item_func;
	FT_Pointer      item_data;

} FT_CleanupItemRec;

typedef struct FT_CleanupChunkRec_*   FT_CleanupChunk;

typedef struct FT_CleanupChunkRec_ {
	FT_CleanupChunk    link;
	FT_CleanupItemRec  items[ FT2_1_3_CLEANUP_CHUNK_SIZE ];

} FT_CleanupChunkRec;


typedef struct FT_CleanupStackRec_ {
	FT_CleanupItem     top;
	FT_CleanupItem     limit;
	FT_CleanupChunk    chunk;
	FT_CleanupChunkRec chunk_0;  /* avoids stupid dynamic allocation */
	FT_Memory          memory;

} FT_CleanupStackRec, *FT_CleanupStack;


FT2_1_3_BASE( void )
ft_cleanup_stack_push( FT_CleanupStack  stack,
					   FT_Pointer       item,
					   FT_CleanupFunc   item_func,
					   FT_Pointer       item_data );

FT2_1_3_BASE( void )
ft_cleanup_stack_pop( FT_CleanupStack   stack,
					  FT_Int            destroy );

FT2_1_3_BASE( FT_CleanupItem )
ft_cleanup_stack_peek( FT_CleanupStack  stack );

FT2_1_3_BASE( void )
ft_xhandler_enter( FT_XHandler  xhandler,
				   FT_Memory    memory );

FT2_1_3_BASE( void )
ft_xhandler_exit( FT_XHandler  xhandler );


FT2_1_3_BASE( void )
ft_cleanup_throw( FT_CleanupStack  stack,
				  FT_Error         error );

FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTEXCEPT_H */
