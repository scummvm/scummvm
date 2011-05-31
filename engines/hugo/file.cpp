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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/debug.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "common/config-manager.h"
#include "graphics/thumbnail.h"
#include "gui/saveload.h"

#include "hugo/hugo.h"
#include "hugo/file.h"
#include "hugo/schedule.h"
#include "hugo/display.h"
#include "hugo/util.h"
#include "hugo/object.h"
#include "hugo/text.h"
#include "hugo/mouse.h"

namespace Hugo {

namespace {
static const char s_bootCypher[] = "Copyright 1992, David P Gray, Gray Design Associates";
static const int s_bootCypherLen = sizeof(s_bootCypher) - 1;
}


FileManager::FileManager(HugoEngine *vm) : _vm(vm) {
	has_read_header = false;
	firstUIFFl = true;
}

FileManager::~FileManager() {
}

/**
 * Name scenery and objects picture databases
 */
const char *FileManager::getBootFilename() const {
	return "HUGO.BSF";
}

const char *FileManager::getObjectFilename() const {
	return "objects.dat";
}

const char *FileManager::getSceneryFilename() const {
	return "scenery.dat";
}

const char *FileManager::getSoundFilename() const {
	return "sounds.dat";
}

const char *FileManager::getStringFilename() const {
	return "strings.dat";
}

const char *FileManager::getUifFilename() const {
	return "uif.dat";
}

/**
 * Convert 4 planes (RGBI) data to 8-bit DIB format
 * Return original plane data ptr
 */
byte *FileManager::convertPCC(byte *p, const uint16 y, const uint16 bpl, image_pt dataPtr) const {
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
seq_t *FileManager::readPCX(Common::ReadStream &f, seq_t *seqPtr, byte *imagePtr, const bool firstFl, const char *name) {
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
		error("Bad data file format: %s", name);

	// Allocate memory for seq_t if 0
	if (seqPtr == 0) {
		if ((seqPtr = (seq_t *)malloc(sizeof(seq_t))) == 0)
			error("Insufficient memory to run game.");
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
	if (imagePtr == 0)
		imagePtr = (byte *)malloc((size_t) size);

	assert(imagePtr);

	seqPtr->imagePtr = imagePtr;

	// Process the image data, converting to 8-bit DIB format
	uint16 y = 0;                                   // Current line index
	byte  pline[kXPix];                             // Hold 4 planes of data
	byte  *p = pline;                               // Ptr to above
	while (y < seqPtr->lines) {
		byte c = f.readByte();
		if ((c & kRepeatMask) == kRepeatMask) {
			byte d = f.readByte();                  // Read data byte
			for (int i = 0; i < (c & kLengthMask); i++) {
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
void FileManager::readImage(const int objNum, object_t *objPtr) {
	debugC(1, kDebugFile, "readImage(%d, object_t *objPtr)", objNum);

	/**
	 * Structure of object file lookup entry
	 */
	struct objBlock_t {
		uint32 objOffset;
		uint32 objLength;
	};

	if (!objPtr->seqNumb)                           // This object has no images
		return;

	if (_vm->isPacked()) {
		_objectsArchive.seek((uint32)objNum * sizeof(objBlock_t), SEEK_SET);

		objBlock_t objBlock;                        // Info on file within database
		objBlock.objOffset = _objectsArchive.readUint32LE();
		objBlock.objLength = _objectsArchive.readUint32LE();

		_objectsArchive.seek(objBlock.objOffset, SEEK_SET);
	} else {
		Common::String buf;
		buf = _vm->_picDir + Common::String(_vm->_text->getNoun(objPtr->nounIndex, 0)) + ".PIX";
		if (!_objectsArchive.open(buf)) {
			buf = Common::String(_vm->_text->getNoun(objPtr->nounIndex, 0)) + ".PIX";
			if (!_objectsArchive.open(buf))
				error("File not found: %s", buf.c_str());
		}
	}

	bool  firstImgFl = true;                        // Initializes pcx read function
	seq_t *seqPtr = 0;                              // Ptr to sequence structure

	// Now read the images into an images list
	for (int j = 0; j < objPtr->seqNumb; j++) {     // for each sequence
		for (int k = 0; k < objPtr->seqList[j].imageNbr; k++) { // each image
			if (k == 0) {                           // First image
				// Read this image - allocate both seq and image memory
				seqPtr = readPCX(_objectsArchive, 0, 0, firstImgFl, _vm->_text->getNoun(objPtr->nounIndex, 0));
				objPtr->seqList[j].seqPtr = seqPtr;
				firstImgFl = false;
			} else {                                // Subsequent image
				// Read this image - allocate both seq and image memory
				seqPtr->nextSeqPtr = readPCX(_objectsArchive, 0, 0, firstImgFl, _vm->_text->getNoun(objPtr->nounIndex, 0));
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
	case kCycleInvisible:                           // (May become visible later)
	case kCycleAlmostInvisible:
	case kCycleNotCycling:
	case kCycleForward:
		objPtr->currImagePtr = objPtr->seqList[0].seqPtr;
		break;
	case kCycleBackward:
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
sound_pt FileManager::getSound(const int16 sound, uint16 *size) {
	debugC(1, kDebugFile, "getSound(%d)", sound);

	// No more to do if SILENCE (called for cleanup purposes)
	if (sound == _vm->_soundSilence)
		return 0;

	// Open sounds file
	Common::File fp;                                // Handle to SOUND_FILE
	if (!fp.open(getSoundFilename())) {
		warning("Hugo Error: File not found %s", getSoundFilename());
		return 0;
	}

	if (!has_read_header) {
		for (int i = 0; i < kMaxSounds; i++) {
			s_hdr[i].size = fp.readUint16LE();
			s_hdr[i].offset = fp.readUint32LE();
		}
		if (fp.err())
			error("Wrong sound file format");
		has_read_header = true;
	}

	*size = s_hdr[sound].size;
	if (*size == 0)
		error("Wrong sound file format or missing sound %d", sound);

	// Allocate memory for sound or music, if possible
	sound_pt soundPtr = (byte *)malloc(s_hdr[sound].size); // Ptr to sound data
	assert(soundPtr);

	// Seek to data and read it
	fp.seek(s_hdr[sound].offset, SEEK_SET);
	if (fp.read(soundPtr, s_hdr[sound].size) != s_hdr[sound].size)
		error("Wrong sound file format");

	fp.close();

	return soundPtr;
}

/**
 * Save game to supplied slot
 */
bool FileManager::saveGame(const int16 slot, const Common::String &descrip) {
	debugC(1, kDebugFile, "saveGame(%d, %s)", slot, descrip.c_str());

	const EnginePlugin *plugin = NULL;
	int16 savegameId;
	Common::String savegameDescription;
	EngineMan.findGame(_vm->getGameId(), &plugin);

	if (slot == -1) {
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser("Save game:", "Save");
		dialog->setSaveMode(true);
		savegameId = dialog->runModalWithPluginAndTarget(plugin, ConfMan.getActiveDomainName());
		savegameDescription = dialog->getResultString();
		delete dialog;
	} else {
		savegameId = slot;
		if (!descrip.empty()) {
			savegameDescription = descrip;
		} else {
			savegameDescription = Common::String::format("Quick save #%d", slot);
		}
	}

	if (savegameId < 0)                             // dialog aborted
		return false;

	Common::String savegameFile = _vm->getSavegameFilename(savegameId);
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::OutSaveFile *out = saveMan->openForSaving(savegameFile);

	if (!out) {
		warning("Can't create file '%s', game not saved", savegameFile.c_str());
		return false;
	}

	// Write version.  We can't restore from obsolete versions
	out->writeByte(kSavegameVersion);

	if (savegameDescription == "") {
		savegameDescription = "Untitled savegame";
	}

	out->writeSint16BE(savegameDescription.size() + 1);
	out->write(savegameDescription.c_str(), savegameDescription.size() + 1);

	Graphics::saveThumbnail(*out);

	TimeDate curTime;
	_vm->_system->getTimeAndDate(curTime);

	uint32 saveDate = (curTime.tm_mday & 0xFF) << 24 | ((curTime.tm_mon + 1) & 0xFF) << 16 | ((curTime.tm_year + 1900) & 0xFFFF);
	uint16 saveTime = (curTime.tm_hour & 0xFF) << 8 | ((curTime.tm_min) & 0xFF);

	out->writeUint32BE(saveDate);
	out->writeUint16BE(saveTime);

	_vm->_object->saveObjects(out);

	const status_t &gameStatus = _vm->getGameStatus();

	// Save whether hero image is swapped
	out->writeByte(_vm->_heroImage);

	// Save score
	out->writeSint16BE(_vm->getScore());

	// Save story mode
	out->writeByte((gameStatus.storyModeFl) ? 1 : 0);

	// Save jumpexit mode
	out->writeByte((_vm->_mouse->getJumpExitFl()) ? 1 : 0);

	// Save gameover status
	out->writeByte((gameStatus.gameOverFl) ? 1 : 0);

	// Save screen states
	for (int i = 0; i < _vm->_numStates; i++)
		out->writeByte(_vm->_screenStates[i]);

	_vm->_scheduler->saveSchedulerData(out);
	// Save palette table
	_vm->_screen->savePal(out);

	// Save maze status
	out->writeByte((_vm->_maze.enabledFl) ? 1 : 0);
	out->writeByte(_vm->_maze.size);
	out->writeSint16BE(_vm->_maze.x1);
	out->writeSint16BE(_vm->_maze.y1);
	out->writeSint16BE(_vm->_maze.x2);
	out->writeSint16BE(_vm->_maze.y2);
	out->writeSint16BE(_vm->_maze.x3);
	out->writeSint16BE(_vm->_maze.x4);
	out->writeByte(_vm->_maze.firstScreenIndex);

	out->writeByte((byte)_vm->getGameStatus().viewState);

	out->finalize();

	delete out;

	return true;
}

/**
 * Restore game from supplied slot number
 */
bool FileManager::restoreGame(const int16 slot) {
	debugC(1, kDebugFile, "restoreGame(%d)", slot);

	const EnginePlugin *plugin = NULL;
	int16 savegameId;
	EngineMan.findGame(_vm->getGameId(), &plugin);

	if (slot == -1) {
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser("Restore game:", "Restore");
		dialog->setSaveMode(false);
		savegameId = dialog->runModalWithPluginAndTarget(plugin, ConfMan.getActiveDomainName());
		delete dialog;
	} else {
		savegameId = slot;
	}

	if (savegameId < 0)                             // dialog aborted
		return false;

	Common::String savegameFile = _vm->getSavegameFilename(savegameId);
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in = saveMan->openForLoading(savegameFile);

	if (!in)
		return false;

	// Initialize new-game status
	_vm->initStatus();

	// Check version, can't restore from different versions
	int saveVersion = in->readByte();
	if (saveVersion != kSavegameVersion) {
		warning("Savegame of incompatible version");
		delete in;
		return false;
	}

	// Skip over description
	int32 saveGameNameSize = in->readSint16BE();
	in->skip(saveGameNameSize);

	Graphics::skipThumbnail(*in);

	in->skip(6);                                    // Skip date & time

	// If hero image is currently swapped, swap it back before restore
	if (_vm->_heroImage != kHeroIndex)
		_vm->_object->swapImages(kHeroIndex, _vm->_heroImage);

	_vm->_object->restoreObjects(in);

	_vm->_heroImage = in->readByte();

	// If hero swapped in saved game, swap it
	byte heroImg = _vm->_heroImage;
	if (heroImg != kHeroIndex)
		_vm->_object->swapImages(kHeroIndex, _vm->_heroImage);
	_vm->_heroImage = heroImg;

	status_t &gameStatus = _vm->getGameStatus();

	int score = in->readSint16BE();
	_vm->setScore(score);

	gameStatus.storyModeFl = (in->readByte() == 1);
	_vm->_mouse->setJumpExitFl(in->readByte() == 1);
	gameStatus.gameOverFl = (in->readByte() == 1);
	for (int i = 0; i < _vm->_numStates; i++)
		_vm->_screenStates[i] = in->readByte();

	_vm->_scheduler->restoreSchedulerData(in);

	// Restore palette and change it if necessary
	_vm->_screen->restorePal(in);

	// Restore maze status
	_vm->_maze.enabledFl = (in->readByte() == 1);
	_vm->_maze.size = in->readByte();
	_vm->_maze.x1 = in->readSint16BE();
	_vm->_maze.y1 = in->readSint16BE();
	_vm->_maze.x2 = in->readSint16BE();
	_vm->_maze.y2 = in->readSint16BE();
	_vm->_maze.x3 = in->readSint16BE();
	_vm->_maze.x4 = in->readSint16BE();
	_vm->_maze.firstScreenIndex = in->readByte();

	_vm->_scheduler->restoreScreen(*_vm->_screen_p);
	if ((_vm->getGameStatus().viewState = (vstate_t) in->readByte()) != kViewPlay)
		_vm->_screen->hideCursor();


	delete in;
	return true;
}

/**
 * Read the encrypted text from the boot file and print it
 */
void FileManager::printBootText() {
	debugC(1, kDebugFile, "printBootText()");

	Common::File ofp;
	if (!ofp.open(getBootFilename())) {
		if (_vm->getPlatform() == Common::kPlatformPC) {
			//TODO initialize properly _boot structure
			warning("printBootText - Skipping as Dos versions may be a freeware or shareware");
			return;
		} else {
			Utils::notifyBox(Common::String::format("Missing startup file '%s'", getBootFilename()));
			_vm->getGameStatus().doQuitFl = true;
			return;
		}
	}

	// Allocate space for the text and print it
	char *buf = (char *)malloc(_vm->_boot.exit_len + 1);
	if (buf) {
		// Skip over the boot structure (already read) and read exit text
		ofp.seek((long)sizeof(_vm->_boot), SEEK_SET);
		if (ofp.read(buf, _vm->_boot.exit_len) != (size_t)_vm->_boot.exit_len) {
			Utils::notifyBox(Common::String::format("Error while reading startup file '%s'", getBootFilename()));
			_vm->getGameStatus().doQuitFl = true;
			return;
		}

		// Decrypt the exit text, using CRYPT substring
		int i;
		for (i = 0; i < _vm->_boot.exit_len; i++)
			buf[i] ^= s_bootCypher[i % s_bootCypherLen];

		buf[i] = '\0';
		Utils::notifyBox(buf);
	}

	free(buf);
	ofp.close();
}

/**
 * Reads boot file for program environment.  Fatal error if not there or
 * file checksum is bad.  De-crypts structure while checking checksum
 */
void FileManager::readBootFile() {
	debugC(1, kDebugFile, "readBootFile()");

	Common::File ofp;
	if (!ofp.open(getBootFilename())) {
		if (_vm->_gameVariant == kGameVariantH1Dos) {
			//TODO initialize properly _boot structure
			warning("readBootFile - Skipping as H1 Dos may be a freeware");
			memset(_vm->_boot.distrib, '\0', sizeof(_vm->_boot.distrib));
			_vm->_boot.registered = kRegFreeware;
			return;
		} else if (_vm->getPlatform() == Common::kPlatformPC) {
			warning("readBootFile - Skipping as H2 and H3 Dos may be shareware");
			memset(_vm->_boot.distrib, '\0', sizeof(_vm->_boot.distrib));
			_vm->_boot.registered = kRegShareware;
			return;
		} else {
			Utils::notifyBox(Common::String::format("Missing startup file '%s'", getBootFilename()));
			_vm->getGameStatus().doQuitFl = true;
			return;
		}
	}

	if (ofp.size() < (int32)sizeof(_vm->_boot)) {
		Utils::notifyBox(Common::String::format("Corrupted startup file '%s'", getBootFilename()));
		_vm->getGameStatus().doQuitFl = true;
		return;
	}

	_vm->_boot.checksum = ofp.readByte();
	_vm->_boot.registered = ofp.readByte();
	ofp.read(_vm->_boot.pbswitch, sizeof(_vm->_boot.pbswitch));
	ofp.read(_vm->_boot.distrib, sizeof(_vm->_boot.distrib));
	_vm->_boot.exit_len = ofp.readUint16LE();

	byte *p = (byte *)&_vm->_boot;

	byte checksum = 0;
	for (uint32 i = 0; i < sizeof(_vm->_boot); i++) {
		checksum ^= p[i];
		p[i] ^= s_bootCypher[i % s_bootCypherLen];
	}
	ofp.close();

	if (checksum) {
		Utils::notifyBox(Common::String::format("Corrupted startup file '%s'", getBootFilename()));
		_vm->getGameStatus().doQuitFl = true;
	}
}

/**
 * Returns address of uif_hdr[id], reading it in if first call
 * This file contains, between others, the bitmaps of the fonts used in the application
 * UIF means User interface database (Windows Only)
 */
uif_hdr_t *FileManager::getUIFHeader(const uif_t id) {
	debugC(1, kDebugFile, "getUIFHeader(%d)", id);

	// Initialize offset lookup if not read yet
	if (firstUIFFl) {
		firstUIFFl = false;
		// Open unbuffered to do far read
		Common::File ip;                            // Image data file
		if (!ip.open(getUifFilename()))
			error("File not found: %s", getUifFilename());

		if (ip.size() < (int32)sizeof(UIFHeader))
			error("Wrong UIF file format");

		for (int i = 0; i < kMaxUifs; ++i) {
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
void FileManager::readUIFItem(const int16 id, byte *buf) {
	debugC(1, kDebugFile, "readUIFItem(%d, ...)", id);

	// Open uif file to read data
	Common::File ip;                                // UIF_FILE handle
	if (!ip.open(getUifFilename()))
		error("File not found: %s", getUifFilename());

	// Seek to data
	uif_hdr_t *UIFHeaderPtr = getUIFHeader((uif_t)id);
	ip.seek(UIFHeaderPtr->offset, SEEK_SET);

	// We support pcx images and straight data
	seq_t *dummySeq;                                // Dummy seq_t for image data
	switch (id) {
	case UIF_IMAGES:                                // Read uif images file
		dummySeq = readPCX(ip, 0, buf, true, getUifFilename());
		free(dummySeq);
		break;
	default:                                        // Read file data into supplied array
		if (ip.read(buf, UIFHeaderPtr->size) != UIFHeaderPtr->size)
			error("Wrong UIF file format");
		break;
	}

	ip.close();
}

/**
 * Read the uif image file (inventory icons)
 */
void FileManager::readUIFImages() {
	debugC(1, kDebugFile, "readUIFImages()");

	readUIFItem(UIF_IMAGES, _vm->_screen->getGUIBuffer());   // Read all uif images
}

} // End of namespace Hugo

