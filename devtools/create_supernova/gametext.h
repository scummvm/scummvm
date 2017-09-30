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
 * This is a utility for generating a data file for the supernova engine.
 * It contains strings extracted from the original executable as well
 * as translations and is required for the engine to work properly.
 */

#ifndef GAMETEXT_H
#define GAMETEXT_H

#include <stddef.h>

// This file contains the strings in German and is encoded using CP850 encoding.
// Other language should be provided as po files also using the CP850 encoding.

// TODO: add the strings from the engine here, add an Id string in comment.
// And in the engine add a StringId enum with all the Ids = index in this array.

const char *gameText[] = {
	// 0
	"Gehe",         // kCommandGo
	"Schau",        // kCommandLook
	"Nimm",         // kCommandTake
	"\231ffne",     // kCommandOpen
	"Schlie\341e",  // kCommandClose
	// 5
	"Dr\201cke",    // kCommandPress
	"Ziehe",        // kCommandPull
	"Benutze",      // kCommandUse
	"Rede",         // kCommandTalk
	"Gib",          // kCommandGive
	// 10
	"Gehe zu ",     // kStatusCommandGo
	"Schau ",       // kStatusCommandLook
	"Nimm ",        // kStatusCommandTake
	"\231ffne ",    // kStatusCommandOpen
	"Schlie\341e ", // kStatusCommandClose
	// 15
	"Dr\201cke ",   // kStatusCommandPress
	"Ziehe ",       // kStatusCommandPull
	"Benutze ",     // kStatusCommandUse
	"Rede mit ",    // kStatusCommandTalk
	"Gib ",         // kStatusCommandGive
	// 20
	"Hmm, er scheint kaputt zu sein.",       // kBroken
	"^(C) 1994 Thomas und Steffen Dingel#",  // kIntro1
	"Story und Grafik:^ Thomas Dingel#",     // kIntro2
	"Programmierung:^ Steffen Dingel#",      // kIntro3
	"Musik:^ Bernd Hoffmann#",               // kIntro4
	// 25
	"Getestet von ...#",                     // kIntro5
	"^Matthias Neef#",                       // kIntro6
	"^Sascha Otterbach#",                    // kIntro7
	"^Thomas Mazzoni#",                      // kIntro8
	"^Matthias Klein#",                      // kIntro9
	// 30
	"^Gerrit Rothmaier#",                    // kIntro10
	"^Thomas Hassler#",                      // kIntro11
	"^Rene Koch#",                           // kIntro12
	"°",                                     // kIntro13
	"Keycard",                               // kKeycard
	// 35
	"Die Keycard f\224r deine Schr\204nke.", // kKeycardDesc
	"Taschenmesser",                         // kKnife
	"Es ist nicht mehr das sch\204rfste.",   // kKnifeDesc
	"Armbanduhr",                            // kWatch
	"Discman",                               // kDiscman
	// 40
	"Es ist eine \"Mad Monkeys\"-CD darin.", // kDiscmanDesc
	"", //
	"", //
	"", //
	"", //
	// 45
	"", //
	"", //
	"", //
	"", //
	"", //
	// 50
	"", //
	"", //
	"", //
	"", //
	"", //
	// 55
	"", //
	"", //
	"", //
	"", //
	"", //
	// 60
	"", //
	"", //
	"", //
	"", //
	"", //
	// 65
	"", //
	"", //
	"", //
	"", //
	"", //
	// 70
	"", //
	"", //
	"", //
	"", //
	"", //
	// 75
	"", //
	"", //
	"", //
	"", //
	"", //
	// 80
	"", //
	"", //
	"", //
	"", //
	"", //
	// 85
	"", //
	"", //
	"", //
	"", //
	"", //
	// 90
	"", //
	"", //
	"", //
	"", //
	"", //
	// 95
	"", //
	"", //
	"", //
	"", //
	"", //
	// 100
	"", //
	"", //
	"", //
	"", //
	"", //
	// 105
	"", //
	"", //
	"", //
	"", //
	"", //
	// 110
	"", //
	"", //
	"", //
	"", //
	"", //
	// 115
	"", //
	"", //
	"", //
	"", //
	"", //
	// 120
	"", //
	"", //
	"", //
	"", //
	"", //
	// 125
	"", //
	"", //
	"", //
	"", //
	"", //
	// 130
	"", //
	"", //
	"", //
	"", //
	"", //
	// 135
	"", //
	"", //
	"", //
	"", //
	"", //
	// 140
	"", //
	"", //
	"", //
	"", //
	"", //
	// 145
	"", //
	"", //
	"", //
	"", //
	"", //
	// 150
	"", //
	"", //
	"", //
	"", //
	"", //
	// 155
	"", //
	"", //
	"", //
	"", //
	"", //
	// 160
	"", //
	"", //
	"", //
	"", //
	"", //
	// 165
	"", //
	"", //
	"", //
	"", //
	"", //
	// 170
	"", //
	"", //
	"", //
	"", //
	"", //
	// 175
	"", //
	"", //
	"", //
	"", //
	"", //
	// 180
	"", //
	"", //
	"", //
	"", //
	"", //
	// 185
	"", //
	"", //
	"", //
	"", //
	"", //
	NULL
};



#endif // GAMETEXT_H
