/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/p4.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_sfx_description.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/debug.h"
#include "engines/icb/sound/music_manager.h"
#include "engines/icb/sound/speech_manager.h"
#include "engines/icb/sound/fx_manager.h"
#include "engines/icb/sound.h"
#include "engines/icb/res_man_pc.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/mission.h"

namespace ICB {

bool8 DoesClusterContainFile(pxString clustername, uint32 hash_to_find, uint32 &fileoffset, uint32 &filesize) {
	// Manually open the cluster file
	Common::SeekableReadStream *stream = openDiskFileForBinaryStreamRead(clustername.c_str());

	if (stream == NULL)
		Fatal_error(pxVString("Failed to open cluster: %s", clustername.c_str()));

	// Read in first 16 bytes so we can get the header size
	uint32 data[4];
	stream->read(data, sizeof(uint32) * 4);

	// Get the size in bytes of the cluster header
	uint32 clustersize = data[2];

	// Seek to beginning of file
	stream->seek(0, SEEK_SET);

	// Get storage
	uint8 *memory = new uint8[clustersize];
	if (!memory)
		Fatal_error("DoesClusterContainFile() was refused memory allocation.");

	// Read the header into memory
	stream->read(memory, sizeof(uint8) * clustersize);
	// Close the file
	delete stream;

	// Cast memory to header structure
	Cluster_API *clu = (Cluster_API *)memory;

	// Look for the file in the cluster
	int nFiles = (int)clu->ho.noFiles;

	int i;
	for (i = 0; i < nFiles; i++) {
		// Have we found it
		if (hash_to_find == clu->hn[i].hash)
			break;
	}

	// Check i < nFiles
	if (i >= nFiles) {
		// Couldn't find the file
		fileoffset = 0;
		filesize = 0;
		return FALSE8;
	}

	// Get the figures we need for streaming
	filesize = clu->hn[i].size;
	fileoffset = clu->hn[i].offset;

	// Tidy up
	delete[] memory;

	return TRUE8;
}

void SetupSndEngine() {}

void LoadSessionSounds(const cstr cluster) { Tdebug("sounds.txt", "Setting up session sfx data cluster \"%s\"", cluster); }

void LoadMissionSounds(const cstr cluster) { Tdebug("sounds.txt", "Setting up mission sfx data cluster \"%s\"", cluster); }

// in Hz
int32 GetSamplePitch(const cstr sampleName, bool8 isInSession) {
	if (g_theFxManager) {
		Tdebug("sounds.txt", "Getting sample rate for %s (isInSession=%d) = %d", sampleName, isInSession,
		       g_theFxManager->GetDefaultRate(pxVString("samples\\pc\\%s.wav", sampleName)));

		int rate = 0;
		pxString smp;
		smp.Format("%s.wav", sampleName);

		uint32 b_offset, sz;

		if (!DoesClusterContainFile(pxVString("g\\samples.clu"), HashString(smp), b_offset, sz))
			Fatal_error(pxVString("Couldn't find %s in global sample cluster", (const char *)smp));

		// Pass sample name only if we're running from clusters
		rate = (g_theFxManager->GetDefaultRate(smp, b_offset));

		// Return sample rate of a wav file
		return rate;
	}

	return false;
}

void StartSample(int32 ch, const cstr sampleName, bool8 isInSession, int looping) {
	int32 result;

	if (g_theFxManager) {
		// Unload sample if channel has one
		g_theFxManager->Unregister(ch);

		// Load the sample into memory
		pxString smp;
		smp.Format("%s.wav", sampleName);

		uint32 b_offset, sz;

		if (!DoesClusterContainFile(pxVString("g\\samples.clu"), HashString(smp), b_offset, sz))
			Fatal_error(pxVString("Couldn't find %s in global sample cluster", (const char *)smp));

		// Pass sample name only if we're running from clusters
		result = g_theFxManager->Register(ch, pxVString("%s.wav", sampleName), 0, b_offset);

		// Set channel looping status
		g_theFxManager->SetLooping(ch, looping);
		// Begin palyback
		g_theFxManager->Play(ch);

		Tdebug("sounds.txt", "playing sample: %s in channel %d (is in session=%d result=%d looping=%d)", sampleName, ch, isInSession, result, looping);
	}
}

// stop channel ch
void StopSample(int32 ch) {
	// Halt playback for this channel
	if (g_theFxManager)
		g_theFxManager->Stop(ch);
}

// set volume 0=none, 127=full... and pan -128 - 128 (l-r)
void SetChannelVolumeAndPan(int32 ch, int32 volume, int32 pan) {
	if (g_theFxManager) {
		// Set channel volume and pan (cumulative effect)
		g_theFxManager->SetVolume(ch, volume);
		g_theFxManager->SetPan(ch, pan);
	}

	Tdebug("sounds.txt", "Setting channel %d vol=%d pan=%d", ch, volume, pan);
}

// set pitch in hz
void SetChannelPitch(int32 ch, int32 pitch) {
	if (g_theFxManager) {
		// Set channel playback frequency
		g_theFxManager->SetPitch(ch, pitch);
	}

	Tdebug("sounds.txt", "Setting channel %d pitch=%d", ch, pitch);
}

// Speech support routines

// just return 1, no preloading so always continue
int PreloadSpeech(uint32) { return 1; }

// do the line of text
int SayLineOfSpeech(uint32 speechHash) {
	if ((g_theSpeechManager) && (GetSpeechVolume() > 0)) {

		// If we are testing the translations wavs then we need to look elsewhere for the wav files
		if (tt) {
			// Righto, where do we find them?
			char hashfile[20];
			EngineHashToFile(speechHash, hashfile);

			pxString wavFileName = pxVString("%s\\speech\\%s.wav", tt_text, hashfile);

			if (checkFileExists(wavFileName)) {
				// The wav has been found. Speak it

				// OK, go for it...
				g_theSpeechManager->StartSpeech(wavFileName, 0, (uint8)GetSpeechVolume());

				return g_theSpeechManager->GetLength();
			} else {
				Message_box(pxVString("Wav %s not found", (const char *)wavFileName));
				return (2 * 12);
			}
		}

		// Need to look in the session music cluster first off

		pxString clustername;

		/*  const char* cluster_root = "thegame\\english\\pc\\everything\\cd1" ;*/

		// Construct full pathname to the session speech cluster
		char h_mission_name[8];
		pxString missionname = g_mission->Fetch_tiny_mission_name();
		// Make the mission name lowercase!
		missionname.ToLower();
		HashFile(missionname, h_mission_name);

		// Convert to lower-case for FS operations
		pxString missionHash(h_mission_name);
		missionHash.ToLower();
		pxString sessionHash(g_mission->session->Fetch_session_h_name());
		sessionHash.ToLower();

		clustername.Format("m\\%s\\%s\\speech.clu", missionHash.c_str(), sessionHash.c_str());

		uint32 file_offset, file_size;
		if (!DoesClusterContainFile(clustername, speechHash, file_offset, file_size)) {

			clustername.Format("g\\speech.clu");

			// Take a look
			if (!DoesClusterContainFile(clustername, speechHash, file_offset, file_size)) {
				// OK - rather than blowing up, use the default wav
				if (!DoesClusterContainFile(clustername, HashString("unavail"), file_offset, file_size)) {
					Fatal_error("Speech cluster doesn't contain unavail.wav?  Is it even there?");
				}
			}
		}

		// OK, go for it...
		g_theSpeechManager->StartSpeech(clustername, file_offset, (uint8)GetSpeechVolume());

		return g_theSpeechManager->GetLength();
	}

	// Sound device is unavailable for some reason so return a hardcoded
	// delay of 3 seconds (which the user can click through anyhow)
	return (3 * 12);
}

// This is the same as StopSpeechPlayback for the PC but different for the PSX !
void CancelSpeechPlayback() {
	if (g_theSpeechManager) {
		g_theSpeechManager->StopSpeech();
	}
}

// Should this really just stop the speech or should it continue until the sample has finished ?
// The PSX version waits until the speech has finished
void StopSpeechPlayback() {
	if (g_theSpeechManager) {
		g_theSpeechManager->StopSpeech();
	}
}

// Don't know if there's any call for this but what the heck...
bool8 IsSpeechPlaying() {
	if (g_theSpeechManager) {
		return g_theSpeechManager->IsPlaying();
	}

	return FALSE8;
}

} // End of namespace ICB
