/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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

class Scumm;

class Bundle {
protected:

private:
	int32 compDecode(byte * src, byte * dst);
	int32 decompressCodec(int32 codec, byte * comp_input, byte * comp_output, int32 size);
	CompTable _compVoiceTable[50];
	CompTable _compMusicTable[2500];
	FILE * _voiceFile;
	FILE * _musicFile;
	BundleAudioTable * _bundleVoiceTable;
	BundleAudioTable * _bundleMusicTable;
	int32 _numVoiceFiles;
	int32 _numMusicFiles;
	Scumm * _scumm;
	int32 _lastSong;

public:
	Bundle(Scumm * parent);
	~Bundle();

	bool openVoiceFile(char * filename);
	bool openMusicFile(char * filename);
	int32 decompressVoiceSampleByName(char * name, byte * comp_final);
	int32 decompressVoiceSampleByIndex(int32 index, byte * comp_final);
	int32 decompressMusicSampleByName(char * name, int32 number, byte * comp_final);
	int32 decompressMusicSampleByIndex(int32 index, int32 number, byte * comp_final);
	int32 getNumberOfMusicSamplesByIndex(int32 index);
	int32 getNumberOfMusicSamplesByName(char * name);
};

#endif

