// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef MCMP_MGR_H
#define MCMP_MGR_H

#include "stdafx.h"
#include "bits.h"
#include "debug.h"
#include "timer.h"

#include "mixer/mixer.h"
#include "mixer/audiostream.h"

#include "imuse/imuse_sndmgr.h"

class McmpMgr {
private:

	struct CompTable {
		byte codec;
		int32 decompSize;
		int32 compSize;
		int32 offset;
	};

	CompTable *_compTable;
	int16 _numCompItems;
	int _curSample;
	FILE *_file;
	byte _compOutput[0x2000];
	byte *_compInput;
	int _outputSize;
	int _lastBlock;

public:

	McmpMgr();
	~McmpMgr();

	bool openSound(const char *filename, byte **resPtr, int &offsetData);
	int32 decompressSample(int32 offset, int32 size, byte **comp_final);
};

#endif
