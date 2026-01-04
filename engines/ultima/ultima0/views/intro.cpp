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

#include "ultima/ultima0/views/intro.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

bool Intro::msgFocus(const FocusMessage &msg) {
	_page = 0;
	return true;
}

void Intro::draw() {
	auto s = getSurface();
	s.clear();

	switch (_page) {
	case 0:
		s.writeString(Common::Point(5, 1), "Many, many, many years ago the");
		s.writeString(Common::Point(0, 3), "Dark Lord Mondain, Archfoe of British,");
		s.writeString(Common::Point(0, 5), "traversed the lands of Akalabeth,");
		s.writeString(Common::Point(0, 7), "spreading evil and death as he passed.");
		s.writeString(Common::Point(0, 9), "By the time Mondain was driven from the");
		s.writeString(Common::Point(0, 11), "land by British, bearer of the White");
		s.writeString(Common::Point(0, 13), "Light, he had done much damage unto");
		s.writeString(Common::Point(0, 15), "the lands.");
		s.writeString(Common::Point(0, 17), "`Tis thy duty to help rid Akalabeth of");
		s.writeString(Common::Point(0, 19), "the foul beasts which infest it,");
		s.writeString(Common::Point(0, 21), "while trying to stay alive!!!");
		break;

	case 1:
		s.writeString(Common::Point(0,  0), "The Player's Stats:");
		s.writeString(Common::Point(0,  2), "Hit Points- Amount of Damage a Player");
		s.writeString(Common::Point(0,  4), "            can absorb before Death");
		s.writeString(Common::Point(0,  6), "Strength--- Related to Damage Inflicted");
		s.writeString(Common::Point(0,  8), "            by Player against Monsters.");
		s.writeString(Common::Point(0, 10), "Dexterity-- Related to the Probability");
		s.writeString(Common::Point(0, 12), "            of a Player hitting a Monst.");
		s.writeString(Common::Point(0, 14), "Stamina---- Related to Player Defense");
		s.writeString(Common::Point(0, 16), "            against Monsters");
		s.writeString(Common::Point(0, 18), "Wisdom----- This Attribute is used");
		s.writeString(Common::Point(0, 20), "            in Special (Quest!) Routines");
		s.writeString(Common::Point(0, 22), "Gold------- Money!! Cash!! Assets!!");
		break;

	case 2:
		s.writeString(Common::Point(0,  0), "The Towns and Buying Items:");
		s.writeString(Common::Point(0,  2), "     To buy any items one need only");
		s.writeString(Common::Point(0,  4), "type the first letter of the item");
		s.writeString(Common::Point(0,  6), "wanted. The cost of the respective");
		s.writeString(Common::Point(0,  8), "items is displayed while in the town.");
		s.writeString(Common::Point(0, 10), "The Game is started in a town somewhere");
		s.writeString(Common::Point(0, 12), "on the 20x20 map");
		s.writeString(Common::Point(20, 14), "Fighters and Magi", Graphics::kTextAlignCenter);
		s.writeString(Common::Point(0, 16), "     The disadvantage of being a");
		s.writeString(Common::Point(0, 18), "fighter is the lack of the ability to");
		s.writeString(Common::Point(0, 20), "control the magic amulet, whereas magi");
		s.writeString(Common::Point(0, 22), "can not use rapier and bows.");
		break;

	case 3:
		s.writeString(Common::Point(1,  0), "Movement:");
		s.writeString(Common::Point(1,  2), "-Key-     Outdoors      Dungeon");
		s.writeString(Common::Point(1,  4), "  UP      Move North    Move Forward");
		s.writeString(Common::Point(1,  6), " LEFT     Move West     Turn Left");
		s.writeString(Common::Point(1,  8), "RIGHT     Move East     Turn Right");
		s.writeString(Common::Point(1, 10), " DOWN     Move South    Turn Around");
		s.writeString(Common::Point(1, 12), "  Z       Statistics    Statistics");
		s.writeString(Common::Point(1, 14), "  A       N/A           Attack");
		s.writeString(Common::Point(1, 16), "  P       Pause         Pause");
		s.writeString(Common::Point(1, 18), "  E       Go Into Town  Climb Ladder");
		s.writeString(Common::Point(1, 20), "  E       Go Castle     Go Hole");
		s.writeString(Common::Point(1, 22), "SPACE     Pass          Pass");
		break;

	case 4:
		s.writeString(Common::Point(0, 2), "     Thou doest know the basics of");
		s.writeString(Common::Point(0, 4), "the game, experiment with the commands.");
		s.writeString(Common::Point(0, 6), "There is much left unsaid for");
		s.writeString(Common::Point(0, 8), "thee to discover in the future...");
		s.writeString(Common::Point(0, 10), "Go now unto the world and seek");
		s.writeString(Common::Point(0, 12), "adventure where thou might!!!");
		s.writeString(Common::Point(0, 14), "P.S.-Search out the Castle of");
		s.writeString(Common::Point(0, 16), "Lord British, Use the -E- Key to go in!");

	default:
		break;
	}

	s.writeString(Common::Point(20, (_page == 4) ? 18 : 24), "Press any Key to Continue",
		Graphics::kTextAlignCenter);
}

void Intro::nextPage() {
	if (++_page < 5)
		redraw();
	else
		replaceView("Title");
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
