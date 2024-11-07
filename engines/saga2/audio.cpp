/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "common/config-manager.h"
#include "audio/audiostream.h"
#include "audio/mididrv.h"
#include "audio/decoders/raw.h"

#include "saga2/saga2.h"
#include "saga2/detection.h"
#include "saga2/audio.h"
#include "saga2/fta.h"
#include "saga2/shorten.h"
#include "saga2/hresmgr.h"
#include "saga2/music.h"
#include "saga2/annoy.h"
#include "saga2/player.h"

namespace Saga2 {

static const StaticPoint32 kVeryFarAway = {32767, 32766};

const uint32 kFullVolumeDist = 75;
const uint32 kOffVolumeDist = 200;

const uint32        baseMusicID     = MKTAG('M', 'I', 'L', 'O'),
                    goodMusicID     = MKTAG('M', 'I', 'H', 'I'),
                    soundID         = MKTAG('L', 'O', 'U', 'D'),
                    loopedID        = MKTAG('L', 'O', 'O', 'P'),
                    voiceID         = MKTAG('T', 'A', 'L', 'K');

extern hResource *soundResFile;          // script resources
extern hResource *voiceResFile;          // script resources

hResContext *voiceRes, *musicRes, *soundRes, *loopRes, *longRes;

bool haveKillerSoundCard();
void disableBGLoop(bool s = true);
void enableBGLoop();
void audioStressTest();
extern GameObject *getViewCenterObject();

bool bufCheckResID(hResContext *hrc, uint32 s);
bool hResCheckResID(hResContext *hrc, uint32 s);
bool hResCheckResID(hResContext *hrc, uint32 s[]);

/* ===================================================================== *
   ATTENUATOR routines
 * ===================================================================== */

//-----------------------------------------------------------------------
//	our distance based volume attenuator

static byte volumeFromDist(Point32 loc, byte maxVol) {
	TilePoint tp(loc.x, loc.y, 0);
	uint32 dist = tp.quickHDistance();
	if (dist < kFullVolumeDist) {
		return ABS(maxVol);
	} else if (dist < kOffVolumeDist) {
		return ABS((int)(maxVol * ((int)((kOffVolumeDist - kFullVolumeDist) - (dist - kFullVolumeDist))) / (kOffVolumeDist - kFullVolumeDist)));
	}
	return 0;
}

/* ===================================================================== *
   Module code
 * ===================================================================== */

//-----------------------------------------------------------------------
//	after system initialization - startup code

void startAudio() {
	uint32 musicID = haveKillerSoundCard() ? goodMusicID : baseMusicID;

	musicRes = soundResFile->newContext(musicID, "music resource");
	if (musicRes == nullptr)
		error("Musicians on Strike (No music resource context)!\n");

	soundRes = soundResFile->newContext(soundID, "sound resource");
	if (soundRes == nullptr)
		error("No sound effect resource context!\n");

	longRes = soundResFile->newContext(soundID, "long sound resource");
	if (longRes == nullptr)
		error("No sound effect resource context!\n");

	loopRes = soundResFile->newContext(loopedID, "loops resource");
	if (loopRes == nullptr)
		error("No loop effect resource context!\n");

	voiceRes = voiceResFile->newContext(voiceID, "voice resource");
	if (voiceRes == nullptr)
		error("Laryngitis Error (No voice resource context)!\n");

	g_vm->_audio->initAudioInterface(musicRes);

	if (g_vm->getGameId() == GID_FTA2) {
		// kludgy in memory click sounds
		g_vm->_audio->_clickSizes[0] = 0;
		g_vm->_audio->_clickSizes[1] = soundRes->size(MKTAG('C', 'L', 'K', 1));
		g_vm->_audio->_clickSizes[2] = soundRes->size(MKTAG('C', 'L', 'K', 2));
		g_vm->_audio->_clickData[0] = nullptr;
		g_vm->_audio->_clickData[1] = (uint8 *)LoadResource(soundRes, MKTAG('C', 'L', 'K', 1), "Click 1");
		g_vm->_audio->_clickData[2] = (uint8 *)LoadResource(soundRes, MKTAG('C', 'L', 'K', 2), "Click 2");
	}
}

void cleanupAudio() {
	if (g_vm->_audio) {
		delete g_vm->_audio;
		g_vm->_audio = nullptr;

		delete musicRes;
		musicRes = nullptr;

		delete soundRes;
		soundRes = nullptr;

		delete longRes;
		longRes = nullptr;

		delete loopRes;
		loopRes = nullptr;

		delete voiceRes;
		voiceRes = nullptr;
	}
}

//-----------------------------------------------------------------------
// audio event loop


void audioEventLoop() {
	if (g_vm->_audio->playFlag())
		g_vm->_audio->playMe();

	audioEnvironmentCheck();
}

/* ===================================================================== *
   Assorted code
 * ===================================================================== */

void makeCombatSound(uint8 cs, Location l) {
	playSoundAt(MKTAG('C', 'B', 'T', cs), l);
}

void makeGruntSound(uint8 cs, Location l) {
	playSoundAt(MKTAG('G', 'N', 'T', cs), l);
}


//-----------------------------------------------------------------------
//	check for higher quality MIDI card

bool haveKillerSoundCard() {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	MusicType driverType = MidiDriver::getMusicType(dev);

	switch (driverType) {
	case MT_ADLIB:
	case MT_MT32:
		return true;

	default:
		return false;
	}
}

//-----------------------------------------------------------------------
// unwritten music toggler

void toggleMusic() {
}

/* ===================================================================== *
   Audio hooks for videos
 * ===================================================================== */

//-----------------------------------------------------------------------
//  suspend & resume calls


void suspendLoops() {
	disableBGLoop(false);
}

void resumeLoops() {
	if (loopRes)
		enableBGLoop();
}

void suspendMusic() {
	audioEnvironmentSuspend(true);
}

void resumeMusic() {
	if (musicRes)
		audioEnvironmentSuspend(false);
}

void suspendAudio() {
	if (g_vm->_audio) {
		suspendMusic();
		suspendLoops();
		g_vm->_audio->suspend();
	}
}

void resumeAudio() {
	if (g_vm->_audio) {
		if (soundRes != nullptr || voiceRes != nullptr) {
			g_vm->_audio->resume();
			resumeLoops();
			resumeMusic();
		}
	}
}

//-----------------------------------------------------------------------
//  UI volume change hook

void volumeChanged() {
	if (g_vm->_audio->getVolume(kVolSfx))
		resumeLoops();
	else
		suspendLoops();

	if (g_vm->_audio->getVolume(kVolMusic)) {
		resumeMusic();

		g_vm->_audio->_music->syncSoundSettings();
	} else
		suspendMusic();
}

/* ===================================================================== *
   main audio playback routines
 * ===================================================================== */

Point32 translateLocation(Location playAt) {
	GameObject *go = getViewCenterObject();
	Location cal = Location(go->getWorldLocation(), go->IDParent());

	if (playAt._context == cal._context) {
		Point32 p = Point32(playAt.u - cal.u, playAt.v - cal.v);
		return p;
	}
	return kVeryFarAway;
}

//-----------------------------------------------------------------------
//	MIDI playback

void playMusic(uint32 s) {
	debugC(1, kDebugSound, "playMusic(%s)", tag2strP(s));

	if (hResCheckResID(musicRes, s)) {
		g_vm->_audio->playMusic(s, 1);
	} else
		g_vm->_audio->stopMusic();
}

//-----------------------------------------------------------------------
// in memory sfx

void playMemSound(uint32 s) {
	debugC(1, kDebugSound, "playMemSound(%s)", tag2strP(s));

	Audio::AudioStream *aud = Audio::makeRawStream(g_vm->_audio->_clickData[s], g_vm->_audio->_clickSizes[s], 22050, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN, DisposeAfterUse::NO);

	g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &g_vm->_audio->_clickSoundHandle, aud);
}

