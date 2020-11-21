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

#ifndef __AC_GUIBUTTON_H
#define __AC_GUIBUTTON_H

#include <vector>
#include "gui/guiobject.h"
#include "util/string.h"

#define GUIBUTTON_LEGACY_TEXTLENGTH 50

namespace AGS
{
namespace Common
{

enum MouseButton
{
    kMouseNone  = -1,
    kMouseLeft  =  0,
    kMouseRight =  1,
};

enum GUIClickAction
{
    kGUIAction_None       = 0,
    kGUIAction_SetMode    = 1,
    kGUIAction_RunScript  = 2,
};

enum LegacyButtonAlignment
{
    kLegacyButtonAlign_TopCenter = 0,
    kLegacyButtonAlign_TopLeft = 1,
    kLegacyButtonAlign_TopRight = 2,
    kLegacyButtonAlign_CenterLeft = 3,
    kLegacyButtonAlign_Centered = 4,
    kLegacyButtonAlign_CenterRight = 5,
    kLegacyButtonAlign_BottomLeft = 6,
    kLegacyButtonAlign_BottomCenter = 7,
    kLegacyButtonAlign_BottomRight = 8,
};

class GUIButton : public GUIObject
{
public:
    GUIButton();

    const String &GetText() const;
    bool IsClippingImage() const;

    // Operations
    void Draw(Bitmap *ds) override;
    void SetClipImage(bool on);
    void SetText(const String &text);

    // Events
    bool OnMouseDown() override;
    void OnMouseEnter() override;
    void OnMouseLeave() override;
    void OnMouseUp() override;
  
    // Serialization
    void ReadFromFile(Stream *in, GuiVersion gui_version) override;
    void WriteToFile(Stream *out) const override;
    void ReadFromSavegame(Common::Stream *in, GuiSvgVersion svg_ver) override;
    void WriteToSavegame(Common::Stream *out) const override;

// TODO: these members are currently public; hide them later
public:
    int32_t     Image;
    int32_t     MouseOverImage;
    int32_t     PushedImage;
    int32_t     CurrentImage;
    int32_t     Font;
    color_t     TextColor;
    FrameAlignment TextAlignment;
    // Click actions for left and right mouse buttons
    // NOTE: only left click is currently in use
    static const int ClickCount = kMouseRight + 1;
    GUIClickAction ClickAction[ClickCount];
    int32_t        ClickData[ClickCount];

    bool        IsPushed;
    bool        IsMouseOver;

private:
    void DrawImageButton(Bitmap *ds, bool draw_disabled);
    void DrawText(Bitmap *ds, bool draw_disabled);
    void DrawTextButton(Bitmap *ds, bool draw_disabled);
    void PrepareTextToDraw();

    // Defines button placeholder mode; the mode is set
    // depending on special tags found in button text
    enum GUIButtonPlaceholder
    {
        kButtonPlace_None,
        kButtonPlace_InvItemStretch,
        kButtonPlace_InvItemCenter,
        kButtonPlace_InvItemAuto
    };

    // Text property set by user
    String _text;
    // type of content placeholder, if any
    GUIButtonPlaceholder _placeholder;
    // A flag indicating unnamed button; this is a convenience trick:
    // buttons are created named "New Button" in the editor, and users
    // often do not clear text when they want a graphic button.
    bool _unnamed;
    // Prepared text buffer/cache
    String _textToDraw;
};

} // namespace Common
} // namespace AGS

extern std::vector<AGS::Common::GUIButton> guibuts;
extern int numguibuts;

int UpdateAnimatingButton(int bu);
void StopButtonAnimation(int idxn);

#endif // __AC_GUIBUTTON_H
