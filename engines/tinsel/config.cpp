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
 * This file contains configuration functionality
 */

//#define USE_3FLAGS 1

#include "tinsel/config.h"
#include "tinsel/dw.h"
#include "tinsel/sound.h"
#include "tinsel/music.h"

#include "common/file.h"
#include "common/config-manager.h"

#include "sound/mixer.h"

namespace Tinsel {

//----------------- GLOBAL GLOBAL DATA --------------------

int dclickSpeed = DOUBLE_CLICK_TIME;
int volMidi = MAXMIDIVOL;
int volSound = MAXSAMPVOL;
int volVoice = MAXSAMPVOL;
int speedText = DEFTEXTSPEED;
int bSubtitles = false;
int bSwapButtons = 0;
LANGUAGE language = TXT_ENGLISH;
int bAmerica = 0;


// Shouldn't really be here, but time is short...
bool bNoBlocking;

/**
 * WriteConfig()
 */

void WriteConfig(void) {
	ConfMan.setInt("dclick_speed", dclickSpeed);
	ConfMan.setInt("music_volume", (volMidi * Audio::Mixer::kMaxChannelVolume) / MAXMIDIVOL);
	ConfMan.setInt("sfx_volume", (volSound * Audio::Mixer::kMaxChannelVolume) / MAXSAMPVOL);
	ConfMan.setInt("speech_volume", (volVoice * Audio::Mixer::kMaxChannelVolume) / MAXSAMPVOL);
	ConfMan.setInt("talkspeed", (speedText * 255) / 100);
	ConfMan.setBool("subtitles", bSubtitles);
	//ConfMan.setBool("swap_buttons", bSwapButtons ? 1 : 0);
	//ConfigData.language = language;	// not necessary, as language has been set in the launcher
	//ConfigData.bAmerica = bAmerica;		// EN_USA / EN_GRB
}

/*---------------------------------------------------------------------*\
|	ReadConfig()							|
|-----------------------------------------------------------------------|
|
\*---------------------------------------------------------------------*/
void ReadConfig(void) {
	if (ConfMan.hasKey("dclick_speed"))
		dclickSpeed = ConfMan.getInt("dclick_speed");

	volMidi = (ConfMan.getInt("music_volume") * MAXMIDIVOL) / Audio::Mixer::kMaxChannelVolume;
	volSound = (ConfMan.getInt("sfx_volume") * MAXSAMPVOL) / Audio::Mixer::kMaxChannelVolume;
	volVoice = (ConfMan.getInt("speech_volume") * MAXSAMPVOL) / Audio::Mixer::kMaxChannelVolume;

	if (ConfMan.hasKey("talkspeed"))
		speedText = (ConfMan.getInt("talkspeed") * 100) / 255;
	if (ConfMan.hasKey("subtitles"))
		bSubtitles = ConfMan.getBool("subtitles");

	// FIXME: If JAPAN is set, subtitles are forced OFF in the original engine

	//bSwapButtons = ConfMan.getBool("swap_buttons") == 1 ? true : false;
	//ConfigData.language = language;	// not necessary, as language has been set in the launcher
	//ConfigData.bAmerica = bAmerica;		// EN_USA / EN_GRB

// The flags here control how many country flags are displayed in one of the option dialogs.
#if defined(USE_3FLAGS) || defined(USE_4FLAGS) || defined(USE_5FLAGS)
	language = ConfigData.language;
 #ifdef USE_3FLAGS
	if (language == TXT_ENGLISH || language == TXT_ITALIAN) {
		language = TXT_GERMAN;
		bSubtitles = true;
	}
 #endif
 #ifdef USE_4FLAGS
	if (language == TXT_ENGLISH) {
		language = TXT_GERMAN;
		bSubtitles = true;
	}
 #endif
#else
	language = TXT_ENGLISH;
#endif
}

bool isJapanMode() {
#ifdef JAPAN
	return true;
#else
	return false;
#endif
}

} // end of namespace Tinsel
