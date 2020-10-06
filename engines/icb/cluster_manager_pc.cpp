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

#include "engines/icb/common/px_rccommon.h"
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

	m_src_fp = NULL;
	m_dst_fp = NULL;

	m_currentFileSize = 0;
	m_chunkCounter = 0;
	m_bytesDone = 0;

	m_movieMemoryPointer = NULL;

	m_installDone = FALSE8;

	memset(m_progressBits, 0, NUMBER_OF_PROGRESS_BITS);
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
#if 0
	// To kill compiler warning
	mission;

#ifdef PC_DEMO
	return;
#endif

#ifdef FROM_PC_CD
	int cd = WhichCD(mission);

	if (!CheckForCD(cd)) {
		MissingCD(cd);
	}
#endif
#endif
}

bool8 ClusterManager::CheckDiscInsertedWithCancel(MISSION_ID mission) {
#if 0
#ifdef PC_DEMO
	return TRUE8;
#endif

#if 0 // was #ifdef FROM_PC_CD

	int cd = WhichCD(mission);

	if (!CheckForCD(cd)) {
		return MissingCDWithCancel(cd);
	}

#endif
#endif
	// No user cancel
	return FALSE8;
}

void ClusterManager::CheckAnyDiscInserted() {
#if 0
#ifdef PC_DEMO
	return;
#endif

	if (!CheckForCD(1)) {
		if (!CheckForCD(2)) {
			if (!CheckForCD(3))
				MissingCD(1);
		}
	}
#endif
}

bool8 ClusterManager::StartMissionInstall(MISSION_ID /*mission*/) {
	return FALSE8;

#if 0
	// First off ensure we have the correct CD in the drive for this mission
	CheckDiscInserted(mission);

	// Nothing more to do on a full install
	if (m_minimumInstall == FALSE8)
		return FALSE8;

	// Check if we already have this mission on the disk
	MISSION_ID on_disk;
	if (IsMissionDataInstalled(on_disk)) {
		if (mission == on_disk) {
			// Data already there
			return FALSE8;
		}
	}

	// Set flag to false
	m_installDone = FALSE8;

	// Pointer to the mission name
	char *missionName = mission_names[mission];

	// Load the loading movie :)

	// Full path to the torture movie (from hard disk at the moment)
	pxString loadingMovie;
	loadingMovie.Format("gmovies\\loading.bik");
	loadingMovie.ConvertPath();
	// Load the movie into memory for playback
	uint32 movieSize = GetFileSize(loadingMovie);
	if (movieSize == 0)
		Fatal_error("Couldn't get filesize of loading movie");

	// Get storage
	m_movieMemoryPointer = new char[movieSize];

	// Open the movie file and read it straight into memory
	FILE *movieFp = openDiskFileForBinaryRead(loadingMovie);
	if (movieFp == NULL)
		Fatal_error(pxVString("Failed to open movie file: %s for reading", (const char *)loadingMovie));

	if (fread(m_movieMemoryPointer, 1, movieSize, movieFp) != movieSize)
		Fatal_error("ClusterManager::StartMissionInstall() failed to read from file");

	// Close the file
	fclose(movieFp);

	// Register this pointer with bink
	if (!g_while_u_wait_SequenceManager.Register(m_movieMemoryPointer, FALSE8, TRUE8, BINKFROMMEMORY)) {
		Fatal_error("ClusterManager::StartMissionInstall() - Couldn't register loading movie");
	}

	// Reset progress display
	InitialiseProgressBits();
	m_bytesDone = 0;
	m_frameCounter = 0;

	// Deletion might take a second or so, so draw initial frame
	DrawCoverFrame();

	// First thing to do is ensure no mission data is on the hard disk
	CleanHardDisk();

	// Load the appropriate filelist to memory for parsing
	LoadFileList(mission);

	// As we now know that mission directory is empty check current disk space
	CalculateFreeDiskSpace();

	// Now validate that we have room to put this mission to the hard-disk
	if (m_filelistTotalBytes > m_bytesFreeOnInstalledDrive) {
		// Hard to know how to deal with this really
		Fatal_error("Not enough disk space to install mission.  Please make some room and try again.");
	}

	// Ensure all required directories exist for this mission
	MakeDirectoryTree(mission);

	return TRUE8;
#endif
}

