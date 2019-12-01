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

#include "ultima8/misc/pent_include.h"
#include "FMOplMidiDriver.h"

#ifdef USE_FMOPL_MIDI

#include "XMidiEvent.h"

#ifdef PENTAGRAM_IN_EXULT
#include "databuf.h"
#else
#include "ultima8/filesys/idata_source.h"
#include "ultima8/games/game_data.h"
#include "MusicFlex.h"
#endif

#include <cmath>

namespace Ultima8 {

const MidiDriver::MidiDriverDesc FMOplMidiDriver::desc =
    MidiDriver::MidiDriverDesc("FMOpl", createInstance);

//#define LUCAS_MODE

// We fudge with the velocity. If it's 16, it complies to AIL 2.0 Specs
#define VEL_FUDGE 2

/* This is the internal emulated MIDI driver using the included OPL2 sound chip
 * FM instrument definitions below borrowed from the Allegro library by
 * Phil Hassey, <philhassey@hotmail.com> (www.imitationpickles.org)
 */

const unsigned char FMOplMidiDriver::midi_fm_instruments_table[128][11] = {
	/* This set of GM instrument patches was provided by Jorrit Rouwe...
	 */
	{0x21, 0x21, 0x8f, 0x0c, 0xf2, 0xf2, 0x45, 0x76, 0x00, 0x00, 0x08}, /* Acoustic Grand */
	{0x31, 0x21, 0x4b, 0x09, 0xf2, 0xf2, 0x54, 0x56, 0x00, 0x00, 0x08}, /* Bright Acoustic */
	{0x31, 0x21, 0x49, 0x09, 0xf2, 0xf2, 0x55, 0x76, 0x00, 0x00, 0x08}, /* Electric Grand */
	{0xb1, 0x61, 0x0e, 0x09, 0xf2, 0xf3, 0x3b, 0x0b, 0x00, 0x00, 0x06}, /* Honky-Tonk */
	{0x01, 0x21, 0x57, 0x09, 0xf1, 0xf1, 0x38, 0x28, 0x00, 0x00, 0x00}, /* Electric Piano 1 */
	{0x01, 0x21, 0x93, 0x09, 0xf1, 0xf1, 0x38, 0x28, 0x00, 0x00, 0x00}, /* Electric Piano 2 */
	{0x21, 0x36, 0x80, 0x17, 0xa2, 0xf1, 0x01, 0xd5, 0x00, 0x00, 0x08}, /* Harpsichord */
	{0x01, 0x01, 0x92, 0x09, 0xc2, 0xc2, 0xa8, 0x58, 0x00, 0x00, 0x0a}, /* Clav */
	{0x0c, 0x81, 0x5c, 0x09, 0xf6, 0xf3, 0x54, 0xb5, 0x00, 0x00, 0x00}, /* Celesta */
	{0x07, 0x11, 0x97, 0x89, 0xf6, 0xf5, 0x32, 0x11, 0x00, 0x00, 0x02}, /* Glockenspiel */
	{0x17, 0x01, 0x21, 0x09, 0x56, 0xf6, 0x04, 0x04, 0x00, 0x00, 0x02}, /* Music Box */
	{0x18, 0x81, 0x62, 0x09, 0xf3, 0xf2, 0xe6, 0xf6, 0x00, 0x00, 0x00}, /* Vibraphone */
	{0x18, 0x21, 0x23, 0x09, 0xf7, 0xe5, 0x55, 0xd8, 0x00, 0x00, 0x00}, /* Marimba */
	{0x15, 0x01, 0x91, 0x09, 0xf6, 0xf6, 0xa6, 0xe6, 0x00, 0x00, 0x04}, /* Xylophone */
	{0x45, 0x81, 0x59, 0x89, 0xd3, 0xa3, 0x82, 0xe3, 0x00, 0x00, 0x0c}, /* Tubular Bells */
	{0x03, 0x81, 0x49, 0x89, 0x74, 0xb3, 0x55, 0x05, 0x01, 0x00, 0x04}, /* Dulcimer */
	{0x71, 0x31, 0x92, 0x09, 0xf6, 0xf1, 0x14, 0x07, 0x00, 0x00, 0x02}, /* Drawbar Organ */
	{0x72, 0x30, 0x14, 0x09, 0xc7, 0xc7, 0x58, 0x08, 0x00, 0x00, 0x02}, /* Percussive Organ */
	{0x70, 0xb1, 0x44, 0x09, 0xaa, 0x8a, 0x18, 0x08, 0x00, 0x00, 0x04}, /* Rock Organ */
	{0x23, 0xb1, 0x93, 0x09, 0x97, 0x55, 0x23, 0x14, 0x01, 0x00, 0x04}, /* Church Organ */
	{0x61, 0xb1, 0x13, 0x89, 0x97, 0x55, 0x04, 0x04, 0x01, 0x00, 0x00}, /* Reed Organ */
	{0x24, 0xb1, 0x48, 0x09, 0x98, 0x46, 0x2a, 0x1a, 0x01, 0x00, 0x0c}, /* Accoridan */
	{0x61, 0x21, 0x13, 0x09, 0x91, 0x61, 0x06, 0x07, 0x01, 0x00, 0x0a}, /* Harmonica */
	{0x21, 0xa1, 0x13, 0x92, 0x71, 0x61, 0x06, 0x07, 0x00, 0x00, 0x06}, /* Tango Accordian */
	{0x02, 0x41, 0x9c, 0x89, 0xf3, 0xf3, 0x94, 0xc8, 0x01, 0x00, 0x0c}, /* Acoustic Guitar(nylon) */
	{0x03, 0x11, 0x54, 0x09, 0xf3, 0xf1, 0x9a, 0xe7, 0x01, 0x00, 0x0c}, /* Acoustic Guitar(steel) */
	{0x23, 0x21, 0x5f, 0x09, 0xf1, 0xf2, 0x3a, 0xf8, 0x00, 0x00, 0x00}, /* Electric Guitar(jazz) */
	{0x03, 0x21, 0x87, 0x89, 0xf6, 0xf3, 0x22, 0xf8, 0x01, 0x00, 0x06}, /* Electric Guitar(clean) */
	{0x03, 0x21, 0x47, 0x09, 0xf9, 0xf6, 0x54, 0x3a, 0x00, 0x00, 0x00}, /* Electric Guitar(muted) */
	{0x23, 0x21, 0x4a, 0x0e, 0x91, 0x84, 0x41, 0x19, 0x01, 0x00, 0x08}, /* Overdriven Guitar */
	{0x23, 0x21, 0x4a, 0x09, 0x95, 0x94, 0x19, 0x19, 0x01, 0x00, 0x08}, /* Distortion Guitar */
	{0x09, 0x84, 0xa1, 0x89, 0x20, 0xd1, 0x4f, 0xf8, 0x00, 0x00, 0x08}, /* Guitar Harmonics */
	{0x21, 0xa2, 0x1e, 0x09, 0x94, 0xc3, 0x06, 0xa6, 0x00, 0x00, 0x02}, /* Acoustic Bass */
	{0x31, 0x31, 0x12, 0x09, 0xf1, 0xf1, 0x28, 0x18, 0x00, 0x00, 0x0a}, /* Electric Bass(finger) */
	{0x31, 0x31, 0x8d, 0x09, 0xf1, 0xf1, 0xe8, 0x78, 0x00, 0x00, 0x0a}, /* Electric Bass(pick) */
	{0x31, 0x32, 0x5b, 0x09, 0x51, 0x71, 0x28, 0x48, 0x00, 0x00, 0x0c}, /* Fretless Bass */
	{0x01, 0x21, 0x8b, 0x49, 0xa1, 0xf2, 0x9a, 0xdf, 0x00, 0x00, 0x08}, /* Slap Bass 1 */
	{0x21, 0x21, 0x8b, 0x11, 0xa2, 0xa1, 0x16, 0xdf, 0x00, 0x00, 0x08}, /* Slap Bass 2 */
	{0x31, 0x31, 0x8b, 0x09, 0xf4, 0xf1, 0xe8, 0x78, 0x00, 0x00, 0x0a}, /* Synth Bass 1 */
	{0x31, 0x31, 0x12, 0x09, 0xf1, 0xf1, 0x28, 0x18, 0x00, 0x00, 0x0a}, /* Synth Bass 2 */
	{0x31, 0x21, 0x15, 0x09, 0xdd, 0x56, 0x13, 0x26, 0x01, 0x00, 0x08}, /* Violin */
	{0x31, 0x21, 0x16, 0x09, 0xdd, 0x66, 0x13, 0x06, 0x01, 0x00, 0x08}, /* Viola */
	{0x71, 0x31, 0x49, 0x09, 0xd1, 0x61, 0x1c, 0x0c, 0x01, 0x00, 0x08}, /* Cello */
	{0x21, 0x23, 0x4d, 0x89, 0x71, 0x72, 0x12, 0x06, 0x01, 0x00, 0x02}, /* Contrabass */
	{0xf1, 0xe1, 0x40, 0x09, 0xf1, 0x6f, 0x21, 0x16, 0x01, 0x00, 0x02}, /* Tremolo Strings */
	{0x02, 0x01, 0x1a, 0x89, 0xf5, 0x85, 0x75, 0x35, 0x01, 0x00, 0x00}, /* Pizzicato Strings */
	{0x02, 0x01, 0x1d, 0x89, 0xf5, 0xf3, 0x75, 0xf4, 0x01, 0x00, 0x00}, /* Orchestral Strings */
	{0x10, 0x11, 0x41, 0x09, 0xf5, 0xf2, 0x05, 0xc3, 0x01, 0x00, 0x02}, /* Timpani */
	{0x21, 0xa2, 0x9b, 0x0a, 0xb1, 0x72, 0x25, 0x08, 0x01, 0x00, 0x0e}, /* String Ensemble 1 */
	{0xa1, 0x21, 0x98, 0x09, 0x7f, 0x3f, 0x03, 0x07, 0x01, 0x01, 0x00}, /* String Ensemble 2 */
	{0xa1, 0x61, 0x93, 0x09, 0xc1, 0x4f, 0x12, 0x05, 0x00, 0x00, 0x0a}, /* SynthStrings 1 */
	{0x21, 0x61, 0x18, 0x09, 0xc1, 0x4f, 0x22, 0x05, 0x00, 0x00, 0x0c}, /* SynthStrings 2 */
	{0x31, 0x72, 0x5b, 0x8c, 0xf4, 0x8a, 0x15, 0x05, 0x00, 0x00, 0x00}, /* Choir Aahs */
	{0xa1, 0x61, 0x90, 0x09, 0x74, 0x71, 0x39, 0x67, 0x00, 0x00, 0x00}, /* Voice Oohs */
	{0x71, 0x72, 0x57, 0x09, 0x54, 0x7a, 0x05, 0x05, 0x00, 0x00, 0x0c}, /* Synth Voice */
	{0x90, 0x41, 0x00, 0x09, 0x54, 0xa5, 0x63, 0x45, 0x00, 0x00, 0x08}, /* Orchestra Hit */
	{0x21, 0x21, 0x92, 0x0a, 0x85, 0x8f, 0x17, 0x09, 0x00, 0x00, 0x0c}, /* Trumpet */
	{0x21, 0x21, 0x94, 0x0e, 0x75, 0x8f, 0x17, 0x09, 0x00, 0x00, 0x0c}, /* Trombone */
	{0x21, 0x61, 0x94, 0x09, 0x76, 0x82, 0x15, 0x37, 0x00, 0x00, 0x0c}, /* Tuba */
	{0x31, 0x21, 0x43, 0x09, 0x9e, 0x62, 0x17, 0x2c, 0x01, 0x01, 0x02}, /* Muted Trumpet */
	{0x21, 0x21, 0x9b, 0x09, 0x61, 0x7f, 0x6a, 0x0a, 0x00, 0x00, 0x02}, /* French Horn */
	{0x61, 0x22, 0x8a, 0x0f, 0x75, 0x74, 0x1f, 0x0f, 0x00, 0x00, 0x08}, /* Brass Section */
	{0xa1, 0x21, 0x86, 0x8c, 0x72, 0x71, 0x55, 0x18, 0x01, 0x00, 0x00}, /* SynthBrass 1 */
	{0x21, 0x21, 0x4d, 0x09, 0x54, 0xa6, 0x3c, 0x1c, 0x00, 0x00, 0x08}, /* SynthBrass 2 */
	{0x31, 0x61, 0x8f, 0x09, 0x93, 0x72, 0x02, 0x0b, 0x01, 0x00, 0x08}, /* Soprano Sax */
	{0x31, 0x61, 0x8e, 0x09, 0x93, 0x72, 0x03, 0x09, 0x01, 0x00, 0x08}, /* Alto Sax */
	{0x31, 0x61, 0x91, 0x09, 0x93, 0x82, 0x03, 0x09, 0x01, 0x00, 0x0a}, /* Tenor Sax */
	{0x31, 0x61, 0x8e, 0x09, 0x93, 0x72, 0x0f, 0x0f, 0x01, 0x00, 0x0a}, /* Baritone Sax */
	{0x21, 0x21, 0x4b, 0x09, 0xaa, 0x8f, 0x16, 0x0a, 0x01, 0x00, 0x08}, /* Oboe */
	{0x31, 0x21, 0x90, 0x09, 0x7e, 0x8b, 0x17, 0x0c, 0x01, 0x01, 0x06}, /* English Horn */
	{0x31, 0x32, 0x81, 0x09, 0x75, 0x61, 0x19, 0x19, 0x01, 0x00, 0x00}, /* Bassoon */
	{0x32, 0x21, 0x90, 0x09, 0x9b, 0x72, 0x21, 0x17, 0x00, 0x00, 0x04}, /* Clarinet */
	{0xe1, 0xe1, 0x1f, 0x09, 0x85, 0x65, 0x5f, 0x1a, 0x00, 0x00, 0x00}, /* Piccolo */
	{0xe1, 0xe1, 0x46, 0x09, 0x88, 0x65, 0x5f, 0x1a, 0x00, 0x00, 0x00}, /* Flute */
	{0xa1, 0x21, 0x9c, 0x09, 0x75, 0x75, 0x1f, 0x0a, 0x00, 0x00, 0x02}, /* Recorder */
	{0x31, 0x21, 0x8b, 0x09, 0x84, 0x65, 0x58, 0x1a, 0x00, 0x00, 0x00}, /* Pan Flute */
	{0xe1, 0xa1, 0x4c, 0x09, 0x66, 0x65, 0x56, 0x26, 0x00, 0x00, 0x00}, /* Blown Bottle */
	{0x62, 0xa1, 0xcb, 0x09, 0x76, 0x55, 0x46, 0x36, 0x00, 0x00, 0x00}, /* Skakuhachi */
	{0x62, 0xa1, 0xa2, 0x09, 0x57, 0x56, 0x07, 0x07, 0x00, 0x00, 0x0b}, /* Whistle */
	{0x62, 0xa1, 0x9c, 0x09, 0x77, 0x76, 0x07, 0x07, 0x00, 0x00, 0x0b}, /* Ocarina */
	{0x22, 0x21, 0x59, 0x09, 0xff, 0xff, 0x03, 0x0f, 0x02, 0x00, 0x00}, /* Lead 1 (square) */
	{0x21, 0x21, 0x0e, 0x09, 0xff, 0xff, 0x0f, 0x0f, 0x01, 0x01, 0x00}, /* Lead 2 (sawtooth) */
	{0x22, 0x21, 0x46, 0x89, 0x86, 0x64, 0x55, 0x18, 0x00, 0x00, 0x00}, /* Lead 3 (calliope) */
	{0x21, 0xa1, 0x45, 0x09, 0x66, 0x96, 0x12, 0x0a, 0x00, 0x00, 0x00}, /* Lead 4 (chiff) */
	{0x21, 0x22, 0x8b, 0x09, 0x92, 0x91, 0x2a, 0x2a, 0x01, 0x00, 0x00}, /* Lead 5 (charang) */
	{0xa2, 0x61, 0x9e, 0x49, 0xdf, 0x6f, 0x05, 0x07, 0x00, 0x00, 0x02}, /* Lead 6 (voice) */
	{0x20, 0x60, 0x1a, 0x09, 0xef, 0x8f, 0x01, 0x06, 0x00, 0x02, 0x00}, /* Lead 7 (fifths) */
	{0x21, 0x21, 0x8f, 0x86, 0xf1, 0xf4, 0x29, 0x09, 0x00, 0x00, 0x0a}, /* Lead 8 (bass+lead) */
	{0x77, 0xa1, 0xa5, 0x09, 0x53, 0xa0, 0x94, 0x05, 0x00, 0x00, 0x02}, /* Pad 1 (new age) */
	{0x61, 0xb1, 0x1f, 0x89, 0xa8, 0x25, 0x11, 0x03, 0x00, 0x00, 0x0a}, /* Pad 2 (warm) */
	{0x61, 0x61, 0x17, 0x09, 0x91, 0x55, 0x34, 0x16, 0x00, 0x00, 0x0c}, /* Pad 3 (polysynth) */
	{0x71, 0x72, 0x5d, 0x09, 0x54, 0x6a, 0x01, 0x03, 0x00, 0x00, 0x00}, /* Pad 4 (choir) */
	{0x21, 0xa2, 0x97, 0x09, 0x21, 0x42, 0x43, 0x35, 0x00, 0x00, 0x08}, /* Pad 5 (bowed) */
	{0xa1, 0x21, 0x1c, 0x09, 0xa1, 0x31, 0x77, 0x47, 0x01, 0x01, 0x00}, /* Pad 6 (metallic) */
	{0x21, 0x61, 0x89, 0x0c, 0x11, 0x42, 0x33, 0x25, 0x00, 0x00, 0x0a}, /* Pad 7 (halo) */
	{0xa1, 0x21, 0x15, 0x09, 0x11, 0xcf, 0x47, 0x07, 0x01, 0x00, 0x00}, /* Pad 8 (sweep) */
	{0x3a, 0x51, 0xce, 0x09, 0xf8, 0x86, 0xf6, 0x02, 0x00, 0x00, 0x02}, /* FX 1 (rain) */
	{0x21, 0x21, 0x15, 0x09, 0x21, 0x41, 0x23, 0x13, 0x01, 0x00, 0x00}, /* FX 2 (soundtrack) */
	{0x06, 0x01, 0x5b, 0x09, 0x74, 0xa5, 0x95, 0x72, 0x00, 0x00, 0x00}, /* FX 3 (crystal) */
	{0x22, 0x61, 0x92, 0x8c, 0xb1, 0xf2, 0x81, 0x26, 0x00, 0x00, 0x0c}, /* FX 4 (atmosphere) */
	{0x41, 0x42, 0x4d, 0x09, 0xf1, 0xf2, 0x51, 0xf5, 0x01, 0x00, 0x00}, /* FX 5 (brightness) */
	{0x61, 0xa3, 0x94, 0x89, 0x11, 0x11, 0x51, 0x13, 0x01, 0x00, 0x06}, /* FX 6 (goblins) */
	{0x61, 0xa1, 0x8c, 0x89, 0x11, 0x1d, 0x31, 0x03, 0x00, 0x00, 0x06}, /* FX 7 (echoes) */
	{0xa4, 0x61, 0x4c, 0x09, 0xf3, 0x81, 0x73, 0x23, 0x01, 0x00, 0x04}, /* FX 8 (sci-fi) */
	{0x02, 0x07, 0x85, 0x0c, 0xd2, 0xf2, 0x53, 0xf6, 0x00, 0x01, 0x00}, /* Sitar */
	{0x11, 0x13, 0x0c, 0x89, 0xa3, 0xa2, 0x11, 0xe5, 0x01, 0x00, 0x00}, /* Banjo */
	{0x11, 0x11, 0x06, 0x09, 0xf6, 0xf2, 0x41, 0xe6, 0x01, 0x02, 0x04}, /* Shamisen */
	{0x93, 0x91, 0x91, 0x09, 0xd4, 0xeb, 0x32, 0x11, 0x00, 0x01, 0x08}, /* Koto */
	{0x04, 0x01, 0x4f, 0x09, 0xfa, 0xc2, 0x56, 0x05, 0x00, 0x00, 0x0c}, /* Kalimba */
	{0x21, 0x22, 0x49, 0x09, 0x7c, 0x6f, 0x20, 0x0c, 0x00, 0x01, 0x06}, /* Bagpipe */
	{0x31, 0x21, 0x85, 0x09, 0xdd, 0x56, 0x33, 0x16, 0x01, 0x00, 0x0a}, /* Fiddle */
	{0x20, 0x21, 0x04, 0x8a, 0xda, 0x8f, 0x05, 0x0b, 0x02, 0x00, 0x06}, /* Shanai */
	{0x05, 0x03, 0x6a, 0x89, 0xf1, 0xc3, 0xe5, 0xe5, 0x00, 0x00, 0x06}, /* Tinkle Bell */
	{0x07, 0x02, 0x15, 0x09, 0xec, 0xf8, 0x26, 0x16, 0x00, 0x00, 0x0a}, /* Agogo */
	{0x05, 0x01, 0x9d, 0x09, 0x67, 0xdf, 0x35, 0x05, 0x00, 0x00, 0x08}, /* Steel Drums */
	{0x18, 0x12, 0x96, 0x09, 0xfa, 0xf8, 0x28, 0xe5, 0x00, 0x00, 0x0a}, /* Woodblock */
	{0x10, 0x00, 0x86, 0x0c, 0xa8, 0xfa, 0x07, 0x03, 0x00, 0x00, 0x06}, /* Taiko Drum */
	{0x11, 0x10, 0x41, 0x0c, 0xf8, 0xf3, 0x47, 0x03, 0x02, 0x00, 0x04}, /* Melodic Tom */
	{0x01, 0x10, 0x8e, 0x09, 0xf1, 0xf3, 0x06, 0x02, 0x02, 0x00, 0x0e}, /* Synth Drum */
	{0x0e, 0xc0, 0x00, 0x09, 0x1f, 0x1f, 0x00, 0xff, 0x00, 0x03, 0x0e}, /* Reverse Cymbal */
	{0x06, 0x03, 0x80, 0x91, 0xf8, 0x56, 0x24, 0x84, 0x00, 0x02, 0x0e}, /* Guitar Fret Noise */
	{0x0e, 0xd0, 0x00, 0x0e, 0xf8, 0x34, 0x00, 0x04, 0x00, 0x03, 0x0e}, /* Breath Noise */
	{0x0e, 0xc0, 0x00, 0x09, 0xf6, 0x1f, 0x00, 0x02, 0x00, 0x03, 0x0e}, /* Seashore */
	{0xd5, 0xda, 0x95, 0x49, 0x37, 0x56, 0xa3, 0x37, 0x00, 0x00, 0x00}, /* Bird Tweet */
	{0x35, 0x14, 0x5c, 0x11, 0xb2, 0xf4, 0x61, 0x15, 0x02, 0x00, 0x0a}, /* Telephone ring */
	{0x0e, 0xd0, 0x00, 0x09, 0xf6, 0x4f, 0x00, 0xf5, 0x00, 0x03, 0x0e}, /* Helicopter */
	{0x26, 0xe4, 0x00, 0x09, 0xff, 0x12, 0x01, 0x16, 0x00, 0x01, 0x0e}, /* Applause */
	{0x00, 0x00, 0x00, 0x09, 0xf3, 0xf6, 0xf0, 0xc9, 0x00, 0x02, 0x0e}  /* Gunshot */

};

/* logarithmic relationship between midi and FM volumes */
const int FMOplMidiDriver::my_midi_fm_vol_table[128] = {
	0, 11, 16, 19, 22, 25, 27, 29, 32, 33, 35, 37, 39, 40, 42, 43,
	45, 46, 48, 49, 50, 51, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62,
	64, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 75, 76, 77,
	78, 79, 80, 80, 81, 82, 83, 83, 84, 85, 86, 86, 87, 88, 89, 89,
	90, 91, 91, 92, 93, 93, 94, 95, 96, 96, 97, 97, 98, 99, 99, 100,
	101, 101, 102, 103, 103, 104, 104, 105, 106, 106, 107, 107, 108,
	109, 109, 110, 110, 111, 112, 112, 113, 113, 114, 114, 115, 115,
	116, 117, 117, 118, 118, 119, 119, 120, 120, 121, 121, 122, 122,
	123, 123, 124, 124, 125, 125, 126, 126, 127
};

/* lucas move volume table */
int FMOplMidiDriver::lucas_fm_vol_table[128];

const unsigned char FMOplMidiDriver::adlib_opadd[9] = {
	0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12
};

//
// Constructor
//
FMOplMidiDriver::FMOplMidiDriver() : LowLevelMidiDriver() {
}

//
// Open the device
//
int FMOplMidiDriver::open() {
	int i, j;

	for (i = 0; i < 128; i++) {
		for (j = 0; j < 11; j++)
			myinsbank[i][j] = midi_fm_instruments_table[i][j];

		myinsbank[i][INDEX_PERC] = 0x80;

		// Setup square root table here
		lucas_fm_vol_table[i] = (int)((double)std::sqrt((double)my_midi_fm_vol_table[i]) * 11); /* TO CHANGE !!! */

		// Clear the xmidibanks
		xmidibanks[i] = 0;
	}
	for (i = 0; i < 16; i++) {
		ch[i].inum = 0;
		ch[i].xmidi = false;
		ch[i].xmidi_bank = 0;
		ch[i].vol = 127;
		ch[i].expression = 127;
		ch[i].nshift = -13;//-13;
		ch[i].on = 1;
		ch[i].pitchbend = 0x2000;
		ch[i].pan = 64;
	}

	/* General init */
	for (i = 0; i < 9; i++) {
		chp[i][CHP_CHAN] = -1;
		chp[i][CHP_COUNTER] = 0;
		chp[i][CHP_VEL] = 0;
	}

#ifndef PENTAGRAM_IN_EXULT
	IDataSource *timbres = GameData::get_instance()->getMusic()->getAdlibTimbres();
	if (timbres) {
		loadXMIDITimbres(timbres);
	} else {
		perr << "FMOplMidiDriver: Error, unable to load Adlib Timbres in open()" << std::endl;
		return 1;
	}
	delete timbres;
#endif

	opl = FMOpl_Pentagram::OPLCreate(OPL_TYPE_YM3812, 3579545, sample_rate);

	return 0;
}

//
// Close the device
//
void FMOplMidiDriver::close() {
	// Destroy the Opl device
	if (opl) FMOpl_Pentagram::OPLDestroy(opl);

	// Reset the relevant members
	opl = 0;

	// Clear the xmidibanks
	for (int i = 0; i < 128; i++) {
		delete xmidibanks[i];
		xmidibanks[i] = 0;
	}

}

//
// Generate the samples
//
void FMOplMidiDriver::lowLevelProduceSamples(int16 *samples, uint32 num_samples) {
	if (!opl)
		memset(samples, 0, num_samples * sizeof(int16) * (stereo ? 2 : 1));
	else if (stereo)
		FMOpl_Pentagram::YM3812UpdateOne_Stereo(opl, samples, num_samples);
	else
		FMOpl_Pentagram::YM3812UpdateOne_Mono(opl, samples, num_samples);
}

int FMOplMidiDriver::midi_calc_volume(int channel, int vel) {
	vel += (VEL_FUDGE - 1) * 128;

	int nv = (ch[channel].vol * ch[channel].expression * vel) / (16129 * VEL_FUDGE);
//	int nv = (ch[channel].vol * ch[channel].expression * vel) >> 14;

	if (nv > 127)
		nv = 127;

	nv = my_midi_fm_vol_table[nv];

	return nv;
}


//
// Send a single packed midi command to the Opl (to be played now)
//
void FMOplMidiDriver::send(uint32 b) {
	unsigned char channel = (char)(b & 0x0F);

	// Discard everything on channel 9 for now
	if (channel == 9 && !xmidibanks[127]) return;

	switch (b & 0xF0) {
	case 0x80:                                  /*note off */
		// Quick hack, but we should never use it. since note offs will never be sent
		b &= 0xFFFF;
	case 0x90: {                                /*note on */
		unsigned char note = (unsigned char)((b >> 8) & 0x7F);
		unsigned char vel = (unsigned char)((b >> 16) & 0x7F);
		int i, j;
		int onl, on, nv;
		on = -1;

		// First send a note off, if it's found
		for (i = 0; i < 9; i++)
			if ((chp[i][CHP_CHAN] == channel) && (chp[i][CHP_NOTE] == note)) {
				midi_fm_endnote(i);
				chp[i][CHP_CHAN] = -1;
			}

		if (vel != 0 && ch[channel].on != 0) {

			// Increment each counter
			for (i = 0; i < 9; i++) chp[i][CHP_COUNTER]++;

			// Try to find the last channel that was used, that is currently unused
			j = 0;
			onl = 0;
			for (i = 0; i < 9; i++)
				if ((chp[i][CHP_CHAN] == -1) && (chp[i][CHP_COUNTER] > onl)) {
					onl = chp[i][CHP_COUNTER];
					on = i;
					j = 1;
				}

			// If we didn't find a free chan, use the oldest chan
			if (on == -1) {
				onl = 0;
				for (i = 0; i < 9; i++)
					if (chp[i][CHP_COUNTER] > onl) {
						onl = chp[i][CHP_COUNTER];
						on = i;
					}
			}

			// If we didn't find a free note, send a note off to the Adlib for the chan we are using
			if (j == 0) midi_fm_endnote(on);

			// Send the instrument adlib
			if ((ch[channel].inum >= 0) && (ch[channel].inum < 128)) {
				if (channel == 9 && xmidibanks[127])
					midi_fm_instrument(on, xmidibanks[127]->insbank[note]);
				else
					midi_fm_instrument(on, ch[channel].ins);
			}

			// Calculate the adlib volume
			nv = midi_calc_volume(channel, vel);

			// Send note on
			midi_fm_playnote(on, note + ch[channel].nshift, nv * 2, ch[channel].pitchbend);

			FMOpl_Pentagram::OPLSetPan(opl, on, ch[channel].pan);

			// Update the shadows
			chp[on][CHP_CHAN] = channel;
			chp[on][CHP_NOTE] = note;
			chp[on][CHP_COUNTER] = 0;
			chp[on][CHP_VEL] = vel;
		}
	}
	break;

	case 0xa0: {                                /*key after touch */
		unsigned char note = (unsigned char)((b >> 8) & 0x7F);
		unsigned char vel = (unsigned char)((b >> 16) & 0x7F);
		int nv = midi_calc_volume(channel, vel);

		for (int i = 0; i < 9; i++)
			if ((chp[CHP_CHAN][0] == channel) & (chp[i][CHP_NOTE] == note)) {
				chp[i][CHP_VEL] = vel;
				midi_fm_volume(i, nv * 2);
			}
	}
	break;

	case 0xb0: {                                /* control change */
		int i;
		unsigned char ctrl = (unsigned char)((b >> 8) & 0x7F);
		unsigned char vel = (unsigned char)((b >> 16) & 0x7F);

		/* FIXME: Except for Volume, the Modulation and Sustain
		   code is just a random guess. */
		switch (ctrl) {
		case 0x00:                              /* Bank Change */
			break;
		case 0x01:                              /* Modulation */
			for (i = 0; i < 9; i++)
				if (chp[i][CHP_CHAN] == channel)
					midi_write_adlib(0x20 + adlib_opadd[i], vel);
			break;
		case 0x07:                              /* Volume */
			ch[channel].vol = vel;
			midi_update_volume(channel);
			break;
		case 0x0A:                              /* Pan */
			ch[channel].pan = vel;
			for (i = 0; i < 9; i++)
				if (chp[i][CHP_CHAN] == channel)
					FMOpl_Pentagram::OPLSetPan(opl, i, ch[channel].pan);
			break;
		case 0x0B:                              /* Expression */
			ch[channel].expression = vel;
			midi_update_volume(channel);
			break;
		case 0x40:                              /* Sustain on/off */
			for (i = 0; i < 9; i++)
				if (chp[i][CHP_CHAN] == channel)
					midi_write_adlib(0x80 + adlib_opadd[i], vel);
			break;
		case 0x5B:                              /* Extended depth effect */
			//debug(1,
			//          "MIDI sub-command 0xB0 (Control Change) case %02X (Extended Depth) not handled in MIDIEMU driver.",
			//          ctrl);
			break;
		case 0x5D:                              /* Chorus depth */
			//debug(1,
			//          "MIDI sub-command 0xB0 (Control Change) case %02X (Chorus Depth) not handled in MIDIEMU driver.",
			//          ctrl);
			break;
		//
		// XMIDI
		//
		case XMIDI_CONTROLLER_CHAN_LOCK:
			//PERR("Uh oh. Detected a XMIDI Channel Lock (" << (int) vel << ") on channel " << (int) channel << ", but we don't support them");
			break;
		case XMIDI_CONTROLLER_CHAN_LOCK_PROT:
			//PERR("Uh oh. Detected a XMIDI Channel Lock Protect (" << (int) vel << ") on channel " << (int) channel << ", but we don't support them");
			break;
		case XMIDI_CONTROLLER_VOICE_PROT:
			//PERR("Uh oh. Detected a XMIDI Voice Protect (" << (int) vel << ") on channel " << (int) channel << ", but we don't support them");
			break;
		case XMIDI_CONTROLLER_TIMBRE_PROT:
			//PERR("Uh oh. Detected a XMIDI Timbre Protect (" << (int) vel << ") on channel " << (int) channel << ", but we don't support them");
			break;
		case XMIDI_CONTROLLER_BANK_CHANGE:
			//POUT("Detected a XMIDI Bank Change (" << (int) vel << ") on channel " << (int) channel);
			// Set the bank
			ch[channel].xmidi_bank = vel;
			break;
		case XMIDI_CONTROLLER_IND_CTRL_PREFIX:
			//PERR("Uh oh. Detected a XMIDI Indirect Controller Prefix (" << (int) vel << ") on channel " << (int) channel << ", but we don't support them");
			break;
		case XMIDI_CONTROLLER_CLEAR_BB_COUNT:
			//PERR("Uh oh. Detected a XMIDI Clear Beat/Bar Count (" << (int) vel << ") on channel " << (int) channel << ", but we don't support them");
			break;
		case XMIDI_CONTROLLER_CALLBACK_TRIG:
			//PERR("Uh oh. Detected a XMIDI Callback Trigger (" << (int) vel << ") on channel " << (int) channel << ", but we don't support them");
			break;
		case XMIDI_CONTROLLER_SEQ_BRANCH_INDEX:
			//PERR("Uh oh. Detected a XMIDI Sequence Branch Index (" << (int) vel << ") on channel " << (int) channel << ", but we don't support them");
			break;
		//
		// END XMIDI
		//
		case 0x7B:                              /* All notes off */
			for (i = 0; i < 9; i++) {
				if (chp[i][CHP_CHAN] == channel) {
					midi_fm_endnote(i);
					chp[i][CHP_CHAN] = -1;
				}
			}
			break;
		default:
			//PERR("MIDI sub-command 0xB0 (Control Change) case 0x" << std::hex << (int) ctrl << std::dec << " not handled in MIDIEMU driver.");
			break;
		}
	}
	break;

	case 0xc0: {                                /* patch change */
		unsigned char instrument = (unsigned char)((b >> 8) & 0x7F);
		ch[channel].inum = instrument;
		//std::POUT << "Setting instrument: " << ((unsigned int) instrument) << " for chan " << ((unsigned int) channel) << std::endl;

		unsigned char *ins = 0;
		int b = -1;

		// Search for xmidi ins.
		if (ch[channel].xmidi) for (b = ch[channel].xmidi_bank; b >= 0; b--) {
				xmidibank *bank = xmidibanks[b];
				if (!bank) continue;
				if (bank->insbank[instrument][INDEX_PERC] &= 0x80) {
					ins = bank->insbank[instrument];
					break;
				}
			}

		if (!ins) ins = myinsbank[instrument];

		for (int j = 0; j < 12; j++)
			ch[channel].ins[j] = ins[j];

		//POUT("Detected a Patch Change (" << (int) instrument << ") on channel " << (int) channel << " xmidibank: " << b);

	}
	break;

	case 0xd0:                                      /* channel touch */
		//debug(1, "MIDI command 0xD0 (Channel Touch) not handled in MIDIEMU driver.");
		break;

	case 0xe0: {
		//break;                        /* pitch wheel */
		int pitchbend = ((b >> 8) & 0x7F) | (((b >> 16) & 0x7F) << 7);
		ch[channel].pitchbend = pitchbend;
		for (int i = 0; i < 9; i++) {
			if (chp[i][CHP_CHAN] == channel) {
				int nv = midi_calc_volume(channel, chp[i][CHP_VEL]);
				midi_fm_playnote(i, chp[i][CHP_NOTE] + ch[channel].nshift, nv * 2, pitchbend);
			}
		}
	}
	break;

	default:
		//fprintf(stderr, "Unknown : %08x\n", (int)b);
		break;
	}
}

void FMOplMidiDriver::midi_write_adlib(unsigned int reg, unsigned char val) {
	FMOpl_Pentagram::OPLWrite(opl, 0, reg);
	FMOpl_Pentagram::OPLWrite(opl, 1, val);
	adlib_data[reg] = val;
}

/*

typedef struct
{
   unsigned char mod_avekm;     // 0    (20)
   unsigned char mod_ksl_tl;    // 1    (40)
   unsigned char mod_ad;        // 2    (60)
   unsigned char mod_sr;        // 3    (80)
   unsigned char mod_ws;        // 4    (E0)

   unsigned char fb_c;          // 5

   unsigned char car_avekm;     // 6    amp, sussnd             (20)
   unsigned char car_ksl_tl;    // 7    outlev, keyscale        (43)
   unsigned char car_ad;        // 8    Attack Delay        AR  (63)
   unsigned char car_sr;        // 9    SustainLev Release  DR  (83)
   unsigned char car_ws;        // 10   Waveform                (E0)
}
AD_instrument;

case 0x20:   am,vib,ksr,eg type,mul

*/

void FMOplMidiDriver::midi_fm_instrument(int voice, unsigned char *inst) {
#if 0
	/* Just gotta make sure this happens because who knows when it'll be reset otherwise.... */
#endif


#if 0 && defined(LUCAS_MODE)
	midi_write_adlib(OPL_REG_KSLTL_C + adlib_opadd[voice], 0x3f);
	if ((inst[INDEX_FB_C] & 1) == 0)
		midi_write_adlib(OPL_REG_KSLTL_M + adlib_opadd[voice], inst[INDEX_KSLTL_M]);
	else
		midi_write_adlib(OPL_REG_KSLTL_M + adlib_opadd[voice], 0x3f);
#elif 0
	midi_write_adlib(OPL_REG_KSLTL_M + adlib_opadd[voice], inst[INDEX_KSLTL_M]);
	if ((inst[INDEX_FB_C] & 1) == 0)
		midi_write_adlib(OPL_REG_KSLTL_C + adlib_opadd[voice], inst[INDEX_KSLTL_C]);
	else
		midi_write_adlib(OPL_REG_KSLTL_C + adlib_opadd[voice], 0);
#else
#endif

	midi_write_adlib(OPL_REG_AVEKM_M + adlib_opadd[voice], inst[INDEX_AVEKM_M]);
	midi_write_adlib(OPL_REG_KSLTL_M + adlib_opadd[voice], inst[INDEX_KSLTL_M]);
	midi_write_adlib(OPL_REG_AD_M + adlib_opadd[voice], inst[INDEX_AD_M]);
	midi_write_adlib(OPL_REG_SR_M + adlib_opadd[voice], inst[INDEX_SR_M]);
	midi_write_adlib(OPL_REG_WAVE_M + adlib_opadd[voice], inst[INDEX_WAVE_M]);

	if (!(inst[INDEX_PERC] & 0x7f)) {
		midi_write_adlib(OPL_REG_AVEKM_C + adlib_opadd[voice], inst[INDEX_AVEKM_C]);
		midi_write_adlib(OPL_REG_KSLTL_C + adlib_opadd[voice], inst[INDEX_KSLTL_C]);
		midi_write_adlib(OPL_REG_AD_C + adlib_opadd[voice], inst[INDEX_AD_C]);
		midi_write_adlib(OPL_REG_SR_C + adlib_opadd[voice], inst[INDEX_SR_C]);
		midi_write_adlib(OPL_REG_WAVE_C + adlib_opadd[voice], inst[INDEX_WAVE_C]);
		midi_write_adlib(OPL_REG_FB_C + voice, inst[INDEX_FB_C]);
		midi_write_adlib(0xbd, 0);

		if (!inst[INDEX_PERC]) {
			PERR("Tried setting an instrument that wasn't loaded");
		}
	}
}

void FMOplMidiDriver::midi_update_volume(int channel) {
	for (int i = 0; i < 9; i++)
		if ((chp[i][CHP_CHAN] == channel)) {

			int nv = midi_calc_volume(channel, chp[i][CHP_VEL]);
			midi_fm_volume(i, nv * 2);
		}
}

void FMOplMidiDriver::midi_fm_volume(int voice, int volume) {
	volume >>= 2;

	if ((adlib_data[0xc0 + voice] & 1) == 1)
		midi_write_adlib(0x40 + adlib_opadd[voice],
		                 (unsigned char)((63 - volume) |
		                                 (adlib_data[0x40 + adlib_opadd[voice]] & 0xc0)));
	midi_write_adlib(0x43 + adlib_opadd[voice],
	                 (unsigned char)((63 - volume) | (adlib_data[0x43 + adlib_opadd[voice]] & 0xc0)));
}

const int FMOplMidiDriver::fnums[12] = {
	0x16b, 0x181, 0x198, 0x1b0,
	0x1ca, 0x1e5, 0x202, 0x220,
	0x241, 0x263, 0x287, 0x2ae
};

/* These tables 'borrowed' from Timidity tables.c

   Copyright (C) 1999-2001 Masanao Izumo <mo@goice.co.jp>
   Copyright (C) 1995 Tuukka Toivonen <tt@cgs.fi>
*/
const double FMOplMidiDriver::bend_fine[256] = {
	1.0, 1.0002256593050698, 1.0004513695322617, 1.0006771306930664,
	1.0009029427989777, 1.0011288058614922, 1.0013547198921082, 1.0015806849023274,
	1.0018067009036538, 1.002032767907594, 1.0022588859256572, 1.0024850549693551,
	1.0027112750502025, 1.0029375461797159, 1.0031638683694153, 1.0033902416308227,
	1.0036166659754628, 1.0038431414148634, 1.0040696679605541, 1.0042962456240678,
	1.0045228744169397, 1.0047495543507072, 1.0049762854369111, 1.0052030676870944,
	1.0054299011128027, 1.0056567857255843, 1.00588372153699, 1.006110708558573,
	1.0063377468018897, 1.0065648362784985, 1.0067919769999607, 1.0070191689778405,
	1.0072464122237039, 1.0074737067491204, 1.0077010525656616, 1.0079284496849015,
	1.0081558981184175, 1.008383397877789, 1.008610948974598, 1.0088385514204294,
	1.0090662052268706, 1.0092939104055114, 1.0095216669679448, 1.0097494749257656,
	1.009977334290572, 1.0102052450739643, 1.0104332072875455, 1.0106612209429215,
	1.0108892860517005, 1.0111174026254934, 1.0113455706759138, 1.0115737902145781,
	1.0118020612531047, 1.0120303838031153, 1.0122587578762337, 1.012487183484087,
	1.0127156606383041, 1.0129441893505169, 1.0131727696323602, 1.0134014014954713,
	1.0136300849514894, 1.0138588200120575, 1.0140876066888203, 1.0143164449934257,
	1.0145453349375237, 1.0147742765327674, 1.0150032697908125, 1.0152323147233171,
	1.015461411341942, 1.0156905596583505, 1.0159197596842091, 1.0161490114311862,
	1.0163783149109531, 1.0166076701351838, 1.0168370771155553, 1.0170665358637463,
	1.0172960463914391, 1.0175256087103179, 1.0177552228320703, 1.0179848887683858,
	1.0182146065309567, 1.0184443761314785, 1.0186741975816487, 1.0189040708931674,
	1.0191339960777379, 1.0193639731470658, 1.0195940021128593, 1.0198240829868295,
	1.0200542157806898, 1.0202844005061564, 1.0205146371749483, 1.0207449257987866,
	1.0209752663893958, 1.0212056589585028, 1.0214361035178368, 1.0216666000791297,
	1.0218971486541166, 1.0221277492545349, 1.0223584018921241, 1.0225891065786274,
	1.0228198633257899, 1.0230506721453596, 1.023281533049087, 1.0235124460487257,
	1.0237434111560313, 1.0239744283827625, 1.0242054977406807, 1.0244366192415495,
	1.0246677928971357, 1.0248990187192082, 1.025130296719539, 1.0253616269099028,
	1.0255930093020766, 1.0258244439078401, 1.0260559307389761, 1.0262874698072693,
	1.0265190611245079, 1.0267507047024822, 1.0269824005529853, 1.027214148687813,
	1.0274459491187637, 1.0276778018576387, 1.0279097069162415, 1.0281416643063788,
	1.0283736740398595, 1.0286057361284953, 1.0288378505841009, 1.0290700174184932,
	1.0293022366434921, 1.0295345082709197, 1.0297668323126017, 1.0299992087803651,
	1.030231637686041, 1.0304641190414621, 1.0306966528584645, 1.0309292391488862,
	1.0311618779245688, 1.0313945691973556, 1.0316273129790936, 1.0318601092816313,
	1.0320929581168212, 1.0323258594965172, 1.0325588134325767, 1.0327918199368598,
	1.0330248790212284, 1.0332579906975481, 1.0334911549776868, 1.033724371873515,
	1.0339576413969056, 1.0341909635597348, 1.0344243383738811, 1.0346577658512259,
	1.034891246003653, 1.0351247788430489, 1.0353583643813031, 1.0355920026303078,
	1.0358256936019572, 1.0360594373081489, 1.0362932337607829, 1.0365270829717617,
	1.0367609849529913, 1.0369949397163791, 1.0372289472738365, 1.0374630076372766,
	1.0376971208186156, 1.0379312868297725, 1.0381655056826686, 1.0383997773892284,
	1.0386341019613787, 1.0388684794110492, 1.0391029097501721, 1.0393373929906822,
	1.0395719291445176, 1.0398065182236185, 1.0400411602399278, 1.0402758552053915,
	1.0405106031319582, 1.0407454040315787, 1.0409802579162071, 1.0412151647977996,
	1.0414501246883161, 1.0416851375997183, 1.0419202035439705, 1.0421553225330404,
	1.042390494578898, 1.042625719693516, 1.0428609978888699, 1.043096329176938,
	1.0433317135697009, 1.0435671510791424, 1.0438026417172486, 1.0440381854960086,
	1.0442737824274138, 1.044509432523459, 1.044745135796141, 1.0449808922574599,
	1.0452167019194181, 1.0454525647940205, 1.0456884808932754, 1.0459244502291931,
	1.0461604728137874, 1.0463965486590741, 1.046632677777072, 1.0468688601798024,
	1.0471050958792898, 1.047341384887561, 1.0475777272166455, 1.047814122878576,
	1.048050571885387, 1.0482870742491166, 1.0485236299818055, 1.0487602390954964,
	1.0489969016022356, 1.0492336175140715, 1.0494703868430555, 1.0497072096012419,
	1.0499440858006872, 1.0501810154534512, 1.050417998571596, 1.0506550351671864,
	1.0508921252522903, 1.0511292688389782, 1.0513664659393229, 1.0516037165654004,
	1.0518410207292894, 1.0520783784430709, 1.0523157897188296, 1.0525532545686513,
	1.0527907730046264, 1.0530283450388465, 1.0532659706834067, 1.0535036499504049,
	1.0537413828519411, 1.0539791694001188, 1.0542170096070436, 1.0544549034848243,
	1.0546928510455722, 1.0549308523014012, 1.0551689072644284, 1.0554070159467728,
	1.0556451783605572, 1.0558833945179062, 1.0561216644309479, 1.0563599881118126,
	1.0565983655726334, 1.0568367968255465, 1.0570752818826903, 1.0573138207562065,
	1.057552413458239, 1.0577910600009348, 1.0580297603964437, 1.058268514656918,
	1.0585073227945128, 1.0587461848213857, 1.058985100749698, 1.0592240705916123
};

const double FMOplMidiDriver::bend_coarse[128] = {
	1.0, 1.0594630943592953, 1.122462048309373, 1.189207115002721,
	1.2599210498948732, 1.3348398541700344, 1.4142135623730951, 1.4983070768766815,
	1.5874010519681994, 1.681792830507429, 1.7817974362806785, 1.8877486253633868,
	2.0, 2.1189261887185906, 2.244924096618746, 2.3784142300054421,
	2.5198420997897464, 2.6696797083400687, 2.8284271247461903, 2.996614153753363,
	3.1748021039363992, 3.363585661014858, 3.5635948725613571, 3.7754972507267741,
	4.0, 4.2378523774371812, 4.4898481932374912, 4.7568284600108841,
	5.0396841995794928, 5.3393594166801366, 5.6568542494923806, 5.993228307506727,
	6.3496042078727974, 6.727171322029716, 7.1271897451227151, 7.5509945014535473,
	8.0, 8.4757047548743625, 8.9796963864749824, 9.5136569200217682,
	10.079368399158986, 10.678718833360273, 11.313708498984761, 11.986456615013454,
	12.699208415745595, 13.454342644059432, 14.25437949024543, 15.101989002907095,
	16.0, 16.951409509748721, 17.959392772949972, 19.027313840043536,
	20.158736798317967, 21.357437666720553, 22.627416997969522, 23.972913230026901,
	25.398416831491197, 26.908685288118864, 28.508758980490853, 30.203978005814196,
	32.0, 33.902819019497443, 35.918785545899944, 38.054627680087073,
	40.317473596635935, 42.714875333441107, 45.254833995939045, 47.945826460053802,
	50.796833662982394, 53.817370576237728, 57.017517960981706, 60.407956011628393,
	64.0, 67.805638038994886, 71.837571091799887, 76.109255360174146,
	80.63494719327187, 85.429750666882214, 90.509667991878089, 95.891652920107603,
	101.59366732596479, 107.63474115247546, 114.03503592196341, 120.81591202325679,
	128.0, 135.61127607798977, 143.67514218359977, 152.21851072034829,
	161.26989438654374, 170.85950133376443, 181.01933598375618, 191.78330584021521,
	203.18733465192958, 215.26948230495091, 228.07007184392683, 241.63182404651357,
	256.0, 271.22255215597971, 287.35028436719938, 304.43702144069658,
	322.53978877308765, 341.71900266752868, 362.03867196751236, 383.56661168043064,
	406.37466930385892, 430.53896460990183, 456.14014368785394, 483.26364809302686,
	512.0, 542.44510431195943, 574.70056873439876, 608.87404288139317,
	645.0795775461753, 683.43800533505737, 724.07734393502471, 767.13322336086128,
	812.74933860771785, 861.07792921980365, 912.28028737570787, 966.52729618605372,
	1024.0, 1084.8902086239189, 1149.4011374687975, 1217.7480857627863,
	1290.1591550923506, 1366.8760106701147, 1448.1546878700494, 1534.2664467217226
};

void FMOplMidiDriver::midi_fm_playnote(int voice, int note, int volume, int pitchbend) {
	int freq = fnums[note % 12];
	int oct = note / 12;
	int c;
	float pf;

	pitchbend -= 0x2000;
	if (pitchbend != 0) {
		pitchbend *= 2;
		if (pitchbend >= 0)
			pf = (float)(bend_fine[(pitchbend >> 5) & 0xFF] * bend_coarse[(pitchbend >> 13) & 0x7F]);
		else {
			pitchbend = -pitchbend;
			pf =
			    (float)(1.0 / (bend_fine[(pitchbend >> 5) & 0xFF] * bend_coarse[(pitchbend >> 13) & 0x7F]));
		}
		freq = (int)((float)freq * pf);

		while (freq >= (fnums[0] * 2)) {
			freq /= 2;
			oct += 1;
		}
		while (freq < fnums[0]) {
			freq *= 2;
			oct -= 1;
		}
	}

	midi_fm_volume(voice, volume);
	midi_write_adlib(0xa0 + voice, (unsigned char)(freq & 0xff));

	c = ((freq & 0x300) >> 8) + (oct << 2) + (1 << 5);
	midi_write_adlib(0xb0 + voice, (unsigned char)c);
}

void FMOplMidiDriver::midi_fm_endnote(int voice) {
	midi_write_adlib(0xb0 + voice, (unsigned char)(adlib_data[0xb0 + voice] & (255 - 32)));
}

void FMOplMidiDriver::loadTimbreLibrary(IDataSource *ds, TimbreLibraryType type) {
	int i;

	// Clear the xmidibanks
	for (i = 0; i < 128; i++) {
		delete xmidibanks[i];
		xmidibanks[i] = 0;
	}

	for (i = 0; i < 16; i++) ch[i].xmidi = false;

	if (type == TIMBRE_LIBRARY_XMIDI_AD)
		loadXMIDITimbres(ds);
	else if (type == TIMBRE_LIBRARY_U7VOICE_AD)
		loadU7VoiceTimbres(ds);
}

void FMOplMidiDriver::loadXMIDITimbres(IDataSource *ds) {
	bool read[128];
	std::memset(read, false, sizeof(read));

	// Read all the timbres
	uint32 i;
	for (i = 0; ds->getPos() < ds->getSize(); i++) {
		// Seek to the entry
		ds->seek(i * 6);

		uint32 patch = (uint8) ds->read1();
		uint32 bank = (uint8) ds->read1();

		// If we read both == 255 then we've read all of them
		if (patch == 255 || bank == 255) {
			//POUT ("Finished " << patch << ": ");
			break;
		}

		// Get offset and seek to it
		uint32 offset = ds->read4();

		//POUT ("Patch " << i << " = " << bank << ":" << patch << " @ " << offset);

		// Check to see if the bank exists
		if (!xmidibanks[bank]) {
			xmidibanks[bank] = new xmidibank;
			std::memset(xmidibanks[bank], 0, sizeof(xmidibank));
		}

		// Seek to offset
		ds->seek(offset);

		// Get len of the entry
		uint16 len = ds->read2();

		// Only accept lens that are 0xC
		if (len != 0xE) {
			POUT("Invalid Patch " << bank << ":" << patch << " len was " << len << " " << std::hex << len << std::dec);
			//continue;
		}

		// Skip 1 byte
		ds->skip(1);

		struct {
			unsigned char mod_avekm;     // 0    (20)
			unsigned char mod_ksl_tl;    // 1    (40)
			unsigned char mod_ad;        // 2    (60)
			unsigned char mod_sr;        // 3    (80)
			unsigned char mod_ws;        // 4    (E0)

			unsigned char fb_c;          // 5

			unsigned char car_avekm;     // 6    amp, sussnd             (20)
			unsigned char car_ksl_tl;    // 7    outlev, keyscale        (43)
			unsigned char car_ad;        // 8    Attack Delay        AR  (63)
			unsigned char car_sr;        // 9    SustainLev Release  DR  (83)
			unsigned char car_ws;        // 10   Waveform                (E0)
		} xmidi_ins;

		ds->read(&xmidi_ins, 11);

		unsigned char *ins = xmidibanks[bank]->insbank[patch];

		ins[INDEX_AVEKM_M] = xmidi_ins.mod_avekm;
		ins[INDEX_KSLTL_M] = xmidi_ins.mod_ksl_tl;
		ins[INDEX_AD_M] = xmidi_ins.mod_ad;
		ins[INDEX_SR_M] = xmidi_ins.mod_sr;
		ins[INDEX_WAVE_M] = xmidi_ins.mod_ws;
		ins[INDEX_AVEKM_C] = xmidi_ins.car_avekm;
		ins[INDEX_KSLTL_C] = xmidi_ins.car_ksl_tl;
		ins[INDEX_AD_C] = xmidi_ins.car_ad;
		ins[INDEX_SR_C] = xmidi_ins.car_sr;
		ins[INDEX_WAVE_C] = xmidi_ins.car_ws;
		ins[INDEX_FB_C] = xmidi_ins.fb_c;
		ins[INDEX_PERC] = 0x80; // Note that XMIDI uses a different method to U7:TBG
	}

//	POUT ("FMOplMidiDriver: " << i << " timbres read");

	for (i = 0; i < 16; i++) ch[i].xmidi = true;
}

void FMOplMidiDriver::loadU7VoiceTimbres(IDataSource *ds) {
	struct u7voice_adlib_ins {
		unsigned char mod_avekm;            // 0:   (20)
		unsigned char mod_ksl_tl;           // 1:   (40)
		unsigned char mod_ad;               // 2:   (60)
		unsigned char mod_sr;               // 3:   (80)
		unsigned char mod_ws;               // 4:   (E0)

		unsigned char car_avekm;            // 5:   amp, sussnd             (22)
		unsigned char car_ksl_tl;           // 6:   outlev, keyscale        (43)
		unsigned char car_ad;               // 7:   Attack Delay        AR  (63)
		unsigned char car_sr;               // 8:   SustainLev Release  DR  (83)
		unsigned char car_ws;               // 9:   Waveform                (E3)

		unsigned char fb_c;                 // 10:  Feedback/Connection

		// NOT IMPLEMENTED from here on!

		unsigned char perc_voice;           // 11   Percussion voice number !!
		// (0=melodic, 6=bass drum, etc.)

		unsigned char car_vel_sense;        // 12:  carrier velocity sensitivity
		unsigned char mod_vel_sense;        // 13:  modulator velocity sensitivity
		unsigned char bend_sense;           // 14:  pitch bend sensitivity
		unsigned char wheel_sense;          // 15:  modulation wheel sensitivity
		unsigned char lfo_speed;            // 16:  lfo speed
		unsigned char lfo_depth;            // 17:  lfo depth

		unsigned short pe_start_level;      // 18-19:  pitch envelope start level
		unsigned short pe_attack_rate1;     // 20-21:  pitch envelope attack rate 1
		unsigned short pe_attack_level1;    // 22-23:  pitch envelope attack level 1
		unsigned short pe_attack_rate2;     // 24-25:  pitch envelope attack rate 2
		unsigned short pe_attack_level2;    // 26-27:  pitch envelope attack level 2
		unsigned short pe_decay_rate;       // 28-29:  pitch envelope decay rate
		unsigned short pe_sustain_level;    // 30-31:  pitch envelope sustain level
		unsigned short pe_release_rate;     // 32-33:  pitch envelope release rate
		unsigned short pe_end_level;        // 34-35:  pitch envelope end level

		unsigned char deturn;               // 36:  detune
		unsigned char transpose;            // 37:  transpose
		unsigned char next_partial;         // 38:  next partial number (+1; 0=no more partials)
		unsigned char key_follow;           // 39:  key follow
		unsigned char reserved[7];          // 40-46:  reserved

		unsigned char prog_num;             // 47:  program change number

		void read(IDataSource *ds) {
			mod_avekm = ds->read1();
			mod_ksl_tl = ds->read1();
			mod_ad = ds->read1();
			mod_sr = ds->read1();
			mod_ws = ds->read1();
			car_avekm = ds->read1();
			car_ksl_tl = ds->read1();
			car_ad = ds->read1();
			car_sr = ds->read1();
			car_ws = ds->read1();
			fb_c = ds->read1();

			// NOT IMPLEMENTED from here on!

			perc_voice = ds->read1();

			car_vel_sense = ds->read1();
			mod_vel_sense = ds->read1();
			bend_sense = ds->read1();
			wheel_sense = ds->read1();
			lfo_speed = ds->read1();
			lfo_depth = ds->read1();

			pe_start_level = ds->read2();
			pe_attack_rate1 = ds->read2();
			pe_attack_level1 = ds->read2();
			pe_attack_rate2 = ds->read2();
			pe_attack_level2 = ds->read2();
			pe_decay_rate = ds->read2();
			pe_sustain_level = ds->read2();
			pe_release_rate = ds->read2();
			pe_end_level = ds->read2();

			deturn = ds->read1();
			transpose = ds->read1();
			next_partial = ds->read1();
			key_follow = ds->read1();
			ds->read((char *) reserved, 7);

			prog_num = ds->read1();
		}
	} u7voice_ins;

	//POUT("Size of u7voice_adlib_ins " << sizeof(u7voice_adlib_ins));

	int count = ds->read1() & 0xFF;

	// Read all the timbres
	int i;
	for (i = 0; i < count; i++) {

		// Read the timbre
		u7voice_ins.read(ds);

		uint32 patch = u7voice_ins.prog_num;
		uint32 bank = 0;

		//POUT ("Patch " << i << " = " << bank << ":" << patch);

		// Check to see if the bank exists
		if (!xmidibanks[bank]) {
			xmidibanks[bank] = new xmidibank;
			std::memset(xmidibanks[bank], 0, sizeof(xmidibank));
		}

		if (patch > 127) {
			//PERR ("Don't know how to handle this");
			continue;
		}

		unsigned char *ins = xmidibanks[bank]->insbank[patch];

		ins[INDEX_AVEKM_M] = u7voice_ins.mod_avekm;
		ins[INDEX_KSLTL_M] = u7voice_ins.mod_ksl_tl;
		ins[INDEX_AD_M] = u7voice_ins.mod_ad;
		ins[INDEX_SR_M] = u7voice_ins.mod_sr;
		ins[INDEX_WAVE_M] = u7voice_ins.mod_ws;
		ins[INDEX_AVEKM_C] = u7voice_ins.car_avekm;
		ins[INDEX_KSLTL_C] = u7voice_ins.car_ksl_tl;
		ins[INDEX_AD_C] = u7voice_ins.car_ad;
		ins[INDEX_SR_C] = u7voice_ins.car_sr;
		ins[INDEX_WAVE_C] = u7voice_ins.car_ws;
		ins[INDEX_FB_C] = u7voice_ins.fb_c;
		ins[INDEX_PERC] = u7voice_ins.perc_voice | 0x80;    // Note that XMIDI uses a different method to U7:TBG

		if (u7voice_ins.next_partial || u7voice_ins.key_follow) {
			//  POUT("next_partial " << (int) u7voice_ins.next_partial << "  key_follow " << (int) u7voice_ins.key_follow);
		}
	}

	//POUT (i << " timbres read");

	for (i = 0; i < 16; i++) ch[i].xmidi = true;
}

} // End of namespace Ultima8

#endif
