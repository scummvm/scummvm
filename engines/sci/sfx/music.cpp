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

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/sfx/music.h"
#include "sound/audiostream.h"
#include "common/config-manager.h"

namespace Sci {

static const int nMidiParams[] = { 2, 2, 2, 2, 1, 1, 2, 0 };

static int f_compare(const void *arg1, const void *arg2) {
	return ((sciSound *)arg2)->prio - ((sciSound *)arg1)->prio;
}

SciMusic::SciMusic() {
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
	_midiType = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PCSPK
			| MDT_PREFER_MIDI);
	_pMidiDrv = MidiDriver::createMidi(_midiType);
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
	// palylist
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
bool SciMusic::restoreState(Common::InSaveFile *pFile){
	// TODO
#if 0
	if (pFile->readLine() != "AUDIO")
		return false;
#endif
	return true;
}
//----------------------------------------
void SciMusic::stopAll() {
	_pMixer->stopAll();
	//audioStop();
	for(uint i = 0; i < _playList.size(); i++){
		soundStop(_playList[i]);
		soundKill(_playList[i]);
	}
}
//----------------------------------------
void SciMusic::miditimerCallback(void *p) {
	SciMusic* aud = (SciMusic *)p;
	aud->onTimer();
}
//----------------------------------------
uint16 SciMusic::soundGetVoices() {
	uint16 res;
	switch (_midiType) {
	case MD_PCSPK:
		res = 1;
		break;
	case MD_PCJR:
		res = 3;
		break;
	case MD_ADLIB:
		res = 8;
		break;
	case MD_MT32:
		res = 16;
		break;
	default:
		res = 1;
		break;
	}
	return res;
}
//----------------------------------------
void SciMusic::sortPlayList() {
	sciSound ** pData = _playList.begin();
	qsort(pData, _playList.size(), sizeof(sciSound *), &f_compare);
}

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
void SciMusic::loadPatch() {
	if (_midiType == MD_MT32)
		loadPatchMT32();
}

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
//----------------------------------------
void SciMusic::soundInitSnd(SoundRes*res, sciSound *pSnd) {
	//_mutex.lock();
	SoundRes::tagTrack *pTrack = NULL;
	switch (_midiType) {
	case MD_PCSPK:
		pTrack = res->getTrackByType(SoundRes::kTrackSpeaker);
		break;
	case MD_PCJR:
		pTrack = res->getTrackByType(SoundRes::kTrackTandy);
		break;
	case MD_ADLIB:
		pTrack = res->getTrackByType(SoundRes::kTrackAdlib);
		break;
	case MD_MT32:
		pTrack = res->getTrackByType(SoundRes::kTrackMT32);
		break;
	}
	// attempting to select default MT-32/Roland track
	if (!pTrack)
		pTrack = res->getTrackByType(SoundRes::kTrackMT32);
	if (pTrack) {
		// if MIDI device is selected but there is no digital track in sound resource
		// try to use adlib's digital sample if possible
		if (_midiType <= MD_MT32 && pTrack->nDigital == 0xFF && _bMultiMidi) {
			if (res->getTrackByType(SoundRes::kTrackAdlib)->nDigital != 0xFF)
				pTrack = res->getTrackByType(SoundRes::kTrackAdlib);
		}
		// play digital sample
		if (pTrack->nDigital != 0xFF) {
			byte *pdata = pTrack->aChannels[pTrack->nDigital].ptr;
			int rate = READ_LE_UINT16(pdata);
			uint32 size = READ_LE_UINT16(pdata + 2);
			assert(READ_LE_UINT16(pdata + 4) == 0);	// Possibly a compression flag
			//assert(READ_LE_UINT16(pdata + 6) == size);
			if (pSnd->pStreamAud)
				delete pSnd->pStreamAud;
			pSnd->pStreamAud = Audio::makeLinearInputStream(pdata + 8, size, rate,
					Audio::Mixer::FLAG_UNSIGNED, 0, 0);
			pSnd->hCurrentAud = Audio::SoundHandle();
		} else {// play MIDI track
			if (pSnd->pMidiParser == NULL) {
				pSnd->pMidiParser = new MidiParser_SCI();
				pSnd->pMidiParser->setMidiDriver(_pMidiDrv);
				pSnd->pMidiParser->setTimerRate(_dwTempo);
			}
			pSnd->pMidiParser->loadMusic(pTrack, pSnd);
		}
	}
	//_mutex.unlock();
}
//----------------------------------------
void SciMusic::onTimer() {
	_mutex.lock();
	uint sz = _playList.size();
	for (uint i = 0; i < sz; i++) {
		if (_playList[i]->sndStatus != kPlaying)
			continue;
		if (_playList[i]->pMidiParser) {
			if (_playList[i]->FadeStep)
				doFade(_playList[i]);
			_playList[i]->pMidiParser->onTimer();
			_playList[i]->ticker = (uint16)_playList[i]->pMidiParser->getTick();
		} else if (_playList[i]->pStreamAud) {
			if (_pMixer->isSoundHandleActive(_playList[i]->hCurrentAud)
					== false) {
				_playList[i]->ticker = 0xFFFF;
				_playList[i]->signal = 0xFFFF;
				_playList[i]->sndStatus = kStopped;
			} else
				_playList[i]->ticker = (uint16)(_pMixer->getSoundElapsedTime(
						_playList[i]->hCurrentAud) * 0.06);
		}
	}//for()
	_mutex.unlock();
}
//---------------------------------------------
void SciMusic::doFade(sciSound *pSnd) {
	if (pSnd->FadeTicker)
		pSnd->FadeTicker--;
	else {
		pSnd->FadeTicker = pSnd->FadeTickerStep;
		if (pSnd->volume + pSnd->FadeStep > pSnd->FadeTo) {
			pSnd->volume = pSnd->FadeTo;
			pSnd->FadeStep = 0;
		} else
			pSnd->volume += pSnd->FadeStep;
		pSnd->pMidiParser->setVolume(pSnd->volume);
	}
}

//---------------------------------------------
void SciMusic::soundPlay(sciSound *pSnd) {
	//_mutex.lock();
	uint sz = _playList.size(), i;
	// searching if sound is already in _playList
	for (i = 0; i < sz && _playList[i] != pSnd; i++)
		;
	if (i == sz) {// not found
		_playList.push_back(pSnd);
		sortPlayList();
	}

	if (pSnd->pStreamAud && _pMixer->isSoundHandleActive(pSnd->hCurrentAud)
			== false)
		_pMixer->playInputStream(Audio::Mixer::kSFXSoundType, &pSnd->hCurrentAud,
				pSnd->pStreamAud, -1, pSnd->volume, 0, false);
	else if (pSnd->pMidiParser) {
		pSnd->pMidiParser->setVolume(pSnd->volume);
		if (pSnd->sndStatus == kStopped)
			pSnd->pMidiParser->jumpToTick(0);
	}
	pSnd->sndStatus = kPlaying;
	//_mutex.unlock();
}
//---------------------------------------------
void SciMusic::soundStop(sciSound *pSnd) {
	//_mutex.lock();
	pSnd->sndStatus = kStopped;
	if (pSnd->pStreamAud)
		_pMixer->stopHandle(pSnd->hCurrentAud);
	if (pSnd->pMidiParser)
		pSnd->pMidiParser->stop();
	//_mutex.unlock();
}
//---------------------------------------------
void SciMusic::soundSetVolume(sciSound *pSnd, byte volume) {
	if (pSnd->pStreamAud)
		_pMixer->setChannelVolume(pSnd->hCurrentAud, volume);
	else if (pSnd->pMidiParser)
		pSnd->pMidiParser->setVolume(volume);
}
//---------------------------------------------
void SciMusic::soundSetPriority(sciSound *pSnd, byte prio) {
	pSnd->prio = prio;
	sortPlayList();
}
//---------------------------------------------
void SciMusic::soundKill(sciSound *pSnd) {
	//_mutex.lock();
	pSnd->sndStatus = kStopped;
	if (pSnd->pMidiParser) {
		pSnd->pMidiParser->unloadMusic();
		delete pSnd->pMidiParser;
		pSnd->pMidiParser = NULL;
	}
	if (pSnd->pStreamAud) {
		_pMixer->stopHandle(pSnd->hCurrentAud);
		pSnd->pStreamAud = NULL;
	}

	uint sz = _playList.size(), i;
	// searching if sound is already in _playList
	for (i = 0; i < sz; i++) {
		if (_playList[i] == pSnd) {
			_playList.remove_at(i);
			break;
		}
	}
	//_mutex.unlock();
}
//---------------------------------------------
void SciMusic::soundPause(sciSound *pSnd) {
	pSnd->sndStatus = kPaused;
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
	// TODO:balance volume to prevent music to be too loud
	_pMixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, vol);
	_pMixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, vol);
	_pMixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, vol);
	_pMixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, vol);
}

