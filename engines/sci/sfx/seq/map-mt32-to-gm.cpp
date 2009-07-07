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
 * $URL$
 * $Id$
 *
 */

#include "common/scummsys.h"
#include "sci/resource.h"
#include "sci/engine/state.h"
#include "sci/sfx/seq/instrument-map.h"

namespace Sci {

#define DEBUG_MT32_TO_GM

static const char *GM_Instrument_Names[] = {
	/*000*/  "Acoustic Grand Piano",
	/*001*/  "Bright Acoustic Piano",
	/*002*/  "Electric Grand Piano",
	/*003*/  "Honky-tonk Piano",
	/*004*/  "Electric Piano 1",
	/*005*/  "Electric Piano 2",
	/*006*/  "Harpsichord",
	/*007*/  "Clavinet",
	/*008*/  "Celesta",
	/*009*/  "Glockenspiel",
	/*010*/  "Music Box",
	/*011*/  "Vibraphone",
	/*012*/  "Marimba",
	/*013*/  "Xylophone",
	/*014*/  "Tubular Bells",
	/*015*/  "Dulcimer",
	/*016*/  "Drawbar Organ",
	/*017*/  "Percussive Organ",
	/*018*/  "Rock Organ",
	/*019*/  "Church Organ",
	/*020*/  "Reed Organ",
	/*021*/  "Accordion",
	/*022*/  "Harmonica",
	/*023*/  "Tango Accordion",
	/*024*/  "Acoustic Guitar (nylon)",
	/*025*/  "Acoustic Guitar (steel)",
	/*026*/  "Electric Guitar (jazz)",
	/*027*/  "Electric Guitar (clean)",
	/*028*/  "Electric Guitar (muted)",
	/*029*/  "Overdriven Guitar",
	/*030*/  "Distortion Guitar",
	/*031*/  "Guitar Harmonics",
	/*032*/  "Acoustic Bass",
	/*033*/  "Electric Bass (finger)",
	/*034*/  "Electric Bass (pick)",
	/*035*/  "Fretless Bass",
	/*036*/  "Slap Bass 1",
	/*037*/  "Slap Bass 2",
	/*038*/  "Synth Bass 1",
	/*039*/  "Synth Bass 2",
	/*040*/  "Violin",
	/*041*/  "Viola",
	/*042*/  "Cello",
	/*043*/  "Contrabass",
	/*044*/  "Tremolo Strings",
	/*045*/  "Pizzicato Strings",
	/*046*/  "Orchestral Harp",
	/*047*/  "Timpani",
	/*048*/  "String Ensemble 1",
	/*049*/  "String Ensemble 2",
	/*050*/  "SynthStrings 1",
	/*051*/  "SynthStrings 2",
	/*052*/  "Choir Aahs",
	/*053*/  "Voice Oohs",
	/*054*/  "Synth Voice",
	/*055*/  "Orchestra Hit",
	/*056*/  "Trumpet",
	/*057*/  "Trombone",
	/*058*/  "Tuba",
	/*059*/  "Muted Trumpet",
	/*060*/  "French Horn",
	/*061*/  "Brass Section",
	/*062*/  "SynthBrass 1",
	/*063*/  "SynthBrass 2",
	/*064*/  "Soprano Sax",
	/*065*/  "Alto Sax",
	/*066*/  "Tenor Sax",
	/*067*/  "Baritone Sax",
	/*068*/  "Oboe",
	/*069*/  "English Horn",
	/*070*/  "Bassoon",
	/*071*/  "Clarinet",
	/*072*/  "Piccolo",
	/*073*/  "Flute",
	/*074*/  "Recorder",
	/*075*/  "Pan Flute",
	/*076*/  "Blown Bottle",
	/*077*/  "Shakuhachi",
	/*078*/  "Whistle",
	/*079*/  "Ocarina",
	/*080*/  "Lead 1 (square)",
	/*081*/  "Lead 2 (sawtooth)",
	/*082*/  "Lead 3 (calliope)",
	/*083*/  "Lead 4 (chiff)",
	/*084*/  "Lead 5 (charang)",
	/*085*/  "Lead 6 (voice)",
	/*086*/  "Lead 7 (fifths)",
	/*087*/  "Lead 8 (bass+lead)",
	/*088*/  "Pad 1 (new age)",
	/*089*/  "Pad 2 (warm)",
	/*090*/  "Pad 3 (polysynth)",
	/*091*/  "Pad 4 (choir)",
	/*092*/  "Pad 5 (bowed)",
	/*093*/  "Pad 6 (metallic)",
	/*094*/  "Pad 7 (halo)",
	/*095*/  "Pad 8 (sweep)",
	/*096*/  "FX 1 (rain)",
	/*097*/  "FX 2 (soundtrack)",
	/*098*/  "FX 3 (crystal)",
	/*099*/  "FX 4 (atmosphere)",
	/*100*/  "FX 5 (brightness)",
	/*101*/  "FX 6 (goblins)",
	/*102*/  "FX 7 (echoes)",
	/*103*/  "FX 8 (sci-fi)",
	/*104*/  "Sitar",
	/*105*/  "Banjo",
	/*106*/  "Shamisen",
	/*107*/  "Koto",
	/*108*/  "Kalimba",
	/*109*/  "Bag pipe",
	/*110*/  "Fiddle",
	/*111*/  "Shannai",
	/*112*/  "Tinkle Bell",
	/*113*/  "Agogo",
	/*114*/  "Steel Drums",
	/*115*/  "Woodblock",
	/*116*/  "Taiko Drum",
	/*117*/  "Melodic Tom",
	/*118*/  "Synth Drum",
	/*119*/  "Reverse Cymbal",
	/*120*/  "Guitar Fret Noise",
	/*121*/  "Breath Noise",
	/*122*/  "Seashore",
	/*123*/  "Bird Tweet",
	/*124*/  "Telephone Ring",
	/*125*/  "Helicopter",
	/*126*/  "Applause",
	/*127*/  "Gunshot"
};

/* The GM Percussion map is downwards compatible to the MT32 map, which is used in SCI */
static const char *GM_Percussion_Names[] = {
	/*00*/  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/*10*/  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/*20*/  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/*30*/  0, 0, 0, 0,
	/* The preceeding percussions are not covered by the GM standard */
	/*34*/  "Acoustic Bass Drum",
	/*35*/  "Bass Drum 1",
	/*36*/  "Side Stick",
	/*37*/  "Acoustic Snare",
	/*38*/  "Hand Clap",
	/*39*/  "Electric Snare",
	/*40*/  "Low Floor Tom",
	/*41*/  "Closed Hi-Hat",
	/*42*/  "High Floor Tom",
	/*43*/  "Pedal Hi-Hat",
	/*44*/  "Low Tom",
	/*45*/  "Open Hi-Hat",
	/*46*/  "Low-Mid Tom",
	/*47*/  "Hi-Mid Tom",
	/*48*/  "Crash Cymbal 1",
	/*49*/  "High Tom",
	/*50*/  "Ride Cymbal 1",
	/*51*/  "Chinese Cymbal",
	/*52*/  "Ride Bell",
	/*53*/  "Tambourine",
	/*54*/  "Splash Cymbal",
	/*55*/  "Cowbell",
	/*56*/  "Crash Cymbal 2",
	/*57*/  "Vibraslap",
	/*58*/  "Ride Cymbal 2",
	/*59*/  "Hi Bongo",
	/*60*/  "Low Bongo",
	/*61*/  "Mute Hi Conga",
	/*62*/  "Open Hi Conga",
	/*63*/  "Low Conga",
	/*64*/  "High Timbale",
	/*65*/  "Low Timbale",
	/*66*/  "High Agogo",
	/*67*/  "Low Agogo",
	/*68*/  "Cabasa",
	/*69*/  "Maracas",
	/*70*/  "Short Whistle",
	/*71*/  "Long Whistle",
	/*72*/  "Short Guiro",
	/*73*/  "Long Guiro",
	/*74*/  "Claves",
	/*75*/  "Hi Wood Block",
	/*76*/  "Low Wood Block",
	/*77*/  "Mute Cuica",
	/*78*/  "Open Cuica",
	/*79*/  "Mute Triangle",
	/*80*/  "Open Triangle"
};

/*******************************************
 * Fancy instrument mappings begin here... *
 *******************************************/


static struct {
	const char *name;
	int8 gm_instr;
	int8 gm_rhythm_key;
} MT32_PresetTimbreMaps[] = {
	/*000*/  {"AcouPiano1", 0, SFX_UNMAPPED},
	/*001*/  {"AcouPiano2", 1, SFX_UNMAPPED},
	/*002*/  {"AcouPiano3", 0, SFX_UNMAPPED},
	/*003*/  {"ElecPiano1", 4, SFX_UNMAPPED},
	/*004*/  {"ElecPiano2", 5, SFX_UNMAPPED},
	/*005*/  {"ElecPiano3", 4, SFX_UNMAPPED},
	/*006*/  {"ElecPiano4", 5, SFX_UNMAPPED},
	/*007*/  {"Honkytonk ", 3, SFX_UNMAPPED},
	/*008*/  {"Elec Org 1", 16, SFX_UNMAPPED},
	/*009*/  {"Elec Org 2", 17, SFX_UNMAPPED},
	/*010*/  {"Elec Org 3", 18, SFX_UNMAPPED},
	/*011*/  {"Elec Org 4", 18, SFX_UNMAPPED},
	/*012*/  {"Pipe Org 1", 19, SFX_UNMAPPED},
	/*013*/  {"Pipe Org 2", 19, SFX_UNMAPPED},
	/*014*/  {"Pipe Org 3", 20, SFX_UNMAPPED},
	/*015*/  {"Accordion ", 21, SFX_UNMAPPED},
	/*016*/  {"Harpsi 1  ", 6, SFX_UNMAPPED},
	/*017*/  {"Harpsi 2  ", 6, SFX_UNMAPPED},
	/*018*/  {"Harpsi 3  ", 6, SFX_UNMAPPED},
	/*019*/  {"Clavi 1   ", 7, SFX_UNMAPPED},
	/*020*/  {"Clavi 2   ", 7, SFX_UNMAPPED},
	/*021*/  {"Clavi 3   ", 7, SFX_UNMAPPED},
	/*022*/  {"Celesta 1 ", 8, SFX_UNMAPPED},
	/*023*/  {"Celesta 2 ", 8, SFX_UNMAPPED},
	/*024*/  {"Syn Brass1", 62, SFX_UNMAPPED},
	/*025*/  {"Syn Brass2", 63, SFX_UNMAPPED},
	/*026*/  {"Syn Brass3", 62, SFX_UNMAPPED},
	/*027*/  {"Syn Brass4", 63, SFX_UNMAPPED},
	/*028*/  {"Syn Bass 1", 38, SFX_UNMAPPED},
	/*029*/  {"Syn Bass 2", 39, SFX_UNMAPPED},
	/*030*/  {"Syn Bass 3", 38, SFX_UNMAPPED},
	/*031*/  {"Syn Bass 4", 39, SFX_UNMAPPED},
	/*032*/  {"Fantasy   ", 88, SFX_UNMAPPED},
	/*033*/  {"Harmo Pan ", 89, SFX_UNMAPPED},
	/*034*/  {"Chorale   ", 52, SFX_UNMAPPED},
	/*035*/  {"Glasses   ", 98, SFX_UNMAPPED},
	/*036*/  {"Soundtrack", 97, SFX_UNMAPPED},
	/*037*/  {"Atmosphere", 99, SFX_UNMAPPED},
	/*038*/  {"Warm Bell ", 89, SFX_UNMAPPED},
	/*039*/  {"Funny Vox ", 85, SFX_UNMAPPED},
	/*040*/  {"Echo Bell ", 39, SFX_UNMAPPED},
	/*041*/  {"Ice Rain  ", 101, SFX_UNMAPPED},
	/*042*/  {"Oboe 2001 ", 68, SFX_UNMAPPED},
	/*043*/  {"Echo Pan  ", 87, SFX_UNMAPPED},
	/*044*/  {"DoctorSolo", 86, SFX_UNMAPPED},
	/*045*/  {"Schooldaze", 103, SFX_UNMAPPED},
	/*046*/  {"BellSinger", 88, SFX_UNMAPPED},
	/*047*/  {"SquareWave", 80, SFX_UNMAPPED},
	/*048*/  {"Str Sect 1", 48, SFX_UNMAPPED},
	/*049*/  {"Str Sect 2", 48, SFX_UNMAPPED},
	/*050*/  {"Str Sect 3", 49, SFX_UNMAPPED},
	/*051*/  {"Pizzicato ", 45, SFX_UNMAPPED},
	/*052*/  {"Violin 1  ", 40, SFX_UNMAPPED},
	/*053*/  {"Violin 2  ", 40, SFX_UNMAPPED},
	/*054*/  {"Cello 1   ", 42, SFX_UNMAPPED},
	/*055*/  {"Cello 2   ", 42, SFX_UNMAPPED},
	/*056*/  {"Contrabass", 43, SFX_UNMAPPED},
	/*057*/  {"Harp 1    ", 46, SFX_UNMAPPED},
	/*058*/  {"Harp 2    ", 46, SFX_UNMAPPED},
	/*059*/  {"Guitar 1  ", 24, SFX_UNMAPPED},
	/*060*/  {"Guitar 2  ", 25, SFX_UNMAPPED},
	/*061*/  {"Elec Gtr 1", 26, SFX_UNMAPPED},
	/*062*/  {"Elec Gtr 2", 27, SFX_UNMAPPED},
	/*063*/  {"Sitar     ", 104, SFX_UNMAPPED},
	/*064*/  {"Acou Bass1", 32, SFX_UNMAPPED},
	/*065*/  {"Acou Bass2", 33, SFX_UNMAPPED},
	/*066*/  {"Elec Bass1", 34, SFX_UNMAPPED},
	/*067*/  {"Elec Bass2", 39, SFX_UNMAPPED},
	/*068*/  {"Slap Bass1", 36, SFX_UNMAPPED},
	/*069*/  {"Slap Bass2", 37, SFX_UNMAPPED},
	/*070*/  {"Fretless 1", 35, SFX_UNMAPPED},
	/*071*/  {"Fretless 2", 35, SFX_UNMAPPED},
	/*072*/  {"Flute 1   ", 73, SFX_UNMAPPED},
	/*073*/  {"Flute 2   ", 73, SFX_UNMAPPED},
	/*074*/  {"Piccolo 1 ", 72, SFX_UNMAPPED},
	/*075*/  {"Piccolo 2 ", 72, SFX_UNMAPPED},
	/*076*/  {"Recorder  ", 74, SFX_UNMAPPED},
	/*077*/  {"Panpipes  ", 75, SFX_UNMAPPED},
	/*078*/  {"Sax 1     ", 64, SFX_UNMAPPED},
	/*079*/  {"Sax 2     ", 65, SFX_UNMAPPED},
	/*080*/  {"Sax 3     ", 66, SFX_UNMAPPED},
	/*081*/  {"Sax 4     ", 67, SFX_UNMAPPED},
	/*082*/  {"Clarinet 1", 71, SFX_UNMAPPED},
	/*083*/  {"Clarinet 2", 71, SFX_UNMAPPED},
	/*084*/  {"Oboe      ", 68, SFX_UNMAPPED},
	/*085*/  {"Engl Horn ", 69, SFX_UNMAPPED},
	/*086*/  {"Bassoon   ", 70, SFX_UNMAPPED},
	/*087*/  {"Harmonica ", 22, SFX_UNMAPPED},
	/*088*/  {"Trumpet 1 ", 56, SFX_UNMAPPED},
	/*089*/  {"Trumpet 2 ", 56, SFX_UNMAPPED},
	/*090*/  {"Trombone 1", 57, SFX_UNMAPPED},
	/*091*/  {"Trombone 2", 57, SFX_UNMAPPED},
	/*092*/  {"Fr Horn 1 ", 60, SFX_UNMAPPED},
	/*093*/  {"Fr Horn 2 ", 60, SFX_UNMAPPED},
	/*094*/  {"Tuba      ", 58, SFX_UNMAPPED},
	/*095*/  {"Brs Sect 1", 61, SFX_UNMAPPED},
	/*096*/  {"Brs Sect 2", 61, SFX_UNMAPPED},
	/*097*/  {"Vibe 1    ", 11, SFX_UNMAPPED},
	/*098*/  {"Vibe 2    ", 11, SFX_UNMAPPED},
	/*099*/  {"Syn Mallet", 15, SFX_UNMAPPED},
	/*100*/  {"Wind Bell ", 88, SFX_UNMAPPED},
	/*101*/  {"Glock     ", 9, SFX_UNMAPPED},
	/*102*/  {"Tube Bell ", 14, SFX_UNMAPPED},
	/*103*/  {"Xylophone ", 13, SFX_UNMAPPED},
	/*104*/  {"Marimba   ", 12, SFX_UNMAPPED},
	/*105*/  {"Koto      ", 107, SFX_UNMAPPED},
	/*106*/  {"Sho       ", 111, SFX_UNMAPPED},
	/*107*/  {"Shakuhachi", 77, SFX_UNMAPPED},
	/*108*/  {"Whistle 1 ", 78, SFX_UNMAPPED},
	/*109*/  {"Whistle 2 ", 78, SFX_UNMAPPED},
	/*110*/  {"BottleBlow", 76, SFX_UNMAPPED},
	/*111*/  {"BreathPipe", 121, SFX_UNMAPPED},
	/*112*/  {"Timpani   ", 47, SFX_UNMAPPED},
	/*113*/  {"MelodicTom", 117, SFX_UNMAPPED},
	/*114*/  {"Deep Snare", SFX_MAPPED_TO_RHYTHM, 37},
	/*115*/  {"Elec Perc1", 115, SFX_UNMAPPED}, /* ? */
	/*116*/  {"Elec Perc2", 118, SFX_UNMAPPED}, /* ? */
	/*117*/  {"Taiko     ", 116, SFX_UNMAPPED},
	/*118*/  {"Taiko Rim ", 118, SFX_UNMAPPED},
	/*119*/  {"Cymbal    ", SFX_MAPPED_TO_RHYTHM, 50},
	/*120*/  {"Castanets ", SFX_MAPPED_TO_RHYTHM, SFX_UNMAPPED},
	/*121*/  {"Triangle  ", 112, SFX_UNMAPPED},
	/*122*/  {"Orche Hit ", 55, SFX_UNMAPPED},
	/*123*/  {"Telephone ", 124, SFX_UNMAPPED},
	/*124*/  {"Bird Tweet", 123, SFX_UNMAPPED},
	/*125*/  {"OneNoteJam", SFX_UNMAPPED, SFX_UNMAPPED}, /* ? */
	/*126*/  {"WaterBells", 98, SFX_UNMAPPED},
	/*127*/  {"JungleTune", SFX_UNMAPPED, SFX_UNMAPPED} /* ? */
};

static struct {
	const char *name;
	int8 gm_instr;
	int8 gm_rhythmkey;
} MT32_RhythmTimbreMaps[] = {
	/*00*/  {"Acou BD   ", SFX_MAPPED_TO_RHYTHM, 34},
	/*01*/  {"Acou SD   ", SFX_MAPPED_TO_RHYTHM, 37},
	/*02*/  {"Acou HiTom", 117, 49},
	/*03*/  {"AcouMidTom", 117, 46},
	/*04*/  {"AcouLowTom", 117, 40},
	/*05*/  {"Elec SD   ", SFX_MAPPED_TO_RHYTHM, 39},
	/*06*/  {"Clsd HiHat", SFX_MAPPED_TO_RHYTHM, 41},
	/*07*/  {"OpenHiHat1", SFX_MAPPED_TO_RHYTHM, 45},
	/*08*/  {"Crash Cym ", SFX_MAPPED_TO_RHYTHM, 48},
	/*09*/  {"Ride Cym  ", SFX_MAPPED_TO_RHYTHM, 50},
	/*10*/  {"Rim Shot  ", SFX_MAPPED_TO_RHYTHM, 36},
	/*11*/  {"Hand Clap ", SFX_MAPPED_TO_RHYTHM, 38},
	/*12*/  {"Cowbell   ", SFX_MAPPED_TO_RHYTHM, 55},
	/*13*/  {"Mt HiConga", SFX_MAPPED_TO_RHYTHM, 61},
	/*14*/  {"High Conga", SFX_MAPPED_TO_RHYTHM, 62},
	/*15*/  {"Low Conga ", SFX_MAPPED_TO_RHYTHM, 63},
	/*16*/  {"Hi Timbale", SFX_MAPPED_TO_RHYTHM, 64},
	/*17*/  {"LowTimbale", SFX_MAPPED_TO_RHYTHM, 65},
	/*18*/  {"High Bongo", SFX_MAPPED_TO_RHYTHM, 59},
	/*19*/  {"Low Bongo ", SFX_MAPPED_TO_RHYTHM, 60},
	/*20*/  {"High Agogo", 113, 66},
	/*21*/  {"Low Agogo ", 113, 67},
	/*22*/  {"Tambourine", SFX_MAPPED_TO_RHYTHM, 53},
	/*23*/  {"Claves    ", SFX_MAPPED_TO_RHYTHM, 74},
	/*24*/  {"Maracas   ", SFX_MAPPED_TO_RHYTHM, 69},
	/*25*/  {"SmbaWhis L", 78, 71},
	/*26*/  {"SmbaWhis S", 78, 70},
	/*27*/  {"Cabasa    ", SFX_MAPPED_TO_RHYTHM, 68},
	/*28*/  {"Quijada   ", SFX_MAPPED_TO_RHYTHM, 72},
	/*29*/  {"OpenHiHat2", SFX_MAPPED_TO_RHYTHM, 43}
};

static int8 MT32_PresetRhythmKeymap[] = {
	SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED,
	SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED,
	SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED,
	SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, 34, 34, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
	50, SFX_UNMAPPED, SFX_UNMAPPED, 53, SFX_UNMAPPED, 55, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, 59,
	60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
	70, 71, 72, SFX_UNMAPPED, 74, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED,
	SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED,
	SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED,
	SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED,
	SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED,
	SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED, SFX_UNMAPPED
};

/* +++ - Don't change unless you've got a good reason
   ++  - Looks good, sounds ok
   +   - Not too bad, but is it right?
   ?   - Where do I map this one?
   ??  - Any good ideas?
   ??? - I'm clueless?
   R   - Rhythm... */
static struct {
	const char *name;
	int8 gm_instr;
	int8 gm_rhythm_key;
} MT32_MemoryTimbreMaps[] = {
	{"AccPnoKA2 ", 1, SFX_UNMAPPED},     /* ++ (KQ1) */
	{"Acou BD   ", SFX_MAPPED_TO_RHYTHM, 34},   /* R (PQ2) */
	{"Acou SD   ", SFX_MAPPED_TO_RHYTHM, 37},   /* R (PQ2) */
	{"AcouPnoKA ", 0, SFX_UNMAPPED},     /* ++ (KQ1) */
	{"BASS      ", 32, SFX_UNMAPPED},    /* + (LSL3) */
	{"BASSOONPCM", 70, SFX_UNMAPPED},    /* + (CB) */
	{"BEACH WAVE", 122, SFX_UNMAPPED},   /* + (LSL3) */
	{"BagPipes  ", 109, SFX_UNMAPPED},
	{"BassPizzMS", 45, SFX_UNMAPPED},    /* ++ (HQ) */
	{"BassoonKA ", 70, SFX_UNMAPPED},    /* ++ (KQ1) */
	{"Bell    MS", 112, SFX_UNMAPPED},   /* ++ (iceMan) */
	{"Bells   MS", 112, SFX_UNMAPPED},   /* + (HQ) */
	{"Big Bell  ", 14, SFX_UNMAPPED},    /* + (CB) */
	{"Bird Tweet", 123, SFX_UNMAPPED},
	{"BrsSect MS", 61, SFX_UNMAPPED},    /* +++ (iceMan) */
	{"CLAPPING  ", 126, SFX_UNMAPPED},   /* ++ (LSL3) */
	{"Cabasa    ", SFX_MAPPED_TO_RHYTHM, 68},   /* R (HBoG) */
	{"Calliope  ", 82, SFX_UNMAPPED},    /* +++ (HQ) */
	{"CelticHarp", 46, SFX_UNMAPPED},    /* ++ (CoC) */
	{"Chicago MS", 1, SFX_UNMAPPED},     /* ++ (iceMan) */
	{"Chop      ", 117, SFX_UNMAPPED},
	{"Chorale MS", 52, SFX_UNMAPPED},    /* + (CoC) */
	{"ClarinetMS", 71, SFX_UNMAPPED},
	{"Claves    ", SFX_MAPPED_TO_RHYTHM, 74},   /* R (PQ2) */
	{"Claw    MS", 118, SFX_UNMAPPED},    /* + (HQ) */
	{"ClockBell ", 14, SFX_UNMAPPED},    /* + (CB) */
	{"ConcertCym", SFX_MAPPED_TO_RHYTHM, 54},   /* R ? (KQ1) */
	{"Conga   MS", SFX_MAPPED_TO_RHYTHM, 63},   /* R (HQ) */
	{"CoolPhone ", 124, SFX_UNMAPPED},   /* ++ (LSL3) */
	{"CracklesMS", 115, SFX_UNMAPPED}, /* ? (CoC, HQ) */
	{"CreakyD MS", SFX_UNMAPPED, SFX_UNMAPPED}, /* ??? (KQ1) */
	{"Cricket   ", 120, SFX_UNMAPPED}, /* ? (CB) */
	{"CrshCymbMS", SFX_MAPPED_TO_RHYTHM, 56},   /* R +++ (iceMan) */
	{"CstlGateMS", SFX_UNMAPPED, SFX_UNMAPPED}, /* ? (HQ) */
	{"CymSwellMS", SFX_MAPPED_TO_RHYTHM, 54},   /* R ? (CoC, HQ) */
	{"CymbRollKA", SFX_MAPPED_TO_RHYTHM, 56},   /* R ? (KQ1) */
	{"Cymbal Lo ", SFX_UNMAPPED, SFX_UNMAPPED}, /* R ? (LSL3) */
	{"card      ", SFX_UNMAPPED, SFX_UNMAPPED}, /* ? (HBoG) */
	{"DirtGtr MS", 30, SFX_UNMAPPED},    /* + (iceMan) */
	{"DirtGtr2MS", 29, SFX_UNMAPPED},    /* + (iceMan) */
	{"E Bass  MS", 33, SFX_UNMAPPED},    /* + (SQ3) */
	{"ElecBassMS", 33, SFX_UNMAPPED},
	{"ElecGtr MS", 27, SFX_UNMAPPED},    /* ++ (iceMan) */
	{"EnglHornMS", 69, SFX_UNMAPPED},
	{"FantasiaKA", 88, SFX_UNMAPPED},
	{"Fantasy   ", 99, SFX_UNMAPPED},    /* + (PQ2) */
	{"Fantasy2MS", 99, SFX_UNMAPPED},    /* ++ (CoC, HQ) */
	{"Filter  MS", 95, SFX_UNMAPPED},    /* +++ (iceMan) */
	{"Filter2 MS", 95, SFX_UNMAPPED},    /* ++ (iceMan) */
	{"Flame2  MS", 121, SFX_UNMAPPED},   /* ? (HQ) */
	{"Flames  MS", 121, SFX_UNMAPPED},   /* ? (HQ) */
	{"Flute   MS", 73, SFX_UNMAPPED},    /* +++ (HQ) */
	{"FogHorn MS", 58, SFX_UNMAPPED},
	{"FrHorn1 MS", 60, SFX_UNMAPPED},    /* +++ (HQ) */
	{"FunnyTrmp ", 56, SFX_UNMAPPED},    /* ++ (CB) */
	{"GameSnd MS", 80, SFX_UNMAPPED},
	{"Glock   MS", 9, SFX_UNMAPPED},     /* +++ (HQ) */
	{"Gunshot   ", 127, SFX_UNMAPPED},   /* +++ (CB) */
	{"Hammer  MS", SFX_UNMAPPED, SFX_UNMAPPED}, /* ? (HQ) */
	{"Harmonica2", 22, SFX_UNMAPPED},    /* +++ (CB) */
	{"Harpsi 1  ", 6, SFX_UNMAPPED},     /* + (HBoG) */
	{"Harpsi 2  ", 6, SFX_UNMAPPED},     /* +++ (CB) */
	{"Heart   MS", 116, SFX_UNMAPPED},   /* ? (iceMan) */
	{"Horse1  MS", 115, SFX_UNMAPPED},   /* ? (CoC, HQ) */
	{"Horse2  MS", 115, SFX_UNMAPPED},   /* ? (CoC, HQ) */
	{"InHale  MS", 121, SFX_UNMAPPED},   /* ++ (iceMan) */
	{"KNIFE     ", 120, SFX_UNMAPPED},   /* ? (LSL3) */
	{"KenBanjo  ", 105, SFX_UNMAPPED},   /* +++ (CB) */
	{"Kiss    MS", 25, SFX_UNMAPPED},    /* ++ (HQ) */
	{"KongHit   ", SFX_UNMAPPED, SFX_UNMAPPED}, /* ??? (KQ1) */
	{"Koto      ", 107, SFX_UNMAPPED},   /* +++ (PQ2) */
	{"Laser   MS", 81, SFX_UNMAPPED},    /* ?? (HQ) */
	{"Meeps   MS", 62, SFX_UNMAPPED},    /* ? (HQ) */
	{"MTrak   MS", 62, SFX_UNMAPPED},    /* ?? (iceMan) */
	{"MachGun MS", 127, SFX_UNMAPPED},   /* ? (iceMan) */
	{"OCEANSOUND", 122, SFX_UNMAPPED},   /* + (LSL3) */
	{"Oboe 2001 ", 68, SFX_UNMAPPED},    /* + (PQ2) */
	{"Ocean   MS", 122, SFX_UNMAPPED},   /* + (iceMan) */
	{"PPG 2.3 MS", 75, SFX_UNMAPPED},    /* ? (iceMan) */
	{"PianoCrank", SFX_UNMAPPED, SFX_UNMAPPED}, /* ? (CB) */
	{"PicSnareMS", SFX_MAPPED_TO_RHYTHM, 39},   /* R ? (iceMan) */
	{"PiccoloKA ", 72, SFX_UNMAPPED},    /* +++ (KQ1) */
	{"PinkBassMS", 39, SFX_UNMAPPED},
	{"Pizz2     ", 45, SFX_UNMAPPED},    /* ++ (CB) */
	{"Portcullis", SFX_UNMAPPED, SFX_UNMAPPED}, /* ? (KQ1) */
	{"Raspbry MS", 81, SFX_UNMAPPED},    /* ? (HQ) */
	{"RatSqueek ", 72, SFX_UNMAPPED},    /* ? (CB, CoC) */
	{"Record78  ", SFX_UNMAPPED, SFX_UNMAPPED}, /* +++ (CB) */
	{"RecorderMS", 74, SFX_UNMAPPED},    /* +++ (CoC) */
	{"Red Baron ", 125, SFX_UNMAPPED},   /* ? (CB) */
	{"ReedPipMS ", 20, SFX_UNMAPPED},    /* +++ (Coc) */
	{"RevCymb MS", 119, SFX_UNMAPPED},
	{"RifleShot ", 127, SFX_UNMAPPED},   /* + (CB) */
	{"RimShot MS", SFX_MAPPED_TO_RHYTHM, 36},   /* R */
	{"SHOWER    ", 52, SFX_UNMAPPED},    /* ? (LSL3) */
	{"SQ Bass MS", 32, SFX_UNMAPPED},    /* + (SQ3) */
	{"ShakuVibMS", 79, SFX_UNMAPPED},    /* + (iceMan) */
	{"SlapBassMS", 36, SFX_UNMAPPED},    /* +++ (iceMan) */
	{"Snare   MS", SFX_MAPPED_TO_RHYTHM, 37},   /* R (HQ) */
	{"Some Birds", 123, SFX_UNMAPPED},   /* + (CB) */
	{"Sonar   MS", 78, SFX_UNMAPPED},    /* ? (iceMan) */
	{"Soundtrk2 ", 97, SFX_UNMAPPED},    /* +++ (CB) */
	{"Soundtrack", 97, SFX_UNMAPPED},    /* ++ (CoC) */
	{"SqurWaveMS", 80, SFX_UNMAPPED},
	{"StabBassMS", 34, SFX_UNMAPPED},    /* + (iceMan) */
	{"SteelDrmMS", 114, SFX_UNMAPPED},   /* +++ (iceMan) */
	{"StrSect1MS", 48, SFX_UNMAPPED},    /* ++ (HQ) */
	{"String  MS", 45, SFX_UNMAPPED},    /* + (CoC) */
	{"Syn-Choir ", 91, SFX_UNMAPPED},
	{"Syn Brass4", 63, SFX_UNMAPPED},    /* ++ (PQ2) */
	{"SynBass MS", 38, SFX_UNMAPPED},
	{"SwmpBackgr", 120, SFX_UNMAPPED},    /* ?? (CB,HQ) */
	{"T-Bone2 MS", 57, SFX_UNMAPPED},    /* +++ (HQ) */
	{"Taiko     ", 116, 34},      /* +++ (Coc) */
	{"Taiko Rim ", 118, 36},      /* +++ (LSL3) */
	{"Timpani1  ", 47, SFX_UNMAPPED},    /* +++ (CB) */
	{"Tom     MS", 117, 47},      /* +++ (iceMan) */
	{"Toms    MS", 117, 47},      /* +++ (CoC, HQ) */
	{"Tpt1prtl  ", 56, SFX_UNMAPPED},    /* +++ (KQ1) */
	{"TriangleMS", 112, 80},      /* R (CoC) */
	{"Trumpet 1 ", 56, SFX_UNMAPPED},    /* +++ (CoC) */
	{"Type    MS", 114, SFX_UNMAPPED},   /* ? (iceMan) */
	{"WaterBells", 98, SFX_UNMAPPED},    /* + (PQ2) */
	{"WaterFallK", SFX_UNMAPPED, SFX_UNMAPPED}, /* ? (KQ1) */
	{"Whiporill ", 123, SFX_UNMAPPED},   /* + (CB) */
	{"Wind      ", SFX_UNMAPPED, SFX_UNMAPPED}, /* ? (CB) */
	{"Wind    MS", SFX_UNMAPPED, SFX_UNMAPPED}, /* ? (HQ, iceMan) */
	{"Wind2   MS", SFX_UNMAPPED, SFX_UNMAPPED}, /* ? (CoC) */
	{"Woodpecker", 115, SFX_UNMAPPED},   /* ? (CB) */
	{"WtrFall MS", SFX_UNMAPPED, SFX_UNMAPPED}, /* ? (CoC, HQ, iceMan) */
	{0, 0, 0}
};

static int8 lookup_instrument(const char *iname) {
	int i = 0;

	while (MT32_MemoryTimbreMaps[i].name) {
		if (scumm_strnicmp(iname, MT32_MemoryTimbreMaps[i].name, 10) == 0)
			return MT32_MemoryTimbreMaps[i].gm_instr;
		i++;
	}
	return SFX_UNMAPPED;
}

static int8 lookup_rhythm_key(const char *iname) {
	int i = 0;

	while (MT32_MemoryTimbreMaps[i].name) {
		if (scumm_strnicmp(iname, MT32_MemoryTimbreMaps[i].name, 10) == 0)
			return MT32_MemoryTimbreMaps[i].gm_rhythm_key;
		i++;
	}
	return SFX_UNMAPPED;
}

static void print_map(int sci, int ins, int rhythm, int mt32) {
#ifdef DEBUG_MT32_TO_GM
	if (ins == SFX_UNMAPPED || (ins == SFX_MAPPED_TO_RHYTHM && rhythm == SFX_UNMAPPED)) {
		printf("[MT32-to-GM] No mapping available for [%i] `%s' (%i)\n",
		          sci, MT32_PresetTimbreMaps[mt32].name, mt32);
		return;
	}

	if (ins == SFX_MAPPED_TO_RHYTHM) {
		printf("[MT32-to-GM] Mapping [%i] `%s' (%i) to `%s' [R] (%i)\n",
		          sci, MT32_PresetTimbreMaps[mt32].name, mt32,
		          GM_Percussion_Names[rhythm], rhythm);
		return;
	}

	printf("[MT32-to-GM] Mapping [%i] `%s' (%i) to `%s' (%i)\n",
	          sci, MT32_PresetTimbreMaps[mt32].name, mt32,
	          GM_Instrument_Names[ins], ins);
#endif
}

static void print_map_mem(int sci, int ins, int rhythm, char *mt32) {
#ifdef DEBUG_MT32_TO_GM
	char name[11];

	strncpy(name, mt32, 10);
	name[10] = 0;

	if (ins == SFX_UNMAPPED || (ins == SFX_MAPPED_TO_RHYTHM && rhythm == SFX_UNMAPPED)) {
		printf("[MT32-to-GM] No mapping available for [%i] `%s'\n",
		          sci, name);
		return;
	}

	if (ins == SFX_MAPPED_TO_RHYTHM) {
		printf("[MT32-to-GM] Mapping [%i] `%s' to `%s' [R] (%i)\n",
		          sci, name, GM_Percussion_Names[rhythm], rhythm);
		return;
	}

	printf("[MT32-to-GM] Mapping [%i] `%s' to `%s' (%i)\n",
	          sci, name, GM_Instrument_Names[ins], ins);
#endif
}

static void print_map_rhythm(int sci, int ins, int rhythm, int mt32) {
#ifdef DEBUG_MT32_TO_GM
	if (ins == SFX_UNMAPPED || (ins == SFX_MAPPED_TO_RHYTHM && rhythm == SFX_UNMAPPED)) {
		printf("[MT32-to-GM] No mapping available for [%i] `%s' [R] (%i)\n",
		          sci, MT32_RhythmTimbreMaps[mt32].name, mt32);
		return;
	}

	if (ins == SFX_MAPPED_TO_RHYTHM) {
		printf("[MT32-to-GM] Mapping [%i] `%s' [R] (%i) to `%s' [R] (%i)\n",
		          sci, MT32_RhythmTimbreMaps[mt32].name, mt32,
		          GM_Percussion_Names[rhythm], rhythm);
		return;
	}

	printf("[MT32-to-GM] Mapping [%i] `%s' [R] (%i) to `%s' (%i)\n",
	          sci, MT32_RhythmTimbreMaps[mt32].name, mt32,
	          GM_Instrument_Names[ins], ins);
#endif
}

static void print_map_rhythm_mem(int sci, int rhythm, char *mt32) {
#ifdef DEBUG_MT32_TO_GM
	char name[11];

	strncpy(name, mt32, 10);
	name[10] = 0;

	if (rhythm == SFX_UNMAPPED) {
		printf("[MT32-to-GM] No mapping available for [%i] `%s'\n",
		          sci, name);
		return;
	}

	printf("[MT32-to-GM] Mapping [%i] `%s' to `%s' (%i)\n",
	          sci, name, GM_Percussion_Names[rhythm], rhythm);
#endif
}

sfx_instrument_map_t *sfx_instrument_map_mt32_to_gm(byte *data, size_t size) {
	int memtimbres, patches;
	uint8 group, number, keyshift, finetune, bender_range;
	uint8 *patchpointer;
	uint32 pos;
	sfx_instrument_map_t * map;
	int i;
	int type;

	map = sfx_instrument_map_new(0);

	for (i = 0; i < SFX_INSTRUMENTS_NR; i++) {
		map->patch_map[i].patch = MT32_PresetTimbreMaps[i].gm_instr;
		map->patch_key_shift[i] = 0;
		map->patch_volume_adjust[i] = 0;
		map->patch_bend_range[i] = 12;
		map->velocity_map_index[i] = SFX_NO_VELOCITY_MAP;
	}

	map->percussion_volume_adjust = 0;
	map->percussion_velocity_map_index = SFX_NO_VELOCITY_MAP;

	for (i = 0; i < SFX_RHYTHM_NR; i++) {
		map->percussion_map[i] = MT32_PresetRhythmKeymap[i];
		map->percussion_velocity_scale[i] = SFX_MAX_VELOCITY;
	}

	if (!data) {
		printf("[MT32-to-GM] No MT-32 patch data supplied, using default mapping\n");
		return map;
	}

	type = sfx_instrument_map_detect(data, size);

	if (type == SFX_MAP_UNKNOWN) {
		printf("[MT32-to-GM] Patch data format unknown, using default mapping\n");
		return map;
	}
	if (type == SFX_MAP_MT32_GM) {
		printf("[MT32-to-GM] Patch data format not supported, using default mapping\n");
		return map;
	}

	memtimbres = *(data + 0x1EB);
	pos = 0x1EC + memtimbres * 0xF6;

	if (size > pos && ((0x100 * *(data + pos) + *(data + pos + 1)) == 0xABCD)) {
		patches = 96;
		pos += 2 + 8 * 48;
	} else
		patches = 48;

	printf("[MT32-to-GM] %d MT-32 Patches detected\n", patches);
	printf("[MT32-to-GM] %d MT-32 Memory Timbres\n", memtimbres);

	printf("[MT32-to-GM] Mapping patches..\n");

	for (i = 0; i < patches; i++) {
		char *name;

		if (i < 48)
			patchpointer = data + 0x6B + 8 * i;
		else
			patchpointer = data + 0x1EC + 8 * (i - 48) + memtimbres * 0xF6 + 2;

		group = *patchpointer;
		number = *(patchpointer + 1);
		keyshift = *(patchpointer + 2);
		finetune = *(patchpointer + 3);
		bender_range = *(patchpointer + 4);

		switch (group) {
		case 0:
			map->patch_map[i].patch = MT32_PresetTimbreMaps[number].gm_instr;
			map->patch_map[i].rhythm = MT32_PresetTimbreMaps[number].gm_rhythm_key;
			print_map(i, map->patch_map[i].patch, map->patch_map[i].rhythm, number);
			break;
		case 1:
			map->patch_map[i].patch = MT32_PresetTimbreMaps[number + 64].gm_instr;
			map->patch_map[i].rhythm = MT32_PresetTimbreMaps[number + 64].gm_rhythm_key;
			print_map(i, map->patch_map[i].patch, map->patch_map[i].rhythm, number + 64);
			break;
		case 2:
			name = (char *) data + 0x1EC + number * 0xF6;
			map->patch_map[i].patch = lookup_instrument(name);
			map->patch_map[i].rhythm = SFX_UNMAPPED;
			print_map_mem(i, map->patch_map[i].patch, map->patch_map[i].rhythm, name);
			break;
		case 3:
			map->patch_map[i].patch = MT32_RhythmTimbreMaps[number].gm_instr;
			map->patch_map[i].rhythm = SFX_UNMAPPED;
			print_map_rhythm(i, map->patch_map[i].patch, map->patch_map[i].rhythm, number);
			break;
		default:
			break;
		}

		/* map->patch_key_shift[i] = (int) (keyshift & 0x3F) - 24; */
		map->patch_bend_range[i] = bender_range & 0x1F;
	}

	if (size > pos && ((0x100 * *(data + pos) + *(data + pos + 1)) == 0xDCBA)) {
		printf("[MT32-to-GM] Mapping percussion..\n");

		for (i = 0; i < 64 ; i++) {
			number = *(data + pos + 4 * i + 2);

			if (number < 64) {
				char *name = (char *) data + 0x1EC + number * 0xF6;
				map->percussion_map[i + 23] = lookup_rhythm_key(name);
				print_map_rhythm_mem(i, map->percussion_map[i + 23], name);
			} else {
				if (number < 94) {
					map->percussion_map[i + 23] = MT32_RhythmTimbreMaps[number - 64].gm_rhythmkey;
					print_map_rhythm(i, SFX_MAPPED_TO_RHYTHM, map->percussion_map[i + 23], number - 64);
				} else
					map->percussion_map[i + 23] = SFX_UNMAPPED;
			}

			map->percussion_velocity_scale[i + 23] = *(data + pos + 4 * i + 3) * SFX_MAX_VELOCITY / 100;
		}
	}

	return map;
}

} // End of namespace Sci
