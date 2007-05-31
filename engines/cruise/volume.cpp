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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL:https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/branches/gsoc2007-fsnode/engines/cruise/volume.cpp $
 * $Id:volume.cpp 26949 2007-05-26 20:23:24Z david_corrales $
 *
 */

#include "cruise/cruise_main.h"

namespace Cruise {

FILE *PAL_fileHandle = NULL;
uint8 *PAL_ptr = NULL;

int16 numLoadedPal;
int16 fileData2;

void loadPal(volumeDataStruct *entry) {
	char name[20];

	return;

	if (PAL_fileHandle) {
		fclose(PAL_fileHandle);
	}

	removeExtention(entry->ident, name);
	strcat(name, ".PAL");

	PAL_fileHandle = fopen(name, "rb");

	fread(&numLoadedPal, 2, 1, PAL_fileHandle);
	fread(&fileData2, 2, 1, PAL_fileHandle);

	flipShort(&numLoadedPal);
	flipShort(&fileData2);

	PAL_ptr = (uint8 *) malloc(numLoadedPal * fileData2);
}

int getVolumeDataEntry(volumeDataStruct *entry) {
	char buffer[256];
	int i;

	volumeNumEntry = 0;
	volumeNumberOfEntry = 0;

	if (currentVolumeFile.isOpen()) {
		freeDisk();
	}

	askDisk(-1);

	strcpyuint8(buffer, entry->ident);

	currentVolumeFile.open(buffer);

	if (!currentVolumeFile.isOpen()) {
		return (-14);
	}

	changeCursor(1);

	currentVolumeFile.read(&volumeNumberOfEntry, 2);
	currentVolumeFile.read(&volumeSizeOfEntry, 2);

	flipShort(&volumeNumberOfEntry);
	flipShort(&volumeSizeOfEntry);

	volumeNumEntry = volumeNumberOfEntry;

	assert(volumeSizeOfEntry == 14 + 4 + 4 + 4 + 4);

	volumePtrToFileDescriptor =
	    (fileEntry *) mallocAndZero(sizeof(fileEntry) * volumeNumEntry);

	for (i = 0; i < volumeNumEntry; i++) {
		volumePtrToFileDescriptor[i].name[0] = 0;
		volumePtrToFileDescriptor[i].offset = 0;
		volumePtrToFileDescriptor[i].size = 0;
		volumePtrToFileDescriptor[i].extSize = 0;
		volumePtrToFileDescriptor[i].unk3 = 0;
	}

	for (i = 0; i < volumeNumEntry; i++) {
		currentVolumeFile.read(&volumePtrToFileDescriptor[i].name, 14);
		currentVolumeFile.read(&volumePtrToFileDescriptor[i].offset,
		    4);
		currentVolumeFile.read(&volumePtrToFileDescriptor[i].size, 4);
		currentVolumeFile.read(&volumePtrToFileDescriptor[i].extSize,
		    4);
		currentVolumeFile.read(&volumePtrToFileDescriptor[i].unk3, 4);
	}

	for (i = 0; i < volumeNumEntry; i++) {
		flipLong(&volumePtrToFileDescriptor[i].offset);
		flipLong(&volumePtrToFileDescriptor[i].size);
		flipLong(&volumePtrToFileDescriptor[i].extSize);
	}

	loadPal(entry);

	return 0;
}

int searchFileInVolCnf(uint8 *fileName, int32 diskNumber) {
	int foundDisk = -1;
	int i;

	for (i = 0; i < numOfDisks; i++) {
		if (volumeData[i].diskNumber == diskNumber) {
			int j;
			int numOfEntry = volumeData[i].size / 13;

			for (j = 0; j < numOfEntry; j++) {
				if (!strcmpuint8(volumeData[i].ptr[j].name,
					fileName)) {
					return (i);
				}
			}
		}
	}

	return (foundDisk);
}

int32 findFileInDisksSub1(uint8 *fileName) {
	int foundDisk = -1;
	int i;

	for (i = 0; i < numOfDisks; i++) {
		int j;
		int numOfEntry = volumeData[i].size / 13;

		for (j = 0; j < numOfEntry; j++) {
			if (!strcmpuint8(volumeData[i].ptr[j].name, fileName)) {
				return (i);
			}
		}
	}

	return (foundDisk);
}

void strToUpper(uint8 *fileName) {
	char character;

	do {
		character = *fileName;

		if (character >= 'a' && character <= 'z') {
			character &= 0xDF;
			*fileName = character;
		}

		fileName++;

	} while (character);
}

void freeDisk(void) {
	if (currentVolumeFile.isOpen()) {
		currentVolumeFile.close();
		free(volumePtrToFileDescriptor);
	}

	/* TODO
	 * if(PAL_fileHandle)
	 * {
	 * freeAllDataPtr();
	 * }
	 */
}

int16 findFileInList(uint8 *fileName) {
	int i;

	if (!currentVolumeFile.isOpen()) {
		return (-1);
	}

	strToUpper(fileName);

	if (volumeNumEntry <= 0) {
		return (-1);
	}

	for (i = 0; i < volumeNumEntry; i++) {
		if (!strcmpuint8(volumePtrToFileDescriptor[i].name, fileName)) {
			return (i);
		}
	}

	return (-1);
}

void askDisk(int16 discNumber) {
	char diskNumberString[256];
	uint8 fileName[256];
	uint8 string[256];
	char messageDrawn = 0;

	if (discNumber != -1) {
		currentDiskNumber = discNumber;
	}
	// skip drive selection stuff

	strcpyuint8(fileName, "VOL.");
	sprintf(diskNumberString, "%d", currentDiskNumber);
	strcatuint8(fileName, diskNumberString);

	strcpyuint8(string, "INSERER LE DISQUE ");
	strcatuint8(string, diskNumberString);
	strcatuint8(string, " EN ");

	//while (Common::File::exists((const char*)fileName))
	{
		if (!messageDrawn) {
			drawMsgString(string);
			messageDrawn = 1;
		}
	}

	changeCursor(currentCursor);
}

int16 findFileInDisks(uint8 *fileName) {
	int disk;
	int fileIdx;

	strToUpper(fileName);

	if (!volumeDataLoaded) {
		printf("CNF wasn't loaded, reading now...\n");
		if (currentVolumeFile.isOpen()) {
			askDisk(-1);
			freeDisk();
		}

		askDisk(1);
		readVolCnf();
	}

	if (currentVolumeFile.isOpen()) {
		askDisk(-1);
	}

	fileIdx = findFileInList(fileName);

	if (fileIdx >= 0) {
		return (fileIdx);
	}

	disk = searchFileInVolCnf(fileName, currentDiskNumber);

	if (disk >= 0) {
		int temp;

		printf("File found on disk %d\n", disk);

		if (currentVolumeFile.isOpen()) {
			askDisk(-1);
		}

		freeDisk();

		askDisk(volumeData[disk].diskNumber);

		getVolumeDataEntry(&volumeData[disk]);

		temp = findFileInList(fileName);

		if (temp >= 0)
			return (temp);

		return (-1);

	} else {
		int temp;

		temp = findFileInDisksSub1(fileName);

		if (temp >= 0) {
			int temp2;

			askDisk(volumeData[temp].diskNumber);

			getVolumeDataEntry(&volumeData[temp]);

			temp2 = findFileInList(fileName);

			if (temp2 >= 0)
				return (temp2);
		}

		return (-1);
	}
}

int16 readVolCnf(void) {
	int i;
	Common::File fileHandle;
	short int sizeHEntry;

	volumeDataLoaded = 0;

	for (i = 0; i < 20; i++) {
		volumeData[i].ident[0] = 0;
		volumeData[i].ptr = NULL;
		volumeData[i].diskNumber = i + 1;
		volumeData[i].size = 0;
	}

	fileHandle.open("VOL.CNF");

	if (!fileHandle.isOpen()) {
		return (0);
	}

	fileHandle.read(&numOfDisks, 2);
	flipShort(&numOfDisks);

	fileHandle.read(&sizeHEntry, 2);
	flipShort(&sizeHEntry);	// size of one header entry - 20 bytes

	for (i = 0; i < numOfDisks; i++) {
		//      fread(&volumeData[i],20,1,fileHandle);
		fileHandle.read(&volumeData[i].ident, 10);
		fileHandle.read(&volumeData[i].ptr, 4);
		fileHandle.read(&volumeData[i].diskNumber, 2);
		fileHandle.read(&volumeData[i].size, 4);

		flipShort(&volumeData[i].diskNumber);
		printf("Disk number: %d\n", volumeData[i].diskNumber);
		flipLong(&volumeData[i].size);
	}

	for (i = 0; i < numOfDisks; i++) {
		fileName *ptr;

		fileHandle.read(&volumeData[i].size, 4);
		flipLong(&volumeData[i].size);

		ptr = (fileName *) mallocAndZero(volumeData[i].size);

		volumeData[i].ptr = ptr;

		if (!ptr) {
			fileHandle.close();
			return (-2);
		}

		fileHandle.read(ptr, volumeData[i].size);
	}

	fileHandle.close();

	volumeDataLoaded = 1;

//#define dumpResources
#ifdef dumpResources

	for (i = 0; i < numOfDisks; i++) {
		int j;
		char nameBuffer[256];
		fileEntry *buffer;

		sprintf(nameBuffer, "D%d.", i + 1);

		fileHandle = fopen(nameBuffer, "rb");

		short int numEntry;
		short int sizeEntry;

		fread(&numEntry, 2, 1, fileHandle);
		fread(&sizeEntry, 2, 1, fileHandle);

		flipShort(&numEntry);
		flipShort(&sizeEntry);

		buffer = (fileEntry *) mallocAndZero(numEntry * sizeEntry);

		fread(buffer, numEntry * sizeEntry, 1, fileHandle);

		for (j = 0; j < numEntry; j++) {
			flipLong(&buffer[j].offset);
			flipLong(&buffer[j].size);
			flipLong(&buffer[j].unk2);
			flipLong(&buffer[j].unk3);

			fseek(fileHandle, buffer[j].offset, SEEK_SET);

			char *bufferLocal;
			bufferLocal = (char *)mallocAndZero(buffer[j].size);

			fread(bufferLocal, buffer[j].size, 1, fileHandle);

			char nameBuffer[256];

			sprintf(nameBuffer, "D%d.dmp/%s", i + 1,
			    buffer[j].name);

			if (buffer[j].size == buffer[j].unk2) {
				FILE *fOut = fopen(nameBuffer, "wb+");
				fwrite(bufferLocal, buffer[j].size, 1, fOut);
				fclose(fOut);
			} else {
				char *uncompBuffer =
				    (char *)mallocAndZero(buffer[j].unk2 +
				    500);

				delphineUnpack((uint8 *) uncompBuffer, (const uint8 *) bufferLocal, buffer[j].size);

				FILE *fOut = fopen(nameBuffer, "wb+");
				fwrite(uncompBuffer, buffer[j].unk2, 1,
				    fOut);
				fclose(fOut);

				//free(uncompBuffer);

			}

			free(bufferLocal);
		}
	}

#endif

	return (1);
}

///////////////////////////::

void drawMsgString(uint8 *string) {
	//printf("%s\n",string);
}

} // End of namespace Cruise