//---------------------------------------------
//  MidiParser_SCI
//
MidiParser_SCI::MidiParser_SCI() :
	MidiParser() {
	_pMidiData = NULL;
	// mididata contains delta in 1/60th second
	// values of ppqn and tempo are found experimentally and may be wrong
	_ppqn = 1;
	setTempo(16667);
}
//---------------------------------------------
MidiParser_SCI::~MidiParser_SCI() {
	unloadMusic();
}
//---------------------------------------------
bool MidiParser_SCI::loadMusic(SoundRes::tagTrack *ptrack, sciSound *psnd) {
	unloadMusic();
	_pTrack = ptrack;
	_pSnd = psnd;
	setVolume(psnd->volume);
	midiMixChannels();

	_num_tracks = 1;
	_tracks[0] = _pMidiData;
	setTrack(0);
	_loopTick = 0;
	return true;
}

void MidiParser_SCI::unloadMusic() {
	allNotesOff();
	resetTracking();
	_num_tracks = 0;
	if (_pMidiData) {
		delete[] _pMidiData;
		_pMidiData = NULL;
	}
}

void MidiParser_SCI::parseNextEvent(EventInfo &info) {
	info.start = _position._play_pos;
	info.delta = *(_position._play_pos++);

	// Process the next info.
	if ((_position._play_pos[0] & 0xF0) >= 0x80)
		info.event = *(_position._play_pos++);
	else
		info.event = _position._running_status;
	if (info.event < 0x80)
		return;

	_position._running_status = info.event;
	switch (info.command()) {
	case 0xC:
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = 0;
		if (info.channel() == 0xF) {// SCI special case
			if (info.basic.param1 != 0x7F)
				_pSnd->signal = info.basic.param1;
			else
				_loopTick = _position._play_tick;
		}
		break;
	case 0xD:
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = 0;
		break;

	case 0xB:
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = *(_position._play_pos++);
		if (info.channel() == 0xF) {// SCI special
			if (info.basic.param1 == 0x60)
				_pSnd->dataInc++;
			// BF 50 x - set reverb to x
			// BF 60 x - dataInc++
			// BF 52 x - bHold=x
		}
		if (info.basic.param1 == 7) // channel volume change -scale it
			info.basic.param2 = info.basic.param2 * _volume / 0x7F;
		info.length = 0;
		break;

	case 0x8:
	case 0x9:
	case 0xA:
	case 0xE:
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = *(_position._play_pos++);
		if (info.command() == 0x9 && info.basic.param2 == 0)
			info.event = info.channel() | 0x80;
		info.length = 0;
		break;

	case 0xF: // System Common, Meta or SysEx event
		switch (info.event & 0x0F) {
		case 0x2: // Song Position Pointer
			info.basic.param1 = *(_position._play_pos++);
			info.basic.param2 = *(_position._play_pos++);
			break;

		case 0x3: // Song Select
			info.basic.param1 = *(_position._play_pos++);
			info.basic.param2 = 0;
			break;

		case 0x6:
		case 0x8:
		case 0xA:
		case 0xB:
		case 0xC:
		case 0xE:
			info.basic.param1 = info.basic.param2 = 0;
			break;

		case 0x0: // SysEx
			info.length = readVLQ(_position._play_pos);
			info.ext.data = _position._play_pos;
			_position._play_pos += info.length;
			break;

		case 0xF: // META event
			info.ext.type = *(_position._play_pos++);
			info.length = readVLQ(_position._play_pos);
			info.ext.data = _position._play_pos;
			_position._play_pos += info.length;
			if (info.ext.type == 0x2F) {// end of track reached
				if (_pSnd->loop) {
					jumpToTick(_loopTick);
					_pSnd->loop--;
				} else {
					_pSnd->sndStatus = kStopped;
					_pSnd->signal = 0xFFFF;
				}
			}
			break;
		default:
			warning(
					"MidiParser_SCI::parseNextEvent: Unsupported event code %x",
					info.event);
		} // // System Common, Meta or SysEx event
	}// switch (info.command())
}

