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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "gui/error.h"

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/resource.h"
#include "sci/engine/features.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/sound/midiparser_sci.h"
#include "sci/sound/music.h"

//#define DEBUG_REMAP

namespace Sci {

SciMusic::SciMusic(SciVersion soundVersion, bool useDigitalSFX)
	: _soundVersion(soundVersion), _soundOn(true), _masterVolume(15), _globalReverb(0), _useDigitalSFX(useDigitalSFX) {

	// Reserve some space in the playlist, to avoid expensive insertion
	// operations
	_playList.reserve(10);

	for (int i = 0; i < 16; i++) {
		_usedChannel[i] = 0;
		_channelRemap[i] = -1;
		_channelMap[i]._song = 0;
		_channelMap[i]._channel = -1;
	}

	_queuedCommands.reserve(1000);
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

	Common::Platform platform = g_sci->getPlatform();
	uint32 deviceFlags;
	if (g_sci->_features->generalMidiOnly()) {
		deviceFlags = MDT_MIDI;
	} else {
		deviceFlags = MDT_PCSPK | MDT_PCJR | MDT_ADLIB | MDT_MIDI;
	}

	// Default to MIDI for Windows versions of SCI1.1 games, as their
	// soundtrack is written for GM.
	if (g_sci->_features->useAltWinGMSound())
		deviceFlags |= MDT_PREFER_GM;

	// SCI_VERSION_0_EARLY games apparently don't support the CMS. At least there
	// is no patch resource 101 and I also haven't seen any CMS driver file so far.
	if (getSciVersion() > SCI_VERSION_0_EARLY && getSciVersion() <= SCI_VERSION_1_1)
		deviceFlags |= MDT_CMS;

	if (g_sci->getPlatform() == Common::kPlatformFMTowns) {
		if (getSciVersion() > SCI_VERSION_1_EARLY)
			deviceFlags = MDT_TOWNS;
		else
			deviceFlags |= MDT_TOWNS;
	}

	if (g_sci->getPlatform() == Common::kPlatformPC98)
		deviceFlags |= MDT_PC98;

	uint32 dev = MidiDriver::detectDevice(deviceFlags);
	_musicType = MidiDriver::getMusicType(dev);

	if (g_sci->_features->useAltWinGMSound() && _musicType != MT_GM) {
		warning("A Windows CD version with an alternate MIDI soundtrack has been chosen, "
				"but no MIDI music device has been selected. Reverting to the DOS soundtrack");
		g_sci->_features->forceDOSTracks();
#ifdef ENABLE_SCI32
	} else if (g_sci->_features->generalMidiOnly() && _musicType != MT_GM) {
		warning("This game only supports General MIDI, but a non-GM device has "
				"been selected. Some music may be wrong or missing");
#endif
	}

	switch (_musicType) {
	case MT_ADLIB:
		// FIXME: There's no Amiga sound option, so we hook it up to AdLib
		if (g_sci->getPlatform() == Common::kPlatformAmiga || platform == Common::kPlatformMacintosh)
			_pMidiDrv = MidiPlayer_AmigaMac_create(_soundVersion, platform);
		else
			_pMidiDrv = MidiPlayer_AdLib_create(_soundVersion);
		break;
	case MT_PCJR:
		_pMidiDrv = MidiPlayer_PCJr_create(_soundVersion);
		break;
	case MT_PCSPK:
		_pMidiDrv = MidiPlayer_PCSpeaker_create(_soundVersion);
		break;
	case MT_CMS:
		_pMidiDrv = MidiPlayer_CMS_create(_soundVersion);
		break;
	case MT_TOWNS:
		_pMidiDrv = MidiPlayer_FMTowns_create(_soundVersion);
		break;
	case MT_PC98:		
		_pMidiDrv = MidiPlayer_PC9801_create(_soundVersion);
		break;
	default:
		if (ConfMan.getBool("native_fb01"))
			_pMidiDrv = MidiPlayer_Fb01_create(_soundVersion);
		else
			_pMidiDrv = MidiPlayer_Midi_create(_soundVersion);
	}

	if (_pMidiDrv && !_pMidiDrv->open()) {
		_pMidiDrv->setTimerCallback(this, &miditimerCallback);
		_dwTempo = _pMidiDrv->getBaseTempo();
	} else {
		if (g_sci->getGameId() == GID_FUNSEEKER ||
			(g_sci->getGameId() == GID_GK2 && g_sci->isDemo())) {
			// HACK: The Fun Seeker's Guide demo doesn't have patch 3 and the version
			// of the Adlib driver (adl.drv) that it includes is unsupported. That demo
			// doesn't have any sound anyway, so this shouldn't be fatal.
		} else {
			const char *missingFiles = _pMidiDrv->reportMissingFiles();
			if (missingFiles) {
				Common::String message = _(
					"The selected audio driver requires the following file(s):\n\n"
				);
				message += missingFiles;
				message += _("\n\n"
					"Some audio drivers (at least for some games) were made\n"
					"available by Sierra as aftermarket patches and thus might not\n"
					"have been installed as part of the original game setup.\n\n"
					"Please copy these file(s) into your game data directory.\n\n"
					"However, please note that the file(s) might not be available\n"
					"separately but only as content of (patched) resource bundles.\n"
					"In that case you may need to apply the original Sierra patch.\n\n"
				);
				::GUI::displayErrorDialog(message.c_str());
			}
			error("Failed to initialize sound driver");
		}
	}

	// Find out what the first possible channel is (used, when doing channel
	// remapping).
	_driverFirstChannel = _pMidiDrv->getFirstChannel();
	_driverLastChannel = _pMidiDrv->getLastChannel();
	if (getSciVersion() <= SCI_VERSION_0_LATE)
		_globalReverb = _pMidiDrv->getReverb();	// Init global reverb for SCI0

	_currentlyPlayingSample = NULL;
	_timeCounter = 0;
	_needsRemap = false;
}

void SciMusic::miditimerCallback(void *p) {
	SciMusic *sciMusic = (SciMusic *)p;

	Common::StackLock lock(sciMusic->_mutex);
	sciMusic->onTimer();
}

void SciMusic::onTimer() {
	const MusicList::iterator end = _playList.end();
	// sending out queued commands that were "sent" via main thread
	sendMidiCommandsFromQueue();

	// remap channels, if requested
	if (_needsRemap)
		remapChannels(false);
	_needsRemap = false;

	for (MusicList::iterator i = _playList.begin(); i != end; ++i)
		(*i)->onTimer();
}

void SciMusic::putMidiCommandInQueue(byte status, byte firstOp, byte secondOp) {
	putMidiCommandInQueue(status | ((uint32)firstOp << 8) | ((uint32)secondOp << 16));
}

void SciMusic::putMidiCommandInQueue(uint32 midi) {
	_queuedCommands.push_back(midi);
}

// This sends the stored commands from queue to driver (is supposed to get
// called only during onTimer()). At least mt32 emulation doesn't like getting
// note-on commands from main thread (if we directly send, we would get a crash
// during piano scene in lsl5).
void SciMusic::sendMidiCommandsFromQueue() {
	uint curCommand = 0;
	uint commandCount = _queuedCommands.size();

	while (curCommand < commandCount) {
		_pMidiDrv->send(_queuedCommands[curCommand]);
		curCommand++;
	}
	_queuedCommands.clear();
}

void SciMusic::clearPlayList() {
	// we must NOT lock our mutex here. Playlist is modified inside soundKill() which will lock the mutex
	//  during deletion. If we lock it here, a deadlock may occur within soundStop() because that one
	//  calls the mixer, which will also lock the mixer mutex and if the mixer thread is active during
	//  that time, we will get a deadlock.
	while (!_playList.empty()) {
		soundStop(_playList[0]);
		soundKill(_playList[0]);
	}
}

void SciMusic::pauseAll(bool pause) {
	const MusicList::iterator end = _playList.end();
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
#ifdef ENABLE_SCI32
		// The entire DAC will have been paused by the caller;
		// do not pause the individual samples too
		if (_soundVersion >= SCI_VERSION_2 && (*i)->isSample) {
			continue;
		}
#endif
		soundToggle(*i, pause);
	}
}

