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

#include "ags/lib/allegro.h"
#include "ags/plugins/plugin_base.h"
#include "ags/plugins/ags_agi/ags_agi.h"
#include "ags/plugins/ags_app_open_url/ags_app_open_url.h"
#include "ags/plugins/ags_blend/ags_blend.h"
#include "ags/plugins/ags_clipboard/ags_clipboard.h"
#include "ags/plugins/ags_collision_detector/ags_collision_detector.h"
#include "ags/plugins/ags_controller/ags_controller.h"
#include "ags/plugins/ags_creditz/ags_creditz1.h"
#include "ags/plugins/ags_creditz/ags_creditz2.h"
#include "ags/plugins/ags_fire/ags_fire.h"
#include "ags/plugins/ags_flashlight/ags_flashlight.h"
#include "ags/plugins/ags_galaxy_steam/ags_wadjeteye_steam.h"
#include "ags/plugins/ags_galaxy_steam/ags_galaxy_steam.h"
#include "ags/plugins/ags_joy/ags_joy.h"
#include "ags/plugins/ags_nickenstien_gfx/ags_nickenstien_gfx.h"
#include "ags/plugins/ags_pal_render/ags_pal_render.h"
#include "ags/plugins/ags_parallax/ags_parallax.h"
#include "ags/plugins/ags_shell/ags_shell.h"
#include "ags/plugins/ags_snow_rain/ags_snow_rain.h"
#include "ags/plugins/ags_sock/ags_sock.h"
#include "ags/plugins/ags_sprite_font/ags_sprite_font.h"
#include "ags/plugins/ags_sprite_font/ags_sprite_font_clifftop.h"
#include "ags/plugins/ags_tcp_ip/ags_tcp_ip.h"
#include "ags/plugins/ags_touch/ags_touch.h"
#include "ags/plugins/ags_trans/ags_trans.h"
#include "ags/plugins/ags_wadjet_util/ags_wadjet_util.h"
#include "ags/plugins/ags_waves/ags_waves.h"
#include "ags/ags.h"
#include "ags/detection.h"
#include "common/str.h"

namespace AGS3 {
namespace Plugins {

Plugins::PluginBase *pluginOpen(const char *filename) {
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

	if (fname.equalsIgnoreCase("AGS_AGI"))
		return new AGSAgi::AGSAgi();

	if (fname.equalsIgnoreCase("agsappopenurl"))
		return new AGSAppOpenURL::AGSAppOpenURL();

	if (fname.equalsIgnoreCase("AGSBlend"))
		return new AGSBlend::AGSBlend();

	if (fname.equalsIgnoreCase("AGSClipboard"))
		return new AGSClipboard::AGSClipboard();

	if (fname.equalsIgnoreCase("AGSController"))
		return new AGSController::AGSController();

	if (fname.equalsIgnoreCase("AGS_Collision_Detector"))
		return new AGSCollisionDetector::AGSCollisionDetector();

	if (fname.equalsIgnoreCase("agsCreditz"))
		return new AGSCreditz::AGSCreditz1();

	if (fname.equalsIgnoreCase("agsCreditz2"))
		return new AGSCreditz::AGSCreditz2();

	if (fname.equalsIgnoreCase("AGS_Fire"))
		return new AGSFire::AGSFire();

	if (fname.equalsIgnoreCase("AGSFlashlight"))
		return new AGSFlashlight::AGSFlashlight();

	if (fname.equalsIgnoreCase("AGSJoy"))
		return new AGSJoy::AGSJoy();

	if (fname.equalsIgnoreCase("AGSPalRender"))
		return new AGSPalRender::AGSPalRender();

	if (fname.equalsIgnoreCase("ags_parallax") ||
			fname.equalsIgnoreCase("AGSParallax"))
		return new AGSParallax::AGSParallax();

	if (fname.equalsIgnoreCase("ags_shell") || fname.equalsIgnoreCase("agsshell"))
		return new AGSShell::AGSShell();

	if (fname.equalsIgnoreCase("AGSSnowRain") || fname.equalsIgnoreCase("ags_snowrain"))
		return new AGSSnowRain::AGSSnowRain();

	if (fname.equalsIgnoreCase("AGSSock"))
		return new AGSSock::AGSSock();

	if (fname.equalsIgnoreCase("AGSSpriteFont") || fname.equalsIgnoreCase("agsplugin.spritefont")) {
		if (version == ::AGS::kClifftopGames)
			return new AGSSpriteFont::AGSSpriteFontClifftopGames();
		return new AGSSpriteFont::AGSSpriteFont();
	}

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

	if (fname.equalsIgnoreCase("AGSTouch"))
		return new AGSTouch::AGSTouch();

	if (fname.equalsIgnoreCase("AGSTrans"))
		return new AGSTrans::AGSTrans();

	if (fname.equalsIgnoreCase("AGSWadjetUtil"))
		return new AGSWadjetUtil::AGSWadjetUtil();

	if (fname.equalsIgnoreCase("agswaves"))
		return new AGSWaves::AGSWaves();

	debug("Plugin '%s' is not yet supported", fname.c_str());
	return nullptr;
}

int pluginClose(Plugins::PluginBase *lib) {
	PluginBase *plugin = static_cast<PluginBase *>(lib);
	delete plugin;
	return 0;
}

const char *pluginError() {
	return nullptr;
}

/*------------------------------------------------------------------*/

ScriptMethodParams::ScriptMethodParams() {

}

ScriptMethodParams::ScriptMethodParams(int val1) {
	push_back(val1);
}

ScriptMethodParams::ScriptMethodParams(int val1, int val2) {
	push_back(val1);
	push_back(val2);
}

ScriptMethodParams::ScriptMethodParams(int val1, int val2, int val3) {
	push_back(val1);
	push_back(val2);
	push_back(val3);
}

ScriptMethodParams::ScriptMethodParams(int val1, int val2, int val3, int val4) {
	push_back(val1);
	push_back(val2);
	push_back(val3);
	push_back(val4);
}

#define GET_CHAR c = format[0]; format.deleteChar(0)

Common::String ScriptMethodParams::format(int formatIndex) {
	Common::String result;

	Common::String format((const char *)(*this)[formatIndex]);
	Common::String paramFormat;
	char c;
	++formatIndex;

	while (!format.empty()) {
		GET_CHAR;

		if (c != '%') {
			result += c;

		} else if (format.hasPrefix("%")) {
			GET_CHAR;
			result += '%';

		} else {
			// Form up a format specifier
			paramFormat = "%";
			while (!format.empty()) {
				GET_CHAR;
				paramFormat += c;

				if (Common::isAlpha(c))
					break;
			}

			// Convert the parameter to a string. Not sure if all the
			// casts are necessary, but it's better safe than sorry
			// for big endian systems
			switch (tolower(paramFormat.lastChar())) {
			case 'c':
				result += Common::String::format(paramFormat.c_str(), (char)(*this)[formatIndex]);
				break;
			case 's':
			case 'p':
				result += Common::String::format(paramFormat.c_str(), (void *)(*this)[formatIndex]);
				break;
			default:
				result += Common::String::format(paramFormat.c_str(), (int)(*this)[formatIndex]);
				break;
			}

			formatIndex++;
		}
	}

	return result;
}

} // namespace Plugins
} // namespace AGS3
