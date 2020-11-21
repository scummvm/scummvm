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
//
//
//
//=============================================================================
#ifndef __AGS_EE_MAIN__MAINHEADER_H
#define __AGS_EE_MAIN__MAINHEADER_H

#include "core/platform.h"

#include "main/maindefines_ex.h"

#include "ac/math.h"
#include "script/script_runtime.h"
#include "gui/animatingguibutton.h"
#include "gui/guibutton.h"
#include "gfx/gfxfilter.h"
#include "util/string_utils.h"
#include "device/mousew32.h"
#include "ac/route_finder.h"
#include "util/misc.h"
#include "script/cc_error.h"

// include last since we affect windows includes
#include "ac/file.h"

#if AGS_PLATFORM_OS_ANDROID
#include <sys/stat.h>
#include <android/log.h>

extern "C" void selectLatestSavegame();
extern bool psp_load_latest_savegame;
#endif

#endif // __AGS_EE_MAIN__MAINHEADER_H