void SciMusic::stopAll() {
	const MusicList::iterator end = _playList.end();
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
		soundStop(*i);
	}
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

// We return the currently active music slot for SCI0
MusicEntry *SciMusic::getActiveSci0MusicSlot() {
	const MusicList::iterator end = _playList.end();
	MusicEntry *highestPrioritySlot = NULL;
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
		MusicEntry *playSlot = *i;
		if (playSlot->pMidiParser) {
			if (playSlot->status == kSoundPlaying)
				return playSlot;
			if (playSlot->status == kSoundPaused) {
				if ((!highestPrioritySlot) || (highestPrioritySlot->priority < playSlot->priority))
					highestPrioritySlot = playSlot;
			}
		}
	}
	return highestPrioritySlot;
}

void SciMusic::setGlobalReverb(int8 reverb) {
	Common::StackLock lock(_mutex);
	if (reverb != 127) {
		// Set global reverb normally
		_globalReverb = reverb;

		// Check the reverb of the active song...
		const MusicList::iterator end = _playList.end();
		for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
			if ((*i)->status == kSoundPlaying) {
				if ((*i)->reverb == 127)			// Active song has no reverb
					_pMidiDrv->setReverb(reverb);	// Set the global reverb
				break;
			}
		}
	} else {
		// Set reverb of the active song
		const MusicList::iterator end = _playList.end();
		for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
			if ((*i)->status == kSoundPlaying) {
				_pMidiDrv->setReverb((*i)->reverb);	// Set the song's reverb
				break;
			}
		}
	}
}

byte SciMusic::getCurrentReverb() {
	Common::StackLock lock(_mutex);
	return _pMidiDrv->getReverb();
}

// A larger priority value has higher priority. For equal priority values,
// songs that have been added later have higher priority.
static bool musicEntryCompare(const MusicEntry *l, const MusicEntry *r) {
	return (l->priority > r->priority) || (l->priority == r->priority && l->time > r->time);
}

void SciMusic::sortPlayList() {
	// Sort the play list in descending priority order
	Common::sort(_playList.begin(), _playList.end(), musicEntryCompare);
}

