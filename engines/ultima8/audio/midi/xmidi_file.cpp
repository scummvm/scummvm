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
#include "XMidiFile.h"
#include "XMidiEvent.h"
#include "XMidiEventList.h"
#include "XMidiNoteStack.h"

#ifdef PENTAGRAM_IN_EXULT
#include "ultima8/games/game.h"
#include "databuf.h"
#else
#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"
#endif

namespace Ultima8 {

#ifndef UNDER_CE
using std::atof;
using std::atoi;
using std::memcmp;
using std::memcpy;
using std::memset;
using std::size_t;
#endif
using std::string;
using std::endl;

#ifndef UNDER_CE
#define XMidiEvent__Malloc XMidiEvent::Malloc
#define XMidiEvent__Calloc XMidiEvent::Calloc
#else

template<class T>
static inline T *XMidiEvent__Malloc(size_t num = 1) {
	return static_cast<T *>(std::malloc(num));
}

template<class T>
static inline T *XMidiEvent__Calloc(size_t num = 1, size_t sz = 0) {
	if (!sz) sz = sizeof(T);
	return static_cast<T *>(std::calloc(num, sz));
}

#endif

//#include "gamma.h"

// This is used to correct incorrect patch, vol and pan changes in midi files
// The bias is just a value to used to work out if a vol and pan belong with a
// patch change. This is to ensure that the default state of a midi file is with
// the tracks centred, unless the first patch change says otherwise.
#define PATCH_VOL_PAN_BIAS  5


// This is a default set of patches to convert from MT32 to GM
// The index is the MT32 Patch nubmer and the value is the GM Patch
// This is only suitable for music that doesn'tdo timbre changes
// XMidis that contain Timbre changes will not convert properly
const char XMidiFile::mt32asgm[128] = {
	0,  // 0    Piano 1
	1,  // 1    Piano 2
	2,  // 2    Piano 3 (synth)
	4,  // 3    EPiano 1
	4,  // 4    EPiano 2
	5,  // 5    EPiano 3
	5,  // 6    EPiano 4
	3,  // 7    Honkytonk
	16, // 8    Organ 1
	17, // 9    Organ 2
	18, // 10   Organ 3
	16, // 11   Organ 4
	19, // 12   Pipe Organ 1
	19, // 13   Pipe Organ 2
	19, // 14   Pipe Organ 3
	21, // 15   Accordion
	6,  // 16   Harpsichord 1
	6,  // 17   Harpsichord 2
	6,  // 18   Harpsichord 3
	7,  // 19   Clavinet 1
	7,  // 20   Clavinet 2
	7,  // 21   Clavinet 3
	8,  // 22   Celesta 1
	8,  // 23   Celesta 2
	62, // 24   Synthbrass 1 (62)
	63, // 25   Synthbrass 2 (63)
	62, // 26   Synthbrass 3 Bank 8
	63, // 27   Synthbrass 4 Bank 8
	38, // 28   Synthbass 1
	39, // 29   Synthbass 2
	38, // 30   Synthbass 3 Bank 8
	39, // 31   Synthbass 4 Bank 8
	88, // 32   Fantasy
	90, // 33   Harmonic Pan - No equiv closest is polysynth(90) :(
	52, // 34   Choral ?? Currently set to SynthVox(54). Should it be ChoirAhhs(52)???
	92, // 35   Glass
	97, // 36   Soundtrack
	99, // 37   Atmosphere
	14, // 38   Warmbell, sounds kind of like crystal(98) perhaps Tubular Bells(14) would be better. It is!
	54, // 39   FunnyVox, sounds alot like Bagpipe(109) and Shania(111)
	98, // 40   EchoBell, no real equiv, sounds like Crystal(98)
	96, // 41   IceRain
	68, // 42   Oboe 2001, no equiv, just patching it to normal oboe(68)
	95, // 43   EchoPans, no equiv, setting to SweepPad
	81, // 44   DoctorSolo Bank 8
	87, // 45   SchoolDaze, no real equiv
	112,    // 46   Bell Singer
	80, // 47   SquareWave
	48, // 48   Strings 1
	48, // 49   Strings 2 - should be 49
	44, // 50   Strings 3 (Synth) - Experimental set to Tremollo Strings - should be 50
	45, // 51   Pizzicato Strings
	40, // 52   Violin 1
	40, // 53   Violin 2 ? Viola
	42, // 54   Cello 1
	42, // 55   Cello 2
	43, // 56   Contrabass
	46, // 57   Harp 1
	46, // 58   Harp 2
	24, // 59   Guitar 1 (Nylon)
	25, // 60   Guitar 2 (Steel)
	26, // 61   Elec Guitar 1
	27, // 62   Elec Guitar 2
	104,    // 63   Sitar
	32, // 64   Acou Bass 1
	32, // 65   Acou Bass 2
	33, // 66   Elec Bass 1
	34, // 67   Elec Bass 2
	36, // 68   Slap Bass 1
	37, // 69   Slap Bass 2
	35, // 70   Fretless Bass 1
	35, // 71   Fretless Bass 2
	73, // 72   Flute 1
	73, // 73   Flute 2
	72, // 74   Piccolo 1
	72, // 75   Piccolo 2
	74, // 76   Recorder
	75, // 77   Pan Pipes
	64, // 78   Sax 1
	65, // 79   Sax 2
	66, // 80   Sax 3
	67, // 81   Sax 4
	71, // 82   Clarinet 1
	71, // 83   Clarinet 2
	68, // 84   Oboe
	69, // 85   English Horn (Cor Anglais)
	70, // 86   Bassoon
	22, // 87   Harmonica
	56, // 88   Trumpet 1
	56, // 89   Trumpet 2
	57, // 90   Trombone 1
	57, // 91   Trombone 2
	60, // 92   French Horn 1
	60, // 93   French Horn 2
	58, // 94   Tuba
	61, // 95   Brass Section 1
	61, // 96   Brass Section 2
	11, // 97   Vibes 1
	11, // 98   Vibes 2
	99, // 99   Syn Mallet Bank 1
	112,    // 100  WindBell no real equiv Set to TinkleBell(112)
	9,  // 101  Glockenspiel
	14, // 102  Tubular Bells
	13, // 103  Xylophone
	12, // 104  Marimba
	107,    // 105  Koto
	111,    // 106  Sho?? set to Shanai(111)
	77, // 107  Shakauhachi
	78, // 108  Whistle 1
	78, // 109  Whistle 2
	76, // 110  Bottle Blow
	76, // 111  Breathpipe no real equiv set to bottle blow(76)
	47, // 112  Timpani
	117,    // 113  Melodic Tom
	116,    // 114  Deap Snare no equiv, set to Taiko(116)
	118,    // 115  Electric Perc 1
	118,    // 116  Electric Perc 2
	116,    // 117  Taiko
	115,    // 118  Taiko Rim, no real equiv, set to Woodblock(115)
	119,    // 119  Cymbal, no real equiv, set to reverse cymbal(119)
	115,    // 120  Castanets, no real equiv, in GM set to Woodblock(115)
	112,    // 121  Triangle, no real equiv, set to TinkleBell(112)
	55, // 122  Orchestral Hit
	124,    // 123  Telephone
	123,    // 124  BirdTweet
	94, // 125  Big Notes Pad no equiv, set to halo pad (94)
	98, // 126  Water Bell set to Crystal Pad(98)
	121 // 127  Jungle Tune set to Breath Noise
};

// Same as above, except include patch changes
// so GS instruments can be used
const char XMidiFile::mt32asgs[256] = {
	0, 0,   // 0    Piano 1
	1, 0,   // 1    Piano 2
	2, 0,   // 2    Piano 3 (synth)
	4, 0,   // 3    EPiano 1
	4, 0,   // 4    EPiano 2
	5, 0,   // 5    EPiano 3
	5, 0,   // 6    EPiano 4
	3, 0,   // 7    Honkytonk
	16, 0,  // 8    Organ 1
	17, 0,  // 9    Organ 2
	18, 0,  // 10   Organ 3
	16, 0,  // 11   Organ 4
	19, 0,  // 12   Pipe Organ 1
	19, 0,  // 13   Pipe Organ 2
	19, 0,  // 14   Pipe Organ 3
	21, 0,  // 15   Accordion
	6, 0,   // 16   Harpsichord 1
	6, 0,   // 17   Harpsichord 2
	6, 0,   // 18   Harpsichord 3
	7, 0,   // 19   Clavinet 1
	7, 0,   // 20   Clavinet 2
	7, 0,   // 21   Clavinet 3
	8, 0,   // 22   Celesta 1
	8, 0,   // 23   Celesta 2
	62, 0,  // 24   Synthbrass 1 (62)
	63, 0,  // 25   Synthbrass 2 (63)
	62, 0,  // 26   Synthbrass 3 Bank 8
	63, 0,  // 27   Synthbrass 4 Bank 8
	38, 0,  // 28   Synthbass 1
	39, 0,  // 29   Synthbass 2
	38, 0,  // 30   Synthbass 3 Bank 8
	39, 0,  // 31   Synthbass 4 Bank 8
	88, 0,  // 32   Fantasy
	90, 0,  // 33   Harmonic Pan - No equiv closest is polysynth(90) :(
	52, 0,  // 34   Choral ?? Currently set to SynthVox(54). Should it be ChoirAhhs(52)???
	92, 0,  // 35   Glass
	97, 0,  // 36   Soundtrack
	99, 0,  // 37   Atmosphere
	14, 0,  // 38   Warmbell, sounds kind of like crystal(98) perhaps Tubular Bells(14) would be better. It is!
	54, 0,  // 39   FunnyVox, sounds alot like Bagpipe(109) and Shania(111)
	98, 0,  // 40   EchoBell, no real equiv, sounds like Crystal(98)
	96, 0,  // 41   IceRain
	68, 0,  // 42   Oboe 2001, no equiv, just patching it to normal oboe(68)
	95, 0,  // 43   EchoPans, no equiv, setting to SweepPad
	81, 0,  // 44   DoctorSolo Bank 8
	87, 0,  // 45   SchoolDaze, no real equiv
	112, 0, // 46   Bell Singer
	80, 0,  // 47   SquareWave
	48, 0,  // 48   Strings 1
	48, 0,  // 49   Strings 2 - should be 49
	44, 0,  // 50   Strings 3 (Synth) - Experimental set to Tremollo Strings - should be 50
	45, 0,  // 51   Pizzicato Strings
	40, 0,  // 52   Violin 1
	40, 0,  // 53   Violin 2 ? Viola
	42, 0,  // 54   Cello 1
	42, 0,  // 55   Cello 2
	43, 0,  // 56   Contrabass
	46, 0,  // 57   Harp 1
	46, 0,  // 58   Harp 2
	24, 0,  // 59   Guitar 1 (Nylon)
	25, 0,  // 60   Guitar 2 (Steel)
	26, 0,  // 61   Elec Guitar 1
	27, 0,  // 62   Elec Guitar 2
	104, 0, // 63   Sitar
	32, 0,  // 64   Acou Bass 1
	32, 0,  // 65   Acou Bass 2
	33, 0,  // 66   Elec Bass 1
	34, 0,  // 67   Elec Bass 2
	36, 0,  // 68   Slap Bass 1
	37, 0,  // 69   Slap Bass 2
	35, 0,  // 70   Fretless Bass 1
	35, 0,  // 71   Fretless Bass 2
	73, 0,  // 72   Flute 1
	73, 0,  // 73   Flute 2
	72, 0,  // 74   Piccolo 1
	72, 0,  // 75   Piccolo 2
	74, 0,  // 76   Recorder
	75, 0,  // 77   Pan Pipes
	64, 0,  // 78   Sax 1
	65, 0,  // 79   Sax 2
	66, 0,  // 80   Sax 3
	67, 0,  // 81   Sax 4
	71, 0,  // 82   Clarinet 1
	71, 0,  // 83   Clarinet 2
	68, 0,  // 84   Oboe
	69, 0,  // 85   English Horn (Cor Anglais)
	70, 0,  // 86   Bassoon
	22, 0,  // 87   Harmonica
	56, 0,  // 88   Trumpet 1
	56, 0,  // 89   Trumpet 2
	57, 0,  // 90   Trombone 1
	57, 0,  // 91   Trombone 2
	60, 0,  // 92   French Horn 1
	60, 0,  // 93   French Horn 2
	58, 0,  // 94   Tuba
	61, 0,  // 95   Brass Section 1
	61, 0,  // 96   Brass Section 2
	11, 0,  // 97   Vibes 1
	11, 0,  // 98   Vibes 2
	99, 0,  // 99   Syn Mallet Bank 1
	112, 0, // 100  WindBell no real equiv Set to TinkleBell(112)
	9, 0,   // 101  Glockenspiel
	14, 0,  // 102  Tubular Bells
	13, 0,  // 103  Xylophone
	12, 0,  // 104  Marimba
	107, 0, // 105  Koto
	111, 0, // 106  Sho?? set to Shanai(111)
	77, 0,  // 107  Shakauhachi
	78, 0,  // 108  Whistle 1
	78, 0,  // 109  Whistle 2
	76, 0,  // 110  Bottle Blow
	76, 0,  // 111  Breathpipe no real equiv set to bottle blow(76)
	47, 0,  // 112  Timpani
	117, 0, // 113  Melodic Tom
	116, 0, // 114  Deap Snare no equiv, set to Taiko(116)
	118, 0, // 115  Electric Perc 1
	118, 0, // 116  Electric Perc 2
	116, 0, // 117  Taiko
	115, 0, // 118  Taiko Rim, no real equiv, set to Woodblock(115)
	119, 0, // 119  Cymbal, no real equiv, set to reverse cymbal(119)
	115, 0, // 120  Castanets, no real equiv, in GM set to Woodblock(115)
	112, 0, // 121  Triangle, no real equiv, set to TinkleBell(112)
	55, 0,  // 122  Orchestral Hit
	124, 0, // 123  Telephone
	123, 0, // 124  BirdTweet
	94, 0,  // 125  Big Notes Pad no equiv, set to halo pad (94)
	98, 0,  // 126  Water Bell set to Crystal Pad(98)
	121, 0  // 127  Jungle Tune set to Breath Noise
};

// Reverse mapping. GM Notes converted to MT-32 patches
const char XMidiFile::gmasmt32[128] = {
	0x00, 0x01, 0x03, 0x07, 0x05, 0x06, 0x11, 0x15,
	0x16, 0x65, 0x65, 0x62, 0x68, 0x67, 0x66, 0x69,

	0x0C, 0x09, 0x0A, 0x0D, 0x0E, 0x0F, 0x57, 0x0F,
	0x3B, 0x3C, 0x3B, 0x3E, 0x3D, 0x3B, 0x3E, 0x3E,

	0x40, 0x43, 0x42, 0x47, 0x44, 0x45, 0x42 , 0x46,
	0x35, 0x34, 0x36, 0x38, 0x35, 0x33, 0x39 , 0x70,

	0X30, 0x32, 0x30, 0x32, 0x22, 0x2A, 0x21, 0x7A,
	0X58, 0x5A, 0x5E, 0x59, 0x5C, 0x5F, 0x59, 0x5B,

	0x4E, 0x4F, 0x50, 0x51, 0x54, 0x55, 0x56, 0x53,
	0x4B, 0x49, 0x4C, 0x4D, 0x6E, 0x6B, 0x6C, 0x48,

	0x2F, 0x43, 0x4B, 0x33, 0x3D, 0x48, 0x34, 0x43,
	0x20, 0x21, 0x43, 0x22, 0x20, 0x20, 0x21, 0x21,

	0x29, 0x24, 0x23, 0x25, 0x2D, 0x21, 0x2B, 0x20,
	0x3F, 0x69, 0x69, 0x69, 0x33, 0x51, 0x34, 0x51,

	0x17, 0x67, 0x67, 0x71, 0x75, 0x71, 0x74, 0x77,
	0x7C, 0x78, 0x77, 0x7C, 0x7B, 0x78, 0x77, 0x72
};

//
// MT32 SysEx
//
static const uint32 sysex_data_start = 7;       // Data starts at byte 7
static const uint32 sysex_max_data_size = 256;


//
// Percussion
//

static const uint32 rhythm_base = 0x030110; // Note, these are 7 bit!
static const uint32 rhythm_mem_size = 4;

static const uint32 rhythm_first_note = 24;
static const uint32 rhythm_num_notes = 64;

// Memory offset based on index in the table
static inline uint32 rhythm_mem_offset(uint32 index_num) {
	return index_num * 4;
}

// Memory offset based on note key num
static inline uint32 rhythm_mem_offset_note(uint32 rhythm_note_num) {
	return (rhythm_note_num - rhythm_first_note) * 4;
}

struct RhythmSetupData {
	uint8       timbre;                 // 0-94 (M1-M64,R1-30,OFF)
	uint8       output_level;           // 0-100
	uint8       panpot;                 // 0-14 (L-R)
	uint8       reverb_switch;          // 0-1 (off,on)
};


//
// Timbre Memory Consts
//
static const uint32 timbre_base = 0x080000; // Note, these are 7 bit!
static const uint32 timbre_mem_size = 246;
static inline uint32 timbre_mem_offset(uint32 timbre_num) {
	return timbre_num * 256;
}


//
// Patch Memory Consts
//
static const uint32 patch_base = 0x050000;      // Note, these are 7 bit!
static const uint32 patch_mem_size = 8;
static inline uint32 patch_mem_offset(uint32 patch_num) {
	return patch_num * 8;
}

struct PatchMemData {
	uint8       timbre_group;           // 0-3  (group A, group B, Memory, Rhythm)
	uint8       timbre_num;             // 0-63
	uint8       key_shift;              // 0-48
	uint8       fine_tune;              // 0-100 (-50 - +50)
	uint8       bender_range;           // 0-24
	uint8       assign_mode;            // 0-3 (POLY1, POLY2, POLY3, POLY4)
	uint8       reverb_switch;          // 0-1 (off,on)
	uint8       dummy;
};

static const PatchMemData patch_template = {
	2,      // timbre_group
	0,      // timbre_num
	24,     // key_shift
	50,     // fine_tune
	24,     // bender_range
	0,      // assign_mode
	1,      // reverb_switch
	0       // dummy
};


//
// System Area Consts
//
static const uint32 system_base = 0x100000; // Note, these are 7 bit!
static const uint32 system_mem_size = 0x17; // Display is 20 ASCII characters (32-127)
#define system_mem_offset(setting) ((uintptr)(&((systemArea*)0)->setting))

struct systemArea {
	char masterTune;                    // MASTER TUNE 0-127 432.1-457.6Hz
	char reverbMode;                    // REVERB MODE 0-3 (room, hall, plate, tap delay)
	char reverbTime;                    // REVERB TIME 0-7 (1-8)
	char reverbLevel;                   // REVERB LEVEL 0-7 (1-8)
	char reserveSettings[9];            // PARTIAL RESERVE (PART 1) 0-32
	char chanAssign[9];                 // MIDI CHANNEL (PART1) 0-16 (1-16,OFF)
	char masterVol;                     // MASTER VOLUME 0-100
};

static const char system_init_reverb[3] = { 0, 3, 2 };              // reverb mode = 0, time = 3, level = 2
static const char system_part_chans[9] =  { 1, 2, 3, 4, 5, 6, 7, 8, 9 }; // default (0-based) chans for each part
static const char system_part_rsv[9] = { 3, 4, 3, 4, 3, 4, 3, 4, 4 }; // # of reserved AIL partials/channel

//
// Display  Consts
//
static const uint32 display_base = 0x200000;    // Note, these are 7 bit!
static const uint32 display_mem_size = 0x14;    // Display is 20 ASCII characters (32-127)

// Display messages                         0123456789ABCDEF0123
#ifdef PENTAGRAM_IN_EXULT
static const char display[]              = " Uploading Timbres! ";
static const char display_black_gate[]   = "BG Uploading Timbres";
static const char display_serpent_isle[] = "SI Uploading Timbres";

static const char display_beginning[] =    "--==|  Exult!  |==--";
static const char display_beginning_bg[] = " U7: The Black Gate ";
static const char display_beginning_si[] = "U7: The Serpent Isle";
#else

static const char display[]              = " Uploading Timbres! ";
static const char display_beginning[] =    "--=| Pentagram! |=--";
#endif

//
// All Dev Reset
//
static const uint32 all_dev_reset_base = 0x7f0000;


//
// U7 Percussion Table
//
// Why this crap wasn't in the flexes i will never know
//

// The key num that the data below belongs to (subtract 24 to get memory num)
static uint8 U7PercussionNotes[] = {
	28, 33, 74, 76, 77, 78, 79, 80,
	81, 82, 83, 84, 85, 86, 87, 0
};

// The RhythmSetup data
static RhythmSetupData U7PercussionData[] = {
	{   0,  0x5A,   0x07,   0   },  // 28
	{   6,  0x64,   0x07,   1   },  // 33
	{   1,  0x5A,   0x05,   0   },  // 74
	{   1,  0x5A,   0x06,   0   },  // 76
	{   1,  0x5A,   0x07,   0   },  // 77
	{   2,  0x64,   0x07,   1   },  // 78
	{   1,  0x5A,   0x08,   0   },  // 79
	{   5,  0x5A,   0x07,   1   },  // 80
	{   1,  0x5A,   0x09,   0   },  // 81
	{   3,  0x5F,   0x07,   1   },  // 82
	{   4,  0x64,   0x04,   1   },  // 83
	{   4,  0x64,   0x05,   1   },  // 84
	{   4,  0x64,   0x06,   1   },  // 85
	{   4,  0x64,   0x07,   1   },  // 86
	{   4,  0x64,   0x08,   1   }   // 87
};

//GammaTable<unsigned char> XMidiFile::VolumeCurve(128);

// Constructor
XMidiFile::XMidiFile(IDataSource *source, int pconvert) : num_tracks(0),
	events(NULL), convert_type(pconvert),
	do_reverb(false), do_chorus(false) {
	std::memset(bank127, 0, sizeof(bank127));

	ExtractTracks(source);

	// SysEx data
	if (pconvert >= XMIDIFILE_HINT_U7VOICE_MT_FILE) InsertDisplayEvents();
}

XMidiFile::~XMidiFile() {
	if (events) {
		for (int i = 0; i < num_tracks; i++) {
			events[i]->decerementCounter();
			events[i] = NULL;
		}
		//delete [] events;
		XMidiEvent::Free(events);
	}
}

XMidiEventList *XMidiFile::GetEventList(uint32 track) {
	if (!events) {
		perr << "No midi data in loaded." << endl;
		return 0;
	}

	if (track >= num_tracks) {
		perr << "Can't retrieve MIDI data, track out of range" << endl;
		return 0;
	}

	return events[track];
}

// Sets current to the new event and updates list
void XMidiFile::CreateNewEvent(int time) {
	if (!list) {
		list = current = XMidiEvent__Calloc<XMidiEvent>();
		if (time > 0)
			current->time = time;
		return;
	}

	if (time < 0 || list->time > time) {
		XMidiEvent *event = XMidiEvent__Calloc<XMidiEvent>();
		event->next = list;
		list = current = event;
		return;
	}

	if (!current || current->time > time)
		current = list;

	while (current->next) {
		if (current->next->time > time) {
			XMidiEvent *event = XMidiEvent__Calloc<XMidiEvent>();

			event->next = current->next;
			current->next = event;
			current = event;
			current->time = time;
			return;
		}

		current = current->next;
	}

	current->next = XMidiEvent__Calloc<XMidiEvent>();
	current = current->next;
	current->time = time;
}

//
// GetVLQ
//
// Get a Conventional Variable Length Quantity
//
int XMidiFile::GetVLQ(IDataSource *source, uint32 &quant) {
	int i;
	quant = 0;
	unsigned int data;

	for (i = 0; i < 4; i++) {
		data = source->read1();
		quant <<= 7;
		quant |= data & 0x7F;

		if (!(data & 0x80)) {
			i++;
			break;
		}

	}
	return i;
}

//
// GetVLQ2
//
// Get a XMidiFile Variable Length Quantity
//
int XMidiFile::GetVLQ2(IDataSource *source, uint32 &quant) {
	int i;
	quant = 0;
	int data = 0;

	for (i = 0; i < 4; i++) {
		data = source->read1();
		if (data & 0x80) {
			source->skip(-1);
			break;
		}
		quant += data;
	}
	return i;
}

//
// MovePatchVolAndPan.
//
// Well, this is just a modified version of what that method used to do. This
// is a massive optimization. Speed up should be quite impressive
//
void XMidiFile::ApplyFirstState(first_state &fs, int chan_mask) {
	for (int channel = 0; channel < 16; channel++) {
		XMidiEvent *patch = fs.patch[channel];
		XMidiEvent *vol = fs.vol[channel];
		XMidiEvent *pan = fs.pan[channel];
		XMidiEvent *bank = fs.bank[channel];
		XMidiEvent *reverb = NULL;
		XMidiEvent *chorus = NULL;
		XMidiEvent *temp;

		// Got no patch change, return and don't try fixing it
		if (!patch || !(chan_mask & 1 << channel)) continue;
#if 0
		std::cout << "Channel: " << channel + 1 << std::endl;
		std::cout << "Patch: " << (unsigned int) patch->data[0] << " @ " << patch->time << std::endl;
		if (bank) std::cout << " Bank: " << (unsigned int) bank->data[1] << " @ " << bank->time << std::endl;
		if (vol) std::cout << "  Vol: " << (unsigned int) vol->data[1] << " @ " << vol->time << std::endl;
		if (pan) std::cout << "  Pan: " << ((signed int) pan->data[1]) - 64 << " @ " << pan->time << std::endl;
		std::cout << std::endl;
#endif

		// Copy Patch Change Event
		temp = patch;
		patch = XMidiEvent__Calloc<XMidiEvent>();
		patch->time = temp->time;
		patch->status = channel | (MIDI_STATUS_PROG_CHANGE << 4);
		patch->data[0] = temp->data[0];

		// Copy Volume
		if (vol && (vol->time > patch->time + PATCH_VOL_PAN_BIAS || vol->time < patch->time - PATCH_VOL_PAN_BIAS))
			vol = NULL;

		temp = vol;
		vol = XMidiEvent__Calloc<XMidiEvent>();
		vol->status = channel | (MIDI_STATUS_CONTROLLER << 4);
		vol->data[0] = 7;

		if (!temp) {
//			if (convert_type) vol->data[1] = VolumeCurve[90];
			if (convert_type) vol->data[1] = 90;
			else vol->data[1] = 90;
		} else
			vol->data[1] = temp->data[1];


		// Copy Bank
		if (bank && (bank->time > patch->time + PATCH_VOL_PAN_BIAS || bank->time < patch->time - PATCH_VOL_PAN_BIAS))
			bank = NULL;

		temp = bank;

		bank = XMidiEvent__Calloc<XMidiEvent>();
		bank->status = channel | (MIDI_STATUS_CONTROLLER << 4);

		if (!temp)
			bank->data[1] = 0;
		else
			bank->data[1] = temp->data[1];

		// Copy Pan
		if (pan && (pan->time > patch->time + PATCH_VOL_PAN_BIAS || pan->time < patch->time - PATCH_VOL_PAN_BIAS))
			pan = NULL;

		temp = pan;
		pan = XMidiEvent__Calloc<XMidiEvent>();
		pan->status = channel | (MIDI_STATUS_CONTROLLER << 4);
		pan->data[0] = 10;

		if (!temp)
			pan->data[1] = 64;
		else
			pan->data[1] = temp->data[1];

		if (do_reverb) {
			reverb = XMidiEvent__Calloc<XMidiEvent>();
			reverb->status = channel | (MIDI_STATUS_CONTROLLER << 4);
			reverb->data[0] = 91;
			reverb->data[1] = reverb_value;
		}

		if (do_chorus) {
			chorus = XMidiEvent__Calloc<XMidiEvent>();
			chorus->status = channel | (MIDI_STATUS_CONTROLLER << 4);
			chorus->data[0] = 93;
			chorus->data[1] = chorus_value;
		}

		vol->time = 0;
		pan->time = 0;
		patch->time = 0;
		bank->time = 0;

		if (do_reverb && do_chorus) reverb->next = chorus;
		else if (do_reverb) reverb->next = bank;
		if (do_chorus) chorus->next = bank;
		bank->next = vol;
		vol->next = pan;
		pan->next = patch;

		patch->next = list;
		if (do_reverb) list = reverb;
		else if (do_chorus) list = chorus;
		else list = bank;
	}
}

#ifndef BEOS
// Unsigned 64 Bit Int emulation. Only supports SOME operations
struct uint64 {
	uint32  low;        // Low is first so uint64 can be cast as uint32 to get low dword
	uint32  high;       //

