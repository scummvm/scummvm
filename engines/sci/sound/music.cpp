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
#include "sound/decoders/raw.h"
#include "common/config-manager.h"

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/resource.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/sound/midiparser_sci.h"
#include "sci/sound/music.h"

namespace Sci {

SciMusic::SciMusic(SciVersion soundVersion)
	: _soundVersion(soundVersion), _soundOn(true), _masterVolume(0) {

	// Reserve some space in the playlist, to avoid expensive insertion
	// operations
	_playList.reserve(10);

	for (int i = 0; i < 16; i++)
		_usedChannel[i] = 0;
}

SciMusic::~SciMusic() {
	if (_pMidiDrv) {
		_pMidiDrv->close();
		delete _pMidiDrv;
	}
}

void SciMusic::init() {
	// system init
	_pMixer = g_system->getMixer();
	// SCI sound init
	_dwTempo = 0;

	MidiDriverType midiType;

	// Default to MIDI in SCI32 games, as many don't have AdLib support.
	// WORKAROUND: Default to MIDI in Amiga SCI1_EGA+ games as we don't support those patches yet.
	// We also don't yet support the 7.pat file of SCI1+ Mac games or SCI0 Mac patches, so we
	// default to MIDI in those games to let them run.
	Common::Platform platform = g_sci->getPlatform();

	if (getSciVersion() >= SCI_VERSION_2 || platform == Common::kPlatformMacintosh || (platform == Common::kPlatformAmiga && getSciVersion() >= SCI_VERSION_1_EGA))
		midiType = MidiDriver::detectMusicDriver(MDT_PCSPK | MDT_ADLIB | MDT_MIDI | MDT_PREFER_MIDI);
	else
		midiType = MidiDriver::detectMusicDriver(MDT_PCSPK | MDT_ADLIB | MDT_MIDI);

	switch (midiType) {
	case MD_ADLIB:
		// FIXME: There's no Amiga sound option, so we hook it up to AdLib
		if (g_sci->getPlatform() == Common::kPlatformAmiga)
			_pMidiDrv = MidiPlayer_Amiga_create(_soundVersion);
		else
			_pMidiDrv = MidiPlayer_AdLib_create(_soundVersion);
		break;
	case MD_PCJR:
		_pMidiDrv = MidiPlayer_PCJr_create(_soundVersion);
		break;
	case MD_PCSPK:
		_pMidiDrv = MidiPlayer_PCSpeaker_create(_soundVersion);
		break;
	default:
		if (ConfMan.getBool("enable_fb01"))
			_pMidiDrv = MidiPlayer_Fb01_create(_soundVersion);
		else
			_pMidiDrv = MidiPlayer_Midi_create(_soundVersion);
	}

	if (_pMidiDrv && !_pMidiDrv->open()) {
		_pMidiDrv->setTimerCallback(this, &miditimerCallback);
		_dwTempo = _pMidiDrv->getBaseTempo();
	} else {
		error("Failed to initialize sound driver");
	}

	_bMultiMidi = ConfMan.getBool("multi_midi");
}

void SciMusic::clearPlayList() {
	Common::StackLock lock(_mutex);

	while (!_playList.empty()) {
		soundStop(_playList[0]);
		soundKill(_playList[0]);
	}
}

void SciMusic::pauseAll(bool pause) {
	const MusicList::iterator end = _playList.end();
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
		soundToggle(*i, pause);
	}
}

void SciMusic::stopAll() {
	Common::StackLock lock(_mutex);

	const MusicList::iterator end = _playList.end();
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
		soundStop(*i);
	}
}


void SciMusic::miditimerCallback(void *p) {
	SciMusic *aud = (SciMusic *)p;

	Common::StackLock lock(aud->_mutex);
	aud->onTimer();
}

void SciMusic::soundSetSoundOn(bool soundOnFlag) {
	Common::StackLock lock(_mutex);

	_soundOn = soundOnFlag;
	_pMidiDrv->playSwitch(soundOnFlag);
}

