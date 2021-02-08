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

#include "ags/lib/allegro.h"
#include "ags/plugins/dll.h"
#include "ags/plugins/ags_blend/ags_blend.h"
#include "ags/plugins/ags_creditz/ags_creditz1.h"
#include "ags/plugins/ags_creditz/ags_creditz2.h"
#include "ags/plugins/ags_flashlight/ags_flashlight.h"
#include "ags/plugins/ags_galaxy_steam/ags_galaxy_steam.h"
#include "ags/plugins/ags_pal_render/ags_pal_render.h"
#include "ags/plugins/ags_snow_rain/ags_snow_rain.h"
#include "ags/plugins/ags_sprite_font/ags_sprite_font.h"
#include "ags/plugins/ags_tcp_ip/ags_tcp_ip.h"
#include "ags/ags.h"
#include "ags/detection.h"
#include "common/str.h"

namespace AGS3 {
namespace Plugins {

void *pluginOpen(const char *filename) {
	Common::String fname(filename);

	// Check for if the game specifies a specific plugin version for this game
	int version = 0;
	for (const ::AGS::PluginVersion *v = ::AGS::g_vm->getNeededPlugins();
	        v && v->_plugin; ++v) {
		if (Common::String(v->_plugin).equalsIgnoreCase(filename)) {
			version = v->_version;
			break;
		}
	}

	if (fname.equalsIgnoreCase("ags_tcp_ip"))
		return new AGSTcpIp::AGSTcpIp();

	if (fname.equalsIgnoreCase("AGSBlend"))
		return new AGSBlend::AGSBlend();

	if (fname.equalsIgnoreCase("agsCreditz"))
		return new AGSCreditz::AGSCreditz1();

	if (fname.equalsIgnoreCase("agsCreditz2"))
		return new AGSCreditz::AGSCreditz2();

	if (fname.equalsIgnoreCase("AGSFlashlight"))
		return new AGSFlashlight::AGSFlashlight();

	if (fname.equalsIgnoreCase("AGSPalRender"))
		return new AGSPalRender::AGSPalRender();

	if (fname.equalsIgnoreCase("AGSSnowRain") || fname.equalsIgnoreCase("ags_snowrain"))
		return new AGSSnowRain::AGSSnowRain();

	if (fname.equalsIgnoreCase("AGSSpriteFont"))
		return new AGSSpriteFont::AGSSpriteFont();

	if (fname.equalsIgnoreCase("agsgalaxy") || fname.equalsIgnoreCase("agsgalaxy-unified") ||
			fname.equalsIgnoreCase("agsgalaxy-disjoint"))
		return new AGSGalaxySteam::AGSGalaxy();

	if (fname.equalsIgnoreCase("agsteam") || fname.equalsIgnoreCase("agsteam-unified") ||
			fname.equalsIgnoreCase("agsteam-disjoint"))
		return new AGSGalaxySteam::AGSSteam();

	return nullptr;
}

int pluginClose(void *lib) {
	DLL *dll = static_cast<DLL *>(lib);
	delete dll;
	return 0;
}

void *pluginSym(void *lib, const char *method) {
	DLL *dll = static_cast<DLL *>(lib);
	return (*dll)[method];
}

const char *pluginError() {
	return nullptr;
}

/*------------------------------------------------------------------*/

DLL::DLL() {
	DLL_METHOD(AGS_PluginV2);
	DLL_METHOD(AGS_EditorStartup);
	DLL_METHOD(AGS_EditorShutdown);
	DLL_METHOD(AGS_EditorProperties);
	DLL_METHOD(AGS_EditorSaveGame);
	DLL_METHOD(AGS_EditorLoadGame);
	DLL_METHOD(AGS_EngineStartup);
	DLL_METHOD(AGS_EngineShutdown);
	DLL_METHOD(AGS_EngineOnEvent);
	DLL_METHOD(AGS_EngineDebugHook);
	DLL_METHOD(AGS_EngineInitGfx);
}

int DLL::AGS_EditorStartup(IAGSEditor *) {
	return 0;
}

void DLL::AGS_EditorShutdown() {
}

void DLL::AGS_EditorProperties(HWND) {
}

int DLL::AGS_EditorSaveGame(char *, int) {
	return 0;
}

void DLL::AGS_EditorLoadGame(char *, int) {
}

void DLL::AGS_EngineStartup(IAGSEngine *) {
}

void DLL::AGS_EngineShutdown() {
}

NumberPtr DLL::AGS_EngineOnEvent(int, NumberPtr) {
	return 0;
}

int DLL::AGS_EngineDebugHook(const char *, int, int) {
	return 0;
}

void DLL::AGS_EngineInitGfx(const char *driverID, void *data) {
	assert(!strcmp(driverID, "Software"));
}

} // namespace Plugins
} // namespace AGS3