	uint64() : low(0), high(0) { }
	uint64(uint32 i) : low(i), high(0) { }
	uint64(uint32 h, uint32 l) : low(l), high(h) { }
	uint64(const uint64 &i) : low(i.low), high(i.high) { }

	inline void addlow(uint32 l) {
		uint32 mid = (low >> 16);
		low = (low & 0xFFFF) + (l & 0xFFFF);
		mid += (low >> 16) + (l >> 16);
		low = (low & 0xFFFF) + (mid << 16);
		high += mid >> 16;
	}

	// uint64 operations

	inline uint64 &operator = (uint64 &o) {
		low = o.low;
		high = o.high;
		return *this;
	}

	inline uint64 &operator += (uint64 &o) {
		addlow(o.low);
		high += o.high;
		return *this;
	}

	inline uint64 operator + (uint64 &o) {
		uint64 n(*this);
		n.addlow(o.low);
		n.high += o.high;
		return n;
	}

	// uint32 operations

	inline uint64 &operator = (uint32 i) {
		low = i;
		high = 0;
		return *this;
	}

	inline uint64 &operator += (uint32 i) {
		addlow(i);
		return *this;
	}

	inline uint64 operator + (uint32 i) {
		uint64 n(*this);
		n.addlow(i);
		return n;
	}

