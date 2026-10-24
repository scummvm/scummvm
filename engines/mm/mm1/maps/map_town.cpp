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

#include "mm/mm1/maps/map_town.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"
#include "mm/mm1/utils/strings.h"

namespace MM {
namespace MM1 {
namespace Maps {

struct TownText {
	const char *_classic;
	const char *_enhanced;
};

static const TownText BLACKSMITH_TEXT[5] = {
	{
		"A man wearing a leather apron speaks:\n"
			"\"Distinguished travelers, you've come to\n"
			"the right place. Can I help you (Y/N)?\"",
		nullptr
	},
	{
		"Busily forging a long sword,an extremely\n"
			"beautiful young girl says with a\n"
			"seductive smile, \"Can i help you (Y/N)?\"",
		"Busily forging a long sword, an extremely beautiful young girl says with a seductive "
			"smile, \"Can i help you (Y/N)?\""
	},
	{
		"A man enclosed in a steel cage says,\n"
			"\"Can i help you find anything (Y/N)?\"",
		nullptr
	},
	{
		"A young gnome materializes, asking,\n"
			"\"Would you be wanting my services(Y/N)?\"",
		"A young gnome materializes, asking,\n"
			"\"Would you be wanting my services (Y/N)?\""
	},
	{
		"A boistrous half-orc proclaims,\n"
			"\"We carry the very latest,\n"
			"browse (Y/N)?\"",
		nullptr
	}
};

static const TownText INN_TEXT[5] = {
	{
		"The innekeeper asks: \"Would you like to sign in (Y/N)?\"",
		nullptr
	},
	{
		"Lara, the inkeepers voluptuous\n"
			"daughter, asks if you will be spending\n"
			"the night... sign in (Y/N)?",
		nullptr
	},
	{
		"Standing behind a large desk, a tall\n"
			"bearded half-orc grunts, \"If you don't\n"
			"sign in, you can't stay! Well (Y/N)?\"",
		nullptr
	},
	{
		"An eerie silence pervades the deserted\n"
			"inn. suddenly, a ghostly apparition\n"
			"appears, asking, \"Sign in (Y/N)?\"",
		nullptr
	},
	{
		"Adjusting his spectacles, an elderly\n"
			"gnome eyes the party and asks,\n"
			"\"Signing in (Y/N)?\"",
		nullptr
	}
};

static const TownText MARKET_TEXT[5] = {
	{
		"Behind the counter, an overweight dwarf\n"
			"exclaims: \"You look like a hungry bunch!\n"
			"Would you like to buy some food (Y/N)?\"",
		nullptr
	},
	{
		"From behind the fruit stand\n"
			"a sultry voice asks, \"See anything\n"
			"you like (Y/N)?\"",
		nullptr
	},
	{
		"A putrid smell pervades the room...\n"
			"look for some food (Y/N)?",
		nullptr
	},
	{
		"A faint outline of a man appears,\n"
			"saying, \"Need some feed (Y/N)?\"",
		nullptr
	},
	{
		"The chef says,\"I only serve the best!\n"
			"Hungry (Y/N)?\"",
		nullptr
	}
};

static const TownText TEMPLE_TEXT[5] = {
	{
		"Several ornately robed clerics approach\n"
			"the party and ask, \"Do you seek our help (Y/N)?\"",
		nullptr
	},
	{
		"The once magnificent temple shows\n"
			"signs of neglect and decay. The high\n"
			"priestess asks, \"Need help (Y/N)?\"",
		nullptr
	},
	{
		"An ancient looking cleric staggers up to\n"
			"the party gasping, \"I can help,\n"
			"really i can!\" Accept the offer (Y/N)?",
		nullptr
	},
	{
		"Bright lights swirl throughout the room.A horrifying voice shrieks,\n"
			"\"Need help (Y/N)?\"",
		"Bright lights swirl throughout the room. A horrifying voice shrieks,\n"
			"\"Need help (Y/N)?\""
	},
	{
		"Several fat clerics welcome you.\n"
			"Need help (Y/N)?",
		nullptr
	}
};

static const TownText TRAINING_TEXT[5] = {
	{
		"Before you are various groups engaged in\n"
			"training exercises. Worg,the guildmaster\n"
			"asks, \"Do you require training (Y/N)?\"",
		"Before you are various groups engaged in training exercises. Worg, the guildmaster asks, "
			"\"Do you require training (Y/N)?\""
	},
	{
		"A tall, muscular female clad in leather\n"
			"armor approaches asking, \"Strangers,\n"
			"are you here for training (Y/N)?\"",
		nullptr
	},
	{
		"several adventurers armored in dragon\n"
			"scales, address the party,\n"
			"\"Looking for training (Y/N)?\"",
		nullptr
	},
	{
		"Several empty suits of armor approach\n"
			"asking, \"Need training (Y/N)?\"",
		nullptr
	},
	{
		"Three huge armor clad knights bark in\n"
			"unison, \"Want a good workout (Y/N)?\"",
		nullptr
	}
};

static Common::String townText(const TownText &text) {
	if (g_engine->isEnhanced() && text._enhanced)
		return text._enhanced;

	return mapText(text._classic);
}

void MapTown::blacksmith() {
	visitedBusiness();

	send(SoundMessage(
		townText(BLACKSMITH_TEXT[_mapIndex]),
		[]() {
			g_events->send("Blacksmith", GameMessage("DISPLAY"));
		}
	));
}

void MapTown::inn() {
	visitedBusiness();

	send(SoundMessage(
		townText(INN_TEXT[_mapIndex]),
		[]() {
			g_events->replaceView("Inn", true);
		}
	));
}

void MapTown::market() {
	visitedBusiness();

	send(SoundMessage(
		townText(MARKET_TEXT[_mapIndex]),
		[]() {
			g_events->send("Market", GameMessage("DISPLAY"));
		}
	));
}

void MapTown::tavern() {
	visitedBusiness();

	send(SoundMessage(
		"Step up to the bar (Y/N)?",
		[]() {
			g_events->send("Tavern", GameMessage("DISPLAY"));
		}
	));
}

void MapTown::temple() {
	visitedBusiness();

	send(SoundMessage(
		townText(TEMPLE_TEXT[_mapIndex]),
		[]() {
			g_events->send("Temple", GameMessage("DISPLAY"));
		}
	));
}

void MapTown::training() {
	visitedBusiness();

	send(SoundMessage(
		townText(TRAINING_TEXT[_mapIndex]),
		[]() {
			g_events->send("Training", GameMessage("DISPLAY"));
		}
	));
}

void MapTown::showSign(const Common::String &msg) {
	send(SoundMessage(
		2, 1, mapText("A sign above the door reads:\n"),
		6, 2, msg
	));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