uint16 SciMusic::soundGetVoices() {
	Common::StackLock lock(_mutex);

	return _pMidiDrv->getPolyphony();
}

MusicEntry *SciMusic::getSlot(reg_t obj) {
	Common::StackLock lock(_mutex);

	const MusicList::iterator end = _playList.end();
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
		if ((*i)->soundObj == obj)
			return *i;
	}

	return NULL;
}

void SciMusic::setReverb(byte reverb) {
	Common::StackLock lock(_mutex);
	_pMidiDrv->setReverb(reverb);
}

static bool musicEntryCompare(const MusicEntry *l, const MusicEntry *r) {
	return (l->priority > r->priority);
}

void SciMusic::sortPlayList() {
	// Sort the play list in descending priority order
	Common::sort(_playList.begin(), _playList.end(), musicEntryCompare);
}

void SciMusic::soundInitSnd(MusicEntry *pSnd) {
	int channelFilterMask = 0;
	SoundResource::Track *track = pSnd->soundRes->getTrackByType(_pMidiDrv->getPlayId());

	// If MIDI device is selected but there is no digital track in sound resource
	// try to use adlib's digital sample if possible
	// Also, if the track couldn't be found, load the digital track, as some games
	// depend on this (e.g. the Longbow demo)
	if (!track || (_bMultiMidi && track->digitalChannelNr == -1)) {
		SoundResource::Track *digital = pSnd->soundRes->getDigitalTrack();
		if (digital)
			track = digital;
	}

	if (track) {
		// Play digital sample
		if (track->digitalChannelNr != -1) {
			byte *channelData = track->channels[track->digitalChannelNr].data;
			delete pSnd->pStreamAud;
			byte flags = Audio::FLAG_UNSIGNED;
			// Amiga SCI1 games had signed sound data
			if (_soundVersion >= SCI_VERSION_1_EARLY && g_sci->getPlatform() == Common::kPlatformAmiga)
				flags = 0;
			int endPart = track->digitalSampleEnd > 0 ? (track->digitalSampleSize - track->digitalSampleEnd) : 0;
			pSnd->pStreamAud = Audio::makeRawStream(channelData + track->digitalSampleStart, 
								track->digitalSampleSize - track->digitalSampleStart - endPart, 
								track->digitalSampleRate, flags, DisposeAfterUse::NO);
			delete pSnd->pLoopStream;
			pSnd->pLoopStream = 0;
			pSnd->soundType = Audio::Mixer::kSFXSoundType;
			pSnd->hCurrentAud = Audio::SoundHandle();
		} else {
			// play MIDI track
			_mutex.lock();
			pSnd->soundType = Audio::Mixer::kMusicSoundType;
			if (pSnd->pMidiParser == NULL) {
				pSnd->pMidiParser = new MidiParser_SCI(_soundVersion, this);
				pSnd->pMidiParser->setMidiDriver(_pMidiDrv);
				pSnd->pMidiParser->setTimerRate(_dwTempo);
			}

			pSnd->pauseCounter = 0;

			// Find out what channels to filter for SCI0
			channelFilterMask = pSnd->soundRes->getChannelFilterMask(_pMidiDrv->getPlayId(), _pMidiDrv->hasRhythmChannel());
			pSnd->pMidiParser->loadMusic(track, pSnd, channelFilterMask, _soundVersion);

			_mutex.unlock();
		}
	}
}

// This one checks, if requested channel is available -> in that case give caller that channel
//  Otherwise look for an unused one
int16 SciMusic::tryToOwnChannel(MusicEntry *caller, int16 bestChannel) {
	// Don't even try this for SCI0
	if (_soundVersion <= SCI_VERSION_0_LATE)
		return bestChannel;
	if (!_usedChannel[bestChannel]) {
		// currently unused, so give it to caller directly
		_usedChannel[bestChannel] = caller;
		return bestChannel;
	}
	// otherwise look for unused channel
	for (int channelNr = 0; channelNr < 15; channelNr++) {
		if (!_usedChannel[channelNr]) {
			_usedChannel[channelNr] = caller;
			return channelNr;
		}
	}
	error("no free channels");
}