//-----------------------------------------------------------------------
// on disk sfx

void playSound(uint32 s) {
	debugC(1, kDebugSound, "playSound(%s)", tag2strP(s));

	if (hResCheckResID(soundRes, s))
		g_vm->_audio->queueSound(s, 1, Here);
}

//-----------------------------------------------------------------------
// on disk sfx (x2 buffered)

void playLongSound(uint32 s) {
	debugC(1, kDebugSound, "playLongSound(%s)", tag2strP(s));

	if (hResCheckResID(longRes, s))
		g_vm->_audio->queueVoice(s);
	else
		g_vm->_audio->stopVoice();
}

//-----------------------------------------------------------------------
// on disk voice (x2 buffered)

void playVoice(uint32 s) {
	debugC(1, kDebugSound, "playVoice(%s)", tag2strP(s));

	if (s && hResCheckResID(voiceRes, s))
		g_vm->_audio->queueVoice(s, Here);
	else
		g_vm->_audio->stopVoice();
}

//-----------------------------------------------------------------------
// supplemental interface for speech

bool sayVoice(uint32 s[]) {
	debugCN(1, kDebugSound, "sayVoice([%s", tag2strP(s[0]));

	for (uint32 i = 1; s[i]; i++)
		debugCN(1, kDebugSound, ", %s", tag2strP(s[i]));

	debugC(1, kDebugSound, "])");

	bool worked = false;

	if (hResCheckResID(voiceRes, s)) {
		g_vm->_audio->queueVoice(s, Here);
		if (g_vm->_audio->talking())
			worked = true;
	}

	return worked;
}