void SciMusic::soundInitSnd(MusicEntry *pSnd) {
	// Remove all currently mapped channels of this MusicEntry first,
	// since they will no longer be valid.
	for (int i = 0; i < 16; ++i) {
		if (_channelMap[i]._song == pSnd) {
			_channelMap[i]._song = 0;
			_channelMap[i]._channel = -1;
		}
	}

	int channelFilterMask = 0;
	SoundResource::Track *track = pSnd->soundRes->getTrackByType(_pMidiDrv->getPlayId());

	// If MIDI device is selected but there is no digital track in sound
	// resource try to use Adlib's digital sample if possible. Also, if the
	// track couldn't be found, load the digital track, as some games depend on
	// this (e.g. the Longbow demo).
	if (!track || (_useDigitalSFX && track->digitalChannelNr == -1)) {
		SoundResource::Track *digital = pSnd->soundRes->getDigitalTrack();
		if (digital)
			track = digital;
	}

	pSnd->time = ++_timeCounter;

	if (track) {
		bool playSample;

		if (_soundVersion <= SCI_VERSION_0_LATE && !_useDigitalSFX) {
			// For SCI0 the digital sample is present in the same track as the
			// MIDI. If the user specifically requests not to use the digital
			// samples, play the MIDI data instead. If the MIDI portion of the
			// track is empty however, play the digital sample anyway. This is
			// necessary for e.g. the "Where am I?" sample in the SQ3 intro.
			playSample = false;

			if (track->channelCount == 2) {
				SoundResource::Channel &chan = track->channels[0];
				if (chan.data.size() < 2 || chan.data[1] == SCI_MIDI_EOT) {
					playSample = true;
				}
			}
		} else
			playSample = (track->digitalChannelNr != -1);

		// Play digital sample
		if (playSample) {
			const SciSpan<const byte> &channelData = track->channels[track->digitalChannelNr].data;
			delete pSnd->pStreamAud;
			byte flags = Audio::FLAG_UNSIGNED;
			// Amiga SCI1 games had signed sound data
			if (_soundVersion >= SCI_VERSION_1_EARLY && g_sci->getPlatform() == Common::kPlatformAmiga)
				flags = 0;
			int endPart = track->digitalSampleEnd > 0 ? (track->digitalSampleSize - track->digitalSampleEnd) : 0;
			const uint size = track->digitalSampleSize - track->digitalSampleStart - endPart;
			pSnd->pStreamAud = Audio::makeRawStream(channelData.getUnsafeDataAt(track->digitalSampleStart),
								size, track->digitalSampleRate, flags, DisposeAfterUse::NO);
			assert(pSnd->pStreamAud);
			delete pSnd->pLoopStream;
			pSnd->pLoopStream = 0;
			pSnd->soundType = Audio::Mixer::kSFXSoundType;
			pSnd->hCurrentAud = Audio::SoundHandle();
			pSnd->playBed = false;
			pSnd->overridePriority = false;
			pSnd->isSample = true;
		} else {
			// play MIDI track
			Common::StackLock lock(_mutex);
			pSnd->soundType = Audio::Mixer::kMusicSoundType;
			if (pSnd->pMidiParser == NULL) {
				pSnd->pMidiParser = new MidiParser_SCI(_soundVersion, this);
				pSnd->pMidiParser->setMidiDriver(_pMidiDrv);
				pSnd->pMidiParser->setTimerRate(_dwTempo);
				pSnd->pMidiParser->setMasterVolume(_masterVolume);
			}

			pSnd->pauseCounter = 0;

			// Find out what channels to filter for SCI0
			channelFilterMask = pSnd->soundRes->getChannelFilterMask(_pMidiDrv->getPlayId(), _pMidiDrv->hasRhythmChannel());

			for (int i = 0; i < 16; ++i)
				pSnd->_usedChannels[i] = 0xFF;
			for (int i = 0; i < track->channelCount; ++i) {
				SoundResource::Channel &chan = track->channels[i];

				pSnd->_usedChannels[i] = chan.number;
				pSnd->_chan[chan.number]._dontRemap = (chan.flags & 2);
				pSnd->_chan[chan.number]._prio = chan.prio;
				pSnd->_chan[chan.number]._voices = chan.poly;

				// CHECKME: Some SCI versions use chan.flags & 1 for this:
				pSnd->_chan[chan.number]._dontMap = false;

				// FIXME: Most MIDI tracks use the first 10 bytes for
				// fixed MIDI commands. SSCI skips those the first iteration,
				// but _does_ update channel state (including volume) with
				// them. Specifically, prio/voices, patch, volume, pan.
				// This should probably be implemented in
				// MidiParser_SCI::loadMusic.
			}

			pSnd->pMidiParser->mainThreadBegin();
			// loadMusic() below calls jumpToTick.
			// Disable sound looping and hold before jumpToTick is called,
			// otherwise the song may keep looping forever when it ends in
			// jumpToTick (e.g. LSL3, when going left from room 210).
			uint16 prevLoop = pSnd->loop;
			int16 prevHold = pSnd->hold;
			pSnd->loop = 0;
			pSnd->hold = -1;
			pSnd->playBed = false;
			pSnd->overridePriority = false;

			pSnd->pMidiParser->loadMusic(track, pSnd, channelFilterMask, _soundVersion);
			pSnd->reverb = pSnd->pMidiParser->getSongReverb();

			// Restore looping and hold
			pSnd->loop = prevLoop;
			pSnd->hold = prevHold;
			pSnd->pMidiParser->mainThreadEnd();
		}
	}
}

