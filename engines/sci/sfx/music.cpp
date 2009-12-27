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

#include "sound/audiostream.h"
#include "common/config-manager.h"

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/sfx/midiparser.h"
#include "sci/sfx/music.h"
#include "sci/sfx/softseq/pcjr.h"

namespace Sci {

static int f_compare(const void *arg1, const void *arg2) {
	return ((const MusicEntry *)arg2)->prio - ((const MusicEntry *)arg1)->prio;
}

SciMusic::SciMusic(SciVersion soundVersion)
	: _soundVersion(soundVersion), _soundOn(true) {

	// Reserve some space in the playlist, to avoid expensive insertion
	// operations
	_playList.reserve(10);
}

SciMusic::~SciMusic() {
	if (_pMidiDrv) {
		_pMidiDrv->close();
		delete _pMidiDrv;
	}
}
//----------------------------------------
void SciMusic::init() {
	// system init
	_pMixer = g_system->getMixer();
	_pMixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt(
			"sfx_volume"));
	_pMixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType,
			ConfMan.getInt("speech_volume"));
	_pMixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType,
			ConfMan.getInt("music_volume"));
	// SCI sound init
	_dwTempo = 0;

	_midiType = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PCSPK);

	// Sanity check
	if (_midiType != MD_ADLIB && _midiType != MD_PCJR && _midiType != MD_PCSPK) {
		warning("Unhandled MIDI type, switching to Adlib");
		_midiType = MD_ADLIB;
	}

	switch (_midiType) {
	case MD_ADLIB:
		// FIXME: There's no Amiga sound option, so we hook it up to Adlib
		if (((SciEngine *)g_engine)->getPlatform() == Common::kPlatformAmiga)
			_pMidiDrv = MidiPlayer_Amiga_create();
		else
			_pMidiDrv = MidiPlayer_Adlib_create();
		break;
	case MD_PCJR:
		_pMidiDrv = new MidiPlayer_PCJr();
		break;
	case MD_PCSPK:
		_pMidiDrv = new MidiPlayer_PCSpeaker();
		break;
	default:
		break;
	}

	if (_pMidiDrv) {
		_pMidiDrv->open();
		_pMidiDrv->setTimerCallback(this, &miditimerCallback);
		_dwTempo = _pMidiDrv->getBaseTempo();
	} else
		warning("Can't initialise music driver");
	_bMultiMidi = ConfMan.getBool("multi_midi");
}
//----------------------------------------
bool SciMusic::saveState(Common::OutSaveFile *pFile) {
	// TODO
#if 0
	pFile->writeString("AUDIO\n");
	// playlist
	int sz = _playList.size();
	pFile->writeUint16LE(sz);
	for(int i = 0; i < sz; i++) {
		pFile->writeUint16LE(ptr2heap((byte*)_playList[i]));
		// member variable
		pFile->writeUint16LE(_audVolNum);
		pFile->writeByte(_langID);
		pFile->writeUint16LE(_audioType);
		pFile->writeUint16LE(_audioRate);
		// TODO: current playing stream (hCurrentAud) info
	}
#endif
	return true;
}

//----------------------------------------
void SciMusic::stopAll() {
	_pMixer->stopAll();

	while (!_playList.empty()) {
		soundStop(_playList[0]);
		soundKill(_playList[0]);
	}
}
//----------------------------------------
void SciMusic::miditimerCallback(void *p) {
	SciMusic* aud = (SciMusic *)p;
	aud->onTimer();
}
//----------------------------------------
uint16 SciMusic::soundGetVoices() {
	switch (_midiType) {
	case MD_PCSPK:
		return 1;
	case MD_PCJR:
		return 3;
	case MD_ADLIB:
		return 8;
	case MD_MT32:
		return 16;
	default:
		return 1;
	}
}
//----------------------------------------
void SciMusic::sortPlayList() {
	MusicEntry ** pData = _playList.begin();
	qsort(pData, _playList.size(), sizeof(MusicEntry *), &f_compare);
}

#if 0
void SciMusic::patchSysEx(byte * addr, byte *pdata, int len) {
	byte *buff = new byte[7 + len + 1];
	uint16 chk = 0;
	int i;

	buff[0] = 0x41;
	buff[1] = 0x10;
	buff[2] = 0x16;
	buff[3] = 0x12;
	buff[4] = addr[0];
	buff[5] = addr[1];
	buff[6] = addr[2];
	for (i = 0; i < len; i++) {
		buff[7 + i] = pdata[i];
		chk += pdata[i];
	}
	chk += addr[0] + addr[1] + addr[2];
	buff[7 + i] = 128 - chk % 128;
	_pMidiDrv->sysEx(buff, len + 8);
	delete[] buff;
}

