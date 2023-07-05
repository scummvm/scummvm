#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_SYSTEM_MEMORY_H

static FT2_1_3_Memory
ft_memory_new_default( FT_ULong  size ) {
	return (FT2_1_3_Memory) ft_malloc( size );
}

static void
ft_memory_destroy_default( FT2_1_3_Memory  memory ) {
	ft_free( memory );
}


/* notice that in normal builds, we use the ISO C library functions */
/* 'malloc', 'free' and 'realloc' directly..                        */
/*                                                                  */
static const FT2_1_3_Memory_FuncsRec  ft_memory_funcs_default_rec = {
	(FT2_1_3_Memory_CreateFunc)  ft_memory_new_iso,
	(FT2_1_3_Memory_DestroyFunc) ft_memory_destroy_iso,
	(FT2_1_3_Memory_AllocFunc)   ft_malloc,
	(FT2_1_3_Memory_FreeFunc)    ft_free,
	(FT2_1_3_Memory_ReallocFunc) ft_realloc
};

FT2_1_3_APIVAR_DEF( const FT2_1_3_Memory_Funcs )
ft_memory_funcs_default = &ft_memory_funcs_defaults_rec;
