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

#include "bladerunner/ui/kia_section_pogo.h"

#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/font.h"
#include "bladerunner/game_info.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/time.h"
#include "bladerunner/game_constants.h"

namespace BladeRunner {

const Color256 KIASectionPogo::kTextColors[] = {
	{ 0, 0, 0 },
	{ 16, 8, 8 },
	{ 32, 24, 8 },
	{ 56, 32, 16 },
	{ 72, 48, 16 },
	{ 88, 56, 24 },
	{ 104, 72, 32 },
	{ 128, 80, 40 },
	{ 136, 96, 48 },
	{ 152, 112, 56 },
	{ 168, 128, 72 },
	{ 184, 144, 88 },
	{ 200, 160, 96 },
	{ 216, 184, 112 },
	{ 232, 200, 128 },
	{ 240, 224, 144 }
};

const char *KIASectionPogo::kStrings[] = {
	"Air Conditioning",
	"Amy Shoopman",
	"Andy B. and the Milk Carton Kids",
	"Area 51",
	"Aspirin",
	"Babylon 5",
	"Bandit",
	"Bauer Inline Skates",
	"Bill Randolph",
	"Bill (Mr. Motorola) and Sarah",
	"Boo Berry and Frankenberry",
	"Brett W. Sperry",
	"Brianhead Ski Resort",
	"\"Bubba\"",
	"Bubbles",
	"Building 2 Parking",
	"The Buke",
	"Chan \"The Emporer\" Lee",
	"Cheezy Poofs",
	"Chuck \"Walter\" Karras",
	"Cinco De Mayo",
	"Club Med",
	"Code Complete",
	"Coffee Pub, Las Vegas",
	"Coke",
	"Coin Magic",
	"Count Chocula",
	"Dad",
	"David Arkenstone",
	"Digital Camera",
	"Direct X Team",
	"Denis and Joanne Dyack",
	"Blue Bayou, Disneyland",
	"Dongle-Boy",
	"Doves and Sparrows",
	"Dovey",
	"Draracles",
	"Dry Air",
	"Ed Del Castillo",
	"Eric \"Kick Ass\" Cartman",
	"FHM",
	"Fog City Diner",
	"Fog Studios",
	"Gatorade",
	"Gandhi Cuisine of India",
	"Giant Lava Lamp",
	"Eric and Nancy Gooch",
	"Grayford Family",
	"Comet Hale-Bopp",
	"Joseph B. Hewitt IV",
	"Hercules",
	"Hillbilly Jeopardy",
	"Home Cookin'",
	"Hooey Stick",
	"The Hypnotist",
	"Insects on the Move",
	"Intel",
	"James Hong",
	"Jasmine",
	"The Mysterious Cockatiel",
	"Joe's Frog",
	"\"Jed\"",
	"Jeeps",
	"\"Jeeter\"",
	"Jeff Brown",
	"JoeB",
	"\"Joe-Bob McClintock\"",
	"Joseph Turkel",
	"Jose Cuervo",
	"Juggling Balls",
	"Keith Parkinson",
	"Khan",
	"King of the Hill",
	"Kurt O. and the Toothbrush Squad",
	"Leonard and Shirley Legg",
	"\"Leroy\"",
	"Brion James",
	"Louis and his \"friend\"",
	"M.C. Crammer and Janie",
	"Men's Room Magna-Doodle",
	"Mark and Deepti Rowland",
	"Metro Pizza, Las Vegas",
	"Matt Vella",
	"Maui",
	"1 Million Candlepower Spotlight",
	"Mom",
	"Movie-makers",
	"Mr. Nonsense",
	"\"Needles\"",
	"Nerf Weaponry",
	"Nimbus",
	"Norm Vordahl",
	"KNPR",
	"Olive Garden",
	"Onkyo",
	"Orangey",
	"Osbur, the Human Resource Manager",
	"Our Cheery Friend Leary",
	"Ousted Gnome King",
	"Pepsi",
	"Peta Wilson",
	"Pogo the Mockingbird",
	"Poker Nights",
	"Pirates",
	"Playmate Lingerie Calendar",
	"Pop-Ice",
	"Powerhouse Gym",
	"Rade McDowell",
	"Red Rock Canyon",
	"Refrigeration",
	"Rhoda",
	"Richard and Kimberly Weier",
	"Ridley Scott",
	"Ruud the Dude",
	"Our old pal Rick Parks",
	"Ruby's Diner",
	"Savatage",
	"Scully and Mulder",
	"Sean Young",
	"Seinfeld",
	"The Shadow",
	"\"Shakes\"",
	"Shorts",
	"Silly Putty",
	"The Simpsons",
	"Thomas Christensen",
	"We love you Steve Wetherill!!!",
	"\"Skank\"",
	"\"Slice\"",
	"SSG",
	"Steve and Anne Tall",
	"South Park",
	"Snap 'n Pops",
	"Sneaker",
	"Star Wars Trilogy",
	"Nonstop Summer Pool Parties",
	"Sunsets",
	"T-Bone and Angie",
	"T-shirts",
	"Julio Schembari, Tango Pools",
	"The Thermostat Key",
	"The Wizard",
	"Tomb Raider",
	"Tom Elmer II",
	"Tujia Linden",
	"Turbo",
	"Tweeter",
	"Twonky",
	"Ty and Judy Coon",
	"The Courtyard",
	"U.F.C.",
	"Uli Boehnke",
	"\"Virgil\"",
	"Virtual Boy",
	"Westwood Offroad Excursion Team",
	"William Sanderson",
	"Xena",
	"Zion National Park"
};

KIASectionPogo::KIASectionPogo(BladeRunnerEngine *vm) : KIASectionBase(vm) {
	_stringIndex = 0;
	_timeLast    = 0;

	for (int i = 0; i < kStringCount; ++i) {
		_strings[i] = nullptr;
	}

	for (int i = 0; i < kLineCount; ++i) {
		_lineTexts[i]    = nullptr;
		_lineTimeouts[i] = 0;
		_lineOffsets[i]  = 0;
	}
}

void KIASectionPogo::open() {
	_stringIndex = 0;
	for (int i = 0; i < kStringCount; ++i) {
		_strings[i] = kStrings[i];
	}

	for (int i = 0; i < kStringCount; ++i) {
		int j = _vm->_rnd.getRandomNumberRng(i, kStringCount - 1);
		SWAP(_strings[i], kStrings[j]);
	}

	for (int i = 0; i < kLineCount; ++i) {
		_lineTexts[i] = nullptr;
		_lineTimeouts[i] = _vm->_rnd.getRandomNumberRng(0, 63);
		_lineOffsets[i] = 0;
	}

	_timeLast = _vm->_time->currentSystem();

	_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxAUDLAFF1), 100, 0, 0, 50, 0);
}

