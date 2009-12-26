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
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/resource.h"
#include "sci/sfx/music.h"
#include "sci/sfx/softseq/pcjr.h"

namespace Sci {

static const int nMidiParams[] = { 2, 2, 2, 2, 1, 1, 2, 0 };

static int f_compare(const void *arg1, const void *arg2) {
	return ((const MusicEntry *)arg2)->prio - ((const MusicEntry *)arg1)->prio;
}

SciMusic::SciMusic(SciVersion soundVersion)
	: _soundVersion(soundVersion) {
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
bool SciMusic::restoreState(Common::InSaveFile *pFile){
	if (pFile->readLine() != "AUDIO")
		return false;

	return true;
}
//----------------------------------------
void SciMusic::stopAll() {
	_mutex.lock();
	_pMixer->stopAll();

	for (uint i = 0; i < _playList.size(); i++){
		soundStop(_playList[i]);
		soundKill(_playList[i]);
	}

	_mutex.unlock();
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
	_mutex.lock();
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
		if (_midiType <= MD_MT32 && track->nDigital == 0xFF && _bMultiMidi) {
			if (pSnd->soundRes->getTrackByType(SoundResource::TRACKTYPE_ADLIB)->nDigital != 0xFF)
				track = pSnd->soundRes->getTrackByType(SoundResource::TRACKTYPE_ADLIB);
		}
		// play digital sample
		if (track->nDigital != 0xFF) {
			byte *channelData = track->channels[track->nDigital].data;
			int rate = READ_LE_UINT16(channelData);
			uint32 size = READ_LE_UINT16(channelData + 2);
			assert(READ_LE_UINT16(channelData + 4) == 0);	// Possibly a compression flag
			//assert(READ_LE_UINT16(channelData + 6) == size);
			if (pSnd->pStreamAud)
				delete pSnd->pStreamAud;
			pSnd->pStreamAud = Audio::makeLinearInputStream(channelData + 8, size, rate,
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

	_mutex.unlock();
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
				SegManager *segMan = ((SciEngine *)g_engine)->getEngineState()->_segMan;
				PUT_SEL32V(segMan, _playList[i]->soundObj, signal, SIGNAL_OFFSET);
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
	_mutex.lock();

	if (pSnd->fadeTicker)
		pSnd->fadeTicker--;
	else {
		pSnd->fadeTicker = pSnd->fadeTickerStep;
		if (pSnd->volume + pSnd->fadeStep > pSnd->fadeTo) {
			pSnd->volume = pSnd->fadeTo;
			pSnd->fadeStep = 0;
		} else
			pSnd->volume += pSnd->fadeStep;
		pSnd->pMidiParser->setVolume(pSnd->volume);
	}

	_mutex.unlock();
}

//---------------------------------------------
void SciMusic::soundPlay(MusicEntry *pSnd) {
	_mutex.lock();

	uint sz = _playList.size(), i;
	// searching if sound is already in _playList
	for (i = 0; i < sz && _playList[i] != pSnd; i++)
		;
	if (i == sz) {// not found
		_playList.push_back(pSnd);
		sortPlayList();
	}

	if (pSnd->pStreamAud && !_pMixer->isSoundHandleActive(pSnd->hCurrentAud)) {
		_pMixer->playInputStream(Audio::Mixer::kSFXSoundType, &pSnd->hCurrentAud,
				pSnd->pStreamAud, -1, pSnd->volume, 0, false);
	} else if (pSnd->pMidiParser) {
		pSnd->pMidiParser->setVolume(pSnd->volume);
		if (pSnd->status == kSndStatusStopped)
			pSnd->pMidiParser->jumpToTick(0);
	}
	pSnd->status = kSndStatusPlaying;

	_mutex.unlock();
}
//---------------------------------------------
void SciMusic::soundStop(MusicEntry *pSnd) {
	_mutex.lock();

	pSnd->status = kSndStatusStopped;
	if (pSnd->pStreamAud)
		_pMixer->stopHandle(pSnd->hCurrentAud);
	if (pSnd->pMidiParser)
		pSnd->pMidiParser->stop();

	_mutex.unlock();
}
//---------------------------------------------
void SciMusic::soundSetVolume(MusicEntry *pSnd, byte volume) {
	_mutex.lock();

	if (pSnd->pStreamAud)
		_pMixer->setChannelVolume(pSnd->hCurrentAud, volume);
	else if (pSnd->pMidiParser)
		pSnd->pMidiParser->setVolume(volume);

	_mutex.unlock();
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
	_mutex.lock();

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

	uint sz = _playList.size(), i;
	// Remove sound from playlist
	for (i = 0; i < sz; i++) {
		if (_playList[i] == pSnd) {
			delete _playList[i]->soundRes;
			_playList[i]->soundRes = 0;
			_playList.remove_at(i);
			break;
		}
	}

	_mutex.unlock();
}
//---------------------------------------------
void SciMusic::soundPause(MusicEntry *pSnd) {
	_mutex.lock();

	pSnd->status = kSndStatusPaused;
	if (pSnd->pStreamAud)
		_pMixer->pauseHandle(pSnd->hCurrentAud, true);
	else if (pSnd->pMidiParser)
		pSnd->pMidiParser->pause();

	_mutex.unlock();
}

//---------------------------------------------
uint16 SciMusic::soundGetMasterVolume() {
	return _pMixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) * 0xF
			/ Audio::Mixer::kMaxMixerVolume;
}
//---------------------------------------------
void SciMusic::soundSetMasterVolume(uint16 vol) {
	_mutex.lock();

	vol = vol & 0xF; // 0..15
	vol = vol * Audio::Mixer::kMaxMixerVolume / 0xF;
	_pMixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, vol);
	_pMixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, vol);
	_pMixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, vol);
	_pMixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, vol);

	_mutex.unlock();
}

