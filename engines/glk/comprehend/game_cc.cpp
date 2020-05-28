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

#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/game_data.h"
#include "glk/comprehend/game.h"

namespace Glk {
namespace Comprehend {

static void cc_clear_companion_flags(struct comprehend_game *game)
{
	/* Clear the Sabrina/Erik action flags */
	game->info->flags[0xa] = 0;
	game->info->flags[0xb] = 0;
}

static bool cc_common_handle_special_opcode(struct comprehend_game *game,
					    uint8 operand)
{
	switch (operand) {
	case 0x03:
		/*
		 * Game over - failure.
		 *
		 * FIXME - If playing the second disk this should restart
		 *         from the beginning of the first disk.
		 */
		game_restart(game);
		break;

	case 0x06:
		game_save(game);
		break;

	case 0x07:
		/*
		 * FIXME - This will only correctly restore games that were
		 *         saved for the disk currently being played.
		 */
		game_restore(game);
		return true;
	}

	return false;
}

static void cc1_handle_special_opcode(struct comprehend_game *game,
				      uint8 operand)
{
	if (cc_common_handle_special_opcode(game, operand))
		return;

	switch (operand) {
	case 0x05:
		/*
		 * Completed first part (disk 1) of the game.
		 *
		 * FIXME - This should automatically load disk 2.
		 */
		error("[Completed disk 1 - to continue run Re-Comprehend with the 'cc2' game]");
		break;
	}
}

static void cc2_handle_special_opcode(struct comprehend_game *game,
				      uint8 operand)
{
	if (cc_common_handle_special_opcode(game, operand))
		return;

	switch (operand) {
	case 0x01:
		/* Enter the Vampire's throne room */
		eval_function(game, &game->info->functions[0xe], NULL, NULL);
		break;

	case 0x05:
		/*
		 * Won the game.
		 *
		 * FIXME - The merchant ship should arrives, etc.
		 */
		game_restart(game);
		break;
	}
}

static void cc2_before_prompt(struct comprehend_game *game)
{
	cc_clear_companion_flags(game);
}

static void cc1_before_prompt(struct comprehend_game *game)
{
	cc_clear_companion_flags(game);
}

static struct game_strings cc1_strings = { 0x9 };

static struct game_ops cc1_ops = {
	nullptr,
	cc1_before_prompt,
	nullptr,
	nullptr,
	nullptr,
	cc1_handle_special_opcode
};

static struct game_ops cc2_ops = {
	nullptr,
	cc2_before_prompt,
	nullptr,
	nullptr,
	nullptr,
	cc2_handle_special_opcode
};

struct comprehend_game game_crimson_crown_1 = {
	"Crimson Crown (Part 1/2)",
	"cc1",
	nullptr,
	"CC1.GDA",
	{ {"MA.MS1", 0x89} },
	{"RA.MS1", "RB.MS1", "RC.MS1"},
	{"OA.MS1", "OB.MS1"},
	"G%d.MS0",
	0,
	&cc1_strings,
	&cc1_ops,
	nullptr
};

struct comprehend_game game_crimson_crown_2 = {
	"Crimson Crown (Part 2/2)",
	"cc2",
	nullptr,
	"CC2.GDA",
	{ {"MA.MS2", 0x89} },
	{"RA.MS2", "RB.MS2"},
	{"OA.MS2", "OB.MS2"},
	"G%d.MS0",
	0,
	nullptr,
	&cc2_ops,
	nullptr
};

} // namespace Comprehend
} // namespace Glk
