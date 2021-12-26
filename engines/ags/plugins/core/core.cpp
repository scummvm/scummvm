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

#include "ags/plugins/core/core.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void EngineExports::AGS_EngineStartup(IAGSEngine *engine) {
	_audioChannel.AGS_EngineStartup(engine);
	_audioClip.AGS_EngineStartup(engine);
	_button.AGS_EngineStartup(engine);
	_character.AGS_EngineStartup(engine);
	_dateTime.AGS_EngineStartup(engine);
	_dialog.AGS_EngineStartup(engine);
	_dialogOptionsRenderingInfo.AGS_EngineStartup(engine);
	_drawingSurface.AGS_EngineStartup(engine);
	_dynamicSprite.AGS_EngineStartup(engine);
	_file.AGS_EngineStartup(engine);
	_game.AGS_EngineStartup(engine);
	_globalAPI.AGS_EngineStartup(engine);
	_gui.AGS_EngineStartup(engine);
	_guiControl.AGS_EngineStartup(engine);
	_hotspot.AGS_EngineStartup(engine);
	_invWindow.AGS_EngineStartup(engine);
	_inventoryItem.AGS_EngineStartup(engine);
	_label.AGS_EngineStartup(engine);
	_listbox.AGS_EngineStartup(engine);
	_math.AGS_EngineStartup(engine);
	_mouse.AGS_EngineStartup(engine);
	_object.AGS_EngineStartup(engine);
	_overlay.AGS_EngineStartup(engine);
	_parser.AGS_EngineStartup(engine);
	_region.AGS_EngineStartup(engine);
	_room.AGS_EngineStartup(engine);
	_slider.AGS_EngineStartup(engine);
	_string.AGS_EngineStartup(engine);
	_system.AGS_EngineStartup(engine);
	_textbox.AGS_EngineStartup(engine);
	_viewFrame.AGS_EngineStartup(engine);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
