/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"

#include "base/engine.h"
#include "common/config-manager.h"
#include "common/str.h"
#include "sound/mididrv.h"

/** Internal list of all available 'midi' drivers. */
static const struct MidiDriverDescription midiDrivers[] = {
	{"auto", "Default", MD_AUTO},
	{"null", "No music", MD_NULL},

#if defined(WIN32) && !defined(_WIN32_WCE)
	{"windows", "Windows MIDI", MD_WINDOWS},
#endif

#if defined(UNIX) && !defined(__BEOS__) && !defined(MACOSX)
	{"seq", "SEQ", MD_SEQ},
#endif

#if defined(MACOSX)
	{"qt", "QuickTime", MD_QTMUSIC},
	{"core", "CoreAudio", MD_COREAUDIO},
#endif

#if defined(__MORPHOS__)
	{"etude", "Etude", MD_ETUDE},
#endif

#if defined(UNIX) && defined(USE_ALSA)
	{"alsa", "ALSA", MD_ALSA},
#endif

	{"adlib", "Adlib", MD_ADLIB},
	{"towns", "FM Towns", MD_TOWNS},
	{"pcspk", "PC Speaker", MD_PCSPK},
	{"pcjr", "IBM PCjr", MD_PCJR},
#ifdef USE_MT32EMU
	{"mt32", "MT-32", MD_MT32},
#endif

#if defined(__PALM_OS__)
	{"ypa1", "Yamaha Pa1", MD_YPA1},
	{"zodiac", "Tapwave Zodiac", MD_ZODIAC},
#endif
	{0, 0, 0}
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
	int musicDriver = parseMusicDriver(ConfMan.get("music_driver"));
	/* Use the adlib sound driver if auto mode is selected,
	 * and the game is one of those that want adlib as
	 * default, OR if the game is an older game that doesn't
	 * support anything else anyway. */
	if (musicDriver == MD_AUTO || musicDriver < 0) {
		if (midiFlags & MDT_PREFER_NATIVE) {
			if (musicDriver == MD_AUTO) {
				#if defined (WIN32) && !defined(_WIN32_WCE)
					musicDriver = MD_WINDOWS; // MD_WINDOWS is default MidiDriver on windows targets
				#elif defined(MACOSX)
					musicDriver = MD_COREAUDIO;
				#elif defined(__PALM_OS__)	// must be before mac
					musicDriver = MD_YPA1;	// TODO : cahnge this and use Zodiac driver when needed
				#elif defined(__MORPHOS__)
					musicDriver = MD_ETUDE;
				#elif defined (_WIN32_WCE) || defined(UNIX) || defined(X11_BACKEND)
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

#ifdef USE_MT32EMU
	case MD_MT32:      return MidiDriver_MT32_create(g_engine->_mixer);
#endif

	case MD_TOWNS:     return MidiDriver_YM2612_create(g_engine->_mixer);

	// Right now PC Speaker and PCjr are handled
	// outside the MidiDriver architecture, so
	// don't create anything for now.
	case MD_PCSPK:
	case MD_PCJR:      return NULL;
#if defined(__PALM_OS__)
	case MD_YPA1:      return MidiDriver_YamahaPa1_create();
#ifndef DISABLE_TAPWAVE
	case MD_ZODIAC:    return MidiDriver_Zodiac_create();
#endif
#endif
#if defined(WIN32) && !defined(_WIN32_WCE)
	case MD_WINDOWS:   return MidiDriver_WIN_create();
#endif
#if defined(__MORPHOS__)
	case MD_ETUDE:     return MidiDriver_ETUDE_create();
#endif
#if defined(UNIX) && !defined(__BEOS__) && !defined(MACOSX)
	case MD_SEQ:       return MidiDriver_SEQ_create();
#endif
#if (defined(MACOSX) || defined(macintosh)) && !defined(__PALM_OS__)
	case MD_QTMUSIC:   return MidiDriver_QT_create();
#endif
#if defined(MACOSX)
	case MD_COREAUDIO: return MidiDriver_CORE_create();
#endif
#if defined(UNIX) && defined(USE_ALSA)
	case MD_ALSA:      return MidiDriver_ALSA_create();
#endif
	}

	error("Invalid midi driver selected");
	return NULL;
}
