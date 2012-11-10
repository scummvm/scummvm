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

#include "common/system.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "hopkins/sound.h"
#include "hopkins/globals.h"
#include "hopkins/hopkins.h"

namespace Hopkins {

SoundManager::SoundManager() {
	SPECIAL_SOUND = 0;
	SOUNDVOL = 0;
	VOICEVOL = 0;
	MUSICVOL = 0;
	OLD_SOUNDVOL = 0;
	OLD_MUSICVOL = 0;
	OLD_VOICEVOL = 0;
	SOUNDOFF = true;
	MUSICOFF = true;
	VOICEOFF = true;
	TEXTOFF = false;
	SOUND_FLAG = false;
	VBL_MERDE = false;
	CARD_SB = false;

	for (int i = 0; i < VOICE_COUNT; ++i)
		Common::fill((byte *)&Voice[i], (byte *)&Voice[i] + sizeof(VoiceItem), 0);
	for (int i = 0; i < SWAV_COUNT; ++i)
		Common::fill((byte *)&Swav[i], (byte *)&Swav[i] + sizeof(SwavItem), 0);
	for (int i = 0; i < 2; ++i)
		Common::fill((byte *)&Music[i], (byte *)&Music[i] + sizeof(MusicItem), 0);
}

void SoundManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
	SPECIAL_SOUND = 0;
}

void SoundManager::WSOUND_INIT() {
	warning("TODO: WSOUND_INIT");
}

void SoundManager::VERIF_SOUND() {
//	warning("TODO: VERIF_SOUND");
}

void SoundManager::LOAD_ANM_SOUND() {
	warning("TODO: LOAD_ANIM_SOUND");
}

void SoundManager::LOAD_WAV(const Common::String &file, int a2) {
	warning("TODO: LOAD_WAV");
}

void SoundManager::PLAY_ANM_SOUND(int soundNumber) {
	if (!_vm->_globals.CENSURE && SPECIAL_SOUND == 2) {
		if (soundNumber == 20)
			PLAY_SAMPLE2(5);
		if (soundNumber == 57)
			PLAY_SAMPLE2(1);
		if (soundNumber == 63)
			PLAY_SAMPLE2(1);
		if (soundNumber == 69)
			PLAY_SAMPLE2(1);
		if (soundNumber == 75)
			PLAY_SAMPLE2(2);
		if (soundNumber == 109)
			PLAY_SAMPLE2(3);
		if (soundNumber == 122)
			PLAY_SAMPLE2(4);
	}
	if (SPECIAL_SOUND == 1 && soundNumber == 17)
		PLAY_SOUND("SOUND42.WAV");
	if (SPECIAL_SOUND == 199 && soundNumber == 72)
		PLAY_WAV(1);
	if (SPECIAL_SOUND == 198 && soundNumber == 15)
		PLAY_WAV(1);
	if (SPECIAL_SOUND == 16 && soundNumber == 25)
		PLAY_WAV(1);
	if (SPECIAL_SOUND == 14 && soundNumber == 625)
		PLAY_WAV(1);
	if (SPECIAL_SOUND == 208 && soundNumber == 40)
		PLAY_WAV(1);
	if (SPECIAL_SOUND == 210 && soundNumber == 2)
		PLAY_WAV(1);
	if (SPECIAL_SOUND == 211 && soundNumber == 22)
		PLAY_WAV(1);
	if (SPECIAL_SOUND == 5 && soundNumber == 19)
		PLAY_WAV(1);
	if (SPECIAL_SOUND == 17) {
		if (soundNumber == 6)
			PLAY_SAMPLE2(1);
		if (soundNumber == 14)
			PLAY_SAMPLE2(2);
		if (soundNumber == 67)
			PLAY_SAMPLE2(3);
	}
	if (SPECIAL_SOUND == 229) {
		if (soundNumber == 15)
			PLAY_WAV(1);
		if (soundNumber == 91)
			PLAY_WAV(2);
	}
}

void SoundManager::WSOUND(int soundNumber) {
	warning("TODO: WSOUND");
}

