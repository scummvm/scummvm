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

class FileManager {
public:
	FileManager(HugoEngine &vm);


	bool     fileExists(char *filename);

	char    *fetchString(int index);

	sound_pt getSound(short sound, uint16 *size);

	void     closePlaybackFile();
	void     closeDatabaseFiles();
	void     initSavedGame();
	void     instructions();
	void     openDatabaseFiles();
	void     readBackground(int screenIndex);
	void     readBootFile();
	void     readConfig();
	void     readImage(int objNum, object_t *objPtr);
	void     readOverlay(int screenNum, image_pt image, ovl_t overlayType);
	void     readUIFItem(short id, byte *buf);
	void     restoreGame(short slot);
	void     restoreSeq(object_t *obj);
	void     saveGame(short slot, const char *descrip);
	void     saveSeq(object_t *obj);
	void     writeConfig();

private:
	HugoEngine &_vm;

	Common::File _stringArchive;                        /* Handle for string file */
	Common::File _sceneryArchive;                       /* Handle for scenery file */
	Common::File _objectsArchive;                       /* Handle for objects file */

	byte *convertPCC(byte *p, uint16 y, uint16 bpl, image_pt data_p);
	seq_t *readPCX(Common::File &f, seq_t *seqPtr, byte *imagePtr, bool firstFl, const char *name);
	uif_hdr_t *getUIFHeader(uif_t id);

//Strangerke : Not used?
	void     openPlaybackFile(bool playbackFl, bool recordFl);
	void     printBootText();
//	bool     pkkey();
//	char     pbget();
};


} // end of namespace Hugo
#endif //HUGO_FILE_H
