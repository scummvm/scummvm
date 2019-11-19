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

#include "glk/magnetic/magnetic.h"
#include "glk/magnetic/defs.h"
#include "common/config-manager.h"

namespace Glk {
namespace Magnetic {

Magnetic *g_vm;

Magnetic::Magnetic(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		gms_gamma_mode(GAMMA_NORMAL), gms_animation_enabled(true),
		gms_prompt_enabled(true), gms_abbreviations_enabled(true), gms_commands_enabled(true),
		gms_graphics_enabled(false) {
	g_vm = this;
}

void Magnetic::runGame() {
	initialize();
	gms_main();
}

void Magnetic::initialize() {
	// Local handling for Glk special commands
	if (ConfMan.hasKey("commands_enabled"))
		gms_commands_enabled = ConfMan.getBool("commands_enabled");
	// Abbreviation expansions
	if (ConfMan.hasKey("abbreviations_enabled"))
		gms_abbreviations_enabled = ConfMan.getBool("abbreviations_enabled");
	// Pictures enabled
	if (ConfMan.hasKey("graphics_enabled"))
		gms_graphics_enabled = ConfMan.getBool("graphics_enabled");
	// Automatic gamma correction on pictures
	if (ConfMan.hasKey("gamma_mode") && !ConfMan.getBool("gamma_mode"))
		gms_gamma_mode = GAMMA_OFF;
	// Animations
	if (ConfMan.hasKey("animation_enabled"))
		gms_animation_enabled = ConfMan.getBool("animation_enabled");
	// Prompt enabled
	if (ConfMan.hasKey("prompt_enabled"))
		gms_prompt_enabled = ConfMan.getBool("prompt_enabled");

	// Close the already opened gamefile, since the Magnetic code will open it on it's own
	_gameFile.close();
}

Common::Error Magnetic::readSaveData(Common::SeekableReadStream *rs) {
	// TODO
	return Common::kReadingFailed;
}

Common::Error Magnetic::writeGameData(Common::WriteStream *ws) {
	// TODO
	return Common::kWritingFailed;
}

} // End of namespace Magnetic
} // End of namespace Glk
