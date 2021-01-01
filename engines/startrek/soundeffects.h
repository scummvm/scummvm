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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARTREK_SOUNDEFFECTS_H
#define STARTREK_SOUNDEFFECTS_H


// Sound effects for "playSoundEffectIndex" function
enum SoundEffects {
	// Sounds 0 - 3 are blank
	kSfxTricorder = 4,
	kSfxDoor = 5,
	kSfxPhaser = 6,
	kSfxButton = 7,
	kSfxTransporterDematerialize = 8,
	kSfxTransporterMaterialize = 9,
	kSfxTransporterEnergize = 10,
	// Sounds 11 - 15 are blank
	kSfxSelection = 16,
	// Sounds 17 - 33 are blank
	kSfxHailing = 34,
	// Sound 35 is blank
	kSfxPhaser2 = 36,
	kSfxPhotonTorpedoes = 37,
	kSfxShieldHit = 38,
	kSfxUnk = 39,
	kSfxRedAlert = 40,
	kSfxWarp = 41
};


enum MidiTracks {
	MIDITRACK_0,
	MIDITRACK_1,
	MIDITRACK_2,
	MIDITRACK_3,
	MIDITRACK_4,
	MIDITRACK_5,
	MIDITRACK_6,
	MIDITRACK_7,
	MIDITRACK_8,
	MIDITRACK_9,
	MIDITRACK_10,
	MIDITRACK_11,
	MIDITRACK_12,
	MIDITRACK_13,
	MIDITRACK_14,
	MIDITRACK_15,
	MIDITRACK_16,
	MIDITRACK_17,
	MIDITRACK_18,
	MIDITRACK_19,
	MIDITRACK_20,
	MIDITRACK_21,
	MIDITRACK_22,
	MIDITRACK_23,
	MIDITRACK_24,
	MIDITRACK_25,
	MIDITRACK_26,
	MIDITRACK_27,
	MIDITRACK_28,
	MIDITRACK_29,
	MIDITRACK_30,
	MIDITRACK_31,
	MIDITRACK_32,
	MIDITRACK_33
};

#endif
