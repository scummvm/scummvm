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
#ifndef __AGS_EE_AC__DIALOGOPTIONSRENDERING_H
#define __AGS_EE_AC__DIALOGOPTIONSRENDERING_H

#include "ac/dynobj/scriptdialog.h"
#include "ac/dynobj/scriptdialogoptionsrendering.h"

int  DialogOptionsRendering_GetX(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetX(ScriptDialogOptionsRendering *dlgOptRender, int newX);
int  DialogOptionsRendering_GetY(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetY(ScriptDialogOptionsRendering *dlgOptRender, int newY);
int  DialogOptionsRendering_GetWidth(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetWidth(ScriptDialogOptionsRendering *dlgOptRender, int newWidth);
int  DialogOptionsRendering_GetHeight(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetHeight(ScriptDialogOptionsRendering *dlgOptRender, int newHeight);
int  DialogOptionsRendering_GetHasAlphaChannel(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetHasAlphaChannel(ScriptDialogOptionsRendering *dlgOptRender, bool hasAlphaChannel);
int  DialogOptionsRendering_GetParserTextboxX(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetParserTextboxX(ScriptDialogOptionsRendering *dlgOptRender, int newX);
int  DialogOptionsRendering_GetParserTextboxY(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetParserTextboxY(ScriptDialogOptionsRendering *dlgOptRender, int newY);
int  DialogOptionsRendering_GetParserTextboxWidth(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetParserTextboxWidth(ScriptDialogOptionsRendering *dlgOptRender, int newWidth);
ScriptDialog* DialogOptionsRendering_GetDialogToRender(ScriptDialogOptionsRendering *dlgOptRender);
ScriptDrawingSurface* DialogOptionsRendering_GetSurface(ScriptDialogOptionsRendering *dlgOptRender);
int  DialogOptionsRendering_GetActiveOptionID(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetActiveOptionID(ScriptDialogOptionsRendering *dlgOptRender, int activeOptionID);

#endif // __AGS_EE_AC__DIALOGOPTIONSRENDERING_H
