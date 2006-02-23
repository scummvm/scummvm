/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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

#include "common/stdafx.h"
#include "common/system.h"
#include "common/file.h"

#include "cine/cine.h"

uint16 snd_eventsDelay;
int snd_songIsPlaying = 0;
uint8 snd_nullInstrument[] = { 0, 0 };
sfxStateStruct snd_sfxState;

static uint8 snd_mute = 0;
static char snd_songFileName[30];

/* LVDT specific */
static Common::File *snd_baseSndFile = NULL;
static uint16 snd_numBasesonEntries = 0;
static BasesonEntryStruct *snd_basesonEntries = NULL;

int snd_loadBasesonEntries(const char *fileName) {
	int i;

	snd_baseSndFile = new Common::File();
	snd_baseSndFile->open(fileName);
	if (!snd_baseSndFile->isOpen())
		return -1;

	snd_numBasesonEntries = snd_baseSndFile->readUint16BE();
	snd_baseSndFile->readUint16BE();	/* entry_size */
	snd_basesonEntries = (BasesonEntryStruct *)malloc(snd_numBasesonEntries * sizeof(BasesonEntryStruct));
	if (snd_basesonEntries) {
		for (i = 0; i < snd_numBasesonEntries; ++i) {
			BasesonEntryStruct *be = &snd_basesonEntries[i];
			snd_baseSndFile->read(be->name, 14);
			be->offset = snd_baseSndFile->readUint32BE();
			be->size = snd_baseSndFile->readUint32BE();
			be->unpackedSize = snd_baseSndFile->readUint32BE();
			snd_baseSndFile->readUint32BE();	/* unused */
		}
	}
	return 0;
}

void snd_clearBasesonEntries() {
	snd_baseSndFile->close();
	delete snd_baseSndFile;
	free(snd_basesonEntries);
	snd_basesonEntries = NULL;
	snd_numBasesonEntries = 0;
}

static int snd_findBasesonEntry(const char *entryName) {
	int i;
	char *p;
	char basesonEntryName[20];

	assert(strlen(entryName) < 20);
	strcpy(basesonEntryName, entryName);
	for (p = basesonEntryName; *p; ++p) {
		if (*p >= 'a' && *p <= 'z')
			*p += 'A' - 'a';
	}

	for (i = 0; i < snd_numBasesonEntries; ++i) {
		if (strcmp(snd_basesonEntries[i].name, basesonEntryName) == 0)
			return i;
	}
	return -1;
}

static uint8 *snd_loadBasesonEntry(const char *entryName) {
	int entryNum;
	uint8 *entryData = NULL;

	if (gameType == Cine::GID_OS) {
		entryNum = findFileInBundle((const char *)entryName);
		if (entryNum != -1)
			entryData = readBundleFile(entryNum);
	} else {
		entryNum = snd_findBasesonEntry(entryName);
		if (entryNum != -1 && entryNum < snd_numBasesonEntries) {
			const BasesonEntryStruct *be = &snd_basesonEntries[entryNum];
			entryData = (uint8 *)malloc(be->unpackedSize);
			if (entryData) {
				if (be->unpackedSize > be->size) {
					uint8 *tempData = (uint8 *)malloc(be->size);
					if (tempData) {
						snd_baseSndFile->seek(be->offset, SEEK_SET);
						snd_baseSndFile->read(tempData, be->size);
						decomp(tempData + be->size - 4, entryData + be->unpackedSize, be->unpackedSize);
						free(tempData);
					}
				} else {
					snd_baseSndFile->seek(be->offset, SEEK_SET);
					snd_baseSndFile->read(entryData, be->size);
				}
			}
		}
	}

	return entryData;
}

void snd_stopSong() {
	int i;

	snd_songFileName[0] = '\0';
	snd_songIsPlaying = 0;
	snd_fadeOutCounter = 0;

	for (i = 0; i < 4; ++i)
		(*snd_driver.stopChannel) (i);

	snd_adlibDriverStopSong();
	snd_freeSong();
}

void snd_freeSong() {
	int i;

	for (i = 0; i < 15; ++i) {
		if (snd_sfxState.instruments[i] != snd_nullInstrument)
			free(snd_sfxState.instruments[i]);
	}
	free(snd_sfxState.songData);
	memset(&snd_sfxState, 0, sizeof(snd_sfxState));
}

