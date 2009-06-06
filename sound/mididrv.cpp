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

#include "engines/engine.h"
#include "common/config-manager.h"
#include "common/str.h"
#include "common/system.h"
#include "common/util.h"
#include "sound/mididrv.h"

/** Internal list of all available 'midi' drivers. */
static const MidiDriverDescription s_musicDrivers[] = {

	// The flags for the "auto" & "null" drivers indicate that they are anything
	// you want it to be.
	{"auto", "<default>", MD_AUTO, MDT_MIDI | MDT_PCSPK | MDT_ADLIB | MDT_TOWNS},
	{"null", "No music", MD_NULL, MDT_MIDI | MDT_PCSPK | MDT_ADLIB | MDT_TOWNS},

#if defined(WIN32) && !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
	{"windows", "Windows MIDI", MD_WINDOWS, MDT_MIDI},
#endif

#if defined(UNIX) && defined(USE_ALSA)
	{"alsa", "ALSA", MD_ALSA, MDT_MIDI},
#endif

#if defined(__MINT__)
	{"stmidi", "Atari ST MIDI", MD_STMIDI, MDT_MIDI},
#endif

#if defined(UNIX) && !defined(__BEOS__) && !defined(MACOSX) && !defined(__MAEMO__) && !defined(__MINT__)
	{"seq", "SEQ", MD_SEQ, MDT_MIDI},
#endif

#if defined(IRIX)
	{"dmedia", "DMedia", MD_DMEDIA, MDT_MIDI},
#endif

#if defined(__amigaos4__)
	{"camd", "CAMD", MD_CAMD, MDT_MIDI},
#endif

#if defined(MACOSX)
	{"core", "CoreAudio", MD_COREAUDIO, MDT_MIDI},
//	{"coreaudio", "CoreAudio", MD_COREAUDIO, MDT_MIDI},
	{"coremidi", "CoreMIDI", MD_COREMIDI, MDT_MIDI},
	{"qt", "QuickTime", MD_QTMUSIC, MDT_MIDI},
#endif

#if defined(PALMOS_MODE)
#	if defined(COMPILE_CLIE)
	{"ypa1", "Yamaha Pa1", MD_YPA1, MDT_MIDI},
#	elif defined(COMPILE_ZODIAC) && (!defined(ENABLE_SCUMM) || !defined(PALMOS_ARM))
	{"zodiac", "Tapwave Zodiac", MD_ZODIAC, MDT_MIDI},
#	endif
#endif

#ifdef USE_FLUIDSYNTH
	{"fluidsynth", "FluidSynth", MD_FLUIDSYNTH, MDT_MIDI},
#endif
#ifdef USE_MT32EMU
	{"mt32", "MT-32", MD_MT32, MDT_MIDI},
#endif

	// The flags for the "adlib" driver indicates that it can do adlib and MIDI.
	{"adlib", "AdLib", MD_ADLIB, MDT_ADLIB},
	{"pcspk", "PC Speaker", MD_PCSPK, MDT_PCSPK},
	{"pcjr", "IBM PCjr", MD_PCJR, MDT_PCSPK},
	{"cms", "Creative Music System", MD_CMS, MDT_CMS},
	{"towns", "FM Towns", MD_TOWNS, MDT_TOWNS},
#if defined(UNIX)
	{"timidity", "TiMidity", MD_TIMIDITY, MDT_MIDI},
#endif

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
	return s_musicDrivers;
}

const MidiDriverDescription *MidiDriver::findMusicDriver(const Common::String &str) {
	if (str.empty())
		return 0;

	const char *s = str.c_str();
	int len = 0;
	const MidiDriverDescription *md = s_musicDrivers;

	// Scan for string end or a colon
	while (s[len] != 0 && s[len] != ':')
		len++;

	while (md->name) {
		// Compare the string passed to us with the current table entry.
		// We ignore any characters following an (optional) colon ':'
		// contained in str.
		if (!scumm_strnicmp(md->name, s, len)) {
			return md;
		}
		md++;
	}

	return 0;
}

static MidiDriverType getDefaultMIDIDriver() {
#if defined(WIN32) && !defined(_WIN32_WCE) &&  !defined(__SYMBIAN32__)
	return MD_WINDOWS;
#elif defined(MACOSX)
	return MD_COREAUDIO;
#elif defined(PALMOS_MODE)
  #if defined(COMPILE_CLIE)
	return MD_YPA1;
  #elif defined(COMPILE_ZODIAC)
	return MD_ZODIAC;
  #else
	return MD_NULL;
  #endif
#else
	return MD_NULL;
#endif
}

