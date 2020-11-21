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

#ifndef __AC_GUITEXTBOX_H
#define __AC_GUITEXTBOX_H

#include <vector>
#include "gui/guiobject.h"
#include "util/string.h"

namespace AGS
{
namespace Common
{

class GUITextBox : public GUIObject
{
public:
    GUITextBox();

    bool IsBorderShown() const;

    // Operations
    void Draw(Bitmap *ds) override;
    void SetShowBorder(bool on);
 
    // Events
    void OnKeyPress(int keycode) override;
 
    // Serialization
    void ReadFromFile(Stream *in, GuiVersion gui_version) override;
    void WriteToFile(Stream *out) const override;
    void ReadFromSavegame(Stream *in, GuiSvgVersion svg_ver) override;
    void WriteToSavegame(Stream *out) const override;
 
// TODO: these members are currently public; hide them later
public:
    int32_t Font;
    String  Text;
    color_t TextColor;

private:
    int32_t TextBoxFlags;

    void DrawTextBoxContents(Bitmap *ds, color_t text_color);
};

} // namespace Common
} // namespace AGS

extern std::vector<AGS::Common::GUITextBox> guitext;
extern int numguitext;

#endif // __AC_GUITEXTBOX_H