void SciMusic::onTimer() {
	const MusicList::iterator end = _playList.end();
	for (MusicList::iterator i = _playList.begin(); i != end; ++i)
		(*i)->onTimer();
}

void SciMusic::soundPlay(MusicEntry *pSnd) {
	_mutex.lock();

	uint playListCount = _playList.size();
	uint playListNo = playListCount;
	MusicEntry *alreadyPlaying = NULL;

	// searching if sound is already in _playList
	for (uint i = 0; i < playListCount; i++) {
		if (_playList[i] == pSnd)
			playListNo = i;
		if ((_playList[i]->status == kSoundPlaying) && (_playList[i]->pMidiParser))
			alreadyPlaying = _playList[i];
	}
	if (playListNo == playListCount) { // not found
		_playList.push_back(pSnd);
		sortPlayList();
	}

	_mutex.unlock();	// unlock to perform mixer-related calls

	if (pSnd->pMidiParser) {
		if ((_soundVersion <= SCI_VERSION_0_LATE) && (alreadyPlaying)) {
			// Music already playing in SCI0?
			if (pSnd->priority > alreadyPlaying->priority) {
				// And new priority higher? pause previous music and play new one immediately
				// Example of such case: lsl3, when getting points (jingle is played then)
				soundPause(alreadyPlaying);
				alreadyPlaying->isQueued = true;
			} else {
				// And new priority equal or lower? queue up music and play it afterwards done by
				//  SoundCommandParser::updateSci0Cues()
				// Example of such case: iceman room 14
				pSnd->isQueued = true;
				pSnd->status = kSoundPaused;
				return;
			}
		}
	}

	if (pSnd->pStreamAud && !_pMixer->isSoundHandleActive(pSnd->hCurrentAud)) {
		if (pSnd->loop > 1) {
			pSnd->pLoopStream = new Audio::LoopingAudioStream(pSnd->pStreamAud,
			                                                  pSnd->loop, DisposeAfterUse::NO);
			_pMixer->playStream(pSnd->soundType, &pSnd->hCurrentAud,
			                         pSnd->pLoopStream, -1, pSnd->volume, 0,
			                         DisposeAfterUse::NO);
		} else {
			// Rewind in case we play the same sample multiple times (non-looped) like in pharkas right at the start
			pSnd->pStreamAud->rewind();
			_pMixer->playStream(pSnd->soundType, &pSnd->hCurrentAud,
			                         pSnd->pStreamAud, -1, pSnd->volume, 0,
			                         DisposeAfterUse::NO);
		}
	} else {
		_mutex.lock();
		if (pSnd->pMidiParser) {
			pSnd->pMidiParser->tryToOwnChannels();
			pSnd->pMidiParser->setVolume(pSnd->volume);
			if (pSnd->status == kSoundStopped)
				pSnd->pMidiParser->jumpToTick(0);
			else
				// Fast forward to the last position and perform associated events when loading
				pSnd->pMidiParser->jumpToTick(pSnd->ticker, true);
		}
		_mutex.unlock();
	}

	pSnd->status = kSoundPlaying;
}

void SciMusic::soundStop(MusicEntry *pSnd) {
	pSnd->status = kSoundStopped;
	if (_soundVersion <= SCI_VERSION_0_LATE)
		pSnd->isQueued = false;
	if (pSnd->pStreamAud)
		_pMixer->stopHandle(pSnd->hCurrentAud);

	_mutex.lock();
	if (pSnd->pMidiParser)
		pSnd->pMidiParser->stop();
	_mutex.unlock();
}

