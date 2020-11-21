//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
#ifndef __AGS_EE_MAIN__ENGINESETUP_H
#define __AGS_EE_MAIN__ENGINESETUP_H

#include "util/geometry.h"
#include "gfx/gfxdefines.h"

// Sets up game viewport and object scaling parameters depending on game.
// TODO: this is part of the game init, not engine init, move it later
void engine_init_resolution_settings(const Size game_size);
// Setup engine after the graphics mode has changed
void engine_post_gfxmode_setup(const Size &init_desktop);
// Prepare engine for graphics mode release; could be called before switching display mode too
void engine_pre_gfxmode_release();
// Prepare engine to the graphics mode shutdown and gfx driver destruction
void engine_pre_gfxsystem_shutdown();
// Applies necessary changes after screen<->virtual coordinate transformation has changed
void on_coordinates_scaling_changed();

#endif // __AGS_EE_MAIN__ENGINESETUP_H
