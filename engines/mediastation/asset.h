/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MEDIASTATION_ASSET_H
#define MEDIASTATION_ASSET_H

#include "common/func.h"

#include "mediastation/subfile.h"
#include "mediastation/chunk.h"
#include "mediastation/mediascript/builtins.h"
#include "mediastation/mediascript/operand.h"

namespace MediaStation {

enum class AssetType;
class AssetHeader;

class Asset {
public:
	Asset(AssetHeader *header) : _header(header) {};
	virtual ~Asset();

	// Does any needed frame drawing, audio playing, event handlers, etc.
	virtual void process() {
		return;
	}
	// Runs built-in bytecode methods.
	virtual Operand callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) = 0;
	// Called to have the asset do any processing, like drawing new frames,
	// handling time-based event handlers, and such. Some assets don't have any
	// processing to do.
	virtual bool isActive() const {
		return _isActive;
	}

	// These are not pure virtual so if an asset doesnʻt read any chunks or
	// subfiles it doesnʻt need to just implement these with an error message.
	virtual void readChunk(Chunk &chunk);
	virtual void readSubfile(Subfile &subfile, Chunk &chunk);

	AssetType type() const;
	uint zIndex() const;
	AssetHeader *getHeader() const {
		return _header;
	}

protected:
	AssetHeader *_header = nullptr;
	bool _isActive = false;
	uint _startTime = 0;
	uint _lastProcessedTime = 0;
	// TODO: Rename this to indicate the time is in milliseconds.
	uint _duration = 0;
};

} // End of namespace MediaStation

#endif