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

#ifndef AGS_PLUGINS_CORE_CORE_H
#define AGS_PLUGINS_CORE_CORE_H

#include "ags/plugins/core/audio_channel.h"
#include "ags/plugins/core/audio_clip.h"
#include "ags/plugins/core/button.h"
#include "ags/plugins/core/character.h"
#include "ags/plugins/core/date_time.h"
#include "ags/plugins/core/dialog.h"
#include "ags/plugins/core/dialog_options_rendering_info.h"
#include "ags/plugins/core/drawing_surface.h"
#include "ags/plugins/core/dynamic_sprite.h"
#include "ags/plugins/core/file.h"
#include "ags/plugins/core/game.h"
#include "ags/plugins/core/global_api.h"
#include "ags/plugins/core/gui.h"
#include "ags/plugins/core/gui_control.h"
#include "ags/plugins/core/hotspot.h"
#include "ags/plugins/core/inventory_item.h"
#include "ags/plugins/core/inv_window.h"
#include "ags/plugins/core/label.h"
#include "ags/plugins/core/listbox.h"
#include "ags/plugins/core/maths.h"
#include "ags/plugins/core/mouse.h"
#include "ags/plugins/core/object.h"
#include "ags/plugins/core/overlay.h"
#include "ags/plugins/core/parser.h"
#include "ags/plugins/core/region.h"
#include "ags/plugins/core/room.h"
#include "ags/plugins/core/slider.h"
#include "ags/plugins/core/string.h"
#include "ags/plugins/core/system.h"
#include "ags/plugins/core/textbox.h"
#include "ags/plugins/core/view_frame.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class EngineExports {
private:
	AudioChannel _audioChannel;
	AudioClip _audioClip;
	Button _button;
	Character _character;
	DateTime _dateTime;
	Dialog _dialog;
	DialogOptionsRenderingInfo _dialogOptionsRenderingInfo;
	DrawingSurface _drawingSurface;
	DynamicSprite _dynamicSprite;
	File _file;
	Game _game;
	GlobalAPI _globalAPI;
	GUI _gui;
	GUIControl _guiControl;
	Hotspot _hotspot;
	InvWindow _invWindow;
	InventoryItem _inventoryItem;
	Label _label;
	ListBox _listbox;
	Maths _math;
	Mouse _mouse;
	Object _object;
	Overlay _overlay;
	Parser _parser;
	Region _region;
	Room _room;
	Slider _slider;
	String _string;
	System _system;
	Textbox _textbox;
	ViewFrame _viewFrame;

public:
	void AGS_EngineStartup(IAGSEngine *engine);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
