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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "common/config-manager.h"

#include "saga2/saga2.h"
#include "saga2/audio.h"
#include "saga2/fta.h"
#include "saga2/shorten.h"
#include "saga2/hresmgr.h"
#include "saga2/music.h"
#include "saga2/annoy.h"
#include "saga2/player.h"

namespace Saga2 {

audioInterface *audio;

static const StaticPoint32 VeryFarAway = {32767, 32766};

const uint32 fullVolumeDist = 75;
const uint32 offVolumeDist = 200;

const uint32        baseMusicID     = MKTAG('M', 'I', 'L', 'O'),
                    goodMusicID     = MKTAG('M', 'I', 'H', 'I'),
                    soundID         = MKTAG('L', 'O', 'U', 'D'),
                    loopedID        = MKTAG('L', 'O', 'O', 'P'),
                    voiceID         = MKTAG('T', 'A', 'L', 'K');

extern hResource        *soundResFile;          // script resources
extern hResource        *voiceResFile;          // script resources

extern int32            clickSizes[];
extern uint8            *clickData[];


soundSegment            currentMidi;
soundSegment            currentLoop;

hResContext         *voiceRes,
                    *musicRes,
                    *soundRes,
                    *loopRes,
                    *longRes;


static audioAttenuationFunction oldAttenuator;

bool haveKillerSoundCard(void);
void writeConfig(void);
void disableBGLoop(bool s = true);
void enableBGLoop(void);
void audioStressTest(void);
extern GameObject *getViewCenterObject(void);
void playSoundAt(uint32 s, Location playAt);
void playSoundAt(uint32 s, Point32 playAt);
bool sayVoiceAt(uint32 s[], Location l);
bool sayVoiceAt(uint32 s[], Point32 l);
void playLoopAt(uint32 s, Location l);
void playLoopAt(uint32 s, Point32 l);

bool bufCheckResID(hResContext *hrc, uint32 s);
bool hResCheckResID(hResContext *hrc, uint32 s);
bool hResCheckResID(hResContext *hrc, uint32 s[]);

/* ===================================================================== *
   ATTENUATOR routines
 * ===================================================================== */

//-----------------------------------------------------------------------
//	our distance based volume attenuator

static ATTENUATOR(volumeFromDist) {
	TilePoint tp(loc.x, loc.y, 0);
	uint32 dist = tp.quickHDistance();
	if (dist < fullVolumeDist) {
		return abs(maxVol);
	} else if (dist < offVolumeDist) {
		return abs((int)(maxVol * ((int)((offVolumeDist - fullVolumeDist) - (dist - fullVolumeDist))) / (offVolumeDist - fullVolumeDist)));
	}
	return 0;
}

/* ===================================================================== *
   Module code
 * ===================================================================== */

//-----------------------------------------------------------------------
//	after system initialization - startup code

void startAudio(void) {
	uint32 musicID = haveKillerSoundCard() ? goodMusicID : baseMusicID;

	musicRes = soundResFile->newContext(musicID, "music resource");
	if (musicRes == NULL)
		error("Musicians on Strike (No music resource context)!\n");

	soundRes = soundResFile->newContext(soundID, "sound resource");
	if (soundRes == NULL)
		error("No sound effect resource context!\n");

	longRes = soundResFile->newContext(soundID, "long sound resource");
	if (longRes == NULL)
		error("No sound effect resource context!\n");

	loopRes = soundResFile->newContext(loopedID, "loops resource");
	if (loopRes == NULL)
		error("No loop effect resource context!\n");

	voiceRes = voiceResFile->newContext(voiceID, "voice resource");
	if (voiceRes == NULL)
		error("Laryngitis Error (No voice resource context)!\n");

	audio->initAudioInterface(musicRes);
	audio->setMusicFadeStyle(0, 0, 0);
	oldAttenuator = audio->setAttenuator(&volumeFromDist);

	// kludgy in memory click sounds
	clickSizes[0] = 0;
	clickSizes[1] = soundRes->size(MKTAG('C', 'L', 'K', 1));
	clickSizes[2] = soundRes->size(MKTAG('C', 'L', 'K', 2));
	clickData[0] = NULL;
	clickData[1] = (uint8 *)LoadResource(soundRes, MKTAG('C', 'L', 'K', 1), "Click 1");
	clickData[2] = (uint8 *)LoadResource(soundRes, MKTAG('C', 'L', 'K', 2), "Click 2");

	if (!ConfMan.getInt("music_volume"))
		audio->disable(volMusic);
	if (!ConfMan.getInt("speech_volume"))
		audio->disable(volVoice);
	if (!ConfMan.getInt("sfx_volume")) {
		audio->disable(volLoops);
		audio->disable(volSound);
	}
}

//-----------------------------------------------------------------------
// audio event loop


void audioEventLoop(void) {
	if (audio->playFlag())
		audio->playMe();

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

bool haveKillerSoundCard(void) {
	warning("STUB: haveKillerSoundCard()"); // Check here for sound card type
	return true;
}

//-----------------------------------------------------------------------
// unwritten music toggler

void toggleMusic(void) {
}

/* ===================================================================== *
   Audio hooks for videos
 * ===================================================================== */

//-----------------------------------------------------------------------
//  suspend & resume calls


void suspendLoops(void) {
	disableBGLoop(false);
}

void resumeLoops(void) {
	//if (audio->enabled(volLoops))
	if (loopRes)
		enableBGLoop();
}

void suspendMusic(void) {
	audioEnvironmentSuspend(true);
}

void resumeMusic(void) {
	//if (audio->enabled(volMusic))
	if (musicRes)
		audioEnvironmentSuspend(false);
}

void suspendAudio(void) {
	if (audio) {
		suspendMusic();
		suspendLoops();
		audio->suspend();
	}
}

void resumeAudio(void) {
	if (audio) {
		//if (audio->enabled(volSound)||audio->enabled(volVoice))
		if (soundRes != NULL || voiceRes != NULL) {
			audio->resume();
			resumeLoops();
			resumeMusic();
		}
	}
}

//-----------------------------------------------------------------------
//  UI volume change hook

void volumeChanged(void) {
	if (audio->getVolume(volLoops))
		resumeLoops();
	else
		suspendLoops();

	if (audio->getVolume(volMusic))
		resumeMusic();
	else
		suspendMusic();
}

/* ===================================================================== *
   main audio playback routines
 * ===================================================================== */

Point32 translateLocation(Location playAt) {
	GameObject *go = getViewCenterObject();
	Location cal = Location(go->getWorldLocation(), go->IDParent());

	if (playAt.context == cal.context) {
		Point32 p = Point32(playAt.u - cal.u, playAt.v - cal.v);
		return p;
	}
	return VeryFarAway;
}

//-----------------------------------------------------------------------
//	MIDI playback

void playMusic(uint32 s) {
	debugC(1, kDebugSound, "playMusic(%s)", tag2strP(s));

	currentMidi = s;

	if (hResCheckResID(musicRes, s)) {
		audio->playMusic(s, 0);
	} else
		audio->stopMusic();
}

//-----------------------------------------------------------------------
// in memory sfx

void playMemSound(uint32 s) {
	debugC(1, kDebugSound, "playMemSound(%s)", tag2strP(s));

	if (bufCheckResID(NULL, s))
		audio->queueSound(s, 1, Here);
}

//-----------------------------------------------------------------------
// on disk sfx

void playSound(uint32 s) {
	debugC(1, kDebugSound, "playSound(%s)", tag2strP(s));

	if (hResCheckResID(soundRes, s))
		audio->queueSound(s, 1, Here);
}

//-----------------------------------------------------------------------
// on disk sfx (x2 buffered)

void playLongSound(uint32 s) {
	debugC(1, kDebugSound, "playLongSound(%s)", tag2strP(s));

	if (hResCheckResID(longRes, s))
		audio->queueVoice(s);
	else
		audio->stopVoice();
}

//-----------------------------------------------------------------------
// on disk voice (x2 buffered)

void playVoice(uint32 s) {
	debugC(1, kDebugSound, "playVoice(%s)", tag2strP(s));

	if (hResCheckResID(voiceRes, s)) {
		if (s)
			audio->queueVoice(s, Here);
		else
			audio->stopVoice();
	}
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
		audio->queueVoice(s, Here);
		if (audio->talking())
			worked = true;
	}

	return worked;
}

//-----------------------------------------------------------------------
// main loop playback

void _playLoop(uint32 s) {
	warning("STUB: _playLoop(%s)", tag2strP(s));

	currentLoop = s;
	if (currentLoop == audio->currentLoop() && 0)
		return;

	audio->stopLoop();

	byte *data = loopRes->loadResource(s, "loop sound");
	uint32 size = loopRes->getSize(s, "loop sound");

	warning("Size: %d", size);

	Common::hexdump(data, MIN<uint>(size, 256));
	audio->queueLoop(s, 0, Here);
}

//-----------------------------------------------------------------------
// loop playback that disables background loops

void playLoop(uint32 s) {
	if (s) {
		//disableBGLoop(s);
	} else {
		_playLoop(s);
		//enableBGLoop();
	}
}


//-----------------------------------------------------------------------
// attenuated sound players

void playSoundAt(uint32 s, Point32 p) {
	debugC(1, kDebugSound, "playSoundAt(%s, %d,%d)", tag2strP(s), p.x, p.y);

	if (hResCheckResID(soundRes, s))
		audio->queueSound(s, 1, p);
}

void playSoundAt(uint32 s, Location playAt) {
	Point32 p = translateLocation(playAt);
	if (p != VeryFarAway)
		playSoundAt(s, p);
}

//-----------------------------------------------------------------------
// voice playback w/ attenuation

bool sayVoiceAt(uint32 s[], Point32 p) {
	debugCN(1, kDebugSound, "sayVoiceAt([%s", tag2strP(s[0]));

	for (uint32 i = 1; s[i]; i++)
		debugCN(1, kDebugSound, ", %s", tag2strP(s[i]));

	debugC(1, kDebugSound, "], %d,%d)", p.x, p.y);

	audio->queueVoice(s, p);

	return true;
}

bool sayVoiceAt(uint32 s[], Location playAt) {
	Point32 p = translateLocation(playAt);
	if (p != VeryFarAway)
		return sayVoiceAt(s, p);
	return false;
}

//-----------------------------------------------------------------------
// loop playback w/ attenuation

void playLoopAt(uint32 s, Point32 loc) {
	debugC(1, kDebugSound, "playLoopAt(%s, %d,%d)", tag2strP(s), loc.x, loc.y);

	if (hResCheckResID(loopRes, s))
		audio->queueLoop(s, 0, loc);
	else
		audio->stopLoop();
}

void addAuxTheme(Location loc, soundSegment lid);
void killAuxTheme(soundSegment lid);
void killAllAuxThemes(void);

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
	audio->setLoopPosition(loc);
}

