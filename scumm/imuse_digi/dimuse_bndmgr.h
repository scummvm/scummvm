/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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

#ifndef BUNDLE_MGR_H
#define BUNDLE_MGR_H

#include "common/scummsys.h"
#include "common/file.h"

namespace Scumm {

class BundleMgr {

private:

	struct CompTable {
		int32 offset;
		int32 size;
		int32 codec;
	};

	struct AudioTable {
		char filename[13];
		int32 size;
		int32 offset;
	};

	CompTable *_compTable;
	AudioTable *_bundleTable;
	int32 _numFiles;
	int32 _curSample;
	File _file;

public:
	BundleMgr();
	~BundleMgr();

	bool openFile(const char *filename, const char *directory);
	void closeFile();
	int32 decompressSampleByName(const char *name, int32 offset, int32 size, byte **comp_final);
	int32 decompressSampleByIndex(int32 index, int32 offset, int32 size, byte **comp_final, int header_size);
	int32 decompressSampleByCurIndex(int32 offset, int32 size, byte **comp_final, int header_size);
};

namespace BundleCodecs {

uint32 decode12BitsSample(byte *src, byte **dst, uint32 size);
void initializeImcTables();
int32 decompressCodec(int32 codec, byte *comp_input, byte *comp_output, int32 input_size);

} // End of namespace BundleCodecs

} // End of namespace Scumm

#endif
