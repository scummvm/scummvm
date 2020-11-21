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

#include "gui/guiinv.h"
#include "gui/guimain.h"
#include "ac/draw.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "ac/characterextras.h"
#include "ac/spritecache.h"
#include "gfx/bitmap.h"


extern GameSetupStruct game;
extern int gui_disabled_style;
extern GameState play;
extern CharacterExtras *charextra;
extern SpriteCache spriteset;


namespace AGS
{
namespace Common
{

int GUIInvWindow::GetCharacterId() const
{
    if (CharId < 0)
        return game.playercharacter;

    return CharId;
}

void GUIInvWindow::Draw(Bitmap *ds)
{
    const bool enabled = IsGUIEnabled(this);
    if (!enabled && (gui_disabled_style == GUIDIS_BLACKOUT))
        return;

    // backwards compatibility
    play.inv_numinline = ColCount;
    play.inv_numdisp = RowCount * ColCount;
    play.obsolete_inv_numorder = charextra[game.playercharacter].invorder_count;
    // if the user changes top_inv_item, switch into backwards
    // compatibiltiy mode
    if (play.inv_top)
        play.inv_backwards_compatibility = 1;
    if (play.inv_backwards_compatibility)
        TopItem = play.inv_top;

    // draw the items
    const int leftmost_x = X;
    int at_x = X;
    int at_y = Y;
    int lastItem = TopItem + (ColCount * RowCount);
    if (lastItem > charextra[GetCharacterId()].invorder_count)
        lastItem = charextra[GetCharacterId()].invorder_count;

    for (int item = TopItem; item < lastItem; ++item)
    {
        // draw inv graphic
        draw_gui_sprite(ds, game.invinfo[charextra[GetCharacterId()].invorder[item]].pic, at_x, at_y, true);
        at_x += data_to_game_coord(ItemWidth);

        // go to next row when appropriate
        if ((item - TopItem) % ColCount == (ColCount - 1))
        {
            at_x = leftmost_x;
            at_y += data_to_game_coord(ItemHeight);
        }
    }

    if (!enabled &&
        gui_disabled_style == GUIDIS_GREYOUT && 
        play.inventory_greys_out == 1)
    {
        // darken the inventory when disabled
        GUI::DrawDisabledEffect(ds, RectWH(X, Y, Width, Height));
    }
}

} // namespace Common
} // namespace AGS
