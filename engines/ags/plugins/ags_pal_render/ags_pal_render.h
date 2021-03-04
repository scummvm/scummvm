/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
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

#ifndef AGS_PLUGINS_AGS_PAL_RENDER_AGS_PAL_RENDER_H
#define AGS_PLUGINS_AGS_PAL_RENDER_AGS_PAL_RENDER_H

#include "ags/plugins/plugin_base.h"
#include "ags/plugins/serializer.h"

namespace AGS3 {
namespace Plugins {
namespace AGSPalRender {

class AGSPalRender : public PluginBase {
private:
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *lpEngine);
	static void AGS_EngineShutdown();
	static int64 AGS_EngineOnEvent(int event, NumberPtr data);
	static void syncGame(Serializer &s);
public:
	AGSPalRender();
};

} // namespace AGSPalRender
} // namespace Plugins
} // namespace AGS3

#endif
