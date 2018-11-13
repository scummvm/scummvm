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

#include "gargoyle/frotz/frotz.h"
#include "gargoyle/frotz/frotz_types.h"
#include "common/config-manager.h"

namespace Gargoyle {
namespace Frotz {

Frotz *g_vm;

Frotz::Frotz(OSystem *syst, const GargoyleGameDescription *gameDesc) :
		Processor(syst, gameDesc) {
	g_vm = this;
}

Frotz::~Frotz() {
	reset_memory();
}

void Frotz::runGame(Common::SeekableReadStream *gameFile) {
	story_fp = gameFile;
	initialize();

	// Game loop
	interpret();
}

void Frotz::initialize() {
	if (ConfMan.hasKey("attribute_assignment") && ConfMan.getBool("attribute_assignment"))
		_attribute_assignment = true;
	if (ConfMan.hasKey("attribute_testing") && ConfMan.getBool("attribute_testing"))
		_attribute_testing = true;
	if (ConfMan.hasKey("ignore_errors") && ConfMan.getBool("ignore_errors"))
		_ignore_errors = true;
	if (ConfMan.hasKey("object_movement") && ConfMan.getBool("object_movement"))
		_object_movement = true;
	if (ConfMan.hasKey("object_locating") && ConfMan.getBool("object_locating"))
		_object_locating = true;
	if (ConfMan.hasKey("piracy") && ConfMan.getBool("piracy"))
		_piracy = true;
	if (ConfMan.hasKey("save_quetzal") && ConfMan.getBool("save_quetzal"))
		_save_quetzal = true;
	if (ConfMan.hasKey("random_seed"))
		_random.setSeed(ConfMan.getInt("random_seed"));
	if (ConfMan.hasKey("script_cols"))
		_script_cols = ConfMan.getInt("script_cols");
	if (ConfMan.hasKey("tandy_bit") && ConfMan.getBool("tandy_bit"))
		_user_tandy_bit = true;
	if (ConfMan.hasKey("undo_slots"))
		_undo_slots = ConfMan.getInt("undo_slots");
	if (ConfMan.hasKey("expand_abbreviations") && ConfMan.getBool("expand_abbreviations"))
		_expand_abbreviations = true;
	if (ConfMan.hasKey("err_report_mode")) {
		_err_report_mode = ConfMan.getInt("err_report_mode");
		if ((_err_report_mode < ERR_REPORT_NEVER) || (_err_report_mode > ERR_REPORT_FATAL))
			_err_report_mode = ERR_DEFAULT_REPORT_MODE;
	}

	// Call process initialization
	Processor::initialize();

	// Restart the game
	z_restart();
}

Common::Error Frotz::loadGameState(int slot) {
	// TODO
	return Common::kNoError;
}

Common::Error Frotz::saveGameState(int slot, const Common::String &desc) {
	// TODO
	return Common::kNoError;
}

} // End of namespace Scott
} // End of namespace Gargoyle