	inline uint64 &operator *= (uint32 i) {
		// High 16 bits
		uint32 h1 =   i >> 16;
		uint32 h2 = low >> 16;
		//uint32 h3 = high >> 16;

		// Low 16 Bits
		uint32 l1 =   i & 0xFFFF;
		uint32 l2 = low & 0xFFFF;
		uint32 l3 = high & 0xFFFF;

		// The accumulator
		uint32 accum;

		// 0 -> 32
		low = l1 * l2;
		high = 0;

		// 16 -> 48
		accum = h1 * l2;
		addlow(accum << 16);
		high += accum >> 16;

		// 16 -> 48
		accum = l1 * h2;
		addlow(accum << 16);
		high += accum >> 16;

		// 32 -> 64
		high += h1 * h2;

		// 32 -> 64
		high += l1 * l3;

		// 48 -> 80
		high += (h1 * l3) << 16;

		// 48 -> 80
		high += (l1 * l3) << 16;

		return *this;
	}

	inline uint64 operator * (uint32 i) {
		uint64 n(*this);
		return n *= i;
	}

	inline uint64 &operator /= (uint32 div) {

		// If there isn't a high dword, we only need to work on the low dword
		if (!high) {
			low /= div;
			return *this;
		}

		// modulus of last division
		uint32 mod = high;
		uint32 l = low;

		// Low shift
		uint32 shift = 32;
		low = 0;

		// Only High DWORD
		// Can divide
		if (mod >= div) {
			high = mod / div;
			mod %= div;
		} else high = 0;

		// Only Both high and low
		while (--shift) {

			mod <<= 1;
			mod |= (l >> shift) & 1;

			// Can divide
			if (mod >= div) {
				uint32 v = mod / div;
				mod %= div;
				addlow(v << shift);
				high += v >> (32 - shift);
			}
		}


		// Only Low DWORD
		mod <<= 1;
		mod |= l & 1;

		// Can divide
		if (mod >= div) {
			uint32 v = mod / div;
			mod %= div;
			addlow(v << shift);
		}

		return *this;
	}

