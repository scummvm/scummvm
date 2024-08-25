/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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
 */

#include "engines/icb/p4.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_sfx_description.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/common/px_clu_api.h"
#include "engines/icb/icb.h"
#include "engines/icb/debug.h"
#include "engines/icb/sound.h"
#include "engines/icb/sound_lowlevel.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/res_man.h"
#include "engines/icb/session.h"
#include "engines/icb/mission.h"
#include "engines/icb/common/px_sound_constants.h"
#include "engines/icb/sound_logic.h"
#include "engines/icb/remora.h"

#include "common/textconsole.h"

namespace ICB {

const char *menuSoundID = "__MENU__";
uint32 menuSoundIDHash = NULL_HASH;

// max is 127, min is zero
int32 speechVolume = 64;
int32 sfxVolume = 64;
int32 musicVolume = 64;

int32 currentSoundLevel;

int32 GetCurrentSoundLevel() { return currentSoundLevel; }

int32 GetSpeechVolume() { return speechVolume; }

int32 GetSfxVolume() { return sfxVolume; }

int32 GetMusicVolume() { return musicVolume; }

void SetSpeechVolume(int32 v) {
	if ((v < 0) || (v > 128))
		Fatal_error("Speech volume must be 0-128 not %d", v);
	speechVolume = v;
}

void SetSfxVolume(int32 v) {
	if ((v < 0) || (v > 128))
		Fatal_error("Sfx volume must be 0-128 not %d", v);
	sfxVolume = v;
	// sfx volume will change next update...
}

void SetMusicVolume(int32 v) {
	if ((v < 0) || (v > 128))
		Fatal_error("Music volume must be 0-128 not %d", v);
	musicVolume = v;
}

// max volume
#define MAX_VOLUME 127
#define SPEECH_ON_VOLUME 48
#define REMORA_ACTIVE_VOLUME 24

#define RESERVED_CHANNELS ((1 << SPEECH_CHANNEL) | (1 << MUSIC_CHANNEL))

#define NUMBER_CHANNELS 24

#define MAX_Y_DISTANCE_SQR (200 * 200)

bool8 soundOn = TRUE8;

bool8 pauseSound = TRUE8;

int32 speechOnSliderValue = MAX_VOLUME;

const uint8 pitchMakerMults[12 * 16] = PITCH_MULT;
const uint8 pitchMakerDivs[12 * 16] = PITCH_DIV;

class PitchMaker {

#define STANDARD_PITCH 4096

public:
	int32 GetPitch(int32 initialPitch, int32 envValue) {
		int32 v;
		int32 e;

		e = envValue;
		v = STANDARD_PITCH;

		// shifting up an octave
		while (e >= (128 * 12)) {
			v <<= 1; // *2
			e -= (128 * 12);
		}

		// shifting down an octave
		while (e <= (-128 * 12)) {
			v >>= 1; // /2
			e += (128 * 12);
		}

		// divide by 8 so there are 16 per semi-tone
		e /= 8;

		// use exp tables for - and + envValue
		if (e < 0) {
			v = (v * pitchMakerDivs[-e]) / 128;
		} else if (e > 0) {
			v = (v * pitchMakerMults[e]) / 128;
		}

		return (initialPitch * v) / STANDARD_PITCH;
	}
};

PitchMaker pitchMaker;

const uint8 volMakerFunction[MAX_VOLUME + 1] = VOL_FUNCTION;

class VolMaker {

	  public:
	int32 GetVol(int32 i) {
		if (i < 0)
			return 0;
		else if (i > 127)
			return 127;
		else
			return (sfxVolume * volMakerFunction[i]) >> 7; // sfxVolume is 0 (none) to full (128)
	}

