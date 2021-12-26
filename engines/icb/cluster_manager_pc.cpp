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

#include "engines/icb/common/px_string.h"
#include "engines/icb/res_man_pc.h"
#include "engines/icb/debug_pc.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/cluster_manager_pc.h"
#include "engines/icb/options_manager_pc.h"
#include "engines/icb/movie_pc.h"

#include "common/textconsole.h"
#include "common/file.h"

namespace ICB {

// Global ClusterManger instance
ClusterManager *g_theClusterManager;

// Function prototypes
void RecursivelyDeleteDirectory(const char *path);
void ValidateDirectoryToDelete(const char *path);
uint32 GetFileSize(const char *path);
void MakeDirectoryTree(MISSION_ID mission);

// Number of bytes to read from CD and write to hard disk per cycle
#define CHUNKSIZE 25600 // ((300 * 1024) / 12) based on 12fps for 2xCD drive

// This is the number of attempts we check the cd drive on the prompt screen
#define CD_SEARCH_DELAY 1000

// Private for the title background movie handling
MovieManager *g_while_u_wait_SequenceManager;

// Colours used by progress display
uint32 g_progressColourMap[7] = {
	0x3C3C3C, // OFF  60  60  60
	0xFEFEFE, //  |   254 254 254
	0xCAD5E4, //  |   202 213 228
	0xA6B8D4, //  |   166 184 212
	0x89A1C7, //  |   137 161 199
	0x587AB0, //  V   88  122 176
	0x2E579C  // ON   46  87  156
};

// Controls the decay on the progress bit colouring
#define PROGRESS_BIT_DELAY 6

ClusterManager::ClusterManager() {
	memset(m_cdroot1, 0, 1024);
	memset(m_cdroot2, 0, 1024);
	m_multipleCDDrives = FALSE8;
	m_activeCDDrive = 1;
	memset(m_missionDir, 0, 8);
	m_bytesFreeOnInstalledDrive = 0;
	m_minimumInstall = FALSE8;

	memset(m_theList, 0, MAX_BYTESIZE_OF_A_FILELIST);
	m_filelistTotalBytes = 0;
	m_filelistCursor = -1;
	m_filelistSize = 0;

	m_src_fp = nullptr;
	m_dst_fp = nullptr;

	m_currentFileSize = 0;
	m_chunkCounter = 0;
	m_bytesDone = 0;

	m_movieMemoryPointer = nullptr;

	m_installDone = FALSE8;

	memset(m_progressBits, 0, sizeof(PROGRESS_BIT) * NUMBER_OF_PROGRESS_BITS);
	m_bitsDone = 0;
	m_frameCounter = 0;
	m_currentLanguage = T_ENGLISH;
}

ClusterManager::~ClusterManager() {}

void ClusterManager::Initialise() {
	// First we need to discover what install method has been employed.
	MinimumInstallCheck();

	// Obtain drive information (CD path(s) and free space)
	InterrogateDrives();

	// Starting with an empty mission directory covers against the game
	// crashing out or data being modified between executes.
	CleanHardDisk();

	// Require a disc on startup
	CheckAnyDiscInserted();

	// Line number 7398
	const char *testline = g_theOptionsManager->GetTextFromReference(HashString("opt_missingdisc"));

	if (strcmp(testline, "Please insert disc %d") == 0)
		m_currentLanguage = T_ENGLISH;
	else if (strcmp(testline, "Veuillez ins\xE9rer le disque %d") == 0)
		m_currentLanguage = T_FRENCH;
	else if (strcmp(testline, "Inserisci il disco %d") == 0)
		m_currentLanguage = T_ITALIAN;
	else if (strcmp(testline, "Bitte CD %d einlegen") == 0)
		m_currentLanguage = T_GERMAN;
	else if (strcmp(testline, "Por favor, inserta el disco %d") == 0)
		m_currentLanguage = T_SPANISH;
	else if (strcmp(testline, "\xC2\xF1\xF2\xE0\xE2\xFC\xF2\xE5 \xE4\xE8\xF1\xEA %d") == 0)
		m_currentLanguage = T_RUSSIAN;
	else
		// Must be polish by default
		m_currentLanguage = T_POLISH;
}

void ClusterManager::CheckDiscInserted(MISSION_ID /*mission*/) {
}

bool8 ClusterManager::CheckDiscInsertedWithCancel(MISSION_ID mission) {
	// No user cancel
	return FALSE8;
}

void ClusterManager::CheckAnyDiscInserted() {
}

bool8 ClusterManager::StartMissionInstall(MISSION_ID /*mission*/) {
	return FALSE8;
}

bool8 ClusterManager::InstallMission() {
	// Nothing to do on a full install
	if (m_minimumInstall == FALSE8)
		return FALSE8;

	return FALSE8;
}

void ClusterManager::InterrogateDrives() {
}

void ClusterManager::CalculateFreeDiskSpace(void) {
	m_bytesFreeOnInstalledDrive = 256 * 1024 * 1024;
}

char *ClusterManager::GetCDRoot(void) {
	if (m_activeCDDrive == 1)
		return m_cdroot1;
	else
		return m_cdroot2;
}

int32 ClusterManager::WhichCD(MISSION_ID mission) {
	// All demos exist on one CD only
	int32 demo = g_globalScriptVariables->GetVariable("demo");
	if (demo != 0)
		return 1;

	if (mission >= MISSION1 && mission <= MISSION3)
		return 1;
	else if (mission >= MISSION4 && mission <= MISSION7)
		return 2;
	else if (mission >= MISSION8 && mission <= MISSION10)
		return 3;
	else
		Fatal_error("ClusterManager::WhichCD() can't resolve unknown mission parameter");

	// Never gonna get here are we
	return 0;
}

bool8 ClusterManager::CheckForCD(int32 /*number*/) {
	strcpy(m_cdroot1, "");
	strcpy(m_cdroot2, "");
	return TRUE8;
}

void ClusterManager::MinimumInstallCheck() {
	m_minimumInstall = FALSE8;
}

bool8 ClusterManager::IsMissionDataInstalled(MISSION_ID &m) {
	for (uint32 i = 0; i < NUMBER_OF_MISSIONS; i++) {
		// Make the mission directories one by one and see if any exist
		char h_mission[8];
		HashFile(g_mission_names[i], h_mission);

		pxString missionDirectory;
		missionDirectory.Format("m\\%s\\", h_mission);

		if (checkFileExists(missionDirectory)) {
			m = (MISSION_ID)i;
			return TRUE8;
		}
	}

	// No mission directories on the hard disk
	return FALSE8;
}

void ClusterManager::CleanHardDisk() {
	// Can't be letting that happen now can we
	if (m_minimumInstall == FALSE8)
		return;
}

void ClusterManager::MissingCD(int32 /*number*/) {
}

bool8 ClusterManager::MissingCDWithCancel(int32 /*number*/) {
	return FALSE8;
}

void ClusterManager::LoadFileList(MISSION_ID /*mission*/) {
	if (m_minimumInstall == FALSE8)
		return;
}

char *ClusterManager::GetFileListEntry() {
	if (m_filelistCursor == -1)
		Fatal_error("Can't retrieve filelist entry without loading a filelist first!");

	char *line = nullptr;

	// End of file check
	if (m_filelistCursor < m_filelistSize) {
		line = &(m_theList[m_filelistCursor]);

		// Move to next line
		m_filelistCursor += strlen((const char *)&(m_theList[m_filelistCursor]));

		// Skip any terminators to get to the start of the next line
		while (m_theList[m_filelistCursor] == 0)
			m_filelistCursor++;
	}

	return line;
}

bool8 ClusterManager::DrawCoverFrame(void) {
	// Draw a frame of the torture movie
	g_while_u_wait_SequenceManager->drawFrame(working_buffer_id);

	// Have we finished both movie playback and mission install
	if (m_installDone) {
		// Release bink from playing the movie
		g_while_u_wait_SequenceManager->kill();
		// Free up resources
		delete[] m_movieMemoryPointer;

		// Quit only when movie has finished
		return FALSE8;
	}

	DrawProgressBits();

	// Update screen manually
	surface_manager->Flip();

	return TRUE8;
}

void ClusterManager::InitialiseProgressBits() {
	// Tweakable
	int32 width = 5;
	int32 height = 15;
	int32 spacing = 2;
	int32 initialY = SCREEN_DEPTH - height - 30;

	// Calculate entire width so we can centre things
	int32 length = (NUMBER_OF_PROGRESS_BITS * (width + spacing)) - spacing;
	int32 initialX = (SCREEN_WIDTH / 2) - (length / 2);

	for (int32 i = 0; i < NUMBER_OF_PROGRESS_BITS; i++) {
		m_progressBits[i].r.left = initialX;
		m_progressBits[i].r.top = initialY;
		m_progressBits[i].r.right = initialX + width;
		m_progressBits[i].r.bottom = initialY + height;

		m_progressBits[i].state = 0;

		// Now increment
		initialX += width + spacing;
	}

	m_bitsDone = 0;
}

void ClusterManager::UpdateProgressBits() {
	if (m_frameCounter % PROGRESS_BIT_DELAY == 0) {
		// Update the state of all bits
		for (int32 i = 0; i < NUMBER_OF_PROGRESS_BITS; i++) {
			if (m_progressBits[i].state > 0 && m_progressBits[i].state != 6) {
				m_progressBits[i].state = m_progressBits[i].state + 1;
			}
		}
	}

	// Bytes per progress bit
	float progress_inc = (float)(m_filelistTotalBytes / NUMBER_OF_PROGRESS_BITS);

	// The number of bits that should bit switched on
	uint32 bitsOn = (int32)((float)m_bytesDone / progress_inc);

	// Do we need to switch on a new bit
	if (bitsOn > m_bitsDone) {
		m_progressBits[m_bitsDone].state = 1;
		m_bitsDone++;
	}
}

void ClusterManager::DrawProgressBits() {
	for (int32 i = 0; i < NUMBER_OF_PROGRESS_BITS; i++) {
		Fill_rect(m_progressBits[i].r.left, m_progressBits[i].r.top, m_progressBits[i].r.right, m_progressBits[i].r.bottom, g_progressColourMap[m_progressBits[i].state]);
	}
}

void ClusterManager::Shutdown(void) {
}

void RecursivelyDeleteDirectory(const char * /*path*/) {
}

void ValidateDirectoryToDelete(const char *path) {
	if (strcmp(path, pxVString("m\\FP3YNHA\\")) == 0)
		return;
	else if (strcmp(path, pxVString("m\\HWYIPVA\\")) == 0)
		return;
	else if (strcmp(path, pxVString("m\\TPQUB4D\\")) == 0)
		return;
	else if (strcmp(path, pxVString("m\\RIGABTB\\")) == 0)
		return;
	else if (strcmp(path, pxVString("m\\GAIYO3A\\")) == 0)
		return;
	else if (strcmp(path, pxVString("m\\NMUFF0B\\")) == 0)
		return;
	else if (strcmp(path, pxVString("m\\1QYUOAA\\")) == 0)
		return;
	else if (strcmp(path, pxVString("m\\TT3WADD\\")) == 0)
		return;
	else
		Fatal_error(pxVString("ValidateDirectoryToDelete() failed on: %s", path));
}

uint32 GetFileSize(const char *path) {
	Common::File file;

	if (!file.open(path)) {
		return 0;
	}

	return (uint32)file.size();
}

const char *MissionIdToName(MISSION_ID mission) {
	switch (mission) {
	case MISSION1:
		return g_m01;
	case MISSION2:
		return g_m02;
	case MISSION3:
		return g_m03;
	case MISSION4:
		return g_m04;
	case MISSION5:
		return g_m05;
	case MISSION7:
		return g_m07;
	case MISSION8:
		return g_m08;
	case MISSION9:
		return g_m08;
	case MISSION10:
		return g_m10;
	}

	Fatal_error("MissionIdToName() should never get here - smack AndyB");
	return nullptr;
}

void MakeDirectoryTree(MISSION_ID /*mission*/) {
}

} // End of namespace ICB
