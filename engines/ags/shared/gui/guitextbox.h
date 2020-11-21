/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

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
