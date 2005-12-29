/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"

#include "base/engine.h"
#include "common/config-manager.h"
#include "common/str.h"
#include "common/util.h"
#include "sound/mididrv.h"

/** Internal list of all available 'midi' drivers. */
static const struct MidiDriverDescription midiDrivers[] = {

	// The flags for the "auto" driver indicate that it is anything you want
	// it to be.
	{"auto", "Default", MD_AUTO, MDT_MIDI | MDT_PCSPK | MDT_ADLIB | MDT_TOWNS},

	// The flags for the "null" driver indicate that it does nothing, really.
	{"null", "No music", MD_NULL, MDT_NONE},

#if defined(WIN32) && !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
	{"windows", "Windows MIDI", MD_WINDOWS, MDT_MIDI},
#endif

#if defined(UNIX) && defined(USE_ALSA)
	{"alsa", "ALSA", MD_ALSA, MDT_MIDI},
#endif

#if defined(UNIX) && !defined(__BEOS__) && !defined(MACOSX)
	{"seq", "SEQ", MD_SEQ, MDT_MIDI},
#endif

#if defined(MACOSX)
	{"qt", "QuickTime", MD_QTMUSIC, MDT_MIDI},
	{"core", "CoreAudio", MD_COREAUDIO, MDT_MIDI},
//	{"coreaudio", "CoreAudio", MD_COREAUDIO, MDT_MIDI},
	{"coremidi", "CoreMIDI", MD_COREMIDI, MDT_MIDI},
#endif

#if defined(PALMOS_MODE)
	{"ypa1", "Yamaha Pa1", MD_YPA1, MDT_MIDI},
	{"zodiac", "Tapwave Zodiac", MD_ZODIAC, MDT_MIDI},
#endif

#if defined(__MORPHOS__)
	{"etude", "Etude", MD_ETUDE, MDT_MIDI},
#endif
#ifdef USE_FLUIDSYNTH
	{"fluidsynth", "FluidSynth", MD_FLUIDSYNTH, MDT_MIDI},
#endif
#ifdef USE_MT32EMU
	{"mt32", "MT-32", MD_MT32, MDT_MIDI},
#endif

	// The flags for the "adlibe" driver indicate that it can do adlib and MIDI.
	{"adlib", "Adlib", MD_ADLIB, MDT_ADLIB | MDT_MIDI},
	{"pcspk", "PC Speaker", MD_PCSPK, MDT_PCSPK},
	{"pcjr", "IBM PCjr", MD_PCJR, MDT_PCSPK},
	{"towns", "FM Towns", MD_TOWNS, MDT_TOWNS},

	{0, 0, MD_NULL, MDT_NONE}
};

const byte MidiDriver::_mt32ToGm[128] = {
//	  0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
	  0,   1,   0,   2,   4,   4,   5,   3,  16,  17,  18,  16,  16,  19,  20,  21, // 0x
	  6,   6,   6,   7,   7,   7,   8, 112,  62,  62,  63,  63,  38,  38,  39,  39, // 1x
	 88,  95,  52,  98,  97,  99,  14,  54, 102,  96,  53, 102,  81, 100,  14,  80, // 2x
	 48,  48,  49,  45,  41,  40,  42,  42,  43,  46,  45,  24,  25,  28,  27, 104, // 3x
	 32,  32,  34,  33,  36,  37,  35,  35,  79,  73,  72,  72,  74,  75,  64,  65, // 4x
	 66,  67,  71,  71,  68,  69,  70,  22,  56,  59,  57,  57,  60,  60,  58,  61, // 5x
	 61,  11,  11,  98,  14,   9,  14,  13,  12, 107, 107,  77,  78,  78,  76,  76, // 6x
	 47, 117, 127, 118, 118, 116, 115, 119, 115, 112,  55, 124, 123,   0,  14, 117  // 7x
};

const byte MidiDriver::_gmToMt32[128] = {
//	  0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
	  5,   1,   2,   7,   3,   5,  16,  21,  22, 101, 101,  97, 104, 103, 102,  20, // 0x
	  8,   9,  11,  12,  14,  15,  87,  15,  59,  60,  61,  62,  67,  44,  79,  23, // 1x
	 64,  67,  66,  70,  68,  69,  28,  31,  52,  54,  55,  56,  49,  51,  57, 112, // 2x
	 48,  50,  45,  26,  34,  35,  45, 122,  89,  90,  94,  81,  92,  95,  24,  25, // 3x
	 80,  78,  79,  78,  84,  85,  86,  82,  74,  72,  76,  77, 110, 107, 108,  76, // 4x
	 47,  44, 111,  45,  44,  34,  44,  30,  32,  33,  88,  34,  35,  35,  38,  33, // 5x
	 41,  36, 100,  37,  40,  34,  43,  40,  63,  21,  99, 105, 103,  86,  55,  84, // 6x
	101, 103, 100, 120, 117, 113,  99, 128, 128, 128, 128, 124, 123, 128, 128, 128, // 7x
};

const MidiDriverDescription *MidiDriver::getAvailableMidiDrivers() {
	return midiDrivers;
}

int MidiDriver::parseMusicDriver(const Common::String &str) {
	if (str.isEmpty())
		return -1;

	const char *s = str.c_str();
	const MidiDriverDescription *md = getAvailableMidiDrivers();

	while (md->name) {
		if (!scumm_stricmp(md->name, s)) {
			return md->id;
		}
		md++;
	}

	return -1;
}