void moveLoop(Location loc) {
	Point32 p = translateLocation(loc);
	if (p != VeryFarAway) {
		moveLoop(p);
	}
}

//-----------------------------------------------------------------------
// supplemental interface check for speech

bool stillDoingVoice(uint32 sampno) {
	warning("STUB: stillDoingVoice(%s)", tag2strP(sampno));

	return g_system->getMixer()->isSoundHandleActive(audio->_speechSoundHandle);
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
	assert(IDstr != NULL);
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
	if (IDstr == NULL)
		playSound(0);
	else
		playSound(parse_res_id(IDstr));
}

void PlayLongSound(char IDstr[]) {
	if (IDstr == NULL)
		playLongSound(0);
	else
		playLongSound(parse_res_id(IDstr));
}

void PlayVoice(char IDstr[]) {
	if (IDstr == NULL)
		playVoice(0);
	else
		playVoice(parse_res_id(IDstr));
}

void PlayLoop(char IDstr[]) {
	if (IDstr == NULL)
		playLoop(0);
	else
		playLoop(parse_res_id(IDstr));
}

void PlayLoopAt(char IDstr[], Location l) {
	if (IDstr == NULL)
		playLoop(0);
	else
		playLoopAt(parse_res_id(IDstr), l);
}

void PlayMusic(char IDstr[]) {
	warning("STUB: PlayMusic()");
	if (IDstr == NULL)
		playMusic(0);
	else
		playMusic(parse_res_id(IDstr));
}