	inline uint64 operator / (uint32 i) {
		uint64 n(*this);
		return n /= i;
	}

	inline uint64 &operator %= (uint32 div) {

		// If there isn't a high dword, we only need to work on the low dword
		if (!high) {
			low %= div;
			return *this;
		}

		// Remainder of last division
		uint32 mod = high;

		// Low shift
		uint32 shift = 32;

		while (1) {

			// Can divide
			if (mod >= div) mod %= div;

			if (shift == 0) break;

			mod <<= 1;
			shift--;
			mod |= (low >> shift) & 1;
		}

		high = 0;
		low = mod;

		return *this;
	}

	inline uint64 operator % (uint32 i) {
		uint64 n(*this);
		return n %= i;
	}

	inline operator uint32() {
		return low;
	}

	void printx() {
		if (high) std::printf("%X%08X", high, low);
		else printf("%X", low);
	}
};
#endif

//
// AdjustTimings
//
// It converts the midi's to use 120 Hz timing, and also calcs the duration of
// the notes. It also strips the tempo events, and adds it's own
//
// This is used by Midi's ONLY! It will do nothing with XMidiFile
//
void XMidiFile::AdjustTimings(uint32 ppqn) {
	uint32      tempo = 500000;
	uint32      time_prev = 0;
	uint32      hs_rem = 0;
	uint32      hs     = 0;

	ppqn *= 10000;

	// Virtual playing
	XMidiNoteStack notes;

	for (XMidiEvent *event = list; event; event = event->next) {

		// Note 64 bit int is required because multiplication by tempo can
		// require 52 bits in some circumstances

		uint64 aim = event->time - time_prev;
		aim *= tempo;

		hs_rem += aim % ppqn;
		hs += aim / ppqn;
		hs += hs_rem / ppqn;
		hs_rem %= ppqn;

		time_prev = event->time;
		event->time = (hs * 6) / 5 + (6 * hs_rem) / (5 * ppqn);

		// Note on and note off handling
		if (event->status <= 0x9F) {

			// Add if it's a note on and remove if it's a note off
			if ((event->status >> 4) == MIDI_STATUS_NOTE_ON && event->data[1])
				notes.Push(event);
			else {
				XMidiEvent *prev = notes.FindAndPop(event);
				if (prev) prev->ex.note_on.duration = event->time - prev->time;
			}

		} else if (event->status == 0xFF && event->data[0] == 0x51) {

			tempo = (event->ex.sysex_data.buffer[0] << 16) +
			        (event->ex.sysex_data.buffer[1] << 8) +
			        event->ex.sysex_data.buffer[2];

			event->ex.sysex_data.buffer[0] = 0x07;
			event->ex.sysex_data.buffer[1] = 0xA1;
			event->ex.sysex_data.buffer[2] = 0x20;
		}
	}

	//std::cout << "Max Polyphony: " << notes.GetMaxPolyphony() << std::endl;
	static const unsigned char tempo_buf[5] = { 0x51, 0x03, 0x07, 0xA1, 0x20 };
	IBufferDataSource ds(tempo_buf, 5);
	current = list;
	ConvertSystemMessage(0, 0xFF, &ds);
}


// Converts Events
//
// Source is at the first data byte
// size 1 is single data byte (ConvertEvent Only)
// size 2 is dual data byte
// size 3 is XMI Note on (ConvertNote only)
// Returns bytes converted
//
// ConvertNote is used for Note On's and Note offs
// ConvertSystemMessage is used for SysEx events and Meta events
// ConvertEvent is used for everything else

int XMidiFile::ConvertEvent(const int time, const unsigned char status, IDataSource *source, const int size, first_state &fs) {
	int data;

	data = source->read1();


	// Bank changes are handled here
	if ((status >> 4) == 0xB && data == 0) {
		data = source->read1();

		bank127[status & 0xF] = false;

		if (convert_type == XMIDIFILE_CONVERT_MT32_TO_GM
		        || convert_type == XMIDIFILE_CONVERT_MT32_TO_GS
		        || convert_type == XMIDIFILE_CONVERT_MT32_TO_GS127)
			return 2;

		CreateNewEvent(time);
		current->status = status;
		current->data[0] = 0;
		current->data[1] = data;

		// Set the bank
		if (!fs.bank[status & 0xF] || fs.bank[status & 0xF]->time > time) fs.bank[status & 0xF] = current;

		if (convert_type == XMIDIFILE_CONVERT_GS127_TO_GS && data == 127)
			bank127[status & 0xF] = true;

		return 2;
	}

	// Handling for patch change mt32 conversion, probably should go elsewhere
	if ((status >> 4) == 0xC && (status & 0xF) != 9 && convert_type != XMIDIFILE_CONVERT_NOCONVERSION) {
		if (convert_type == XMIDIFILE_CONVERT_MT32_TO_GM) {
			data = mt32asgm[data];
		} else if (convert_type == XMIDIFILE_CONVERT_GM_TO_MT32) {
			data = gmasmt32[data];
		} else if ((convert_type == XMIDIFILE_CONVERT_GS127_TO_GS && bank127[status & 0xF]) ||
		           convert_type == XMIDIFILE_CONVERT_MT32_TO_GS) {
			CreateNewEvent(time);
			current->status = 0xB0 | (status & 0xF);
			current->data[0] = 0;
			current->data[1] = mt32asgs[data * 2 + 1];

			data = mt32asgs[data * 2];

			// Set the bank
			if (!fs.bank[status & 0xF] || fs.bank[status & 0xF]->time > time) fs.bank[status & 0xF] = current;
		} else if (convert_type == XMIDIFILE_CONVERT_MT32_TO_GS127) {
			CreateNewEvent(time);
			current->status = 0xB0 | (status & 0xF);
			current->data[0] = 0;
			current->data[1] = 127;

			// Set the bank
			if (!fs.bank[status & 0xF] || fs.bank[status & 0xF]->time > time) fs.bank[status & 0xF] = current;
		}
	}// Disable patch changes on Track 10 is doing a conversion
	else if ((status >> 4) == 0xC && (status & 0xF) == 9 && convert_type != XMIDIFILE_CONVERT_NOCONVERSION) {
		return size;
	}

	CreateNewEvent(time);
	current->status = status;

	current->data[0] = data;

	// Check for patch change, and update fs if req
	if ((status >> 4) == 0xC) {
		if (!fs.patch[status & 0xF] || fs.patch[status & 0xF]->time > time)
			fs.patch[status & 0xF] = current;
	}
	// Controllers
	else if ((status >> 4) == 0xB) {
		// Volume
		if (current->data[0] == 7) {
			if (!fs.vol[status & 0xF] || fs.vol[status & 0xF]->time > time)
				fs.vol[status & 0xF] = current;
		}
		// Pan
		else if (current->data[0] == 10) {
			if (!fs.pan[status & 0xF] || fs.pan[status & 0xF]->time > time)
				fs.pan[status & 0xF] = current;
		}
		// Sequence Branch Index
		else if (current->data[0] == XMIDI_CONTROLLER_SEQ_BRANCH_INDEX) {
			current->ex.branch_index.next_branch = branches;
			branches = current;
		}
	}

	if (size == 1)
		return 1;

	current->data[1] = source->read1();

	// Volume modify the volume controller, only if converting
//	if (convert_type && (current->status >> 4) == MIDI_STATUS_CONTROLLER && current->data[0] == 7)
//		current->data[1] = VolumeCurve[current->data[1]];

	return 2;
}

int XMidiFile::ConvertNote(const int time, const unsigned char status, IDataSource *source, const int size) {
	uint32  delta = 0;
	int data;

	data = source->read1();

	CreateNewEvent(time);
	current->status = status;

	current->data[0] = data;
	current->data[1] = source->read1();

	// Volume modify the note on's, only if converting
//	if (convert_type && (current->status >> 4) == MIDI_STATUS_NOTE_ON && current->data[1])
//		current->data[1] = VolumeCurve[current->data[1]];

	if (size == 2)
		return 2;

	// XMI Note On handling

	// Get the duration
	int i = GetVLQ(source, delta);

	// Set the duration
	current->ex.note_on.duration = delta;

	// This is an optimization
	XMidiEvent *prev = current;

	// Create a note off
	CreateNewEvent(time + delta);

	current->status = status;
	current->data[0] = data;
	current->data[1] = 0;

	// Optimization
	current = prev;

	return i + 2;
}

// Simple routine to convert system messages
int XMidiFile::ConvertSystemMessage(const int time, const unsigned char status, IDataSource *source) {
	int i = 0;

	CreateNewEvent(time);
	current->status = status;

	// Handling of Meta events
	if (status == 0xFF) {
		current->data[0] = source->read1();
		i++;
	}

	i += GetVLQ(source, current->ex.sysex_data.len);

	if (!current->ex.sysex_data.len) {
		current->ex.sysex_data.buffer = NULL;
		return i;
	}

	current->ex.sysex_data.buffer = XMidiEvent__Malloc<unsigned char>(current->ex.sysex_data.len);

	source->read(reinterpret_cast<char *>(current->ex.sysex_data.buffer), current->ex.sysex_data.len);

	return i + current->ex.sysex_data.len;
}

// If data is NULL, then it is assumed that sysex_buffer already contains the data
// address_base is 7-bit, while address_offset is 8 bit!
int XMidiFile::CreateMT32SystemMessage(const int time, uint32 address_base, uint16 address_offset, uint32 len, const void *data, IDataSource *source) {
	CreateNewEvent(time);
	// SysEx status
	current->status = 0xF0;

	// Allocate the buffer
	current->ex.sysex_data.len = sysex_data_start + len + 2;
	unsigned char *sysex_buffer = current->ex.sysex_data.buffer =
	                                  XMidiEvent__Malloc<unsigned char>(current->ex.sysex_data.len);

	// MT32 Sysex Header
	sysex_buffer[0] = 0x41;     // Roland SysEx ID
	sysex_buffer[1] = 0x10;     // Device ID (assume 0x10, Device 17)
	sysex_buffer[2] = 0x16;     // MT-32 Model ID
	sysex_buffer[3] = 0x12;     // DTI Command ID (set data)

	// 7-bit address
	uint32 actual_address = address_offset;
	actual_address += (address_base >> 2) & (0x7f << 14);
	actual_address += (address_base >> 1) & (0x7f << 7);
	actual_address += (address_base >> 0) & (0x7f << 0);
	sysex_buffer[4] = (actual_address >> 14) & 0x7F;
	sysex_buffer[5] = (actual_address >> 7) & 0x7F;
	sysex_buffer[6] = actual_address & 0x7F;

	// Only copy if required
	if (data) std::memcpy(sysex_buffer + sysex_data_start, data, len);
	else if (source) source->read(sysex_buffer + sysex_data_start, len);

	// Calc checksum
	char checksum = 0;
	for (uint32 j = 4; j < sysex_data_start + len; j++)
		checksum += sysex_buffer[j];

	checksum = checksum & 0x7f;
	if (checksum) checksum = 0x80 - checksum;

	// Set checksum
	sysex_buffer[sysex_data_start + len] = checksum;

	// Terminator
	sysex_buffer[sysex_data_start + len + 1] = 0xF7;

	return sysex_data_start + len + 2;
}

// XMidiFile and Midi to List. Returns bit mask of channels used
int XMidiFile::ConvertFiletoList(IDataSource *source, const bool is_xmi, first_state &fs) {
	int     time = 0;           // 120th of a second
	uint32  data;
	int     end = 0;
	uint32  status = 0;
	int     play_size = 2;
	uint32  file_size = source->getSize();
	int     retval = 0;

	if (is_xmi) play_size = 3;

	while (!end && source->getPos() < file_size) {
		if (!is_xmi) {
			GetVLQ(source, data);
			time += data;

			data = source->read1();

			if (data >= 0x80) {
				status = data;
			} else
				source->skip(-1);
		} else {
			GetVLQ2(source, data);
			time += data;

			status = source->read1();
		}

		switch (status >> 4) {
		case MIDI_STATUS_NOTE_ON:
			retval |= 1 << (status & 0xF);
			ConvertNote(time, status, source, play_size);
			break;

		case MIDI_STATUS_NOTE_OFF:
			ConvertNote(time, status, source, 2);
			break;

		// 2 byte data
		case MIDI_STATUS_AFTERTOUCH:
		case MIDI_STATUS_CONTROLLER:
		case MIDI_STATUS_PITCH_WHEEL:
			ConvertEvent(time, status, source, 2, fs);
			break;


		// 1 byte data
		case MIDI_STATUS_PROG_CHANGE:
		case MIDI_STATUS_PRESSURE:
			ConvertEvent(time, status, source, 1, fs);
			break;


		case MIDI_STATUS_SYSEX:
			if (status == 0xFF) {
				int pos = source->getPos();
				uint32  data = source->read1();

				if (data == 0x2F)                   // End, of track
					end = 1;
				else if (data == 0x51 && is_xmi) {  // XMidiFile doesn't use tempo
					GetVLQ(source, data);
					source->skip(data);
					break;
				}

				source->seek(pos);
			}
			ConvertSystemMessage(time, status, source);
			break;

		default:
			break;
		}

	}

	return retval;
}

// Assumes correct XMidiFile
int XMidiFile::ExtractTracksFromXmi(IDataSource *source) {
	int             num = 0;
	uint32          len = 0;
	char            buf[32];

	first_state fs;

	while (source->getPos() < source->getSize() && num != num_tracks) {
		// Read first 4 bytes of name
		source->read(buf, 4);
		len = source->read4high();

		// Skip the FORM entries
		if (!memcmp(buf, "FORM", 4)) {
			source->skip(4);
			source->read(buf, 4);
			len = source->read4high();
		}

		if (memcmp(buf, "EVNT", 4)) {
			source->skip((len + 1) & ~1);
			continue;
		}

		list = NULL;
		branches = NULL;
		memset(&fs, 0, sizeof(fs));

		int begin = source->getPos();

		// Convert it
		int chan_mask = ConvertFiletoList(source, true, fs);

		// Apply the first state
//		ApplyFirstState(fs, chan_mask);

		// Add tempo
		static const unsigned char tempo_buf[5] = { 0x51, 0x03, 0x07, 0xA1, 0x20 };
		IBufferDataSource ds(tempo_buf, 5);
		current = list;
		ConvertSystemMessage(0, 0xFF, &ds);

		// Set the list
		events[num]->events = list;
		events[num]->branches = branches;
		events[num]->chan_mask = chan_mask;

		// Increment Counter
		num++;

		// go to start of next track
		source->seek(begin + ((len + 1) & ~1));
	}

	// Return how many were converted
	return num;
}

int XMidiFile::ExtractTracksFromMid(IDataSource *source, const uint32 ppqn, const int num_tracks, const bool type1) {
	int         num = 0;
	uint32      len = 0;
	char        buf[32];
	int         chan_mask = 0;

	first_state fs;
	memset(&fs, 0, sizeof(fs));

	list = NULL;
	branches = NULL;

	while (source->getPos() < source->getSize() && num != num_tracks) {
		// Read first 4 bytes of name
		source->read(buf, 4);
		len = source->read4high();

		if (memcmp(buf, "MTrk", 4)) {
			source->skip(len);
			continue;
		}

		int begin = source->getPos();

		// Convert it
		chan_mask |= ConvertFiletoList(source, false, fs);

		if (!type1) {
			ApplyFirstState(fs, chan_mask);
			AdjustTimings(ppqn);
			events[num]->events = list;
			events[num]->branches = branches;
			branches = NULL;
			list = NULL;
			memset(&fs, 0, sizeof(fs));
			chan_mask = 0;
		}

		// Increment Counter
		num++;
		source->seek(begin + len);
	}

	if (type1) {
		ApplyFirstState(fs, chan_mask);
		AdjustTimings(ppqn);
		events[0]->events = list;
		events[0]->branches = branches;
		return num == num_tracks ? 1 : 0;
	}

	// Return how many were converted
	return num;
}

int XMidiFile::ExtractTracks(IDataSource *source) {
	uint32      i = 0;
	int     start;
	uint32      len;
	uint32      chunk_len;
	int         count;
	char        buf[32];

	int         format_hint = convert_type;

	if (convert_type >= XMIDIFILE_HINT_U7VOICE_MT_FILE)
		convert_type = XMIDIFILE_CONVERT_NOCONVERSION;

	/*
	string s;

	config->value("config/audio/midi/reverb/enabled",s,"no");
	if (s == "yes") do_reverb = true;
	config->set("config/audio/midi/reverb/enabled",s,true);

	config->value("config/audio/midi/reverb/level",s,"---");
	if (s == "---") config->value("config/audio/midi/reverb",s,"64");
	reverb_value = atoi(s.c_str());
	if (reverb_value > 127) reverb_value = 127;
	else if (reverb_value < 0) reverb_value = 0;
	config->set("config/audio/midi/reverb/level",reverb_value,true);

	config->value("config/audio/midi/chorus/enabled",s,"no");
	if (s == "yes") do_chorus = true;
	config->set("config/audio/midi/chorus/enabled",s,true);

	config->value("config/audio/midi/chorus/level",s,"---");
	if (s == "---") config->value("config/audio/midi/chorus",s,"16");
	chorus_value = atoi(s.c_str());
	if (chorus_value > 127) chorus_value = 127;
	else if (chorus_value < 0) chorus_value = 0;
	config->set("config/audio/midi/chorus/level",chorus_value,true);

	config->value("config/audio/midi/volume_curve",s,"---");
	if (s == "---") config->value("config/audio/midi/gamma",s,"1");
	VolumeCurve.set_gamma (atof(s.c_str()));
	int igam = (int) ((VolumeCurve.get_gamma()*10000)+0.5);
	snprintf (buf, 32, "%d.%04d", igam/10000, igam%10000);
	config->set("config/audio/midi/volume_curve",buf,true);
	*/
	do_reverb = false;
	do_chorus = false;
	reverb_value = 0;
	chorus_value = 0;

	// Read first 4 bytes of header
	source->read(buf, 4);

	// Could be XMidiFile
	if (!memcmp(buf, "FORM", 4)) {
		// Read length of
		len = source->read4high();

		start = source->getPos();

		// Read 4 bytes of type
		source->read(buf, 4);

		// XDIRless XMidiFile, we can handle them here.
		if (!memcmp(buf, "XMID", 4)) {
			perr << "Warning: XMidiFile doesn't have XDIR" << endl;
			num_tracks = 1;

		} // Not an XMidiFile that we recognise
		else if (memcmp(buf, "XDIR", 4)) {
			perr << "Not a recognised XMID" << endl;
			return 0;

		} // Seems Valid
		else {
			num_tracks = 0;

			for (i = 4; i < len; i++) {
				// Read 4 bytes of type
				source->read(buf, 4);

				// Read length of chunk
				chunk_len = source->read4high();

				// Add eight bytes
				i += 8;

				if (memcmp(buf, "INFO", 4)) {
					// Must allign
					source->skip((chunk_len + 1) & ~1);
					i += (chunk_len + 1) & ~1;
					continue;
				}

				// Must be at least 2 bytes long
				if (chunk_len < 2)
					break;

				num_tracks = source->read2();
				break;
			}

			// Didn't get to fill the header
			if (num_tracks == 0) {
				perr << "Not a valid XMID" << endl;
				return 0;
			}

			// Ok now to start part 2
			// Goto the right place
			source->seek(start + ((len + 1) & ~1));

			// Read 4 bytes of type
			source->read(buf, 4);

			// Not an XMID
			if (memcmp(buf, "CAT ", 4)) {
				perr << "Not a recognised XMID (" << buf[0] << buf[1] << buf[2] << buf[3] << ") should be (CAT )" << endl;
				return 0;
			}

			// Now read length of this track
			len = source->read4high();

			// Read 4 bytes of type
			source->read(buf, 4);

			// Not an XMID
			if (memcmp(buf, "XMID", 4)) {
				perr << "Not a recognised XMID (" << buf[0] << buf[1] << buf[2] << buf[3] << ") should be (XMID)" << endl;
				return 0;
			}

		}

		// Ok it's an XMID, so pass it to the ExtractCode

		events = XMidiEvent__Calloc<XMidiEventList *>(num_tracks); //new XMidiEvent *[info.tracks];

		for (i = 0; i < num_tracks; i++)
			events[i] = XMidiEvent__Calloc<XMidiEventList>();

		count = ExtractTracksFromXmi(source);

		if (count != num_tracks) {
			perr << "Error: unable to extract all (" << num_tracks << ") tracks specified from XMidiFile. Only (" << count << ")" << endl;

			int i = 0;

			for (i = 0; i < num_tracks; i++) {
				events[i]->decerementCounter();
				events[i] = NULL;
			}

			//delete [] events;
			XMidiEvent::Free(events);

			return 0;
		}

		return 1;

	}// Definately a Midi
	else if (!memcmp(buf, "MThd", 4)) {
		// Simple read length of header
		len = source->read4high();

		if (len < 6) {
			perr << "Not a valid MIDI" << endl;
			return 0;
		}

		int type = source->read2high();

		int actual_num = num_tracks = source->read2high();

		// Type 1 only has 1 track, even though it says it has more
		if (type == 1) num_tracks = 1;

		events = XMidiEvent__Calloc<XMidiEventList *>(num_tracks); //new XMidiEvent *[info.tracks];
		const uint32 ppqn = source->read2high();

		for (i = 0; i < num_tracks; i++)
			events[i] = XMidiEvent__Calloc<XMidiEventList>();

		count = ExtractTracksFromMid(source, ppqn, actual_num, type == 1);

		if (count != num_tracks) {
			perr << "Error: unable to extract all (" << num_tracks << ") tracks specified from MIDI. Only (" << count << ")" << endl;

			for (i = 0; i < num_tracks; i++) {
				events[i]->decerementCounter();
				events[i] = NULL;
			}

			XMidiEvent::Free(events);

			return 0;

		}

		return 1;

	}// A RIFF Midi, just pass the source back to this function at the start of the midi file
	else if (!memcmp(buf, "RIFF", 4)) {
		// Read len
		len = source->read4();

		// Read 4 bytes of type
		source->read(buf, 4);

		// Not an RMID
		if (memcmp(buf, "RMID", 4)) {
			perr << "Invalid RMID" << endl;
			return 0;
		}

		// Is a RMID

		for (i = 4; i < len; i++) {
			// Read 4 bytes of type
			source->read(buf, 4);

			chunk_len = source->read4();

			i += 8;

			if (memcmp(buf, "data", 4)) {
				// Must allign
				source->skip((chunk_len + 1) & ~1);
				i += (chunk_len + 1) & ~1;
				continue;
			}

			return ExtractTracks(source);

		}

		perr << "Failed to find midi data in RIFF Midi" << endl;
		return 0;
	} else if (format_hint == XMIDIFILE_HINT_U7VOICE_MT_FILE) {
		return ExtractTracksFromU7V(source);
	} else if (format_hint == XMIDIFILE_HINT_XMIDI_MT_FILE) {
		return ExtractTracksFromXMIDIMT(source);
	}

	return 0;
}

int XMidiFile::ExtractTracksFromU7V(IDataSource *source) {
	uint32          i, j;
	int             num = 0;
	int             time = 0;
	int             time_inc = 32;


	first_state fs;

	list = NULL;
	branches = NULL;
	memset(&fs, 0, sizeof(fs));

	// Convert it
	int chan_mask = 0;

	source->seek(0);
	uint32 num_timbres = source->read1();
	pout << num_timbres << " custom timbres..." << std::endl;

	if (source->getSize() != 247 * num_timbres + 1) {
		perr << "File size didn't match timbre count. Wont convert." << std::endl;
		return 0;
	}

	//
	// All Dev Reset
	//

	char one = 1;
	CreateMT32SystemMessage(time, all_dev_reset_base, 0, 1, &one);
	time += time_inc;


	// Now do each timbre and patch
	for (i = 0; i < num_timbres; i++) {
		//
		// Timbre
		//
		CreateMT32SystemMessage(time, timbre_base, timbre_mem_offset(i), timbre_mem_size, 0, source);

		//
		// Patch
		//

		// Default patch
		PatchMemData patch_data = patch_template;

		// Set the timbre num
		patch_data.timbre_num = i;

		CreateMT32SystemMessage(time,
		                        patch_base,
		                        patch_mem_offset(source->read1() - 1),
		                        patch_mem_size,
		                        &patch_data);

		//time += time_inc;
	}

	//
	// Rhythm Setup
	//

	i = 0;
	while (U7PercussionNotes[i]) {
		// Work out how many we can send at a time
		for (j = i + 1; U7PercussionNotes[j]; j++) {
			// If the next isn't actually the next, then we can't upload it
			if (U7PercussionNotes[j - 1] + 1 != U7PercussionNotes[j]) break;
		}

		int count = j - i;

		CreateMT32SystemMessage(time,
		                        rhythm_base,
		                        rhythm_mem_offset_note(U7PercussionNotes[i]),
		                        rhythm_mem_size * count,
		                        &U7PercussionData[i]);

		time += time_inc;
		i += count;
	}

	//
	// Set system volume
	//

	char seventy = 70;

	CreateMT32SystemMessage(time, system_base, system_mem_offset(masterVol), 1, &seventy);
	time += time_inc;

	// Apply the first state
	//ApplyFirstState(fs, chan_mask);

	// Add tempo
	static const unsigned char tempo_buf[5] = { 0x51, 0x03, 0x07, 0xA1, 0x20 };
	IBufferDataSource ds(tempo_buf, 5);
	current = list;
	ConvertSystemMessage(0, 0xFF, &ds);

	num_tracks = 1;
	events = XMidiEvent__Calloc<XMidiEventList *>(1); //new XMidiEvent *[info.tracks];
	events[0] = XMidiEvent__Calloc<XMidiEventList>();
	events[0]->events = list;
	events[0]->branches = branches;
	events[0]->chan_mask = chan_mask;

	// Increment Counter
	num++;

	// Return how many were converted
	return num;

}

int XMidiFile::ExtractTracksFromXMIDIMT(IDataSource *source) {
	int             num = 0;
	int             time = 0;
	int             time_inc = 32;


	first_state fs;

	list = NULL;
	branches = NULL;
	memset(&fs, 0, sizeof(fs));

	// Convert it
	int chan_mask = 0;

	source->seek(0);

	//
	// All Dev Reset
	//

	char one = 1;
	CreateMT32SystemMessage(time, all_dev_reset_base, 0, 1, &one);
	time += time_inc;

	// Channel assignment
	CreateMT32SystemMessage(time, system_base, system_mem_offset(chanAssign), 9, system_part_chans);
	time += time_inc;

	// Partial Rerserve
	CreateMT32SystemMessage(time, system_base, system_mem_offset(reserveSettings), 9, system_part_rsv);
	time += time_inc;

	// Reverb settings
	CreateMT32SystemMessage(time, system_base, 1, 3, system_init_reverb);
	time += time_inc;

	// Add tempo
	static const unsigned char tempo_buf[5] = { 0x51, 0x03, 0x07, 0xA1, 0x20 };
	IBufferDataSource ds(tempo_buf, 5);
	current = list;
	ConvertSystemMessage(0, 0xFF, &ds);

	num_tracks = 1;
	events = XMidiEvent__Calloc<XMidiEventList *>(1); //new XMidiEvent *[info.tracks];
	events[0] = XMidiEvent__Calloc<XMidiEventList>();
	events[0]->events = list;
	events[0]->branches = branches;
	events[0]->chan_mask = chan_mask;

	// Increment Counter
	num++;

	// Return how many were converted
	return num;
}

void XMidiFile::InsertDisplayEvents() {
	// Change the display
	current = list = events[0]->events;
	while (current->next) current = current->next;


	//
	// Display
	//

	const char *display = ::display;
	const char *display_beginning = ::display_beginning;
#ifdef PENTAGRAM_IN_EXULT
	if (Game::get_game_type() == SERPENT_ISLE) {
		display = display_serpent_isle;
		display_beginning = display_beginning_si;
	} else if (Game::get_game_type() == BLACK_GATE) {
		display = display_black_gate;
		display_beginning = display_beginning_bg;
	}
#endif

	CreateMT32SystemMessage(current->time, display_base, 0, display_mem_size, display);
	CreateMT32SystemMessage(-1, display_base, 0, display_mem_size, display_beginning);

	events[0]->events = list;

}

} // End of namespace Ultima8
