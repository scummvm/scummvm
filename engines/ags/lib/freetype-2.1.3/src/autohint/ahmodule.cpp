/***************************************************************************/
/*                                                                         */
/*  ahmodule.c                                                             */
/*                                                                         */
/*    Auto-hinting module implementation (declaration).                    */
/*                                                                         */
/*  Copyright 2000-2001, 2002 Catharon Productions Inc.                    */
/*  Author: David Turner                                                   */
/*                                                                         */
/*  This file is part of the Catharon Typography Project and shall only    */
/*  be used, modified, and distributed under the terms of the Catharon     */
/*  Open Source License that should come with this file under the name     */
/*  `CatharonLicense.txt'.  By continuing to use, modify, or distribute    */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/*  Note that this license is compatible with the FreeType license.        */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftmodule.h"
#include "ahhint.h"


#ifdef  DEBUG_HINTER
AH_Hinter  ah_debug_hinter       = NULL;
FT2_1_3_Bool    ah_debug_disable_horz = 0;
FT2_1_3_Bool    ah_debug_disable_vert = 0;
#endif

typedef struct  FT2_1_3_AutoHinterRec_ {
	FT2_1_3_ModuleRec  root;
	AH_Hinter     hinter;

} FT2_1_3_AutoHinterRec;


FT2_1_3_CALLBACK_DEF( FT2_1_3_Error )
ft_autohinter_init( FT2_1_3_AutoHinter  module ) {
	FT2_1_3_Error  error;


	error = ah_hinter_new( module->root.library, &module->hinter );
#ifdef DEBUG_HINTER
	if ( !error )
		ah_debug_hinter = module->hinter;
#endif
	return error;
}


FT2_1_3_CALLBACK_DEF( void )
ft_autohinter_done( FT2_1_3_AutoHinter  module ) {
	ah_hinter_done( module->hinter );

#ifdef DEBUG_HINTER
	ah_debug_hinter = NULL;
#endif
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_Error )
ft_autohinter_load_glyph( FT2_1_3_AutoHinter  module,
                          FT2_1_3_GlyphSlot   slot,
                          FT2_1_3_Size        size,
                          FT2_1_3_UInt        glyph_index,
                          FT2_1_3_Int32       load_flags ) {
	return ah_hinter_load_glyph( module->hinter,
	                             slot, size, glyph_index, load_flags );
}


FT2_1_3_CALLBACK_DEF( void )
ft_autohinter_reset_globals( FT2_1_3_AutoHinter  module,
                             FT2_1_3_Face        face ) {
	FT2_1_3_UNUSED( module );

	if ( face->autohint.data )
		ah_hinter_done_face_globals( (AH_Face_Globals)(face->autohint.data) );
}


FT2_1_3_CALLBACK_DEF( void )
ft_autohinter_get_globals( FT2_1_3_AutoHinter  module,
                           FT2_1_3_Face        face,
                           void**         global_hints,
                           long*          global_len ) {
	ah_hinter_get_global_hints( module->hinter, face,
	                            global_hints, global_len );
}


FT2_1_3_CALLBACK_DEF( void )
ft_autohinter_done_globals( FT2_1_3_AutoHinter  module,
                            void*          global_hints ) {
	ah_hinter_done_global_hints( module->hinter, global_hints );
}


FT2_1_3_CALLBACK_TABLE_DEF
const FT2_1_3_AutoHinter_ServiceRec  ft_autohinter_service = {
	ft_autohinter_reset_globals,
	ft_autohinter_get_globals,
	ft_autohinter_done_globals,
	ft_autohinter_load_glyph
};


FT2_1_3_CALLBACK_TABLE_DEF
const FT2_1_3_Module_Class  autohint_module_class = {
	ft_module_hinter,
	sizeof ( FT2_1_3_AutoHinterRec ),

	"autohinter",
	0x10000L,   /* version 1.0 of the autohinter  */
	0x20000L,   /* requires FreeType 2.0 or above */

	(const void*) &ft_autohinter_service,

	(FT2_1_3_Module_Constructor)ft_autohinter_init,
	(FT2_1_3_Module_Destructor) ft_autohinter_done,
	(FT2_1_3_Module_Requester)  0
};


/* END */