bool8 ClusterManager::InstallMission() {
	// Nothing to do on a full install
	if (m_minimumInstall == FALSE8)
		return FALSE8;

	// Just to allow compilation on XCode:
	return FALSE8;
#if 0
	char *filename;
	char sourcePath[128];
	char destPath[128];

	// Do we need to open new files
	if (m_src_fp == NULL && m_dst_fp == NULL) {
		// Get next filename from the filelist
		filename = GetFileListEntry();

		// Have we finished
		if (filename != NULL) {
			// Construct the new source and destination names
			sprintf(sourcePath, "m\\%s\\%s", m_missionDir, filename);
			sprintf(destPath, "m\\%s\\%s", m_missionDir, filename);

			// Get size
			m_currentFileSize = GetFileSize(sourcePath);
			if (m_currentFileSize == 0)
				Fatal_error(pxVString("Couldn't get size of file: %s", sourcePath));

			// New file so reset chunk counter
			m_chunkCounter = 0;

			// Open the source file
			m_src_fp = openDiskFileForBinaryRead(sourcePath);
			if (m_src_fp == NULL)
				Fatal_error(pxVString("ClusterManager::InstallMission() couldn't open file: %s for reading", sourcePath));

			// Now open the destination file
			m_dst_fp = openDiskFileForBinaryWrite(destPath);
			if (m_dst_fp == NULL)
				Fatal_error(pxVString("ClusterManager::InstallMission() couldn't open file: %s for writing", destPath));
		} else {
			// Huzzah!  All done!
			m_installDone = TRUE8;
		}
	}

	if (m_installDone == FALSE8) {
		// Default chunk size
		uint32 chunkSize = CHUNKSIZE;

		// End of file flag
		bool8 endOfFile = FALSE8;

		// Do we need to deal with the dreggs of a file
		if ((m_chunkCounter + 1) * CHUNKSIZE > m_currentFileSize) {
			// Calculate size of dog end
			chunkSize = m_currentFileSize - (m_chunkCounter * CHUNKSIZE);
			endOfFile = TRUE8;
		}

		// Get a chunk sized piece of memory
		char *data = new char[chunkSize];
		if (data == NULL)
			Fatal_error("ClusterManager::InstallMission() couldn't get memory!");

		// Read source file to memory
		if (fread(data, 1, chunkSize, m_src_fp) != chunkSize)
			Fatal_error("ClusterManager::InstallMission() failed to read from file");

		// Write memory to destination file
		if (fwrite(data, 1, chunkSize, m_dst_fp) != chunkSize)
			Fatal_error("ClusterManager::InstallMission() failed to write to file");

		// Free up the memory we've used
		delete [] data;

		// Done a chunk
		m_chunkCounter++;
		m_bytesDone += chunkSize;

		// End of file check
		if (endOfFile) {
			// End of file reached

			// Close the source file
			fclose(m_src_fp);
			m_src_fp = NULL;
			// Close the destination file
			fclose(m_dst_fp);
			m_dst_fp = NULL;
		}
	}

	// Update progress status
	UpdateProgressBits();

	// Increment counter
	m_frameCounter++;

	// Update the screen visuals
	return DrawCoverFrame();
#endif
}

void ClusterManager::InterrogateDrives() {
#if 0
	int buffersize = 128;
	char driveStrings[128];

	m_multipleCDDrives = FALSE8;

	// Fill with terminators
	memset(driveStrings, 0, buffersize);

	// This function lists all drives on the current system separated by terminators
	int len = GetLogicalDriveStrings(buffersize, driveStrings);

	// A pointer to the string list
	char *ptr = driveStrings;
	int cursor = 0;
	bool8 foundCDdrive = FALSE8;

	while (cursor < len) {
		if (GetDriveType(ptr + cursor) == DRIVE_CDROM) {
			if (foundCDdrive == FALSE) {
				// Record the drive that's the (first) CD drive
				strcpy(m_cdroot1, (const char *)(ptr + cursor));
				foundCDdrive = TRUE8;
			} else {
				// Record the drive that's the (second) CD drive
				strcpy(m_cdroot2, (const char *)(ptr + cursor));
				m_multipleCDDrives = TRUE8;
				break;
			}
		}

		// Move to next drive sting in the list
		cursor += strlen((const char *)ptr + cursor) + 1;
	}

	// Now see how much space we have on the current (therefore installed) drive
	CalculateFreeDiskSpace();
#endif
}

