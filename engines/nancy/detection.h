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

#ifndef NANCY_DETECTION_H
#define NANCY_DETECTION_H

#include "engines/advancedDetector.h"

namespace Nancy {

enum GameType {
	kGameTypeNone 	 = 0,
	kGameTypeVampire = 1,	// The Vampire Diaries - 1996-11-01
	kGameTypeNancy1  = 2,	// Secrets Can Kill - 1998-11-05
	kGameTypeNancy2  = 3,	// Stay Tuned for Danger - 1999-11-13
	kGameTypeNancy3  = 4,	// Message in a Haunted Mansion - 2000-11-24
	kGameTypeNancy4  = 5,	// Treasure in the Royal Tower - 2001-08-01
	kGameTypeNancy5  = 6,	// The Final Scene - 2001-11-01
	kGameTypeNancy6  = 7,	// Secret of the Scarlet Hand - 2002-08-12
	kGameTypeNancy7  = 8,	// Ghost Dogs of Moon Lake - 2002-11-01
	kGameTypeNancy8  = 9,	// The Haunted Carousel - 2003-08-22
	kGameTypeNancy9  = 10,	// Danger on Deception Island - 2003-10-03
	kGameTypeNancy10 = 11,	// The Secret of Shadow Ranch - 2004-08-30
	kGameTypeNancy11 = 12,	// Curse of Blackmoor Manor - 2004-10-05
	kGameTypeNancy12 = 13,	// Secret of the Old Clock - 2005-06-12
	kGameTypeNancy13 = 14,	// Last Train to Blue Moon Canyon - 2005-09-15
	kGameTypeNancy14 = 15,	// Danger by Design - 2006-07-24
	kGameTypeNancy15 = 16,	// The Creature of Kapu Cave - 2006-10-01
	kGameTypeNancy16 = 17,	// The White Wolf of Icicle Creek - 2007-06-07
	kGameTypeNancy17 = 18,	// Legend of the Crystal Skull - 2007-10-08
	kGameTypeNancy18 = 19,	// The Phantom of Venice - 2008-07-11
	kGameTypeNancy19 = 20,	// The Haunting of Castle Malloy - 2008-10-16
	kGameTypeNancy20 = 21,	// Ransom of the Seven Ships - 2009-07-14 (discontinued)
	kGameTypeNancy21 = 22,	// Warnings at Waverly Academy - 2009-10-13
	kGameTypeNancy22 = 23,	// Trail of the Twister - 2010-06-29
	kGameTypeNancy1New = 24,// Secrets Can Kill Remastered - 2010-08-24
	kGameTypeNancy23 = 25,	// Shadow at the Water's Edge - 2010-10-19
	kGameTypeNancy24 = 26,	// The Captive Curse - 2011-06-28
	kGameTypeNancy25 = 27,	// Alibi in Ashes - 2011-10-18
	kGameTypeNancy26 = 28,	// Tomb of the Lost Queen - 2012-05-08
	kGameTypeNancy27 = 29,	// The Deadly Device - 2012-10-23
	kGameTypeNancy28 = 30,	// Ghost of Thornton Hall - 2013-05-14
	kGameTypeNancy29 = 31,	// The Silent Spy - 2013-10-22
	kGameTypeNancy30 = 32,	// The Shattered Medallion - 2014-05-20
	kGameTypeNancy31 = 33,	// Labyrinth of Lies - 2014-10-14
	kGameTypeNancy32 = 34,	// Sea of Darkness - 2015-05-19
	// Midnight in Salem was made with Unity
	// Mystery of the Seven Keys was made with Unity
};

enum NancyGameFlags {
	GF_COMPRESSED 		= 1 << 0
};

struct NancyGameDescription {
	AD_GAME_DESCRIPTION_HELPERS(desc);

	ADGameDescription desc;
	GameType gameType;
};

enum NancyDebugChannels {
	kDebugEngine = 1,
	kDebugActionRecord,
	kDebugScene,
	kDebugSound,
	kDebugVideo,
	kDebugHypertext,
};

// Settings found in the original engine
#define GAMEOPTION_PLAYER_SPEECH		GUIO_GAMEOPTIONS1
#define GAMEOPTION_CHARACTER_SPEECH		GUIO_GAMEOPTIONS2
#define GAMEOPTION_AUTO_MOVE			GUIO_GAMEOPTIONS3

// Patch settings, general
#define GAMEOPTION_FIX_SOFTLOCKS		GUIO_GAMEOPTIONS4
#define GAMEOPTION_FIX_ANNOYANCES		GUIO_GAMEOPTIONS5

// Patch settings, specific to each game
#define GAMEOPTION_NANCY2_TIMER			GUIO_GAMEOPTIONS6

#define GAMEOPTION_ORIGINAL_SAVELOAD	GUIO_GAMEOPTIONS7

} // End of namespace Nancy

#endif // NANCY_DETECTION_H
