/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef BUNDLE_H
#define BUNDLE_H

#include "scummsys.h"
#include "file.h"

class Bundle {

private:

	struct CompTable {
		int32 offset;
		int32 size;
		int32 codec;
	};
	
	struct BundleAudioTable {
		char filename[13];
		int32 size;
		int32 offset;
	};
	
	CompTable *_compVoiceTable;
	CompTable *_compMusicTable;
	BundleAudioTable *_bundleVoiceTable;
	BundleAudioTable *_bundleMusicTable;
	int32 _numVoiceFiles;
	int32 _numMusicFiles;
	int32 _lastSong;

	byte _destImcTable[93];
	uint32 _destImcTable2[5697];

	File _voiceFile;
	File _musicFile;

	void initializeImcTables();

	int32 decompressCodec(int32 codec, byte *comp_input, byte *comp_output, int32 size);

	int32 decompressVoiceSampleByIndex(int32 index, byte **comp_final);
	int32 decompressMusicSampleByIndex(int32 index, int32 number, byte *comp_final);
	int32 getNumberOfMusicSamplesByIndex(int32 index);

public:
	Bundle();
	~Bundle();

	bool openVoiceFile(const char *filename, const char *directory);
	bool openMusicFile(const char *filename, const char *directory);
	void closeVoiceFile();
	void closeMusicFile();

	int32 decompressVoiceSampleByName(const char *name, byte **comp_final);
	int32 decompressMusicSampleByName(const char *name, int32 number, byte *comp_final);
	int32 getNumberOfMusicSamplesByName(const char *name);
};

#endif