//---------------------------------------------
//  MidiParser_SCI
//
MidiParser_SCI::MidiParser_SCI() :
	MidiParser() {
	_mixedData = NULL;
	// mididata contains delta in 1/60th second
	// values of ppqn and tempo are found experimentally and may be wrong
	_ppqn = 1;
	setTempo(16667);

	_signalSet = false;
	_signalToSet = 0;
}
//---------------------------------------------
MidiParser_SCI::~MidiParser_SCI() {
	unloadMusic();
}
//---------------------------------------------
bool MidiParser_SCI::loadMusic(SoundResource::Track *track, MusicEntry *psnd, int channelFilterMask, SciVersion soundVersion) {
	unloadMusic();
	_track = track;
	_pSnd = psnd;
	_soundVersion = soundVersion;

	setVolume(psnd->volume);

	if (channelFilterMask) {
		// SCI0 only has 1 data stream, but we need to filter out channels depending on music hardware selection
		midiFilterChannels(channelFilterMask);
	} else {
		midiMixChannels();
	}

	_num_tracks = 1;
	_tracks[0] = _mixedData;
	setTrack(0);
	_loopTick = 0;
	return true;
}

void MidiParser_SCI::unloadMusic() {
	allNotesOff();
	resetTracking();
	_num_tracks = 0;
	if (_mixedData) {
		delete[] _mixedData;
		_mixedData = NULL;
	}
}

void MidiParser_SCI::parseNextEvent(EventInfo &info) {
	SegManager *segMan = ((SciEngine *)g_engine)->getEngineState()->_segMan;	// HACK

	// Set signal AFTER waiting for delta, otherwise we would set signal too soon resulting in all sorts of bugs
	if (_signalSet) {
		_signalSet = false;
		PUT_SEL32V(segMan, _pSnd->soundObj, signal, _signalToSet);
		warning("signal %04x", _signalToSet);
	}

	info.start = _position._play_pos;
	info.delta = 0;
	while (*_position._play_pos == 0xF8) {
		info.delta += 240;
		_position._play_pos++;
	}
	info.delta += *(_position._play_pos++);

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
			if (info.basic.param1 != 0x7F) {
				_signalSet = true;
				_signalToSet = info.basic.param1;
			} else {
				_loopTick = _position._play_tick;
			}
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
			if (info.basic.param1 == 0x60) {
				switch (_soundVersion) {
				case SCI_VERSION_0_EARLY:
				case SCI_VERSION_0_LATE:
					_pSnd->dataInc += info.basic.param2;
					_signalSet = true;
					_signalToSet = 0x7f + _pSnd->dataInc;
					break;
				case SCI_VERSION_1_EARLY:
				case SCI_VERSION_1_LATE:
					_pSnd->dataInc++;
					break;
				default:
					break;
				}
			}
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
					_pSnd->status = kSndStatusStopped;
					PUT_SEL32V(segMan, _pSnd->soundObj, signal, 0xFFFF);
					warning("signal EOT");
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

	for (int i = 0; i < _track->channelCount; i++) {
		if (_track->channels[i].time == -1) // channel ended
			continue;
		next = *_track->channels[i].data; // when the next event shoudl occur
		if (next == 0xF8) // 0xF8 means 240 ticks delay
			next = 240;
		next += _track->channels[i].time;
		if (next < closest) {
			curr = i;
			closest = next;
		}
	}

	return curr;
}
//----------------------------------------
byte *MidiParser_SCI::midiMixChannels() {
	int totalSize = 0;
	byte **dataPtr = new byte *[_track->channelCount];

	for (int i = 0; i < _track->channelCount; i++) {
		dataPtr[i] = _track->channels[i].data;
		_track->channels[i].time = 0;
		_track->channels[i].prev = 0;
		totalSize += _track->channels[i].size;
	}

	byte *mixedData = new byte[totalSize * 2]; // FIXME: creates overhead and still may be not enough to hold all data
	_mixedData = mixedData;
	long ticker = 0;
	byte curr, delta;
	byte cmd, par1, global_prev = 0;
	long new_delta;
	SoundResource::Channel *channel;
	while ((curr = midiGetNextChannel(ticker)) != 0xFF) { // there is still active channel
		channel = &_track->channels[curr];
		delta = *channel->data++;
		channel->time += (delta == 0xF8 ? 240 : delta); // when the comamnd is supposed to occur
		if (delta == 0xF8)
			continue;
		new_delta = channel->time - ticker;
		ticker += new_delta;

		cmd = *channel->data++;
		if (cmd != 0xFC) {
			// output new delta
			while (new_delta > 240) {
				*mixedData++ = 0xF8;
				new_delta -= 240;
			}
			*mixedData++ = (byte)new_delta;
		}
		switch (cmd) {
		case 0xF0: // sysEx
			*mixedData++ = cmd;
			do {
				par1 = *channel->data++;
				*mixedData++ = par1; // out
			} while (par1 != 0xF7);
			break;
		case 0xFC: // end channel
			channel->time = -1; // FIXME
			break;
		default: // MIDI command
			if (cmd & 0x80)
				par1 = *channel->data++;
			else {// running status
				par1 = cmd;
				cmd = channel->prev;
			}
			if (cmd != global_prev)
				*mixedData++ = cmd; // out cmd
			*mixedData++ = par1;// pout par1
			if (nMidiParams[(cmd >> 4) - 8] == 2)
				*mixedData++ = *channel->data++; // out par2
			channel->prev = cmd;
			global_prev = cmd;
		}// switch(cmd)
	}// while (curr)
	// mixing finished. inserting stop event
	*mixedData++ = 0;
	*mixedData++ = 0xFF;
	*mixedData++ = 0x2F;
	*mixedData++ = 0x00;
	*mixedData++ = 0x00;

	for (int channelNr = 0; channelNr < _track->channelCount; channelNr++)
		_track->channels[channelNr].data = dataPtr[channelNr];

	delete[] dataPtr;
	return _mixedData;
}

