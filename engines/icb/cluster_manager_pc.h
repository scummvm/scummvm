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

#ifndef ICB_CLUSTER_MANAGER_PC_H_INCLUDED
#define ICB_CLUSTER_MANAGER_PC_H_INCLUDED

#include "engines/icb/common/px_rccommon.h"

namespace ICB {

// Global cluster controller declaration
class ClusterManager;
class MovieManager;
extern ClusterManager *g_theClusterManager;
extern MovieManager *g_while_u_wait_SequenceManager;
// Byte limit of filelists
#define MAX_BYTESIZE_OF_A_FILELIST (4 * 1024)

enum MISSION_ID { MISSION1 = 0, MISSION2 = 1, MISSION3 = 2, MISSION4 = 3, MISSION5 = 4, MISSION7 = 5, MISSION8 = 6, MISSION9 = 7, MISSION10 = 8 };

#define NUMBER_OF_PROGRESS_BITS 32

typedef struct {
	LRECT r;
	int32 state;
} PROGRESS_BIT;

enum TLANGUAGE { T_ENGLISH, T_FRENCH, T_ITALIAN, T_GERMAN, T_SPANISH, T_RUSSIAN, T_POLISH };

class ClusterManager {
private:
	// Drive path of CD drives on installed system
	char m_cdroot1[1024];
	char m_cdroot2[1024];
	bool8 m_multipleCDDrives;
	int m_activeCDDrive;

	// Working mission directory name (hashed)
	char m_missionDir[8];

	// You guessed it
	uint32 m_bytesFreeOnInstalledDrive;

	// As cluster management is only necessary on minimum installs this
	// acts as an on\off switch (state determined via VISE installer)
	bool8 m_minimumInstall;

	// Filelist handling
	char m_theList[MAX_BYTESIZE_OF_A_FILELIST];
	uint32 m_filelistTotalBytes;
	int32 m_filelistCursor;
	int32 m_filelistSize;

	// These are to handle the chunk copying
	Common::SeekableReadStream *m_src_fp;
	Common::WriteStream *m_dst_fp;
	uint32 m_currentFileSize;
	uint32 m_chunkCounter;
	uint32 m_bytesDone;
	bool8 m_installDone;

	// Pointer to memory resident bink file
	char *m_movieMemoryPointer;

	// Progress bar structures and counter
	PROGRESS_BIT m_progressBits[NUMBER_OF_PROGRESS_BITS];
	uint32 m_bitsDone;

	// Used to control decay on progress bar
	uint32 m_frameCounter;

	TLANGUAGE m_currentLanguage;

public:
	ClusterManager();
	~ClusterManager();

	// Startup
	void Initialise();

	// Useful methods for external modules
	bool8 AreWeRunningAMinimumInstall() { return m_minimumInstall; }
	char *GetCDRoot();
	void CheckDiscInserted(MISSION_ID mission);
	bool8 CheckDiscInsertedWithCancel(MISSION_ID mission);
	void CheckAnyDiscInserted();

	// These handle the mission swapping
	bool8 StartMissionInstall(MISSION_ID mission);
	bool8 InstallMission();

	// Shutdown
	void Shutdown();

	// Language enquirer (set in Initialise() function called at startup)
	TLANGUAGE GetLanguage() { return m_currentLanguage; }

private:
	// Administration functions
	void InterrogateDrives();
	void CalculateFreeDiskSpace();
	void MinimumInstallCheck();
	bool8 IsMissionDataInstalled(MISSION_ID &m);
	void CleanHardDisk();
	int WhichCD(MISSION_ID mission);
	bool8 CheckForCD(int number);
	void MissingCD(int number);
	bool8 MissingCDWithCancel(int number);

	// Filelist handling
	void LoadFileList(MISSION_ID mission);
	char *GetFileListEntry();

	// Visual update
	bool8 DrawCoverFrame();

	// Progress bar management
	void InitialiseProgressBits();
	void UpdateProgressBits();
	void DrawProgressBits();
};

} // End of namespace ICB

#endif
