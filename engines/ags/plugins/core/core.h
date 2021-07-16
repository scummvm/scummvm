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

public:
	void AGS_EngineStartup(IAGSEngine *engine);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
