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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/system.h"
#include "common/file.h"
#include "common/savefile.h"

#include "hugo/game.h"
#include "hugo/hugo.h"
#include "hugo/file.h"
#include "hugo/global.h"
#include "hugo/schedule.h"
#include "hugo/display.h"
#include "hugo/util.h"

namespace Hugo {
FileManager::FileManager(HugoEngine &vm) : _vm(vm) {
}

FileManager::~FileManager() {
}

byte *FileManager::convertPCC(byte *p, uint16 y, uint16 bpl, image_pt dataPtr) {
// Convert 4 planes (RGBI) data to 8-bit DIB format
// Return original plane data ptr
	debugC(2, kDebugFile, "convertPCC(byte *p, %d, %d, image_pt data_p)", y, bpl);

	dataPtr += y * bpl * 8;                         // Point to correct DIB line
	for (int16 r = 0, g = bpl, b = g + bpl, i = b + bpl; r < bpl; r++, g++, b++, i++) { // Each byte in all planes
		for (int8 bit = 7; bit >= 0; bit--) {       // Each bit in byte
			*dataPtr++ = (((p[r] >> bit & 1) << 0) |
			              ((p[g] >> bit & 1) << 1) |
			              ((p[b] >> bit & 1) << 2) |
			              ((p[i] >> bit & 1) << 3));
		}
	}
	return p;
}

seq_t *FileManager::readPCX(Common::File &f, seq_t *seqPtr, byte *imagePtr, bool firstFl, const char *name) {
// Read a pcx file of length len.  Use supplied seq_p and image_p or
// allocate space if NULL.  Name used for errors.  Returns address of seq_p
// Set first TRUE to initialize b_index (i.e. not reading a sequential image in file).
	debugC(1, kDebugFile, "readPCX(..., %s)", name);

	// Read in the PCC header and check consistency
	static PCC_header_t PCC_header;
	PCC_header.mfctr = f.readByte();
	PCC_header.vers = f.readByte();
	PCC_header.enc = f.readByte();
	PCC_header.bpx = f.readByte();
	PCC_header.x1 = f.readUint16LE();
	PCC_header.y1 = f.readUint16LE();
	PCC_header.x2 = f.readUint16LE();
	PCC_header.y2 = f.readUint16LE();
	PCC_header.xres = f.readUint16LE();
	PCC_header.yres = f.readUint16LE();
	f.read(PCC_header.palette, sizeof(PCC_header.palette));
	PCC_header.vmode = f.readByte();
	PCC_header.planes = f.readByte();
	PCC_header.bytesPerLine = f.readUint16LE();
	f.read(PCC_header.fill2, sizeof(PCC_header.fill2));

	if (PCC_header.mfctr != 10)
		Utils::Error(PCCH_ERR, "%s", name);

	// Allocate memory for seq_t if 0
	if (seqPtr == 0) {
		if ((seqPtr = (seq_t *)malloc(sizeof(seq_t))) == 0)
			Utils::Error(HEAP_ERR, "%s", name);
	}
	
	// Find size of image data in 8-bit DIB format
	// Note save of x2 - marks end of valid data before garbage
	uint16 bytesPerLine4 = PCC_header.bytesPerLine * 4; // 4-bit bpl
	seqPtr->bytesPerLine8 = bytesPerLine4 * 2;      // 8-bit bpl
	seqPtr->lines = PCC_header.y2 - PCC_header.y1 + 1;
	seqPtr->x2 = PCC_header.x2 - PCC_header.x1 + 1;
	// Size of the image
	uint16 size = seqPtr->lines * seqPtr->bytesPerLine8;

	// Allocate memory for image data if NULL
	if (imagePtr == 0) {
		if ((imagePtr = (byte *)malloc((size_t) size)) == 0)
			Utils::Error(HEAP_ERR, "%s", name);
	}

	seqPtr->imagePtr = imagePtr;

	// Process the image data, converting to 8-bit DIB format
	uint16 y = 0;                                   // Current line index
	byte  pline[XPIX];                              // Hold 4 planes of data
	byte  *p = pline;                               // Ptr to above
	while (y < seqPtr->lines) {
		byte c = f.readByte();
		if ((c & REP_MASK) == REP_MASK) {
			byte d = f.readByte();                  // Read data byte
			for (int i = 0; i < (c & LEN_MASK); i++) {
				*p++ = d;
				if ((uint16)(p - pline) == bytesPerLine4)
					p = convertPCC(pline, y++, PCC_header.bytesPerLine, imagePtr);
			}
		} else {
			*p++ = c;
			if ((uint16)(p - pline) == bytesPerLine4)
				p = convertPCC(pline, y++, PCC_header.bytesPerLine, imagePtr);
		}
	}
	return seqPtr;
}

void FileManager::readImage(int objNum, object_t *objPtr) {
// Read object file of PCC images into object supplied
	debugC(1, kDebugFile, "readImage(%d, object_t *objPtr)", objNum);

	if (!objPtr->seqNumb)                           // This object has no images
		return;

	if (_vm.isPacked()) {
		_objectsArchive.seek((uint32)objNum * sizeof(objBlock_t), SEEK_SET);

		objBlock_t objBlock;                        // Info on file within database
		objBlock.objOffset = _objectsArchive.readUint32LE();
		objBlock.objLength = _objectsArchive.readUint32LE();

		_objectsArchive.seek(objBlock.objOffset, SEEK_SET);
	} else {
		char *buf = (char *) malloc(2048 + 1);      // Buffer for file access
		strcat(strcat(strcpy(buf, _vm._picDir), _vm._arrayNouns[objPtr->nounIndex][0]), OBJEXT);
		if (!_objectsArchive.open(buf)) {
			warning("File %s not found, trying again with %s%s", buf, _vm._arrayNouns[objPtr->nounIndex][0], OBJEXT);
			strcat(strcpy(buf, _vm._arrayNouns[objPtr->nounIndex][0]), OBJEXT);
			if (!_objectsArchive.open(buf))
				Utils::Error(FILE_ERR, "%s", buf);
		}
	}

	bool  firstFl = true;                           // Initializes pcx read function
	seq_t *seqPtr = 0;                              // Ptr to sequence structure

	// Now read the images into an images list
	for (int j = 0; j < objPtr->seqNumb; j++) {     // for each sequence
		for (int k = 0; k < objPtr->seqList[j].imageNbr; k++) { // each image
			if (k == 0) {                           // First image
				// Read this image - allocate both seq and image memory
				seqPtr = readPCX(_objectsArchive, 0, 0, firstFl, _vm._arrayNouns[objPtr->nounIndex][0]);
				objPtr->seqList[j].seqPtr = seqPtr;
				firstFl = false;
			} else {                                // Subsequent image
				// Read this image - allocate both seq and image memory
				seqPtr->nextSeqPtr = readPCX(_objectsArchive, 0, 0, firstFl, _vm._arrayNouns[objPtr->nounIndex][0]);
				seqPtr = seqPtr->nextSeqPtr;
			}

			// Compute the bounding box - x1, x2, y1, y2
			// Note use of x2 - marks end of valid data in row
			uint16 x2 = seqPtr->x2;
			seqPtr->x1 = seqPtr->x2;
			seqPtr->x2 = 0;
			seqPtr->y1 = seqPtr->lines;
			seqPtr->y2 = 0;

			image_pt dibPtr = seqPtr->imagePtr;
			for (int y = 0; y < seqPtr->lines; y++, dibPtr += seqPtr->bytesPerLine8 - x2) {
				for (int x = 0; x < x2; x++) {
					if (*dibPtr++) {                // Some data found
						if (x < seqPtr->x1)
							seqPtr->x1 = x;
						if (x > seqPtr->x2)
							seqPtr->x2 = x;
						if (y < seqPtr->y1)
							seqPtr->y1 = y;
						if (y > seqPtr->y2)
							seqPtr->y2 = y;
					}
				}
			}
		}
		seqPtr->nextSeqPtr = objPtr->seqList[j].seqPtr; // loop linked list to head
	}

	// Set the current image sequence to first or last
	switch (objPtr->cycling) {
	case INVISIBLE:                                 // (May become visible later)
	case ALMOST_INVISIBLE:
	case NOT_CYCLING:
	case CYCLE_FORWARD:
		objPtr->currImagePtr = objPtr->seqList[0].seqPtr;
		break;
	case CYCLE_BACKWARD:
		objPtr->currImagePtr = seqPtr;
		break;
	default:
		warning("Unexpected cycling: %d", objPtr->cycling);
	}

	if (!_vm.isPacked())
		_objectsArchive.close();
}

sound_pt FileManager::getSound(int16 sound, uint16 *size) {
// Read sound (or music) file data.  Call with SILENCE to free-up
// any allocated memory.  Also returns size of data
	debugC(1, kDebugFile, "getSound(%d, %d)", sound, *size);

	// No more to do if SILENCE (called for cleanup purposes)
	if (sound == _vm._soundSilence)
		return 0;

	// Open sounds file
	Common::File fp;                                // Handle to SOUND_FILE
	if (!fp.open(SOUND_FILE)) {
		warning("Hugo Error: File not found %s", SOUND_FILE);
		return 0;
	}

	// If this is the first call, read the lookup table
	static bool has_read_header = false;
	static sound_hdr_t s_hdr[MAX_SOUNDS];           // Sound lookup table

	if (!has_read_header) {
		if (fp.read(s_hdr, sizeof(s_hdr)) != sizeof(s_hdr))
			Utils::Error(FILE_ERR, "%s", SOUND_FILE);
		has_read_header = true;
	}

	*size = s_hdr[sound].size;
	if (*size == 0)
		Utils::Error(SOUND_ERR, "%s", SOUND_FILE);

	// Allocate memory for sound or music, if possible
	sound_pt soundPtr = (byte *)malloc(s_hdr[sound].size); // Ptr to sound data
	if (soundPtr == 0) {
		Utils::Warn("%s", "Low on memory");
		return 0;
	}

	// Seek to data and read it
	fp.seek(s_hdr[sound].offset, SEEK_SET);
	if (fp.read(soundPtr, s_hdr[sound].size) != s_hdr[sound].size)
		Utils::Error(FILE_ERR, "%s", SOUND_FILE);

	fp.close();

	return soundPtr;
}

bool FileManager::fileExists(char *filename) {
// Return whether file exists or not
	Common::File f;
	if (f.open(filename)) {
		f.close();
		return true;
	}
	return false;
}

void FileManager::saveSeq(object_t *obj) {
// Save sequence number and image number in given object
	debugC(1, kDebugFile, "saveSeq");

	bool found = false;
	for (int j = 0; !found && (j < obj->seqNumb); j++) {
		seq_t *q = obj->seqList[j].seqPtr;
		for (int k = 0; !found && (k < obj->seqList[j].imageNbr); k++) {
			if (obj->currImagePtr == q) {
				found = true;
				obj->curSeqNum = j;
				obj->curImageNum = k;
			} else {
				q = q->nextSeqPtr;
			}
		}
	}
}

void FileManager::restoreSeq(object_t *obj) {
// Set up cur_seq_p from stored sequence and image number in object
	debugC(1, kDebugFile, "restoreSeq");

	seq_t *q = obj->seqList[obj->curSeqNum].seqPtr;
	for (int j = 0; j < obj->curImageNum; j++)
		q = q->nextSeqPtr;
	obj->currImagePtr = q;
}

void FileManager::saveGame(int16 slot, const char *descrip) {
// Save game to supplied slot (-1 is INITFILE)
	debugC(1, kDebugFile, "saveGame(%d, %s)", slot, descrip);

	// Get full path of saved game file - note test for INITFILE
	char    path[256];                                  // Full path of saved game
	if (slot == -1)
		sprintf(path, "%s", _vm._initFilename);
	else
		sprintf(path, _vm._saveFilename, slot);

	Common::WriteStream *out = 0;
	if (!(out = _vm.getSaveFileManager()->openForSaving(path))) {
		warning("Can't create file '%s', game not saved", path);
		return;
	}

	// Write version.  We can't restore from obsolete versions
	out->write(&kSavegameVersion, sizeof(kSavegameVersion));

	// Save description of saved game
	out->write(descrip, DESCRIPLEN);

	// Save objects
	for (int i = 0; i < _vm._numObj; i++) {
		// Save where curr_seq_p is pointing to
		saveSeq(&_vm._objects[i]);
		out->write(&_vm._objects[i], sizeof(object_t));
	}

	const status_t &gameStatus = _vm.getGameStatus();

	// Save whether hero image is swapped
	out->write(&_vm._heroImage, sizeof(_vm._heroImage));

	// Save score
	int score = _vm.getScore();
	out->write(&score, sizeof(score));

	// Save story mode
	out->write(&gameStatus.storyModeFl, sizeof(gameStatus.storyModeFl));

	// Save jumpexit mode
	out->write(&gameStatus.jumpExitFl, sizeof(gameStatus.jumpExitFl));

	// Save gameover status
	out->write(&gameStatus.gameOverFl, sizeof(gameStatus.gameOverFl));

	// Save screen states
	out->write(_vm._screenStates, sizeof(*_vm._screenStates) * _vm._numScreens);

	// Save points table
	out->write(_vm._points, sizeof(point_t) * _vm._numBonuses);

	// Now save current time and all current events in event queue
	_vm.scheduler().saveEvents(out);

	// Save palette table
	_vm.screen().savePal(out);

	// Save maze status
	out->write(&_maze, sizeof(maze_t));

	out->finalize();

	delete out;
}

void FileManager::restoreGame(int16 slot) {
// Restore game from supplied slot number (-1 is INITFILE)
	debugC(1, kDebugFile, "restoreGame(%d)", slot);

	// Initialize new-game status
	_vm.initStatus();

	// Get full path of saved game file - note test for INITFILE
	char path[256];                                    // Full path of saved game
	if (slot == -1)
		sprintf(path, "%s", _vm._initFilename);
	else
		sprintf(path, _vm._saveFilename, slot);

	Common::SeekableReadStream *in = 0;
	if (!(in = _vm.getSaveFileManager()->openForLoading(path)))
		return;

	// Check version, can't restore from different versions
	int saveVersion;
	in->read(&saveVersion, sizeof(saveVersion));
	if (saveVersion != kSavegameVersion) {
		Utils::Error(GEN_ERR, "%s", "Savegame of incompatible version");
		return;
	}

	// Skip over description
	in->seek(DESCRIPLEN, SEEK_CUR);

	// If hero image is currently swapped, swap it back before restore
	if (_vm._heroImage != HERO)
		_vm.scheduler().swapImages(HERO, _vm._heroImage);

	// Restore objects, retain current seqList which points to dynamic mem
	// Also, retain cmnd_t pointers
	for (int i = 0; i < _vm._numObj; i++) {
		object_t *p = &_vm._objects[i];
		seqList_t seqList[MAX_SEQUENCES];
		memcpy(seqList, p->seqList, sizeof(seqList_t));
		uint16 cmdIndex = p->cmdIndex;
		in->read(p, sizeof(object_t));
		p->cmdIndex = cmdIndex;
		memcpy(p->seqList, seqList, sizeof(seqList_t));
	}

	in->read(&_vm._heroImage, sizeof(_vm._heroImage));

	// If hero swapped in saved game, swap it
	int heroImg = _vm._heroImage;
	if (heroImg != HERO)
		_vm.scheduler().swapImages(HERO, _vm._heroImage);
	_vm._heroImage = heroImg;

	status_t &gameStatus = _vm.getGameStatus();

	int score;
	in->read(&score, sizeof(score));
	_vm.setScore(score);

	in->read(&gameStatus.storyModeFl, sizeof(gameStatus.storyModeFl));
	in->read(&gameStatus.jumpExitFl, sizeof(gameStatus.jumpExitFl));
	in->read(&gameStatus.gameOverFl, sizeof(gameStatus.gameOverFl));
	in->read(_vm._screenStates, sizeof(*_vm._screenStates) * _vm._numScreens);

	// Restore points table
	in->read(_vm._points, sizeof(point_t) * _vm._numBonuses);

	// Restore ptrs to currently loaded objects
	for (int i = 0; i < _vm._numObj; i++)
		restoreSeq(&_vm._objects[i]);

	// Now restore time of the save and the event queue
	_vm.scheduler().restoreEvents(in);

	// Restore palette and change it if necessary
	_vm.screen().restorePal(in);

	// Restore maze status
	in->read(&_maze, sizeof(maze_t));

	delete in;
}

void FileManager::initSavedGame() {
// Initialize the size of a saved game (from the fixed initial game).
// If status.initsave is TRUE, or the initial saved game is not found,
// force a save to create one.  Normally the game will be shipped with
// the initial game file but useful to force a write during development
// when the size is changeable.
// The net result is a valid INITFILE, with status.savesize initialized.
	debugC(1, kDebugFile, "initSavedGame");

	// Get full path of INITFILE
	char path[256];                                 // Full path of INITFILE
	sprintf(path, "%s", _vm._initFilename);

	// Force save of initial game
	if (_vm.getGameStatus().initSaveFl)
		saveGame(-1, "");

	// If initial game doesn't exist, create it
	Common::SeekableReadStream *in = 0;
	if (!(in = _vm.getSaveFileManager()->openForLoading(path))) {
		saveGame(-1, "");
		if (!(in = _vm.getSaveFileManager()->openForLoading(path))) {
			Utils::Error(WRITE_ERR, "%s", path);
			return;
		}
	}

	// Must have an open saved game now
	_vm.getGameStatus().saveSize = in->size();
	delete in;

	// Check sanity - maybe disk full or path set to read-only drive?
	if (_vm.getGameStatus().saveSize == -1)
		Utils::Error(WRITE_ERR, "%s", path);
}

void FileManager::openPlaybackFile(bool playbackFl, bool recordFl) {
	debugC(1, kDebugFile, "openPlaybackFile(%d, %d)", (playbackFl) ? 1 : 0, (recordFl) ? 1 : 0);

	if (playbackFl) {
		if (!(fpb = fopen(PBFILE, "r+b")))
			Utils::Error(FILE_ERR, "%s", PBFILE);
	} else if (recordFl) {
		fpb = fopen(PBFILE, "wb");
	}
	pbdata.time = 0;                                // Say no key available
}

void FileManager::closePlaybackFile() {
	fclose(fpb);
}

void FileManager::printBootText() {
// Read the encrypted text from the boot file and print it
	debugC(1, kDebugFile, "printBootText");

	Common::File ofp;
	if (!ofp.open(BOOTFILE)) {
		if (_vm._gameVariant == 3) {
			//TODO initialize properly _boot structure
			warning("printBootText - Skipping as H1 Dos may be a freeware");
			return;
		} else {
			Utils::Error(FILE_ERR, "%s", BOOTFILE);
		}
	}

	// Allocate space for the text and print it
	char *buf = (char *)malloc(_boot.exit_len + 1);
	if (buf) {
		// Skip over the boot structure (already read) and read exit text
		ofp.seek((long)sizeof(_boot), SEEK_SET);
		if (ofp.read(buf, _boot.exit_len) != (size_t)_boot.exit_len)
			Utils::Error(FILE_ERR, "%s", BOOTFILE);

		// Decrypt the exit text, using CRYPT substring
		int i;
		for (i = 0; i < _boot.exit_len; i++)
			buf[i] ^= CRYPT[i % strlen(CRYPT)];

		buf[i] = '\0';
		//Box(BOX_OK, "%s", buf_p);
		//MessageBox(hwnd, buf_p, "License", MB_ICONINFORMATION);
		warning("printBootText(): License: %s", buf);
	}

	free(buf);
	ofp.close();
}

void FileManager::readBootFile() {
// Reads boot file for program environment.  Fatal error if not there or
// file checksum is bad.  De-crypts structure while checking checksum
	debugC(1, kDebugFile, "readBootFile");

	Common::File ofp;
	if (!ofp.open(BOOTFILE)) {
		if (_vm._gameVariant == 3) {
			//TODO initialize properly _boot structure
			warning("readBootFile - Skipping as H1 Dos may be a freeware");
			return;
		} else {
			Utils::Error(FILE_ERR, "%s", BOOTFILE);
		}
	}

	if (ofp.size() < (int32)sizeof(_boot))
		Utils::Error(FILE_ERR, "%s", BOOTFILE);

	_boot.checksum = ofp.readByte();
	_boot.registered = ofp.readByte();
	ofp.read(_boot.pbswitch, sizeof(_boot.pbswitch));
	ofp.read(_boot.distrib, sizeof(_boot.distrib));
	_boot.exit_len = ofp.readUint16LE();

	byte *p = (byte *)&_boot;

	byte checksum = 0;
	for (uint32 i = 0; i < sizeof(_boot); i++) {
		checksum ^= p[i];
		p[i] ^= CRYPT[i % strlen(CRYPT)];
	}
	ofp.close();

	if (checksum)
		Utils::Error(GEN_ERR, "%s", "Program startup file invalid");
}

uif_hdr_t *FileManager::getUIFHeader(uif_t id) {
// Returns address of uif_hdr[id], reading it in if first call
	debugC(1, kDebugFile, "getUIFHeader(%d)", id);

	static bool firstFl = true;
	static uif_hdr_t UIFHeader[MAX_UIFS];           // Lookup for uif fonts/images

	// Initialize offset lookup if not read yet
	if (firstFl) {
		firstFl = false;
		// Open unbuffered to do far read
		Common::File ip;                            // Image data file
		if (!ip.open(UIF_FILE))
			Utils::Error(FILE_ERR, "%s", UIF_FILE);

		if (ip.size() < (int32)sizeof(UIFHeader))
			Utils::Error(FILE_ERR, "%s", UIF_FILE);

		for (int i = 0; i < MAX_UIFS; ++i) {
			UIFHeader[i].size = ip.readUint16LE();
			UIFHeader[i].offset = ip.readUint32LE();
		}

		ip.close();
	}
	return &UIFHeader[id];
}

void FileManager::readUIFItem(int16 id, byte *buf) {
// Read uif item into supplied buffer.
	debugC(1, kDebugFile, "readUIFItem(%d, ...)", id);

	// Open uif file to read data
	Common::File ip;                                // UIF_FILE handle
	if (!ip.open(UIF_FILE))
		Utils::Error(FILE_ERR, "%s", UIF_FILE);

	// Seek to data
	uif_hdr_t *UIFHeaderPtr = getUIFHeader((uif_t)id);
	ip.seek(UIFHeaderPtr->offset, SEEK_SET);

	// We support pcx images and straight data
	seq_t dummySeq;                                 // Dummy seq_t for image data
	switch (id) {
	case UIF_IMAGES:                                // Read uif images file
		readPCX(ip, &dummySeq, buf, true, UIF_FILE);
		break;
	default:                                        // Read file data into supplied array
		if (ip.read(buf, UIFHeaderPtr->size) != UIFHeaderPtr->size)
			Utils::Error(FILE_ERR, "%s", UIF_FILE);
		break;
	}

	ip.close();
}

void FileManager::instructions() {
// Simple instructions given when F1 pressed twice in a row
// Only in DOS versions

	Common::File f;
	if (!f.open(HELPFILE)) {
		warning("help.dat not found");
		return;
	}

	char readBuf[2];
	while (f.read(readBuf, 1)) {
		char line[1024], *wrkLine;
		wrkLine = line;
		wrkLine[0] = readBuf[0];
		wrkLine++;
		do {
			f.read(wrkLine, 1);
		} while (*wrkLine++ != EOP);
		wrkLine[-2] = '\0';                         // Remove EOP and previous CR
		Utils::Box(BOX_ANY, "%s", line);
		wrkLine = line;
		f.read(readBuf, 2);                         // Remove CRLF after EOP
	}
	f.close();
}

FileManager_v1d::FileManager_v1d(HugoEngine &vm) : FileManager(vm) {
}

FileManager_v1d::~FileManager_v1d() {
}

void FileManager_v1d::openDatabaseFiles() {
	debugC(1, kDebugFile, "openDatabaseFiles");
}

void FileManager_v1d::closeDatabaseFiles() {
	debugC(1, kDebugFile, "closeDatabaseFiles");
}

void FileManager_v1d::readOverlay(int screenNum, image_pt image, ovl_t overlayType) {
// Open and read in an overlay file, close file
	debugC(1, kDebugFile, "readOverlay(%d, ...)", screenNum);

	const char *ovl_ext[] = {".b", ".o", ".ob"};
	char *buf = (char *) malloc(2048 + 1);          // Buffer for file access

	strcat(strcpy(buf, _vm._screenNames[screenNum]), ovl_ext[overlayType]);

	if (!fileExists(buf)) {
		for (uint32 i = 0; i < OVL_SIZE; i++)
			image[i] = 0;
		return;
	}

	if (!_sceneryArchive1.open(buf))
		Utils::Error(FILE_ERR, "%s", buf);

	image_pt tmpImage = image;                      // temp ptr to overlay file

	_sceneryArchive1.read(tmpImage, OVL_SIZE);
	_sceneryArchive1.close();
}

void FileManager_v1d::readBackground(int screenIndex) {
// Read a PCX image into dib_a
	debugC(1, kDebugFile, "readBackground(%d)", screenIndex);

	char *buf = (char *) malloc(2048 + 1);          // Buffer for file access
	strcat(strcpy(buf, _vm._screenNames[screenIndex]), ".ART");
	if (!_sceneryArchive1.open(buf))
		Utils::Error(FILE_ERR, "%s", buf);
	// Read the image into dummy seq and static dib_a
	seq_t dummySeq;                                 // Image sequence structure for Read_pcx
	readPCX(_sceneryArchive1, &dummySeq, _vm.screen().getFrontBuffer(), true, _vm._screenNames[screenIndex]);

	_sceneryArchive1.close();
}

char *FileManager_v1d::fetchString(int index) {
	debugC(1, kDebugFile, "fetchString(%d)", index);

	return _vm._stringtData[index];
}

FileManager_v2d::FileManager_v2d(HugoEngine &vm) : FileManager(vm) {
}

FileManager_v2d::~FileManager_v2d() {
}

void FileManager_v2d::openDatabaseFiles() {
	debugC(1, kDebugFile, "openDatabaseFiles");

	if (!_stringArchive.open(STRING_FILE))
		Utils::Error(FILE_ERR, "%s", STRING_FILE);
	if (!_sceneryArchive1.open("scenery.dat"))
		Utils::Error(FILE_ERR, "%s", "scenery.dat");
	if (!_objectsArchive.open(OBJECTS_FILE))
		Utils::Error(FILE_ERR, "%s", OBJECTS_FILE);
}

void FileManager_v2d::closeDatabaseFiles() {
	debugC(1, kDebugFile, "closeDatabaseFiles");

	_stringArchive.close();
	_sceneryArchive1.close();
	_objectsArchive.close();
}

void FileManager_v2d::readBackground(int screenIndex) {
// Read a PCX image into dib_a
	debugC(1, kDebugFile, "readBackground(%d)", screenIndex);

	_sceneryArchive1.seek((uint32) screenIndex * sizeof(sceneBlock_t), SEEK_SET);

	sceneBlock_t sceneBlock;                        // Read a database header entry
	sceneBlock.scene_off = _sceneryArchive1.readUint32LE();
	sceneBlock.scene_len = _sceneryArchive1.readUint32LE();
	sceneBlock.b_off = _sceneryArchive1.readUint32LE();
	sceneBlock.b_len = _sceneryArchive1.readUint32LE();
	sceneBlock.o_off = _sceneryArchive1.readUint32LE();
	sceneBlock.o_len = _sceneryArchive1.readUint32LE();
	sceneBlock.ob_off = _sceneryArchive1.readUint32LE();
	sceneBlock.ob_len = _sceneryArchive1.readUint32LE();

	_sceneryArchive1.seek(sceneBlock.scene_off, SEEK_SET);

	// Read the image into dummy seq and static dib_a
	seq_t dummySeq;                                 // Image sequence structure for Read_pcx
	readPCX(_sceneryArchive1, &dummySeq, _vm.screen().getFrontBuffer(), true, _vm._screenNames[screenIndex]);
}

void FileManager_v2d::readOverlay(int screenNum, image_pt image, ovl_t overlayType) {
// Open and read in an overlay file, close file
	debugC(1, kDebugFile, "readOverlay(%d, ...)", screenNum);

    image_pt tmpImage = image;                  // temp ptr to overlay file
	_sceneryArchive1.seek((uint32)screenNum * sizeof(sceneBlock_t), SEEK_SET);

	sceneBlock_t sceneBlock;                        // Database header entry
	sceneBlock.scene_off = _sceneryArchive1.readUint32LE();
	sceneBlock.scene_len = _sceneryArchive1.readUint32LE();
	sceneBlock.b_off = _sceneryArchive1.readUint32LE();
	sceneBlock.b_len = _sceneryArchive1.readUint32LE();
	sceneBlock.o_off = _sceneryArchive1.readUint32LE();
	sceneBlock.o_len = _sceneryArchive1.readUint32LE();
	sceneBlock.ob_off = _sceneryArchive1.readUint32LE();
	sceneBlock.ob_len = _sceneryArchive1.readUint32LE();

	uint32 i = 0;
	switch (overlayType) {
	case BOUNDARY:
		_sceneryArchive1.seek(sceneBlock.b_off, SEEK_SET);
		i = sceneBlock.b_len;
		break;
	case OVERLAY:
		_sceneryArchive1.seek(sceneBlock.o_off, SEEK_SET);
		i = sceneBlock.o_len;
		break;
	case OVLBASE:
		_sceneryArchive1.seek(sceneBlock.ob_off, SEEK_SET);
		i = sceneBlock.ob_len;
		break;
	default:
		Utils::Error(FILE_ERR, "%s", "Bad ovl_type");
		break;
	}
	if (i == 0) {
		for (i = 0; i < OVL_SIZE; i++)
			image[i] = 0;
		return;
	}

	// Read in the overlay file using MAC Packbits.  (We're not proud!)
	int16 k = 0;                                    // byte count
	do {
		int8 data = _sceneryArchive1.readByte();    // Read a code byte
		if ((byte)data == 0x80)                     // Noop
			k = k;
		else if (data >= 0) {                       // Copy next data+1 literally
			for (i = 0; i <= (byte)data; i++, k++)
				*tmpImage++ = _sceneryArchive1.readByte();
		} else {                                    // Repeat next byte -data+1 times
			int16 j = _sceneryArchive1.readByte();

			for (i = 0; i < (byte)(-data + 1); i++, k++)
				*tmpImage++ = j;
		}
	} while (k < OVL_SIZE);
}

char *FileManager_v2d::fetchString(int index) {
// Fetch string from file, decode and return ptr to string in memory
	debugC(1, kDebugFile, "fetchString(%d)", index);

	// Get offset to string[index] (and next for length calculation)
	_stringArchive.seek((uint32)index * sizeof(uint32), SEEK_SET);
	uint32 off1, off2;
	if (_stringArchive.read((char *)&off1, sizeof(uint32)) == 0)
		Utils::Error(FILE_ERR, "%s", "String offset");
	if (_stringArchive.read((char *)&off2, sizeof(uint32)) == 0)
		Utils::Error(FILE_ERR, "%s", "String offset");

	// Check size of string
	if ((off2 - off1) >= MAX_BOX)
		Utils::Error(FILE_ERR, "%s", "Fetched string too long!");

	// Position to string and read it into gen purpose _textBoxBuffer
	_stringArchive.seek(off1, SEEK_SET);
	if (_stringArchive.read(_textBoxBuffer, (uint16)(off2 - off1)) == 0)
		Utils::Error(FILE_ERR, "%s", "Fetch_string");

	// Null terminate, decode and return it
	_textBoxBuffer[off2-off1] = '\0';
	_vm.scheduler().decodeString(_textBoxBuffer);
	return _textBoxBuffer;
}


FileManager_v1w::FileManager_v1w(HugoEngine &vm) : FileManager_v2d(vm) {
}

FileManager_v1w::~FileManager_v1w() {
}

void FileManager_v1w::readOverlay(int screenNum, image_pt image, ovl_t overlayType) {
// Open and read in an overlay file, close file
	debugC(1, kDebugFile, "readOverlay(%d, ...)", screenNum);

	image_pt tmpImage = image;                      // temp ptr to overlay file
	_sceneryArchive1.seek((uint32)screenNum * sizeof(sceneBlock_t), SEEK_SET);

	sceneBlock_t sceneBlock;                        // Database header entry
	sceneBlock.scene_off = _sceneryArchive1.readUint32LE();
	sceneBlock.scene_len = _sceneryArchive1.readUint32LE();
	sceneBlock.b_off = _sceneryArchive1.readUint32LE();
	sceneBlock.b_len = _sceneryArchive1.readUint32LE();
	sceneBlock.o_off = _sceneryArchive1.readUint32LE();
	sceneBlock.o_len = _sceneryArchive1.readUint32LE();
	sceneBlock.ob_off = _sceneryArchive1.readUint32LE();
	sceneBlock.ob_len = _sceneryArchive1.readUint32LE();

	uint32 i = 0;
	switch (overlayType) {
	case BOUNDARY:
		_sceneryArchive1.seek(sceneBlock.b_off, SEEK_SET);
		i = sceneBlock.b_len;
		break;
	case OVERLAY:
		_sceneryArchive1.seek(sceneBlock.o_off, SEEK_SET);
		i = sceneBlock.o_len;
		break;
	case OVLBASE:
		_sceneryArchive1.seek(sceneBlock.ob_off, SEEK_SET);
		i = sceneBlock.ob_len;
		break;
	default:
		Utils::Error(FILE_ERR, "%s", "Bad ovl_type");
		break;
	}
	if (i == 0) {
		for (i = 0; i < OVL_SIZE; i++)
			image[i] = 0;
		return;
	}
	_sceneryArchive1.read(tmpImage, OVL_SIZE);
}

FileManager_v3d::FileManager_v3d(HugoEngine &vm) : FileManager_v2d(vm) {
}

FileManager_v3d::~FileManager_v3d() {
}

void FileManager_v3d::readBackground(int screenIndex) {
// Read a PCX image into dib_a
	debugC(1, kDebugFile, "readBackground(%d)", screenIndex);

	_sceneryArchive1.seek((uint32) screenIndex * sizeof(sceneBlock_t), SEEK_SET);
	
	sceneBlock_t sceneBlock;                        // Read a database header entry
	sceneBlock.scene_off = _sceneryArchive1.readUint32LE();
	sceneBlock.scene_len = _sceneryArchive1.readUint32LE();
	sceneBlock.b_off = _sceneryArchive1.readUint32LE();
	sceneBlock.b_len = _sceneryArchive1.readUint32LE();
	sceneBlock.o_off = _sceneryArchive1.readUint32LE();
	sceneBlock.o_len = _sceneryArchive1.readUint32LE();
	sceneBlock.ob_off = _sceneryArchive1.readUint32LE();
	sceneBlock.ob_len = _sceneryArchive1.readUint32LE();

	seq_t dummySeq;                                 // Image sequence structure for Read_pcx
	if (screenIndex < 20) {
		_sceneryArchive1.seek(sceneBlock.scene_off, SEEK_SET);
		// Read the image into dummy seq and static dib_a
		readPCX(_sceneryArchive1, &dummySeq, _vm.screen().getFrontBuffer(), true, _vm._screenNames[screenIndex]);
	} else {
		_sceneryArchive2.seek(sceneBlock.scene_off, SEEK_SET);
		// Read the image into dummy seq and static dib_a
		readPCX(_sceneryArchive2, &dummySeq, _vm.screen().getFrontBuffer(), true, _vm._screenNames[screenIndex]);
	}
}

void FileManager_v3d::openDatabaseFiles() {
	debugC(1, kDebugFile, "openDatabaseFiles");

	if (!_stringArchive.open(STRING_FILE))
		Utils::Error(FILE_ERR, "%s", STRING_FILE);
	if (!_sceneryArchive1.open("scenery1.dat"))
		Utils::Error(FILE_ERR, "%s", "scenery1.dat");
	if (!_sceneryArchive2.open("scenery2.dat"))
		Utils::Error(FILE_ERR, "%s", "scenery2.dat");
	if (!_objectsArchive.open(OBJECTS_FILE))
		Utils::Error(FILE_ERR, "%s", OBJECTS_FILE);
}

void FileManager_v3d::closeDatabaseFiles() {
	debugC(1, kDebugFile, "closeDatabaseFiles");

	_stringArchive.close();
	_sceneryArchive1.close();
	_sceneryArchive2.close();
	_objectsArchive.close();
}

void FileManager_v3d::readOverlay(int screenNum, image_pt image, ovl_t overlayType) {
// Open and read in an overlay file, close file
	debugC(1, kDebugFile, "readOverlay(%d, ...)", screenNum);

	image_pt     tmpImage = image;                  // temp ptr to overlay file
	_sceneryArchive1.seek((uint32)screenNum * sizeof(sceneBlock_t), SEEK_SET);
	
	sceneBlock_t sceneBlock;                        // Database header entry
	sceneBlock.scene_off = _sceneryArchive1.readUint32LE();
	sceneBlock.scene_len = _sceneryArchive1.readUint32LE();
	sceneBlock.b_off = _sceneryArchive1.readUint32LE();
	sceneBlock.b_len = _sceneryArchive1.readUint32LE();
	sceneBlock.o_off = _sceneryArchive1.readUint32LE();
	sceneBlock.o_len = _sceneryArchive1.readUint32LE();
	sceneBlock.ob_off = _sceneryArchive1.readUint32LE();
	sceneBlock.ob_len = _sceneryArchive1.readUint32LE();

	uint32 i = 0;
	
	if (screenNum < 20) {
		switch (overlayType) {
		case BOUNDARY:
			_sceneryArchive1.seek(sceneBlock.b_off, SEEK_SET);
			i = sceneBlock.b_len;
			break;
		case OVERLAY:
			_sceneryArchive1.seek(sceneBlock.o_off, SEEK_SET);
			i = sceneBlock.o_len;
			break;
		case OVLBASE:
			_sceneryArchive1.seek(sceneBlock.ob_off, SEEK_SET);
			i = sceneBlock.ob_len;
			break;
		default:
			Utils::Error(FILE_ERR, "%s", "Bad ovl_type");
			break;
		}
		if (i == 0) {
			for (i = 0; i < OVL_SIZE; i++)
				image[i] = 0;
			return;
		}
	
		// Read in the overlay file using MAC Packbits.  (We're not proud!)
		int16 k = 0;                                // byte count
		do {
			int8 data = _sceneryArchive1.readByte();// Read a code byte
			if ((byte)data == 0x80)                 // Noop
				k = k;
			else if (data >= 0) {                   // Copy next data+1 literally
				for (i = 0; i <= (byte)data; i++, k++)
					*tmpImage++ = _sceneryArchive1.readByte();
			} else {                            // Repeat next byte -data+1 times
				int16 j = _sceneryArchive1.readByte();
	
				for (i = 0; i < (byte)(-data + 1); i++, k++)
					*tmpImage++ = j;
			}
		} while (k < OVL_SIZE);
	} else {
		switch (overlayType) {
		case BOUNDARY:
			_sceneryArchive2.seek(sceneBlock.b_off, SEEK_SET);
			i = sceneBlock.b_len;
			break;
		case OVERLAY:
			_sceneryArchive2.seek(sceneBlock.o_off, SEEK_SET);
			i = sceneBlock.o_len;
			break;
		case OVLBASE:
			_sceneryArchive2.seek(sceneBlock.ob_off, SEEK_SET);
			i = sceneBlock.ob_len;
			break;
		default:
			Utils::Error(FILE_ERR, "%s", "Bad ovl_type");
			break;
		}
		if (i == 0) {
			for (i = 0; i < OVL_SIZE; i++)
				image[i] = 0;
			return;
		}
	
		// Read in the overlay file using MAC Packbits.  (We're not proud!)
		int16 k = 0;                                // byte count
		do {
			int8 data = _sceneryArchive2.readByte();// Read a code byte
			if ((byte)data == 0x80)                 // Noop
				k = k;
			else if (data >= 0) {                   // Copy next data+1 literally
				for (i = 0; i <= (byte)data; i++, k++)
					*tmpImage++ = _sceneryArchive2.readByte();
			} else {                                // Repeat next byte -data+1 times
				int16 j = _sceneryArchive2.readByte();
	
				for (i = 0; i < (byte)(-data + 1); i++, k++)
					*tmpImage++ = j;
			}
		} while (k < OVL_SIZE);
	}
}
} // End of namespace Hugo

