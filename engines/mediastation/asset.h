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

#include "common/keyboard.h"

#include "mediastation/mediastation.h"
#include "mediastation/datafile.h"
#include "mediastation/mediascript/scriptconstants.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/assetheader.h"

namespace MediaStation {

class AssetHeader;

class Asset {
public:
	Asset(AssetHeader *header) : _header(header) {};
	virtual ~Asset();

	// Does any needed frame drawing, audio playing, event handlers, etc.
	virtual void process() { return; }

	// Runs built-in bytecode methods.
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args);

	virtual bool isSpatialActor() const { return false; }
	virtual bool isActive() const { return _isActive; }

	// These are not pure virtual so if an asset doesnʻt read any chunks or
	// subfiles it doesnʻt need to just implement these with an error message.
	virtual void readChunk(Chunk &chunk);
	virtual void readSubfile(Subfile &subfile, Chunk &chunk);

	void setInactive();
	void setActive();
	void processTimeEventHandlers();
	void runEventHandlerIfExists(EventType eventType, const ScriptValue &arg);
	void runEventHandlerIfExists(EventType eventType);

	AssetType type() const { return _header->_type; }
	uint id() const { return _header->_id; }
	AssetHeader *getHeader() const { return _header; }

protected:
	AssetHeader *_header = nullptr;
	bool _isActive = false;
	uint _startTime = 0;
	uint _lastProcessedTime = 0;
	uint _duration = 0;
};

class SpatialEntity : public Asset {
public:
	SpatialEntity(AssetHeader *header) : Asset(header) {};

	virtual void redraw(Common::Rect &rect) { return; }
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;

	virtual bool isSpatialActor() const override { return true; }
	virtual bool isVisible() const { return false; }
	Common::Point getTopLeft() { return Common::Point(_header->_boundingBox.left, _header->_boundingBox.top); }
	Common::Rect getBbox() const { return _header->_boundingBox; }
	int zIndex() const { return _header->_zIndex; }

	void moveTo(int16 x, int16 y);
	void moveToCentered(int16 x, int16 y);
	void setBounds(const Common::Rect &bounds);
	void setZIndex(int zIndex);

	virtual void invalidateLocalBounds();
	virtual void invalidateLocalZIndex();
};

} // End of namespace MediaStation

#endif