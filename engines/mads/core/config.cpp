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

#include "common/config-manager.h"
#include "mads/core/config.h"
#include "mads/core/env.h"
#include "mads/core/inter.h"
#include "mads/core/kernel.h"
#include "mads/engine.h"

namespace MADS {
namespace MADSV2 {

ConfigFile config_file = {
	0, 0x220,
	0, 0x220, 7, 1,
	true, true, 1,
	0, 0,
	0, 0,
	0, true,
	0, 0,
	false, 'D',
	false,
	false,
	7, 0, 0, 0, 0,
	false
};

void read_config_file() {
	ConfMan.registerDefault("music_mute", false);
	ConfMan.registerDefault("sfx_mute", false);
	ConfMan.registerDefault("speech_mute", false);

	ConfMan.registerDefault("interface_hotspots", true);
	ConfMan.registerDefault("inventory_mode", true);
	ConfMan.registerDefault("animated_interface", true);
	ConfMan.registerDefault("show_speech_boxes", true);
	ConfMan.registerDefault("naughtiness", true);
	ConfMan.registerDefault("difficulty", -1);

	config_file.music_flag = !ConfMan.getBool("music_mute") && !ConfMan.getBool("mute");
	config_file.sound_flag = !ConfMan.getBool("sfx_mute") && !ConfMan.getBool("mute");
	config_file.speech_flag = !ConfMan.getBool("speech_mute") && !ConfMan.getBool("mute");

	config_file.interface_hotspots = ConfMan.getBool("interface_hotspots") ? INTERFACE_BRAINDEAD : INTERFACE_MACINTOSH;
	config_file.inventory_mode = ConfMan.getBool("inventory_mode") ? INVENTORY_SPINNING : INVENTORY_SQUAT;
	config_file.animated_interface = ConfMan.getBool("animated_interface") ? INTERFACE_ANIMATED : INTERFACE_STILL;
	config_file.naughtiness = ConfMan.getBool("naughtiness") ? NAUGHTY : NICE;

	config_file.quotes_enabled = ConfMan.getBool("quotes_enabled");
	config_file.screen_fade = ConfMan.getBool("screen_fade");
	config_file.panning_speed = ConfMan.getBool("panning_speed");

	config_file.show_speech_boxes = ConfMan.getBool("show_speech_boxes");
	config_file.original_save_load = ConfMan.getBool("original_menus");

	if (ConfMan.hasKey("difficulty") && !g_engine->isDemo())
		game.difficulty = ConfMan.getInt("difficulty");
}

void write_config_file() {
	ConfMan.setBool("music_mute", !config_file.music_flag);
	ConfMan.setBool("sfx_mute", !config_file.sound_flag);
	ConfMan.setBool("speech_mute", !config_file.speech_flag);

	ConfMan.setInt("interface_hotspots", config_file.interface_hotspots);
	ConfMan.setInt("inventory_spinning", config_file.inventory_mode);
	ConfMan.setBool("animated_interface", config_file.animated_interface);

	ConfMan.setBool("quotes_enabled", config_file.quotes_enabled);
	ConfMan.setBool("screen_fade", config_file.screen_fade);
	ConfMan.setBool("panning_speed", config_file.panning_speed);

	ConfMan.setBool("show_speech_boxes", config_file.show_speech_boxes);
	if (g_engine->getGameID() == GType_RexNebular)
	ConfMan.setInt("naughtiness", config_file.naughtiness);

	ConfMan.flushToDisk();
}

void global_load_config_parameters() {
	inter_report_hotspots = (config_file.interface_hotspots == INTERFACE_BRAINDEAD);
	inter_spinning_objects = true;
	inter_animation_running = true;

	kernel_panning_speed = config_file.panning_speed;
	kernel_screen_fade = config_file.screen_fade;

	if (config_file.cd_version_installed) {
		env_search_cd = true;
		env_cd_drive = (char)config_file.cd_drive;
	}
}

void global_unload_config_parameters() {
	config_file.interface_hotspots = inter_report_hotspots ? INTERFACE_BRAINDEAD : INTERFACE_MACINTOSH;

	config_file.panning_speed = kernel_panning_speed;
	config_file.screen_fade = kernel_screen_fade;
}

} // namespace MADSV2
} // namespace MADS