	int32 GetVolMusic(int32 i) {
		if (i < 0)
			return 0;
		else if (i > 127)
			return 127;
		else
			return (musicVolume * volMakerFunction[i]) >> 7; // sfxVolume is 0 (none) to full (128)
	}
};

VolMaker volMaker;

const char *gunSfxVar = "gun_sfx";
const char *defaultGunSfx = "gunshot";
const char *gunDesc = "gunshot";

const char *ricochetSfxVar = "ricochet_sfx";
const char *defaultRicochetSfx = "ricochet";
const char *ricochetDesc = "ricochet";

const char *openSfxVar = "open_sfx";
const char *defaultOpenSfx = "door_test";
const char *openDesc = "open";

const char *closeSfxVar = "close_sfx";
const char *defaultCloseSfx = "door_test";
const char *closeDesc = "close";

const char *addingMediSfxVar = "add_medi_sfx";
const char *defaultAddingMediSfx = "add_medi";
const char *defaultUsingMediSfx = "use_medi";
const char *addingMediDesc = "adding medipack";

const char *addingClipSfxVar = "add_clip_sfx";
const char *defaultAddingClipSfx = "add_clip";
const char *addingClipDesc = "adding clip";

const char *activateRemoraSfxVar = "remora_enter_sfx";
const char *activateRemoraSfx = "remora_enter";
const char *activateRemoraDesc = "entering Remora";

const char *emailSfxVar = "remora_enter_sfx";
const char *defaultEmailSfx = "remora_enter";
const char *emailDesc = "email arriving";

const char *deactivateRemoraSfxVar = "remora_exit_sfx";
const char *deactivateRemoraSfx = "remora_exit";
const char *deactivateRemoraDesc = "exiting Remora";

const char *tinkleSfxVar = "tinkle_sfx";
const char *defaultTinkleSfx = "cartridge_tinkle";
const char *tinkleDesc = "tinkle";

const char *menuUpDownSfx = "menu\\menu_up_down";
const char *menuSelectSfx = "menu\\menu_select";
const char *menuCancelSfx = "menu\\menu_cancel";

#define INT_1 128

// returns the value given by env at x (using INT maths)
// env is in format where 128=1
// x is in same units...
int32 EvalEnv(const CEnvelope &env, int32 x) {

	int32 pa = env.a;
	int32 pb = env.b;
	int32 pc = env.c;
	int32 pd = env.d;
	int32 xxx, xx;

	if (pa == 0)
		xxx = 0;
	else if (abs(pa) < INT_1)
		xxx = (pa * x * x * x) / (INT_1 * INT_1 * INT_1);
	else if (abs(pa) < (INT_1 * INT_1))
		xxx = (pa * (x * x * x / INT_1)) / (INT_1 * INT_1);
	// xxx=((pd/INT_1)*x*x*x)/(INT_1*INT_1);
	else if (abs(pa) < (INT_1 * INT_1 * INT_1))
		xxx = ((pa / INT_1) * (x * x * x / (INT_1))) / (INT_1);
	else
		xxx = ((pa / (INT_1 * INT_1)) * (x * x * x / INT_1));
	// xxx=((pd/(INT_1*INT_1))*x*x*x)/(INT_1);

	if (pb == 0)
		xx = 0;
	else if (abs(pb) < (INT_1 * INT_1))
		xx = (pb * x * x) / (INT_1 * INT_1);
	else if (abs(pb) < (INT_1 * INT_1 * INT_1))
		xx = (pb / INT_1) * (x * x / INT_1);
	else
		xx = (pb / (INT_1 * INT_1)) * (x * x);

	return xxx + xx + ((x * pc) / (INT_1)) + (pd);
}

// get the sfxlist file!
LinkedDataFile *GetMissionSfxFile() {
	uint32 fileHash;
	uint32 clusterHash;
	LinkedDataFile *f = nullptr;

	// if no mission return NULL
	if (!g_mission) {
		// PC NEEDS THIS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		Fatal_error("No global mission sound so no special sfx!");

	}
	// get the file...
	else {

		fileHash = NULL_HASH;
		clusterHash = MS->Fetch_session_cluster_hash();
		f = (LinkedDataFile *)private_session_resman->Res_open("m_sfxlist", fileHash, MS->Fetch_session_cluster(), clusterHash);

	}

	if ((LinkedDataObject::GetHeaderVersion(f) != SFX_VERSION) || (FROM_LE_32(f->header.type) != FT_COMPILED_SFX))
		Fatal_error("Sound: mission::the.cmpsfxlist, Header wrong, engine:%d,%08x file:%d,%08x\n", SFX_VERSION, FT_COMPILED_SFX, LinkedDataObject::GetHeaderVersion(f), FROM_LE_32(f->header.type));

	return f;
}

LinkedDataFile *GetSessionSfxFile() {

	// if no session return NULL
	if ((!g_mission) || (!(g_mission->session))) {
		warning("no session so no sfx file!");
		return nullptr;
	}

	uint32 fileHash = NULL_HASH;
	uint32 clusterHash = MS->Fetch_session_cluster_hash();
	LinkedDataFile *f;

	// For the PC clustering the sfx file does not have the path, just the name

	f = (LinkedDataFile *)private_session_resman->Res_open(

	    "s_sfxlist",

	    fileHash, MS->Fetch_session_cluster(), clusterHash);

	if ((LinkedDataObject::GetHeaderVersion(f) != SFX_VERSION) || (FROM_LE_32(f->header.type) != FT_COMPILED_SFX))
		Fatal_error("Sound: session::the.cmpsfxlist, Header wrong, engine:%d,%08x file:%d,%08x\n", SFX_VERSION, FT_COMPILED_SFX, LinkedDataObject::GetHeaderVersion(f), FROM_LE_32(f->header.type));
	return f;
}

// get a pointer to sfx (number) in the mission or session sfx file
CSfx *GetMissionSfx(int32 number) {
	LinkedDataFile *linkedSfx;

	linkedSfx = GetMissionSfxFile();

	return (CSfx *)LinkedDataObject::Fetch_item_by_number(linkedSfx, number);
}

CSfx *GetSessionSfx(int32 number) {
	LinkedDataFile *linkedSfx;

	linkedSfx = GetSessionSfxFile();

	return (CSfx *)LinkedDataObject::Fetch_item_by_number(linkedSfx, number);
}

// return a number for the sfx (in either mision or session) -1 means the sfx is not in the sound file (probabily in the other one)
int32 WhichMissionSfx(uint32 sfx) {
	LinkedDataFile *linkedSfx;
	uint32 n;

	linkedSfx = GetMissionSfxFile();
	if (linkedSfx == nullptr)
		return -1;

	n = LinkedDataObject::Fetch_item_number_by_hash(linkedSfx, sfx);

	if (n == PX_LINKED_DATA_FILE_ERROR)
		return -1;
	else
		return (int32)n;
}

int32 WhichSessionSfx(uint32 sfx) {
	LinkedDataFile *linkedSfx;
	uint32 n;

	linkedSfx = GetSessionSfxFile();
	if (linkedSfx == nullptr)
		return -1;

	n = LinkedDataObject::Fetch_item_number_by_hash(linkedSfx, sfx);

	if (n == PX_LINKED_DATA_FILE_ERROR)
		return -1;
	else
		return (int32)n;
}

bool8 SfxExists(uint32 sfxHash) {
	int32 sfxNumber;

	sfxNumber = WhichMissionSfx(sfxHash);

	if (sfxNumber == -1) {
		sfxNumber = WhichSessionSfx(sfxHash);
	}

	if (sfxNumber == -1)
		return FALSE8;
	else
		return TRUE8;
}

bool8 SfxExists(const char *sfx) { return SfxExists(HashString(sfx)); }

int32 channelUsage = 0;

#define IS_CHANNEL_USED(CH) ((channelUsage | RESERVED_CHANNELS) & (1 << CH))
#define IS_CHANNEL_FREE(CH) ((~(channelUsage | RESERVED_CHANNELS)) & (1 << CH))
#define SET_CHANNEL_USED(CH)                                                                                                                                                       \
	{ channelUsage |= (1 << CH); }
#define SET_CHANNEL_FREE(CH)                                                                                                                                                       \
	{ channelUsage &= (~(1 << CH)); }

int32 GetFreeChannel() {
	int32 i;
	for (i = 0; i < NUMBER_CHANNELS; i++)
		if (IS_CHANNEL_FREE(i)) {
			return i;
		}
	return -1;
}

#define UPDATES_PER_SECOND 10
#define MAX_ENV_POSITION (128 * 128)

#define VOLUME_SLIDE 48 // how quickly we fade out a sound when it's time to cut off (this happens rarely but we don't want a pop)

// Get sfx for this registered sound
CSfx *CRegisteredSound::GetSfx() {
	CSfx *the_sfx = nullptr;

	if (m_sfxNumber == -1)
		Fatal_error("sfx is not found in session or mission");

	if (m_inSession)
		the_sfx = GetSessionSfx(m_sfxNumber);
	else
		the_sfx = GetMissionSfx(m_sfxNumber);

	if (!the_sfx)
		Fatal_error("Can't find registered SFX??? (number %d m_inSession=%d)", m_sfxNumber, m_inSession);

	return the_sfx;
}

// Zero out all the values of a sound
void CRegisteredSound::Wipe() {
	m_objID = NO_REGISTERED_SOUND;
	m_sndHash = NULL_HASH;
	m_channel = -1;

	m_x = REAL_ZERO;
	m_y = REAL_ZERO;
	m_z = REAL_ZERO;

	m_restart_time = -1;

	m_sfxNumber = NULL_HASH;

	m_velocity = 0;
	m_position = 0; // position*128

	m_volume = 0;
	m_current_pitch = 0;
	m_sample_pitch = 0;
	m_rand_pitch_value = 0;
	m_next_random_pos = 0;
	m_pan = 0;

	m_xoffset = REAL_ZERO;
	m_yoffset = REAL_ZERO;
	m_zoffset = REAL_ZERO;

	m_objMoving = 0;
	m_volume_offset = 0;
	m_inSession = FALSE8;
	m_turnOff = FALSE8;
	m_remove = FALSE8;
}

// update a sound if it is being turned off (reduce volume until 0 then destroy)
void CRegisteredSound::TurnOff() {
	if (m_turnOff) {
		if (m_channel == -1) {
			m_turnOff = FALSE8;
			if (m_remove)
				m_objID = NO_REGISTERED_SOUND; // remove sound
			return;
		}

		if (m_volume == 0) {
			Tdebug("sounds.txt", "Finally turning off %d!", m_channel);

			if (soundOn)
				StopSample(m_channel);

			SET_CHANNEL_FREE(m_channel);
			m_channel = -1;
			m_turnOff = FALSE8;
			if (m_remove)
				m_objID = NO_REGISTERED_SOUND; // remove sound
		} else {
			m_volume -= VOLUME_SLIDE;
			if (m_volume < 0)
				m_volume = 0;

			if (soundOn)
				SetChannelVolumeAndPan(m_channel, volMaker.GetVol(m_volume), m_pan);
		}
	}
}

// update every game cycle...
void CRegisteredSound::UpdateGameCycle(int32 newVol, int32 newPan) {
	CSfx *sfx;

	if (m_objID == NO_REGISTERED_SOUND)
		return;

	// if we're beyond next random update point (this must be done before wrap around...
	sfx = GetSfx();

	// new random pitch value
	if ((sfx->m_rand_mode) && (m_position > m_next_random_pos)) {
		GetRandom(sfx);
		m_next_random_pos += (128 * 128 - 1) / (sfx->m_rand_mode);
	}

	// stop sound if nesesary or loop
	if (m_position > MAX_ENV_POSITION) {
		if (!((sfx->m_looping) & SFX_LOOPING_FLAG)) {
			m_position = MAX_ENV_POSITION - 1; // hold here while turning off...

			Tdebug("sounds.txt", "sound ending");

			// turn off next cycle..
			if (m_channel != -1) {
				m_turnOff = TRUE8; // this will keep updating until m_channel is set to -1 by turn off at which point the sound will end
				m_remove = TRUE8;  // remove when finished turning off
			} else
				m_objID = NO_REGISTERED_SOUND; // don't need to update anymore...
		} else {
			// Tdebug("sounds.txt","sound looping...");

			m_next_random_pos = 0; // reset random pos thingy

			// if sample is not looping then replay...
			if ((m_channel != -1) && (!((sfx->m_looping) & WAV_LOOPING_FLAG))) {
				m_position = 0;
			} else {                                // sample is looping or sound isn't on so just reset wave
				m_position -= MAX_ENV_POSITION; // reset wave

				if (m_position <= 0) // definitely don't restart accidentally
					m_position = 1;
			}
		}
	}

	if (m_position < 0)
		m_position++;

	// if we're currently playing then update the low level if m_position=0 then time to start up sound
	if ((m_channel != -1) && (m_position >= 0)) {
		int32 v, p;
		int32 myPos;

		myPos = (m_position * (sfx->m_pitch.div)) & (128 * 128 - 1);                                        // work out where we are on the pitch graph...
		p = pitchMaker.GetPitch(m_sample_pitch, m_rand_pitch_value + EvalEnv(sfx->m_pitch, myPos / INT_1)); // get envelope pitch

		// if paused sounds then set pitch to zero

		if ((pauseSound) && ((m_sndHash != menuSoundIDHash) || (m_objID != SPECIAL_SOUND))) {
			// printf("channel %d pitch is now 0\n",m_channel);
			p = 0;
		}

		// has changed so update pitch

		if (p != m_current_pitch) {
			m_current_pitch = p;

			if (soundOn)
				SetChannelPitch(m_channel, m_current_pitch);
		}

		// VOLUME

		if (m_turnOff) {
			v = m_volume; // if we're turning off then ignore volume envelope
		} else {
			myPos = (m_position * (sfx->m_volume.div)) & (128 * 128 - 1); // volume pos
			v = (newVol * EvalEnv(sfx->m_volume, myPos / INT_1)) / 128;   // get new volume from envelope and input volume

			// decrease volume if speech is playing...
			v = (v * speechOnSliderValue) / 128;

			// limit to 0-127 for volume
			if (v > MAX_VOLUME)
				v = MAX_VOLUME;
			else if (v < 0)
				v = 0;
		}

		// current sound level is measure of how much sound is in scene
		// here it is both affected by distance from player and
		// volume on the curve...
		currentSoundLevel += (v * v);

		// if either volume or pan has changed

		if ((v != m_volume) || (newPan != m_pan)) {
			m_volume = v;
			m_pan = newPan;

			// only set if sound on
			if (soundOn)
				SetChannelVolumeAndPan(m_channel, volMaker.GetVol((m_volume * m_volume_offset) / 128), m_pan);
		}

		// if we need to start the sound effect

		if (m_position == 0) {
			Tdebug("sounds.txt", "Starting sound");

			m_position = 1; // so we don't try and start it twice in the unlikely event
			// that no 10hz signal goes between calls...

			// only start sample if sound is on
			if (soundOn) {
				StartSample(m_channel, sfx->GetSampleName(), m_inSession, (sfx->m_looping) & WAV_LOOPING_FLAG);
			}
		}
	}
}

// update at 10hz EXACTLY
void CRegisteredSound::Update10Hz() {
	if (m_objID == NO_REGISTERED_SOUND)
		return;

	// only update special menu sounds during 10hz signal...
	// if sounds are paused and sound is not a special menu sound (either sndHash doesn't match or sound object is not special)
	if ((pauseSound) && ((m_sndHash != menuSoundIDHash) || (m_objID != SPECIAL_SOUND)))
		return;

	if (m_position >= 0)
		m_position += m_velocity;
}

int32 GetSoundCloser(int32 objID, PXreal x, PXreal y, PXreal z);

// make a sound hearable, get a channel and set to it...
bool8 CRegisteredSound::SetHearable() {
	int32 ch;

	// sound is already hearable so ignore this call
	if (m_channel != -1)
		return TRUE8;

	Tdebug("sounds.txt", "Sound is now hearable");

	// find a free channel
	ch = GetFreeChannel();
	Tdebug("sounds.txt", "Channel %d", ch);

	// if we couldn't get a channel then we might want to knock out another sound
	if (ch == -1) {

		ch = GetSoundCloser(m_objID, m_x, m_y, m_z);
		if (ch == -1)
			return TRUE8; // still don't return true just don't set a channel
	}

	SET_CHANNEL_USED(ch);

	// reset volume and pitch ready to be set (keep existing pan)
	m_volume = 0xffff;
	m_current_pitch = 0xffff;

	// set channel
	m_channel = ch;

	m_position = m_restart_time; // start sample at next update...
	m_restart_time = -1;

	return TRUE8;
}

// make a sound unhearable, remove from playing sounds list...
void CRegisteredSound::SetUnhearable() {
	// already unhearable or turning off so do nothing...
	if ((m_channel == -1) || (m_turnOff))
		return;

	Tdebug("sounds.txt", "Sound is now unhearable");

	// start turning off sound... (we are definitely playing it...)
	m_turnOff = TRUE8;
	// don't remove though
}

// remove the sound
void CRegisteredSound::Remove() {
	// stop sample etc
	if (m_channel == -1) {
		m_objID = NO_REGISTERED_SOUND; // just remove it
	} else {
		m_turnOff = TRUE8; // fade down
		m_remove = TRUE8;  // then remove
	}
}

void CRegisteredSound::GetRandom(CSfx *sfx) {
	if (sfx->m_rand_pitch)
		m_rand_pitch_value = (g_icb->getRandomSource()->getRandomNumber(2 * 128 * sfx->m_rand_pitch - 1)) - (128 * sfx->m_rand_pitch);
	else
		m_rand_pitch_value = 0;
}

// register a new sound setting internal params
void CRegisteredSound::Register(const char *sndName, const char *sfxName, uint32 sfxHash, int8 volume) {
	CSfx *sfx;

	// simple params
	m_sndHash = HashString(sndName);

	if (sfxHash == NULL_HASH)
		sfxHash = HashString(sfxName);

	m_sfxNumber = WhichMissionSfx(sfxHash);
	m_inSession = FALSE8;

	if (m_sfxNumber == -1) {
		m_sfxNumber = WhichSessionSfx(sfxHash);
		m_inSession = TRUE8;
	}

	if (m_sfxNumber == -1)
		Fatal_error("sfx %s(%08x) is not found in session or mission", sfxName, sfxHash);

	// check it's okay...
	sfx = GetSfx();

	// set velocity = MAX_POSITION/((duration/128)*UPDATES_PER_SECOND)= 128*128*128/(duration*UPDATES_PER_SECOND)
	m_velocity = (MAX_ENV_POSITION * 128) / (sfx->m_duration * UPDATES_PER_SECOND);
	Tdebug("sounds.txt", "length=%d secs vel %d\n", (sfx->m_duration) / 128, m_velocity);

	// start next frame..
	m_restart_time = -1; // when hearable start at -1
	m_position = -1;

	m_channel = -1;

	m_volume = 0; // default, gets changed if sound is heard
	m_pan = 0;    // default (centre) will get changed before playing if necessary

	m_sample_pitch = GetSamplePitch(sfx->GetSampleName(), m_inSession);

	m_current_pitch = 0; // always alter pitch before playing

	GetRandom(sfx);
	m_next_random_pos = 0;

	m_remove = FALSE8;
	m_turnOff = FALSE8;

	m_xoffset = m_yoffset = m_zoffset = (PXreal)0;

	m_volume_offset = volume;

	padding1 = 0;
}

// register a sound and take position from the position of the object registering the sound
void CRegisteredSound::RegisterFromObject(const uint32 objID, const char *sndName, const char *sfxName, uint32 sfxHash, PXreal xo, PXreal yo, PXreal zo, int8 volume) {
	Register(sndName, sfxName, sfxHash, volume);
	m_xoffset = xo;
	m_yoffset = yo;
	m_zoffset = zo;

	// set this last so any updates don't know it's turned on yet...
	m_objID = objID;

	if ((MS->logic_structs[m_objID]->image_type) == VOXEL) {
		Tdebug("sounds.txt", "sound creator is an actor");
		m_objMoving = 1;
	} else
		m_objMoving = 0;

	MS->logic_structs[m_objID]->GetPosition(m_x, m_y, m_z);

	// This tells the sound logic about the new sound, to drive events etc.
	g_oSoundLogicEngine->NewSound(objID, (int32)m_x, (int32)m_y, (int32)m_z, GetSfx(), m_sndHash);
}

// register a sound as an absolute position
void CRegisteredSound::RegisterFromAbsolute(const uint32 objID, const char *sndName, const char *sfxName, uint32 sfxHash, PXreal x, PXreal y, PXreal z, int8 volume) {

	Register(sndName, sfxName, sfxHash, volume);

	// set this last so any updates don't know it's turned on yet...
	m_objID = objID;

	m_x = x;
	m_y = y;
	m_z = z;
	m_objMoving = 0;
}

#define MAX_SCREEN (float)(SCREEN_WIDTH / 2)

// if the sound if coming froma moving souce, this updates the position, otherwise it does nothing
void CRegisteredSound::GetPosition() {
	if (m_objMoving) {
		MS->logic_structs[m_objID]->GetPosition(m_x, m_y, m_z);
		m_x += m_xoffset;
		m_y += m_yoffset;
		m_z += m_zoffset;
	}
}

// gets the volume and pan values for a sound, based on the position of the sound relative to the actor / camera
void CRegisteredSound::GetVolumeAndPan(int32 &vol, int32 &pan) {
	// special sound, always central, etc
	if (m_objID == SPECIAL_SOUND) {
		vol = (int32)m_z; // get sound from z position...!
		pan = (int32)m_x; // pan is x position!
		return;
	}

	GetPosition();
	vol = g_oSoundLogicEngine->ProcessSound((int32)m_x, (int32)m_y, (int32)m_z, GetSfx());

	PXvector v = {m_x, m_y, m_z};
	PXvector screenPos;
	bool8 dontUse;

	if (!MS->SetOK()) {
		pan = 0;
		return;
	}

	PXcamera &cam = MS->GetCamera();
	PXWorldToFilm(v, cam, dontUse, screenPos);

	if (screenPos.x < (-MAX_SCREEN))
		screenPos.x = (-MAX_SCREEN);
	if (screenPos.x > MAX_SCREEN)
		screenPos.x = MAX_SCREEN;

	// /2 for centralisation
	pan = (int32)(128 * screenPos.x) / (SCREEN_WIDTH);

	return;
}

CRegisteredSound *g_registeredSounds[MAX_REGISTERED_SOUNDS];

int32 assignedSounds = 0;

// must be called at 10hz time...
void UpdateSounds10Hz() {
	int32 i;

	for (i = 0; i < MAX_REGISTERED_SOUNDS; i++) {
		if (g_registeredSounds[i])
			g_registeredSounds[i]->Update10Hz();
	}
}

// called every game cycle sets hearable and unhearable sounds...
void UpdateHearableSounds() {
	int32 i;
	int32 assigned;
	int32 volume;
	int32 pan;

	assigned = 0;

	// no sound...
	currentSoundLevel = 0;

	for (i = 0; i < MAX_REGISTERED_SOUNDS; i++) {
		// we might need to do this even if sound is not currently used
		g_registeredSounds[i]->TurnOff();

		if (g_registeredSounds[i]->IsUsed()) {
			// work out if is current hearable by the camera

			// if speical sound or sound is not paused then we get the volume and pan from manager
			if (((g_registeredSounds[i]->m_objID == SPECIAL_SOUND) && (g_registeredSounds[i]->m_sndHash == menuSoundIDHash)) || (!pauseSound)) {
				g_registeredSounds[i]->GetVolumeAndPan(volume, pan);

				// if volume is greater than zero
				if (volume > 0) {
					if (!g_registeredSounds[i]->SetHearable())
						Fatal_error("Can't find a free channel to play sound");
				} else
					g_registeredSounds[i]->SetUnhearable(); // no Update required we are turning off or are off screen....
			} else {
				volume = 0;
				pan = 0;
			}

			// update the wave etc
			g_registeredSounds[i]->UpdateGameCycle(volume, pan);
		}
	}

	currentSoundLevel = (currentSoundLevel * 100) / (128 * 128);
	if (currentSoundLevel > 100)
		currentSoundLevel = 100;

	assignedSounds = assigned;

	// speech volume slider

	int32 speechOnSliderTarget;

	// if somebody speaking...
	if ((g_mission) && (g_mission->session) && (MS->speech_info[CONV_ID].total_subscribers > 0) && (GetSpeechVolume() > 0))
		speechOnSliderTarget = SPEECH_ON_VOLUME;
	// if remora active even lower volume
	else if (g_icb->getGameType() == GType_ICB && g_oRemora->IsActive())
		speechOnSliderTarget = REMORA_ACTIVE_VOLUME;
	// otherwise our target is full volume
	else
		speechOnSliderTarget = MAX_VOLUME;

	// update
	if (speechOnSliderValue > speechOnSliderTarget) {
		speechOnSliderValue -= VOLUME_SLIDE;
		if (speechOnSliderValue < speechOnSliderTarget)
			speechOnSliderValue = speechOnSliderTarget;
	} else if (speechOnSliderValue < speechOnSliderTarget) {
		speechOnSliderValue += VOLUME_SLIDE;
		if (speechOnSliderValue > speechOnSliderTarget)
			speechOnSliderValue = speechOnSliderTarget;
	}
}

int32 GetSoundCloser(int32 objID, PXreal x, PXreal y, PXreal z) {
	PXreal px = REAL_ZERO;
	PXreal py = REAL_ZERO;
	PXreal pz = REAL_ZERO; // play pos

	PXreal dist;     // distance of this object
	PXreal thisDist; // distance of comparing object

	int32 i;
	int32 c;

	// if
	if (objID != SPECIAL_SOUND) {
		MS->player.log->GetPosition(px, py, pz);

		x -= px;
		y -= py;
		z -= pz;
		dist = x * x + y * y + z * z;
	} else // special sound so distance is by definition 0
		dist = (PXreal)0;

	PXreal maxDist = dist;
	int32 sfxReplace = -1;
	int32 channelReplace = -1;

	for (i = 0; i < MAX_REGISTERED_SOUNDS; i++) {
		c = g_registeredSounds[i]->m_channel;

		// can only replace a sound that has a channel and is not a special sound
		if ((c != -1) && (g_registeredSounds[i]->m_objID != SPECIAL_SOUND)) {
			g_registeredSounds[i]->GetPosition();
			x = g_registeredSounds[i]->m_x - px;
			y = g_registeredSounds[i]->m_y - py;
			z = g_registeredSounds[i]->m_z - pz;
			thisDist = x * x + y * y + z * z;

			// if this distance from player is more than the maximum so far then this is the furthest sound
			// yet found, and should therefore be replaced...
			if (thisDist > maxDist) {
				// this is the new max distance
				maxDist = thisDist;

				// this is the sfx and channcel to take
				sfxReplace = i;
				channelReplace = c;
			}
		}
	}

	// we found a sound further away than us therefore we can have it's channcel
	// the sfx is sfxReplace, the channel we are stealing is channelReplace
	if (channelReplace != -1) {
		Tdebug("sounds.txt", "replacing sound %d (channel %d) because it's too far away (dist: %g, our dist: %g)", sfxReplace, channelReplace, maxDist, dist);

		// the sfx now has no channel
		g_registeredSounds[sfxReplace]->m_channel = -1;

		// we return the channel we have just pinched

		return channelReplace;
	}

	// otherwise we found no channel so return -1
	return -1;
}

void TurnOffSound() { soundOn = FALSE8; }

void TurnOnSound() { soundOn = TRUE8; }

int32 GetFreeSound(const char *sfxName) {
	int32 i;

	for (i = 0; i < MAX_REGISTERED_SOUNDS; i++) {
		if (g_registeredSounds[i]->IsFree()) {
			Tdebug("sounds.txt", "sfx: %s  registered sound: %d", sfxName, i);
			return i;
		}
	}

	Fatal_error("No free sounds! %s", sfxName);
	return -1;
}

int32 FindSound(uint32 obj, uint32 sndHash, int32 start = 0) {
	int32 i;

	for (i = start; i < MAX_REGISTERED_SOUNDS; i++) {
		if (g_registeredSounds[i]->IsThisSound(obj, sndHash))
			return i;
	}

	// should this be a fatal error..??
	return -1;
}

// fn_play_sfx_offset(sfxName,sndID,x,y,z,isNico)
// this is the generic function
void RegisterSoundOffset(uint32 obj, const char *offsetName, const char *sfxName, uint32 sfxHash, const char *sndID, PXreal xo, PXreal yo, PXreal zo, int32 isNico, int32 time,
						 int8 volume_offset) {
	// check we have the hash before we register any sounds...!
	if (menuSoundIDHash == NULL_HASH)
		menuSoundIDHash = HashString(menuSoundID);

	int32 i;
	i = GetFreeSound(sfxName);

	// for getting nico position
	PXreal x, y, z;

	if ((obj != SPECIAL_SOUND) && (pauseSound))
		warning("Registering sound whilst sound paused!");

	// special sound
	if (obj == SPECIAL_SOUND) {
		g_registeredSounds[i]->RegisterFromAbsolute(obj, sndID, sfxName, sfxHash, xo, yo, zo, volume_offset);
	}
	// absolute sound (no name)
	else if ((offsetName == nullptr) || (strcmp(offsetName, "") == 0)) {
		// absolute address
		g_registeredSounds[i]->RegisterFromAbsolute(obj, sndID, sfxName, sfxHash, xo, yo, zo, volume_offset);
	}
	// object is a nico marker
	else if (isNico) {
		// is nico so get position of it...
		// x=, y=, z=
		_feature_info *fi = (_feature_info *)(LinkedDataObject::Fetch_item_by_name(MS->features, offsetName));
		x = fi->x;
		y = fi->y;
		z = fi->z;
		g_registeredSounds[i]->RegisterFromAbsolute(obj, sndID, sfxName, sfxHash, x + xo, y + yo, z + zo, volume_offset);
	}
	// object is a mega
	else {
		// is mega object so attach sound to it
		// obj=
		obj = LinkedDataObject::Fetch_item_number_by_name(MS->objects, offsetName);
		g_registeredSounds[i]->RegisterFromObject(obj, sndID, sfxName, sfxHash, xo, yo, zo, volume_offset);
	}

	Tdebug("sounds.txt", "restart time is %d %d\n", (-1) - (time), time);
	g_registeredSounds[i]->m_restart_time = (-1) - (time);
}

// register a sound from an object
void RegisterSound(uint32 obj, const char *sfxName, uint32 sfxHash, const char *sndID, int8 volume_offset) {
	const char *name;

	if (obj == SPECIAL_SOUND)
		name = nullptr;
	else
		name = (const char *)(LinkedDataObject::Fetch_items_name_by_number(MS->objects, obj));

	RegisterSoundOffset(obj, name, sfxName, sfxHash, sndID, (PXreal)0, (PXreal)0, (PXreal)0, 0, 0, volume_offset);
}

// register a sound from an absolute position
void RegisterSoundAbsolute(uint32 obj, const char *sfxName, uint32 sfxHash, const char *sndID, PXreal x, PXreal y, PXreal z, int8 volume_offset) {
	RegisterSoundOffset(obj, nullptr, sfxName, sfxHash, sndID, x, y, z, 0, 0, volume_offset);
}

void RegisterSoundTime(uint32 obj, const char *sfxName, uint32 sfxHash, const char *sndID, int32 time, int8 volume_offset) {
	const char *name;

	if (obj == SPECIAL_SOUND)
		name = nullptr;
	else
		name = (const char *)(LinkedDataObject::Fetch_items_name_by_number(MS->objects, obj));

	RegisterSoundOffset(obj, name, sfxName, sfxHash, sndID, (PXreal)0, (PXreal)0, (PXreal)0, 0, time, volume_offset);
}

// special sound
// for menus
void RegisterMenuSound(const char *sfxName, uint32 sfxHash, int32 volume, int32 pan, int8 volume_offset) {
	// volume is z of position
	RegisterSoundOffset(SPECIAL_SOUND, nullptr, sfxName, sfxHash, menuSoundID, (PXreal)pan, (PXreal)0, (PXreal)volume, 0, 0, volume_offset);
}

// special sound
// for in game (these are paused just like any other...
void RegisterSoundSpecial(const char *sfxName, uint32 sfxHash, const char *sndID, int32 volume, int32 pan, int8 volume_offset) {
	// volume is z of position
	RegisterSoundOffset(SPECIAL_SOUND, nullptr, sfxName, sfxHash, sndID, (PXreal)pan, (PXreal)0, (PXreal)volume, 0, 0, volume_offset);
}

void RemoveRegisteredSound(uint32 obj, const char *sndID) {
	int32 i;
	uint32 sndIDHash;

	sndIDHash = HashString(sndID);

	int32 start;

	// do object sounds with id

	// start at beginning
	start = 0;

	do {
		i = FindSound(obj, sndIDHash, start);
		// okay we have one (either object or special)
		if (i != -1)
			g_registeredSounds[i]->Remove();

		// next start of search is one above this...
		start = i + 1;

	} while (i != -1);

	// now do special sounds with ID

	// restart
	start = 0;

	do {
		i = FindSound(SPECIAL_SOUND, sndIDHash, start);

		// okay we have one (either object or special)
		if (i != -1)
			g_registeredSounds[i]->Remove();

		// next start of search is one above this...
		start = i + 1;

	} while (i != -1);

	// okay all done
}

void RemoveAllSoundsWithID(uint32 obj) {
	int32 i;

	for (i = 0; i < MAX_REGISTERED_SOUNDS; i++) {
		if (g_registeredSounds[i]->GetObjectID() == obj)
			g_registeredSounds[i]->Remove();
	}
}

void StopAllSoundsNow() {
	Tdebug("sounds.txt", "stopping");
	uint32 i;

	for (i = 0; i < MAX_REGISTERED_SOUNDS; i++) {
		g_registeredSounds[i]->Wipe();
	}

	for (i = 0; i < NUMBER_CHANNELS; i++) {
		if (soundOn) {
			StopSample(i);
		}

		SET_CHANNEL_FREE(i);
	}

	Tdebug("sounds.txt", "stopped");
}

// pause all sounds except special sounds...
void PauseSounds() {
	pauseSound = TRUE8;
	UpdateHearableSounds();
}

void UnpauseSounds() {
	pauseSound = FALSE8;
}

} // End of namespace ICB