int MidiDriver::detectMusicDriver(int midiFlags) {
	/*
	TODO: The code in this method is very complicated and convuluted. Maybe we
	can improve it. 
	
	First off, one needs to understand why it is so complex. It tries to honor
	the user's music_driver config, but with the restrictions imposed by midiFlags.
	Hence it must either select a suitable default driver (for example if
	musicDriver is set to MD_AUTO), or it must try to fall back to a suitable
	driver resp. the NULL driver.
	
	Different games support different output drivers, as indicated by midiFlags.
	Some of the occuring combinations are:
	 - TOWNS games always want towns or null
	 - some scumm games allow only pcspk, pcjr
	 - some scumm games allow pcspk, pcjr, adlib, MIDI
	 - some games allow adlib, MIDI
	 - some games only allow MIDI
	 - did I miss something?
	
	My hope is that we can simplify the whole selection process by iterating over
	the list of available drivers and looking at their "flags" member.
	*/

	int musicDriver = parseMusicDriver(ConfMan.get("music_driver"));
	/* Use the adlib sound driver if auto mode is selected,
	 * and the game is one of those that want adlib as
	 * default, OR if the game is an older game that doesn't
	 * support anything else anyway. */
	if (musicDriver == MD_AUTO || musicDriver < 0) {
		if (midiFlags & MDT_PREFER_MIDI) {
			if (musicDriver == MD_AUTO) {
				#if defined(WIN32) && !defined(_WIN32_WCE) &&  !defined(__SYMBIAN32__)
					musicDriver = MD_WINDOWS; // MD_WINDOWS is default MidiDriver on windows targets
				#elif defined(MACOSX)
					musicDriver = MD_COREAUDIO;
				#elif defined(PALMOS_MODE)	// must be before mac
					musicDriver = MD_YPA1;	// TODO : change this and use Zodiac driver when needed
				#elif defined(__MORPHOS__)
					musicDriver = MD_ETUDE;
				#elif defined(_WIN32_WCE) || defined(UNIX) || defined(X11_BACKEND) || defined (__SYMBIAN32__)
					// Always use MIDI emulation via adlib driver on CE and UNIX device
				
					// TODO: We should, for the Unix targets, attempt to detect
					// whether a sequencer is available, and use it instead.
					musicDriver = MD_ADLIB;
				#else
					musicDriver = MD_NULL;
				#endif
			} else
				musicDriver = MD_ADLIB;
		} else
			musicDriver = MD_TOWNS;
	}
	bool nativeMidiDriver =
		(musicDriver != MD_NULL && musicDriver != MD_ADLIB &&
		 musicDriver != MD_PCSPK && musicDriver != MD_PCJR &&
		 musicDriver != MD_TOWNS);

	if (nativeMidiDriver && !(midiFlags & MDT_NATIVE))
		musicDriver = MD_TOWNS;
	if (musicDriver == MD_TOWNS && !(midiFlags & MDT_TOWNS))
		musicDriver = MD_ADLIB;
	if (musicDriver == MD_ADLIB && !(midiFlags & MDT_ADLIB))
		musicDriver = MD_PCJR;
	if ((musicDriver == MD_PCSPK || musicDriver == MD_PCJR) && !(midiFlags & MDT_PCSPK))
		musicDriver = MD_NULL;

	return musicDriver;
}

MidiDriver *MidiDriver::createMidi(int midiDriver) {
	switch(midiDriver) {
	case MD_NULL:      return MidiDriver_NULL_create();

	// In the case of Adlib, we won't specify anything.
	// IMuse is designed to set up its own Adlib driver
	// if need be, and we only have to specify a native
	// driver.
	case MD_ADLIB:     return NULL;

#ifdef USE_FLUIDSYNTH
	case MD_FLUIDSYNTH:	return MidiDriver_FluidSynth_create(g_engine->_mixer);
#endif
#ifdef USE_MT32EMU
	case MD_MT32:      return MidiDriver_MT32_create(g_engine->_mixer);
#endif

	case MD_TOWNS:     return MidiDriver_YM2612_create(g_engine->_mixer);

	// Right now PC Speaker and PCjr are handled
	// outside the MidiDriver architecture, so
	// don't create anything for now.
	case MD_PCSPK:
	case MD_PCJR:      return NULL;
#if defined(PALMOS_MODE)
#if defined(COMPILE_CLIE)
	case MD_YPA1:      return MidiDriver_YamahaPa1_create();
#elif defined(COMPILE_ZODIAC)
	case MD_ZODIAC:    return MidiDriver_Zodiac_create();
#endif
#endif
#if defined(WIN32) && !defined(_WIN32_WCE) && !defined(__SYMBIAN32__) 
	case MD_WINDOWS:   return MidiDriver_WIN_create();
#endif
#if defined(__MORPHOS__)
	case MD_ETUDE:     return MidiDriver_ETUDE_create();
#endif
#if defined(UNIX) && !defined(__BEOS__) && !defined(MACOSX)
	case MD_SEQ:       return MidiDriver_SEQ_create();
#endif
#if (defined(MACOSX) || defined(macintosh)) && !defined(PALMOS_MODE)
	case MD_QTMUSIC:   return MidiDriver_QT_create();
#endif
#if defined(MACOSX)
	case MD_COREAUDIO: return MidiDriver_CORE_create();
	case MD_COREMIDI: return MidiDriver_CoreMIDI_create();
#endif
#if defined(UNIX) && defined(USE_ALSA)
	case MD_ALSA:      return MidiDriver_ALSA_create();
#endif
	}

	error("Invalid midi driver selected");
	return NULL;
}

