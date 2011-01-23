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

namespace Hugo {
/**
* Enumerate overlay file types
*/
enum ovl_t {kOvlBoundary, kOvlOverlay, kOvlBase};

class FileManager {
public:
	FileManager(HugoEngine *vm);
	virtual ~FileManager();

	bool     fileExists(char *filename);
	sound_pt getSound(int16 sound, uint16 *size);

	void     readBootFile();
	void     readImage(int objNum, object_t *objPtr);
	void     readUIFImages();
	void     readUIFItem(int16 id, byte *buf);
	bool     restoreGame(int16 slot);
	bool     saveGame(int16 slot, Common::String descrip);

	// Name scenery and objects picture databases
	const char *getBootFilename()    { return "HUGO.BSF";    }
	const char *getObjectFilename()  { return "objects.dat"; }
	const char *getSceneryFilename() { return "scenery.dat"; }
	const char *getSoundFilename()   { return "sounds.dat";  }
	const char *getStringFilename()  { return "strings.dat"; }
	const char *getUifFilename()     { return "uif.dat";     }

	virtual void openDatabaseFiles() = 0;
	virtual void closeDatabaseFiles() = 0;
	virtual void instructions() = 0;

	virtual void readBackground(int screenIndex) = 0;
	virtual void readOverlay(int screenNum, image_pt image, ovl_t overlayType) = 0;

	virtual char *fetchString(int index) = 0;

protected:
	HugoEngine *_vm;
	static const int kMaxUifs = 32;                 // Max possible uif items in hdr
	static const int kMaxSounds = 64;               // Max number of sounds
	static const int kRepeatMask = 0xC0;            // Top 2 bits mean a repeat code
	static const int kLengthMask = 0x3F;            // Lower 6 bits are length

	/**
	* Structure of scenery file lookup entry
	*/
	struct sceneBlock_t {
		uint32 scene_off;
		uint32 scene_len;
		uint32 b_off;
		uint32 b_len;
		uint32 o_off;
		uint32 o_len;
		uint32 ob_off;
		uint32 ob_len;
	};

	Common::File _stringArchive;                    // Handle for string file
	Common::File _sceneryArchive1;                  // Handle for scenery file
	Common::File _objectsArchive;                   // Handle for objects file

	seq_t *readPCX(Common::File &f, seq_t *seqPtr, byte *imagePtr, bool firstFl, const char *name);
	const char *getBootCypher();

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

	virtual void closeDatabaseFiles();
	virtual void instructions();
	virtual void openDatabaseFiles();
	virtual void readBackground(int screenIndex);
	virtual void readOverlay(int screenNum, image_pt image, ovl_t overlayType);
	virtual char *fetchString(int index);
};

class FileManager_v2d : public FileManager_v1d {
public:
	FileManager_v2d(HugoEngine *vm);
	~FileManager_v2d();

	virtual void closeDatabaseFiles();
	virtual void openDatabaseFiles();
	virtual void readBackground(int screenIndex);
	virtual void readOverlay(int screenNum, image_pt image, ovl_t overlayType);
	char *fetchString(int index);
};

class FileManager_v3d : public FileManager_v2d {
public:
	FileManager_v3d(HugoEngine *vm);
	~FileManager_v3d();

	void closeDatabaseFiles();
	void openDatabaseFiles();
	void readBackground(int screenIndex);
	void readOverlay(int screenNum, image_pt image, ovl_t overlayType);
private:
	Common::File _sceneryArchive2;                  // Handle for scenery file
};

class FileManager_v2w : public FileManager_v2d {
public:
	FileManager_v2w(HugoEngine *vm);
	~FileManager_v2w();

	void instructions();
};

class FileManager_v1w : public FileManager_v2w {
public:
	FileManager_v1w(HugoEngine *vm);
	~FileManager_v1w();

	void readOverlay(int screenNum, image_pt image, ovl_t overlayType);
};

} // End of namespace Hugo
#endif //HUGO_FILE_H
