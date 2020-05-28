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
#include "glk/comprehend/util.h"

namespace Glk {
namespace Comprehend {

struct tr_monster {
	uint8		object;
	uint8		dead_flag;
	unsigned	min_turns_before;
	unsigned	room_allow_flag;
	unsigned	randomness;
};

static struct tr_monster tr_werewolf = {
	0x21, 7, (1 << 6), 5, 5
};

static struct tr_monster tr_vampire = {
	0x26, 5, (1 << 7), 0, 5
};

static void tr_update_monster(struct comprehend_game *game,
			      struct tr_monster *monster_info)
{
	struct item *monster;
	struct room *room;
	uint16 turn_count;

	room = &game->info->rooms[game->info->current_room];
	turn_count = game->info->variable[VAR_TURN_COUNT];

	monster = get_item(game, monster_info->object);
	if (monster->room == game->info->current_room) {
		/* The monster is in the current room - leave it there */
		return;
	}

	if ((room->flags & monster_info->room_allow_flag) &&
	    !game->info->flags[monster_info->dead_flag] &&
	    turn_count > monster_info->min_turns_before) {
		/*
		 * The monster is alive and allowed to move to the current
		 * room. Randomly decide whether on not to. If not, move
		 * it back to limbo.
		 */
		if ((g_comprehend->getRandomNumber(0x7fffffff) % monster_info->randomness) == 0) {
			move_object(game, monster, game->info->current_room);
			game->info->variable[0xf] = turn_count + 1;
		} else {
			move_object(game, monster, ROOM_NOWHERE);
		}
	}
}

static int tr_room_is_special(struct comprehend_game *game, unsigned room_index,
			      unsigned *room_desc_string)
{
	struct room *room = &game->info->rooms[room_index];

	if (room_index == 0x28) {
		if (room_desc_string)
			*room_desc_string = room->string_desc;
		return ROOM_IS_DARK;
	}

	return ROOM_IS_NORMAL;
}

static bool tr_before_turn(struct comprehend_game *game)
{
	tr_update_monster(game, &tr_werewolf);
	tr_update_monster(game, &tr_vampire);
	return false;
}

static void tr_handle_special_opcode(struct comprehend_game *game,
				     uint8 operand)
{
	switch (operand) {
	case 0x01:
		/*
		 * FIXME - Called when the mice are dropped and the cat chases
		 *         them.
		 */
		break;

	case 0x02:
		/* FIXME - Called when the gun is fired */
		break;

	case 0x06:
		game_save(game);
		break;

	case 0x07:
		game_restore(game);
		break;

	case 0x03:
		/* Game over - failure */
	case 0x05:
		/* Won the game */
	case 0x08:
		/* Restart game */
		game_restart(game);
		break;

	case 0x09:
		/*
		 * Show the Zin screen in reponse to doing 'sing some enchanted
		 * evening' in his cabin.
		 */
		draw_location_image(&game->info->room_images, 41);
		console_get_key();
		game->info->update_flags |= UPDATE_GRAPHICS;
		break;
	}
}

static void read_string(char *buffer, size_t size)
{
#ifdef TODO
	char *p;

	printf("> ");
	fgets(buffer, size, stdin);

	/* Remove trailing newline */
	p = strchr(buffer, '\n');
	if (p)
		*p = '\0';
#else
	error("TODO");
#endif
}

static void tr_before_game(struct comprehend_game *game)
{
	char buffer[128];

	/* Welcome to Transylvania - sign your name */
	console_println(game, game->info->strings.strings[0x20]);
	read_string(buffer, sizeof(buffer));

	/*
	 * Transylvania uses replace word 0 as the player's name, the game
	 * data file stores a bunch of dummy characters, so the length is
	 * limited (the original game will break if you put a name in that
	 * is too long).
	 */
	if (!game->info->replace_words[0])
		game->info->replace_words[0] = xstrndup(buffer, strlen(buffer));
	else
		snprintf(game->info->replace_words[0],
			 strlen(game->info->replace_words[0]),
			 "%s", buffer);

	/* And your next of kin - This isn't store by the game */
	console_println(game, game->info->strings.strings[0x21]);
	read_string(buffer, sizeof(buffer));
}

static struct game_strings tr_strings = {
	EXTRA_STRING_TABLE(0x8a)
};

static struct game_ops tr_ops = {
	tr_before_game,
	nullptr,
	tr_before_turn,
	nullptr,
	tr_room_is_special,
	tr_handle_special_opcode,
};

struct comprehend_game game_transylvania = {
	"Transylvania",
	"tr",
	"TR.GDA",
	{
		{"MA.MS1", 0x88},
		{"MB.MS1", 0x88},
		{"MC.MS1", 0x88},
		{"MD.MS1", 0x88},
		{"ME.MS1", 0x88},
	},
	{"RA.MS1", "RB.MS1", "RC.MS1"},
	{"OA.MS1", "OB.MS1", "OC.MS1"},
	"G%d.MS0",
	0,
	&tr_strings,
	&tr_ops,
	nullptr
};

} // namespace Comprehend
} // namespace Glk