////////////////////////////////////////////////////////////////

bool initAudio() {
	warning("STUB: initAudio()");
	audio = new audioInterface();
	return true;
}

void cleanupAudio() {
	warning("STUB: cleanupAudio()");
	delete audio;
}

audioInterface::audioInterface() {
	_music = nullptr;
}

audioInterface::~audioInterface() {
	delete _music;
}

void audioInterface::initAudioInterface(hResContext *musicContext) {
	_music = new Music(musicContext, g_system->getMixer());
}

void audioInterface::cleanupAudioInterface(void) {
	warning("STUB: audioInterface::cleanupAudioInterface()");
}

void audioInterface::suspendGameClock(void) {
	warning("STUB: audioInterace::suspendGameClock()");
}

void audioInterface::resumeGameClock(void) {
	warning("STUB: audioInterface::resumeGameClock()");
}

bool audioInterface::playFlag(void) {
	debugC(5, kDebugSound, "STUB: audioInterface::playFlag()");
	bool isSoundActive = g_system->getMixer()->isSoundHandleActive(_speechSoundHandle);
	return !isSoundActive && _speechQueue.size() > 0;
}

void audioInterface::playMe(void) {
	warning("STUB: audioInterface::PlayMe()");
	SoundInstance si = _speechQueue.pop();

	Common::SeekableReadStream *stream = loadResourceToStream(voiceRes, si.seg, "voice data");

	Audio::AudioStream *aud = makeShortenStream(*stream);

	g_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &_speechSoundHandle, aud);

	delete stream;
}