// This is used for SCI0 sound-data. SCI0 only has one stream that may contain several channels and according to output
//  device we remove certain channels from that data
byte *MidiParser_SCI::midiFilterChannels(int channelMask) {
	SoundResource::Channel *channel = &_track->channels[0];
	byte *channelData = channel->data;
	byte *channelDataEnd = channel->data + channel->size;
	byte *filterData = new byte[channel->size + 5];
	byte curChannel, curByte, curDelta;
	byte command, lastCommand;
	int delta = 0;
	//int dataLeft = channel->size;
	int midiParamCount;

	_mixedData = filterData;
	command = 0;
	midiParamCount = 0;
	lastCommand = 0;
	curChannel = 15;

	while (channelData < channelDataEnd) {
		curDelta = *channelData++;
		if (curDelta == 0xF8) {
			delta += 240;
			continue;
		}
		delta += curDelta;
		curByte = *channelData++;

		switch (curByte) {
		case 0xF0: // sysEx
		case 0xFC: // end of channel
			command = curByte;
			curChannel = 15;
			break;
		default:
			if (curByte & 0x80) {
				command = curByte;
				curChannel = command & 0x0F;
				midiParamCount = nMidiParams[(command >> 4) - 8];
			}
		}
		if ((1 << curChannel) & channelMask) {
			if (command != 0xFC) {
				debugC(2, kDebugLevelSound, "\nDELTA ");
				// Write delta
				while (delta > 240) {
					*filterData++ = 0xF8;
					debugC(2, kDebugLevelSound, "F8 ");
					delta -= 240;
				}
				*filterData++ = (byte)delta;
				debugC(2, kDebugLevelSound, "%02X ", delta);
				delta = 0;
			}
			// Write command
			switch (command) {
			case 0xF0: // sysEx
				*filterData++ = command;
				debugC(2, kDebugLevelSound, "%02X ", command);
				do {
					curByte = *channelData++;
					*filterData++ = curByte; // out
				} while (curByte != 0xF7);
				lastCommand = command;
				break;

			case 0xFC: // end of channel
				break;

			default: // MIDI command
				if (lastCommand != command) {
					*filterData++ = command;
					debugC(2, kDebugLevelSound, "%02X ", command);
					lastCommand = command;
				}
				if (midiParamCount > 0) {
					if (curByte & 0x80) {
						debugC(2, kDebugLevelSound, "%02X ", *channelData);
						*filterData++ = *channelData++;
					} else {
						debugC(2, kDebugLevelSound, "%02X ", curByte);
						*filterData++ = curByte;
					}
				}
				if (midiParamCount > 1) {
					debugC(2, kDebugLevelSound, "%02X ", *channelData);
					*filterData++ = *channelData++;
				}
			}
		} else {
			if (curByte & 0x80) {
				channelData += midiParamCount;
			} else {
				channelData += midiParamCount - 1;
			}
		}
	}
	// Stop event
	*filterData++ = 0;    // delta
	*filterData++ = 0xFF; // Meta-Event
	*filterData++ = 0x2F; // End-Of-Track
	*filterData++ = 0x00;
	*filterData++ = 0x00;

	return _mixedData;
}

void MidiParser_SCI::setVolume(byte bVolume) {
	if (bVolume > 0x7F)
		bVolume = 0x7F;
	if (_volume != bVolume) {
		_volume = bVolume;

		// sending volume change to all active channels
		for (int i = 0; i < _track->channelCount; i++)
			if (_track->channels[i].number <= 0xF)
				_driver->send(0xB0 + _track->channels[i].number, 7, _volume);
	}
}
} // end of namespace SCI