void KIASectionPogo::draw(Graphics::Surface &surface) {
	// Timing fixed for 60Hz by ScummVM team
	uint32 timeNow = _vm->_time->currentSystem();
	bool updateTimeout = false;
	// unsigned difference is intentional
	if (timeNow - _timeLast > (1000u / 60u)) {
		updateTimeout = true;
		_timeLast = timeNow;
	}

	const char *title = "We 3 coders give special thanks to:";
	_vm->_mainFont->drawString(&surface, title, 313 - _vm->_mainFont->getStringWidth(title) / 2, 143, surface.w, surface.format.RGBToColor(240, 232, 192));

	int y = 158;
	int lineTextWidth;
	for (int i = 0; i < kLineCount; ++i) {
		if (updateTimeout) {
			if (_lineTimeouts[i] > 0) {
				--_lineTimeouts[i];
			} else {
				_lineTexts[i] = _strings[_stringIndex];
				_lineTimeouts[i] = 63;
				 lineTextWidth = _vm->_mainFont->getStringWidth(_lineTexts[i]);
				_lineOffsets[i] = _vm->_rnd.getRandomNumberRng(0, (306 -  lineTextWidth) > 0 ? (306 - lineTextWidth) : 0) + 155;

				_stringIndex = (_stringIndex + 1) % kStringCount;
			}
		}

		if (_lineTexts[i]) {
			int colorIndex = _lineTimeouts[i];
			if (colorIndex >= 32) {
				colorIndex = 63 - colorIndex;
			}
			colorIndex /= 2;
			_vm->_mainFont->drawString(&surface, _lineTexts[i], _lineOffsets[i], y, surface.w, surface.format.RGBToColor(kTextColors[colorIndex].r, kTextColors[colorIndex].g, kTextColors[colorIndex].b));
		}
		y += 10;
	}
}

} // End of namespace BladeRunner