void SciMusic::patchUpdateAddr(byte *addr, int len) {
	addr[2] += len;
	if (addr[2] >= 0x7F) {
		addr[1]++;
		addr[2] -= 0x80;
	}
}
#endif

// FIXME: This should be done at the driver level
#if 0
void SciMusic::loadPatch() {
	if (_midiType == MD_MT32)
		loadPatchMT32();
}
#endif

#if 0
// currently loads patch 1.pat for Roland/MT-32 device
void SciMusic::loadPatchMT32() {
	//byte sysText[] = { 0x20, 0, 0 };
	byte sysMem[] = { 0x5, 0, 0 }; // patch memory
	byte sysRhytm[] = { 0x3, 0x1, 0x10 }; // rhytm
	byte sysMsg3[15] = { 0x41, 0x10, 0x16, 0x12, 0x52, 0, 0xA, 0x16, 0x16,
			0x16, 0x16, 0x16, 0x16, 0x20, 0x80 };
	byte sysTimbre[] = { 0x8, 0, 0 }; // timbre memory
	byte sysSystem[] = { 0x10, 0, 4 }; // partial reserve & midi channel
	byte arr[3][11];

	Resource *res = ((SciEngine *)g_engine)->getResourceManager()->findResource(ResourceId(kResourceTypePatch, 1), 0);

	if (res) {
		byte *pData = res->data, *p;
		// welcome message
		//patchSysEx(sysText, pData + 20, 20);
		// reading reverb mode, time and level
		p = pData + 74;
		for (int i = 0; i < 11; i++) {
			arr[0][i] = *p++;
			arr[1][i] = *p++;
			arr[2][i] = *p++;
		}
		// sub_657 - patch memory
		for (int i = 0; i < 48; i++) {
			patchSysEx(sysMem, p, 8);
			patchUpdateAddr(sysMem, 8);
			p += 8;
		}
		// sub_696 - timbre
		byte dl = *p++, cl = 0;
		while (dl--) {
			patchSysEx(sysTimbre, p, 14); // common area
			patchUpdateAddr(sysTimbre, 14);
			patchSysEx(sysTimbre, p + 14, 58);// partial 1
			patchUpdateAddr(sysTimbre, 58);
			patchSysEx(sysTimbre, p + 72, 58);// partial 2
			patchUpdateAddr(sysTimbre, 58);
			patchSysEx(sysTimbre, p + 130, 58);// partial 3
			patchUpdateAddr(sysTimbre, 58);
			patchSysEx(sysTimbre, p + 188, 58);// partial 4
			patchUpdateAddr(sysTimbre, 58);
			p += 246;
			cl += 2;
			sysTimbre[1] = cl;
			sysTimbre[2] = 0;
		}
		// patch memory or rhytm
		uint16 flag = READ_BE_UINT16(p);
		p += 2;
		if (flag == 0xABCD) {
			// sub_657
			for (int i = 0; i < 48; i++) {
				patchSysEx(sysMem, p, 8);
				patchUpdateAddr(sysMem, 8);
				p += 8;
			}
		} else if (flag == 0xDCBA) {
			// sub_756
			for (int i = 0; i < 64; i++) {
				patchSysEx(sysRhytm, p, 4);
				patchUpdateAddr(sysRhytm, 4);
				p += 4;
			}
			patchSysEx(sysSystem, p, 18);
		}
		// after-init text message
		//patchSysEx(sysText, pData, 20);
		// some final sysex
		_pMidiDrv->sysEx(sysMsg3, 15);
		// releasing patch resource
		//g_sci->ResMgr.ResUnload(SCI_RES_PATCH, 1);
	debug("MT-32 patch loaded");
	}
}
#endif