void audioInterface::playMusic(soundSegment s, int16 loopFactor, sampleLocation where) {
	warning("STUB: audioInterface::playMusic()");
	_music->play(s, loopFactor ? MUSIC_LOOP : MUSIC_NORMAL);
}

void audioInterface::stopMusic(void) {
	warning("STUB: audioInterface::stopMusic()");
	_music->stop();
}

void audioInterface::queueSound(soundSegment s, int16 loopFactor, sampleLocation where) {
	warning("STUB: audioInterface::queueSound(%s,  @%d,%d)", tag2strP(s), where.x, where.y);
	SoundInstance si;

	si.seg = s;
	si.loop = loopFactor;
	si.loc = where;

	_sfxQueue.push(si);
}

void audioInterface::queueLoop(soundSegment s, int16 loopFactor, sampleLocation where) {
	warning("STUB: audioInterface::queueLoop()");
}

void audioInterface::stopLoop(void) {
	warning("STUB: audioInterface::stopLoop()");
}

void audioInterface::setLoopPosition(sampleLocation newLoc) {
	warning("STUB: audioInterface::setLoopPosition(%d,%d)", newLoc.x, newLoc.y);
}

void audioInterface::queueVoice(soundSegment s, sampleLocation where) {
	warning("STUB: audioInterface::queueVoice(soundSegment, sampleLocation)");
	SoundInstance si;

	si.seg = s;
	si.loop = false;
	si.loc = where;

	_speechQueue.push(si);
}

void audioInterface::queueVoice(soundSegment s[], sampleLocation where) {
	warning("STUB: audioInterface::queueVoice(soundSegment [], sampleLocation)");
	SoundInstance si;

	soundSegment *p = s;
	while (*p) {
		si.seg = *p;
		si.loop = false;
		si.loc = where;

		_speechQueue.push(si);
		p++;
	}
}

void audioInterface::stopVoice(void) {
	warning("STUB: audioInterface::stopVoice()");
}

bool audioInterface::talking(void) {
	warning("STUB: audioInterface::talking()");
	return false;
}

bool audioInterface::saying(soundSegment s) {
	warning("STUB: audioInterface::saying()");
	return false;
}

bool audioInterface::active(void) {
	warning("STUB: audioInterface::active()");
	return true;
}

void audioInterface::enable(volumeTarget i, bool onOff) {
	warning("STUB: audioInterface::enable()");
}

void audioInterface::setVolume(volumeTarget targ, volumeMode op, Volume val) {
	warning("STUB: audioInterface::setVolume()");
}

Volume audioInterface::getVolume(volumeTarget src) {
	warning("STUB: audioInterface::getVolume()");
	return 0;
}

void audioInterface::setMusicFadeStyle(int16 tOut, int16 tIn, int16 tOver) {
	warning("STUB: audioInterface::setMusicFadeStyle()");
}

void audioInterface::suspend(void) {
	warning("STUB: audioInterface::suspend()");
}

void audioInterface::resume(void) {
	warning("STUB: audioInterface::resume()");
}

audioAttenuationFunction audioInterface::setAttenuator(audioAttenuationFunction newAF) {
	warning("STUB: audioInterface::setAttenuator()");
	return nullptr;
}

bool bufCheckResID(hResContext *hrc, uint32 s) {
	return s != 0;
}

bool hResCheckResID(hResContext *hrc, uint32 s) {
	if (hrc != NULL)
		return hrc->seek(s);
	return false;
}

bool hResCheckResID(hResContext *hrc, uint32 s[]) {
	if (s != NULL) {
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
