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
#include "ags/plugins/plugin_base.h"
#include "ags/plugins/ags_blend/ags_blend.h"
#include "ags/plugins/ags_controller/ags_controller.h"
#include "ags/plugins/ags_creditz/ags_creditz1.h"
#include "ags/plugins/ags_creditz/ags_creditz2.h"
#include "ags/plugins/ags_flashlight/ags_flashlight.h"
#include "ags/plugins/ags_galaxy_steam/ags_wadjeteye_steam.h"
#include "ags/plugins/ags_galaxy_steam/ags_galaxy_steam.h"
#include "ags/plugins/ags_joy/ags_joy.h"
#include "ags/plugins/ags_nickenstien_gfx/ags_nickenstien_gfx.h"
#include "ags/plugins/ags_pal_render/ags_pal_render.h"
#include "ags/plugins/ags_shell/ags_shell.h"
#include "ags/plugins/ags_snow_rain/ags_snow_rain.h"
#include "ags/plugins/ags_sprite_font/ags_sprite_font.h"
#include "ags/plugins/ags_sprite_font/ags_sprite_font_clifftop.h"
#include "ags/plugins/ags_tcp_ip/ags_tcp_ip.h"
#include "ags/plugins/ags_wadjet_util/ags_wadjet_util.h"
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

	if (fname.equalsIgnoreCase("AGSController"))
		return new AGSController::AGSController();

	if (fname.equalsIgnoreCase("agsCreditz"))
		return new AGSCreditz::AGSCreditz1();

	if (fname.equalsIgnoreCase("agsCreditz2"))
		return new AGSCreditz::AGSCreditz2();

	if (fname.equalsIgnoreCase("AGSFlashlight"))
		return new AGSFlashlight::AGSFlashlight();

	if (fname.equalsIgnoreCase("AGSJoy"))
		return new AGSJoy::AGSJoy();

	if (fname.equalsIgnoreCase("AGSPalRender"))
		return new AGSPalRender::AGSPalRender();

	if (fname.equalsIgnoreCase("ags_shell"))
		return new AGSShell::AGSShell();

	if (fname.equalsIgnoreCase("AGSSnowRain") || fname.equalsIgnoreCase("ags_snowrain"))
		return new AGSSnowRain::AGSSnowRain();

	if ((fname.equalsIgnoreCase("AGSSpriteFont") && version == ::AGS::kClifftopGames))
		return new AGSSpriteFont::AGSSpriteFontClifftopGames();

	if (fname.equalsIgnoreCase("AGSSpriteFont") || fname.equalsIgnoreCase("agsplugin.spritefont"))
		return new AGSSpriteFont::AGSSpriteFont();

	if (fname.equalsIgnoreCase("agsgalaxy") || fname.equalsIgnoreCase("agsgalaxy-unified") ||
			fname.equalsIgnoreCase("agsgalaxy-disjoint"))
		return new AGSGalaxySteam::AGSGalaxy();

	if (fname.equalsIgnoreCase("ags_Nickenstien_GFX"))
		return new AGSNickenstienGFX::AGSNickenstienGFX();

	if (fname.equalsIgnoreCase("agsteam") && version == ::AGS::kWadjetEye)
		return new AGSGalaxySteam::AGSWadjetEyeSteam();

	if (fname.equalsIgnoreCase("agsteam") || fname.equalsIgnoreCase("agsteam-unified") ||
			fname.equalsIgnoreCase("agsteam-disjoint"))
		return new AGSGalaxySteam::AGSSteam();

	if (fname.equalsIgnoreCase("AGSWadjetUtil"))
		return new AGSWadjetUtil::AGSWadjetUtil();

	debug("Plugin '%s' is not yet supported", fname.c_str());
	return nullptr;
}

int pluginClose(void *lib) {
	PluginBase *plugin = static_cast<PluginBase *>(lib);
	delete plugin;
	return 0;
}

void *pluginSym(void *lib, const char *method) {
	PluginBase *plugin = static_cast<PluginBase *>(lib);
	return (*plugin)[method];
}

const char *pluginError() {
	return nullptr;
}

/*------------------------------------------------------------------*/

PluginBase::PluginBase() {
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

int PluginBase::AGS_EditorStartup(IAGSEditor *) {
	return 0;
}

void PluginBase::AGS_EditorShutdown() {
}

void PluginBase::AGS_EditorProperties(HWND) {
}

int PluginBase::AGS_EditorSaveGame(char *, int) {
	return 0;
}

void PluginBase::AGS_EditorLoadGame(char *, int) {
}

void PluginBase::AGS_EngineStartup(IAGSEngine *) {
}

void PluginBase::AGS_EngineShutdown() {
}

int64 PluginBase::AGS_EngineOnEvent(int, NumberPtr) {
	return 0;
}

int PluginBase::AGS_EngineDebugHook(const char *, int, int) {
	return 0;
}

void PluginBase::AGS_EngineInitGfx(const char *driverID, void *data) {
	assert(!strcmp(driverID, "Software"));
}

} // namespace Plugins
} // namespace AGS3