//-----------------------------------------------------------------------
// main loop playback

void _playLoop(uint32 s) {
	if (s == g_vm->_audio->currentLoop())
		return;

	g_vm->_audio->stopLoop();

	if (!s)
		return;

	g_vm->_audio->playLoop(s, 0, Here);
}

//-----------------------------------------------------------------------
// loop playback that disables background loops

void playLoop(uint32 s) {
	if (s) {
	} else {
		_playLoop(s);
	}
}


//-----------------------------------------------------------------------
// attenuated sound players

void playSoundAt(uint32 s, Point32 p) {
	debugC(1, kDebugSound, "playSoundAt(%s, %d,%d)", tag2strP(s), p.x, p.y);

	if (hResCheckResID(soundRes, s))
		g_vm->_audio->queueSound(s, 1, p);
}

void playSoundAt(uint32 s, Location playAt) {
	Point32 p = translateLocation(playAt);
	if (p != kVeryFarAway)
		playSoundAt(s, p);
}

//-----------------------------------------------------------------------
// voice playback w/ attenuation

bool sayVoiceAt(uint32 s[], Point32 p) {
	debugCN(1, kDebugSound, "sayVoiceAt([%s", tag2strP(s[0]));

	for (uint32 i = 1; s[i]; i++)
		debugCN(1, kDebugSound, ", %s", tag2strP(s[i]));

	debugC(1, kDebugSound, "], %d,%d)", p.x, p.y);

	g_vm->_audio->queueVoice(s, p);

	return true;
}

bool sayVoiceAt(uint32 s[], Location playAt) {
	Point32 p = translateLocation(playAt);
	if (p != kVeryFarAway)
		return sayVoiceAt(s, p);
	return false;
}

//-----------------------------------------------------------------------
// loop playback w/ attenuation

void playLoopAt(uint32 s, Point32 loc) {
	debugC(1, kDebugSound, "playLoopAt(%s, %d,%d)", tag2strP(s), loc.x, loc.y);

	if (hResCheckResID(loopRes, s))
		g_vm->_audio->playLoop(s, 0, loc);
	else
		g_vm->_audio->stopLoop();
}

void addAuxTheme(Location loc, uint32 lid);
void killAuxTheme(uint32 lid);
void killAllAuxThemes();

void playLoopAt(uint32 s, Location playAt) {
	debugC(1, kDebugSound, "playLoopAt(%s, %d,%d,%d)", tag2strP(s), playAt.u, playAt.v, playAt.z);

	if (s) {
		addAuxTheme(playAt, s);
	} else {
		killAllAuxThemes();
	}
}

//-----------------------------------------------------------------------
// loop attenuation

void moveLoop(Point32 loc) {
	g_vm->_audio->setLoopPosition(loc);
}

