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
#ifndef __AGS_EE_AC__GUICONTROL_H
#define __AGS_EE_AC__GUICONTROL_H

#include "gui/guiobject.h"
#include "gui/guibutton.h"
#include "gui/guiinv.h"
#include "gui/guilabel.h"
#include "gui/guilistbox.h"
#include "gui/guislider.h"
#include "gui/guitextbox.h"
#include "ac/dynobj/scriptgui.h"

using AGS::Common::GUIObject;
using AGS::Common::GUIButton;
using AGS::Common::GUIInvWindow;
using AGS::Common::GUILabel;
using AGS::Common::GUIListBox;
using AGS::Common::GUISlider;
using AGS::Common::GUITextBox;

GUIObject	*GetGUIControlAtLocation(int xx, int yy);
int			GUIControl_GetVisible(GUIObject *guio);
void		GUIControl_SetVisible(GUIObject *guio, int visible);
int			GUIControl_GetClickable(GUIObject *guio);
void		GUIControl_SetClickable(GUIObject *guio, int enabled);
int			GUIControl_GetEnabled(GUIObject *guio);
void		GUIControl_SetEnabled(GUIObject *guio, int enabled);
int			GUIControl_GetID(GUIObject *guio);
ScriptGUI*	GUIControl_GetOwningGUI(GUIObject *guio);
GUIButton*	GUIControl_GetAsButton(GUIObject *guio);
GUIInvWindow* GUIControl_GetAsInvWindow(GUIObject *guio);
GUILabel*	GUIControl_GetAsLabel(GUIObject *guio);
GUIListBox* GUIControl_GetAsListBox(GUIObject *guio);
GUISlider*	GUIControl_GetAsSlider(GUIObject *guio);
GUITextBox* GUIControl_GetAsTextBox(GUIObject *guio);
int			GUIControl_GetX(GUIObject *guio);
void		GUIControl_SetX(GUIObject *guio, int xx);
int			GUIControl_GetY(GUIObject *guio);
void		GUIControl_SetY(GUIObject *guio, int yy);
int         GUIControl_GetZOrder(GUIObject *guio);
void        GUIControl_SetZOrder(GUIObject *guio, int zorder);
void		GUIControl_SetPosition(GUIObject *guio, int xx, int yy);
int			GUIControl_GetWidth(GUIObject *guio);
void		GUIControl_SetWidth(GUIObject *guio, int newwid);
int			GUIControl_GetHeight(GUIObject *guio);
void		GUIControl_SetHeight(GUIObject *guio, int newhit);
void		GUIControl_SetSize(GUIObject *guio, int newwid, int newhit);
void		GUIControl_SendToBack(GUIObject *guio);
void		GUIControl_BringToFront(GUIObject *guio);

#endif // __AGS_EE_AC__GUICONTROL_H
