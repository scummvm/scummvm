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

#include "ac/global_dynamicsprite.h"
#include "util/wgt2allg.h" // Allegro RGB, PALETTE
#include "ac/draw.h"
#include "ac/dynamicsprite.h"
#include "ac/path_helper.h"
#include "ac/spritecache.h"
#include "ac/runtime_defines.h" //MAX_PATH
#include "gfx/graphicsdriver.h"
#include "gfx/bitmap.h"

using namespace AGS::Common;
using namespace AGS::Engine;

extern SpriteCache spriteset;
extern IGraphicsDriver *gfxDriver;

int LoadImageFile(const char *filename)
{
    ResolvedPath rp;
    if (!ResolveScriptPath(filename, true, rp))
        return 0;

    Bitmap *loadedFile = BitmapHelper::LoadFromFile(rp.FullPath);
    if (!loadedFile && !rp.AltPath.IsEmpty() && rp.AltPath.Compare(rp.FullPath) != 0)
        loadedFile = BitmapHelper::LoadFromFile(rp.AltPath);
    if (!loadedFile)
        return 0;

    int gotSlot = spriteset.GetFreeIndex();
    if (gotSlot <= 0)
        return 0;

    add_dynamic_sprite(gotSlot, ReplaceBitmapWithSupportedFormat(loadedFile));

    return gotSlot;
}
