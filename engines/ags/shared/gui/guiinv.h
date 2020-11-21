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

#ifndef __AC_GUIINV_H
#define __AC_GUIINV_H

#include <vector>
#include "gui/guiobject.h"

namespace AGS
{
namespace Common
{

class GUIInvWindow : public GUIObject
{
public:
    GUIInvWindow();

    // This function has distinct implementations in Engine and Editor
    int GetCharacterId() const;

    // Operations
    // This function has distinct implementations in Engine and Editor
    void Draw(Bitmap *ds) override;

    // Events
    void OnMouseEnter() override;
    void OnMouseLeave() override;
    void OnMouseUp() override;
    void OnResized() override;

    // Serialization
    void ReadFromFile(Stream *in, GuiVersion gui_version) override;
    void WriteToFile(Stream *out) const override;
    void ReadFromSavegame(Common::Stream *in, GuiSvgVersion svg_ver) override;
    void WriteToSavegame(Common::Stream *out) const override;

// TODO: these members are currently public; hide them later
public:
    bool    IsMouseOver;
    int32_t CharId; // whose inventory (-1 = current player)
    int32_t ItemWidth;
    int32_t ItemHeight;
    int32_t ColCount;
    int32_t RowCount;
    int32_t TopItem;

private:
    void CalculateNumCells();
};

} // namespace Common
} // namespace AGS

extern std::vector<AGS::Common::GUIInvWindow> guiinv;
extern int numguiinv;

#endif // __AC_GUIINV_H
