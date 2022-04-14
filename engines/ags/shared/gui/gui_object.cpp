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

#include "ags/shared/ac/common.h" // quit
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/gui/gui_object.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

GUIObject::GUIObject() {
	Id = 0;
	ParentId = 0;
	Flags = kGUICtrl_DefFlags;
	X = 0;
	Y = 0;
	Width = 0;
	Height = 0;
	ZOrder = -1;
	IsActivated = false;
	_scEventCount = 0;
}

String GUIObject::GetScriptName() const {
	return Name;
}

int GUIObject::GetEventCount() const {
	return _scEventCount;
}

String GUIObject::GetEventName(int event) const {
	if (event < 0 || event >= _scEventCount)
		return "";
	return _scEventNames[event];
}

String GUIObject::GetEventArgs(int event) const {
	if (event < 0 || event >= _scEventCount)
		return "";
	return _scEventArgs[event];
}

bool GUIObject::IsDeleted() const {
	return (Flags & kGUICtrl_Deleted) != 0;
}

bool GUIObject::IsEnabled() const {
	return (Flags & kGUICtrl_Enabled) != 0;
}

bool GUIObject::IsOverControl(int x, int y, int leeway) const {
	return x >= X && y >= Y && x < (X + Width + leeway) && y < (Y + Height + leeway);
}

bool GUIObject::IsTranslated() const {
	return (Flags & kGUICtrl_Translated) != 0;
}

bool GUIObject::IsVisible() const {
	return (Flags & kGUICtrl_Visible) != 0;
}

void GUIObject::SetClickable(bool on) {
	if (on)
		Flags |= kGUICtrl_Clickable;
	else
		Flags &= ~kGUICtrl_Clickable;
}

void GUIObject::SetEnabled(bool on) {
	if (on)
		Flags |= kGUICtrl_Enabled;
	else
		Flags &= ~kGUICtrl_Enabled;
	NotifyParentChanged();
}

void GUIObject::SetTranslated(bool on) {
	if (on)
		Flags |= kGUICtrl_Translated;
	else
		Flags &= ~kGUICtrl_Translated;
	NotifyParentChanged();
}

void GUIObject::SetVisible(bool on) {
	if (on)
		Flags |= kGUICtrl_Visible;
	else
		Flags &= ~kGUICtrl_Visible;
}

// TODO: replace string serialization with StrUtil::ReadString and WriteString
// methods in the future, to keep this organized.
void GUIObject::WriteToFile(Stream *out) const {
	out->WriteInt32(Flags);
	out->WriteInt32(X);
	out->WriteInt32(Y);
	out->WriteInt32(Width);
	out->WriteInt32(Height);
	out->WriteInt32(ZOrder);
	Name.Write(out);
	out->WriteInt32(_scEventCount);
	for (int i = 0; i < _scEventCount; ++i)
		EventHandlers[i].Write(out);
}

void GUIObject::ReadFromFile(Stream *in, GuiVersion gui_version) {
	Flags = in->ReadInt32();
	// reverse particular flags from older format
	if (gui_version < kGuiVersion_350)
		Flags ^= kGUICtrl_OldFmtXorMask;
	X = in->ReadInt32();
	Y = in->ReadInt32();
	Width = in->ReadInt32();
	Height = in->ReadInt32();
	ZOrder = in->ReadInt32();
	if (gui_version < kGuiVersion_350) { // NOTE: reading into actual variables only for old savegame support
		IsActivated = in->ReadInt32() != 0;
	}

	if (gui_version >= kGuiVersion_unkn_106)
		Name.Read(in);
	else
		Name.Free();

	for (int i = 0; i < _scEventCount; ++i) {
		EventHandlers[i].Free();
	}

	if (gui_version >= kGuiVersion_unkn_108) {
		int evt_count = in->ReadInt32();
		if (evt_count > _scEventCount)
			quit("Error: too many control events, need newer version");
		for (int i = 0; i < evt_count; ++i) {
			EventHandlers[i].Read(in);
		}
	}
}

void GUIObject::ReadFromSavegame(Stream *in, GuiSvgVersion svg_ver) {
	// Properties
	Flags = in->ReadInt32();
	// reverse particular flags from older format
	if (svg_ver < kGuiSvgVersion_350)
		Flags ^= kGUICtrl_OldFmtXorMask;
	X = in->ReadInt32();
	Y = in->ReadInt32();
	Width = in->ReadInt32();
	Height = in->ReadInt32();
	ZOrder = in->ReadInt32();
	// Dynamic state
	IsActivated = in->ReadBool() ? 1 : 0;
}

void GUIObject::WriteToSavegame(Stream *out) const {
	// Properties
	out->WriteInt32(Flags);
	out->WriteInt32(X);
	out->WriteInt32(Y);
	out->WriteInt32(Width);
	out->WriteInt32(Height);
	out->WriteInt32(ZOrder);
	// Dynamic state
	out->WriteBool(IsActivated != 0);
}


HorAlignment ConvertLegacyGUIAlignment(LegacyGUIAlignment align) {
	switch (align) {
	case kLegacyGUIAlign_Left:
		return kHAlignLeft;
	case kLegacyGUIAlign_Right:
		return kHAlignRight;
	case kLegacyGUIAlign_Center:
		return kHAlignCenter;
	}
	return kHAlignNone;
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
