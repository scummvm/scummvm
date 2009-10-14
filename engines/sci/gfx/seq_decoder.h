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

#include "common/file.h"
#include "common/rect.h"

namespace Sci {

struct SeqFrame {
	Common::Rect frameRect;
	byte colorKey;
	byte *data;
};

class ResourceManager;
class SciGuiScreen;

/**
 * Decoder for image sequences
 */
class SeqDecoder {
public:
	SeqDecoder() : _fileStream(0) { }
	~SeqDecoder();
	bool loadFile(Common::String fileName, ResourceManager *resMan, SciGuiScreen *screen);
	void closeFile();
	SeqFrame *getFrame(bool &hasNext);

private:
	bool decodeFrame(byte *runlength_data, int runlength_size,
		byte *literal_data, int literal_size, byte *dest, int xl, int yl,
		int color_key);

	Common::SeekableReadStream *_fileStream;
	int _frameCount;
	int _currentFrame;
};

} // End of namespace Sci