void moveLoop(Location loc) {
	Point32 p = translateLocation(loc);
	if (p != kVeryFarAway) {
		moveLoop(p);
	}
}

//-----------------------------------------------------------------------
// supplemental interface check for speech

bool stillDoingVoice(uint32 sampno) {
	bool result = g_vm->_audio->saying(sampno);

	debugC(1, kDebugSound, "stillDoingVoice(%s) -> %d", tag2strP(sampno), result);

	return result;
}

bool stillDoingVoice(uint32 s[]) {
	uint32 *p = s;

	while (*p) {
		if (g_vm->_audio->saying(*p++))
			return true;
	}

	return false;
}

/* ===================================================================== *
   SAGA compatible audio playback routines
 * ===================================================================== */

//-----------------------------------------------------------------------
// derive an ID from SAGA string

uint32 parse_res_id(char IDstr[]) {
	uint32 a[5] = {0, 0, 0, 0, 0};
	uint32 a2;
	uint32 i, j;
	assert(IDstr != nullptr);
	if (strlen(IDstr)) {
		for (i = 0, j = 0; i < strlen(IDstr); i++) {
			if (IDstr[i] == ':') {
				a2 = atoi(IDstr + i + 1);
				return MKTAG(a[0], a[1], a[2], a2);
			} else {
				a[j++] = IDstr[i];
			}
		}
	}
	return MKTAG(a[0], a[1], a[2], a[3]);
}

//-----------------------------------------------------------------------
// playback aliases

void PlaySound(char IDstr[]) {
	if (IDstr == nullptr)
		playSound(0);
	else
		playSound(parse_res_id(IDstr));
}

void PlayLongSound(char IDstr[]) {
	if (IDstr == nullptr)
		playLongSound(0);
	else
		playLongSound(parse_res_id(IDstr));
}

void PlayVoice(char IDstr[]) {
	if (IDstr == nullptr)
		playVoice(0);
	else
		playVoice(parse_res_id(IDstr));
}

void PlayLoop(char IDstr[]) {
	if (IDstr == nullptr)
		playLoop(0);
	else
		playLoop(parse_res_id(IDstr));
}

void PlayLoopAt(char IDstr[], Location l) {
	if (IDstr == nullptr)
		playLoop(0);
	else
		playLoopAt(parse_res_id(IDstr), l);
}

void PlayMusic(char IDstr[]) {
	if (IDstr == nullptr)
		playMusic(0);
	else
		playMusic(parse_res_id(IDstr));
}

////////////////////////////////////////////////////////////////

bool initAudio() {
	g_vm->_audio = new AudioInterface();
	return true;
}

AudioInterface::AudioInterface() {
	_music = nullptr;
	_mixer = g_system->getMixer();

	memset(_clickSizes, 0, sizeof(_clickSizes));
	memset(_clickData, 0, sizeof(_clickData));
}

AudioInterface::~AudioInterface() {
	delete _music;
	free(_clickData[1]);
	free(_clickData[2]);
}

void AudioInterface::initAudioInterface(hResContext *musicContext) {
	_music = new Music(musicContext);
}

bool AudioInterface::playFlag() {
	debugC(5, kDebugSound, "AudioInterface::playFlag()");
	if (_speechQueue.size() == 0 && !_mixer->isSoundHandleActive(_speechSoundHandle))
		_currentSpeech.seg = 0;

	return _speechQueue.size() > 0 || _sfxQueue.size() > 0;
}

void AudioInterface::playMe() {
	if (_speechQueue.size() > 0 && !_mixer->isSoundHandleActive(_speechSoundHandle)) {
		SoundInstance si = _speechQueue.front();
		_speechQueue.pop_front();

		_currentSpeech = si;

		Common::SeekableReadStream *stream = loadResourceToStream(voiceRes, si.seg, "voice data");
		Audio::AudioStream *aud = makeShortenStream(*stream);
		byte vol = g_vm->_speechVoice ? volumeFromDist(si.loc, getVolume(kVolVoice)) : 0;

		_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechSoundHandle, aud, -1, vol);

		delete stream;
	}

	if (_sfxQueue.size() > 0) {
		SoundInstance si = _sfxQueue.pop();

		Common::SeekableReadStream *stream = loadResourceToStream(soundRes, si.seg, "sound data");
		Audio::AudioStream *aud = Audio::makeRawStream(stream, 22050, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN);
		byte vol = volumeFromDist(si.loc, getVolume(kVolSfx));

		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sfxSoundHandle, aud, -1, vol);
	}
}

