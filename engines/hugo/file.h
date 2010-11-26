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

#ifndef HUGO_FILE_H
#define HUGO_FILE_H

// TODO get rid of those defines
#define HELPFILE "help.dat"
#define EOP '#'                                     // Marks end of a page in help file

struct PCC_header_t {                               // Structure of PCX file header
	byte   mfctr, vers, enc, bpx;
	uint16  x1, y1, x2, y2;                         // bounding box
	uint16  xres, yres;
	byte   palette[3 * NUM_COLORS];                 // EGA color palette
	byte   vmode, planes;
	uint16 bytesPerLine;                            // Bytes per line
	byte   fill2[60];
};                                                  // Header of a PCC file

namespace Hugo {

class FileManager {
public:
	FileManager(HugoEngine *vm);
	virtual ~FileManager();


	bool     fileExists(char *filename);
	sound_pt getSound(short sound, uint16 *size);

	void     instructions();
	void     readBootFile();
	void     readImage(int objNum, object_t *objPtr);
	void     readUIFImages();
	void     readUIFItem(short id, byte *buf);
	void     restoreGame(short slot);
	void     saveGame(short slot, const char *descrip);

	virtual void openDatabaseFiles() = 0;
	virtual void closeDatabaseFiles() = 0;

	virtual void readBackground(int screenIndex) = 0;
	virtual void readOverlay(int screenNum, image_pt image, ovl_t overlayType) = 0;

	virtual char *fetchString(int index) = 0;

protected:
	HugoEngine *_vm;

	Common::File _stringArchive;                    // Handle for string file
	Common::File _sceneryArchive1;                  // Handle for scenery file
	Common::File _objectsArchive;                   // Handle for objects file

	seq_t *readPCX(Common::File &f, seq_t *seqPtr, byte *imagePtr, bool firstFl, const char *name);
private:

	byte *convertPCC(byte *p, uint16 y, uint16 bpl, image_pt data_p);
	uif_hdr_t *getUIFHeader(uif_t id);

//Strangerke : Not used?
	void     printBootText();
};

class FileManager_v1d : public FileManager {
public:
	FileManager_v1d(HugoEngine *vm);
	~FileManager_v1d();

	void openDatabaseFiles();
	void closeDatabaseFiles();
	void readBackground(int screenIndex);
	void readOverlay(int screenNum, image_pt image, ovl_t overlayType);
	char *fetchString(int index);
};

class FileManager_v2d : public FileManager {
public:
	FileManager_v2d(HugoEngine *vm);
	~FileManager_v2d();

	void openDatabaseFiles();
	void closeDatabaseFiles();
	void readBackground(int screenIndex);
	void readOverlay(int screenNum, image_pt image, ovl_t overlayType);
	char *fetchString(int index);
};

class FileManager_v3d : public FileManager_v2d {
public:
	FileManager_v3d(HugoEngine *vm);
	~FileManager_v3d();

	void openDatabaseFiles();
	void closeDatabaseFiles();
	void readBackground(int screenIndex);
	void readOverlay(int screenNum, image_pt image, ovl_t overlayType);
private:
	Common::File _sceneryArchive2;                  // Handle for scenery file
};

class FileManager_v1w : public FileManager_v2d {
public:
	FileManager_v1w(HugoEngine *vm);
	~FileManager_v1w();

	void readOverlay(int screenNum, image_pt image, ovl_t overlayType);
};

} // End of namespace Hugo
#endif //HUGO_FILE_H