//----------------------------------------
void SciMusic::soundInitSnd(MusicEntry *pSnd) {
	SoundResource::Track *track = NULL;
	int channelFilterMask = 0;

	switch (_midiType) {
	case MD_PCSPK:
		track = pSnd->soundRes->getTrackByType(SoundResource::TRACKTYPE_SPEAKER);
		break;
	case MD_PCJR:
		track = pSnd->soundRes->getTrackByType(SoundResource::TRACKTYPE_TANDY);
		break;
	case MD_ADLIB:
		track = pSnd->soundRes->getTrackByType(SoundResource::TRACKTYPE_ADLIB);
		break;
	case MD_MT32:
		track = pSnd->soundRes->getTrackByType(SoundResource::TRACKTYPE_MT32);
		break;
	default:
		break;
	}

	// attempting to select default MT-32/Roland track
	if (!track)
		track = pSnd->soundRes->getTrackByType(SoundResource::TRACKTYPE_MT32);
	if (track) {
		// if MIDI device is selected but there is no digital track in sound resource
		// try to use adlib's digital sample if possible
		if (_midiType <= MD_MT32 && track->digitalChannelNr == -1 && _bMultiMidi) {
			if (pSnd->soundRes->getTrackByType(SoundResource::TRACKTYPE_ADLIB)->digitalChannelNr != -1)
				track = pSnd->soundRes->getTrackByType(SoundResource::TRACKTYPE_ADLIB);
		}
		// play digital sample
		if (track->digitalChannelNr != -1) {
			byte *channelData = track->channels[track->digitalChannelNr].data;
			if (pSnd->pStreamAud)
				delete pSnd->pStreamAud;
			pSnd->pStreamAud = Audio::makeLinearInputStream(channelData, track->digitalSampleSize, track->digitalSampleRate,
					Audio::Mixer::FLAG_UNSIGNED, 0, 0);
			pSnd->hCurrentAud = Audio::SoundHandle();
		} else {
			// play MIDI track
			if (pSnd->pMidiParser == NULL) {
				pSnd->pMidiParser = new MidiParser_SCI();
				pSnd->pMidiParser->setMidiDriver(_pMidiDrv);
				pSnd->pMidiParser->setTimerRate(_dwTempo);
			}
			// Find out what channels to filter for SCI0
			channelFilterMask = pSnd->soundRes->getChannelFilterMask(_pMidiDrv->getPlayMask(_soundVersion));
			pSnd->pMidiParser->loadMusic(track, pSnd, channelFilterMask, _soundVersion);
		}
	}
}
//----------------------------------------
void SciMusic::onTimer() {
	_mutex.lock();

	uint sz = _playList.size();
	for (uint i = 0; i < sz; i++) {
		if (_playList[i]->status != kSndStatusPlaying)
			continue;
		if (_playList[i]->pMidiParser) {
			if (_playList[i]->fadeStep)
				doFade(_playList[i]);
			_playList[i]->pMidiParser->onTimer();
			_playList[i]->ticker = (uint16)_playList[i]->pMidiParser->getTick();
		} else if (_playList[i]->pStreamAud) {
			if (!_pMixer->isSoundHandleActive(_playList[i]->hCurrentAud)) {
				_playList[i]->ticker = 0xFFFF;
				_playList[i]->status = kSndStatusStopped;

				// Signal the engine scripts that the sound is done playing
				// FIXME: is there any other place this can be triggered properly?
				SegManager *segMan = ((SciEngine *)g_engine)->getEngineState()->_segMan;	// HACK
				PUT_SEL32V(segMan, _playList[i]->soundObj, signal, SIGNAL_OFFSET);
				if (_soundVersion <= SCI_VERSION_0_LATE)
					PUT_SEL32V(segMan, _playList[i]->soundObj, state, kSndStatusStopped);
			} else {
				_playList[i]->ticker = (uint16)(_pMixer->getSoundElapsedTime(
						_playList[i]->hCurrentAud) * 0.06);
			}
		}
	}//for()
	
	_mutex.unlock();
}
//---------------------------------------------
void SciMusic::doFade(MusicEntry *pSnd) {
	// This is called from inside onTimer, where the mutex is already locked

	if (pSnd->fadeTicker)
		pSnd->fadeTicker--;
	else {
		pSnd->fadeTicker = pSnd->fadeTickerStep;
		if (pSnd->volume + pSnd->fadeStep > pSnd->fadeTo) {
			pSnd->volume = pSnd->fadeTo;
			pSnd->fadeStep = 0;
		} else {
			pSnd->volume += pSnd->fadeStep;
		}

		pSnd->pMidiParser->setVolume(pSnd->volume);

		if (pSnd->fadeStep == 0) {
			// Signal the engine scripts that the sound is done fading
			// FIXME: is this correct?
			//SegManager *segMan = ((SciEngine *)g_engine)->getEngineState()->_segMan;	// HACK
			//PUT_SEL32V(segMan, pSnd->soundObj, signal, SIGNAL_OFFSET);
		}
	}
}

