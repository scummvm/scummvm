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

#include <algorithm>
#include "ac/game_version.h"
#include "ac/spritecache.h"
#include "debug/out.h"
#include "font/fonts.h"
#include "gui/guibutton.h"
#include "gui/guiinv.h"
#include "gui/guilabel.h"
#include "gui/guilistbox.h"
#include "gui/guimain.h"
#include "gui/guislider.h"
#include "gui/guitextbox.h"
#include "util/stream.h"
#include "util/string_utils.h"

using namespace AGS::Common;

#define MOVER_MOUSEDOWNLOCKED -4000

int guis_need_update = 1;
int all_buttons_disabled = 0, gui_inv_pic = -1;
int gui_disabled_style = 0;

namespace AGS
{
namespace Common
{

/* static */ String GUIMain::FixupGUIName(const String &name)
{
    if (name.GetLength() > 0 && name[0u] != 'g')
        return String::FromFormat("g%c%s", name[0u], name.Mid(1).Lower().GetCStr());
    return name;
}

GUIMain::GUIMain()
{
    InitDefaults();
}

void GUIMain::InitDefaults()
{
    ID            = 0;
    Name.Empty();
    _flags        = kGUIMain_DefFlags;

    X             = 0;
    Y             = 0;
    Width         = 0;
    Height        = 0;
    BgColor       = 8;
    BgImage       = 0;
    FgColor       = 1;
    Padding       = TEXTWINDOW_PADDING_DEFAULT;
    PopupStyle    = kGUIPopupNormal;
    PopupAtMouseY = -1;
    Transparency  = 0;
    ZOrder        = -1;

    FocusCtrl     = 0;
    HighlightCtrl = -1;
    MouseOverCtrl = -1;
    MouseDownCtrl = -1;
    MouseWasAt.X  = -1;
    MouseWasAt.Y  = -1;

    OnClickHandler.Empty();

    _controls.clear();
    _ctrlRefs.clear();
    _ctrlDrawOrder.clear();
}

int GUIMain::FindControlUnderMouse(int leeway, bool must_be_clickable) const
{
    if (loaded_game_file_version <= kGameVersion_262)
    {
        // Ignore draw order On 2.6.2 and lower
        for (size_t i = 0; i < _controls.size(); ++i)
        {
            if (!_controls[i]->IsVisible())
                continue;
            if (!_controls[i]->IsClickable() && must_be_clickable)
                continue;
            if (_controls[i]->IsOverControl(mousex, mousey, leeway))
                return i;
        }
    }
    else
    {
        for (size_t i = _controls.size(); i-- > 0;)
        {
            const int ctrl_index = _ctrlDrawOrder[i];
            if (!_controls[ctrl_index]->IsVisible())
                continue;
            if (!_controls[ctrl_index]->IsClickable() && must_be_clickable)
                continue;
            if (_controls[ctrl_index]->IsOverControl(mousex, mousey, leeway))
                return ctrl_index;
        }
    }
    return -1;
}

int GUIMain::FindControlUnderMouse() const
{
    return FindControlUnderMouse(0, true);
}

int GUIMain::FindControlUnderMouse(int leeway) const
{
    return FindControlUnderMouse(leeway, true);
}

int GUIMain::GetControlCount() const
{
    return (int32_t)_controls.size();
}

GUIObject *GUIMain::GetControl(int index) const
{
    if (index < 0 || (size_t)index >= _controls.size())
        return nullptr;
    return _controls[index];
}

GUIControlType GUIMain::GetControlType(int index) const
{
    if (index < 0 || (size_t)index >= _ctrlRefs.size())
        return kGUIControlUndefined;
    return _ctrlRefs[index].first;
}

int32_t GUIMain::GetControlID(int index) const
{
    if (index < 0 || (size_t)index >= _ctrlRefs.size())
        return -1;
    return _ctrlRefs[index].second;
}

bool GUIMain::IsClickable() const
{
    return (_flags & kGUIMain_Clickable) != 0;
}

bool GUIMain::IsConcealed() const
{
    return (_flags & kGUIMain_Concealed) != 0;
}

bool GUIMain::IsDisplayed() const
{
    if(!IsVisible()) return false;
    if(IsConcealed()) return false;
    if(Transparency == 255) return false;
    return true;
}

bool GUIMain::IsInteractableAt(int x, int y) const
{
    if (!IsDisplayed())
        return false;
    if (!IsClickable())
        return false;
    if ((x >= X) & (y >= Y) & (x < X + Width) & (y < Y + Height))
        return true;
    return false;
}

bool GUIMain::IsTextWindow() const
{
    return (_flags & kGUIMain_TextWindow) != 0;
}

bool GUIMain::IsVisible() const
{
    return (_flags & kGUIMain_Visible) != 0;
}

void GUIMain::AddControl(GUIControlType type, int id, GUIObject *control)
{
    _ctrlRefs.push_back(std::make_pair(type, id));
    _controls.push_back(control);
}

void GUIMain::RemoveAllControls()
{
    _ctrlRefs.clear();
    _controls.clear();
}

bool GUIMain::BringControlToFront(int index)
{
    return SetControlZOrder(index, (int)_controls.size() - 1);
}

void GUIMain::Draw(Bitmap *ds)
{
    DrawAt(ds, X, Y);
}

void GUIMain::DrawAt(Bitmap *ds, int x, int y)
{
    SET_EIP(375)

    if ((Width < 1) || (Height < 1))
        return;

    Bitmap subbmp;
    subbmp.CreateSubBitmap(ds, RectWH(x, y, Width, Height));

    SET_EIP(376)
    // stop border being transparent, if the whole GUI isn't
    if ((FgColor == 0) && (BgColor != 0))
        FgColor = 16;

    if (BgColor != 0)
        subbmp.Fill(subbmp.GetCompatibleColor(BgColor));

    SET_EIP(377)

    color_t draw_color;
    if (FgColor != BgColor)
    {
        draw_color = subbmp.GetCompatibleColor(FgColor);
        subbmp.DrawRect(Rect(0, 0, subbmp.GetWidth() - 1, subbmp.GetHeight() - 1), draw_color);
        if (get_fixed_pixel_size(1) > 1)
            subbmp.DrawRect(Rect(1, 1, subbmp.GetWidth() - 2, subbmp.GetHeight() - 2), draw_color);
    }

    SET_EIP(378)

    if (BgImage > 0 && spriteset[BgImage] != nullptr)
        draw_gui_sprite(&subbmp, BgImage, 0, 0, false);

    SET_EIP(379)

    if (all_buttons_disabled && gui_disabled_style == GUIDIS_BLACKOUT)
        return; // don't draw GUI controls

    for (size_t ctrl_index = 0; ctrl_index < _controls.size(); ++ctrl_index)
    {
        set_eip_guiobj(_ctrlDrawOrder[ctrl_index]);

        GUIObject *objToDraw = _controls[_ctrlDrawOrder[ctrl_index]];

        if (!objToDraw->IsEnabled() && gui_disabled_style == GUIDIS_BLACKOUT)
            continue;
        if (!objToDraw->IsVisible())
            continue;

        objToDraw->Draw(&subbmp);

        int selectedColour = 14;

        if (HighlightCtrl == _ctrlDrawOrder[ctrl_index])
        {
            if (outlineGuiObjects)
                selectedColour = 13;
            draw_color = subbmp.GetCompatibleColor(selectedColour);
            DrawBlob(&subbmp, objToDraw->X + objToDraw->Width - get_fixed_pixel_size(1) - 1, objToDraw->Y, draw_color);
            DrawBlob(&subbmp, objToDraw->X, objToDraw->Y + objToDraw->Height - get_fixed_pixel_size(1) - 1, draw_color);
            DrawBlob(&subbmp, objToDraw->X, objToDraw->Y, draw_color);
            DrawBlob(&subbmp, objToDraw->X + objToDraw->Width - get_fixed_pixel_size(1) - 1, 
                    objToDraw->Y + objToDraw->Height - get_fixed_pixel_size(1) - 1, draw_color);
        }
        if (outlineGuiObjects)
        {
            // draw a dotted outline round all objects
            draw_color = subbmp.GetCompatibleColor(selectedColour);
            for (int i = 0; i < objToDraw->Width; i += 2)
            {
                subbmp.PutPixel(i + objToDraw->X, objToDraw->Y, draw_color);
                subbmp.PutPixel(i + objToDraw->X, objToDraw->Y + objToDraw->Height - 1, draw_color);
            }
            for (int i = 0; i < objToDraw->Height; i += 2)
            {
                subbmp.PutPixel(objToDraw->X, i + objToDraw->Y, draw_color);
                subbmp.PutPixel(objToDraw->X + objToDraw->Width - 1, i + objToDraw->Y, draw_color);
            }
        }
    }

    SET_EIP(380)
}

void GUIMain::DrawBlob(Bitmap *ds, int x, int y, color_t draw_color)
{
    ds->FillRect(Rect(x, y, x + get_fixed_pixel_size(1), y + get_fixed_pixel_size(1)), draw_color);
}

void GUIMain::Poll()
{
    int mxwas = mousex, mywas = mousey;

    mousex -= X;
    mousey -= Y;
    if (mousex != MouseWasAt.X || mousey != MouseWasAt.Y)
    {
        int ctrl_index = FindControlUnderMouse();

        if (MouseOverCtrl == MOVER_MOUSEDOWNLOCKED)
            _controls[MouseDownCtrl]->OnMouseMove(mousex, mousey);
        else if (ctrl_index != MouseOverCtrl)
        {
            if (MouseOverCtrl >= 0)
                _controls[MouseOverCtrl]->OnMouseLeave();

            if (ctrl_index >= 0 && !IsGUIEnabled(_controls[ctrl_index]))
                // the control is disabled - ignore it
                MouseOverCtrl = -1;
            else if (ctrl_index >= 0 && !_controls[ctrl_index]->IsClickable())
                // the control is not clickable - ignore it
                MouseOverCtrl = -1;
            else
            {
                // over a different control
                MouseOverCtrl = ctrl_index;
                if (MouseOverCtrl >= 0)
                {
                    _controls[MouseOverCtrl]->OnMouseEnter();
                    _controls[MouseOverCtrl]->OnMouseMove(mousex, mousey);
                }
            }
            guis_need_update = 1;
        } 
        else if (MouseOverCtrl >= 0)
            _controls[MouseOverCtrl]->OnMouseMove(mousex, mousey);
    }

    MouseWasAt.X = mousex;
    MouseWasAt.Y = mousey;
    mousex = mxwas;
    mousey = mywas;
}

HError GUIMain::RebuildArray()
{
    GUIControlType thistype;
    int32_t thisnum;

    _controls.resize(_ctrlRefs.size());
    for (size_t i = 0; i < _controls.size(); ++i)
    {
        thistype = _ctrlRefs[i].first;
        thisnum = _ctrlRefs[i].second;

        if (thisnum < 0)
            return new Error(String::FromFormat("GUIMain (%d): invalid control ID %d in ref #%d", ID, thisnum, i));

        if (thistype == kGUIButton)
            _controls[i] = &guibuts[thisnum];
        else if (thistype == kGUILabel)
            _controls[i] = &guilabels[thisnum];
        else if (thistype == kGUIInvWindow)
            _controls[i] = &guiinv[thisnum];
        else if (thistype == kGUISlider)
            _controls[i] = &guislider[thisnum];
        else if (thistype == kGUITextBox)
            _controls[i] = &guitext[thisnum];
        else if (thistype == kGUIListBox)
            _controls[i] = &guilist[thisnum];
        else
            return new Error(String::FromFormat("GUIMain (%d): unknown control type %d in ref #%d", ID, thistype, i));

        _controls[i]->ParentId = ID;
        _controls[i]->Id = i;
    }

    ResortZOrder();
    return HError::None();
}

bool GUIControlZOrder(const GUIObject *e1, const GUIObject *e2)
{
    return e1->ZOrder < e2->ZOrder;
}

void GUIMain::ResortZOrder()
{
    std::vector<GUIObject*> ctrl_sort = _controls;
    std::sort(ctrl_sort.begin(), ctrl_sort.end(), GUIControlZOrder);

    _ctrlDrawOrder.resize(ctrl_sort.size());
    for (size_t i = 0; i < ctrl_sort.size(); ++i)
        _ctrlDrawOrder[i] = ctrl_sort[i]->Id;
}

void GUIMain::SetClickable(bool on)
{
    if (on)
        _flags |= kGUIMain_Clickable;
    else
        _flags &= ~kGUIMain_Clickable;
}

void GUIMain::SetConceal(bool on)
{
    if (on)
        _flags |= kGUIMain_Concealed;
    else
        _flags &= ~kGUIMain_Concealed;
}

bool GUIMain::SendControlToBack(int index)
{
    return SetControlZOrder(index, 0);
}

bool GUIMain::SetControlZOrder(int index, int zorder)
{
    if (index < 0 || (size_t)index >= _controls.size())
        return false; // no such control

    zorder = Math::Clamp(zorder, 0, (int)_controls.size() - 1);
    const int old_zorder = _controls[index]->ZOrder;
    if (old_zorder == zorder)
        return false; // no change

    const bool move_back = zorder < old_zorder; // back is at zero index
    const int  left      = move_back ? zorder : old_zorder;
    const int  right     = move_back ? old_zorder : zorder;
    for (size_t i = 0; i < _controls.size(); ++i)
    {
        const int i_zorder = _controls[i]->ZOrder;
        if (i_zorder == old_zorder)
            _controls[i]->ZOrder = zorder; // the control we are moving
        else if (i_zorder >= left && i_zorder <= right)
        {
            // controls in between old and new positions shift towards free place
            if (move_back)
                _controls[i]->ZOrder++; // move to front
            else
                _controls[i]->ZOrder--; // move to back
        }
    }
    ResortZOrder();
    OnControlPositionChanged();
    return true;
}

void GUIMain::SetTextWindow(bool on)
{
    if (on)
        _flags |= kGUIMain_TextWindow;
    else
        _flags &= ~kGUIMain_TextWindow;
}

void GUIMain::SetTransparencyAsPercentage(int percent)
{
    Transparency = GfxDef::Trans100ToLegacyTrans255(percent);
    guis_need_update = 1;
}

void GUIMain::SetVisible(bool on)
{
    if (on)
        _flags |= kGUIMain_Visible;
    else
        _flags &= ~kGUIMain_Visible;
}

void GUIMain::OnControlPositionChanged()
{
    // force it to re-check for which control is under the mouse
    MouseWasAt.X = -1;
    MouseWasAt.Y = -1;
}

void GUIMain::OnMouseButtonDown()
{
    if (MouseOverCtrl < 0)
        return;

    // don't activate disabled buttons
    if (!IsGUIEnabled(_controls[MouseOverCtrl]) || !_controls[MouseOverCtrl]->IsVisible() ||
        !_controls[MouseOverCtrl]->IsClickable())
    return;

    MouseDownCtrl = MouseOverCtrl;
    if (_controls[MouseOverCtrl]->OnMouseDown())
        MouseOverCtrl = MOVER_MOUSEDOWNLOCKED;
    _controls[MouseDownCtrl]->OnMouseMove(mousex - X, mousey - Y);
    guis_need_update = 1;
}

void GUIMain::OnMouseButtonUp()
{
    // FocusCtrl was locked - reset it back to normal, but On the
    // locked object so that a OnMouseLeave gets fired if necessary
    if (MouseOverCtrl == MOVER_MOUSEDOWNLOCKED)
    {
        MouseOverCtrl = MouseDownCtrl;
        MouseWasAt.X = -1;  // force update
    }

    if (MouseDownCtrl < 0)
        return;

    _controls[MouseDownCtrl]->OnMouseUp();
    MouseDownCtrl = -1;
    guis_need_update = 1;
}

void GUIMain::ReadFromFile(Stream *in, GuiVersion gui_version)
{
    // Legacy text window tag
    char tw_flags[GUIMAIN_LEGACY_TW_FLAGS_SIZE] = {0};
    if (gui_version < kGuiVersion_350)
        in->Read(tw_flags, sizeof(tw_flags));

    if (gui_version < kGuiVersion_340)
    {
        Name.ReadCount(in, GUIMAIN_LEGACY_NAME_LENGTH);
        OnClickHandler.ReadCount(in, GUIMAIN_LEGACY_EVENTHANDLER_LENGTH);
    }
    else
    {
        Name = StrUtil::ReadString(in);
        OnClickHandler = StrUtil::ReadString(in);
    }
    X             = in->ReadInt32();
    Y             = in->ReadInt32();
    Width         = in->ReadInt32();
    Height        = in->ReadInt32();
    if (gui_version < kGuiVersion_350)
    { // NOTE: reading into actual variables only for old savegame support
        FocusCtrl = in->ReadInt32();
    }
    const size_t ctrl_count = in->ReadInt32();
    PopupStyle    = (GUIPopupStyle)in->ReadInt32();
    PopupAtMouseY = in->ReadInt32();
    BgColor       = in->ReadInt32();
    BgImage       = in->ReadInt32();
    FgColor       = in->ReadInt32();
    if (gui_version < kGuiVersion_350)
    { // NOTE: reading into actual variables only for old savegame support
        MouseOverCtrl = in->ReadInt32();
        MouseWasAt.X  = in->ReadInt32();
        MouseWasAt.Y  = in->ReadInt32();
        MouseDownCtrl = in->ReadInt32();
        HighlightCtrl = in->ReadInt32();
    }
    _flags         = in->ReadInt32();
    Transparency  = in->ReadInt32();
    ZOrder        = in->ReadInt32();
    ID            = in->ReadInt32();
    Padding       = in->ReadInt32();
    if (gui_version < kGuiVersion_350)
        in->Seek(sizeof(int32_t) * GUIMAIN_LEGACY_RESERVED_INTS);

    if (gui_version < kGuiVersion_350)
    {
        if (tw_flags[0] == kGUIMain_LegacyTextWindow)
            _flags |= kGUIMain_TextWindow;
        // reverse particular flags from older format
        _flags ^= kGUIMain_OldFmtXorMask;
        GUI::ApplyLegacyVisibility(*this, (LegacyGUIVisState)in->ReadInt32());
    }

    // pre-3.4.0 games contained array of 32-bit pointers; these values are unused
    // TODO: error if ctrl_count > LEGACY_MAX_OBJS_ON_GUI
    if (gui_version < kGuiVersion_340)
        in->Seek(LEGACY_MAX_OBJS_ON_GUI * sizeof(int32_t));
    if (ctrl_count > 0)
    {
        _ctrlRefs.resize(ctrl_count);
        for (size_t i = 0; i < ctrl_count; ++i)
        {
            const int32_t ref_packed = in->ReadInt32();
            _ctrlRefs[i].first = (GUIControlType)((ref_packed >> 16) & 0xFFFF);
            _ctrlRefs[i].second = ref_packed & 0xFFFF;
        }
    }
    // Skip unused control slots in pre-3.4.0 games
    if (gui_version < kGuiVersion_340 && ctrl_count < LEGACY_MAX_OBJS_ON_GUI)
        in->Seek((LEGACY_MAX_OBJS_ON_GUI - ctrl_count) * sizeof(int32_t));
}

void GUIMain::WriteToFile(Stream *out) const
{
    StrUtil::WriteString(Name, out);
    StrUtil::WriteString(OnClickHandler, out);
    out->WriteInt32(X);
    out->WriteInt32(Y);
    out->WriteInt32(Width);
    out->WriteInt32(Height);
    out->WriteInt32(_ctrlRefs.size());
    out->WriteInt32(PopupStyle);
    out->WriteInt32(PopupAtMouseY);
    out->WriteInt32(BgColor);
    out->WriteInt32(BgImage);
    out->WriteInt32(FgColor);
    out->WriteInt32(_flags);
    out->WriteInt32(Transparency);
    out->WriteInt32(ZOrder);
    out->WriteInt32(ID);
    out->WriteInt32(Padding);
    for (size_t i = 0; i < _ctrlRefs.size(); ++i)
    {
        int32_t ref_packed = ((_ctrlRefs[i].first & 0xFFFF) << 16) | (_ctrlRefs[i].second & 0xFFFF);
        out->WriteInt32(ref_packed);
    }
}

void GUIMain::ReadFromSavegame(Common::Stream *in, GuiSvgVersion svg_version)
{
    // Properties
    _flags = in->ReadInt32();
    X = in->ReadInt32();
    Y = in->ReadInt32();
    Width = in->ReadInt32();
    Height = in->ReadInt32();
    BgImage = in->ReadInt32();
    Transparency = in->ReadInt32();
    if (svg_version < kGuiSvgVersion_350)
    {
        // reverse particular flags from older format
        _flags ^= kGUIMain_OldFmtXorMask;
        GUI::ApplyLegacyVisibility(*this, (LegacyGUIVisState)in->ReadInt32());
    }
    ZOrder = in->ReadInt32();

    if (svg_version >= kGuiSvgVersion_350)
    {
        BgColor = in->ReadInt32();
        FgColor = in->ReadInt32();
        Padding = in->ReadInt32();
        PopupAtMouseY = in->ReadInt32();
    }

    // Dynamic values
    FocusCtrl = in->ReadInt32();
    HighlightCtrl = in->ReadInt32();
    MouseOverCtrl = in->ReadInt32();
    MouseDownCtrl = in->ReadInt32();
    MouseWasAt.X = in->ReadInt32();
    MouseWasAt.Y = in->ReadInt32();
}

void GUIMain::WriteToSavegame(Common::Stream *out) const
{
    // Properties
    out->WriteInt32(_flags);
    out->WriteInt32(X);
    out->WriteInt32(Y);
    out->WriteInt32(Width);
    out->WriteInt32(Height);
    out->WriteInt32(BgImage);
    out->WriteInt32(Transparency);
    out->WriteInt32(ZOrder);
    out->WriteInt32(BgColor);
    out->WriteInt32(FgColor);
    out->WriteInt32(Padding);
    out->WriteInt32(PopupAtMouseY);
    // Dynamic values
    out->WriteInt32(FocusCtrl);
    out->WriteInt32(HighlightCtrl);
    out->WriteInt32(MouseOverCtrl);
    out->WriteInt32(MouseDownCtrl);
    out->WriteInt32(MouseWasAt.X);
    out->WriteInt32(MouseWasAt.Y);
}


namespace GUI
{

GuiVersion GameGuiVersion = kGuiVersion_Initial;

void DrawDisabledEffect(Bitmap *ds, const Rect &rc)
{
    color_t draw_color = ds->GetCompatibleColor(8);
    for (int at_x = rc.Left; at_x <= rc.Right; ++at_x)
    {
        for (int at_y = rc.Top + at_x % 2; at_y <= rc.Bottom; at_y += 2)
        {
            ds->PutPixel(at_x, at_y, draw_color);
        }
    }
}

void DrawTextAligned(Bitmap *ds, const char *text, int font, color_t text_color, const Rect &frame, FrameAlignment align)
{
    int text_height = wgettextheight(text, font);
    if (align & kMAlignVCenter)
        text_height++; // CHECKME
    Rect item = AlignInRect(frame, RectWH(0, 0, wgettextwidth(text, font), text_height), align);
    wouttext_outline(ds, item.Left, item.Top, font, text_color, text);
}

void DrawTextAlignedHor(Bitmap *ds, const char *text, int font, color_t text_color, int x1, int x2, int y, FrameAlignment align)
{
    int x = AlignInHRange(x1, x2, 0, wgettextwidth(text, font), align);
    wouttext_outline(ds, x, y, font, text_color, text);
}

HError ResortGUI(std::vector<GUIMain> &guis, bool bwcompat_ctrl_zorder = false)
{
    // set up the reverse-lookup array
    for (size_t gui_index = 0; gui_index < guis.size(); ++gui_index)
    {
        GUIMain &gui = guis[gui_index];
        HError err = gui.RebuildArray();
        if (!err)
            return err;
        for (int ctrl_index = 0; ctrl_index < gui.GetControlCount(); ++ctrl_index)
        {
            GUIObject *gui_ctrl = gui.GetControl(ctrl_index);
            gui_ctrl->ParentId = gui_index;
            gui_ctrl->Id = ctrl_index;
            if (bwcompat_ctrl_zorder)
                gui_ctrl->ZOrder = ctrl_index;
        }
        gui.ResortZOrder();
    }
    guis_need_update = 1;
    return HError::None();
}

HError ReadGUI(std::vector<GUIMain> &guis, Stream *in, bool is_savegame)
{
    if (in->ReadInt32() != (int)GUIMAGIC)
        return new Error("ReadGUI: unknown format or file is corrupt");

    GameGuiVersion = (GuiVersion)in->ReadInt32();
    Debug::Printf(kDbgMsg_Info, "Game GUI version: %d", GameGuiVersion);
    size_t gui_count;
    if (GameGuiVersion < kGuiVersion_214)
    {
        gui_count = (size_t)GameGuiVersion;
        GameGuiVersion = kGuiVersion_Initial;
    }
    else if (GameGuiVersion > kGuiVersion_Current)
        return new Error(String::FromFormat("ReadGUI: format version not supported (required %d, supported %d - %d)",
            GameGuiVersion, kGuiVersion_Initial, kGuiVersion_Current));
    else
        gui_count = in->ReadInt32();
    guis.resize(gui_count);

    // import the main GUI elements
    for (size_t i = 0; i < gui_count; ++i)
    {
        GUIMain &gui = guis[i];
        gui.InitDefaults();
        gui.ReadFromFile(in, GameGuiVersion);

        // perform fixups
        if (gui.Height < 2)
            gui.Height = 2;
        if (GameGuiVersion < kGuiVersion_unkn_103)
            gui.Name.Format("GUI%d", i);
        if (GameGuiVersion < kGuiVersion_260)
            gui.ZOrder = i;
        if (GameGuiVersion < kGuiVersion_270)
            gui.OnClickHandler.Empty();
        if (GameGuiVersion < kGuiVersion_331)
            gui.Padding = TEXTWINDOW_PADDING_DEFAULT;
        // fix names for 2.x: "GUI" -> "gGui"
        if (loaded_game_file_version <= kGameVersion_272)
            gui.Name = GUIMain::FixupGUIName(gui.Name);

        // GUI popup style and visibility
        if (GameGuiVersion < kGuiVersion_350 && !is_savegame)
        {
            // Convert legacy normal-off style into normal one
            if (gui.PopupStyle == kGUIPopupLegacyNormalOff)
            {
                gui.PopupStyle = kGUIPopupNormal;
                gui.SetVisible(false);
            }
            // Normal GUIs and PopupMouseY GUIs should start with Visible = true
            else
            {
                gui.SetVisible(gui.PopupStyle != kGUIPopupModal);
            }
        }

        // PopupMouseY GUIs should be initially concealed
        if (gui.PopupStyle == kGUIPopupMouseY)
            gui.SetConceal(true);
        // Assign ID to order in array
        gui.ID = i;
    }

    // buttons
    numguibuts = in->ReadInt32();
    guibuts.resize(numguibuts);
    for (int i = 0; i < numguibuts; ++i)
    {
        guibuts[i].ReadFromFile(in, GameGuiVersion);
    }
    // labels
    numguilabels = in->ReadInt32();
    guilabels.resize(numguilabels);
    for (int i = 0; i < numguilabels; ++i)
    {
        guilabels[i].ReadFromFile(in, GameGuiVersion);
    }
    // inv controls
    numguiinv = in->ReadInt32();
    guiinv.resize(numguiinv);
    for (int i = 0; i < numguiinv; ++i)
    {
        guiinv[i].ReadFromFile(in, GameGuiVersion);
    }

    if (GameGuiVersion >= kGuiVersion_214)
    {
        // sliders
        numguislider = in->ReadInt32();
        guislider.resize(numguislider);
        for (int i = 0; i < numguislider; ++i)
        {
            guislider[i].ReadFromFile(in, GameGuiVersion);
        }
    }
    if (GameGuiVersion >= kGuiVersion_222)
    {
        // text boxes
        numguitext = in->ReadInt32();
        guitext.resize(numguitext);
        for (int i = 0; i < numguitext; ++i)
        {
            guitext[i].ReadFromFile(in, GameGuiVersion);
        }
    }
    if (GameGuiVersion >= kGuiVersion_230)
    {
        // list boxes
        numguilist = in->ReadInt32();
        guilist.resize(numguilist);
        for (int i = 0; i < numguilist; ++i)
        {
            guilist[i].ReadFromFile(in, GameGuiVersion);
        }
    }
    return ResortGUI(guis, GameGuiVersion < kGuiVersion_272e);
}

void WriteGUI(const std::vector<GUIMain> &guis, Stream *out)
{
    out->WriteInt32(GUIMAGIC);
    out->WriteInt32(kGuiVersion_Current);
    out->WriteInt32(guis.size());

    for (size_t i = 0; i < guis.size(); ++i)
    {
        guis[i].WriteToFile(out);
    }
    out->WriteInt32(numguibuts);
    for (int i = 0; i < numguibuts; ++i)
    {
        guibuts[i].WriteToFile(out);
    }
    out->WriteInt32(numguilabels);
    for (int i = 0; i < numguilabels; ++i)
    {
        guilabels[i].WriteToFile(out);
    }
    out->WriteInt32(numguiinv);
    for (int i = 0; i < numguiinv; ++i)
    {
        guiinv[i].WriteToFile(out);
    }
    out->WriteInt32(numguislider);
    for (int i = 0; i < numguislider; ++i)
    {
        guislider[i].WriteToFile(out);
    }
    out->WriteInt32(numguitext);
    for (int i = 0; i < numguitext; ++i)
    {
        guitext[i].WriteToFile(out);
    }
    out->WriteInt32(numguilist);
    for (int i = 0; i < numguilist; ++i)
    {
        guilist[i].WriteToFile(out);
    }
}

void ApplyLegacyVisibility(GUIMain &gui, LegacyGUIVisState vis)
{
    // kGUIPopupMouseY had its own rules, which we practically reverted now
    if (gui.PopupStyle == kGUIPopupMouseY)
    {
        // it was only !Visible if the legacy Visibility was Concealed
        gui.SetVisible(vis != kGUIVisibility_Concealed);
        // and you could tell it's overridden by behavior when legacy Visibility is Off
        gui.SetConceal(vis == kGUIVisibility_Off);
    }
    // Other GUI styles were simple
    else
    {
        gui.SetVisible(vis != kGUIVisibility_Off);
        gui.SetConceal(false);
    }
}

} // namespace GUI

} // namespace Common
} // namespace AGS
