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

#ifndef AGS_ENGINE_AC_GLOBAL_GUI_H
#define AGS_ENGINE_AC_GLOBAL_GUI_H

namespace AGS3 {

// IsGUIOn tells whether GUI is actually displayed on screen right now
int  IsGUIOn(int guinum);
// This is an internal script function, and is undocumented.
// It is used by the editor's automatic macro generation.
// TODO: find out how relevant this comment is?
int  FindGUIID(const char *GUIName);
// Sets GUI visible property on
void InterfaceOn(int ifn);
// Sets GUI visible property off
void InterfaceOff(int ifn);
void CentreGUI(int ifn);
int  GetTextWidth(const char *text, int fontnum);
int  GetTextHeight(const char *text, int fontnum, int width);
int  GetFontHeight(int fontnum);
int  GetFontLineSpacing(int fontnum);
void SetGUIBackgroundPic(int guin, int slotn);
void DisableInterface();
void EnableInterface();
// Returns 1 if user interface is enabled, 0 if disabled
int  IsInterfaceEnabled();
// pass trans=0 for fully solid, trans=100 for fully transparent
void SetGUITransparency(int ifn, int trans);
void SetGUIClickable(int guin, int clickable);
void SetGUIZOrder(int guin, int z);
void SetGUISize(int ifn, int widd, int hitt);
void SetGUIPosition(int ifn, int xx, int yy);
void SetGUIObjectSize(int ifn, int objn, int newwid, int newhit);
void SetGUIObjectEnabled(int guin, int objn, int enabled);
void SetGUIObjectPosition(int guin, int objn, int xx, int yy);
int GetGUIObjectAt(int xx, int yy);
int GetGUIAt(int xx, int yy);
void SetTextWindowGUI(int guinum);

} // namespace AGS3

#endif