//---------------------------------------------
void SciMusic::soundPlay(MusicEntry *pSnd) {

	uint sz = _playList.size(), i;
	// searching if sound is already in _playList
	for (i = 0; i < sz && _playList[i] != pSnd; i++)
		;
	if (i == sz) {// not found
		_mutex.lock();
		_playList.push_back(pSnd);
		sortPlayList();
		_mutex.unlock();
	}

	if (pSnd->pStreamAud && !_pMixer->isSoundHandleActive(pSnd->hCurrentAud)) {
		_pMixer->playInputStream(Audio::Mixer::kSFXSoundType, &pSnd->hCurrentAud,
				pSnd->pStreamAud, -1, pSnd->volume, 0, false);
	} else if (pSnd->pMidiParser) {
		_mutex.lock();
		pSnd->pMidiParser->setVolume(pSnd->volume);
		if (pSnd->status == kSndStatusStopped)
			pSnd->pMidiParser->jumpToTick(0);
		_mutex.unlock();
	}

	pSnd->status = kSndStatusPlaying;
}
//---------------------------------------------
void SciMusic::soundStop(MusicEntry *pSnd) {
	pSnd->status = kSndStatusStopped;
	if (pSnd->pStreamAud)
		_pMixer->stopHandle(pSnd->hCurrentAud);
	if (pSnd->pMidiParser)
		pSnd->pMidiParser->stop();
}
//---------------------------------------------
void SciMusic::soundSetVolume(MusicEntry *pSnd, byte volume) {
	if (pSnd->pStreamAud)
		_pMixer->setChannelVolume(pSnd->hCurrentAud, volume);
	else if (pSnd->pMidiParser)
		pSnd->pMidiParser->setVolume(volume);
}
//---------------------------------------------
void SciMusic::soundSetPriority(MusicEntry *pSnd, byte prio) {
	_mutex.lock();

	pSnd->prio = prio;
	sortPlayList();

	_mutex.unlock();
}
//---------------------------------------------
void SciMusic::soundKill(MusicEntry *pSnd) {

	pSnd->status = kSndStatusStopped;

	if (pSnd->pMidiParser) {
		pSnd->pMidiParser->unloadMusic();
		delete pSnd->pMidiParser;
		pSnd->pMidiParser = NULL;
	}
	if (pSnd->pStreamAud) {
		_pMixer->stopHandle(pSnd->hCurrentAud);
		pSnd->pStreamAud = NULL;
	}

	_mutex.lock();

	uint sz = _playList.size(), i;
	// Remove sound from playlist
	for (i = 0; i < sz; i++) {
		if (_playList[i] == pSnd) {
			delete _playList[i]->soundRes;
			delete _playList[i];
			_playList.remove_at(i);
			break;
		}
	}

	_mutex.unlock();
}
//---------------------------------------------
void SciMusic::soundPause(MusicEntry *pSnd) {
	pSnd->status = kSndStatusPaused;
	if (pSnd->pStreamAud)
		_pMixer->pauseHandle(pSnd->hCurrentAud, true);
	else if (pSnd->pMidiParser)
		pSnd->pMidiParser->pause();
}

//---------------------------------------------
uint16 SciMusic::soundGetMasterVolume() {
	return _pMixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) * 0xF
			/ Audio::Mixer::kMaxMixerVolume;
}
//---------------------------------------------
void SciMusic::soundSetMasterVolume(uint16 vol) {
	vol = vol & 0xF; // 0..15
	vol = vol * Audio::Mixer::kMaxMixerVolume / 0xF;
	_pMixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, vol);
	_pMixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, vol);
	_pMixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, vol);
	_pMixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, vol);
}

void SciMusic::reconstructSounds(int savegame_version) {
	_mutex.lock();

	SegManager *segMan = ((SciEngine *)g_engine)->getEngineState()->_segMan;	// HACK
	ResourceManager *resMan = ((SciEngine *)g_engine)->getEngineState()->resMan;	// HACK

	for (uint32 i = 0; i < _playList.size(); i++) {
		if (savegame_version < 14) {
			if (_soundVersion >= SCI_VERSION_1_EARLY) {
				_playList[i]->dataInc = GET_SEL32V(segMan, _playList[i]->soundObj, dataInc);
				_playList[i]->volume = GET_SEL32V(segMan, _playList[i]->soundObj, vol);
			} else {
				_playList[i]->volume = 100;
			}
		}

		_playList[i]->soundRes = new SoundResource(_playList[i]->resnum, resMan, _soundVersion);
		soundInitSnd(_playList[i]);
	}

	_mutex.unlock();
}

} // end of namespace SCI