bool SoundManager::VOICE_MIX(int voiceId, int voiceMode) {
	int fileNumber; 
	int oldMusicVol; 
	bool breakFlag; 
	Common::String prefix;
	Common::String filename;
	Common::File f;
	int catPos;

	fileNumber = voiceId;
	if (!CARD_SB || VOICEOFF == 1)
		return false;

	if ((unsigned int)(voiceMode - 1) <= 1
	        && (voiceId == 4
	            || voiceId == 16
	            || voiceId == 121
	            || voiceId == 142
	            || voiceId == 182
	            || voiceId == 191
	            || voiceId == 212
	            || voiceId == 225
	            || voiceId == 239
	            || voiceId == 245
	            || voiceId == 297
	            || voiceId == 308
	            || voiceId == 333
	            || voiceId == 348
	            || voiceId == 352
	            || voiceId == 358
	            || voiceId == 364
	            || voiceId == 371
	            || voiceId == 394
	            || voiceId == 414
	            || voiceId == 429
	            || voiceId == 442
	            || voiceId == 446
	            || voiceId == 461
	            || voiceId == 468
	            || voiceId == 476
	            || voiceId == 484
	            || voiceId == 491
	            || voiceId == 497
	            || voiceId == 501
	            || voiceId == 511
	            || voiceId == 520
	            || voiceId == 536
	            || voiceId == 554
	            || voiceId == 566
	            || voiceId == 573
	            || voiceId == 632
	            || voiceId == 645))
		fileNumber = 684;
	
	if ((unsigned int)(voiceMode - 1) <= 1) {
		prefix = "DF";
	}
	if (voiceMode == 3) {
		prefix = "IF";
	}
	if (voiceMode == 4) {
		prefix = "TF";
	}
	if (voiceMode == 5) {
		prefix = "OF";
	}

	filename = Common::String::format("%s%d.WAV", prefix.c_str(), fileNumber);
	
	if (!_vm->_fileManager.RECHERCHE_CAT(filename, 9)) {
		if (_vm->_globals.FR == 1)
			_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPVOICE, "RES_VFR.RES");
		if (!_vm->_globals.FR)
			_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPVOICE, "RES_VAN.RES");
		if (_vm->_globals.FR == 2)
			_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPVOICE, "RES_VES.RES");

		catPos = _vm->_globals.CAT_POSI;
	} else {
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPVOICE, filename);
	
		if (!f.exists(_vm->_globals.NFICHIER))
			return false;

		catPos = 0;
	}

	SDL_LVOICE(catPos);
	oldMusicVol = MUSICVOL;
	if (!MUSICOFF && MUSICVOL > 2)
		MUSICVOL = (signed int)((long double)MUSICVOL - (long double)MUSICVOL / 100.0 * 45.0);

	PLAY_VOICE_SDL();

	breakFlag = 0;
	do {
		if (SPECIAL_SOUND != 4 && !VBL_MERDE)
			_vm->_eventsManager.VBL();
		if (_vm->_eventsManager.BMOUSE())
			break;
		_vm->_eventsManager.CONTROLE_MES();
		if (_vm->_eventsManager.ESC_KEY)
			break;
		if (!VOICE_STAT(2))
			breakFlag = true;
	} while (!_vm->shouldQuit() && !breakFlag);


	STOP_VOICE(2);
	DEL_SAMPLE_SDL(20);
	MUSICVOL = oldMusicVol;
	_vm->_eventsManager.ESC_KEY = false;
	VBL_MERDE = 0;
	return true;
}

void SoundManager::DEL_SAMPLE(int soundNumber) {
	warning("TODO: DEL_SAMPLE");
}

void SoundManager::PLAY_SOUND(const Common::String &file) {
	warning("TODO: PLAY_SOUND");
}

void SoundManager::PLAY_SOUND2(const Common::String &file) {
	warning("TODO: PLAY_SOUND2");
}

void SoundManager::MODSetSampleVolume() {
	warning("TODO MODSetSampleVolume");
}

void SoundManager::MODSetVoiceVolume() {
	warning("TODO MODSetVoiceVolume");
}

void SoundManager::MODSetMusicVolume(int volume) {
	warning("TODO MODSetMusicVolume");
}

void SoundManager::CHARGE_SAMPLE(int a1, const Common::String &file) {
	warning("TODO: CHARGE_SAMPLE");
}

void SoundManager::PLAY_SAMPLE2(int idx) {
	warning("PLAY_SAMPLE2");
}

void SoundManager::PLAY_WAV(int a1) {
	warning("PLAY_WAV");
}

bool SoundManager::VOICE_STAT(int voiceIndex) {
	return Voice[voiceIndex].active;
}

void SoundManager::STOP_VOICE(int voiceIndex) {
	int wavIndex; 

	if (Voice[voiceIndex].active) {
		Voice[voiceIndex].active = false;
		wavIndex = Voice[voiceIndex].wavIndex;
		if (Swav[wavIndex].active) {
			if (Swav[wavIndex].field24 == 1)
				DEL_SAMPLE_SDL(wavIndex);
		}
	}
	Voice[voiceIndex].audioLen = 0;
	Voice[voiceIndex].fieldC = 0;
	Voice[voiceIndex].audioBuf = NULL;
	Voice[voiceIndex].active = false;
	Voice[voiceIndex].field14 = 0;
	Voice[voiceIndex].audioStream = NULL;
}

void SoundManager::SDL_LVOICE(int catPos) {
	if (!SDL_LoadVoice(_vm->_globals.NFICHIER, catPos, Swav[20]))
		error("Couldn't load the sample %s", _vm->_globals.NFICHIER.c_str());

	Swav[20].active = true;
}
	
void SoundManager::PLAY_VOICE_SDL() {
	if (!Swav[20].active)
		error("Bad handle");

	if (!Voice[2].active) {
		int wavIndex = Voice[2].wavIndex;
		if (Swav[wavIndex].active && Swav[wavIndex].field24 == 1)
			DEL_SAMPLE_SDL(wavIndex);
	}

	Voice[2].audioLen = Swav[20].audioLen;
	Voice[2].fieldC = 0;
	Voice[2].audioBuf = Swav[20].audioBuf;
	Voice[2].active = true;
	Voice[2].field14 = 4;
	Voice[2].audioStream = Swav[20].audioStream;
}

bool SoundManager::DEL_SAMPLE_SDL(int wavIndex) {
	if (Swav[wavIndex].active) {
		Swav[wavIndex].active = false;
		free(Swav[wavIndex].audioBuf);
		delete Swav[wavIndex].audioStream;
		return true;
	} else {
		return false;
	}
}

bool SoundManager::SDL_LoadVoice(const Common::String &filename, size_t fileOffset, SwavItem &item) {
	Common::File f;
	if (!f.open(filename))
		error("Could not open %s for reading", filename.c_str());

	f.seek(fileOffset);
	item.audioStream = Audio::makeWAVStream(&f, DisposeAfterUse::NO);
	f.close();

	return true;
}

} // End of namespace Hopkins
