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
#include "common/savefile.h"

#include "hugo/hugo.h"
#include "hugo/file.h"
#include "hugo/global.h"
#include "hugo/schedule.h"
#include "hugo/display.h"
#include "hugo/util.h"
#include "hugo/object.h"

namespace Hugo {
FileManager::FileManager(HugoEngine *vm) : _vm(vm) {
}

FileManager::~FileManager() {
}

/**
* Convert 4 planes (RGBI) data to 8-bit DIB format
* Return original plane data ptr
*/
byte *FileManager::convertPCC(byte *p, uint16 y, uint16 bpl, image_pt dataPtr) {
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

/**
* Read a pcx file of length len.  Use supplied seq_p and image_p or
* allocate space if NULL.  Name used for errors.  Returns address of seq_p
* Set first TRUE to initialize b_index (i.e. not reading a sequential image in file).
*/
seq_t *FileManager::readPCX(Common::File &f, seq_t *seqPtr, byte *imagePtr, bool firstFl, const char *name) {
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

/**
* Read object file of PCC images into object supplied
*/
void FileManager::readImage(int objNum, object_t *objPtr) {
	debugC(1, kDebugFile, "readImage(%d, object_t *objPtr)", objNum);

	if (!objPtr->seqNumb)                           // This object has no images
		return;

	if (_vm->isPacked()) {
		_objectsArchive.seek((uint32)objNum * sizeof(objBlock_t), SEEK_SET);

		objBlock_t objBlock;                        // Info on file within database
		objBlock.objOffset = _objectsArchive.readUint32LE();
		objBlock.objLength = _objectsArchive.readUint32LE();

		_objectsArchive.seek(objBlock.objOffset, SEEK_SET);
	} else {
		char *buf = (char *) malloc(2048 + 1);      // Buffer for file access
		strcat(strcat(strcpy(buf, _vm->_picDir), _vm->_arrayNouns[objPtr->nounIndex][0]), OBJEXT);
		if (!_objectsArchive.open(buf)) {
			warning("File %s not found, trying again with %s%s", buf, _vm->_arrayNouns[objPtr->nounIndex][0], OBJEXT);
			strcat(strcpy(buf, _vm->_arrayNouns[objPtr->nounIndex][0]), OBJEXT);
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
				seqPtr = readPCX(_objectsArchive, 0, 0, firstFl, _vm->_arrayNouns[objPtr->nounIndex][0]);
				objPtr->seqList[j].seqPtr = seqPtr;
				firstFl = false;
			} else {                                // Subsequent image
				// Read this image - allocate both seq and image memory
				seqPtr->nextSeqPtr = readPCX(_objectsArchive, 0, 0, firstFl, _vm->_arrayNouns[objPtr->nounIndex][0]);
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
		assert(seqPtr);
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

	if (!_vm->isPacked())
		_objectsArchive.close();
}

/**
* Read sound (or music) file data.  Call with SILENCE to free-up
* any allocated memory.  Also returns size of data
*/
sound_pt FileManager::getSound(int16 sound, uint16 *size) {
	debugC(1, kDebugFile, "getSound(%d, %d)", sound, *size);

	// No more to do if SILENCE (called for cleanup purposes)
	if (sound == _vm->_soundSilence)
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

/**
* Return whether file exists or not
*/
bool FileManager::fileExists(char *filename) {
	Common::File f;
	if (f.open(filename)) {
		f.close();
		return true;
	}
	return false;
}

/**
* Save game to supplied slot (-1 is INITFILE)
*/
void FileManager::saveGame(int16 slot, const char *descrip) {
	debugC(1, kDebugFile, "saveGame(%d, %s)", slot, descrip);

	// Get full path of saved game file - note test for INITFILE
	Common::String path; // Full path of saved game

	if (slot == -1)
		path = _vm->_initFilename;
	else
		path = Common::String::format(_vm->_saveFilename.c_str(), slot);

	Common::WriteStream *out = _vm->getSaveFileManager()->openForSaving(path);
	if (!out) {
		warning("Can't create file '%s', game not saved", path.c_str());
		return;
	}

	// Write version.  We can't restore from obsolete versions
	out->write(&kSavegameVersion, sizeof(kSavegameVersion));

	// Save description of saved game
	out->write(descrip, DESCRIPLEN);

	// Save objects
	for (int i = 0; i < _vm->_numObj; i++) {
		// Save where curr_seq_p is pointing to
		_vm->_object->saveSeq(&_vm->_object->_objects[i]);
		out->write(&_vm->_object->_objects[i], sizeof(object_t));
	}

	const status_t &gameStatus = _vm->getGameStatus();

	// Save whether hero image is swapped
	out->write(&_vm->_heroImage, sizeof(_vm->_heroImage));

	// Save score
	int score = _vm->getScore();
	out->write(&score, sizeof(score));

	// Save story mode
	out->write(&gameStatus.storyModeFl, sizeof(gameStatus.storyModeFl));

	// Save jumpexit mode
	out->write(&gameStatus.jumpExitFl, sizeof(gameStatus.jumpExitFl));

	// Save gameover status
	out->write(&gameStatus.gameOverFl, sizeof(gameStatus.gameOverFl));

	// Save screen states
	out->write(_vm->_screenStates, sizeof(*_vm->_screenStates) * _vm->_numScreens);

	// Save points table
	out->write(_vm->_points, sizeof(point_t) * _vm->_numBonuses);

	// Now save current time and all current events in event queue
	_vm->_scheduler->saveEvents(out);

	// Save palette table
	_vm->_screen->savePal(out);

	// Save maze status
	out->write(&_maze, sizeof(maze_t));

	out->finalize();

	delete out;
}

/**
* Restore game from supplied slot number (-1 is INITFILE)
*/
void FileManager::restoreGame(int16 slot) {
	debugC(1, kDebugFile, "restoreGame(%d)", slot);

	// Initialize new-game status
	_vm->initStatus();

	// Get full path of saved game file - note test for INITFILE
	Common::String path; // Full path of saved game

	if (slot == -1)
		path = _vm->_initFilename;
	else
		path = Common::String::format(_vm->_saveFilename.c_str(), slot);

	Common::SeekableReadStream *in = _vm->getSaveFileManager()->openForLoading(path);
	if (!in)
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
	if (_vm->_heroImage != HERO)
		_vm->_object->swapImages(HERO, _vm->_heroImage);

	// Restore objects, retain current seqList which points to dynamic mem
	// Also, retain cmnd_t pointers
	for (int i = 0; i < _vm->_numObj; i++) {
		object_t *p = &_vm->_object->_objects[i];
		seqList_t seqList[MAX_SEQUENCES];
		memcpy(seqList, p->seqList, sizeof(seqList_t));
		uint16 cmdIndex = p->cmdIndex;
		in->read(p, sizeof(object_t));
		p->cmdIndex = cmdIndex;
		memcpy(p->seqList, seqList, sizeof(seqList_t));
	}

	in->read(&_vm->_heroImage, sizeof(_vm->_heroImage));

	// If hero swapped in saved game, swap it
	int heroImg = _vm->_heroImage;
	if (heroImg != HERO)
		_vm->_object->swapImages(HERO, _vm->_heroImage);
	_vm->_heroImage = heroImg;

	status_t &gameStatus = _vm->getGameStatus();

	int score;
	in->read(&score, sizeof(score));
	_vm->setScore(score);

	in->read(&gameStatus.storyModeFl, sizeof(gameStatus.storyModeFl));
	in->read(&gameStatus.jumpExitFl, sizeof(gameStatus.jumpExitFl));
	in->read(&gameStatus.gameOverFl, sizeof(gameStatus.gameOverFl));
	in->read(_vm->_screenStates, sizeof(*_vm->_screenStates) * _vm->_numScreens);

	// Restore points table
	in->read(_vm->_points, sizeof(point_t) * _vm->_numBonuses);

	// Restore ptrs to currently loaded objects
	for (int i = 0; i < _vm->_numObj; i++)
		_vm->_object->restoreSeq(&_vm->_object->_objects[i]);

	// Now restore time of the save and the event queue
	_vm->_scheduler->restoreEvents(in);

	// Restore palette and change it if necessary
	_vm->_screen->restorePal(in);

	// Restore maze status
	in->read(&_maze, sizeof(maze_t));

	delete in;
}

/**
* Initialize the size of a saved game (from the fixed initial game).
* If status.initsave is TRUE, or the initial saved game is not found,
* force a save to create one.  Normally the game will be shipped with
* the initial game file but useful to force a write during development
* when the size is changeable.
* The net result is a valid INITFILE, with status.savesize initialized.
*/
void FileManager::initSavedGame() {
	debugC(1, kDebugFile, "initSavedGame");

	// Force save of initial game
	if (_vm->getGameStatus().initSaveFl)
		saveGame(-1, "");

	// If initial game doesn't exist, create it
	Common::SeekableReadStream *in = _vm->getSaveFileManager()->openForLoading(_vm->_initFilename);
	if (!in) {
		saveGame(-1, "");
		in = _vm->getSaveFileManager()->openForLoading(_vm->_initFilename);
		if (!in) {
			Utils::Error(WRITE_ERR, "%s", _vm->_initFilename.c_str());
			return;
		}
	}

	// Must have an open saved game now
	_vm->getGameStatus().saveSize = in->size();
	delete in;

	// Check sanity - maybe disk full or path set to read-only drive?
	if (_vm->getGameStatus().saveSize == -1)
		Utils::Error(WRITE_ERR, "%s", _vm->_initFilename.c_str());
}

/**
* Read the encrypted text from the boot file and print it
*/
void FileManager::printBootText() {
	debugC(1, kDebugFile, "printBootText");

	Common::File ofp;
	if (!ofp.open(BOOTFILE)) {
		if (_vm->_gameVariant == 3) {
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

/**
* Reads boot file for program environment.  Fatal error if not there or
* file checksum is bad.  De-crypts structure while checking checksum
*/
void FileManager::readBootFile() {
	debugC(1, kDebugFile, "readBootFile");

	Common::File ofp;
	if (!ofp.open(BOOTFILE)) {
		if (_vm->_gameVariant == 3) {
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

/**
* Returns address of uif_hdr[id], reading it in if first call
*/
uif_hdr_t *FileManager::getUIFHeader(uif_t id) {
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

/**
* Read uif item into supplied buffer.
*/
void FileManager::readUIFItem(int16 id, byte *buf) {
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

/**
* Simple instructions given when F1 pressed twice in a row
* Only in DOS versions
*/
void FileManager::instructions() {
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

/**
* Read the uif image file (inventory icons)
*/
void FileManager::readUIFImages() {
	debugC(1, kDebugFile, "readUIFImages");

	readUIFItem(UIF_IMAGES, _vm->_screen->getGUIBuffer());   // Read all uif images
}

} // End of namespace Hugo

