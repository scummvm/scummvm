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
/*  ahmodule.c                                                             */
/*    Auto-hinting module implementation (declaration).                    */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftmodule.h"
#include "engines/ags/lib/freetype-2.1.3/modules/autohint/ahhint.h"

namespace AGS3 {
namespace FreeType213 {


#ifdef  DEBUG_HINTER
AH_Hinter ah_debug_hinter = NULL;
FT_Bool ah_debug_disable_horz = 0;
FT_Bool ah_debug_disable_vert = 0;
#endif

typedef struct FT_AutoHinterRec_ {
	FT_ModuleRec root;
	AH_Hinter hinter;
} FT_AutoHinterRec;

FT_CALLBACK_DEF(FT_Error)
ft_autohinter_init(FT_AutoHinter module) {
	FT_Error error;

	error = ah_hinter_new(module->root.library, &module->hinter);
#ifdef DEBUG_HINTER
	if (!error)
		ah_debug_hinter = module->hinter;
#endif
	return error;
}

FT_CALLBACK_DEF(void)
ft_autohinter_done(FT_AutoHinter module) {
	ah_hinter_done(module->hinter);

#ifdef DEBUG_HINTER
	ah_debug_hinter = NULL;
#endif
}

FT_CALLBACK_DEF(FT_Error)
ft_autohinter_load_glyph(FT_AutoHinter module, FT_GlyphSlot slot, FT_Size size, FT_UInt glyph_index, FT_Int32 load_flags) {
	return ah_hinter_load_glyph(module->hinter, slot, size, glyph_index, load_flags);
}

FT_CALLBACK_DEF(void)
ft_autohinter_reset_globals(FT_AutoHinter module, FT_Face face) {
	FT_UNUSED(module);

	if (face->autohint.data)
		ah_hinter_done_face_globals((AH_Face_Globals)(face->autohint.data));
}

FT_CALLBACK_DEF(void)
ft_autohinter_get_globals(FT_AutoHinter module, FT_Face face, void **global_hints, long *global_len) {
	ah_hinter_get_global_hints(module->hinter, face, global_hints, global_len);
}

FT_CALLBACK_DEF(void)
ft_autohinter_done_globals(FT_AutoHinter module, void *global_hints) {
	ah_hinter_done_global_hints(module->hinter, global_hints);
}

FT_CALLBACK_TABLE_DEF
const FT_AutoHinter_ServiceRec ft_autohinter_service = {
	ft_autohinter_reset_globals,
	ft_autohinter_get_globals,
	ft_autohinter_done_globals,
	ft_autohinter_load_glyph};

FT_CALLBACK_TABLE_DEF
const FT_Module_Class autohint_module_class = {
	ft_module_hinter,
	sizeof(FT_AutoHinterRec),

	"autohinter",
	0x10000L, /* version 1.0 of the autohinter  */
	0x20000L, /* requires FreeType 2.0 or above */

	(const void *)&ft_autohinter_service,

	(FT_Module_Constructor)ft_autohinter_init,
	(FT_Module_Destructor)ft_autohinter_done,
	(FT_Module_Requester)0};


} // End of namespace FreeType213
} // End of namespace AGS3