void SciMusic::soundSetVolume(MusicEntry *pSnd, byte volume) {
	assert(volume <= MUSIC_VOLUME_MAX);
	if (pSnd->pStreamAud) {
		_pMixer->setChannelVolume(pSnd->hCurrentAud, volume * 2); // Mixer is 0-255, SCI is 0-127
	} else if (pSnd->pMidiParser) {
		_mutex.lock();
		pSnd->pMidiParser->setVolume(volume);
		_mutex.unlock();
	}
}

void SciMusic::soundSetPriority(MusicEntry *pSnd, byte prio) {
	Common::StackLock lock(_mutex);

	pSnd->priority = prio;
	sortPlayList();
}

void SciMusic::soundKill(MusicEntry *pSnd) {
	pSnd->status = kSoundStopped;

	_mutex.lock();
	if (pSnd->pMidiParser) {
		pSnd->pMidiParser->unloadMusic();
		delete pSnd->pMidiParser;
		pSnd->pMidiParser = NULL;
	}
	_mutex.unlock();

	if (pSnd->pStreamAud) {
		_pMixer->stopHandle(pSnd->hCurrentAud);
		delete pSnd->pStreamAud;
		pSnd->pStreamAud = NULL;
		delete pSnd->pLoopStream;
		pSnd->pLoopStream = 0;
	}

	_mutex.lock();
	uint sz = _playList.size(), i;
	// Remove used channels
	for (i = 0; i < 15; i++) {
		if (_usedChannel[i] == pSnd)
			_usedChannel[i] = 0;
	}
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

void SciMusic::soundPause(MusicEntry *pSnd) {
	pSnd->pauseCounter++;
	if (pSnd->status != kSoundPlaying)
		return;
	pSnd->status = kSoundPaused;
	if (pSnd->pStreamAud) {
		_pMixer->pauseHandle(pSnd->hCurrentAud, true);
	} else {
		_mutex.lock();
		if (pSnd->pMidiParser)
			pSnd->pMidiParser->pause();
		_mutex.unlock();
	}
}

void SciMusic::soundResume(MusicEntry *pSnd) {
	if (pSnd->pauseCounter > 0)
		pSnd->pauseCounter--;
	if (pSnd->pauseCounter != 0)
		return;
	if (pSnd->status != kSoundPaused)
		return;
	if (pSnd->pStreamAud) {
		_pMixer->pauseHandle(pSnd->hCurrentAud, false);
		pSnd->status = kSoundPlaying;
	} else {
		soundPlay(pSnd);
	}
}

void SciMusic::soundToggle(MusicEntry *pSnd, bool pause) {
	if (pause)
		soundPause(pSnd);
	else
		soundResume(pSnd);
}

uint16 SciMusic::soundGetMasterVolume() {
	return _masterVolume;
}

void SciMusic::soundSetMasterVolume(uint16 vol) {
	_masterVolume = vol;

	Common::StackLock lock(_mutex);

	if (_pMidiDrv)
		_pMidiDrv->setVolume(vol);
}

void SciMusic::printPlayList(Console *con) {
	Common::StackLock lock(_mutex);

	const char *musicStatus[] = { "Stopped", "Initialized", "Paused", "Playing" };

	for (uint32 i = 0; i < _playList.size(); i++) {
		MusicEntry *song = _playList[i];
		con->DebugPrintf("%d: %04x:%04x, resource id: %d, status: %s, %s type\n", i,
						PRINT_REG(song->soundObj), song->resourceId,
						musicStatus[song->status], song->pMidiParser ? "MIDI" : "digital audio");
	}
}

void SciMusic::printSongInfo(reg_t obj, Console *con) {
	Common::StackLock lock(_mutex);

	const char *musicStatus[] = { "Stopped", "Initialized", "Paused", "Playing" };

	const MusicList::iterator end = _playList.end();
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
		MusicEntry *song = *i;
		if (song->soundObj == obj) {
			con->DebugPrintf("Resource id: %d, status: %s\n", song->resourceId, musicStatus[song->status]);
			con->DebugPrintf("dataInc: %d, hold: %d, loop: %d\n", song->dataInc, song->hold, song->loop);
			con->DebugPrintf("signal: %d, priority: %d\n", song->signal, song->priority);
			con->DebugPrintf("ticker: %d, volume: %d\n", song->ticker, song->volume);

			if (song->pMidiParser) {
				con->DebugPrintf("Type: MIDI\n");
				if (song->soundRes) {
					SoundResource::Track *track = song->soundRes->getTrackByType(_pMidiDrv->getPlayId());
					con->DebugPrintf("Channels: %d\n", track->channelCount);
				}
			} else if (song->pStreamAud || song->pLoopStream) {
				con->DebugPrintf("Type: digital audio (%s), sound active: %s\n",
					song->pStreamAud ? "non looping" : "looping",
					_pMixer->isSoundHandleActive(song->hCurrentAud) ? "yes" : "no");
				if (song->soundRes) {
					con->DebugPrintf("Sound resource information:\n");
					SoundResource::Track *track = song->soundRes->getTrackByType(_pMidiDrv->getPlayId());
					if (track && track->digitalChannelNr != -1) {
						con->DebugPrintf("Sample size: %d, sample rate: %d, channels: %d, digital channel number: %d\n",
							track->digitalSampleSize, track->digitalSampleRate, track->channelCount, track->digitalChannelNr);
					}
				}
			}

			return;
		}
	}

	con->DebugPrintf("Song object not found in playlist");
}

