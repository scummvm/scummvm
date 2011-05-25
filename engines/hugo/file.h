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

#ifndef HUGO_FILE_H
#define HUGO_FILE_H

namespace Hugo {
/**
 * Enumerate overlay file types
 */
enum ovl_t {kOvlBoundary, kOvlOverlay, kOvlBase};

struct uif_hdr_t {                                  // UIF font/image look up
	uint16  size;                                   // Size of uif item
	uint32  offset;                                 // Offset of item in file
};


class FileManager {
public:
	FileManager(HugoEngine *vm);
	virtual ~FileManager();

	sound_pt getSound(const int16 sound, uint16 *size);

	void     readBootFile();
	void     readImage(const int objNum, object_t *objPtr);
	void     readUIFImages();
	void     readUIFItem(const int16 id, byte *buf);
	bool     restoreGame(const int16 slot);
	bool     saveGame(const int16 slot, const Common::String &descrip);

	// Name scenery and objects picture databases
	const char *getBootFilename()    const;
	const char *getObjectFilename()  const;
	const char *getSceneryFilename() const;
	const char *getSoundFilename()   const;
	const char *getStringFilename()  const;
	const char *getUifFilename()     const;

	virtual void openDatabaseFiles() = 0;
	virtual void closeDatabaseFiles() = 0;
	virtual void instructions() const = 0;

	virtual void readBackground(const int screenIndex) = 0;
	virtual void readOverlay(const int screenNum, image_pt image, ovl_t overlayType) = 0;

	virtual const char *fetchString(const int index) = 0;

protected:
	HugoEngine *_vm;
	static const int kMaxUifs = 32;                 // Max possible uif items in hdr
	static const int kMaxSounds = 64;               // Max number of sounds
	static const int kRepeatMask = 0xC0;            // Top 2 bits mean a repeat code
	static const int kLengthMask = 0x3F;            // Lower 6 bits are length
	static const int kNumColors = 16;               // Num colors to save in palette

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

	struct PCC_header_t {                           // Structure of PCX file header
		byte   mfctr, vers, enc, bpx;
		uint16  x1, y1, x2, y2;                     // bounding box
		uint16  xres, yres;
		byte   palette[3 * kNumColors];             // EGA color palette
		byte   vmode, planes;
		uint16 bytesPerLine;                        // Bytes per line
		byte   fill2[60];
	};                                              // Header of a PCC file

	bool firstUIFFl;
	uif_hdr_t UIFHeader[kMaxUifs];                  // Lookup for uif fonts/images

	Common::File _stringArchive;                    // Handle for string file
	Common::File _sceneryArchive1;                  // Handle for scenery file
	Common::File _objectsArchive;                   // Handle for objects file

	PCC_header_t PCC_header;

	seq_t *readPCX(Common::ReadStream &f, seq_t *seqPtr, byte *imagePtr, const bool firstFl, const char *name);

	// If this is the first call, read the lookup table
	bool has_read_header;
	sound_hdr_t s_hdr[kMaxSounds];                  // Sound lookup table

private:
	byte *convertPCC(byte *p, const uint16 y, const uint16 bpl, image_pt dataPtr) const;
	uif_hdr_t *getUIFHeader(const uif_t id);

//Strangerke : Not used?
	void     printBootText();
};

class FileManager_v1d : public FileManager {
public:
	FileManager_v1d(HugoEngine *vm);
	~FileManager_v1d();

	virtual void closeDatabaseFiles();
	virtual void instructions() const;
	virtual void openDatabaseFiles();
	virtual void readBackground(const int screenIndex);
	virtual void readOverlay(const int screenNum, image_pt image, ovl_t overlayType);
	virtual const char *fetchString(const int index);
};

class FileManager_v2d : public FileManager_v1d {
public:
	FileManager_v2d(HugoEngine *vm);
	~FileManager_v2d();

	virtual void closeDatabaseFiles();
	virtual void openDatabaseFiles();
	virtual void readBackground(const int screenIndex);
	virtual void readOverlay(const int screenNum, image_pt image, ovl_t overlayType);
	const   char *fetchString(const int index);
private:
	char *_fetchStringBuf;
};

class FileManager_v3d : public FileManager_v2d {
public:
	FileManager_v3d(HugoEngine *vm);
	~FileManager_v3d();

	void closeDatabaseFiles();
	void openDatabaseFiles();
	void readBackground(const int screenIndex);
	void readOverlay(const int screenNum, image_pt image, ovl_t overlayType);
private:
	Common::File _sceneryArchive2;                  // Handle for scenery file
};

class FileManager_v2w : public FileManager_v2d {
public:
	FileManager_v2w(HugoEngine *vm);
	~FileManager_v2w();

	void instructions() const;
};

class FileManager_v1w : public FileManager_v2w {
public:
	FileManager_v1w(HugoEngine *vm);
	~FileManager_v1w();

	void readOverlay(const int screenNum, image_pt image, ovl_t overlayType);
};

} // End of namespace Hugo
#endif //HUGO_FILE_H
