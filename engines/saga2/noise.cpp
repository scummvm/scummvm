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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "common/config-manager.h"
#include "audio/mixer.h"

#include "saga2/saga2.h"
#include "saga2/fta.h"
#include "saga2/audio.h"
#include "saga2/audiores.h"
#include "saga2/audiodec.h"
#include "saga2/audiofnc.h"
#include "saga2/annoy.h"
#include "saga2/player.h"
#include "saga2/queues.h"
#include "saga2/audiosmp.h"
#include "saga2/audqueue.h"
#include "saga2/audiosys.h"
#include "saga2/hresmgr.h"
#include "saga2/shorten.h"

namespace Saga2 {

//#define AUDIO_DISABLED

#define AUDIO_REFRESH_RATE 5

static const StaticPoint32 VeryFarAway = {32767, 32766};

const uint32 fullVolumeDist = 75;
const uint32 offVolumeDist = 200;

const uint32        baseMusicID     = MKTAG('M', 'I', 'L', 'O'),
                    goodMusicID     = MKTAG('M', 'I', 'H', 'I'),
                    soundID         = MKTAG('L', 'O', 'U', 'D'),
                    loopedID        = MKTAG('L', 'O', 'O', 'P'),
                    voiceID         = MKTAG('T', 'A', 'L', 'K');

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern audioInterface   *audio;
extern hResource        *soundResFile;          // script resources
extern hResource        *voiceResFile;          // script resources

extern int32            clickSizes[];
extern uint8            *clickData[];


/* ===================================================================== *
   Globals
 * ===================================================================== */

soundSegment            currentMidi;
soundSegment            currentLoop;

//-----------------------------------------------------------------------
//	decoder sets

decoderSet              *voiceDec,
                        *soundDec,
                        *longSoundDec,
                        *musicDec,
                        *loopDec,
                        *memDec;

//-----------------------------------------------------------------------
//	resource contexts

hResContext         *voiceRes,
                    *musicRes,
                    *soundRes,
                    *loopRes,
                    *longRes;


/* ===================================================================== *
   Locals
 * ===================================================================== */

static audioAttenuationFunction oldAttenuator;

/* ===================================================================== *
   Prototypes & inlines
 * ===================================================================== */

#define killIt(p)  if (p) delete p; p=NULL
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
   Decoder routines
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Specific DECODER routines

BUFFERLOD(seekMusic)   {
	return hResSeek(sb, ss, musicRes, true);
}
BUFFERLOD(readMusic)   {
	return hResRead(sb, ss, musicRes);
}
BUFFERLOD(flushMusic)  {
	return hResFlush(sb, ss, musicRes);
}
BUFFERLOD(seekLongSound)   {
	return hResSeek(sb, ss, longRes, false);
}
BUFFERLOD(readLongSound)   {
	return hResRead(sb, ss, longRes);
}
BUFFERLOD(flushLongSound)  {
	return hResFlush(sb, ss, longRes);
}
BUFFERLOD(seekSound)   {
	return hResSeek(sb, ss, soundRes, true);
}
BUFFERLOD(readSound)   {
	return hResRead(sb, ss, soundRes);
}
BUFFERLOD(flushSound)  {
	return hResFlush(sb, ss, soundRes);
}
BUFFERLOD(seekLoop)   {
	return hResSeek(sb, ss, loopRes, true);
}
BUFFERLOD(readLoop)   {
	return hResRead(sb, ss, loopRes);
}
BUFFERLOD(flushLoop)  {
	return hResFlush(sb, ss, loopRes);
}
BUFFERLOD(seekVoice)   {
	return hResSeek(sb, ss, voiceRes, false);
}
BUFFERLOD(readVoice)   {
	return hResRead(sb, ss, voiceRes);
}
BUFFERLOD(flushVoice)  {
	return hResFlush(sb, ss, voiceRes);
}
BUFFERLOD(seekMemSound)   {
	return bufSeek(sb, ss);
}
BUFFERLOD(readMemSound)   {
	return bufRead(sb, ss);
}
BUFFERLOD(flushMemSound)  {
	return bufFlush(sb, ss);
}

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
	audioInterfaceSettings audioBufferSizes = audioInterfaceSettings(
	            (int16)  2,       // number of sound buffers
	            (uint32) 32768,   // voice buffer size   32k
	            (uint32) 65536,   // music buffer size   64k
	            (uint32) 131072,   // sound buffer size 128k
	            (uint32) 400000    // sound buffer size
	        );

	bool disVoice = true, disMusic = true, disSound = true, disLoops = true;

