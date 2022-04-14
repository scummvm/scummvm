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

#ifndef AGS_SHARED_GUI_GUI_OBJECT_H
#define AGS_SHARED_GUI_GUI_OBJECT_H

#include "ags/shared/core/types.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/gui/gui_defines.h"
#include "ags/shared/util/string.h"
#include "ags/globals.h"

namespace AGS3 {

struct KeyInput;

namespace AGS {
namespace Shared {

enum LegacyGUIAlignment {
	kLegacyGUIAlign_Left = 0,
	kLegacyGUIAlign_Right = 1,
	kLegacyGUIAlign_Center = 2
};

class GUIObject {
public:
	GUIObject();
	virtual ~GUIObject() {}

	String          GetScriptName() const;

	String          GetEventArgs(int event) const;
	int             GetEventCount() const;
	String          GetEventName(int event) const;
	bool            IsDeleted() const;
	// tells if control itself is enabled
	bool            IsEnabled() const;
	// overridable routine to determine whether the mouse is over the control
	virtual bool    IsOverControl(int x, int y, int leeway) const;
	bool            IsTranslated() const;
	bool            IsVisible() const;
	// implemented separately in engine and editor
	bool            IsClickable() const;

	// Operations
	virtual void    Draw(Bitmap *ds) {
	}
	void            SetClickable(bool on);
	void            SetEnabled(bool on);
	void            SetTranslated(bool on);
	void            SetVisible(bool on);

	// Events
	// Key pressed for control
	virtual void    OnKeyPress(const KeyInput &ki) {}
	// Mouse button down - return 'True' to lock focus
	virtual bool    OnMouseDown() {
		return false;
	}
	// Mouse moves onto control
	virtual void    OnMouseEnter() {
	}
	// Mouse moves off control
	virtual void    OnMouseLeave() {
	}
	// Mouse moves over control - x,y relative to gui
	virtual void    OnMouseMove(int x, int y) {
	}
	// Mouse button up
	virtual void    OnMouseUp() {
	}
	// Control was resized
	virtual void    OnResized() {
	}

	// Serialization
	virtual void    ReadFromFile(Shared::Stream *in, GuiVersion gui_version);
	virtual void    WriteToFile(Shared::Stream *out) const;
	virtual void    ReadFromSavegame(Shared::Stream *in, GuiSvgVersion svg_ver);
	virtual void    WriteToSavegame(Shared::Stream *out) const;

	// TODO: these members are currently public; hide them later
public:
	// Notifies parent GUI that this control has changed
	void     NotifyParentChanged();

	int32_t  Id;         // GUI object's identifier
	int32_t  ParentId;   // id of parent GUI
	String   Name;       // script name

	int32_t  X;
	int32_t  Y;
	int32_t  Width;
	int32_t  Height;
	int32_t  ZOrder;
	bool     IsActivated; // signals user interaction

	String   EventHandlers[MAX_GUIOBJ_EVENTS]; // script function names

protected:
	uint32_t Flags;      // generic style and behavior flags

	// TODO: explicit event names & handlers for every event
	int32_t  _scEventCount;                    // number of supported script events
	String   _scEventNames[MAX_GUIOBJ_EVENTS]; // script event names
	String   _scEventArgs[MAX_GUIOBJ_EVENTS];  // script handler params
};

// Converts legacy alignment type used in GUI Label/ListBox data (only left/right/center)
HorAlignment ConvertLegacyGUIAlignment(LegacyGUIAlignment align);

} // namespace Shared
} // namespace AGS

// Tells if all controls are disabled

// Tells if the given control is considered enabled, taking global flag into account
inline bool IsGUIEnabled(AGS::Shared::GUIObject *g) {
	return (_G(all_buttons_disabled) < 0) && g->IsEnabled();
}

} // namespace AGS3

#endif