void SciMusic::soundPlay(MusicEntry *pSnd) {
	_mutex.lock();

	if (_soundVersion <= SCI_VERSION_1_EARLY && pSnd->playBed) {
		// If pSnd->playBed, and version <= SCI1_EARLY, then kill
		// existing sounds with playBed enabled.

		uint playListCount = _playList.size();
		for (uint i = 0; i < playListCount; i++) {
			if (_playList[i] != pSnd && _playList[i]->playBed) {
				debugC(2, kDebugLevelSound, "Automatically stopping old playBed song from soundPlay");
				MusicEntry *old = _playList[i];
				_mutex.unlock();
				soundStop(old);
				_mutex.lock();
				break;
			}
		}
	}

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
	}

	pSnd->time = ++_timeCounter;
	sortPlayList();

	_mutex.unlock();	// unlock to perform mixer-related calls

	if (pSnd->pMidiParser) {
		if ((_soundVersion <= SCI_VERSION_0_LATE) && (alreadyPlaying)) {
			// Music already playing in SCI0?
			if (pSnd->priority > alreadyPlaying->priority) {
				// And new priority higher? pause previous music and play new one immediately.
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

	if (pSnd->isSample) {
#ifdef ENABLE_SCI32
		if (_soundVersion >= SCI_VERSION_2) {
			// TODO: Sound number, loop state, and volume come from soundObj
			// in SSCI. Getting them from MusicEntry could cause a bug if the
			// soundObj was updated by a game script and not copied back to
			// MusicEntry.
			g_sci->_audio32->restart(ResourceId(kResourceTypeAudio, pSnd->resourceId), true, pSnd->loop != 0 && pSnd->loop != 1, pSnd->volume, pSnd->soundObj, false);
			return;
		} else
#endif
		if (!_pMixer->isSoundHandleActive(pSnd->hCurrentAud)) {
			if ((_currentlyPlayingSample) && (_pMixer->isSoundHandleActive(_currentlyPlayingSample->hCurrentAud))) {
				// Another sample is already playing, we have to stop that one
				// SSCI is only able to play 1 sample at a time
				// In Space Quest 5 room 250 the player is able to open the air-hatch and kill himself.
				//  In that situation the scripts are playing 2 samples at the same time and the first sample
				//  is not supposed to play.
				// TODO: SSCI actually calls kDoAudio(play) internally, which stops other samples from being played
				//        but such a change isn't trivial, because we also handle Sound resources in here, that contain samples
				_pMixer->stopHandle(_currentlyPlayingSample->hCurrentAud);
				warning("kDoSound: sample already playing, old resource %d, new resource %d", _currentlyPlayingSample->resourceId, pSnd->resourceId);
			}
			// Sierra SCI ignores volume set when playing samples via kDoSound
			//  At least freddy pharkas/CD has a script bug that sets volume to 0
			//  when playing the "score" sample
			if (pSnd->loop > 1) {
				pSnd->pLoopStream = new Audio::LoopingAudioStream(pSnd->pStreamAud,
																pSnd->loop, DisposeAfterUse::NO);
				_pMixer->playStream(pSnd->soundType, &pSnd->hCurrentAud,
										pSnd->pLoopStream, -1, _pMixer->kMaxChannelVolume, 0,
										DisposeAfterUse::NO);
			} else {
				// Rewind in case we play the same sample multiple times
				// (non-looped) like in pharkas right at the start
				pSnd->pStreamAud->rewind();
				_pMixer->playStream(pSnd->soundType, &pSnd->hCurrentAud,
										pSnd->pStreamAud, -1, _pMixer->kMaxChannelVolume, 0,
										DisposeAfterUse::NO);
			}
			// Remember the sample, that is now playing
			_currentlyPlayingSample = pSnd;
		}
	} else {
		if (pSnd->pMidiParser) {
			Common::StackLock lock(_mutex);
			pSnd->pMidiParser->mainThreadBegin();

			// The track init always needs to be done. Otherwise some sounds will not be properly set up (bug #11476).
			// It is also safe to do this for paused tracks, since the jumpToTick() command in line 602 will parse through
			// the song from the beginning up to the resume position and ensure that the actual current voice mapping,
			// instrument and volume settings etc. are correct.
 			pSnd->pMidiParser->initTrack();

			if (pSnd->status != kSoundPaused)
				pSnd->pMidiParser->sendInitCommands();
			pSnd->pMidiParser->setVolume(pSnd->volume);

			// Disable sound looping and hold before jumpToTick is called,
			// otherwise the song may keep looping forever when it ends in jumpToTick.
			// This is needed when loading saved games, or when a game
			// stops the same sound twice (e.g. LSL3 Amiga, going left from
			// room 210 to talk with Kalalau). Fixes bugs #3083151 and #3106107.
			uint16 prevLoop = pSnd->loop;
			int16 prevHold = pSnd->hold;
			pSnd->loop = 0;
			pSnd->hold = -1;

			if (pSnd->status == kSoundStopped)
				pSnd->pMidiParser->jumpToTick(0);
			else {
				// Fast forward to the last position and perform associated events when loading
				pSnd->pMidiParser->jumpToTick(pSnd->ticker, true, true, true);
			}

			// Restore looping and hold
			pSnd->loop = prevLoop;
			pSnd->hold = prevHold;
			pSnd->pMidiParser->mainThreadEnd();
		}
	}

	pSnd->status = kSoundPlaying;

	_mutex.lock();
	remapChannels();
	_mutex.unlock();
}

void SciMusic::soundStop(MusicEntry *pSnd) {
	SoundStatus previousStatus = pSnd->status;
	pSnd->status = kSoundStopped;
	if (_soundVersion <= SCI_VERSION_0_LATE)
		pSnd->isQueued = false;
	if (pSnd->isSample) {
#ifdef ENABLE_SCI32
		if (_soundVersion >= SCI_VERSION_2) {
			g_sci->_audio32->stop(ResourceId(kResourceTypeAudio, pSnd->resourceId), pSnd->soundObj);
		} else {
#endif
			if (_currentlyPlayingSample == pSnd)
				_currentlyPlayingSample = NULL;
			_pMixer->stopHandle(pSnd->hCurrentAud);
#ifdef ENABLE_SCI32
		}
#endif
	}

	if (pSnd->pMidiParser) {
		Common::StackLock lock(_mutex);
		pSnd->pMidiParser->mainThreadBegin();
		// We shouldn't call stop in case it's paused, otherwise we would send
		// allNotesOff() again
		if (previousStatus == kSoundPlaying)
			pSnd->pMidiParser->stop();
		pSnd->pMidiParser->mainThreadEnd();
		remapChannels();
	}

	pSnd->fadeStep = 0; // end fading, if fading was in progress
}

void SciMusic::soundSetVolume(MusicEntry *pSnd, byte volume) {
	assert(volume <= MUSIC_VOLUME_MAX);
	if (!pSnd->isSample && pSnd->pMidiParser) {
		Common::StackLock lock(_mutex);
		pSnd->pMidiParser->mainThreadBegin();
		pSnd->pMidiParser->setVolume(volume);
		pSnd->pMidiParser->mainThreadEnd();
	}
}

// this is used to set volume of the sample, used for fading only!
void SciMusic::soundSetSampleVolume(MusicEntry *pSnd, byte volume) {
	assert(volume <= MUSIC_VOLUME_MAX);
	assert(pSnd->pStreamAud);
	_pMixer->setChannelVolume(pSnd->hCurrentAud, volume * 2); // Mixer is 0-255, SCI is 0-127
}

void SciMusic::soundSetPriority(MusicEntry *pSnd, byte prio) {
	Common::StackLock lock(_mutex);

	pSnd->priority = prio;
	pSnd->time = ++_timeCounter;
	sortPlayList();
}

void SciMusic::soundKill(MusicEntry *pSnd) {
	pSnd->status = kSoundStopped;

	_mutex.lock();
	remapChannels();

	if (pSnd->pMidiParser) {
		pSnd->pMidiParser->mainThreadBegin();
		pSnd->pMidiParser->unloadMusic();
		pSnd->pMidiParser->mainThreadEnd();
		delete pSnd->pMidiParser;
		pSnd->pMidiParser = NULL;
	}

	_mutex.unlock();

	if (pSnd->isSample) {
#ifdef ENABLE_SCI32
		if (_soundVersion >= SCI_VERSION_2) {
			g_sci->_audio32->stop(ResourceId(kResourceTypeAudio, pSnd->resourceId), pSnd->soundObj);
		} else {
#endif
			if (_currentlyPlayingSample == pSnd) {
				// Forget about this sound, in case it was currently playing
				_currentlyPlayingSample = NULL;
			}
			_pMixer->stopHandle(pSnd->hCurrentAud);
#ifdef ENABLE_SCI32
		}
#endif
		delete pSnd->pStreamAud;
		pSnd->pStreamAud = NULL;
		delete pSnd->pLoopStream;
		pSnd->pLoopStream = 0;
		pSnd->isSample = false;
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

void SciMusic::soundPause(MusicEntry *pSnd) {
	// SCI seems not to be pausing samples played back by kDoSound at all
	//  It only stops looping samples (actually doesn't loop them again before they are unpaused)
	//  Examples: Space Quest 1 death by acid drops (pause is called even specifically for the sample, see bug #3038048)
	//             Eco Quest 1 during the intro when going to the abort-menu
	//             In both cases sierra sci keeps playing
	//            Leisure Suit Larry 1 doll scene - it seems that pausing here actually just stops
	//             further looping from happening
	//  This is a somewhat bigger change, I'm leaving in the old code in here just in case
	//  I'm currently pausing looped sounds directly, non-looped sounds won't get paused
	if ((pSnd->pStreamAud) && (!pSnd->pLoopStream))
		return;
	pSnd->pauseCounter++;
	if (pSnd->status != kSoundPlaying)
		return;
	pSnd->status = kSoundPaused;
	if (pSnd->pStreamAud) {
		_pMixer->pauseHandle(pSnd->hCurrentAud, true);
	} else {
		if (pSnd->pMidiParser) {
			Common::StackLock lock(_mutex);
			pSnd->pMidiParser->mainThreadBegin();
			pSnd->pMidiParser->pause();
			pSnd->pMidiParser->mainThreadEnd();
			remapChannels();
		}
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
#ifdef ENABLE_SCI32
	if (_soundVersion >= SCI_VERSION_2_1_EARLY && pSnd->isSample) {
		if (pause) {
			g_sci->_audio32->pause(ResourceId(kResourceTypeAudio, pSnd->resourceId), pSnd->soundObj);
		} else {
			g_sci->_audio32->resume(ResourceId(kResourceTypeAudio, pSnd->resourceId), pSnd->soundObj);
		}

		return;
	}
#endif

	if (pause)
		soundPause(pSnd);
	else
		soundResume(pSnd);
}

uint16 SciMusic::soundGetMasterVolume() {
	if (ConfMan.getBool("mute")) {
		// When a game is muted, the master volume is set to zero so that
		// mute applies to external MIDI devices, but this should not be
		// communicated to the game as it will cause the UI to be drawn with
		// the wrong (zero) volume for music
		return (ConfMan.getInt("music_volume") + 1) * MUSIC_MASTERVOLUME_MAX / Audio::Mixer::kMaxMixerVolume;
	}

	return _masterVolume;
}

void SciMusic::soundSetMasterVolume(uint16 vol) {
	_masterVolume = vol;

	Common::StackLock lock(_mutex);

	const MusicList::iterator end = _playList.end();
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
		if ((*i)->pMidiParser)
			(*i)->pMidiParser->setMasterVolume(vol);
	}
}

void SciMusic::sendMidiCommand(uint32 cmd) {
	Common::StackLock lock(_mutex);
	_pMidiDrv->send(cmd);
}

void SciMusic::sendMidiCommand(MusicEntry *pSnd, uint32 cmd) {
	Common::StackLock lock(_mutex);
	if (!pSnd->pMidiParser) {
		// FPFP calls kDoSound SendMidi to mute and unmute its gameMusic2 sound
		//  object but some scenes set this to an audio sample. In Act 2, room
		//  660 sets this to audio of restaurant customers talking. Walking up
		//  the hotel stairs from room 500 to 235 calls gameMusic2:mute and
		//  triggers this if gameMusic2 hasn't changed. Bug #10952
		warning("tried to cmdSendMidi on non midi slot (%04x:%04x)", PRINT_REG(pSnd->soundObj));
		return;
	}

	pSnd->pMidiParser->mainThreadBegin();
	pSnd->pMidiParser->sendFromScriptToDriver(cmd);
	pSnd->pMidiParser->mainThreadEnd();
}

void SciMusic::printPlayList(Console *con) {
	Common::StackLock lock(_mutex);

	const char *musicStatus[] = { "Stopped", "Initialized", "Paused", "Playing" };

	for (uint32 i = 0; i < _playList.size(); i++) {
		MusicEntry *song = _playList[i];
		con->debugPrintf("%d: %04x:%04x (%s), resource id: %d, status: %s, %s type\n",
						i, PRINT_REG(song->soundObj),
						g_sci->getEngineState()->_segMan->getObjectName(song->soundObj),
						song->resourceId, musicStatus[song->status],
						song->pMidiParser ? "MIDI" : "digital audio");
	}
}

void SciMusic::printSongInfo(reg_t obj, Console *con) {
	Common::StackLock lock(_mutex);

	const char *musicStatus[] = { "Stopped", "Initialized", "Paused", "Playing" };

	const MusicList::iterator end = _playList.end();
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
		MusicEntry *song = *i;
		if (song->soundObj == obj) {
			con->debugPrintf("Resource id: %d, status: %s\n", song->resourceId, musicStatus[song->status]);
			con->debugPrintf("dataInc: %d, hold: %d, loop: %d\n", song->dataInc, song->hold, song->loop);
			con->debugPrintf("signal: %d, priority: %d\n", song->signal, song->priority);
			con->debugPrintf("ticker: %d, volume: %d\n", song->ticker, song->volume);

			if (song->pMidiParser) {
				con->debugPrintf("Type: MIDI\n");
				if (song->soundRes) {
					SoundResource::Track *track = song->soundRes->getTrackByType(_pMidiDrv->getPlayId());
					con->debugPrintf("Channels: %d\n", track->channelCount);
				}
			} else if (song->pStreamAud || song->pLoopStream) {
				con->debugPrintf("Type: digital audio (%s), sound active: %s\n",
					song->pStreamAud ? "non looping" : "looping",
					_pMixer->isSoundHandleActive(song->hCurrentAud) ? "yes" : "no");
				if (song->soundRes) {
					con->debugPrintf("Sound resource information:\n");
					SoundResource::Track *track = song->soundRes->getTrackByType(_pMidiDrv->getPlayId());
					if (track && track->digitalChannelNr != -1) {
						con->debugPrintf("Sample size: %d, sample rate: %d, channels: %d, digital channel number: %d\n",
							track->digitalSampleSize, track->digitalSampleRate, track->channelCount, track->digitalChannelNr);
					}
				}
			}

			return;
		}
	}

	con->debugPrintf("Song object not found in playlist");
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
	hold = -1;
	reverb = -1;

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
	isSample = false;

	for (int i = 0; i < 16; ++i) {
		_usedChannels[i] = 0xFF;
		_chan[i]._prio = 127;
		_chan[i]._voices = 0;
		_chan[i]._dontRemap = false;
		_chan[i]._mute = false;
	}
}

MusicEntry::~MusicEntry() {
}

void MusicEntry::onTimer() {
	if (!signal) {
		if (!signalQueue.empty()) {
			// no signal set, but signal in queue, set that one
			signal = signalQueue[0];
			signalQueue.remove_at(0);
		}
	}

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

		// Only process MIDI streams in this thread, not digital sound effects
		if (pMidiParser) {
			pMidiParser->setVolume(volume);
		}

		fadeSetVolume = true; // set flag so that SoundCommandParser::cmdUpdateCues will set the volume of the stream
	}
}

void MusicEntry::setSignal(int newSignal) {
	// For SCI0, we cache the signals to set, as some songs might
	// update their signal faster than kGetEvent is called (which is where
	// we manually invoke kDoSoundUpdateCues for SCI0 games). SCI01 and
	// newer handle signalling inside kDoSoundUpdateCues. Refer to bug #3042981
	if (g_sci->_features->detectDoSoundType() <= SCI_VERSION_0_LATE) {
		if (!signal) {
			signal = newSignal;
		} else {
			// signal already set and waiting for getting to scripts, queue new one
			signalQueue.push_back(newSignal);
		}
	} else {
		// Set the signal directly for newer games, otherwise the sound
		// object might be deleted already later on (refer to bug #3045913)
		signal = newSignal;
	}
}


void ChannelRemapping::swap(int i, int j) {
	DeviceChannelUsage t1;
	int t2;
	bool t3;

	t1 = _map[i]; _map[i] = _map[j]; _map[j] = t1;
	t2 = _prio[i]; _prio[i] = _prio[j]; _prio[j] = t2;
	t2 = _voices[i]; _voices[i] = _voices[j]; _voices[j] = t2;
	t3 = _dontRemap[i]; _dontRemap[i] = _dontRemap[j]; _dontRemap[j] = t3;
}

void ChannelRemapping::evict(int i) {
	_freeVoices += _voices[i];

	_map[i]._song = 0;
	_map[i]._channel = -1;
	_prio[i] = 0;
	_voices[i] = 0;
	_dontRemap[i] = false;
}

void ChannelRemapping::clear() {
	for (int i = 0; i < 16; ++i) {
		_map[i]._song = 0;
		_map[i]._channel = -1;
		_prio[i] = 0;
		_voices[i] = 0;
		_dontRemap[i] = false;
	}
}

ChannelRemapping& ChannelRemapping::operator=(ChannelRemapping& other) {
	for (int i = 0; i < 16; ++i) {
		_map[i] = other._map[i];
		_prio[i] = other._prio[i];
		_voices[i] = other._voices[i];
		_dontRemap[i] = other._dontRemap[i];
	}
	_freeVoices = other._freeVoices;

	return *this;
}

int ChannelRemapping::lowestPrio() const {
	int max = 0;
	int channel = -1;
	for (int i = 0; i < 16; ++i) {
		if (_prio[i] > max) {
			max = _prio[i];
			channel = i;
		}
	}
	return channel;
}


void SciMusic::remapChannels(bool mainThread) {
	if (_soundVersion <= SCI_VERSION_0_LATE)
		return;

	// NB: This function should only be called with _mutex locked
	// Make sure to set the mainThread argument correctly.


	ChannelRemapping *map = determineChannelMap();

	DeviceChannelUsage currentMap[16];

#ifdef DEBUG_REMAP
	debug("Remap results:");
#endif

	// Save current map, and then start from an empty map
	for (int i = 0; i < 16; ++i) {
		currentMap[i] = _channelMap[i];
		_channelMap[i]._song = 0;
		_channelMap[i]._channel = -1;
	}

	// Inform MidiParsers of any unmapped channels
	const MusicList::iterator end = _playList.end();
	int songIndex = -1;
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
		MusicEntry *song = *i;
		songIndex++;

		if (!song || !song->pMidiParser)
			continue;

		bool channelMapped[16];
#ifdef DEBUG_REMAP
		bool channelUsed[16];
#endif
		for (int j = 0; j < 16; ++j) {
			channelMapped[j] = false;
#ifdef DEBUG_REMAP
			channelUsed[j] = false;
#endif
		}

		for (int j = 0; j < 16; ++j) {
			if (map->_map[j]._song == song) {
				int channel = map->_map[j]._channel;
				assert(channel >= 0 && channel <= 0x0F);
				channelMapped[channel] = true;
			}
#ifdef DEBUG_REMAP
			if (song->_usedChannels[j] <= 0x0F)
				channelUsed[song->_usedChannels[j]] = true;
#endif
		}

		for (int j = 0; j < 16; ++j) {
			if (!channelMapped[j]) {
				if (mainThread) song->pMidiParser->mainThreadBegin();
				song->pMidiParser->remapChannel(j, -1);
				if (mainThread) song->pMidiParser->mainThreadEnd();
#ifdef DEBUG_REMAP
				if (channelUsed[j])
					debug(" Unmapping song %d, channel %d", songIndex, j);
#endif
			}
		}
	}

	// Now reshuffle the channels on the device.

	// First, set up any dontRemap channels
	for (int i = 0; i < 16; ++i) {

		if (!map->_map[i]._song || !map->_map[i]._song->pMidiParser || !map->_dontRemap[i])
			continue;

		songIndex = -1;
		for (MusicList::iterator iter = _playList.begin(); iter != end; ++iter) {
			songIndex++;
			if (map->_map[i]._song == *iter)
				break;
		}

		_channelMap[i] = map->_map[i];
		map->_map[i]._song = 0; // mark as done

		// If this channel was not yet mapped to the device, reset it
		if (currentMap[i] != _channelMap[i]) {
#ifdef DEBUG_REMAP
			debug(" Mapping (dontRemap) song %d, channel %d to device channel %d", songIndex, _channelMap[i]._channel, i);
#endif
			if (mainThread) _channelMap[i]._song->pMidiParser->mainThreadBegin();
			_channelMap[i]._song->pMidiParser->remapChannel(_channelMap[i]._channel, i);
			if (mainThread) _channelMap[i]._song->pMidiParser->mainThreadEnd();
		}

	}

	// Next, we look for channels which were already playing.
	// We keep those on the same device channel as before.
	for (int i = 0; i < 16; ++i) {

		if (!map->_map[i]._song)
			continue;

		songIndex = -1;
		for (MusicList::iterator iter = _playList.begin(); iter != end; ++iter) {
			songIndex++;
			if (map->_map[i]._song == *iter)
				break;
		}


		for (int j = 0; j < 16; ++j) {
			if (map->_map[i] == currentMap[j]) {
				// found it
				_channelMap[j] = map->_map[i];
				map->_map[i]._song = 0; // mark as done
#ifdef DEBUG_REMAP
				debug(" Keeping song %d, channel %d on device channel %d", songIndex, _channelMap[j]._channel, j);
#endif
				break;
			}
		}
	}

	// Then, remap the rest.
	for (int i = 0; i < 16; ++i) {

		if (!map->_map[i]._song || !map->_map[i]._song->pMidiParser)
			continue;

		songIndex = -1;
		for (MusicList::iterator iter = _playList.begin(); iter != end; ++iter) {
			songIndex++;
			if (map->_map[i]._song == *iter)
				break;
		}

		for (int j = _driverLastChannel; j >= _driverFirstChannel; --j) {
			if (_channelMap[j]._song == 0) {
				_channelMap[j] = map->_map[i];
				map->_map[i]._song = 0;
#ifdef DEBUG_REMAP
				debug(" Mapping song %d, channel %d to device channel %d", songIndex, _channelMap[j]._channel, j);
#endif
				if (mainThread) _channelMap[j]._song->pMidiParser->mainThreadBegin();
				_channelMap[j]._song->pMidiParser->remapChannel(_channelMap[j]._channel, j);
				if (mainThread) _channelMap[j]._song->pMidiParser->mainThreadEnd();
				break;
			}
		}

	}

	// And finally, stop any empty channels
	for (int i = _driverLastChannel; i >= _driverFirstChannel; --i) {
		if (!_channelMap[i]._song && currentMap[i]._song)
			resetDeviceChannel(i, mainThread);
	}

	delete map;
}


ChannelRemapping *SciMusic::determineChannelMap() {
#ifdef DEBUG_REMAP
	debug("Remap: avail chans: %d-%d", _driverFirstChannel, _driverLastChannel);
#endif

	ChannelRemapping *map = new ChannelRemapping;
	ChannelRemapping backupMap;
	map->clear();
	map->_freeVoices = _pMidiDrv->getPolyphony();

	if (_playList.empty())
		return map;

	// TODO: set reverb, either from first song, or from global???

	MusicList::iterator songIter;
	int songIndex = -1;
	for (songIter = _playList.begin(); songIter != _playList.end(); ++songIter) {
		songIndex++;
		MusicEntry *song = *songIter;
		if (song->status != kSoundPlaying)
			continue;

		// If song is digital, skip.
		// CHECKME: Is this condition correct?
		if (!song->pMidiParser) {
#ifdef DEBUG_REMAP
			debug(" Song %d (%p), digital?", songIndex, (void*)song);
#endif
			continue;
		}


#ifdef DEBUG_REMAP
		const char* name = g_sci->getEngineState()->_segMan->getObjectName(song->soundObj);
		debug(" Song %d (%p) [%s], prio %d%s", songIndex, (void*)song, name, song->priority, song->playBed ? ", bed" : "");
#endif

		// Store backup. If we fail to map this song, we will revert to this.
		backupMap = *map;

		bool songMapped = true;

		for (int i = 0; i < 16; ++i) {
			int c = song->_usedChannels[i];
			if (c == 0xFF || c == 0xFE || c == 0x0F)
				continue;
			const MusicEntryChannel &channel = song->_chan[c];
			if (channel._dontMap) {
#ifdef DEBUG_REMAP
				debug("  Channel %d dontMap, skipping", c);
#endif
				continue;
			}
			if (channel._mute) {
#ifdef DEBUG_REMAP
				debug("  Channel %d muted, skipping", c);
#endif
				continue;
			}

			bool dontRemap = channel._dontRemap || song->playBed;

#ifdef DEBUG_REMAP
			debug("  Channel %d: prio %d, %d voice%s%s", c, channel._prio, channel._voices, channel._voices == 1 ? "" : "s", dontRemap ? ", dontRemap" : "" );
#endif

			DeviceChannelUsage dc = { song, c };

			// our target
			int devChannel = -1;

			if (dontRemap && map->_map[c]._song == 0) {
				// unremappable channel, with channel still free
				devChannel = c;
			}

			// try to find a free channel
			if (devChannel == -1) {
				for (int j = 0; j < 16; ++j) {
					if (map->_map[j] == dc) {
						// already mapped?! (Can this happen?)
						devChannel = j;
						break;
					}
					if (map->_map[j]._song)
						continue;

					if (j >= _driverFirstChannel && j <= _driverLastChannel)
						devChannel = j;
				}
			}

			int prio = channel._prio;
			if (prio > 0) {
				// prio > 0 means non-essential
				prio = (16 - prio) + 16*songIndex;
			}

			if (devChannel == -1 && prio > 0) {
				// no empty channel, but this isn't an essential channel,
				// so we just skip it.
#ifdef DEBUG_REMAP
				debug("   skipping non-essential");
#endif
				continue;
			}

			// try to empty a previous channel if this is an essential channel
			if (devChannel == -1) {
				devChannel = map->lowestPrio();
				if (devChannel != -1)
					map->evict(devChannel);
			}

			if (devChannel == -1) {
				// failed to map this song.
#ifdef DEBUG_REMAP
				debug("   no free (or lower priority) channel found");
#endif
				songMapped = false;
				break;
			}

			if (map->_map[devChannel] == dc) {
				// already mapped?! (Can this happen?)
				continue;
			}

			int neededVoices = channel._voices;
			// do we have enough free voices?
			if (map->_freeVoices < neededVoices) {
				// We only care for essential channels.
				// Note: In early SCI1 interpreters, a song started by 'playBed'
				// would not be skipped even if some channels couldn't be
				// mapped due to voice limits. So, we treat all channels as
				// non-essential here for playBed songs.
				if (prio > 0 || (song->playBed && _soundVersion <= SCI_VERSION_1_EARLY)) {
#ifdef DEBUG_REMAP
					debug("   not enough voices; need %d, have %d. Skipping this channel.", neededVoices, map->_freeVoices);
#endif
					continue;
				}
				do {
					int j = map->lowestPrio();
					if (j == -1) {
#ifdef DEBUG_REMAP
						debug("   not enough voices; need %d, have %d", neededVoices, map->_freeVoices);
#endif
						// failed to free enough voices.
						songMapped = false;
						break;
					}
#ifdef DEBUG_REMAP
					debug("   creating room for voices; evict %d", j);
#endif
					map->evict(j);
				} while (map->_freeVoices < neededVoices);

				if (!songMapped) {
					// failed to map this song.
					break;
				}
			}

			// We have a channel and enough free voices now.
#ifdef DEBUG_REMAP
			debug("   trying to map to %d", devChannel);
#endif

			map->_map[devChannel] = dc;
			map->_voices[devChannel] = neededVoices;
			map->_prio[devChannel] = prio;
			map->_dontRemap[devChannel] = dontRemap;
			map->_freeVoices -= neededVoices;

			if (!dontRemap || devChannel == c) {
				// If this channel fits here, we're done.
#ifdef DEBUG_REMAP
				debug("    OK");
#endif
				continue;
			}

			// If this channel can't be remapped, we need to move it or fail.

			if (!map->_dontRemap[c]) {
				// Target channel can be remapped, so just swap
				map->swap(devChannel, c);
				continue;
			}
#ifdef DEBUG_REMAP
			debug("    but %d is already dontRemap", c);
#endif

			if (prio > 0) {
				// Channel collision, but this channel is non-essential,
				// so drop it.
				// TODO: Maybe we should have checked this before making room?
				map->evict(devChannel);
				continue;
			}

			if (map->_prio[c] > 0) {
				// Channel collision, but the other channel is non-essential,
				// so we take its place.
				map->evict(c);
				map->swap(devChannel, c);
				continue;
			}

			// Otherwise, we have two essential channels claiming the same
			// device channel.
			songMapped = false;
			break;
		}

		if (!songMapped) {
			// We failed to map this song, so unmap all its channels.
#ifdef DEBUG_REMAP
			debug(" Failed song");
#endif
			*map = backupMap;
		}
	}

	return map;
}

void SciMusic::resetDeviceChannel(int devChannel, bool mainThread) {
	assert(devChannel >= 0 && devChannel <= 0x0F);

	if (mainThread) {
		putMidiCommandInQueue(0x0040B0 | devChannel); // sustain off
		putMidiCommandInQueue(0x007BB0 | devChannel); // notes off
		putMidiCommandInQueue(0x004BB0 | devChannel); // release voices
	} else {
		_pMidiDrv->send(0x0040B0 | devChannel); // sustain off
		_pMidiDrv->send(0x007BB0 | devChannel); // notes off
		_pMidiDrv->send(0x004BB0 | devChannel); // release voices
	}
}



} // End of namespace Sci