void AudioInterface::playMusic(uint32 s, int16 loopFactor, Point32 where) {
	_music->play(s, loopFactor ? MUSIC_LOOP : MUSIC_NORMAL);

	_currentMusic.seg = s;
	_currentMusic.loop = loopFactor;
	_currentMusic.loc = where;
}

void AudioInterface::stopMusic() {
	_music->stop();
}

void AudioInterface::queueSound(uint32 s, int16 loopFactor, Point32 where) {
	SoundInstance si;

	si.seg = s;
	si.loop = loopFactor;
	si.loc = where;

	_sfxQueue.push(si);
}

void AudioInterface::playLoop(uint32 s, int16 loopFactor, Point32 where) {
	_currentLoop.seg = s;
	_currentLoop.loop = loopFactor;
	_currentLoop.loc = where;

	Common::SeekableReadStream *stream = loadResourceToStream(loopRes, s, "loop data");
	Audio::SeekableAudioStream *aud = Audio::makeRawStream(stream, 22050, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN);
	Audio::AudioStream *laud = Audio::makeLoopingAudioStream(aud, loopFactor);
	byte vol = volumeFromDist(where, getVolume(kVolSfx));

	_mixer->playStream(Audio::Mixer::kSFXSoundType, &g_vm->_audio->_loopSoundHandle, laud, -1, vol);
}

void AudioInterface::stopLoop() {
	_mixer->stopHandle(_loopSoundHandle);
}

void AudioInterface::setLoopPosition(Point32 newLoc) {
	if (_currentLoop.loc == newLoc)
		return;

	_currentLoop.loc = newLoc;
	byte vol = volumeFromDist(newLoc, getVolume(kVolSfx));

	_mixer->setChannelVolume(_loopSoundHandle, vol);
}

void AudioInterface::queueVoice(uint32 s, Point32 where) {
	SoundInstance si;

	si.seg = s;
	si.loop = false;
	si.loc = where;

	_speechQueue.push_back(si);
}

void AudioInterface::queueVoice(uint32 s[], Point32 where) {
	SoundInstance si;

	uint32 *p = s;
	while (*p) {
		si.seg = *p;
		si.loop = false;
		si.loc = where;

		_speechQueue.push_back(si);
		p++;
	}
}

void AudioInterface::stopVoice() {
	_mixer->stopHandle(_speechSoundHandle);
}

bool AudioInterface::talking() {
	return _mixer->isSoundHandleActive(_speechSoundHandle);
}

bool AudioInterface::saying(uint32 s) {
	if (_currentSpeech.seg == s)
		return true;

	for (Common::List<SoundInstance>::iterator it = _speechQueue.begin(); it != _speechQueue.end(); ++it)
		if ((*it).seg == s)
			return true;

	return false;
}

byte AudioInterface::getVolume(VolumeTarget src) {
	switch (src) {
	case kVolMusic:
		return ConfMan.getInt("music_volume");

	case kVolSfx:
		return ConfMan.getInt("sfx_volume");

	case kVolVoice:
		return ConfMan.getInt("speech_volume");
	}

	return 0;
}

void AudioInterface::suspend() {
	_mixer->pauseAll(true);
}

void AudioInterface::resume() {
	_mixer->pauseAll(false);
}

bool bufCheckResID(hResContext *hrc, uint32 s) {
	return s != 0;
}

bool hResCheckResID(hResContext *hrc, uint32 s) {
	if (hrc != nullptr)
		return hrc->seek(s);
	return false;
}

bool hResCheckResID(hResContext *hrc, uint32 s[]) {
	if (s != nullptr) {
		if (s[0] == 0)
			return false;

		for (int i = 0; s[i]; i++) {
			if (!hResCheckResID(hrc, s[i]))
				return false;
		}
	}
	return true;
}

} // end of namespace Saga2
