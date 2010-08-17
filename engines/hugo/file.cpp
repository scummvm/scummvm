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

byte *FileManager::convertPCC(byte *p, uint16 y, uint16 bpl, image_pt dataPtr) {
// Convert 4 planes (RGBI) data to 8-bit DIB format
// Return original plane data ptr
	uint16 r, g, b, i;                              // Byte index within each plane
	int8   bit;                                     // Bit index within a byte

	debugC(2, kDebugFile, "convertPCC(byte *p, %d, %d, image_pt data_p)", y, bpl);

	dataPtr += y * bpl * 8;                         // Point to correct DIB line
	for (r = 0, g = bpl, b = g + bpl, i = b + bpl; r < bpl; r++, g++, b++, i++) // Each byte in all planes
		for (bit = 7; bit >= 0; bit--)                                          // Each bit in byte
			*dataPtr++ = (((p[r] >> bit & 1) << 0) |
			              ((p[g] >> bit & 1) << 1) |
			              ((p[b] >> bit & 1) << 2) |
			              ((p[i] >> bit & 1) << 3));
	return p;
}

seq_t *FileManager::readPCX(Common::File &f, seq_t *seqPtr, byte *imagePtr, bool firstFl, const char *name) {
// Read a pcx file of length len.  Use supplied seq_p and image_p or
// allocate space if NULL.  Name used for errors.  Returns address of seq_p
// Set first TRUE to initialize b_index (i.e. not reading a sequential image in file).

	struct {                                            // Structure of PCX file header
		byte   mfctr, vers, enc, bpx;
		uint16  x1, y1, x2, y2;                         // bounding box
		uint16  xres, yres;
		byte   palette[48];                             // EGA color palette
		byte   vmode, planes;
		uint16 bytesPerLine;                            // Bytes per line
		byte   fill2[60];
	} PCC_header;                                       // Header of a PCC file

	byte  c, d;                                     // code and data bytes from PCX file
	byte  pline[XPIX];                              // Hold 4 planes of data
	byte  *p = pline;                               // Ptr to above
	byte  i;                                        // PCX repeat count
	uint16 bytesPerLine4;                           // BPL in 4-bit format
	uint16 size;                                    // Size of image
	uint16 y = 0;                                   // Current line index

	debugC(1, kDebugFile, "readPCX(..., %s)", name);

	// Read in the PCC header and check consistency
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
		Utils::Error(PCCH_ERR, name);

	// Allocate memory for seq_t if NULL
	if (seqPtr == NULL)
		if ((seqPtr = (seq_t *)malloc(sizeof(seq_t))) == NULL)
			Utils::Error(HEAP_ERR, name);

	// Find size of image data in 8-bit DIB format
	// Note save of x2 - marks end of valid data before garbage
	bytesPerLine4 = PCC_header.bytesPerLine * 4;    // 4-bit bpl
	seqPtr->bytesPerLine8 = bytesPerLine4 * 2;      // 8-bit bpl
	seqPtr->lines = PCC_header.y2 - PCC_header.y1 + 1;
	seqPtr->x2 = PCC_header.x2 - PCC_header.x1 + 1;
	size = seqPtr->lines * seqPtr->bytesPerLine8;

	// Allocate memory for image data if NULL
	if (imagePtr == NULL)
		if ((imagePtr = (byte *)malloc((size_t) size)) == NULL)
			Utils::Error(HEAP_ERR, name);
	seqPtr->imagePtr = imagePtr;

	// Process the image data, converting to 8-bit DIB format
	while (y < seqPtr->lines) {
		c = f.readByte();
		if ((c & REP_MASK) == REP_MASK) {
			d = f.readByte();                       // Read data byte
			for (i = 0; i < (c & LEN_MASK); i++) {
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
	byte       x, y, j, k;
	uint16     x2;                                  // Limit on x in image data
	seq_t     *seqPtr;                              // Ptr to sequence structure
	image_pt   dibPtr;                              // Ptr to DIB data
	objBlock_t objBlock;                            // Info on file within database
	bool       firstFl = true;                      // Initializes pcx read function

	debugC(1, kDebugFile, "readImage(%d, object_t *objPtr)", objNum);

	if (!objPtr->seqNumb)                           // This object has no images
		return;

	if (_vm.isPacked()) {
		_objectsArchive.seek((uint32)objNum * sizeof(objBlock_t), SEEK_SET);

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
				Utils::Error(FILE_ERR, buf);
		}
	}

	// Now read the images into an images list
	for (j = 0; j < objPtr->seqNumb; j++) {         // for each sequence
		for (k = 0; k < objPtr->seqList[j].imageNbr; k++) { // each image
			if (k == 0) {                           // First image
				// Read this image - allocate both seq and image memory
				seqPtr = readPCX(_objectsArchive, NULL, NULL, firstFl, _vm._arrayNouns[objPtr->nounIndex][0]);
				objPtr->seqList[j].seqPtr = seqPtr;
				firstFl = false;
			} else {                                // Subsequent image
				// Read this image - allocate both seq and image memory
				seqPtr->nextSeqPtr = readPCX(_objectsArchive, NULL, NULL, firstFl, _vm._arrayNouns[objPtr->nounIndex][0]);
				seqPtr = seqPtr->nextSeqPtr;
			}

			// Compute the bounding box - x1, x2, y1, y2
			// Note use of x2 - marks end of valid data in row
			x2 = seqPtr->x2;
			seqPtr->x1 = seqPtr->x2;
			seqPtr->x2 = 0;
			seqPtr->y1 = seqPtr->lines;
			seqPtr->y2 = 0;
			dibPtr = seqPtr->imagePtr;
			for (y = 0; y < seqPtr->lines; y++, dibPtr += seqPtr->bytesPerLine8 - x2)
				for (x = 0; x < x2; x++)
					if (*dibPtr++) {                    // Some data found
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
	}

	if (!_vm.isPacked())
		_objectsArchive.close();
}

void FileManager::readBackground(int screenIndex) {
// Read a PCX image into dib_a
	seq_t        seq;                               // Image sequence structure for Read_pcx
	sceneBlock_t sceneBlock;                        // Read a database header entry

	debugC(1, kDebugFile, "readBackground(%d)", screenIndex);

	if (_vm.isPacked()) {
		_sceneryArchive.seek((uint32) screenIndex * sizeof(sceneBlock_t), SEEK_SET);

		sceneBlock.scene_off = _sceneryArchive.readUint32LE();
		sceneBlock.scene_len = _sceneryArchive.readUint32LE();
		sceneBlock.b_off = _sceneryArchive.readUint32LE();
		sceneBlock.b_len = _sceneryArchive.readUint32LE();
		sceneBlock.o_off = _sceneryArchive.readUint32LE();
		sceneBlock.o_len = _sceneryArchive.readUint32LE();
		sceneBlock.ob_off = _sceneryArchive.readUint32LE();
		sceneBlock.ob_len = _sceneryArchive.readUint32LE();

		_sceneryArchive.seek(sceneBlock.scene_off, SEEK_SET);
	} else {
		char *buf = (char *) malloc(2048 + 1);      // Buffer for file access
		strcat(strcat(strcpy(buf, _vm._picDir), _vm._screenNames[screenIndex]), BKGEXT);
		if (!_sceneryArchive.open(buf)) {
			warning("File %s not found, trying again with %s.ART", buf, _vm._screenNames[screenIndex]);
			strcat(strcpy(buf, _vm._screenNames[screenIndex]), ".ART");
			if (!_sceneryArchive.open(buf))
				Utils::Error(FILE_ERR, buf);
		}
	}

	// Read the image into dummy seq and static dib_a
	readPCX(_sceneryArchive, &seq, _vm.screen().getFrontBuffer(), true, _vm._screenNames[screenIndex]);

	if (!_vm.isPacked())
		_sceneryArchive.close();
}

sound_pt FileManager::getSound(int16 sound, uint16 *size) {
// Read sound (or music) file data.  Call with SILENCE to free-up
// any allocated memory.  Also returns size of data

	static sound_hdr_t s_hdr[MAX_SOUNDS];           // Sound lookup table
	sound_pt           soundPtr;                    // Ptr to sound data
	Common::File       fp;                          // Handle to SOUND_FILE
//	bool               music = sound < NUM_TUNES;    // TRUE if music, else sound file

	debugC(1, kDebugFile, "getSound(%d, %d)", sound, *size);

	// No more to do if SILENCE (called for cleanup purposes)
	if (sound == _vm._soundSilence)
		return(NULL);

	// Open sounds file
	if (!fp.open(SOUND_FILE)) {
//		Error(FILE_ERR, SOUND_FILE);
		warning("Hugo Error: File not found %s", SOUND_FILE);
		return(NULL);
	}

	// If this is the first call, read the lookup table
	static bool has_read_header = false;
	if (!has_read_header) {
		if (fp.read(s_hdr, sizeof(s_hdr)) != sizeof(s_hdr))
			Utils::Error(FILE_ERR, SOUND_FILE);
		has_read_header = true;
	}

	*size = s_hdr[sound].size;
	if (*size == 0)
		Utils::Error(SOUND_ERR, SOUND_FILE);

	// Allocate memory for sound or music, if possible
	if ((soundPtr = (byte *)malloc(s_hdr[sound].size)) == 0) {
		Utils::Warn(false, "Low on memory");
		return(NULL);
	}

	// Seek to data and read it
	fp.seek(s_hdr[sound].offset, SEEK_SET);
	if (fp.read(soundPtr, s_hdr[sound].size) != s_hdr[sound].size)
		Utils::Error(FILE_ERR, SOUND_FILE);

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

void FileManager::readOverlay(int screenNum, image_pt image, ovl_t overlayType) {
// Open and read in an overlay file, close file
	uint32       i;
	int16        j, k;
	int8         data;                              // Must be 8 bits signed
	image_pt     tmpImage = image;                  // temp ptr to overlay file
	sceneBlock_t sceneBlock;                        // Database header entry

	debugC(1, kDebugFile, "readOverlay(%d, ...)", screenNum);

	if (_vm.isPacked()) {
		_sceneryArchive.seek((uint32)screenNum * sizeof(sceneBlock_t), SEEK_SET);

		sceneBlock.scene_off = _sceneryArchive.readUint32LE();
		sceneBlock.scene_len = _sceneryArchive.readUint32LE();
		sceneBlock.b_off = _sceneryArchive.readUint32LE();
		sceneBlock.b_len = _sceneryArchive.readUint32LE();
		sceneBlock.o_off = _sceneryArchive.readUint32LE();
		sceneBlock.o_len = _sceneryArchive.readUint32LE();
		sceneBlock.ob_off = _sceneryArchive.readUint32LE();
		sceneBlock.ob_len = _sceneryArchive.readUint32LE();

		switch (overlayType) {
		case BOUNDARY:
			_sceneryArchive.seek(sceneBlock.b_off, SEEK_SET);
			i = sceneBlock.b_len;
			break;
		case OVERLAY:
			_sceneryArchive.seek(sceneBlock.o_off, SEEK_SET);
			i = sceneBlock.o_len;
			break;
		case OVLBASE:
			_sceneryArchive.seek(sceneBlock.ob_off, SEEK_SET);
			i = sceneBlock.ob_len;
			break;
		default:
			Utils::Error(FILE_ERR, "Bad ovl_type");
			break;
		}
		if (i == 0) {
			for (i = 0; i < OVL_SIZE; i++)
				image[i] = 0;
			return;
		}
	} else {
		const char  *ovl_ext[] = {".b", ".o", ".ob"};
		char *buf = (char *) malloc(2048 + 1);      // Buffer for file access

		strcat(strcpy(buf, _vm._screenNames[screenNum]), ovl_ext[overlayType]);

		if (!fileExists(buf)) {
			for (i = 0; i < OVL_SIZE; i++)
				image[i] = 0;
			return;
		}

		if (!_sceneryArchive.open(buf))
			Utils::Error(FILE_ERR, buf);

//		if (eof(f_scenery)) {
//			_lclose(f_scenery);
//			return;
//		}
	}

	switch (_vm._gameVariant) {
	case 0:                                         // Hugo 1 DOS and WIN don't pack data
	case 3:
		_sceneryArchive.read(tmpImage, OVL_SIZE);
		break;
	default:
		// Read in the overlay file using MAC Packbits.  (We're not proud!)
		k = 0;                                      // byte count
		do {
			data = _sceneryArchive.readByte();      // Read a code byte
			if ((byte)data == 0x80)             // Noop
				k = k;
			else if (data >= 0) {                   // Copy next data+1 literally
				for (i = 0; i <= (byte)data; i++, k++)
					*tmpImage++ = _sceneryArchive.readByte();
			} else {                            // Repeat next byte -data+1 times
				j = _sceneryArchive.readByte();

				for (i = 0; i < (byte)(-data + 1); i++, k++)
					*tmpImage++ = j;
			}
		} while (k < OVL_SIZE);
		break;
	}

	if (!_vm.isPacked())
		_sceneryArchive.close();
}

void FileManager::saveSeq(object_t *obj) {
// Save sequence number and image number in given object
	byte   j, k;
	seq_t *q;
	bool   found;

	debugC(1, kDebugFile, "saveSeq");

	for (j = 0, found = false; !found && (j < obj->seqNumb); j++) {
		q = obj->seqList[j].seqPtr;
		for (k = 0; !found && (k < obj->seqList[j].imageNbr); k++) {
			if (obj->currImagePtr == q) {
				found = true;
				obj->curSeqNum = j;
				obj->curImageNum = k;
			} else
				q = q->nextSeqPtr;
		}
	}
}

void FileManager::restoreSeq(object_t *obj) {
// Set up cur_seq_p from stored sequence and image number in object
	int    j;
	seq_t *q;

	debugC(1, kDebugFile, "restoreSeq");

	q = obj->seqList[obj->curSeqNum].seqPtr;
	for (j = 0; j < obj->curImageNum; j++)
		q = q->nextSeqPtr;
	obj->currImagePtr = q;
}

void FileManager::saveGame(int16 slot, const char *descrip) {
// Save game to supplied slot (-1 is INITFILE)
	int     i;
	char    path[256];                                  // Full path of saved game

	debugC(1, kDebugFile, "saveGame(%d, %s)", slot, descrip);

	// Get full path of saved game file - note test for INITFILE
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
	for (i = 0; i < _vm._numObj; i++) {
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
	int       i;
	char      path[256];                            // Full path of saved game
	object_t *p;
	seqList_t seqList[MAX_SEQUENCES];
//	cmdList  *cmds;                                  // Save command list pointer
	uint16    cmdIndex;                             // Save command list pointer
//	char      ver[sizeof(VER)];                      // Compare versions

	debugC(1, kDebugFile, "restoreGame(%d)", slot);

	// Initialize new-game status
	_vm.initStatus();

	// Get full path of saved game file - note test for INITFILE
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
		Utils::Error(GEN_ERR, "Savegame of incompatible version");
		return;
	}

	// Skip over description
	in->seek(DESCRIPLEN, SEEK_CUR);

	// If hero image is currently swapped, swap it back before restore
	if (_vm._heroImage != HERO)
		_vm.scheduler().swapImages(HERO, _vm._heroImage);

	// Restore objects, retain current seqList which points to dynamic mem
	// Also, retain cmnd_t pointers
	for (i = 0; i < _vm._numObj; i++) {
		p = &_vm._objects[i];
		memcpy(seqList, p->seqList, sizeof(seqList_t));
		cmdIndex = p->cmdIndex;
		in->read(p, sizeof(object_t));
		p->cmdIndex = cmdIndex;
		memcpy(p->seqList, seqList, sizeof(seqList_t));
	}

	in->read(&_vm._heroImage, sizeof(_vm._heroImage));

	// If hero swapped in saved game, swap it
	if ((i = _vm._heroImage) != HERO)
		_vm.scheduler().swapImages(HERO, _vm._heroImage);
	_vm._heroImage = i;

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
	for (i = 0; i < _vm._numObj; i++)
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
	Common::File f;                                 // Handle of saved game file
	char path[256];                                 // Full path of INITFILE

	debugC(1, kDebugFile, "initSavedGame");

	// Get full path of INITFILE
	sprintf(path, "%s", _vm._initFilename);


	// Force save of initial game
	if (_vm.getGameStatus().initSaveFl)
		saveGame(-1, "");

	// If initial game doesn't exist, create it
	Common::SeekableReadStream *in = 0;
	if (!(in = _vm.getSaveFileManager()->openForLoading(path))) {
		saveGame(-1, "");
		if (!(in = _vm.getSaveFileManager()->openForLoading(path))) {
			Utils::Error(WRITE_ERR, path);
			return;
		}
	}

	// Must have an open saved game now
	_vm.getGameStatus().saveSize = in->size();
	delete in;

	// Check sanity - maybe disk full or path set to read-only drive?
	if (_vm.getGameStatus().saveSize == -1)
		Utils::Error(WRITE_ERR, path);
}

// Record and playback handling stuff:
typedef struct {
//	int    key;                                     // Character
	uint32 time;                                    // Time at which character was pressed
} pbdata_t;
static pbdata_t pbdata;
FILE            *fpb;

void FileManager::openPlaybackFile(bool playbackFl, bool recordFl) {
	debugC(1, kDebugFile, "openPlaybackFile(%d, %d)", (playbackFl) ? 1 : 0, (recordFl) ? 1 : 0);

	if (playbackFl) {
		if (!(fpb = fopen(PBFILE, "r+b")))
			Utils::Error(FILE_ERR, PBFILE);
	} else if (recordFl)
		fpb = fopen(PBFILE, "wb");
	pbdata.time = 0;                                // Say no key available
}

void FileManager::closePlaybackFile() {
	fclose(fpb);
}

void FileManager::openDatabaseFiles() {
//TODO : HUGO 1 DOS uses _stringtData instead of a strings.dat
//This should be tested adequately and should be handled by an error and not by a warning.
	debugC(1, kDebugFile, "openDatabaseFiles");

	if (!_stringArchive.open(STRING_FILE))
//		Error(FILE_ERR, STRING_FILE);
		warning("Hugo Error: File not found %s", STRING_FILE);
	if (_vm.isPacked()) {
		if (!_sceneryArchive.open(SCENERY_FILE))
			Utils::Error(FILE_ERR, SCENERY_FILE);
		if (!_objectsArchive.open(OBJECTS_FILE))
			Utils::Error(FILE_ERR, OBJECTS_FILE);
	}
}

void FileManager::closeDatabaseFiles() {
// TODO: stringArchive shouldn't be closed in Hugo 1 DOS
	debugC(1, kDebugFile, "closeDatabaseFiles");

	_stringArchive.close();
	if (_vm.isPacked()) {
		_sceneryArchive.close();
		_objectsArchive.close();
	}
}

char *FileManager::fetchString(int index) {
//TODO : HUGO 1 DOS uses _stringtData instead of a strings.dat
// Fetch string from file, decode and return ptr to string in memory
	uint32 off1, off2;

	debugC(1, kDebugFile, "fetchString(%d)", index);

	// Get offset to string[index] (and next for length calculation)
	_stringArchive.seek((uint32)index * sizeof(uint32), SEEK_SET);
	if (_stringArchive.read((char *)&off1, sizeof(uint32)) == 0)
		Utils::Error(FILE_ERR, "String offset");
	if (_stringArchive.read((char *)&off2, sizeof(uint32)) == 0)
		Utils::Error(FILE_ERR, "String offset");

	// Check size of string
	if ((off2 - off1) >= MAX_BOX)
		Utils::Error(FILE_ERR, "Fetched string too long!");

	// Position to string and read it into gen purpose _textBoxBuffer
	_stringArchive.seek(off1, SEEK_SET);
	if (_stringArchive.read(_textBoxBuffer, (uint16)(off2 - off1)) == 0)
		Utils::Error(FILE_ERR, "Fetch_string");

	// Null terminate, decode and return it
	_textBoxBuffer[off2-off1] = '\0';
	_vm.scheduler().decodeString(_textBoxBuffer);
	return _textBoxBuffer;
}

void FileManager::printBootText() {
// Read the encrypted text from the boot file and print it
	Common::File ofp;
	int  i;
	char *buf;

	debugC(1, kDebugFile, "printBootText");

	if (!ofp.open(BOOTFILE))
		Utils::Error(FILE_ERR, BOOTFILE);

	// Allocate space for the text and print it
	buf = (char *)malloc(_boot.exit_len + 1);
	if (buf) {
		// Skip over the boot structure (already read) and read exit text
		ofp.seek((long)sizeof(_boot), SEEK_SET);
		if (ofp.read(buf, _boot.exit_len) != (size_t)_boot.exit_len)
			Utils::Error(FILE_ERR, BOOTFILE);

		// Decrypt the exit text, using CRYPT substring
		for (i = 0; i < _boot.exit_len; i++)
			buf[i] ^= CRYPT[i % strlen(CRYPT)];

		buf[i] = '\0';
		//Box(BOX_OK, buf_p);
		//MessageBox(hwnd, buf_p, "License", MB_ICONINFORMATION);
		warning("printBootText(): License: %s", buf);
	}

	free(buf);
	ofp.close();
}

void FileManager::readBootFile() {
// Reads boot file for program environment.  Fatal error if not there or
// file checksum is bad.  De-crypts structure while checking checksum
	byte checksum;
	byte *p;
	Common::File ofp;
	uint32 i;

	debugC(1, kDebugFile, "readBootFile");

	if (!ofp.open(BOOTFILE))
		Utils::Error(FILE_ERR, BOOTFILE);

	if (ofp.size() < (int32)sizeof(_boot))
		Utils::Error(FILE_ERR, BOOTFILE);

	_boot.checksum = ofp.readByte();
	_boot.registered = ofp.readByte();
	ofp.read(_boot.pbswitch, sizeof(_boot.pbswitch));
	ofp.read(_boot.distrib, sizeof(_boot.distrib));
	_boot.exit_len = ofp.readUint16LE();

	p = (byte *)&_boot;
	for (i = 0, checksum = 0; i < sizeof(_boot); i++) {
		checksum ^= p[i];
		p[i] ^= CRYPT[i % strlen(CRYPT)];
	}
	ofp.close();

	if (checksum)
		Utils::Error(GEN_ERR, "Program startup file invalid");
}

void FileManager::readConfig() {
// Read the user's config if it exists
	Common::File f;
	fpath_t  path;
	config_t tmpConfig = _config;

	debugC(1, kDebugFile, "readConfig");

	sprintf(path, "%s%s", _vm.getGameStatus().path, CONFIGFILE);
	if (f.open(path)) {
		// If config format changed, ignore it and use defaults
		if (f.read(&_config, sizeof(_config)) != sizeof(_config))
			_config = tmpConfig;

		f.close();
	}
}

void FileManager::writeConfig() {
// Write the user's config
	FILE   *f;
	fpath_t path;

	debugC(1, kDebugFile, "writeConfig");

	// Write user's config
	// No error checking in case CD-ROM with no alternate path specified
	sprintf(path, "%s%s", _vm.getGameStatus().path, CONFIGFILE);
	if ((f = fopen(path, "w+")) != NULL)
		fwrite(&_config, sizeof(_config), 1, f);

	fclose(f);
}

uif_hdr_t *FileManager::getUIFHeader(uif_t id) {
// Returns address of uif_hdr[id], reading it in if first call
	static uif_hdr_t UIFHeader[MAX_UIFS];           // Lookup for uif fonts/images
	static bool firstFl = true;
	Common::File ip;                                // Image data file

	debugC(1, kDebugFile, "getUIFHeader(%d)", id);

	// Initialize offset lookup if not read yet
	if (firstFl) {
		firstFl = false;
		// Open unbuffered to do far read
		if (!ip.open(UIF_FILE))
			Utils::Error(FILE_ERR, UIF_FILE);

		if (ip.size() < (int32)sizeof(UIFHeader))
			Utils::Error(FILE_ERR, UIF_FILE);

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
	Common::File ip;                                // UIF_FILE handle
	uif_hdr_t *UIFHeaderPtr;                        // Lookup table of items
	seq_t seq;                                      // Dummy seq_t for image data

	debugC(1, kDebugFile, "readUIFItem(%d, ...)", id);

	// Open uif file to read data
	if (!ip.open(UIF_FILE))
		Utils::Error(FILE_ERR, UIF_FILE);

	// Seek to data
	UIFHeaderPtr = getUIFHeader((uif_t)id);
	ip.seek(UIFHeaderPtr->offset, SEEK_SET);

	// We support pcx images and straight data
	switch (id) {
	case UIF_IMAGES:                                // Read uif images file
		readPCX(ip, &seq, buf, true, UIF_FILE);
		break;
	default:                                        // Read file data into supplied array
		if (ip.read(buf, UIFHeaderPtr->size) != UIFHeaderPtr->size)
			Utils::Error(FILE_ERR, UIF_FILE);
		break;
	}

	ip.close();
}

void FileManager::instructions() {
// Simple instructions given when F1 pressed twice in a row
// Only in DOS versions
#define HELPFILE "help.dat"
#define EOP '#' /* Marks end of a page in help file */

	Common::File f;
	char line[1024], *wrkLine;
	char readBuf[2];

	wrkLine = line;
	if (!f.open(UIF_FILE))
		Utils::Error(FILE_ERR, HELPFILE);

	while (f.read(readBuf, 1)) {
		wrkLine[0] = readBuf[0];
		do {
			f.read(wrkLine, 1);
		} while (*wrkLine++ != EOP);
		wrkLine[-2] = '\0';      /* Remove EOP and previous CR */
		Utils::Box(BOX_ANY, line);
		f.read(wrkLine, 1);    /* Remove CR after EOP */
	}
	f.close();
}

} // end of namespace Hugo