MusicEntry::MusicEntry() {
	soundObj = NULL_REG;

	soundRes = 0;
	resourceId = 0;

	isQueued = false;

	dataInc = 0;
	ticker = 0;
	signal = 0;
	priority = 0;
	loop = 0;
	volume = MUSIC_VOLUME_DEFAULT;
	hold = 0;

	pauseCounter = 0;
	sampleLoopCounter = 0;

	fadeTo = 0;
	fadeStep = 0;
	fadeTicker = 0;
	fadeTickerStep = 0;
	fadeSetVolume = false;
	fadeCompleted = false;
	stopAfterFading = false;

	status = kSoundStopped;

	soundType = Audio::Mixer::kMusicSoundType;

	pStreamAud = 0;
	pLoopStream = 0;
	pMidiParser = 0;
}

MusicEntry::~MusicEntry() {
}

void MusicEntry::onTimer() {
	if (status != kSoundPlaying)
		return;

	// Fade MIDI and digital sound effects
	if (fadeStep)
		doFade();

	// Only process MIDI streams in this thread, not digital sound effects
	if (pMidiParser) {
		pMidiParser->onTimer();
		ticker = (uint16)pMidiParser->getTick();
	}
}

void MusicEntry::doFade() {
	if (fadeTicker)
		fadeTicker--;
	else {
		fadeTicker = fadeTickerStep;
		volume += fadeStep;
		if (((fadeStep > 0) && (volume >= fadeTo)) || ((fadeStep < 0) && (volume <= fadeTo))) {
			volume = fadeTo;
			fadeStep = 0;
			fadeCompleted = true;
		}
#ifdef ENABLE_SCI32
		// Disable fading for SCI32 - sound drivers have issues when fading in (gabriel knight 1 sierra title)
		if (getSciVersion() >= SCI_VERSION_2) {
			volume = fadeTo;
			fadeStep = 0;
			fadeCompleted = true;
		}
#endif

		// Only process MIDI streams in this thread, not digital sound effects
		if (pMidiParser) {
			pMidiParser->setVolume(volume);
		}

		fadeSetVolume = true; // set flag so that SoundCommandParser::cmdUpdateCues will set the volume of the stream
	}
}

} // End of namespace Sci