void ClusterManager::CalculateFreeDiskSpace(void) {
	warning("STUB: ClusterManager::CalculateFreeDiskSpace()");
#if 0
	DWORD sectorsPerCluster;
	DWORD bytesPerSector;
	DWORD numberOfFreeClusters;
	DWORD totalNumberOfClusters;

	if (GetDiskFreeSpace(NULL, &sectorsPerCluster, &bytesPerSector, &numberOfFreeClusters, &totalNumberOfClusters) == 0) {
		char error_message[256];
		// Figure out why we failed
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
		              NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		              (LPTSTR)&error_message, 0, NULL);

		Fatal_error(pxVString("ClusterManager::InterrogateDrives() GetLastError(): %s", error_message));
	}

	// For megabytes free, divide this number by (1024*1024)
	m_bytesFreeOnInstalledDrive = numberOfFreeClusters * sectorsPerCluster * bytesPerSector;
#endif
	m_bytesFreeOnInstalledDrive = 256 * 1024 * 1024;
}

char *ClusterManager::GetCDRoot(void) {
	if (m_activeCDDrive == 1)
		return m_cdroot1;
	else
		return m_cdroot2;
}

int ClusterManager::WhichCD(MISSION_ID mission) {
	// All demos exist on one CD only
	int demo = g_globalScriptVariables.GetVariable("demo");
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

bool8 ClusterManager::CheckForCD(int /*number*/) {
#if 1 // was #ifdef FROM_PC_CD
	strcpy(m_cdroot1, "");
	strcpy(m_cdroot2, "");
	return TRUE8;
#else // FROM_PC_CD

	return TRUE8;

#endif

#if 0
	char volumeName[128];
	char volumeName2[128];
	DWORD volumeNameLength = 128;
	DWORD unused = 0;

	// This is to prevent against windows getting arsey when there's no CD in the CD
	// drive; which, of course, is what we're trying to handle within this code.
	UINT oldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

	// Get the volume name - don't care about anything else
	GetVolumeInformation(m_cdroot1, volumeName, volumeNameLength, NULL, NULL, NULL, NULL, unused);

	if (m_multipleCDDrives)
		GetVolumeInformation(m_cdroot2, volumeName2, volumeNameLength, NULL, NULL, NULL, NULL, unused);

	// Return this to it's previous state now that we've finished
	SetErrorMode(oldErrorMode);

	char *seekingVolume = NULL;

	if (number == 1)
		seekingVolume = "ICB_CD1";
	else if (number == 2)
		seekingVolume = "ICB_CD2";
	else if (number == 3)
		seekingVolume = "ICB_CD3";
	else
		Fatal_error(pxVString("ClusterManager::CheckForCD(# %d) unknown CD number to find!", number));

	// Is the correctly named CD in the drive
	if (strcmp(volumeName, seekingVolume) == 0) {
		m_activeCDDrive = 1;
		return TRUE8;
	}

	// Multiple drive support
	if (m_multipleCDDrives) {
		if (strcmp(volumeName2, seekingVolume) == 0) {
			m_activeCDDrive = 2;
			return TRUE8;
		}
	}

	return FALSE8;
#endif
}

void ClusterManager::MinimumInstallCheck() {
#if 0
	// Look for an empty file that the installer placed next to the engine
	if (checkFileExists("minimum", 0))
		m_minimumInstall = TRUE8;
	else
#endif
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

#if 0
	// This needs to be a recursive delete under the clustered mission directory

	// Ideally this would only need to remove a single mission at a time
	// but to be safe I think it should try to remove everything under missions

	// For all missions
	for (uint32 i = 0; i < NUMBER_OF_MISSIONS; i++) {
		char h_mission[8];
		HashFile(mission_names[i], h_mission);

		pxString missionDirectory;
		missionDirectory.Format("m\\%s\\", h_mission);

		// First determine if this mission is on the hard disk
		if (!checkFileExists(missionDirectory)) // amode = 0
			continue;

		// Validation function (don't want to be deleting shit without being 100%)
		ValidateDirectoryToDelete(missionDirectory);

		// Remove everything under the mission directory
		RecursivelyDeleteDirectory(missionDirectory);

		// Finally delete the mission directory itself (only possible if empty)

		// Need to make sure that it's not read-only first
		_chmod(missionDirectory, _S_IWRITE);

		// Delete directory
		if (rmdir(missionDirectory) == -1) {
			int lastChar = missionDirectory.GetLen() - 1;

			// Need to chop of slash character
			if (missionDirectory[lastChar] == '\\') {
				missionDirectory[lastChar] = '\0';

				// Try again
				if (rmdir(missionDirectory) == -1) {
					// Couldn't delete this!
					Fatal_error(pxVString("Couldn't delete directory: %s", (const char *)missionDirectory));
				}
			} else {
				// Couldn't delete this!
				Fatal_error(pxVString("Couldn't delete directory: %s", (const char *)missionDirectory));
			}
		}
	}
#endif
}

void ClusterManager::MissingCD(int /*number*/) {
#if 0
	uint32      halfScreen = SCREEN_WIDTH / 2;
	uint32      temp;
	char        msg[128];
	uint8      *ad;
	uint32      pitch;

	// This function needs to display a pleasant message and loop internally until we either
	// detect the correct cd in the drive or the user cancels

	const char *message = g_theOptionsManager->GetTextFromReference(HashString("opt_missingdisc"));
	char *ok = (char *)g_theOptionsManager->GetTextFromReference(HashString("opt_continue"));
	char *quit = (char *)g_theOptionsManager->GetTextFromReference(HashString("opt_quit"));

	// Blank the screen
	surface_manager->Clear_surface(working_buffer_id);

	sprintf(msg, message, number);
	bool8 okSelected = TRUE8;
	bool8 checking = FALSE8;
	bool8 gotCD = FALSE8;

	while (gotCD == FALSE8) {
		// Important this bit
		Poll_direct_input();

		// Deal with user input first
		if (Read_DI_once_keys(Common::KEYCODE_RETURN) || Read_DI_once_keys(Common::KEYCODE_SPACE)) {
			if (okSelected) {
				checking = TRUE8;
			} else {
				// User wishes to quit
				Common::Event event;
				event.type = Common::EVENT_QUIT;
				g_system->getEventManager()->pushEvent(event);
				return;
			}
		}
		if (Read_DI_once_keys(Common::KEYCODE_LEFT) || Read_DI_once_keys(Common::KEYCODE_RIGHT)) {
			// Toggle selection
			if (okSelected)
				okSelected = FALSE8;
			else
				okSelected = TRUE8;
		}

		// This block looks for the cd over a short loop
		if (checking) {
			// Alter the screen display to show we are checking
			ad = surface_manager->Lock_surface(working_buffer_id);
			pitch = surface_manager->Get_pitch(working_buffer_id);

			// Display message
			g_theOptionsManager->DisplayText(ad, pitch, msg, 0, 90, PALEFONT, TRUE8);

			// Display choices
			temp = g_theOptionsManager->CalculateStringWidth(ok);
			g_theOptionsManager->DisplayText(ad, pitch, ok, halfScreen - temp - 20, 130, PALEFONT, FALSE8);
			g_theOptionsManager->DisplayText(ad, pitch, quit, halfScreen + 20, 130, PALEFONT, FALSE8);

			surface_manager->Unlock_surface(working_buffer_id);

			// Update screen manually
			surface_manager->Flip();

			for (uint32 tik = 0; tik < CD_SEARCH_DELAY; tik++) {
				// Check for the CD again
				if (CheckForCD(number)) {
					// Cool, got it

					// Black out screen manually
					surface_manager->Clear_surface(working_buffer_id);
					surface_manager->Flip();

					gotCD = TRUE8;
					break;
				}
			}
			checking = FALSE8;
		}

		// we don't use this function so SendMessage is just disabled
		//SendMessage(windowHandle, WM_PAINT, 0, 0);

		if (gotCD)
			break;

		ad = surface_manager->Lock_surface(working_buffer_id);
		pitch = surface_manager->Get_pitch(working_buffer_id);

		// Display message
		g_theOptionsManager->DisplayText(ad, pitch, msg, 0, 90, NORMALFONT, TRUE8, TRUE8);

		// Display choices
		temp = g_theOptionsManager->CalculateStringWidth(ok);
		g_theOptionsManager->DisplayText(ad, pitch, ok, halfScreen - temp - 20, 130, (bool8)(okSelected == TRUE8) ? SELECTEDFONT : NORMALFONT, FALSE8);
		g_theOptionsManager->DisplayText(ad, pitch, quit, halfScreen + 20, 130, (bool8)(okSelected == FALSE8) ? SELECTEDFONT : NORMALFONT, FALSE8);

		surface_manager->Unlock_surface(working_buffer_id);

		// Update screen manually
		surface_manager->Flip();
	}
#endif
}

bool8 ClusterManager::MissingCDWithCancel(int /*number*/) {
#if 0
	uint32      halfScreen = SCREEN_WIDTH / 2;
	uint32      temp;
	char        msg[128];
	uint8      *ad;
	uint32      pitch;

	// This function needs to display a pleasant message and loop internally until we either
	// detect the correct cd in the drive or the user cancels

	const char *message = g_theOptionsManager->GetTextFromReference(HashString("opt_missingdisc"));
	char *ok = (char *)g_theOptionsManager->GetTextFromReference(HashString("opt_continue"));
	char *quit = (char *)g_theOptionsManager->GetTextFromReference(HashString("opt_exitgame"));
	char *cancel = (char *)g_theOptionsManager->GetTextFromReference(HashString("opt_back"));

	// Blank the screen
	surface_manager->Clear_surface(working_buffer_id);

	sprintf(msg, message, number);
	int selection = 0;
	bool8 checking = FALSE8;
	bool8 gotCD = FALSE8;

	while (gotCD == FALSE8) {
		// Important this bit
		Poll_direct_input();

		// Deal with user input first
		if (Read_DI_once_keys(Common::KEYCODE_RETURN) || Read_DI_once_keys(Common::KEYCODE_SPACE)) {
			if (selection == 0) {
				checking = TRUE8;
			} else if (selection == 1) {
				// User cancelled
				return TRUE8;
			} else {
				// User wishes to quit
				Common::Event event;
				event.type = Common::EVENT_QUIT;
				g_system->getEventManager()->pushEvent(event);
				return TRUE8;
			}
		}
		if (Read_DI_once_keys(Common::KEYCODE_UP)) {
			// Toggle selection
			if (selection == 0)
				selection = 2;
			else
				selection--;
		}
		if (Read_DI_once_keys(Common::KEYCODE_DOWN)) {
			// Toggle selection
			if (selection == 2)
				selection = 0;
			else
				selection++;
		}

		// This block looks for the cd over a short loop
		if (checking) {
			// Alter the screen display to show we are checking
			ad = surface_manager->Lock_surface(working_buffer_id);
			pitch = surface_manager->Get_pitch(working_buffer_id);

			// Display message
			g_theOptionsManager->DisplayText(ad, pitch, msg, 0, 90, PALEFONT, TRUE8);

			// Display choices
			temp = g_theOptionsManager->CalculateStringWidth(ok);
			g_theOptionsManager->DisplayText(ad, pitch, ok, halfScreen - (temp / 2), 130, PALEFONT, FALSE8);
			temp = g_theOptionsManager->CalculateStringWidth(cancel);
			g_theOptionsManager->DisplayText(ad, pitch, cancel, halfScreen - (temp / 2), 160, PALEFONT, FALSE8);
			temp = g_theOptionsManager->CalculateStringWidth(quit);
			g_theOptionsManager->DisplayText(ad, pitch, quit, halfScreen - (temp / 2), 200, PALEFONT, FALSE8);

			surface_manager->Unlock_surface(working_buffer_id);

			// Update screen manually
			surface_manager->Flip();

			for (uint32 tik = 0; tik < CD_SEARCH_DELAY; tik++) {
				// Check for the CD again
				if (CheckForCD(number)) {
					// Cool, got it

					// Black out screen manually
					surface_manager->Clear_surface(working_buffer_id);
					surface_manager->Flip();

					gotCD = TRUE8;
					break;
				}
			}
			checking = FALSE8;
		}

		// we don't use this function so SendMessage is just disabled
		//SendMessage(windowHandle, WM_PAINT, 0, 0);

		if (gotCD)
			break;

		ad = surface_manager->Lock_surface(working_buffer_id);
		pitch = surface_manager->Get_pitch(working_buffer_id);

		// Display message
		g_theOptionsManager->DisplayText(ad, pitch, msg, 0, 90, NORMALFONT, TRUE8, TRUE8);

		// Display choices
		temp = g_theOptionsManager->CalculateStringWidth(ok);
		g_theOptionsManager->DisplayText(ad, pitch, ok, halfScreen - (temp / 2), 130, (bool8)(selection == 0) ? SELECTEDFONT : NORMALFONT, FALSE8);
		temp = g_theOptionsManager->CalculateStringWidth(cancel);
		g_theOptionsManager->DisplayText(ad, pitch, cancel, halfScreen - (temp / 2), 160, (bool8)(selection == 1) ? SELECTEDFONT : NORMALFONT, FALSE8);
		temp = g_theOptionsManager->CalculateStringWidth(quit);
		g_theOptionsManager->DisplayText(ad, pitch, quit, halfScreen - (temp / 2), 200, (bool8)(selection == 2) ? SELECTEDFONT : NORMALFONT, FALSE8);

		surface_manager->Unlock_surface(working_buffer_id);

		// Update screen manually
		surface_manager->Flip();
	}
#endif
	return FALSE8;
}

void ClusterManager::LoadFileList(MISSION_ID /*mission*/) {
	if (m_minimumInstall == FALSE8)
		return;

#if 0
	// Unload if we need to
	if (m_filelistCursor != -1) {
		memset(m_theList, 0, MAX_BYTESIZE_OF_A_FILELIST);
		m_filelistCursor = -1;
	}

	// Make path to the correct filelist (on the CD)
	pxString filelist;
	HashFile(mission_names[mission], m_missionDir);

	filelist.Format("%sm\\%s\\filelist.txt", GetCDRoot(), m_missionDir);

	// Cool, now find out how big it is
	m_filelistSize = GetFileSize(filelist);
	if (m_filelistSize == 0)
		Fatal_error(pxVString("Couldn't get size of %s", (const char *)filelist));

	// Less four bytes for the total byte count
	m_filelistSize -= 4;

	// Open the file in binary mode
	FILE *fp = openDiskFileForBinaryRead(filelist);
	if (fp == NULL)
		Fatal_error(pxVString("Couldn't open file %s for reading", (const char *)filelist));

	// Read the total byte count
	if (fread(&m_filelistTotalBytes, 1, 4, fp) != 4)
		Fatal_error("Failed to read total byte count from filelist");

	// Read that puppy in
	if (fread(m_theList, 1, m_filelistSize, fp) != (uint32)m_filelistSize)
		Fatal_error("Failed to read total byte count from filelist");

	fclose(fp);

	// Reset our cursor
	m_filelistCursor = 0;

	// Get pointer to the list in memory
	char *ptr = (char *)m_theList;

	// Now change all the newline markers to terminators
	while (ptr[m_filelistCursor]) {
		// New line encountered
		if (ptr[m_filelistCursor] == 0x0d) {
			// Overwrite with terminators
			ptr[m_filelistCursor] = 0;
			ptr[m_filelistCursor + 1] = 0;
			m_filelistCursor++;
		}

		m_filelistCursor++;
	}

	// Reset our cursor
	m_filelistCursor = 0;
#endif
}

char *ClusterManager::GetFileListEntry() {
	if (m_filelistCursor == -1)
		Fatal_error("Can't retrieve filelist entry without loading a filelist first!");

	char *line = NULL;

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
	int width = 5;
	int height = 15;
	int spacing = 2;
	int initialY = SCREEN_DEPTH - height - 30;

	// Calculate entire width so we can centre things
	int length = (NUMBER_OF_PROGRESS_BITS * (width + spacing)) - spacing;
	int initialX = (SCREEN_WIDTH / 2) - (length / 2);

	for (int i = 0; i < NUMBER_OF_PROGRESS_BITS; i++) {
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
		for (int i = 0; i < NUMBER_OF_PROGRESS_BITS; i++) {
			if (m_progressBits[i].state > 0 && m_progressBits[i].state != 6) {
				m_progressBits[i].state = m_progressBits[i].state + 1;
			}
		}
	}

	// Bytes per progress bit
	float progress_inc = (float)(m_filelistTotalBytes / NUMBER_OF_PROGRESS_BITS);

	// The number of bits that should bit switched on
	uint bitsOn = (int)((float)m_bytesDone / progress_inc);

	// Do we need to switch on a new bit
	if (bitsOn > m_bitsDone) {
		m_progressBits[m_bitsDone].state = 1;
		m_bitsDone++;
	}
}

void ClusterManager::DrawProgressBits() {
	for (int i = 0; i < NUMBER_OF_PROGRESS_BITS; i++) {
		Fill_rect(m_progressBits[i].r.left, m_progressBits[i].r.top, m_progressBits[i].r.right, m_progressBits[i].r.bottom, g_progressColourMap[m_progressBits[i].state]);
	}
}

void ClusterManager::Shutdown(void) {
	// Only works on minimum install
	CleanHardDisk();
}

void RecursivelyDeleteDirectory(const char * /*path*/) {
	warning("STUB: RecursivelyDeleteDirectory()");
#if 0
	// Parse this directory
	DirectorySearch search(path);

	// While there are things in the directory
	while (search.GetNextFile()) {
		// Is this a file or a directory
		if (search.GetFileType() & _A_SUBDIR) {
			pxString subdir = search.GetPathName();

			// Recurse on this directory
			RecursivelyDeleteDirectory(subdir);

			// Then remove it too

			// Ensure write access
			_chmod(subdir, _S_IWRITE);

			// Delete subdirectory
			if (rmdir(subdir) == -1) {
				int lastChar = subdir.GetLen() - 1;

				// Need to chop of slash character
				if (subdir[lastChar] == '\\') {
					subdir[lastChar] = '\0';

					// Try again
					if (rmdir(subdir) == -1) {
						// Couldn't delete this!
						Fatal_error(pxVString("Couldn't delete directory: %s", (const char *)subdir));
					}
				} else {
					// Couldn't delete this!
					Fatal_error(pxVString("Couldn't delete directory: %s", (const char *)subdir));
				}
			}
		} else {
			// Need to make sure that files aren't read-only first
			_chmod(search.GetPathName(), _S_IWRITE);

			// Delete file
			if (_unlink(search.GetPathName()) == -1) {
				// Couldn't delete this file
				MessageBox(NULL, pxVString("Couldn't delete file: %s", (const char *)search.GetPathName()), "ClusterManager Says:", MB_OK);
			}
		}
	}
#endif
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
	return NULL;
}

void MakeDirectoryTree(MISSION_ID /*mission*/) {
	warning("STUB: MakeDirectoryTree()");
#if 0
	char pathToMake[128];

	// First make the mission directory
	char missionDir[8];
	HashFile(mission_names[mission], missionDir);

	sprintf(pathToMake, "m\\%s", missionDir);
	if (_mkdir(pathToMake) != 0)
		Fatal_error(pxVString("MakeDirectoryTree(): Failed to make directory: %s", pathToMake));

	// Get the name of this mission
	char *mission_name = MissionIdToName(mission);

	// Now the session directories
	for (uint32 i = 0; i < MAX_SESSIONS; i += 2) {
		// Only build the session directories for this mission
		if (strcmp(mission_name, sessions[i]) == 0) {
			char sessionDir[8];
			HashFile(sessions[i + 1], sessionDir);

			sprintf(pathToMake, "m\\%s\\%s", missionDir, sessionDir);
			if (_mkdir(pathToMake) != 0)
				Fatal_error(pxVString("MakeDirectoryTree(): Failed to make directory: %s", pathToMake));
		}
	}
#endif
}

} // End of namespace ICB