MidiDriverType MidiDriver::parseMusicDriver(const Common::String &str) {
	const MidiDriverDescription *md = findMusicDriver(str);
	if (md)
		return md->id;
	return MD_AUTO;
}

MidiDriverType MidiDriver::detectMusicDriver(int flags) {
	MidiDriverType musicDriver;

	// Query the selected music driver (defaults to MD_AUTO).
	const MidiDriverDescription *md = findMusicDriver(ConfMan.get("music_driver"));

	// Check whether the selected music driver is compatible with the
	// given flags.
	if (!md || !(md->flags & flags))
		musicDriver = MD_AUTO;
	else
		musicDriver = md->id;

	// If the selected driver is MD_AUTO, we try to determine
	// a suitable and "optimal" music driver.
	if (musicDriver == MD_AUTO) {

		if (flags & MDT_PREFER_MIDI) {
			// A MIDI music driver is preferred. Of course this implies
			// that MIDI is actually listed in flags, so we verify that.
			assert(flags & MDT_MIDI);

			// Query the default MIDI driver. It's possible that there
			// is none, in which case we revert to AUTO mode.
			musicDriver = getDefaultMIDIDriver();
			if (musicDriver == MD_NULL)
				musicDriver = MD_AUTO;
		}

		if (musicDriver == MD_AUTO) {
			// MIDI is not preferred, or no default MIDI device is available.
			// In this case we first try the alternate drivers before checking
			// for a 'real' MIDI driver.

			if (flags & MDT_TOWNS)
				musicDriver = MD_TOWNS;
			else if (flags & MDT_ADLIB)
				musicDriver = MD_ADLIB;
			else if (flags & MDT_PCSPK)
				musicDriver = MD_PCJR;
			else if (flags & MDT_MIDI)
				musicDriver = getDefaultMIDIDriver();
			else
				musicDriver = MD_NULL;
		}
	}

	return musicDriver;
}

MidiDriver *MidiDriver::createMidi(int midiDriver) {
	switch (midiDriver) {
	case MD_NULL:      return MidiDriver_NULL_create(g_system->getMixer());

	case MD_ADLIB:     return MidiDriver_ADLIB_create(g_system->getMixer());

	case MD_TOWNS:     return MidiDriver_YM2612_create(g_system->getMixer());

	// Right now PC Speaker and PCjr are handled
	// outside the MidiDriver architecture, so
	// don't create anything for now.
	case MD_PCSPK:
	case MD_CMS:
	case MD_PCJR:      return NULL;

#ifdef USE_FLUIDSYNTH
	case MD_FLUIDSYNTH:	return MidiDriver_FluidSynth_create(g_system->getMixer());
#endif

#ifdef USE_MT32EMU
	case MD_MT32:      return MidiDriver_MT32_create(g_system->getMixer());
#endif

#if defined(PALMOS_MODE)
#if defined(COMPILE_CLIE)
	case MD_YPA1:      return MidiDriver_YamahaPa1_create(g_system->getMixer());
#elif defined(COMPILE_ZODIAC) && (!defined(ENABLE_SCUMM) || !defined(PALMOS_ARM))
	case MD_ZODIAC:    return MidiDriver_Zodiac_create(g_system->getMixer());
#endif
#endif

#if defined(WIN32) && !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
	case MD_WINDOWS:   return MidiDriver_WIN_create(g_system->getMixer());
#endif
#if defined(__MINT__)
	case MD_STMIDI:    return MidiDriver_STMIDI_create(g_system->getMixer());
#endif
#if defined(UNIX) && !defined(__BEOS__) && !defined(MACOSX) && !defined(__MAEMO__) && !defined(__MINT__)
	case MD_SEQ:       return MidiDriver_SEQ_create(g_system->getMixer());
#endif
#if defined(UNIX)
	case MD_TIMIDITY:  return MidiDriver_TIMIDITY_create(g_system->getMixer());
#endif
#if defined(IRIX)
	case MD_DMEDIA:    return MidiDriver_DMEDIA_create(g_system->getMixer());
#endif
#if defined(MACOSX)
	case MD_QTMUSIC:   return MidiDriver_QT_create(g_system->getMixer());
	case MD_COREAUDIO: return MidiDriver_CORE_create(g_system->getMixer());
	case MD_COREMIDI:  return MidiDriver_CoreMIDI_create(g_system->getMixer());
#endif
#if defined(UNIX) && defined(USE_ALSA)
	case MD_ALSA:      return MidiDriver_ALSA_create(g_system->getMixer());
#endif
#if defined(__amigaos4__)
	case MD_CAMD:      return MidiDriver_CAMD_create(g_system->getMixer());
#endif
	}

	error("Invalid midi driver selected");
	return NULL;
}
