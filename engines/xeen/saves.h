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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef XEEN_SAVES_H
#define XEEN_SAVES_H

#include "common/scummsys.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "graphics/surface.h"
#include "xeen/party.h"
#include "xeen/files.h"

namespace Xeen {

struct XeenSavegameHeader {
	uint8 _version;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _year, _month, _day;
	int _hour, _minute;
	int _totalFrames;
};

class XeenEngine;
class SavesManager;

class OutFile : public Common::WriteStream {
private:
	XeenEngine *_vm;
	Common::String _filename;
	Common::MemoryWriteStreamDynamic _backingStream;
public:
	OutFile(XeenEngine *vm, const Common::String filename);

	void finalize();

	uint32 write(const void *dataPtr, uint32 dataSize) override;

	int32 pos() const override;
};

class SavesManager: public BaseCCArchive {
	friend class OutFile;
private:
	XeenEngine *_vm;
	Party &_party;
	byte *_data;
	Common::HashMap<uint16, Common::MemoryWriteStreamDynamic *> _newData;

	void load(Common::SeekableReadStream *stream);
public:
	/**
	 * Synchronizes a boolean array as a bitfield set
	 */
	static void syncBitFlags(Common::Serializer &s, bool *startP, bool *endP);
public:
	bool _wonWorld;
	bool _wonDarkSide;
public:
	SavesManager(XeenEngine *vm, Party &party);

	~SavesManager();

	/**
	 * Sets up the dynamic data for the game for a new game
	 */
	void reset();

	void readCharFile();

	void writeCharFile();

	void saveChars();

	// Archive implementation
	virtual Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const;
};

} // End of namespace Xeen

#endif	/* XEEN_SAVES_H */
