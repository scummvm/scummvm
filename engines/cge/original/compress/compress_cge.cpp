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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compress_cge.h"

#define BUFFER_SIZE 8192
#define SEED 0xA5

static void crypt(const char *src, const char *dest) {
	FILE *fIn = fopen(src, "rb");
	FILE *fOut = fopen(dest, "wb");
	byte buffer[BUFFER_SIZE];
	size_t bytesRead;
	
	while ((bytesRead = fread(&buffer[0], 1, BUFFER_SIZE, fIn)) > 0) {
		for (int i = 0; i < bytesRead; ++i)
			buffer[i] ^= SEED;

		fwrite(&buffer[0], 1, bytesRead, fOut);
	}

	fclose(fIn);
	fclose(fOut);
}

static void decrypt() {
	crypt("vol.cat", "vol_uncompressed.cat");
}

static void encrypt() {
	crypt("vol_uncompressed.cat", "vol.cat");
}

static void readData(FILE *f, byte *buff, int size) {
	int bytesRead = fread(buff, 1, size, f);
	for (int i = 0; i < bytesRead; ++i)
		buff[i] ^= SEED;
}

static void writeData(FILE *f, byte *buff, int size) {
	for (int i = 0; i < size; ++i)
		buff[i] ^= SEED;
	fwrite(buff, 1, size, f);
}

static void uncompress(const char *destFolder) {
	FILE *volCat, *volDat, *fOut, *fFiles;
	BtPage btPage;

	volCat = fopen("vol.cat", "rb");
	volDat = fopen("vol.dat", "rb");
	fFiles = fopen("files.txt", "w");

	// Get in a list of pages individual files will be on
	readData(volCat, (byte *)&btPage, sizeof(BtPage));

	int pageList[1000];
	int pageCount = btPage._hea._count;
	pageList[0] = btPage._hea._down;
	for (int i = 0; i < pageCount; ++i)
		pageList[i + 1] = btPage._inn[i]._down;
	
	// Loop through the pages of individual files
	for (int i = 0; i <= pageCount; ++i) {
		// Move to correct page and read it
		fseek(volCat, pageList[i] * sizeof(BtPage), SEEK_SET);
		readData(volCat, (byte *)&btPage, sizeof(BtPage));
		
		// Process the files
		for (unsigned int fileNum = 0; fileNum < btPage._hea._count; ++fileNum) {
			char fname[256];
			strcpy(fname, "files\\");
			strcat(fname, btPage._lea[fileNum]._key);
			
			// Add filename to files list
			fprintf(fFiles, "%s\n", btPage._lea[fileNum]._key);

			fOut = fopen(fname, "wb");
			byte *buffer = (byte *)malloc(btPage._lea[fileNum]._size);

			fseek(volDat, btPage._lea[fileNum]._mark, SEEK_SET);
			readData(volDat, buffer, btPage._lea[fileNum]._size);
			fwrite(buffer, 1, btPage._lea[fileNum]._size, fOut);

			fclose(fOut);
			free(buffer);
		}
	}

	fclose(volCat);
	fclose(volDat);
	fclose(fFiles);
}

#define MAX_FILES 5000

static void recompress(const char *srcFolder) {
	FILE *volCat, *volDat, *fIn;
	BtPage btPage;

	// Load in the list of files to recompress
	char files[MAX_FILES][kBtKeySize];
	int fileCount = 0;
	fIn = fopen("files.txt", "r");
	while (!feof(fIn)) {
		fscanf(fIn, "%s", &files[fileCount++][0]);
		if (fileCount == MAX_FILES) {
			printf("Max files reached\n");
			exit(0);
		}
	}
	fclose(fIn);

	// Open vol cat and dat files for writing
	volCat = fopen("vol.cat", "wb");
	volDat = fopen("vol.dat", "wb");

	/* Build the index page */
	// Header
	memset(&btPage, 0, sizeof(BtPage));
	int pageCount = fileCount / LEA_SIZE;
	btPage._hea._count = pageCount;
	btPage._hea._down = 1;

	// Innert file list - lists the first file of the next page
	for (int pageNum = 0; pageNum < pageCount; ++pageNum) {
		int nextFile = (pageNum + 1) * LEA_SIZE;

		btPage._inn[pageNum]._down = pageNum + 2;
		strcpy((char *)&btPage._inn[pageNum]._key[0], files[nextFile]);
	}

	// Write out the index page
	writeData(volCat, (byte *)&btPage, sizeof(BtPage));

	// Loop through processing each page and the dat file
	pageCount = (fileCount + LEA_SIZE - 1) / LEA_SIZE;
	int fileIndex = 0;
	for (int pageNum = 0; pageNum < pageCount; ++pageNum) {
		int startFile = pageNum * LEA_SIZE;
		int lastFile = (pageNum + 1) * LEA_SIZE - 1;
		if (lastFile >= fileCount)
			lastFile = fileCount - 1;

		// Header
		memset(&btPage, 0, sizeof(BtPage));
		btPage._hea._count = lastFile - startFile + 1;
		btPage._hea._down = 0xffff;

		for (int fileNum = 0; fileNum < btPage._hea._count; ++fileNum, ++fileIndex) {
			// Set filename and offset in dat file
			strcpy(btPage._lea[fileNum]._key, &files[fileIndex][0]);
			btPage._lea[fileNum]._mark = ftell(volDat);

			// Load the given file and write it into the dat file
			char fname[32];
			strcpy(fname, srcFolder);
			strcat(fname, files[fileIndex]);

			// Open the file and get the size
			fIn = fopen(fname, "rb");
			fseek(fIn, 0, SEEK_END);
			int fileSize = ftell(fIn);
			fseek(fIn, 0, SEEK_SET);
			btPage._lea[fileNum]._size = fileSize;

			// Allocate buffer space for the file
			byte *buffer = (byte *)malloc(fileSize);

			// Read it in, encrypt it, and write it out
			fread(buffer, 1, fileSize, fIn);
			writeData(volDat, buffer, fileSize);

			free(buffer);
			fclose(fIn);
		}

		// Write out the page
		writeData(volCat, (byte *)&btPage, sizeof(BtPage));
	}

	fclose(volCat);
	fclose(volDat);
}


int main(int argc,  const char *argv[]) {
	recompress("english_files\\");
//	uncompress("files\\");
	return 0;
}

