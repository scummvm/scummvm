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

#ifndef AGS_ENGINE_AC_GUICONTROL_H
#define AGS_ENGINE_AC_GUICONTROL_H

#include "ags/shared/gui/gui_object.h"
#include "ags/shared/gui/gui_button.h"
#include "ags/shared/gui/gui_inv.h"
#include "ags/shared/gui/gui_label.h"
#include "ags/shared/gui/gui_listbox.h"
#include "ags/shared/gui/gui_slider.h"
#include "ags/shared/gui/gui_textbox.h"
#include "ags/engine/ac/dynobj/script_gui.h"

namespace AGS3 {

using AGS::Shared::GUIObject;
using AGS::Shared::GUIButton;
using AGS::Shared::GUIInvWindow;
using AGS::Shared::GUILabel;
using AGS::Shared::GUIListBox;
using AGS::Shared::GUISlider;
using AGS::Shared::GUITextBox;

GUIObject *GetGUIControlAtLocation(int xx, int yy);
int         GUIControl_GetVisible(GUIObject *guio);
void        GUIControl_SetVisible(GUIObject *guio, int visible);
int         GUIControl_GetClickable(GUIObject *guio);
void        GUIControl_SetClickable(GUIObject *guio, int enabled);
int         GUIControl_GetEnabled(GUIObject *guio);
void        GUIControl_SetEnabled(GUIObject *guio, int enabled);
int         GUIControl_GetID(GUIObject *guio);
ScriptGUI *GUIControl_GetOwningGUI(GUIObject *guio);
GUIButton *GUIControl_GetAsButton(GUIObject *guio);
GUIInvWindow *GUIControl_GetAsInvWindow(GUIObject *guio);
GUILabel *GUIControl_GetAsLabel(GUIObject *guio);
GUIListBox *GUIControl_GetAsListBox(GUIObject *guio);
GUISlider *GUIControl_GetAsSlider(GUIObject *guio);
GUITextBox *GUIControl_GetAsTextBox(GUIObject *guio);
int         GUIControl_GetX(GUIObject *guio);
void        GUIControl_SetX(GUIObject *guio, int xx);
int         GUIControl_GetY(GUIObject *guio);
void        GUIControl_SetY(GUIObject *guio, int yy);
int         GUIControl_GetZOrder(GUIObject *guio);
void        GUIControl_SetZOrder(GUIObject *guio, int zorder);
void        GUIControl_SetPosition(GUIObject *guio, int xx, int yy);
int         GUIControl_GetWidth(GUIObject *guio);
void        GUIControl_SetWidth(GUIObject *guio, int newwid);
int         GUIControl_GetHeight(GUIObject *guio);
void        GUIControl_SetHeight(GUIObject *guio, int newhit);
void        GUIControl_SetSize(GUIObject *guio, int newwid, int newhit);
void        GUIControl_SendToBack(GUIObject *guio);
void        GUIControl_BringToFront(GUIObject *guio);

} // namespace AGS3

#endif