	if (audio->active()) {
		voiceDec = new decoderSet();
		voiceDec->addDecoder(new soundDecoder(&readVoice, &seekVoice, &flushVoice));
		voiceDec->addDecoder(new soundDecoder(&readDecompress, &seekDecompress, &flushDecompress, 16384, audio, 1));

		musicDec = new decoderSet();
		musicDec->addDecoder(new soundDecoder(&readMusic, &seekMusic, &flushMusic));

		soundDec = new decoderSet();
		soundDec->addDecoder(new soundDecoder(&readSound, &seekSound, &flushSound));

		longSoundDec = new decoderSet();
		longSoundDec->addDecoder(new soundDecoder(&readLongSound, &seekLongSound, &flushLongSound));

		loopDec = new decoderSet();
		loopDec->addDecoder(new soundDecoder(&readLoop, &seekLoop, &flushLoop));

		memDec = new decoderSet();
		memDec->addDecoder(new soundDecoder(&readMemSound, &seekMemSound, &flushMemSound));

		uint32 musicID = haveKillerSoundCard() ? goodMusicID : baseMusicID;

		disVoice = !ConfMan.getInt("speech_volume");
		disMusic = !ConfMan.getInt("music_volume");
		disSound = !ConfMan.getInt("sfx_volume");
		disLoops = disSound;

		if (!disMusic) {
			musicRes = soundResFile->newContext(musicID, "music resource");
			if (musicRes == NULL)
				error("Musicians on Strike (No music resource context)!\n");
		}

		if (!disSound) {
			soundRes = soundResFile->newContext(soundID, "sound resource");
			if (soundRes == NULL)
				error("No sound effect resource context!\n");
		}

		if (!disSound) {
			longRes = soundResFile->newContext(soundID, "long sound resource");
			if (longRes == NULL)
				error("No sound effect resource context!\n");
		}

		if (!disLoops) {
			loopRes = soundResFile->newContext(loopedID, "loops resource");
			if (loopRes == NULL)
				error("No loop effect resource context!\n");
		}

		if (!disVoice) {
			voiceRes = voiceResFile->newContext(voiceID, "voice resource");
			if (voiceRes == NULL)
				error("Laryngitis Error (No voice resource context)!\n");
		}

		audio->initAudioInterface(audioBufferSizes);
		//audio->setMusicFadeStyle(15,15,5);
		audio->setMusicFadeStyle(0, 0, 0);
		oldAttenuator = audio->setAttenuator(&volumeFromDist);
	}


	if (audio->activeDIG()) {
		// kludgy in memory click sounds
		clickSizes[0] = 0;
		clickSizes[1] = soundRes->size(MKTAG('C', 'L', 'K', 1));
		clickSizes[2] = soundRes->size(MKTAG('C', 'L', 'K', 2));
		clickData[0] = NULL;
		clickData[1] = (uint8 *) LoadResource(soundRes, MKTAG('C', 'L', 'K', 1), "Click 1");
		clickData[2] = (uint8 *) LoadResource(soundRes, MKTAG('C', 'L', 'K', 2), "Click 2");
	}

	if (disMusic)
		audio->disable(volMusic);
	if (disVoice)
		audio->disable(volVoice);
	if (disLoops)
		audio->disable(volLoops);
	if (disSound)
		audio->disable(volSound);
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
//	hook used by videos

HDIGDRIVER &digitalAudioDriver(void) {
	assert(audio);
	return audio->dig;
}

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

	Common::SeekableReadStream *stream = loadResourceToStream(musicRes, s, "music data");
	Common::DumpFile out;

	Common::String path = Common::String::format("./dumps/mus%s.mus", tag2strP(s));

	if (out.open(path, true)) {
		int size = musicRes->getSize(s, "music data");
		byte *buf = (byte *)malloc(size);
		stream->read(buf, size);
		out.write(buf, size);
		out.flush();
		out.close();
	}

	delete stream;


	if (hResCheckResID(musicRes, s)) {
		audio->queueMusic(s, musicDec, 0);
	} else
		audio->stopMusic();
}

//-----------------------------------------------------------------------
// in memory sfx

void playMemSound(uint32 s) {
	debugC(1, kDebugSound, "playMemSound(%s)", tag2strP(s));

	if (bufCheckResID(NULL, s))
		audio->queueSound(s, memDec, 1, Here);
}

//-----------------------------------------------------------------------
// on disk sfx

void playSound(uint32 s) {
	debugC(1, kDebugSound, "playSound(%s)", tag2strP(s));

	if (hResCheckResID(soundRes, s))
		audio->queueSound(s, soundDec, 1, Here);
}

//-----------------------------------------------------------------------
// on disk sfx (x2 buffered)

void playLongSound(uint32 s) {
	debugC(1, kDebugSound, "playLongSound(%s)", tag2strP(s));

	if (hResCheckResID(longRes, s))
		audio->queueVoice(s, longSoundDec);
	else
		audio->stopVoice();
}

//-----------------------------------------------------------------------
// on disk voice (x2 buffered)