//----------------------------------------
byte MidiParser_SCI::midiGetNextChannel(long ticker) {
	byte curr = 0xFF;
	long closest = ticker + 1000000, next = 0;
	for (int i = 0; i < _pTrack->nChannels; i++) {
		if (_pTrack->aChannels[i].time == -1) // channel ended
			continue;
		next = *_pTrack->aChannels[i].ptr; // when the next event shoudl occur
		if (next == 0xF8) // 0xF8 means 240 ticks delay
			next = 240;
		next += _pTrack->aChannels[i].time;
		if (next < closest) {
			curr = i;
			closest = next;
		}
	}
	return curr;
}
//----------------------------------------
byte *MidiParser_SCI::midiMixChannels() {
	int lSize = 0;
	byte **pDataPtr = new byte *[_pTrack->nChannels];
	for (int i = 0; i < _pTrack->nChannels; i++) {
		pDataPtr[i] = _pTrack->aChannels[i].ptr;
		_pTrack->aChannels[i].time = 0;
		_pTrack->aChannels[i].prev = 0;
		lSize += _pTrack->aChannels[i].size;
	}
	byte *pOutData = new byte[lSize * 2]; // FIXME: creates overhead and still may be not enough to hold all data
	_pMidiData = pOutData;
	long ticker = 0;
	byte curr, delta;
	byte cmd, par1, global_prev = 0;
	long new_delta;
	SoundRes::tagChannel *pCh;
	while ((curr = midiGetNextChannel(ticker)) != 0xFF) { // there is still active channel
		pCh = &_pTrack->aChannels[curr];
		delta = *pCh->ptr++;
		pCh->time += (delta == 0xF8 ? 240 : delta); // when the comamnd is supposed to occur
		if (delta == 0xF8)
			continue;
		new_delta = pCh->time - ticker;
		ticker += new_delta;

		cmd = *pCh->ptr++;
		if (cmd != 0xFC) {
			// output new delta
			while (new_delta > 240) {
				*pOutData++ = 0xF8;
				new_delta -= 240;
			}
			*pOutData++ = (byte)new_delta;
		}
		switch (cmd) {
		case 0xF0: // sysEx
			*pOutData++ = cmd;
			do {
				par1 = *pCh->ptr++;
				*pOutData++ = par1; // out
			} while (par1 != 0xF7);
			break;
		case 0xFC: // end channel
			pCh->time = -1; // FIXME
			break;
		default: // MIDI command
			if (cmd & 0x80)
				par1 = *pCh->ptr++;
			else {// running status
				par1 = cmd;
				cmd = pCh->prev;
			}
			if (cmd != global_prev)
				*pOutData++ = cmd; // out cmd
			*pOutData++ = par1;// pout par1
			if (nMidiParams[(cmd >> 4) - 8] == 2)
				*pOutData++ = *pCh->ptr++; // out par2
			pCh->prev = cmd;
			global_prev = cmd;
		}// switch(cmd)
	}// while (curr)
	// mixing finished. inserting stop event
	*pOutData++ = 0;
	*pOutData++ = 0xFF;
	*pOutData++ = 0x2F;
	*pOutData++ = 0x00;
	*pOutData++ = 0x00;

	for (int i = 0; i < _pTrack->nChannels; i++)
		_pTrack->aChannels[i].ptr = pDataPtr[i];
	delete[] pDataPtr;
	return _pMidiData;
}

void MidiParser_SCI::setVolume(byte bVolume) {
	if (bVolume > 0x7F)
		bVolume = 0x7F;
	if (_volume != bVolume) {
		_volume = bVolume;
		// sending volume change to all active channels
		for (int i = 0; i < _pTrack->nChannels; i++)
			if (_pTrack->aChannels[i].number <= 0xF)
				_driver->send(0xB0 + _pTrack->aChannels[i].number, 7, _volume);
	}
}

//---------------------------

SoundRes::tagTrack* SoundRes::getTrackByNumber(uint16 number) {
	if (/*number >= 0 &&*/number < nTracks)
		return &aTracks[number];
	return NULL;
}

SoundRes::tagTrack* SoundRes::getTrackByType(kTrackType type) {
	for (int i = 0; i < nTracks; i++)
		if (aTracks[i].type == type)
			return &aTracks[i];
	return NULL;
}

} // end of namespace SCI