int snd_loadSong(const char *songName) {
	int i;

	while (snd_fadeOutCounter != 0 && snd_fadeOutCounter < 100)
		g_system->delayMillis(40);

	snd_fadeOutCounter = 0;

	if (snd_songIsPlaying)
		snd_stopSong();

	if ((gameType == Cine::GID_OS) && (strncmp(songName, "INTRO", 5) == 0))
		return 0;

	strcpy(snd_songFileName, songName);
	if (gameType == Cine::GID_OS)
		strcat(snd_songFileName, ".IST");

	snd_sfxState.songData = snd_loadBasesonEntry(songName);
	if (!snd_sfxState.songData)
		return 0;

	for (i = 0; i < 15; ++i) {
		char instrumentName[13];
		memcpy(instrumentName, snd_sfxState.songData + 20 + i * 30, 12);
		instrumentName[12] = '\0';

		snd_sfxState.instruments[i] = snd_nullInstrument;
		if (strlen(instrumentName) != 0) {
			char *dot = strrchr(instrumentName, '.');
			if (dot)
				*dot = '\0';

			if (gameType == Cine::GID_OS)
				strcat(instrumentName, ".ADL");
			else
				strcat(instrumentName, ".INS");

			snd_sfxState.instruments[i] =
			    snd_loadBasesonEntry(instrumentName);
		}
	}
	return 1;
}

void snd_fadeOutSong() {
	if (snd_songIsPlaying) {
		snd_songFileName[0] = '\0';
		snd_songIsPlaying = 0;
		snd_fadeOutCounter = 1;
	}
}

void snd_playSong() {
	if (strlen(snd_songFileName) != 0) {
		snd_sfxState.currentInstrumentChannel[0] = -1;
		snd_sfxState.currentInstrumentChannel[1] = -1;
		snd_sfxState.currentInstrumentChannel[2] = -1;
		snd_sfxState.currentInstrumentChannel[3] = -1;
		snd_sfxState.currentOrder = 0;
		snd_sfxState.currentPos = 0;
		snd_sfxState.numOrders = snd_sfxState.songData[470];
		snd_eventsDelay = (252 - snd_sfxState.songData[471]) * 25 * 2 / 1060;
		snd_songTicksCounter = 0;
		snd_songIsPlaying = 1;
	}
}

void snd_handleEvents() {
	int i;
	const uint8 *patternData = snd_sfxState.songData + 600;
	const uint8 *orderTable = snd_sfxState.songData + 472;
	uint16 patternNum = orderTable[snd_sfxState.currentOrder] * 1024;

	for (i = 0; i < 4; ++i) {
		snd_handlePattern(i, patternData + patternNum + snd_sfxState.currentPos);
		patternData += 4;
	}

	if (snd_fadeOutCounter != 0 && snd_fadeOutCounter < 100)
		snd_fadeOutCounter += 4;

	snd_sfxState.currentPos += 16;
	if (snd_sfxState.currentPos >= 1024) {
		snd_sfxState.currentPos = 0;
		++snd_sfxState.currentOrder;
		if (snd_sfxState.currentOrder == snd_sfxState.numOrders)
			snd_sfxState.currentOrder = 0;
	}
}

void snd_handlePattern(int channelNum, const uint8 *patternData) {
	uint16 instrNum = patternData[2] >> 4;
	snd_adlibInstrumentsTable[channelNum] = snd_nullInstrument;
	if (instrNum != 0) {
		if (snd_sfxState.currentInstrumentChannel[channelNum] != instrNum) {
			snd_sfxState.currentInstrumentChannel[channelNum] = instrNum;
			(*snd_driver.setupChannel) (channelNum, snd_sfxState.instruments[instrNum - 1], instrNum - 1);
		} else if (snd_fadeOutCounter != 0) {
			instrNum = snd_sfxState.currentInstrumentChannel[channelNum];
			if (instrNum != 0)
				(*snd_driver.setupChannel)(channelNum, snd_sfxState.instruments[instrNum - 1], instrNum - 1);
		}
		snd_adlibInstrumentsTable[channelNum] = snd_sfxState.instruments[instrNum - 1];
	}
	if (snd_mute != 0)
		(*snd_driver.stopChannel)(channelNum);
	else {
		int16 freq = (int16)readU16BE(patternData);
		if (freq > 0) {
			(*snd_driver.stopChannel)(channelNum);
			(*snd_driver.setChannelFrequency)(channelNum, freq);
		}
	}
}