void playVoice(uint32 s) {
	debugC(1, kDebugSound, "playVoice(%s)", tag2strP(s));

	if (hResCheckResID(voiceRes, s)) {
		if (s)
			audio->queueVoice(s, voiceDec, Here);
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
		audio->queueVoice(s, voiceDec, Here);
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

	Common::hexdump(data, MIN(size, 256U));
	audio->queueLoop(s, loopDec, 0, Here);
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
		audio->queueSound(s, soundDec, 1, p);
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

	audio->queueVoice(s, voiceDec, p);

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
		audio->queueLoop(s, loopDec, 0, loc);
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

	return audio->queue.isSpeechPlaying(sampno);
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



/* ===================================================================== *
   DEBUGGING & TESTING
 * ===================================================================== */

#if DEBUG

//-----------------------------------------------------------------------
//	Test events

//-----------------------------------------------------------------------
//  This call annoyingly says "o-pen" or "cloooose" when using doors
//-----------------------------------------------------------------------

int annoyingTestSound(int32 sampID) {
	if (debugStatuses) {
		WriteStatusF(6, "Queued sound : %X ", MKTAG('T', 'S', 'T', sampID));
	}
	playSound(MKTAG('S', 'F', 'X', sampID));
	return 0;
}

int annoyingTestSound2(int32 sampID) {
	playSound(MKTAG('T', 'S', 'T', sampID));
	return 0;
}

//-----------------------------------------------------------------------
//  This call annoyingly says "o-pen" or "cloooose" when using doors
//-----------------------------------------------------------------------

int annoyingTestVoice(int32 sampID) {
	playVoice(MKTAG('T', 'S', 'T', sampID));
	return 0;
}

//-----------------------------------------------------------------------
//  This call annoyingly plays cheesy music when 'M' is hit
//-----------------------------------------------------------------------

int annoyingTestMusic(int32 sampID) {
	//playMusic(MKTAG('M', 'I', 'D', sampID));
	playMusic(MKTAG('X', 'M', 'I', sampID));
	return 0;
}

static char convBuf[5];

inline uint32 extendID(int16 smallID) {
	sprintf(convBuf, "%4.4d", smallID);
	return smallID ? MKTAG(convBuf[0] + 'A' - '0', convBuf[1], convBuf[2], convBuf[3]) : 0 ;
}

int16 aResponse[20] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9,
	1234, 1237, 1255, 1264, 1268, 1273, 0, 0, 0, 0, 9999
};

void voiceTest1(void) {
	playVoice(extendID(aResponse[rand() % 20]));
}

void voiceTest2(void) {
	int16 i = rand() % 8;
	switch (i) {
	case 0:
		playLoop(0);
		break;
	case 1:
		PlayLoop("TER:1");
		break;
	case 2:
		PlayLoop("TER:6");
		break;
	case 3:
		PlayLoop("TER:5");
		break;
	case 4:
		PlayLoop(":0");
		break;
	case 5:
		playLoop(MKTAG('T', 'E', 'R', 2));
		break;
	case 6:
		playLoop(MKTAG('T', 'E', 'R', 3));
		break;
	case 7:
		playLoop(MKTAG('T', 'E', 'R', 8));
		break;
	}
}

void soundTest1(void) {
	int16 i = rand() % 8;
	switch (i) {
	case 0:
		playSound(0);
		break;
	case 1:
		playSound(MKTAG('S', 'F', 'X', 5));
		break;
	case 2:
		playSound(MKTAG('S', 'F', 'X', 8));
		break;
	case 3:
		playSound(MKTAG('S', 'F', 'X', 20));
		break;
	case 4:
		PlaySound("SFX:11");
		break;
	case 5:
		PlaySound("SFX:15");
		break;
	case 6:
		playSound(MKTAG('S', 'F', 'X', 3));
		break;
	case 7:
		playSound(MKTAG('B', 'A', 'D', 47)); // put down a card
	}
}

void soundTest2(void) {
	int16 i = rand() % 8;
	switch (i) {
	case 0:
		playMusic(0);
		break;
	case 1:
		PlayMusic(":0");
		break;
	case 2:
		playMusic(MKTAG('X', 'M', 'I', 1));
		break;
	case 3:
		playMusic(MKTAG('X', 'M', 'I', 2));
		break;
	case 4:
		playMusic(MKTAG('X', 'M', 'I', 3));
		break;
	case 5:
		PlayMusic("XMI:1");
		break;
	case 6:
		PlayMusic("XMI:2");
		break;
	case 7:
		PlayMusic("BAD:3");
		break;
	}
}

void queueRandSound(void) {
	int16 i = rand() % 4;
	switch (i) {
	case 0:
		soundTest1();
		break;
	case 1:
		soundTest2();
		break;
	case 2:
		voiceTest1();
		break;
	case 3:
		voiceTest2();
		break;
	}

}

int testingAudio = 0;
static int32 nextone = 0;
static uint32 lastdelta = 0;

void audioStressTest(void) {
	if (randomAudioTesting) {
		if (gameTime > nextone) {
			if (audioVerbosity > 3) {
				char msg[80];
				sprintf(msg, "%d tick interval\n", lastdelta);
				audioLog(msg);
			}
			queueRandSound();
			lastdelta = (rand() % 1000);
			nextone = gameTime + lastdelta;
		}
	}
}


#endif //DEBUG

} // end of namespace Saga2
